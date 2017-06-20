/*
 * $Id: bme3200.c,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BME3200 SOC Initialization implementation
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/sbx_util.h>
#include <soc/sbx/fabric/sbZfFabBm3200BwBandwidthParameterEntry.hx>
#include <soc/sbx/fabric/sbZfFabBm3200BwPortRateEntry.hx>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/fabric.h>

#include <soc/sbx/link.h>

#ifdef BCM_BME3200_SUPPORT

#define BME3200_READ_REQUEST(unit, reg, otherAddr) \
  SOC_SBX_UTIL_READ_REQUEST(unit, PT, reg, otherAddr)

#define BME3200_WRITE_REQUEST(unit, reg, otherAddr) \
 SOC_SBX_UTIL_WRITE_REQUEST(unit, PT, reg, otherAddr)

#define BME3200_UTIL_WAIT_FOR_ACK(unit, reg, nTimeoutInMs, nStatus) \
 SOC_SBX_UTIL_WAIT_FOR_ACK(unit, PT, reg, nTimeoutInMs, nStatus)

static soc_block_info_t soc_blocks_bcm83332_a0[] = {
    /* block type,    number, schan, cmic */
    { SOC_BLK_GXPORT,   0,    -1,    -1 },   /* sfi */
    {-1,               -1,    -1,    -1 } /* end */
};

soc_driver_t soc_driver_bcm83332_a0 = {
	/* type         */	SOC_CHIP_BME3200_A0,
	/* chip_string  */	"bme3200",
	/* origin       */	"Unknown",
	/* pci_vendor   */	BROADCOM_VENDOR_ID,
	/* pci_device   */	BME3200_DEVICE_ID,
	/* pci_revision */	BME3200_A0_REV_ID,
	/* num_cos      */	0,
	/* reg_info     */	NULL,
    /* reg_unique_acc */ NULL,
        /* reg_above_64_info */ NULL,
        /* reg_array_info */    NULL,
	/* mem_info     */	NULL,
    /* mem_unique_acc */ NULL,
	/* mem_aggr     */	NULL,
        /* mem_array_info */    NULL,
	/* block_info   */	soc_blocks_bcm83332_a0,
	/* port_info    */	NULL,
	/* counter_maps */	NULL,
	/* features     */	soc_bm3200_features,
	/* init         */	NULL,
    /* services     */      NULL,
    /* port_num_blktype */  0,
    /* cmicd_base */        0

};  /* soc_driver */

static soc_block_info_t soc_blocks_bcm83332_b0[] = {
    /* block type,    number, schan, cmic */
    { SOC_BLK_GXPORT,   0,    -1,    -1 },   /* sfi */
    {-1,               -1,    -1,    -1 } /* end */
};

/* default config */
static soc_port_info_t soc_port_info_bcm83332_b0[] = {
    /* blk, block index */
    { 0,   0   },   /*  0  SFI.0  */
    { 0,   1   },   /*  1  SFI.1  */
    { 0,   2   },   /*  2  SFI.2  */
    { 0,   3   },   /*  3  SFI.3  */
    { 0,   4   },   /*  4  SFI.4  */
    { 0,   5   },   /*  5  SFI.5  */
    { 0,   6   },   /*  6  SFI.6  */
    { 0,   7   },   /*  7  SFI.7  */
    { 0,   8   },   /*  8  SFI.8  */
    { 0,   9   },   /*  9  SFI.9  */
    { 0,   10  },   /*  10 SFI.10 */
    { 0,   11  },   /*  11 SFI.11 */
    { 0,   12  },   /*  12 SFI.12 */
    { 0,   13  },   /*  13 SFI.13 */
    { 0,   14  },   /*  14 SFI.14 */
    { 0,   15  },   /*  15 SFI.15 */
    { 0,   16  },   /*  16 SFI.16 */
    { 0,   17  },   /*  17 SFI.17 */
    { 0,   18  },   /*  18 SFI.18 */
    { 0,   19  },   /*  19 SFI.19 */
    { 0,   20  },   /*  20 SFI.20 */
    { 0,   21  },   /*  21 SFI.21 */
    { 0,   22  },   /*  22 SFI.22 */
    { 0,   23  },   /*  23 SFI.23 */
    { 0,   24  },   /*  24 SFI.24 */
    { 0,   25  },   /*  25 SFI.25 */
    { 0,   26  },   /*  26 SFI.26 */
    { 0,   27  },   /*  27 SFI.27 */
    { 0,   28  },   /*  28 SFI.28 */
    { 0,   29  },   /*  29 SFI.29 */
    { 0,   30  },   /*  30 SFI.30 */
    { 0,   31  },   /*  31 SFI.31 */
    { 0,   32  },   /*  32 SFI.32 */
    { 0,   33  },   /*  33 SFI.33 */
    { 0,   34  },   /*  34 SFI.34 */
    { 0,   35  },   /*  35 SFI.35 */
    { 0,   36  },   /*  36 SFI.36 */
    { 0,   37  },   /*  37 SFI.37 */
    { 0,   38  },   /*  38 SFI.38 */
    { 0,   39  },   /*  39 SFI.39 */
    {-1,   -1  },   /*  end       */

};

soc_driver_t soc_driver_bcm83332_b0 = {
	/* type         */	SOC_CHIP_BME3200_B0,
	/* chip_string  */	"bme3200",
	/* origin       */	"Unknown",
	/* pci_vendor   */	BROADCOM_VENDOR_ID,
	/* pci_device   */	BME3200_DEVICE_ID,
	/* pci_revision */	BME3200_B0_REV_ID,
	/* num_cos      */	0,
	/* reg_info     */	NULL,
    /* reg_unique_acc */ NULL,
	/* reg_above_64_info */ NULL,
        /* reg_array_info */    NULL,
	/* mem_info     */	NULL,
    /* mem_unique_acc */ NULL,
	/* mem_aggr     */	NULL,
        /* mem_array_info */    NULL,
	/* block_info   */	soc_blocks_bcm83332_b0,
	/* port_info    */	NULL,
	/* counter_maps */	NULL,
	/* features     */	soc_bm3200_features,
	/* init         */	NULL,
    /* services     */      NULL,
    /* port_num_blktype */  0,
    /* cmicd_base */        0

};  /* soc_driver */

#define SB_FAB_DEVICE_BM3200_NUM_ESETS           (128)
#define SB_FAB_DEVICE_BM3200_MAX_LOGICAL_PORTS  (4096)
#define SB_FAB_DEVICE_BM3200_MAX_VOQS          (16384)

#define printf  bsl_printf

typedef void (*ifn_t)(int unit, uint32 data);

typedef struct {
    uint32	mask;
    ifn_t	intr_fn;
    uint32	intr_data;
    char	*intr_name;
} intr_handler_t;

STATIC intr_handler_t soc_bm3200_intr_handlers[] = {
    {SAND_HAL_PT_PI_PT_ERROR0_FO_INT_MASK,
     soc_bm3200_fo_error, 0, "SBPT_FO_ERROR"},
};

#define BM3200_INTR_HANDLERS_COUNT	COUNTOF(soc_bm3200_intr_handlers)

void
soc_bm3200_config_all_linkdriver(int unit)
{

    /* This is somewhat redundant b/c the deq,dtx are set in the init routine
     * leaving for now in case we need to adjust the settings after initializtion.
     */
    int        nLink;
    sbhandle   devHandle;

    devHandle = SOC_SBX_CFG(unit)->DeviceHandle;
    for ( nLink=0; nLink<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; nLink++ ) {
        soc_bm3200_config_linkdriver((sbFabUserDeviceHandle_t)devHandle, nLink, &(SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink]));
    }

}


int
soc_bm3200_init(int unit, soc_sbx_config_t *cfg)
{
    HW_BM3200_INIT_ST    hwBm3200InitParams;
    int                  uPlaneId;
    uint32               uStatus;
    sbFabStatus_t        status;
    sbLinkThresholdConfig_t linkThresholdConfig;

    sal_memset(&hwBm3200InitParams, 0, sizeof(HW_BM3200_INIT_ST));
    /* always apply soft reset (ie. toggle soft reset) for the BM3200 - registers will not change */
    hwBm3200InitParams.reset                    = SOC_SBX_CFG(unit)->reset_ul;
    hwBm3200InitParams.userDeviceHandle         = (sbhandle)(SOC_SBX_CFG(unit)->DeviceHandle);

    /* cmode and uBmEpochLength depends on fabric configuration
     */
    switch ( SOC_SBX_CFG(unit)->uFabricConfig ) {
	case SOC_SBX_SYSTEM_CFG_DMODE:
	    /* DMode (Default)
	     * - Bm3200 + Qe2000
	     */
	    hwBm3200InitParams.bmCmode_ul = 0;
	    hwBm3200InitParams.bmEpochLength_ul = SOC_SBX_CFG(unit)->epoch_length_in_timeslots;
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "soc_bm3200_init: unit %d unsupported"
	                           " fabric_configuration %d"),
	               unit, SOC_SBX_CFG(unit)->uFabricConfig));
	    break;
    }

    /* redundancy setup */
    hwBm3200InitParams.bmEnableAutoFailover = 0;
    hwBm3200InitParams.bmEnableAutoLinkDisable = 0;

    switch (SOC_SBX_CFG(unit)->uRedMode) {
	case bcmFabricRed1Plus1Both:
	case bcmFabricRed1Plus1LS:
	    hwBm3200InitParams.bmEnableAutoFailover = 1;
        hwBm3200InitParams.bmEnableAutoLinkDisable = 1;
	    break;
	case bcmFabricRedLS:
	    hwBm3200InitParams.bmEnableAutoLinkDisable = 1;
	    break;
	case bcmFabricRedManual:
	    break;
	case bcmFabricRed1Plus1ELS:
	case bcmFabricRedELS:
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "soc_bm3200_init: unit %d unsupported"
	                           " redundancy mode %d"),
	               unit, SOC_SBX_CFG(unit)->uRedMode));
	    return BCM_E_PARAM;
    }

    /* INA setup */
    hwBm3200InitParams.spMode = SOC_SBX_CFG(unit)->sp_mode;

    hwBm3200InitParams.bmModeLatency_ul         = 1;
    hwBm3200InitParams.bBmRunSelfTest           = SOC_SBX_CFG(unit)->bRunSelfTest;
    hwBm3200InitParams.bmMaxFailedLinks         = SOC_SBX_CFG(unit)->uMaxFailedLinks;
    hwBm3200InitParams.bmSerializerMask_ul      = SOC_SBX_CFG_BM3200(unit)->uSerializerMask;
    hwBm3200InitParams.bmLocalBmId_ul           = SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId;
    hwBm3200InitParams.bmDefaultBmId_ul         = SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId;
    hwBm3200InitParams.lcmXcfgABInputPolarityReversed_ul = SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed;


    status = GetLinkThresholdConfig(SOC_SBX_CFG(unit)->uLinkThresholdIndex,
				    &linkThresholdConfig);

    if (status == SB_FAB_STATUS_OK) {
	hwBm3200InitParams.siLsThreshold_ul = linkThresholdConfig.uLsThreshold;
	hwBm3200InitParams.siLsWindow_ul = linkThresholdConfig.uLsWindowIn256Clocks;
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "BM3200 uLinkThresholdIndex(%d) initialization "
	                       "parameter not initialized correctly."
	                       " Should be [0,100]\n"),
	           SOC_SBX_CFG(unit)->uLinkThresholdIndex));
    }

    /* Enable and power up all serdes, force them all to tx low.  The BCM layer
     * will take care of removing the force low when the port is enabled.  This
     * allows the serdes to be brought up in a predictable manner.
     */
    SOC_SBX_CFG_BM3200(unit)->uSeSerializerMaskHi = 0xFF;
    SOC_SBX_CFG_BM3200(unit)->uSeSerializerMaskLo = ~(SOC_SBX_CFG_BM3200(unit)->uSerializerMask);

    COMPILER_64_SET(hwBm3200InitParams.seSerializerMask_ull, 
                    SOC_SBX_CFG_BM3200(unit)->uSeSerializerMaskHi,
                    SOC_SBX_CFG_BM3200(unit)->uSeSerializerMaskLo);
    COMPILER_64_SET(hwBm3200InitParams.lcmSerializerMask_ull,
                    SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi,
                    SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo);

    sal_memcpy(hwBm3200InitParams.lcmXcfg_ul, SOC_SBX_CFG_BM3200(unit)->uLcmXcfg,
	       (size_t)(SB_FAB_MAX_NUM_DATA_PLANES * \
                        SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS * \
		        sizeof(uint32)));

    for (uPlaneId=0; uPlaneId< SB_FAB_MAX_NUM_DATA_PLANES; uPlaneId++) {
         hwBm3200InitParams.lcmPlaneValid_ul[uPlaneId] = \
                       SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[uPlaneId];
    }

    uStatus = hwBm3200Init(&hwBm3200InitParams);

    if (uStatus != HW_BM3200_STATUS_OK_K) {
        LOG_CLI((BSL_META_U(unit,
                            "Bm3200 initialization failed with uStatus(0x%x)\n"),uStatus));
        return (uStatus);
    }

    soc_bm3200_config_all_linkdriver(unit);

    /* Initialize SOC link control module */
    soc_linkctrl_init(unit, &soc_linkctrl_driver_sbx);

    return 0;
}

void
soc_bm3200_fo_error(int unit, uint32 ignored)
{
    uint32 intrs, uData;
    bcm_fabric_control_redundancy_info_t red_info;

    COMPILER_REFERENCE(ignored);

    intrs = SAND_HAL_READ(unit, PT, FO_ERROR);
    if (intrs) {
      SAND_HAL_WRITE(unit, PT, FO_ERROR_MASK, 0xffffffff);

      if (SAND_HAL_GET_FIELD(PT, FO_ERROR, AUTO_SWITCHOVER_EVENT, intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Auto Switchover Interrupt\n")));
      }
      if (SAND_HAL_GET_FIELD(PT, FO_ERROR, AUTO_DIS_EVENT, intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Link disable Interrupt\n")));
      }
      if (SAND_HAL_GET_FIELD(PT, FO_ERROR, AUTO_QE_DIS_EVENT, intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Qe disable Interrupt\n")));
      }

      /* Call back returning the redundancy info */
      if (((ifn_t)SOC_SBX_STATE(unit)->fabric_state->red_f) != NULL) {
	uData = SAND_HAL_READ(unit, PT, FO_STATUS);
	red_info.active_arbiter_id = SAND_HAL_GET_FIELD(PT, FO_STATUS, ACTIVE_BM, uData);
	COMPILER_64_SET(red_info.xbars,0,SAND_HAL_GET_FIELD(PT, FO_STATUS, ENABLED_LINKS, uData));
	((ifn_t)(SOC_SBX_STATE(unit)->fabric_state->red_f))(unit, (uint32)&red_info);
      }
    }

    return;
}

static void soc_bm3200_isr2(void *_unit);
void
soc_bm3200_isr(void *_unit)
{
#ifdef VXWORKS
	int unit = (int)_unit;

	/*
	 * There's only one soc_bm3200_isr allowed to be connected
	 * to FPGA int line.  So this ISR has to handle interrupts
	 * from all BME3200 chip/units.  '_unit' is the first BME3200
	 * chip/unit attached if there's more than one BME3200.
	 */ 
	while (SOC_CONTROL(unit) &&
		(SOC_CONTROL(unit)->info.chip_type == SOC_INFO_CHIP_TYPE_BME3200)) {
		soc_bm3200_isr2((void*)unit);
		unit++;
	}
#else
	soc_bm3200_isr2(_unit);
#endif
}

static void
soc_bm3200_isr2(void *_unit)
{
  /* 
     Methodology has changed:

     Kernel ISR will blindly mask PI_INTERRUPT_MASK, PI_UNIT_INTERRUPTX_MASK, and
     we have no idea what those masks were prior to it doing this.

     Must call all functions in table since top level PI_INTERRUPT will be masked
     Each handler must unmask it's own interrupts, or rely on subsequent user API calls
     (such as Failoer Re-Arming) to unmask interrupts

  */

    int                 i, unit = PTR_TO_INT(_unit);
    soc_control_t       *soc = SOC_CONTROL(unit);

    if (soc == NULL || !(soc->soc_flags & SOC_F_ATTACHED)) {
	    return;
    }

    soc->stat.intr++;		/* Update count */

    for (i=0; i < BM3200_INTR_HANDLERS_COUNT; i++) {
      LOG_INFO(BSL_LS_SOC_INTR,
               (BSL_META("soc_bm3200_isr unit %d: dispatch %s\n"),
                unit, soc_bm3200_intr_handlers[i].intr_name));
      (*soc_bm3200_intr_handlers[i].intr_fn)
	(unit, soc_bm3200_intr_handlers[i].intr_data);
    }
}

soc_error_t
soc_bm3200_config_linkdriver(int unit, int nLink, sbLinkDriverConfig_t *pLinkDriverConfig)
{
    sbLinkSpecificDriverConfig_t specificConfig;
    sbFabStatus_t status;
    uint32 uData;

    if ( nLink < 0 || nLink > SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Bad link(%d) requested for BM3200."
                               " Valid range is [0,%d)\n"),
                   nLink, SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS));
        return SB_FAB_STATUS_BAD_LINK_ID;
    }

    status = GetLinkSpecificConfig((sbFabUserDeviceHandle_t)unit, nLink, pLinkDriverConfig, &specificConfig);
    if ( status != SB_FAB_STATUS_OK ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Could not get link specific"
                               " configuration for link(%d)\n"), nLink));
        return status;
    }

    /* now that we have nDtx, etc. program the correct link config and we're done */
    {
        uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG3);

        uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG3, DEQ,   uData, specificConfig.u.rambusRs2314.nDeq);
        uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG3, DTX,   uData, specificConfig.u.rambusRs2314.nDtx);
        uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG3, HIDRV, uData, specificConfig.u.rambusRs2314.nHiDrv);
        uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG3, LODRV, uData, specificConfig.u.rambusRs2314.nLoDrv);

        SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG3, uData);
    }

    return SB_FAB_STATUS_OK;
}

/*
 * Get/set a block of 4 ESET Nodes Masks.  uBaseEset MUST be multiple of 4
 */
int soc_bm3200_eset_block_set(int nUnit, uint32 uBaseEset, uint32 uNodesMasks[4] )
{
    int i;
    uint32 uEsetData4 = 0x0, uSbRv;

    if( uBaseEset & 0x3 ) {
        return SOC_E_PARAM;
    }

    
    for(i = 0; i < 4; i++) {
        if (uNodesMasks[i]) {
            uEsetData4 |= (0x1 << i);
        } else {
            uEsetData4 &= ~(0x1 << i);
        }
    }

    uSbRv = hwBm3200NmEmapReadWrite((sbhandle)nUnit, uBaseEset,
                                    &uNodesMasks[0], &uNodesMasks[1],
                                    &uNodesMasks[2], &uNodesMasks[3],
                                    &uEsetData4,
                                    0 /* write */);


    return translate_sbx_result( uSbRv );

}

int soc_bm3200_eset_block_get(int nUnit, uint32 uBaseEset, uint32 uNodesMasks[4] )
{
    uint32 uEsetData4, uSbRv;

    if( uBaseEset & 0x3 ) {
        return SOC_E_PARAM;
    }


    uSbRv = hwBm3200NmEmapReadWrite((sbhandle)nUnit, uBaseEset,
                                    &uNodesMasks[0], &uNodesMasks[1],
                                    &uNodesMasks[2], &uNodesMasks[3],
                                    &uEsetData4, /* discard */
                                    1 /* read */);


    return translate_sbx_result( uSbRv );
}

/*
 * Get/set a single ESET entry
 */
int soc_bm3200_eset_set(int nUnit, uint32 uEset, uint32 uNodesMask, uint32 uMcFullEvalMin)
{
    uint32 uEsetData[5], uSbRv;
    uint32 uEsetMemoryIdx = uEset >> 2;
    uint32 uEsetEntryOffset = uEset & 0x3;

    uSbRv = hwBm3200NmEmapReadWrite((sbhandle)nUnit, uEsetMemoryIdx,
                                    &uEsetData[0], &uEsetData[1],
                                    &uEsetData[2], &uEsetData[3],
                                    &uEsetData[4],
                                    1 /* read */);

    if( uSbRv ) {
        return translate_sbx_result( uSbRv );
    }

    uEsetData[uEsetEntryOffset] = uNodesMask;
    /* If there are nodes present in the ESET, we can set the mc_full_eval_min field per user */
    if( uNodesMask ) {
        uEsetData[4] &= ~(0x1 << uEsetEntryOffset);

	if (uMcFullEvalMin != BCM_FABRIC_DISTRIBUTION_SCHED_ALL) {
	    uEsetData[4] |= (1 << uEsetEntryOffset);
	}
    } else {
      /* If there are no nodes, we need to always clear this bit due to a bm3200 hw issue */
      /* if traffic comes in when the eset has no entries but mc_full_eval_min is cleared */
      /* traffic can lock up. The user should call bcm_fabric_distribution_control_set()  */
      /* only after adding nodes to the ESET on a bm3200.                                 */
	uEsetData[4] &= ~(0x1 << uEsetEntryOffset);
    }

    uSbRv = hwBm3200NmEmapReadWrite((sbhandle)nUnit, uEsetMemoryIdx,
                                    &uEsetData[0], &uEsetData[1],
                                    &uEsetData[2], &uEsetData[3],
                                    &uEsetData[4],
                                    0 /* write */);

    return translate_sbx_result( uSbRv );
}


int soc_bm3200_eset_get(int nUnit, uint32 uEset, uint32 *pNodesMask, uint32 *pMcFullEvalMin)
{
    uint32 uEsetData[5], uSbRv;
    uint32 uEsetMemoryIdx = uEset >> 2;
    uint32 uEsetEntryOffset = uEset & 0x3;

    uSbRv = hwBm3200NmEmapReadWrite((sbhandle)nUnit, uEsetMemoryIdx,
                                    &uEsetData[0], &uEsetData[1],
                                    &uEsetData[2], &uEsetData[3],
                                    &uEsetData[4],
                                    1 /* read */);

    if( uSbRv ) {
        return translate_sbx_result( uSbRv );
    }

    *pNodesMask = uEsetData[uEsetEntryOffset];

    if (uEsetData[4] & (1 << uEsetEntryOffset)) {
        *pMcFullEvalMin = BCM_FABRIC_DISTRIBUTION_SCHED_ANY;
    }else {
        *pMcFullEvalMin = BCM_FABRIC_DISTRIBUTION_SCHED_ALL;
    }

    return translate_sbx_result( uSbRv );
}


int
soc_bm3200_xb_test_pkt_get(int unit, int egress, int xb_port, int *cnt)
{
    uint32 addr = ((egress & 1) << 7) | (xb_port & 0x3F);
    int status;

    BME3200_READ_REQUEST(unit, XB_TSTCNT, addr);
    BME3200_UTIL_WAIT_FOR_ACK(unit, XB_TSTCNT, 100, status);
    *cnt = SAND_HAL_READ(unit, PT, XB_TSTCNT_ACC_DATA);

    return (status < 0) ? SOC_E_TIMEOUT : SOC_E_NONE;
}

int
soc_bm3200_xb_test_pkt_clear(int unit, int egress, int xb_port)
{
    uint32 addr = ((egress & 1) << 7) | (xb_port & 0x3F);
    int status;

    SAND_HAL_WRITE(unit, PT, XB_TSTCNT_ACC_DATA, 0);
    BME3200_WRITE_REQUEST(unit, XB_TSTCNT, addr);
    BME3200_UTIL_WAIT_FOR_ACK(unit, XB_TSTCNT, 100, status);

    return (status < 0) ? SOC_E_TIMEOUT : SOC_E_NONE;
}


int
soc_bm3200_lcm_mode_set(int unit, soc_lcm_mode_t mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        if (mode == lcmModeNormal)
        {
            SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, NO_DESKEW, 0);
        } else {
            SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, NO_DESKEW, 1);
        }

        SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, XCFG_MODE, mode);

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

int
soc_bm3200_lcm_mode_get(int unit, soc_lcm_mode_t *mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        uint32 data = SAND_HAL_READ(unit, PT, XB_CONFIG0);
        *mode = SAND_HAL_GET_FIELD(PT, XB_CONFIG0, XCFG_MODE, data);
        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}


int
soc_bm3200_lcm_fixed_config(int unit, int configAB,
			    bcm_port_t xcfg[], int num_xcfgs)
{


    if (SOC_IS_SBX_BME(unit)) {
        if (num_xcfgs < 40) {
            return BCM_E_PARAM;
        }


#define SBX_XCFG_SET(r,x)     SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_##r, XCFG##x, xcfg[(x)])

        if (configAB) {
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A0,
                           (SBX_XCFG_SET(A0, 0) |
                            SBX_XCFG_SET(A0, 1) |
                            SBX_XCFG_SET(A0, 2) |
                            SBX_XCFG_SET(A0, 3) |
                            SBX_XCFG_SET(A0, 4)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A1,
                           (SBX_XCFG_SET(A1, 5) |
                            SBX_XCFG_SET(A1, 6) |
                            SBX_XCFG_SET(A1, 7) |
                            SBX_XCFG_SET(A1, 8) |
                            SBX_XCFG_SET(A1, 9)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A2,
                           (SBX_XCFG_SET(A2, 10) |
                            SBX_XCFG_SET(A2, 11) |
                            SBX_XCFG_SET(A2, 12) |
                            SBX_XCFG_SET(A2, 13) |
                            SBX_XCFG_SET(A2, 14)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A3,
                           (SBX_XCFG_SET(A3, 15) |
                            SBX_XCFG_SET(A3, 16) |
                            SBX_XCFG_SET(A3, 17) |
                            SBX_XCFG_SET(A3, 18) |
                            SBX_XCFG_SET(A3, 19)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A4,
                           (SBX_XCFG_SET(A4, 20) |
                            SBX_XCFG_SET(A4, 21) |
                            SBX_XCFG_SET(A4, 22) |
                            SBX_XCFG_SET(A4, 23) |
                            SBX_XCFG_SET(A4, 24)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A5,
                           (SBX_XCFG_SET(A5, 25) |
                            SBX_XCFG_SET(A5, 26) |
                            SBX_XCFG_SET(A5, 27) |
                            SBX_XCFG_SET(A5, 28) |
                            SBX_XCFG_SET(A5, 29)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A6,
                           (SBX_XCFG_SET(A6, 30) |
                            SBX_XCFG_SET(A6, 31) |
                            SBX_XCFG_SET(A6, 32) |
                            SBX_XCFG_SET(A6, 33) |
                            SBX_XCFG_SET(A6, 34)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A7,
                           (SBX_XCFG_SET(A7, 35) |
                            SBX_XCFG_SET(A7, 36) |
                            SBX_XCFG_SET(A7, 37) |
                            SBX_XCFG_SET(A7, 38) |
                            SBX_XCFG_SET(A7, 39)));



        } else {

            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B0,
                           (SBX_XCFG_SET(B0, 0) |
                            SBX_XCFG_SET(B0, 1) |
                            SBX_XCFG_SET(B0, 2) |
                            SBX_XCFG_SET(B0, 3) |
                            SBX_XCFG_SET(B0, 4)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B1,
                           (SBX_XCFG_SET(B1, 5) |
                            SBX_XCFG_SET(B1, 6) |
                            SBX_XCFG_SET(B1, 7) |
                            SBX_XCFG_SET(B1, 8) |
                            SBX_XCFG_SET(B1, 9)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B2,
                           (SBX_XCFG_SET(B2, 10) |
                            SBX_XCFG_SET(B2, 11) |
                            SBX_XCFG_SET(B2, 12) |
                            SBX_XCFG_SET(B2, 13) |
                            SBX_XCFG_SET(B2, 14)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B3,
                           (SBX_XCFG_SET(B3, 15) |
                            SBX_XCFG_SET(B3, 16) |
                            SBX_XCFG_SET(B3, 17) |
                            SBX_XCFG_SET(B3, 18) |
                            SBX_XCFG_SET(B3, 19)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B4,
                           (SBX_XCFG_SET(B4, 20) |
                            SBX_XCFG_SET(B4, 21) |
                            SBX_XCFG_SET(B4, 22) |
                            SBX_XCFG_SET(B4, 23) |
                            SBX_XCFG_SET(B4, 24)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B5,
                           (SBX_XCFG_SET(B5, 25) |
                            SBX_XCFG_SET(B5, 26) |
                            SBX_XCFG_SET(B5, 27) |
                            SBX_XCFG_SET(B5, 28) |
                            SBX_XCFG_SET(B5, 29)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B6,
                           (SBX_XCFG_SET(B6, 30) |
                            SBX_XCFG_SET(B6, 31) |
                            SBX_XCFG_SET(B6, 32) |
                            SBX_XCFG_SET(B6, 33) |
                            SBX_XCFG_SET(B6, 34)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B7,
                           (SBX_XCFG_SET(B7, 35) |
                            SBX_XCFG_SET(B7, 36) |
                            SBX_XCFG_SET(B7, 37) |
                            SBX_XCFG_SET(B7, 38) |
                            SBX_XCFG_SET(B7, 39)));



        }

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

int
soc_bm3200_bwp_read(int unit, int32 queue, int32 *pgamma, int32 *psigma) {

    int status;
    int32 addr;
    uint32 data;
    sbZfFabBm3200BwBandwidthParameterEntry_t zfBwpEntry;

    /* this is a 32 bit entry but the field is 16 bits */
    addr = queue;

    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP,
				    0 /* repository */, addr, &data);

    sbZfFabBm3200BwBandwidthParameterEntry_Unpack(&zfBwpEntry, (uint8*)&data, 4);

    *pgamma = zfBwpEntry.m_nGamma;
    *psigma = zfBwpEntry.m_nSigma;

    return status;
}


int
soc_bm3200_bwp_write(int unit, int32 queue,
		     int32 gamma, int32 sigma) {
    int status = 0;
    int32 addr;
    sbZfFabBm3200BwBandwidthParameterEntry_t zfBwpEntry;
    int32 write_value;

    addr = queue;

    zfBwpEntry.m_nGamma = gamma;
    zfBwpEntry.m_nSigma = sigma;

    sbZfFabBm3200BwBandwidthParameterEntry_Pack(&zfBwpEntry, (uint8*)&write_value, 4);

    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP,
				     0 /* nRepository */, addr, write_value);
    if (status) {
	return status;
    }
    return status;
}


int
soc_bm3200_prt_read(int unit, int32 bw_group, int32 *psp_queues_in_bag, int32 *pqueues_in_bag,
		    int32 *pbase_queue, int32 *pbag_rate_in_bytes_per_epoch ) {

    int status = 0;
    int32 addr;
    uint32 read_value[2];
    sbZfFabBm3200BwPortRateEntry_t zfPrtEntry;

    /* this is a 32 bit entry but the field is 16 bits */
    addr = bw_group;

    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
				    1 /* repository */, addr, &read_value[0]);
    if (status) {
	return status;
    }

    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
				    0 /* repository */, addr, &read_value[1]);
    if (status) {
	return status;
    }

    sbZfFabBm3200BwPortRateEntry_Unpack(&zfPrtEntry, (uint8*)&read_value, 8);

   *pbag_rate_in_bytes_per_epoch =  zfPrtEntry.m_nLineRate;
   *pbase_queue = zfPrtEntry.m_nGroup;
   *pqueues_in_bag = zfPrtEntry.m_nGroups;
   *psp_queues_in_bag = zfPrtEntry.m_nSpGroups;

    return status;
}


int
soc_bm3200_prt_write(int unit, int32 bw_group, int32 sp_queues_in_bag,
		     int32 queues_in_bag, int32 base_queue, int32 bag_rate_in_bytes_per_epoch) {

    int status = 0;
    int32 addr;
    int32 write_value[2];
    sbZfFabBm3200BwPortRateEntry_t zfPrtEntry;

    addr = bw_group;

    sbZfFabBm3200BwPortRateEntry_InitInstance(&zfPrtEntry);
    zfPrtEntry.m_nLineRate = bag_rate_in_bytes_per_epoch;
    zfPrtEntry.m_nGroup    = base_queue;
    zfPrtEntry.m_nGroups   = queues_in_bag;
    zfPrtEntry.m_nSpGroups = sp_queues_in_bag;

    sbZfFabBm3200BwPortRateEntry_Pack(&zfPrtEntry, (uint8*)&write_value, 8);


    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
				     1 /* repository */, addr, write_value[0]);
    if (status) {
	return status;
    }
    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
				     0 /* repository */, addr, write_value[1]);
    if (status) {
	return status;
    }
    return status;
}


int
soc_bm3200_bw_mem_read(int unit, int32 bw_table_id,
		       int32 repository, int32 addr, uint32 *pdata) {
    uint32 timeout;
    uint32 ctrl;
    int status;

    SAND_HAL_WRITE_FORCE(unit, PT, BW_ERROR, 3);

    /* if the bank to read is 0 */
    if (repository == 0) {

	ctrl=0;
	ctrl = SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  ACK,            1) |
	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  REQ,           1) |
	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  RD_WR_N,       1) |
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  TBL_ID,        bw_table_id) |
  	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  OFS,           addr);


	SAND_HAL_WRITE_FORCE(unit, PT, BW_R0_ACC_CTRL, ctrl);

	status = SB_FAB_STATUS_BM3200_BW_TABLE_BANK0_ACCESS_FAILED;
	timeout = 100;
	while(timeout--)
	{
	    ctrl = SAND_HAL_READ(unit, PT, BW_R0_ACC_CTRL);
	    if(SAND_HAL_GET_FIELD(PT, BW_R0_ACC_CTRL, ACK, ctrl))
	    {
		*pdata = SAND_HAL_READ(unit, PT, BW_R0_ACC_DAT);

		SAND_HAL_WRITE_FORCE(unit, PT, BW_R0_ACC_CTRL,
					   SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, ACK, 0x1));

		status = SB_FAB_STATUS_OK;
		break;
	    }
	}
    }
    else /* Bank1 */ {
	ctrl=0;
	ctrl = SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  ACK,            1) |
	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  REQ,           1) |
	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  RD_WR_N,       1) |
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  TBL_ID,        bw_table_id) |
  	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  OFS,           addr);


	SAND_HAL_WRITE_FORCE(unit, PT, BW_R1_ACC_CTRL, ctrl);

	status = SB_FAB_STATUS_BM3200_BW_TABLE_BANK1_ACCESS_FAILED;
	timeout = 100;
	while(timeout--)
	{
	    ctrl = SAND_HAL_READ(unit, PT, BW_R1_ACC_CTRL);
	    if(SAND_HAL_GET_FIELD(PT, BW_R1_ACC_CTRL, ACK, ctrl))
	    {
		*pdata = SAND_HAL_READ(unit, PT, BW_R1_ACC_DAT);

		SAND_HAL_WRITE_FORCE(unit, PT, BW_R1_ACC_CTRL,
					   SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, ACK, 0x1));

		status = SB_FAB_STATUS_OK;
		break;
	    }
	}
    }
    {
	/* Make sure there is no error. */
	int32 nBwError;
	nBwError = SAND_HAL_READ(unit, PT, BW_ERROR) & 3;

	if (nBwError) {
	    int32 nErrorInfo;

	    if (repository == 0) {
		nErrorInfo = SAND_HAL_READ(unit, PT, BW_R0_ERR_INFO);
		status = SB_FAB_STATUS_BM3200_BW_TABLE_BANK0_ACCESS_FAILED;
	    }
	    else {
		nErrorInfo = SAND_HAL_READ(unit, PT, BW_R1_ERR_INFO);
		status = SB_FAB_STATUS_BM3200_BW_TABLE_BANK1_ACCESS_FAILED;
	    }
            COMPILER_REFERENCE(nErrorInfo);
	}
    }

    return status;
}

int
soc_bm3200_bw_mem_write(int unit, int32 bw_table_id,
			int32 repository, int32 addr, uint32 write_data) {
    uint32 timeout;
    uint32 ctrl;
    int status;

    SAND_HAL_WRITE(unit, PT, BW_ERROR, 3);

    /* if the bank to write is 0 */
    if (repository == 0) {

	SAND_HAL_WRITE(unit, PT, BW_R0_ACC_DAT, write_data);

	ctrl=0;
	ctrl = SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  ACK,            1) |
	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  REQ,           1) |
	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  RD_WR_N,       0) | /* write */
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  TBL_ID,        bw_table_id) |
  	        SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL,  OFS,           addr);


	SAND_HAL_WRITE(unit, PT, BW_R0_ACC_CTRL, ctrl);

	status = SB_FAB_STATUS_BM3200_BW_TABLE_BANK0_ACCESS_FAILED;
	timeout = 100;
	while(timeout--)
	{
	    ctrl = SAND_HAL_READ_POLL(unit, PT, BW_R0_ACC_CTRL);
	    if(SAND_HAL_GET_FIELD(PT, BW_R0_ACC_CTRL, ACK, ctrl))
	    {

		SAND_HAL_WRITE(unit, PT, BW_R0_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, ACK, 0x1));

		status = 0;
		break;
	    }
	}
    }
    else /* Bank1 */ {

	SAND_HAL_WRITE(unit, PT, BW_R1_ACC_DAT, write_data);

	ctrl=0;
	ctrl = SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  ACK,            1) |
	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  REQ,           1) |
	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  RD_WR_N,       0) | /* write */
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  TBL_ID,        bw_table_id) |
  	        SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL,  OFS,           addr);


	SAND_HAL_WRITE(unit, PT, BW_R1_ACC_CTRL, ctrl);

	status = -1;

	timeout = 100;
	while(timeout--)
	{
	    ctrl = SAND_HAL_READ_POLL(unit, PT, BW_R1_ACC_CTRL);
	    if(SAND_HAL_GET_FIELD(PT, BW_R1_ACC_CTRL, ACK, ctrl))
	    {

		SAND_HAL_WRITE(unit, PT, BW_R1_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, ACK, 0x1));
		status = 0;
		break;
	    }
	}
    }
#ifdef BCM_EASY_RELOAD_SUPPORT
    if (!SOC_IS_RELOADING(unit))
#endif
    {
	int32 nBwError;
	/* Make sure there is no error. */
	nBwError = SAND_HAL_READ(unit, PT, BW_ERROR) & 3;

	if (nBwError) {
	    int32 nErrorInfo;

	    if (repository == 0) {
		nErrorInfo = SAND_HAL_READ(unit, PT, BW_R0_ERR_INFO);
		status = -1;
	    }
	    else {
		nErrorInfo = SAND_HAL_READ(unit, PT, BW_R1_ERR_INFO);
		status = -1;
	    }
            COMPILER_REFERENCE(nErrorInfo);
	    LOG_CLI((BSL_META_U(unit,
                                "bw_error occurred during write to repository(%d)\n"), repository));
	}
    }

    return status;
}

int
soc_bm3200_features(int unit, soc_feature_t feature)
{
    if (SOC_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CFG(unit) == NULL) {
        return(FALSE);
    }

    switch(feature) {
        case soc_feature_discard_ability:
            switch(SOC_SBX_CFG_BM3200(unit)->uDeviceMode) {
                case SOC_SBX_BME_ARBITER_MODE:
                case SOC_SBX_BME_ARBITER_XBAR_MODE:
                    return(TRUE);

                case SOC_SBX_BME_XBAR_MODE:
                case SOC_SBX_BME_LCM_MODE:
                default:
                    return(FALSE);
            }

        case soc_feature_distribution_ability:
            return(TRUE);

        case soc_feature_standalone:
            return(FALSE);

        case soc_feature_arbiter:
            switch(SOC_SBX_CFG_BM3200(unit)->uDeviceMode) {
                case SOC_SBX_BME_ARBITER_MODE:
                case SOC_SBX_BME_ARBITER_XBAR_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

        case soc_feature_xbar:
            switch(SOC_SBX_CFG_BM3200(unit)->uDeviceMode) {
                case SOC_SBX_BME_XBAR_MODE:
                case SOC_SBX_BME_ARBITER_XBAR_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

        case soc_feature_lcm:
            switch(SOC_SBX_CFG_BM3200(unit)->uDeviceMode) {
                case SOC_SBX_BME_LCM_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

        case soc_feature_hybrid:
            return((SOC_SBX_CFG(unit)->bHybridMode == TRUE) ? TRUE : FALSE);
            break;

        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
		
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
                    return((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
                    break;

                case SOC_SBX_SYSTEM_CFG_DMODE:
                default:
                    return(FALSE);
                    break;
            }
            break;

        default:
            return(FALSE);
    }
}

int
soc_bm3200_port_info_config(int unit)
{
    int                 i, port;
    soc_info_t          *si;
    int                 total_ports;
    soc_port_info_t     *port_info;
    soc_driver_t        *driver;

    si     = &SOC_INFO(unit);
    driver = SOC_DRIVER(unit);

    /* Load port mapping into 'port_info' and 'block_info' device driver */
    /* free old array if its not the default */
    if (driver->port_info &&
        (driver->port_info != soc_port_info_bcm83332_b0))
    {
        sal_free(driver->port_info);
        driver->port_info = NULL;
    }

    /* set defaults first */
    driver->port_info = soc_port_info_bcm83332_b0;

    total_ports = (SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS    +
                   1);                                 /* end of list entry */

    port_info = sal_alloc((sizeof(soc_port_info_t) * total_ports),
                          "bm_port_info");

    if (port_info == NULL) {
        return BCM_E_MEMORY;
    }

    /* init port info to the 'end of list' entry */
    for (port=0; port<total_ports; port++) {
        port_info[port].blk     = -1;
        port_info[port].bindex  = -1;
    }

    driver->port_info = port_info;

    port = 0;
    /* SFI & SCI Ports */
    for (i=0; i<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; i++, port++) {
        port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_GXPORT, 0);
        port_info[port].bindex = i;
    }

    si->sci.min = si->sci.max = 0;
    si->sfi.min = 0;
    si->sfi.max = si->sfi.min + SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS - 1;
    for (port = si->sfi.min; port <= si->sfi.max; port++) {
        SBX_ADD_PORT(sfi,port);
        SBX_ADD_PORT(all,port);
        si->port_type[port] = SOC_BLK_GXPORT;
        sal_snprintf(SOC_PORT_NAME(unit,port),
                     sizeof(si->port_name[port]),
                     "sfi%d", (port - si->sfi.min));
    }

    return BCM_E_NONE;

}

/*
 *   Function
 *      soc_bm3200_eset_init
 *   Purpose
 *      Initialize the ESET table in the BME.
 *   Parameters
 *      (in) unit      - BCM unit number
 *
 *   Returns
 *     BCM_E_NONE if successful
 *     BCM_E_* appropriately if not
 */
int
soc_bm3200_eset_init(int unit)
{
    int i, rv;
    uint32 uNodesMasks[4];
    int nNode, reg;
    int nNumNodes = soc_property_get(unit, spn_NUM_MODULES, SBX_MAX_NODES);

    /* return if the NodeMap is disabled on this unit, it's not
     * neccessarily an error because the BME3200 can be an SE, a BME or an LCM
     * and the SDK has no way of knowing how it's configured other than by
     * the NM enable bit
     */
    reg = SAND_HAL_READ(unit, PT, PI_CORE_CLOCK_CONFIG0);
    if (SAND_HAL_GET_FIELD(PT, PI_CORE_CLOCK_CONFIG0, NM_CORE_CLK_DISABLE, reg)) {
        return SOC_E_NONE;
    }


    /* Build up the Nodes Masks Block to write
     * For systems with 7 nodes or less, we can construct an ESET
     * table with all permutations of egress nodes - 2^7 <= 128 ESETs
     * For systems with more than 7 nodes, all multicast queues will
     * point to a single ESET containing all egress nodes
     */
    if( nNumNodes < 8 ) {
        for(nNode = 0; nNode < nNumNodes; nNode++) {
            for(i = 0; i < 4; i++) {
                uNodesMasks[i] = (nNode << 2) | i;
            }

            /* special case - eset[0] => all nodes */
            if (nNode == 0) {
                uNodesMasks[0] = ~0;
            }

            rv = soc_bm3200_eset_block_set(unit, nNode << 2, uNodesMasks);
            if( SOC_FAILURE(rv) ){
                return rv;
            }
        }

    } else {
        int nBaseEset;

        /* all nodes */
        for(i = 0; i < 4; i++) {
            /* coverity[large_shift : FALSE] */
            uNodesMasks[i] = (1 << nNumNodes) - 1;
        }

        for(nBaseEset = 0; nBaseEset < HW_BM3200_PT_EMAP_MEM_SIZE; nBaseEset++) {
            rv = soc_bm3200_eset_block_set(unit, nBaseEset << 2, uNodesMasks);
            if( BCM_FAILURE(rv) ){
                return rv;
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_bm3200_epoch_in_timeslot_config_get(int unit, int num_queues, uint32 *epoch_in_timeslots)
{
    int rv = SOC_E_NONE;
    uint32 wred_deadline, dmnd_deadline, alloc_deadline;
    int32 nCosPerPort;

    /* In DMode we only enable the WRED and ALLOC phases of allocation: */

    /*
     * WRED processes 2 queues per timeslot, it also requires three
     * timeslots to prime the pipeline and three to drain it... If
     * WRED has not completed by its deadline, it will wind down,
     * if it has not completed within 8 timeslots of its deadline
     * it will be squelched...
     */
    wred_deadline = SB_FAB_DEVICE_BM3200_EPOCH_PRIME_PIPE + (num_queues + 1) / 2;
    wred_deadline = wred_deadline + SB_FAB_DEVICE_BM3200_EPOCH_WRED_KILL;

    /*
     * demand estimation is performed by the QEs in DMode. This
     * deadline merely determines when the BME expects that the
     * QEs will have completed their task. We set it greater than
     * the wred_deadline (arbitrarily).
     */
    dmnd_deadline = wred_deadline + 2;

    nCosPerPort = 1;

    /*
     * allocation runs at better than 2 queues per timeslot.
     * allocation must have completed by its deadline or it will be
     * squelched. allocation requires 16 timeslots to prime its
     * pipeline and 16 to wind down...
     */
    alloc_deadline = dmnd_deadline + SB_FAB_DEVICE_BM3200_EPOCH_ALLOC_PRIME_PIPE +
                                                        (num_queues * nCosPerPort) / 2;

    (*epoch_in_timeslots) = alloc_deadline + 1;

    return(rv);
}

#endif
