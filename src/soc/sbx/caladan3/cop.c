/*
 * $Id: cop.c,v 1.33.24.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    cop.c
 * Purpose: Caladan3 COP drivers
 * Requires:
 */

#include <shared/bsl.h>
#include <soc/types.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/wb_db_cop.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <sal/compiler.h>



#define _SOC_CALADAN3_COP_DEBUG

#define COP_INJECT_LOCK(unit, cop)   sal_mutex_take(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.nInjectLock[(cop)], sal_mutex_FOREVER)
#define COP_INJECT_UNLOCK(unit, cop) sal_mutex_give(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.nInjectLock[(cop)]);

#define COP_RING_LOCK(unit, cop)   sal_mutex_take(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.nRingLock[(cop)], sal_mutex_FOREVER)
#define COP_RING_UNLOCK(unit, cop) sal_mutex_give(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.nRingLock[(cop)]);

#define _COP_IF_ERROR_INJECT_UNLOCK_RETURN(op, cop)				\
    do { int __rv__; if ((__rv__ = (op)) < 0) {COP_INJECT_UNLOCK(unit, cop); return(__rv__);} } while(0)

#define _COP_TIMER_INTERRUPT_EN_BITPOS (0)
#define _COP_TIMER_START_BITPOS        (1)

static int _soc_sbx_caladan3_cop_object_state_init(int unit,
						   int cop,
						   int segment,
						   int id,
						   uint32 param,
						   uint32 param_hi);

static int _soc_sbx_caladan3_cop_ocm_alloc(int unit,
					   int cop,
					   uint32 size,
					   int32 *addr,
					   int32 *alloc_size);

static int _soc_sbx_caladan3_cop_null_profile_create(int unit);

static int _soc_sbx_caladan3_cop_segment_config(int unit,
						int cop,
						int segment,
						soc_sbx_caladan3_cop_segment_config_t *config);

static int _soc_sbx_caladan3_cop_ring_process(int unit, int cop);

static void _soc_sbx_caladan3_cop_ring_process_thread(void *unit_vp);

static INLINE int _soc_sbx_caladan3_cop_timer_event_queue_count(soc_sbx_caladan3_cop_timer_queue_t *q)
{
    return (q->nTail+1);
}

static INLINE int _soc_sbx_caladan3_cop_timer_event_queue_full(soc_sbx_caladan3_cop_timer_queue_t *q)
{
    return (_soc_sbx_caladan3_cop_timer_event_queue_count(q) > (q->nMaxDepth));
}

static INLINE int _soc_sbx_caladan3_cop_timer_event_queue_empty(soc_sbx_caladan3_cop_timer_queue_t *q)
{
    return q->nTail < 0;
}

static INLINE void _soc_sbx_caladan3_cop_timer_event_enqueue(soc_sbx_caladan3_cop_timer_queue_t *q,
							     uint32 cop, uint32 segment, uint32 timer,
							     uint32 forced, uint32 active_when_forced)
{
    q->nTail++;
    q->sQueue[q->nQId][q->nTail].uCop = cop;
    q->sQueue[q->nQId][q->nTail].uSegment = segment;
    q->sQueue[q->nQId][q->nTail].uTimer = timer;
    q->sQueue[q->nQId][q->nTail].bForced = forced;
    q->sQueue[q->nQId][q->nTail].bActiveWhenForced = active_when_forced;
}

/*
 *   Function
 *     soc_sbx_caladan3_cop_timer_event_queue_size_set
 *   Purpose
 *      COP timer expired event queue size set
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) size   : timer expired event queue size
 *   Returns
 *       SOC_E_NONE - successfully dequeued one timer expire event
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_timer_event_queue_size_set(int unit, int cop, int size)
{
    int nRv = SOC_E_NONE;
    int bRestart = FALSE;
    soc_sbx_caladan3_cop_timer_queue_t *pQueue;
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sRing);

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	pQueue = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sTimerQueue[cop];
    }

    if ((size < 4096) && (size != 0)) {
	size = 4096; /* make sure it's not too small */
    }

    if (size != pQueue->nMaxDepth) {
	if (pRing->running[cop]) {
	    nRv = soc_sbx_caladan3_cop_ring_process_thread_stop(unit, cop);
	    if (SOC_FAILURE(nRv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s Failed to stop COP ring processing thread on unit %d\n"),
                           FUNCTION_NAME(), unit));
		return nRv;
	    }
	    bRestart = TRUE;
	}

	/* reallocate all queue memories */
	if (pQueue->sQueue[0]) {
	    sal_free(pQueue->sQueue[0]);
	}
	pQueue->sQueue[0] = sal_alloc(sizeof(soc_sbx_caladan3_cop_timer_expire_event_t)*size*2,
				      "COP timer expire event queue");
	if (pQueue->sQueue[0] == NULL) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to allocate COP %d segment control database on unit %d\n"),
                       FUNCTION_NAME(), cop, unit));
	    return SOC_E_MEMORY;
	} else {
	    pQueue->sQueue[1] = pQueue->sQueue[0]+size;
	}
	sal_memset(pQueue->sQueue[0], 0, sizeof(soc_sbx_caladan3_cop_timer_expire_event_t)*size*2);
	pQueue->nMaxDepth = size;
	pQueue->nTail = -1;
	pQueue->nQId = 0;
	pQueue->nReadTail = -1;
	pQueue->nReadHead = 0;

	if (bRestart) {
	    nRv = soc_sbx_caladan3_cop_ring_process_thread_start(unit, cop);
	    if (SOC_FAILURE(nRv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s Failed to restart COP ring processing thread on unit %d\n"),
                           FUNCTION_NAME(), unit));
		return nRv;
	    }
	}
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_cop_timer_event_callback_register
 *   Purpose
 *      COP timer expired event callback function register
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cb     : callback function pointer
 *   Returns
 *       SOC_E_NONE - successfully dequeued one timer expire event
 */
int
soc_sbx_caladan3_cop_timer_event_callback_register(int unit,
						   soc_sbx_caladan3_cop_timer_event_callback_f cb)
{
    SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.fNotifyCb = cb;
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_cop_timer_event_dequeue
 *   Purpose
 *      COP timer expired event dequeue
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (OUT)event  : timer expired event
 *   Returns
 *       SOC_E_NONE - successfully dequeued one timer expire event
 *       SOC_E_EMPTY- no timer expire event pending
 */
int soc_sbx_caladan3_cop_timer_event_dequeue(int unit, int cop,
					     soc_sbx_caladan3_cop_timer_expire_event_t *event)
{
    soc_sbx_caladan3_cop_timer_queue_t *q = &(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sTimerQueue[cop]);

    if (q->nReadHead > q->nReadTail) {
	/* processed all elements of current buffer, check the other buffer */
	COP_RING_LOCK(unit, cop);
	if (_soc_sbx_caladan3_cop_timer_event_queue_empty(q)) {
	    /* the other buffer is empty too, return empty */
	    q->nReadHead = 0;
	    q->nReadTail = -1;
	    COP_RING_UNLOCK(unit, cop);
	    return SOC_E_EMPTY;
	} else {
	    /* switch buffer */
	    q->nReadTail = q->nTail;
	    q->nReadHead = 0;
	    q->nTail = -1;
	    q->nQId = !q->nQId;   /* ping pong buffer switch */
	    COP_RING_UNLOCK(unit, cop);
	}
    }

    *event = q->sQueue[!q->nQId][q->nReadHead];
    q->nReadHead++;

    return SOC_E_NONE;
}

/* for now, using the CMICm FIFO read DMA for timer expire event */
#ifdef _COP_TIMER_EVENT_FIFO_DMA_DISABLE
#undef _COP_TIMER_EVENT_FIFO_DMA_DISABLE
#endif

#define _SOC_CALADAN3_COP_CMICM_CMC (1)

/*
 *   Function
 *     sbx_caladan3_cop_hw_init
 *   Purpose
 *      COP hardware initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_hw_init(int unit) 
{
    int nRv = SOC_E_NONE;
    uint32 uRegValue = 0;
    soc_timeout_t timeout;
    uint32 nTimeoutInUsec;
    int nInitDone, nCop, nSegment;
    uint32 uCop;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    for (nCop = 0; nCop < SOC_SBX_CALADAN3_COP_NUM_COP; nCop++) {
	uCop = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(nCop);
	/*== bring cop out of reset */
	SOC_IF_ERROR_RETURN(READ_CO_GLOBAL_CONFIGr(unit, uCop, &uRegValue));
	soc_reg_field_set(unit, CO_GLOBAL_CONFIGr, &uRegValue, SOFT_RESETf, 1);
	soc_reg_field_set(unit, CO_GLOBAL_CONFIGr, &uRegValue, PKT_MODE_LENf, 1);
	SOC_IF_ERROR_RETURN(WRITE_CO_GLOBAL_CONFIGr(unit, uCop, uRegValue));
	
	/*== init internal mems */
	uRegValue = 0;
	soc_reg_field_set(unit, CO_MEMORY_INITr, &uRegValue, METER_PROFILE_INITf, 1);
	soc_reg_field_set(unit, CO_MEMORY_INITr, &uRegValue, METER_MONITOR_COUNTER_INITf, 1);
	SOC_IF_ERROR_RETURN(WRITE_CO_MEMORY_INITr(unit, uCop, uRegValue));

	if (SAL_BOOT_QUICKTURN) {    
	    nTimeoutInUsec = 20000 * MILLISECOND_USEC;
	} else {
	    nTimeoutInUsec = 20 * MILLISECOND_USEC;
	}
	
	nInitDone = FALSE;
	soc_timeout_init(&timeout, nTimeoutInUsec,0);
	while(!soc_timeout_check(&timeout)) {
	    SOC_IF_ERROR_RETURN(READ_CO_MEMORY_INIT_DONEr(unit, uCop, &uRegValue));
	    
	    nInitDone = soc_reg_field_get(unit, CO_MEMORY_INIT_DONEr,
					  uRegValue, METER_PROFILE_INIT_DONEf);
	    
	    nInitDone &= soc_reg_field_get(unit, CO_MEMORY_INIT_DONEr,
					   uRegValue, METER_MONITOR_COUNTER_INIT_DONEf);
	    
	    if (nInitDone) {
		/* both init done is set */
		break;
	    }
	}

	/* fake success on PCID simulation */
	if ((nInitDone == FALSE) && (!SAL_BOOT_PLISIM))  {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to init memory for COP %d on unit %d\n"),
                       FUNCTION_NAME(), nCop, unit));
	    return SOC_E_TIMEOUT;
	}

	/* config segments */
	for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_COP_NUM_SEGMENT; nSegment++) {
	    nRv = _soc_sbx_caladan3_cop_segment_config(unit, nCop, nSegment, &pCopCfg->segments[nCop][nSegment]);
	    if (SOC_FAILURE(nRv)) {		
		return SOC_E_PARAM;
	    }
	}    

	/* interrupts handling */
	/* 
	 * NOTE: even though the timer expire event for each COP is hooked
	 *   up to FIFO read DMA engine in CMICm CMC1, for now, we choose
	 *   not to enable that machnism. Even though it's more efficient,
	 *   current SDK interrupt handler infrastructure only process CMC0
	 *   interrupts for now. 
	 *   For now, we will be using the mechnism similiar to FE2000,
	 *   use interrupts in CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUS and
	 *   in the interrupt handler, read CO_WATCHDOG_TIMER_EXPIRED_FIFO
	 *   to get the list of expired timer.
	 *   Similiar to FE2000 PMU driver, the interrupt handler is 
	 *   implemented in the OAM driver since it's the only user FOR NOW!!
	 */
	SOC_IF_ERROR_RETURN(WRITE_CO_ERROR_MASKr(unit, uCop, 0xffffFFFF));
	SOC_IF_ERROR_RETURN(WRITE_CO_REFRESH_OVERFLOW_ERROR_MASKr(unit, uCop, 0xffffFFFF));
	SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_DISABLE_ERROR_MASKr(unit, uCop, 0xffffFFFF));
	SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_RANGE_ERROR_MASKr(unit, uCop, 0xffffFFFF));
	SOC_IF_ERROR_RETURN(WRITE_CO_ECC_ERROR_MASKr(unit, uCop, 0xffffFFFF));

#ifdef _COP_TIMER_EVENT_FIFO_DMA_DISABLE
	/* enable both NONEMPTY and OVERFLOW interrupts */
	SOC_IF_ERROR_RETURN(WRITE_CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUS_MASKr(unit, uCop, 0));
#else
	/* disable both NONEMPTY and OVERFLOW interrupts */
	SOC_IF_ERROR_RETURN(WRITE_CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUS_MASKr(unit, uCop, 0xffffFFFF));
#endif /* _COP_TIMER_EVENT_FIFO_DMA_DISABLE */
    }

    /* create NULL profiles */
    nRv = _soc_sbx_caladan3_cop_null_profile_create(unit);  

    /*== enable CMICm sbus slave interrupt for COP blocks */
    nRv = soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_CO0_INTR_POS)|
				 (1<<SOC_SBX_CALADAN3_CO1_INTR_POS));
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to enable COP block interrupt on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }
 
#ifndef _COP_TIMER_EVENT_FIFO_DMA_DISABLE
    /* enable FIFO dma for COP0 and COP1, which is FIFO READ DMA channel 0/1 on CMC1 */
    soc_cmicm_cmcx_intr0_enable(unit, _SOC_CALADAN3_COP_CMICM_CMC,
				IRQ_CMCx_FIFO_CH_DMA(SOC_MEM_FIFO_DMA_CHANNEL_0));

    soc_cmicm_cmcx_intr0_enable(unit, _SOC_CALADAN3_COP_CMICM_CMC,
				IRQ_CMCx_FIFO_CH_DMA(SOC_MEM_FIFO_DMA_CHANNEL_1));
#endif /* _COP_TIMER_EVENT_FIFO_DMA_DISABLE */

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_driver_init
 *   Purpose
 *      Drivier initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_driver_init(int unit) 
{
    int nRv = SOC_E_NONE;
    int nCop, nRingSize, nEntrySize;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Caladan3 cop driver init called on unit %d.\n"),
                 unit));
    if (pCopCfg->bDriverInit) {
        soc_sbx_caladan3_cop_driver_uninit(unit);
        /*return SOC_E_INIT;*/
    }

    /* default parameters */
    pCopCfg->fNotifyCb = NULL;
    pCopCfg->sRing.nRingEntries = 4096; /* default to 4K */
    pCopCfg->sRing.nRingThresh = 1;     /* default to trigger as soon as non-empty */
    pCopCfg->uMonitorDmaBuffer = NULL;  /* auto-allocation later */

    for (nCop = 0; nCop < SOC_SBX_CALADAN3_COP_NUM_COP; nCop++) {
	pCopCfg->sRing.pRingPciBase[nCop] = NULL; /* auto-allocation later */
	pCopCfg->sRing.pRingPciRead[nCop] = NULL; /* auto-allocation later */
	pCopCfg->sRing.pid[nCop] = SAL_THREAD_ERROR;
	pCopCfg->sRing.trigger[nCop] = NULL;
	pCopCfg->sRing.running[nCop] = FALSE;

	/* COP segment management structure init */
	pCopCfg->segments[nCop] = sal_alloc(sizeof(soc_sbx_caladan3_cop_segment_config_t) *
					 SOC_SBX_CALADAN3_COP_NUM_SEGMENT, "COP segment database");
	if (pCopCfg->segments[nCop] == NULL) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to allocate COP %d segment control database on unit %d\n"),
                       FUNCTION_NAME(), nCop, unit));
	    soc_sbx_caladan3_cop_driver_uninit(unit);
	    return SOC_E_MEMORY;
	}
	sal_memset(pCopCfg->segments[nCop], 0, sizeof(soc_sbx_caladan3_cop_segment_config_t) *
		   SOC_SBX_CALADAN3_COP_NUM_SEGMENT);
   
	/* COP profile management structure init */
	pCopCfg->profiles[nCop] = sal_alloc(sizeof(soc_sbx_caladan3_cop_profile_t) *
					    SOC_SBX_CALADAN3_COP_NUM_PROFILE, "COP profile database");
	if (pCopCfg->profiles[nCop] == NULL) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to allocate COP %d profile control database on unit %d\n"),
                       FUNCTION_NAME(), nCop, unit));
	    soc_sbx_caladan3_cop_driver_uninit(unit);
	    return SOC_E_MEMORY;
	}
	sal_memset(pCopCfg->profiles[nCop], 0, sizeof(soc_sbx_caladan3_cop_profile_t) *
		   SOC_SBX_CALADAN3_COP_NUM_PROFILE);    

	/* ring buffer */
	nRingSize = 0;
	if (pCopCfg->sRing.pRingPciBase[nCop] == NULL) {
	    /* allocate ring buffer in case caller has not done that */
	    nEntrySize = SOC_MEM_INFO(unit,CO_WATCHDOG_TIMER_EXPIRED_FIFOm).bytes;
	    nRingSize = pCopCfg->sRing.nRingEntries * nEntrySize;
	    pCopCfg->sRing.pRingPciBase[nCop] = soc_cm_salloc(unit, nRingSize, "COP ring buffer");
	    if (pCopCfg->sRing.pRingPciBase[nCop] == NULL) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s failed to allocat %d bytes for COP ring buffer on unit %d\n"),
                           FUNCTION_NAME(), nRingSize, unit));
		soc_sbx_caladan3_cop_driver_uninit(unit);
		return SOC_E_MEMORY;
	    } else {
		LOG_VERBOSE(BSL_LS_SOC_COMMON,
		            (BSL_META_U(unit,
		                        "%s: ring buffer set at 0x%x on unit %d\n"),
		             FUNCTION_NAME(), (uint32)pCopCfg->sRing.pRingPciBase[nCop], unit));
	    }
	    /* point the read pointer to the base */
	    pCopCfg->sRing.pRingPciRead[nCop] = pCopCfg->sRing.pRingPciBase[nCop];
	}
	sal_memset(pCopCfg->sRing.pRingPciBase[nCop], 0, nRingSize);

	/* create lock for command injection */
	if (pCopCfg->nInjectLock[nCop] == NULL) {
	    pCopCfg->nInjectLock[nCop] = sal_mutex_create("COP command injection lock");
	    if (!pCopCfg->nInjectLock[nCop]) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Failed to create mutex for COP command injection on unit %d\n"),
                           FUNCTION_NAME(), unit));
		soc_sbx_caladan3_cop_driver_uninit(unit);
		return SOC_E_MEMORY;
	    }
	}

	/* create lock for timer expire event fifo read dma ring */
	if (pCopCfg->nRingLock[nCop] == NULL) {
	    pCopCfg->nRingLock[nCop] = sal_mutex_create("COP command ring lock");
	    if (!pCopCfg->nRingLock[0]) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Failed to create mutex for COP %d ring on unit %d\n"),
                           FUNCTION_NAME(), nCop, unit));
		soc_sbx_caladan3_cop_driver_uninit(unit);
		return SOC_E_MEMORY;
	    }
	}
    }

#ifdef BCM_WARM_BOOT_SUPPORT

    /* if warm boot then overwrite profile from scache */
    nRv = soc_sbx_cop_wb_state_init(unit);
    
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed loading SOC COP profiles from scache, error %d \n"),
                   FUNCTION_NAME(), unit, nRv));
        return nRv;
    }
#endif

    /* allocate dma buffer for meter monitor counters */
    if (pCopCfg->uMonitorDmaBuffer == NULL) {
	pCopCfg->uMonitorDmaBuffer = (co_meter_monitor_counter_entry_t *) \
	    soc_cm_salloc(unit, SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS * sizeof(co_meter_monitor_counter_entry_t),
			  "cop-meter-monitor-dmabuffer");
	if (pCopCfg->uMonitorDmaBuffer == NULL) {
	    soc_sbx_caladan3_cop_driver_uninit(unit);
	    return SOC_E_MEMORY;
	}
    }

    nRv = soc_sbx_caladan3_cop_hw_init(unit);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Hardware init failed %d \n"),
                   FUNCTION_NAME(), unit, nRv));
	return nRv;
    }

    if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {

        /* start up ring processing thread, 
         * the thread is triggered by FIFO read dma 
         * interrupts only
         */
        for (nCop = 0; nCop < SOC_SBX_CALADAN3_COP_NUM_COP; nCop++) {
            nRv = soc_sbx_caladan3_cop_timer_event_queue_size_set(unit, nCop, 4096);
            if (SOC_FAILURE(nRv)) {
	        LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to config COP %d timer event queue size failed %d \n"),
                           FUNCTION_NAME(), unit, nCop, nRv));
	        return nRv;
	    }
      
	    nRv = soc_sbx_caladan3_cop_ring_process_thread_start(unit, nCop);
	    if (SOC_FAILURE(nRv)) {
	        LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to start COP %d timer event ring process thread %d \n"),
                           FUNCTION_NAME(), unit, nCop, nRv));
	        return nRv;
	    }
        }
    }

    pCopCfg->bDriverInit = TRUE;
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_driver_uninit
 *   Purpose
 *      COP driver un-initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_driver_uninit(int unit) 
{
    int nCop;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    /* COP driver clean up */
    if (pCopCfg->segments[0] != NULL) {
	sal_free(pCopCfg->segments[0]);
	pCopCfg->segments[0] = NULL;
    }

    if (pCopCfg->segments[1] != NULL) {
	sal_free(pCopCfg->segments[1]);
	pCopCfg->segments[1] = NULL;
    }

    if (pCopCfg->profiles[0] != NULL) {
	sal_free(pCopCfg->profiles[0]);
	pCopCfg->profiles[0] = NULL;
    }

    if (pCopCfg->profiles[1] != NULL) {
	sal_free(pCopCfg->profiles[1]);
	pCopCfg->profiles[1] = NULL;
    }

    if (pCopCfg->uMonitorDmaBuffer != NULL) {
        soc_cm_sfree(unit, pCopCfg->uMonitorDmaBuffer);	
	pCopCfg->uMonitorDmaBuffer = NULL;
    }

    for (nCop = 0; nCop < SOC_SBX_CALADAN3_COP_NUM_COP; nCop++) {
        if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {
	    (void)soc_sbx_caladan3_cop_ring_process_thread_stop(unit, nCop);
        }

	if (pCopCfg->nInjectLock[nCop] != NULL) {
	    sal_mutex_destroy(pCopCfg->nInjectLock[nCop]);	
	    pCopCfg->nInjectLock[nCop] = NULL;
	}

	if (pCopCfg->nRingLock[nCop] != NULL) {
	    sal_mutex_destroy(pCopCfg->nRingLock[nCop]);	
	    pCopCfg->nRingLock[nCop] = NULL;
	}

	if (pCopCfg->sTimerQueue[nCop].sQueue[0]) {
	    sal_free(pCopCfg->sTimerQueue[nCop].sQueue[0]);
	    pCopCfg->sTimerQueue[nCop].sQueue[0] = NULL;
	    pCopCfg->sTimerQueue[nCop].sQueue[1] = NULL;
	}
       
        if (pCopCfg->sRing.pRingPciBase[nCop]) {
            soc_cm_sfree(unit, pCopCfg->sRing.pRingPciBase[nCop]);
        }
    }

    pCopCfg->sRing.nRingEntries = 0; 
    pCopCfg->sRing.nRingThresh = 0;  
    pCopCfg->bDriverInit = FALSE;
    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cop_segment_register
 *   Purpose
 *      register a cop segment
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) num_entry : number of entries in the segment
 *      (IN) type      : type of segment
 *      (IN) config    : type specific segment config parameter
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_segment_register(int unit,
					  int cop,
					  int segment,
					  int num_entry,
					  soc_sbx_caladan3_cop_segment_type_e_t type,
					  soc_sbx_caladan3_cop_segment_type_specific_config_t *config)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_segment_config_t sConfig;

    if (config == NULL) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s please specify segment parameters on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_PARAM;
    }
    
    sal_memset(&sConfig, 0, sizeof(sConfig));

    sConfig.bEnabled = TRUE;
    sConfig.nSegmentLimit = num_entry - 1;
    sConfig.nSegmentType = type;
    sConfig.u = *config;

    nRv = _soc_sbx_caladan3_cop_segment_config(unit, cop, segment, &sConfig);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_segment_unregister
 *   Purpose
 *      unregister a cop segment
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_segment_unregister(int unit,
					    int cop,
					    int segment)
{
    int nRv;
    soc_sbx_caladan3_cop_segment_config_t sConfig;

    sal_memset(&sConfig, 0, sizeof(sConfig));
    sConfig.bEnabled = FALSE;

    nRv = _soc_sbx_caladan3_cop_segment_config(unit, cop, segment, &sConfig);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_segment_read
 *   Purpose
 *      read a cop segment configuration
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (OUT) num_entry: number of entries in the segment
 *      (OUT) type     : type of segment
 *      (out) config   : type specific segment config parameter
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_segment_read(int unit,
				      int cop,
				      int segment,
				      int *num_entry,
				      soc_sbx_caladan3_cop_segment_type_e_t *type,
				      soc_sbx_caladan3_cop_segment_type_specific_config_t *config)
{
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if ((segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* check if segment is enabled */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if (pSegCfg->bEnabled != TRUE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not in use on unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;	
    }

    if (num_entry != NULL) {
	*num_entry = pSegCfg->nSegmentLimit+1;
    }

    if (type != NULL) {
	*type = pSegCfg->nSegmentType;
    }

    if (config != NULL) {
	*config = pSegCfg->u;
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cop_ocm_memory_size_set
 *   Purpose
 *      config the size of OCM memory used by COP, required before register a segment
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) size    : ocm size in bytes
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_ocm_memory_size_set(int unit,
					     int32 cop,
					     uint32 size)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg; 

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    /* make sure it's multiple of 64bits */
    if (size % sizeof(uint64)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d OCM memory need to be in multiple of 64bits on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    /* NOTE: OCM memory is initialized to 0 by the OCM driver 
     *       no need to clear it here.
     */
    pCopCfg->uCOPOcmSize[cop] = size/sizeof(uint64);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_ocm_alloc
 *   Purpose
 *      Allocate a chunk of ocm memory
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) cop     : cop instance 0 or 1
 *      (IN) size    : size of the requested alloc in bytes
 *      (OUT)addr    : OCM address allocated. 
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_PARAM - OCM memory was not allocated for the COP driver
 *       SOC_E_MEMORY - failed to allocate OCM memory
 */
static int _soc_sbx_caladan3_cop_ocm_alloc(int unit,
					   int cop,
					   uint32 size,
					   int32 *addr,
					   int32 *alloc_size)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;    
    int nSegment, nTempSegment, nValidSegment, nNumValidSegments;
    uint32 bFound, uPrevAddr, uPrevSize, uAllocSize;
    uint32 *uAllocDb;

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if (pCopCfg->uCOPOcmSize[cop] == 0) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s OCM memory size has not been set for COP %d on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    /* we only has upto 32 segments and this is mostly init time code
     * so do a simple but not efficient alloctor for now
     */
    uAllocDb = sal_alloc(sizeof(uint32)*2*SOC_SBX_CALADAN3_COP_NUM_SEGMENT,
			 "COP ocm alloctor");
    if (uAllocDb == NULL) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Could not allocate COP ocm allocator database on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_MEMORY;
    }

    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_COP_NUM_SEGMENT; nSegment++) {
	uAllocDb[2*nSegment] = pCopCfg->uCOPOcmSize[cop];
	uAllocDb[2*nSegment+1] = 0;
    }

    /* convert and round up to next multiple of 64 bits */
    if (size % sizeof(uint64)) {
	uAllocSize = (size + (sizeof(uint64)-1))/sizeof(uint64);
    } else {
	uAllocSize = size/sizeof(uint64);
    }

    /* build a sorted array of allocated ocm space for COP */
    nNumValidSegments = 0;
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_COP_NUM_SEGMENT; nSegment++) {
	if (pCopCfg->segments[cop][nSegment].bEnabled == FALSE) {
	    /* skip unused segments */
	    continue;
	}
	for (nValidSegment = 0; nValidSegment <= nNumValidSegments; nValidSegment++) {
	    if (pCopCfg->segments[cop][nSegment].nSegmentOcmBase < uAllocDb[2*nValidSegment]) {
		/* insert the current segment */
		for (nTempSegment = nNumValidSegments+1; nTempSegment > nValidSegment;
		     nTempSegment--) {
		    uAllocDb[2*nTempSegment] = uAllocDb[2*(nTempSegment-1)];
		    uAllocDb[2*nTempSegment+1] = uAllocDb[2*(nTempSegment-1)+1];
		}
		uAllocDb[2*nValidSegment] = pCopCfg->segments[cop][nSegment].nSegmentOcmBase;
		uAllocDb[2*nValidSegment+1] = pCopCfg->segments[cop][nSegment].nSegmentOcmSize;
		break;
	    }
	}
	nNumValidSegments++;
    }    

    /* find a chunk of OCM memory that can fit */
    bFound = FALSE;
    uPrevAddr = 0;
    uPrevSize = 0;
    for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_COP_NUM_SEGMENT; nSegment++) {
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
                              "%s Failed to allocate 0x%x bytes OCM memory for COP on unit %d\n"),
                   FUNCTION_NAME(), size, unit));
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Current allocation on unit %d\n"),
                   FUNCTION_NAME(), unit));
	
	for (nSegment = 0; nSegment < SOC_SBX_CALADAN3_COP_NUM_SEGMENT; nSegment++) {
	    if (pCopCfg->segments[cop][nSegment].bEnabled == FALSE) {
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
 *     sbx_caladan3_cop_segment_config
 *   Purpose
 *      Config COP segment, associated OCM memory is NOT initialized here
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) config : cop segment config info
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
static int _soc_sbx_caladan3_cop_segment_config(int unit,
						int cop,
						int segment,
						soc_sbx_caladan3_cop_segment_config_t *config)
{
    int nRv = SOC_E_NONE;    
    uint32 uRegValue = 0, uRegValue2 = 0;
    int nCop;
    uint64 uuVisitPeriod, uuTemp;
    uint32 uVisitInterval = 0, uTemp = 0;
    uint32 uOcmSize;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    /* check COP instance and segment ID */    
    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(cop);
    }

    if ((segment < 0) || (segment >= SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    if (config->bEnabled == FALSE) {
	/* disable a segment */
	SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_ENABLEr(unit, nCop, &uRegValue));
	uRegValue &= (~(1 << segment));
	SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_ENABLEr(unit, nCop, uRegValue));

	if (pCopCfg->segments[cop] != NULL) {
	    pCopCfg->segments[cop][segment] = *config;
	}
	return SOC_E_NONE;
    }

    /* segment type specific config */
    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_CONFIGr(unit, nCop, segment, &uRegValue));
    switch (config->nSegmentType) {
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER:
	    /* error mask and error color */
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      METER_ERROR_MASKf, config->u.sPolicer.bErrorMask?1:0);

	    if (config->u.sPolicer.nErrorColor >= SOC_SBX_CALADAN3_COP_POLICER_ERROR_COLOR_MAX) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unsupported error color %d for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sPolicer.nErrorColor, segment, unit));
		return SOC_E_PARAM;
	    }

	    if (config->u.sPolicer.uMaxBurstBits > (64 * 1024 * 1024 * 8)) {
		/* support upto 64MBytes burst size */
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unsupported max burst size 0x%x bits (max 512M bits)"
                                      "for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sPolicer.uMaxBurstBits, segment, unit));
		return SOC_E_PARAM;		
	    }
#if 0
	    if (config->u.sPolicer.uMaxRateKbps > 100000000) {
		/* support upto 100Gbps rate */
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unsupported max rate %d kbps (max 100Gbps)"
                                      "for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sPolicer.uMaxRateKbps, segment, unit));
		return SOC_E_PARAM;		
	    }
#endif
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      METER_ERROR_COLORf, config->u.sPolicer.nErrorColor);

	    /* calculate visit interval and visit period for policer
	     *  ASSUMPTION/FACTS:
	     *   C3 clock                : 1GHz
	     *   policer bucket size     : 2^27
	     *   line rate               : 100Gbps
	     *   MTU                     : 16K bytes
	     *   serviceDelay            : assume worst case 32 epoch * 1792 cycles/epoch = 57344
	     *                             (NOTE: serviceDelay seems only play a minor role here
	     *                              when we limite max burst size to be smaller than 64MBytes)
	     *  INPUT:
	     *   numPolicerInSegment
	     *   maxPolicerRate          
	     *   maxPolicerBurst
	     *  FORMULA:
	     *   serviceDelay = 
	     *   visit_period = floor((2^27-maxPolicerBurst-MTU))/maxPolicerRate - serviceDelay
	     *   visit_interval = visit_period / numPolicerInSegment
	     */
	    if (config->nSegmentLimit < 0) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s require number of policer to config segment %d on unit %d\n"),
                           FUNCTION_NAME(), segment, unit));
		return SOC_E_PARAM;
	    }

	    /* calculate the visit_period, rate only make difference between [1Gbps - 100Gbps] */
	    uTemp = config->u.sPolicer.uMaxRateKbps / 1000000;
	    if (uTemp < 1) {
		uTemp = 1;
	    } else if (uTemp > (100000000/1000000)) {
		/* max rate supported is 100Gbps */
		uTemp = 100;
	    }
	    uTemp = ((1<<27) - (1<<14) - (config->u.sPolicer.uMaxBurstBits/8)) * 8 / uTemp - 57344;
	    
	    /* calculate visit_interval */
	    uVisitInterval = uTemp / (config->nSegmentLimit+1);
	    if (uVisitInterval == 1) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s invalid (policer max rate %d, policer max burst %d policer limit %d) "
                                      "combination for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sPolicer.uMaxRateKbps,
                           config->u.sPolicer.uMaxBurstBits,
                           config->nSegmentLimit, segment, unit));
		return SOC_E_PARAM;
	    } else if (uVisitInterval > 0xFFFFF) {
		uVisitInterval = 0xFFFFF;
	    }
	    
	    /* put in lower 20 bits of visit_period */
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_LOr, &uRegValue2,
			      VISIT_PERIOD_19_0f, (uTemp & 0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, uRegValue2));
	    
	    /* put in higer 20 bits of visit_period */
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_HIr, &uRegValue2,
			      VISIT_PERIOD_39_20f, ((uTemp >> 20) & 0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, uRegValue2));
	    
	    /* put in the visit_interval */
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_VISIT_INTERVALr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_VISIT_INTERVALr, &uRegValue2,
			      VISIT_INTERVALf, (uVisitInterval&0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_VISIT_INTERVALr(unit, nCop, segment, uRegValue2));

	    /* calculate OCM size */
	    uOcmSize = (config->nSegmentLimit+1) * sizeof(uint64);
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER:
	    /* timer mode */
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      TIMER_MODE64f, config->u.sTimer.bMode64?1:0);
		
	    /*   INPUT:
	     *    numTimerInSegment
	     *    nnTimerTickNs
	     *    bMode64
	     *   FORMULA:
	     *    visit_period = number of cycles per timer tick (40 bits)
	     *    visit_interval = floor((visit_period * 2)/(numTimerInSegment * (bMode64?2:1))
	     */
	    if (config->nSegmentLimit < 0) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s require number of timers to update timer tick for segment %d on unit %d\n"),
                           FUNCTION_NAME(), segment, unit));
		return SOC_E_PARAM;
	    }

	    /* adjust for cop clock, which is tied with core clock
	     * assuming we are supporting clock in multiple of 100Mhz
	     */
	    uTemp = (config->u.sTimer.nTimerTickUs * (SOC_SBX_CFG(unit)->uClockSpeedInMHz / 100)) /10;

	    /* 0x4189374B = 0xFFFFFFFFFF/1000 */
	    if (uTemp > (0x4189374B)) {
		/* at 1G HZ, 40bits timer tick, make sure we don't overflow */
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s timer tick %d us not supported for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sTimer.nTimerTickUs, segment, unit));
		return SOC_E_PARAM;			
	    }
	    
	    /* Don't have a 64 bits multiplier, user shift/sub to achieve following
	     *   nTimerTickUs * 1000 = nTimerTickUs * (1024 - 16 - 8)
	     */
            COMPILER_64_SET(uuTemp, 0, uTemp);
	    COMPILER_64_SHL(uuTemp, 10);                 /* uuTemp = nTimerTickUs * 1024 */
	    uuVisitPeriod = uuTemp;
	    COMPILER_64_SHR(uuTemp, 6);                  /* uuTemp = nTimerTickUs * 16 */
	    COMPILER_64_SUB_64(uuVisitPeriod, uuTemp);   
	    COMPILER_64_SHR(uuTemp, 1);                  /* uuTemp = nTimerTickUs * 8 */
	    COMPILER_64_SUB_64(uuVisitPeriod, uuTemp);   /* uuVistPeriod = nTimerTickUs * 1000 */
	    uuTemp =  uuVisitPeriod;
	    
	    /* put in lower 20 bits of uuVisitPeriod */
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_LOr, &uRegValue2,
			      VISIT_PERIOD_19_0f, (COMPILER_64_LO(uuVisitPeriod) & 0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, uRegValue2));
	    
	    /* put in higer 20 bits of uuVisitPeriod */
	    COMPILER_64_SHR(uuVisitPeriod, 20);
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_HIr, &uRegValue2,
			      VISIT_PERIOD_39_20f, (COMPILER_64_LO(uuVisitPeriod) & 0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, uRegValue2));
	    
	    /* calculate visit_interval */
	    if (config->u.sTimer.bMode64 != TRUE) {
		COMPILER_64_SHL(uuTemp, 1);                  /* uuTemp = uuVisitPeriod * 2 */
	    }
	    nRv = soc_sbx_div64(uuTemp, config->nSegmentLimit+1, &uVisitInterval);
	    if (SOC_FAILURE(nRv) || (uVisitInterval >= 0xFFFFF)) {
		/* set it to longest possible */
		uVisitInterval = 0;
	    } else if (uVisitInterval == 1) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s invalid (timer tick %d, timer limit %d) combination "
                                      "for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sTimer.nTimerTickUs,
                           config->nSegmentLimit, segment, unit));
		return SOC_E_PARAM;
	    }
	    
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_VISIT_INTERVALr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_VISIT_INTERVALr, &uRegValue2,
			      VISIT_INTERVALf, (uVisitInterval&0xFFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_VISIT_INTERVALr(unit, nCop, segment, uRegValue2));

	    /* calculate OCM size */
	    if (config->u.sTimer.bMode64) {
		uOcmSize = (config->nSegmentLimit + 1) * sizeof(uint64);
	    } else {
		uOcmSize = (config->nSegmentLimit + 1) * sizeof(uint32);
	    }
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER:
	    /* sequence checker mode */
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      CHECKER_MODE32f, config->u.sChecker.bMode32?1:0);

	    /* sequence checker range */
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_LOr, &uRegValue2,
			      SEQUENCE_RANGE_15_0f, (config->u.sChecker.uSequenceRange & 0xFFFF));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, uRegValue2));
	    
	    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, &uRegValue2));
	    soc_reg_field_set(unit, CO_SEGMENT_MISC_CONFIG_HIr, &uRegValue2,
			      SEQUENCE_RANGE_31_16f, (config->u.sChecker.uSequenceRange>>16));
	    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, uRegValue2));

	    /* calculate OCM size */
	    if (config->u.sChecker.bMode32) {
		uOcmSize = (config->nSegmentLimit + 1) * sizeof(uint32);
	    } else {
		uOcmSize = (config->nSegmentLimit + 1) * sizeof(uint16);
	    }
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT:
	    /* coherent table configs */
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      COHERENT_TABLE_RETURN_NEXTf, config->u.sCoherent.bReturnNext?1:0);
	    if (config->u.sCoherent.nOverflowMode >= SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_MAX) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unsupported overflow mode %d for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sCoherent.nOverflowMode, segment, unit));
		return SOC_E_PARAM;
	    }		    
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      COHERENT_TABLE_OVERFLOW_MODEf, config->u.sCoherent.nOverflowMode);

	    if (config->u.sCoherent.nFormat >= SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_MAX) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unsupported format %d for segment %d on unit %d\n"),
                           FUNCTION_NAME(), config->u.sCoherent.nFormat, segment, unit));
		return SOC_E_PARAM;
	    }		    
	    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue,
			      COHERENT_TABLE_FORMATf, config->u.sCoherent.nFormat);

	    if ((config->u.sCoherent.nFormat == SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_64BIT) ||
		(config->u.sCoherent.nFormat == SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2x32BIT)) {
		uOcmSize = (config->nSegmentLimit + 1) * sizeof(uint64);
	    } else {
		uOcmSize = (config->nSegmentLimit + 1) * (1<<config->u.sCoherent.nFormat) / 8;
		if ((uOcmSize == 0) && (config->nSegmentLimit >= 0)) {
		    /* at least take 1 64bits entry */
		    uOcmSize = sizeof(uint64);
		} else {
		    /* round up to multiple of 64bits */
		    uOcmSize = ((uOcmSize + (sizeof(uint64)-1)) / sizeof(uint64)) * sizeof(uint64); 
		}
	    }
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unsupported segment type %d for segment %d on unit %d\n"),
                       FUNCTION_NAME(), config->nSegmentType, segment, unit));
	    return SOC_E_PARAM;
    }

    soc_reg_field_set(unit, CO_SEGMENT_CONFIGr, &uRegValue, Tf, config->nSegmentType);
    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_CONFIGr(unit, nCop, segment, uRegValue));	

    /* segment limit */
    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_LIMITr(unit, nCop, segment,
						config->nSegmentLimit));

    /* segment base, allocate ocm memory */
    nRv = _soc_sbx_caladan3_cop_ocm_alloc(unit, cop, uOcmSize, &config->nSegmentOcmBase,
					  &config->nSegmentOcmSize);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to allocate OCM memory for COP %d "
                              "segment %d on unit %d.\n"),
                   cop, segment, unit));
	return nRv;
    } else {
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "COP %d segment %d ocm base at 0x%x on unit %d.\n"),
	             cop, segment, (uint32)config->nSegmentOcmBase, unit));
    }

    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_BASEr(unit, nCop, segment,
					       config->nSegmentOcmBase));

    /* segment enable */
    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_ENABLEr(unit, nCop, &uRegValue));
    uRegValue |= (1 << segment);
    SOC_IF_ERROR_RETURN(WRITE_CO_SEGMENT_ENABLEr(unit, nCop, uRegValue));

    /* update the control info */
    if (pCopCfg->segments[cop] != NULL) {
	pCopCfg->segments[cop][segment] = *config;
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_command_inject
 *   Purpose
 *      inject a command through command inject interface
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) operation : operation type
 *                       SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_STICKY operation could only
 *                       done on a Coherent table segment with overflow mode set to 
 *                       SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_STICKY and with 
 *                       a SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_COUNT command.
 *      (IN) command   : 3 words command, word 0 for CO_INJECT_DATA0 (bits 31-0)
 *      (OUT)response  : 2 words response. word 0 for CO_INJECT_DATA0 (bits 31-0)
 *                       set to NULL if no response needed
 *   Returns
 *       SOC_E_NONE    - successfully initialized
 *       SOC_E_TIMEOUT - command timed out for Non-STICKY operation
 *                       command abort timed out for STICKY operation
 *       SOC_E_BUSY    - apply for STICKY operation 
 */
int soc_sbx_caladan3_cop_command_inject(int unit,
					int cop,
					int segment,
					soc_sbx_caladan3_cop_command_operation_e_t operation,
					uint32 *command,
					uint32 *response)
{
    int nRv = SOC_E_NONE;
    int nCop, nDone;
    uint32 uRegValue = 0;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;
    sal_usecs_t    sTimeout = 10 * MILLISECOND_USEC;
    soc_timeout_t  sTo;

    /* sanity checks */
    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(cop);
    }

    if ((segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* check if segment is enabled */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if (pSegCfg->bEnabled != TRUE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not in use on unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;	
    }

    if (command == NULL) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d no command specified on unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;	
    }

    /* make sure the sticky operation is done on the correctly configed segment */
    if ((operation == SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT_STICKY) &&
	((pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT) ||
	 (pSegCfg->u.sCoherent.nOverflowMode != SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_STICKY) ||
	 (SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_COUNT !=soc_reg_field_get(unit, CO_INJECT_DATA2r,
									  command[2], TOPf)))) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not support sticky count command on unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    /*need to make sure no other injection is outstanding, use LOCKs */
    COP_INJECT_LOCK(unit, cop);

    /* For now, only support abort during semaphore operation */
    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(WRITE_CO_INJECT_DATA0r(unit, nCop, command[0]), cop);
    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(WRITE_CO_INJECT_DATA1r(unit, nCop, command[1]), cop);
    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(WRITE_CO_INJECT_DATA2r(unit, nCop, command[2]), cop);

    /* initiate the operation */
    uRegValue = 0;
    soc_reg_field_set(unit, CO_INJECT_CTRLr, &uRegValue, REQf, 1);
    soc_reg_field_set(unit, CO_INJECT_CTRLr, &uRegValue, SEGMENTf, (uint32) segment);
    soc_reg_field_set(unit, CO_INJECT_CTRLr, &uRegValue, ABORTf, 0);
    switch (operation) {
	case SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT:
	    soc_reg_field_set(unit, CO_INJECT_CTRLr, &uRegValue, STICKY_WAITf, 0);
	    break;
	case SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT_STICKY:
	    soc_reg_field_set(unit, CO_INJECT_CTRLr, &uRegValue, STICKY_WAITf, 1);
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s cop %d segment %d unsupported operation %d on unit %d\n"),
                       FUNCTION_NAME(), cop, segment, operation, unit));
	    COP_INJECT_UNLOCK(unit, cop);
	    return SOC_E_PARAM;		    
    }
    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(WRITE_CO_INJECT_CTRLr(unit, nCop, uRegValue), cop);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s inject command ctrl 0x%x, command 0x%8x 0x%8x 0x%8x on cop %d segment %d unit %d.\n"),
                 FUNCTION_NAME(), uRegValue, command[0], command[1], command[2], cop, segment, unit));

    /* poll for done */
    if (SAL_BOOT_QUICKTURN) {
	/* quickturn is much more slower */
	sTimeout *= 100;
    }

    nDone = FALSE;
    soc_timeout_init(&sTo, sTimeout, 0);
    while(!soc_timeout_check(&sTo)) {
	nRv = READ_CO_INJECT_STATUSr(unit, nCop, &uRegValue);
	if (SOC_FAILURE(nRv)) {
	    break;
	}
	nDone = soc_reg_field_get(unit, CO_INJECT_STATUSr, uRegValue, ACKf);
	if (nDone) {
	    WRITE_CO_INJECT_STATUSr(unit, nCop, uRegValue);
	    break;
	}
    }

    if (nDone || SAL_BOOT_PLISIM) {
	/* return response */
	if (response != NULL) {
	    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(READ_CO_INJECT_DATA0r(unit, nCop, &response[0]), cop);
	    _COP_IF_ERROR_INJECT_UNLOCK_RETURN(READ_CO_INJECT_DATA1r(unit, nCop, &response[1]), cop);
	}
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: cop %d sbus injected command timed out\n"),
                   FUNCTION_NAME(), cop));
	nRv = SOC_E_TIMEOUT;

	if (operation == SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT_STICKY) {
	    
	    /* abort */

	    /* poll for abort_ack bit, it should be done quickly */

	    /* check the ack bit, if it's set, it means we got the semphore lock anyway,
	     * return SOC_E_NONE. If it's not set, it mean we didn't get the lock,
	     * return SOC_E_BUSY to indicate that we should try to relock it. If 
	     * abort timed out, return SOC_E_TIMEOUT to indicate failure
	     */
	    nRv = SOC_E_BUSY;
	}
    }

    COP_INJECT_UNLOCK(unit, cop);
    return nRv;    
}


/*
 *   Function
 *     sbx_caladan3_cop_policer_state_init
 *   Purpose
 *      COP Init an policer entry in OCM memory
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) id        : object ID in the segment
 *      (IN) profile   : policer profile ID
 *
 *   Returns
 *       SOC_E_NONE -  successfully initialized
 *       SOC_E_*       failed
 */
int soc_sbx_caladan3_cop_policer_state_init(int unit,
					    int cop,
					    int segment,
					    int id,
					    uint32 profile)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s cop %d init policer %d with profile %d on segment %d unit %d.\n"),
                 FUNCTION_NAME(), cop, id, profile, segment, unit));

    return _soc_sbx_caladan3_cop_object_state_init(unit, cop, segment,
						   id, profile, 0);
}

/*
 *   Function
 *     sbx_caladan3_cop_timer_state_init
 *   Purpose
 *      COP Init an timer entry in OCM memory
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) id        : object ID in the segment
 *      (IN) timeout   : timer timeout value
 *      (IN) interrupt_en   : TRUE  - timer expire trigger interrupt
 *                            FALSE - no interrupt
 *      (IN) start     : TRUE - timer start once created
 *                       FALSE- timer will not start after created. ucode need to start it.
 *   Returns
 *       SOC_E_NONE -  successfully initialized
 *       SOC_E_*       failed
 */
int soc_sbx_caladan3_cop_timer_state_init(int unit,
					  int cop,
					  int segment,
					  int id,
					  uint32 timeout,
					  uint32 interrupt_en,
					  uint32 start)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s cop %d init timer %d with timeout 0x%x interrupt %s "
                            "on segment %d unit %d.\n"),
                 FUNCTION_NAME(), cop, id, timeout, interrupt_en?"enabled":"disabled",
                 segment, unit));

    return _soc_sbx_caladan3_cop_object_state_init(unit, cop, segment, id, timeout,
						   (((interrupt_en?1:0) << _COP_TIMER_INTERRUPT_EN_BITPOS ) |
						    ((start?1:0) << _COP_TIMER_START_BITPOS)));
}

/*
 *   Function
 *     sbx_caladan3_cop_sequence_checker_state_init
 *   Purpose
 *      COP Init an sequence checker entry in OCM memory
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) id        : object ID in the segment
 *      (IN) seq_number: initial value of sequence number
 *
 *   Returns
 *       SOC_E_NONE -  successfully initialized
 *       SOC_E_*       failed
 */
int soc_sbx_caladan3_cop_sequence_checker_state_init(int unit,
						     int cop,
						     int segment,
						     int id,
						     uint32 seq_number)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s cop %d init SN checker %d with init sequence number 0x%x "
                            "on segment %d unit %d.\n"),
                 FUNCTION_NAME(), cop, id, seq_number, segment, unit));

    return _soc_sbx_caladan3_cop_object_state_init(unit, cop, segment,
						   id, seq_number, 0);
}

/*
 *   Function
 *     sbx_caladan3_cop_sequence_checker_state_init
 *   Purpose
 *      COP Init an coherent table entry in OCM memory
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) id        : object ID in the segment
 *      (IN) bits31_0  : bits 31-0 of the table entry initial value
 *      (IN) bits63_32 : bits 63-32 of the table entry initial value
 *                       only apply when table entry is 64bits wide
 *
 *   Returns
 *       SOC_E_NONE -  successfully initialized
 *       SOC_E_*       failed
 */
int soc_sbx_caladan3_cop_coherent_table_state_init(int unit,
						   int cop,
						   int segment,
						   int id,
						   uint32 bits31_0,
						   uint32 bits63_32)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s cop %d init coherent table %d with 0x%x%x on segment %d unit %d.\n"),
                 FUNCTION_NAME(), cop, id, bits63_32, bits31_0, segment, unit));

    return _soc_sbx_caladan3_cop_object_state_init(unit, cop, segment,
						   id, bits31_0, bits63_32);
}

/*
 *   Function
 *     _soc_sbx_caladan3_cop_object_state_init
 *   Purpose
 *      COP init state of an object in OCM memory through command injection interface
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) cop       : cop instance 0 or 1
 *      (IN) segment   : cop segment 0-31
 *      (IN) id        : object ID in the segment
 *      (IN) param     : segment type specific param
 *      (IN) param_hi  : segment type specific param
 *
 *   Returns
 *       SOC_E_NONE -  successfully initialized
 *       SOC_E_PARAM   wrong parameter
 *       SOC_E_TIMEOUT init timedout
 */
static int _soc_sbx_caladan3_cop_object_state_init(int unit,
						   int cop,
						   int segment,
						   int id,
						   uint32 param,
						   uint32 param_hi)
{
    int nRv = SOC_E_NONE;
    uint32 uCommand[3];
    soc_sbx_caladan3_cop_segment_type_e_t eType;
    co_meter_state_entry_t            sPolicer;
    co_watchdog_timer_state_entry_t   sTimer;
    co_sequence_checker_state_entry_t sChecker;
    co_coherent_table_state_entry_t   sCoherent;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;
   
    /* build commands */
    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if ((segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* check if segment is enabled */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if (pSegCfg->bEnabled != TRUE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not in use on unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;	
    }

    /* check if id is in range */
    if ((id < 0) || (id > pSegCfg->nSegmentLimit)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s id %d out of cop %d segment %d ID range"
                              " 0-%d on unit %d\n"),
                   FUNCTION_NAME(), id, cop, segment, pSegCfg->nSegmentLimit, unit));
	return SOC_E_PARAM;	
    }

    /* build command buffer based on segment type */
    eType = pSegCfg->nSegmentType;
    switch (eType) {
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER:	    
	    uCommand[2] = 0;
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], WEf, 1);
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], OFFSETf, id);

	    if(param > 0x3FF) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s policer profile %d out of range"
                                      " 0-0x3FF on cop %d segment %d unit %d\n"),
                           FUNCTION_NAME(), param, cop, segment, unit));
		return SOC_E_PARAM;
	    }

	    /* profile = param, bkt_e=bkt_c=0x7ffFFFF.
	     * If NULL_PROFILE, bkt_e=bkt_c=0
	     */
	    sal_memset(&sPolicer, 0, sizeof(sPolicer));
	    soc_mem_field32_set(unit, CO_METER_STATEm, &sPolicer, PROFILEf, param & 0x3FF);
	    if ((param & 0x3FF) != SOC_SBX_CALADAN3_COP_NULL_PROFILE) {
		soc_mem_field32_set(unit, CO_METER_STATEm, &sPolicer, BKT_Ef, 0x7ffFFFF);
		soc_mem_field32_set(unit, CO_METER_STATEm, &sPolicer, BKT_Cf, 0x7ffFFFF);
	    }

	    uCommand[1] = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer,
					      DATA_63_32f);
	    uCommand[0] = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer,
					      DATA_31_0f);
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER:
	    uCommand[2] = 0;
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], WEf, 1);
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], OFFSETf, id);

	    if(param > 0x3fffFFFF) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s timer timeout %d out of range"
                                      " 0-0x3fffFFFF on cop %d segment %d unit %d\n"),
                           FUNCTION_NAME(), param, cop, segment, unit));
		return SOC_E_PARAM;
	    }
	    
	    /* started = 1, timeout = param, interrupt_en = param_hi?1:0 */
	    sal_memset(&sTimer, 0, sizeof(sTimer));
      	    if (pSegCfg->u.sTimer.bMode64) {
		soc_mem_field32_set(unit, CO_WATCHDOG_TIMER_STATEm, &sTimer, TIMEOUTf,
				    param);
	    }
	    soc_mem_field32_set(unit, CO_WATCHDOG_TIMER_STATEm, &sTimer, TIMERf,
				param & 0x3fffFFFF);
	    soc_mem_field32_set(unit, CO_WATCHDOG_TIMER_STATEm, &sTimer, STARTEDf,
				(param_hi & (1<<_COP_TIMER_START_BITPOS))?1:0);
	    soc_mem_field32_set(unit, CO_WATCHDOG_TIMER_STATEm, &sTimer, INTERRUPT_ENf,
				(param_hi & (1<<_COP_TIMER_INTERRUPT_EN_BITPOS))?1:0);

	    uCommand[1] = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm, 
					      &sTimer, DATA_63_32f);
	    uCommand[0] = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm,
					      &sTimer, DATA_31_0f);
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER:
	    uCommand[2] = 0;
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], WEf, 1);
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], OFFSETf, id);

	    if ((pSegCfg->u.sChecker.bMode32 == FALSE) &&
		(param > 0xFFFF)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s initial sequence number 0x%x out of range"
                                      " 0-0xFFFF on cop %d segment %d unit %d\n"),
                           FUNCTION_NAME(), param, cop, segment, unit));
		return SOC_E_PARAM;
	    }
	    	    
	    /* sequence number init value = param */
	    sal_memset(&sChecker, 0, sizeof(sChecker));
	    if (pSegCfg->u.sChecker.bMode32) {
		soc_mem_field32_set(unit, CO_SEQUENCE_CHECKER_STATEm, &sChecker,
				    SQN_32BITSf, param);				    
	    } else {
		soc_mem_field32_set(unit, CO_SEQUENCE_CHECKER_STATEm, &sChecker,
				    SQN_16BITSf, param & 0xFFFF);
	    }

	    uCommand[1] = soc_mem_field32_get(unit, CO_SEQUENCE_CHECKER_STATEm,
					      &sChecker, DATA_63_32f);
	    uCommand[0] = soc_mem_field32_get(unit, CO_SEQUENCE_CHECKER_STATEm,
					      &sChecker, DATA_31_0f);
	    break;
	case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT:
	    uCommand[2] = 0;
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], TOPf,
			      SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_WRITE);
	    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uCommand[2], OFFSETf, id);

	    if ((pSegCfg->u.sCoherent.nFormat <= SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_16BIT) &&
		(param > ((2<<pSegCfg->u.sCoherent.nFormat)-1))) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s initial coherent table value 0x%x out of range"
                                      " 0-0x%x on cop %d segment %d unit %d\n"),
                           FUNCTION_NAME(), param, (2<<pSegCfg->u.sCoherent.nFormat)-1,
                           cop, segment, unit));
		return SOC_E_PARAM;
	    }

	    /* table init value = param */
	    sal_memset(&sCoherent, 0, sizeof(sCoherent));
	    switch (pSegCfg->u.sCoherent.nFormat) {
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_1BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_1BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_2BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_4BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_4BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_8BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_8BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_16BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_16BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_32BIT:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_32BITf, param);
		    break;
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_64BIT:
		case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2x32BIT:
		default:
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_32BITf, param);
		    soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
					CT_63_32BITf, param_hi);
		    break;
	    }
	    
	    uCommand[1] = soc_mem_field32_get(unit, CO_COHERENT_TABLE_STATEm,
					      &sCoherent, DATA_63_32f);
	    uCommand[0] = soc_mem_field32_get(unit, CO_COHERENT_TABLE_STATEm,
					      &sCoherent, DATA_31_0f);
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s cop %d segment %d unsupported segment type on unit %d\n"),
                       FUNCTION_NAME(), cop, segment, unit));
	    return SOC_E_PARAM;
    }

    /* initialize throught the sbus command injection register interface
     * no need to get back the command response
     */
    nRv = soc_sbx_caladan3_cop_command_inject(unit, cop, segment,
					      SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT,
					      uCommand,
					      NULL);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to inject command on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return nRv;
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_profile_alloc
 *   Purpose
 *      COP allocate a profile
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) cop     : cop instance 0 or 1
 *      (IN) config  : profile config parameters
 *      (OUT) profile: allocated profile.
 *                     SOC_SBX_CALADAN3_COP_NULL_PROFILE if failed
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_profile_alloc(int unit,
				       int cop,
				       soc_sbx_caladan3_cop_profile_t *config,
				       uint32 *profile)
{
    int nRv = SOC_E_NONE;
    int32 nCop;
    uint32 bFound, uId;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;
#define SOC_SBX_CALADAN3_CA3_3098_WORK_AROUND 1

#ifdef SOC_SBX_CALADAN3_CA3_3098_WORK_AROUND
    int count = 0;
    co_meter_profile_entry_t sHwProfile_inHw;
#endif

    *profile = SOC_SBX_CALADAN3_COP_NULL_PROFILE;

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = CO_BLOCK(unit, cop);
    }

    /* search for matching profile */
    bFound = FALSE;
    for (uId = SOC_SBX_CALADAN3_COP_NULL_PROFILE + 1;
	 uId < SOC_SBX_CALADAN3_COP_NUM_PROFILE;
	 uId++) {
	if (pCopCfg->profiles[cop][uId].uRefCount) {
	    if ((pCopCfg->profiles[cop][uId].uCIR == config->uCIR) &&
		(pCopCfg->profiles[cop][uId].uCBS == config->uCBS) &&
		(pCopCfg->profiles[cop][uId].uEIR == config->uEIR) &&
		(pCopCfg->profiles[cop][uId].uEBS == config->uEBS) &&
		(pCopCfg->profiles[cop][uId].uRfcMode == config->uRfcMode) &&
		(soc_mem_compare_entry(unit, CO_METER_PROFILEm,
				       &(pCopCfg->profiles[cop][uId].sHwProfile),
				       &(config->sHwProfile)) == 0)) {
		/* profile settings match */
		bFound = TRUE;
		*profile = uId;
		break;
	    }
	}
    }

    if (!bFound) {
	/* allocate an unused profile */
	for (uId = SOC_SBX_CALADAN3_COP_NULL_PROFILE + 1;
	     uId < SOC_SBX_CALADAN3_COP_NUM_PROFILE;
	     uId++) {
	    if (pCopCfg->profiles[cop][uId].uRefCount == 0) {
		bFound = TRUE;

                /* Initialize profile data */
                pCopCfg->profiles[cop][uId].uCIR = config->uCIR;
                pCopCfg->profiles[cop][uId].uCBS = config->uCBS;
                pCopCfg->profiles[cop][uId].uEIR = config->uEIR;
                pCopCfg->profiles[cop][uId].uEBS = config->uEBS;
                pCopCfg->profiles[cop][uId].uRfcMode = config->uRfcMode;
                pCopCfg->profiles[cop][uId].sHwProfile = config->sHwProfile;

		*profile = uId;
		break;
	    }
	}

	if (!bFound) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s COP %d exhaust all profiles on unit %d\n"),
                       FUNCTION_NAME(), cop, unit));
	    return SOC_E_RESOURCE;
	} else {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "%s allocated new profile %d on cop %d unit %d.\n"),
	                 FUNCTION_NAME(), *profile, cop, unit));
	}

#ifdef SOC_SBX_CALADAN3_CA3_3098_WORK_AROUND
	/* config the profile memory */
    do{
        if(count)
        {
	        LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
                    "Additional #%d attempt  to write CO_METER_PROFILEm\n"), count));
        }
	    SOC_IF_ERROR_RETURN(WRITE_CO_METER_PROFILEm(unit, nCop, *profile, &(config->sHwProfile)));
	    SOC_IF_ERROR_RETURN(READ_CO_METER_PROFILEm(unit, nCop, *profile, &sHwProfile_inHw));
        count++;
        if(count > 10)
        {
	        LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s nCop %d profile %d exhausted all 10 attemts to write on unit %d\n"),
                       FUNCTION_NAME(), nCop, *profile, unit));
            break;
        }
    }while((config->sHwProfile.entry_data[0] != sHwProfile_inHw.entry_data[0]) 
        || (config->sHwProfile.entry_data[1] != sHwProfile_inHw.entry_data[1]));

#else
	/* config the profile memory */
	SOC_IF_ERROR_RETURN(WRITE_CO_METER_PROFILEm(unit, nCop, *profile, &(config->sHwProfile)));
#endif
    } else {
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s found matching profile %d on cop %d unit %d.\n"),
	             FUNCTION_NAME(), *profile, cop, unit));
    }

    pCopCfg->profiles[cop][*profile].uRefCount++;
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_profile_dealloc
 *   Purpose
 *      COP dealloc a profile (decrement reference count,
 *        and clear profile memory once the refcount goes to 0)
 *   Parameters
 *      (IN) unit    : unit number of the device
 *      (IN) cop     : cop instance 0 or 1
 *      (IN) profile : profile ID.
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 *   NOTE: 
 *       NULL profile SOC_SBX_CALADAN3_COP_NULL_PROFILE
 *       can not be de-allocated
 */
int soc_sbx_caladan3_cop_profile_dealloc(int unit,
					 int cop,
					 uint32 profile)
{
    int nRv = SOC_E_NONE;
    int32 nCop;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = CO_BLOCK(unit, cop);
    }

    if (profile >= SOC_SBX_CALADAN3_COP_NUM_PROFILE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d profile %d out of range 0-1023 on unit %d\n"),
                   FUNCTION_NAME(), cop, profile, unit));
	return SOC_E_PARAM;
    }

    if (profile == SOC_SBX_CALADAN3_COP_NULL_PROFILE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d null profile %d can not be deallocated on unit %d\n"),
                   FUNCTION_NAME(), cop, profile, unit));
	return SOC_E_PARAM;
    }

    if (pCopCfg->profiles[cop][profile].uRefCount == 0) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d profile %d not in use on unit %d\n"),
                   FUNCTION_NAME(), cop, profile, unit));
	return SOC_E_PARAM;
    }

    pCopCfg->profiles[cop][profile].uRefCount--;
    if (pCopCfg->profiles[cop][profile].uRefCount == 0) {
	/* clear the memory */
	sal_memset(&(pCopCfg->profiles[cop][profile]), 0, sizeof(soc_sbx_caladan3_cop_profile_t));

	nRv = WRITE_CO_METER_PROFILEm(unit, nCop, profile, &(pCopCfg->profiles[cop][profile].sHwProfile));

	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s failed to delete policer profile %d on cop %d unit %d\n"),
                       FUNCTION_NAME(), profile, cop, unit));
	    return nRv;
	} else {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "%s deleted profile %d on cop %d unit %d.\n"),
	                 FUNCTION_NAME(), profile, cop, unit));
	}
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_null_profile_create
 *   Purpose
 *      COP create NULL profiles on both COPs.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
static int _soc_sbx_caladan3_cop_null_profile_create(int unit)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;

    sal_memset(&(pCopCfg->profiles[0][SOC_SBX_CALADAN3_COP_NULL_PROFILE]),
	       0, sizeof(soc_sbx_caladan3_cop_profile_t));
    
    nRv = WRITE_CO_METER_PROFILEm(unit, CO0_BLOCK(unit), SOC_SBX_CALADAN3_COP_NULL_PROFILE,
				  &(pCopCfg->profiles[0][SOC_SBX_CALADAN3_COP_NULL_PROFILE].sHwProfile));

    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create NULL profile on cop0 unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    } else {

    }

    sal_memset(&(pCopCfg->profiles[1][SOC_SBX_CALADAN3_COP_NULL_PROFILE]),
	       0, sizeof(soc_sbx_caladan3_cop_profile_t));
    
    nRv = WRITE_CO_METER_PROFILEm(unit, CO1_BLOCK(unit), SOC_SBX_CALADAN3_COP_NULL_PROFILE,
				  &(pCopCfg->profiles[1][SOC_SBX_CALADAN3_COP_NULL_PROFILE].sHwProfile));

    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create NULL profile on cop1 unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_encode_rate
 *   Purpose
 *      encode rate/burst to exp/mant format
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) mem    : memory type
 *      (IN) field  : field type
 *      (IN) raw    : raw value, depends on mem/field
 *      (OUT) exp   : converted exponent value
 *      (OUT) mant  : converted mantissa value
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_encode_rate(int unit,
				     soc_mem_t mem,
				     soc_field_t field,
				     uint32 raw,
				     uint32 *exp,
				     uint32 *mant)
{
    int nRv = SOC_E_NONE;
    uint64 uuTemp;
    uint32 uTemp;

    if (raw == 0) {
	*exp = 0;
	*mant = 0;
    }
    switch (mem) {
	case CO_METER_PROFILEm:
	    switch (field) {
		case CIRf:
		case EIRf:
		    /* CIR/EIR encoding
		     *   CIR passed in kbps
		     *   CIR convert to bytes/cycle
		     *   CIR = (1+mant/2048)*(1<<(exp-27));
		     */

		    /* adjust for cop clock, which is tied with core clock
		     * assuming we are supporting clock in multiple of 100Mhz
		     */
		    raw = (raw * 10) / (SOC_SBX_CFG(unit)->uClockSpeedInMHz / 100);

            if( 0 == raw) {
                *exp = 0;
                *mant = 0;
            } else if (raw >= 0xF414BDC) {
                /* over flowed at around 256Gbps */
                *exp = 31;
                *mant = 0x7FF;
            } else {
			/* raw * 1000 / (8 * 1000000000) = bytes/cycle
			 * CIR * (1<<23) = (1+mant/2048)*(1<<(exp-4))
			 * raw * 1024 * 1024 / (1000000) = (1+mant/2048)*(1<<(exp-4))
			 */
			COMPILER_64_SET(uuTemp, 0, raw);
			COMPILER_64_SHL(uuTemp, 20);
			nRv = soc_sbx_div64(uuTemp, 1000000, &uTemp);
			if (SOC_FAILURE(nRv)) {
			    /* should never happen here */
			    return nRv;
			}

			if(uTemp <= 0xFFF) {
			    /* find the exponent */
			    for(*exp = 0; *exp < 12; (*exp)++) {
				if( (uTemp >> *exp) == 1 ) {
				    *exp = *exp + 4;
				    break;
				}
			    }
			    /* find mantissa */
			    *mant = (uTemp << (15 - *exp)) & 0x7FF;
			} else {
			    for(*exp=12; *exp <=31; (*exp)++) {
				if( (uTemp >> *exp) == 1 ) {
				    *exp = *exp + 4;
				    break;
				}
			    } 
			    *mant = (uTemp >> (*exp - 15)) & 0x7FF;
			}
		    } 
		    LOG_VERBOSE(BSL_LS_SOC_COMMON,
		                (BSL_META_U(unit,
		                            "%s converted CIR/EIR 0x%x kbps to exp %d mant %d.\n"),
		                 FUNCTION_NAME(), raw, *exp, *mant));
		    break;
		case CBSf:
		case EBSf:
		    /* CBS/EBS encoding 
		     *   CBS passed in bytes
		     *   mant: 7bits, exp:5 bits
		     *   CBS = (0x80 + mant) >> (exp-8)
		     */
            if (raw >= ((0x80+0x7f) << (31-8))) {
                *exp = 31;
                *mant = 0x7F;
            } else {
                /* find the exponent */
                for (*exp = 0; (raw >> *exp) > 0; (*exp)++)
                    ;
                /* find mantissa */
                if (*exp > 8) {
                    *mant = (raw >> (*exp - 8)) & 0x7F;
                } else {
                    *mant = (raw << (8 - *exp)) & 0x7F;
                }
            }
		    LOG_VERBOSE(BSL_LS_SOC_COMMON,
		                (BSL_META_U(unit,
		                            "%s converted CBS/EBS 0x%x bytes to exp %d mant %d.\n"),
		                 FUNCTION_NAME(), raw, *exp, *mant));
		    break;
		default:
		    nRv = SOC_E_UNAVAIL;
		    break;
	    }
	    break;
	default:
	    nRv = SOC_E_UNAVAIL;
	    break;
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_policer_create
 *   Purpose
 *      COP create a policer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) policer: policer ID
 *      (IN) config : policer configuration parameter
 *      (OUT) handle: policer handle, should be used when delete
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_policer_create(int unit,
					uint32 cop,
					uint32 segment,
					uint32 policer,
					soc_sbx_caladan3_cop_policer_config_t *config,
					uint32 *handle)
{
    int nRv = SOC_E_NONE;
    uint32 uCirExp, uCirMant, uEirExp, uEirMant;
    uint32 uCbsExp, uCbsMant, uEbsExp, uEbsMant;
    uint32 uProfileID, uLenAdjust;
    uint32 bRfc2698;
    soc_sbx_caladan3_cop_profile_t sProfile;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;
    soc_sbx_caladan3_cop_policer_config_t sConfig;
    uint32 uRegValue;
    int nCop, nScaleFactor, nCirScale, nEirScale;

    /*== step 1: check if the policer id is valid and in_use 
     *         check if policer parameter fit the segment config
     */
    if (((int)cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(cop);
    }

    if (((int)segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create policer on cop %d segment %d unit %d "
                              "which is not an enabled policer segment\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    if (policer > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), policer, cop, segment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }
    
    if (config == NULL) {
	/* make sure there is a config to be safe */
	return SOC_E_PARAM;
    }

    if ((config->uCIR > pSegCfg->u.sPolicer.uMaxRateKbps) ||
	(config->uEIR > pSegCfg->u.sPolicer.uMaxRateKbps)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d CIR/EIR too high for cop %d segment %d unit %d "
                              "max rate 0x%x kbps\n"),
                   FUNCTION_NAME(), policer, cop, segment, unit,
                   pSegCfg->u.sPolicer.uMaxRateKbps));
	return SOC_E_PARAM;
    }

    /*== step 2: convert the policer CIR/EIR/CBS/EBS to exp/mant format */
    sConfig = *config;
    switch(sConfig.uRfcMode) {
	case SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2697:
	    sConfig.uEIR = 0;
	    sConfig.bCoupling = TRUE;
	    bRfc2698 = FALSE;
	    break;
	case SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2698:	    
	    sConfig.bCoupling = FALSE;
	    bRfc2698 = TRUE;
	    break;
	case SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_4115:
	    sConfig.bCoupling = FALSE;
	    bRfc2698 = FALSE;
	    break;
	case SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_MEF:
	    bRfc2698 = FALSE;
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s policer ID %d unsupported rfc mode on unit %d\n"),
                       FUNCTION_NAME(), policer, unit));
	    return SOC_E_PARAM;
    }

    nRv = soc_sbx_caladan3_cop_encode_rate(unit, CO_METER_PROFILEm,
					   CIRf, sConfig.uCIR,
					   &uCirExp, &uCirMant);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to convert CIR 0x%x on unit %d\n"),
                   FUNCTION_NAME(), sConfig.uCIR, unit));
	return nRv;
    }

    nRv = soc_sbx_caladan3_cop_encode_rate(unit, CO_METER_PROFILEm,
					   EIRf, sConfig.uEIR,
					   &uEirExp, &uEirMant);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to convert EIR 0x%x on unit %d\n"),
                   FUNCTION_NAME(), sConfig.uEIR, unit));
	return nRv;
    }

    nRv = soc_sbx_caladan3_cop_encode_rate(unit, CO_METER_PROFILEm,
					   CBSf, sConfig.uCBS,
					   &uCbsExp, &uCbsMant);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to convert CBS 0x%x on unit %d\n"),
                   FUNCTION_NAME(), sConfig.uCBS, unit));
	return nRv;
    }

    nRv = soc_sbx_caladan3_cop_encode_rate(unit, CO_METER_PROFILEm,
					   EBSf, sConfig.uEBS,
					   &uEbsExp, &uEbsMant);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to convert EBS 0x%x on unit %d\n"),
                   FUNCTION_NAME(), sConfig.uEBS, unit));
	return nRv;
    }

    /*   Limit burst size based on segment visit_interval, policer rate
     *   and segment max burst size.
     *   scaling_factor = min(12, max(0, 38 - (floor(log2(visit_interval)) + rate.e)))
     *   max_burst_size = segment.max_burst_size >> scaling_factor
     */
    SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_HIr(unit, nCop, segment, &uRegValue));
    nScaleFactor = soc_reg_field_get(unit, CO_SEGMENT_MISC_CONFIG_HIr,
				     uRegValue, VISIT_PERIOD_39_20f);
    if (nScaleFactor) {
	/* soc_sbx_caladan3_msb_bit_pos seems return 0 for both
	 * nScaleFactor == 0 and nScaleFactor == 1.... use it anyway since
	 * we checked nScaleFactor != 0 already.
	 */
	nScaleFactor = soc_sbx_caladan3_msb_bit_pos(nScaleFactor);
	nScaleFactor += 20;
    } else {
	SOC_IF_ERROR_RETURN(READ_CO_SEGMENT_MISC_CONFIG_LOr(unit, nCop, segment, &uRegValue));
	nScaleFactor = soc_reg_field_get(unit, CO_SEGMENT_MISC_CONFIG_LOr,
					 uRegValue, VISIT_PERIOD_19_0f);
	nScaleFactor = soc_sbx_caladan3_msb_bit_pos(nScaleFactor);
    }
    if (!nScaleFactor) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not a valid policer segment on unit %d\n"),
                   FUNCTION_NAME(), nCop, segment, unit));
	return SOC_E_PARAM;
    }

    /* check CBS after scalefactor */
    nCirScale = (38 - nScaleFactor - uCirExp);
    if (nCirScale < 0) {
	nCirScale = 0;
    } else if (nCirScale > SOC_SBX_CALADAN3_COP_POLICER_MAX_SCALEFACTOR) {
	nCirScale = SOC_SBX_CALADAN3_COP_POLICER_MAX_SCALEFACTOR;
    }
    if (sConfig.uCIR == 0) {
	nCirScale = 0;
    }
    if (config->uCBS*8 > (pSegCfg->u.sPolicer.uMaxBurstBits>>nCirScale)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d CBS too high for policer CIR 0x%x kbps"
                              "cop %d segment %d unit %d max burstsize allowed 0x%x bits\n"),
                   FUNCTION_NAME(), policer, config->uCIR, cop, segment, unit,
                   pSegCfg->u.sPolicer.uMaxBurstBits>>nCirScale));
	return SOC_E_PARAM;
    }

    /* check EBS after scalefactor */
    nEirScale = (38 - nScaleFactor - uEirExp);
    if (nEirScale < 0) {
	nEirScale = 0;
    } else if (nEirScale > SOC_SBX_CALADAN3_COP_POLICER_MAX_SCALEFACTOR) {
	nEirScale = SOC_SBX_CALADAN3_COP_POLICER_MAX_SCALEFACTOR;
    }
    if (sConfig.uEIR == 0) {
	nEirScale = 0;
    }

    if (sConfig.uRfcMode == SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2697) {
	if (config->uEBS*8 > (pSegCfg->u.sPolicer.uMaxBurstBits>>nCirScale)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s policer ID %d EBS too high for rfc2697 mode policer CIR 0x%x kbps"
                                  "cop %d segment %d unit %d max burstsize allowed 0x%x bits\n"),
                       FUNCTION_NAME(), policer, config->uCIR, cop, segment, unit,
                       pSegCfg->u.sPolicer.uMaxBurstBits>>nCirScale));
	    return SOC_E_PARAM;
	}
    } else if (config->uEBS*8 > (pSegCfg->u.sPolicer.uMaxBurstBits>>nEirScale)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d EBS too high for policer EIR 0x%x kbps"
                              "cop %d segment %d unit %d max burstsize allowed 0x%x bits\n"),
                   FUNCTION_NAME(), policer, config->uEIR, cop, segment, unit,
                   pSegCfg->u.sPolicer.uMaxBurstBits>>nEirScale));
	return SOC_E_PARAM;
    }

    /*== step 3: allocate profile, associate policer with the profile */
    /* construct the profile, cache CIR/EIR/CBS/EBS/RfcMode since those
     * could not be recovered from hardware
     */
    sal_memset(&sProfile, 0, sizeof(sProfile));
    sProfile.uRfcMode = sConfig.uRfcMode;
    sProfile.uCIR = sConfig.uCIR;
    sProfile.uEIR = sConfig.uEIR;
    sProfile.uCBS = sConfig.uCBS;
    sProfile.uEBS = sConfig.uEBS;
    
    if (sConfig.nLenAdjust >= 0) {
	uLenAdjust = sConfig.nLenAdjust;
    } else {
	uLenAdjust = 256 + sConfig.nLenAdjust;
    }
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			LEN_ADJf, uLenAdjust & 0xFF);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			PKT_MODEf, sConfig.bPktMode?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			BKTE_NODECf, sConfig.bEBSNoDecrement?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			BKTC_NODECf, sConfig.bCBSNoDecrement?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			BKTE_STRICTf, sConfig.bEIRStrict?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			BKTC_STRICTf, sConfig.bCIRStrict?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			RFC2698f, bRfc2698?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			CP_FLAGf, sConfig.bCoupling?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			DROP_ON_REDf, sConfig.bDropOnRed?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			BLINDf, sConfig.bBlindMode?1:0);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			EBS_EXPONENTf, uEbsExp);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			EBS_MANTISSAf, uEbsMant);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			CBS_EXPONENTf, uCbsExp);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			CBS_MANTISSAf, uCbsMant);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			EIR_EXPONENTf, uEirExp);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			EIR_MANTISSAf, uEirMant);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			CIR_EXPONENTf, uCirExp);
    soc_mem_field32_set(unit, CO_METER_PROFILEm, &sProfile.sHwProfile,
			CIR_MANTISSAf, uCirMant);

    /* allocate profile */
    nRv = soc_sbx_caladan3_cop_profile_alloc(unit, cop, &sProfile, &uProfileID);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to allocate profile for policer %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), policer, cop, segment, unit));
	return nRv;    
    } else {
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s policer %d on cop %d segment %d using profile %d on unit %d.\n"),
	             FUNCTION_NAME(), policer, cop, segment, uProfileID, unit));
    }

    /*== step 4: init hardware */
    nRv = soc_sbx_caladan3_cop_policer_state_init(unit, cop, segment, policer, 
						  uProfileID);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create policer %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), policer, cop, segment, unit));
	return nRv;
    }    

    *handle = _COP_HANDLE_SET(cop, segment, policer);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_policer_delete
 *   Purpose
 *      COP delete a policer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : policer handle, returned by policer create
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_policer_delete(int unit,
					uint32 handle)
{
    int nRv = SOC_E_NONE;
    uint32 uCop, uSegment, uId, uOcmAddr, uProfileID;
    co_meter_state_entry_t sPolicer;
    sbx_caladan3_ocm_port_e_t eCopPort;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    /*== step 1: read profile in ocm memory */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    if (uCop == 0) {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP0_PORT;
    } else {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP1_PORT;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create policer on cop %d segment %d unit %d "
                              "which is not an enabled policer segment\n"),
                   FUNCTION_NAME(), uCop, uSegment, unit));
	return SOC_E_PARAM;
    }

    if (uId > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /* try to read OCM memory to find out profile used by the policer */
    uOcmAddr = uId + pSegCfg->nSegmentOcmBase;
    nRv = soc_sbx_caladan3_ocm_port_mem_read(unit, eCopPort, -1, uOcmAddr, uOcmAddr,
					     (uint32 *)&sPolicer);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to read policer state on cop%d id %d unit %d\n"),
                   FUNCTION_NAME(), uCop, uId, unit));
	return nRv;
    }

    uProfileID = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer,
				     PROFILEf);

    if (uProfileID == SOC_SBX_CALADAN3_COP_NULL_PROFILE) {
	if (SAL_BOOT_PLISIM) {
	    /* no real memory read on pcid, so just return here */
	    return SOC_E_NONE;
	} else {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s policer %d on cop %d segment %d unit %d is not active"
                                    " before delete\n"),
                         FUNCTION_NAME(), uId, uCop, uSegment, unit));
            return SOC_E_NONE;
	}
    }

    /*== step 2: point to NULL profile, push to OCM memory */
    nRv = soc_sbx_caladan3_cop_policer_state_init(unit, uCop, uSegment, uId, 
						  SOC_SBX_CALADAN3_COP_NULL_PROFILE);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to delete policer %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit));
	return nRv;
    }

    /*== step 3: update profile database */
    nRv = soc_sbx_caladan3_cop_profile_dealloc(unit, uCop, uProfileID);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_policer_read
 *   Purpose
 *      COP read policer config
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : policer handle, returned by policer create
 *      (OUT) config: policer configuration parameter
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_policer_read(int unit,
				      uint32 handle,
				      soc_sbx_caladan3_cop_policer_config_t *config)
{
    return soc_sbx_caladan3_cop_policer_read_ext(unit, handle, config, NULL);
}

/*
 *   Function
 *     sbx_caladan3_cop_policer_read_ext
 *   Purpose
 *      COP read policer config and it's associated profile
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : policer handle, returned by policer create
 *      (OUT)config : policer configuration parameter pointer
 *      (OUT)profile: policer profile pointer
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_policer_read_ext(int unit,
					  uint32 handle,
					  soc_sbx_caladan3_cop_policer_config_t *config,
					  uint32 *profile)
{
    int nRv = SOC_E_NONE;
    uint32 uOcmAddr, uProfileID, uLenAdjust;
    co_meter_state_entry_t sPolicer;
    sbx_caladan3_ocm_port_e_t eCopPort;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;
    uint32 uCop, uSegment, uId;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    /*== step 1: parameter check */
    if (((int)uCop < 0) || (uCop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), uCop, unit));
	return SOC_E_PARAM;
    }

    if (((int)uSegment < 0) || (uSegment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), uSegment, unit));
	return SOC_E_PARAM;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to read policer on cop %d segment %d unit %d "
                              "which is not an enabled policer segment\n"),
                   FUNCTION_NAME(), uCop, uSegment, unit));
	return SOC_E_PARAM;
    }

    if (uId > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /*== step 2: read profile in ocm memory */
    if (uCop == 0) {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP0_PORT;
    } else {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP1_PORT;
    }

    /* try to read OCM memory to find out profile used by the policer */
    uOcmAddr = uId + pSegCfg->nSegmentOcmBase;
    nRv = soc_sbx_caladan3_ocm_port_mem_read(unit, eCopPort, -1, uOcmAddr, uOcmAddr,
					     (uint32 *)&sPolicer);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to read policer state on cop%d id %d unit %d\n"),
                   FUNCTION_NAME(), uCop, uId, unit));
	return nRv;
    }

    uProfileID = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer,
				     PROFILEf);

    if ((uProfileID == SOC_SBX_CALADAN3_COP_NULL_PROFILE) &&
        !SAL_BOOT_PLISIM) {
	    return SOC_E_NOT_FOUND;
    }   

    /* get the policer config from the profile data */
    if (profile != NULL) {
	*profile = uProfileID;
    }

    if (config != NULL) {
	config->uCIR = pCopCfg->profiles[uCop][uProfileID].uCIR;
	config->uEIR = pCopCfg->profiles[uCop][uProfileID].uEIR;
	config->uCBS = pCopCfg->profiles[uCop][uProfileID].uCBS;
	config->uEBS = pCopCfg->profiles[uCop][uProfileID].uEBS;
	config->uRfcMode = pCopCfg->profiles[uCop][uProfileID].uRfcMode;

	uLenAdjust = soc_mem_field32_get(unit, CO_METER_PROFILEm,
					 &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
					 LEN_ADJf);
	if (uLenAdjust >= 128) {
	    config->nLenAdjust = (uLenAdjust - 256);
	} else {
	    config->nLenAdjust = uLenAdjust;
	    
	}
	config->bPktMode = soc_mem_field32_get(unit, CO_METER_PROFILEm,
					       &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
					       PKT_MODEf);
	config->bEBSNoDecrement = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						      &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						      BKTE_NODECf);
	config->bCBSNoDecrement = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						      &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						      BKTC_NODECf);
	config->bEIRStrict = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						 &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						 BKTE_STRICTf);
	config->bCIRStrict = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						 &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						 BKTC_STRICTf);
	config->bCoupling = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						&pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						CP_FLAGf);
	config->bDropOnRed = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						 &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						 DROP_ON_REDf);
	config->bBlindMode = soc_mem_field32_get(unit, CO_METER_PROFILEm,
						 &pCopCfg->profiles[uCop][uProfileID].sHwProfile,
						 BLINDf);	
    }

    return nRv;
}


/*
 *   Function
 *     sbx_caladan3_cop_timer_create
 *   Purpose
 *      COP create a timer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) timer  : timer ID
 *      (IN) config : timer configuration parameter
 *      (OUT)handle : timer handle, should be used when delete
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_timer_create(int unit,
				      uint32 cop,
				      uint32 segment,
				      uint32 timer,
				      soc_sbx_caladan3_cop_timer_config_t *config,
				      uint32 *handle)
{
    int nRv = SOC_E_NONE;
    uint32 uTimeout = 0;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    /*== step 1: parameter check */
    if (((int)cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if (((int)segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create timer on cop %d segment %d unit %d "
                              "which is not an enabled timer segment\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    if (timer > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s timer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), timer, cop, segment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    if (config->uTimeout < pSegCfg->u.sTimer.nTimerTickUs) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d timer tick is %d us, "
                              "can not support timer %d timeout value %d us on unit %d "),
                   FUNCTION_NAME(), cop, segment, pSegCfg->u.sTimer.nTimerTickUs,
                   timer, config->uTimeout, unit));
	return SOC_E_PARAM;	
    } else {
	uTimeout = config->uTimeout / pSegCfg->u.sTimer.nTimerTickUs;
    }

    /*== step 2: init hardware */
    nRv = soc_sbx_caladan3_cop_timer_state_init(unit, cop, segment, timer, 
						uTimeout, config->bInterrupt?1:0, config->bStart?1:0);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create timer %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), timer, cop, segment, unit));
	return nRv;
    }    

    *handle = _COP_HANDLE_SET(cop, segment, timer);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_timer_delete
 *   Purpose
 *      COP delete a timer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : timer handle, returned by timer create
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_timer_delete(int unit,
				      uint32 handle)
{
    int nRv = SOC_E_NONE;
    uint32 uCop, uSegment, uId;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    /*== step 1: read timer in ocm memory */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create timer on cop %d segment %d unit %d "
                              "which is not an enabled timer segment\n"),
                   FUNCTION_NAME(), uCop, uSegment, unit));
	return SOC_E_PARAM;
    }

    if (uId > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s timer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /*== step 2: init hardware */
    nRv = soc_sbx_caladan3_cop_timer_state_init(unit, uCop, uSegment, uId, 0, 0, 0);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to delete timer %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit));
	return nRv;
    }    

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_timer_read
 *   Purpose
 *      COP read config for a timer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : timer handle, returned by timer create
 *      (OUT)config : timer configuration parameter pointer
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_timer_read(int unit,
				    uint32 handle,
				    soc_sbx_caladan3_cop_timer_config_t *config)
{
    int nRv = SOC_E_NONE;
    uint32 uTimeout, uOcmAddr;
    co_watchdog_timer_state_entry_t sTimer;
    sbx_caladan3_ocm_port_e_t eCopPort;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;
    uint32 uCop, uSegment, uId;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    if (((int)uCop < 0) || (uCop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), uCop, unit));
	return SOC_E_PARAM;
    }

    if (((int)uSegment < 0) || (uSegment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), uSegment, unit));
	return SOC_E_PARAM;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to read timer on cop %d segment %d unit %d "
                              "which is not an enabled timer segment\n"),
                   FUNCTION_NAME(), uCop, uSegment, unit));
	return SOC_E_PARAM;
    }

    if (uId > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s timer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), uId, uCop, uSegment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /*== step 2: read timer state in ocm memory */
    /* 
     * NOTE: the timeout value could be changed by LRP after created
     */
    if (uCop == 0) {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP0_PORT;
    } else {
	eCopPort = SOC_SBX_CALADAN3_OCM_COP1_PORT;
    }
#if 0
    if (pSegCfg->u.sTimer.bMode64 == FALSE) {
	uOcmAddr = uId/2 + pSegCfg->nSegmentOcmBase;	
    } else {
	uOcmAddr = uId + pSegCfg->nSegmentOcmBase;	
    }
#endif
    uOcmAddr = uId + pSegCfg->nSegmentOcmBase;
    nRv = soc_sbx_caladan3_ocm_port_mem_read(unit, eCopPort, -1, uOcmAddr, uOcmAddr,
					     (uint32 *)&sTimer);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to read timer state on cop%d segment %d id %d unit %d\n"),
                   FUNCTION_NAME(), uCop, uSegment, uId, unit));
	return nRv;
    }
    
    if (pSegCfg->u.sTimer.bMode64 == FALSE) {
	/* 32bits mode is one shot mode so doesn't have a timeout value */
	config->uTimeout = 0;

	/* timer 0 is on high32 bits while timer 1 is on low32 bits */
	if (uId % 2) {
	    uTimeout = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm,
					   &sTimer, DATA_63_32f);
	    soc_mem_field32_set(unit, CO_WATCHDOG_TIMER_STATEm, &sTimer, DATA_31_0f,
				uTimeout);
	    
	}
	config->bInterrupt = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm,
						 &sTimer, INTERRUPT_ENf);
    } else {
	config->uTimeout = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm,
					       &sTimer, TIMEOUTf);
	config->bInterrupt = soc_mem_field32_get(unit, CO_WATCHDOG_TIMER_STATEm,
						 &sTimer, INTERRUPT_ENf);
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_seq_checker_create
 *   Purpose
 *      COP create a sequence number checker
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) seq_checker  : sequence checker ID
 *      (IN) init_value   : checker init value
 *      (OUT)handle       : checker handle, should be used when delete
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_seq_checker_create(int unit,
					    uint32 cop,
					    uint32 segment,
					    uint32 seq_checker,
					    uint32 init_value,
					    uint32 *handle)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    /*== step 1: parameter check */
    if (((int)cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if (((int)segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create seq_checker on cop %d segment %d unit %d "
                              "which is not an enabled sequence checker segment\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    if (seq_checker > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s seq_checker ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), seq_checker, cop, segment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    if ((pSegCfg->u.sChecker.bMode32 == FALSE) &&
	(init_value > 0xFFFF)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s seq_checker ID %d init_value more than 16 bits "
                              "on 16bits sequence checker segment cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), seq_checker, cop, segment, unit));
	return SOC_E_PARAM;	
    } 

    /*== step 2: init hardware */
    nRv = soc_sbx_caladan3_cop_sequence_checker_state_init(unit, cop, segment, seq_checker, 
							   init_value);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create seq_checker %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), seq_checker, cop, segment, unit));
	return nRv;
    }    

    if (handle != NULL) {
	*handle = _COP_HANDLE_SET(cop, segment, seq_checker);
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_seq_checker_delete
 *   Purpose
 *      COP delete a sequence number checker
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : checker handle, returned by checker create
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_seq_checker_delete(int unit,
					    uint32 handle)
{
    int nRv = SOC_E_NONE;
    uint32 uCop, uSegment, uId;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    /*== step 2: clear the value */
    nRv = soc_sbx_caladan3_cop_seq_checker_create(unit, uCop, uSegment, uId, 0, NULL);
    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_coherent_table_create
 *   Purpose
 *      COP create a coherent table
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) seq_checker   : coherent table entry ID
 *      (IN) init_bits31_0 : coherent table entry low 32 bits value
 *      (IN) init_bits63_32: coherent table entry high 32 bits value
 *                           unused bits should be 0. for example
 *                           when entry is 4 bits only, init_bits63_32 should
 *                           be 0 and bits31 to 4 of init_bits31_0 should be 0
 *      (OUT)handle        : table entry handle, should be used when delete
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_coherent_table_create(int unit,
					       uint32 cop,
					       uint32 segment,
					       uint32 coherent_table,
					       uint32 init_bits31_0,
					       uint32 init_bits63_32,
					       uint32 *handle)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    /*== step 1: parameter check */
    if (((int)cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    }

    if (((int)segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to create coherent table entry on cop %d segment %d unit %d "
                              "which is not an enabled coherent table segment\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    if (coherent_table > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s coherent_table ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), coherent_table, cop, segment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /*== step 2: init hardware */
    nRv = soc_sbx_caladan3_cop_coherent_table_state_init(unit, cop, segment, coherent_table, 
							 init_bits31_0, init_bits63_32);
    if (SOC_FAILURE(nRv)){
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to create coherent_table %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), coherent_table, cop, segment, unit));
	return nRv;
    }    

    if (handle != NULL) {
	*handle = _COP_HANDLE_SET(cop, segment, coherent_table);    
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_coherent_table_get
 *   Purpose
 *      COP read a coherent table entry
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : coherent table handle
 *      (IN) entry  : cop table entry ID
 *      (OUT) init_bits31_0 : coherent table entry low 32 bits value
 *      (OUT) init_bits63_32: coherent table entry high 32 bits value
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_coherent_table_get(int unit,
                           uint32 handle,
                           uint32 entry,
                           uint32 *bits31_0,
                           uint32 *bits63_32)
{
    int nRv = SOC_E_NONE;
    uint32 uData[3];
    uint32 uCop, uSegment, uId;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);


    /*== step 2: get from hardware */
    uData[0] = 0;
    uData[1] = 0;
    uData[2] = 0;
    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uData[2], TOPf,
            SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_READ);
    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uData[2], OFFSETf, entry);

    nRv = soc_sbx_caladan3_cop_command_inject(unit, uCop, uSegment,
            SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT, uData, uData);
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to get entry %d coherent_table %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), entry, uId, uCop, uSegment, unit));
        return nRv;
    }

    *bits31_0 = uData[0];
    *bits63_32 = uData[1];

    return nRv;
}

/*
 *   Function
 *     soc_sbx_caladan3_cop_coherent_table_set
 *   Purpose
 *      COP set a coherent table entry
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : coherent table handle
 *      (IN) entry  : cop table entry ID
 *      (IN) init_bits31_0 : coherent table entry low 32 bits value
 *      (IN) init_bits63_32: coherent table entry high 32 bits value
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_coherent_table_set(int unit,
                           uint32 handle,
                           uint32 entry,
                           uint32 bits31_0,
                           uint32 bits63_32)
{
    int nRv = SOC_E_NONE;
    uint32 uData[3];
    uint32 uCop, uSegment, uId;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg = NULL;
    co_coherent_table_state_entry_t   sCoherent;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    /* check if segment is enabled */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if (pSegCfg->bEnabled != TRUE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s cop %d segment %d not in use on unit %d\n"),
                   FUNCTION_NAME(), uCop, uSegment, unit));
	return SOC_E_PARAM;	
    }

    /*== step 2: set value to hardware */
    uData[2] = 0;
    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uData[2], TOPf,
              SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_WRITE);
    soc_reg_field_set(unit, CO_INJECT_DATA2r, &uData[2], OFFSETf, entry);
    
    if ((pSegCfg->u.sCoherent.nFormat <= SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_16BIT) &&
    (bits31_0 > ((2<<pSegCfg->u.sCoherent.nFormat)-1))) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s initial coherent table value 0x%x out of range"
                          " 0-0x%x on cop %d segment %d unit %d\n"),
               FUNCTION_NAME(), bits31_0, (2<<pSegCfg->u.sCoherent.nFormat)-1,
               uCop, uSegment, unit));
    return SOC_E_PARAM;
    }
    
    /* table init value = bits31_0 */
    sal_memset(&sCoherent, 0, sizeof(sCoherent));
    switch (pSegCfg->u.sCoherent.nFormat) {
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_1BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_1BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_2BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_4BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_4BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_8BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_8BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_16BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_16BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_32BIT:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_32BITf, bits31_0);
        break;
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_64BIT:
    case SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2x32BIT:
    default:
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_32BITf, bits31_0);
        soc_mem_field32_set(unit, CO_COHERENT_TABLE_STATEm, &sCoherent,
                CT_63_32BITf, bits63_32);
        break;
    }
    
    uData[1] = soc_mem_field32_get(unit, CO_COHERENT_TABLE_STATEm,
                      &sCoherent, DATA_63_32f);
    uData[0] = soc_mem_field32_get(unit, CO_COHERENT_TABLE_STATEm,
                      &sCoherent, DATA_31_0f);

    /* set throught the sbus command injection register interface
     *    no need to get back the command response
     */
    nRv = soc_sbx_caladan3_cop_command_inject(unit, uCop, uSegment,
					      SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT,
					      uData,
					      NULL);
    if (SOC_FAILURE(nRv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to set entry %d coherent_table %d on cop %d segment %d unit %d\n"),
                   FUNCTION_NAME(), entry, uId, uCop, uSegment, unit));
        return nRv;
    }

    return nRv;
}


/*
 *   Function
 *     sbx_caladan3_cop_coherent_table_delete
 *   Purpose
 *      COP delete a coherent table
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : coherent table entry handle, returned by coherent table create
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_coherent_table_delete(int unit,
					       uint32 handle)
{
    int nRv = SOC_E_NONE;
    uint32 uCop, uSegment, uId;

    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);

    /*== step 2: clear the value */
    nRv = soc_sbx_caladan3_cop_coherent_table_create(unit, uCop, uSegment, uId, 0, 0, NULL);

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_recover_policer
 *   Purpose
 *      COP 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_recover_policer(int unit,
					 uint32 policer,
					 uint32 profile)
{
    int nRv = SOC_E_NONE;

    /* warmboot function, TBD */

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_ring_processing_wakeup
 *   Purpose
 *      Wakeup COP ring-processing thread for fifo dma overflow/timeout interrupt 
 *       or manual flush done interrupt
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) d1-4   : not used
 *   Returns
 *      VOID
 */
void soc_sbx_caladan3_cop_ring_processing_wakeup(void *unit_vp,
						 void *d1, void *d2, void *d3, void *d4)
{
    int unit = PTR_TO_INT(unit_vp);
    int nCop = PTR_TO_INT(d1);
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sRing);

    /* wake up ring processing thread */
    if (pRing->trigger[nCop]) {
	sal_sem_give(pRing->trigger[nCop]);
    }
}

/*
 *   Function
 *     sbx_caladan3_cop_ring_process
 *   Purpose
 *      COP timer event fifo read dma ring process
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
static int _soc_sbx_caladan3_cop_ring_process(int unit, int cop)
{
    int nRv = SOC_E_NONE;



    int32 nCount, nLoop, i;
    uint32 uD0, uRegValue = 0;
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sRing);
    soc_sbx_caladan3_cop_timer_queue_t *pQueue = &(SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.sTimerQueue[cop]);
    int32 *pRingPciEnd = pRing->pRingPciBase[cop] + pRing->nRingEntries;
    uint32 uOverflow, uTimeout;
    int    nCmc = CMC1;
    int    nCh;

    if (cop == 0) {
	nCh = SOC_MEM_FIFO_DMA_CHANNEL_0;
    } else {
	nCh = SOC_MEM_FIFO_DMA_CHANNEL_1;
    }

    /* make sure ring is inited */
    if (pRing->pRingPciRead[cop] == NULL) {
	return SOC_E_INIT;
    }

    /* ring process thread (periodic), ring almost full interrupt from CMC and 
     * manual flush done interrupt from COP can all trigger this call.
     * use mutex to protect it.
     */

    COP_RING_LOCK(unit, cop);

    /* loop 3 times at most to prevent tight loop, wait for next wakeup */
    for (nLoop = 3; nLoop > 0; nLoop--) {
	/* get entry count, process if nonzero else break */
	nRv = soc_mem_fifo_dma_get_num_entries(unit, (nCmc*4+nCh), &nCount);
	if (SOC_FAILURE(nRv)) {
	    if (nRv == SOC_E_EMPTY) {
		nRv = SOC_E_NONE;
	    }
	    break;
	}

	/* invalidate cache for these entries */
	if (pRing->pRingPciRead[cop] + nCount > pRingPciEnd) {
	    soc_cm_sinval(unit, pRing->pRingPciRead[cop], (pRingPciEnd - pRing->pRingPciRead[cop])*sizeof(uint32));
	    soc_cm_sinval(unit, pRing->pRingPciBase[cop], (nCount - (pRingPciEnd - pRing->pRingPciRead[cop]))*sizeof(uint32));
	} else {
	    soc_cm_sinval(unit, pRing->pRingPciRead[cop], nCount*sizeof(uint32));
	}
	
	/* process entries */
	for (i = 0; i < nCount; i++) {
	    /* process the entry */
	    uD0 = *(pRing->pRingPciRead[cop]);
	    
#ifdef _SOC_CALADAN3_COP_DEBUG
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
	                (BSL_META_U(unit,
	                            "%s ring message 0x%x at 0x%x on unit %d.\n"),
	                 FUNCTION_NAME(), uD0, (uint32)pRing->pRingPciRead[cop], unit));
#endif /* _SOC_CALADAN3_COP_DEBUG */
	    
	    if (_soc_sbx_caladan3_cop_timer_event_queue_full(pQueue)) {
		LOG_WARN(BSL_LS_SOC_COMMON,
		         (BSL_META_U(unit,
		                     "%s timer queue full on unit %d, drop timer expire event 0x%x\n"),
		          FUNCTION_NAME(), unit, uD0));
		continue;
	    }

	    /* enqueue the message to the timer expire event queue */
	    _soc_sbx_caladan3_cop_timer_event_enqueue(pQueue, cop,
						      (uD0 >> 21) & 0x1F,
						      (uD0 & 0x1fFFFF),
						      (uD0 >> 27) & 0x1,
						      (uD0 >> 26) & 0x1);
						      
	    /* advance the read pointer */
	    pRing->pRingPciRead[cop]++;
	    if (pRing->pRingPciRead[cop] >= pRingPciEnd) {
		pRing->pRingPciRead[cop] = pRing->pRingPciBase[cop];
	    }
	}

	/* set HW number of entries read */
	nRv = soc_mem_fifo_dma_set_entries_read(unit, (nCmc*4+nCh), nCount);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not set entries read for COP fifo dma on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    COP_RING_UNLOCK(unit, cop);
	    return nRv;
	}

#ifdef _SOC_CALADAN3_COP_DEBUG
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s COP %d ring thread processed %d enries on unit %d.\n"),
	             FUNCTION_NAME(), cop, nCount, unit));
#endif /* _SOC_CALADAN3_COP_DEBUG */
    }
    
    /* any errors? */
    uRegValue = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(nCmc, nCh));
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
	soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_CLR_OFFSET(nCmc, nCh), 
		      uRegValue);    
#ifdef _SOC_CALADAN3_COP_DEBUG
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s COP FIFO DMA cleared overflow/timeout errors on unit %d.\n"),
	             FUNCTION_NAME(), unit));
#endif /* _SOC_CALADAN3_COP_DEBUG */
    }

    COP_RING_UNLOCK(unit, cop);    

    return SOC_E_NONE;
}

static void _soc_sbx_caladan3_cop_ring_process_thread(void *unit_vp)
{
    int unit = PTR_TO_INT(unit_vp) & 0xfffFFFF;
    int nCop = PTR_TO_INT(unit_vp) >> 28;
    int uRegValue;
    int nRv = SOC_E_NONE;
    int nCmc, nCh;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(pCopCfg->sRing);
    soc_sbx_caladan3_cop_timer_queue_t *pQueue = &(pCopCfg->sTimerQueue[nCop]);

    nCmc = CMC1;
    if (nCop == 0) {
	nCh = SOC_MEM_FIFO_DMA_CHANNEL_0;
    } else {
	nCh = SOC_MEM_FIFO_DMA_CHANNEL_1;
    }

    /* wake up by fifo read DMA interrupt to process the host ring buffer */
    while (pRing->running[nCop]) {
	/* wait for FIFO dma interrupt to wake up the ring processing */
	(void)sal_sem_take(pRing->trigger[nCop], sal_sem_FOREVER);

#ifdef _SOC_CALADAN3_COP_DEBUG
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
	            (BSL_META_U(unit,
	                        "%s: waked up by interrupts on cop %d unit %d\n"),
	             FUNCTION_NAME(), nCop, unit));
#endif /* _SOC_CALADAN3_COP_DEBUG */

	/* process the ring */
	nRv = _soc_sbx_caladan3_cop_ring_process(unit, nCop);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s failed to process cop %d ring buffers on unit %d\n"),
                       FUNCTION_NAME(), nCop, unit));
	    break;
	}

	/* fatal error handling */
	uRegValue = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(nCmc, nCh));
	if (soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr, uRegValue, DONEf)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FIFO DMA engine terminated for cmc[%d]:ch[%d]\n"), nCmc, nCh));
	    if (soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_STATr, uRegValue, ERRORf)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "FIFO DMA engine encountered error: [0x%x]\n"), uRegValue));
	    }
	    
	    nRv = SOC_E_INTERNAL;
	    break;
	} 

	/* notify the upper layer for further event processing */
	if ((pCopCfg->fNotifyCb != NULL) &&
	    (!_soc_sbx_caladan3_cop_timer_event_queue_empty(pQueue))) {
	    pCopCfg->fNotifyCb(unit, nCop);
	}

	/* fifo dma overflow/timeout interrupt reenable */
	soc_cmicm_cmcx_intr0_enable(unit, nCmc, IRQ_CMCx_FIFO_CH_DMA(nCh));
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: COP %d ring process thread exit with status 0x%x running state %d on unit %d\n"),
                 FUNCTION_NAME(), nCop, nRv, pRing->running[nCop], unit));

    /* stop the COP fifo dma channel
     *  (void)soc_mem_fifo_dma_stop(unit, _SOC_CALADAN3_COP_CMICM_CH);
     * if the thread is stopped, don't stop the channel, rely on interrupts to handle it
     */
    pRing->pid[nCop] = SAL_THREAD_ERROR;
    pRing->running[nCop] = FALSE;

    sal_thread_exit(0); 
}


int soc_sbx_caladan3_cop_ring_process_thread_start(int unit, int cop)
{
    int nPri;
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(pCopCfg->sRing);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on COP %d unit %d.\n"),
                 FUNCTION_NAME(), cop, unit));

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	return nRv;
    }

    /* Create semaphores to trigger */
    if (pRing->trigger[cop] == NULL) {
	pRing->trigger[cop] = sal_sem_create("cop_ring_trigger",
					      sal_sem_BINARY, 0);
	if (pRing->trigger[cop] == NULL) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not create COP %d ring thread trigger semaphore on unit %d\n"),
                       FUNCTION_NAME(), cop, unit));
	    return SOC_E_MEMORY;
	}
    }
    
    /* config buffer ring, COP0 is using CMC1 Channel 0, COP1 is using CMC1 Channel 1 */
    nRv = soc_mem_fifo_dma_start(unit, ((CMC1*4) + cop), CO_WATCHDOG_TIMER_EXPIRED_FIFOm,
				 CO_BLOCK(unit, cop), pCopCfg->sRing.nRingEntries,
				 (void *)pCopCfg->sRing.pRingPciBase[cop]);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s failed to config COP buffer ring on unit %d\n"),
                   FUNCTION_NAME(), unit));
	return nRv;
    }

    if (cop == 0) {
        WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr(unit, pCopCfg->sRing.nRingThresh);
    } else {
        WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_HOSTMEM_THRESHOLDr(unit, pCopCfg->sRing.nRingThresh);
    }

    /* start up the thread */    
    if (pRing->pid[cop] == SAL_THREAD_ERROR) {
	nPri = soc_property_get(unit, spn_LINKSCAN_THREAD_PRI, -1);
	if (nPri < 0) {
	    /* default to 60 */
	    nPri = SOC_SBX_CALADAN3_COP_RING_THREAD_DEFAULT_PRI;
	} else {
	    /* lower priority than linkscan */
	    nPri += 10;
	}
	
	pRing->running[cop] = TRUE;
	pRing->pid[cop] = sal_thread_create(pRing->name[cop], SAL_THREAD_STKSZ, nPri,
					     _soc_sbx_caladan3_cop_ring_process_thread,
					     INT_TO_PTR((unit & 0xfffFFFF)|((cop & 0xF)<<28)));
	
	if (pRing->pid[cop] == SAL_THREAD_ERROR) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Could not start COP %d ring process thread on unit %d\n"),
                       FUNCTION_NAME(), cop, unit));
	    pRing->running[cop] = FALSE;
	    return SOC_E_MEMORY;
	}

    }

    return SOC_E_NONE;
}

int soc_sbx_caladan3_cop_ring_process_thread_stop(int unit, int cop)
{
    int nRv = SOC_E_NONE;
    soc_sbx_caladan3_cop_config_t *pCopCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg;    
    soc_sbx_caladan3_cop_ring_config_t *pRing = &(pCopCfg->sRing);
    sal_thread_t   sSamplePid;
    sal_usecs_t    sTimeout;
    soc_timeout_t  sTo;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on COP %d unit %d.\n"),
                 FUNCTION_NAME(), cop, unit));

    if (pRing->running[cop]) {
	/* wait for 1 second */
	sTimeout = SECOND_USEC;
	
	pRing->running[cop] = FALSE;
	
	sal_sem_give(pRing->trigger[cop]);
	
	soc_timeout_init(&sTo, sTimeout, 0);
	while ((sSamplePid = pRing->pid[cop]) != SAL_THREAD_ERROR) {
	    if (soc_timeout_check(&sTo)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_cop_ring_process_thread_stop:"
                                      "cop %d thread did not exit\n"), cop));
		pRing->pid[cop] = SAL_THREAD_ERROR;
		nRv = SOC_E_INTERNAL;
		break;
	    }
	    
	    sal_usleep(10000);
	}    
    }

    return nRv;
}

/*
 *   Function
 *     sbx_caladan3_cop_sw_dump
 *   Purpose
 *      COP 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
void soc_sbx_caladan3_cop_sw_dump(int unit)
{
    /* seems not much useful, TBD for now */
}

/*
 *   Function
 *     sbx_caladan3_cop_diag_policer_monitor_setup
 *   Purpose
 *      COP setup policer monitor for a policer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) cop    : cop instance 0 or 1
 *      (IN) segment: cop segment 0-31
 *      (IN) policer: policer ID
 *      (OUT)monitor: monitor handle, should be used when free/read monitor
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_diag_policer_monitor_setup(int unit, int cop, int segment,
						    int policer, int *monitor)
{
    int nRv = SOC_E_NONE;
    int nCop, nMonitor, nFreeMonitor, nMatchedMonitor;
    uint32 uRegValue, uIndex, uData[4];
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg;

    if ((cop < 0) || (cop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s COP %d doesn't exist on unit %d\n"),
                   FUNCTION_NAME(), cop, unit));
	return SOC_E_PARAM;
    } else {
	nCop = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(cop);
    }

    if ((segment < 0) || (segment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s segment %d out of range on unit %d\n"),
                   FUNCTION_NAME(), segment, unit));
	return SOC_E_PARAM;
    }

    /* check if segment is enabled */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[cop][segment];
    if ((pSegCfg->bEnabled != TRUE) ||
	(pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s try to read policer on cop %d segment %d unit %d "
                              "which is not an enabled policer segment\n"),
                   FUNCTION_NAME(), cop, segment, unit));
	return SOC_E_PARAM;
    }

    if (policer > pSegCfg->nSegmentLimit) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s policer ID %d out of rang on cop %d segment %d unit %d "
                              "valid range 0-0x%x\n"),
                   FUNCTION_NAME(), policer, cop, segment, unit, pSegCfg->nSegmentLimit));
	return SOC_E_PARAM;
    }

    /* search all policer monitor resource to see if there is already 
     * one being setup. it's for diag only, so don't care about performance, 
     * get everything from hardware
     */
    nFreeMonitor = -1;
    nMatchedMonitor = -1;
    for (nMonitor = 0; nMonitor < SOC_SBX_CALADAN3_COP_POLICER_MAX_MONITOR; nMonitor++) {
	SOC_IF_ERROR_RETURN(READ_CO_METER_MONITOR_CONFIGr(unit, nCop, nMonitor, &uRegValue));

	/* find the first free monitor */
	if (soc_reg_field_get(unit, CO_METER_MONITOR_CONFIGr, uRegValue, ENABLEf) == 0) {
	    if (nFreeMonitor < 0) {
		nFreeMonitor = nMonitor;
	    }
	} else {
	    /* find the enabled monitor matches the segment/entry config */
	    if ((soc_reg_field_get(unit, CO_METER_MONITOR_CONFIGr, uRegValue, SEGMENTf) == segment) &&
		(soc_reg_field_get(unit, CO_METER_MONITOR_CONFIGr, uRegValue, OFFSETf) == policer)) {
		if (nMatchedMonitor < 0) {
		    nMatchedMonitor = nMonitor;
		}
	    }
	}
    }

    /* setup a monitor */
    if (nMatchedMonitor < 0) {
	if (nFreeMonitor < 0) {
	    /* no resource */
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s no policer monitor available on cop %d unit %d\n"),
                       FUNCTION_NAME(), cop, unit));
	    return SOC_E_FULL;
	} else {
	    /* setup a new monitor */

	    /* clear the counter memory for the new monitor */
	    uData[0] = uData[1] = uData[2] = uData[3] = 0;
	    for (uIndex=0; uIndex < SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS; uIndex++) {
		soc_mem_field_set(unit, CO_METER_MONITOR_COUNTERm, &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer[uIndex].entry_data[0],
				  USER_DATAf, uData);		
	    }
	    nRv = soc_mem_write_range(unit, CO_METER_MONITOR_COUNTERm, CO_BLOCK(unit, cop),
				      nFreeMonitor * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS,
				      (nFreeMonitor+1) * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS - 1,
				      (void *)SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer);
	    if (SOC_FAILURE(nRv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to DMA clear policer monitor %d counters \n"),
                           FUNCTION_NAME(), unit, nFreeMonitor));
		return nRv;
	    }

	    /* config and enable the monitor. NOTE: have to clear counters before enable */
	    SOC_IF_ERROR_RETURN(READ_CO_METER_MONITOR_CONFIGr(unit, nCop, nFreeMonitor, &uRegValue));
	    soc_reg_field_set(unit, CO_METER_MONITOR_CONFIGr, &uRegValue, ENABLEf, 1);
	    soc_reg_field_set(unit, CO_METER_MONITOR_CONFIGr, &uRegValue, SEGMENTf, segment);
	    soc_reg_field_set(unit, CO_METER_MONITOR_CONFIGr, &uRegValue, OFFSETf, policer);
	    SOC_IF_ERROR_RETURN(WRITE_CO_METER_MONITOR_CONFIGr(unit, nCop, nFreeMonitor, uRegValue));
	    
	    /* return the monitor handle */
	    *monitor = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_SET(cop, nFreeMonitor);
	}
    } else {
	/* found a match, return it */
	*monitor = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_SET(cop, nFreeMonitor);
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cop_diag_policer_monitor_free
 *   Purpose
 *      COP free policer monitor for a policer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) monitor: monitor handle, returned by the monitor_setup API
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_cop_diag_policer_monitor_free(int unit, int monitor)
{
    int nRv = SOC_E_NONE;
    int nCop, nMonitor, nCopInstance;
    uint32 uRegValue, uIndex, uData[4];
  
    nCop = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_COP(monitor);
    nMonitor = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_MONITOR(monitor);

    if ((nCop >= SOC_SBX_CALADAN3_COP_NUM_COP) ||
	(nMonitor >= SOC_SBX_CALADAN3_COP_POLICER_MAX_MONITOR)) {
	/* invalid */
	/* coverity[dead_error_line] */
	return SOC_E_PARAM;
    }

    /* disable monitor */
    nCopInstance = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(nCop);
    uRegValue = 0;
    SOC_IF_ERROR_RETURN(WRITE_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, uRegValue));    

    /* clear the monitor counter memory */
    uData[0] = uData[1] = uData[2] = uData[3] = 0;
    for (uIndex=0; uIndex < SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS; uIndex++) {
	soc_mem_field_set(unit, CO_METER_MONITOR_COUNTERm, &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer[uIndex].entry_data[0],
			  USER_DATAf, uData);		
    }
    nRv = soc_mem_write_range(unit, CO_METER_MONITOR_COUNTERm, CO_BLOCK(unit, nCop),
			      nMonitor * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS,
			      (nMonitor+1) * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS - 1,
			      (void *)SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to DMA clear policer monitor %d counters \n"),
                   FUNCTION_NAME(), unit, nMonitor));
	return nRv;
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_cop_diag_policer_monitor_read
 *   Purpose
 *      COP read policer monitor counter
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) monitor: monitor handle, returned by the monitor_setup API
 *      (IN) clear_on_read: clear counter after read
 *      (OUT)counter: counters associated with the monitor
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       SOC_E_* as appropriate otherwise
 *   NOTE:
 *       clear counter while traffic is running might lead to miss counting of some packets/bytes
 */
int soc_sbx_caladan3_cop_diag_policer_monitor_read(int unit, int monitor, int clear_on_read,
						   soc_sbx_caladan3_cop_policer_monitor_counter_t *counter)
{
    int nRv = SOC_E_NONE;
    int nCop, nMonitor, nCopInstance;
    uint32 uRegValue, uIndex, uData[4];
    uint64 *pData = NULL;

    nCop = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_COP(monitor);
    nMonitor = SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_MONITOR(monitor);

    if ((nCop >= SOC_SBX_CALADAN3_COP_NUM_COP) ||
	(nMonitor >= SOC_SBX_CALADAN3_COP_POLICER_MAX_MONITOR)) {
	/* invalid */
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d policer monitor %d on COP %d doesn't exist\n"),
                   FUNCTION_NAME(), unit, nMonitor, nCop));
	return SOC_E_PARAM;
    }

    /* make monitor is enabled */
    nCopInstance = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(nCop);
    uRegValue = 0;
    SOC_IF_ERROR_RETURN(READ_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, &uRegValue));
    if (soc_reg_field_get(unit, CO_METER_MONITOR_CONFIGr, uRegValue, ENABLEf) ==0) {
	/* not enabled */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d policer monitor %d not enabled on COP %d\n"),
                   FUNCTION_NAME(), unit, nMonitor, nCop));
	return SOC_E_PARAM;
    }

    /* read the monitor counters */
    nRv = soc_mem_read_range(unit, CO_METER_MONITOR_COUNTERm, CO_BLOCK(unit, nCop),
			     nMonitor * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS,
			     (nMonitor+1) * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS - 1,
			     (void *)SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer);
    if (SOC_FAILURE(nRv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to DMA read policer monitor %d counters \n"),
                   FUNCTION_NAME(), unit, nMonitor));
	return nRv;
    }

    pData = (uint64 *)counter;
    for (uIndex=0; uIndex < SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS; uIndex++) {
	soc_mem_field64_get(unit, CO_METER_MONITOR_COUNTERm, &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer[uIndex].entry_data[0],
			  PACKET_COUNTf, pData);
	pData += 1;
	soc_mem_field64_get(unit, CO_METER_MONITOR_COUNTERm, &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer[uIndex].entry_data[0],
			  BYTE_COUNTf, pData);
	pData += 1;
    }    

    /* clear the monitor counter memory */
    if (clear_on_read) {
	/* disable monitor first, not safe to clear counters while monitor enabled */
	SOC_IF_ERROR_RETURN(READ_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, &uRegValue));
	soc_reg_field_set(unit, CO_METER_MONITOR_CONFIGr, &uRegValue, ENABLEf, 0);
	SOC_IF_ERROR_RETURN(WRITE_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, uRegValue));	
	
	/* clear counters */
	uData[0] = uData[1] = uData[2] = uData[3] = 0;
	for (uIndex=0; uIndex < SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS; uIndex++) {
	    soc_mem_field_set(unit, CO_METER_MONITOR_COUNTERm, &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer[uIndex].entry_data[0],
			      USER_DATAf, uData);		
	}
	nRv = soc_mem_write_range(unit, CO_METER_MONITOR_COUNTERm, CO_BLOCK(unit, nCop),
				  nMonitor * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS,
				  (nMonitor+1) * SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS - 1,
				  (void *)SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.uMonitorDmaBuffer);
	if (SOC_FAILURE(nRv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to DMA clear policer monitor %d counters \n"),
                       FUNCTION_NAME(), unit, nMonitor));
	    return nRv;
	}

	/* reenable monitor. All packets between disable and reenable of monitor are not counted */
	SOC_IF_ERROR_RETURN(READ_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, &uRegValue));
	soc_reg_field_set(unit, CO_METER_MONITOR_CONFIGr, &uRegValue, ENABLEf, 1);
	SOC_IF_ERROR_RETURN(WRITE_CO_METER_MONITOR_CONFIGr(unit, nCopInstance, nMonitor, uRegValue));	
    }

    return SOC_E_NONE;
}

#endif /* BCM_CALADAN3_SUPPORT */


/*
 * Function:
 *    soc_sbx_caladan3_cop_policer_pkt_mode_len_get
 * Purpose:
 *    Get COP model global packet mode length
 */
int
soc_sbx_caladan3_cop_policer_pkt_mode_len_get(int unit, int cop, uint32* len)
{
    uint32 regval = 0;

    SOC_IF_ERROR_RETURN(READ_CO_GLOBAL_CONFIGr(unit, cop, &regval));

    *len = soc_reg_field_get(unit, CO_GLOBAL_CONFIGr, regval, PKT_MODE_LENf);

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_cop_policer_pkt_mode_len_set
 * Purpose:
 *    Set COP model global packet mode length
 */
int
soc_sbx_caladan3_cop_policer_pkt_mode_len_set(int unit, int cop, uint32 len)
{
    uint32 regval = 0;

    SOC_IF_ERROR_RETURN(READ_CO_GLOBAL_CONFIGr(unit, cop, &regval));
    soc_reg_field_set(unit, CO_GLOBAL_CONFIGr, &regval, PKT_MODE_LENf, len);
    SOC_IF_ERROR_RETURN(WRITE_CO_GLOBAL_CONFIGr(unit, cop, regval));

    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_cop_policer_token_number_get
 *   Purpose
 *      COP read the token number of bucket C and E
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : policer handle, returned by policer create
 *      (OUT) *token_c : token number of bucket C
 *      (OUT) *token_e : token number of bucket E
 *   Returns
 *       SOC_E_NONE - successfully read
 *       SOC_E_* as appropriate otherwise
 */
int
soc_sbx_caladan3_cop_policer_token_number_get(int unit,
				      uint32 handle,
				      uint32 *token_c,
				      uint32 *token_e)
{
    int nRv = SOC_E_NONE;
    uint32 uOcmAddr = 0;
    co_meter_state_entry_t sPolicer;
    sbx_caladan3_ocm_port_e_t eCopPort;
    soc_sbx_caladan3_cop_segment_config_t *pSegCfg = NULL;
    uint32 uCop = 0;
    uint32 uSegment = 0;
    uint32 uId = 0;	
    
    /*== step 1: convert ID */
    uCop = _COP_HANDLE_GET_COP(handle);
    uSegment = _COP_HANDLE_GET_SEGMENT(handle);
    uId = _COP_HANDLE_GET_ID(handle);
    
    /*== step 1: parameter check */
    if (((int)uCop < 0) || (uCop >= SOC_SBX_CALADAN3_COP_NUM_COP)) {
    /* coverity[dead_error_begin] */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s COP %d doesn't exist on unit %d\n"),
               FUNCTION_NAME(), uCop, unit));
    return SOC_E_PARAM;
    }
    
    if (((int)uSegment < 0) || (uSegment > SOC_SBX_CALADAN3_COP_NUM_SEGMENT)) {
    /* coverity[dead_error_begin] */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s segment %d out of range on unit %d\n"),
               FUNCTION_NAME(), uSegment, unit));
    return SOC_E_PARAM;
    }
    
    /* make sure ID is within segment limit */
    pSegCfg = &SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.segments[uCop][uSegment];
    if ((pSegCfg->bEnabled != TRUE) ||
    (pSegCfg->nSegmentType != SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s try to read policer on cop %d segment %d unit %d "
                          "which is not an enabled policer segment\n"),
               FUNCTION_NAME(), uCop, uSegment, unit));
    return SOC_E_PARAM;
    }
    
    if (uId > pSegCfg->nSegmentLimit) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s policer ID %d out of rang on cop %d segment %d unit %d "
                          "valid range 0-0x%x\n"),
               FUNCTION_NAME(), uId, uCop, uSegment, unit, pSegCfg->nSegmentLimit));
    return SOC_E_PARAM;
    }
    
    /*== step 2: read profile in ocm memory */
    if (uCop == 0) {
    eCopPort = SOC_SBX_CALADAN3_OCM_COP0_PORT;
    } else {
    eCopPort = SOC_SBX_CALADAN3_OCM_COP1_PORT;
    }
    
    /* try to read OCM memory to find out profile used by the policer */
    uOcmAddr = uId + pSegCfg->nSegmentOcmBase;
    nRv = soc_sbx_caladan3_ocm_port_mem_read(unit, eCopPort, -1, uOcmAddr, uOcmAddr,
    				     (uint32 *)&sPolicer);
    if (SOC_FAILURE(nRv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s failed to read policer state on cop%d id %d unit %d\n"),
               FUNCTION_NAME(), uCop, uId, unit));
    return nRv;
    }
    
    *token_c = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer, BKT_Cf);	
    *token_e = soc_mem_field32_get(unit, CO_METER_STATEm, &sPolicer, BKT_Ef);
    
    return nRv;
}

