
/*
 * $Id: qe2000_spi.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ============================================================
 * == qe2000_spi.c - QE Initialization RESET and SPI4 Only      ==
 * ============================================================
 */
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/glue.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/qe2000_spi.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_common.h>

static void hwQe2000InReset(sbhandle userDeviceHandle);
static void hwQe2000UnReset(sbhandle userDeviceHandle);

static uint32 hwQe2000BringupSpi4Tx(sbhandle userDeviceHandle,
				      uint32 numPorts_ul,
				      uint32 maxBurst1_ul,
				      uint32 maxBurst2_ul,
				      uint32 dataMaxT_ul,
				      uint32 alpha_ul,
				      uint32 spiTsInvert_ul,
				      uint32 spi_ul,
				      uint32 txIgnStatus_ul,
				      uint64 requeuePortsMask_ull,
                                      uint32 cal_m );

static void hwQe2000ResetSpi4Clk(sbhandle userDeviceHandle, uint32 spi_ul);

static uint32 hwQe2000BringupSpi4Rx(sbhandle userDeviceHandle,
				      uint32 numPorts_ul,
				      uint64 jumboMask_ull,
				      uint32 minFrameSz_ul,
				      uint32 maxFrameSz_ul,
				      int32  alignDelay_ul,
				      uint32 spiRsInvert_ul,
				      uint32 spi_ul,
                                      uint32 cal_m );

static uint32 hwQe2000SpiClockSetup(sbhandle userDeviceHandle,
                                      uint32 spiPort,
				      uint32 uSpiRefClockSpeed,
				      uint32 uSpiClockSpeed);

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000Init()
 *
 * OVERVIEW:        Reset the QE, initialize SPI interfaces.
 *
 * ARGUMENTS:       hwQe2000Init_sp
 *
 * RETURNS:
 *                  status of initialization - HW_QE2000_STS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Reset the QE, initialize SPI interfaces.
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32 hwQe2000InitSpi(HW_QE2000_INIT_ST *hwQe2000Init_sp)
{
    uint32 sts_ul;
    uint32 nSpi0TxChans, nSpi0RxChans, nSpi1TxChans, nSpi1RxChans;


    /* Overrides if you are in full packet mode, always ignore status, EI block handles this */
    if (hwQe2000Init_sp->bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_FULL) {
	hwQe2000Init_sp->Spi0TxIgnStatus_ul = 1;
    }

    if (hwQe2000Init_sp->bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_FULL) {
	hwQe2000Init_sp->Spi1TxIgnStatus_ul = 1;
    }

    /* For N:1 mode set the SPI Tx/Rx channels to 1 */
    nSpi0TxChans = hwQe2000Init_sp->Spi0TxChans_ul;
    nSpi0RxChans = hwQe2000Init_sp->Spi0RxChans_ul;
    nSpi1TxChans = hwQe2000Init_sp->Spi1TxChans_ul;
    nSpi1RxChans = hwQe2000Init_sp->Spi1RxChans_ul;
    if (hwQe2000Init_sp->bEiSpiFullPacketMode[0] == SOC_SBX_PORT_MODE_PKT_IL_N_1) {
        nSpi0TxChans = 1;
        nSpi0RxChans = 1;
    }
    if (hwQe2000Init_sp->bEiSpiFullPacketMode[1] == SOC_SBX_PORT_MODE_PKT_IL_N_1) {
        nSpi1TxChans = 1;
        nSpi1RxChans = 1;
    }

    /*
    ** Soft reset the QE
    */
    if (hwQe2000Init_sp->reset) {
        hwQe2000InReset(hwQe2000Init_sp->userDeviceHandle);
    }
    hwQe2000UnReset(hwQe2000Init_sp->userDeviceHandle);

    /* Check if SPI-0 clock setting is other then default */
    if (hwQe2000Init_sp->uSpiClockSpeed[0] != 0) {
        sts_ul = hwQe2000SpiClockSetup(hwQe2000Init_sp->userDeviceHandle, 0,
                                       hwQe2000Init_sp->uSpiRefClockSpeed[0],
				       hwQe2000Init_sp->uSpiClockSpeed[0]);
        if (sts_ul != HW_QE2000_STS_OK_K) {
            return(sts_ul);
        }
    }

    /* Check if SPI-1 clock setting is other then default */
    if (hwQe2000Init_sp->uSpiClockSpeed[1] != 0) {
        sts_ul = hwQe2000SpiClockSetup(hwQe2000Init_sp->userDeviceHandle, 0,
                                       hwQe2000Init_sp->uSpiRefClockSpeed[1],
				       hwQe2000Init_sp->uSpiClockSpeed[1]);
        if (sts_ul != HW_QE2000_STS_OK_K) {
            return(sts_ul);
        }
    }

    /*
    ** Bring up the Spi4 Transmit interface 0
    */
    sts_ul = hwQe2000BringupSpi4Tx(hwQe2000Init_sp->userDeviceHandle,
				   nSpi0TxChans,
				   hwQe2000Init_sp->Spi0MaxBurst1_ul,
				   hwQe2000Init_sp->Spi0MaxBurst2_ul,
				   hwQe2000Init_sp->Spi0DataMaxT_ul,
				   hwQe2000Init_sp->Spi0Alpha_ul,
				   hwQe2000Init_sp->Spi0TsInvert_ul,
				   0,
				   hwQe2000Init_sp->Spi0TxIgnStatus_ul,
				   hwQe2000Init_sp->uuRequeuePortsMask[0],
                                   hwQe2000Init_sp->Spi0TxCalM);

    if (HW_QE2000_STS_OK_K != sts_ul)
    {
	return sts_ul;
    }

    /*
    ** Bring up the Spi4 Transmit interface 1
    */
    sts_ul = hwQe2000BringupSpi4Tx(hwQe2000Init_sp->userDeviceHandle,
				   nSpi1TxChans,
				   hwQe2000Init_sp->Spi1MaxBurst1_ul,
				   hwQe2000Init_sp->Spi1MaxBurst2_ul,
				   hwQe2000Init_sp->Spi1DataMaxT_ul,
				   hwQe2000Init_sp->Spi1Alpha_ul,
				   hwQe2000Init_sp->Spi1TsInvert_ul,
				   1,
				   hwQe2000Init_sp->Spi1TxIgnStatus_ul,
				   hwQe2000Init_sp->uuRequeuePortsMask[1],
                                   hwQe2000Init_sp->Spi1TxCalM);

    if (HW_QE2000_STS_OK_K != sts_ul)
    {
	return sts_ul;
    }


    /*
    ** Bring up the Spi4 Receive interface 0
    */
    sts_ul = hwQe2000BringupSpi4Rx(hwQe2000Init_sp->userDeviceHandle,
				   nSpi0RxChans,
				   hwQe2000Init_sp->Spi0JumboMask_ull,
				   hwQe2000Init_sp->Spi0MinFrameSz_ul,
				   hwQe2000Init_sp->Spi0MaxFrameSz_ul,
				   hwQe2000Init_sp->Spi0AlignDelay_ul,
				   hwQe2000Init_sp->Spi0RsInvert_ul,
				   0,
                                   hwQe2000Init_sp->Spi0RxCalM );

    if (HW_QE2000_STS_OK_K != sts_ul)
    {
	return sts_ul;
    }
    /*
    ** Bring up the Spi4 Receive interface 1
    */
   sts_ul = hwQe2000BringupSpi4Rx(hwQe2000Init_sp->userDeviceHandle,
				  nSpi1RxChans,
				  hwQe2000Init_sp->Spi1JumboMask_ull,
				  hwQe2000Init_sp->Spi1MinFrameSz_ul,
				  hwQe2000Init_sp->Spi1MaxFrameSz_ul,
				  hwQe2000Init_sp->Spi1AlignDelay_ul,
				  hwQe2000Init_sp->Spi1RsInvert_ul,
				  1,
                                  hwQe2000Init_sp->Spi1RxCalM );

    if (HW_QE2000_STS_OK_K != sts_ul)
    {
	return sts_ul;
    }

    return HW_QE2000_STS_OK_K;
}
/*****************************************************************************
 * FUNCTION NAME:   hwQe2000InReset()
 *
 * OVERVIEW:        Put the QE in soft reset.
 *
 * ARGUMENTS:       baseAddr_ul - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Soft reset the QE device.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/

static void hwQe2000InReset(sbhandle userDeviceHandle)
{
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CONFIG, CII_ENABLE, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_RESET, 0xF);
    thin_delay(HW_QE2000_100_MSEC_K);

#if 0
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST0_CORE_RESET, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST1_CORE_RESET, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR0_CORE_RESET, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR1_CORE_RESET, 0x1);
    thin_delay(HW_QE2000_100_MSEC_K);
#endif
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000UnReset()
 *
 * OVERVIEW:        Take the QE out of soft reset.
 *
 * ARGUMENTS:       baseAddr_ul - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Take the QE out of soft reset.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static void hwQe2000UnReset(sbhandle userDeviceHandle)
{
    SAND_HAL_WRITE(userDeviceHandle, KA, PC_RESET, 0x0);
    thin_delay(HW_QE2000_100_MSEC_K);
#if 0
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST0_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST1_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR0_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR1_CORE_RESET, 0x0);
    thin_delay(HW_QE2000_100_MSEC_K);
#endif
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000BringupSpi4Tx()
 *
 * OVERVIEW:        Bring up the QE SPI4 transmit.
 *
 * ARGUMENTS:
 *         userDeviceHandle  - base address of the device.
 *         numPorts_ul  - number of SPI-4 ports to enable
 *         maxBurst1_ul - max burst which can be sent when starving SB_ASSERTed.
 *         maxBurst2_ul - max burst which can be sent when hungry is SB_ASSERTed.
 *         dataMaxT_ul  - max interval between scheduling of training sequences
 *                        HW_FE_TRAIN_DISABLE = 0x0  no training
 *                        HW_FE_TRAIN_2TO20     2^20 cycles
 *                        HW_FE_TRAIN_2TO22     2^22 cycles
 *                        HW_FE_TRAIN_2TO24     2^24 cycles
 *                        HW_FE_TRAIN_2TO26     2^26 cycles
 *                        HW_FE_TRAIN_2TO28     2^28 cycles
 *                        HW_FE_TRAIN_2TO30     2^30 cycles
 *                        HW_FE_TRAIN_2TO32     2^32 cycles
 *
 * RETURNS:
 *                  HW_FE_STS_OK_K (0) or negative number on error.
 *
 * DESCRIPTION:     Initializes the following SPI4 transmit on the FE:
 *                          disable SPI transmit
 *                          program SPIT
 *                          setup calendar
 *                          enable
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32 hwQe2000BringupSpi4Tx(sbhandle      userDeviceHandle,
				      uint32      numPorts_ul,
				      uint32      maxBurst1_ul,
				      uint32      maxBurst2_ul,
				      uint32      dataMaxT_ul,
				      uint32      alpha_ul,
				      uint32      spiTsInvert_ul,
				      uint32      spi_ul,
				      uint32      txIgnStatus_ul,
				      uint64      requeuePortsMask_ull,
                                      uint32      cal_m)
{

    uint32 nData=0;
    uint32 nArbLen = numPorts_ul-1;
    uint64 nnTemp = COMPILER_64_INIT(0,0);
    int32 i;

    if (numPorts_ul == 0) {
        return HW_QE2000_STS_OK_K;
    }

    /* must increase ArbLen for requeuePortsMask bits set, since requeue
       ports are not included in numPorts
       See bug 22538 */
    for (i=0;i<64;i++){
	if (i < 32) {
	    COMPILER_64_SET(nnTemp, 0, 1 << i);
	} else if (i < 64) {
	    COMPILER_64_SET(nnTemp, 1 << (i - 32), 0);
	}
        COMPILER_64_AND(nnTemp, requeuePortsMask_ull);
	if (!COMPILER_64_IS_ZERO(nnTemp)) {
	    nArbLen++;
	}
    }

    if (numPorts_ul > HW_QE2000_MAX_SPI4_PORTS_K) {
	return HW_QE2000_STS_SPI4_PORT_OUT_OF_RANGE_K;
    }
    /* Setup SPI-4 Calendar (fixed) */
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG0, 0x03020100);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG1, 0x07060504);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG2, 0x0b0a0908);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG3, 0x0f0e0d0c);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG4, 0x13121110);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG5, 0x17161514);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG6, 0x1b1a1918);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG7, 0x1f1e1d1c);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG8, 0x23222120);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG9, 0x27262524);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG10,0x2b2a2928);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CAL_CONFIG11,0x2f2e2d2c);

    /* jts - May 11 2004 - Must take SLE out of reset first */
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG1, PC_RESET, 0x0);

    nData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_ARB_LEN, nData, nArbLen);
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_CAL_LEN, nData, numPorts_ul-1);
    /*  nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_CAL_M, nData, numPorts_ul-1); */
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_CAL_M, nData, cal_m);
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_ENABLE_TEST, nData, 1);

    if (txIgnStatus_ul) {
	nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_IGN_STAT, nData, 1);
    } else {
	nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_IGN_STAT, nData, 0);
    }

    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, nData);

    nData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG1);
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, TSCLK_EDGE, nData, (!spiTsInvert_ul)); /* Invert ts clk */
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, LINES_PER_PORT, nData, 0x3); /* 256 bytes per port */
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, TX_CTRL_MAX_DISABLE, nData, 0x1 );  /* Control only at SOP/EOP */
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, TX_MAXBURST2, nData, maxBurst2_ul );
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, TX_MAXBURST1, nData, maxBurst1_ul );
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, DATA_MAX_T  , nData, dataMaxT_ul );  /* disable interval training */
    nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG1, ALPHA       , nData, alpha_ul  );  /* repeat training only once */
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG1, nData);

    /* When requeueing (mirror-only) must ignore status */
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_IGNORE_STAT0,
			  COMPILER_64_LO(requeuePortsMask_ull));
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_IGNORE_STAT1,
			  COMPILER_64_HI(requeuePortsMask_ull));


    /* jts - May 25 2004 - must be out of reset BEFORE we enable... */
    if ( spi_ul == 0 ) {
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST0_CORE_RESET, 0x0);
    }
    else {
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, ST1_CORE_RESET, 0x0);
    }

    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, TX_ENABLE, 0x1);

    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000BringupSpi4Rx()
 *
 * OVERVIEW:        Bringup QE SPI4 receive.
 *
 * ARGUMENTS:       numPorts_ul - number of SPI-4 ports to enable
 *                  jumboMask_ull - mask indicating which ports support jumbo frames
 *
 *                  minFrameSz_ul - minimum frame size for all ports
 *                  maxFrameSz_ul - maximum frame size for all non-jumbo ports.
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     Bringup the SPI4 receive on the QE:
 *                             reset SPI clock
 *                             disable receive
 *                             enable receiver ports
 *                             setup calendar to be fixed for all enabled ports
 *                             setup training delay
 *                             setup min/max frame sizes (minFrameSz_ul)
 *                                                       (maxFrameSz_ul)
 *                                 (jumbo ports size fixed at 9216 + 12 for rt hdr)
 *                             setup DIP4 LOS thresh and enable
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32 hwQe2000BringupSpi4Rx(sbhandle userDeviceHandle,
				      uint32     numPorts_ul,
				      uint64     jumboMask_ull,
				      uint32     minFrameSz_ul,
				      uint32     maxFrameSz_ul,
				      int32      alignDelay_ul,
				      uint32     spiRsInvert_ul,
				      uint32     spi_ul,
				      uint32     cal_m)
{
    int32 nValue=0;
    int32 i;
    int64 nnPortEnable = COMPILER_64_INIT(0,0);
    int64 nnTemp = COMPILER_64_INIT(0,0);
    int32 nCalEntry = 0;

    if (numPorts_ul > HW_QE2000_MAX_SPI4_PORTS_K) {
	return HW_QE2000_STS_SPI4_PORT_OUT_OF_RANGE_K;
    }

    if(spi_ul == 1){
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR1_CORE_RESET, 0x1);
    }
    else {
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR0_CORE_RESET, 0x1);
    }

    /*
    ** reset SPI clock generation
    */
    hwQe2000ResetSpi4Clk(userDeviceHandle, spi_ul);

    /* Write SrAlignDelay */
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG3, ALIGN_DLY, alignDelay_ul);

    /* Create Port mask */
    for (i = 0; i < numPorts_ul; i++) {
	if (i < 32) {
	    COMPILER_64_SET(nnTemp, 0, 1 << i);
	} else if (i < 64) {
	    COMPILER_64_SET(nnTemp, 1 << (i - 32), 0);
	}
	COMPILER_64_OR(nnPortEnable, nnTemp); 
    }
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG1, RX_PORT_ENABLE, COMPILER_64_LO(nnPortEnable));
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG2, RX_PORT_ENABLE, COMPILER_64_HI(nnPortEnable));

    for (i = 0; i < numPorts_ul; i++) {
	if (HW_QE2000_ZIN_MASK64(i, nnPortEnable)) {
	    if (HW_QE2000_ZIN_MASK64(i, nnPortEnable)) {
		SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_P0_FRAME_SIZE, i,
					    SAND_HAL_SET_FIELD(KA, SR0_P0_FRAME_SIZE, MIN_FRAME_SIZE, minFrameSz_ul) |
					    SAND_HAL_SET_FIELD(KA, SR0_P0_FRAME_SIZE, MAX_FRAME_SIZE, HW_QE2000_MAX_FRM_SZ_JMBO_K+HW_QE2000_SHIMHDR_SZ_K)
					    );
	    }
	    else {
		SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_P0_FRAME_SIZE, i,
					    SAND_HAL_SET_FIELD(KA, SR0_P0_FRAME_SIZE, MIN_FRAME_SIZE, minFrameSz_ul) |
					    SAND_HAL_SET_FIELD(KA, SR0_P0_FRAME_SIZE, MAX_FRAME_SIZE, maxFrameSz_ul)
					    );
	    }

	    nValue = SAND_HAL_READ_INDEX_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CAL_CONFIG0, nCalEntry/4);
	    nValue = (nValue&~(0xff<<((nCalEntry%4)*8))) | (i<<((nCalEntry%4)*8));
	    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, KA, SR, spi_ul,  SR0_CAL_CONFIG0, nCalEntry/4, nValue);
	    nCalEntry++;
	}
    }

    /* set the calendar length and m value */
    nValue = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0);
    nValue = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_CAL_LEN, nValue, (numPorts_ul-1));
    nValue = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_CAL_M, nValue, cal_m);
    nValue = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_DIP4_LOS_THRESH, nValue, 2);
    nValue = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_ENABLE_TEST, nValue, 1);

    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, nValue);

    if(spi_ul == 1){
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR1_CORE_RESET, 0x0);
    }
    else {
	SAND_HAL_RMW_FIELD(userDeviceHandle, KA, PC_CORE_RESET0, SR0_CORE_RESET, 0x0);
    }

    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, RX_ENABLE, 0x1);

    /* ejs - May 06 2005 - invert the status clock */
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_PC_CONFIG, RSCLK_EDGE, (!spiRsInvert_ul) );
    SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, PC_RESET, 0x0);

    return HW_QE2000_STS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000ResetSpi4Clk()
 *
 * OVERVIEW:        Reset the SPI4 TX/RX Hyperphy PLL
 *
 * ARGUMENTS:       NONE
 *
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     Reset the SPI4 Clock.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:     hwBaseAddr_sp variable has been initialized with FE base.
 *
 *****************************************************************************/
static void hwQe2000ResetSpi4Clk(sbhandle userDeviceHandle, uint32 spi_ul)
{
    uint32 sr0_cfg1, st0_cfg1, reg;

    /*
    ** Read the Spi4 Configurations in
    */
    st0_cfg1 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);
    sr0_cfg1 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0);

    /*
    ** Disable Spi4-Tx path during reset
    */
    reg = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_ENABLE, st0_cfg1, 0);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, reg);

    /*
    ** Disable Spi4-Rx path during reset
    */
    reg = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_ENABLE, sr0_cfg1, 0);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, reg);
    thin_delay(HW_QE2000_100_MSEC_K);

    /*
    ** Restore the RX config
    */
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr0_cfg1);

    /*
    ** Restore the TX config
    */
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, st0_cfg1);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000Spi4TxForceTrainingOn()
 *
 * OVERVIEW:        Turn on training on the transmit SPI4 interface.
 *
 * ARGUMENTS:       userDeviceHandle - base address (handle) to device.
 *
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     This turns on tx_force_training in ST_CONFIG1 then delays
 *                  before returning.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    Delay.
 *
 *****************************************************************************/
void hwQe2000Spi4TxForceTrainingOn(sbhandle userDeviceHandle, uint32 spi_ul)
{
    uint32 reg;
    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);
    reg = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_FORCE_TRAINING, reg, 1);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA,ST, spi_ul, ST0_CONFIG0, reg);
    thin_delay(HW_QE2000_100_MSEC_K);
}
/*****************************************************************************
 * FUNCTION NAME:   hwFeSpi4TxForceTrainingOff()
 *
 * OVERVIEW:        Turn off training on the transmit SPI4 interface.
 *
 * ARGUMENTS:       userDeviceHandle - base address (handle) to device.
 *
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     This turns off tx_force_training in ST_CONFIG1 then delays
 *                  before returning.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    Delay.
 *
 *****************************************************************************/

void hwQe2000Spi4TxForceTrainingOff(sbhandle userDeviceHandle, uint32 spi_ul)
{
    uint32 reg;
    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);
    reg = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_FORCE_TRAINING, reg, 0);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, reg);
    thin_delay(HW_QE2000_100_MSEC_K);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000Spi4RxStatus()
 *
 * OVERVIEW:        Determine health of SPI4 rx.
 *
 * ARGUMENTS:       userDeviceHandle - handle to device.
 *
 * RETURNS:
 *                  -1 if not trained or insync in SR_STATUS
 *                  SR_ERROR1 register value if non-zero (error)
 *                  0 if good.
 *
 * DESCRIPTION:     Checks SR_STATUS and SR_ERROR1 for SPI4 rx status.
 *                  Clears out SR_ERROR1 and rechecks after delay.
 *
 * ASSUMPTIONS:	    SPI4 interface is enabled.
 *
 * SIDE EFFECTS:    Delay
 *
 *****************************************************************************/

uint32 hwQe2000Spi4RxStatus(sbhandle userDeviceHandle, uint32 spi_ul)
{
    uint32 reg, rxsync;
    /*
    ** Read regs  and clear out
    */
    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_STATUS);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_STATUS, reg);

    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_ERROR);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_ERROR, reg);

    /*
    ** delay
    */
    thin_delay(HW_QE2000_500_MSEC_K);

    /*
    ** Check sr_status to see whether trained and insync
    */
    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_STATUS);
    rxsync = SAND_HAL_GET_FIELD(KA, SR0_STATUS, RX_INSYNC, reg);

    if (rxsync == 0) {
	return(0xffffffff); /* to indicate an error */
    }

    reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_ERROR);

    if (reg  != 0) {
      return(reg);
    }
    return(0);
}

/*****************************************************************************
 * FUNCTION NAME:   hwQe2000Spi4TxStatus()
 *
 * OVERVIEW:        Determine health of SPI4 tx.
 *
 * ARGUMENTS:       userDeviceHandle - handle to device.
 *
 * RETURNS:
 *                  -1 if not trained or insync in ST_STATUS
 *                  ST_ERROR register value if non-zero (error)
 *                  0 if good.
 *
 * DESCRIPTION:     Checks ST_STATUS and ST_ERROR for SPI4 tx status.
 *                  Clears out ST_ERROR and rechecks after delay.
 *
 * ASSUMPTIONS:	    SPI4 interface is enabled.
 *
 * SIDE EFFECTS:    3 second delay
 *
 *****************************************************************************/
uint32 hwQe2000Spi4TxStatus(sbhandle userDeviceHandle, uint32 spi_ul)
{
  uint32 reg, txsync, locktx;

  reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_STATUS);
  txsync = SAND_HAL_GET_FIELD(KA, ST0_STATUS, TX_INSYNC, reg);
  locktx = SAND_HAL_GET_FIELD(KA, ST0_STATUS, LOCKTX, reg);

  if (0 == txsync || 0 == locktx) {
    return(-1);
  }

  reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_ERROR);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_ERROR, reg);

  /*
  ** Re-read register to see if any new errors cropped up after delaying
  */
  thin_delay(HW_QE2000_500_MSEC_K);

  reg = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_ERROR);

  if (reg != 0) {
    return(reg);
  }
  return(0);
}


void hwQe2000Spi4RxEnableGet(sbhandle userDeviceHandle, uint32 *enable_ul, uint32 spi_ul)
{
    uint32 sr_cfg0;
    sr_cfg0 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0);

    *enable_ul = 0;
    if (SAND_HAL_GET_FIELD(KA, SR0_CONFIG0, PC_RESET, sr_cfg0) == 0) {
        if (SAND_HAL_GET_FIELD(KA, SR0_CONFIG0, RX_ENABLE, sr_cfg0) == 1) {
            *enable_ul = 1;
        }
    }
}

void hwQe2000Spi4RxEnable(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul)
{
    uint32 sr_cfg0;
    sr_cfg0 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0);

    if (enable_ul){

        sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, PC_RESET, sr_cfg0, 0);
	SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_QE2000_100_MSEC_K);

	sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_ENABLE, sr_cfg0, 1);
	SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_QE2000_100_MSEC_K);

    }
    else {
	sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_ENABLE, sr_cfg0, 0);
	SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_QE2000_100_MSEC_K);

	sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, PC_RESET, sr_cfg0, 1);
	SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_QE2000_100_MSEC_K);

    }

}



void hwQe2000Spi4TxEnableGet(sbhandle userDeviceHandle, uint32 *enable_ul, uint32 spi_ul)
{
    uint32 st_cfg0;
    st_cfg0 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);

    *enable_ul = 0;
    if (SAND_HAL_GET_FIELD(KA, ST0_CONFIG0, TX_ENABLE, st_cfg0) == 1) {
        *enable_ul = 1;

    }
}

void hwQe2000Spi4TxEnable(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul)
{
   uint32 reg, st_cfg1;

   st_cfg1 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0);

   if (enable_ul) {
       reg = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_ENABLE, st_cfg1, 1);
   }
   else {
     reg = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG0, TX_ENABLE, st_cfg1, 0);
   }
   SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spi_ul, ST0_CONFIG0, reg);
   thin_delay(HW_QE2000_100_MSEC_K);
}

void hwQe2000Spi4RxRequestTraining(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul)
{
    uint32 sr_cfg0;

   sr_cfg0 = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0);

   if (enable_ul) {
       sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_FORCE_TRAINING, sr_cfg0, 1);
   }
   else {
       sr_cfg0 = SAND_HAL_MOD_FIELD(KA, SR0_CONFIG0, RX_FORCE_TRAINING, sr_cfg0, 0);
   }
   SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, SR, spi_ul, SR0_CONFIG0, sr_cfg0);

   thin_delay(HW_QE2000_100_MSEC_K);

}


static uint32
hwQe2000SpiClockSetup(sbhandle userDeviceHandle,
                      uint32 spiPort,
                      uint32 uSpiRefClockSpeed,
                      uint32 uSpiClockSpeed)
{
    uint32 rc = HW_QE2000_STS_OK_K;
    uint32 *freq_p;
    int index = 0;
    int txpll_m = 0;
    int txpll_n = 0;
    int txpll_p = 0;
    uint32 refClk = uSpiRefClockSpeed; /* in KHz */
    uint32 freq, numerator, denominator;
    uint32 nData = 0;


    if (uSpiClockSpeed == 0) {
        return(rc);
    }
    if (uSpiRefClockSpeed == 0) {
        return(HW_QE2000_STS_FAILURE_K);
    }

    freq_p = sal_alloc((sizeof(uint32) * HW_QE2000_SFI_TXPLL_M_MAX *
                          HW_QE2000_SFI_TXPLL_N_MAX * HW_QE2000_SFI_TXPLL_P_MAX), "SPI Freq");
    if (freq_p == NULL) {
        return(HW_QE2000_STS_FAILURE_K);
    }

    /* NOTE: currently not using TXDIV_SEL. Valid range 100 MHz - 125 MHz */
    for (txpll_m = 1; txpll_m < HW_QE2000_SFI_TXPLL_M_MAX; txpll_m++) {
        for (txpll_n = 0; txpll_n < HW_QE2000_SFI_TXPLL_N_MAX; txpll_n++) {
            for (txpll_p = 0; txpll_p < HW_QE2000_SFI_TXPLL_P_MAX; txpll_p++) {
                numerator= refClk * txpll_m;
                denominator= (txpll_n + 1) * (txpll_p + 1);
                freq = numerator / denominator;
                index = (txpll_m * HW_QE2000_SFI_TXPLL_N_MAX * HW_QE2000_SFI_TXPLL_P_MAX) +
                          (txpll_n * HW_QE2000_SFI_TXPLL_P_MAX) + (txpll_p);
                *(freq_p + index) = freq; 
            }
        }
    }

    /* Currently pickup the parameters with the first match that occurs */
    for (txpll_m = 1; txpll_m < HW_QE2000_SFI_TXPLL_M_MAX; txpll_m++) {
        for (txpll_n = 0; txpll_n < HW_QE2000_SFI_TXPLL_N_MAX; txpll_n++) {
            for (txpll_p = 0; txpll_p < HW_QE2000_SFI_TXPLL_P_MAX; txpll_p++) {
                index = (txpll_m * HW_QE2000_SFI_TXPLL_N_MAX * HW_QE2000_SFI_TXPLL_P_MAX) +
                          (txpll_n * HW_QE2000_SFI_TXPLL_P_MAX) + (txpll_p);
                if (*(freq_p + index) != uSpiClockSpeed) {
                    continue;
                }

                nData = SAND_HAL_READ_STRIDE(userDeviceHandle, KA, ST, spiPort, ST0_CONFIG2);
                nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG2, TXPLL_M, nData, txpll_m);
                nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG2, TXPLL_N, nData, txpll_n);
	        nData = SAND_HAL_MOD_FIELD(KA, ST0_CONFIG2, TXPLL_P, nData, txpll_p);
                SAND_HAL_WRITE_STRIDE(userDeviceHandle, KA, ST, spiPort, ST0_CONFIG2, nData);

                sal_free(freq_p);
                return(rc);
            }
        }
    }

    sal_free(freq_p);

    rc = HW_QE2000_STS_FAILURE_K;
    return(rc);
}

