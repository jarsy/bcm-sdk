/*
 * $Id: cmu.c,v 1.20.24.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    cmu.c
 * Purpose: Caladan3 CMU drivers
 * Requires:
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#include <soc/mem.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/sbx/caladan3/ocm.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <soc/sbx/caladan3/wb_db_cmu.h>

#define _SOC_CALADAN3_CMU_TIMEOUT (100000)

/* #define _SOC_CALADAN3_CMU_DEBUG */

#define RING_LOCK(unit)   sal_mutex_take(SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg.nRingLock, sal_mutex_FOREVER)
#define RING_UNLOCK(unit) sal_mutex_give(SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg.nRingLock);


static void _soc_sbx_caladan3_cmu_ring_process_thread(void *unit_vp);
static int _soc_sbx_caladan3_cmu_ring_process(int unit, int flush);
static int _soc_sbx_caladan3_cmu_ocm_alloc(int unit, int32 port, uint32 size, int32 *addr, int32 *alloc_size);

/*
 *   Function
 *     sbx_caladan3_cmu_hw_init
 *   Purpose
 *      CMU hardware initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_hw_init(int unit) 
{
    int nRv = SOC_E_NONE;
    uint32 uRegValue = 0;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;
    int nSegment;

    /*== config buffer ring, CMU is using CMC0 Channel 0 for Ejection FIFO */
    if (SOC_PCI_CMC(unit) != _SOC_CALADAN3_CMU_CMICM_CMC) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s SOC_PCI_CMC %d is not CMC0 on unit %d\n"),
                   FUNCTION_NAME(), SOC_PCI_CMC(unit), unit));
	return SOC_E_PARAM;
    }

    /* cmic max ring buffer size is 16K * 64bits, otherwise it will stall. check here */
    if (pCmuCfg->ring.nRingEntries * SOC_MEM_INFO(unit,CM_EJECTION_FIFOm).bytes >= 16*1024*8) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s ring buffer size %d is larger than supported 16K entries on unit %d\n"),
                   FUNCTION_NAME(), pCmuCfg->ring.nRingEntries, unit));
	return SOC_E_PARAM;
    }

    nRv = soc_mem_fifo_dma_start(unit, _SOC_CALADAN3_CMU_CMICM_CH, CM_EJECTION_FIFOm,
				 MEM_BLOCK_ANY, pCmuCfg->ring.nRingEntries,
				 (void *)pCmuCfg->ring.pRingPciBase);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to config CMU buffer ring on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }

    if (pCmuCfg->ring.nRingThresh > pCmuCfg->ring.nRingEntries - 256) {
	/* make sure we don't trigger interrupt too late */
	pCmuCfg->ring.nRingThresh = pCmuCfg->ring.nRingEntries - 256;
    }
    WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr(unit, pCmuCfg->ring.nRingThresh);

    /*== config segments */
    nRv = soc_sbx_caladan3_cmu_segment_verify(unit);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU segment verify failed on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }

    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
	nRv = soc_sbx_caladan3_cmu_segment_set(unit, nSegment,
						  &(pCmuCfg->segments[nSegment]));
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s failed to config CMU segment %d on unit %d\n"),
                       FUNCTION_NAME(), nSegment, unit));
	    return nRv;
	}
    }

    /*== random seeds if user provide 0 */
    if (pCmuCfg->uLFSRseed == 0) {
	pCmuCfg->uLFSRseed = (uint32)sal_rand();
    }
    WRITE_CM_STACE_LFSR_SEEDr(unit, pCmuCfg->uLFSRseed);

    /*== dual ejection */
    SOC_IF_ERROR_RETURN(READ_CM_CONTROL_REGISTERr(unit, &uRegValue));
    soc_reg_field_set(unit, CM_CONTROL_REGISTERr, &uRegValue, DUAL_EJECT_MODEf,
		      (pCmuCfg->bDualEjection)?1:0);
    SOC_IF_ERROR_RETURN(WRITE_CM_CONTROL_REGISTERr(unit, uRegValue));    
    
    /*== background ejection rate */
    if (pCmuCfg->uBGEjectRate == 0) {
	/* background ejection rate is 0, disable all background ejection */
	SOC_IF_ERROR_RETURN(WRITE_CM_BACKGROUND_EJECT_ENABLEr(unit, 0));
	SOC_IF_ERROR_RETURN(WRITE_CM_BACKGROUND_EJECT_RATEr(unit, 0xFFFFFFFF));
    } else {
	if (pCmuCfg->uBGEjectRate > 100000) {
	    /* force the background ejection rate to be less than 100KHZ */
	    pCmuCfg->uBGEjectRate = 100000;
	}

	SOC_IF_ERROR_RETURN(WRITE_CM_BACKGROUND_EJECT_RATEr(unit, 1000000000/pCmuCfg->uBGEjectRate));
    }

    /*== manual ejection */
    if (pCmuCfg->uManualEjectRate == 0) {
	/* background ejection rate is 0, disable all background ejection */
	SOC_IF_ERROR_RETURN(WRITE_CM_MANUAL_EJECT_RATEr(unit, 0xFFFFFFFF));
    } else {
	if (pCmuCfg->uManualEjectRate > 100000) {
	    /* force the background ejection rate to be less than 100KHZ */
	    pCmuCfg->uManualEjectRate = 100000;
	}

	SOC_IF_ERROR_RETURN(WRITE_CM_MANUAL_EJECT_RATEr(unit, 1000000000/pCmuCfg->uManualEjectRate));
    }
    SOC_IF_ERROR_RETURN(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue));
    soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, ENABLEf,
		      (pCmuCfg->uManualEjectRate)?1:0);
    SOC_IF_ERROR_RETURN(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue));

    /*== bring cmu out of reset */
    SOC_IF_ERROR_RETURN(READ_CM_CONTROL_REGISTERr(unit, &uRegValue));
    soc_reg_field_set(unit, CM_CONTROL_REGISTERr, &uRegValue, CM_RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CM_CONTROL_REGISTERr(unit, uRegValue));

    /*== mask out all CMU interrupts other than manual eject done. ??ECC errors?? */
    SOC_IF_ERROR_RETURN(WRITE_CM_DISABLED_SEGMENT_ERROR_MASKr(unit, 0xffffFFFF));
    SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_LIMIT_ERROR_MASKr(unit, 0xffffFFFF));
    SOC_IF_ERROR_RETURN(WRITE_CM_ERROR_STATUS_MASKr(unit, 0xffffFFFF));
    SOC_IF_ERROR_RETURN(WRITE_CM_TRACE_IF_STATUS_MASKr(unit, 0xffffFFFF));
    SOC_IF_ERROR_RETURN(WRITE_CM_ECC_ERROR_MASKr(unit, 0xffffFFFF));

    uRegValue = 0;
    soc_reg_field_set(unit, CM_INTERRUPT_STATUSr, &uRegValue, EJECTION_FIFO_READYf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CM_INTERRUPT_STATUS_MASKr(unit, uRegValue));    

    /*== enable CMICm sbus slave interrupt for CMU block */
    nRv = soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_CM_INTR_POS));
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to enable CMU block interrupt on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }

    /*== enable CMICm interrupt to handle ring buffer processing */
    nRv = soc_cmicm_intr0_enable(unit, IRQ_CMCx_FIFO_CH_DMA(SOC_MEM_FIFO_DMA_CHANNEL_0));
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to enable FIFO DMA interrupt on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;	
    }

    return nRv;
}
/*
 *   Function
 *     sbx_caladan3_cmu_init_final
 *   Purpose
 *      For sequencing purposes, need indication that CMU state is completely initialized
 *      prior to warmboot initialization.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_init_final(int unit)
{
    int nRv = BCM_E_NONE;

#ifdef BCM_WARM_BOOT_SUPPORT 
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;
    nRv = soc_sbx_caladan3_wb_cmu_state_init(unit, pCmuCfg);
#endif /* BCM_WARM_BOOT_SUPPORT */
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_driver_init
 *   Purpose
 *      Drivier initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_driver_init(int unit) 
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;
    int nSegment, nRingSize, nEntrySize;
    uint32 flags;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Caladan3 cmu driver init called on unit %d.\n"),
                 unit));

    if (pCmuCfg->bDriverInit) {
	soc_sbx_caladan3_cmu_driver_uninit(unit);
        /*return SOC_E_INIT;*/
    }

    /* default parameters */
    pCmuCfg->bDualEjection = TRUE;
    pCmuCfg->uLFSRseed = 0;            /* random seed */
    pCmuCfg->uBGEjectRate = 1000;      /* 1KHZ background ejection */
    pCmuCfg->uManualEjectRate = 10000; /* 10KHZ background ejection */
    pCmuCfg->nFlushLock = NULL;
    pCmuCfg->bFlushing = FALSE;
    pCmuCfg->ring.nRingEntries = 4096; /* default to be same as SV, might need tuning or soc property later */
    pCmuCfg->ring.pRingPciBase = NULL; /* auto-allocation later */
    pCmuCfg->ring.pRingPciRead = NULL; /* auto-allocation later */
    pCmuCfg->ring.nRingThresh = 2048;  /* default to half of ring, might need tuning or soc property later */
    pCmuCfg->ring.pid = SAL_THREAD_ERROR;  /* default to half of ring, might need tuning or soc property later */
    if (SAL_BOOT_QUICKTURN) {
        pCmuCfg->uBGEjectRate *= 50;      /* 50KHZ background ejection */
	pCmuCfg->uManualEjectRate *= 50;  /* 500KHZ background ejection */
    }

    /* CMU segment management structure init */
    pCmuCfg->segments = sal_alloc(sizeof(soc_sbx_caladan3_cmu_segment_config_t) *
				  SOC_SBX_CALADAN3_CMU_NUM_SEGMENT, "CMU segment database");
    if (pCmuCfg->segments == NULL) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to allocate CMU segment control database on unit %d\n"),
                   FUNCTION_NAME(), unit));
	soc_sbx_caladan3_cmu_driver_uninit(unit);
	return SOC_E_MEMORY;
    }
    sal_memset(pCmuCfg->segments, 0, sizeof(soc_sbx_caladan3_cmu_segment_config_t) *
	       SOC_SBX_CALADAN3_CMU_NUM_SEGMENT);
    

    /* for now, all segments default to be disabled */
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
	pCmuCfg->segments[nSegment].bEnabled = FALSE;
	pCmuCfg->segments[nSegment].nSegmentPort = 0;
	pCmuCfg->segments[nSegment].nSegmentOcmBase = 0;
	pCmuCfg->segments[nSegment].nSegmentOcmSize = 0;
	pCmuCfg->segments[nSegment].nSegmentType = 0;
	pCmuCfg->segments[nSegment].pSegmentPciBase = NULL;
	pCmuCfg->segments[nSegment].bBGEjectEnabled = FALSE;
	pCmuCfg->segments[nSegment].bSTACEEnabled = TRUE;
	pCmuCfg->segments[nSegment].nSegmentLimit = 0;
    }

    /* ring buffer */
    nRingSize = 0;
    if (pCmuCfg->ring.pRingPciBase == NULL) {
	/* allocate ring buffer in case caller has not done that */
	nEntrySize = SOC_MEM_INFO(unit,CM_EJECTION_FIFOm).bytes;
	nRingSize = pCmuCfg->ring.nRingEntries * nEntrySize;
	pCmuCfg->ring.pRingPciBase = soc_cm_salloc(unit, nRingSize, "CMU ring buffer");
	if (pCmuCfg->ring.pRingPciBase == NULL) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s failed to allocat %d bytes for CMU ring buffer on unit %d\n"),
                       FUNCTION_NAME(), nRingSize, unit));
	    soc_sbx_caladan3_cmu_driver_uninit(unit);
	    return SOC_E_MEMORY;
	} else {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "%s: ring buffer set at 0x%x on unit %d\n"),
	                 FUNCTION_NAME(), (uint32)pCmuCfg->ring.pRingPciBase, unit));
	}
	/* point the read pointer to the base */
	pCmuCfg->ring.pRingPciRead = pCmuCfg->ring.pRingPciBase;
    }
    sal_memset(pCmuCfg->ring.pRingPciBase, 0, nRingSize);

    /* create lock for forced flush */
    if (pCmuCfg->nFlushLock == NULL) {
	pCmuCfg->nFlushLock = sal_mutex_create("CMU flush lock");
	if (!pCmuCfg->nFlushLock) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Failed to create mutex for CMU flush on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    soc_sbx_caladan3_cmu_driver_uninit(unit);
	    return SOC_E_MEMORY;
	}
    }

    /* create lock for ring buffer processing */
    if (pCmuCfg->nRingLock == NULL) {
	pCmuCfg->nRingLock = sal_mutex_create("CMU ring lock");
	if (!pCmuCfg->nRingLock) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Failed to create mutex for CMU ring on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    soc_sbx_caladan3_cmu_driver_uninit(unit);
	    return SOC_E_MEMORY;
	}
    }

    /* init flush watermark */
    pCmuCfg->uCMUWatermark = _SOC_CALADAN3_CMU_TIMEOUT;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* hardware init */
    if (!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */
  
        nRv = soc_sbx_caladan3_cmu_hw_init(unit);
        if (SOC_FAILURE(nRv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Hardware init failed %d \n"),
                       FUNCTION_NAME(), unit, nRv));
            soc_sbx_caladan3_cmu_driver_uninit(unit);
            return nRv;
        }
#ifdef BCM_WARM_BOOT_SUPPORT
    }
#endif /* BCM_WARM_BOOT_SUPPORT */
  
    /* start up the ring processing thread */
    /* default to 1ms, could accumulate up to 504 Entry given max ejection rate
     * is around 504KHz based on analysis in the CMU spec
     */
    pCmuCfg->ring.interval = MILLISECOND_USEC;
    if (SAL_BOOT_QUICKTURN) {
	pCmuCfg->ring.interval *= 500; /* default to 500ms on quickturn */
    }

    if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {
        flags = 0; /* flags are not used for now */
#ifdef BCM_WARM_BOOT_SUPPORT
        if (!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */
            nRv = soc_sbx_caladan3_cmu_ring_process_thread_start(unit, flags, pCmuCfg->ring.interval);
            if (SOC_FAILURE(nRv)) {
                return nRv;
            }
#ifdef BCM_WARM_BOOT_SUPPORT
        }
#endif /* BCM_WARM_BOOT_SUPPORT */
    }

    pCmuCfg->bDriverInit = TRUE;
    return nRv;
}


/*
 *   Function
 *     sbx_caladan3_cmu_hw_uninit
 *   Purpose
 *      CMU hardware uninitializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_hw_uninit(int unit) 
{
    int nRv = SOC_E_NONE;

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_driver_uninit
 *   Purpose
 *      CMU driver un-initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_driver_uninit(int unit)

{
    int32 nSegment;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;

   
    if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {
        /* stop the ring processing thread */
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_cmu_ring_process_thread_stop(unit));
    }

    /* free dma ring memory */
    if (pCmuCfg->ring.pRingPciBase != NULL) {
	soc_cm_sfree(unit, pCmuCfg->ring.pRingPciBase);
	pCmuCfg->ring.pRingPciBase = NULL;
	pCmuCfg->ring.pRingPciRead = NULL;
    }

    /* segments database */
    if (pCmuCfg->segments != NULL) {
	/* walk used segments and deallocate any associated memory */ 
	for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
	    if ((pCmuCfg->segments[nSegment].bEnabled == TRUE) &&
		(pCmuCfg->segments[nSegment].pSegmentPciBase != NULL)) {
		sal_free(pCmuCfg->segments[nSegment].pSegmentPciBase);
		pCmuCfg->segments[nSegment].pSegmentPciBase = NULL;
	    }
	}
	sal_free(pCmuCfg->segments);
	pCmuCfg->segments = NULL;
    }    

    if (pCmuCfg->ring.trigger != NULL) {
	sal_sem_destroy(pCmuCfg->ring.trigger);
        pCmuCfg->ring.trigger = NULL;
    }

    /* deallocate locks */
    if (pCmuCfg->nFlushLock != NULL) {
	sal_mutex_destroy(pCmuCfg->nFlushLock);
	pCmuCfg->nFlushLock = NULL;
    }

    if (pCmuCfg->nRingLock != NULL) {
	sal_mutex_destroy(pCmuCfg->nRingLock);
	pCmuCfg->nRingLock = NULL;
    }

    pCmuCfg->bDriverInit = FALSE;

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cmu_ring_processing_wakeup
 *   Purpose
 *      Wakeup CMU ring-processing thread for fifo dma overflow/timeout interrupt 
 *       or manual flush done interrupt
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) d1-4   : not used
 *   Returns
 *      VOID
 */
void soc_sbx_caladan3_cmu_ring_processing_wakeup(void *unit_vp,
						 void *d1, void *d2, void *d3, void *d4)
{
    int unit = PTR_TO_INT(unit_vp);
    soc_sbx_caladan3_cmu_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg.ring);

    /* wake up ring processing thread */
    if (pRing->trigger) {
	sal_sem_give(pRing->trigger);
    }
}

/*
 *   Function
 *     _sbx_caladan3_cmu_ring_process_thread
 *   Purpose
 *      CMU ring buffer processing thread
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
static void _soc_sbx_caladan3_cmu_ring_process_thread(void *unit_vp)
{
    int unit = PTR_TO_INT(unit_vp);
    int uRegValue;
    int nRv = SOC_E_NONE;
    int nWakeup;
    soc_sbx_caladan3_cmu_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg.ring);

    /* wake up periodically to process the host ring buffer */
    while (pRing->interval != 0) {
        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_sbx_scoreboard_stop wants it to.
         */
#ifdef _SOC_CALADAN3_CMU_DEBUG
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: sleep %d on unit %d\n"),
                   FUNCTION_NAME(), pRing->interval, unit));
#endif

	nWakeup = sal_sem_take(pRing->trigger, pRing->interval);

	if (pRing->interval == 0) {
            break;
        }

#ifdef _SOC_CALADAN3_CMU_DEBUG
	if (nWakeup >= 0) {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "%s: waked up by interrupts on unit %d\n"),
	                 FUNCTION_NAME(), unit));

	} else {
	    LOG_DEBUG(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "%s: waked up by timer interval %d us on unit %d\n"),
	               FUNCTION_NAME(), pRing->interval, unit));
	}
#endif /* _SOC_CALADAN3_CMU_DEBUG */

#ifdef BCM_WARM_BOOT_SUPPORT
        /* Only process if we are not in warmboot - need to wait until complete */
        if (SOC_WARM_BOOT(unit)) {
            continue;
        }
#endif  

        /* process the ring */
        nRv = _soc_sbx_caladan3_cmu_ring_process(unit, FALSE);
        if (SOC_FAILURE(nRv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s failed to process ring buffers on unit %d\n"),
                       FUNCTION_NAME(), unit));
            break;
        }


        /* Pause during fast reconfig to prevent physical to logical mapping from being corrupted */
        while (SOC_RECONFIG_TDM) {
            sal_usleep(100);
        }

	/* fatal error handling */
	uRegValue = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(_SOC_CALADAN3_CMU_CMICM_CMC,
									     _SOC_CALADAN3_CMU_CMICM_CH));
	if (soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr, uRegValue, DONEf)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FIFO DMA engine terminated for cmc[%d]:ch[%d]\n"), 
                       _SOC_CALADAN3_CMU_CMICM_CMC, _SOC_CALADAN3_CMU_CMICM_CH));
	    if (soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr, uRegValue, ERRORf)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "FIFO DMA engine encountered error: [0x%x]\n"), uRegValue));
	    }
	    
	    nRv = SOC_E_INTERNAL;
	    break;
	} 

	/* reenable interrupts */
	if (nWakeup >= 0) {
	    /* manual flush done interrupt reenable */
	    soc_cmicm_intr3_enable(unit, 1<<SOC_SBX_CALADAN3_CM_INTR_POS);

	    /* fifo dma overflow/timeout interrupt reenable */
	    soc_cmicm_intr0_enable(unit, IRQ_CMCx_FIFO_CH_DMA(SOC_MEM_FIFO_DMA_CHANNEL_0));
	}
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: exit with status 0x%x interval %d on unit %d\n"),
                 FUNCTION_NAME(), nRv, pRing->interval, unit));

    /* stop the CMU fifo dma channel
     *  (void)soc_mem_fifo_dma_stop(unit, _SOC_CALADAN3_CMU_CMICM_CH);
     * if the thread is stopped, don't stop the channel, rely on interrupts to handle it
     */
    pRing->pid = SAL_THREAD_ERROR;
    pRing->interval  = 0;

    sal_thread_exit(0); 
}

/*
 *   Function
 *     sbx_caladan3_cmu_ring_process
 *   Purpose
 *      Process entries from the CMU ring counter structure. This will empty
 *      entries from the ring update structure and synchronize their values
 *      with the shadowed 64-bit host-side counters. This needs to be called
 *      periodically, either by interrupt or polling to guarantee the ring
 *      has room for the CMU to operate.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) flush  : flush 
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
#define _SOC_CALADAN3_CMU_MESSAGE_LAST_FLUSH    (1<<0)
#define _SOC_CALADAN3_CMU_MESSAGE_RANGE_TRACKER (1<<1)
static int _soc_sbx_caladan3_cmu_ring_process(int unit, int flush)
{
    int nRv = SOC_E_NONE;
    int32 nCount, nLoop, i;
    int64 *pData;
    uint32 uD0=0, uD1=0;
    uint32 uRegValue = 0;
    int32 nFlushDone = FALSE;
    int32 nEntrySize = SOC_MEM_INFO(unit,CM_EJECTION_FIFOm).bytes / sizeof(uint32);
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    
    soc_sbx_caladan3_cmu_ring_config_t *pRing = &(pCmuCfg->ring);
    int32 *pRingPciEnd = pRing->pRingPciBase + (pRing->nRingEntries * nEntrySize);
    uint32 uOverflow, uTimeout;
    int   nCmc = _SOC_CALADAN3_CMU_CMICM_CMC;
    uint8 uCh = _SOC_CALADAN3_CMU_CMICM_CH;

    /* make sure ring is inited */
    if (pRing->pRingPciRead == NULL) {
	return SOC_E_INIT;
    }

    /* ring process thread (periodic), ring almost full interrupt from CMC and 
     * manual flush done interrupt from CMU can all trigger this call.
     * use mutex to protect it.
     */

    RING_LOCK(unit);

    /* loop 3 times at most to prevent tight loop, wait for next wakeup */
    for (nLoop = 3; nLoop > 0; nLoop--) {
	/* get entry count, process if nonzero else break */
	nRv = soc_mem_fifo_dma_get_num_entries(unit, uCh, &nCount);
	if (SOC_FAILURE(nRv)) {
	    if (nRv == SOC_E_EMPTY) {
		nRv = SOC_E_NONE;
	    }
	    break;
	}

	/* invalidate cache for these entries */
	if (pRing->pRingPciRead + nCount * nEntrySize > pRingPciEnd) {
	    soc_cm_sinval(unit, pRing->pRingPciRead, (pRingPciEnd - pRing->pRingPciRead)*sizeof(uint32));
	    soc_cm_sinval(unit, pRing->pRingPciBase, (nCount*nEntrySize - (pRingPciEnd - pRing->pRingPciRead))*sizeof(uint32));
	} else {
	    soc_cm_sinval(unit, pRing->pRingPciRead, nCount*nEntrySize*sizeof(uint32));
	}
	
	/* process entries */
	for (i = 0; i < nCount; i++) {
	    /* process the entry */
	    uD0 = *(pRing->pRingPciRead);
	    uD1 = *(pRing->pRingPciRead+1);
	    
	    pData = (int64 *)(uD0 & 0xffffFFF8);
	    
#ifdef _SOC_CALADAN3_CMU_DEBUG
            if (uD1 !=0) {
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s ring message 0x%x 0x%x at 0x%x on unit %d.\n"),
                  FUNCTION_NAME(), uD0, uD1, (uint32)pRing->pRingPciRead, unit));
            }
#endif /* _SOC_CALADAN3_CMU_DEBUG */
	    
	    /* update counter */
	    if ( (!SOC_WARM_BOOT(unit)) && (!flush) ) {
		if (uD0 & _SOC_CALADAN3_CMU_MESSAGE_RANGE_TRACKER) {
		    /* replace instead of accumulate */
		    COMPILER_64_SET(*pData, 0, uD1);
		} else {
                    if (uD0 != 0) {
                        COMPILER_64_ADD_32(*pData, uD1);
                    } else {
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,"%s Null pointer in CMU ring, ignore entry on unit %d\n"),
                                     FUNCTION_NAME(), unit));
                    }
		}
	    }
	    
	    /* check the L bit for manual flushing, there should be only 1 manual flush
	     * in progress at anytime
	     */
	    if (uD0 & _SOC_CALADAN3_CMU_MESSAGE_LAST_FLUSH) {
		pCmuCfg->bFlushing = FALSE;
		nFlushDone = TRUE;
	    }
	    
	    /* advance the read pointer */
	    pRing->pRingPciRead += nEntrySize;
	    if (pRing->pRingPciRead >= pRingPciEnd) {
		pRing->pRingPciRead = pRing->pRingPciBase;
	    }
	}

	/* set HW number of entries read */
	nRv = soc_mem_fifo_dma_set_entries_read(unit, uCh, nCount);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not set entries read for CMU fifo dma on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    RING_UNLOCK(unit);
	    return nRv;
	}

#ifdef _SOC_CALADAN3_CMU_DEBUG
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s CMU ring thread processed %d enries on unit %d.\n"),
	             FUNCTION_NAME(), nCount, unit));
#endif /* _SOC_CALADAN3_CMU_DEBUG */
    }
    
    /* check and clear the manual flushing done interrupt in case it happens 
     * before the L bit being processed 
     */
    if (nFlushDone || pCmuCfg->bFlushing) {
	nRv = READ_CM_INTERRUPT_STATUSr(unit, &uRegValue);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to read CMU interrupt status on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    RING_UNLOCK(unit);
	    return nRv;
	}
	if (soc_reg_field_get(unit, CM_INTERRUPT_STATUSr, uRegValue, MANUAL_EJECT_DONEf)) {
	    WRITE_CM_INTERRUPT_STATUSr(unit, 1);
	}
    }
    
    /* Pause during fast reconfig to prevent physical to logical mapping from being corrupted */
    while (SOC_RECONFIG_TDM) {
        sal_usleep(100);
    }
    
    /* any errors? */
    uRegValue = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(nCmc, uCh));
    uOverflow = soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr,
				  uRegValue, HOSTMEM_TIMEOUTf);
    if (uOverflow) {
	uTimeout = soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr,
				     uRegValue, HOSTMEM_OVERFLOWf);
	uOverflow |= uTimeout;
    }

    /* errors handling */
    if (uOverflow) {
	/* clear errors */
	uRegValue = 0;
	soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STAT_CLRr, &uRegValue, 
			  HOSTMEM_OVERFLOWf, 1);
	soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STAT_CLRr, &uRegValue, 
			  HOSTMEM_TIMEOUTf, 1);
	soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_CLR_OFFSET(nCmc, uCh), 
		      uRegValue);    
#ifdef _SOC_CALADAN3_CMU_DEBUG
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s CMU FIFO DMA cleared overflow/timeout errors on unit %d.\n"),
	             FUNCTION_NAME(), unit));
#endif /* _SOC_CALADAN3_CMU_DEBUG */
    }

    RING_UNLOCK(unit);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_ring_process_thread_start
 *   Purpose
 *      start CMU ring buffer processing thread
 *   Parameters
 *      (IN) unit     : unit number of the device
 *      (IN) flags    : flags (not used for now)
 *      (IN) interval : thread running interval in us
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_ring_process_thread_start(int unit, uint32 flags, sal_usecs_t interval)
{
    int nPri;
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    
    soc_sbx_caladan3_cmu_ring_config_t *pRing = &(pCmuCfg->ring);
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on unit %d.\n"),
                 FUNCTION_NAME(), unit));
    
    sal_snprintf(pRing->name, sizeof(pRing->name), "socCMUring.%d", unit);
    
    pRing->flags = flags;
    
    if (interval == 0) {
        return SOC_E_NONE;
    }
    
    /* Create semaphores to trigger */
    if (pRing->trigger == NULL) {
        pRing->trigger = sal_sem_create("cmu_ring_trigger",
                                        sal_sem_BINARY, 0);
        if (pRing->trigger == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not create CMU ring thread trigger semaphore on unit %d\n"),
                       FUNCTION_NAME(), unit));
            return SOC_E_MEMORY;
        }
    }
    
    /* check that it's using right CMC */
    if (SOC_PCI_CMC(unit) != _SOC_CALADAN3_CMU_CMICM_CMC) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s SOC_PCI_CMC %d is not CMC0 on unit %d\n"),
                   FUNCTION_NAME(), SOC_PCI_CMC(unit), unit));
        return SOC_E_PARAM;
    }
    
    /* config buffer ring, CMU is using CMC0 Channel 0 for Ejection FIFO */
    nRv = soc_mem_fifo_dma_start(unit, _SOC_CALADAN3_CMU_CMICM_CH, CM_EJECTION_FIFOm,
                                 MEM_BLOCK_ANY, pCmuCfg->ring.nRingEntries,
                                 (void *)pCmuCfg->ring.pRingPciBase);
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to config CMU buffer ring on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return nRv;
    }
    
    if (pCmuCfg->ring.nRingThresh > pCmuCfg->ring.nRingEntries - 256) {
        /* make sure we don't trigger interrupt too late */
        pCmuCfg->ring.nRingThresh = pCmuCfg->ring.nRingEntries - 256;
    }
    WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr(unit, pCmuCfg->ring.nRingThresh);
    
    if (pRing->interval != 0) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_cmu_ring_process_thread_stop(unit));
    }
    
    /* start up the thread */    
    pRing->interval = interval;
    if (pRing->pid == SAL_THREAD_ERROR) {
        nPri = soc_property_get(unit, spn_LINKSCAN_THREAD_PRI, -1);
        if (nPri < 0) {
            /* default to 60 */
            nPri = SOC_SBX_CALADAN3_CMU_RING_THREAD_DEFAULT_PRI;
        } else {
            /* lower priority than linkscan */
            nPri += 10;
        }
        
        pRing->pid = sal_thread_create(pRing->name, SAL_THREAD_STKSZ, nPri,
                                       _soc_sbx_caladan3_cmu_ring_process_thread,
                                       INT_TO_PTR(unit));
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s, unit %d, CMU thread - pid:%d\n"),
                     FUNCTION_NAME(), unit, (int)(pRing->pid)));
        if (pRing->pid == SAL_THREAD_ERROR) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not start CMU ring process thread on unit %d\n"),
                       FUNCTION_NAME(), unit));
            pRing->interval = 0;
            return SOC_E_MEMORY;
        }
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cmu_ring_process_thread_stop
 *   Purpose
 *      stop CMU ring buffer processing thread
 *   Parameters
 *      (IN) unit     : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_ring_process_thread_stop(int unit)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg.ring);
    sal_thread_t   sSamplePid;
    sal_usecs_t    sTimeout;
    soc_timeout_t  sTo;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on unit %d.\n"),
                 FUNCTION_NAME(), unit));

    if (pRing->interval != 0) {

	/* wait for ten times ring process interval */
        sTimeout = pRing->interval * 10;

        /*
         * Signal by setting interval to 0, and wake up thread to speed
         * its exit.  It may also be waiting for the hardware interrupt
         * semaphore.  Wait a limited amount of time for it to exit.
         */
        pRing->interval = 0;

        sal_sem_give(pRing->trigger);

        soc_timeout_init(&sTo, sTimeout, 0);
        while ((sSamplePid = pRing->pid) != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&sTo)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_cmu_ring_process_thread_stop:"
                                      "thread did not exit\n")));
                pRing->pid = SAL_THREAD_ERROR;
                nRv = SOC_E_INTERNAL;
                break;
            }

            sal_usleep(10000);
        }
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s unit %d, CMU thread stopped\n"),
                 FUNCTION_NAME(), unit));
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_set
 *   Purpose
 *      CMU segment config
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) segment: segment ID
 *      (IN) config : pointer to segment config parameters
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_segment_set(int unit,
				     int segment,
				     soc_sbx_caladan3_cmu_segment_config_t *config)
{
    int nRv = SOC_E_NONE;    
    uint32 uRegValue = 0;
    uint32 uOcmBase, uLimit, uType;
    void *pBuf;

    if (((int)segment < 0) || (segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* sanity check segment type is valid */
    if ((config->nSegmentType != SOC_SBX_API_PARAM_NO_CHANGE) &&
	!SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_VALID(config->nSegmentType)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment type %d for segment %d on unit %d\n"),
                   FUNCTION_NAME(), config->nSegmentType, segment, unit));
	return SOC_E_PARAM;
    }

    /* sanity check segment port is valid */
    if ((config->nSegmentPort != SOC_SBX_API_PARAM_NO_CHANGE) &&
	!((config->nSegmentPort >=0 ) &&
	  (config->nSegmentPort < SOC_SBX_CALADAN3_CMU_NUM_OCM_PORT))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment OCM port %d for segment %d on unit %d\n"),
                   FUNCTION_NAME(), config->nSegmentPort, segment, unit));
	return SOC_E_PARAM;
    }

    /* sanity check segment OCM base and counter ID (limit) is valid */
    if ((config->nSegmentOcmBase != SOC_SBX_API_PARAM_NO_CHANGE) ||
	(config->nSegmentLimit != SOC_SBX_API_PARAM_NO_CHANGE)   ||
	(config->nSegmentType != SOC_SBX_API_PARAM_NO_CHANGE)) {
	/* make sure the counter ID don't overflow the OCM memory */
	if (config->nSegmentOcmBase == SOC_SBX_API_PARAM_NO_CHANGE) {
	    SOC_IF_ERROR_RETURN(READ_CM_SEGMENT_TABLE_OCM_BASEr(unit, segment, &uOcmBase));
	} else if ((config->nSegmentOcmBase < 0) ||
		   (config->nSegmentOcmBase > 0xFFFFF)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s invalid CMU segment OCM base 0x%x for segment %d on unit %d\n"),
                       FUNCTION_NAME(), config->nSegmentPort, segment, unit));
	    return SOC_E_PARAM;
	} else {
	    /* SOC_MEM_FIELD32_VALUE_MAX macro is too heavy, hardcode for now */
	    uOcmBase = config->nSegmentOcmBase;
	}

	if (config->nSegmentLimit == SOC_SBX_API_PARAM_NO_CHANGE) {
	    SOC_IF_ERROR_RETURN(READ_CM_SEGMENT_TABLE_LIMITr(unit, segment, &uLimit));
	} else if ((config->nSegmentLimit < 0) ||
		   (config->nSegmentLimit > 0x1FFFFF)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s invalid CMU segment limit 0x%x for segment %d on unit %d\n"),
                       FUNCTION_NAME(), config->nSegmentLimit, segment, unit));
	    return SOC_E_PARAM;
	} else {
	    uLimit = config->nSegmentLimit;
	}

	if (config->nSegmentType == SOC_SBX_API_PARAM_NO_CHANGE) {
	    SOC_IF_ERROR_RETURN(READ_CM_SEGMENT_TABLE_CONFIGr(unit, segment, &uRegValue));
	    uType = soc_reg_field_get(unit, CM_SEGMENT_TABLE_CONFIGr, uRegValue, Tf);
	} else {
	    uType = config->nSegmentType;
	}

	if ((uOcmBase + (uLimit >> (uType & 0x1))) > 0x1FFFFF) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s CMU segment %d base 0x%x can't have %d counter on unit %d\n"),
                       FUNCTION_NAME(), segment, config->nSegmentOcmBase,
                       config->nSegmentLimit+1, unit));
	    return SOC_E_PARAM;
	}
    }

    /* sanity check PCI address */
    if ((config->bEnabled == TRUE) && (config->pSegmentPciBase == NULL)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU segment %d pci base 0x%x not valid on unit %d\n"),
                   FUNCTION_NAME(), segment, (uint32)config->pSegmentPciBase, unit));
	return SOC_E_PARAM;	
    }

    /* segment OCM base */
    if (config->nSegmentOcmBase != SOC_SBX_API_PARAM_NO_CHANGE) {
	SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_TABLE_OCM_BASEr(unit, segment,
							     config->nSegmentOcmBase));
    }
    
    /* segment limit */
    if (config->nSegmentLimit != SOC_SBX_API_PARAM_NO_CHANGE) {
	SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_TABLE_LIMITr(unit, segment,
							  config->nSegmentLimit));
    }

    /* segment type and port */
    SOC_IF_ERROR_RETURN(READ_CM_SEGMENT_TABLE_CONFIGr(unit, segment, &uRegValue));
    if (config->nSegmentType != SOC_SBX_API_PARAM_NO_CHANGE) {
	soc_reg_field_set(unit, CM_SEGMENT_TABLE_CONFIGr, &uRegValue, Tf, config->nSegmentType);
    }
    if (config->nSegmentPort != SOC_SBX_API_PARAM_NO_CHANGE) {
	soc_reg_field_set(unit, CM_SEGMENT_TABLE_CONFIGr, &uRegValue, PORTf, config->nSegmentPort);
    }
    if (config->bSTACEEnabled != SOC_SBX_API_PARAM_NO_CHANGE) {
	soc_reg_field_set(unit, CM_SEGMENT_TABLE_CONFIGr, &uRegValue, DISABLE_STACEf,
			  (config->bSTACEEnabled)?0:1);
    }
    SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_TABLE_CONFIGr(unit, segment, uRegValue));

    /* segment pci base address */
    if ((int32)config->pSegmentPciBase != SOC_SBX_API_PARAM_NO_CHANGE) {
	/* make sure pci base address is 64-bits aligned */
	pBuf = (char*)config->pSegmentPciBase + 7;
	pBuf = (void*)((uint32)pBuf & ~0x7);	
	SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_TABLE_PCI_BASEr(unit, segment,
							     (uint32)pBuf));
    }

    /* segment enable */
    if (config->bEnabled != SOC_SBX_API_PARAM_NO_CHANGE) {
	SOC_IF_ERROR_RETURN(READ_CM_SEGMENT_ENABLEr(unit, &uRegValue));
	if (config->bEnabled) {
	    uRegValue |= (1 << segment);
	} else {
	    uRegValue &= (~(1 << segment));
	}
	SOC_IF_ERROR_RETURN(WRITE_CM_SEGMENT_ENABLEr(unit, uRegValue));
    }

    /* segment background eject enable */
    if (config->bBGEjectEnabled != SOC_SBX_API_PARAM_NO_CHANGE) {
	SOC_IF_ERROR_RETURN(READ_CM_BACKGROUND_EJECT_ENABLEr(unit, &uRegValue));
	if (config->bBGEjectEnabled) {
	    uRegValue |= (1 << segment);
	} else {
	    uRegValue &= (~(1 << segment));
	}
	SOC_IF_ERROR_RETURN(WRITE_CM_BACKGROUND_EJECT_ENABLEr(unit, uRegValue));
    }

    /* flush the segment if enabled and a range tracker */
    if ((config->bEnabled) && (config->nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_RANGE)) {
	nRv = soc_sbx_caladan3_cmu_segment_clear(unit, segment);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to clear CMU segment %d on unit %d\n"),
                       FUNCTION_NAME(), segment, unit));
	}
    }
    
    return nRv;
}			     

/*
 *   Function
 *     sbx_caladan3_cmu_counter_read
 *   Purpose
 *      Read CMU counter within a segment.
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) segment : Memory segment to read from
 *      (IN) start   : Which entry/entries to read
 *      (IN) size    : Number of entries to read
 *      (IN) data    : Pointer to array for return results
 *      (IN) sync    : Force flush synchronization
 *      (IN) clear   : Clear
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 *   NOTE:
 *       Range Tracker read always reset the range tracker
 */
int soc_sbx_caladan3_cmu_counter_read(int unit,
                                      uint32 segment,
                                      uint32 start,
                                      uint32 size,
                                      uint64 *data,
                                      uint8  sync,
                                      uint8  clear)
{
    int nRv = SOC_E_NONE;
    uint32 uTimeout = _SOC_CALADAN3_CMU_TIMEOUT/100;
    uint64 *pBufStart;
    uint64 *pBufStart_align;
    uint32 uRegValue, uBufSize;
    uint32 bMutex = FALSE;
    int nOcmIndexMin, nOcmIndexMax;
    int wordIdx;
    uint32 *pWordData;
    uint32 *pWordBufStart;
    sbx_caladan3_ocm_port_e_t eOcmPort;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    

    /* simple 64bits counter can not be ejected, and should never be cleared.
     * read directly from hardware
     */
    if (((int)segment < 0) || (segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    if (pCmuCfg->uManualEjectRate == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU manual ejection is not enabled on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_PARAM;
    }

    if (pCmuCfg->segments[segment].bEnabled == FALSE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s The flushed CMU segment %d is not enabled on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;	
    }

    if ((size == 0) || (start+size-1) > pCmuCfg->segments[segment].nSegmentLimit) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s counter ID flushed our of range of CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;	
    }

    if (pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_SIMPLE_64B) {
	/* simple 64bits counter, manual flush will only clear the counter, the only way
	 * to get the counter is read OCM memory directly.
	 */
	eOcmPort = (pCmuCfg->segments[segment].nSegmentPort)?\
	    (SOC_SBX_CALADAN3_OCM_CMU1_PORT):(SOC_SBX_CALADAN3_OCM_CMU0_PORT);
	nOcmIndexMin = pCmuCfg->segments[segment].nSegmentOcmBase + start;
	nOcmIndexMax = nOcmIndexMin + size - 1;
	pBufStart_align = (uint64 *)((char *)pCmuCfg->segments[segment].pSegmentPciBase + 7);
	pBufStart_align = (void *) ((uint32)pBufStart_align & ~0x7);
	pBufStart = (uint64 *)pBufStart_align + start;
	uBufSize = size * sizeof(uint64);

	nRv = soc_sbx_caladan3_ocm_port_mem_read(unit, eOcmPort, -1, nOcmIndexMin, nOcmIndexMax,
						 (uint32 *)pBufStart);

	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to read segment %d counter %d to %d "
                                  "through OCM on unit %d\n"),
                       FUNCTION_NAME(), segment, start, start+size-1, unit));
	    return nRv;	    
	} else {
	    /* Original word0 should be LSB, word1 should be MSB.
	    * Handle the endian here to be coherent with other counter type.
	    * Word0 changes to be MSB, 
	    * Word1 changes to be LSB here.
	    */
	    pWordBufStart = (uint32 *)pBufStart;
        pWordData = (uint32 *)data;
	    for(wordIdx = 0; wordIdx < size * 2; wordIdx+=2) {
            pWordData[wordIdx] = pWordBufStart[wordIdx + 1];
            pWordData[wordIdx + 1] = pWordBufStart[wordIdx];
        }
	}

	/* for now, we don't allow user to clear the 64 bits simple counter. Doing that might lead
	 * to loss of packet count, and 64 bits counter won't wrap around in decades.
	 * we can clear the 64 bits simple counter simply by flush it and not waiting for the
	 * the flush done (since it will not generate an eject message).
	 *    if (!clear) {
	 *        return nRv;
	 *    }
	 */
	return nRv;
    }

    /* start the manual flush */
    if ((sync) && (!SOC_WARM_BOOT(unit))) {
	FLUSH_LOCK(unit);
	pCmuCfg->bFlushing = TRUE;
	nRv = soc_sbx_caladan3_cmu_segment_manual_flush(unit, segment, start, size);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s CMU manual flush already in progress on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    FLUSH_UNLOCK(unit);
	    return nRv;
	} else {
	    bMutex = TRUE;
	}
    }
    
    /* wait for flush to be done and processed by cmu counter thread */
    while (pCmuCfg->bFlushing && uTimeout && sync) {
	sal_usleep(10 * MILLISECOND_USEC);
	uTimeout--;
    }
    
    /* record watermark if taking longer */
    if (pCmuCfg->uCMUWatermark > uTimeout) {
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "CMU manual flush watermark is 0x%x on unit %d.\n"),
	             uTimeout, unit));
	pCmuCfg->uCMUWatermark = uTimeout;
    }
    
    /* handle timeout */
    if (uTimeout == 0) {
        nRv = READ_CM_INTERRUPT_STATUSr(unit, &uRegValue);
        if (nRv != BCM_E_NONE) {
            if (bMutex) {
                FLUSH_UNLOCK(unit);
            }
            return nRv;
        }
	if (soc_reg_field_get(unit, CM_INTERRUPT_STATUSr, uRegValue, MANUAL_EJECT_DONEf)) {
	    /* CMU completed manual flush but ring process was not done 
	     * in time, return value from last successful flush
	     */
	    LOG_WARN(BSL_LS_SOC_COMMON,
	             (BSL_META_U(unit,
	                         "WARNING: did not receive CMU manual eject complete "
	                         " event, but done bit set on unit %d.\n"), unit));
	    
	    nRv = SOC_E_TIMEOUT;
	} else {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s CMU flush timed out on unit %d\n"),
                       FUNCTION_NAME(), unit));
            if (bMutex) {
                FLUSH_UNLOCK(unit);
            }
	    return SOC_E_FAIL;
	}
    }
    
    /* use ring lock since ring process might update counters */
    RING_LOCK(unit);

    pBufStart_align = (uint64 *)((char *)pCmuCfg->segments[segment].pSegmentPciBase + 7);
    pBufStart_align = (void *) ((uint32)pBufStart_align & ~0x7);

    /* copy data, if ring process ran after flush and updated counter again, use it */
    if ((pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_64B) ||
	(pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_32B)) {
        pBufStart = (uint64 *)pBufStart_align + (start *2);
	uBufSize = size * sizeof(uint64) * 2;
    } else {
	pBufStart = (uint64 *)pBufStart_align + start;
	uBufSize = size * sizeof(uint64);
    }
    sal_memcpy(data, pBufStart, uBufSize);

    /* clear as requested */
    if (clear) {
	sal_memset(pBufStart, 0, uBufSize);
    }

    RING_UNLOCK(unit);

    /* release mutex */
    if (bMutex) {
	FLUSH_UNLOCK(unit);
    }

    return nRv;
}
/*
 *   Function
 *     sbx_caladan3_cmu_segment_flush_all
 *   Purpose
 *      Flush all counters within a segment
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) segment : Memory segment to read from
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 *   NOTE:
 *       Only called from Warmboot module
 */
int soc_sbx_caladan3_cmu_segment_flush_all(int unit,
                                           uint32 segment)
{
    int nRv = SOC_E_NONE;
    uint32 uTimeout = _SOC_CALADAN3_CMU_TIMEOUT/100;
    uint32 uRegValue;
    uint32 bMutex = FALSE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;
    uint32 start = 0;
    uint32 size;
    uint32 sync = TRUE;

    size = pCmuCfg->segments[segment].nSegmentLimit;

    /* simple 64bits counter can not be ejected, and should never be cleared.
     * read directly from hardware
     */
    if (((int)segment < 0) || (segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;
    }
    
    if (pCmuCfg->uManualEjectRate == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU manual ejection is not enabled on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    if (pCmuCfg->segments[segment].bEnabled == FALSE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s The flushed CMU segment %d is not enabled on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;	
    }
    
    if (pCmuCfg->segments[segment].nSegmentType != SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_SIMPLE_64B) {
    
        /* start the manual flush */
        if (sync) {
            FLUSH_LOCK(unit);
            pCmuCfg->bFlushing = TRUE;
            nRv = soc_sbx_caladan3_cmu_segment_manual_flush(unit, segment, start, size);
            if (SOC_FAILURE(nRv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s CMU manual flush already in progress on unit %d\n"),
                           FUNCTION_NAME(), unit));
                FLUSH_UNLOCK(unit);
                return nRv;
            } else {
                bMutex = TRUE;
            }
        }
        
        /* wait for flush to be done and processed by cmu counter thread */
        while (pCmuCfg->bFlushing && uTimeout && sync) {
            sal_usleep(10 * MILLISECOND_USEC);
            uTimeout--;
        }
    
        /* record watermark if taking longer */
        if (pCmuCfg->uCMUWatermark > uTimeout) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "CMU manual flush watermark is 0x%x on unit %d.\n"),
                         uTimeout, unit));
            pCmuCfg->uCMUWatermark = uTimeout;
        }
    
        /* handle timeout */
        if (uTimeout == 0) {
            nRv = READ_CM_INTERRUPT_STATUSr(unit, &uRegValue);
            if (nRv != BCM_E_NONE) {
                if (bMutex) {
                    FLUSH_UNLOCK(unit);
                }
                return nRv;
            }
            if (soc_reg_field_get(unit, CM_INTERRUPT_STATUSr, uRegValue, MANUAL_EJECT_DONEf)) {
                /* CMU completed manual flush but ring process was not done 
                 * in time, return value from last successful flush
                 */
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "WARNING: did not receive CMU manual eject complete "
                                     " event, but done bit set on unit %d.\n"), unit));
                
                nRv = SOC_E_TIMEOUT;
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s CMU flush timed out on unit %d\n"),
                           FUNCTION_NAME(), unit));
                if (bMutex) {
                    FLUSH_UNLOCK(unit);
            }
                return SOC_E_FAIL;
            }
        }
        
    } else {
        /* No flush required for simple 64 bit counters, user must read OCM memory directly */
    }    
    /* release mutex */
    if (bMutex) {
        FLUSH_UNLOCK(unit);
    }
    
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_ocm_memory_size_set
 *   Purpose
 *      config the size of OCM memory used by CMU
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) size    : ocm size in bytes
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_ocm_memory_size_set(int unit,
					     int32 port,
					     uint32 size)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg; 

    /* make sure it's multiple of 64bits */
    if (size % sizeof(uint64)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU OCM memory need to be in multiple of 64bits on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_PARAM;
    }

    if (port == SOC_SBX_CALADAN3_OCM_CMU0_PORT) {
	port = 0;
    } else if (port == SOC_SBX_CALADAN3_OCM_CMU1_PORT) {
	port = 1;
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU OCM port %d on unit %d\n"),
                   FUNCTION_NAME(), port, unit));
	return SOC_E_PARAM;
    }

    /* NOTE: OCM memory is initialized to 0 by the OCM driver 
     *       no need to clear it here.
     */
    pCmuCfg->uCMUOcmSize[port] = size/sizeof(uint64);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_verify
 *   Purpose
 *      Check if there are any overlap among CMU segments
 *      ??Maybe this should be guaranteed by the OCM allocator??
 *   Parameters
 *      (IN) unit    : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_segment_verify(int unit)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    
    int nSegment, nTempSegment;

    
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
        if (pCmuCfg->segments[nSegment].bEnabled == FALSE) {
            continue;
        }
        for (nTempSegment = nSegment; nTempSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT;
	     nTempSegment++) {
            /* make sure there is no overlap in ocm_memory */
            /* NOTE: OCM allocator guaranty there is no overlap */
        }
    }
    
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_ocm_alloc
 *   Purpose
 *      Allocate a chunk of ocm memory
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) port    : CMU ocm port, 0 or 1
 *      (IN) size    : size of the requested alloc in bytes
 *      (OUT)addr    : OCM address allocated. 
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_PARAM - OCM memory was not allocated for the CMU driver
 *       SOC_E_MEMORY - failed to allocate OCM memory
 */

static int _soc_sbx_caladan3_cmu_ocm_alloc(int unit, int32 port, uint32 size, int32 *addr, int32 *alloc_size)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    
    int nSegment, nTempSegment, nValidSegment, nNumValidSegments;
    uint32 bFound, uPrevAddr, uPrevSize, uAllocSize;
    uint32 *uAllocDb;
    
    if ((port < 0) || (port >= SOC_SBX_CALADAN3_CMU_NUM_OCM_PORT)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment OCM port %d on unit %d\n"),
                   FUNCTION_NAME(), port, unit));
        return SOC_E_PARAM;
    }
    
    if (pCmuCfg->uCMUOcmSize[port] == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s OCM memory size has not been set for CMU port%d on unit %d\n"),
                   FUNCTION_NAME(), port, unit));
        return SOC_E_PARAM;
    }
    
    /* we only has upto 32 segments and this is mostly init time code
     * so do a simple but not efficient alloctor for now
     */
    uAllocDb = sal_alloc(sizeof(uint32)*2*SOC_SBX_CALADAN3_CMU_NUM_SEGMENT,
                         "CMU ocm alloctor");
    if (uAllocDb == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Could not allocate CMU ocm allocator database on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_MEMORY;
    }
    
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
        uAllocDb[2*nSegment] = pCmuCfg->uCMUOcmSize[port];
        uAllocDb[2*nSegment+1] = 0;
    }
    
    /* convert and round up to next multiple of 64 bits */
    if (size % sizeof(uint64)) {
        uAllocSize = (size + (sizeof(uint64)-1))/sizeof(uint64);
    } else {
        uAllocSize = size/sizeof(uint64);
    }
    
    /* build a sorted array of allocated ocm space for CMU */
    nNumValidSegments = 0;
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
        if ((pCmuCfg->segments[nSegment].bEnabled == FALSE) ||
            (pCmuCfg->segments[nSegment].nSegmentPort != port)) {
            /* skip unused segments or port on a different OCM port */
            continue;
        }
        for (nValidSegment = 0; nValidSegment <= nNumValidSegments; nValidSegment++) {
            if (pCmuCfg->segments[nSegment].nSegmentOcmBase < uAllocDb[2*nValidSegment]) {
                /* insert the current segment */
                for (nTempSegment = nNumValidSegments+1; nTempSegment > nValidSegment;
                     nTempSegment--) {
                    uAllocDb[2*nTempSegment] = uAllocDb[2*(nTempSegment-1)];
                    uAllocDb[2*nTempSegment+1] = uAllocDb[2*(nTempSegment-1)+1];
                }
                uAllocDb[2*nValidSegment] = pCmuCfg->segments[nSegment].nSegmentOcmBase;
                uAllocDb[2*nValidSegment+1] = pCmuCfg->segments[nSegment].nSegmentOcmSize;
            }
        }
        nNumValidSegments++;
    }    
    
    /* find a chunk of OCM memory that can fit */
    bFound = FALSE;
    uPrevAddr = 0;
    uPrevSize = 0;
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
        if (uAllocDb[2*nSegment]-(uPrevAddr+uPrevSize) >= uAllocSize) {
            bFound = TRUE;
            break;
        }
        uPrevAddr = uAllocDb[2*nSegment];
        uPrevSize = uAllocDb[2*nSegment+1];
    }
    
    if (bFound) {
        if (addr != NULL) {
            *addr = uPrevAddr+uPrevSize;
            *alloc_size = uAllocSize;
        } else {
            sal_free(uAllocDb);
            return SOC_E_PARAM;
        }
    } else {
	/* dump all current memory allocation */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to allocate 0x%x bytes OCM memory for CMU on unit %d\n"),
                   FUNCTION_NAME(), size, unit));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Current allocation on unit %d\n"),
                   FUNCTION_NAME(), unit));
        
        for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
            if (pCmuCfg->segments[nSegment].bEnabled == FALSE) {
                /* skip unused segments */
                continue;
            }
      	}
    }
    
    sal_free(uAllocDb);
    
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_background_flush_enable
 *   Purpose
 *      background segment flush enable
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) segment : segment ID
 *      (IN) enable  : enable background flush if TRUE
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_segment_background_flush_enable(int unit,
							 uint32 segment,
							 uint8 enable)
{
    uint32 uRegValue = 0;
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;    
    
    if (((int)segment < 0) || (segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;
    }

    /* don't enable background flush if it's simple64 counter segment or range tracker */
    if (enable 
        &&((pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_SIMPLE_64B) ||
           (pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_RANGE))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s The CMU segment %d on unit %d is simple 64bits counter"
                              "or range tracker, background flush not supported\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;	
    }

    SOC_IF_ERROR_RETURN(READ_CM_BACKGROUND_EJECT_ENABLEr(unit, &uRegValue));
    if (enable) {
        uRegValue |= (1 << segment);
    } else {
        uRegValue &= (~(1 << segment));
    }
    SOC_IF_ERROR_RETURN(WRITE_CM_BACKGROUND_EJECT_ENABLEr(unit, uRegValue));
    
    pCmuCfg->segments[segment].bBGEjectEnabled = (enable)?TRUE:FALSE;
    
    return nRv;    
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_background_flush_status
 *   Purpose
 *      get background segment flush status
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (OUT)segment : current segment being background ejected
 *      (OUT)counter : current counter being background ejected
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_PARAM when background ejection is not in progress (enabled)
 */
int soc_sbx_caladan3_cmu_segment_background_flush_status(int unit,
							 uint32 *segment,
							 uint32 *counter)
{
    int nRv = SOC_E_NONE;
    uint8  bInProgress;
    uint32 uRegValue = 0;

    SOC_IF_ERROR_RETURN(READ_CM_BACKGROUND_EJECT_STATUSr(unit, &uRegValue));
    bInProgress = soc_reg_field_get(unit, CM_BACKGROUND_EJECT_STATUSr,
                                    uRegValue, IN_PROGRESSf);
    if (!bInProgress) {
        nRv = SOC_E_PARAM;
    } else {
        *segment = soc_reg_field_get(unit, CM_BACKGROUND_EJECT_STATUSr,
                                     uRegValue, CURRENT_SEGMENTf);
        *counter = soc_reg_field_get(unit, CM_BACKGROUND_EJECT_STATUSr,
                                     uRegValue, CURRENT_IDf);
    }
    
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_manual_flush
 *   Purpose
 *      manual flush a range of counters in a segment
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) segment : segment ID to be flushed
 *      (IN) start   : start counter ID to be flushed
 *      (IN) size    : number of counters to be flushed
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_segment_manual_flush(int unit,
                                              uint32 segment,
                                              uint32 start,
                                              uint32 size)
{
    int nRv = SOC_E_NONE;
    uint32 uRegValue = 0;
    uint32 uCurrent = 0;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;

    /* parameter checks */
    if (((int)segment < 0) || (segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;
    }
    
    if (pCmuCfg->uManualEjectRate == 0) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU manual ejection is not enabled on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    if (pCmuCfg->segments[segment].bEnabled == FALSE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s The flushed CMU segment %d is not enabled on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;	
    }
    
    if ((start+size-1) > pCmuCfg->segments[segment].nSegmentLimit) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s counter ID flushed our of range of CMU segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_PARAM;	
    }
    
    /* status checks */
    nRv = soc_sbx_caladan3_cmu_segment_manual_flush_status(unit, &uCurrent);
    if (!SOC_FAILURE(nRv)) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s manual flush in progress on segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
        return SOC_E_BUSY;
    }
    
    /* issue request */
    if (size > 0) {
        SOC_IF_ERROR_RETURN(WRITE_CM_MANUAL_EJECT_LIMITr(unit, start+size-1));
    }
    
    SOC_IF_ERROR_RETURN(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue));
    if (size > 0) {
        soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, GOf, 1);
    } else {
        /* config but do nothing */
        soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, GOf, 0);
    }
    soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, SEGMENTf, segment);
    soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, START_IDf, start);
    SOC_IF_ERROR_RETURN(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue));
    
    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_manual_flush_status
 *   Purpose
 *      get the manual flush status
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (OUT)counter : current counter being manual ejected
 *   Returns
 *       SOC_E_NONE - successful
 *       SOC_E_BUSY when no manual ejection is in progress
 */
int soc_sbx_caladan3_cmu_segment_manual_flush_status(int unit,
						     uint32 *counter)
{
    int nRv = SOC_E_NONE;
    uint8 bInProgress;
    uint32 uRegValue = 0;

    SOC_IF_ERROR_RETURN(READ_CM_MANUAL_EJECT_STATUSr(unit, &uRegValue));
    bInProgress = soc_reg_field_get(unit, CM_MANUAL_EJECT_STATUSr,
				    uRegValue, IN_PROGRESSf);
    if (!bInProgress) {
	nRv = SOC_E_BUSY;
    } else {
	*counter = soc_reg_field_get(unit, CM_MANUAL_EJECT_STATUSr,
				     uRegValue, CURRENT_IDf);
    }
    
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cmu_segment_clear
 *   Purpose
 *      clear the counter segment memory
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) segment : segment ID to be cleared
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_segment_clear(int unit,
				       uint32 segment)
{
    int nRv = SOC_E_NONE;
    uint32 uCurrent = 0, uCount = 0, uSize= 0;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;

    /* clear OCM memory for the segment
     *   OCM mem_init should init all segments to 0 at init time
     *   How about detach time?
     */
    nRv = soc_sbx_caladan3_cmu_segment_manual_flush(unit, segment, 0,
						   pCmuCfg->segments[segment].nSegmentLimit+1);
    if (SOC_FAILURE(nRv)) {
	return nRv;
    }

    /* polling for status */
    while (1) {
	nRv = soc_sbx_caladan3_cmu_segment_manual_flush_status(unit, &uCurrent);
	if (SOC_FAILURE(nRv)) {
	    /* no manual ejection in progress */
	    nRv = SOC_E_NONE;
	    break;
	} else {
	    uCount++;
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "Loop %d: CMU segment %d size 0x%d manual flushing "
	                            " current counter %d on unit %d.\n"),
	                 uCount, segment, pCmuCfg->segments[segment].nSegmentLimit+1,
	                 uCurrent, unit));
	    sal_usleep(10 * MILLISECOND_USEC);
	}

	/* believe this is used only at detach time or init time, could afford
	 * to wait a while here
	 */
	if (uCount >= 5000) {
	    nRv = SOC_E_TIMEOUT;
	    break;
	}
    }

    /* clear host buffer for the segment */
    if (pCmuCfg->segments[segment].pSegmentPciBase != NULL) {
	uSize = pCmuCfg->segments[segment].nSegmentLimit * sizeof(uint64);
	if ((pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_64B) ||
	    (pCmuCfg->segments[segment].nSegmentType == SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_32B)) {
	    /* Turbo counter has 2 counters in host buffer */
	    uSize *= 2;
	}
	/* we allocated extra 7 bytes for alignment, make sure all is cleared */
	sal_memset(pCmuCfg->segments[segment].pSegmentPciBase, 0, uSize+7);	
    }

    return nRv;
}

/*
 *   Function
 *     soc_sbx_caladan3_cmu_counter_group_register
 *   Purpose
 *      Add a segment and its associated host-side buffer that will cache
 *      the updaed messages sentby the the CMU. This function should be
 *      called for each unique counter group, implying that for each segment
 *   Parameters
 *      (IN)     unit         : unit number of the device
 *      (IN/OUT) segment      : (IN) -1 to auto-allocate segment ID 
 *                                   0-31 to specify segment ID
 *      (IN)     num_counters : Max number of table entries in this segment
 *      (IN)     port         : OCM port (0-1)
 *      (IN)     ocm_base     : base logical address of OCM memory
 *                              -1 for auto-allocation
 *      (IN)     type         : Counter Type (see soc_sbx_caladan3_cmu_segment_type_e_t)
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_counter_group_register(int unit,
                                                uint32 *segment,
                                                uint32 num_counters,
                                                uint32 port,
                                                soc_sbx_caladan3_cmu_segment_type_e_t type)
{
    int nRv = SOC_E_NONE;
    uint32 uHostWords;
    int32 nSegment, uOcmSize;
    void  *pBuf;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;
    
    switch (type) {
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_64B:
	    uHostWords = num_counters * 2;
	    uOcmSize = num_counters * sizeof(uint64);
	    break;
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_32B:
	    uHostWords = num_counters * 2;
	    uOcmSize = num_counters * sizeof(uint32);
	    break;
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_SIMPLE_64B:
	    uHostWords = num_counters;
	    uOcmSize = num_counters * sizeof(uint64);
	    break;
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_SIMPLE_32B:
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_RANGE:
	    uHostWords = num_counters;
	    uOcmSize = num_counters * sizeof(uint32);
	    break;
	case SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_MAX:
	default:
	    /* illegal encoding */
	    uHostWords = 0;
	    return SOC_E_PARAM;
    }
    
    /* used specified segment or allocate a segment */
    if (*segment == -1) {

        /* auto allocate a segment */
        for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
            if (pCmuCfg->segments[nSegment].bEnabled == FALSE) {
                *segment = nSegment;
                break;
            }
        }
        if (nSegment == SOC_SBX_CALADAN3_CMU_NUM_SEGMENT) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s no CMU segment available on unit %d\n"),
                       FUNCTION_NAME(), unit));
            return SOC_E_RESOURCE;
        }
    } else {
        /* check the specified segment */
        if (((int)(*segment) < 0) ||
            (*segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {
            return SOC_E_PARAM;
        }
        
        if (pCmuCfg->segments[*segment].bEnabled == TRUE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s CMU segment %d already in use on unit %d\n"),
                       FUNCTION_NAME(), *segment, unit));
            return SOC_E_RESOURCE;
        }
    }
    
    /* allocate the host-side cache buffer, align to 64bits double word boundary */
    pBuf = sal_alloc(sizeof(uint64) * uHostWords + 7, "CMU counter buffer");
    if (!pBuf) {
        return SOC_E_MEMORY;
    } else {
        sal_memset(pBuf, 0, sizeof(uint64) * uHostWords + 7);
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CMU segment %d counter buffer at 0x%x on unit %d.\n"),
                     *segment, (uint32)pBuf, unit));
    }
    
    /* allocate ocm memory (logical address seen by CMU) */
    if (port == SOC_SBX_CALADAN3_OCM_CMU0_PORT) {
        pCmuCfg->segments[*segment].nSegmentPort = 0;
    } else if (port == SOC_SBX_CALADAN3_OCM_CMU1_PORT) {
        pCmuCfg->segments[*segment].nSegmentPort = 1;
    } else {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s invalid CMU OCM port %d on unit %d\n"),
                   FUNCTION_NAME(), port, unit));
    }
    nRv =_soc_sbx_caladan3_cmu_ocm_alloc(unit, pCmuCfg->segments[*segment].nSegmentPort,
                                         uOcmSize, &pCmuCfg->segments[*segment].nSegmentOcmBase,
                                         &pCmuCfg->segments[*segment].nSegmentOcmSize);
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to allocate OCM memory on port %d for "
                              "CMU segment %d on unit %d.\n"),
                   pCmuCfg->segments[*segment].nSegmentPort, *segment, unit));
        sal_free(pBuf);
        return nRv;
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CMU segment %d ocm base at 0x%x on unit %d.\n"),
                     *segment, (uint32)pCmuCfg->segments[*segment].nSegmentOcmBase, unit));
    }
    
    /* config the segment */
    pCmuCfg->segments[*segment].bEnabled = TRUE;
    pCmuCfg->segments[*segment].nSegmentType = type;
    pCmuCfg->segments[*segment].pSegmentPciBase = (int64   *)pBuf;
    pCmuCfg->segments[*segment].bBGEjectEnabled = FALSE;
    pCmuCfg->segments[*segment].bSTACEEnabled = TRUE;
    pCmuCfg->segments[*segment].nSegmentLimit = num_counters - 1;

    nRv = soc_sbx_caladan3_cmu_segment_set(unit, *segment, &pCmuCfg->segments[*segment]);
    if (SOC_FAILURE(nRv)) {
        /* mark the segment as unused */
        pCmuCfg->segments[*segment].bEnabled = FALSE;
        
        /* free the host memory */
        sal_free(pCmuCfg->segments[*segment].pSegmentPciBase);
        
        /* init other segment state */
        pCmuCfg->segments[*segment].nSegmentPort = 0;
        pCmuCfg->segments[*segment].nSegmentOcmBase = 0;
        pCmuCfg->segments[*segment].nSegmentOcmSize = 0;
        pCmuCfg->segments[*segment].pSegmentPciBase = NULL;
        pCmuCfg->segments[*segment].nSegmentLimit = 0;
        pCmuCfg->segments[*segment].nSegmentType = 0;
    }

    return nRv;
}

/*
 *   Function
 *     soc_sbx_caladan3_cmu_counter_group_unregister
 *   Purpose
 *      Delete a segment and its associated host-side buffer
 *   Parameters
 *      (IN)     unit         : unit number of the device
 *      (IN)     segment      : (IN) 0-31 to specify segment ID

 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cmu_counter_group_unregister(int unit,
						uint32 segment)
{
    int nRv = SOC_E_NONE;
    int64 *pBuff;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg = &SOC_SBX_CFG_CALADAN3(unit)->cmu_cfg;

    if (((int)segment < 0) ||
	(segment >= SOC_SBX_CALADAN3_CMU_NUM_SEGMENT)) {
        return SOC_E_PARAM;
    }

    if (pCmuCfg->segments[segment].bEnabled != TRUE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s CMU segment %d not register on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    pBuff = pCmuCfg->segments[segment].pSegmentPciBase;

    /* disable the segment */
    pCmuCfg->segments[segment].bEnabled = FALSE;
    pCmuCfg->segments[segment].nSegmentPort = 0;
    pCmuCfg->segments[segment].nSegmentOcmBase = 0;
    pCmuCfg->segments[segment].nSegmentOcmSize = 0;
    pCmuCfg->segments[segment].nSegmentType = 0;
    pCmuCfg->segments[segment].pSegmentPciBase = NULL;
    pCmuCfg->segments[segment].bBGEjectEnabled = FALSE;
    pCmuCfg->segments[segment].bSTACEEnabled = TRUE;
    pCmuCfg->segments[segment].nSegmentLimit = 0;

    nRv = soc_sbx_caladan3_cmu_segment_set(unit, segment, &pCmuCfg->segments[segment]);
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Could not remove segment %d on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
    }

    /* free buffers */
    if (pBuff != NULL) {
        sal_free(pBuff);
    }

    return nRv;
}

#endif
