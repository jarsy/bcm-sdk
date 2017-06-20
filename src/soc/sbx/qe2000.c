/*
 * $Id: qe2000.c,v 1.102 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 SOC Initialization implementation
 */

#include <shared/bsl.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_counter.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/qe2000_scoreboard.h>
#include <soc/sbx/sbx_util.h>
#include <soc/sbx/fabric/sbZfFabQe2000LnaFullRemapEntry.hx>
#include <bcm_int/sbx/cosq.h>
#include <soc/sbx/link.h>
#include <bcm_int/sbx/lock.h>

#include <assert.h>

#ifdef BCM_QE2000_SUPPORT

static qe2000_qs_mem_desc_t  qe2000_qs_mem[] =
{
    { TRUE,  { 0xFFFFFFFF, } }, /* 0:  Rate_A          */
    { TRUE,  { 0xFFFFFFFF, } }, /* 1:  Rate_B          */
    { FALSE, { 0x01FFFFFF, } }, /* 2:  Credit          */
    { TRUE,  { 0x0000003F, } }, /* 3:  Depth_Hplen     */
    { TRUE,  { 0x0001FFFF, } }, /* 4:  Q2EC            */
    { TRUE,  { 0x000000FF, } }, /* 5:  Queue_para      */
    { TRUE,  { 0x00FFFFFF, } }, /* 6:  Shape_Rate      */
    { TRUE,  { 0x00FFFFFF, } }, /* 7:  Shape_Max_Burst */
    { FALSE, { 0xFFFFFFFF, } }, /* 8:  Shape           */
    { FALSE, { 0xFFFFFFFF, } }, /* 9:  Age             */
    { TRUE,  { 0x0000001F, } }, /* 10: Age_Thresh_key  */
    { FALSE, { 0x0000FFFF, } }, /* 11: Age_Thresh_Lut  */
    { FALSE, { 0x000000FF, } }, /* 12: Pri_Lut         */
    { FALSE, { 0x000001FF, } }, /* 13: Pri             */
    { TRUE,  { 0x00007FFF, } }, /* 14: E2Q             */
    { FALSE, { 0xFFFFFFFF, } }, /* 15: LastSentPri     */
};

static qe2000_qs_mem_desc_t  qe2000_qs_lna_mem[] =
{
    { FALSE, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 } }, /* 0:  INA Pri          */
    { FALSE, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 } }, /* 1:  INA NextPri      */
    { TRUE,  { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x003FFFFF } }, /* 2:  Port Remap       */
    { FALSE, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 } }, /* 3:  Unused           */
};

static soc_block_info_t soc_blocks_bcm83200_a3[] = {
    /* block type,    number, schan, cmic */
    { SOC_BLK_GXPORT,   0,    -1,    -1 },   /* sfi & sci */
    { SOC_BLK_SPI,      0,    -1,    -1 },
    { SOC_BLK_SPI,      1,    -1,    -1 },
    { SOC_BLK_CMIC,     0,    -1,    -1 },
    {-1,               -1,    -1,    -1 } /* end */
};

/* default config, spi-subports added dynamically */
static soc_port_info_t soc_port_info_bcm83200_a3[] = {
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
    { 0,   18  },   /*  18 SCI.0  */
    { 0,   19  },   /*  19 SCI.1  */
    { 1,   -1  },   /*  20 SPI0   */
    { 2,   -1  },   /*  21 SPI1   */
    { 3,   -1  },   /*  22 CPU    */
    {-1,   -1  },   /*  end       */

};

soc_driver_t soc_driver_bcm83200_a3 = {
    /* type         */  SOC_CHIP_QE2000_A4,
    /* chip_string  */  "qe2000",
    /* origin       */  "Unknown",
    /* pci_vendor   */  BROADCOM_VENDOR_ID,
    /* pci_device   */  QE2000_DEVICE_ID,
    /* pci_revision */  QE2000_A1_REV_ID,
    /* num_cos      */  0,
    /* reg_info     */  NULL,
    /* reg_unique_acc */ NULL,
    /* reg_above_64_info */ NULL,
    /* reg_array_info */ NULL,
    /* mem_info     */  NULL,
    /* mem_unique_acc */ NULL,
    /* mem_aggr     */  NULL,
    /* mem_array_info */ NULL,
    /* block_info   */  soc_blocks_bcm83200_a3,
    /* port_info    */  soc_port_info_bcm83200_a3,
    /* counter_maps */  NULL,
    /* features     */  soc_qe2000_features,
    /* init         */  NULL,
    /* services     */  NULL,
    /* port_num_blktype */ 0,
    /* cmicd_base   */  0
};  /* soc_driver */

/*
 * Assume the same #Cos queues for all ports
 * (Node(5b) << 7) | (Port(6b) << 1) | Mc(1b)
 */
#define MASK_TO_SIZE(x,sz)    ((x) & (((1 << (sz)) - 1)))

#define QE2000_1_MSEC_K      (1000000)

/* Indirect memory reads - note the use of macros here */
#define SOC_QE2000_READ_REQUEST(userDeviceHandle, reg, otherAddr) SAND_HAL_WRITE((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_QE2000_READ_REQUEST_CLR_ON_RD(userDeviceHandle, reg, otherAddr) SAND_HAL_WRITE((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, CLR_ON_RD,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_QE2000_WRITE_REQUEST(userDeviceHandle, reg, otherAddr) SAND_HAL_WRITE((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )

#define SOC_QE2000_WRITE_REQUEST_WITH_PARITY(userDeviceHandle, otherAddr, parity) SAND_HAL_WRITE((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, CORRUPT_PARITY, parity)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )


#define SOC_QE2000_WAIT_FOR_ACK(userDeviceHandle, reg, nTimeoutInMillisecs, nStatus)\
{\
int32 nTimeCnt = 0;\
nStatus = 0;\
while (1) {\
uint32 uDataWaitForAck = SAND_HAL_READ_POLL((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL);\
if (SAND_HAL_GET_FIELD(KA, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){\
  uDataWaitForAck = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0);\
  uDataWaitForAck = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1);\
  SAND_HAL_WRITE((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, uDataWaitForAck);\
  break; }\
if (nTimeCnt/100 >= nTimeoutInMillisecs){\
   nStatus = -1;\
   break; \
}\
thin_delay(HW_QE2000_10_USEC_K);\
nTimeCnt++;\
}\
}


#define SOC_QE2000_READ_REQUEST_EASY_RELOAD(userDeviceHandle, reg, otherAddr) SAND_HAL_WRITE_EASY_RELOAD((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )


#define SOC_QE2000_WAIT_FOR_ACK_EASY_RELOAD(userDeviceHandle, reg, nTimeoutInMillisecs, nStatus)\
{\
int32 nTimeCnt = 0;\
nStatus = 0;\
while (1) {\
 uint32 uDataWaitForAck = SAND_HAL_READ((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL);\
if (SAND_HAL_GET_FIELD(KA, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){\
uDataWaitForAck = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0);\
uDataWaitForAck = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1);\
SAND_HAL_WRITE_EASY_RELOAD((sbhandle)userDeviceHandle, KA, reg##_ACC_CTRL, uDataWaitForAck);\
break; }\
if ((nTimeCnt/100) >= nTimeoutInMillisecs){\
nStatus = -1;\
break; \
}\
thin_delay(HW_QE2000_10_USEC_K);\
nTimeCnt++;\
}\
}

/* Common check for validity of queue parameter */
#define QE2000_QUEUE_VALID_CHECK(q) do { if((q) > 16383) {return SOC_E_PARAM;} } while(0)

#define BCM_XBAR_WAIT_XCNFG_TIME (4) /* milliseconds */

#define printf bsl_printf

#define QE2000_RATE_DELTA_MAX_PORT_SPEEDS    16
#define QE2000_RATE_DELTA_MAX_PERCENTAGE      4


static uint32
soc_sbx_qe2000_rate_delta_port_speed[QE2000_RATE_DELTA_MAX_PORT_SPEEDS] =
{
       1000,    2000,    5000,     7000,
      10000,   20000,   50000,    70000,
     100000,  200000,  500000,   700000,
    1000000, 2000000, 5000000, 10000000
};

static uint32
soc_sbx_qe2000_rate_delta_percentage[QE2000_RATE_DELTA_MAX_PERCENTAGE] =
{
    10, 13, 17, 20
};

/* Reference qe2000_init.c Array "nRateDeltaMaxInBytesPerEpoch" */
static uint32
soc_sbx_qe2000_standalone_rate_delta_port_speed[QE2000_RATE_DELTA_MAX_PORT_SPEEDS] =
{
       1000,    2000,    5000,     7000,
      10000,   20000,   50000,    70000,
     100000,  200000,  500000,   700000,
    1000000, 2000000, 5000000, 10000000
};

/* QE2000--CUSTOM1--BM9600 system */
static uint32
soc_sbx_bs_qe2000_custom1_bm9600[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
{
   95,
   19,  38,  57,  76,  95, 114, 133, 152, 171,  /* links: 01-09 */
  190, 208, 228, 247, 247, 247, 247, 247, 247   /* links: 10-18 */
};


int
soc_qe2000_custom1_config_burst_size_lines_get(int unit, int32 *value, int32 *value_cong, uint64 xbars)
{
    int      rv = SOC_E_NONE;
    uint32   qe2k_links = 0;
    int      i;
    uint32  *burst_size_config_p;


    /* mask of logical xbars not belonging to this device */
    COMPILER_64_SET(xbars,0, (COMPILER_64_LO(xbars) & ~(SOC_SBX_CFG(unit)->module_custom1_links)));

    switch(SOC_SBX_CFG(unit)->uFabricConfig) {
        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
            burst_size_config_p = &soc_sbx_bs_qe2000_custom1_bm9600[0];
            break;

        case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
            
            burst_size_config_p = &soc_sbx_bs_qe2000_custom1_bm9600[0];
            break;

        default:
            rv = SOC_E_CONFIG;
            break;
    }

    SOC_IF_ERROR_RETURN(rv);

    /* determine number of links */
    for (i = 0; i < 64; i++) {
        /* Count number of enabled links */
        qe2k_links += (COMPILER_64_BITTEST(xbars, i));
    }
    if (qe2k_links > SB_FAB_DEVICE_QE2000_SFI_LINKS) {
        qe2k_links = SB_FAB_DEVICE_QE2000_SFI_LINKS;
    }

    if (qe2k_links == 0) {
        /* set default value */
        qe2k_links = (SB_FAB_DEVICE_QE2000_SFI_LINKS - 2);
    }

    (*value) = *(burst_size_config_p + qe2k_links);

    /* set congested burst size to 85% */
    (*value_cong) = ((*value) * 85) / 100;

    return(rv);
}

int
soc_qe2000_burst_size_lines_get(int unit, int32 *value, int32* value_cong, uint64 xbars)
{
    int     rv = SOC_E_NONE;
    uint32  ts_opt, opt_packlets, max_bw = 0, qe2k_links;

    if ((value == NULL) || (value_cong == NULL)) {
        return SOC_E_PARAM;
    }

    /* mask of logical xbars not belonging to this device */
    COMPILER_64_SET(xbars,0, (COMPILER_64_LO(xbars) & ~(SOC_SBX_CFG(unit)->module_custom1_links)));

    if (SOC_SBX_CFG(unit)->module_custom1_in_system == TRUE) {
        rv = soc_qe2000_custom1_config_burst_size_lines_get(unit, value, value_cong, xbars);
        return(rv);
    }

    switch(SOC_SBX_CFG(unit)->uFabricConfig) {
    case SOC_SBX_SYSTEM_CFG_DMODE:
        ts_opt = SB_FAB_DEVICE_BM3200_MIN_TIMESLOT_IN_NS_DMODE;
        /* in DMODE, for burst size calculations assume max bandwidth is
         * limited by DDR throughput.
         */
        max_bw = SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT;
        break;
    case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
        ts_opt = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_LEGACY;
        /* in VPORT_LEGACY, for burst size calculations assume max bandwidth is
         * limited by DDR throughput. If number of links is smaller, timeslot 
         * is adjusted to reflect lower overall bandwidth 
         */
        max_bw = SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT;
        break;
    case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
        ts_opt = opt_packlets = 0;
        rv = soc_sbx_fabric_get_timeslot_optimized(unit, &ts_opt, 
                                                   &opt_packlets);
        if (SOC_FAILURE(rv) || (ts_opt == 0) || (opt_packlets == 0)) {
            /* some internal error. Assume BW limited by DDR throughput */
            max_bw = SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT;
            rv = SOC_E_NONE;
            break;
        }
        /* This is the maximum number of links for any QE2K type node */
        qe2k_links = soc_property_get(unit, spn_QE2K_LINKS, 
                                      SB_FAB_DEVICE_QE2000_SFI_LINKS);
        max_bw = (opt_packlets * 8 * 16 * qe2k_links)/ts_opt;
        /* Above max_bw is max fabric BW. See if limited by DDR throughput*/
        if (max_bw > SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT) {
            max_bw = SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT;
        }
        break;
    default:
        rv = SOC_E_CONFIG;
        break;
    }
    SOC_IF_ERROR_RETURN(rv);

    /* roundup to next possible number of lines */
    *value = (ts_opt * max_bw)/(16 * 8) + ((ts_opt * max_bw)%(16 * 8) ? 1 : 0);
    if (*value > 0xff) { /* 255 is max possible burst size (8bit value) */
        *value = 0xff;
    }

    /* set congested burst size to 85% */
    (*value_cong) = ((*value) * 85) / 100;

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_qe2000_detach
 * Purpose:
 *     Cleanup and free all resources allocated during device specific
 *     initialization routine.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
soc_sbx_qe2000_detach(int unit)
{
    soc_driver_t  *driver;

    /* Free port info configuration */
    driver = SOC_DRIVER(unit);
    if (driver != NULL) {
        if ((driver->port_info != NULL) &&
            (driver->port_info != soc_port_info_bcm83200_a3)) {
            sal_free(driver->port_info);
            driver->port_info = NULL;
        }
    }

    return SOC_E_NONE;
}


int
soc_qe2000_spi_init(int unit, soc_sbx_config_t *cfg)
{

    HW_QE2000_INIT_ST *qe2000Init_sp;
    uint32 sts_ul;
    uint32 error = 0;
    uint32 subport = 0;

    qe2000Init_sp = sal_alloc(sizeof(HW_QE2000_INIT_ST), "Qe_init_st");
    if (qe2000Init_sp == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(qe2000Init_sp, 0, sizeof(*qe2000Init_sp));

    /*
     * QE initialization (Reset and SPI-4 only)
     */
    qe2000Init_sp->reset                 = SOC_SBX_CFG(unit)->reset_ul;
    qe2000Init_sp->userDeviceHandle      = SOC_SBX_CFG(unit)->DeviceHandle;
    qe2000Init_sp->nodeNum_ul            = SOC_SBX_CFG_QE2000(unit)->nodeNum_ul;
    qe2000Init_sp->uuRequeuePortsMask[0] = SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0];
    qe2000Init_sp->uuRequeuePortsMask[1] = SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1];
    qe2000Init_sp->bEiSpiFullPacketMode[0] = SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0];
    qe2000Init_sp->bEiSpiFullPacketMode[1] = SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1];

    qe2000Init_sp->uEiLines[0] = SOC_SBX_CFG_QE2000(unit)->uEiLines[0];
    qe2000Init_sp->uEiLines[1] = SOC_SBX_CFG_QE2000(unit)->uEiLines[1];
    qe2000Init_sp->Spi0RxChans_ul = SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[0];
    qe2000Init_sp->Spi0TxChans_ul = SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[0];
    for (subport=0; subport<HW_QE2000_NUM_SPI_SUBPORTS; subport++) {
	qe2000Init_sp->uSpiSubportSpeed[0][subport] = SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[0][subport];
    }
    qe2000Init_sp->Spi0RxCalM = SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0];
    qe2000Init_sp->Spi0TxCalM = SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0];

    qe2000Init_sp->Spi0MaxBurst1_ul     = 0xF;
    qe2000Init_sp->Spi0MaxBurst2_ul     = 6;
    COMPILER_64_SET(qe2000Init_sp->Spi0JumboMask_ull,0, 0xFFFF);
    qe2000Init_sp->Spi0AlignDelay_ul    = 30;
    qe2000Init_sp->Spi0RsInvert_ul  = 1;
    qe2000Init_sp->Spi0TsInvert_ul  = 1;
    /* 26 is Max sized ethernet header LLC-SNAP + VLAN tag */
    qe2000Init_sp->Spi0MinFrameSz_ul    = MB_MIN_FRM_SZ_K - 26 + MB_SHIMHDR_SZ_K - MB_CRC_SZ_K;
    qe2000Init_sp->Spi0MaxFrameSz_ul    = MB_MAX_FRM_SZ_NRML_K + MB_SHIMHDR_SZ_K;
    /*
    ** Set up to send retraining sequence 3 times, at a 41.94ms period.
    */
    qe2000Init_sp->Spi0DataMaxT_ul        = HW_QE1000_TRAIN_2TO24;
    qe2000Init_sp->Spi0Alpha_ul           = 2;

    /*  spi 1 */
    qe2000Init_sp->Spi1RxChans_ul = SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[1];
    qe2000Init_sp->Spi1TxChans_ul = SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[1];
    for (subport=0; subport<HW_QE2000_NUM_SPI_SUBPORTS; subport++) {
	qe2000Init_sp->uSpiSubportSpeed[1][subport] = SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[1][subport];
    }
    qe2000Init_sp->Spi1RxCalM = SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1];
    qe2000Init_sp->Spi1TxCalM = SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1];

    qe2000Init_sp->Spi1MaxBurst1_ul     = 0xF;
    qe2000Init_sp->Spi1MaxBurst2_ul     = 6;
    COMPILER_64_SET(qe2000Init_sp->Spi1JumboMask_ull,0,0x3);
    qe2000Init_sp->Spi1AlignDelay_ul    = 30;
    qe2000Init_sp->Spi1RsInvert_ul  = 1;
    qe2000Init_sp->Spi1TsInvert_ul  = 1;
    /* 26 is Max sized ethernet header LLC-SNAP + VLAN tag */
    qe2000Init_sp->Spi1MinFrameSz_ul    = MB_MIN_FRM_SZ_K - 26 + MB_SHIMHDR_SZ_K - MB_CRC_SZ_K;
    qe2000Init_sp->Spi1MaxFrameSz_ul    = MB_MAX_FRM_SZ_JMBO_K + MB_SHIMHDR_SZ_K;
    /*
    ** Set up to send retraining sequence 3 times, at a 41.94ms period.
    */
    qe2000Init_sp->Spi1DataMaxT_ul        = HW_QE1000_TRAIN_2TO24;
    qe2000Init_sp->Spi1Alpha_ul           = 2;

    sts_ul = hwQe2000InitSpi(qe2000Init_sp);
    if (sts_ul != HW_QE2000_STS_OK_K)
    {
	/* mbError(0, " %s: QE A initialization failed %x\n",
	   FUNCTION_NAME(), sts_ul); */
	if (!error) error = sts_ul;
    }

    sal_free(qe2000Init_sp);
    return(error);
}


int
soc_qe2000_init(int unit, soc_sbx_config_t *cfg)
{
    uint32 uStatus = 0;
    uint32 xcfgIdx;
    uint32 sfiIdx;
    uint32 sciIdx;
    uint32 fabricMode;
    HW_QE2000_INIT_ST *hwQe2000InitParams_sp;
    int32 nLink;
    sbLinkThresholdConfig_t linkThresholdConfig;
    sbFabStatus_t status;
    uint32 subport = 0;
    uint32 uData = 0;
    uint16 dev_id;
    uint8  rev_id;
    uint32 port;
    uint32 timeslot;
    uint64 uuZero = COMPILER_64_INIT(0,0);
    SOC_SBX_WARM_BOOT_DECLARE(uint32 _wb);


#if 0
    if (SAL_BOOT_BCMSIM) {
        soc_qe2000_mvt_init(unit);
        soc_sbx_txrx_init(unit);
        LOG_CLI((BSL_META_U(unit,
                            "Skipping QE2000 HW init \n")));
        return 0;
    }
#endif

    /*
     * Create a buffer to store custom statistics 
     */
    SOC_SBX_CFG(unit)->custom_stats = sal_alloc(sizeof(uint32) * SBX_FAB_ERRORS * 
						  (SB_FAB_DEVICE_QE2000_SFI_LINKS+SB_FAB_DEVICE_QE2000_SCI_LINKS), 
						  "custom stats");

    if (SOC_SBX_CFG(unit)->custom_stats == NULL) {
	return SOC_E_MEMORY;
    }

    hwQe2000InitParams_sp = sal_alloc(sizeof(HW_QE2000_INIT_ST), "Qe_init_st");
    if (hwQe2000InitParams_sp == NULL) {
	sal_free (SOC_SBX_CFG(unit)->custom_stats);
        return SOC_E_MEMORY;
    }

    /* mbMessage(1, "set up hwQe2000InitParams_sp\n"); */
    sal_memset(hwQe2000InitParams_sp, 0, sizeof(HW_QE2000_INIT_ST));

    hwQe2000InitParams_sp->reset                    = (SOC_SBX_CFG(unit)->reset_ul!=0)?TRUE:FALSE;
    hwQe2000InitParams_sp->userDeviceHandle         = SOC_SBX_CFG(unit)->DeviceHandle;

    /* Dmode and uEpochSizeInNs depends on fabric configuration */
    timeslot = soc_sbx_fabric_get_timeslot_size(unit, 
                              soc_property_get(unit, spn_QE2K_LINKS, 
                                               SB_FAB_DEVICE_QE2000_SFI_LINKS),
                              SOC_SBX_CFG_QE2000(unit)->bHalfBus,
                              soc_feature(unit, soc_feature_hybrid));
    switch (fabricMode = soc_property_get(unit, spn_FABRIC_CONFIGURATION, 
                                          SOC_SBX_SYSTEM_CFG_DMODE)) {
	case SOC_SBX_SYSTEM_CFG_DMODE:
	    /* DMode (Default)
	     * - Bm3200 + Qe2000
	     */
	    hwQe2000InitParams_sp->bDmode = TRUE;           /* Dmode by default */
	    hwQe2000InitParams_sp->uEpochSizeInNs = SOC_SBX_CFG(unit)->epoch_length_in_timeslots * timeslot;
	    break;
	case SOC_SBX_SYSTEM_CFG_VPORT:
	    /* Vport
		 * - Bm9600 + Qe4000
		 */
	    break;
	case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
	case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
	    /* Vport Legacy
	     * - Bm9600 + Qe2000
	     */
	    /* Vport Mix
	     * - Bm9600 + Qe2000 + Qe4000
	     */
	    hwQe2000InitParams_sp->bDmode = TRUE;           /* Dmode by default */
	    hwQe2000InitParams_sp->uEpochSizeInNs = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS * timeslot;
	    break;
	default:
	    LOG_ERROR(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "soc_sbx_init: unit %d unknown"
	                           " fabric_configuration %d"),
	               unit, fabricMode));
	    break;
    }
    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	/* TME mode use fixed 1ms Epoch size */
	hwQe2000InitParams_sp->uEpochSizeInNs = (SB_FAB_TME_EPOCH_IN_TIMESLOTS * 
                                             SB_FAB_TME_MIN_TIMESLOT_IN_NS);
    }

    hwQe2000InitParams_sp->uClockSpeedInMHz         = SOC_SBX_CFG(unit)->uClockSpeedInMHz;
    hwQe2000InitParams_sp->uNodeId                  = SOC_SBX_CFG_QE2000(unit)->nodeNum_ul;
    switch (SOC_SBX_CFG(unit)->bTmeMode) {
    case SOC_SBX_QE_MODE_FIC:
	hwQe2000InitParams_sp->bStandAlone = FALSE;
	hwQe2000InitParams_sp->bHybrid = FALSE;
	break;
    case SOC_SBX_QE_MODE_TME:
	hwQe2000InitParams_sp->bStandAlone = TRUE;
	hwQe2000InitParams_sp->bHybrid = FALSE;
	break;
    case SOC_SBX_QE_MODE_HYBRID:
	hwQe2000InitParams_sp->bStandAlone = FALSE;
	hwQe2000InitParams_sp->bHybrid = TRUE;
	break;
    default:
	LOG_ERROR(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "soc_sbx_init: unit %d unknown tme_mode %d"),
	           unit, SOC_SBX_CFG(unit)->bTmeMode));
	break;
    }

    hwQe2000InitParams_sp->hybridModeQueueDemarcation = 0;
    if (hwQe2000InitParams_sp->bHybrid == TRUE) {
        hwQe2000InitParams_sp->hybridModeQueueDemarcation = SOC_SBX_HYBRID_DEMARCATION_QID(unit);
    }

    hwQe2000InitParams_sp->bPmDdrTrain              = TRUE;
    hwQe2000InitParams_sp->uSciDefaultBmId          = SOC_SBX_CFG(unit)->uActiveScId;

    hwQe2000InitParams_sp->bEgressFifoIndependentFlowControl =
        (soc_qe2000_features(unit, soc_feature_egr_independent_fc)) ? TRUE : FALSE;

    /*
     * If we are in Cmode, temporarily set the link enable to be all 1K.
     * This may change after card install.  If we are in Dmode, there
     * are no QE1000s.
     */
    if ((hwQe2000InitParams_sp->bStandAlone) ||
        (hwQe2000InitParams_sp->bDmode)) {
        hwQe2000InitParams_sp->bFabCrc32          = TRUE;
        hwQe2000InitParams_sp->uQe1KLinkEnMask    = 0;
    } else {
        hwQe2000InitParams_sp->bFabCrc32          = FALSE;
        hwQe2000InitParams_sp->uQe1KLinkEnMask    = 0; /* 0x3ffff */;
    }

    /* mbMessage(1, "continuing setting up hwQe2000InitParams_sp\n");  */

    /* deprecate brdType, using soc_properties to configure device attributes
     */

    hwQe2000InitParams_sp->uSpiRefClockSpeed[0] = SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0];
    hwQe2000InitParams_sp->uSpiClockSpeed[0] = SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0];
    hwQe2000InitParams_sp->uSpiRefClockSpeed[1] = SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1];
    hwQe2000InitParams_sp->uSpiClockSpeed[1] = SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1];
    hwQe2000InitParams_sp->uNumPhySpiPorts[0]     = SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0];
    hwQe2000InitParams_sp->uNumPhySpiPorts[1]     = SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1];
    for (subport=0; subport<HW_QE2000_NUM_SPI_SUBPORTS; subport++) {
	hwQe2000InitParams_sp->uSpiSubportSpeed[0][subport] = SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[0][subport];
	hwQe2000InitParams_sp->uSpiSubportSpeed[1][subport] = SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[1][subport];
    }
    hwQe2000InitParams_sp->bEiSpiFullPacketMode[0] = SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0];
    hwQe2000InitParams_sp->bEiSpiFullPacketMode[1] = SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1];
    hwQe2000InitParams_sp->uEiLines[0] = SOC_SBX_CFG_QE2000(unit)->uEiLines[0];
    hwQe2000InitParams_sp->uEiLines[1] = SOC_SBX_CFG_QE2000(unit)->uEiLines[1];
    hwQe2000InitParams_sp->uuRequeuePortsMask[0]  = SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0];
    hwQe2000InitParams_sp->uuRequeuePortsMask[1]  = SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1];
    hwQe2000InitParams_sp->bPmHalfBus             = SOC_SBX_CFG_QE2000(unit)->bHalfBus;
    hwQe2000InitParams_sp->bPmRunSelfTest         = SOC_SBX_CFG(unit)->bRunSelfTest; /* TBD: Appears to be broken in half bus mode */
    hwQe2000InitParams_sp->bRunLongDdrMemoryTest  = SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest;
    hwQe2000InitParams_sp->uRbSpi0PortCount       = SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0];
    for (port = 0; port < SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; port++) {
        hwQe2000InitParams_sp->uInterleaveBurstSize[port] = SOC_SBX_CFG_QE2000(unit)->uInterleaveBurstSize[port];
    }
    hwQe2000InitParams_sp->uQsMaxNodes            = SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes;
    assert( hwQe2000InitParams_sp->uQsMaxNodes <= 64 );
    hwQe2000InitParams_sp->spMode = SOC_SBX_CFG(unit)->sp_mode;

    hwQe2000InitParams_sp->bQm512MbDdr2           = SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2;
    hwQe2000InitParams_sp->uQmMaxArrivalRateMbs   = SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs;
    hwQe2000InitParams_sp->nGlobalShapingAdjustInBytes = SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes;
    hwQe2000InitParams_sp->bSv2_5GbpsLinks        = SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks;
    hwQe2000InitParams_sp->uEgMVTSize             = SOC_SBX_CFG_QE2000(unit)->uEgMVTSize;
    hwQe2000InitParams_sp->uEgMcDropOnFull        = SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull;
    hwQe2000InitParams_sp->uEiPortInactiveTimeout = SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout;
    hwQe2000InitParams_sp->uScGrantoffset         = SOC_SBX_CFG_QE2000(unit)->uScGrantoffset;
    hwQe2000InitParams_sp->uScGrantDelay          = SOC_SBX_CFG_QE2000(unit)->uScGrantDelay;


    hwQe2000InitParams_sp->uEgMcEfTimeout     = SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout;
    hwQe2000InitParams_sp->uEgMcNefTimeout    = SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout;

    hwQe2000InitParams_sp->nQueuesPerShaperIngress = SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress;
    hwQe2000InitParams_sp->bMixHighAndLowRateFlows = SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows;
    hwQe2000InitParams_sp->uSfiTimeslotOffsetInClocks = SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks;
    hwQe2000InitParams_sp->uSfiDataLinkInitMask    = 0;  /* QE1000 related, no use since sdk not support QE1000*/
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
	hwQe2000InitParams_sp->uSfiHeaderPad       = 3;  /* value from SV for QE2K Interop */
    } else {
	hwQe2000InitParams_sp->uSfiHeaderPad       = 0;
    }

    for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
      
	hwQe2000InitParams_sp->uEgressMcastEfDescFifoSize[port] = SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port];	
	hwQe2000InitParams_sp->bEgressMcastEfDescFifoInUse[port] = SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[port];							
	hwQe2000InitParams_sp->uEgressMcastNefDescFifoSize[port] = SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port];
	hwQe2000InitParams_sp->bEgressMcastNefDescFifoInUse[port] = SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[port];	
    }

    
    /* Setup Egress priorities */
    hwQe2000InitParams_sp->uEgHiMcPri = soc_property_get(unit, spn_QE_HI_MC_PRIORITY, -1);
    if (hwQe2000InitParams_sp->uEgHiMcPri == (uint16)-1) {
        hwQe2000InitParams_sp->uEgHiMcPri = soc_property_get(unit, spn_EGRESS_MCAST_EF_PRI, 1);
    }

    hwQe2000InitParams_sp->uEgLoMcPri = soc_property_get(unit, spn_QE_LO_MC_PRIORITY, -1);
    if (hwQe2000InitParams_sp->uEgLoMcPri == (uint16)-1) {
        hwQe2000InitParams_sp->uEgLoMcPri = soc_property_get(unit, spn_EGRESS_MCAST_NEF_PRI, 2);
    }

    /* EP Configuration */
    hwQe2000InitParams_sp->bEpDisable = SOC_SBX_CFG_QE2000(unit)->bEpDisable;

    status = soc_qe2000_burst_size_lines_get(unit, 
					     &hwQe2000InitParams_sp->nLinesPerTimeslot,
					     &hwQe2000InitParams_sp->nLinesPerTimeslotCongested, uuZero);
    if (status) {
      sal_free(hwQe2000InitParams_sp);
      return status;
    }

    if(hwQe2000InitParams_sp->bPmHalfBus) {
        hwQe2000InitParams_sp->nLinesPerTimeslot <<= 1;
        hwQe2000InitParams_sp->nLinesPerTimeslotCongested <<= 1;
    }
    hwQe2000InitParams_sp->nLinesPerTimeslotQe1k = 0;

    if (SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks == -1) {
	/* Bug 24260, if the user doesn't set this value, set it to 18 clocks for full bus and 26 for half bus */
	if(SOC_SBX_CFG_QE2000(unit)->bHalfBus == TRUE) {
	    SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = 26;
	} else {
	    SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = 18;
	}
    }
    hwQe2000InitParams_sp->uScTxdmaSotDelayInClocks = SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks;

    status = GetLinkThresholdConfig(SOC_SBX_CFG(unit)->uLinkThresholdIndex,
				    &linkThresholdConfig);

    if (status == SB_FAB_STATUS_OK) {
        hwQe2000InitParams_sp->uSiLsThreshold = linkThresholdConfig.uLsThreshold;
        hwQe2000InitParams_sp->uSiLsWindow = linkThresholdConfig.uLsWindowIn256Clocks;
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "QE200 uLinkThresholdIndex(%d) initialization "
                               "parameter not initialized correctly."
                               " Should be [0,100]\n"),
                   SOC_SBX_CFG(unit)->uLinkThresholdIndex));
    }

    /* initialize xcfg to loopback (0x7f) to achieve time alignment on the
     * SFI links
     */
    if (fabricMode == 0) {
	for (sfiIdx = 0; sfiIdx < HW_QE2000_NUM_SFI_LINKS; sfiIdx++) {
	    for (xcfgIdx = 0; xcfgIdx < HW_QE2000_NUM_XCFG_REMAP_ENTRIES; xcfgIdx++) {
		hwQe2000InitParams_sp->uSfiXconfig[sfiIdx][xcfgIdx] = 0x7f;
	    }
	}
    } else {
	/* Vport Legacy
	 * - Bm9600 + Qe2000 + Qe4000
	 * Map XCFG 0-71 to itself, xcfg table is on Bm9600 now
	 */
	for (sfiIdx = 0; sfiIdx < HW_QE2000_NUM_SFI_LINKS; sfiIdx++) {
	    for (xcfgIdx = 0; xcfgIdx < HW_QE2000_NUM_XCFG_REMAP_ENTRIES; xcfgIdx++) {
  	        /* Polaris lower bit is A/B plane */
		if (xcfgIdx < 64) {
		    hwQe2000InitParams_sp->uSfiXconfig[sfiIdx][xcfgIdx] = xcfgIdx;
		} else {
		    hwQe2000InitParams_sp->uSfiXconfig[sfiIdx][xcfgIdx] = 0xff;
		}
	    }
	}
    }

    for (sciIdx = 0; sciIdx < HW_QE2000_NUM_SCI_LINKS; sciIdx++) {
        for (sfiIdx = 0; sfiIdx < HW_QE2000_NUM_SFI_LINKS; sfiIdx++) {
            hwQe2000InitParams_sp->uSciLinkEnRemap[sciIdx][sfiIdx] = sfiIdx;
        }
    }

    for (sciIdx = 0; sciIdx < HW_QE2000_NUM_SCI_LINKS; sciIdx++) {
        for (sfiIdx = 0; sfiIdx < HW_QE2000_NUM_SFI_LINKS; sfiIdx++) {
            hwQe2000InitParams_sp->uSciLinkStatusRemap[sciIdx][sfiIdx] = sfiIdx;
        }
    }

    /* version specific init setup */
    uData = SAND_HAL_READ((sbhandle)unit, KA, PC_REVISION);
    rev_id = SAND_HAL_GET_FIELD(KA, PC_REVISION, REVISION, uData);
    dev_id = SAND_HAL_GET_FIELD(KA, PC_REVISION, DEVICE_ID, uData);
    COMPILER_REFERENCE(rev_id);
    COMPILER_REFERENCE(dev_id);

    hwQe2000InitParams_sp->bWorkaround25276Enable = TRUE;

    /* demand scale factor */
    hwQe2000InitParams_sp->nDemandScale         = SOC_SBX_CFG(unit)->demand_scale;

    /* length adjust format */
     hwQe2000InitParams_sp->uPacketAdjustFormat = SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat;	

    /* mbMessage(1, "done setting up hwQe2000InitParams_sp\n"); */

    if (!SAL_BOOT_BCMSIM) {
        uStatus = hwQe2000Init(hwQe2000InitParams_sp);
    } else {
        /* this sets the spi port control regs - EI_SPI[0/1]_CONFIG0 regs */
        soc_qe2000_egress_to_spi_port_set(unit,
                                          SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0],
                                          0, /* spi0 */
                                          SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);


        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, EG_MC_CONFIG0, MCGROUP_SIZE, 0, SOC_SBX_CFG_QE2000(unit)->uEgMVTSize);
        SAND_HAL_WRITE((sbhandle)unit, KA, EG_MC_CONFIG0, uData);

        /* skip rest of QE HW init */
        uStatus = HW_QE2000_STS_OK_K;
    }
    sal_free(hwQe2000InitParams_sp);

    /* mbMessage(1, "done calling hwQe2000Init() status = %d\n", (int32)uStatus); */

    if (uStatus != HW_QE2000_STS_OK_K) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Qe2000 initialization failed with"
                               " uStatus(0x%x) will return %d\n"),
                   uStatus, SB_FAB_STATUS_QE2000_INIT_FAILED));
        return (uStatus);
    }

    for (nLink=0; nLink<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
         soc_qe2000_config_linkdriver((sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, nLink,
                          &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nLink]));
    }

    /*
     * Initialize SPI
     */
    SOC_SBX_CFG(unit)->reset_ul = 0;          /* Should not reset QE when init SPI */
    uStatus = soc_qe2000_spi_init(unit, cfg);

    if (uStatus != HW_QE2000_STS_OK_K) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_qe2000_init: unit %d "
                              "unable to spi init e=%d\n"), unit, uStatus));
        LOG_CLI((BSL_META_U(unit,
                            "failed spi init\n")));
        return uStatus;
    }

    SOC_SBX_WARM_BOOT_IGNORE((int)(sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, _wb);
    uStatus = soc_sbx_txrx_init(unit);
    SOC_SBX_WARM_BOOT_OBSERVE((int)(sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, _wb);

    if (uStatus != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_qe2000_init: unit %d "
                              "TXRX init failed e=%d\n"), unit, uStatus));
        LOG_CLI((BSL_META_U(unit,
                            "failed txrx init\n")));
        return uStatus;
    }

    SOC_SBX_WARM_BOOT_IGNORE((int)(sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, _wb);
    SAND_HAL_RMW_FIELD((sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, KA,
                       PC_CONFIG, CII_ENABLE, 0x1);
    SAND_HAL_RMW_FIELD((sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, KA,
                       PC_INTERRUPT_MASK, PCI_COMPLETION_DISINT, 0);
    SOC_SBX_WARM_BOOT_OBSERVE((int)(sbFabUserDeviceHandle_t)SOC_SBX_CFG(unit)->DeviceHandle, _wb);

    {
	/* Add 16 queues for each destination port (0-49) */
	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	    SOC_SBX_CONTROL(unit)->numQueues = 1024; /* 16 cos levels */
	} else {
	    SOC_SBX_CONTROL(unit)->numQueues = SBX_MAX_QID/SBX_MAX_COS; /* 8 or 4 cos levels */
	}
    }

    if (soc_property_get(unit, spn_QE_MVT_OLD_CONFIGURATION, 0)) { /* to be removed */
        soc_qe2000_mvt_init(unit);
    }
    else {
        soc_qe2000_mvt_initialization(unit);
        soc_qe2000_bd_mvt_init(unit); /* to be REMOVED */
    }

    SOC_SBX_CONTROL(unit)->ucodetype = soc_sbx_configured_ucode_get(unit);

    /* Initialize SOC link control module */
    soc_linkctrl_init(unit, &soc_linkctrl_driver_sbx);

    return(uStatus);
}


soc_error_t
soc_qe2000_config_linkdriver(int unit, int nLink, sbLinkDriverConfig_t *pLinkDriverConfig)
{
    sbLinkSpecificDriverConfig_t specificConfig;
    sbFabStatus_t status;
    uint32 uData;
    int32 nSfiLink;
    int32 nSciLink;

    if ( nLink < 0 || nLink > SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Bad link(%d) requested for QE2000."
                               " Valid range is [0,%d)\n"),
                   nLink, SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS));
        return SB_FAB_STATUS_BAD_LINK_ID;
    }

    status = GetLinkSpecificConfig((sbFabUserDeviceHandle_t)unit, nLink, pLinkDriverConfig, &specificConfig);
    if ( status != SB_FAB_STATUS_OK ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Could not get link specific configuration"
                               " for link(%d)\n"), nLink));
        return status;
    }

    /* now that we have nDtx, etc. program the correct link config and we're done */
    {

        if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
            nSfiLink = nLink;

            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF,
                                         nSfiLink, SF0_SI_CONFIG3);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DEQ,   uData,
                                       specificConfig.u.rambusRs2314.nDeq);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DTX,   uData,
                                       specificConfig.u.rambusRs2314.nDtx);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, HIDRV, uData,
                                       specificConfig.u.rambusRs2314.nHiDrv);
            uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, LODRV, uData,
                                       specificConfig.u.rambusRs2314.nLoDrv);

            SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink,
                                  SF0_SI_CONFIG3, uData);
        } else {
            /* SCI link 0 or 1 */
            nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

            if ( nSciLink == 0 ) {
                uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG3);

                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DEQ,   uData,
                                           specificConfig.u.rambusRs2314.nDeq);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DTX,   uData,
                                           specificConfig.u.rambusRs2314.nDtx);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, HIDRV, uData,
                                           specificConfig.u.rambusRs2314.nHiDrv);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, LODRV, uData,
                                           specificConfig.u.rambusRs2314.nLoDrv);

                SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG3, uData);
            } else {
                uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG3);

                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DEQ,   uData,
                                           specificConfig.u.rambusRs2314.nDeq);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DTX,   uData,
                                           specificConfig.u.rambusRs2314.nDtx);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, HIDRV, uData,
                                           specificConfig.u.rambusRs2314.nHiDrv);
                uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, LODRV, uData,
                                           specificConfig.u.rambusRs2314.nLoDrv);

                SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG3, uData);
            }
        }
    }

    return SB_FAB_STATUS_OK;
}

soc_error_t
soc_qe2000_q2ec_set(int unit, int nQueue, int nMc, int nNode, int nEPort,
		    int nECos)
{
    int nStatus;
    uint32 nQ2EC;

    QE2000_QUEUE_VALID_CHECK(nQueue);

    if((nMc > 1) || (nNode > 63) || (nEPort > 63) || (nECos > 15)) {
        return SOC_E_PARAM;
    }

    nQ2EC = ((nMc & 0x1) << 16) | ((nNode & 0x3F) << 10) |
      ((nEPort & 0x3F) << 4) | (nECos & 0xF);

    /* LOG_CLI((BSL_META_U(unit,
                           "Q2EC queue(%d) mapped to node(%d)/port(%d)/cos(%d)\n"), nQueue, nNode, nEPort, nECos)); */
    nStatus = soc_qe2000_qs_mem_write(unit, 0x4, nQueue, nQ2EC);

    if( 0 != nStatus ){
	return( SOC_E_NOT_FOUND );
    }

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_q2ec_get(int unit, int nQueue, int *pnMc, int *pnNode, int *pnEPort,
        int *pnECos)
{
    int nStatus;
    uint32 uRaw;

    QE2000_QUEUE_VALID_CHECK(nQueue);

    if ( !pnMc || !pnNode || !pnEPort || !pnECos ) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x4, nQueue, &uRaw);

    if( 0 != nStatus ){
	return( SOC_E_NOT_FOUND );
    }

    *pnMc     = !!(uRaw & 0x10000);
    *pnNode  = (uRaw >> 10) & 0x3f;
    *pnEPort = (uRaw >> 4) &0x3f;
    *pnECos  = (uRaw & 0xF);

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_rate_a_get(int unit, int queue, int *value)
{
    int nStatus;
    uint32 uRaw;

    QE2000_QUEUE_VALID_CHECK(queue);

    if (!value ) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x0, queue, &uRaw);

    if( 0 != nStatus ) {
	return( SOC_E_NOT_FOUND );
    }

    *value   = uRaw;

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_rate_a_set(int unit, int queue, int value)
{
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x0, queue, value);

    if( 0 != nStatus ) {
	return( SOC_E_NOT_FOUND );
    }

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_rate_b_get(int unit, int queue, uint32 *value)
{
    int nStatus;
    uint32 uRaw;

    QE2000_QUEUE_VALID_CHECK(queue);

    if (!value ) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x1, queue, &uRaw);

    if( 0 != nStatus ) {
      return( SOC_E_NOT_FOUND );
    }

    *value   = uRaw;

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_rate_b_set(int unit, int queue, uint32 value)
{
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x1, queue, value);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_credit_get(int unit, int queue, uint32 *value)
{
    int nStatus;
    uint32 uRaw;

    QE2000_QUEUE_VALID_CHECK(queue);

    if (!value) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x2, queue, &uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    *value   = uRaw;

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_credit_set(int unit, int queue, uint32 value)
{
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if(value > 0x1FFFFFF) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_write(unit, 0x2, queue, value & 0x1FFFFFF);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_shape_maxburst_get (int unit, int queue, int *shape_enable, int *maxburst)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);
    if(!shape_enable || !maxburst) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x7, queue, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *shape_enable = (uRaw >> 23) & 0x1;
    *maxburst = uRaw & 0x7FFFFF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_shape_maxburst_set (int unit, int queue, int shape_enable, int maxburst)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if((shape_enable > 1) || (maxburst > 0x7FFFFF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((shape_enable & 0x1) << 23) | (maxburst & 0x7FFFFF);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x7, queue, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_shape_bucket_get (int unit, int queue, int *shape_bucket)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);
    if(!shape_bucket) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x8, queue, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *shape_bucket = uRaw & 0xFFFFFF;
    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_shape_bucket_set (int unit, int queue, int shape_bucket)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    uRaw = (shape_bucket & 0xFFFFFF);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x8, queue, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_get (int unit, int queue, int *nonempty, int *anemic_event, int *ef_event, int *cnt)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);
    if(!nonempty || !anemic_event || !ef_event || !cnt) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x9, queue, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *nonempty     = (uRaw >> 11) & 0x1;
    *anemic_event = (uRaw >> 10) & 0x1;
    *ef_event     = (uRaw >>  8) & 0x1;
    *cnt          = uRaw & 0xFF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_set (int unit, int queue, int nonempty, int anemic_event, int ef_event, int cnt)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if((nonempty > 1) || (anemic_event > 1) || (ef_event > 1) ||
       (cnt > 0xFF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((nonempty & 0x1) << 10) | ((anemic_event & 0x1) << 9) |
        ((ef_event & 0x1) << 8) | (cnt & 0xFF);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x9, queue, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_thresh_key_get (int unit, int queue, int *age_thresh_key)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);
    if(!age_thresh_key) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0xA, queue, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *age_thresh_key = uRaw & 0x1F;
    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_thresh_key_set (int unit, int queue, int age_thresh_key)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if(age_thresh_key > 0x1F) {
        return SOC_E_PARAM;
    }

    uRaw = age_thresh_key & 0x1F;

    nStatus = soc_qe2000_qs_mem_write(unit, 0xA, queue, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_thresh_get (int unit, int idx, int *anemic_thresh, int *ef_thresh)
{
    uint32 uRaw;
    int nStatus;

    if((idx > 31) || !anemic_thresh || !ef_thresh) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0xB, idx, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *anemic_thresh = (uRaw >> 8) & 0xFF;
    *ef_thresh      = uRaw & 0xFF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_age_thresh_set (int unit, int idx, int anemic_thresh, int ef_thresh)
{
    uint32 uRaw;
    int nStatus;

    if((idx > 31) || (anemic_thresh > 0xFF) || (ef_thresh > 0xFF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((anemic_thresh & 0xFF) << 8) | ((ef_thresh & 0xFF));

    nStatus = soc_qe2000_qs_mem_write(unit, 0xB, idx, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_pri_lut_get (int unit, int idx, int *pri, int *next_pri)
{
    uint32 uRaw;
    int nStatus;

    if((idx > 8191) || !pri || !next_pri) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0xC, idx, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *pri      = (uRaw >> 4) & 0xF;
    *next_pri = uRaw & 0xF;

    return SOC_E_NONE;
}

extern soc_error_t
soc_qe2000_pri_lut_set (int unit, int idx, int pri, int next_pri)
{
    uint32 uRaw;
    int nStatus;

    if((idx > 8191) || (pri > 0xF) || (next_pri > 0xF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((pri & 0xF) << 4) | ((next_pri & 0xF));

    nStatus = soc_qe2000_qs_mem_write(unit, 0xC, idx, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_priority_get (int unit, int idx, int *shaped, int *pri, int *next_pri)
{
    uint32 uRaw;
    int nStatus;

    /* even though priority table's size is only 16*4224, the addrs is 128K
       entry, some multicast address space is skipped
    */

    if((idx > (128*1024-1)) || !shaped || !pri || !next_pri) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0xD, idx, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *shaped   = (uRaw >> 8) & 0x1;
    *pri      = (uRaw >> 4) & 0xF;
    *next_pri = uRaw & 0xF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_priority_set (int unit, int idx, int shaped, int pri, int next_pri)
{
    uint32 uRaw;
    int nStatus;

    /* even though priority table's size is only 16*4224, the addrs is 128K
       entry, some multicast address space is skipped
    */

    if((idx > (128*1024-1)) || (shaped > 1) || (pri > 0xF) || (next_pri > 0xF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((shaped & 0x1) << 8) | ((pri & 0xF) << 4) | ((next_pri & 0xF));

    nStatus = soc_qe2000_qs_mem_write(unit, 0xD, idx, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_lastsentpri_get (int unit, int idx, int *pri, int *next_pri)
{
    uint32 uRaw;
    int nStatus;

    if((idx > (16*4224-1)) || !pri || !next_pri) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0xF, idx, &uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    *pri      = (uRaw >> 4) & 0xF;
    *next_pri = uRaw & 0xF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_lastsentpri_set (int unit, int idx, int pri, int next_pri)
{
    uint32 uRaw;
    int nStatus;

    if((idx > (16*4224-1)) || (pri > 0xF) || (next_pri > 0xF)) {
        return SOC_E_PARAM;
    }

    uRaw = ((pri & 0xF) << 4) | ((next_pri & 0xF));

    nStatus = soc_qe2000_qs_mem_write(unit, 0xF, idx, uRaw);
    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_depth_length_get(int unit, int queue, int * nDepth, int *nPktLen)
{
    int nStatus;
    uint32 uRaw;

    QE2000_QUEUE_VALID_CHECK(queue);

    if( !nDepth || !nPktLen ) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x3, queue, &uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    *nPktLen = (uRaw >> 4) & 0x3;
    *nDepth = uRaw & 0xF;

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_depth_length_set(int unit, int queue, int nDepth, int nPktLen)
{
    int nStatus;
    uint32 nDepthLength;

    QE2000_QUEUE_VALID_CHECK(queue);

    if((nDepth > 0xF) || (nPktLen > 0x3)) {
        return SOC_E_PARAM;
    }

    nDepthLength = (nDepth & 0xF) | ((nPktLen & 0x3) << 4);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x3, queue, nDepthLength);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return( SOC_E_NONE );
}


void
soc_qe2000_egress_to_spi_port_set(int unit, int nEPort, int nSpi, int nSpiPort)
{
    int nSpi0Base = 0;
    int nSpi1Base = 0;

    if(nSpi == 0) {
	nSpi1Base = nEPort;
	assert(nEPort == nSpiPort);
    } else {
	nSpi1Base = nEPort - nSpiPort;
    }

    SAND_HAL_RMW_FIELD((sbhandle)unit, KA, EI_SPI0_CONFIG0, PORT_OFFSET, nSpi0Base);
    SAND_HAL_RMW_FIELD((sbhandle)unit, KA, EI_SPI1_CONFIG0, PORT_OFFSET, nSpi1Base);
}

void
soc_qe2000_egress_to_spi_port_get(int unit, int nEPort, int *nSpi,
				  int *nSpiPort)
{
    uint32 data   = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_CONFIG0);
    int nSpi0Base = SAND_HAL_GET_FIELD(KA, EI_SPI0_CONFIG0, PORT_OFFSET,
                                          data);
    int nSpi1Base = SAND_HAL_GET_FIELD(KA, EI_SPI1_CONFIG0, PORT_OFFSET,
                                          data);

    /*
     * Assumes that spibase1 is greater than spibase0
     */
    if(nEPort >= nSpi1Base && (nSpi1Base != 0)) {
	*nSpi     = 1;
	*nSpiPort = nEPort-nSpi1Base;
    } else {
	*nSpi     = 0;
	*nSpiPort = nEPort-nSpi0Base;
    }
}

soc_error_t
soc_qe2000_e2q_set(int unit, int nMc, int nENode, int nEPort,
		   int nQueue, int nEnable)
{
    int nE2Q;
    int nStatus;
    int nAddr;

    /* LOG_CLI((BSL_META_U(unit,
                           "E2Q node(%d)/port(%d) mapped to queue(%d)\n"), nENode, nEPort, nQueue)); */
    nE2Q = ((nEnable & 0x1) << 14) | (nQueue & 0x3FFF);
    nAddr = ((nMc & 0x1) << 12) | ((nENode & 0x3F) << 6) | (nEPort & 0xFFF);

    nStatus = soc_qe2000_qs_mem_write(unit, 0xE, nAddr, nE2Q);

    if (0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_e2q_get(int unit, int nMc, int uENode, int uEPort,
		   int *pnQueue, int *pnEnable)
{
    uint32 uRaw;
    int nStatus;
    int nAddr;

    if (!pnQueue || !pnEnable) {
        return SOC_E_PARAM;
    }

    nAddr = ((nMc & 0x1) << 12) | ((uENode & 0x3F) << 6) | (uEPort & 0xFFF);

    nStatus = soc_qe2000_qs_mem_read(unit, 0xE, nAddr, &uRaw);

    if (0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    *pnQueue =  uRaw & 0x3FFF;
    *pnEnable = !!(uRaw & (1<<14));

    return( SOC_E_NONE );
}

soc_error_t
soc_qe2000_queue_para_get(int unit, int queue, int *local, int *hold_ts, int *q_type)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if( !local || !hold_ts || !q_type) {
        return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x5, queue, &uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    *local   = (uRaw >> 7) & 0x1;
    *hold_ts = (uRaw >> 4) & 0x7;
    *q_type  = uRaw & 0xF;

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_queue_para_set(int unit, int queue, int local, int hold_ts, int q_type)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if((local > 1) || (hold_ts > 7) || (q_type > 15)) {
        return SOC_E_PARAM;
    }

    uRaw = ((local & 0x1) << 7) | ((hold_ts & 0x7) << 4) | (q_type & 0xF);

    nStatus = soc_qe2000_qs_mem_write(unit, 0x5, queue, uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_shape_rate_get(int unit, int queue, int *mode, int *shape_rate)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if (!mode || !shape_rate) {
	return SOC_E_PARAM;
    }

    nStatus = soc_qe2000_qs_mem_read(unit, 0x6, queue, &uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    *mode = (uRaw >> 23) & 0x1;
    if(*mode) {
        /* dynamic */
        *shape_rate = uRaw & 0xF;
    } else {
        /* static */
        *shape_rate = uRaw & 0x3FFFFF;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_shape_rate_set(int unit, int queue, int mode, int shape_rate)
{
    uint32 uRaw;
    int nStatus;

    QE2000_QUEUE_VALID_CHECK(queue);

    if((mode > 1) || (shape_rate > 0x3FFFFF) || ((mode == 1) && shape_rate > 0xF)) {
        return SOC_E_PARAM;
    }

    if(mode) {
        /* dynamic */
        uRaw = ((mode & 0x1) << 23) | ((shape_rate & 0xF));

	/* This mode is not supported for now */
	return SOC_E_PARAM;
    } else {
        /* static */
        uRaw = ((mode & 0x1) << 23) | ((shape_rate & 0x3FFFFF));
    }

    nStatus = soc_qe2000_qs_mem_write(unit, 0x6, queue, uRaw);

    if( 0 != nStatus ) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

soc_error_t
soc_qe2000_queue_info_get(int unit, int q, int *mc, int *node, int *port,
			  int *cos, int* baseq, int *enabled)
{
    int nStatus;

    nStatus = soc_qe2000_q2ec_get(unit, q, mc, node, port, cos);

    if (nStatus != SOC_E_NONE) {
        return SOC_E_NOT_FOUND;
    }

    nStatus = soc_qe2000_e2q_get(unit, *mc, *node, *port, baseq, enabled);

    if (nStatus != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}


uint32  soc_qe2000_qs_lna_mem_read(int unit, int32 nAddr, int32 nTable,
				     uint32 *puData0, uint32 *puData1, uint32 *puData2, uint32 *puData3, uint32 *puData4) {
    uint32 ulTimeOut;
    uint32 ulCntrl;

    ulCntrl = SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1) |
        SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, REQ, 0x1) |
        SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, RD_WR_N, 0x1) |
        SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTable) |
        SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, nAddr);

    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, ulCntrl);

    ulTimeOut = 500;
    while(ulTimeOut--) {
        ulCntrl = SAND_HAL_READ_POLL((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL);
        if (SAND_HAL_GET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, ulCntrl)) {
            *puData0 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA0);
            *puData1 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA1);
            *puData2 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA2);
            *puData3 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA3);
            *puData4 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA4);
            SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, 0 );
            SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1));
            return 0;
        }
    }

    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, 0 );
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL,
                   SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1));

    LOG_CLI((BSL_META_U(unit,
                        "lna memory read timed out addr(0x%x)\n"), nAddr));
    return -1;
}


sbFabStatus_t soc_qe2000_qs_lna_mem_write(int unit, int32 nAddr, int32 nTable,
					  uint32 uData0, uint32 uData1, uint32 uData2, uint32 uData3, uint32 uData4) {
    uint32 ulTimeOut;
    uint32 ulCntrl;
    uint32 status = 0;

    {
        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA0, uData0);
        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA1, uData1);
        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA2, uData2);
        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA3, uData3);
        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA4, uData4);

        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL,
                   SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1));

        ulCntrl = SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1) |
            SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, REQ, 0x1) |
            SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, RD_WR_N, 0x0) |
            SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTable) |
            SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, nAddr);

        SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, ulCntrl);

        for (ulTimeOut = 500; ulTimeOut > 0; ulTimeOut--) {
            ulCntrl = SAND_HAL_READ_POLL((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL);
            if (SAND_HAL_GET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, ulCntrl)) {
                SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, 0 );
                SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL,
                               SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1));

                break;
            }
        }

        status = (ulTimeOut > 0) ? 0: -1;

        if (status  == -1) {
            SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL, 0 );
            SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_CTRL,
                   SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ACK, 0x1));

	    LOG_CLI((BSL_META_U(unit,
                                "lna memory write timed out addr(0x%x)\n"), nAddr));
        }


    }
    return(status);
}

int32
soc_qe2000_lna_mem_full_remap_table_write(int unit, int32 nVirtualNodePort, int32 nDestPortId) {
    int32 nAddr;
    int32 nPortLocation;
    uint32 uData[6]; /* make a little large to simplify data word position calc */
    int32 nVirtualNodeId = (nVirtualNodePort>>5)&0x1F;
    int32 nVirtualPortId = nVirtualNodePort&0x1F;
    sbZfFabQe2000LnaFullRemapEntry_t zfRemapEntry;

    if ( nVirtualPortId <= 24 ) {
        nAddr = (nVirtualNodeId<<1) | 1;
        nPortLocation = nVirtualPortId;
    } else {
        nAddr = (nVirtualNodeId<<1) | 0;
        nPortLocation = nVirtualPortId-25;
    }

    soc_qe2000_qs_lna_mem_read(unit, nAddr, 0x2, /* port remap table */
			       &uData[0],&uData[1],&uData[2],&uData[3],&uData[4]);

    sbZfFabQe2000LnaFullRemapEntry_Unpack(&zfRemapEntry, (uint8*)uData, 20);
    zfRemapEntry.m_nRemap[nPortLocation] = nDestPortId;

    sbZfFabQe2000LnaFullRemapEntry_Pack(&zfRemapEntry, (uint8*)uData, 20);

    soc_qe2000_qs_lna_mem_write(unit, nAddr, 0x2, /* port remap table */
				uData[0],uData[1],uData[2],uData[3],uData[4]);

    /* Only need this code for debug */
#if 000
    soc_qe2000_qs_lna_mem_read(unit, nAddr, 0x2, /* port remap table */
			       &uData[0],&uData[1],&uData[2],&uData[3],&uData[4]);

    sbZfFabQe2000LnaFullRemapEntry_Unpack(&zfRemapEntry, (uint8*)uData, 20);
#endif
    /* LOG_CLI((BSL_META_U(unit,
                           "vnode[%d]vport[%d]portid[%d]=%d\n"), nVirtualNodeId, nVirtualPortId, nPortLocation, zfRemapEntry.m_nRemap[nPortLocation])); */

    return 0;
}

int32
soc_qe2000_eg_mem_port_remap_table_write(int unit, int32 nVirtualNodePort,
					 sbBool_t bMc, sbBool_t bTme,
					 sbBool_t bEnableFifo, int32 nEgressFifoId) {
    uint32 uData=0;
    int32 status;
    int32 nAddr;

    nAddr = (nVirtualNodePort & 0x3FF);

    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA0, DATA, uData,
                               ((bMc&0x1)<<8) |
                               ((bEnableFifo&0x1)<<7) |
                               (nEgressFifoId&0x7F));

    status = soc_qe2000_eg_mem_write(unit, nAddr, 0x0 /* Port Remap nTable */, uData, 0, 0);
    return status;
}


soc_error_t
soc_qe2000_modid_set(int unit, int node)
{

    int sfi;
    SAND_HAL_RMW_FIELD((sbhandle)unit, KA, QS_LNA_CONFIG, NODE_ID, node);

    for (sfi=0; sfi<HW_QE2000_NUM_SFI_LINKS; sfi++) {
        SAND_HAL_RMW_FIELD_STRIDE(unit, KA, SF, sfi,
                                  SF0_CONFIG, NODE, node);
    }

    SAND_HAL_RMW_FIELD((sbhandle)unit, KA, SV_CONFIG0, NODE, node);

    hwQe2000ConfigMcSrcId((sbhandle)unit, node,
                          SOC_SBX_CONTROL(unit)->spi_subport_count[0]);

    return BCM_E_NONE;
}

soc_error_t
soc_qe2000_sfi_rd_wr_xcfg( int unit, int nRead, bcm_port_t port,
			   int modid, bcm_port_t* pXbport )
{
    bcm_error_t rv = BCM_E_NONE;
    uint32 uRegData;

    /* if write action, write the xconfig to the data reg for indirect
     * memory write
     */
    if (nRead == 0)
    {
        uRegData =
            SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, XCNFG,
			       *pXbport);

        SAND_HAL_WRITE_STRIDE_FORCE(unit, KA, SF, SOC_PORT_BLOCK_INDEX(unit, port),
			      SF0_XCNFG_REMAP_MEM_ACC_DATA,
                              uRegData);
    }

    uRegData =
        ( SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, ACK, 1) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, REQ, 1) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, RD_WR_N, nRead) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, modid) );

    SAND_HAL_WRITE_STRIDE_FORCE(unit, KA, SF, SOC_PORT_BLOCK_INDEX(unit, port),
			  SF0_XCNFG_REMAP_MEM_ACC_CTRL,
                          uRegData);

    /* wait for indirect memory access to complete */
    rv = soc_qe2000_sfi_xcnfg_mem_ack_wait( unit, port, BCM_XBAR_WAIT_XCNFG_TIME );

    if (BCM_SUCCESS(rv)) {
        uRegData = SAND_HAL_READ_STRIDE(unit, KA, SF, SOC_PORT_BLOCK_INDEX(unit, port),
					SF0_XCNFG_REMAP_MEM_ACC_DATA);
        *pXbport = SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA,
                                      XCNFG, uRegData);
    }

    return rv;
}

soc_error_t
soc_qe2000_sfi_xcnfg_mem_ack_wait( int unit, bcm_port_t port,
				   uint32 uMsTimeout )
{
    uint32 uRegData = 0;
    bcm_error_t rv = BCM_E_NONE;
    int i;

    for (i = 0; i < uMsTimeout; i++) {
        uRegData = SAND_HAL_READ_STRIDE(unit, KA, SF, SOC_PORT_BLOCK_INDEX(unit, port),
                                        SF0_XCNFG_REMAP_MEM_ACC_CTRL);

        if (SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, ACK,
                               uRegData))
        {
            break;
        }
        sal_usleep(1000); /* sleep one millisecond */
    }

    /* Read complete, or we timed out */
    if (SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL,
                           ACK, uRegData) == 0 )
    {
        rv = BCM_E_TIMEOUT;
    }

    return rv;
}

/*****************************************************************************
 * FUNCTION NAME:  soc_qe2000_eb_mem_read
 *
 * OVERVIEW:       Read QE2000 EB Memory.
 *
 * ARGUMENTS:      userDeviceHandle - handle to access the device.
 *                 uOffset - offset into the memory.
 *                 puData - pointer to the data to write.
 *
 * RETURNS:        0 on success
 *
 *
 * DESCRIPTION:   Read a location in EB memory.  This function reads from the
 *                hardware even if EASY_RELOAD and we are reloading.  That is why
 *                the code uses the SAND_HAL_WRITE_EASY_RELOAD function instead
 *                of regular SAND_HAL_WRITES.  This is required because we need
 *                to read the MVT entry state information even if we are reloading.
 *
 *****************************************************************************/
#define HW_QE2000_IND_MEM_TIMEOUT            100

uint32 soc_qe2000_eb_mem_read( int unit,
			    uint32 uOffset,
			    uint32 puData[8])
{
    int nTimeout;
    int nAck;
    uint32 uCtlReg;

    /* assert( userDeviceHandle ); */
    assert( uOffset < (HW_QE2000_EB_MEM_MAX_OFFSET+1) );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    uCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE_EASY_RELOAD( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, uCtlReg );

    /*
     * Build up our command
     */
    uCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, uOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE_EASY_RELOAD( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, uCtlReg );

    nTimeout = HW_QE2000_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        uCtlReg = SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, uCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            puData[0] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE0 ));
            puData[1] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE1 ));
            puData[2] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE2 ));
            puData[3] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE3 ));
            puData[4] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE4 ));
            puData[5] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE5 ));
            puData[6] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE6 ));
            puData[7] = SAND_SWAP_32(SAND_HAL_READ( (sbhandle)unit, KA, EB_MEM_ACC_SLICE7 ));

            uCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, uCtlReg, 1 );
            uCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, uCtlReg, 0 );
            SAND_HAL_WRITE_EASY_RELOAD( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, uCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( -1 );
    }

    return( 0 );

}

/*****************************************************************************
 * FUNCTION NAME:  soc_qe2000_eb_mem_write
 *
 * OVERVIEW:       Write QE2000 EB Memory.
 *
 * ARGUMENTS:      (sbhandle)unit - handle to access the device.
 *                 uOffset - offset into the memory.
 *                 puData - pointer to the data to write.
 *
 * RETURNS:        0 on success
 *
 *
 * DESCRIPTION:   Write a location in EB memory.
 *
 *****************************************************************************/
int soc_qe2000_eb_mem_write( int unit,
			uint32 uOffset,
			uint32 uData[8])
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    /* assert( (sbhandle)unit ); */
    assert( uOffset < (HW_QE2000_EB_MEM_MAX_OFFSET + 1));

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    /* Address BUG 21826:  Use entire Write Mask */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, WRITE_MASK, 0xFF) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, uOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE0, SAND_SWAP_32(uData[0]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE1, SAND_SWAP_32(uData[1]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE2, SAND_SWAP_32(uData[2]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE3, SAND_SWAP_32(uData[3]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE4, SAND_SWAP_32(uData[4]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE5, SAND_SWAP_32(uData[5]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE6, SAND_SWAP_32(uData[6]) );
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_SLICE7, SAND_SWAP_32(uData[7]) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = HW_QE2000_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ_POLL( (sbhandle)unit, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( (sbhandle)unit, KA, EB_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( -1 );
    }

    return( 0 );

}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32 soc_qe2000_qs_mem_write(int unit,
				uint32 nTableId,
				uint32 uAddress,
				uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;
    uint32 uDataRead;
    int32 nRetryCnt = 0;


    for (nRetryCnt = 0; nRetryCnt < QE2000_QS_MEM_RETRY_CNT; nRetryCnt++) {

	uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTableId) |
	    SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, uAddress);
	
	SAND_HAL_WRITE((sbhandle)unit, KA, QS_MEM_ACC_DATA, uData);
	SOC_QE2000_WRITE_REQUEST(unit, QS_MEM, uLocalAddr);
	SOC_QE2000_WAIT_FOR_ACK(unit, QS_MEM, 1000, uStatus);
	
	/* Memory timeout waiting for ack */
	if (uStatus) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout writing QS memory table(%d) address(0x%x)\n"), nTableId, uAddress));
	    LOG_CLI((BSL_META_U(unit,
                                "Timeout writing QS memory table(%d) address(0x%x)\n"), nTableId, uAddress));
	    return uStatus;
	}
	
        if (qe2000_qs_mem[nTableId].bCheckMemory == FALSE ||
                                          SOC_IS_RELOADING(unit)) {
	    break;
	}

	/* GNATS 36214/33557 confirm writes to certain QS memories */
	uStatus = soc_qe2000_qs_mem_read(unit, nTableId, uAddress, &uDataRead);
	if (uStatus) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout on readback of QS memory table(%d) address(0x%x)\n"), nTableId, uAddress));
	    return uStatus;
	}
	
	if ( (uData     & qe2000_qs_mem[nTableId].nMemMask[0]) != 
	     (uDataRead & qe2000_qs_mem[nTableId].nMemMask[0])   ) {
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "QS Table(0x%x) Not Updated, Retrying Write, "
                                    "ExpectedData, uData: 0x%x\n"
                                    "CurrentData, uData: 0x%x\n"),
                         nTableId, (uData &  qe2000_qs_mem[nTableId].nMemMask[0]),
                         (uDataRead & qe2000_qs_mem[nTableId].nMemMask[0])));
	    uStatus = SOC_E_MEMORY;
	} else {
	    /* readback success */
	    uStatus = SOC_E_NONE;
	    break;
	}
    }
    return uStatus;
}


/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_mem_read(int unit,
			       uint32 nTableId,
			       uint32 uAddress,
			       uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTableId)|
		  SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_READ_REQUEST(unit, QS_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_MEM, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_MEM_ACC_DATA);
    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_mem_read_easy_reload(int unit,
                                            uint32 nTableId,
                                            uint32 uAddress,
                                            uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTableId)|
		  SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_READ_REQUEST_EASY_RELOAD(unit, QS_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK_EASY_RELOAD(unit, QS_MEM, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_MEM_ACC_DATA);
    return uStatus;
}


/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_rank_read(int unit,
				uint32 uAddress,
				uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_RANK_ACC_CTRL, ADDR, uAddress);

    SOC_QE2000_READ_REQUEST(unit, QS_RANK, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_RANK, 1000, uStatus);

    *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_RANK_ACC_DATA);

    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:   soc_qe2000_qs_rank_write()
 *
 * OVERVIEW:        Write the QS Rank memory.
 *
 * ARGUMENTS:       unit - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:    Write the QS rank memory.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
uint32 soc_qe2000_qs_rank_write(int unit,
				 uint32 uAddress,
				 uint32 uData)
{
  uint32 uStatus;
  uint32 uLocalAddr = 0;

  uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_RANK_ACC_CTRL, ADDR, uAddress);

  SAND_HAL_WRITE((sbhandle)unit, KA, QS_RANK_ACC_DATA, uData);
  SOC_QE2000_WRITE_REQUEST(unit, QS_RANK, uLocalAddr);
  SOC_QE2000_WAIT_FOR_ACK(unit, QS_RANK, 1000, uStatus);
  return uStatus;
}


/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_rand_write(int unit,
				 uint32 uAddress,
				 uint32 uData)
{
  uint32 uStatus = 0;
  uint32 uLocalAddr = 0;

  uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_RAND_ACC_CTRL, ADDR, uAddress);

  SAND_HAL_WRITE((sbhandle)unit, KA, QS_RAND_ACC_DATA, uData);
  SOC_QE2000_WRITE_REQUEST(unit, QS_RAND, uLocalAddr);
  SOC_QE2000_WAIT_FOR_ACK(unit, QS_RAND, 1000, uStatus);
  return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_rand_read(int unit,
				uint32 uAddress,
				uint32* pData)
{
  uint32 uStatus = 0;
  uint32 uLocalAddr = 0;

  uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_RAND_ACC_CTRL, ADDR, uAddress);

  SOC_QE2000_READ_REQUEST(unit, QS_RAND,uLocalAddr);
  SOC_QE2000_WAIT_FOR_ACK(unit, QS_RAND, 1000, uStatus);
  *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_RAND_ACC_DATA);
  return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_lna_rank_write(int unit,
				    uint32 uAddress,
				    uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_LNA_RANK_ACC_CTRL, ADDR, uAddress);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_RANK_ACC_DATA, uData);
    SOC_QE2000_WRITE_REQUEST(unit, QS_LNA_RANK, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_RANK, 1000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_lna_rank_read(int unit,
				   uint32 uAddress,
				   uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_LNA_RANK_ACC_CTRL, ADDR, uAddress);

    SOC_QE2000_READ_REQUEST(unit, QS_LNA_RANK, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_RANK, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_RANK_ACC_DATA);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_lna_rand_write(int unit,
				    uint32 uAddress,
				    uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ADDR, uAddress);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_RAND_ACC_DATA, uData);
    SOC_QE2000_WRITE_REQUEST(unit, QS_LNA_RAND, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_RAND, 1000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_lna_rand_read(int unit,
				   uint32 uAddress,
				   uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_READ_REQUEST(unit, QS_LNA_RAND, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_RAND, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_RAND_ACC_DATA);
    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32 soc_qe2000_qs_mem_lna_write(int unit,
				   uint32 nTableId,
				   uint32 uAddress,
				   uint32 uData4,
				   uint32 uData3,
				   uint32 uData2,
				   uint32 uData1,
				   uint32 uData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;
    int32 nRetryCnt;
    uint32 uData0Read, uData1Read, uData2Read, uData3Read, uData4Read;

    for (nRetryCnt = 0; nRetryCnt < QE2000_QS_MEM_RETRY_CNT; nRetryCnt++) {

    uLocalAddr = SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTableId) |
	         SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, uAddress);

    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA3, uData3);
    SAND_HAL_WRITE((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA4, uData4);
    SOC_QE2000_WRITE_REQUEST(unit, QS_LNA_MEM,
				 uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_MEM, 1000, uStatus);


	/* Memory timeout waiting for ack */
	if (uStatus) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout writing QS memory table(%d) address(0x%x)\n"), nTableId, uAddress));
	    return uStatus;
	}

        if (qe2000_qs_lna_mem[nTableId].bCheckMemory == FALSE) {
	    break;
	}

	/* GNATS 36214/33557 confirm writes to certain QS memories */
	uStatus = soc_qe2000_qs_mem_lna_read(unit, nTableId, uAddress, &uData0Read, &uData1Read, &uData2Read, &uData3Read, &uData4Read);
	if (uStatus) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout on readback of QS memory table(%d) address(0x%x)\n"), nTableId, uAddress));
	    return uStatus;
	}
	
        if ( ((uData0Read & qe2000_qs_lna_mem[nTableId].nMemMask[0]) !=
	      (uData0 & qe2000_qs_lna_mem[nTableId].nMemMask[0])) ||
             ((uData1Read & qe2000_qs_lna_mem[nTableId].nMemMask[1]) !=
	      (uData1 & qe2000_qs_lna_mem[nTableId].nMemMask[1])) ||
             ((uData2Read & qe2000_qs_lna_mem[nTableId].nMemMask[2]) !=
	      (uData2 & qe2000_qs_lna_mem[nTableId].nMemMask[2])) ||
             ((uData3Read & qe2000_qs_lna_mem[nTableId].nMemMask[3]) !=
	      (uData3 & qe2000_qs_lna_mem[nTableId].nMemMask[3])) ||
             ((uData4Read & qe2000_qs_lna_mem[nTableId].nMemMask[4]) !=
	      (uData4 & qe2000_qs_lna_mem[nTableId].nMemMask[4])) ) {

            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "QS_LNA Table(0x%x) Not Updated, Retrying Write, "
                                 "ExpectedData, uData0: 0x%x uData1: 0x%x uData2: 0x%x uData3: 0x%x uData4: 0x%x\n"
                                 "CurrentData, uData0: 0x%x uData1: 0x%x uData2: 0x%x uData3: 0x%x uData4: 0x%x\n"),
                      nTableId, (uData0 & qe2000_qs_lna_mem[nTableId].nMemMask[0]),
                      (uData1 & qe2000_qs_lna_mem[nTableId].nMemMask[1]),
                      (uData2 & qe2000_qs_lna_mem[nTableId].nMemMask[2]),
                      (uData3 & qe2000_qs_lna_mem[nTableId].nMemMask[3]),
                      (uData4 & qe2000_qs_lna_mem[nTableId].nMemMask[4]),
                      
                      (uData0Read & qe2000_qs_lna_mem[nTableId].nMemMask[0]),
                      (uData1Read & qe2000_qs_lna_mem[nTableId].nMemMask[1]),
                      (uData2Read & qe2000_qs_lna_mem[nTableId].nMemMask[2]),
                      (uData3Read & qe2000_qs_lna_mem[nTableId].nMemMask[3]),
                      (uData4Read & qe2000_qs_lna_mem[nTableId].nMemMask[4])));
	    
			    
	} else {
	    /* readback success */
	    break;
	}
    }
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qs_mem_lna_read(int unit,
				  uint32 nTableId,
				  uint32 uAddress,
				  uint32* pData4,
				  uint32* pData3,
				  uint32* pData2,
				  uint32* pData1,
				  uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTableId)|
		  SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_READ_REQUEST(unit, QS_LNA_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, QS_LNA_MEM, 1000, uStatus);
    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA2);
    *pData3 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA3);
    *pData4 = SAND_HAL_READ((sbhandle)unit, KA, QS_LNA_MEM_ACC_DATA4);
    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32 soc_qe2000_qm_fb_cache_fifo_write(int unit,
					uint32 uData)
{
    uint32 uStatus = 0;

    SAND_HAL_WRITE((sbhandle)unit, KA, QM_FB_CACHE_ACC_DATA, uData);
    SOC_QE2000_WRITE_REQUEST(unit, QM_FB_CACHE, 0);
    SOC_QE2000_WAIT_FOR_ACK(unit, QM_FB_CACHE, 1000, uStatus);
    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_qm_fb_cache_fifo_read(int unit,
				       uint32* pData)
{
    uint32 uStatus = 0;

    SOC_QE2000_READ_REQUEST(unit, QM_FB_CACHE, 0);
    SOC_QE2000_WAIT_FOR_ACK(unit, QM_FB_CACHE, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, QM_FB_CACHE_ACC_DATA);
    return uStatus;
}

uint32
soc_qe2000_qm_mem_write(int unit,
			int32 nAddr,
			int32 nTable,
			uint32 uData0,
			uint32 uData1,
			uint32 uData2,
			uint32 uData3)
{
    uint32 uTimeOut;
    uint32 uCtrl;
    int status = SOC_E_NONE;

#if 0
    LOG_CLI((BSL_META_U(unit,
                        "Writing QM Table(0x%x), uData0: 0x%x, uData1: 0x%x, uData2: 0x%x, uData3: 0x%x\n"),
             nTable, uData0, uData1, uData2, uData3));
#endif

    SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA0, uData0);
    if (nTable != 0x7) {
	SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA1, uData1);
	SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA2, uData2);
	SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA3, uData3);
    }
    SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_CTRL,
                   SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, 0x1));


    uCtrl = 0;
    uCtrl |= SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, 0x1) |
	SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, REQ,  1) |
	SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, 0) |
	SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, nTable) |
	SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, nAddr);

    SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_CTRL, uCtrl);

    for (uTimeOut = 100; uTimeOut > 0; uTimeOut--) {
	uCtrl = SAND_HAL_READ_POLL(unit, KA, QM_MEM_ACC_CTRL);
	if (SAND_HAL_GET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uCtrl)) {
	    break;
	}
    }

    status = (uTimeOut > 0) ? SOC_E_NONE : SOC_E_TIMEOUT;

    if (status == SOC_E_TIMEOUT) {
	LOG_CLI((BSL_META_U(unit,
                            "Timeout waiting for QM indirect access ACK (table(%d) addr(0x%x/%d).\n"),
                 nTable, nAddr, nAddr));
    }

    return(status);
}

uint32
soc_qe2000_qm_mem_read(int unit,
		       int32 nAddr,
		       int32 nTable,
		       uint32 *puData0,
		       uint32 *puData1,
		       uint32 *puData2,
		       uint32 *puData3)
{
    uint32 uTimeOut;
    uint32 uCtrl;

    uCtrl = 0;
    uCtrl = SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, 0x1) |
            SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, REQ, 1) |
            SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, RD_WR_N, 1) |
            SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, nTable) |
            SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, nAddr);


    /* In easy_reload, we need to read the BW Port config table for the bag information */
    /* if we are reloading.                                                             */
#ifdef BCM_EASY_RELOAD_SUPPORT
    if (nTable == 0x0F) {
	SAND_HAL_WRITE_EASY_RELOAD(unit, KA, QM_MEM_ACC_CTRL, uCtrl);
    } else
#endif
    {
        SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_CTRL, uCtrl);
    }
    uTimeOut = 100;
    while (uTimeOut--) {
    /* In easy_reload, we need to read the BW Port config table for the bag information */
    /* if we are reloading.                                                             */
#ifdef BCM_EASY_RELOAD_SUPPORT
	if (nTable == 0x0F) {
	    uCtrl = SAND_HAL_READ(unit, KA, QM_MEM_ACC_CTRL);
	} else
#endif
	{
	    uCtrl = SAND_HAL_READ_POLL(unit, KA, QM_MEM_ACC_CTRL);
	}
        if (SAND_HAL_GET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, uCtrl)) {
            *puData0 = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA0);
            if (nTable != 0x7) {
                *puData1 = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA1);
                *puData2 = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA2);
                *puData3 = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA3);
            }
            SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ACK, 0x1));

            return 0;
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "ERROR: Timeout waiting for QM indirect access ACK (table(%d) addr(0x%x/%d).\n"),
             nTable, nAddr, nAddr));


    return SOC_E_MEMORY;
}

uint32 soc_qe2000_qstate_mem_read(int unit,
                                    int32 nQueue,
                                    int32 *pbEnable,
				    int32 *pnAnemicWatermarkSel,
                                    int32 *pnMinBuffers,
				    int32 *pnMaxBuffers)
{
    int32 status = SOC_E_NONE;
    uint32 uData0, uData1, uData2, uData3;

    status = soc_qe2000_qm_mem_read(unit, nQueue, 0x00, &uData0, &uData1, &uData2, &uData3);
    if (status) {
        return status;
    }

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    *pbEnable = (uData0 & 0x00000001);
    *pnAnemicWatermarkSel = ((uData0 & 0x0000001C) >> 2);
    *pnMaxBuffers = (((uData1 & 0x00001FFF) << 1) + ((uData0 & 0x80000000) >> 31));
    *pnMinBuffers = ((uData1 & 0x07FFE000) >> 13);
    return status;
}

uint32 soc_qe2000_qwred_config_set(int unit,
				     int32 queue,
				     int32 template,
				     int32 gain)
{

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    uint32 uData0 = ( ((template << 4) & 0x000000F0) | (gain & 0x0000000F) );
    return ( soc_qe2000_qm_mem_write(unit, queue, 0x0F, uData0, 0x00, 0x00, 0x00) );
}

uint32 soc_qe2000_qdemand_config_read(int unit,
					int32 queue,
					int32 *p_rate_delta_max,
					int32 *p_qla_demand_mask)
{
    int32 status = SOC_E_NONE;
    uint32 uData0, uData1, uData2, uData3;

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    status = soc_qe2000_qm_mem_read(unit, queue, 0x08, &uData0, &uData1, &uData2, &uData3);
    if (status) {
        return status;
    }

    *p_rate_delta_max = ((uData0 & 0x0000007E) >> 1);
    *p_qla_demand_mask = (uData0 & 0x00000001);
    return status;
}

uint32 soc_qe2000_qdemand_config_set(int unit,
				       int32 queue,
				       int32 rate_delta_max_index,
				       int32 qla_demand_mask)
{

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    uint32 uData0 = ( ((rate_delta_max_index << 1) & 0x0000007E) | (qla_demand_mask & 0x00000001) );
    return ( soc_qe2000_qm_mem_write(unit, queue, 0x08, uData0, 0x00, 0x00, 0x00) );
}

uint32 soc_qe2000_qbyte_adjust_set(int unit,
				     int32 queue,
				     int32 sign,
				     int32 bytes)
{

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    uint32 uData0 = ( ((sign << 6) & 0x00000040) | (bytes & 0x0000003F) );
    return ( soc_qe2000_qm_mem_write(unit, queue, 0x06, uData0, 0x00, 0x00, 0x00) );
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

int32
soc_qe2000_eg_mem_write(int unit, int32 nAddr, int32 nTable,
			uint32 uData0, uint32 uData1, uint32 uData2) {
    uint32 ulTimeOut;
    uint32 ulCntrl;

    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_DATA2, uData2);

    ulCntrl = SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, 0x1) |
        SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, REQ, 0x1) |
        SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, 0x0) |
        SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, nTable) |
        SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, nAddr);

    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL, ulCntrl);

#if 0
    LOG_CLI((BSL_META_U(unit,
                        "*>soc_qe2000_eg_mem_write. Table 0x%x Addr 0x%x Data 0x%x 0x%x 0x%x\n"),
             nTable, nAddr, uData0, uData1, uData2));
#endif

    ulTimeOut = 800;
    while(ulTimeOut--) {
        ulCntrl = SAND_HAL_READ_POLL((sbhandle)unit, KA, EG_MEM_ACC_CTRL);
        if (SAND_HAL_GET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, ulCntrl)) {
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, 0x1));

            return 0;
        }
    }

    return -1;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_eg_mem_read(int unit,
			       uint32 uAddress,
			       uint32 uTableId,
			       uint32* pData0,
			       uint32* pData1,
			       uint32* pData2)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    LOG_VERBOSE(BSL_LS_SOC_SOCMEM,
                (BSL_META_U(unit,
                            "soc_qe2000_eg_mem_read. Table 0x%x Addr 0x%x\n"), uTableId, uAddress));

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uTableId)|
		  SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uAddress));

   /* for EASY_RELOAD, read the memory always, regardless of whether reloading or not */
    SOC_QE2000_READ_REQUEST_EASY_RELOAD(unit, EG_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK_EASY_RELOAD(unit, EG_MEM, 1000, uStatus);

    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EG_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, EG_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ((sbhandle)unit, KA, EG_MEM_ACC_DATA2);
    LOG_VERBOSE(BSL_LS_SOC_SOCMEM,
                (BSL_META_U(unit,
                            "soc_qe2000_eg_mem_read. LocAddr 0x%x Data 0x%x 0x%x 0x%x\n"),
                 uLocalAddr, *pData0, *pData1, *pData2));
    return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ei_mem_write(int unit,
				uint32 uTableId,
				uint32 uAddress,
				uint32 uData0){
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    SAND_HAL_WRITE((sbhandle)unit, KA, EI_MEM_ACC_DATA, uData0);

    uLocalAddr =  SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, MEM_SEL, uTableId) |
	          SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, ADDR, uAddress);
    SOC_QE2000_WRITE_REQUEST(unit, EI_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, EI_MEM, 1000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ei_mem_read(int unit,
			       uint32 uTableId,
			       uint32 uAddress,
			       uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, MEM_SEL, uTableId)|
		  SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, ADDR, uAddress));
    SOC_QE2000_READ_REQUEST(unit, EI_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, EI_MEM, 1000, uStatus);
    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EI_MEM_ACC_DATA);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_pm_mem_write(int unit,
				uint32 uAddress,
				uint32 uData3,
				uint32 uData2,
				uint32 uData1,
				uint32 uData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    SAND_HAL_WRITE((sbhandle)unit, KA, PM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, PM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE((sbhandle)unit, KA, PM_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE((sbhandle)unit, KA, PM_MEM_ACC_DATA3, uData3);

    uLocalAddr =  (SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, uAddress) |
		   SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, HASH, ((uAddress>>25)&0x1)));

    SOC_QE2000_WRITE_REQUEST(unit, PM_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, PM_MEM, 5000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_pm_mem_read(int unit,
			       uint32 uAddress,
			       uint32* pData3,
			       uint32* pData2,
			       uint32* pData1,
			       uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = 	(SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, uAddress)|
			 SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, HASH, ((uAddress>>25)&0x1)));

    SOC_QE2000_READ_REQUEST(unit, PM_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, PM_MEM, 1000, uStatus);

    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, PM_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, PM_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ((sbhandle)unit, KA, PM_MEM_ACC_DATA2);
    *pData3 = SAND_HAL_READ((sbhandle)unit, KA, PM_MEM_ACC_DATA3);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_sf_mem_write(int unit,
				uint32 uAddress,
				uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr =  SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, uAddress);

    SAND_HAL_WRITE((sbhandle)unit, KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, uData);
    SOC_QE2000_WRITE_REQUEST(unit, SF0_XCNFG_REMAP_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, SF0_XCNFG_REMAP_MEM, 1000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_sf_mem_read(int unit,
			       uint32 uAddress,
			       uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr =  SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, uAddress);

    SOC_QE2000_READ_REQUEST(unit, SF0_XCNFG_REMAP_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, SF0_XCNFG_REMAP_MEM, 1000, uStatus);
    *pData = SAND_HAL_READ((sbhandle)unit, KA, SF0_XCNFG_REMAP_MEM_ACC_DATA);
  return uStatus;
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_amcl_mem_write(int unit,
				    uint32 uAddress,
				    uint32 uData1,
				    uint32 uData0,
				    sbBool_t bCauseParityError)
{
    uint32 uStatus = 0;
#if 0
    uint32 uLocalAddr = 0;

    uLocalAddr =  SAND_HAL_SET_FIELD(KA,
				     EP_AM_CL_MEM_ACC_CTRL,
				     ADDR, uAddress);
#endif

    SAND_HAL_WRITE((sbhandle)unit, KA, EP_AM_CL_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, EP_AM_CL_MEM_ACC_DATA1, uData1);
    
#if 0
    SOC_QE2000_WRITE_REQUEST_WITH_PARITY(unit, EP_AM_CL_MEM,
					     uLocalAddr,
					     bCauseParityError);
#endif
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_AM_CL_MEM, 1000, uStatus);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_amcl_mem_read(int unit,
				   uint32 uAddress,
				   uint32 bClearOnRead,
				   uint32* pData1,
				   uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    if ( bClearOnRead ) {
	uLocalAddr =  SAND_HAL_SET_FIELD(KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST_CLR_ON_RD(unit, EP_AM_CL_MEM, uLocalAddr);

    } else {
	uLocalAddr =  SAND_HAL_SET_FIELD(KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST(unit, EP_AM_CL_MEM, uLocalAddr);
    }
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_AM_CL_MEM, 1000, uStatus);
    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EP_AM_CL_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, EP_AM_CL_MEM_ACC_DATA1);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_bmbf_mem_write(int unit,
				    uint32 uAddress,
				    uint32 uData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr =  SAND_HAL_SET_FIELD(KA,
				     EP_BM_BF_MEM_ACC_CTRL,
				     ADDR, uAddress);

    SAND_HAL_WRITE((sbhandle)unit, KA, EP_BM_BF_MEM_ACC_DATA, uData0);
    SOC_QE2000_WRITE_REQUEST(unit, EP_BM_BF_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_BM_BF_MEM, 1000, uStatus);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_bmbf_mem_read(int unit,
				   uint32 uAddress,
				   uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA,
				    EP_BM_BF_MEM_ACC_CTRL,
				    ADDR,
				    uAddress);

    SOC_QE2000_READ_REQUEST(unit, EP_BM_BF_MEM, uLocalAddr);

    SOC_QE2000_WAIT_FOR_ACK(unit, EP_BM_BF_MEM, 1000, uStatus);
    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EP_BM_BF_MEM_ACC_DATA);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_mmip_mem_write(int unit,
				    uint32 uAddress,
				    uint32 uData1,
				    uint32 uData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = SAND_HAL_SET_FIELD(KA,
				    EP_MM_IP_MEM_ACC_CTRL,
				    ADDR, uAddress);

    SAND_HAL_WRITE((sbhandle)unit, KA, EP_MM_IP_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, EP_MM_IP_MEM_ACC_DATA1, uData1);

    SOC_QE2000_WRITE_REQUEST(unit, EP_MM_IP_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_MM_IP_MEM, 1000, uStatus);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_mmip_mem_read(int unit,
				   uint32 uAddress,
				   uint32 bClearOnRead,
				   uint32* pData1,
				   uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    if ( bClearOnRead ) {
	uLocalAddr =  SAND_HAL_SET_FIELD(KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST_CLR_ON_RD(unit, EP_MM_IP_MEM, uLocalAddr);

    } else {
	uLocalAddr =  SAND_HAL_SET_FIELD(KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST(unit, EP_MM_IP_MEM, uLocalAddr);
    }
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_MM_IP_MEM, 1000, uStatus);

    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EP_MM_IP_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, EP_MM_IP_MEM_ACC_DATA1);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_mmbf_mem_write(int unit,
				    uint32 uAddress,
				    uint32 uData1,
				    uint32 uData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr =  SAND_HAL_SET_FIELD(KA,
				     EP_MM_BF_MEM_ACC_CTRL,
				     ADDR, uAddress);

    SAND_HAL_WRITE((sbhandle)unit, KA, EP_MM_BF_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE((sbhandle)unit, KA, EP_MM_BF_MEM_ACC_DATA1, uData1);

    SOC_QE2000_WRITE_REQUEST(unit, EP_MM_BF_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_MM_BF_MEM, 1000, uStatus);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_ep_mmbf_mem_read(int unit,
				   uint32 uAddress,
				   sbBool_t bClearOnRead,
				   uint32* pData1,
				   uint32* pData0)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    if ( bClearOnRead ) {
	uLocalAddr = SAND_HAL_SET_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST_CLR_ON_RD(unit, EP_MM_BF_MEM, uLocalAddr);
    } else {
	uLocalAddr = SAND_HAL_SET_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, uAddress);
	SOC_QE2000_READ_REQUEST(unit, EP_MM_BF_MEM, uLocalAddr);
    }
    SOC_QE2000_WAIT_FOR_ACK(unit, EP_MM_BF_MEM, 1000, uStatus);
    *pData0 = SAND_HAL_READ((sbhandle)unit, KA, EP_MM_BF_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ((sbhandle)unit, KA, EP_MM_BF_MEM_ACC_DATA1);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_rb_pol_mem_write(int unit, uint32 uTableId, uint32 uAddress, uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = ( SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, SELECT, uTableId) |
		   SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, ADDRESS, uAddress));

    SAND_HAL_WRITE((sbhandle)unit, KA, RB_POL_MEM_ACC_DATA, uData);
    SOC_QE2000_WRITE_REQUEST(unit, RB_POL_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, RB_POL_MEM, 1000, uStatus);
    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_rb_pol_mem_read(int unit,
				  uint32 uTableId,
				  uint32 uAddress,
				  uint32* pData)
{

    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, SELECT, uTableId)|
		  SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, ADDRESS, uAddress));

    SOC_QE2000_READ_REQUEST(unit, RB_POL_MEM, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, RB_POL_MEM, 1000, uStatus);

    *pData = SAND_HAL_READ((sbhandle)unit, KA, RB_POL_MEM_ACC_DATA);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_rb_class_mem_write(int unit,
				     uint32 nSpi,
				     uint32 uTableId,
				     uint32 uAddress,
				     uint32 uData0,
				     uint32 uData1)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    if ( nSpi == 0 ) {
	uLocalAddr =  (SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, SELECT, uTableId) |
		       SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, ADDRESS, uAddress));

	SAND_HAL_WRITE((sbhandle)unit, KA, RB_CLASS0_MEM_ACC_DATA0, uData0);
	SAND_HAL_WRITE((sbhandle)unit, KA, RB_CLASS0_MEM_ACC_DATA1, uData1);

	SOC_QE2000_WRITE_REQUEST(unit, RB_CLASS0_MEM, uLocalAddr);
	SOC_QE2000_WAIT_FOR_ACK(unit, RB_CLASS0_MEM, 1000, uStatus);
	return uStatus;
    } else if ( nSpi == 1 ) {

	uLocalAddr = (SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, SELECT, uTableId) |
		      SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, ADDRESS, uAddress));

	SAND_HAL_WRITE((sbhandle)unit, KA, RB_CLASS1_MEM_ACC_DATA0, uData0);
	SAND_HAL_WRITE((sbhandle)unit, KA, RB_CLASS1_MEM_ACC_DATA1, uData1);

	SOC_QE2000_WRITE_REQUEST(unit, RB_CLASS1_MEM, uLocalAddr);
	SOC_QE2000_WAIT_FOR_ACK(unit, RB_CLASS1_MEM, 1000, uStatus);
	return uStatus;
    } else {
	return uStatus;
  }
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32 soc_qe2000_rb_class_mem_read(int unit,
				    uint32 nSpi,
				    uint32 uTableId,
				    uint32 uAddress,
				    uint32* pData0,
				    uint32* pData1)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    if ( nSpi == 0 ) {

	uLocalAddr =  (SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, SELECT, uTableId)|
		       SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, ADDRESS, uAddress));

	SOC_QE2000_READ_REQUEST(unit, RB_CLASS0_MEM, uLocalAddr);
	SOC_QE2000_WAIT_FOR_ACK(unit, RB_CLASS0_MEM, 1000, uStatus);

	*pData0 = SAND_HAL_READ((sbhandle)unit, KA, RB_CLASS0_MEM_ACC_DATA0);
	*pData1 = SAND_HAL_READ((sbhandle)unit, KA, RB_CLASS0_MEM_ACC_DATA1);
	return uStatus;

    } else if ( nSpi == 1 ) {

	uLocalAddr = (SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, SELECT, uTableId)|
		      SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, ADDRESS, uAddress));

	SOC_QE2000_READ_REQUEST(unit, RB_CLASS1_MEM, uLocalAddr);
	SOC_QE2000_WAIT_FOR_ACK(unit, RB_CLASS1_MEM, 1000, uStatus);

	*pData0 = SAND_HAL_READ((sbhandle)unit, KA, RB_CLASS1_MEM_ACC_DATA0);
	*pData1 = SAND_HAL_READ((sbhandle)unit, KA, RB_CLASS1_MEM_ACC_DATA1);

	return uStatus;

  } else {

      return uStatus;
  }
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_pm_dll_lut_write(int unit,
				   uint32 uTableId,
				   uint32 uAddress,
				   uint32 uData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    SAND_HAL_WRITE((sbhandle)unit, KA, PM_DLL_ACC_DATA, uData);

    uLocalAddr = (SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, uTableId) |
		  SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_WRITE_REQUEST(unit, PM_DLL,uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, PM_DLL, 1000, uStatus);

    return uStatus;
}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:
 *
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

uint32 soc_qe2000_pm_dll_lut_read(int unit,
				  uint32 uTableId,
				  uint32 uAddress,
				  uint32* pData)
{
    uint32 uStatus = 0;
    uint32 uLocalAddr = 0;

    uLocalAddr =  (SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, uTableId) |
		   SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, uAddress));

    SOC_QE2000_READ_REQUEST(unit, PM_DLL, uLocalAddr);
    SOC_QE2000_WAIT_FOR_ACK(unit, PM_DLL, 1000, uStatus);

    *pData = SAND_HAL_READ((sbhandle)unit, KA, PM_DLL_ACC_DATA);

    return uStatus;
}


int
soc_qe2000_features(int unit, soc_feature_t feature)
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
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) ||
		     (SOC_SBX_CFG(unit)->bTmeMode ==  SOC_SBX_QE_MODE_HYBRID)) ?
		    TRUE : FALSE );
            break;

        case soc_feature_egress_metering:
            /* Support is available for egress shaping */
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "rate egress_metering feature available\n")));
            return( TRUE );
            break;

        case soc_feature_cosq_gport_stat_ability:
            /* Support is available for egress shaping */
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "rate egress_metering feature available\n")));
            return( TRUE );
            break;

        case soc_feature_mc_group_ability:
            return(TRUE);

        case soc_feature_standalone:
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) ?
		     TRUE : FALSE) );
            break;

        case soc_feature_packet_adj_len:
            return( TRUE );
            break;

        case soc_feature_hybrid:
            return( ( (SOC_SBX_CFG(unit)->bHybridMode == TRUE) &&
                      (SOC_SBX_CFG(unit)->bTmeMode != TRUE /* SOC_SBX_QE_MODE_TME */) ) ? TRUE : FALSE);
            break;

        case soc_feature_node:
            return(TRUE);
            break;

        case soc_feature_node_hybrid:
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) ? TRUE : FALSE) );
            break;

        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
                case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
		    return((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
                    break;

                case SOC_SBX_SYSTEM_CFG_DMODE:
                default:
                    return(FALSE);
                    break;
            }
            break;

        case soc_feature_egr_multicast_independent_fc:
            return((SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
            break;

        default:
            return(FALSE);
    }
}

int
soc_qe2000_port_info_config(int unit)
{

    int                 i, port, spi;
    soc_sbx_control_t   *sbx;
    soc_info_t          *si;
    int                 total_ports, total_spi_subports;
    soc_port_info_t     *port_info;
    soc_driver_t        *driver;

    si     = &SOC_INFO(unit);
    sbx    = SOC_SBX_CONTROL(unit);
    driver = SOC_DRIVER(unit);

    /* Load port mapping into 'port_info' and 'block_info' device driver */
    /* free old array if its not the default */
    if (driver->port_info &&
        (driver->port_info != soc_port_info_bcm83200_a3))
    {
        sal_free(driver->port_info);
        driver->port_info = NULL;
    }

    /* set defaults first */
    driver->port_info = soc_port_info_bcm83200_a3;

    sbx->spi_subport_count[0] = soc_property_get(unit, spn_QE_SPI_0_SUBPORTS, 14);
    sbx->spi_subport_count[1] = soc_property_get(unit, spn_QE_SPI_1_SUBPORTS, 14);

    /* only support upto 49 spi subport, spi subport 49 is reserved as CPU port */
    total_spi_subports = sbx->spi_subport_count[0] + sbx->spi_subport_count[1];
    assert( total_spi_subports <= 49 );

    total_ports = (49                                       +
                   1                                        +  /* CPU port */
		   SB_FAB_DEVICE_QE2000_SFI_LINKS           +
                   SB_FAB_DEVICE_QE2000_SCI_LINKS           +
                   SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES  +
                   1);                                 /* end of list entry */

    port_info = sal_alloc((sizeof(soc_port_info_t) * total_ports),
                          "qe_port_info");

    if (port_info == NULL) {
        return SOC_E_MEMORY;
    }

    /* init port info to the 'end of list' entry */
    for (port=0; port<total_ports; port++) {
        port_info[port].blk     = -1;
        port_info[port].bindex  = -1;
    }

    driver->port_info = port_info;

    port = 0;
    /* SPI Sub-Ports */
    for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
        port_info[port].blk  = soc_sbx_block_find(unit, SOC_BLK_SPI, spi);
        for (i=0; i<sbx->spi_subport_count[spi]; i++, port++) {
            port_info[port].bindex = i;
        }
    }

    /* CPU Port */
    port = 49;
    port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_CMIC, 0);
    port_info[port].bindex = -1;
    port++;

    /* SFI & SCI Ports */
    for (i=0; i<SB_FAB_DEVICE_QE2000_SFI_LINKS; i++, port++) {
        port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_GXPORT, 0);
        port_info[port].bindex = i;
    }
    for (i=0; i<SB_FAB_DEVICE_QE2000_SCI_LINKS; i++, port++) {
        port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_EXP, 0);
        port_info[port].bindex = i;
    }

    /* SPI Bus Ports */
    for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++, port++) {
        /* bindex = -1 signifies this is a SPI bus and not a sub-port */
        port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_SPI, spi);
        port_info[port].bindex = spi;
    }


    
    /* spi subports */
    si->spi_subport.min = 0;
    si->spi_subport.max = (si->spi_subport.min       +
                           sbx->spi_subport_count[0] +
                           sbx->spi_subport_count[1] - 1);

    /* If this assert occurs,change to the definition in bcm_int/sbx/cosq.h is required */
    assert(si->spi_subport.min == BCM_INT_SBX_QE2000_SOC_SPI_PORT_MIN);

    for (port = si->spi_subport.min; port <= si->spi_subport.max; port++) {
        SBX_ADD_PORT(spi_subport,port);
        SBX_ADD_PORT(port,port);
        SBX_ADD_PORT(all,port);
        si->port_type[port] = SOC_BLK_SPI;
        sal_snprintf(SOC_PORT_NAME(unit,port),
                     sizeof(si->port_name[port]),
                     "spi_s%d", (port - si->spi_subport.min));
    }

    /* spi subport 49 is always created as cpu port */
    port = 49;
    /* cpu port */
    si->cmic_port = port;
    si->cmic_block = SOC_BLK_CMIC;
    si->port_type[port] = SOC_BLK_CMIC;
    SOC_PBMP_PORT_ADD(si->cmic_bitmap, (si->cmic_port));
    SBX_ADD_PORT(all, (si->cmic_port));
    port++;

    /* If this assert occurs,change to the definition in bcm_int/sbx/cosq.h is required */
    assert(si->cmic_port == BCM_INT_SBX_QE2000_SOC_CPU_PORT_MIN);

    si->sfi.min = port;
    si->sfi.max = si->sfi.min + 17;
    for (port = si->sfi.min; port <= si->sfi.max; port++) {
        SBX_ADD_PORT(sfi,port);
        SBX_ADD_PORT(port,port);
        SBX_ADD_PORT(all,port);
        si->port_type[port] = SOC_BLK_GXPORT;
        sal_snprintf(SOC_PORT_NAME(unit,port),
                     sizeof(si->port_name[port]),
                     "sfi%d", (port - si->sfi.min));
    }

    si->sci.min = si->sfi.max + 1;
    si->sci.max = si->sci.min + 1;
    for (port = si->sci.min; port <= si->sci.max; port++) {
        SBX_ADD_PORT(sci,port);
        SBX_ADD_PORT(port,port);
        SBX_ADD_PORT(all,port);
        si->port_type[port] = SOC_BLK_GXPORT;
        sal_snprintf(SOC_PORT_NAME(unit,port),
                     sizeof(si->port_name[port]),
                     "sci%d", (port - si->sci.min));
    }

    /* 2 spi ports */
    si->spi.min = si->sci.max + 1;
    si->spi.max = si->spi.min + 1;
    for (port = si->spi.min; port <= si->spi.max; port++) {
        SBX_ADD_PORT(spi, port);
        SBX_ADD_PORT(all,port);
        si->port_type[port] = SOC_BLK_SPI;
        sal_snprintf(SOC_PORT_NAME(unit,port),
                     sizeof(si->port_name[port]),
                     "spi%d", (port - si->spi.min));
    }

    return SOC_E_NONE;

}



uint32
soc_qe2000_qm_counter_base_set(int unit,
		       int32 nBaseQueue,
               int32 enable)
{
    if (nBaseQueue > SOC_SBX_CFG(unit)->num_queues - 16) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid Base Queue Requested %d [0x%x]\n"), nBaseQueue, nBaseQueue));
        return SOC_E_PARAM;
    }



    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Set Counter Base to %d [0x%x]\n"), nBaseQueue, nBaseQueue));
    SAND_HAL_WRITE(unit, KA, QM_SELECTED_Q, nBaseQueue);

    return SOC_E_NONE;

}


uint32
soc_qe2000_qm_counter_base_get(int unit,
		       int32 *nBaseQueue)
{

    *nBaseQueue = SAND_HAL_READ(unit, KA, QM_SELECTED_Q);
     LOG_VERBOSE(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Get Counter Base to %d [0x%x]\n"), *nBaseQueue, *nBaseQueue));
    return SOC_E_NONE;

}

uint32
soc_qe2000_qm_counter_read(int unit,
		       int32 set,
		       uint32 *puCounterBase)
{
    uint32 uCtrl;
    uint32 uPacketCount;
    uint32 uByteCount;
    int nQ;
    int nCounter;

    /* We only get the requested queue at a time */

    nQ = set;
    for ( nCounter=0; nCounter < (QE2000_COUNTER_QM_COUNT/2); nCounter++) {

        uCtrl = SAND_HAL_SET_FIELD(KA, QM_SLQ_PTR, SLQ_PTR, nQ*12+nCounter);
        uCtrl |= SAND_HAL_SET_FIELD(KA, QM_SLQ_PTR, GO, 1);

        SAND_HAL_WRITE(unit, KA, QM_SLQ_PTR, uCtrl);

        uCtrl = SAND_HAL_SET_FIELD(KA, QM_SLQ_PTR, SLQ_PTR, nQ*12+nCounter);
        uCtrl |= SAND_HAL_SET_FIELD(KA, QM_SLQ_PTR, GO, 0);

        SAND_HAL_WRITE(unit, KA, QM_SLQ_PTR, uCtrl);

        uPacketCount = SAND_HAL_READ(unit, KA, QM_SLQ_PKT_CNT);
        uByteCount = SAND_HAL_READ(unit, KA, QM_SLQ_BYTE_CNT);

#define REAL_COUNTER
#ifdef REAL_COUNTER
        *puCounterBase++ = uPacketCount;
        *puCounterBase++ = uByteCount;
        if (uPacketCount || uByteCount ) {
            LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit,
                                    "Q %d C %d p %d b %d 0x%x\n"), nQ, nCounter, uPacketCount, uByteCount, uCtrl));
        }
#else
        /* Simulate Counter action */
        *puCounterBase++ = 1 + (nCounter*2);
        *puCounterBase++ = 2 + (nCounter*2);
#endif
    }


    return SOC_E_NONE;
}

uint32
soc_qe2000_rate_delta_config(int unit, uint32 timeslot_size, uint32 epoch_size)
{
    int rc = SOC_E_NONE;
    int cur_index, percent_off, template_index;
    uint64 ns_in_epoch, rate_delta_port_speed;
    uint32 rate_delta = 0;

    COMPILER_64_SET(ns_in_epoch, 0, timeslot_size);
    COMPILER_64_UMUL_32(ns_in_epoch, epoch_size);

    for (cur_index = 0; cur_index < QE2000_RATE_DELTA_MAX_PORT_SPEEDS; cur_index++) {
        for (percent_off = 0; percent_off < QE2000_RATE_DELTA_MAX_PERCENTAGE; percent_off++) {
            rate_delta_port_speed =  ns_in_epoch;
            COMPILER_64_UMUL_32(rate_delta_port_speed, soc_sbx_qe2000_rate_delta_port_speed[cur_index]);

	    
	    
	    if (soc_sbx_div64(rate_delta_port_speed, (1000000 * 8), &rate_delta) == -1) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: rate_delta_config > 64bits\n")));
		return(SOC_E_INTERNAL);
	    }

	    rate_delta = (rate_delta * soc_sbx_qe2000_rate_delta_percentage[percent_off]) / 100;
            template_index = cur_index * QE2000_RATE_DELTA_MAX_PERCENTAGE + percent_off;

            if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
                if (template_index == 0) {
                    rate_delta = 0;
                }
            }
            rc = soc_qe2000_qm_mem_write(unit, template_index, 0x9 /* Rate Delta Max */,
                                         rate_delta, 0, 0, 0);
            if (rc != SOC_E_NONE) {
                return(rc);
            }
        }
    }
    return(rc);
}

uint32
soc_qe2000_rate_delta_index_get(int unit, uint32 bag_rate, uint32 *index)
{
    int rc = SOC_E_NONE;
    int cur_index, percent_off = 2;
    uint32 *rate_delta_port_speed_p;


    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
        rate_delta_port_speed_p = &soc_sbx_qe2000_standalone_rate_delta_port_speed[0];
    }
    else {
        rate_delta_port_speed_p = &soc_sbx_qe2000_rate_delta_port_speed[0];
    }

    for (cur_index = 0; cur_index < QE2000_RATE_DELTA_MAX_PORT_SPEEDS; cur_index++) {
        if (bag_rate < *(rate_delta_port_speed_p + cur_index)) {
            break;
        }
    }

    if (cur_index >= QE2000_RATE_DELTA_MAX_PORT_SPEEDS) {
        cur_index = QE2000_RATE_DELTA_MAX_PORT_SPEEDS - 1;
    }

    (*index) = (cur_index * QE2000_RATE_DELTA_MAX_PERCENTAGE) + percent_off;

    return(rc);
}

int
soc_qe2000_queue_min_util_set(int unit, int32 nQueue, int template)
{
    int rc = SOC_E_UNAVAIL;


    return(rc);
}

int
soc_qe2000_queue_min_util_get(int unit, int32 nQueue, int *template)
{
    int rc = SOC_E_NONE;
    int32  enable, anemic_watermark_sel, min_buffers, max_buffers;
#if 0
    uint32 uData0, uData1, uData2, uData3;
#endif /* 0 */

#if 0
    rc = soc_qe2000_qm_mem_read(unit, nQueue, 00,
		                       &uData0, &uData1, &uData2, &uData3);

    (*template) = (uData0 > 2) & 0x03;
#endif /* 0 */

    rc = soc_qe2000_qstate_mem_read(unit, nQueue,
                                    &enable, &anemic_watermark_sel, &min_buffers, &max_buffers);
    (*template) = anemic_watermark_sel;

    return(rc);
}

int
soc_qe2000_queue_max_age_set(int unit, int32 nQueue, int template)
{
    int rc = SOC_E_NONE;


#if 0
    rc = soc_qe2000_qs_mem_write(unit, 0xA, nQueue, template);
#endif /* 0 */

    rc = soc_qe2000_age_thresh_key_set(unit, nQueue, template);

    return(rc);
}

int
soc_qe2000_queue_max_age_get(int unit, int32 nQueue, int *template)
{
    int rc = SOC_E_NONE;


#if 0
    rc = soc_qe2000_qs_mem_read(unit, 0xA, nQueue, (uint32 *)template);
#endif /* 0 */

    rc = soc_qe2000_age_thresh_key_get(unit, nQueue, template);

    return(rc);
}

int
soc_qe2000_template_min_util_recall(int unit, int dest, int hwUtil, int *util)
{
    uint32 config_reg_value, lines_per_timeslot, bytes_per_timeslot;
    uint32 util_fld;

    /*
     *  This computes the value that *should* have been provided by the user.
     *
     *  The hardware value is used in the cache, and this provides a value to
     *  the BCM layer that will select the value in hardware.
     */
    config_reg_value = SAND_HAL_READ(unit, KA, QM_CONFIG2);
    switch (dest) {
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_LOCAL, config_reg_value);
        break;
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE1000:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE1K, config_reg_value);
        break;
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, config_reg_value);
        break;
    default:
        return SOC_E_PARAM;
    }
    bytes_per_timeslot = lines_per_timeslot * 16;
    util_fld = (hwUtil * 1600) / bytes_per_timeslot;
    if (100 < util_fld) {
        /* Overflow always amounts to 100% */
        *util = 100;
    } else {
        *util = util_fld;
    }
    return SOC_E_NONE;
}

int
soc_qe2000_template_min_util_adjust(int unit, int dest, int util, int *hwUtil)
{
    uint32       util_fld;
    uint32       config_reg_value, lines_per_timeslot, bytes_per_timeslot;

    /*
     *  Special case -- if util of 100%, QE2K waits until timeout to send
     *  non-complete timeslots from a queue, so we can just set maximum
     *  possible hardware value.  This makes 100% only take 1 template, even if
     *  timeslot sizes are different.  The hardware manual does not seem to
     *  indicate any support for spanning timeslots, so bogusly large values
     *  here would simply indicate never consider a partial timeslot full.
     */
    if (100 == util) {
        *hwUtil = 0xFF;
        return SOC_E_NONE;
    }
    /*
     *  Any value outside of 0%..100% is invalid (and we covered 100% above).
     *
     *  It seems that 0% might be valid, indicating that the QE2K should never
     *  wait to send a partial timeslot, so we allow it.
     */
    if ((0 > util) || (99 < util)) {
        return SOC_E_PARAM;
    }

    /*
     *  This computes the value that *should* be written to hardware.
     *
     *  This value is used in the cache for a few reasons:
     *    - it is easier to recover for easy-reload
     *    - it is easier to consolidate equivalent but different values
     *    - it might reuse hardware values across different destination types
     *
     *  In addition, it exposes the hardware granularity to the user, so he
     *  knows what to expect more accurately.
     */
    config_reg_value = SAND_HAL_READ(unit, KA, QM_CONFIG2);
    switch (dest) {
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA,
                                                QM_CONFIG2,
                                                LINES_PER_TS_LOCAL,
                                                config_reg_value);
        break;
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE1000:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA,
                                                QM_CONFIG2,
                                                LINES_PER_TS_QE1K,
                                                config_reg_value);
        break;
    case SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000:
        lines_per_timeslot = SAND_HAL_GET_FIELD(KA,
                                                QM_CONFIG2,
                                                LINES_PER_TS_QE2K,
                                                config_reg_value);
        break;
    default:
        return SOC_E_PARAM;
    }
    bytes_per_timeslot = lines_per_timeslot * 16;
    util_fld = (bytes_per_timeslot * util) / 100 +
                     (((bytes_per_timeslot * util) % 100) ? 1 : 0);
    util_fld = (util_fld / 16) + ((util_fld % 16) ? 1 : 0);
    /*
     *  We already took care of the 100% case, so we never want to round to it
     *  accidentally.  Make sure that the non-100% cases are never quite full,
     *  but also ensure we don't say zero unless it was intentional (that is,
     *  don't say zero unless the caller said zero).  If the only choice is
     *  between 0% and 100%, choose 100% (this should only occur if timeslot
     *  size is 1).
     */
    if (lines_per_timeslot <= util_fld) {
        if (1 == lines_per_timeslot) {
            *hwUtil = 1;
        } else {
            *hwUtil = lines_per_timeslot - 1;
        }
    } else {
        *hwUtil = util_fld;
    }
    return SOC_E_NONE;
}

int
soc_qe2000_template_min_util_set(int unit, int32 template, int hwUtil)
{
    int          rc = SOC_E_NONE;
    uint32       reg_value;

    if (0xFF < hwUtil) {
        return SOC_E_PARAM;
    }

    if (template < 4) {
        reg_value = SAND_HAL_READ(unit, KA, QM_ANEMIC_WATERMARK1);
        switch (template) {
            case 0:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK1,
                                               ANEMIC_WATERMARK0,
                                               reg_value,
                                               hwUtil);
                break;
            case 1:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK1,
                                               ANEMIC_WATERMARK1,
                                               reg_value,
                                               hwUtil);
                break;
            case 2:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK1,
                                               ANEMIC_WATERMARK2,
                                               reg_value,
                                               hwUtil);
                break;
            case 3:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK1,
                                               ANEMIC_WATERMARK3,
                                               reg_value,
                                               hwUtil);
                break;
        }
        SAND_HAL_WRITE(unit, KA, QM_ANEMIC_WATERMARK1, reg_value);
    }
    else {
        reg_value = SAND_HAL_READ(unit, KA, QM_ANEMIC_WATERMARK0);
        switch ((template - 4)) {
            case 0:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK0,
                                               ANEMIC_WATERMARK4,
                                               reg_value,
                                               hwUtil);
                break;
            case 1:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK0,
                                               ANEMIC_WATERMARK5,
                                               reg_value,
                                               hwUtil);
                break;
            case 2:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK0,
                                               ANEMIC_WATERMARK6,
                                               reg_value,
                                               hwUtil);
                break;
            case 3:
            default:
                reg_value = SAND_HAL_MOD_FIELD(KA,
                                               QM_ANEMIC_WATERMARK0,
                                               ANEMIC_WATERMARK7,
                                               reg_value,
                                               hwUtil);
                break;
        }
        SAND_HAL_WRITE(unit, KA, QM_ANEMIC_WATERMARK0, reg_value);
    }

    return(rc);
}

int
soc_qe2000_template_max_age_recall(int unit, int hwAge, int *age)
{
    /*
     *  This computes the value that *should* have been supplied by the user.
     *
     *  The hardware value is used in the cache, and this provides a value to
     *  the BCM layer that will select the value in hardware.
     */
    if (0xFF > hwAge) {
        *age = hwAge * SB_FAB_DEVICE_QE2000_CONN_AGE_UNIT;
    } else {
        
        *age = -1;
    }
    return SOC_E_NONE;
}


int
soc_qe2000_template_max_age_adjust(int unit, int age, int *hwAge)
{
    uint32 age_fld;

    /*
     *  This computes the value that *should* be written to hardware.
     *
     *  This value is used in the cache for a few reasons:
     *    - it is easier to recover for easy-reload
     *    - it is easier to consolidate equivalent but different values
     *    - it might reuse hardware values across different destination types
     *
     *  In addition, it exposes the hardware granularity to the user, so he
     *  knows what to expect more accurately.
     */
    
    age_fld = ((uint)age / SB_FAB_DEVICE_QE2000_CONN_AGE_UNIT) +
              ((((uint)age % SB_FAB_DEVICE_QE2000_CONN_AGE_UNIT) == 0) ? 0 : 1);
    if (age_fld > 0xFF) {
        age_fld = 0xFF;
    }
    *hwAge = age_fld;
    return SOC_E_NONE;
}

int
soc_qe2000_template_max_age_set(int unit, int32 template, int hwAge)
{
    uint32 reg_value;
    int rc;

    if (hwAge > 0xFF) {
        return(SOC_E_PARAM);
    }

    
    reg_value = (hwAge << 8) | 0xFF;

    rc = soc_qe2000_qs_mem_write(unit, 0xB, template, reg_value);
    if (rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Age threshold LUT write"
                               " timed out key 0x%x\n"), template));
    }
    return(rc);
}

int
soc_qe2000_mc_pointer_fifo_get(int unit, int32 fifo, int32 *size, int32 *base, int32 *drop_on_full)
{
    uint32 uData;
    uint32 uCtrl;
    uint32 uTimeOut;
    int rc = SOC_E_NONE;

    *size = -1;
    *base = -1;
    *drop_on_full = -1;

    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x8);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, fifo);
    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL, uData);
    
    uTimeOut = 800;
    while(uTimeOut--) {
        uCtrl = SAND_HAL_READ_POLL((sbhandle)unit, KA, EG_MEM_ACC_CTRL);
        if (SAND_HAL_GET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uCtrl)) {

	    uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MEM_ACC_DATA0);  

            *drop_on_full = (uData >> 25) & 1;
            *size         = (uData >> 10) & 7;
	    *base         = (uData & 0x3ff);

            SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, 0x1));

            return rc;
        }
    }
    return -1;
}

int
soc_qe2000_mc_pointer_fifo_set(int unit, int32 fifo, int32 size, int32 base, int32 drop_on_full)
{

    uint32 uData = 0;
    uint32 uCtrl;
    uint32 uTimeOut;
    int rc = SOC_E_NONE;

#ifdef DEBUG_EG_FIFO_PTRS
    LOG_CLI((BSL_META_U(unit,
                        "fifo(%d) size(2^%d) base(%d)\n"), fifo, size, base));
#endif

    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_DATA0, DATA, uData, (drop_on_full<<25 | size<<10 | base));
    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_DATA0, uData);
    
    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, REQ, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, RD_WR_N, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, uData, 0x8);
    uData = SAND_HAL_MOD_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, uData, fifo);
    SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL, uData);
    uTimeOut = 800;
    while(uTimeOut--) {
        uCtrl = SAND_HAL_READ_POLL((sbhandle)unit, KA, EG_MEM_ACC_CTRL);

        if (SAND_HAL_GET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, uCtrl)) {
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_MEM_ACC_CTRL,
                           SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ACK, 0x1));

            return rc;
        }
    }
    return -1;    
}
 
#endif /* BCM_QE2000_SUPPORT */
