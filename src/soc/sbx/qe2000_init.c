/*
 * $Id: qe2000_init.c,v 1.74 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ============================================================
 * == qe2000_init.c - QE Reset and Initialization            ==
 * ============================================================
 */

#define INTERNAL_MEMBIST_TIMEOUT     (4) /* 400ms */

#include <shared/bsl.h>
#include <assert.h>
#include <soc/cm.h>
#include <soc/debug.h>
#include <soc/sbx/glue.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/qe2000_util.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sbx_util.h>
#include <soc/sbx/fabric/sbZfHwQe2000QsPriLutAddr.hx>

#include <soc/sbx/fabric/sbZfHwQe2000QsPriLutEntry.hx>

#undef MAXPRI /* 22314: for netBSD kernel build of this file MAXPRI is already defined as 127 */

#define QE2000_ERROR      (1)

/* #define QE2000DEBUG */

#ifdef QE2000DEBUG
#define QE2000_DEBUG_API  (2)
#define QE2000_DEBUG_FUNC (4)
#else
#define QE2000_DEBUG_API  (0)
#define QE2000_DEBUG_FUNC (0)
#endif

uint32 qe2000Debug = QE2000_DEBUG_API | QE2000_DEBUG_FUNC | QE2000_ERROR;

#define QE2000PRINTF(flag, fmt) if (qe2000Debug & (flag)) bsl_printf fmt

static void hwQe2000InReset(sbhandle userDeviceHandle);
static void hwQe2000UnReset(sbhandle userDeviceHandle);

static sbBool_t
hwQe2000InitDone(sbhandle userDeviceHandle,
                 uint32     ctlreg,
                 uint32     init,
                 uint32     done,
                 uint32     timeout_secs);

static sbBool_t
hwQe2000RegDone(sbhandle userDeviceHandle,
               uint32 ctlreg,
               uint32 done,
               uint32 timeout_in100msecs);

static uint32 hwQe2000InitPm(sbhandle userDeviceHandle,
                               sbBool_t bQm512MbDdr2,
                               sbBool_t bPmHalfBus,
                               sbBool_t bDmode,
                               sbBool_t bFabCrc32,
                               sbBool_t bPmRunSelfTest,
                               uint32 uClockSpeedInMHz,
                               sbBool_t bStandAlone,
                               sbBool_t bHybrid,
                               sbBool_t bPmDdrTrain,
                               int32  nLinesPerTimeslot,
                               int32  nLinesPerTimeslotCongested,
                               int32  nLinesPerTimeslotQe1k,
                               sbBool_t bRunLongDdrMemoryTest,
                               int32 uPacketAdjustFormat);

static uint32 hwQe2000_RunMode1PmDdrTest(sbhandle userDeviceHandle,
                                           uint32 uStartAddress,
                                           uint32 uEndAddress);

static uint32 hwQe2000_RunMode2PmDdrTest(sbhandle userDeviceHandle,
                                           uint32 uStartAddress,
                                           uint32 uEndAddress);

static uint32 hwQe2000InitSetUpDrDllTrim(sbhandle userDeviceHandle,
                                           uint32 nDr,
                                           uint32 nReg,
                                           uint32 nDlyTrim);

static uint32 hwQe2000InitPmMemWrite(sbhandle userDeviceHandle,
                                    uint32 nAddress,
                                    uint32 nData3,
                                    uint32 nData2,
                                    uint32 nData1,
                                    uint32 nData0);

static uint32 hwQe2000PmMemRead(sbhandle userDeviceHandle,
                                   uint32 nAddress,
                                   uint32* pData3,
                                   uint32* pData2,
                                   uint32* pData1,
                                   uint32* pData0);

static uint32 hwQe2000TrainingPmDdr(sbhandle userDeviceHandle,
                                      sbBool_t bPmHalfBus);

static uint32
hwQe2000InitQs(sbhandle userDeviceHandle,
               sbBool_t bDmode,
               uint32 uQsMaxNodes,
               int32  nQueuesPerShaperIngress,
               uint32 uEpochSizeInNs,
               uint32 uClockSpeedInMHz,
               uint32 uNodeId,
               sbBool_t bStandAlone,
               sbBool_t bHybrid,
               sbBool_t bMixHighAndLowRateFlows,
               int32  spMode);

static uint32
hwQe2000InitQsReconfigurePriLutSetup(sbhandle userDeviceHandle);

static uint32
hwQe2000InitQsReconfigurePriLutVportMixSetup(sbhandle userDeviceHandle);


#if 01 /* AB501 090109  */

uint32
hwQe2000InitQsReconfigurePriLut(sbhandle userDeviceHandle,
                                sbBool_t bMultiQueuePerShaper,
                                sbBool_t bMixHighAndLowRateFlows);
uint32
hwQe2000InitQsReconfigurePriLutEx(sbhandle userDeviceHandle,
				  sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots);
uint32
hwQe2000InitQsReconfigurePriLutEx2(sbhandle userDeviceHandle,
				  sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots);

uint32
hwQe2000InitQsReconfigurePriLutEx3(sbhandle userDeviceHandle,
				  sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots);
#endif


static uint32
hwQe2000InitRb(sbhandle userDeviceHandle,
               uint32 uRbSpi0PortCount,
               sbBool_t bFabCrc32,
               uint32 uPacketAdjustFormat);

static uint32
hwQe2000InitQm(sbhandle userDeviceHandle,
               sbBool_t bPmHalfBus,
               sbBool_t bQm512MbDdr2,
               uint32 uEpochSizeInNs,
               uint32 uClockSpeedInMHz,
               sbBool_t bDmode,
               uint32 uQmMaxArrivalRateMbs,
               sbBool_t bStandAlone,
               sbBool_t bHybrid,
               uint32 hybridModeQueueDemarcation,
               int32 nGlobalShapingAdjustInBytes,
               int32 nLinesPerTimeslotCongested,
               int32 nLinesPerTimeslotQe1k,
	       int32 nDemandScale);

static uint32
hwQe2000InitSfi(sbhandle userDeviceHandle,
               uint32 uSfiDataLinkInitMask,
               uint32 uNodeId,
               uint32 uClockSpeedInMHz,
               uint32 uSfiXconfig[ HW_QE2000_NUM_SFI_LINKS][HW_QE2000_NUM_XCFG_REMAP_ENTRIES],
               uint32 uSiLsThreshold,
	       uint32 uSiLsWindow,
	       uint32 uSfiHeaderPad);

static uint32
hwQe2000InitSci(sbhandle userDeviceHandle,
                uint32 uSciLinkEnRemap[HW_QE2000_NUM_SCI_LINKS][ HW_QE2000_NUM_SFI_LINKS],
                uint32 uSciLinkStatusRemap[HW_QE2000_NUM_SCI_LINKS][ HW_QE2000_NUM_SFI_LINKS],
                uint32 uClockSpeedInMHz,
                sbBool_t bStandAlone,
                uint32 uStandAloneEpochSizeInNs,
                uint32 uSiLsThreshold,
                uint32 uSiLsWindow,
                uint32 uSciDefaultBmId,
                uint32 uScTxdmaSotDelayInClocks,
		        uint32 uScGrantoffset,
		        uint32 uScGrantDelay,
                sbBool_t bDmode);

static void hwQe2000SetEgMcSrcId(sbhandle userDeviceHandle,
                                 uint32 uPort,
                                 uint32 uSrcId);

static uint32
hwQe2000InitTx(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               uint32 uQe1KLinkEnMask,
               uint32 uSfiTimeslotOffsetInClocks);

static uint32
hwQe2000InitSv(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               uint32 uNodeId,
               uint32 uNumNodes,
               uint32 uClockSpeedInMHz,
               sbBool_t bSv2_5GbpsLinks,
               uint32 uQe1KLinkEnMask);

static uint32
hwQe2000InitEg(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               sbBool_t bStandAlone,
               sbBool_t bHybrid,
               uint32 uEgMVTSize,
               uint32 uNumPhySpiPorts[2],
               uint32 uSpiSubportSpeed[2][49],
               uint32 uClockSpeedInMHz,
               uint32 uNodeId,
               uint32 uEgMcDropOnFull,
               HW_QE2000_INIT_ST *pQe2000Params);

static uint32
hwQe2000InitEb(sbhandle userDeviceHandle);

static uint32
hwQe2000InitEi(sbhandle userDeviceHandle,
               uint32 uNumPhySpiPorts[2],
	       uint32 uSpiSubportSpeed[2][49],
               uint32 uEiPortInactiveTimeout,
               uint32 uClockSpeedInMHz,
               uint32 bEiSpiFullPacketMode[2],
               HW_QE2000_INIT_ST *pQe2000Params);

static uint32
hwQe2000InitEp(sbhandle userDeviceHandle, 
	       sbBool_t bEpDisable,
               uint32 uNumPhySpiPorts[2],
	       uint64   uuRequeuePortsMask[2],
               uint32 uPacketAdjustFormat);

static uint32
hwQe2000_PmManInitCmd(sbhandle userDeviceHandle, uint32 nCmd, uint32 nManInit);

static void hwQe2000_BringUpPmManually(sbhandle userDeviceHandle);

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000Init()
 *
 * OVERVIEW:        Initialize the QE2000
 *
 * ARGUMENTS:       hwQe2000Init_sp
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Reset the QE, initialize QE2000 internal blocks
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32
hwQe2000Init(HW_QE2000_INIT_ST *hwQe2000Init_sp)
{
    uint32 sts_ul;
    sbhandle userDeviceHandle = hwQe2000Init_sp->userDeviceHandle;
    int rv;
#ifdef BCM_EASY_RELOAD_SUPPORT
    uint32 maxage[SBX_MAX_FABRIC_COS];
    uint32 minuse[SBX_MAX_FABRIC_COS];
    uint32 bytes_per_timeslot;
    unsigned int index;
    uint32 regval;
#endif

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("userDeviceHandle 0x%x\n", (uint32)hwQe2000Init_sp->userDeviceHandle));

    hwQe2000InReset(userDeviceHandle);
    hwQe2000UnReset(userDeviceHandle);

    QE2000PRINTF(QE2000_DEBUG_FUNC, ("uEpochSizeInNs = %d\n", (uint32)hwQe2000Init_sp->uEpochSizeInNs));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("StandAlone = %d bPmHalfBus = %d bQm512MbDdr2 = %d\n",
                                     hwQe2000Init_sp->bStandAlone, hwQe2000Init_sp->bPmHalfBus,hwQe2000Init_sp->bQm512MbDdr2));

    rv = soc_sbx_connect_init((int32)userDeviceHandle, SB_FAB_DEVICE_QE2000_CONN_UTIL_MAX_TEMPLATES,
                                                 SB_FAB_DEVICE_QE2000_CONN_AGE_MAX_TEMPLATES);
    if (rv != SOC_E_NONE) {
        return(rv);
    }

#ifdef BCM_WARM_BOOT_SUPPORT /* AB501 021510  */
    if (SOC_WARM_BOOT((int)userDeviceHandle)){
      return SOC_E_NONE;
    }
#endif
#ifdef BCM_EASY_RELOAD_SUPPORT
    if (SOC_IS_RELOADING((int)userDeviceHandle)) {
        /* get important configuration parameters */
        regval = SAND_HAL_READ((int)userDeviceHandle, KA, QM_CONFIG2);
        bytes_per_timeslot = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, regval) * 16;
        /* read the anemic utilisation template values. */
        for (index = 0;
             index < SB_FAB_DEVICE_QE2000_CONN_UTIL_MAX_TEMPLATES;
             index++) {
            
            switch (index) {
            case 0:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK1,
                                            ANEMIC_WATERMARK0,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK1));
                break;
            case 1:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK1,
                                            ANEMIC_WATERMARK1,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK1));
                break;
            case 2:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK1,
                                            ANEMIC_WATERMARK2,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK1));
                break;
            case 3:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK1,
                                            ANEMIC_WATERMARK3,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK1));
                break;
            case 4:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK0,
                                            ANEMIC_WATERMARK4,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK0));
                break;
            case 5:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK0,
                                            ANEMIC_WATERMARK5,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK0));
                break;
            case 6:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK0,
                                            ANEMIC_WATERMARK6,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK0));
                break;
            case 7:
                regval = SAND_HAL_GET_FIELD(KA,
                                            QM_ANEMIC_WATERMARK0,
                                            ANEMIC_WATERMARK7,
                                            SAND_HAL_READ((int)userDeviceHandle,
                                                          KA,
                                                          QM_ANEMIC_WATERMARK0));
                break;
            }
            minuse[index] = regval;
        }
        /* read the max age template values */
        for (index = 0;
             index < SB_FAB_DEVICE_QE2000_CONN_AGE_MAX_TEMPLATES;
             index++) {
            rv = soc_qe2000_qs_mem_read_easy_reload((int)userDeviceHandle,
                                                    0xB,
                                                    index,
                                                    &regval);
            if (SOC_E_NONE != rv) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: unable to read max_age\n"));
                return HW_QE2000_STS_FAILURE_K;
            }
            maxage[index] = (regval >> 8) & 0xFF;
        }
        /* commit the template values to the cache */
        rv = soc_sbx_connect_reload((int)userDeviceHandle,
                                    &(minuse[0]),
                                    &(maxage[0]));
        if (SOC_E_NONE != rv) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: unable to sync template cache\n"));
            return HW_QE2000_STS_FAILURE_K;
        }
    }
#endif

    sts_ul=hwQe2000InitPm(userDeviceHandle,
                          hwQe2000Init_sp->bQm512MbDdr2,
                          hwQe2000Init_sp->bPmHalfBus,
                          hwQe2000Init_sp->bDmode,
                          hwQe2000Init_sp->bFabCrc32,
                          hwQe2000Init_sp->bPmRunSelfTest,
                          hwQe2000Init_sp->uClockSpeedInMHz,
                          hwQe2000Init_sp->bStandAlone,
                          hwQe2000Init_sp->bHybrid,
                          hwQe2000Init_sp->bPmDdrTrain,
                          hwQe2000Init_sp->nLinesPerTimeslot,
                          hwQe2000Init_sp->nLinesPerTimeslotCongested,
                          hwQe2000Init_sp->nLinesPerTimeslotQe1k,
                          hwQe2000Init_sp->bRunLongDdrMemoryTest,
                          hwQe2000Init_sp->uPacketAdjustFormat);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitQs(userDeviceHandle,
                          hwQe2000Init_sp->bDmode,
                          hwQe2000Init_sp->uQsMaxNodes,
                          hwQe2000Init_sp->nQueuesPerShaperIngress,
                          hwQe2000Init_sp->uEpochSizeInNs,
                          hwQe2000Init_sp->uClockSpeedInMHz,
                          hwQe2000Init_sp->uNodeId,
                          hwQe2000Init_sp->bStandAlone,
                          hwQe2000Init_sp->bHybrid,
                          hwQe2000Init_sp->bMixHighAndLowRateFlows,
                          hwQe2000Init_sp->spMode);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitRb(userDeviceHandle,
                          hwQe2000Init_sp->uRbSpi0PortCount,
                          hwQe2000Init_sp->bFabCrc32,
                          hwQe2000Init_sp->uPacketAdjustFormat);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitQm(userDeviceHandle,
                          hwQe2000Init_sp->bPmHalfBus,
                          hwQe2000Init_sp->bQm512MbDdr2,
                          hwQe2000Init_sp->uEpochSizeInNs,
                          hwQe2000Init_sp->uClockSpeedInMHz,
                          hwQe2000Init_sp->bDmode,
                          hwQe2000Init_sp->uQmMaxArrivalRateMbs,
                          hwQe2000Init_sp->bStandAlone,
                          hwQe2000Init_sp->bHybrid,
                          hwQe2000Init_sp->hybridModeQueueDemarcation,
                          hwQe2000Init_sp->nGlobalShapingAdjustInBytes,
                          hwQe2000Init_sp->nLinesPerTimeslotCongested,
                          hwQe2000Init_sp->nLinesPerTimeslotQe1k,
                          hwQe2000Init_sp->nDemandScale);


    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }


    if (!hwQe2000Init_sp->bStandAlone)
      sts_ul=hwQe2000InitSv(userDeviceHandle,
                            hwQe2000Init_sp->bFabCrc32,
                            hwQe2000Init_sp->uNodeId,
                            hwQe2000Init_sp->uQsMaxNodes,
                            hwQe2000Init_sp->uClockSpeedInMHz,
                            hwQe2000Init_sp->bSv2_5GbpsLinks,
                            hwQe2000Init_sp->uQe1KLinkEnMask);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
      return sts_ul;
    }

    /* must init EB before EG, so that we can write to */
    /* MVT during EG bringup */
    sts_ul=hwQe2000InitEb(userDeviceHandle);
    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    /* do not reorder this call, must call InitEb prior to this */
    sts_ul=hwQe2000InitEg(userDeviceHandle,
                          hwQe2000Init_sp->bFabCrc32,
                          hwQe2000Init_sp->bStandAlone,
                          hwQe2000Init_sp->bHybrid,
                          hwQe2000Init_sp->uEgMVTSize,
                          hwQe2000Init_sp->uNumPhySpiPorts,
			  hwQe2000Init_sp->uSpiSubportSpeed,
                          hwQe2000Init_sp->uClockSpeedInMHz,
                          hwQe2000Init_sp->uNodeId,
                          hwQe2000Init_sp->uEgMcDropOnFull,
                          hwQe2000Init_sp);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitEp(userDeviceHandle, 
			  hwQe2000Init_sp->bEpDisable,
			  hwQe2000Init_sp->uNumPhySpiPorts,
			  hwQe2000Init_sp->uuRequeuePortsMask,
                          hwQe2000Init_sp->uPacketAdjustFormat);
    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitEi(userDeviceHandle,
                          hwQe2000Init_sp->uNumPhySpiPorts,
			  hwQe2000Init_sp->uSpiSubportSpeed,
                          hwQe2000Init_sp->uEiPortInactiveTimeout,
                          hwQe2000Init_sp->uClockSpeedInMHz,
                          hwQe2000Init_sp->bEiSpiFullPacketMode,
                          hwQe2000Init_sp);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    sts_ul=hwQe2000InitTx(userDeviceHandle,
                          hwQe2000Init_sp->bFabCrc32,
                          hwQe2000Init_sp->uQe1KLinkEnMask,
                          hwQe2000Init_sp->uSfiTimeslotOffsetInClocks);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    if (!hwQe2000Init_sp->bStandAlone) {

        sts_ul=hwQe2000InitSfi(userDeviceHandle,
                               hwQe2000Init_sp->uSfiDataLinkInitMask,
                               hwQe2000Init_sp->uNodeId,
                               hwQe2000Init_sp->uClockSpeedInMHz,
                               hwQe2000Init_sp->uSfiXconfig,
                               hwQe2000Init_sp->uSiLsThreshold,
                               hwQe2000Init_sp->uSiLsWindow,
			       hwQe2000Init_sp->uSfiHeaderPad);

        if (sts_ul!= HW_QE2000_STS_OK_K) {
            return sts_ul;
        }
    }

    sts_ul=hwQe2000InitSci(userDeviceHandle,
                           hwQe2000Init_sp->uSciLinkEnRemap,
                           hwQe2000Init_sp->uSciLinkStatusRemap,
                           hwQe2000Init_sp->uClockSpeedInMHz,
                           hwQe2000Init_sp->bStandAlone,
                           hwQe2000Init_sp->uEpochSizeInNs,
                           hwQe2000Init_sp->uSiLsThreshold,
                           hwQe2000Init_sp->uSiLsWindow,
                           hwQe2000Init_sp->uSciDefaultBmId,
                           hwQe2000Init_sp->uScTxdmaSotDelayInClocks,
			               hwQe2000Init_sp->uScGrantoffset,
			               hwQe2000Init_sp->uScGrantDelay,
                           hwQe2000Init_sp->bDmode);

    if (sts_ul!= HW_QE2000_STS_OK_K) {
        return sts_ul;
    }

    {
        int32 uData;

        /* CHANGE PM REFRESH RATE TO "OPERATIONAL" RATE IF WE'RE GETTING GRANTS */
        /* Clear errors */
        uData = SAND_HAL_READ(userDeviceHandle, KA, SC_ERROR);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_ERROR, uData);

        /* Delay then check for errors */
        thin_delay(HW_QE2000_1_MSEC_K);
        uData = SAND_HAL_READ(userDeviceHandle, KA, SC_ERROR);

        if ((SAND_HAL_GET_FIELD(KA, SC_ERROR, SOT_WATCHDOG_TIMEOUT, uData)==0) || (hwQe2000Init_sp->bStandAlone)) {
            /* If getting SOTs, update the refresh count */
            int32 uRefreshCount = 0x798*250/(hwQe2000Init_sp->uClockSpeedInMHz); /* scale refresh frequency based on core clock speed */
            uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG0, REFRESH_CNT, uData, uRefreshCount);
            SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG0, uData);
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("pm_config0: Refresh count (0x%x)\n", uRefreshCount));
        }
        else {
          QE2000PRINTF(QE2000_DEBUG_FUNC, ("Refresh count not updated, perform after init\n"));
        }
    }

    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InReset()
 *
 * OVERVIEW:        Put the QE in soft reset.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Soft reset the QE device.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/

static void hwQe2000InReset(sbhandle userDeviceHandle)
{
    int32 nData;
    SOC_SBX_WARM_BOOT_DECLARE(int32 _wb);

#if defined(BCM_WARM_BOOT_SUPPORT)
    COMPILER_REFERENCE(_wb);
#endif

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, _wb);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CONFIG, CII_ENABLE, 0x0);
    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, _wb);
    nData = SAND_HAL_READ(userDeviceHandle, KA, PC_RESET);
    nData = SAND_HAL_MOD_FIELD(KA, PC_RESET, SOFT_RESET,nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PC_RESET, PC_CORE_RESET,nData, 1);
    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, _wb);
    nData = SAND_HAL_MOD_FIELD(KA, PC_RESET, RESET_RXBUF,nData, 1);
    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, _wb);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_RESET, nData);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_CORE_RESET0, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_CORE_RESET1, 0xffffffff);
    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, _wb);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_TX_RING_PTR, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_TX_RING_PRODUCER, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_COMPLETION_RING_PTR, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_COMPLETION_RING_CONSUMER, 0);
    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, _wb);

    thin_delay(HW_QE2000_100_MSEC_K);

    /* The only W1TC bit is PC_INTERRUPT; this clears it */
    nData = SAND_HAL_READ(userDeviceHandle, KA, PC_INTERRUPT);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_INTERRUPT, nData);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000UnReset()
 *
 * OVERVIEW:        Take the QE out of soft reset.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Take the QE out of soft reset.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static void hwQe2000UnReset(sbhandle userDeviceHandle)
{
    uint32 nData;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    nData = SAND_HAL_READ(userDeviceHandle, KA, PC_RESET);
    nData = SAND_HAL_MOD_FIELD(KA, PC_RESET, PC_CORE_RESET, nData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_RESET, nData);
    thin_delay(HW_QE2000_100_MSEC_K);

    /* Test code, take out of soft reset after PC_CORE_RESET */
    nData = SAND_HAL_MOD_FIELD(KA, PC_RESET, SOFT_RESET, nData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_RESET, nData);
    thin_delay(HW_QE2000_100_MSEC_K);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitDone()
 *
 * OVERVIEW:        Await initialization completion
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Start command, wait for ack, remove ack
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

static sbBool_t
hwQe2000InitDone(sbhandle userDeviceHandle,
                 uint32     ctlreg,
                 uint32     init,
                 uint32     done,
                 uint32     timeout_secs)
{
    uint32 initial_reg, reg;
    uint32 i;
    sbBool_t bTimedout;

#ifdef BCM_EASY_RELOAD_SUPPORT
    /* If we are reloading, we are never checking for done bits, we never wrote to the hardware */
    if (SOC_IS_RELOADING((int32)userDeviceHandle)) {
	bTimedout = FALSE;
	return (bTimedout);
    }
#endif
    /* set the init bit */
    initial_reg=SAND_HAL_READ_OFFS_POLL(userDeviceHandle, ctlreg);
    SAND_HAL_WRITE_OFFS(userDeviceHandle, ctlreg, (initial_reg | init));

    bTimedout = TRUE;

    /* wait for the done to come back */
    for (i = 0; i < (timeout_secs*1000); i++) {

        reg = SAND_HAL_READ_OFFS(userDeviceHandle, ctlreg);

        if (reg & done) {
            bTimedout = FALSE;
            break;
        }
        thin_delay(HW_QE2000_1_MSEC_K);
    }

    /* clear the init (assumes the init bit was clear before the function was called) */
    SAND_HAL_WRITE_OFFS(userDeviceHandle, ctlreg, (initial_reg | done));

    /* if we've timed out, return error */

    return (bTimedout);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000RegDone()
 *
 * OVERVIEW:        Await command completion
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *                  non zero on timeout zero if success.
 *
 * DESCRIPTION:     Start command, wait for ack, remove ack
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

static sbBool_t
hwQe2000RegDone(sbhandle userDeviceHandle,
               uint32 ctlreg,
               uint32 done,
               uint32 timeout_100msecs)
{
    uint32 reg;
    uint32 i;
    sbBool_t bTimedout = TRUE;

    /* wait for the done to come back */

    for (i = 0; i < (timeout_100msecs * 10000); i++) {

        reg = SAND_HAL_READ_OFFS_POLL(userDeviceHandle, ctlreg);

        if (reg & done) {
            bTimedout = FALSE;
            break;
        }
        thin_delay(HW_QE2000_10_USEC_K);
    }

    /* if we've timed out, return error */
    return (bTimedout);
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitSetUpDrDllTrim
 *
 * OVERVIEW:        Sets up the selected DR group's DLL trim delay
 *
 * ARGUMENTS:
 *                  sbhandle userDeviceHandle
 *                  uint32 nDr
 *                  uint32 nReg
 *                  uint32 nDlyTrim
 *
 * RETURNS:         Status
 *
 * DESCRIPTION:     Sets up the selected DR group's DLL trim delay
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

static uint32
hwQe2000InitSetUpDrDllTrim(sbhandle userDeviceHandle, uint32 nDr, uint32 nReg, uint32 nDlyTrim) {
    uint32 uData;

#define PM_MEM_WRITE_TIMEOUT     (4) /* 400 milliseconds */

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DLL_ACC_DATA, nDlyTrim);

    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, REQ, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, RD_WR_N, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, uData, nDr);
    uData = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, uData, nReg+0x80);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DLL_ACC_CTRL, uData);


    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_PM_DLL_ACC_CTRL_OFFSET,
                        0x80000000,
                        PM_MEM_WRITE_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout on PM memory write\n"));
        return HW_QE2000_STS_INIT_PM_MEM_TIMEOUT_ERR_K;
    }

    /* clear req/ack bits */
    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, ACK, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DLL_ACC_CTRL, uData);

  return HW_QE2000_STS_OK_K;

}


static uint32 hwQe2000_PmManInitCmd(sbhandle userDeviceHandle, uint32 nCmd, uint32 nManInit) {
    uint32 nData;
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND, nManInit, nCmd);
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_REQ, nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nData, 1);
    SAND_HAL_WRITE (userDeviceHandle, KA, PM_MAN_INIT, nData);

    /* verify the write happened */
    nData = SAND_HAL_READ_POLL(userDeviceHandle, KA, PM_MAN_INIT);

  if (SAND_HAL_GET_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nData) != 1) {
      QE2000PRINTF(QE2000_ERROR, ("ERROR: PM manual init Ack not SB_ASSERTed, PM_MAN_INIT=0x%x\n",nData));
      assert(0);
  }

  return nData;
}

static void hwQe2000_BringUpPmManually(sbhandle userDeviceHandle){
    uint32 nNopCmd = 0;
    uint32 nPreCmd = 1;
    uint32 nMrsCmd = 2;
    uint32 nRefCmd = 3;
    uint32 nMrDefault      = 0x643;
    uint32 nMr_dllReset    = 0x100;
    uint32 nEMr_ocdDefault = 0x380;
    uint32 nEMrDefault     = 0x41c;
    int32 nRefresh=0;
    uint32 nData;

    /* save it for restore later */
    uint32 nMrData  = SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_MR);

    uint32 nManInit = 0;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_ACTIVE, nManInit, 1);

    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nNopCmd, nManInit); /* Nop */
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nPreCmd, nManInit); /* Precharge */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 2);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /*mode register ba=2, add=0000 */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 3);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /*mode register ba=3, add=0000 */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* enable DLL */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 0);
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nMrDefault | nMr_dllReset);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* DLL reset */

    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nPreCmd, nManInit); /* Precharge */

    for (nRefresh=0; nRefresh<8; nRefresh++) {
        /* assuming 300ns for one pair of write read cycles, */
        /* each refresh consumes at least 75 clocks */
        /* eight refresh will consumes 600 clocks */
        /* more than enough to allow the DLL to lock (200 clocks) */
        nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nRefCmd, nManInit); /* Refresh */
    }

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 0);
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nMrDefault);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* Initialize without DLL reset */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault | nEMr_ocdDefault);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* enable OCD default */

    nData = 0;
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
    nManInit = hwQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* exit OCD setup */

    /* exit manual init mode */
    nManInit = 0;
    nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_ACTIVE, nManInit, 0);
    nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND, nManInit, 0);
    nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_REQ, nManInit, 0);
    nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nManInit, 1);
    SAND_HAL_WRITE (userDeviceHandle, KA, PM_MAN_INIT, nManInit);

    /* restore PM_DDR_MR */
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nMrData);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitPmMemWrite
 *
 * OVERVIEW:        Writes a line of data out of the external ddr memory
 *
 * ARGUMENTS:
 *                  sbhandle userDeviceHandle
 *                  uint32  nAddress
 *                  uint32* pData3
 *                  uint32* pData2
 *                  uint32* pData1
 *                  uint32* pData0
 *
 *
 * RETURNS:         Status
 *
 * DESCRIPTION:     Writes a line of data out of the external ddr memory
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

static uint32
hwQe2000InitPmMemWrite(sbhandle userDeviceHandle, uint32 nAddress, uint32 nData3, uint32 nData2, uint32 nData1, uint32 nData0){

#define PM_MEM_WRITE_TIMEOUT     (4) /* 400 milliseconds */

    uint32 uData;

    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_DATA0, nData0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_DATA1, nData1);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_DATA2, nData2);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_DATA3, nData3);

    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, REQ, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, uData, nAddress);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_CTRL, uData);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_PM_MEM_ACC_CTRL_OFFSET,
                        0x80000000,
                        PM_MEM_WRITE_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM memory timeout error\n"));
        return HW_QE2000_STS_INIT_PM_MEM_TIMEOUT_ERR_K;
    }

    /* clear req/ack bits */
    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, ACK, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_CTRL, uData);
    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000PmMemRead
 *
 * OVERVIEW:        Reads a line of data out of the external ddr memory
 *
 * ARGUMENTS:
 *                  sbhandle userDeviceHandle
 *                  uint32 nAddress
 *                  uint32* pData3
 *                  uint32* pData2
 *                  uint32* pData1
 *                  uint32* pData0
 *
 *
 * RETURNS:         Status
 *
 * DESCRIPTION:     Reads a line of data out of the external ddr memory
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000PmMemRead(sbhandle userDeviceHandle, uint32 nAddress, uint32* pData3, uint32* pData2, uint32* pData1, uint32* pData0){

    uint32 uData;

    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, REQ, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, RD_WR_N, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, uData, nAddress);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_CTRL, uData);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_PM_MEM_ACC_CTRL_OFFSET,
                        0x80000000,
                        PM_MEM_WRITE_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM memory write timeout\n"));
        return HW_QE2000_STS_INIT_PM_MEM_TIMEOUT_ERR_K;
    }

    /* clear req/ack bits */
    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_MEM_ACC_CTRL, ACK, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_MEM_ACC_CTRL, uData);

    /* Read the data out */
    *pData0=SAND_HAL_READ(userDeviceHandle, KA, PM_MEM_ACC_DATA0);
    *pData1=SAND_HAL_READ(userDeviceHandle, KA, PM_MEM_ACC_DATA1);
    *pData2=SAND_HAL_READ(userDeviceHandle, KA, PM_MEM_ACC_DATA2);
    *pData3=SAND_HAL_READ(userDeviceHandle, KA, PM_MEM_ACC_DATA3);

    return HW_QE2000_STS_OK_K;
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000TrainingPmDdr()
 *
 * OVERVIEW:        Runs the training algorithm on the DDR2 interface
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:         status of initialization
 *
 * DESCRIPTION:     Runs the training algorithm on the DDR2 interface
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

static uint32
hwQe2000TrainingPmDdr(sbhandle userDeviceHandle, sbBool_t bPmHalfBus) {
    /*
            - DDR trim delay training
            - Each dram is 16 bit wide, it correspond to a channel
            - Each channel has an upper byte and a lower byte
            - Each byte has its own DLL to trim DQS delay
            - For indirect memory access, channel # = indirect address[2:0]
            - Each DR group is connected to two channels
            - Each DR group therefore controls 4 byte lanes

              addr channel dram dr_group
            - --------------------------
            -  0   0       0    0
            -  1   1       2    1
            -  2   2       4    2
            -  3   3       6    3
            -  4   4       1    0
            -  5   5       3    1
            -  6   6       5    2
            -  7   7       7    3

            - Pseudo code for DDR training

            - (1) Initialize dram for addr[7:0], i.e. for channels[7:0]
            - (2) for each channel
            -       for (delay=0,1,...)
            -         read back the data
            -         for the two byte lanes at that channel
            -         check for data mismatch
            - (3) post process the results above
            -       for each byte lane
            -         find longest span of delay values where there is no error
            -         the mid-point of this span is the optimal delay for this byte lane
    */

    const uint32 uChannelToDrGroup[8] = {0,1,2,3,0,1,2,3};
    const uint32 uHalfBusChannelToDrGroup[4] = {2,3,2,3};
    const uint32 uChannelToReg[8] = {0,0,0,0,2,2,2,2};
    const uint32 uHalfBusChannelToReg[4] = {0,0,2,2};
    uint32 uRdData[4];
    uint32 uWrData[4];
    uint32 uBytePos;
    static uint32 uResultsArr[256][16]; /* index [delay][byte position] */
    uint32 uDly, uMaxLanes;
    uint32 uByte;
    uint32 uW;
    uint32 uWrAddr;
    uint32 uChannel;
    uint32 uByteLane;
    uint32 uMaxAddr = 8;
    uint32 uMaxDelay = 256;
    uint32 uDrGroup;
    uint32 uWord;
    uint32 uReg;
    uint32 uSetUpDrDllStatus;

    const uint32 uUpperByteMask = 0xff00ff00;
    const uint32 uLowerByteMask = ~uUpperByteMask;

    if (bPmHalfBus){
        uMaxLanes = 8;
        uMaxAddr = 4;
    }
    else {
        uMaxLanes = 16;
        uMaxAddr = 8;
    }

    /* this matrix keeps track of error count at a byte position for a
     *  particular trimming delay. Initialize to 0 count.
     */
    for (uDly=0; uDly<256; uDly++) {
        for (uByte=0; uByte<16; uByte++) {
            uResultsArr[uDly][uByte] = 0;
        }
    }

    /* use a data pattern of 0x0000ffff, at the memory interface, this
     * pattern will be translated to 0x55555555 for a maximum bit transitions.
     */
    for (uW=0; uW<4; uW++) {
        uWrData[uW] = 0x0000ffff;
    }

    /* write initial data to memory once */
    for (uWrAddr=0; uWrAddr<uMaxAddr; uWrAddr++) {

        if (hwQe2000InitPmMemWrite(userDeviceHandle,
                                   uWrAddr,
                                   uWrData[3],
                                   uWrData[2],
                                   uWrData[1],
                                   uWrData[0]) != HW_QE2000_STS_OK_K) {

            QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DRR Training Failed\n"));
            return HW_QE2000_STS_INIT_PMC_DDR_TRAINING_FAIL_K;
        }

    }

    for (uChannel=0; uChannel<uMaxAddr; uChannel++) {

        if (bPmHalfBus)
        {
            uDrGroup = uHalfBusChannelToDrGroup[uChannel];
            uReg = uHalfBusChannelToReg[uChannel];
        }
        else
        {
            uDrGroup = uChannelToDrGroup[uChannel];
            uReg = uChannelToReg[uChannel];
        }

        for (uDly = 0; uDly < (uint32)uMaxDelay; uDly++) {
            for(uByte = 0; uByte < 2; uByte++) {

                uSetUpDrDllStatus=hwQe2000InitSetUpDrDllTrim(userDeviceHandle,
                                                 uDrGroup,
                                                 uReg + uByte,
                                                 uDly);
                if (uSetUpDrDllStatus != HW_QE2000_STS_OK_K)
                     return uSetUpDrDllStatus;

                 /* Read back data and check for error */
                if (hwQe2000PmMemRead(userDeviceHandle,
                                uChannel,
                                &uRdData[3],
                                &uRdData[2],
                                &uRdData[1],
                                &uRdData[0]) != HW_QE2000_STS_OK_K) {

                     QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DDR Training failed\n"));
                     return HW_QE2000_STS_INIT_PMC_DDR_TRAINING_FAIL_K;
                }

                for (uWord=0; uWord<4; uWord++) {
                    if((uRdData[uWord] & uUpperByteMask) != (uWrData[uWord] & uUpperByteMask)) {
                        uBytePos = uChannel*2;
                        uResultsArr[uDly][uBytePos]++;
                    }
                    if ((uRdData[uWord] & uLowerByteMask) != (uWrData[uWord] & uLowerByteMask)) {
                        uBytePos = uChannel*2 + 1;
                        uResultsArr[uDly][uBytePos]++;
                    }
                }
            }
        } /* uDly */
    }

    /* post process the result matrix
       for each byte lane, find the longest span of delays that results no error.
       The mid-point of this span is the optimum trim value to use. */
    for (uByteLane = 0; uByteLane < uMaxLanes; uByteLane++) {
        uint32 uPos = 0;
        uint32 uBestPos = 0;
        uint32 uBestCount = 0;

          uPos = 0;
          while (uPos<256) {
            /* uPos: 0,    1,   2,   ..126, 127, 128,.., 255
               uDly: 128,  129, 130, ..254, 255, 0,      127
               =-128, -127          -2   -1  0       127 */
	    if (uPos<128) {
	      uDly = uPos+128;
	    } else {
	      uDly = uPos-128;
	    }
            if (uResultsArr[uDly][uByteLane] == 0) {
              /* find out the run length */
              uint32 uStartPos = uPos;
              uint32 uCount = 0;
              uint32 j;
              for (j=uPos; j<256; j++) {
                uint32 uDlyTmp;
                uCount++;
                uPos++;
                uDlyTmp = (uPos - 128) & 0xff;
                if (uResultsArr[uDlyTmp][uByteLane] != 0) break;
              }

              /* store the best run length + starting position */
              if (uCount>uBestCount) {
                uBestCount=uCount;
                uBestPos=uStartPos;
              }

            } else {
              uPos++;
            }
          }

          if (uBestCount==0) { /* were no good bit positions for this byte lane */
            QE2000PRINTF(QE2000_ERROR, ("ERROR: No good bit positions for this byte lane DDR training failed\n"));
            return HW_QE2000_STS_INIT_PMC_DDR_TRAINING_FAIL_K;
          } else { /* there was at least 1 good bit position */
            uint32 uBestDly;

            uBestDly      = (uBestPos + (uBestPos + uBestCount))/2; /* mid-point of span */
            uBestDly      = (uBestDly - 128) & 0xff;                /* translated to delay number */
           /*
            * now update registers with best trim values
            */
            uChannel = uByteLane >> 1;
            if (bPmHalfBus)
            {
                uDrGroup = uHalfBusChannelToDrGroup[uChannel];
                uReg = uHalfBusChannelToReg[uChannel];
            }
            else
            {
                uDrGroup = uChannelToDrGroup[uChannel];
                uReg = uChannelToReg[uChannel];
            }
            if ((uByteLane & 0x1)==0) uReg |= 1;
            uSetUpDrDllStatus=hwQe2000InitSetUpDrDllTrim(userDeviceHandle, uDrGroup,uReg,uBestDly);
            if (uSetUpDrDllStatus != HW_QE2000_STS_OK_K)
              return uSetUpDrDllStatus;
        }
    }

    return HW_QE2000_STS_OK_K;
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitPm()
 *
 * OVERVIEW:        Initialize the QE2000 PM Block
 *
 * ARGUMENTS:
 *
 *     Input parameters:
 *     TYPE            NAME              SET AT    DESCRIPTION
 *     ----            ----              ------    -----------
 *     sbBool_t        bPmHalfBus        node      128/64 bit external DDR2 bus
 *     sbBool_t        bDmode            system    dmode/cmode
 *     sbBool_t        bFabCrc32         system    32/16 bit crc packet payload protection
 *     uint32        uClockSpeedInMHz  system    clock speed in MHz
 *     sbBool_t        bPmRunSelfTest    node      user can kick off self test at this point if they want
 *     sbBool_t        bStandAlone       SYSTEM    QE2K is operating in standalone switch mode
 *     sbBool_t        bHybrid           SYSTEM    QE2K is operating in hybrid mode
 *
 *     Other notes:
 *     PM_CONFIG0.LEN_ADJ_MODE is not configured here, it could be a system level attribute
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the QE2000 Packet Memory Block
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32 hwQe2000InitPm(sbhandle userDeviceHandle,
                               sbBool_t bQm512MbDdr2,
                               sbBool_t bPmHalfBus,
                               sbBool_t bDmode,
                               sbBool_t bFabCrc32,
                               sbBool_t bPmRunSelfTest,
                               uint32 uClockSpeedInMHz,
                               sbBool_t bStandAlone,
                               sbBool_t bHybrid,
                               sbBool_t bPmDdrTrain,
                               int32  nLinesPerTimeslot,
                               int32  nLinesPerTimeslotCongested,
                               int32  nLinesPerTimeslotQe1k,
                               sbBool_t bRunLongDdrMemoryTest,
                               int32 uPacketAdjustFormat)
{

#define AUTO_INIT_TIMEOUT     (4) /* seconds */

    uint32 uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG0);
    uint32 uRefreshCount;
    uint32 uTrainingPmDdrStatus;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG0, MEM_MODE, uData, bPmHalfBus);


    uRefreshCount = 0x3CC*250/uClockSpeedInMHz; /* scale refresh frequency based on core clock speed */
                                                  /* refresh rates needs to be 2X functional rate until SCI is brought up */
    uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG0, REFRESH_CNT, uData, uRefreshCount);
    uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG0, LEN_ADJ_MODE, uData, uPacketAdjustFormat);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG0, uData);

    if (bStandAlone) {
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG1);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES1_QE2K, uData, 0x01);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES2_QE2K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG1, uData);

        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES1_QE1K, uData, 0x01);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES2_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG2, uData);

        /* Select either 32-bit or 16-bit CRC */
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG3);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, SELECT_CRC32, uData, bFabCrc32);

        /* timeslot size has been updated for TME to be 760ns, lines per timeslot is now set accordingly */
        /* to 208 primary, and 176 during congestion */
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES1, uData, nLinesPerTimeslot);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES2, uData, nLinesPerTimeslotCongested);


        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG3, uData);
    } else if (bHybrid) {
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG1);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES1_QE2K, uData, nLinesPerTimeslot);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES2_QE2K, uData, nLinesPerTimeslotCongested);

        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG1, uData);

        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES1_QE1K, uData, 0x01);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES2_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG2, uData);

        /* Select either 32-bit or 16-bit CRC */
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG3);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, SELECT_CRC32, uData, bFabCrc32);

        /* timeslot size has been updated for TME to be 760ns, lines per timeslot is now set accordingly */
        /* to 208 primary, and 176 during congestion */
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES1, uData, nLinesPerTimeslot);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES2, uData, nLinesPerTimeslotCongested);


        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG3, uData);
    } else {
      if (bDmode) {
        /* QE1K should never be granted, therefore the QE1K lines per timeslot is set to 1 line (both primary and degraded). */

          /* timeslot size has been updated for Dmode to be 760ns, lines per timeslot is now set accordingly */
          /* to 208 primary, and 176 during congestion */
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG1);

        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES1_QE2K, uData, nLinesPerTimeslot);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES2_QE2K, uData, nLinesPerTimeslotCongested);

        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG1, uData);

        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES1_QE1K, uData, 0x01);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES2_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG2, uData);

      } else {
        /* in Cmode, the timeslot duration will be 760ns. The QE2k send 208 lines primarily, and 176 lines during congestion. */
        /* QE1k sends 104 lines both primarily and under congestion. */
        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG1);

        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES1_QE2K, uData, nLinesPerTimeslot);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES2_QE2K, uData, nLinesPerTimeslotCongested);

        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG1, uData);

        uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG2);

        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES1_QE1K, uData, nLinesPerTimeslotQe1k);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG2, NUM_FAB_LINES2_QE1K, uData, nLinesPerTimeslotQe1k);

        SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG2, uData);
      }

      /* Select either 32-bit or 16-bit CRC */
      uData = SAND_HAL_READ(userDeviceHandle, KA, PM_CONFIG3);
      uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, SELECT_CRC32, uData, bFabCrc32);

      /* Local lines per timeslot is set to 4 */
      uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES2, uData, 0x04);
      uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG3, NUM_LOC_LINES1, uData, 0x04);
      SAND_HAL_WRITE(userDeviceHandle, KA, PM_CONFIG3, uData);
    }


    /* Bug 24539, disable redundant memories - bug in QE2000 redundancy logic */
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DEBUG_RB_WR_REQ_MEM, 0xf);

    /* (This is required to run before the block is taken out of soft reset) */
    uData = SAND_HAL_READ(userDeviceHandle, KA, PM_BIST_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(KA, PM_BIST_CONFIG0, BIST_SETUP, uData, 2);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_BIST_CONFIG0, uData);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_BIST_CONFIG1, 0x1ff);

    if (hwQe2000RegDone(userDeviceHandle,
                       SAND_HAL_KA_PM_BIST_STATUS0_OFFSET,
                       0x1ff,
                        INTERNAL_MEMBIST_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM internal memory bist timed out\n"));
        return HW_QE2000_STS_INIT_PM_INTERNAL_MEMBIST_TIMEOUT_ERR_K;
    }

    /* Clear MBIST */
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_BIST_CONFIG1, 0);

    /* Take the PM block out of soft reset */

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, PM_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, DR_CORE_RESET, 0);

#if 0
    /* Kick off auto init */
    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_PM_CONFIG0_OFFSET,
                         SAND_HAL_KA_PM_CONFIG0_AUTO_INIT_START_MASK,
                         SAND_HAL_KA_PM_CONFIG0_AUTO_INIT_DONE_MASK,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM auto init timed out\n"));
        return HW_QE2000_STS_INIT_PMC_AUTO_INIT_TIMEOUT_ERR_K;
    }
#endif

    /* Calibrate the DDR interfaces using the indirect access registers + pm_dll_acc register. */
    /* Dominic is writing code to do this- should be able to use his stuff */

    uData = SAND_HAL_READ(userDeviceHandle, KA, PM_IOCAL0);

    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_GDDRI, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_GDDRII, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_GDDRIII, uData, 0);

    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_TERM150_120, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_TERM300_240, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, PM_IOCAL0, IO_TERM60, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_IOCAL0, uData);

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PM_IOCAL0, IO_UPDATE_ENABLE, 0xff);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PM_IOCAL0, IO_START_CALIBRATION, 1);


    if (hwQe2000RegDone(userDeviceHandle,
                       SAND_HAL_KA_PM_IOCAL1_OFFSET,
                       SAND_HAL_KA_PM_IOCAL1_IO_CALIBRATION_DONE_MASK,
                        INTERNAL_MEMBIST_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: QE2000 IO calibration failed\n"));
        return HW_QE2000_STS_INIT_PM_IO_CALIBRATION_FAILURE_K;
    }

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PM_IOCAL0, IO_UPDATE_ENABLE, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PM_IOCAL0, IO_START_CALIBRATION, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PM_IOCAL1, IO_CALIBRATION_DONE, 1);

    /* Clock Enable */
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR2_CKE_N, 0);

    uData = SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_EMR);
    uData &= ~(1 << 6);
    uData |= (1 << 2);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_EMR, uData);

#if 0
    /* Kick off auto init */
    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_PM_CONFIG0_OFFSET,
                         SAND_HAL_KA_PM_CONFIG0_AUTO_INIT_START_MASK,
                         SAND_HAL_KA_PM_CONFIG0_AUTO_INIT_DONE_MASK,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PMC auto init timeout\n"));
        return HW_QE2000_STS_INIT_PMC_AUTO_INIT_TIMEOUT_ERR_K;
    }
#endif
    hwQe2000_BringUpPmManually(userDeviceHandle);


    if (bPmDdrTrain) {

        /* DDR2 DLL timing Calibration */
        uTrainingPmDdrStatus=hwQe2000TrainingPmDdr(userDeviceHandle, bPmHalfBus);
        if (uTrainingPmDdrStatus != HW_QE2000_STS_OK_K)
            return uTrainingPmDdrStatus;
    }

    /* kick off mode 0 DDR test using pm_ddr_test, pm_ddr_start, pm_ddr_end register */
    if (bPmRunSelfTest) {

        uint32 uEndAddress;

        if (bPmHalfBus == TRUE) {
            uEndAddress = 0x7ffff;
        }
        else {
            uEndAddress = 0xfffff;
        }

        /* When there is 512Mb memory devices, there are no holes, run the test */
        if  (bQm512MbDdr2== TRUE) {
            SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_START, 0x00000000);
            SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_END, uEndAddress);

            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Running mode 0 DDR memory test\n"));
            if (hwQe2000InitDone(userDeviceHandle,
                                 SAND_HAL_KA_PM_DDR_TEST_OFFSET,
                                 0x0001,
                                 0x0002,
                                 AUTO_INIT_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DDR test timed out\n"));
                return HW_QE2000_STS_INIT_PMC_SELFTEST_TIMEOUT_ERR_K;
            }

            uData=SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_TEST);
            if (SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData) ||
                SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DDR Test failed test fail upper(0x%x), test fail lower(0x%x)\n",
                                            SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData),
                                            SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)));
                return HW_QE2000_STS_INIT_PMC_SELFTEST_FAIL_K;
            }
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("mode 0 DDR memory test SUCCEEDS\n"));
        }
        /* There are 256Mb memories, this means there are holes in the memory */
        else {
            uint32 uStartAddress, uAddressHoleStart, uAddressHoleEnd;

            uStartAddress = 0;

            if (bPmHalfBus == TRUE) {
                uAddressHoleStart = 0x1f;
                uAddressHoleEnd = 0x40;
            } else {
                uAddressHoleStart = 0x3f;
                uAddressHoleEnd = 0x80;
            }
            while(uStartAddress < uEndAddress) {
                SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_START, uStartAddress);
                SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_END, uStartAddress + uAddressHoleStart);

                if (hwQe2000InitDone(userDeviceHandle,
                                     SAND_HAL_KA_PM_DDR_TEST_OFFSET,
                                     0x0001,
                                     0x0002,
                                     AUTO_INIT_TIMEOUT)) {
                    QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DDR test timed out\n"));
                    return HW_QE2000_STS_INIT_PMC_SELFTEST_TIMEOUT_ERR_K;
                }

                uStartAddress+= uAddressHoleEnd;

                uData=SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_TEST);
                if (SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData) ||
                    SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)) {
                    QE2000PRINTF(QE2000_ERROR, ("ERROR: PM DDR Test failed test fail upper(0x%x), test fail lower(0x%x)\n",
                                                SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData),
                                                SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)));
                    return HW_QE2000_STS_INIT_PMC_SELFTEST_FAIL_K;
                }
            }
        }
    }

    /* Currently, this test only runs for full size memories */
    if ((bRunLongDdrMemoryTest) && (bQm512MbDdr2 == TRUE)) {
        uint32 uStartAddress = 0;
        uint32 uEndAddress = 0xfffff;
        int32 status;

        if (bPmHalfBus == TRUE) {
            uEndAddress = 0x7ffff;
        }

        /* Run mode 1 test, walking ones and zeros */
        status = hwQe2000_RunMode1PmDdrTest(userDeviceHandle,
                                            uStartAddress,
                                            uEndAddress);

        if (status != 0) {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Mode 1 DDR test FAILS\n"));
            return status;
        } else {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Mode 1 DDR TEST SUCCEEDS\n"));
        }

        /* Run mode 2 test, full speed mimic of system operation */
        status = hwQe2000_RunMode2PmDdrTest(userDeviceHandle,
                                            uStartAddress,
                                            uEndAddress);
        if (status != 0) {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Mode 2 DDR test FAILS\n"));
            return status;
        } else {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Mode 2 DDR TEST SUCCEEDS\n"));
        }

    }
    return HW_QE2000_STS_OK_K;
}

static uint32 hwQe2000_RunMode1PmDdrTest(sbhandle userDeviceHandle,
                                           uint32 uStartAddress,
                                           uint32 uEndAddress) {

    uint32 uData;

    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_START, uStartAddress);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_END, uEndAddress);

    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_DDR_TEST, MODE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, PM_DDR_TEST, RAM_TEST, uData, 1);

    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_TEST, uData);

    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_PM_DDR_TEST_OFFSET,
                         uData,
                         0x0002,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM Mode 1 DDR test timed out\n"));
        return HW_QE2000_STS_INIT_PMC_SELFTEST_TIMEOUT_ERR_K;
    }

    uData=SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_TEST);

    if (SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData) ||
        SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM Mode 1 DDR Test failed test fail upper(0x%x), test fail lower(0x%x)\n",
                                    SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData),
                                    SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)));
        return HW_QE2000_STS_INIT_PMC_SELFTEST_FAIL_K;
    }
    return HW_QE2000_STS_OK_K;
}
static uint32 hwQe2000_RunMode2PmDdrTest(sbhandle userDeviceHandle,
                                           uint32 uStartAddress,
                                           uint32 uEndAddress) {

    uint32 uData;


    SAND_HAL_WRITE(userDeviceHandle, KA, PM_TEST_DATA0, 0x12345678);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_TEST_DATA1, 0xAAAAAAAA);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_TEST_DATA2, 0x98765432);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_TEST_DATA3, 0x55555555);

    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_START, uStartAddress);
    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_END, uEndAddress);

    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, PM_DDR_TEST, MODE, uData, 2);
    uData = SAND_HAL_MOD_FIELD(KA, PM_DDR_TEST, RAM_TEST, uData, 1);

    SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_TEST, uData);


    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_PM_DDR_TEST_OFFSET,
                         uData,
                         0x0002,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM Mode 2 DDR test timed out\n"));
        return HW_QE2000_STS_INIT_PMC_SELFTEST_TIMEOUT_ERR_K;
    }

    uData=SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_TEST);

    if (SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData) ||
        SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: PM Mode 2 DDR Test failed test fail upper(0x%x), test fail lower(0x%x)\n",
                                    SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, uData),
                                    SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, uData)));
        return HW_QE2000_STS_INIT_PMC_SELFTEST_FAIL_K;
    }
    return HW_QE2000_STS_OK_K;
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitQs()
 *
 * OVERVIEW:        Initialize QOS block
 *
 * ARGUMENTS:
 *
 *     Input parameters:
 *     TYPE            NAME           SET AT    DESCRIPTION
 *     ----            ----           ------    -----------
 *     sbBool_t        bDmode            system    dmode/cmode
 *     uint32        uQsMaxNodes       system    maximum number of nodes the system will support
 *     uint32        uNodeId           node      node id # for this node
 *     uint32        uEpochSizeInNs    system    epoch duration in NS
 *     uint32        uClockSpeedInMHz  system    clock speed in MHz
 *
 *
 * RETURNS:
 *
 *
 * DESCRIPTION:     Start command, wait for ack, remove ack
 *
 * Initialize the QOS block.
 *
 *     NOTES:
 *      1) This does not setup the node types of the system (QS_CONFIG2, QS_CONFIG3). I'm assuming these will be set up somewhere else.
 *         (This table needs to be reconfigured every time a new node is added to the system-- at least in a CMODE system)
 *      2) This uses the qos auto init to setup all the memories. This sets up a standard FIC mode queue<->egress port mapping
 *         with 8 COS levels per port.
 *****************************************************************************/
static int32 hwQe2000QsMemWrite(sbhandle userDeviceHandle, int32 nAddr, int32 nTable, uint32 ulData)
{
    uint32 ulTimeOut;
    uint32 ulCntrl;


    SAND_HAL_WRITE(userDeviceHandle, KA, QS_MEM_ACC_DATA, ulData);

    ulCntrl = SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ACK, 0x1) |
        SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, REQ, 0x1) |
        SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, RD_WR_N, 0x0) |
        SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTable) |
        SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, nAddr);

    SAND_HAL_WRITE(userDeviceHandle, KA, QS_MEM_ACC_CTRL, ulCntrl);

    ulTimeOut = 10000;
    while(ulTimeOut--)
    {
        ulCntrl = SAND_HAL_READ_POLL(userDeviceHandle, KA, QS_MEM_ACC_CTRL);
        if(SAND_HAL_GET_FIELD(KA, QS_MEM_ACC_CTRL, ACK, ulCntrl))
        {
            SAND_HAL_WRITE(userDeviceHandle, KA, QS_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ACK, 0x1));

            return( 0 );
        }
#ifndef VXWORKS
        /* Joey ported stephen's change from 1.1.2.7 to 1.1.2.8. thin_delay(HW_QE2000_100_MSEC_K); */
        thin_delay(1000);
#endif
    }
    QE2000PRINTF(QE2000_ERROR, ("ERROR: QS memory write completion timed out\n"));
    return( -1 );
}

/*
 * Starting values for the arbiter's psuedo-random number generator.
 * It really doesn't matter what these are, as long as they are truly random.
 * This data generated by radioactive decay http://www.fourmilab.ch/hotbits
 */
#define KA_RNDM_ARRAY_SIZE 55
#define M(x) ((x) & 0x3f)       /* mask to 6 bits */
static uint32 random_array[KA_RNDM_ARRAY_SIZE] = {
  M(250), M(243), M(253), M(46), M(47),
  M(115), M(122), M(247), M(7), M(191),
  M(18), M(184), M(40), M(251), M(46),
  M(178), M(136), M(184), M(125), M(187),
  M(184), M(247), M(87), M(35), M(243),
  M(8), M(51), M(136), M(132), M(57),
  M(57), M(119), M(173), M(207), M(85),
  M(77), M(237), M(119), M(199), M(124),
  M(152), M(178), M(110), M(254), M(77),
  M(97), M(181), M(179), M(53), M(192),
  M(3), M(221), M(10), M(156), M(63)
};
#undef M

static uint32
hwQe2000InitQs(sbhandle userDeviceHandle,
               sbBool_t bDmode,
               uint32 uQsMaxNodes,
               int32  nQueuesPerShaperIngress,
               uint32 uEpochSizeInNs,
               uint32 uClockSpeedInMHz,
               uint32 uNodeId,
               sbBool_t bStandAlone,
               sbBool_t bHybrid,
               sbBool_t bMixHighAndLowRateFlows,
               int32  spMode)
{

#define QOS_CREDIT_PERIOD_CLOCK_CYCLES 65536 /* number of clock cycles between credit/shaping increments */
#define CMODE_BAA_CREDIT_PERIOD 10240   /* cmode credits are in units of bits/1Kns */
#define BITS_PER_BYTE 8
#define QS_MEM_TIMEOUT (4)              /* 400 milliseconds */

    uint32 uData=0;
    uint32 uRandArrayIndex=0;
    int32 nQueue=0, nEgress=0;
    sbBool_t bMultiQueuePerShaper = FALSE; /* will set to true if more than 1 queue per shaper, pri lut is remapped in this case */
    uint32 uNumberOfPortsType1;
    uint32 uMaxNodesRefresh;
    int cos, template, is_allocated;
    int hwValue, actValue;
    int rv;

   /* MCM run mbist on QS block before taking out of soft reset */

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));

    /* Bug 24539 Disable redundant memory */
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_DEBUG6, 0xffc);

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, QS_BIST_CONFIG0, BIST_SETUP, 2);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_BIST_CONFIG1,  0x003fffff);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_QS_BIST_STATUS0_OFFSET,
                        0x003fffff,
                        INTERNAL_MEMBIST_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: QS BIST timeout\n"));

        return HW_QE2000_STS_INIT_QS_INTERNAL_MEMBIST_TIMEOUT_ERR_K;
    }
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_BIST_CONFIG1,  0);

    /* Take the QOS out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, QS_CORE_RESET, 0);

    uData=SAND_HAL_READ(userDeviceHandle, KA, QS_CONFIG0);
    if (bStandAlone) {
      /* ab 081605 bit banging since this bit is going away in later
        silicon
        uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG0, LOCAL_BAA, uData, 0x1);
      */
      uData |= (1<<24);
      SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG0, uData);

      SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG3, 1);
    }
    else {
        /* ab 081605 bit banging since this bit is going away in later
          silicon
          uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG0, LOCAL_BAA, uData, 0x0);
          */
        uData &= (~(1<<24));
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG0, uData);
    }

    if (SOC_SBX_CFG((int32)userDeviceHandle)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) {
      /* polaris system type use maximum values */
      uNumberOfPortsType1 = 0x3f;
      uMaxNodesRefresh = 0x3f;
    } else { 
      uNumberOfPortsType1 = 0x31;
      uMaxNodesRefresh = uQsMaxNodes;
    }

    uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG1, BACKGROUND_REFRESH, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG1, NUMBER_PORTS_TYPE1, uData, uNumberOfPortsType1);
    uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG1, NUMBER_PORTS_TYPE0, uData, 0x15);
    uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG1, NUMBER_NODES, uData, uMaxNodesRefresh);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG1, uData);

    /* Assuming all nodes are QE2000, safe for now since QE1000 is not supported */
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG2, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG3, 0xffffffff);


    /* Set to TME mode if standalone */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, TME_MODE, uData, bStandAlone);
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, MC_PORT_ID, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, NODE_ID, uData, uNodeId);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_LNA_CONFIG, uData);

    /* CCU settings */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, QS_CCU, DECAY_CREDITS_WHEN_EMPTY, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, QS_CCU, CREDIT_HOLD, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_CCU, uData);


    if (bStandAlone) {

        /* updated this code for bug 22331 using the calculator
         *
         *  used calcRateConversionParameter to calculate this value.
         *   nTimeslot=174 cycles
         *   nEpoch=1436 timeslots
         *   nTotalNumQueue=16384
         *   nTc=65536 cycles
         *   tIncomingRateConversionParameter=2.09829347164858
         *   tOutgoingRateConversionParameter=0.476577758789062
         */
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_MANTISSA,    uData,  488);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, uData,  10);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN,        uData,  1);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_MANTISSA,    uData,  268);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, uData,  7);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN,        uData,  1);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_RATE_CONVERSION, uData);

    }
    else {
        if (bDmode) {
            /* calculator location system_sim/kamino/units/ka_qs
             * nTimeslot=190 cycles, nEpoch=16433 timeslots, 16k queues
             * tIncomingRateConversionParameter=0.167918853910776
             * tOutgoingRateConversionParameter=5.95525741577148
             */
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_MANTISSA,    uData,  381);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, uData,  6);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN,        uData,  1);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_MANTISSA,    uData,  343);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, uData,  11);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN,        uData,  1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QS_RATE_CONVERSION, uData);
        }
        else {
            /* 1k queues, bits/10240ns
             * tIncomingRateConversionParameter=.625
             * tOutgoingRateConversionParameter=1.6
             */
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_MANTISSA,    uData,  320);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, uData,  9);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN,        uData,  1);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_MANTISSA,    uData,  409);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, uData,  8);
            uData = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN,        uData,  1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QS_RATE_CONVERSION, uData);
        }
    }

    /* seed the random number generator memories */
    for (uRandArrayIndex=0; uRandArrayIndex<55; uRandArrayIndex=uRandArrayIndex+1) {

        /* Seed the random # generator for the queue selection */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_RAND_ACC_DATA, DATA, uData, random_array[uRandArrayIndex]);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_RAND_ACC_DATA, uData);
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_RAND_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RAND_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, QS_RAND_ACC_CTRL, ADDR, uData, uRandArrayIndex);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_RAND_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_QS_RAND_ACC_CTRL_OFFSET,
                           SAND_HAL_KA_QS_RAND_ACC_CTRL_ACK_MASK,
                            QS_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: Write QS random number generator memory timed out index: 0x%x\n", uRandArrayIndex));
            return HW_QE2000_STS_INIT_QOS_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_RAND_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_RAND_ACC_CTRL, uData);

        /* Seed the random # generator for the LNA */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_RAND_ACC_DATA, DATA, uData, random_array[uRandArrayIndex]);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_LNA_RAND_ACC_DATA, uData);
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_RAND_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_RAND_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ADDR, uData, uRandArrayIndex);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_LNA_RAND_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_QS_LNA_RAND_ACC_CTRL_OFFSET,
                           0x80000000,
                            QS_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: Write QS LNA random number generator memory timed out index: 0x%x\n", uRandArrayIndex));
            return HW_QE2000_STS_INIT_QOS_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_LNA_RAND_ACC_CTRL, uData);

    }

    /* delay dequeue for hybrid mode */
    if (bHybrid) {
	uData=SAND_HAL_READ(userDeviceHandle, KA, QS_CONFIG4);
        uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG4, DELAY_DEQ_LOCAL, uData, 0xfc);
	SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG4, uData);
    }

    /* Kick off the auto init */
    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_QS_CONFIG0_OFFSET,
                         SAND_HAL_KA_QS_CONFIG0_INIT_MASK,
                         SAND_HAL_KA_QS_CONFIG0_INIT_DONE_MASK,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: QS auto init failed\n"));
        return HW_QE2000_STS_INIT_QOS_AUTOINIT_TIMEOUT_ERR_K;

    }

    /* For hybrid mode, we need to set the TME bit AFTER we init the memory,
     * because otherwise ALL queues get marked local by default. */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, TME_MODE, uData,
			       (bStandAlone || bHybrid));
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, MC_PORT_ID, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, QS_LNA_CONFIG, NODE_ID, uData, uNodeId);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_LNA_CONFIG, uData);

    /* Overwrite Q2EC */
    for (nQueue=0; nQueue<HW_QE2000_MAX_QUEUES; nQueue++) {
        uint32 uStatus;

        /* what we want for the value is mc=0 (bit 16) and node >31  */
        uStatus = hwQe2000QsMemWrite(userDeviceHandle, nQueue, 0x4, 0x0FF00);

        if( 0 != uStatus )
        {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: Q2EC write timed out queue 0x%x\n", nQueue));
            return( -1 );
        }

    }

   /* Overwrite E2Q */
    for (nEgress=0; nEgress<HW_QE2000_MAX_EGRESS; nEgress++) {
        uint32 uStatus;

        /* what we want for the value is for bit 14 (bEnable) to be 0x0 */
        uStatus = hwQe2000QsMemWrite(userDeviceHandle, nEgress, 0xE, 0xFFF);

        if( 0 != uStatus )
        {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: Clear E2Q entry failed egress 0x%x\n", nEgress));
            return( -1 );
        }
    }

    {
        int32 nEset;
        int32 nMc=1;
        uint32 uStatus;

        for ( nEset=0; nEset<128; nEset++ ) {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Writing 0xFFF to E2Q for eset(%d)...",nEset));
            uStatus = hwQe2000QsMemWrite(userDeviceHandle, nEset | (nMc << 12), 0xE, 0xFFF);

            if( 0 != uStatus )
            {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: Clear E2Q entry failed egress 0x%x\n", nEset | (nMc << 12)));
                return( -1 );
            }
        }
    }


    { /* Setup the number of queues per shaper */
        int32 nTsSel;

        switch ( nQueuesPerShaperIngress ) {
            case 1:
                nTsSel = 0;
                bMultiQueuePerShaper = FALSE;
                break;
            case 4:
                nTsSel = 1;
                bMultiQueuePerShaper = TRUE;
                break;
            case 8:
                nTsSel = 2;
                bMultiQueuePerShaper = TRUE;
                break;
            case 16:
                nTsSel = 3;
                bMultiQueuePerShaper = TRUE;
                break;
            default:
                QE2000PRINTF(QE2000_ERROR, ("ERROR: invalid parameter nQueuesPerShaperIngress set to 0x%x\n",
                             nQueuesPerShaperIngress));

                return HW_QE2000_STS_INVALID_QUEUES_PER_SHAPER_INGRESS;
        }
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, QS_CONFIG0, TS_SEL, nTsSel);
    }

    if ( 1 ) { /* always */
        /* rewrite the PRI lookup table based on spec chunk below: */
        /****************************************************************************************
           NOTE: In a scenerio where the QOS is configured to have multiple queues sharing
           the same shaper, having priorities calculcated according to the pseudocode above
           can have an adverse affect. If one of the queues associated with the shaper is
           serviced and then causes the shaper to saturate (go into the shaped state), the
           pseudocode dictates that the serviced queue has its priority set to NOPRI. A
           leak event may then occur for some other queue which causes the shared shaper to
           go under threshold (clearing the shaped state). In this case, the original queue
           will still maintain its priority as NOPRI.

           To fix this, it is recommended that the Priority Lookup Table is configured
           differently. In this case, the ?shaped? bit (used in the first conditional in
           each piece of pseudocode above) should be ignored. To achieve this, the Priority
           Lookup Table should be reconfigured by software after initialization.
        ****************************************************************************************/

        if (spMode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            hwQe2000InitQsReconfigurePriLutSetup(userDeviceHandle);
            uData = SAND_HAL_READ(userDeviceHandle, KA, QS_CONFIG5);
            uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG5, PRIORITY_EF, uData, 0x4000);
            SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG5, uData);
        }
        else {
            uData = SAND_HAL_READ(userDeviceHandle, KA, QS_CONFIG5);
            uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG5, PRIORITY_EF, uData, 0x6000);
            SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG5, uData);
        }

        /* Need to match the QE2000 and Sirius priorities */
        if (SOC_SBX_CFG(((int)userDeviceHandle))->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
            hwQe2000InitQsReconfigurePriLutVportMixSetup(userDeviceHandle);
        }

        hwQe2000InitQsReconfigurePriLut(userDeviceHandle, bMultiQueuePerShaper, bMixHighAndLowRateFlows );
    }

    /* if in C-mode, set the number of queues to 1024, not the 16k default */
    if (!bDmode) {
        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, QS_DEBUG3, LAST_Q_NUMBER, uData, 1023);
        SAND_HAL_WRITE(userDeviceHandle, KA, QS_DEBUG3, uData);
    }



    /* Initialize the Age Threshold Lookup Table - bug 23549             */
    /*********************************************************************/
    /* There are 32 entries laid out as follows:                         */
    /*                                                                   */
    /*   (32 entries)                                                    */
    /*                                                                   */
    /*   15 14 13 12 11 10  9  8   7  6  5  4  3  2  1  0                */
    /*   Anemic Threshold[7:0]   | EF Threshold[7:0]                     */
    /*                                                                   */
    /* the Age Threshold key table is used to determine the 5 bit age    */
    /* threshold key. The Age theshold lookup table is indexed by the    */
    /* queue number.                                                     */
    /*********************************************************************/
    {
        /* Overwrite Age Threshold key table */
        for (nQueue=0; nQueue<HW_QE2000_MAX_QUEUES; nQueue++) {
            uint32 uStatus;

            /* Set all entries to point to age threshold 0 */
            uStatus = hwQe2000QsMemWrite(userDeviceHandle, nQueue, 0xA, 0x0000);

            if( 0 != uStatus )
            {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: Age threshold key write timed out queue 0x%x\n", nQueue));
                return( -1 );
            }
        }

        /* Now, set the age threshold for each key as follows */
        /* key           anemic thresh      EF thresh         */
        /* 0  EF          0x04  (32us)      0xff              */
        /* 1              0x08  (64us)      0xff              */
        /* 2  SP7         0x0C  (96us)      0xff              */
        /* 3              0x10 (128us)      0xff              */
        /* 4  SP6         0x14 (160us)      0xff              */
        /* 5              0x18 (192us)      0xff              */
        /* 6  other       0x1D (240us)      0xff              */
        /* 7              0x20 (256us)      0xff              */
        /* 8              0x24 (288us)      0xff              */
        /* 9              0x28 (320us)      0xff              */
        /* 10             0x2C (352us)      0xff              */
        /* 11             0x30 (384us)      0xff              */
        /* 12             0x34 (416us)      0xff              */
        /* 13             0x38 (448us)      0xff              */
        /* 14             0x3C (480us)      0xff              */
        /* 10             0x40 (512us)      0xff              */

        for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
            if (SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos] != -1) {
                rv = soc_qe2000_template_max_age_adjust((int32)userDeviceHandle,
                                                        SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos],
                                                        &hwValue);
                if (SOC_E_NONE != rv) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("COS %2d max age %4d invalid\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos]));
                    return rv;
                }
                rv = soc_qe2000_template_max_age_recall((int32)userDeviceHandle,
                                                        hwValue,
                                                        &actValue);
                if (BCM_E_NONE != rv) {
                    actValue = -1;
                }

                rv = soc_sbx_connect_max_age_alloc((int32)userDeviceHandle,
                                                   SOC_SBX_CONN_FIXED,
                                                   hwValue,
                                                   &is_allocated,
                                                   &template);
                if (rv != SOC_E_NONE) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("Unable to reserve template for COS %2d"
                                  " max age %4d\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos]));
                    return(rv);
                }
                
                QE2000PRINTF(QE2000_DEBUG_FUNC, 
                             ("COS %2d general max age set %4d -> template %2d,"
                             " hw %02X, actual %4d\n",
                             cos,
                             SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos],
                             template,
                             hwValue,
                             actValue));

                if (is_allocated) {
                    rv = soc_qe2000_template_max_age_set((int32)userDeviceHandle,
                                                         template,
                                                         hwValue);
                    if (rv != SOC_E_NONE) {
                        QE2000PRINTF(QE2000_ERROR,
                                     ("Unable to configure template %d for"
                                      " COS %2d max age %4d\n",
                                      template,
                                      cos,
                                      SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time[cos]));
                        return(rv);
                    }
                }

                SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time_template[cos] = template;
            } else {
                
                SOC_SBX_CFG((int32)userDeviceHandle)->connect_max_age_time_template[cos] = -1;
            }
        }

#if 0
        {
            /*                                      0        1      2        3       4       5       6      7  */
            int32 nAnemicAgeThresholdTable[16] = {0x04ff, 0x08ff, 0x0cff, 0x10ff, 0x14ff, 0x18ff, 0x1dff, 0x20ff, /* anemic age and ef threshold */
                                                    0x24ff, 0x28ff, 0x2cff, 0x30ff, 0x34ff, 0x38ff, 0x3cff, 0x40ff}; /* anemic age only */

            /* Overwrite Age Threshold key table */
            int32 nAnemicAgeThresholdKey;
            uint32 uStatus;
            uint32 nAnemicAgeThreshold;

            for (nAnemicAgeThresholdKey = 0; nAnemicAgeThresholdKey < 15; nAnemicAgeThresholdKey++) {

                nAnemicAgeThreshold = nAnemicAgeThresholdTable[nAnemicAgeThresholdKey];

                uStatus = hwQe2000QsMemWrite(userDeviceHandle, nAnemicAgeThresholdKey, 0xB, nAnemicAgeThreshold);

            if( 0 != uStatus )
            {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: Age threshold LUT write timed out key 0x%x\n", nAnemicAgeThresholdKey));
                return( -1 );
            }
            }
        }
#endif /* 0 */
    }


    /* enable the QOS (i.e. bring it online) */
    uData=SAND_HAL_READ(userDeviceHandle, KA, QS_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(KA, QS_CONFIG0, ENABLE, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, QS_CONFIG0, uData);

    return HW_QE2000_STS_OK_K;
}


static uint32
hwQe2000InitQsReconfigurePriLutSetup(sbhandle userDeviceHandle){
  int32 nData0 = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
  int32 nData1 = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
  /* move hungry to below global strict pri */
  /* move strict pri up one */
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, SATISFIED, nData0, 3); 
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP0, nData1, 4);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP1, nData1, 5);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP2, nData1, 6);  
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP3, nData1, 7);
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, HUNGRY, nData0, 8); 
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP4, nData1, 8+1);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP5, nData1, 9+1);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP6, nData1, 10+1);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP7, nData1, 11+1);
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, HOLD, nData0, 12+1); 
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, EF, nData0, 13+1); 

  SAND_HAL_WRITE(userDeviceHandle, KA, QS_PRIORITY0, nData0);
  SAND_HAL_WRITE(userDeviceHandle, KA, QS_PRIORITY1, nData1);

  return 0;
}

static uint32
hwQe2000InitQsReconfigurePriLutVportMixSetup(sbhandle userDeviceHandle){
  int32_t nData0 = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
  int32_t nData1 = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
  /* move hungry to below global strict pri */
  /* move strict pri up one */
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, SATISFIED, nData0, 2); 
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, HUNGRY, nData0, 3); 
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP0, nData1, 3);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP1, nData1, 4);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP2, nData1, 5);  
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP3, nData1, 6);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP4, nData1, 7);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP5, nData1, 8);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP6, nData1, 9);
  nData1 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY1, SP7, nData1, 10);
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, HOLD, nData0, 11); 
  nData0 = SAND_HAL_MOD_FIELD(KA, QS_PRIORITY0, EF, nData0, 12); 

  SAND_HAL_WRITE(userDeviceHandle, KA, QS_PRIORITY0, nData0);
  SAND_HAL_WRITE(userDeviceHandle, KA, QS_PRIORITY1, nData1);

  return 0;
}

uint32 
hwQe2000InitQsReconfigurePriLut(sbhandle userDeviceHandle,
                                sbBool_t bMultiQueuePerShaper,
                                sbBool_t bMixHighAndLowRateFlows) {
    int32 nIndex;
    uint32 uStatus;
    sbZfHwQe2000QsPriLutEntry_t zfPriLutEntry;
    sbZfHwQe2000QsPriLutAddr_t  zfPriLutAddr;
    /* Get the current priorities */
    int32 nData      = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
    /* 22314: Must #undef MAXPRI for NetBSD build in kernel or we get the following error */
    /* 22314: qe2000_init.c:1793: `SAND_HAL_KA_QS_PRIORITY0_127_MASK' undeclared (first use in this function)*/
    int32 nMaxPri    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, MAXPRI, nData); /* 22314: for netBSD kernel need to undef MAXPRI for this line */
    int32 nSuperEf   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SUPER_EF, nData);
    int32 nEf        = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, EF, nData);
    int32 nHoldPri   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HOLD, nData);
    int32 nHungry    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HUNGRY, nData);
    int32 nSatisfied = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SATISFIED, nData);
    int32 nAnemic    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, ANEMIC, nData);
    int32 nNoPri     = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, NOPRI, nData);
    int32 nData1     = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
    int32 nSP0       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP0, nData1);
    int32 nSP1       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP1, nData1);
    int32 nSP2       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP2, nData1);
    int32 nSP3       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP3, nData1);
    int32 nSP4       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP4, nData1);
    int32 nSP5       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP5, nData1);
    int32 nSP6       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP6, nData1);
    int32 nSP7       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP7, nData1);
    sbBool_t bFixExactMatch=TRUE;
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("Reconfiguring QS Priority Lookup Table with latest version\n"));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMultiQueuePerShaper=%d\n", bMultiQueuePerShaper));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMixHighAndLowRateFlows=%d\n", bMixHighAndLowRateFlows));

    {
        uint32 nDepthThreshold;
        uint32 uData = 0;

        sbZfHwQe2000QsPriLutEntry_InitInstance(&zfPriLutEntry);
        for( nIndex = 0; nIndex < HW_QE2000_QS_MEM_PRI_LUT_TABLE_SIZE; nIndex++ ) {
            sbZfHwQe2000QsPriLutAddr_Unpack(&zfPriLutAddr, (uint8*)&nIndex, 4); /* translate address into individual key components */

            /* Priority */
            if ( zfPriLutAddr.m_nDepth == 0 ) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_nDepth == 1 && !zfPriLutAddr.m_bAnemicAged ) {
                zfPriLutEntry.m_nCPri = nAnemic;
            } else if ( zfPriLutAddr.m_nQType == 2 ) {
                if ( zfPriLutAddr.m_bEfAged ) {
                    zfPriLutEntry.m_nCPri = nSuperEf;
                } else {
                    zfPriLutEntry.m_nCPri = nEf;
                }
            } else if ( zfPriLutAddr.m_nQType == 15 ) {
                zfPriLutEntry.m_nCPri = nSP7;
            } else if ( zfPriLutAddr.m_nQType == 14 ) {
                zfPriLutEntry.m_nCPri = nSP6;
            } else if ( zfPriLutAddr.m_nQType == 13 ) {
                zfPriLutEntry.m_nCPri = nSP5;
            } else if ( zfPriLutAddr.m_nQType == 12 ) {
                zfPriLutEntry.m_nCPri = nSP4;
            } else if ( zfPriLutAddr.m_nQType == 11 ) {
                zfPriLutEntry.m_nCPri = nSP3;
            } else if ( zfPriLutAddr.m_nQType == 10 ) {
                zfPriLutEntry.m_nCPri = nSP2;
            } else if ( zfPriLutAddr.m_nQType ==  9 ) {
                zfPriLutEntry.m_nCPri = nSP1;
            } else if ( zfPriLutAddr.m_nQType ==  8 ) {
                zfPriLutEntry.m_nCPri = nSP0;
	    } else if ( zfPriLutAddr.m_nQType == 0 ) {
		/* map BE to priority 2, which is lower than Satisfied */
                zfPriLutEntry.m_nCPri = 2;
            } else {
                if ( zfPriLutAddr.m_nCreditLevel > 0 ) {
                    zfPriLutEntry.m_nCPri = nHungry;
                } else if ( bMixHighAndLowRateFlows) {
                    if ( zfPriLutAddr.m_nDepth > 6 ) {
                        zfPriLutEntry.m_nCPri = nSP1;
                    } else if ( zfPriLutAddr.m_nDepth > 4 ) {
                        zfPriLutEntry.m_nCPri = nSP0;
                    } else {
                        zfPriLutEntry.m_nCPri = nSatisfied;
                    }
                } else {
                    zfPriLutEntry.m_nCPri = nSatisfied;
                }
            }

            /* Next Priority */
            /* coverity[dead_error_line] */
            nDepthThreshold = bFixExactMatch ? ((zfPriLutEntry.m_nCPri == (UINT)nSatisfied) ? 2 : 3) : 3;

            if ( zfPriLutAddr.m_nDepth < nDepthThreshold ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_nPktLen > 0 ) {
                zfPriLutEntry.m_nNPri = nMaxPri;
            } else if ( zfPriLutAddr.m_bHoldTs && (zfPriLutAddr.m_nDepth > 6)) {
                zfPriLutEntry.m_nNPri = nHoldPri;
            } else {
                zfPriLutEntry.m_nNPri = zfPriLutEntry.m_nCPri;
            }

            /* Unpack */
            sbZfHwQe2000QsPriLutEntry_Pack(&zfPriLutEntry, (uint8*)&uData, 4);

            /* Write */
            uStatus = soc_qe2000_qs_mem_write((int)userDeviceHandle, 
                                              0xC /* PRI LUT */, nIndex, 
                                              uData);
            if( 0 != uStatus ) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: qs_memWrite failed\n"));
                return( -1 );
            }
        }
    }

    return HW_QE2000_STS_OK_K;
}

/* AB501 090109  added nFillNumberOfHalfTimeslots parameter
   This parameter can be 0..7. Using this parameter, the PriLut code
   below will adjust priority based on queue depth, where nSatisfied is
   used up to the nFillNumberOfHalfTimeslots, and then the regular SP
   priority will be used when the queue backs up.
   This scheme is essentially what NEC had, however, there seems to be 
   other problems with NEC PriLut setup, so we simply took the SDK5.6.2GA
   PriLut, and augmented it to allow for nFillNumberOfHalfTimeslots.
   Setting nFillNumberOfHalfTimeslots=0 is identical to the PriLut from
   SDK 5.6.2GA
   For 5 serdes operation, 1x10G LC, use hold_ts=7 and nFillNumberOfHalfTimeslots=7
   For 5 serdes operation, 8x1G LC, use hold_ts=7 and nFillNumberOfHalfTimeslots=0
     NOTE: for 1G LC, you can use nFillNumberOfHalfTimeslots=7, but you will see
     some higher latency on the lowest priority flow.
     NOTE: This function can be called anytime (after init soc/init bcm)
*/

uint32
hwQe2000InitQsReconfigurePriLutEx(sbhandle userDeviceHandle,
                                sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots) {
    int32 nIndex;
    uint32 uStatus;
    sbZfHwQe2000QsPriLutEntry_t zfPriLutEntry;
    sbZfHwQe2000QsPriLutAddr_t  zfPriLutAddr;
    /* Get the current priorities */
    int32 nData      = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
    /* 22314: Must #undef MAXPRI for NetBSD build in kernel or we get the following error */
    /* 22314: qe2000_init.c:1793: `SAND_HAL_KA_QS_PRIORITY0_127_MASK' undeclared (first use in this function)*/
    int32 nMaxPri    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, MAXPRI, nData); /* 22314: for netBSD kernel need to undef MAXPRI for this line */
    int32 nSuperEf   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SUPER_EF, nData);
    int32 nEf        = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, EF, nData);
    int32 nHoldPri   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HOLD, nData);
    int32 nHungry    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HUNGRY, nData);
    int32 nSatisfied = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SATISFIED, nData);
    int32 nAnemic    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, ANEMIC, nData);
    int32 nNoPri     = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, NOPRI, nData);
    int32 nData1     = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
    int32 nSP0       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP0, nData1);
    int32 nSP1       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP1, nData1);
    int32 nSP2       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP2, nData1);
    int32 nSP3       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP3, nData1);
    int32 nSP4       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP4, nData1);
    int32 nSP5       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP5, nData1);
    int32 nSP6       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP6, nData1);
    int32 nSP7       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP7, nData1);
    sbBool_t bFixExactMatch=TRUE;
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("Reconfiguring QS Priority Lookup Table with latest version\n"));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMultiQueuePerShaper=%d\n", bMultiQueuePerShaper));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMixHighAndLowRateFlows=%d\n", bMixHighAndLowRateFlows));

    {
        uint32 nDepthThreshold;
        uint32 uData;

        sbZfHwQe2000QsPriLutEntry_InitInstance(&zfPriLutEntry);
        for( nIndex = 0; nIndex < HW_QE2000_QS_MEM_PRI_LUT_TABLE_SIZE; nIndex++ ) {
            sbZfHwQe2000QsPriLutAddr_Unpack(&zfPriLutAddr, (uint8*)&nIndex, 4); /* translate address into individual key components */

            /* Priority */
            if ( zfPriLutAddr.m_nDepth == 0 ) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_nDepth == 1 && !zfPriLutAddr.m_bAnemicAged ) {
                zfPriLutEntry.m_nCPri = nAnemic;
            } else if ( zfPriLutAddr.m_nQType == 2 ) {
                if ( zfPriLutAddr.m_bEfAged ) {
                    zfPriLutEntry.m_nCPri = nSuperEf;
                } else {
                    zfPriLutEntry.m_nCPri = nEf;
                }
            } else if ( zfPriLutAddr.m_nQType == 15 ) {
                zfPriLutEntry.m_nCPri = 
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP7:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 14 ) {
                zfPriLutEntry.m_nCPri =  
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP6:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 13 ) {
                zfPriLutEntry.m_nCPri =  
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP5:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 12 ) {
                zfPriLutEntry.m_nCPri =  
		  (  zfPriLutAddr.m_bAnemicAged  || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP4:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 11 ) {
                zfPriLutEntry.m_nCPri =  
		  (  zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP3:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 10 ) {
                zfPriLutEntry.m_nCPri =  
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP2:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  9 ) {
                zfPriLutEntry.m_nCPri =  
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP1:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  8 ) {
	        zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_bAnemicAged || zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP0:nSatisfied;
	    } else if ( zfPriLutAddr.m_nQType == 0 ) {
		/* map BE to priority 2, which is lower than Satisfied */
                zfPriLutEntry.m_nCPri = 2;
            } else {
                if ( zfPriLutAddr.m_nCreditLevel > 0 ) {
                    zfPriLutEntry.m_nCPri = nHungry;
                } else if ( bMixHighAndLowRateFlows) {
                    if ( zfPriLutAddr.m_nDepth > 6 ) {
                        zfPriLutEntry.m_nCPri = nSP1;
                    } else if ( zfPriLutAddr.m_nDepth > 4 ) {
                        zfPriLutEntry.m_nCPri = nSP0;
                    } else {
                        zfPriLutEntry.m_nCPri = nSatisfied;
                    }
                } else {
                    zfPriLutEntry.m_nCPri = nSatisfied;
                }
            }

            /* Next Priority */
            /* coverity[dead_error_line] */
            nDepthThreshold = bFixExactMatch ? ((zfPriLutEntry.m_nCPri == (UINT)nSatisfied) ? 2 : 3) : 3;

            if ( zfPriLutAddr.m_nDepth < nDepthThreshold ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_nPktLen > 0 ) {
                zfPriLutEntry.m_nNPri = nMaxPri;
            } else if ( zfPriLutAddr.m_bHoldTs ) {
                zfPriLutEntry.m_nNPri = nHoldPri;
            } else {
                zfPriLutEntry.m_nNPri = zfPriLutEntry.m_nCPri;
            }

            /* Unpack */
            sbZfHwQe2000QsPriLutEntry_Pack(&zfPriLutEntry, (uint8*)&uData, 4);

            /* Write */
            uStatus = soc_qe2000_qs_mem_write((int)userDeviceHandle, 
                                              0xC /* PRI LUT */, nIndex, 
                                              uData);
            if( 0 != uStatus ) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: qs_memWrite failed\n"));
                return( -1 );
            }
        }
    }

    return HW_QE2000_STS_OK_K;
}

uint32
hwQe2000InitQsReconfigurePriLutEx2(sbhandle userDeviceHandle,
                                sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots) {
    int32 nIndex;
    uint32 uStatus;
    sbZfHwQe2000QsPriLutEntry_t zfPriLutEntry;
    sbZfHwQe2000QsPriLutAddr_t  zfPriLutAddr;
    /* Get the current priorities */
    int32 nData      = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
    /* 22314: Must #undef MAXPRI for NetBSD build in kernel or we get the following error */
    /* 22314: qe2000_init.c:1793: `SAND_HAL_KA_QS_PRIORITY0_127_MASK' undeclared (first use in this function)*/
    int32 nMaxPri    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, MAXPRI, nData); /* 22314: for netBSD kernel need to undef MAXPRI for this line */
    int32 nSuperEf   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SUPER_EF, nData);
    int32 nEf        = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, EF, nData);
    int32 nHoldPri   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HOLD, nData);
    int32 nHungry    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HUNGRY, nData);
    int32 nSatisfied = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SATISFIED, nData);
    int32 nAnemic    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, ANEMIC, nData);
    int32 nNoPri     = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, NOPRI, nData);
    int32 nData1     = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
    int32 nSP0       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP0, nData1);
    int32 nSP1       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP1, nData1);
    int32 nSP2       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP2, nData1);
    int32 nSP3       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP3, nData1);
    int32 nSP4       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP4, nData1);
    int32 nSP5       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP5, nData1);
    int32 nSP6       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP6, nData1);
    int32 nSP7       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP7, nData1);
    sbBool_t bFixExactMatch=TRUE;
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("Reconfiguring QS Priority Lookup Table with latest version\n"));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMultiQueuePerShaper=%d\n", bMultiQueuePerShaper));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMixHighAndLowRateFlows=%d\n", bMixHighAndLowRateFlows));

    {
        uint32 nDepthThreshold;
        uint32 uData;

        sbZfHwQe2000QsPriLutEntry_InitInstance(&zfPriLutEntry);
        for( nIndex = 0; nIndex < HW_QE2000_QS_MEM_PRI_LUT_TABLE_SIZE; nIndex++ ) {
            sbZfHwQe2000QsPriLutAddr_Unpack(&zfPriLutAddr, (uint8*)&nIndex, 4); /* translate address into individual key components */

            /* Priority */
            if ( zfPriLutAddr.m_nDepth == 0 ) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_nDepth == 1 && !zfPriLutAddr.m_bAnemicAged ) {
                zfPriLutEntry.m_nCPri = nAnemic;
            } else if ( zfPriLutAddr.m_nQType == 2 ) {
                if ( zfPriLutAddr.m_bEfAged ) {
                    zfPriLutEntry.m_nCPri = nSuperEf;
                } else {
                    zfPriLutEntry.m_nCPri = nEf;
                }
            } else if ( zfPriLutAddr.m_nQType == 15 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP7:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 14 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP6:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 13 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP5:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 12 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP4:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 11 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP3:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 10 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP2:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  9 ) {
                zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP1:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  8 ) {
	        zfPriLutEntry.m_nCPri =
		  ( zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)?nSP0:nSatisfied;
	    } else if ( zfPriLutAddr.m_nQType == 0 ) {
		/* map BE to priority 2, which is lower than Satisfied */
                zfPriLutEntry.m_nCPri = 2;
            } else {
                if ( zfPriLutAddr.m_nCreditLevel > 0 ) {
                    zfPriLutEntry.m_nCPri = nHungry;
                } else if ( bMixHighAndLowRateFlows) {
                    if ( zfPriLutAddr.m_nDepth > 6 ) {
                        zfPriLutEntry.m_nCPri = nSP1;
                    } else if ( zfPriLutAddr.m_nDepth > 4 ) {
                        zfPriLutEntry.m_nCPri = nSP0;
                    } else {
                        zfPriLutEntry.m_nCPri = nSatisfied;
                    }
                } else {
                    zfPriLutEntry.m_nCPri = nSatisfied;
                }
            }

            /* Next Priority */
            /* coverity[dead_error_line] */
            nDepthThreshold = bFixExactMatch ? ((zfPriLutEntry.m_nCPri == (UINT)nSatisfied) ? 2 : 3) : 3;

            if ( zfPriLutAddr.m_nDepth < nDepthThreshold ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_nPktLen > 0 ) {
                zfPriLutEntry.m_nNPri = nMaxPri;
	    } else if ( zfPriLutAddr.m_nDepth <= 3 ) {
                zfPriLutEntry.m_nNPri = nAnemic;
	    } else if ( zfPriLutAddr.m_nDepth < nFillNumberOfHalfTimeslots ) {
                zfPriLutEntry.m_nNPri = nSatisfied;
	    } else if ( zfPriLutAddr.m_bHoldTs ) {
                zfPriLutEntry.m_nNPri = nHoldPri;
            } else {
                zfPriLutEntry.m_nNPri = zfPriLutEntry.m_nCPri;
            }

            /* Unpack */
            sbZfHwQe2000QsPriLutEntry_Pack(&zfPriLutEntry, (uint8*)&uData, 4);

            /* Write */
            uStatus = soc_qe2000_qs_mem_write((int)userDeviceHandle,
                                              0xC /* PRI LUT */, nIndex,
                                              uData);
            if( 0 != uStatus ) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: qs_memWrite failed\n"));
                return( -1 );
            }
        }
    }

    return HW_QE2000_STS_OK_K;
}

uint32
hwQe2000InitQsReconfigurePriLutEx3(sbhandle userDeviceHandle,
                                sbBool_t bMultiQueuePerShaper,
				  sbBool_t bMixHighAndLowRateFlows,
				  int nFillNumberOfHalfTimeslots) {
    int32 nIndex;
    uint32 uStatus;
    sbZfHwQe2000QsPriLutEntry_t zfPriLutEntry;
    sbZfHwQe2000QsPriLutAddr_t  zfPriLutAddr;
    /* Get the current priorities */
    int32 nData      = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY0);
    /* 22314: Must #undef MAXPRI for NetBSD build in kernel or we get the following error */
    /* 22314: qe2000_init.c:1793: `SAND_HAL_KA_QS_PRIORITY0_127_MASK' undeclared (first use in this function)*/
    int32 nMaxPri    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, MAXPRI, nData); /* 22314: for netBSD kernel need to undef MAXPRI for this line */
    int32 nSuperEf   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SUPER_EF, nData);
    int32 nEf        = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, EF, nData);
    int32 nHoldPri   = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HOLD, nData);
    int32 nHungry    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, HUNGRY, nData);
    int32 nSatisfied = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, SATISFIED, nData);
    int32 nAnemic    = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, ANEMIC, nData);
    int32 nNoPri     = SAND_HAL_GET_FIELD(KA, QS_PRIORITY0, NOPRI, nData);
    int32 nData1     = SAND_HAL_READ(userDeviceHandle, KA, QS_PRIORITY1);
    int32 nSP0       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP0, nData1);
    int32 nSP1       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP1, nData1);
    int32 nSP2       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP2, nData1);
    int32 nSP3       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP3, nData1);
    int32 nSP4       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP4, nData1);
    int32 nSP5       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP5, nData1);
    int32 nSP6       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP6, nData1);
    int32 nSP7       = SAND_HAL_GET_FIELD(KA, QS_PRIORITY1, SP7, nData1);
    sbBool_t bFixExactMatch=TRUE;
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("Reconfiguring QS Priority Lookup Table with latest version\n"));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMultiQueuePerShaper=%d\n", bMultiQueuePerShaper));
    QE2000PRINTF(QE2000_DEBUG_FUNC, ("bMixHighAndLowRateFlows=%d\n", bMixHighAndLowRateFlows));

    {
        uint32 nDepthThreshold;
        uint32 uData;

        sbZfHwQe2000QsPriLutEntry_InitInstance(&zfPriLutEntry);
        for( nIndex = 0; nIndex < HW_QE2000_QS_MEM_PRI_LUT_TABLE_SIZE; nIndex++ ) {
            sbZfHwQe2000QsPriLutAddr_Unpack(&zfPriLutAddr, (uint8*)&nIndex, 4); /* translate address into individual key components */

            /* Priority */
            if ( zfPriLutAddr.m_nDepth == 0 ) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper) {
                zfPriLutEntry.m_nCPri = nNoPri;
            } else if ( zfPriLutAddr.m_nDepth == 1 && !zfPriLutAddr.m_bAnemicAged ) {
                zfPriLutEntry.m_nCPri = nAnemic;
            } else if ( zfPriLutAddr.m_nQType == 2 ) {
                if ( zfPriLutAddr.m_bEfAged ) {
                    zfPriLutEntry.m_nCPri = nSuperEf;
                } else {
                    zfPriLutEntry.m_nCPri = nEf;
                }
            } else if ( zfPriLutAddr.m_nQType == 15 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP7:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 14 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP6:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 13 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP5:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 12 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP4:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 11 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP3:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType == 10 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP2:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  9 ) {
                zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP1:nSatisfied;
            } else if ( zfPriLutAddr.m_nQType ==  8 ) {
	        zfPriLutEntry.m_nCPri =
		  ( (zfPriLutAddr.m_nDepth>=nFillNumberOfHalfTimeslots)||zfPriLutAddr.m_bAnemicAged)?nSP0:nSatisfied;
	    } else if ( zfPriLutAddr.m_nQType == 0 ) {
		/* map BE to priority 2, which is lower than Satisfied */
                zfPriLutEntry.m_nCPri = 2;
            } else {
                if ( zfPriLutAddr.m_nCreditLevel > 0 ) {
                    zfPriLutEntry.m_nCPri = nHungry;
                } else if ( bMixHighAndLowRateFlows) {
                    if ( zfPriLutAddr.m_nDepth > 6 ) {
                        zfPriLutEntry.m_nCPri = nSP1;
                    } else if ( zfPriLutAddr.m_nDepth > 4 ) {
                        zfPriLutEntry.m_nCPri = nSP0;
                    } else {
                        zfPriLutEntry.m_nCPri = nSatisfied;
                    }
                } else {
                    zfPriLutEntry.m_nCPri = nSatisfied;
                }
            }

            /* Next Priority */
            /* coverity[dead_error_line] */
            nDepthThreshold = bFixExactMatch ? ((zfPriLutEntry.m_nCPri == (UINT)nSatisfied) ? 2 : 3) : 3;

            if ( zfPriLutAddr.m_nDepth < nDepthThreshold ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_bShaped && !bMultiQueuePerShaper ) {
                zfPriLutEntry.m_nNPri = nNoPri;
            } else if ( zfPriLutAddr.m_nPktLen > 0 ) {
                zfPriLutEntry.m_nNPri = nMaxPri;
	    } else if ( zfPriLutAddr.m_nDepth <= 3 ) {
                zfPriLutEntry.m_nNPri = nAnemic;
	    } else if ( zfPriLutAddr.m_nDepth < nFillNumberOfHalfTimeslots ) {
                zfPriLutEntry.m_nNPri = nSatisfied;
	    } else if ( zfPriLutAddr.m_bHoldTs ) {
                zfPriLutEntry.m_nNPri = nHoldPri;
            } else {
                zfPriLutEntry.m_nNPri = zfPriLutEntry.m_nCPri;
            }

            /* Unpack */
            sbZfHwQe2000QsPriLutEntry_Pack(&zfPriLutEntry, (uint8*)&uData, 4);

            /* Write */
            uStatus = soc_qe2000_qs_mem_write((int)userDeviceHandle,
                                              0xC /* PRI LUT */, nIndex,
                                              uData);
            if( 0 != uStatus ) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: qs_memWrite failed\n"));
                return( -1 );
            }
        }
    }

    return HW_QE2000_STS_OK_K;
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitRb()
 *
 * OVERVIEW:        Initialize RB
 *
 * ARGUMENTS:
 *   Input parameters:
 *   TYPE            NAME           SET AT    DESCRIPTION
 *   ----            ----           ------    -----------
 *   uint32        uRbSpi0PortCount node      number of physical ports on spi0- (set to 1 in the fe1k case)
 *   sbBool_t        bFabCrc32      system    32/16 bit crc packet payload protection
 *
 *
 * RETURNS:
 *
 *
 * DESCRIPTION:
 *
 * Initialize the receive buffer block.
 *
 *   NOTES:
 *   1) Assumes the 2 SPI IDP's in interleave mode, with 8B headers +
 *      length and queue number (i.e. no classifier or length
 *      calculation stuff enabled). When the SPI itself is configured,
 *      certain registers in the RB may need to be reconfigured:
 *      RB_IDP2_CONFIG
 *      RB_IDP3_CONFIG
 *      RB_BASE_NOHDR2
 *      RB_BASE_NOHDR3
 *   2) None of the classifier functionality is enabled or configured
 *   3) The hdr adjust location is left at the default
 *****************************************************************************/

static uint32
hwQe2000InitRb(sbhandle userDeviceHandle,
               uint32 uRbSpi0PortCount,
               sbBool_t bFabCrc32,
               uint32 uPacketAdjustFormat)
{

#define POL_INIT_TIMEOUT     (4) /* seconds */

    uint32 uData;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    /* Select either 32-bit or 16-bit CRC */
    uData = SAND_HAL_READ(userDeviceHandle, KA, RB_CONFIG);
    uData = SAND_HAL_MOD_FIELD(KA, RB_CONFIG, SELECT_CRC32, uData, bFabCrc32);
    uData = SAND_HAL_MOD_FIELD(KA, RB_CONFIG, IDP2_STORE_AND_FWD, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, RB_CONFIG, IDP_ALT_LENADJ, uData, uPacketAdjustFormat);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_CONFIG, uData);

    /* configure the PCI and 2 MIRROR IDPs to SB_ASSERT backpressure at 11KB */
    uData = SAND_HAL_MOD_FIELD(KA, RB_IDP2_BP_CONFIG, MAXBURST2, uData, 0x40);
    uData = SAND_HAL_MOD_FIELD(KA, RB_IDP2_BP_CONFIG, MAXBURST1, uData, 0x40);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_IDP2_BP_CONFIG, uData);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_IDP3_BP_CONFIG, uData);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_IDP4_BP_CONFIG, uData);

    /* configure the SPI IDPs to SB_ASSERT HUNGRY when 8KB of space is available */
    /* and SATISFIED when 4KB of space is available */
    uData = SAND_HAL_MOD_FIELD(KA, RB_IDP2_BP_CONFIG, MAXBURST1, uData, 0x200); /* hungry thresh */
    uData = SAND_HAL_MOD_FIELD(KA, RB_IDP2_BP_CONFIG, MAXBURST2, uData, 0x100); /* satisfied thresh */
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_IDP0_BP_CONFIG, uData);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_IDP1_BP_CONFIG, uData);

    /* SPI0 port base */
    /* (Always set to port 0) */
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP0, PORT_BASE, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP0, PORT_MASK, uData, 0x3f);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP0, PORT_FORCE, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_PORTNUM_IDP0, uData);

    /* SPI1 port base */
    /* (SPI1 starts using ports where SPI0 left off) */
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP1, PORT_BASE, uData, uRbSpi0PortCount);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP1, PORT_MASK, uData, 0x3f);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP1, PORT_FORCE, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_PORTNUM_IDP1, uData);

    /* PCI port base */
    /* (PCI is always port 49 by default) */
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP2, PORT_BASE, uData, 0x31);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP2, PORT_MASK, uData, 0x3f);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP2, PORT_FORCE, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_PORTNUM_IDP2, uData);

    /* MIRROR0 port base */
    /* (MIRROR0 is always port 50 by default) */
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP3, PORT_BASE, uData, 0x32);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP3, PORT_MASK, uData, 0x3f);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP3, PORT_FORCE, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_PORTNUM_IDP3, uData);

    /* MIRROR1 port base */
    /* (MIRROR1 is always port 51 by default) */
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP4, PORT_BASE, uData, 0x33);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP4, PORT_MASK, uData, 0x3f);
    uData = SAND_HAL_MOD_FIELD(KA, RB_PORTNUM_IDP4, PORT_FORCE, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_PORTNUM_IDP4, uData);

    /* bug 24539, disable redundant memory */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_IDP0_ECC_DEBUG, DISABLE_REDUNDANCY, 3);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_IDP1_ECC_DEBUG, DISABLE_REDUNDANCY, 3);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_IDP2_ECC_DEBUG, DISABLE_REDUNDANCY, 3);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_IDP3_ECC_DEBUG, DISABLE_REDUNDANCY, 3);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_IDP4_ECC_DEBUG, DISABLE_REDUNDANCY, 3);

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_BIST_CONFIG0, BIST_SETUP, 2);
    uData = SAND_HAL_MOD_FIELD(KA, RB_BIST_CONFIG1, MBIST_EN, uData, 0x07ffffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, RB_BIST_CONFIG1, uData);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_RB_BIST_STATUS0_OFFSET,
                        0x07ffffff,
                        INTERNAL_MEMBIST_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: RB BIST timeout\n"));

        return HW_QE2000_STS_INIT_RB_INTERNAL_MEMBIST_TIMEOUT_ERR_K;
    }
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, RB_BIST_CONFIG1, MBIST_EN, 0);

    /* Take the RB block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, RB_CORE_RESET, 0);


    /* Kick off policer init (This does not turn on any policers, just inits the policer */
    /* tables). */
    if (hwQe2000InitDone(userDeviceHandle,
                        SAND_HAL_KA_RB_POL_CONFIG_OFFSET,
                        SAND_HAL_KA_RB_POL_CONFIG_POL_INIT_MASK,
                        SAND_HAL_KA_RB_POL_CONFIG_POL_INIT_DONE_MASK,
                         POL_INIT_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: RB policer init timeout\n"));
        return HW_QE2000_STS_INIT_RB_POL_INIT_TIMEOUT_ERR_K;
    }

  return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitQm()
 *
 * OVERVIEW:        Initialize the QE2000 PM Block
 *
 * ARGUMENTS:
 *     Input parameters:
 *     TYPE            NAME                 SET AT    DESCRIPTION
 *     ----            ----                 ------    -----------
 *     sbBool_t        bPmHalfBus           node      128/64 bit external DDR2 bus
 *     sbBool_t        bQm512MbDdr2         node      If true, external DDR2 parts are 512Mbit, else they are 256Mbit
 *     uint32        uEpochSizeInNs       system    epoch duration in NS
 *     uint32        uClockSpeedInMHz     system    clock speed in MHz
 *     sbBool_t        bDmode               system    dmode/cmode
 *     uint32        uQmMaxArrivalRateMbs node      maximum arrival line rate (in Mbps)
 *     sbBool_t        bStandAlone       SYSTEM    QE2K is operating in standalone switch mode
 *     sbBool_t        bHybrid           SYSTEM    QE2K is operating in hybrid mode
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:
 * Initialize the queue manager block.
 *

 *     Notes:
 *     1) karen thinks we should use qnum max for bandwidth allocation
 *     2) SPI port base #'s are not configured (these fields are used for per queue SPI backpressure)
 *        Also, satisfied/hungry SPI backpressure thresholds are not configured.
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitQm(sbhandle userDeviceHandle,
               sbBool_t bPmHalfBus,
               sbBool_t bQm512MbDdr2,
               uint32 uEpochSizeInNs,
               uint32 uClockSpeedInMHz,
               sbBool_t bDmode,
               uint32 uQmMaxArrivalRateMbs,
               sbBool_t bStandAlone, 
               sbBool_t bHybrid,
               uint32 hybridModeQueueDemarcation,
               int32  nGlobalShapingAdjustInBytes,
               int32  nLinesPerTimeslotCongested,
               int32  nLinesPerTimeslotQe1k,
               int32  nDemandScale)
{


#define QM_MEM_TIMEOUT (4)              /* 400 milliseconds */

    uint32 uData;
    uint32 uTotalBufsAvl;
    uint32 uPbUpperAddr;
    uint32 uPbLowerAddr;
    uint32 uFbUpperAddr;
    uint32 uFbLowerAddr;
    uint32 uRandArrayIndex;
    int rc;
    int hwValue, actValue;
    int cos, template, is_allocated;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));

    /* configure the number of 4k data buffers in the packet memory */
    if (bPmHalfBus) {
        if (bQm512MbDdr2) {
            uTotalBufsAvl=0x0fc00; /* half bus, 512Mbit parts: 64512 (63K) buffers */
        } else  {
            uTotalBufsAvl=0x01800; /* half bus, 256Mbit parts: 6144 (6K) buffers */
        }
    } else {
        if (bQm512MbDdr2) {
            uTotalBufsAvl=0x1fe00; /* full bus, 512Mbit parts: 130560 (127K) buffers */
        } else {
            uTotalBufsAvl=0x03000; /* full bus, 256Mbit parts: 12288 (12K) buffers */
        }
    }

    uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG0, TOTAL_BUFS_AVL, uData, uTotalBufsAvl);
    /* Disable aging for now */
    uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG0, HW_AGE_ENABLE, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG0, uData);

    uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG1);
    uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG1, RATE_SHIFT, uData, nDemandScale);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG1, uData);


    if (bQm512MbDdr2) {
        /* configure the external address space (packet buffer and free buffer tables) */
        uPbLowerAddr = 0; /* lower address of packet buffer always starts at 0 */
        uPbUpperAddr = uTotalBufsAvl-1; /* upper address is number of buffers available - 1 */
        uFbLowerAddr = uTotalBufsAvl; /* free buffer memory starts immediately after packet buffer */
        uFbUpperAddr = uTotalBufsAvl + (uTotalBufsAvl/1536) - 1; /* # of buffers in free buffer list is */
                                                                 /* number of free buffs available + number */
                                                                 /* of buffers required to store the FB list - 1 */
    }
    /* For 256Mb memories, there is an errata, 256Mbit memories are currently unsupported due to the fact */
    /* that there are holes in external Free Buffer list memory which the hardware does not account for   */
    /* The hardware treats this external memory as a FIFO bounded by the FB upper and lower addresses.    */
    /* Additionally, there was a bug in the allocation below.  The holes occur every 8 addresses in full  */
    /* bus mode or every 4 addresses in half-bus mode.  Double the address range in the buffer            */
    /* memory needed to be allocated.  When the hardware bug is fixed the following address range should  */
    /* work.                                                                                              */
    else {
        uPbLowerAddr = 0;
        uPbUpperAddr = (uTotalBufsAvl*2)-1; /* upper address is number of buffers available plus holes  - 1*/
        uFbLowerAddr =  uPbUpperAddr+1; /* free buffer memory starts immediately after packet buffer */
        uFbUpperAddr = uFbLowerAddr + (uTotalBufsAvl/1536) - 1; /* # of buffers in free buffer list is */
                                                                /* number of free buffs available + number */
                                                                /* of buffers required to store the FB list - 1 */

    }

    /* FB_BUF_ALLOC_PER_TS field is set to a value other then the default reset value of 1 */
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, 0, 2);
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CONFIG0, FB_EXT_UPPER_ADDR, uData, uFbUpperAddr);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CONFIG0, uData);

    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CONFIG1, FB_EXT_LOWER_ADDR, 0, uFbLowerAddr);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CONFIG1, uData);

    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CONFIG2, PB_EXT_UPPER_ADDR, 0, uPbUpperAddr);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CONFIG2, uData);

    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CONFIG3, PB_EXT_LOWER_ADDR, 0, uPbLowerAddr);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CONFIG3, uData);

    {
        /* configure global header adjustment */
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);
        if ( nGlobalShapingAdjustInBytes < 0 ) {
            uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, GLOBAL_HDR_ADJUST_SIGN, uData, 1);
            nGlobalShapingAdjustInBytes = -nGlobalShapingAdjustInBytes; /* flip to positive value */
        }

        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, GLOBAL_HDR_ADJUST, uData, nGlobalShapingAdjustInBytes);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG2, uData);
    }

    /* configure lines/timeslot parameters */
    if (bStandAlone) {
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_LOCAL, uData, nLinesPerTimeslotCongested);

        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, 0x01);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG2, uData);
    } else if (bHybrid) {
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_LOCAL, uData, nLinesPerTimeslotCongested);

        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, nLinesPerTimeslotCongested);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG2, uData);
    } else if (bDmode) {
        /* QE1K should never be granted, therefore the QE1K lines per timeslot is set to 1 line (both primary and degraded). */

        /* The QMGR always gets configured with the lower (congested) lines per ts */
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_LOCAL, uData, 0x04);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, nLinesPerTimeslotCongested);

        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, uData, 0x01);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG2, uData);
    } else {
        /* The QMGR always gets configured with the lower (congested) lines per ts */
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_LOCAL, uData, 4);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, nLinesPerTimeslotCongested);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, uData, nLinesPerTimeslotQe1k);

        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, nLinesPerTimeslotCongested);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, uData, nLinesPerTimeslotQe1k);

        SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG2, uData);
    }

    /* Set up anemic watermarks */
    for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
      if (SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos] != -1) {
	      if (bStandAlone || bHybrid) {
                /* Allocate template for local targets */
                rc = soc_qe2000_template_min_util_adjust((int32)userDeviceHandle,
                                                         SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL,
                                                         SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos],
                                                         &hwValue);
                if (SOC_E_NONE != rc) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("COS %2d local  min_util %3d%% invalid\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                    return rc;
                }
                rc = soc_qe2000_template_min_util_recall((int32)userDeviceHandle,
                                                         SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL,
                                                         hwValue,
                                                         &actValue);
                if (BCM_E_NONE != rc) {
                    actValue = -1;
                }
                rc = soc_sbx_connect_min_util_alloc((int32)userDeviceHandle,
                                                    SOC_SBX_CONN_FIXED,
                                                    hwValue,
                                                    &is_allocated,
                                                    &template);
                if (SOC_E_NONE != rc) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("Unable to allocate template for COS %2d"
                                  " local  min_util %3d%%\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                    return(rc);
                }
                
                QE2000PRINTF(QE2000_DEBUG_FUNC, 
                             ("COS %2d local  min util set %3d%% -> template %2d,"
                             " hw %02X, actual %3d%%\n",
                             cos,
                             SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos],
                             template,
                             hwValue,
                             actValue));

                if (is_allocated) {
                    rc = soc_qe2000_template_min_util_set((int32)userDeviceHandle,
                                                          template,
                                                          hwValue);
                    if (SOC_E_NONE != rc) {
                        QE2000PRINTF(QE2000_ERROR,
                                     ("Unable to configure template %d for"
                                      " COS %2d local  min_util %3d%%\n",
                                      template,
                                      cos,
                                      SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                        return(rc);
                    }
                }
                SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util_template[cos] = template;
	      }
	      if (!bStandAlone) {
                /* Allocate template for remote targets */
                
                rc = soc_qe2000_template_min_util_adjust((int32)userDeviceHandle,
                                                         SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000,
                                                         SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos],
                                                         &hwValue);
                if (SOC_E_NONE != rc) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("COS %2d remote min_util %3d%% invalid\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                    return rc;
                }
                rc = soc_qe2000_template_min_util_recall((int32)userDeviceHandle,
                                                         SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000,
                                                         hwValue,
                                                         &actValue);
                if (BCM_E_NONE != rc) {
                    actValue = -1;
                }
                rc = soc_sbx_connect_min_util_alloc((int32)userDeviceHandle,
                                                    SOC_SBX_CONN_FIXED,
                                                    hwValue,
                                                    &is_allocated,
                                                    &template);
                if (SOC_E_NONE != rc) {
                    QE2000PRINTF(QE2000_ERROR,
                                 ("Unable to allocate template for COS %2d"
                                  " remote min_util %3d%%\n",
                                  cos,
                                  SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                    return(rc);
                }
                
                QE2000PRINTF(QE2000_DEBUG_FUNC,
                             ("COS %2d remote min util set %3d%% -> template %2d,"
                             " hw %02X, actual %3d%%\n",
                             cos,
                             SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos],
                             template,
                             hwValue,
                             actValue));

                if (is_allocated) {
                    rc = soc_qe2000_template_min_util_set((int32)userDeviceHandle,
                                                          template,
                                                          hwValue);
                    if (SOC_E_NONE != rc) {
                        QE2000PRINTF(QE2000_ERROR,
                                     ("Unable to configure template %d for COS"
                                      " %2d remote min_util %3d%%\n",
                                      template,
                                      cos,
                                      SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos]));
                        return(rc);
                    }
                }
                SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util_remote_template[cos] = template;
	      }
            } else { /* if (this COS level has a defined value) */
                /* should never see this, since all have a default value */
                SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util_template[cos] = -1;
                SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util_remote_template[cos] = -1;
            } /* if (this COS level has a defined value) */
    } /* for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) */

#if 0 
    {
        int32 nLines;
        int32 _nLinesPerTimeslotQe2k;
        int32 _nLinesPerTimeslotQe1k;

        /* Bug 23458 Update Anemic Watermark for latency spike issue */
        /* The following for EF traffic, .5 * _nLinesPerTimeslotQe2k will be used - watermark 3 */
        /*                    for non-EF .8 * _nLinesPerTimeslotQe2k will be used - watermark 6 */

        nLines = SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG2);

        _nLinesPerTimeslotQe2k = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, nLines);
        _nLinesPerTimeslotQe1k = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, nLines);


        if (bDmode) {

#if 0 
            for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
                if (SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos] != -1) {
                    rc = soc_qe2000_template_min_util_adjust((int32)userDeviceHandle,
                                                             SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util[cos],
                                                             &hwValue);
                    if (SOC_E_NONE != rc) {
                        return rc;
                    }

                    rc = soc_sbx_connect_min_util_alloc((int32)userDeviceHandle,
                                                        SOC_SBX_CONN_FIXED,
                                                        hwValue,
                                                        &is_allocated,
                                                        &template);
                    if (rc != SOC_E_NONE) {
                        return(rc);
                    }

                    SOC_SBX_CFG((int32)userDeviceHandle)->connect_min_util_template[cos] = template;

                    if (is_allocated == FALSE) {
                       continue;
                    }

                    rc = soc_qe2000_template_min_util_set((int32)userDeviceHandle,
                                                          template,
                                                          hwValue);
                    if (rc != SOC_E_NONE) {
                        return(rc);
                    }
                }
            }
#endif 

#if 0
            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK7, uData, _nLinesPerTimeslotQe2k * 9/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK6, uData, _nLinesPerTimeslotQe2k * 8/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK5, uData, _nLinesPerTimeslotQe2k * 7/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK4, uData, _nLinesPerTimeslotQe2k * 6/10);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_ANEMIC_WATERMARK0, uData);

            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK3, uData, _nLinesPerTimeslotQe2k * 5/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK2, uData, _nLinesPerTimeslotQe2k * 4/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK1, uData, _nLinesPerTimeslotQe2k * 3/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK0, uData, _nLinesPerTimeslotQe2k * 2/10);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_ANEMIC_WATERMARK1, uData);
#endif /* 0 */

        } else {

            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK7, uData, _nLinesPerTimeslotQe1k * 8/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK6, uData, _nLinesPerTimeslotQe1k * 6/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK5, uData, _nLinesPerTimeslotQe1k * 4/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK0, ANEMIC_WATERMARK4, uData, _nLinesPerTimeslotQe1k * 2/10);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_ANEMIC_WATERMARK0, uData);

            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK3, uData, _nLinesPerTimeslotQe2k * 8/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK2, uData, _nLinesPerTimeslotQe2k * 6/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK1, uData, _nLinesPerTimeslotQe2k * 4/10);
            uData = SAND_HAL_MOD_FIELD(KA, QM_ANEMIC_WATERMARK1, ANEMIC_WATERMARK0, uData, _nLinesPerTimeslotQe2k * 2/10);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_ANEMIC_WATERMARK1, uData);
        }
    }
#endif 

    /* Configure the max # of bytes that arrive over a single epoch + length sum adjust */
    /* bug 22333 Divide by 8 because this is bytes/sec not bits/sec */
    uData = SAND_HAL_MOD_FIELD(KA, QM_DC_CONFIG0, MAX_ARRIVAL_BYTES, 0, (uEpochSizeInNs/1000 * uQmMaxArrivalRateMbs/8));
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_DC_CONFIG0, uData);

    uData = SAND_HAL_MOD_FIELD(KA, QM_DC_CONFIG1, LENGTH_SUM_ADJUST, 0, ((HW_QE2000_MAX_QUEUES * 3 * uQmMaxArrivalRateMbs) / (uClockSpeedInMHz * 8)));

    SAND_HAL_WRITE(userDeviceHandle, KA, QM_DC_CONFIG1, uData);

    /* if in C-mode, set the number of queues to 1024, not the 16k default */
    if (!bDmode) {
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_DEBUG_CONFIG);
        uData = SAND_HAL_MOD_FIELD(KA, QM_DEBUG_CONFIG, DEMAND_QNUM_MAX, uData, 1023);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_DEBUG_CONFIG, uData);
    }

    /* Take QM block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, QM_CORE_RESET, 0);

    /* initialize all WRED curves to non-drop (all 1s) */
    {
        int32 nCurve;
        int32 nQueue;

        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, 0xFFFFFFFF);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA1, 0xFFFFFFFF);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA2, 0xFFFFFFFF);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA3, 0xFFFFFFFF);

        for (nCurve=0; nCurve<48; nCurve++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0x10 /* WRED Curves */);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, nCurve);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: init QM memory timeout\n"));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);
        }

        /* clear the WRED prob memory */
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA1, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA2, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA3, 0x0);

        for (nQueue=0; nQueue<HW_QE2000_MAX_QUEUES; nQueue++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0xF /* WRED Data */);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, nQueue);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: QM memory init timeout entry 0x%x\n", nQueue));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);
        }

#if 0

        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA1, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA2, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA3, 0x0);

        for (nQueue=0; nQueue<HW_QE2000_MAX_QUEUES; nQueue++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0xE);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, nQueue);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: QM memory init timeout entry 0x%x\n", nQueue));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);
        }

#endif /* 0 */

    }

    /* seed the random number generator memory (used for WRED) */
    for (uRandArrayIndex=0; uRandArrayIndex<55; uRandArrayIndex=uRandArrayIndex+1) {
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_DATA0, DATA, uData, random_array[uRandArrayIndex]);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, uData);
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0xb);
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, uRandArrayIndex);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                            SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                            SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                            QM_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: QM memory init timeout entry 0x%x\n", uRandArrayIndex));
            return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

    }

    /*
     * There are 64 rate delta max table entries which set a damping level
     * on the responsiveness of a queue to changes in bandwidth allocation.
     * Recommended setting between 10 and 20% of line rate.  Units are in
     * bytes/epoch.  The smaller the rate delta max, the slower the
     * responsiveness in bw allocation.
     *
     * TME mode
     * rate in b/s = x bytes/epoch * 1byte/8 bits * 1 epoch/1582 timeslots * 1 timeslot/632ns solve for x.
     * x bytes/epoch = (rate b/s) * 632ns/timeslot * 1582 timeslots/epoch * 1 byte/8 bits
     * x bytes/epoch = (rate b/s) * 1.24978x10^-4
     * Set up the rate delta max table for Standalone switch mode as follows.
     */
    if (bStandAlone) {
        int32 nRateEntry;
        int32 nRateDeltaMaxInBytesPerEpoch[64] = {      12,     16,     21,     24, /* 1M */
                                                          25,     32,     42,     48, /* 2M */
                                                          62,     81,    106,    125, /* 5M */
                                                          87,    113,    148,    175, /* 7M */
                                                         125,    162,    212,    250, /*10M */
                                                         250,    324,    425,    500, /* 20M */
                                                         624,    812,   1062,   1249, /* 50M */
                                                         874,   1137,   1487,   1749, /* 70M */
                                                        1249,   1624,   2124,   2500, /* 100M */
                                                        2499,   3249,   4249,   4999, /* 200M */
                                                        6248,   8123,  10623,  12497, /* 500M */
                                                        8748,  11372,  14872,  17496, /* 700M */
                                                       12498,  16247,  21246,  24995, /* 1G */
                                                       24995,  32494,  42492,  49991, /* 2G */
                                                       62489,  81235, 106231, 124978, /* 5G */
                                                      124978, 162471, 212462, 249956}; /* 10G */


        if (SOC_SBX_CFG((int32)userDeviceHandle)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            nRateDeltaMaxInBytesPerEpoch[0] = 0;
        }

        for (nRateEntry=0; nRateEntry<64; nRateEntry++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_DATA0, DATA, uData, nRateDeltaMaxInBytesPerEpoch[nRateEntry]);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, uData);
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0x9 /* Rate Delta Max */);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, nRateEntry);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: QM memory init timeout rate entry 0x%x\n", nRateEntry));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;

            }
        }
    }
    else {

        /*
         * Sample Calculation:
         *
         * logical port rate 10Gbps.
         * rate 10%
         *
         * 632ns timeslot size in D-Mode 16401 byte epoch
         * 760ns timeslot size in C-Mode 5119 byte epoch
         *
         * rate in b/s = x bytes/epoch * 1byte/8 bits * 1 epoch/16401 timeslots * 1 timeslot/632ns solve for x.
         * x bytes/epoch = (rate b/s) * 632ns/timeslot * 16401 timeslots/epoch * 1 byte/8 bits
         * x bytes/epoch = (rate b/s) * 1.295679x10^3
         * Set up the rate delta max table for D-Mode as follows:
         *
         */
        if (bDmode) {

            rc = soc_qe2000_rate_delta_config((int32)userDeviceHandle, SB_FAB_TME_MIN_TIMESLOT_IN_NS, 
                                 SOC_SBX_CFG((int32)userDeviceHandle)->epoch_length_in_timeslots);
            if (rc != SOC_E_NONE) {
                return(rc);
            }
        }
        
        else {
            QE2000PRINTF(QE2000_DEBUG_FUNC, ("Warning: Rate delta max table uninitialized for C-Mode\n"));
        }
    }
    /* kick off the auto init */
    if (hwQe2000InitDone(userDeviceHandle,
                         SAND_HAL_KA_QM_CONFIG0_OFFSET,
                         SAND_HAL_KA_QM_CONFIG0_INIT_MASK,
                         SAND_HAL_KA_QM_CONFIG0_INIT_DONE_MASK,
                         AUTO_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: QM auto init timed out\n"));
        return HW_QE2000_STS_INIT_QM_AUTOINIT_TIMEOUT_ERR_K;
    }


    /* Bug 23399, enable WRED here to avoid WRED initialization issue */
    if (bStandAlone) {
        /* enable wred */
        uData = 0x00;     /* Initiating a WRITE */
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_ENABLE, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_LOWER_QUEUE, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_UPPER_QUEUE, uData, 0x3FFF);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_BW_CONFIG1, uData);

        /* enable bw */
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_BW_CONFIG0,
                       (SAND_HAL_SET_FIELD(KA, QM_BW_CONFIG0, BW_ENABLE,   1)) |
                       (SAND_HAL_SET_FIELD(KA, QM_BW_CONFIG0, BW_PORT_MAX, 4095)));
    }
    else if (bHybrid) {

        /* enable wred */
        uData = 0x00;     /* Initiating a WRITE */
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_ENABLE, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_LOWER_QUEUE, uData, hybridModeQueueDemarcation);
        uData = SAND_HAL_MOD_FIELD(KA, QM_BW_CONFIG1, WRED_UPPER_QUEUE, uData, 0x3FFF);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_BW_CONFIG1, uData);

        /* enable bw */
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_BW_CONFIG0,
                       (SAND_HAL_SET_FIELD(KA, QM_BW_CONFIG0, BW_ENABLE,   1)) |
                       (SAND_HAL_SET_FIELD(KA, QM_BW_CONFIG0, BW_PORT_MAX, 4095)));
    }


    /* enable the QM (i.e. bring it online) */
    uData=SAND_HAL_READ(userDeviceHandle, KA, QM_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG0, ENABLE, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, QM_CONFIG0, uData);

    /* manual FB list init is required for 256 Mbit DDR parts */
    if (!bQm512MbDdr2) {
        uint32 uFbCount;
        uint32 uFbBufAddr;
        uint32 uFbBufEntry;

        /* turn off the free buffer "churner" */
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, 0);

        /* pop all the entries out of the fb fifo */
        /* (pop until we see a head underflow error) */
        while (SAND_HAL_GET_FIELD(KA,
                                  QM_ERROR2,
                                  FB_HEAD_CACHE_UNDERFLOW,
                                  (SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2)))==0){


            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout popping all entries out of the fb fifo\n"));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }
            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

        }

        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_HEAD_CACHE_DEBUG);
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_TAIL_CACHE_DEBUG);
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG0);
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG1);
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2);

        /* clear the underflow error */
        uData=SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_ERROR2, uData);

        /* Push a bogus and identifiable entry on the FB */
        uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_DATA, BUF_ADDR, uData, 0x1ffff);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_DATA, uData);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                            SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_OFFSET,
                            SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_ACK_MASK,
                            QM_MEM_TIMEOUT)) {

            QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout writing fb cache\n"));
            return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
        }
        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

        QE2000PRINTF(QE2000_DEBUG_FUNC, ("Clearing FB fifo underflow error.\n"));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_HEAD_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_HEAD_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_TAIL_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_TAIL_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG0);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG0=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG1);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG1=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_ERROR2=0x%x\n", uData));
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("Pushing most of the entries into the FB fifo.\n"));

        /* push most of the entries onto the fb fifo */
        uFbBufAddr = 0;
        for (uFbCount=0; uFbCount<(uTotalBufsAvl); uFbCount++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_DATA, BUF_ADDR, uData, uFbBufAddr);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_DATA, uData);

            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 0);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: timeoout pushing entry onto fb fifo\n"));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

            uFbBufAddr = uFbBufAddr + 1;

            if (bPmHalfBus) {
                /* Remove all buffer addresses for which bit 2 is set */
                while (uFbBufAddr & 0x4)
                    uFbBufAddr=uFbBufAddr + 1;
            }
            else {
                /* Remove all buffer addresses for which bit 3 is set */
                while (uFbBufAddr & 0x8)
                    uFbBufAddr=uFbBufAddr + 1;
            }
        }

        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_HEAD_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_HEAD_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_TAIL_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_TAIL_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG0);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG0=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG1);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG1=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_ERROR2=0x%x\n", uData));
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("Popping all FB entries until 'marked' entry is seen.\n"));

        /* pop entries out of the fb fifo until we see the "marked" buffer */
        /* (this is is the beginning of the newly loaded buffers) */
        uFbBufEntry=0;
        while (uFbBufEntry != 0x1ffff) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_OFFSET,
                                SAND_HAL_KA_QM_FB_CACHE_ACC_CTRL_ACK_MASK,
                                QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout popping entries out of the fb fifo\n"));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            uFbBufEntry = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_CACHE_ACC_DATA);

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_FB_CACHE_ACC_CTRL, uData);
        }

        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_HEAD_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_HEAD_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_TAIL_CACHE_DEBUG);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_TAIL_CACHE_DEBUG=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG0);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG0=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_FB_EXT_MEMORY_DEBUG1);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_FB_EXT_MEMORY_DEBUG1=0x%x\n", uData));
        uData = SAND_HAL_READ(userDeviceHandle, KA, QM_ERROR2);
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("QM_ERROR2=0x%x\n", uData));

        /* turn on the free buffer "churner" */
        /* FB_BUF_ALLOC_PER_TS field is set to a value other then the default reset value of 1 */
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, 2);

    }

    /* Clear Scoreboard Buffer */
    {
        int32    nScoreboardIndex;


        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA0, 0x00);
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA1, 0x00); /* not necessary */
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA2, 0x00); /* not necessary */
        SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_DATA3, 0x00); /* not necessary */

        for (nScoreboardIndex = 0; nScoreboardIndex < 0x3FFF; nScoreboardIndex++) {
            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, uData, 0x7 /* scoreboard buffer */);
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, uData, nScoreboardIndex);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                                    SAND_HAL_KA_QM_MEM_ACC_CTRL_OFFSET,
                                    SAND_HAL_KA_QM_MEM_ACC_CTRL_ACK_MASK,
                                    QM_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: init QM memory (scoreboard) timeout\n"));
                return HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, QM_MEM_ACC_CTRL, uData);
        }
    }

    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitTx()
 *
 * OVERVIEW:        Initialize the QE2000 TX Block
 *
 * ARGUMENTS:
 *
 * Initialize the TX block
 *
 *     Input parameters:
 *     TYPE            NAME            SET AT    DESCRIPTION
 *     ----            ----            ------    -----------
 *     sbBool_t        bFabCrc32       system    32/16 bit crc packet payload protection
 *     uint32        uQe1KLinkEnMask system    bit mask defining which links are used by the qe1k.
 *                                               A bit being set means that its associated link is
 *                                               used for QE1K traffic.
 *     uint32 uSfiTimeslotOffsetInClocks  node Used to delay sending data to the switch (see .h for detail)
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:

 *     Notes:
 *     1) karen thinks we should use qnum max for bandwidth allocation
 *     2) SPI port base #'s are not configured (these fields are used for per queue SPI backpressure)
 *        Also, satisfied/hungry SPI backpressure thresholds are not configured.
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitTx(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               uint32 uQe1KLinkEnMask,
               uint32 uSfiTimeslotOffsetInClocks)
{
#define TX_LOCAL_DATA_THRESH 0x1ff /* local path can accept up to a 16000B packet */
#define TX_LOCAL_PKT_THRESH 0x80 /* local path can accept up to a 128 packets */

    uint32 uData;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    uData = SAND_HAL_MOD_FIELD(KA, TX_CONFIG0, SELECT_CRC32, 0, bFabCrc32);
    uData = SAND_HAL_MOD_FIELD(KA, TX_CONFIG0, LOC_DATA_THRESH, uData, TX_LOCAL_DATA_THRESH);
    SAND_HAL_WRITE(userDeviceHandle, KA, TX_CONFIG0, uData);

    uData = SAND_HAL_MOD_FIELD(KA, TX_CONFIG1, LOC_PKT_THRESH, 0, TX_LOCAL_PKT_THRESH);
    uData = SAND_HAL_MOD_FIELD(KA, TX_CONFIG1, QE1K_SFI_EN, uData, uQe1KLinkEnMask);

    SAND_HAL_WRITE(userDeviceHandle, KA, TX_CONFIG1, uData);

    /* Take TX block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, TX_CORE_RESET, 0);

    /* Kick off internal memory bist on all memories (some of which contain redundancy) */
    /* (This is required to run before the block is taken out of soft reset) */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, TX_BIST_CONFIG0, BIST_SETUP, 2);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM0_MBIST_EN, 0, 1);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM1_MBIST_EN, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM2_MBIST_EN, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM3_MBIST_EN, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM4_MBIST_EN, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG1, MEM5_MBIST_EN, uData, 1);

    SAND_HAL_WRITE(userDeviceHandle, KA, TX_BIST_CONFIG1, uData);
    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_TX_BIST_STATUS0_OFFSET,
                        0x0000003f,
                        INTERNAL_MEMBIST_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout on tx bist\n"));
        return HW_QE2000_STS_INIT_RB_INTERNAL_MEMBIST_TIMEOUT_ERR_K;
    }
    /*
     * Disable MBIST after complete
     */
    SAND_HAL_WRITE(userDeviceHandle, KA, TX_BIST_CONFIG1, 0);
    /* MCM move this to after bist is run */
    /* Take TX block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, TX_CORE_RESET, 0);

    /* setup timeslot offset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, TX_DEBUG, TIMESLOT_OFFSET, uSfiTimeslotOffsetInClocks);

    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitSfi()
 *
 * OVERVIEW:        Initialize the QE2000 SFI blocks
 *
 * ARGUMENTS:
 *       Input parameters:
 *       TYPE         NAME                 SET AT    DESCRIPTION
 *       ----         ----                 ------    -----------
 *       uint32     uSfiDataLinkInitMask node      bits 17:0 make up a mask defining which data links are used
 *                                                   (set to 1 to enable a link)
 *       uint32     uNodeId              node      This is the node number of this QE2K
 *       uint32     uClockSpeedInMHz     system    clock speed in MHz
 *       uint32     uSfiXconfig[18][256]    node      doubly indexed array which contains the 255 xconfig remap table for each of the 18 SFI links
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:
 * Initialize the SFI blocks.
 *       This powers up the SERDES PHYs for all the SFI's that are used in the system.
 *
 *      It sets the local node number in the sfi. It does NOT:
 *      1) Enable the SI port (SERDES link layer function)
 *      2) Enable the SFI port
 *      3) set up the XCONFIG remap table in the SFI
 *      TBD) Need to receive an xconfig array and write it into the table (just pass thru, no fancy stuff)
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitSfi(sbhandle userDeviceHandle,
                uint32 uSfiDataLinkInitMask,
                uint32 uNodeId,
                uint32 uClockSpeedInMHz,
                uint32 uSfiXconfig[HW_QE2000_NUM_SFI_LINKS][HW_QE2000_NUM_XCFG_REMAP_ENTRIES],
                uint32 uSiLsThreshold,
                uint32 uSiLsWindow,
		uint32 uSfiHeaderPad)
{

#define SERDES_POWERUP_TIMEOUT     (4) /* 400 milliseconds */
#define SF_XCNFG_REMAP_MEM_TIMEOUT (4) /* 400 milliseconds */


    uint32 uCurrentLinkNum;
    uint32 uData;
    uint32 uCurrentXconfig;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_CORE_RESET1, 0); /* all SFI out of reset */

    /* enable all links, but leave TX forced low */
    for (uCurrentLinkNum=0; uCurrentLinkNum<HW_QE2000_NUM_SFI_LINKS; uCurrentLinkNum++) {
        sbBool_t bIsMasterLink;

        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_SD_CONFIG, ENABLE, 1);

        bIsMasterLink = FALSE;
        switch (uCurrentLinkNum) {
            case 0:
            case 16:
            case 3:
            case 4:
            case 7:
            case 8:
            case 11:
            case 12:
            case 15:
                bIsMasterLink = TRUE;
        }
        if ( bIsMasterLink ) {

            if (hwQe2000RegDone(userDeviceHandle,
                                (SAND_HAL_KA_SF0_SI_SD_STATUS_OFFSET+SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum),
                                0x0001,
                                SERDES_POWERUP_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout writing to sfi master link(0x%x)\n", uCurrentLinkNum));
                return HW_QE2000_STS_INIT_SERDES_POWERUP_DONE_TIMEOUT_ERR_K;
            }
        }


        /* bring them up, but force the TX low.  This must be release by the device handler */
        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, 1);

        /* Set the local node number */
        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_CONFIG, NODE, uNodeId);
        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_CONFIG, ENABLE, 1);
        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_CONFIG, RX_HDR_PAD, uSfiHeaderPad);
        SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_CONFIG, TX_HDR_PAD, uSfiHeaderPad);

        { /* jts - added to align with 612 (writes then overwrites) */
            uData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DEQ, uData, 0x8);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DTX, uData, 0xF);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, HIDRV, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, LODRV, uData, 1);
            SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3, uData);
        }
        { /* jts - added to align with bm3200 */
            uData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DEQ, uData, 0xA);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DTX, uData,   0);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, HIDRV, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, LODRV, uData, 0);
            SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3, uData);
        }

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG0,
                              SAND_HAL_SET_FIELD(KA, SF0_SI_CONFIG0, JIT_TOLERANCE, 0x14));

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_DEBUG0,
                              SAND_HAL_SET_FIELD(KA, SF0_SI_DEBUG0, LOWSIG_ASSERT_FILTER, 0x4) |
                              SAND_HAL_SET_FIELD(KA, SF0_SI_DEBUG0, LOWSIG_NEGATE_FILTER, 0xf) |
                              SAND_HAL_SET_FIELD(KA, SF0_SI_DEBUG0, BASM_FAST_TIMEOUT, 0x1));

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_DEBUG2,
                              SAND_HAL_SET_FIELD(KA, SF0_SI_DEBUG2, TX_FIFO_UNDERRUN_CHK_EN, 0x1) |
                              SAND_HAL_SET_FIELD(KA, SF0_SI_DEBUG2, TX_FIFO_AFULL_THRESH, 0x14));

        { /* fns - added to configure window and threshold, was 3, 0xff by default */
            uData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG2);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG2, LS_ERR_THRESH, uData, uSiLsThreshold);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG2, LS_ERROR_WINDOW, uData,   uSiLsWindow);
            SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG2, uData);
        }

    }



    for (uCurrentLinkNum=0; uCurrentLinkNum<HW_QE2000_NUM_SFI_LINKS; uCurrentLinkNum++) {
        /* LOAD THE XCONFIG TABLE */
        for (uCurrentXconfig=0; uCurrentXconfig<HW_QE2000_NUM_XCFG_REMAP_ENTRIES; uCurrentXconfig++) {
            if (uCurrentXconfig<64 || uCurrentXconfig==127 || uCurrentXconfig==255) {
                uData=0;

                if ( uSfiXconfig[uCurrentLinkNum][uCurrentXconfig] != 127 ) {
                    QE2000PRINTF(QE2000_DEBUG_FUNC, ("SfiXcfg[%d][%x] = %d\n", uCurrentLinkNum, uCurrentXconfig,
                                                     uSfiXconfig[uCurrentLinkNum][uCurrentXconfig]));
                }

                uData = SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, XCNFG, uSfiXconfig[uCurrentLinkNum][uCurrentXconfig]);
                SAND_HAL_WRITE_OFFS(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_DATA_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum,
                                    uData);

                uData=0;
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, REQ, uData, 1);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, RD_WR_N, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, uData, uCurrentXconfig);
                SAND_HAL_WRITE_OFFS(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_CTRL_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum,
                                    uData);

                if (hwQe2000RegDone(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_CTRL_OFFSET,
                                    0x80000000,
                                    SF_XCNFG_REMAP_MEM_TIMEOUT)) {

                    QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout writing xcnfg remap memory (0x%x)\n", uCurrentXconfig));
                    return HW_QE2000_STS_INIT_SF_MEM_TIMEOUT_ERR_K;

                }
                /* clear req/ack bits */
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, ACK, uData, 1);
                SAND_HAL_WRITE_OFFS(userDeviceHandle,
                               SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_CTRL_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum,
                               uData);
            }
        }
    }

#if 0
    sbBool_t bCurrentLinkUsed;
    sbBool_t bSlaveLinkUsed;
    uint32 uSlaveLinkNum;

    for (uCurrentLinkNum=0; uCurrentLinkNum<HW_QE2000_NUM_SFI_LINKS; uCurrentLinkNum++) {

        /* BRING UP THE SI PORTS */

        /* jit tolerance should be set to 72ns (18 clock cycles at 250 MHz) per system skew spec */
        uint32 nJitTolerance = 0x12*250/uClockSpeedInMHz;
        uData=SAND_HAL_READ(userDeviceHandle, KA, SF0_SI_CONFIG0);
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, JIT_TOLERANCE, uData, nJitTolerance);
        SAND_HAL_WRITE_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_SI_CONFIG0_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum, uData);

        switch (uCurrentLinkNum) {
        case 0:
            uSlaveLinkNum=2;
            break;
        case 16:
            uSlaveLinkNum=1;
            break;
        case 3:
            uSlaveLinkNum=5;
            break;
        case 4:
            uSlaveLinkNum=6;
            break;
        case 7:
            uSlaveLinkNum=9;
            break;
        case 8:
            uSlaveLinkNum=10;
            break;
        case 11:
            uSlaveLinkNum=13;
            break;
        case 12:
            uSlaveLinkNum=14;
            break;
        case 15:
            uSlaveLinkNum=17;
            break;
        default: /* we can detect current link is slave is uSlaveLinkNum==uCurrentLinkNum */
            uSlaveLinkNum=uCurrentLinkNum;
        }

        bCurrentLinkUsed=(uSfiDataLinkInitMask>>uCurrentLinkNum) & 0x1;
        bSlaveLinkUsed=(uSfiDataLinkInitMask>>uSlaveLinkNum) & 0x1;

        /* If either the current link or it's slave is being used, the current link's SFI must be brought up */
        if (bCurrentLinkUsed || bSlaveLinkUsed) {
            uData=SAND_HAL_READ(userDeviceHandle, KA, PC_CORE_RESET1);

            /* take the SFI out of soft reset */
            /* (write back reset bits uCurrentLinkNum-1:0, clear bit uCurrentLinkNum, set bits 31:uCurrentLinkNum+1) */
            /*SAND_HAL_WRITE(userDeviceHandle, KA, PC_CORE_RESET1, (((1<<uCurrentLinkNum)-1)&uData) | (0x3fffe<<uCurrentLinkNum));*/

            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_SD_CONFIG, ENABLE, uData, 0x1);
            SAND_HAL_WRITE_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_SI_SD_CONFIG_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum, uData);

            { /* jts - added to align with bm3200 */
                uData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DEQ, uData, 10);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DTX, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, HIDRV, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, LODRV, uData, 0);
                SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SF, uCurrentLinkNum, SF0_SI_CONFIG3, uData);
            }


            if (uSlaveLinkNum != uCurrentLinkNum) { /* current link is a master */

                if (hwQe2000RegDone(userDeviceHandle,
                                   (SAND_HAL_KA_SF0_SI_SD_STATUS_OFFSET+SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum),
                                   0x0001,
                                    SERDES_POWERUP_TIMEOUT)) {
                    QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout enabling master serdes (0x%x)\n", uCurrentLinkNum));
                    return HW_QE2000_STS_INIT_SERDES_POWERUP_DONE_TIMEOUT_ERR_K;
                }

                if (!bCurrentLinkUsed) { /* if this is an unused master, power down its serdes */
                    uData=SAND_HAL_READ_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_SI_CONFIG3_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum);
                    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, SERDES_PWRDOWN, uData, 0x1);
                    SAND_HAL_WRITE_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_SI_SD_CONFIG_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum, uData);
                }
            }

        }

        /* BRING UP THE SFI */

        /* Set the local node number */
        uData=SAND_HAL_READ_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_CONFIG_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum);
        uData = SAND_HAL_MOD_FIELD(KA, SF0_CONFIG, NODE, uData, uNodeId);
        SAND_HAL_WRITE_OFFS(userDeviceHandle, SAND_HAL_KA_SF0_CONFIG_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum, uData);


        /* LOAD THE XCONFIG TABLE */
        for (uCurrentXconfig=0; uCurrentXconfig<HW_QE2000_NUM_XCFG_REMAP_ENTRIES; uCurrentXconfig++) {
            if (uCurrentXconfig<64 || uCurrentXconfig==127 || uCurrentXconfig==255) {
                uData=0;

                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, XCNFG, uData, uSfiXconfig[uCurrentLinkNum][uCurrentXconfig]);
                SAND_HAL_WRITE_OFFS(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_DATA_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum,
                                    uData);

                uData=0;
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, REQ, uData, 1);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, RD_WR_N, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, uData, uCurrentXconfig);
                SAND_HAL_WRITE_OFFS(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_CTRL_OFFSET + SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*uCurrentLinkNum,
                                    uData);

                if (hwQe2000RegDone(userDeviceHandle,
                                    SAND_HAL_KA_SF0_XCNFG_REMAP_MEM_ACC_CTRL_OFFSET,
                                    0x80000000,
                                    SF_XCNFG_REMAP_MEM_TIMEOUT)) {

                    QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout writing to xcnfg remap table\n"));
                    return HW_QE2000_STS_INIT_SF_MEM_TIMEOUT_ERR_K;

                }

                /* clear req/ack bits */
                uData=0;
                uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
                SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);
            }
        }
    }
#endif
  return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitSci()
 *
 * OVERVIEW:        Initialize the QE2000 SCI blocks
 *
 * ARGUMENTS:
 *      Input parameters:
 *     TYPE            NAME                       SET AT    DESCRIPTION
 *     ----            ----                       ------    -----------
 *     uint32        uSciLinkEnRemap[2][18]     SYSTEM    link enable remap entry per SCI link per SFI link
 *     uint32        uSciLinkStatusRemap[2][18] SYSTEM    link status remap entry per SCI link per SFI link
 *     uint32        uClockSpeedInMHz           SYSTEM    clock speed in MHz
 *     sbBool_t        bStandAlone                SYSTEM    QE2K is operating in standalone switch mode
 *     uint32        uStandAloneEpochSizeInNs   SYSTEM    epoch duration in NS
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:
 * Initialize the SC receive block.
 *
 *     Notes:
 *     1) assumes default bm is 0
 *     2) No failover/gracedful degradation enabled
 *     3) At the end of the init, it checks to see if any sots are being received:
 *        -masks the sot_watchdog_timeout_count error
 *       -takes block out of soft reset
 *       -waits
 *        -checks the sot_watchdog_timeout count
 *       -if 0, assume that grants are coming in from an external BM and then turn the PM refresh rate down to
 *        "operational" rate. If 1, then do nothing (assume no external arbiter, therefore the PM refresh rate
 *        needs to remain at "init" rate.)
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitSci(sbhandle userDeviceHandle,
                uint32 uSciLinkEnRemap[HW_QE2000_NUM_SCI_LINKS][HW_QE2000_NUM_SFI_LINKS],
                uint32 uSciLinkStatusRemap[HW_QE2000_NUM_SCI_LINKS][HW_QE2000_NUM_SFI_LINKS],
                uint32 uClockSpeedInMHz, sbBool_t bStandAlone, uint32 uStandAloneEpochSizeInNs,
                uint32 uSiLsThreshold, uint32 uSiLsWindow, uint32 uSciDefaultBmId,
                uint32 uScTxdmaSotDelayInClocks, uint32 uScGrantoffset, uint32 uScGrantDelay, sbBool_t bDmode)
{

/* bug 22599 Timeslot size in clocks for standalone switch mode */
#define STANDALONE_TS_IN_CLOCKS (SB_FAB_TME_MIN_TIMESLOT_IN_NS/4)

    uint32 uData=0;
    uint32 uCurrentSci=0;
    uint32 uCurrentRemapLink=0;
    uint32 uTempLinkEnableRemapRegValue=0;
    uint32 uTempLinkStatusRemapRegValue=0;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    uData=0;
    uData = SAND_HAL_READ(userDeviceHandle, KA, SC_CONFIG0);

    /* Should always be 0, we don't want to use missing SOT threshold to determine data link health,
     * this is handled in the Bm3200 by the transmitter byte alignment state machine on page 126
     * in the petronius spec, ls_err_thresh and ls_Err_cnt are used to disable/enable SI port
     */
    uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, MISSING_SOT_THRESH, uData, 0);

    /* This value is * 1024 to get ns in which we will see an SOT, when running */
    /* with only a single link in DMODE we're over 8096ns, so the minimum value */
    /* would be 9.  Since this is a saftey failover mechanism, set to 9*2 to be */
    /* sure that we don't trip falsely */
    uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, SCI_SOT_WATCHDOG_THRESH, uData, 18);

    if (bStandAlone) {
        uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, SELECT_TEST_PATH, uData, 1);
    }
    /* Set which SCI link to listen to if in FIC mode */
    uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, DEFAULT_BM, uData, uSciDefaultBmId);

    uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, GRANT_DELAY, uData, uScGrantDelay);

    SAND_HAL_WRITE(userDeviceHandle, KA, SC_CONFIG0, uData);



    if (bStandAlone) {

        uint32 uStandAloneTsInNs;
        uint32 uEpochSizeInTs;

#define HWQE2000_NULLGRANT20_WORD1 0x807F0000

        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA0, HWQE2000_NULLGRANT20_WORD1);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA1, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA2, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA3, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA4, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA5, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA6, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_DATA7, 0);

        /* Inject idle grants periodically */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL0, COMMAND, uData, 0xAA);
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL0, LENGTH, uData, 8);
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL0, REPEAT_MODE, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL0, DIRECTION, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_CTRL0, uData);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL1, TS_LENGTH, uData, STANDALONE_TS_IN_CLOCKS);
        uStandAloneTsInNs = (STANDALONE_TS_IN_CLOCKS * 1000) / uClockSpeedInMHz;

        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_CTRL1, uData);

        uEpochSizeInTs = (uStandAloneEpochSizeInNs / uStandAloneTsInNs);
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, SC_CMD_INJECT_CTRL2, EPOCH_LENGTH, uData, uEpochSizeInTs);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_CMD_INJECT_CTRL2, uData);

        /* Mask SOT timeout error */
        uData=0;
        uData=SAND_HAL_MOD_FIELD(KA, SC_ERROR_MASK, SOT_WATCHDOG_TIMEOUT_DISINT, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, SC_ERROR_MASK, uData);

        /* Take SC block out of soft reset */
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SC_CORE_RESET, 0);

    } else { /* Not in stand alone switch mode */

        /* LOAD LINK ENABLE/STATUS REMAPPINGS */
        for (uCurrentSci=0; uCurrentSci<HW_QE2000_NUM_SCI_LINKS; uCurrentSci++) {
            for (uCurrentRemapLink=0; uCurrentRemapLink<HW_QE2000_NUM_SFI_LINKS; uCurrentRemapLink++) {
                if (uCurrentRemapLink==0 ||
                    uCurrentRemapLink==6 ||
                    uCurrentRemapLink==12) {
                    uTempLinkEnableRemapRegValue=0;
                    uTempLinkStatusRemapRegValue=0;
                }

                uTempLinkEnableRemapRegValue=uTempLinkEnableRemapRegValue |
                    (uSciLinkEnRemap[uCurrentSci][uCurrentRemapLink]<<((uCurrentRemapLink%6)*5));

                uTempLinkStatusRemapRegValue=uTempLinkStatusRemapRegValue |
                    (uSciLinkStatusRemap[uCurrentSci][uCurrentRemapLink]<<((uCurrentRemapLink%6)*5));

                if (uCurrentSci==0) {
                    if (uCurrentRemapLink==5) { /* Write remap values for SCI0 links 0:5 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_ENABLE_REMAP0, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_STATUS_REMAP0, uTempLinkStatusRemapRegValue);
                    } else if (uCurrentRemapLink==11) { /* Write remap values for SCI0 links 6:11 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_ENABLE_REMAP1, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_STATUS_REMAP1, uTempLinkStatusRemapRegValue);
                    } else if (uCurrentRemapLink==17) { /* Write remap values for SCI0 links 0:5 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_ENABLE_REMAP2, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI0_LINK_STATUS_REMAP2, uTempLinkStatusRemapRegValue);
                    }
                } else if (uCurrentSci==1) {
                    if (uCurrentRemapLink==5) { /* Write remap values for SCI1 links 0:5 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_ENABLE_REMAP0, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_STATUS_REMAP0, uTempLinkStatusRemapRegValue);
                    } else if (uCurrentRemapLink==11) { /* Write remap values for SCI1 links 6:11 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_ENABLE_REMAP1, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_STATUS_REMAP1, uTempLinkStatusRemapRegValue);
                    } else if (uCurrentRemapLink==17) { /* Write remap values for SCI1 links 0:5 */
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_ENABLE_REMAP2, uTempLinkEnableRemapRegValue);
                        SAND_HAL_WRITE(userDeviceHandle, KA, SC_SCI1_LINK_STATUS_REMAP2, uTempLinkStatusRemapRegValue);
                    }
                }
            }
        }

        /* Take SC block out of soft reset */
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SC_CORE_RESET, 0);

        /* SCI0 Initialization */
        {
            SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_SI0_SD_CONFIG, ENABLE, 1);

            if (hwQe2000RegDone(userDeviceHandle,
                                (SAND_HAL_KA_SC_SI0_SD_STATUS_OFFSET),
                                0x0001,
                                SERDES_POWERUP_TIMEOUT)) {

                QE2000PRINTF(QE2000_ERROR, ("ERROR: sci0 serdes powerup timeout\n"));
                return HW_QE2000_STS_INIT_SERDES_POWERUP_DONE_TIMEOUT_ERR_K;
            }

            /* bring them up, but force the TX low.  This must be release by the device handler */
            SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_SI0_DEBUG1, FORCE_SERDES_TX_LOW, 1);

            { /* jts - added to align with 612 (writes then overwrites) */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI0_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DEQ, uData, 0x8);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DTX, uData, 0xF);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, HIDRV, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, LODRV, uData, 1);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_CONFIG3, uData);
            }
            { /* jts - added to align with bm3200 */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI0_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DEQ, uData, 0xA);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DTX, uData,   0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, HIDRV, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, LODRV, uData, 0);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_CONFIG3, uData);
            }

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_CONFIG0,
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_CONFIG0, ENABLE, 0x1) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_CONFIG0, JIT_TOLERANCE, 0x14));

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_DEBUG0,
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_DEBUG0, LOWSIG_ASSERT_FILTER, 0x4) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_DEBUG0, LOWSIG_NEGATE_FILTER, 0xf) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_DEBUG0, BASM_FAST_TIMEOUT, 0x1));

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_DEBUG2,
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_DEBUG2, TX_FIFO_UNDERRUN_CHK_EN, 0x1) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI0_DEBUG2, TX_FIFO_AFULL_THRESH, 0x14));

            { /* fns - added to configure window and threshold, was 3, 0xff by default */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI0_CONFIG2);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG2, LS_ERR_THRESH, uData, uSiLsThreshold);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG2, LS_ERROR_WINDOW, uData,   uSiLsWindow);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI0_CONFIG2, uData);
            }
        }

        /* SCI1 Initialization */
        {
            SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_SI1_SD_CONFIG, ENABLE, 1);

            if (hwQe2000RegDone(userDeviceHandle,
                                (SAND_HAL_KA_SC_SI1_SD_STATUS_OFFSET),
                                0x0001,
                                SERDES_POWERUP_TIMEOUT)) {

                QE2000PRINTF(QE2000_ERROR, ("ERROR: sci1 serdes powerup timeout\n"));
                return HW_QE2000_STS_INIT_SERDES_POWERUP_DONE_TIMEOUT_ERR_K;
            }

            /* bring them up, but force the TX low.  This must be release by the device handler */
            SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_SI1_DEBUG1, FORCE_SERDES_TX_LOW, 1);

            { /* jts - added to align with 612 (writes then overwrites) */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI1_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DEQ, uData, 0x8);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DTX, uData, 0xF);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, HIDRV, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, LODRV, uData, 1);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_CONFIG3, uData);
            }
            { /* jts - added to align with bm3200 */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI1_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DEQ, uData, 0xA);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DTX, uData,   0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, HIDRV, uData, 0);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, LODRV, uData, 0);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_CONFIG3, uData);
            }

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_CONFIG0,
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_CONFIG0, ENABLE, 0x1) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_CONFIG0, JIT_TOLERANCE, 0x14));

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_DEBUG0,
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_DEBUG0, LOWSIG_ASSERT_FILTER, 0x4) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_DEBUG0, LOWSIG_NEGATE_FILTER, 0xf) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_DEBUG0, BASM_FAST_TIMEOUT, 0x1));

            SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_DEBUG2,
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_DEBUG2, TX_FIFO_UNDERRUN_CHK_EN, 0x1) |
                                  SAND_HAL_SET_FIELD(KA, SC_SI1_DEBUG2, TX_FIFO_AFULL_THRESH, 0x14));

            { /* fns - added to configure window and threshold, was 3, 0xff by default */
                uData = SAND_HAL_READ(userDeviceHandle, KA, SC_SI1_CONFIG2);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG2, LS_ERR_THRESH, uData, uSiLsThreshold);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG2, LS_ERROR_WINDOW, uData,   uSiLsWindow);
                SAND_HAL_WRITE(userDeviceHandle, KA, SC_SI1_CONFIG2, uData);
            }
        }
    }

    /* Bug 24260, add txdma SOT delay init parameter due to serializer errors in half-bus mode with 16 links */
    uData = SAND_HAL_READ(userDeviceHandle, KA, SC_CONFIG1);
    uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG1, TXDMA_SOT_DELAY, uData, uScTxdmaSotDelayInClocks);

    /* If in C-Mode, send queue length updates */
    if (!bDmode) {
        uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG1, QL_PER_TIMESLOT, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG1, QL_BEFORE_PRI, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG1, QL_ENABLE, uData, 1);
    }
    SAND_HAL_WRITE(userDeviceHandle, KA, SC_CONFIG1, uData);

    /* Turn on continuous local grant injection for stand alone mode */
    if (bStandAlone) {

      SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_CMD_INJECT_CTRL0, CMD_REQ, 1);

    } else {
        
        SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_CONFIG1, PU_PER_TIMESLOT, 10);
    }

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, SC_DEBUG, GRANT_OFFSET, uScGrantoffset);

    return HW_QE2000_STS_OK_K;
}
/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitSv()
 *
 * OVERVIEW:        Initialize the QE2000 SV blocks
 *
 * ARGUMENTS:
 *      Input parameters:
 *     TYPE            NAME            SET AT    DESCRIPTION
 *     ----            ----            ------    -----------
 *     sbBool_t        bFabCrc32        system    32/16 bit crc packet payload protection
 *     uint32        uNodeId          node      node id # for this node
 *     uint32        uNumNodes        node      number of nodes in system
 *     uint32        uClockSpeedInMHz system    clock speed in MHz
 *     sbBool_t        bSv2_5GbpsLinks  system    set true if the data links are running at 2.5Gbps
 *     uint32        uQe1KLinkEnMask  system    bit mask defining which links are used by the qe1k.
 *                                               A bit being set means that its associated link is
 *                                               used for QE1K traffic.
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the SV block.
 *
 *
 * ASSUMPTIONS:      NOTES:
 *       1) This does not setup the node types for the system (SV_CONFIG2, SV_CONFIG3). I'm assuming these will be set up somewhere else.
 *         (This table needs to be reconfigured every time a new node is added to the system-- at least in a CMODE system)
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitSv(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               uint32 uNodeId,
               uint32 uNumNodes,
               uint32 uClockSpeedInMHz,
               sbBool_t bSv2_5GbpsLinks,
               uint32 uQe1KLinkEnMask)
{

#define SKEW_TOLERANCE_2_5    272
#define SKEW_TOLERANCE_3_125  217

    uint32 uData=0;
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    uData = SAND_HAL_MOD_FIELD(KA, SV_CONFIG0, SELECT_CRC32, uData, bFabCrc32);
    uData = SAND_HAL_MOD_FIELD(KA, SV_CONFIG0, NODE, uData, uNodeId);
    if (bSv2_5GbpsLinks) /* With 250MHz core clock + 2.5Gbps data links, skew tolerance should be 68 */
        uData = SAND_HAL_MOD_FIELD(KA, SV_CONFIG0, SFI_SKEW_TOLERANCE, uData, (SKEW_TOLERANCE_2_5*uClockSpeedInMHz)/1000);
    else /* With 250MHz core clock + 3.125Gbps data links, skew tolerance should be 54 */
        uData = SAND_HAL_MOD_FIELD(KA, SV_CONFIG0, SFI_SKEW_TOLERANCE, uData, (SKEW_TOLERANCE_3_125*uClockSpeedInMHz)/1000);
    SAND_HAL_WRITE(userDeviceHandle, KA, SV_CONFIG0, uData);

    uData = SAND_HAL_MOD_FIELD(KA, SV_CONFIG1, QE1K_SFI_EN, 0, uQe1KLinkEnMask);
    SAND_HAL_WRITE(userDeviceHandle, KA, SV_CONFIG1, uData);

    /* assume qe2k's only */
    SAND_HAL_WRITE(userDeviceHandle, KA, SV_CONFIG2, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, SV_CONFIG3, 0xffffffff);


    /* Take SV block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SV_CORE_RESET, 0);


  return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000SetEgMCSrcId()
 *
 * OVERVIEW:        Set the egress multicast src id.
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:         NONE
 *
 * DESCRIPTION:     Set the egress multicast srce id
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static void hwQe2000SetEgMcSrcId(sbhandle userDeviceHandle,
                                 uint32 uPort,
                                 uint32 uSrcId)
{
  const uint32 uRegStride  = (SAND_HAL_KA_EG_MC_SRC_ID_1_OFFSET -
                           SAND_HAL_KA_EG_MC_SRC_ID_0_OFFSET);

  uint32 uData = SAND_HAL_READ_OFFS(userDeviceHandle,
                                      (SAND_HAL_KA_EG_MC_SRC_ID_0_OFFSET + uRegStride * (uPort >> 1)));

  if (uPort & 1) {
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_SRC_ID_0, P1_SRC_ID,
                               uData, uSrcId);
  } else {
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_SRC_ID_0, P0_SRC_ID,
                               uData, uSrcId);
  }
  SAND_HAL_WRITE_OFFS(userDeviceHandle,
                      (SAND_HAL_KA_EG_MC_SRC_ID_0_OFFSET + uRegStride * (uPort >> 1)),
                      uData);
}
/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitEgMcPtrFifos()
 *
 * OVERVIEW:        Initialize the EG Multicast pointer FIFO sizes (1024 bytes total)
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:         NONE
 *
 * DESCRIPTION:     Initialize the EG multicast pointer FIFO sizes.
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitEgMcPtrFifos(sbhandle userDeviceHandle,
			 uint32 uNumPhySpiPorts[2],
			 uint32 uEgMcDropOnFull,
			 HW_QE2000_INIT_ST *pQe2000Params) {

     uint32 uMcQCtlRemainingSize;
     uint32 uMcQCtlRemainingFifos;
     uint32 uCurrentMcQCtlSize;
     uint32 uMcQCtlSize;
     uint32 uMcQCtlBase;
     uint32 uCurrentMcQFifo;
     uint32 uCurrentDropOnFull;
     uint32 port;
     int32 rv;
     int32 unit = (int)userDeviceHandle;

    /**** MC PORT POINTER FIFO DEPTHS ****/

    uMcQCtlRemainingSize = 1022;
    uMcQCtlRemainingFifos=2*(uNumPhySpiPorts[0]+uNumPhySpiPorts[1]+1); /* 2 fifos per port, ports from both SPI's + 1 PCI port */

#ifdef DEBUG_EG_FIFO_PTRS
    LOG_CLI((BSL_META("uMcQCtlRemainingFifos=%d\n"), uMcQCtlRemainingFifos));
#endif

#ifdef DEBUG_EG_FIFO_PTRS
    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
	LOG_CLI((BSL_META("qe2000_init hwQe2000InitParams_sp->uEgressMcastEfDescFifoInUse[%d]=%d\n"), port, 
                 SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[port]));
	LOG_CLI((BSL_META("qe2000_init hwQe2000InitParams_sp->uEgressMcastNefDescFifoInUse[%d]=%d\n"), port, 
                 SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[port]));
    }
#endif

    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
	
	if ((pQe2000Params->uEgressMcastEfDescFifoSize[port] > 6)  && (pQe2000Params->uEgressMcastEfDescFifoSize[port] != -1)) {
	    QE2000PRINTF(QE2000_ERROR, ("ERROR: QE2000 parameter error out of range 2^n where n must be less than 7 (egress_mcast_ef_desc_sz)"));
	    return SOC_E_PARAM;
	}
	if ((pQe2000Params->uEgressMcastNefDescFifoSize[port] > 6)  && (pQe2000Params->uEgressMcastNefDescFifoSize[port] != -1)) {
	    QE2000PRINTF(QE2000_ERROR, ("ERROR: QE2000 parameter error out of range 2^n where n must be less than 7 (egress_mcast_nef_desc_sz)"));
	    return SOC_E_PARAM;
	}

	if ((pQe2000Params->bEgressMcastEfDescFifoInUse[port]) && (pQe2000Params->uEgressMcastEfDescFifoSize[port] != -1)) {
  	    SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = pQe2000Params->uEgressMcastEfDescFifoSize[port];
	    uMcQCtlRemainingSize -= pQe2000Params->uEgressMcastEfDescFifoSize[port];
	    uMcQCtlRemainingFifos--;
	}

	if (pQe2000Params->bEgressMcastNefDescFifoInUse[port] && (pQe2000Params->uEgressMcastNefDescFifoSize[port] != -1)) {
	    SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = pQe2000Params->uEgressMcastNefDescFifoSize[port];
	    uMcQCtlRemainingSize -= pQe2000Params->uEgressMcastNefDescFifoSize[port];
	    uMcQCtlRemainingFifos--;
	}

    }
#ifdef DEBUG_EG_FIFO_PTRS
    /* Now we have the total number of remaining entries for those not configured by the user, distribute these */
    LOG_CLI((BSL_META("after soc properties uMcQCtlRemainingFifos=%d\n"), uMcQCtlRemainingFifos));
    LOG_CLI((BSL_META("after soc properties uMcQCtlRemainingSize=%d\n"), uMcQCtlRemainingSize)); 
#endif

    /* This code distributes the entries equally between all remaining mcast pointer fifos in use */
    uMcQCtlSize = 6; /* max entries per fifo is 64 */
    while ((uMcQCtlRemainingFifos * (1<<uMcQCtlSize)) > uMcQCtlRemainingSize) {
        uMcQCtlSize--;
    }

#ifdef DEBUG_EG_FIFO_PTRS
    LOG_CLI((BSL_META("uMcQCtlSize=%d\n"), uMcQCtlSize)); 
#endif

    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
	if ((pQe2000Params->bEgressMcastEfDescFifoInUse[port]) && (pQe2000Params->uEgressMcastEfDescFifoSize[port] == -1)) { 
	    SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = uMcQCtlSize;
	}
	if ((pQe2000Params->bEgressMcastNefDescFifoInUse[port]) && (pQe2000Params->uEgressMcastNefDescFifoSize[port] == -1)) { 
	    SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = uMcQCtlSize;
	}
    }
#ifdef DEBUG_EG_FIFO_PTRS
    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
	LOG_CLI((BSL_META("qe2000_init hwQe2000InitParams_sp->uEgressMcastEfDescFifoSize[%d]=%d\n"), port, 
                 SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port]));
	LOG_CLI((BSL_META("qe2000_init hwQe2000InitParams_sp->uEgressMcastNefDescFifoSize[%d]=%d\n"), port, 
                 SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port]));
    }
#endif
    
    uMcQCtlRemainingFifos=2*(uNumPhySpiPorts[0]+uNumPhySpiPorts[1]+1);

    /* Error checks - valid values: uNumMcFifos<=100, uMcQCtlSize>=3 */
    
    uMcQCtlBase=0;
    uCurrentDropOnFull = uEgMcDropOnFull;

    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {

	/* Set up EF Fifo */
	uCurrentMcQCtlSize =  SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port];
	uCurrentMcQFifo = port;

	if ((uCurrentMcQCtlSize != -1) && (uCurrentMcQCtlSize != 0)) {

	    rv = soc_qe2000_mc_pointer_fifo_set((int)userDeviceHandle, uCurrentMcQFifo, uCurrentMcQCtlSize,
						uMcQCtlBase, uCurrentDropOnFull);
	    if (rv) {
	        QE2000PRINTF(QE2000_ERROR, ("ERROR: mc pointer fifo control write error fifo(%d)\n", uCurrentMcQFifo));
		return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
	    }    
	    uMcQCtlBase += (1 << uCurrentMcQCtlSize);
	}
	else {
	    /* Setup unused FIFOs to drop_on_full, 2 entries, and use unused QDAT entries */
	    rv = soc_qe2000_mc_pointer_fifo_set((int)userDeviceHandle, uCurrentMcQFifo, 1 /*size */, 1022 /* base */, TRUE /* drop_on_full */);    
	    if (rv) {
	        QE2000PRINTF(QE2000_ERROR, ("ERROR: mc pointer fifo control write error fifo(%d)\n", uCurrentMcQFifo));
		return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
	    }
	}

	/* Set up NEF Fifo */
	uCurrentMcQCtlSize =  SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port];
	uCurrentMcQFifo = port + 50;
	if ((uCurrentMcQCtlSize != -1) && (uCurrentMcQCtlSize != 0)) {

	    rv = soc_qe2000_mc_pointer_fifo_set((int)userDeviceHandle, uCurrentMcQFifo, uCurrentMcQCtlSize,
						uMcQCtlBase, uCurrentDropOnFull);    
	    uMcQCtlBase += (1 << uCurrentMcQCtlSize);
	}
	else {
	    /* Setup unused FIFOs to drop_on_full, 2 entries, and use unused QDAT entries */
	    rv = soc_qe2000_mc_pointer_fifo_set((int)userDeviceHandle, uCurrentMcQFifo, 1 /*size */, 1022 /* base */, TRUE /* drop_on_full */);    
	    if (rv) {
	        QE2000PRINTF(QE2000_ERROR, ("ERROR: mc pointer fifo control write error fifo(%d)\n", uCurrentMcQFifo));
		return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
	    }
	}
    }
    /**** END MC PORT POINTER FIFO DEPTHS ****/
    return rv;
}


/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitEg()
 *
 * OVERVIEW:        Initialize the QE2000 EG block
 *
 * ARGUMENTS:
 *
 *     Input parameters:
 *     TYPE            NAME               SET AT    DESCRIPTION
 *     ----            ----               ------    -----------
 *     sbBool_t        bFabCrc32          System    32/16 bit crc packet payload protection
 *     uint32        uEgMVTSize         System    0:12K, 1:24K, 2:48K
 *     uint32        uNumPhySpiPorts[2] Node      Number of physical ports attached to this QE egress per SPI (either enabled or not)
 *                                                  This sum of these 2 numbers should be 1, 2, 12, 24, 48. The setup always assumes a
 *                                                  The sum is used to allocate fifo sizes (with the satisfied threshold always set half way):
 *                                                 1: get entire eg data buffer - MVT size (assumes a single 10Gbps port)
 *                                                    PCI gets 40 buffers
 *                                                  2: get half eg data buffer per port - MVT size (assumes 2 10Gbps ports)
 *                                                    PCI gets 40 buffers
 *                                                  12: get 1/12th eg data buffer (- MVT size) per port (assumes 12 1 Gbps ports)
 *                                                 24: oversubscribed memory: 40 buffers per port (assumes 24 1Gbps ports- 1000 buffers allocated including PCI)
 *                                                 48: oversubscribed memory: 40 buffers per port (assumes 49 1Gbps ports- 1960 buffers allocated including PCI)
 *                                                 49: oversubscribed memory: 40 buffers per port (assumes 50 1Gbps ports- 2000 buffers allocated including PCI)
 *                                                     TBD: In the 49 port case, we're not sure if the single SPI port is 10G or 1G.
 *     uint32        uSpiSubportSpeed[2][49]   Node      Speed of SPI subport in Mbps, used for Egress fifo allocation
 *     uint32        uClockSpeedInMHz   System    clock speed in MHz
 *     uint32        uNodeId            Node      This is the node number of this QE2K
 *     uint32        uEgMcDropOnFull    Node      'drop on full' mask for all ports and the PCI.  When a bit is set in this mask, the associated
 *                                                  mutlicast pointer fifo is set to 'drop on full' mode.
 *     HW_QE2000_INIT_ST *pQe2000Params   Allows full access to global parameters
 *
 *
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the EG receive block.
 *
 *     NOTES:
 *     1) Uses "standard" fabric:
 *        2 fifos / fabric path (EF, nonEF)
 *       Fifo selection uses the following priority order:
 *        UC EF
 *        MC EF
 *        UC nonEF
 *        MC nonEF
 *     2) Loads port remap table as follows:
 *        EF fifo num = portnum * 2
 *       nonEF fifo num = portnum * 2 + 1
 *       note: fifos are only enabled for active ports (0 thru # active ports - 1)
 *             Also, PCI is always port 49.
 *     3) Sets MC to shape at the fifo head (versus the tail)
 *     4) Leaves the 2 MC vector tables to their default sizes: 64 for EF, 64 for nonEF
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitEg(sbhandle userDeviceHandle,
               sbBool_t bFabCrc32,
               sbBool_t bStandAlone,
               sbBool_t bHybrid,
               uint32 uEgMVTSize,
               uint32 uNumPhySpiPorts[2],
               uint32 uSpiSubportSpeed[2][49],
               uint32 uClockSpeedInMHz,
               uint32 uNodeId,
               uint32 uEgMcDropOnFull,
               HW_QE2000_INIT_ST *pQe2000Params)
{

#define MC_PORT_TIMEOUT 576000 /* nanoseconds: this is 8X the time for a jumbo frame to be XMIT'd out a 10Mbit port */
#define EF_MC_POINTERS_THRESH 200 /* SB_ASSERT EF MC full/almost full when 200 packets stored */
#define MC_POINTERS_THRESH 200  /* SB_ASSERT MC full/almost full when 200 packets stored */
#define MC_PAGES_HIGH 24 /* SB_ASSERT full/almost full when 24 pages used */
#define MC_PAGES_LOW 8 /* SB_ASSERT almost full when 8 pages used */
#define EG_MEM_TIMEOUT (4) /* 400 milliseconds */
#define EG_FP_INIT_TIMEOUT (4) /* 400 milliseconds */
#define EG_MIN_FREE_PAGES 32 /* allows for 32 KB of data to arrive after the global full thresh trips */

    uint32 uData, uData1, uData2;
    /*uint32 uMcEntriesPerFifo;*/
    uint32 uPortRemapAddr;
    uint32 uPortRemapFifoNum;
    uint32 uPortRemapFifoEnable;
    uint32 uPortRemapMcFifo;
    uint32 uTotalEgDataPages;
    uint32 uEgPagesPerPort;
    uint32 uEgPagesPerWeight;
    uint32 uEgFifoNum;
    uint32 uNumShapers;
    uint32 uTotalWeight;
    uint32 uCurrentSpi;
    uint32 uCurrentPort;
    uint32 uSpiSubportWeight[HW_QE2000_NUM_SPI_INTERFACES][HW_QE2000_NUM_SPI_SUBPORTS];
    uint32 uMcTimeout;
    uint32 rv;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EG_MC_CONFIG0, MC_DISABLE, 0x3);

    if (pQe2000Params->bEgressFifoIndependentFlowControl == FALSE) {
        uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, FULL_STATUS_MODE, 0, 0); /* 2 fifos per fab path */
    } else {
        /* configure to send 100 1 bit FULL status to SCI. This enables independent */
        /* flow control per FIFO. This requires Polaris based system                */
        uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, FULL_STATUS_MODE, 0, 3);
    }
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, HI_MC_PRI, uData, 
                               pQe2000Params->uEgHiMcPri);
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, LO_MC_PRI, uData, 
                               pQe2000Params->uEgLoMcPri);
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, QE2K_CRC32, uData, bFabCrc32);
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG0, TME_MODE, uData, 
			       (bStandAlone || bHybrid));
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_CONFIG0, uData);

    /* GNATS 24768, table size is 256, not 128 so to use full table, configure for 128 each */
    /* Set the size of the 2 MC vector tables to their default, 128 for EF 128 for non-EF */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VEC_TABLE, EF_BASE, 0, 0x0);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VEC_TABLE, BASE, uData, 0x8);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VEC_TABLE, EF_SIZE, uData, 0x8);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VEC_TABLE, SIZE, uData, 0x8);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_VEC_TABLE, uData);

    /* MC FIFO PARAMETERS */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, MCGROUP_SIZE, 0, uEgMVTSize);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, EF_MC_FIFO, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, EF_FIFO_ENABLE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, EF_FIFO_NUM, uData, 100);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, MC_FIFO, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, FIFO_ENABLE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, FIFO_NUM, uData, 101);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_CONFIG0, uData);

    /* MC TIMEOUT PARAMETERS */
    uData = 0;
    

    uMcTimeout = pQe2000Params->uEgMcEfTimeout;   /* nano-seconds */
    uMcTimeout = (uMcTimeout * uClockSpeedInMHz); /* number of cycles */
    uMcTimeout = uMcTimeout / 6;
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VECTOR_AGE, EF_TIMER, uData, (uMcTimeout & 0xFFFF));

    
    uMcTimeout = pQe2000Params->uEgMcNefTimeout;  /* nano-seconds */
    uMcTimeout = uMcTimeout * uClockSpeedInMHz; /* number of cycles */
    uMcTimeout = uMcTimeout / 6;
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_VECTOR_AGE, TIMER, uData,  (uMcTimeout & 0xFFFF));
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_VECTOR_AGE, uData);

    /* Note the vector timeout is disabled (this should not be necessary since we use vector aging) */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_TIMEOUT, ONE, 0, 0);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_TIMEOUT, ONE, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_TIMEOUT, uData);


    /* MC SOURCE ID REGISTERS */
    hwQe2000ConfigMcSrcId(userDeviceHandle, uNodeId, uNumPhySpiPorts[0]);

    /* Take EG block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, EG_CORE_RESET, 0);

    /* FIFO parameter set up */
    switch (uEgMVTSize) {
    case 0: /* MVT takes up 2K pages */
        uTotalEgDataPages=896;
        break;
    case 1: /* MVT takes up 4K pages */
        uTotalEgDataPages=768;
        break;
    default: /* MVT takes up 8K pages */
        uTotalEgDataPages=512;
        break;
    }

    uTotalWeight = 0;
    for (uCurrentSpi=0; uCurrentSpi<HW_QE2000_NUM_SPI_INTERFACES; uCurrentSpi++) {
	for (uCurrentPort=0; uCurrentPort<HW_QE2000_NUM_SPI_SUBPORTS; uCurrentPort++) {
	    uSpiSubportWeight[uCurrentSpi][uCurrentPort] = 0;
	}

	for (uCurrentPort=0; uCurrentPort<uNumPhySpiPorts[uCurrentSpi]; uCurrentPort++) {
	    if (uSpiSubportSpeed[uCurrentSpi][uCurrentPort] == 0) {
	    } else if (uSpiSubportSpeed[uCurrentSpi][uCurrentPort] <= 1000) {
		/* 1Gbps or less get weight 1 */
		uSpiSubportWeight[uCurrentSpi][uCurrentPort] = 1;
	    } else if (uSpiSubportSpeed[uCurrentSpi][uCurrentPort] <= 4000) {
		/* 1 ~ 4Gbps get weight 2 */
		uSpiSubportWeight[uCurrentSpi][uCurrentPort] = 2;
	    } else {
		/* Higher than 4Gbps get weight 4 */
		uSpiSubportWeight[uCurrentSpi][uCurrentPort] = 4;
	    }
	    uTotalWeight += uSpiSubportWeight[uCurrentSpi][uCurrentPort];
	}
    }

    if (uTotalWeight == 0) {
	uTotalWeight = 1;
    }

    uEgPagesPerWeight=(uTotalEgDataPages-40)/uTotalWeight; /* deduct 40 for PCI */

#if 01 /* AB501 090809 MC thresholds are not adequate for single-spi 10Gig MC 
	  operation (20Gig prior to source  knockout)  and jumbos - doubling
	  the thresholds fixes the throughput problem.

	  It's possible that if 2x10Gig ports are supported by QE, 
	  then we need to double once more, or more precisely, the weight
	  of the MC should be the same as the max weight of any one SPI channel */

    /* SET UP MC FIFO THRESHOLDS */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, POINTERS, 0, MC_POINTERS_THRESH);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_HIGH, uData, uEgPagesPerWeight*2);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_LOW, uData, uEgPagesPerWeight);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_THRESH, uData);

    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, POINTERS, 0, EF_MC_POINTERS_THRESH);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_HIGH, uData, uEgPagesPerWeight*2);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_LOW, uData, uEgPagesPerWeight);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_EF_THRESH, uData);

    /* Bug 23867 SET UP MC OVERFLOW THRESHOLDS TO BE LARGER */
    /* 18 pages larger - 1 jumbo frame + ~3 timeslots latency through the fabric */

    /* drop EF MC packet if it requires the EF MC fifo to use > pages + 20 */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, EF_THRESH, 0, uEgPagesPerWeight*2+20);
    /* drop MC packet if it requires the EF MC fifo to use > pages + 20 */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, THRESH, uData, uEgPagesPerWeight*2+20);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_OVERFLOW, uData);

#else

    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, POINTERS, 0, MC_POINTERS_THRESH);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_HIGH, uData, uEgPagesPerWeight);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_LOW, uData, uEgPagesPerWeight/2);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_THRESH, uData);

    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, POINTERS, 0, EF_MC_POINTERS_THRESH);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_HIGH, uData, uEgPagesPerWeight);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_LOW, uData, uEgPagesPerWeight/2);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_EF_THRESH, uData);


    /* Bug 23867 SET UP MC OVERFLOW THRESHOLDS TO BE LARGER */
    /* 18 pages larger - 1 jumbo frame + ~3 timeslots latency through the fabric */

    /* drop EF MC packet if it requires the EF MC fifo to use > pages + 20 */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, EF_THRESH, 0, uEgPagesPerWeight+20);
    /* drop MC packet if it requires the EF MC fifo to use > pages + 20 */
    uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, THRESH, uData, uEgPagesPerWeight+20);
    SAND_HAL_WRITE(userDeviceHandle, KA, EG_MC_OVERFLOW, uData);

#endif



    /*
     * Initialize all shapers to default values. By default all shapers
     * are disabled.
     */
    for (uNumShapers = 0; uNumShapers < 152; uNumShapers++) {
        uData = uData1 = uData2 = 0;
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA0, uData);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA1, uData1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA2, uData2);

        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x5);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, uNumShapers);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_EG_MEM_ACC_CTRL_OFFSET,
                           0x80000000,
                            EG_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: init eg shaper memory timeout\n"));
            return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);
    }

    /* SET UP UC FIFO THRESHOLDS */
    /* (shouldn't matter that unused fifos are set up with thresholds */
    for (uEgFifoNum=0; uEgFifoNum<100; uEgFifoNum++) {
        if (uEgFifoNum>=98) {
	    /* If the PCI fifo, always set the # of pages to 40 */
            uEgPagesPerPort = 40;
	} else {
	    /* 2 Fifos for each port */
	    uCurrentPort = (uEgFifoNum >> 1);

	    /* Allocate fifo size based on its speed */
	    if (uCurrentPort < uNumPhySpiPorts[0]) {
		uEgPagesPerPort = uEgPagesPerWeight * uSpiSubportWeight[0][uCurrentPort];
	    } else {
		uEgPagesPerPort = uEgPagesPerWeight * uSpiSubportWeight[1][uCurrentPort - uNumPhySpiPorts[0]];
	    }

	    /* Unused fifos also setup some threshold */
	    if ( uEgPagesPerPort == 0) {
		uEgPagesPerPort = 40;
	    }
	}

        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA0, DATA, 0,
                                   ((uEgPagesPerPort << 26) |  /* thresh_hi[5:0] */
                                    (uEgPagesPerPort/2)<<16 |  /* thresh_lo */
                                    (0xff)<<8 |                /* shaper_1 (disabled) */
                                    0xff));                    /* shaper_0 (disabled) */


        QE2000PRINTF(QE2000_DEBUG_FUNC, ("Egress Fifo     : 0x%x\n", uEgFifoNum));
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("uEgPagesPerPort = 0x%x\n", uEgPagesPerPort));
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("eg_mem_acc_data0= 0x%x\n", uData));

        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA0, uData);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA1, DATA, 0,
                                   uEgPagesPerPort>>6); /* thresh_hi[9:6] */
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA1, uData);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x2);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, uEgFifoNum);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_EG_MEM_ACC_CTRL_OFFSET,
                           0x80000000,
                            EG_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: init eg memory timeout\n"));
            return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);
    }

    rv = hwQe2000InitEgMcPtrFifos(userDeviceHandle, uNumPhySpiPorts, uEgMcDropOnFull, pQe2000Params); 
    if (rv) {
	QE2000PRINTF(QE2000_ERROR, ("ERROR: init eg mcast fifo pointer setup error\n"));
	return rv;
    }

    /* disable shapers for multicast */
    for (uEgFifoNum=100; uEgFifoNum<200; uEgFifoNum++) {
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA0, DATA, 0,
                                    (0xff)<<8 |                /* shaper_1 (disabled) */
                                    0xff);                     /* shaper_0 (disabled) */

        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA0, uData);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA1, DATA, 0, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA1, uData);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x2);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, uEgFifoNum);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_EG_MEM_ACC_CTRL_OFFSET,
                           0x80000000,
                            EG_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: timeout writing to disable eg shapers\n"));
            return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);
    }

    /* INIT FREE PAGE LIST */
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG1, INIT_FREE_PAGE, 0, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_CONFIG1, FREE_PAGE_NUM, uData, uTotalEgDataPages-1);

    SAND_HAL_WRITE(userDeviceHandle, KA, EG_CONFIG1, uData);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_EG_CONFIG1_OFFSET,
                        SAND_HAL_KA_EG_CONFIG1_INIT_FREE_PAGE_DONE_MASK,
                        EG_FP_INIT_TIMEOUT)) {
        QE2000PRINTF(QE2000_ERROR, ("ERROR: free page list write timeout\n"));
        return HW_QE2000_STS_INIT_EG_FP_INIT_TIMEOUT_ERR_K;
    }

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EG_CONFIG1, MIN_FREE_PAGES, EG_MIN_FREE_PAGES);

    /* SETUP THE PORT REMAPPING TABLE */
    for (uPortRemapAddr=0; uPortRemapAddr<0x400; uPortRemapAddr++) { /* uPortRemapAddr is made up of {0, txmda, ef, qe1k, port[5:0]} */

        if (((uPortRemapAddr>>6) & 0x1) == 0x1) {
            int     nPortTmpAddr;

            nPortTmpAddr = uPortRemapAddr & 0x3f;

            /* QE1k bit is set, QE1000 remapping below */
            uPortRemapFifoEnable=1;
            uPortRemapMcFifo=0;
            if ( ((nPortTmpAddr & 0x1F) >= 18) && ((nPortTmpAddr & 0x1F) <= 21) ) {
                /* this is a qe1k multicast port */
                uPortRemapMcFifo = 1;
            }
            else if ( (nPortTmpAddr >= 22) && (nPortTmpAddr <= 31) ) {
                nPortTmpAddr = nPortTmpAddr - 4;
            } else if (nPortTmpAddr & 0x20) { /* node 5 bit */
                nPortTmpAddr = nPortTmpAddr & 0x1F;
                if (nPortTmpAddr >= 0 && nPortTmpAddr <= 17){
                    nPortTmpAddr = nPortTmpAddr + 28;
                } else if (nPortTmpAddr >= 22 && nPortTmpAddr <= 25){
                    nPortTmpAddr = nPortTmpAddr + 24;
                }
            }

            if ( uPortRemapMcFifo ) {
                if ((uPortRemapAddr>>7) & 0x1) {                     /* EF bit is set */
                    uPortRemapFifoNum = 100;
                } else {                                               /* EF bit is not set */
                    uPortRemapFifoNum = 101;
                }
            } else {
                if ((uPortRemapAddr>>7) & 0x1) {                     /* EF bit is set */
                    uPortRemapFifoNum=2*(nPortTmpAddr & 0x3f);   /* fifo is port# * 2 */
                } else {                                               /* EF bit is not set */
                    uPortRemapFifoNum=2*(nPortTmpAddr & 0x3f)+1; /* fifo is port# * 2 + 1 */
                }
            }
        } else {
            /* QE1k bit is clear, QE2000 remapping below */
        if ( 0 &&
            ((uPortRemapAddr & 0x3f) >= (uNumPhySpiPorts[0]+uNumPhySpiPorts[1])) && /* not a SPI port and */
            (uPortRemapAddr & 0x3f) < 49) {                                         /* not the PCI port */
            uPortRemapFifoNum=0;
            uPortRemapFifoEnable=0;
            uPortRemapMcFifo=0;
        } else { /* port number is valid SPI or PCI port */
            uPortRemapFifoEnable=1;
            uPortRemapMcFifo=0;
            if ((uPortRemapAddr>>7) & 0x1)                     /* EF bit is set */
                uPortRemapFifoNum=2*(uPortRemapAddr & 0x3f);   /* fifo is port# * 2 */
            else                                               /* EF bit is not set */
                uPortRemapFifoNum=2*(uPortRemapAddr & 0x3f)+1; /* fifo is port# * 2 + 1 */
        }
        }

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA0, DATA, uData,
                                   (uPortRemapMcFifo<<8 |
                                    uPortRemapFifoEnable<<7 |
                                    uPortRemapFifoNum));
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA0, uData);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, uPortRemapAddr);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_EG_MEM_ACC_CTRL_OFFSET,
                           0x80000000,
                            EG_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: eg memory write timeout location (0x%x)\n", uPortRemapAddr));
            return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);
    }

    {
        int32 nMaxMvtIndex;
        int32 nIndex;
        HW_QE2000_MVT_ENTRY_ST mvtEntry;

        {
            /* init the mvtEntry to have no ports, source knockout on, and no chained entry */
            int32 nPort;
            for ( nPort=0; nPort<HW_QE2000_MAX_NUM_PORTS_K; nPort++ ) {
                mvtEntry.bPortEnable[nPort] = FALSE;
            }

            mvtEntry.ulMvtdA = 0;
            mvtEntry.ulMvtdB = 0;
            mvtEntry.bSourceKnockout = FALSE;
            mvtEntry.ulNext = 0xffff;
        }

        uData = SAND_HAL_READ(userDeviceHandle, KA, EG_MC_CONFIG0);
        uData = SAND_HAL_GET_FIELD(KA, EG_MC_CONFIG0, MCGROUP_SIZE, uData);
        switch ( uData ) {
            case 0:
                nMaxMvtIndex = 12*1024;
                break;
            case 1:
                nMaxMvtIndex = 24*1024;
                break;
            case 2:
                nMaxMvtIndex = 48*1024;
                break;
            default:
                nMaxMvtIndex = 12*1024; /* should never get here, pick safe value */
        }

        QE2000PRINTF(QE2000_DEBUG_FUNC, ("Initing %d MVT entries...",nMaxMvtIndex));
        for ( nIndex=0; nIndex<nMaxMvtIndex; nIndex++ ) {
            /* write with no sem lock/unlock */
            mvtEntry.ulNext = 0xFFFF;
            hwQe2000MVTSet(userDeviceHandle, mvtEntry, nIndex, NULL, NULL, 0, 1000, NULL);
        }
        QE2000PRINTF(QE2000_DEBUG_FUNC, ("done.\n"));

    }

    /* Clear FIFO Control Memory */
    for (uEgFifoNum=0; uEgFifoNum<200; uEgFifoNum++) {

        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA0, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA1, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_DATA2, 0);

        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x1); /* fifo control memory is 0x1 */
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, uEgFifoNum);

        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

        if (hwQe2000RegDone(userDeviceHandle,
                           SAND_HAL_KA_EG_MEM_ACC_CTRL_OFFSET,
                           0x80000000,
                            EG_MEM_TIMEOUT)) {
            QE2000PRINTF(QE2000_ERROR, ("ERROR: eg fifo control memory timeout fifo (0x%x)\n", uEgFifoNum));
            return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
        }

        /* clear req/ack bits */
        uData=0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uData, 1);
        SAND_HAL_WRITE(userDeviceHandle, KA, EG_MEM_ACC_CTRL, uData);

    }

    /* Enable Global Shaper. Enabled after all shapers are disabled and  */
    /* Fifo Param Table entry points to NULL shapers                     */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EG_CONFIG0, SHAPER_ENABLE, 0x1);

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EG_MC_CONFIG0, MC_DISABLE, 0x0);

    return HW_QE2000_STS_OK_K;
}
/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitEb()
 *
 * OVERVIEW:        Initialize the QE2000 EB block
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the EB block.
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitEb(sbhandle userDeviceHandle)
{
    int i, j;
    uint32 v;

    /** Initialize the EB receive block.

      Input parameters:
      TYPE            NAME               SET AT    DESCRIPTION
      ----            ----               ------    -----------
    */
    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EB_CONFIG, ENABLE, 1);

    /* Take EB block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, EB_CORE_RESET, 0);

    /* Clear the EB */
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE0, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE1, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE2, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE3, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE4, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE5, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE6, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_SLICE7, 0);

    v = SAND_HAL_KA_EB_MEM_ACC_CTRL_WRITE_MASK_MASK
      | SAND_HAL_KA_EB_MEM_ACC_CTRL_REQ_MASK
      | SAND_HAL_KA_EB_MEM_ACC_CTRL_ACK_MASK;
    for (i = 0; i <= SAND_HAL_KA_EB_MEM_ACC_CTRL_ADDR_MASK; i++) {
      SAND_HAL_WRITE(userDeviceHandle, KA, EB_MEM_ACC_CTRL, (v | i));
      for (j = 0; j < 1000; j++) {
        if (SAND_HAL_READ_POLL(userDeviceHandle, KA, EB_MEM_ACC_CTRL)
            & SAND_HAL_KA_EB_MEM_ACC_CTRL_ACK_MASK) {
          break;
        }
      }
      if (j == 1000) {
        return HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K;
      }
    }
    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitEp()
 *
 * OVERVIEW:        Initialize the QE2000 EP block
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the EP block.
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitEp(sbhandle userDeviceHandle, 
	       sbBool_t bEpDisable,
               uint32 uNumPhySpiPorts[2],
               uint64   uuRequeuePortsMask[2],
               uint32 uPacketAdjustFormat)
{

    /** Initialize the EP receive block.

      Input parameters:
      TYPE            NAME               SET AT    DESCRIPTION
      ----            ----               ------    -----------

      notes:
      The EP is taken out of reset, but left disabled (transparent to the data path).
      If the ERH stripping function is required, the ELIB must be used.
    */

#define EP_BF_MEM_TIMEOUT (4) /* 400 milliseconds */

    uint32 uData;
    uint32 uVlanId;
    uint32 uCurrentSpi;
    uint32 uCurrentPort;
    uint32 uPhysicalPort;

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));

    /* Take EP block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, EP_CORE_RESET, 0);

    /* The following sets up the EP-BF for ERH stripping and VLAN tag stripping */
    /* It also enables all ports on all 4k VLANs */

    /* Do not bypass the BF (required for ERH stripping) */
    /* Joey ported from sdk branch -Bypassing the BF for now, ERH wanted for CPU txrx */
    uData=SAND_HAL_READ(userDeviceHandle, KA, EP_CONFIG);
    uData= (bEpDisable == 0) ? SAND_HAL_MOD_FIELD(KA, EP_CONFIG, ENABLE, uData, 1) :
                               SAND_HAL_MOD_FIELD(KA, EP_CONFIG, ENABLE, uData, 0);
    uData=SAND_HAL_MOD_FIELD(KA, EP_CONFIG, BYPASS_IP, uData, 1);
    uData=SAND_HAL_MOD_FIELD(KA, EP_CONFIG, BYPASS_CL, uData, 1);
    uData=SAND_HAL_MOD_FIELD(KA, EP_CONFIG, BYPASS_BF, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_CONFIG, uData);

    /* Set up the port fifo thresholds */
    uData=SAND_HAL_READ(userDeviceHandle, KA, EP_PF_THRESHOLD);
    uData=SAND_HAL_MOD_FIELD(KA, EP_PF_THRESHOLD, REQUEST, uData, 0x32);
    uData=SAND_HAL_MOD_FIELD(KA, EP_PF_THRESHOLD, DATA, uData, 0x60);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_PF_THRESHOLD, uData);

    /* set up the VLAN records in slim mode */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG, LEN_ADJ_POSN, uData, uPacketAdjustFormat);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_CONFIG, 0);

    /* Enable all ports for bridging functions */
    uData=SAND_HAL_READ(userDeviceHandle, KA, EP_BF_PORT0);
    uData=SAND_HAL_MOD_FIELD(KA, EP_BF_PORT0, ENABLE, uData, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PORT0, uData);

    uData=SAND_HAL_READ(userDeviceHandle, KA, EP_BF_PORT1);
    uData=SAND_HAL_MOD_FIELD(KA, EP_BF_PORT1, ENABLE, uData, 0x3ffff);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PORT1, uData);

    /* Disable ERH (i.e. strip ERH) for all ports but enable for CPU */
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_ERH0, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_ERH1, 0x20000);

    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PRI_REWRITE0, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PRI_REWRITE1, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_TAG0, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_TAG1, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_SMAC_MC0, 0);
    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_SMAC_MC1, 0);

    /* set up all VLAN records in default mode where all ports are managed */
    /* and outgoing vlan id is overwritten with incoming vlan id */
    for (uVlanId=0; uVlanId<4096; uVlanId=uVlanId+1) {

      /* Entry 0 of the slim record */
      uData=0;
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA0, 0xaaaaaaaa);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA1, 0xaaaaa);

      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, REQ, uData, 1);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, uData, 0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, uData, 0x0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uData, uVlanId*4);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      if (hwQe2000RegDone(userDeviceHandle,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_OFFSET,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_ACK_MASK,
                          EP_BF_MEM_TIMEOUT)) {
          QE2000PRINTF(QE2000_ERROR, ("ERROR: ep init memory timeout location(0x%x)\n", uVlanId*4));
          return HW_QE2000_STS_INIT_EP_INIT_TIMEOUT_ERR_K;
      }

      /* clear req/ack bits */
      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ACK, uData, 1);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);


      /* Entry 1 of the slim record */
      uData=0;
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA0, 0xaaaaaaaa);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA1, 0xaaaaaa);

      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, REQ, uData, 1);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, uData, 0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, uData, 0x0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uData, uVlanId*4+1);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      if (hwQe2000RegDone(userDeviceHandle,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_OFFSET,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_ACK_MASK,
                          EP_BF_MEM_TIMEOUT)) {
          QE2000PRINTF(QE2000_ERROR, ("ERROR: ep init memory timeout location(0x%x)\n", uVlanId*4+1));
          return HW_QE2000_STS_INIT_EP_INIT_TIMEOUT_ERR_K;
      }

      /* clear req/ack bits */
      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ACK, uData, 1);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      /* Entry 2 of the slim record */
      uData=0;
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA0, 0);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA1, 0);

      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, REQ, uData, 1);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, uData, 0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, uData, 0x0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uData, uVlanId*4+2);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      if (hwQe2000RegDone(userDeviceHandle,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_OFFSET,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_ACK_MASK,
                          EP_BF_MEM_TIMEOUT)) {
          QE2000PRINTF(QE2000_ERROR, ("ERROR: EP BF memory write timeout location(0x%x)\n", uVlanId*4+2));
          return HW_QE2000_STS_INIT_EP_INIT_TIMEOUT_ERR_K;
      }

      /* clear req/ack bits */
      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ACK, uData, 1);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      /* Entry 3 of the slim record */
      uData=0;
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA0, 0);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_DATA1, 0);

      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, REQ, uData, 1);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, uData, 0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, uData, 0x0);
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uData, uVlanId*4+3);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);

      if (hwQe2000RegDone(userDeviceHandle,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_OFFSET,
                          SAND_HAL_KA_EP_MM_BF_MEM_ACC_CTRL_ACK_MASK,
                          EP_BF_MEM_TIMEOUT)) {
          QE2000PRINTF(QE2000_ERROR, ("ERROR: EP MM BF memory write timeout location(0x%x)\n", uVlanId*4+3));
          return HW_QE2000_STS_INIT_EP_INIT_TIMEOUT_ERR_K;
      }

      /* clear req/ack bits */
      uData=0;
      uData = SAND_HAL_MOD_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ACK, uData, 1);
      SAND_HAL_WRITE(userDeviceHandle, KA, EP_MM_BF_MEM_ACC_CTRL, uData);
    }

    for (uCurrentSpi=0; uCurrentSpi<HW_QE2000_NUM_SPI_INTERFACES; uCurrentSpi++) {
	for (uCurrentPort=0; uCurrentPort<uNumPhySpiPorts[uCurrentSpi]; uCurrentPort++) {
            if (COMPILER_64_BITTEST(uuRequeuePortsMask[uCurrentSpi], uCurrentPort)) {
		uPhysicalPort = uCurrentPort;
		assert(uCurrentSpi < 2); /* this doesn't support more than two physical interfaces */
		if (uCurrentSpi == 1) {
		    uPhysicalPort += uNumPhySpiPorts[0];
		}
		assert(uPhysicalPort < 64); /* we don't expect more than 50 */
		if (uPhysicalPort < 32) {
		    uData = SAND_HAL_READ(userDeviceHandle, KA, EP_BF_PORT_MIRROR0);
		    uData |= (1 << uPhysicalPort);
		    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PORT_MIRROR0, uData);
		    uData = SAND_HAL_READ(userDeviceHandle, KA, EP_BF_ERH0);
		    uData |= (1 << uPhysicalPort);
		    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_ERH0, uData);
		} else {
		    uData = SAND_HAL_READ(userDeviceHandle, KA, EP_BF_PORT_MIRROR1);
		    uData |= (1 << (uPhysicalPort - 32));
		    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_PORT_MIRROR1, uData);
		    uData = SAND_HAL_READ(userDeviceHandle, KA, EP_BF_ERH1);
		    uData |= (1 << (uPhysicalPort - 32));
		    SAND_HAL_WRITE(userDeviceHandle, KA, EP_BF_ERH1, uData);
		}
	    }
	}
    }



    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InitEi()
 *
 * OVERVIEW:        Initialize the QE2000 EI block
 *
 * ARGUMENTS:
 *      Input parameters:
 *     TYPE            NAME                  SET AT    DESCRIPTION
 *     ----            ----                  ------    -----------
 *     uint32        uNumPhySpiPorts[2]     Node      Number of physical ports attached to this QE egress per SPI (regardless enabled or not)
 *     uint32        uSpiSubportSpeed[2][49]   Node      Speed of SPI subport in Mbps, used for Egress fifo allocation
 *     uint32        uEiPortInactiveTimeout Node      Time period for a port to remain idle while data is available before the port is
 *                                                      timed out in units of ~16 milliseconds (there is error based on the exact clock period--
 *                                                      at 250MHz core clock, the units are 8.4milliseconds)
 *     uint32        uClockSpeedInMHz       System    clock speed in MHz
 *     HW_QE2000_INIT_ST *pQe2000Params                 Allows full access to global parameters
 *     sbBool_t        bEiFullPacketMode      Node      Run in full packet mode or burst interleaved packet mode
 *
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Initialize the SV block.
 *
 *      notes:
 *     The EI is brought up in a standard configuration:
 *        -Burst interleave mode with parameterizable # of ports per SPI
 *        -mirroring turned off
 *       -Assumed EP is disabled (i.e. can't add bytes to head of packet)
 *       -No SPI channel remapping
 *       -PCI is port 49
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
static uint32
hwQe2000InitEi(sbhandle userDeviceHandle,
               uint32 uNumPhySpiPorts[2],
               uint32 uSpiSubportSpeed[2][49],
               uint32 uEiPortInactiveTimeout,
               uint32 uClockSpeedInMHz,
               uint32 bEiSpiFullPacketMode[2],
               HW_QE2000_INIT_ST *pQe2000Params)
{
#define EI_MEM_TIMEOUT (4) /* 400 milliseconds */

    uint32 uData;
    uint32 uEiPortInactiveTimeoutUnits;
    uint32 uCurrentSpiPort;
    uint32 uCurrentEgPort;
    uint32 uCurrentSpiInterface;
    uint32 uEiPorts31_0Enable;
    uint32 uEiPorts47_32Enable;
    uint32 uLinesPerWeight;
    uint32 uCurrentDestChannel;
    uint32 uCurrentLinePtr;
    uint32 uNextSpiLinePtr=0;
    uint32 uCurrentSpiPortLines;
    uint32 uSpiPortLines[2][49];
    uint32 uSpiPortStartLine[2][49];
    uint32 uEiMemSel;
    int32 nRequeueDestPort=-1;
    int32 nRequeueLastSpiBus=0;
    uint32 uMaxPortSpeed[2];

    QE2000PRINTF(QE2000_DEBUG_API, ("%s\n", FUNCTION_NAME()));
    uEiPortInactiveTimeoutUnits=(0x0f*uClockSpeedInMHz)/250; /* scale timeout units based on the actual clock speed */
                                                      /* to hit ~8.4ms */
    uData = SAND_HAL_MOD_FIELD(KA, EI_CONFIG, PORT_INACTIVE_TIMEOUT_UNITS, 0, uEiPortInactiveTimeoutUnits);
    uData = SAND_HAL_MOD_FIELD(KA, EI_CONFIG, PORT_INACTIVE_TIMEOUT_ENABLE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EI_CONFIG, PORT_INACTIVE_TIMEOUT_PERIOD, uData, uEiPortInactiveTimeout);
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_CONFIG, uData);

    /* SET UP SPI0 */
    uData = SAND_HAL_READ(userDeviceHandle, KA, EI_SPI0_CONFIG0);

    if (bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_FULL) {
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, INTERLEAVE_MODE, uData, 2); /* full packet interleave mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_TYPE, uData, 0); /* packet interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_SIZE, uData, pQe2000Params->uInterleaveBurstSize[0]);
    } else if (bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_N_1) {
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, FLOW_CTRL, uData, 3); /* ignore ei flow control */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, INTERLEAVE_MODE, uData, 1); /* N:1 mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_TYPE, uData, 0); /* packet interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_SIZE, uData, pQe2000Params->uInterleaveBurstSize[0]);
    } else { /* SOC_SBX_PORT_MODE_BURST_IL */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, INTERLEAVE_MODE, uData, 0); /* burst interleave mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_TYPE, uData, 1); /* burst interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_SIZE, uData, (uNumPhySpiPorts[0])>2?5:10); /* ab 100405  >2 due to dual 10gig*/
    }

    uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, MAX_INSERTED_LINES, uData, 4); /* leave as default (even though EP doesn't add lines, this shouldn't hurt) */
    uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, PORT_OFFSET, uData, 0); /* SPI0 always starts at eg port 0 */

    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI0_CONFIG0, uData);

    uEiPorts31_0Enable=0;
    uEiPorts47_32Enable=0;
    for (uCurrentSpiPort=0; uCurrentSpiPort<(uNumPhySpiPorts[0]); uCurrentSpiPort++) {
        if (uCurrentSpiPort<32)
            uEiPorts31_0Enable = uEiPorts31_0Enable | (0x1<<uCurrentSpiPort);
        else
            uEiPorts47_32Enable = uEiPorts47_32Enable | (0x1<<(uCurrentSpiPort-32));
    }

    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI0_CONFIG1, uEiPorts31_0Enable);
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI0_CONFIG2, uEiPorts47_32Enable);

    /* SET UP SPI1 */
    uData = SAND_HAL_READ(userDeviceHandle, KA, EI_SPI1_CONFIG0);

    if (bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_FULL) {
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, INTERLEAVE_MODE, uData, 2); /* full packet interleave mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_TYPE, uData, 0); /* packet interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_SIZE, uData, pQe2000Params->uInterleaveBurstSize[1]);
    } else if (bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_N_1) {
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, FLOW_CTRL, uData, 3); /* ignore ei flow control */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, INTERLEAVE_MODE, uData, 1); /* N:1 mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_TYPE, uData, 0); /* packet interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_SIZE, uData, pQe2000Params->uInterleaveBurstSize[1]);
    } else { /* SOC_SBX_PORT_MODE_BURST_IL */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, INTERLEAVE_MODE, uData, 0); /* burst interleave mode */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_TYPE, uData, 1); /* burst interleave mode request type */
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_SIZE, uData, uNumPhySpiPorts[1]>2?5:10); /* ab 100405 if 1gig ports, it's 5, otherwise it's 10 >2 due to dual 10gig*/
    }
    uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, MAX_INSERTED_LINES, uData, 4); /* leave as default (even though EP doesn't add lines, this shouldn't hurt) */
    uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, PORT_OFFSET, uData, uNumPhySpiPorts[0]); /* SPI1 port #'s always start where SPI0 left off */
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI1_CONFIG0, uData);

    uEiPorts31_0Enable=0;
    uEiPorts47_32Enable=0;

    for (uCurrentSpiPort=0; uCurrentSpiPort<uNumPhySpiPorts[1]; uCurrentSpiPort++) {
        if (uCurrentSpiPort<32)
            uEiPorts31_0Enable = uEiPorts31_0Enable | (0x1<<uCurrentSpiPort);
        else
            uEiPorts47_32Enable = uEiPorts47_32Enable | (0x1<<(uCurrentSpiPort-32));
    }

    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI1_CONFIG1, uEiPorts31_0Enable);
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI1_CONFIG2, uEiPorts47_32Enable);

    /* auto resume required to allow for downstream port removal and re-insert */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EI_EG_PORTS_CONFIG1, PAUSE_MC_AUTO_RESUME, 1);

    /* SET UP PCI PORT */
    uData = SAND_HAL_READ(userDeviceHandle, KA, EI_PCI_CONFIG);
    uData = SAND_HAL_MOD_FIELD(KA, EI_PCI_CONFIG, ENABLE, uData, 1); /* set the enable bit, all other fields remain at their default */
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_PCI_CONFIG, uData);

   /* Bug 24539 Disable redundant memory */
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_MEM_DEBUG2, 0xffff);

    if ( pQe2000Params->bWorkaround25276Enable ) {
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EI_MEM_DEBUG0, SPI0_RD_PTR_MEM_DISABLE_ECC, 1);
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EI_MEM_DEBUG0, SPI1_RD_PTR_MEM_DISABLE_ECC, 1);	
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EI_MEM_DEBUG1, DD_WR_PTR_MEM_DISABLE_ECC, 1);
    }

    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, EI_BIST_CONFIG0, BIST_SETUP, 2);
    uData = 0xe7ffffff;
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_BIST_CONFIG1, uData);

    if (hwQe2000RegDone(userDeviceHandle,
                        SAND_HAL_KA_EI_BIST_STATUS0_OFFSET,
                        0xe7ffffff,
                        INTERNAL_MEMBIST_TIMEOUT)) {

        QE2000PRINTF(QE2000_ERROR, ("ERROR: EI memory bist timeout\n"));
        return HW_QE2000_STS_INIT_EI_INTERNAL_MEMBIST_TIMEOUT_ERR_K;
    }

    /* MCM - move this to after MBIST is done */
    /* Take EI block out of soft reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, EI_CORE_RESET, 0);

    /* Clear MBIST */
    SAND_HAL_WRITE(userDeviceHandle, KA, EI_BIST_CONFIG1, 0);

    /* SET UP SHALLOW FIFOS */
    for (uCurrentSpiInterface=0; uCurrentSpiInterface<2; uCurrentSpiInterface++) {
	/* Fifo size allocation */
	int32 nWeight;
        int32 nNumPhysicalSpiPorts = uNumPhySpiPorts[uCurrentSpiInterface];
	uint32 uSpiPortWeights[49];
	uint32 uSpiPortTotalWeight;
	uint32 uRoughLinesPerWeight;
	uint32 uMinPortSpeed = 0xFFFFFFFF;
	uMaxPortSpeed[uCurrentSpiInterface] = 0;

	for ( uCurrentSpiPort=0; uCurrentSpiPort<49; uCurrentSpiPort++) {
	    uSpiPortWeights[uCurrentSpiPort] = 0;
	    uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort] = 0;
	    uSpiPortStartLine[uCurrentSpiInterface][uCurrentSpiPort] = 0;
	}

	for ( uCurrentSpiPort=0; uCurrentSpiPort<nNumPhysicalSpiPorts; uCurrentSpiPort++) {
	    if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] != 0) &&
		(uMinPortSpeed > uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort])) {
		uMinPortSpeed = uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort];
	    }
	    if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] != 0) &&
		(uMaxPortSpeed[uCurrentSpiInterface] < uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort])) {
		uMaxPortSpeed[uCurrentSpiInterface] = uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort];
	    }
	}

	uSpiPortTotalWeight = 0;
	for ( uCurrentSpiPort=0; uCurrentSpiPort<nNumPhysicalSpiPorts; uCurrentSpiPort++) {
	    /* Fifo size has to be power of 2 and natual aligned */
	    if (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] == 0) {
		uSpiPortWeights[uCurrentSpiPort] = 0;
	    } else if (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] < 2 * uMinPortSpeed) {
		uSpiPortWeights[uCurrentSpiPort] = 1;
	    } else if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] >= 2 * uMinPortSpeed) &&
		       (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] < 4 * uMinPortSpeed)) {
		uSpiPortWeights[uCurrentSpiPort] = 2;
	    } else if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] >= 4 * uMinPortSpeed) &&
		       (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] < 8 * uMinPortSpeed)) {
		uSpiPortWeights[uCurrentSpiPort] = 4;
	    } else if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] >= 8 * uMinPortSpeed) &&
		       (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] < 16 * uMinPortSpeed)) {
		if (uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] >= 8000) {
		    /* If the port speed is higher than 8Gbps, give it more weight */
		    uSpiPortWeights[uCurrentSpiPort] = 16;
		} else {
		    uSpiPortWeights[uCurrentSpiPort] = 8;
		}
	    } else if ((uSpiSubportSpeed[uCurrentSpiInterface][uCurrentSpiPort] >= 16 * uMinPortSpeed)) {
		uSpiPortWeights[uCurrentSpiPort] = 16;
	    }
	    uSpiPortTotalWeight += uSpiPortWeights[uCurrentSpiPort];
	}

        if ( nNumPhysicalSpiPorts == 0 ) {
            uLinesPerWeight = 0;
        } else {
            uRoughLinesPerWeight =
                pQe2000Params->uEiLines[uCurrentSpiInterface]
                / uSpiPortTotalWeight;
            uLinesPerWeight = (0x1 << 30);

            /* find the closest power of two that is
	     * less than or equal to the rough calculation
	     */
            while ( uLinesPerWeight > uRoughLinesPerWeight ) {
                uLinesPerWeight = uLinesPerWeight >> 1;
            }

        }

	/* Calculate Fifo Size */
	for ( uCurrentSpiPort=0; uCurrentSpiPort<nNumPhysicalSpiPorts; uCurrentSpiPort++) {
	    if ( (uLinesPerWeight * uSpiPortWeights[uCurrentSpiPort]) == 0 ) {
		uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort] = 0;
	    } else if ( (uLinesPerWeight * uSpiPortWeights[uCurrentSpiPort]) < 4 ) {
		/* Min 4 lines */
		uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort] = 4;
	    } else if ( (uLinesPerWeight * uSpiPortWeights[uCurrentSpiPort]) > 256 ) {
		/* Max 256 lines */
		uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort] = 256;
	    } else {
		/* both uLinesPerWeight and uSpiPortWeights[uCurrentSpiPort] are power of 2 */
		uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort] = uLinesPerWeight * uSpiPortWeights[uCurrentSpiPort];
	    }
	}

	/* To ensure natual alignment, allocate the Largest fifo (weight) first */
	uCurrentLinePtr = 0;
	for ( nWeight=4; nWeight>=0; nWeight--) {
	    for ( uCurrentSpiPort=0; uCurrentSpiPort<nNumPhysicalSpiPorts; uCurrentSpiPort++) {
		if (uSpiPortWeights[uCurrentSpiPort] == (1<<nWeight)) {
		    uSpiPortStartLine[uCurrentSpiInterface][uCurrentSpiPort] = uCurrentLinePtr;
		    uCurrentLinePtr += uSpiPortLines[uCurrentSpiInterface][uCurrentSpiPort];
		}
	    }
	}
    }

    for (uCurrentEgPort=0; uCurrentEgPort<50; uCurrentEgPort++) {
         /*  Set up for Requeueing */
         /*  we must figure out if a Spi port is Requeue only, and
             if so, we have to assign it's RequeueDestPort as well
             as set RB_LOOPBACK bit in the EI_MEM */
         uint32 nTmpSpi=0;
         uint32 nTmpSpiPort=uCurrentEgPort;
         sbBool_t bRbLoopback=0;

        if (uCurrentEgPort >= uNumPhySpiPorts[0]){
          nTmpSpi=1;
          nTmpSpiPort = uCurrentEgPort - uNumPhySpiPorts[0];
        }
        /* catch transition to next spi bus */
        if(nTmpSpi!=nRequeueLastSpiBus){
          /* reset the nRequeueDestPort */
          nRequeueDestPort=-1;
        }
        nRequeueLastSpiBus = nTmpSpi;
        if (COMPILER_64_BITTEST(pQe2000Params->uuRequeuePortsMask[nTmpSpi], nTmpSpiPort)){
          bRbLoopback=1;
          nRequeueDestPort++;
        }
        /* end setup for requeueing */

        uCurrentLinePtr=uNextSpiLinePtr;  /* avoid warning for possible use of uninit'd variable */

        if (uCurrentEgPort==49) { /* PCI port (unused) */

            uCurrentDestChannel=0; /* not used since spi_dest_channel_src==0 */
            /* BUG FIX: Incident: 050815-000008 */
            uCurrentSpiPortLines=0x20; /* TBS: MUST BE 0x1F, note -1 below... */
            uCurrentLinePtr=0;

        } else if (uCurrentEgPort<uNumPhySpiPorts[0]) { /* SPI0 port */
            uCurrentSpiInterface=0;

            uCurrentDestChannel=0; /* not used since spi_dest_channel_src==0 */
            uCurrentLinePtr=uSpiPortStartLine[0][uCurrentEgPort];
            uCurrentSpiPortLines=uSpiPortLines[0][uCurrentEgPort];

            uNextSpiLinePtr = uCurrentLinePtr+uCurrentSpiPortLines;

        } else if (uCurrentEgPort<(uNumPhySpiPorts[0]+uNumPhySpiPorts[1])) { /* SPI1 port */
            uCurrentSpiInterface=1;

            uCurrentDestChannel=0; /* not used since spi_dest_channel_src==0 */
            uCurrentLinePtr=uSpiPortStartLine[1][(uCurrentEgPort-uNumPhySpiPorts[0])];
            uCurrentSpiPortLines=uSpiPortLines[1][(uCurrentEgPort-uNumPhySpiPorts[0])];

            uNextSpiLinePtr = uCurrentLinePtr+uCurrentSpiPortLines;
        } else { /* unused eg port */
	    uCurrentDestChannel=0; /* not used since spi_dest_channel_src==0 */
	    uCurrentSpiPortLines=0;
	    uCurrentLinePtr=0;
        }


        /* Update these values for full packet mode */
        if ( ((bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_FULL) ||
              (bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_N_1)) && (uCurrentSpiInterface == 0)) {
            uCurrentSpiPortLines = 0;
            uCurrentLinePtr = 0;
        }

        if ( ((bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_FULL) ||
             (bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_N_1)) && (uCurrentSpiInterface == 1)) {
            uCurrentSpiPortLines = 0;
            uCurrentLinePtr = 0;
        }

        QE2000PRINTF(QE2000_DEBUG_FUNC, ("EI FIFO PARAM TABLE  line 0x%x dest 0x%x spilines 0x%x\n",
                                         uCurrentLinePtr, uCurrentDestChannel,  uCurrentSpiPortLines));


        /*
         * NOTE:
         *      In the following code "uCurrentDestChannel" is 0. In N:1 mode the selected
         *      aggregration SPI channel is always 0 on the SPI bus. Thus the control memory
         *      is setup correctly. "EI_SPIn_config3.n_1_spi_channel" field is also set to 0
         *      (the aggregration SPI channel). This is the power on reset value.
         *
         *      In N:1 mode SPI is always configured for a single channel. This channel 0
         *      corresponds to the configuration in control memory and "EI_SPIn_config3"
         *      register. The "ka_st" block will request data for only channel 0 and thus
         *      "ei_spi0_config0.spi_dest_channel_src" need not be set to 1.
         */
        /* write fifo parameters to table */
        uData=0;

        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, DEST_CHANNEL, uData, bRbLoopback?nRequeueDestPort:uCurrentDestChannel);
        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK8, uData, (uCurrentSpiPortLines-1)>>8);
        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK7_0, uData, uCurrentSpiPortLines-1);
        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, RB_LOOPBACK_ONLY, uData, bRbLoopback);
        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, LINE_PTR, uData, uCurrentLinePtr);
        uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, BYTE_PTR, uData, 0);
        SAND_HAL_WRITE(userDeviceHandle, KA, EI_MEM_ACC_DATA, uData);

        /* write the same data to all 3 fifo config memories */
        for (uEiMemSel=0; uEiMemSel<3; uEiMemSel++) {
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_CTRL, REQ, uData, 1);
            uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_CTRL, RD_WR_N, uData, 0);
            uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_CTRL, MEM_SEL, uData, uEiMemSel);
            uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_CTRL, ADDR, uData, uCurrentEgPort);
            SAND_HAL_WRITE(userDeviceHandle, KA, EI_MEM_ACC_CTRL, uData);

            if (hwQe2000RegDone(userDeviceHandle,
                               SAND_HAL_KA_EI_MEM_ACC_CTRL_OFFSET,
                               0x80000000,
                                EI_MEM_TIMEOUT)) {
                QE2000PRINTF(QE2000_ERROR, ("ERROR: EI memory write timeout location(0x%x)\n", uCurrentEgPort));
                return HW_QE2000_STS_INIT_EI_MEM_TIMEOUT_ERR_K;
            }

            /* clear req/ack bits */
            uData=0;
            uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(userDeviceHandle, KA, EI_MEM_ACC_CTRL, uData);
        }
    }

    /* If both SPI has 10G ports and burst interleave mode, set both burst_size to 6 to achieve line rate for 64/65 bytes packets */
    if ((bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_BURST_IL) &&
	(bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_BURST_IL) &&
	(uMaxPortSpeed[0] >= 9000) &&
	(uMaxPortSpeed[1] >= 9000)) {
	/* SPI0 */
	uData = SAND_HAL_READ(userDeviceHandle, KA, EI_SPI0_CONFIG0);
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI0_CONFIG0, BURST_SIZE, uData, 6);
	SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI0_CONFIG0, uData);
	
	/* SPI1 */
	uData = SAND_HAL_READ(userDeviceHandle, KA, EI_SPI1_CONFIG0);
        uData = SAND_HAL_MOD_FIELD(KA, EI_SPI1_CONFIG0, BURST_SIZE, uData, 6);
	SAND_HAL_WRITE(userDeviceHandle, KA, EI_SPI1_CONFIG0, uData);
    }

  return HW_QE2000_STS_OK_K;
}

void
hwQe2000ConfigMcSrcId(sbhandle userDeviceHandle,
                      uint32 uNodeId,
                      uint32 uNumSpi0Ports)
{
    uint32 uPort;

    for (uPort=0; uPort<50; uPort++) {
        uint32 uSrcId;
        int32  nSpi=0;
        uint32 uSpiPort = uPort;

        /* This code can come out once the shim is populated with Node:Port  */
        /* but currently (guadulape) it's populated with Port:Node. */
        /* was : hwQe2000SetEgMcSrcId(userDeviceHandle, uPort, (uNodeId<<6 | uPort));*/

        /*was:        hwQe2000SetEgMcSrcId(userDeviceHandle, uPort, (uPort<<7 | uNodeId)); */

        /* 22210: requires SRC_ID to be populated as: */
        /* 22210: { port[4:0], FE_SRC_KNOCKOUT (always 0), node[4:0], SPI } */

        if ( uPort < uNumSpi0Ports) {
            nSpi = 0;
            uSpiPort = uPort;
        } else {
            nSpi = 1;
            uSpiPort = uPort-(uNumSpi0Ports);
        }

        /*      {      port[4:0]           FE_SRC_KNOCKOUT          node[4:0]            SPI     } */
        /*                                    (always 0)                                           */
        uSrcId = ((uSpiPort&0x1F) << 7)  |    (0x0 << 6)    | ((uNodeId&0x1F) << 1) | (nSpi&0x1);
        hwQe2000SetEgMcSrcId(userDeviceHandle, uPort, uSrcId);
    }
}
