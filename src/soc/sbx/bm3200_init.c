/*
 * $Id: bm3200_init.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ============================================================
 * == bm3200_init.c - BM3200 Initialization                  ==
 * ============================================================
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/glue.h>
/* #include <soc/sbx/sbTypes.h> */
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/hal_common.h>
#include <soc/util.h>



#define PT_PORT_WRITE(p, r, v) SAND_HAL_WRITE_OFFS(userDeviceHandle, SAND_HAL_REG_OFFSET(PT,r) + (p)*SAND_HAL_PT_IF_INSTANCE_ADDR_STRIDE, v)
#define PT_PORT_READ(p, r) SAND_HAL_READ_OFFS(userDeviceHandle, SAND_HAL_REG_OFFSET(PT,r) + (p)*SAND_HAL_PT_IF_INSTANCE_ADDR_STRIDE)



#define SAND_DBG_ERRORS 1
#define SAND_DBG_WARNINGS 1

/* #define BM3200DEBUG */

#ifdef BM3200DEBUG
uint32 bm3200Debug = 1;
#define BM3200PRINTF(flag, fmt) do { \
    LOG_CLI((BSL_META("%s:%d: "),FUNCTION_NAME(),__LINE__)); \
    bsl_printf fmt; \
} while(0)
#else
#define BM3200PRINTF(flag, fmt)
#endif


static void hwBm3200InReset(sbhandle userDeviceHandle);
static void hwBm3200UnReset(sbhandle userDeviceHandle);

static uint32
hwBm3200SetupPi(sbhandle userDeviceHandle,
                uint64     xbSerializerMask_ul,
                uint32     bmSerializerMask_ul,
                uint32     cmode_ul);

static void hwBm3200MaskAllInterrupts(sbhandle userDeviceHandle);

static uint32
hwBm3200RunMbist(sbhandle userDeviceHandle);

static uint32
hwBm3200SetupCrossBar(sbhandle userDeviceHandle,
                      uint64 xbSerializerMask_ull,
                      uint32 xcfgMode_ul);

static int32
hwBm3200EnableSerializer(sbhandle userDeviceHandle,
                         uint32     serializerId,
                         sbBool_t     enable);

static uint32
hwBm3200SetupAm(sbhandle userDeviceHandle,
                uint32     amLocalBmId_ul,
                uint32     amDefaultBmId_ul,
                uint32     bmSerializerMask_ul,
		uint32     bmEnableAutoFailover,
		uint32     bmEnableAutoLinkDisable,
		uint32     bmMaxFailedLinks,
                uint32     spMode);

uint32
hwBm3200NmRandomReadWrite(sbhandle userDeviceHandle,
                          uint32     addr,
                          uint32*    pData0,
                          uint32     rd_wr_n);

static uint32
hwBm3200SetupBwCmode(sbhandle userDeviceHandle,
                     uint32 nQueues,
                     uint32 nVirtualPorts,
                     uint32 nNodes,
                     uint32 epochLength_ul);

static uint32
hwBm3200SetupBwDmode(sbhandle userDeviceHandle,
                     uint32 nQueues,
                     uint32 epochLength_ul);

static uint32
hwBm3200EnableBwManager(sbhandle userDeviceHandle,
			sbBool_t enable);

static uint32
hwBm3200EnableWred(sbhandle userDeviceHandle);

static uint32
hwBm3200SetupBw(sbhandle userDeviceHandle,
                sbBool_t cmode,
                uint32 nNodes,
                uint32 epochLength_ul,
                uint32 modeLatency_ul);

static uint32
hwBm3200SetupIf(sbhandle userDeviceHandle,
                uint64 serializerMask_ull,
                uint64 xbSerializerMask_ull,
                uint32 siLsThreshold_ul,
                uint32 siLsWindow_ul);

static uint32
hwBm3200SetXcfg(sbhandle userDeviceHandle,
                uint32 lcmXcfg_ul[HW_BM3200_MAX_NUM_PLANES][HW_BM3200_PT_NUM_SERIALIZERS],
                uint32 planeActive[HW_BM3200_MAX_NUM_PLANES],
                uint32 *xcfgMode_ulp,
                uint32 lcmXcfgABInputPolarityReversed_ul);

static void
hwBm3200SetXcfgA(sbhandle userDeviceHandle,
                 uint32 xcfg[HW_BM3200_PT_NUM_SERIALIZERS]);

static void
hwBm3200SetXcfgB(sbhandle userDeviceHandle,
                 uint32 xcfg[HW_BM3200_PT_NUM_SERIALIZERS]);

static uint32
hwBwRepositoryReadWrite(sbhandle userDeviceHandle,
                        int32  bank_l,
                        int32  nTableId,
                        uint32 addr_ul,
                        uint32 *data_p,
                        uint32 rd_wr_ul);

static uint32
hwBm3200QltInit(sbhandle userDeviceHandle);
/*****************************************************************************
 * FUNCTION NAME:   hwBm3200Init()
 *
 * OVERVIEW:        Reset the BM3200, initialize bandwith manager interfaces.
 *
 * ARGUMENTS:       hwBm3200Init_sp
 *
 * RETURNS:
 *                  status of initialization - HW_QE1000_STATUS_OK_K (0) or negative
 *                  number on error.
 *
 * DESCRIPTION:     Reset the BM3200 initialize interfaces.
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/
uint32
hwBm3200Init(HW_BM3200_INIT_ST *hwBm3200Init_sp)
{
    uint32 nNodes;
    uint32 i;
    uint32 sts_ul;
    uint32 reg, id, rev;
    sbhandle userDeviceHandle;
    uint32 xcfgMode_ul;
    uint64 xbSerializerMask_ull, ullTmp;

    /* ensure platform specific calibration occurs allowing using this */
    /* function in critical sections.                                  */
    sal_udelay(1);

    xbSerializerMask_ull = hwBm3200Init_sp->seSerializerMask_ull;
    COMPILER_64_OR(xbSerializerMask_ull, hwBm3200Init_sp->lcmSerializerMask_ull);

    userDeviceHandle = hwBm3200Init_sp->userDeviceHandle;

    BM3200PRINTF(BM3200DEBUG, ("Begin init. Handle(0x%x) xbSerializerMask_ull(0x%x%08x)\n\n",(uint32)(userDeviceHandle), COMPILER_64_HI(xbSerializerMask_ull), COMPILER_64_LO(xbSerializerMask_ull)));

    if (!SOC_IS_RELOADING((int32)userDeviceHandle)) {

        reg = SAND_HAL_READ(userDeviceHandle, PT, PI_REVISION);

	id  = SAND_HAL_GET_FIELD(PT, PI_REVISION, IDENTIFICATION, reg);
	rev = SAND_HAL_GET_FIELD(PT, PI_REVISION, REVISION, reg);

	if (id != HW_BM3200_PT_ID ||
	    ((rev != HW_BM3200_PT_REV0) && (rev != HW_BM3200_PT_REV1))) {

	  /*
	   * Not the chip we expected
	   */
	  BM3200PRINTF(BM3200DEBUG, ("Bad Chip: id(0x%x) rev(0x%x)\n\n",id,rev));
	  return HW_BM3200_STATUS_INIT_BM3200_BAD_CHIP_REV_K;
	}
    }

    if (hwBm3200Init_sp->reset) {
        BM3200PRINTF(BM3200DEBUG, ("Taking BM3200 into and out of reset.\n\n"));

        hwBm3200InReset(userDeviceHandle);
    }
    hwBm3200UnReset(userDeviceHandle);


    BM3200PRINTF(BM3200DEBUG, ("Setting up PI\n\n"));
    sts_ul = hwBm3200SetupPi(userDeviceHandle,
                             xbSerializerMask_ull,
                             hwBm3200Init_sp->bmSerializerMask_ul,
                             hwBm3200Init_sp->bmCmode_ul);
    if (sts_ul)
        return sts_ul;


    BM3200PRINTF(BM3200DEBUG, ("Masking interrupts\n\n"));
    hwBm3200MaskAllInterrupts(userDeviceHandle);

    /*
     * Delay for PLL Startup
     */
    thin_delay(HW_BM3200_1_MSEC_K);

    if (hwBm3200Init_sp->bBmRunSelfTest) {
        BM3200PRINTF(BM3200DEBUG, ("Running MBIST\n\n"));
        sts_ul = hwBm3200RunMbist(userDeviceHandle);
        BM3200PRINTF(BM3200DEBUG, ("MBIST DONE \n\n"));
        if (sts_ul)
            return sts_ul;
    }

    if (!COMPILER_64_IS_ZERO(xbSerializerMask_ull)) {

#define HW_BM3200_XCFG_NORMAL 0
#define HW_BM3200_XCFG_A      1 /* test */
#define HW_BM3200_XCFG_B      2 /* test */
#define HW_BM3200_XCFG_A_OR_B 3 /* Normal LCM mode */

        xcfgMode_ul = HW_BM3200_XCFG_NORMAL;

        if (!COMPILER_64_IS_ZERO(hwBm3200Init_sp->lcmSerializerMask_ull)) {

            BM3200PRINTF(BM3200DEBUG, ("Setting XCFG\n\n"));
            sts_ul = hwBm3200SetXcfg(userDeviceHandle,
                                     hwBm3200Init_sp->lcmXcfg_ul,
                                     hwBm3200Init_sp->lcmPlaneValid_ul,
                                     &xcfgMode_ul,
                                     hwBm3200Init_sp->lcmXcfgABInputPolarityReversed_ul);
            if (sts_ul)
                return sts_ul;
        }

        BM3200PRINTF(BM3200DEBUG, ("Setting Crossbar\n\n"));
        sts_ul = hwBm3200SetupCrossBar(userDeviceHandle,
                                       xbSerializerMask_ull,
                                       xcfgMode_ul);
        if (sts_ul)
            return sts_ul;
    }

    if (hwBm3200Init_sp->bmSerializerMask_ul != 0) {

        BM3200PRINTF(BM3200DEBUG, ("Setting up AM\n\n"));
        sts_ul = hwBm3200SetupAm(userDeviceHandle,
                                 hwBm3200Init_sp->bmLocalBmId_ul,
                                 hwBm3200Init_sp->bmDefaultBmId_ul,
                                 hwBm3200Init_sp->bmSerializerMask_ul,
				 hwBm3200Init_sp->bmEnableAutoFailover,
				 hwBm3200Init_sp->bmEnableAutoLinkDisable,
				 hwBm3200Init_sp->bmMaxFailedLinks,
				 hwBm3200Init_sp->spMode);
        if (sts_ul)
            return sts_ul;

        /*
         * Use highest node number + 1 as the number of nodes.
         */
        nNodes = 0;
        for (i = 0; i < HW_BM3200_PT_NUM_NODES; i++) {

            if (hwBm3200Init_sp->bmSerializerMask_ul & (1 << i)) {
                nNodes = i+1;
            }

        }

        BM3200PRINTF(BM3200DEBUG, ("Setting up BW, epochLength in timeslots (0x%x)\n\n", hwBm3200Init_sp->bmEpochLength_ul));
        sts_ul = hwBm3200SetupBw(userDeviceHandle,
                                 hwBm3200Init_sp->bmCmode_ul,
                                 nNodes,
                                 hwBm3200Init_sp->bmEpochLength_ul,
                                 hwBm3200Init_sp->bmModeLatency_ul);
        if (sts_ul)
            return sts_ul;
    }

    BM3200PRINTF(BM3200DEBUG, ("Setting up IF\n\n"));
    COMPILER_64_SET(ullTmp, COMPILER_64_HI(xbSerializerMask_ull), (COMPILER_64_LO(xbSerializerMask_ull) | hwBm3200Init_sp->bmSerializerMask_ul));
    sts_ul = hwBm3200SetupIf(userDeviceHandle,
                             ullTmp,
                              xbSerializerMask_ull,
                              hwBm3200Init_sp->siLsThreshold_ul,
                              hwBm3200Init_sp->siLsWindow_ul);
    if (sts_ul)
        return sts_ul;


    BM3200PRINTF(BM3200DEBUG, ("Enabling WRED\n\n"));
    sts_ul = hwBm3200EnableWred(userDeviceHandle);

    BM3200PRINTF(BM3200DEBUG, ("Complete.\n\n"));
    return HW_BM3200_STATUS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwBm3200MaskAllInterrupts()
 *
 * OVERVIEW:        Mask all interrupts.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Disable all interrupts.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/

static void hwBm3200MaskAllInterrupts(sbhandle userDeviceHandle)
{
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_PT_ERROR0_MASK, ~0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_PT_ERROR1_MASK, ~0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_PT_ERROR2_MASK, ~0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_PT_ERROR3_MASK, ~0);
}
/*****************************************************************************
 * FUNCTION NAME:   hwBm3200InReset()
 *
 * OVERVIEW:        Put the BM3200 in soft reset.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Soft reset the BM3200 device.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/

static void hwBm3200InReset(sbhandle userDeviceHandle)
{
    SAND_HAL_RMW_FIELD(userDeviceHandle, PT, PI_RESET, RESET, 0x1);

    thin_delay(HW_BM3200_100_MSEC_K);
}
/*****************************************************************************
 * FUNCTION NAME:   hwBm3200UnReset()
 *
 * OVERVIEW:        Take the BM3200 out of soft reset.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Take the BM3200 device out of soft reset.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static void hwBm3200UnReset(sbhandle userDeviceHandle)
{
    SAND_HAL_RMW_FIELD(userDeviceHandle, PT, PI_RESET, RESET, 0x0);

    thin_delay(HW_BM3200_100_MSEC_K);
}

static uint32 hwBm3200CalcTsSize(uint32 timeSlotSize_ul)
{
    uint32 ts_size = 0;
    uint32 ts_in_ps = 0, clk_in_ps = 0;

    

    ts_in_ps = timeSlotSize_ul * 1000; /* in ps */
    /* Scale clock speed by 1000 */
    clk_in_ps = 1000000000 / (HW_BM3200_CLOCK_SPEED_IN_HZ /1000);
    ts_size = (ts_in_ps + clk_in_ps - 1) / clk_in_ps;
    return ts_size;
}

/*****************************************************************************
 * FUNCTION NAME:   hwBm3200SetupPi()
 *
 * OVERVIEW:        Set up the Pi.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Soft reset the BM3200 device.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32
hwBm3200SetupPi(sbhandle userDeviceHandle,
                uint64     xbSerializerMask_ul,
                uint32     bmSerializerMask_ul,
                uint32     cmode_ul)
{


    /* Set link modes. */
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_XB_LINK_MODE, COMPILER_64_LO(xbSerializerMask_ul));

    if (cmode_ul) {
        SAND_HAL_WRITE(userDeviceHandle, PT, PI_QE_LINK_MODE, 0);
    }
    else {
	/* joey ported from sdk */
        SAND_HAL_WRITE(userDeviceHandle, PT, PI_QE_LINK_MODE, bmSerializerMask_ul);
    }


    /* bug 22649, always enable all blocks so that no errors are indicated when dumping all registers */
    {
        SAND_HAL_WRITE(userDeviceHandle, PT, PI_CORE_RST_CONFIG0, 0);
        SAND_HAL_WRITE(userDeviceHandle, PT, PI_CORE_RST_CONFIG1, 0);

        SAND_HAL_WRITE(userDeviceHandle, PT, PI_CORE_CLOCK_CONFIG0,0);
        SAND_HAL_WRITE(userDeviceHandle, PT, PI_CORE_CLOCK_CONFIG1,0);
    }

    return HW_BM3200_STATUS_OK_K;
}
/*****************************************************************************
 * FUNCTION NAME:   hwBm3200RunMbist()
 *
 * OVERVIEW:        Run Built-In-Self-Test on memories
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:         status
 *
 * DESCRIPTION:     Enables bist for chip, waits for done, checks for errors,
 *                  returns status
 *
 * ASSUMPTIONS:     Called BEFORE chip brought out of reset
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32
hwBm3200RunMbist(sbhandle userDeviceHandle)
{
    uint32 j;
    uint32 reg;
    uint32 bw_mem_status[4];
    uint32 uAck;
    soc_timeout_t timeout;

    reg = SAND_HAL_READ_POLL(userDeviceHandle, PT, PI_MBIST_MASTER_CONFIG);
    if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, reg)) {
	BM3200PRINTF(BM3200DEBUG, ("MBist already done once\n\n"));
	return HW_BM3200_STATUS_OK_K;
    }

    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG0, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG1, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG2, 0xffffffff);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG3, 0x3fff);

    reg = 0;
    reg = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_ALGO_MODE, reg, 0x0);  /* all phases */
    reg = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_MODE, reg, 0x2);       /* run from PI */
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_MASTER_CONFIG, reg);
    reg = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_ENABLE, reg, 0x1);     /* start test */
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_MASTER_CONFIG, reg);

    /* check for MBist done */
    uAck = 0;
    for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        reg = SAND_HAL_READ_POLL(userDeviceHandle, PT, PI_MBIST_MASTER_CONFIG);
        uAck = SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, reg);
    }
    if (!uAck) {
	BM3200PRINTF(BM3200DEBUG, ("MBIST ack timeout\n"));
	return HW_BM3200_STATUS_INIT_BIST_TIMEOUT_K;
    }

    if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, reg)) {
        if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_GO, reg)) {
            BM3200PRINTF(BM3200DEBUG, ("MBIST done with no error. PI_MBIST_MASTER_CONFIG=0x%x\n\n", reg));
        } else {
            BM3200PRINTF(BM3200DEBUG, ("MBIST done with error. PI_MBIST_MASTER_CONFIG=0x%x\n\n", reg));
        }
    } else {
        BM3200PRINTF(BM3200DEBUG, ("Time out waiting for all MBIST done. PI_MBIST_MASTER_CONFIG=0x%x\n\n", reg));
        BM3200PRINTF(BM3200DEBUG, ("Summary: done119_96=0x%x, done95_64=0x%x, done63_32=0x%x, done31_00=0x%x\n\n",
                                   SAND_HAL_READ(userDeviceHandle, PT, PI_MBIST_DONE_STATUS3),
                                   SAND_HAL_READ(userDeviceHandle, PT, PI_MBIST_DONE_STATUS2),
                                   SAND_HAL_READ(userDeviceHandle, PT, PI_MBIST_DONE_STATUS1),
                                   SAND_HAL_READ(userDeviceHandle, PT, PI_MBIST_DONE_STATUS0)));
        return HW_BM3200_STATUS_INIT_BIST_TIMEOUT_K;
    }

    if (!SOC_IS_RELOADING((int32)userDeviceHandle)) {
	reg = SAND_HAL_READ(userDeviceHandle, PT, BW_MEM_STATUS);
	bw_mem_status[0] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK0_MEM0, reg);
	bw_mem_status[1] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK0_MEM1, reg);
	bw_mem_status[2] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK1_MEM0, reg);
	bw_mem_status[3] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK1_MEM1, reg);

	/* Check for BW mem bank errors */
	for (j = 0; j < 4; j++) {
	    if (bw_mem_status[j] == 0x3) {
		BM3200PRINTF(BM3200DEBUG, ("Bw Memory Bank%d, Mem%d has unrepairable error, BW_MEM_STATUS=0x%x\n\n",
					   bw_mem_status[j]/2, bw_mem_status[j]&1, reg));
		return HW_BM3200_STATUS_INIT_BM3200_BIST_BW_UNREPAIR_K;
	    }

	    if (bw_mem_status[j] == 0x0) {
		BM3200PRINTF(BM3200DEBUG, ("Bw Memory Bank%d, Mem%d did not complete MBIST, BW_MEM_STATUS=0x%x\n\n",
					   bw_mem_status[j]/2, bw_mem_status[j]&1, reg));
		return HW_BM3200_STATUS_INIT_BM3200_BIST_BW_TIMEOUT_K;
	    }

	    if (bw_mem_status[j] == 0x2) {
		BM3200PRINTF(BM3200DEBUG, ("WARNING: Bw Memory Bank%d, Mem%d was repaired. BW_MEM_STATUS=0x%x\n\n",
					   bw_mem_status[j]/2, bw_mem_status[j]&1, reg));
	    }
	}
    } /* end if SOC_IS_RELOADING */

    /* turn off MBist */
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG0, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG1, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG2, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_SLAVE_CONFIG3, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_MBIST_MASTER_CONFIG, 0);

    return HW_BM3200_STATUS_OK_K;
}

/*****************************************************************************
 * FUNCTION NAME:   hwBm3200SetXcfg()
 *
 * OVERVIEW:        Set up LCM for xcfg mode
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Set up LCM xcfg mode.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
#define HW_BM3200_PT_NO_MAPPING 0x3f    /* any value >= 40 */

static uint32
hwBm3200SetXcfg(sbhandle userDeviceHandle,
                uint32    lcmXcfg_ul[HW_BM3200_MAX_NUM_PLANES][HW_BM3200_PT_NUM_SERIALIZERS],
                uint32    planeValid[HW_BM3200_MAX_NUM_PLANES],
                uint32    *xcfgMode_ulp,
                uint32    lcmXcfgABInputPolarityReversed_ul)
{

    uint32 plane_ul;

    for (plane_ul = 0; plane_ul < HW_BM3200_MAX_NUM_PLANES; plane_ul++) {

        if (planeValid[plane_ul] == FALSE) {
            sal_memset(lcmXcfg_ul[plane_ul], HW_BM3200_PT_NO_MAPPING, sizeof lcmXcfg_ul[plane_ul]);
        }
    }

    if (planeValid[0] && planeValid[1]) {
        *xcfgMode_ulp = HW_BM3200_XCFG_A_OR_B; /* LCM mode with external pin driving output to A or B */
    } else if (planeValid[0]) {
        *xcfgMode_ulp = HW_BM3200_XCFG_A;
    } else if (planeValid[1]) {
        *xcfgMode_ulp = HW_BM3200_XCFG_B;
    }

    /* Bug 23532, if xcfg_switch_a_b_n polarity is reversed from the QE, need to reverse the data plane association with A/B */
    if (lcmXcfgABInputPolarityReversed_ul == FALSE) {
        hwBm3200SetXcfgA(userDeviceHandle, lcmXcfg_ul[0]);
        hwBm3200SetXcfgB(userDeviceHandle, lcmXcfg_ul[1]);
    }
    else {
        hwBm3200SetXcfgA(userDeviceHandle, lcmXcfg_ul[1]);
        hwBm3200SetXcfgB(userDeviceHandle, lcmXcfg_ul[0]);
    }

    return HW_BM3200_STATUS_OK_K;
}

static void
hwBm3200SetXcfgA(sbhandle userDeviceHandle,
                 uint32 xcfg[HW_BM3200_PT_NUM_SERIALIZERS])
{
#if 0
    BM3200PRINTF(BM3200DEBUG, ("set_xcfg_a: %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d\n\n",
                               xcfg[0],xcfg[1],xcfg[2],xcfg[3],xcfg[4],xcfg[5],xcfg[6],xcfg[7],xcfg[8],xcfg[9],
                               xcfg[10],xcfg[11],xcfg[12],xcfg[13],xcfg[14],xcfg[15],xcfg[16],xcfg[17],xcfg[18],xcfg[19],
                               xcfg[20],xcfg[21],xcfg[22],xcfg[23],xcfg[24],xcfg[25],xcfg[26],xcfg[27],xcfg[28],xcfg[29],
                               xcfg[30],xcfg[31],xcfg[32],xcfg[33],xcfg[34],xcfg[35],xcfg[36],xcfg[37],xcfg[38],xcfg[39]));
#endif
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A0,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A0, XCFG0, xcfg[0]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A0, XCFG1, xcfg[1]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A0, XCFG2, xcfg[2]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A0, XCFG3, xcfg[3]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A0, XCFG4, xcfg[4]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A1,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A1, XCFG5, xcfg[5]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A1, XCFG6, xcfg[6]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A1, XCFG7, xcfg[7]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A1, XCFG8, xcfg[8]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A1, XCFG9, xcfg[9]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A2,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A2, XCFG10, xcfg[10]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A2, XCFG11, xcfg[11]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A2, XCFG12, xcfg[12]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A2, XCFG13, xcfg[13]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A2, XCFG14, xcfg[14]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A3,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A3, XCFG15, xcfg[15]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A3, XCFG16, xcfg[16]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A3, XCFG17, xcfg[17]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A3, XCFG18, xcfg[18]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A3, XCFG19, xcfg[19]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A4,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A4, XCFG20, xcfg[20]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A4, XCFG21, xcfg[21]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A4, XCFG22, xcfg[22]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A4, XCFG23, xcfg[23]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A4, XCFG24, xcfg[24]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A5,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A5, XCFG25, xcfg[25]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A5, XCFG26, xcfg[26]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A5, XCFG27, xcfg[27]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A5, XCFG28, xcfg[28]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A5, XCFG29, xcfg[29]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A6,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A6, XCFG30, xcfg[30]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A6, XCFG31, xcfg[31]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A6, XCFG32, xcfg[32]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A6, XCFG33, xcfg[33]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A6, XCFG34, xcfg[34]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_A7,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A7, XCFG35, xcfg[35]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A7, XCFG36, xcfg[36]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A7, XCFG37, xcfg[37]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A7, XCFG38, xcfg[38]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_A7, XCFG39, xcfg[39]));
}

static void
hwBm3200SetXcfgB(sbhandle userDeviceHandle,
                 uint32 xcfg[HW_BM3200_PT_NUM_SERIALIZERS])
{
#if 0
    BM3200PRINTF(BM3200DEBUG, ("set_xcfg_b: %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d\n\n",
                               xcfg[0],xcfg[1],xcfg[2],xcfg[3],xcfg[4],xcfg[5],xcfg[6],xcfg[7],xcfg[8],xcfg[9],
                               xcfg[10],xcfg[11],xcfg[12],xcfg[13],xcfg[14],xcfg[15],xcfg[16],xcfg[17],xcfg[18],xcfg[19],
                               xcfg[20],xcfg[21],xcfg[22],xcfg[23],xcfg[24],xcfg[25],xcfg[26],xcfg[27],xcfg[28],xcfg[29],
                               xcfg[30],xcfg[31],xcfg[32],xcfg[33],xcfg[34],xcfg[35],xcfg[36],xcfg[37],xcfg[38],xcfg[39]));
#endif

    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B0,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B0, XCFG0, xcfg[0]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B0, XCFG1, xcfg[1]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B0, XCFG2, xcfg[2]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B0, XCFG3, xcfg[3]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B0, XCFG4, xcfg[4]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B1,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B1, XCFG5, xcfg[5]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B1, XCFG6, xcfg[6]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B1, XCFG7, xcfg[7]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B1, XCFG8, xcfg[8]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B1, XCFG9, xcfg[9]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B2,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B2, XCFG10, xcfg[10]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B2, XCFG11, xcfg[11]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B2, XCFG12, xcfg[12]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B2, XCFG13, xcfg[13]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B2, XCFG14, xcfg[14]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B3,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B3, XCFG15, xcfg[15]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B3, XCFG16, xcfg[16]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B3, XCFG17, xcfg[17]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B3, XCFG18, xcfg[18]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B3, XCFG19, xcfg[19]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B4,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B4, XCFG20, xcfg[20]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B4, XCFG21, xcfg[21]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B4, XCFG22, xcfg[22]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B4, XCFG23, xcfg[23]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B4, XCFG24, xcfg[24]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B5,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B5, XCFG25, xcfg[25]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B5, XCFG26, xcfg[26]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B5, XCFG27, xcfg[27]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B5, XCFG28, xcfg[28]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B5, XCFG29, xcfg[29]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B6,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B6, XCFG30, xcfg[30]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B6, XCFG31, xcfg[31]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B6, XCFG32, xcfg[32]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B6, XCFG33, xcfg[33]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B6, XCFG34, xcfg[34]));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_FIXED_XCFG_B7,
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B7, XCFG35, xcfg[35]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B7, XCFG36, xcfg[36]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B7, XCFG37, xcfg[37]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B7, XCFG38, xcfg[38]) |
             SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_B7, XCFG39, xcfg[39]));
}

/*****************************************************************************
 * FUNCTION NAME:   hwBm3200SetupCrossbar()
 *
 * OVERVIEW:        Bring up crossbars.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Bring up the logical crossbars.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32
hwBm3200SetupCrossBar(sbhandle userDeviceHandle,
                      uint64 xbSerializerMask_ull,
                      uint32 xcfgMode_ul)
{
    uint32 reg;
    uint32 i;
    sbBool_t no_deskew =
        xcfgMode_ul == HW_BM3200_XCFG_NORMAL ? 0 : 1;
    int32 status_ul;
    int32 saved_status_ul = HW_BM3200_STATUS_OK_K;

    reg = SAND_HAL_READ(userDeviceHandle, PT, XB_CONFIG0);
    reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, SOT_POLICING_EN, reg, 1);
    reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, NO_DESKEW, reg, no_deskew);
    reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, XCFG_MODE, reg, xcfgMode_ul);
    reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, TS_JITTER_TOL, reg, 16);
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_CONFIG0, reg);

    for (i = 0; i < HW_BM3200_PT_NUM_SERIALIZERS; i++) {
	status_ul = hwBm3200EnableSerializer(userDeviceHandle, i, (sbBool_t)(COMPILER_64_BITTEST(xbSerializerMask_ull,i)));
	if (status_ul) {
	    BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) Error enabling serializer(%d)",(uint32)(userDeviceHandle), i));
	    saved_status_ul = status_ul;
	}
    }

    SAND_HAL_WRITE(userDeviceHandle, PT, XB_ERROR0_MASK, ~COMPILER_64_LO(xbSerializerMask_ull));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_ERROR1_MASK, ~COMPILER_64_LO(xbSerializerMask_ull));
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_ERROR2_MASK, ~COMPILER_64_LO(xbSerializerMask_ull));

    reg = COMPILER_64_HI(xbSerializerMask_ull);
    reg = reg | (reg << 8) | (reg << 16) | (reg << 24);
    SAND_HAL_WRITE(userDeviceHandle, PT, XB_ERROR3_MASK, ~reg);

    return saved_status_ul;
}
static int32
hwBm3200EnableSerializer(sbhandle userDeviceHandle,
                         uint32     serializerId,
                         sbBool_t     enable)
{
    uint32 reg;
    int32 status_ul = HW_BM3200_STATUS_OK_K;

    if (serializerId < 32)
        reg = SAND_HAL_READ(userDeviceHandle, PT, XB_CONFIG1);
    else
        reg = SAND_HAL_READ(userDeviceHandle, PT, XB_CONFIG0);
    switch (serializerId) {
    case  0: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN0,  reg, enable); break;
    case  1: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN1,  reg, enable); break;
    case  2: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN2,  reg, enable); break;
    case  3: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN3,  reg, enable); break;
    case  4: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN4,  reg, enable); break;
    case  5: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN5,  reg, enable); break;
    case  6: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN6,  reg, enable); break;
    case  7: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN7,  reg, enable); break;
    case  8: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN8,  reg, enable); break;
    case  9: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN9,  reg, enable); break;
    case 10: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN10, reg, enable); break;
    case 11: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN11, reg, enable); break;
    case 12: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN12, reg, enable); break;
    case 13: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN13, reg, enable); break;
    case 14: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN14, reg, enable); break;
    case 15: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN15, reg, enable); break;
    case 16: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN16, reg, enable); break;
    case 17: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN17, reg, enable); break;
    case 18: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN18, reg, enable); break;
    case 19: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN19, reg, enable); break;
    case 20: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN20, reg, enable); break;
    case 21: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN21, reg, enable); break;
    case 22: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN22, reg, enable); break;
    case 23: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN23, reg, enable); break;
    case 24: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN24, reg, enable); break;
    case 25: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN25, reg, enable); break;
    case 26: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN26, reg, enable); break;
    case 27: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN27, reg, enable); break;
    case 28: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN28, reg, enable); break;
    case 29: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN29, reg, enable); break;
    case 30: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN30, reg, enable); break;
    case 31: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG1, IPRT_EN31, reg, enable); break;
    case 32: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN32, reg, enable); break;
    case 33: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN33, reg, enable); break;
    case 34: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN34, reg, enable); break;
    case 35: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN35, reg, enable); break;
    case 36: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN36, reg, enable); break;
    case 37: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN37, reg, enable); break;
    case 38: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN38, reg, enable); break;
    case 39: reg = SAND_HAL_MOD_FIELD(PT, XB_CONFIG0, IPRT_EN39, reg, enable); break;
    default:
      return HW_BM3200_STATUS_INIT_PARAM_ERROR_K;
    }
    if (serializerId < 32)
        SAND_HAL_WRITE(userDeviceHandle, PT, XB_CONFIG1, reg);
    else
        SAND_HAL_WRITE(userDeviceHandle, PT, XB_CONFIG0, reg);

    return status_ul;
}

/*
 * Starting values for the arbiter's psuedo-random number generator.
 * It really doesn't matter what these are, as long as they are truly random.
 * This data generated by radioactive decay http://www.fourmilab.ch/hotbits
 */
#define M(x) ((x) & 0x3f)       /* mask to 6 bits */
static uint32 random_array[HW_BM3200_PT_RANDOM_ARRAY_SIZE] = {
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

uint32
hwBm3200NmRandomReadWrite(sbhandle userDeviceHandle,
                          uint32     addr,
                          uint32*    pData0,
                          uint32     rd_wr_n)
{
    uint32 sts_ul, reg_ul;
    soc_timeout_t timeout;
    uint32 uAck;

    sts_ul = HW_BM3200_STATUS_OK_K;

    reg_ul = SAND_HAL_SET_FIELD(PT, NM_RAND_ACC_CTRL, ACK, 1);
    SAND_HAL_WRITE(userDeviceHandle, PT, NM_RAND_ACC_CTRL, reg_ul);

    reg_ul =  (SAND_HAL_SET_FIELD(PT, NM_RAND_ACC_CTRL, ACK, 1) |
               SAND_HAL_SET_FIELD(PT, NM_RAND_ACC_CTRL, REQ, 1) |
               SAND_HAL_SET_FIELD(PT, NM_RAND_ACC_CTRL, RD_WR_N, rd_wr_n) |
               SAND_HAL_SET_FIELD(PT, NM_RAND_ACC_CTRL, ADDR, addr));

    SAND_HAL_WRITE(userDeviceHandle, PT, NM_RAND_ACC_CTRL, reg_ul);

    /*
     * Check for completion
     */
    uAck = 0;
    for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        reg_ul = SAND_HAL_READ_POLL(userDeviceHandle, PT, NM_RAND_ACC_CTRL);
        uAck = SAND_HAL_GET_FIELD(PT, NM_RAND_ACC_CTRL, ACK, reg_ul);
    }
    if (!uAck) {
	BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) NM memory timeout\n", (int32)userDeviceHandle));
	sts_ul =  HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
    }

    if (rd_wr_n) {
        *pData0 = SAND_HAL_READ(userDeviceHandle, PT, NM_RAND_ACC_DATA);
    }

    return sts_ul;
}

/*****************************************************************************
 * FUNCTION NAME:   hwBm3200SetupAm()
 *
 * OVERVIEW:        Set up the AM block.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Set up the AM block.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32
hwBm3200SetupAm(sbhandle userDeviceHandle,
                uint32     amLocalBmId_ul,
                uint32     amDefaultBmId_ul,
                uint32     bmSerializerMask_ul,
		uint32     bmEnableAutoFailover,
		uint32     bmEnableAutoLinkDisable,
		uint32     bmMaxFailedLinks,
                uint32     spMode)
{
    uint32 sts_ul;
    uint32 addr, reg;
    uint32 port;
    uint32 ts_size;
    uint32 ac_config0;

    ts_size = hwBm3200CalcTsSize(HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS);

    /* clear NM memories */
    for (addr = 0; addr < HW_BM3200_PT_EMAP_MEM_SIZE; addr++) {
        static uint32 z = 0;

        sts_ul = hwBm3200NmEmapReadWrite(userDeviceHandle,
                                         addr,
                                         &z, &z, &z, &z, &z,
                                         0 /* WRITE */);
        if (sts_ul)
            return sts_ul;
    }

    SAND_HAL_WRITE(userDeviceHandle, PT, NM_CONFIG,
             SAND_HAL_SET_FIELD(PT, NM_CONFIG, ENABLE_RANDOM,    1) |
             SAND_HAL_SET_FIELD(PT, NM_CONFIG, ENABLE_INGRESS,   1) |
             SAND_HAL_SET_FIELD(PT, NM_CONFIG, ENABLE_EGRESS,    1) |
             SAND_HAL_SET_FIELD(PT, NM_CONFIG, ENABLE_MULTICAST, 1));

    reg = SAND_HAL_READ(userDeviceHandle, PT, AC_CONFIG2);
    reg = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET, reg, ts_size - 0xf); /* CCC */
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG2, reg);

    /*
     * Note, link_en is set to zero and must be configured to a valid number for
     * traffic to be forwarded over the given serializer - this is done by the fabric interface library
     * as various additional configs are required (default_bm_id, etc... at the time sandFab_ConfigFabric()
     *
     */
    SAND_HAL_WRITE(userDeviceHandle, PT, FO_CONFIG0,
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, LINK_EN, 0) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, DEFAULT_BM_ID, amDefaultBmId_ul) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, LOCAL_BM_ID, amLocalBmId_ul) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_LINK_DIS, bmEnableAutoLinkDisable) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, ENABLE_MORE_LINK_DIS, bmEnableAutoLinkDisable) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_SWITCHOVER, bmEnableAutoFailover) |
             SAND_HAL_SET_FIELD(PT, FO_CONFIG0, MAX_DIS_LINKS, bmMaxFailedLinks));

    reg = SAND_HAL_READ(userDeviceHandle, PT, AC_CONFIG1);

    /* disable arbiter */
    ac_config0 = SAND_HAL_READ(userDeviceHandle, PT, AC_CONFIG0);
    ac_config0 = SAND_HAL_MOD_FIELD(PT, AC_CONFIG0, ENABLE, ac_config0, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG0, ac_config0);

    /* update timeslot size */
    reg = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, TIMESLOT_SIZE, reg, ts_size);
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG1, reg);

    /* update grant offset to be within the timeslot */
    reg = SAND_HAL_READ(userDeviceHandle, PT, AC_CONFIG2);
    reg = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET, reg, ts_size - 0xf);
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG2, reg);

    /* enable arbiter */
    ac_config0 = SAND_HAL_MOD_FIELD(PT, AC_CONFIG0, ENABLE, ac_config0, 1);
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG0, ac_config0);

    /* enable of INA's will be done by the application (via bcm_stk_module_enable) */
    SAND_HAL_WRITE(userDeviceHandle, PT, FO_CONFIG1,
             SAND_HAL_SET_FIELD(PT, FO_CONFIG1, INA_ENABLE, 0));


    /* Set random number data */
    for (addr = 0; addr < HW_BM3200_PT_RANDOM_ARRAY_SIZE; addr++) {
        sts_ul = hwBm3200NmRandomReadWrite(userDeviceHandle,
                                           addr,
                                           &random_array[addr],
                                           FALSE);
        if (sts_ul)
            return sts_ul;
    }
#define HW_BM3200_PT_NUM_AI_PORTS    SB_FAB_DEVICE_BM3200_NUM_AI_PORTS

    for (port = 0; port < HW_BM3200_PT_NUM_AI_PORTS; port++) {

        /* clear INA memories */
#define HW_BM3200_PT_PRI_MEM_SIZE 70

        for (addr = 0; addr < HW_BM3200_PT_PRI_MEM_SIZE; addr++) {
            static uint32 z = 0;
            sts_ul = hwBm3200InaMemoryReadWrite(userDeviceHandle, port, 0, addr,
                                                &z, &z, &z, &z, &z, 0 /* WRITE */);
            if (sts_ul)
                return sts_ul;

            sts_ul = hwBm3200InaMemoryReadWrite(userDeviceHandle, port, 1, addr,
                                                &z, &z, &z, &z, &z, 0 /* WRITE */);
            if (sts_ul)
                return sts_ul;
        }

        if ( (bmSerializerMask_ul & (1 << port)) ) {
            /* this node needs to be enabled */
            reg = PT_PORT_READ(port, IF0_AI_CONFIG);
            reg = SAND_HAL_MOD_FIELD(PT, IF0_AI_CONFIG, PORT_ENABLE, reg, 1);
            reg = SAND_HAL_MOD_FIELD(PT, IF0_AI_CONFIG, RX_BIP8_EVEN, reg, 0);
            reg = SAND_HAL_MOD_FIELD(PT, IF0_AI_CONFIG, TX_BIP8_EVEN, reg, 0);
            PT_PORT_WRITE(port, IF0_AI_CONFIG, reg);
        }
    }

   /* Bug 23462 - Change ina_pri_full_thresh for pri 15,14,13 from 3 to 2 */
    for (port = 0; port < HW_BM3200_PT_NUM_AI_PORTS; port++) {

        int32 nPriFullThresh;

        nPriFullThresh = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_PRI_FULL_THRESH);

        nPriFullThresh = SAND_HAL_MOD_FIELD(PT, INA0_PRI_FULL_THRESH, PRI15_FULL_THRESH, nPriFullThresh, 0x2);
        nPriFullThresh = SAND_HAL_MOD_FIELD(PT, INA0_PRI_FULL_THRESH, PRI14_FULL_THRESH, nPriFullThresh, 0x2);
        if (spMode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            nPriFullThresh = SAND_HAL_MOD_FIELD(PT, INA0_PRI_FULL_THRESH, PRI13_FULL_THRESH, nPriFullThresh, 0x1);
        }
        else {
            nPriFullThresh = SAND_HAL_MOD_FIELD(PT, INA0_PRI_FULL_THRESH, PRI13_FULL_THRESH, nPriFullThresh, 0x2);
        }
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_PRI_FULL_THRESH, nPriFullThresh);

    }

    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG0,
             SAND_HAL_SET_FIELD(PT, AC_CONFIG0, ENABLE, 1) |
             SAND_HAL_SET_FIELD(PT, AC_CONFIG0, FORCE_NULL_GRANT, 0) |
             SAND_HAL_SET_FIELD(PT, AC_CONFIG0, NULL_CYCLE_COUNT, HW_BM3200_NULL_CYCLE_COUNT));

    return HW_BM3200_STATUS_OK_K;
}



uint32
hwBm3200InaMemoryReadWrite(sbhandle userDeviceHandle,
                           uint32     port,
                           uint32     sel,
                           uint32     addr,
                           uint32*    pData0,
                           uint32*    pData1,
                           uint32*    pData2,
                           uint32*    pData3,
                           uint32*    pData4,
                           uint32    rd_wr_n)
{
    uint32 reg_ul, sts_ul;
    soc_timeout_t timeout;
    uint32 uAck;

    if (!rd_wr_n) {

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA0, *pData0);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA1, *pData1);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA2, *pData2);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA3, *pData3);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA4, *pData4);
    }


    reg_ul =  SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, ACK, 1);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_CTRL, reg_ul);

    reg_ul =  (SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, SELECT,    sel) |
               SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, ACK,         0) |
               SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, REQ,         1) |
               SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, RD_WR_N, rd_wr_n) |
               SAND_HAL_SET_FIELD(PT, INA0_MEM_ACC_CTRL, ADDR,     addr));

    SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_CTRL, reg_ul);

    /*
     * Check for completion, timeout in 3 seconds
     */
    uAck = 0;
    for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        reg_ul = SAND_HAL_READ_STRIDE_POLL(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_CTRL);
        uAck = SAND_HAL_GET_FIELD(PT, INA0_MEM_ACC_CTRL, ACK, reg_ul);
    }
    if (!uAck) {
	BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) INA memory timeout\n", (int32)userDeviceHandle));
	sts_ul =  HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
	return sts_ul;
    }


    if (rd_wr_n) {
        *pData0 = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA0);
        *pData1 = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA1);
        *pData2 = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA2);
        *pData3 = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA3);
        *pData4 = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, INA, port, INA0_MEM_ACC_DATA4);
    }

    return HW_BM3200_STATUS_OK_K;
}

uint32
hwBm3200NmEmapReadWrite(sbhandle userDeviceHandle,
                        uint32 addr,
                        uint32* pData0,
                        uint32* pData1,
                        uint32* pData2,
                        uint32* pData3,
                        uint32* pData4,
                        uint32  rd_wr_n)
{
    uint32 reg_ul, sts_ul = HW_BM3200_STATUS_OK_K;
    soc_timeout_t timeout;
    uint32 uAck;
    

    if (!rd_wr_n) {
        SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_DATA0, *pData0);
        SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_DATA1, *pData1);
        SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_DATA2, *pData2);
        SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_DATA3, *pData3);
        SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_DATA4, *pData4);
    }

    reg_ul = SAND_HAL_SET_FIELD(PT, NM_EMAP_ACC_CTRL, ACK, 1);
    SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_CTRL, reg_ul);

    reg_ul =  (
               SAND_HAL_SET_FIELD(PT, NM_EMAP_ACC_CTRL, ACK,         1) |
               SAND_HAL_SET_FIELD(PT, NM_EMAP_ACC_CTRL, REQ,         1) |
               SAND_HAL_SET_FIELD(PT, NM_EMAP_ACC_CTRL, RD_WR_N, rd_wr_n) |
               SAND_HAL_SET_FIELD(PT, NM_EMAP_ACC_CTRL, ADDR,     addr));

    SAND_HAL_WRITE(userDeviceHandle, PT, NM_EMAP_ACC_CTRL, reg_ul);

    /*
     * Timeout for completion
     */
    uAck = 0;
    for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        reg_ul = SAND_HAL_READ_POLL(userDeviceHandle, PT, NM_EMAP_ACC_CTRL);
        uAck = SAND_HAL_GET_FIELD(PT, NM_EMAP_ACC_CTRL, ACK, reg_ul);
    }
    if (!uAck) {
	BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) NM memory timeout\n", (int32)userDeviceHandle));
	sts_ul =  HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
	return sts_ul;
    }

    if (rd_wr_n) {
        *pData0 = SAND_HAL_READ(userDeviceHandle, PT, NM_EMAP_ACC_DATA0);
        *pData1 = SAND_HAL_READ(userDeviceHandle, PT, NM_EMAP_ACC_DATA1);
        *pData2 = SAND_HAL_READ(userDeviceHandle, PT, NM_EMAP_ACC_DATA2);
        *pData3 = SAND_HAL_READ(userDeviceHandle, PT, NM_EMAP_ACC_DATA3);
        *pData4 = SAND_HAL_READ(userDeviceHandle, PT, NM_EMAP_ACC_DATA4);
    }

    return sts_ul;
}

static uint32
hwBm3200EnableWred(sbhandle userDeviceHandle)
{
    uint32 uData;

    /* enable Global WRED functionality */
    uData = SAND_HAL_READ(userDeviceHandle, PT, BW_EPOCH_CONFIG);
    uData = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, WRED_ENABLE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, WRED_MSG_ENABLE, uData, 1);
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_EPOCH_CONFIG, uData);

    return(HW_BM3200_STATUS_OK_K);
}


/*****************************************************************************
 * FUNCTION NAME:   hwBm3200SetupBw()
 *
 * OVERVIEW:        Set up the BW block.
 *
 * ARGUMENTS:       userDeviceHandle - sbhandle for SAND_HAL_FUNCTIONS
 *
 *
 * RETURNS:
 *                  NONE
 *
 *
 * DESCRIPTION:     Set up the Bw block.
 *
 * ASSUMPTIONS:     NONE
 *
 * SIDE EFFECTS:    NONE
 *
 *****************************************************************************/
static uint32
hwBm3200SetupBw(sbhandle userDeviceHandle,
                sbBool_t cmode_ul,
                uint32 nNodes,
                uint32 epochLength_ul,
                uint32 bmModeLatency_ul)
{
    uint32 sts_ul;
    uint32 nVirtualPorts;
    uint32 nQueues;
    uint32 tsSize_ul;

    if (cmode_ul) {

        nVirtualPorts  = HW_BM3200_PT_MAX_CMODE_VIRTUAL_PORTS;
        nQueues = HW_BM3200_PT_MAX_CMODE_QUEUES;

    } else {

        nVirtualPorts  = HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS;
        nQueues = SOC_SBX_CFG((int)userDeviceHandle)->num_queues;

    }

    tsSize_ul = hwBm3200CalcTsSize(HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS);
    COMPILER_REFERENCE(tsSize_ul);

    BM3200PRINTF(BM3200DEBUG, ("cmode=%d modeLatency=%d nodes=%d nVirtualPorts=%d queues=%d\n\n",
                               cmode_ul, bmModeLatency_ul, nNodes, nVirtualPorts, nQueues));

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_MODE,
             SAND_HAL_SET_FIELD(PT, BW_MODE, C_MODE, cmode_ul) |
             SAND_HAL_SET_FIELD(PT, BW_MODE, LATENCY, bmModeLatency_ul));

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_INGRESSES, nNodes);

    /* Set these to the maximum available in each mode - subtract 1 for limit */
    if (cmode_ul) {
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_PORTS, HW_BM3200_PT_MAX_CMODE_VIRTUAL_PORTS - 1);
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_GROUPS, nQueues - 1);
    }
    else {
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_PORTS, HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS - 1);
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_GROUPS, nQueues - 1);
    }

#if 0
        sts_ul = sandFabBM3200WriteRepos(userDeviceHandle);

        if (sts_ul)
            return sts_ul;
#endif

    if (cmode_ul)
        sts_ul = hwBm3200SetupBwCmode(userDeviceHandle, nQueues, nVirtualPorts, nNodes, epochLength_ul);
    else
        sts_ul = hwBm3200SetupBwDmode(userDeviceHandle, nQueues, epochLength_ul);
    if (sts_ul)
        return sts_ul;

    /* joey ported from sdk */
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_EPOCH_CONFIG,
             SAND_HAL_SET_FIELD(PT, BW_EPOCH_CONFIG, BW_MSG_ENABLE,   1) |
             SAND_HAL_SET_FIELD(PT, BW_EPOCH_CONFIG, BW_ENABLE,       1) |
             SAND_HAL_SET_FIELD(PT, BW_EPOCH_CONFIG, WRED_MSG_ENABLE, 1) |
             SAND_HAL_SET_FIELD(PT, BW_EPOCH_CONFIG, WRED_ENABLE,     1) |
             SAND_HAL_SET_FIELD(PT, BW_EPOCH_CONFIG, NUM_TIMESLOTS, epochLength_ul));

    /* Enable bandwidth manager */
    sts_ul = hwBm3200EnableBwManager(userDeviceHandle, TRUE);
    if (sts_ul)
        return sts_ul;

  /*SAND_HAL_WRITE(userDeviceHandle, PT, BW_TAG_ERROR_MASK, ~(UINT)nBwPortEn);*/
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_ERROR_MASK, 0);

#if 0
    reg = SAND_HAL_READ(userDeviceHandle, PT, PI_PT_ERROR0_MASK);
    reg = SAND_HAL_MOD_FIELD(PT, PI_PT_ERROR0_MASK, BW_INT, reg, 0);
    SAND_HAL_WRITE(userDeviceHandle, PT, PI_PT_ERROR0_MASK, reg);
#endif

    return HW_BM3200_STATUS_OK_K;
}

/* bw repository 0 only */
#define HW_BM3200_BANDWIDTH_TABLE_WDT   0
#define HW_BM3200_BANDWIDTH_TABLE_QLOP  1
#define HW_BM3200_BANDWIDTH_TABLE_LTHR  2
#define HW_BM3200_BANDWIDTH_TABLE_NPC2Q 3
#define HW_BM3200_BANDWIDTH_TABLE_Q2NPC 4
#define HW_BM3200_BANDWIDTH_TABLE_BWP   5
#define HW_BM3200_BANDWIDTH_TABLE_R0    6

/* bw repository 0 and 1 */
#define HW_BM3200_BANDWIDTH_TABLE_WAT   7
#define HW_BM3200_BANDWIDTH_TABLE_DST   8
#define HW_BM3200_BANDWIDTH_TABLE_PRT   9

/* bw repository 1 only */
#define HW_BM3200_BANDWIDTH_TABLE_WST  10
#define HW_BM3200_BANDWIDTH_TABLE_WCT  11
#define HW_BM3200_BANDWIDTH_TABLE_QLT  12
#define HW_BM3200_BANDWIDTH_TABLE_R1   13

#define HW_BM3200_BANDWIDTH_TABLE_NUM_REPOSITORIES 14
#define HW_BM3200_BANDWIDTH_TABLE_MAX_REPOSITORY_SIZE 0x8000
#define HW_BM3200_BANDWIDTH_TABLE_NUM_BANKS   2

/* Repository sizes. */
/* CMode */
#define HW_BM3200_BANDWIDTH_TABLE_DST_CMODE_SIZE  0x4000
#define HW_BM3200_BANDWIDTH_TABLE_WAT_CMODE_SIZE   0x400
#define HW_BM3200_BANDWIDTH_TABLE_PRT_CMODE_SIZE   0x280
#define HW_BM3200_BANDWIDTH_TABLE_BWP_CMODE_SIZE   0x400
#define HW_BM3200_BANDWIDTH_TABLE_Q2NPC_CMODE_SIZE 0x200
#define HW_BM3200_BANDWIDTH_TABLE_WDT_CMODE_SIZE   0x200
#define HW_BM3200_BANDWIDTH_TABLE_QLOP_CMODE_SIZE  0x280
#define HW_BM3200_BANDWIDTH_TABLE_LTHR_CMODE_SIZE  0x200
#define HW_BM3200_BANDWIDTH_TABLE_NPC2Q_CMODE_SIZE 0x400

#define HW_BM3200_BANDWIDTH_TABLE_WST_CMODE_SIZE   0x400
#define HW_BM3200_BANDWIDTH_TABLE_QLT_CMODE_SIZE   0x400

static uint32
hwBm3200QltInit(sbhandle userDeviceHandle)
{
    uint32 data_ul;
    uint32 addr_ul;
    int32  bank_l = 1; /* QLT is in bank 1 */
    uint32 sts_ul;

    for (addr_ul=0; addr_ul<HW_BM3200_BANDWIDTH_TABLE_QLT_CMODE_SIZE; addr_ul++) {

        /* Initialize all entries to indicate queue is empty */
        data_ul = 0xffffffff;

        sts_ul = hwBwRepositoryReadWrite(userDeviceHandle, bank_l, HW_BM3200_BANDWIDTH_TABLE_QLT,
                                         addr_ul, &data_ul, 0 /* write */);
        if (sts_ul) {
            BM3200PRINTF(BM3200DEBUG, ("QLT table init failed status(0x%x)\n\n", sts_ul));
            return sts_ul;
        }
    }
    return 0;
}

static uint32
hwBm3200Q2NpcInit(sbhandle userDeviceHandle)
{
    uint32 data_ul;
    uint32 addr_ul;
    int32  bank_l = 0; /* Q2NPC is in bank 0 */
    uint32 sts_ul;

    /* Entries are 16 bits but addressed in 32 bit entities so, 2 entries are obtained for
     * every address, so half the size
     */
    for (addr_ul=0; addr_ul<HW_BM3200_BANDWIDTH_TABLE_Q2NPC_CMODE_SIZE/2; addr_ul++) {

        /* Initialize all entries to invalid NPC is maxed out
         */
        data_ul = 0xffffffff;

        sts_ul = hwBwRepositoryReadWrite(userDeviceHandle, bank_l, HW_BM3200_BANDWIDTH_TABLE_Q2NPC,
                                         addr_ul, &data_ul, 0 /* write */);
        if (sts_ul) {
            BM3200PRINTF(BM3200DEBUG, ("Q2NPC table init failed status(0x%x)\n\n", sts_ul));
            return sts_ul;
        }
    }
    return 0;
}

static uint32
hwBm3200Npc2QInit(sbhandle userDeviceHandle)
{
    uint32 data_ul;
    uint32 addr_ul;
    int32  bank_l = 0; /* NPC2Q is in bank 0 */
    uint32 sts_ul;

    for (addr_ul=0; addr_ul<HW_BM3200_BANDWIDTH_TABLE_NPC2Q_CMODE_SIZE/2; addr_ul++) {

        /* Initialize all entries to invalid queue */
        data_ul = 0xffffffff;

        sts_ul = hwBwRepositoryReadWrite(userDeviceHandle, bank_l, HW_BM3200_BANDWIDTH_TABLE_NPC2Q,
                                         addr_ul, &data_ul, 0 /* write */);
        if (sts_ul) {
            BM3200PRINTF(BM3200DEBUG, ("NPC2Q table init failed status(0x%x)\n\n", sts_ul));
            return sts_ul;
        }
    }
    return 0;
}

#define HW_BM3200_WCT_TEMPLATES 256
#define HW_BM3200_WCT_ENTRIES_PER_TEMPLATE 6 /* 3 curves of 64 bits (2 words) each */

static uint32
hwBm3200WctInit(sbhandle userDeviceHandle)
{
    int32  nBank = 1; /* WCT is in bank 1 */

    /* Initialize all templates to a value which turns WRED off */
    {

        int32  nTemplate;

        for (nTemplate=0; nTemplate<HW_BM3200_WCT_TEMPLATES; nTemplate++) {
            uint32 uAddr = 0;
            uint32 uStartAddr =  nTemplate * (HW_BM3200_WCT_ENTRIES_PER_TEMPLATE + 2);

            /* Add 2 to start address because entries 7 and 8 are skipped because of memory interface */
            for (uAddr = uStartAddr;
                 uAddr < uStartAddr + HW_BM3200_WCT_ENTRIES_PER_TEMPLATE;
                 uAddr++) {

                uint32 uData = 0xffffffff;
                uint32 uStatus;

                uStatus = hwBwRepositoryReadWrite(userDeviceHandle, nBank, HW_BM3200_BANDWIDTH_TABLE_WCT,
                                                  uAddr, &uData, 0 /* write */);
                if (uStatus) {
                    BM3200PRINTF(BM3200DEBUG, ("WCT table init failed status(0x%x) address(0x%x)\n\n", uStatus, uAddr));
                    return uStatus;
                }
            }
        }
    }
    return 0;
}

static uint32
hwBwRepositoryReadWrite(sbhandle userDeviceHandle,
                        int32  bank_l,
                        int32  tableId_ul,
                        uint32 addr_ul,
                        uint32 *data_p,
                        uint32 rd_wr_ul)
{
    uint32 reg_ul;
    uint32 sts_ul = HW_BM3200_STATUS_OK_K;
    uint32 uAck;
    soc_timeout_t timeout;    
    int32 nRegBwError;

    if (bank_l == 0) {

        if (rd_wr_ul) {
            SAND_HAL_WRITE(userDeviceHandle, PT, BW_R0_ACC_DAT, *data_p);
        }

        reg_ul = SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, ACK, 1);
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_R0_ACC_CTRL, reg_ul);

        reg_ul = (SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, ACK, 1)            |
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, REQ, 1)              |
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, TBL_ID, tableId_ul)  |
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, OFS, addr_ul)        |
                SAND_HAL_SET_FIELD(PT, BW_R0_ACC_CTRL, RD_WR_N, rd_wr_ul));

        SAND_HAL_WRITE(userDeviceHandle, PT, BW_R0_ACC_CTRL, reg_ul);

	uAck = 0;
	for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    reg_ul = SAND_HAL_READ_POLL(userDeviceHandle, PT, BW_R0_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PT, BW_R0_ACC_CTRL, ACK, reg_ul);
	}
	if (!uAck) {
	    BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) BW R0 memory timeout\n", (int32)userDeviceHandle));

	    reg_ul = SAND_HAL_READ(userDeviceHandle, PT, BW_ERROR);
	    nRegBwError = SAND_HAL_GET_FIELD(PT, BW_ERROR, R0_ACC_ERROR, reg_ul);
		
	    if (nRegBwError != 0) {
		BM3200PRINTF(BM3200DEBUG, ("Error indicated after writing to BW memory bank(%d) table(%d) addr(%d) bw_error(0x%08x)\n",
					   bank_l, tableId_ul, addr_ul, nRegBwError));
		sts_ul = HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
		
		/* Clear out memory error after reporting */
		SAND_HAL_WRITE(userDeviceHandle, PT, BW_ERROR, 3);
	    }

	    sts_ul =  HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
	    return sts_ul;
	}

        if (rd_wr_ul) {
            *data_p = SAND_HAL_READ(userDeviceHandle, PT, BW_R0_ACC_DAT);
        }

    } else {

        if (!rd_wr_ul) {
            SAND_HAL_WRITE(userDeviceHandle, PT, BW_R1_ACC_DAT, *data_p);
        }
        reg_ul = (SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, ACK, 1)            |
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, REQ, 1)              |
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, TBL_ID, tableId_ul)  |
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, OFS, addr_ul)        |
                SAND_HAL_SET_FIELD(PT, BW_R1_ACC_CTRL, RD_WR_N, rd_wr_ul));

        SAND_HAL_WRITE(userDeviceHandle, PT, BW_R1_ACC_CTRL, reg_ul);

	uAck = 0;
	for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    reg_ul = SAND_HAL_READ_POLL(userDeviceHandle, PT, BW_R1_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PT, BW_R1_ACC_CTRL, ACK, reg_ul);
	}
	if (!uAck) {
	    BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) BW R1 memory timeout\n", (int32)userDeviceHandle));

	    reg_ul = SAND_HAL_READ(userDeviceHandle, PT, BW_ERROR);
	    nRegBwError = SAND_HAL_GET_FIELD(PT, BW_ERROR, R1_ACC_ERROR, reg_ul);
		
	    if (nRegBwError != 0) {
		BM3200PRINTF(BM3200DEBUG, ("Error indicated after writing to BW memory bank(%d) table(%d) addr(%d) bw_error(0x%08x)\n",
					   bank_l, tableId_ul, addr_ul, nRegBwError));
		sts_ul = HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
		
		/* Clear out memory error after reporting */
		SAND_HAL_WRITE(userDeviceHandle, PT, BW_ERROR, 2);
	    }

	    sts_ul =  HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K;
	    return sts_ul;
	}
        if (rd_wr_ul) {
            *data_p = SAND_HAL_READ(userDeviceHandle, PT, BW_R1_ACC_DAT);
        }
    }

    return(sts_ul);

}
#define QMON_KILL 8
#define PRIME_PIPE                    SB_FAB_DEVICE_BM3200_EPOCH_PRIME_PIPE
#define WRED_KILL                     SB_FAB_DEVICE_BM3200_EPOCH_WRED_KILL
#define SETUP 14
#define DMND_KILL 0
#define ALLOC_PRIME_PIPE              SB_FAB_DEVICE_BM3200_EPOCH_ALLOC_PRIME_PIPE
#define ALLOC_KILL 0

#define CLOCKS_TO_SENDMSGS 32 /* number of core clocks to send all messages */
#define SENDMSG_SETUP_CLKS 40 /* number of core clocks to do all reads before sending messages */

#define SHIFT_SETUP 20
#define SHIFT 40


static uint32
hwBm3200SetupBwCmode(sbhandle userDeviceHandle,
                     uint32 nQueues,
                     uint32 nVirtualPorts,
                     uint32 nNodes,
                     uint32 epochLength_ul)
{

    uint32 reg;
    uint32 clock_period_ns;
    uint32 ts_size = hwBm3200CalcTsSize(HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS);
    uint32 qmon_deadline, wred_deadline, dmnd_deadline, alloc_deadline,
        dmnd_cycles;
    uint32 grant_to_critical_delay, base_rx_sot, rx_sot;
    uint32 sendmsg_offset;
    uint32 status;

    /* Clear out QLT length thresholds */
    status = hwBm3200QltInit(userDeviceHandle);

    if (status != HW_BM3200_STATUS_OK_K) {
        return status;
    }

    /* Invalidate Q2NPC */
    status = hwBm3200Q2NpcInit(userDeviceHandle);

    if (status != HW_BM3200_STATUS_OK_K) {
        return status;
    }

    /* Invalidate NPC2Q */
    status = hwBm3200Npc2QInit(userDeviceHandle);

    if (status != HW_BM3200_STATUS_OK_K) {
        return status;
    }

    /* Invalidate WCT */
    status = hwBm3200WctInit(userDeviceHandle);

    if (status != HW_BM3200_STATUS_OK_K) {
        return status;
    }

    /* XXX This is bad, clock_period_ns period may not be an integer */
    clock_period_ns = (1000000000 / HW_BM3200_CLOCK_SPEED_IN_HZ); /* in clocks */

    /*
     * In CMode we receive one queue update per timeslot, we must
     * remain in monitor mode for as long as it takes to receive an
     * update from each queue. At the deadline event, QMON will wind
     * down, if it has not completed within 8 timeslots of its
     * deadline, it will be squelched...
     */
    qmon_deadline = nQueues;
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_QMON, qmon_deadline);
    qmon_deadline = qmon_deadline + QMON_KILL;

    /*
     * WRED processes 2 queues per timeslot, it also requires three
     * timeslots to prime the pipeline and three to drain it... If
     * WRED has not completed by its deadline, it will wind down,
     * if it has not completed within 8 timeslots of its deadline
     * it will be squelched...
     */
    wred_deadline = qmon_deadline + PRIME_PIPE + (nQueues+1) / 2;
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_WRED, wred_deadline);
    wred_deadline = wred_deadline + WRED_KILL;

    /*
     * Demand estimation runs at better than 2 queues per timeslot.
     * estimation must have completed by its deadline or it will be
     * squelched.
     * fns: updated this due to C-Mode estim_p1 errors
     */
    dmnd_cycles = SETUP + nQueues/2;
    dmnd_deadline = wred_deadline + dmnd_cycles;

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_DEMAND, dmnd_deadline);
    dmnd_deadline = dmnd_deadline + DMND_KILL;

    /*
     * allocation runs at better than 2 nQueues per timeslot.
     * allocation must have completed by its deadline or it will be
     * squelched. allocation requires 16 timeslots to prime its
     * pipeline and 16 to wind down...
     */
    alloc_deadline = dmnd_deadline + ALLOC_PRIME_PIPE + (nQueues+1)/2;
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_ALLOC, alloc_deadline);
    alloc_deadline = alloc_deadline + ALLOC_KILL;

    /*
     * set to the C-mode rate of 10G and 20G - in bits/10*1024 ns
     */
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE0, 0x19000);
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE1, 0x32000);

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE,   0);

    /*
     * find the valid window for RxSot configuration.  The earliest
     * possible value is just after the distrib has send messages
     * for this timeslot.  The latest possible value is determined
     * by backing up from the earliest rcv'd QLEN time by the amount
     * of time it takes to shift in all the previous QLEN messages plus
     * the fixed amount of time (found by checking waves to be 11 clocks)
     * that signals (SOTs) are delayed when going from block to block.
     * Also, back off by 3 clocks to allow for random jitter in the QeModel
     */
    grant_to_critical_delay = 272; /* CCC 276? XXX may need to distinguish qe1k and qe2k */
                                   /*              for D mode, we need to re-write */


    grant_to_critical_delay += 11 * clock_period_ns; /* latency of grant out of the BW (ref from ac_bw_sot) */
    grant_to_critical_delay +=  9 * clock_period_ns; /* latency of qlen into the BW */
    grant_to_critical_delay -=  3 * clock_period_ns; /* jitter early */

    base_rx_sot = grant_to_critical_delay / clock_period_ns;

    rx_sot = base_rx_sot - SHIFT_SETUP - SHIFT;

    sendmsg_offset = rx_sot + SENDMSG_SETUP_CLKS;

    if (sendmsg_offset > ts_size) {
        /* we're in the next timeslot.  first get the offset */
        sendmsg_offset = sendmsg_offset % ts_size;
        if (sendmsg_offset < 32) /* CCC */
            sendmsg_offset = 32; /* send after TsHeader/Crit/Req */
    }

    if (sendmsg_offset >= (ts_size - CLOCKS_TO_SENDMSGS)) {
        /* we can't send in this window, send early in the next timeslot */
        sendmsg_offset = 32; /* send after TsHeader/Crit/Req */
    }

    reg = SAND_HAL_READ(userDeviceHandle, PT, AC_CONFIG2);
    reg = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_BW_MSGS_OFFSET, reg, sendmsg_offset);
    SAND_HAL_WRITE(userDeviceHandle, PT, AC_CONFIG2, reg);

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_OFFSET, SAND_HAL_SET_FIELD(PT, BW_OFFSET, RXSOT, rx_sot));

    if (epochLength_ul >= alloc_deadline + 1) {
        BM3200PRINTF(BM3200DEBUG, ("epochLength_ul=%d alloc deadline=%d\n\n",
                                   epochLength_ul, alloc_deadline+1));
        return HW_BM3200_STATUS_OK_K;
    }
    else {
        BM3200PRINTF(BM3200DEBUG, ("Epoch length too small: epochLength_ul=%d alloc deadline=%d\n\n",
                                   epochLength_ul, alloc_deadline+1));

        return HW_BM3200_STATUS_INIT_EPOCH_LENGTH_INVALID_K;
    }
}

static uint32
hwBm3200SetupBwDmode(sbhandle userDeviceHandle,
                     uint32 nQueues,
                     uint32 epochLength_ul)
{
    uint32 wred_deadline, dmnd_deadline, alloc_deadline;
    uint32 dmode_rxsot, dmode_cmd, ts_size;
    int32 status;

    /* Invalidate WCT */
    status = hwBm3200WctInit(userDeviceHandle);

    if (status != HW_BM3200_STATUS_OK_K) {
        return status;
    }

    ts_size = hwBm3200CalcTsSize(HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS);

    /* In DMode we only enable the WRED and ALLOC phases of allocation: */

    /*
     * WRED processes 2 queues per timeslot, it also requires three
     * timeslots to prime the pipeline and three to drain it... If
     * WRED has not completed by its deadline, it will wind down,
     * if it has not completed within 8 timeslots of its deadline
     * it will be squelched...
     */
    wred_deadline = PRIME_PIPE + (nQueues+1) / 2;
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_WRED, wred_deadline);
    wred_deadline = wred_deadline + WRED_KILL;

    /*
     * demand estimation is performed by the QEs in DMode. This
     * deadline merely determines when the BME expects that the
     * QEs will have completed their task. We set it greater than
     * the wred_deadline (arbitrarily).
     */
    dmnd_deadline = wred_deadline + 2;
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_DEMAND, dmnd_deadline);

    {
        int32 nCosPerPort = (HW_BM3200_PT_MAX_DMODE_QUEUES + 1)/nQueues;

        if (nCosPerPort == 0) {
            nCosPerPort = 1;
        }

        nCosPerPort = 1;

        /*
         * allocation runs at better than 2 queues per timeslot.
         * allocation must have completed by its deadline or it will be
         * squelched. allocation requires 16 timeslots to prime its
         * pipeline and 16 to wind down...
         */
        alloc_deadline = dmnd_deadline + ALLOC_PRIME_PIPE + (nQueues * nCosPerPort) / 2;
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_ALLOC, alloc_deadline);
    }

    dmode_rxsot = ts_size - 8 + 1; /* CCC */
    dmode_cmd = ts_size - 35;   /* CCC */

    SAND_HAL_WRITE(userDeviceHandle, PT, BW_OFFSET,
             SAND_HAL_SET_FIELD(PT, BW_OFFSET, CMND, dmode_cmd) |
             SAND_HAL_SET_FIELD(PT, BW_OFFSET, RXSOT, dmode_rxsot));

#if 0
    /* fns: confirmed with Nick Not used in D-Mode */
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE0, 0x7fffff);
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE1, 0x7fffff);
    SAND_HAL_WRITE(userDeviceHandle, PT, BW_NODE,   0);
#endif

    if (epochLength_ul >= alloc_deadline + 1) {
        BM3200PRINTF(BM3200DEBUG, ("epochLength_ul=%d alloc deadline=%d\n\n",
                                   epochLength_ul, alloc_deadline+1));
        return HW_BM3200_STATUS_OK_K;
    }
    else {
        BM3200PRINTF(BM3200DEBUG, ("Epoch length too small: epochLength_ul=%d alloc deadline=%d\n\n",
                                   epochLength_ul, alloc_deadline+1));

        return HW_BM3200_STATUS_INIT_EPOCH_LENGTH_INVALID_K;
    }

}

static uint32
hwBm3200EnableBwManager(sbhandle userDeviceHandle, sbBool_t enable)
{
    uint32 rv = HW_BM3200_STATUS_OK_K;
    int32 nRegister;
    uint32 uAck;
    soc_timeout_t timeout;

    if (enable) {

        nRegister = SAND_HAL_READ(userDeviceHandle, PT, BW_EPOCH_CONFIG);

        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, BW_MSG_ENABLE, nRegister, 1);
        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, BW_ENABLE, nRegister, 1);

        SAND_HAL_WRITE(userDeviceHandle, PT, BW_EPOCH_CONFIG, nRegister);
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, TAG_CHECK_DIS, 0) |
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_EN, 1));

	thin_delay(HW_BM3200_100_MSEC_K);

      if (!SOC_IS_RELOADING((int32)userDeviceHandle)) {
	uAck = 0;
	for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    nRegister = SAND_HAL_READ(userDeviceHandle, PT, BW_EVENT);
	    uAck = SAND_HAL_GET_FIELD(PT, BW_EVENT, SCHED_ACT, nRegister);	    
	}
	if (!uAck) {
	    BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) BW scheduler active timeout\n", (int32)userDeviceHandle));
	    rv =   HW_BM3200_STATUS_INIT_SCHED_ACTIVE_TIMEOUT_K;
	    return rv;
	}
      }

	SAND_HAL_WRITE(userDeviceHandle, PT, BW_EVENT, ~0);

	if (!SOC_IS_RELOADING((int32)userDeviceHandle)) {
	    /* Delay 20ms */
	    thin_delay(2 * HW_BM3200_10_MSEC_K);
	    nRegister = SAND_HAL_READ(userDeviceHandle, PT, BW_ERROR);
	    if (nRegister != 0) {
		BM3200PRINTF(BM3200DEBUG, ("bw_error after enabling scheduler (0x%08x)\n",
					   nRegister));
		rv = HW_BM3200_STATUS_INIT_BM3200_BW_TIMEOUT_K;
	    }
	}

    } else { /* stand by */
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_DIS, 1));
        SAND_HAL_WRITE(userDeviceHandle, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, TAG_CHECK_DIS, 0) |
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_EN, 0));
        rv = HW_BM3200_STATUS_OK_K;
    }
    return rv;
}

static void
config_serdes(sbhandle userDeviceHandle, uint32 sd)
{

    PT_PORT_WRITE(sd, IF0_SD_DEBUG,
                  SAND_HAL_SET_FIELD(PT, IF0_SD_DEBUG, SD_RESET_DLY, 5) |
                  SAND_HAL_SET_FIELD(PT, IF0_SD_DEBUG, SD_RESETPLL_DLY, 5) |
                  SAND_HAL_SET_FIELD(PT, IF0_SD_DEBUG, SD_PWRDN_DLY, 5) |
                  SAND_HAL_SET_FIELD(PT, IF0_SD_DEBUG, PW, 2));
    PT_PORT_WRITE(sd, IF0_SD_CONFIG,
                  SAND_HAL_SET_FIELD(PT, IF0_SD_CONFIG, ENABLE, 1));
}

#define PT_SERDES_POWERUP_TIMEOUT 40000      /* usec */

static uint32
hwBm3200SetupIf(sbhandle userDeviceHandle,
                uint64 serializerMask_ull,
                uint64 xbSerializerMask_ull,
                uint32 siLsThreshold_ul,
                uint32 siLsWindow_ul)
{
    int32 nLink;
    uint32 sd, reg;
    uint32 sts_ul = HW_BM3200_STATUS_OK_K;
    uint64 ullTmp, ullSd;

    BM3200PRINTF(BM3200DEBUG, ("Serializers(0x%02x%08x) xbSerializers(0x%02x%08x) \n\n",COMPILER_64_HI(serializerMask_ull), COMPILER_64_LO(serializerMask_ull), COMPILER_64_HI(xbSerializerMask_ull), COMPILER_64_LO(xbSerializerMask_ull)));
    /*
     * Sequence the serdes bring up to avoid warning messages from the core.
     * first power up Serdes PLL for ports (0,1), (4,5), (8,9), ....
     * then  power up Serdes PLL for ports (2,3), (6,7), (10,11), ....
     * If necessary wait for power-up done of a neighbor serdes.
     */
    for (sd = 0; sd < HW_BM3200_PT_NUM_SERIALIZERS; sd += 4) { /* for ports (0,1), (4,5), (8,9), .... */
        ullTmp = serializerMask_ull;
        COMPILER_64_SET(ullSd, 0, 3);
        COMPILER_64_SHL(ullSd, sd);
        COMPILER_64_AND(ullTmp, ullSd);
        if (!COMPILER_64_IS_ZERO(ullTmp)) {
            BM3200PRINTF(BM3200DEBUG, ("line:%d Enabling serdes pair(0x%x, 0x%x )\n\n",__LINE__, 
                                       COMPILER_64_HI(ullSd), COMPILER_64_LO(ullSd)));
            config_serdes(userDeviceHandle, sd);
        }
    }

    for (sd = 2; sd < HW_BM3200_PT_NUM_SERIALIZERS; sd += 4) { /* for ports (2,3), (6,7), (10,11), .... */
        ullTmp = serializerMask_ull;
        COMPILER_64_SET(ullSd, 0, 3);
        COMPILER_64_SHL(ullSd, sd);
        COMPILER_64_AND(ullTmp, ullSd);
        if (!COMPILER_64_IS_ZERO(ullTmp)) {
            uint32 neighbor_sd = sd - 2;
            sbBool_t shared_pll_done;
	    uint32 uAck = 0;
	    soc_timeout_t timeout;
            shared_pll_done = !(COMPILER_64_BITTEST(serializerMask_ull, neighbor_sd) |
                                COMPILER_64_BITTEST(serializerMask_ull, neighbor_sd + 1));

	    if (!shared_pll_done) {
		for (soc_timeout_init(&timeout, HW_BM3200_TIMEOUT_GENERAL, HW_BM3200_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
		    reg =  PT_PORT_READ(neighbor_sd, IF0_SD_STATUS);
		    uAck = SAND_HAL_GET_FIELD(PT, IF0_SD_STATUS, PWRUP_DONE, reg);
		}
		
		if (!uAck) {
		    BM3200PRINTF(BM3200DEBUG, ("BM3200(%d) if(%d) serdes powerup timeout\n", (int32)userDeviceHandle, neighbor_sd));
		    sts_ul =   HW_BM3200_STATUS_INIT_BM3200_SER_TIMEOUT_K;
		    return sts_ul;
		}
	    }

            config_serdes(userDeviceHandle, sd);
        }
    }

    for (nLink = 0; nLink < HW_BM3200_PT_NUM_SERIALIZERS; nLink++) {

        {
            int32 nRegister;

            /* Force tx serdes low.  This must be enabled by the device handler */
            nRegister = SAND_HAL_READ_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_DEBUG1);
            nRegister |= SAND_HAL_SET_FIELD(PT, IF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, 1);

            SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_DEBUG1, nRegister);
        }


        {
            SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_CONFIG3,
                                  SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG3, DTX, 0x8) |
                                  SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG3, DEQ, 0xf) |
                                  SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG3, LODRV, 0x1));
        }

	/* joey ported from sdk */
        {
            SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_CONFIG3,
                                  SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG3, DEQ, 0xa));
        }

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_CONFIG0,
                              SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG0, JIT_TOLERANCE, 0x14) |
                              SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG0, ENABLE, 1));
        BM3200PRINTF(BM3200DEBUG, ("line:%d wrote to PT if%d_si_config0 (0x%x)\n\n", __LINE__, nLink,
                                   SAND_HAL_READ_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_CONFIG0)));

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_DEBUG2,
                              SAND_HAL_SET_FIELD(PT, IF0_SI_DEBUG2, TX_FIFO_UNDERRUN_CHK_EN, 0x1) |
                              SAND_HAL_SET_FIELD(PT, IF0_SI_DEBUG2, TX_FIFO_AFULL_THRESH, 0x20));

        SAND_HAL_WRITE_STRIDE(userDeviceHandle, PT, IF, nLink, IF0_SI_CONFIG2,
                              SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG2, LS_ERR_THRESH, siLsThreshold_ul) |
                              SAND_HAL_SET_FIELD(PT, IF0_SI_CONFIG2, LS_ERROR_WINDOW, siLsWindow_ul));

    }

    return HW_BM3200_STATUS_OK_K;
}

