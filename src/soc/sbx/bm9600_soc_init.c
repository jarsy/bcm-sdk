/*
 * $Id: bm9600_soc_init.c,v 1.100 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 SOC Initialization implementation
 */

#include <shared/bsl.h>
#include <assert.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>

#ifdef BCM_BM9600_SUPPORT

/* MCM - soc_lcm_mode_t defined for 3200 */
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/sbx_util.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/bm9600.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/fabric.h>

#include <soc/sbx/link.h>
#include "sbZfFabBm9600NmSysportArrayEntry.hx"
#include "sbZfFabBm9600BwR0BwpEntry.hx"
#include "sbZfFabBm9600BwR0BagEntry.hx"
#include "sbZfFabBm9600BwR1BagEntry.hx"
#include "sbZfFabBm9600NmSysportArrayEntry.hx"
#include "sbZfFabBm9600InaSysportMapEntry.hx"
#include "sbZfFabBm9600NmPortsetLinkEntry.hx"
#include "sbZfFabBm9600NmPortsetInfoEntry.hx"

/*
 * local declarations
 */
void soc_bm9600_unit9_error(int unit, uint32 ignored);
static int soc_bm9600_arbiter_capable(int unit, uint8 *bArbiterCapable);


#define BM9600_READ_REQUEST(unit, reg, otherAddr) \
  SOC_SBX_UTIL_READ_REQUEST(unit, PL, reg, otherAddr)

#define BM9600_WRITE_REQUEST(unit, reg, otherAddr) \
 SOC_SBX_UTIL_WRITE_REQUEST(unit, PL, reg, otherAddr)

#define BM9600_UTIL_WAIT_FOR_ACK(unit, reg, nTimeoutInMs, nStatus) \
 SOC_SBX_UTIL_WAIT_FOR_ACK(unit, PL, reg, nTimeoutInMs, nStatus)

extern uint32
hwBm9600NmEmapReadWrite(sbhandle userDeviceHandle,
            uint32 addr,
            uint32* pData0,
            uint32* pData1,
            uint32* pData2,
            uint32* pData3,
            uint32* pData4,
            uint32  rd_wr_n);

static soc_block_info_t soc_blocks_bcm83332[] = {
    /* block type,    number, schan, cmic */
    { SOC_BLK_GXPORT,   0,    -1,    -1 },   /* sfi & sci */
    {-1,               -1,    -1,    -1 } /* end */
};

static soc_port_info_t soc_port_info_bcm88130[] = {
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
    { 0,   40  },   /*  40 SFI.40 */
    { 0,   41  },   /*  41 SFI.41 */
    { 0,   42  },   /*  42 SFI.42 */
    { 0,   43  },   /*  43 SFI.43 */
    { 0,   44  },   /*  44 SFI.44 */
    { 0,   45  },   /*  45 SFI.45 */
    { 0,   46  },   /*  46 SFI.46 */
    { 0,   47  },   /*  47 SFI.47 */
    { 0,   48  },   /*  48 SFI.48 */
    { 0,   49  },   /*  49 SFI.49 */
    { 0,   50  },   /*  50 SFI.50 */
    { 0,   51  },   /*  51 SFI.51 */
    { 0,   52  },   /*  52 SFI.52 */
    { 0,   53  },   /*  53 SFI.53 */
    { 0,   54  },   /*  54 SFI.54 */
    { 0,   55  },   /*  55 SFI.55 */
    { 0,   56  },   /*  56 SFI.56 */
    { 0,   57  },   /*  57 SFI.57 */
    { 0,   58  },   /*  58 SFI.58 */
    { 0,   59  },   /*  59 SFI.59 */
    { 0,   60  },   /*  60 SFI.60 */
    { 0,   61  },   /*  61 SFI.61 */
    { 0,   62  },   /*  62 SFI.62 */
    { 0,   63  },   /*  63 SFI.63 */
    { 0,   64  },   /*  64 SFI.64 */
    { 0,   65  },   /*  65 SFI.65 */
    { 0,   66  },   /*  66 SFI.66 */
    { 0,   67  },   /*  67 SFI.67 */
    { 0,   68  },   /*  68 SFI.68 */
    { 0,   69  },   /*  69 SFI.69 */
    { 0,   70  },   /*  70 SFI.70 */
    { 0,   71  },   /*  71 SFI.71 */
    { 0,   72  },   /*  72 SFI.72 */
    { 0,   73  },   /*  73 SFI.73 */
    { 0,   74  },   /*  74 SFI.74 */
    { 0,   75  },   /*  75 SFI.75 */
    { 0,   76  },   /*  76 SFI.76 */
    { 0,   77  },   /*  77 SFI.77 */
    { 0,   78  },   /*  78 SFI.78 */
    { 0,   79  },   /*  79 SFI.79 */
    { 0,   80  },   /*  80 SFI.80 */
    { 0,   81  },   /*  81 SFI.81 */
    { 0,   82  },   /*  82 SFI.82 */
    { 0,   83  },   /*  83 SFI.83 */
    { 0,   84  },   /*  84 SFI.84 */
    { 0,   85  },   /*  85 SFI.85 */
    { 0,   86  },   /*  86 SFI.86 */
    { 0,   87  },   /*  87 SFI.87 */
    { 0,   88  },   /*  88 SFI.88 */
    { 0,   89  },   /*  89 SFI.89 */
    { 0,   90  },   /*  90 SFI.90 */
    { 0,   91  },   /*  91 SFI.91 */
    { 0,   92  },   /*  92 SFI.92 */
    { 0,   93  },   /*  93 SFI.93 */
    { 0,   94  },   /*  94 SFI.94 */
    { 0,   95  },   /*  95 SFI.95 */
    { 0,   96  },   /*  96 SFI.96 */
    {-1,   -1  },   /*  end       */

};

soc_driver_t soc_driver_bm9600_a0 = {
       /* type         */      SOC_CHIP_BM9600_A0,
       /* chip_string  */      "bm9600",
       /* origin       */      "Unknown",
       /* pci_vendor   */      BROADCOM_VENDOR_ID,
       /* pci_device   */      BM9600_DEVICE_ID,
       /* pci_revision */      BM9600_A0_REV_ID,
       /* num_cos      */      0,
       /* reg_info     */      NULL,
       /* reg_unique_acc */    NULL,
       /* reg_above_64_info */ NULL,
       /* reg_array_info */    NULL,
       /* mem_info     */      NULL,
       /* mem_unique_acc */    NULL,
       /* mem_aggr     */      NULL,
       /* mem_array_info */    NULL,
       /* block_info   */      soc_blocks_bcm83332,
       /* port_info    */      soc_port_info_bcm88130,
       /* counter_maps */      NULL,
       /* features     */      soc_bm9600_features,
       /* init         */      NULL,
       /* services     */      NULL,
       /* port_num_blktype */  0,
       /* cmicd_base */        0
};  /* soc_driver */

soc_driver_t soc_driver_bm9600_b0 = {
    /* type         */  SOC_CHIP_BM9600_B0,
    /* chip_string  */  "bm9600",
    /* origin       */  "Unknown",
    /* pci_vendor   */  BROADCOM_VENDOR_ID,
    /* pci_device   */  BM9600_DEVICE_ID,
    /* pci_revision */  BM9600_B0_REV_ID,
    /* num_cos      */  0,
    /* reg_info     */  NULL,
    /* reg_unique_acc */ NULL,
        /* reg_above_64_info */ NULL,
        /* reg_array_info */    NULL,
    /* mem_info     */  NULL,
    /* mem_unique_acc */ NULL,
    /* mem_aggr     */  NULL,
        /* mem_array_info */    NULL,
    /* block_info   */  soc_blocks_bcm83332,
    /* port_info    */  soc_port_info_bcm88130,
    /* counter_maps */  NULL,
    /* features     */  soc_bm9600_features,
    /* init         */  NULL,
    /* services     */      NULL,
    /* port_num_blktype */  0,
    /* cmicd_base */        0

};  /* soc_driver */

#define SB_FAB_DEVICE_BM9600_NUM_ESETS          (1024)
#define SB_FAB_DEVICE_BM9600_MAX_LOGICAL_PORTS  (4096)
#define SB_FAB_DEVICE_BM9600_MAX_VOQS          (16384)

#define printf  bsl_printf

typedef void (*ifn_t)(int unit, uint32 data);

typedef struct {
    uint32  mask;
    ifn_t   intr_fn;
    uint32  intr_data;
    char    *intr_name;
} intr_handler_t;

/* was this .. SAND_HAL_PL_PI_ERROR0_FO_INT_MASK, soc_bm9600_fo_error, 0, "SBPT_FO_ERROR"},*/
/* then was...  {SAND_HAL_PL_PI_INTERRUPT_MASK_UNIT_INTERRUPT_DISINT_MASK, soc_bm9600_fo_error, 0, "SBPT_FO_ERROR"},*/
STATIC intr_handler_t soc_bm9600_intr_handlers[] = {
    {(1<<9) /* fo is on sub-unit 9 */, soc_bm9600_unit9_error, 0, "SBPT_UNIT9_ERROR"},
};

#define BM9600_INTR_HANDLERS_COUNT  COUNTOF(soc_bm9600_intr_handlers)

void
soc_bm9600_config_all_linkdriver(int unit)
{

    /* This is somewhat redundant b/c the deq,dtx are set in the init routine
     * leaving for now in case we need to adjust the settings after initializtion.
     */
    int        nLink;

    for ( nLink=0; nLink<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; nLink++ ) {
        soc_bm9600_config_linkdriver(unit, nLink);
    }

}

/*
 * Get/set a single ESET entry
 */
int
soc_bm9600_eset_set(int nUnit, uint32 uEset, uint64 uLowNodesMask, uint32 uHiNodesMask, uint32 uMcFullEvalMin, uint32 uEsetFullStatusMode)
{
    int rc;


    rc = hwBm9600EsetSet(nUnit, uEset, uLowNodesMask, uHiNodesMask, uMcFullEvalMin, uEsetFullStatusMode);
    if (rc != SOC_E_NONE) {
        return(rc);
    }

    return(rc);
}


int
soc_bm9600_eset_get(int nUnit, uint32 uEset, uint64 *pLowNodesMask, uint32 *pHiNodesMask, uint32 *pMcFullEvalMin, uint32 *pEsetFullStatusMode)
{
    return  hwBm9600EsetGet(nUnit, uEset, pLowNodesMask, pHiNodesMask, pMcFullEvalMin, pEsetFullStatusMode);
}

/*
 *   Function
 *      soc_sbx_eset_init
 *   Purpose
 *      Initialize the ESET table in teh BME.
 *   Parameters
 *      (in) unit      - BCM unit number
 *
 *   Returns
 *     BCM_E_NONE if successful
 *     BCM_E_* appropriately if not
 */
int
soc_bm9600_sbx_eset_init(int unit)
{
    int rv;
    int eset;


#if 1
    for (eset = 0; eset < BM9600_MAX_MULTICAST_EXTENDED_ESETS; eset++) {
        rv = soc_bm9600_NmEmtWrite(unit, eset, 0x00, 0x00, 0x00);
        if( BCM_FAILURE(rv) ){
            return(rv);
        }
    }
#else /* 1 */
    rv = soc_bm9600_NmEmtClear(unit);
    if( BCM_FAILURE(rv) ){
        return(rv);
    }
#endif /* !1 */

    return(SOC_E_NONE);
}



void
soc_bm9600_unit9_error(int unit, uint32 ignored)
{

   /* Please read note at the top of soc_bm9600_isr */

    uint32 intrs, fo_intrs, uData;
    uint32 serviced = 0;
    bcm_fabric_control_redundancy_info_t red_info;

    COMPILER_REFERENCE(ignored);

    /* We only actually handle FO errors here, but we could also handle
     * XB, NM and BW errors if need be.
     */
    intrs = SAND_HAL_READ(unit, PL, PI_UNIT_INTERRUPT9);
    if (intrs & SAND_HAL_PL_PI_UNIT_INTERRUPT9_FO_MASK) {
        /* Always mask all interrupts if any are active
       In theory, we can mask only the ones that are set
       but we only expect one to be set at any given time
    */
        SAND_HAL_WRITE(unit, PL, FO_EVENT_MASK, 0xffffffff);
    fo_intrs = SAND_HAL_READ(unit, PL, FO_EVENT);

    if (SAND_HAL_GET_FIELD(PL, FO_EVENT, AUTO_FAILOVER, fo_intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Auto Failover Interrupt\n")));
    }
    if (SAND_HAL_GET_FIELD(PL, FO_EVENT, AUTO_LINK_DIS, fo_intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Link disable Interrupt\n")));
    }
    if (SAND_HAL_GET_FIELD(PL, FO_EVENT, AUTO_QE_DIS, fo_intrs)) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "Qe disable Interrupt\n")));
    }
    /* Call back returning the redundancy info */
    if (((ifn_t)SOC_SBX_STATE(unit)->fabric_state->red_f) != NULL) {
        uData = SAND_HAL_READ(unit, PL, FO_STATUS0);
        red_info.active_arbiter_id = SAND_HAL_GET_FIELD(PL, FO_STATUS0, ACTIVE_BM, uData);
        uData = SAND_HAL_READ(unit, PL, FO_STATUS1);
        COMPILER_64_SET(red_info.xbars, 0, SAND_HAL_GET_FIELD(PL, FO_STATUS1, GLOBAL_ENABLED_LINKS, uData));
        ((ifn_t)(SOC_SBX_STATE(unit)->fabric_state->red_f))(unit, (uint32)&red_info);
    }

    serviced = 1;
        COMPILER_REFERENCE(serviced);
    }
#if 0
    else if (intrs & SAND_HAL_PL_PI_UNIT_INTERRUPT9_BW_MASK) {
    /* If need be, add code here to dispatch a BW interrupt. */

    serviced = 1;
    }
    else if (intrs & SAND_HAL_PL_PI_UNIT_INTERRUPT9_NM_MASK) {
    /* If need be, add code here to dispatch an NM interrupt. */

    serviced = 1;
    }
    else if (intrs & SAND_HAL_PL_PI_UNIT_INTERRUPT9_XB_MASK) {
    /* If need be, add code here to dispatch an XB interrupt. */

    serviced = 1;
    }
#endif



    return;
}

void
soc_bm9600_isr(void *_unit)
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

    soc->stat.intr++;       /* Update count */

    for (i=0; i < BM9600_INTR_HANDLERS_COUNT; i++) {
      LOG_INFO(BSL_LS_SOC_INTR,
               (BSL_META("soc_bm9600_isr unit %d: dispatch %s\n"),
                unit, soc_bm9600_intr_handlers[i].intr_name));

      (*soc_bm9600_intr_handlers[i].intr_fn)
    (unit, soc_bm9600_intr_handlers[i].intr_data);
    }
}

int
soc_bm9600_config_linkdriver(int unit, int port)
{
    sbLinkDriverConfig_t *pLinkDriverConfig;
    sbLinkSpecificDriverConfig_t specificConfig;
    uint32     status;
    uint16     full_speed = 0;


    if ( port < 0 || port >= SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS ) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Bad port(%d) requested for BM9600. "
                             "Valid range is [0,%d]\n"), port, 
                  SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS));
        return SOC_E_CONFIG;
    }

    pLinkDriverConfig = &(SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port]);

    status = GetLinkSpecificConfig((sbFabUserDeviceHandle_t)unit, port, 
                                   pLinkDriverConfig, &specificConfig);
    if (status != SB_FAB_STATUS_OK) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Could not get link specific configuration "
                             "for port(%d)\n"), port));
        return SOC_E_UNAVAIL;
    }

    if  ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] == 
          SOC_PORT_ABILITY_SFI) ||

      ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] == 
        SOC_PORT_ABILITY_SCI) &&
       (SOC_SBX_CFG(unit)->uSerdesSpeed < 6250))
     )
      {

        /* We are running at 1/2 speed 3.125G */
    /* Override nPreemphasisPre */
    specificConfig.u.hypercore.equalization.nPreemphasisPre = 0;
        full_speed = 0;
    } else {
    /*
         * We are running at full speed 6.25G 
         * Set high bit in value to indicate to program TX0_TX_BR_DRIVER
         */
    full_speed = 0x8000;
    }
        

    /*
     * Now that we have the data, etc. program the correct link config and 
     * we're done 
     */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PREEMPHASIS,
         specificConfig.u.hypercore.equalization.nPremphasisPost|full_speed));
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_DRIVER_CURRENT,
         specificConfig.u.hypercore.strength.nIDriver|full_speed));
    SOC_IF_ERROR_RETURN
       (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRE_DRIVER_CURRENT,
         specificConfig.u.hypercore.strength.nIPreDriver|full_speed));
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRE_PREEMPHASIS,
         specificConfig.u.hypercore.equalization.nPreemphasisPre|full_speed));

    return SB_FAB_STATUS_OK;
}


int
soc_bm9600_lcm_mode_set(int unit, soc_lcm_mode_t mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        if (mode == lcmModeNormal)
        {
            SAND_HAL_RMW_FIELD(unit, PL, XB_CONFIG0, NO_DESKEW, 0);
        } else {
            SAND_HAL_RMW_FIELD(unit, PL, XB_CONFIG0, NO_DESKEW, 1);
        }

        SAND_HAL_RMW_FIELD(unit, PL, XB_CONFIG0, XCFG_MODE, mode);

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

int
soc_bm9600_lcm_mode_get(int unit, soc_lcm_mode_t *mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        uint32 data = SAND_HAL_READ(unit, PL, XB_CONFIG0);
        *mode = SAND_HAL_GET_FIELD(PL, XB_CONFIG0, XCFG_MODE, data);
        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}


static int
_soc_bm9600_port_ability_get(int unit, int xbport, int *ability_xbport)
{
    uint32 uData;
    int rv = BCM_E_NONE;


    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, xbport, SI0_CONFIG3);
    if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, CH_MODE, uData)) {
        /* This is a single-channel port. */
        if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT, uData)) {
            *ability_xbport = BCM_PORT_ABILITY_SFI;
        }
        else {
            *ability_xbport = BCM_PORT_ABILITY_SCI;
        }
    }
    else {
        /* This is a dual-channel port. */
        if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT, uData)) {
            *ability_xbport = BCM_PORT_ABILITY_DUAL_SFI;
        }
        else {
            *ability_xbport = BCM_PORT_ABILITY_SFI_SCI;
        }
    }

    return(rv);
}

int
soc_bm9600_lcm_fixed_config(int unit,
                            int src_modid,
                            int configAB,
                            int src_xbport,
                            int dst_xbport)
{
    int rv = BCM_E_NONE;
    uint32 remap_entry;
    int node, ability_src_xbport, ability_dst_xbport;
    int src_iprt=0, dst_iprt=0;
#if 000
    uint32 xb_config0;
#endif

    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
        return BCM_E_PARAM;
    }
    node = BCM_STK_MOD_TO_NODE(src_modid);

    /* curently not handling sirius device */
    switch (SOC_SBX_STATE(unit)->stack_state->protocol[node]) {
        case bcmModuleProtocol1:
            break;

        case bcmModuleProtocol2:
            break;

        case bcmModuleProtocol3:
        default:
            return(BCM_E_PARAM);
            break;
    }

    rv = _soc_bm9600_port_ability_get(unit, src_xbport, &ability_src_xbport);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bcm_bm9600_port_ability_get failed on xbport %d\n"), src_xbport));
        return(BCM_E_FAIL);
    }

    rv = _soc_bm9600_port_ability_get(unit, dst_xbport, &ability_dst_xbport);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bcm_bm9600_port_ability_get failed on xbport %d\n"), dst_xbport));
        return(BCM_E_FAIL);
    }

    /* currently not handling SCI ports via LCM */
    if ( (ability_src_xbport == BCM_PORT_ABILITY_SFI_SCI) ||
         (ability_src_xbport == BCM_PORT_ABILITY_SCI) ||
         (ability_dst_xbport == BCM_PORT_ABILITY_SFI_SCI) ||
         (ability_dst_xbport == BCM_PORT_ABILITY_SCI) ) {
        return(BCM_E_PARAM);
    }

    /* curremtly only handling data links */
    if ((ability_src_xbport == BCM_PORT_ABILITY_DUAL_SFI) &&
                                  (ability_dst_xbport == BCM_PORT_ABILITY_DUAL_SFI) ) {
        return(BCM_E_PARAM);
    }

    /* NOTE: Need to handle multiplexing of same QE device links */
    /*       This will be done in the future                     */

    if (ability_src_xbport == BCM_PORT_ABILITY_DUAL_SFI) {
        if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) {
            src_iprt = (src_xbport * 2);
        }
        else if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2) {
            src_iprt = (src_xbport * 2) + 1;
        }
        dst_iprt = (dst_xbport * 2);
    }
    else if (ability_dst_xbport == BCM_PORT_ABILITY_DUAL_SFI) {
        src_iprt = (src_xbport * 2);
        if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) {
            dst_iprt = (dst_xbport * 2);
        }
        else if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2) {
            dst_iprt = (dst_xbport * 2) + 1;
        }
    }
    else {
        src_iprt = (src_xbport * 2);
        dst_iprt = (dst_xbport * 2);
    }

    rv = soc_bm9600_XbXcfgRemapSelectRead(unit, 71, src_iprt, &remap_entry);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n"), src_xbport));
        return(BCM_E_FAIL);
    }

    if (configAB == 0) {
        remap_entry &= ~(0xFf);
        remap_entry |= (dst_iprt & 0xFF);
    }
    else {
        remap_entry &= ~(0xFf00);
        remap_entry |= ((dst_iprt << 8) & 0xFF00);
    }

    rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, 71, src_iprt, remap_entry);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d\n"), src_xbport));
        return(BCM_E_FAIL);
    }

#if 000
    /* reset xbar, for xcfg to take effect */
    xb_config0 = SAND_HAL_READ(unit, PL, XB_CONFIG0);
    xb_config0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOFT_RESET, xb_config0, 1);
    SAND_HAL_WRITE(unit, PL, XB_CONFIG0, xb_config0);

    /* delay */
    thin_delay(250);

    /* take out of reset */
    xb_config0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOFT_RESET, xb_config0, 0);
    SAND_HAL_WRITE(unit, PL, XB_CONFIG0, xb_config0);

    /* delay */
    thin_delay(10000);
#endif
    return(rv);
}

/* Confirm that mapping in table is correct */
int
soc_bm9600_lcm_fixed_config_validate(int unit,
                     int src_modid,
                     int configAB,
                     int src_xbport,
                     int dst_xbport)
{
    int rv = BCM_E_NONE;
    uint32 remap_entry;
    int node, ability_src_xbport, ability_dst_xbport;
    int src_iprt=0, dst_iprt=0;


    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
        return BCM_E_PARAM;
    }
    node = BCM_STK_MOD_TO_NODE(src_modid);

    /* curently not handling sirius device */
    switch (SOC_SBX_STATE(unit)->stack_state->protocol[node]) {
        case bcmModuleProtocol1:
            break;

        case bcmModuleProtocol2:
            break;

        case bcmModuleProtocol3:
        default:
            return(BCM_E_PARAM);
            break;
    }

    rv = _soc_bm9600_port_ability_get(unit, src_xbport, &ability_src_xbport);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bcm_bm9600_port_ability_get failed on xbport %d\n"), src_xbport));
        return(BCM_E_FAIL);
    }

    rv = _soc_bm9600_port_ability_get(unit, dst_xbport, &ability_dst_xbport);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bcm_bm9600_port_ability_get failed on xbport %d\n"), dst_xbport));
        return(BCM_E_FAIL);
    }

    /* currently not handling SCI ports via LCM */
    if ( (ability_src_xbport == BCM_PORT_ABILITY_SFI_SCI) ||
         (ability_src_xbport == BCM_PORT_ABILITY_SCI) ||
         (ability_dst_xbport == BCM_PORT_ABILITY_SFI_SCI) ||
         (ability_dst_xbport == BCM_PORT_ABILITY_SCI) ) {
        return(BCM_E_PARAM);
    }

    /* curremtly only handling data links */
    if ((ability_src_xbport == BCM_PORT_ABILITY_DUAL_SFI) &&
                                  (ability_dst_xbport == BCM_PORT_ABILITY_DUAL_SFI) ) {
        return(BCM_E_PARAM);
    }

    /* NOTE: Need to handle multiplexing of same QE device links */
    /*       This will be done in the future                     */

    if (ability_src_xbport == BCM_PORT_ABILITY_DUAL_SFI) {
        if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) {
            src_iprt = (src_xbport * 2);
        }
        else if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2) {
            src_iprt = (src_xbport * 2) + 1;
        }
        dst_iprt = (dst_xbport * 2);
    }
    else if (ability_dst_xbport == BCM_PORT_ABILITY_DUAL_SFI) {
        src_iprt = (src_xbport * 2);
        if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) {
            dst_iprt = (dst_xbport * 2);
        }
        else if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2) {
            dst_iprt = (dst_xbport * 2) + 1;
        }
    }
    else {
        src_iprt = (src_xbport * 2);
        dst_iprt = (dst_xbport * 2);
    }

    rv = soc_bm9600_XbXcfgRemapSelectRead(unit, 71, src_iprt, &remap_entry);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n"), src_xbport));
        return(BCM_E_FAIL);
    }

    if (configAB == 0) {
      if (dst_iprt != (remap_entry & 0xff)) {
    LOG_CLI((BSL_META_U(unit,
                        "WARNING: mapping failure - src_iprt(%d) dest_iprt read(%d) expected value(%d)\n"),
             src_iprt, (remap_entry&0xff), dst_iprt));
    rv = BCM_E_FAIL;
      }
    }
    else {
      if (dst_iprt != ((remap_entry & 0xff00) >> 8)) {
    LOG_CLI((BSL_META_U(unit,
                        "WARNING: mapping failure - src_iprt(%d) dest_iprt read(%d) expected value(%d)\n"),
             src_iprt, (remap_entry&0xff00) >> 8, dst_iprt));
    rv = BCM_E_FAIL;
      }
    }

    return(rv);
}
/***********************************************
 * read bwp entry for a queue
 */
int
soc_bm9600_bwp_read(int unit,
                     int queue,
                     uint *pgamma,
                     uint *psigma)
{
    int status = 0;
    uint32 addr;
    uint32 read_value;
    sbZfFabBm9600BwR0BwpEntry_t zfBwpEntry;

    addr = queue;

    status = soc_bm9600_BwR0BwpRead(unit, addr, &read_value);
    if (status) {
    return status;
    }

    sbZfFabBm9600BwR0BwpEntry_Unpack(&zfBwpEntry, (uint8*)&read_value, 4);

    *pgamma = zfBwpEntry.m_uGamma;
    *psigma = zfBwpEntry.m_uSigma;

    return status;
}

/***********************************************
 * write bwp entry for a queue
 */
int
soc_bm9600_bwp_write(int unit,
                     int queue,
                     uint gamma,
                     uint sigma)
{
    int status = 0;
    uint32 addr;
    uint32 write_value = 0;
    sbZfFabBm9600BwR0BwpEntry_t zfBwpEntry;

    addr = queue;

    zfBwpEntry.m_uGamma = gamma;
    zfBwpEntry.m_uSigma = sigma;

    sbZfFabBm9600BwR0BwpEntry_Pack(&zfBwpEntry, (uint8*)&write_value, 4);

    status = soc_bm9600_BwR0BwpWrite(unit, addr, write_value);
    if (status) {
    return status;
    }

    return status;
}

int
soc_bm9600_features(int unit, soc_feature_t feature)
{
    uint8 bArbiterCapable = FALSE;


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
            switch(SOC_SBX_CFG_BM9600(unit)->uDeviceMode) {
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

        case soc_feature_arbiter:
            switch(SOC_SBX_CFG_BM9600(unit)->uDeviceMode) {
                case SOC_SBX_BME_ARBITER_MODE:
                case SOC_SBX_BME_ARBITER_XBAR_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

    case soc_feature_arbiter_capable:
        soc_bm9600_arbiter_capable(unit, &bArbiterCapable);      
        return bArbiterCapable;

        break;

        case soc_feature_xbar:
            switch(SOC_SBX_CFG_BM9600(unit)->uDeviceMode) {
                case SOC_SBX_BME_XBAR_MODE:
                case SOC_SBX_BME_ARBITER_XBAR_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

        case soc_feature_lcm:
            switch(SOC_SBX_CFG_BM9600(unit)->uDeviceMode) {
                case SOC_SBX_BME_LCM_MODE:
                    return(TRUE);
                default:
                    return(FALSE);
            }
            break;

        case soc_feature_hybrid:
            return((SOC_SBX_CFG(unit)->bHybridMode == TRUE) ? TRUE : FALSE);
            break;

        case soc_feature_standalone:
            return((SOC_SBX_CFG(unit)->bTmeMode == TRUE) ? TRUE : FALSE);
            break;

        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
                case SOC_SBX_SYSTEM_CFG_VPORT: /* !bm3200 */
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* !bm3200 */
                case SOC_SBX_SYSTEM_CFG_VPORT_MIX: /* !bm3200 */
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
soc_xcfgremap_lb(int unit, int node, int port)
{
    int rv=0;

    rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, node, port*2, ((port*2+1)<<8)|(port*2));
    if (rv) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: failed write node %d, port%d\n"),FUNCTION_NAME(), node, port));
        return rv;
    }
    rv |= soc_bm9600_XbXcfgRemapSelectWrite(unit, node, port*2+1, ((port*2)<<8)|(port*2+1));
    if (rv) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: failed write node %d, port%d\n"),FUNCTION_NAME(), node, port));
        return rv;
    }
    return rv; 
}

int soc_xcfgremap_all_to_lb(int unit) 
{
     int rv, node, port;
     for (node=0;node<BM9600_NUM_NODES; node++){
         for (port=0; port <BM9600_NUM_LINKS; port++){
	     rv = soc_xcfgremap_lb(unit, node, port);
	     if (rv) {
                 LOG_CLI((BSL_META_U(unit,
                                     "%s: failed write node %d, port%d\n"),FUNCTION_NAME(),node, port));
	         return rv;
	     }
	 }
     }
     return 0;
}


/*
 * soc_bm9600_init - Fill in defaults for hardware settings, then pick up SOC params
 *                   from input structure/properties, fill in hardware init
 *                   structure with corresponding values, then call hwBm9600Init
 *   NOTE:   called from sbx_drv.c after that picks up some soc properties
 */
int
soc_bm9600_init(int unit, soc_sbx_config_t *cfg)
{
    bm9600InitParams_t  *pBm9600InitParams = NULL;
    int                  uPlaneId, nLink;
    uint32               uStatus;
    sbFabStatus_t        status;
    sbLinkThresholdConfig_t linkThresholdConfig;
    uint32               uIndex;

    /* Allocate memory for NM Memory Cache */
    status = soc_bm9600_NmMemoryCacheAllocate(unit);
    if (status != SB_FAB_STATUS_OK) {
        status = BCM_E_MEMORY;
        goto err;
    }

    /* Allocate memory for INA cache */
    status = soc_bm9600_InaMemoryCacheAllocate(unit);
    if (status != SB_FAB_STATUS_OK) {
        status = BCM_E_MEMORY;
        goto err;
    }

    pBm9600InitParams = sal_alloc(sizeof(bm9600InitParams_t), "Polaris Init Params");
    if (pBm9600InitParams == NULL) {
        status = BCM_E_MEMORY;
        goto err;
    }

    /* NOTE: the XBAR only device can be converted to a XBAR/Arbiter later */
    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
        (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {
      GetDefaultBmInitParams(unit, pBm9600InitParams);
    } else {
      GetDefaultSeInitParams(unit, pBm9600InitParams);
    }

    pBm9600InitParams->m_nBaseAddr        = (UINT)(SOC_SBX_CFG(unit)->DeviceHandle);

    pBm9600InitParams->nm.bEnableAllEgress = (UINT)(SOC_SBX_CFG(unit)->enable_all_egress_nodes);

    pBm9600InitParams->nm.uNmDualGrantConfig0 = 0;
    pBm9600InitParams->nm.uNmDualGrantConfig1 = 0;
    pBm9600InitParams->nm.uNmDualGrantConfig2 = 0;

   
    switch ( SOC_SBX_CFG(unit)->uFabricConfig ) {
    case SOC_SBX_SYSTEM_CFG_VPORT: /* Vport * - Bm9600 + Sirius */
    case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* Vport Legacy * - Bm9600 + Qe2000 */
    case SOC_SBX_SYSTEM_CFG_VPORT_MIX: /* Vport Mix * - Bm9600 + Qe2000 + Sirius */
        pBm9600InitParams->bw.uEpochLength = SOC_SBX_CFG(unit)->epoch_length_in_timeslots;

        /*
         * for now, setting all timeslot sizes to same value - would want to optimize this per link up */
        for (uIndex = 0; uIndex < BM9600_NUM_TIMESLOTSIZE; ++uIndex) {
          int uTimeslotSizeNs=soc_sbx_fabric_get_timeslot_size(pBm9600InitParams->m_nBaseAddr, uIndex+1, 0, SOC_SBX_CFG(unit)->bHybridMode);
          pBm9600InitParams->fo.uAcTimeslotSize[uIndex]  = uTimeslotSizeNs /  BM9600_BW_CLOCK_PERIOD_200MHZ;
        }
        break;
    default:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_bm9600_init: unit %d unknown "
                               " fabric_configuration %d"),
                   unit, SOC_SBX_CFG(unit)->uFabricConfig));
        break;
    }

   /* All non-SCI serializers will be brought up in the SOC layer to allow
    * for proper serdes bring up.  They will all be forced low, until
    * enabled by bcm_port_enable
    */
    status = GetLinkThresholdConfig(SOC_SBX_CFG(unit)->uLinkThresholdIndex,
                    &linkThresholdConfig);
    if (status == SB_FAB_STATUS_OK) {
    pBm9600InitParams->uSiLsThreshold = linkThresholdConfig.uLsThreshold;
    pBm9600InitParams->uSiLsWindow = linkThresholdConfig.uLsWindowIn256Clocks;
    } else {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "BM9600 uLinkThresholdIndex(%d) initialization "
                           "parameter not initialized correctly."
                           " Should be [0,100]\n"),
               SOC_SBX_CFG(unit)->uLinkThresholdIndex));
    }

   /*
    * Since the user may be requesting fewer than the maximum links,
    * initialize the full link state array here, mark upper links as
    * reserved so that they cannot later be used
    */
    for (nLink=0; nLink<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; nLink++ ) {
        sal_memset(&SOC_SBX_CFG_BM9600(unit)->linkState[nLink], 0xAA,
           sizeof(SOC_SBX_CFG_BM9600(unit)->linkState[nLink]));
        if (nLink < SB_FAB_USER_MAX_NUM_NODES) {
            SOC_SBX_CFG_BM9600(unit)->linkState[nLink].nState = SB_FAB_DEVICE_SERIALIZER_STATE_UNUSED;
        } else {
            SOC_SBX_CFG_BM9600(unit)->linkState[nLink].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
        }
    }

    /* Enable and power up all serdes, force them all to tx low.  The BCM layer
     * will take care of removing the force low when the port is enabled.  This
     * allows the serdes to be brought up in a predictable manner.
     *  --- SerializerMask set up in sbx_drv.c -----
     */
    sal_memcpy(pBm9600InitParams->uLcmXcfg, SOC_SBX_CFG_BM9600(unit)->uLcmXcfg,
           (size_t)(HW_BM9600_MAX_PLANES * \
                        SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS * \
                sizeof(uint32)));

    for (uPlaneId = 0; uPlaneId < SB_FAB_MAX_NUM_DATA_PLANES; uPlaneId++) {
         pBm9600InitParams->uLcmPlaneValid[uPlaneId] = \
                       SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[uPlaneId];
    }

   /*
    *
    */
    pBm9600InitParams->bw.uMaxResponseLatencyInTimeSlots   = 1;
    pBm9600InitParams->bw.bWredEnable                = TRUE;
    pBm9600InitParams->bw.uActiveBagNum              = BM9600_BW_MAX_BAG_NUM;
    pBm9600InitParams->bw.uActiveVoqNum              = SOC_SBX_CFG(unit)->num_queues;

    pBm9600InitParams->fo.uLocalBmId          = SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId;
    pBm9600InitParams->fo.uDefaultBmId        = SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId;
    pBm9600InitParams->fo.uAutoFailoverEnable = 0;
    pBm9600InitParams->fo.uAutoLinkDisEnable  = 0;
    pBm9600InitParams->fo.uMaxDisLinks        = SOC_SBX_CFG(unit)->uMaxFailedLinks;
   /*
    * redundancy setup
    */
    switch (SOC_SBX_CFG(unit)->uRedMode) {
        case bcmFabricRed1Plus1ELS:
            pBm9600InitParams->fo.uMaxDisLinks = 24;
        case bcmFabricRed1Plus1LS:
        case bcmFabricRed1Plus1Both:
            pBm9600InitParams->fo.uAutoFailoverEnable = 1;
            pBm9600InitParams->fo.uAutoLinkDisEnable  = 1;
            break;
        case bcmFabricRedELS:
            pBm9600InitParams->fo.uMaxDisLinks = 24;
        case bcmFabricRedLS:
            pBm9600InitParams->fo.uAutoLinkDisEnable  = 1;
            break;
        case bcmFabricRedManual:
            break;
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_bm3200_init: unit %d unsupported "
                                   " redundancy mode %d"),
                       unit, SOC_SBX_CFG(unit)->uRedMode));
            status = BCM_E_PARAM;
            goto err;
    }

   /*
    * BM9600_NUM_LINKS = 96
    */
    pBm9600InitParams->bw.bBandwidthAllocationEnable = FALSE;
    for (nLink = 0; nLink < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; nLink++ ) {

    pBm9600InitParams->si[nLink].uSerdesAbility = SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[nLink];
        pBm9600InitParams->si[nLink].bBringUp   = 1;
    pBm9600InitParams->ai[nLink].bExpandedQe2kEsetSpace = SOC_SBX_CFG(unit)->use_extended_esets;

    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
        (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {

        if( soc_property_port_get(unit,  nLink, spn_PORT_IS_SCI, 0) ){
        
        pBm9600InitParams->si[nLink].bIsEnabled  = 1;
        pBm9600InitParams->si[nLink].eChannelType  = CONTROL;
        pBm9600InitParams->ai[nLink].bIsEnabled  = 1;
        pBm9600InitParams->ina[nLink].bBringUp  = 1;
        pBm9600InitParams->ina[nLink].bIsEnabled = 1;
        if (pBm9600InitParams->ai[nLink].bExpandedQe2kEsetSpace) {
        pBm9600InitParams->ina[nLink].uEsetLimit = 63;  /* support 1024 esets */
        } else {
            pBm9600InitParams->ina[nLink].uEsetLimit = 7;   /* support 128 esets */
        }
        pBm9600InitParams->bw.bBandwidthAllocationEnable = TRUE;
        pBm9600InitParams->fo.uNumNodes ++;                 /* record number of nodes */
        }else{
        pBm9600InitParams->si[nLink].bIsEnabled  = 1;
        pBm9600InitParams->si[nLink].eChannelType  = DATA;
        pBm9600InitParams->ina[nLink].bIsEnabled = 0;
        }
    } else {
        pBm9600InitParams->si[nLink].bIsEnabled  = 1;
        pBm9600InitParams->si[nLink].eChannelType  = DATA;
        pBm9600InitParams->ai[nLink].bBringUp   = 0;
        pBm9600InitParams->ina[nLink].bIsEnabled = 0;
    }

    /* Sirius only system, set jitter tolerance, this is not used at soc init time now
     * moved to bcm init of port module
     if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
         pBm9600InitParams->si[nLink].uJitTolerance = 24;
     }
     if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
         pBm9600InitParams->si[nLink].uJitTolerance = 24;
     }
    */
    }
    pBm9600InitParams->bIsLcm = FALSE;

    /* Get global speed setting */
    pBm9600InitParams->uSerdesSpeed = SOC_SBX_CFG(unit)->uSerdesSpeed;
    pBm9600InitParams->bSerdesEncoding = SOC_SBX_CFG(unit)->bSerdesEncoding;


    /* Sirius only system, set jitter tolerance */
    if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
     (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {

        if (pBm9600InitParams->uSerdesSpeed == 6500) {
            pBm9600InitParams->xb.bEnableSotPolicing = FALSE;
            pBm9600InitParams->xb.uTsJitterTolerance = 20;
        } else {
            pBm9600InitParams->xb.bEnableSotPolicing = TRUE;
            pBm9600InitParams->xb.uTsJitterTolerance = 26;
        }
    }

    /*
     * LCM Default Configuration
     */
    if (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE) {
        pBm9600InitParams->xb.eXcfgMode = XCFG_LCM_MODE_FIXED_ADDRESS_A;
        /* pBm9600InitParams->xb.uInitMode = BM9600_XCFG_REMAP_INIT0; */

        pBm9600InitParams->bIsLcm = TRUE;
    }

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) {
    pBm9600InitParams->xb.bEnableXcfgReplace = 0;
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
    if (SOC_SBX_CFG(unit)->diag_qe_revid == BCM88230_A0_REV_ID) {
        pBm9600InitParams->xb.bEnableXcfgReplace = 1;
    } else {
        pBm9600InitParams->xb.bEnableXcfgReplace = 0;
    }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
    /* This will restrict how qe2k node links are connected to crossbars,
     * basically all the links of a particular node can not accross port 64 
     * boundary of polaris. Either all less than 64, or all large of equal
     * to 64.
     */
    if (SOC_SBX_CFG(unit)->diag_qe_revid == BCM88230_A0_REV_ID) {
        pBm9600InitParams->xb.bEnableXcfgReplace = 1;
    } else {
        pBm9600InitParams->xb.bEnableXcfgReplace = 0;
    }
    } else {
    pBm9600InitParams->xb.bEnableXcfgReplace = 0;
    }

   /*
    * call hardware init with set up params ...
    */
    uStatus = hwBm9600Init(pBm9600InitParams);

    if (uStatus != HW_BM9600_STATUS_OK_K) {
        LOG_CLI((BSL_META_U(unit,
                            "Bm9600 initialization failed with uStatus(0x%x)\n"),uStatus));
        
        status = BCM_E_INTERNAL;
        goto err;
    }

    /* Initialize SOC link control module */
    soc_linkctrl_init(unit, &soc_linkctrl_driver_sbx);

    sal_free((void *)pBm9600InitParams);
    return soc_xcfgremap_all_to_lb(unit);

err:

    /* De-allocate memory for NM Memory Cache */
    soc_bm9600_NmMemoryCacheDeAllocate(unit);

    if (pBm9600InitParams != NULL) {
        sal_free((void *)pBm9600InitParams);
    }

    return(status);
}


int
soc_bm9600_port_info_config(int unit)
{
     soc_info_t          *si;
     int                  backward_compatible = TRUE;
     int                  sci_only, sfi_only, sci_sfi;
     int                  port;
     soc_driver_t        *driver;

     si  = &SOC_INFO(unit);
     driver = SOC_DRIVER(unit);

     driver->port_info = soc_port_info_bcm88130;
     si->sci.min = si->sci.max = -1;
     si->sfi.min = si->sfi.max = -1;
     si->sfi.num = si->sci.num = 0;

     si->port_num = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;

     for (port = 0; port < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++) {
     sci_only = sfi_only = sci_sfi = FALSE;
     if (backward_compatible) {
         /* Following scheme doesn't  require modify the old config.bcm
          *   SFI only ports, spn_PORT_IS_SCI = 0
          *   SCI only ports, spn_PORT_IS_SCI = 1, spn_PORT_IS_SFI = 0
          *   SCI+SFI ports, spn_PORT_IS_SCI = 1 and spn_PORT_IS_SFI = 1
          */
         if( soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
         if( soc_property_port_get(unit, port, spn_PORT_IS_SFI, 0) ){
             SBX_ADD_PORT(sci,port);
             SBX_ADD_PORT(sfi,port);
             sci_sfi = TRUE;
         } else {
             SBX_ADD_PORT(sci,port);
             sci_only = TRUE;
         }
         } else {
         SBX_ADD_PORT(sfi,port);
         sfi_only = TRUE;
         }
             SBX_ADD_PORT(port,port);
         SBX_ADD_PORT(all,port);
     } else {
         /* Following scheme require modify the old config.bcm to 
          * add spn_PORT_IS_SFI for all sfi ports
          *   SFI only ports, spn_PORT_IS_SCI = 0, spn_PORT_IS_SFI = 1
          *   SCI only ports, spn_PORT_IS_SCI = 1, spn_PORT_IS_SFI = 0
          *   SCI+SFI ports, spn_PORT_IS_SCI = 1 and spn_PORT_IS_SFI = 1
          *   unused ports, spn_PORT_IS_SCI = 0 and spn_PORT_IS_SFI = 0
          */
		 /* coverity[dead_error_line : FALSE] */
         if( soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
         SBX_ADD_PORT(sci,port);         
         sci_only = TRUE;
         }
         if( soc_property_port_get(unit, port, spn_PORT_IS_SFI, 0) ){
         SBX_ADD_PORT(sfi,port);
         if (sci_only) {
             sci_only = FALSE;
             sci_sfi = TRUE;
         } else {
             sfi_only = TRUE;
         }
         }
         if( (soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0)) ||
         (soc_property_port_get(unit, port, spn_PORT_IS_SFI, 0)) ) {
           SBX_ADD_PORT(port,port);
           SBX_ADD_PORT(all,port);
         }
     }

     /* not block based, just pick a block type */
     si->port_type[port] = SOC_BLK_GXPORT;

     if( sci_sfi ) {
         sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
              "sci_sfi%d", port);
     } else if ( sci_only ) {
         sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
              "sci%d", port);
     } else if ( sfi_only ) {
         sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
              "sfi%d", port);
     }
     }
     return SOC_E_NONE;
}

/*
 *   Function
 *      soc_sbx_eset_init
 *   Purpose
 *      Initialize the ESET table in teh BME.
 *   Parameters
 *      (in) unit      - BCM unit number
 *
 *   Returns
 *     BCM_E_NONE if successful
 *     BCM_E_* appropriately if not
 */
int
soc_bm9600_eset_init(int unit)
{
#if 0
    int i, rv;
    uint32 uNodesMasks[4];
    int nNode, reg;
    int nNumNodes = soc_property_get(unit, spn_NUM_MODULES, SBX_MAX_NODES);

    /* return if the NodeMap is disabled on this unit, it's not
     * neccessarily an error because the BM9600 can be an SE, a BME or an LCM
     * and the SDK has no way of knowing how it's configured other than by
     * the NM enable bit
     */

    reg = SAND_HAL_READ(unit, PL, PI_CLOCK_GATE_CONFIG2);
    if (SAND_HAL_GET_FIELD(PL, PI_CLOCK_GATE_CONFIG2, NM_FO_BW_CLK_ENABLE, reg) == 0) {
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

            rv = soc_sbx_eset_block_set(unit, nNode << 2, uNodesMasks);
            if( SOC_FAILURE(rv) ){
                return rv;
            }
        }

    } else {
        int nBaseEset;

        /* all nodes */
        for(i = 0; i < 4; i++) {
            uNodesMasks[i] = (1 << nNumNodes) - 1;
        }

        for(nBaseEset = 0; nBaseEset < HW_BM9600_PL_EMAP_MEM_SIZE; nBaseEset++) {
            rv = soc_sbx_eset_block_set(unit, nBaseEset << 2, uNodesMasks);
            if( BCM_FAILURE(rv) ){
                return rv;
            }
        }
    }
#endif
    return SOC_E_NONE;
}


/***********************************************
 * write bag entry for a bag
 */
int
soc_bm9600_bag_write(int unit,
             int bw_group,
             uint32 num_queues_in_bag,
             uint32 base_queue,
             uint32 bag_rate_bytes_per_epoch)
{
    int status = SOC_E_NONE;
    uint32 addr;
    uint32 write_value = 0;
    sbZfFabBm9600BwR0BagEntry_t zfR0BagEntry;
    sbZfFabBm9600BwR1BagEntry_t zfR1BagEntry;
    uint32 scale = 0;

    addr = bw_group;
    
    /* R0 bag table */
    zfR0BagEntry.m_uBagRate = bag_rate_bytes_per_epoch;
    
    /* Scale up bag rate to compensate the trunking errors 
     * Note that the scale up amount depends on the absolute
     * number writen into the memory table
     */
    if (bag_rate_bytes_per_epoch != 0) {
    scale = (32 * 1024 * 1024) / bag_rate_bytes_per_epoch;
    zfR0BagEntry.m_uBagRate = zfR0BagEntry.m_uBagRate * (scale + 1) / scale;
    }

    sbZfFabBm9600BwR0BagEntry_Pack(&zfR0BagEntry, (uint8*)&write_value, 4);
    
    status = soc_bm9600_BwR0BagWrite(unit, addr, write_value);
    if (status) {
    return status;
    }
    
    /* R1 bag table */
    zfR1BagEntry.m_uBaseVoq  = base_queue;
    zfR1BagEntry.m_uNumVoqs  = num_queues_in_bag;
    
    sbZfFabBm9600BwR1BagEntry_Pack(&zfR1BagEntry, (uint8*)&write_value, 4);
    
    status = soc_bm9600_BwR1BagWrite(unit, addr, write_value);

    return status;
}


/***********************************************
 * write bag entry for a bag
 */
int
soc_bm9600_bag_read(int unit,
            int bw_group,
            uint32 *pnum_queues_in_bag,
            uint32 *pbase_queue,
            uint32 *pbag_rate_bytes_per_epoch)
{
    int status = 0;
    uint32 addr;
    uint32 read_value;
    sbZfFabBm9600BwR0BagEntry_t zfR0BagEntry;
    sbZfFabBm9600BwR1BagEntry_t zfR1BagEntry;
    uint32 scale = 0;

    addr = bw_group;

    status = soc_bm9600_BwR0BagRead(unit, addr, &read_value);
    if (status) {
    return status;
    }

    sbZfFabBm9600BwR0BagEntry_Unpack(&zfR0BagEntry, (uint8*)&read_value, 4);

    /* R0 bag table */

    /* Scale down bag rate to de-compensate the trunking errors 
     * Note that the scale up amount depends on the absolute
     * number writen into the memory table
     */
    if (zfR0BagEntry.m_uBagRate > 0) {
        scale = ( (32 * 1024 * 1024) / zfR0BagEntry.m_uBagRate ) + 1;
        *pbag_rate_bytes_per_epoch = (zfR0BagEntry.m_uBagRate * scale) / (scale + 1);
    } else {
        *pbag_rate_bytes_per_epoch = 0;
    }

    status = soc_bm9600_BwR1BagRead(unit, addr, &read_value);
    if (status) {
    return status;
    }

    sbZfFabBm9600BwR1BagEntry_Unpack(&zfR1BagEntry, (uint8*)&read_value, 4);

    /* R1 bag table */
    *pbase_queue = zfR1BagEntry.m_uBaseVoq;
    *pnum_queues_in_bag = zfR1BagEntry.m_uNumVoqs;

    return status;
}

/***********************************************
 * write sysport array table in NM for a portset/offset
 */
int
soc_bm9600_nm_sysport_array_table_write(uint32 unit,
                    int32 portset,
                    int32 offset,
                    int32 sysport,
                    int32 new_row)
{
    uint32 uData[6];
    sbZfFabBm9600NmSysportArrayEntry_t zfSysportArrayEntry;
    int bSuccess;

    if ( (sysport < 0)                         ||
     ((sysport >= BM9600_MAX_NUM_SYSPORTS) && (sysport != 0xFFF))  ||
     (portset < 0)                         ||
     (portset >=  BM9600_MAX_PORTSETS)     ||
     (offset < 0)                          ||
     (offset >= 16) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range, sysport(%d)"
               " portset(%d) offset(%d)\n"),
               FUNCTION_NAME(), sysport, portset, offset));
    return SOC_E_INTERNAL;
    }

    if(new_row){
    /* If new row, initialize it */
    zfSysportArrayEntry.m_uSpa_0 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_1 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_2 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_3 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_4 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_5 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_6 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_7 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_8 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_9 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_10 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_11 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_12 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_13 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_14 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    zfSysportArrayEntry.m_uSpa_15 = SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID;
    } else {
    /* if old row, read other sysport in same row from hardware */
    bSuccess = soc_bm9600_NmSysportArrayRead(unit, portset, &uData[0], &uData[1], &uData[2],
                         &uData[3], &uData[4], &uData[5]);
    if (bSuccess != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Failed to read NM Sysport Array Memory"
                   " row %d. \n"),
                   FUNCTION_NAME(), (uint32) portset));
        return bSuccess;
    }
    sbZfFabBm9600NmSysportArrayEntry_Unpack(&zfSysportArrayEntry, (uint8 *)uData, 24);
    }

    switch (offset) {
    case 0: {
        zfSysportArrayEntry.m_uSpa_0 = sysport;
        break;
    }
    case 1: {
        zfSysportArrayEntry.m_uSpa_1 = sysport;
        break;
    }
    case 2: {
        zfSysportArrayEntry.m_uSpa_2 = sysport;
        break;
    }
    case 3: {
        zfSysportArrayEntry.m_uSpa_3 = sysport;
        break;
    }
    case 4: {
        zfSysportArrayEntry.m_uSpa_4 = sysport;
        break;
    }
    case 5: {
        zfSysportArrayEntry.m_uSpa_5 = sysport;
        break;
    }
    case 6: {
        zfSysportArrayEntry.m_uSpa_6 = sysport;
        break;
    }
    case 7: {
        zfSysportArrayEntry.m_uSpa_7 = sysport;
        break;
    }
    case 8: {
        zfSysportArrayEntry.m_uSpa_8 = sysport;
        break;
    }
    case 9: {
        zfSysportArrayEntry.m_uSpa_9 = sysport;
        break;
    }
    case 10: {
        zfSysportArrayEntry.m_uSpa_10 = sysport;
        break;
    }
    case 11: {
        zfSysportArrayEntry.m_uSpa_11 = sysport;
        break;
    }
    case 12: {
        zfSysportArrayEntry.m_uSpa_12 = sysport;
        break;
    }
    case 13: {
        zfSysportArrayEntry.m_uSpa_13 = sysport;
        break;
    }
    case 14: {
        zfSysportArrayEntry.m_uSpa_14 = sysport;
        break;
    }
    case 15: {
        zfSysportArrayEntry.m_uSpa_15 = sysport;
        break;
    }
#ifdef NOTDEF_COVERITY          
    default : {
        /* report error, should never happen due to params sanity check above */
        return SOC_E_INTERNAL;
    }
#endif /* NOTDEF_COVERITY: comment out this to make coverity happy */
    }

    /* Write to the sysport array */
    sbZfFabBm9600NmSysportArrayEntry_Pack(&zfSysportArrayEntry, (uint8 *)uData, 24);

    bSuccess = soc_bm9600_NmSysportArrayWrite(unit, portset, uData[0], uData[1], uData[2],
                          uData[3], uData[4], uData[5]);
    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Failed to write NM Sysport Array Memory"
               " row %d. \n"),
               FUNCTION_NAME(), (uint32) portset));
    return bSuccess;
    }

    return bSuccess;
}

/***********************************************
 * read sysport array table in NM for a portset/offset
 */
int
soc_bm9600_nm_sysport_array_table_read(uint32 unit,
                       int32 portset,
                       int32 offset,
                       int32 *sysport)
{
    uint32 uData[6];
    sbZfFabBm9600NmSysportArrayEntry_t zfSysportArrayEntry;
    int bSuccess;

    if ( (sysport == NULL)                     ||
     (portset < 0)                         ||
     (portset >=  BM9600_MAX_PORTSETS)     ||
     (offset < 0)                          ||
     (offset >= 16) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s:  Invalid params range, portset(%d)"
               " offset(%d)\n"),
               FUNCTION_NAME(), portset, offset));
    return SOC_E_INTERNAL;
    }

    /* read row from hardware */
    bSuccess = soc_bm9600_NmSysportArrayRead(unit, portset, &uData[0], &uData[1], &uData[2],
                         &uData[3], &uData[4], &uData[5]);
    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s:  Failed to read NM Sysport Array Memory"
               " row %d. \n"),
               FUNCTION_NAME(), (uint32) portset));
    return bSuccess;
    }

    sbZfFabBm9600NmSysportArrayEntry_Unpack(&zfSysportArrayEntry, (uint8 *)uData, 24);

    switch (offset) {
    case 0: {
        *sysport = zfSysportArrayEntry.m_uSpa_0;
        break;
    }
    case 1: {
        *sysport = zfSysportArrayEntry.m_uSpa_1;
        break;
    }
    case 2: {
        *sysport = zfSysportArrayEntry.m_uSpa_2;
        break;
    }
    case 3: {
        *sysport = zfSysportArrayEntry.m_uSpa_3;
        break;
    }
    case 4: {
        *sysport = zfSysportArrayEntry.m_uSpa_4;
        break;
    }
    case 5: {
        *sysport = zfSysportArrayEntry.m_uSpa_5;
        break;
    }
    case 6: {
        *sysport = zfSysportArrayEntry.m_uSpa_6;
        break;
    }
    case 7: {
        *sysport = zfSysportArrayEntry.m_uSpa_7;
        break;
    }
    case 8: {
        *sysport = zfSysportArrayEntry.m_uSpa_8;
        break;
    }
    case 9: {
        *sysport = zfSysportArrayEntry.m_uSpa_9;
        break;
    }
    case 10: {
        *sysport = zfSysportArrayEntry.m_uSpa_10;
        break;
    }
    case 11: {
        *sysport = zfSysportArrayEntry.m_uSpa_11;
        break;
    }
    case 12: {
        *sysport = zfSysportArrayEntry.m_uSpa_12;
        break;
    }
    case 13: {
        *sysport = zfSysportArrayEntry.m_uSpa_13;
        break;
    }
    case 14: {
        *sysport = zfSysportArrayEntry.m_uSpa_14;
        break;
    }
    case 15: {
        *sysport = zfSysportArrayEntry.m_uSpa_15;
        break;
    }
#ifdef NOTDEF_COVERITY          
    default : {
        /* report error, should never happen due to params sanity check above */
        return SOC_E_INTERNAL;
    }
#endif /* NOTDEF_COVERITY: comment out this to make coverity happy */
    }

    return bSuccess;
}

/***********************************************
 * write portset link table entry
 * assuming index are always same as portset
 */
int
soc_bm9600_portset_link_table_write(uint32 unit,
                    int32 portset,
                    int32 next)
{
    int bSuccess;
    uint32 uData = 0;
    sbZfFabBm9600NmPortsetLinkEntry_t zfPortsetLink;

    if ( (portset < 0)                        ||
     (portset >= BM9600_MAX_PORTSETS)     ||
     (next < 0)                           ||
     ((next > BM9600_MAX_PORTSETS) && (next != 0xFF)) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range portset(%d)"
               " next(%d)\n"), FUNCTION_NAME(), portset, next));
    return SOC_E_INTERNAL;
    }

    zfPortsetLink.m_uIndex = portset;
    zfPortsetLink.m_uNxtPtr = next;

    sbZfFabBm9600NmPortsetLinkEntry_Pack(&zfPortsetLink, (uint8 *)&uData, 2);

    bSuccess = soc_bm9600_NmPortsetLinkWrite(unit, portset, uData);

    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Error writing portset link table"
               " for entry %d\n"),
               FUNCTION_NAME(), (uint32)portset));
    return bSuccess;
    }

    return SOC_E_NONE;
}
/***********************************************
 * read portset link table entry
 * assuming index are always same as portset
 */
int
soc_bm9600_portset_link_table_read(uint32 unit,
                    int32 portset,
                    int32 *next)
{
    int bSuccess;
    uint32 uData;
    sbZfFabBm9600NmPortsetLinkEntry_t zfPortsetLink;

    if ( (portset < 0)                        ||
     (portset >= BM9600_MAX_PORTSETS)     ||
     (next == NULL) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range portset(%d)"
               " next(%d)\n"),
               FUNCTION_NAME(), portset, (int) next));
    return SOC_E_INTERNAL;
    }

    bSuccess = soc_bm9600_NmPortsetLinkRead(unit, portset, &uData);

    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Error reading portset link table for"
               " entry %d\n"),
               FUNCTION_NAME(), (uint32)portset));
    return bSuccess;
    }
    sbZfFabBm9600NmPortsetLinkEntry_Unpack(&zfPortsetLink, (uint8 *)&uData, 2);

    *next = zfPortsetLink.m_uNxtPtr;

    return SOC_E_NONE;
}

/***********************************************
 * write portset info table entry
 */
int
soc_bm9600_portset_info_table_write(uint32 unit,
                    int32 portset,
                    int32 virtual,
                    int32 eopp,
                    int32 start_port,
                    int32 eg_node)
{
    int bSuccess;
    uint32 uData = 0;
    sbZfFabBm9600NmPortsetInfoEntry_t zfPortsetInfo;
    int32 eg_phy_node;


    if ( (portset < 0)                             ||
     (portset >= BM9600_MAX_PORTSETS)          ||
     ((virtual != TRUE) && (virtual != FALSE)) ||
     (eopp < 0)                                ||
     (eopp > 0x7)                              ||
     (start_port < 0)                          ||
     (eg_node < 0)                             ||
     (eg_node >= BM9600_NUM_NODES) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range, portset(%d)"
               " virtual(%d) eopp(%x) start_port(%d),"
               " node(%d)\n"),
               FUNCTION_NAME(), portset, virtual, eopp,
               start_port, eg_node));
    return SOC_E_INTERNAL;
    }

    eg_phy_node = SOC_SBX_L2P_NODE(unit, eg_node);

    zfPortsetInfo.m_uVirtualPort = (virtual != FALSE)?1:0;
    zfPortsetInfo.m_uVportEopp = eopp;
    zfPortsetInfo.m_uStartPort = start_port;
    zfPortsetInfo.m_uEgNode = eg_phy_node;

    sbZfFabBm9600NmPortsetInfoEntry_Pack(&zfPortsetInfo, (uint8 *)&uData, 4);

    bSuccess = soc_bm9600_NmPortsetInfoWrite(unit, portset, uData);

    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Error writing portset info table"
               " for entry %d\n"),
               FUNCTION_NAME(), (uint32)portset));
    return bSuccess;
    }

    return SOC_E_NONE;


}

/***********************************************
 * Read portset info table entry
 */
int
soc_bm9600_portset_info_table_read(uint32 unit,
                    int32 portset,
                    int32 *p_virtual,
                    int32 *p_eopp,
                    int32 *p_start_port,
                    int32 *p_eg_node)
{
    int bSuccess;
    uint32 uData;
    sbZfFabBm9600NmPortsetInfoEntry_t zfPortsetInfo;


    if ( (portset < 0)  ||
     (portset >= BM9600_MAX_PORTSETS) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range, portset(%d)\n"),
               FUNCTION_NAME(), portset));
    return SOC_E_INTERNAL;
    }

    bSuccess = soc_bm9600_NmPortsetInfoRead(unit, portset, &uData);

    if (bSuccess != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s:  Error reading portset info table"
               " for entry %d\n"),
               FUNCTION_NAME(), (uint32)portset));
    return bSuccess;
    }

    sbZfFabBm9600NmPortsetInfoEntry_Unpack(&zfPortsetInfo, (uint8 *)&uData, 4);

    *p_virtual = zfPortsetInfo.m_uVirtualPort;
    *p_eopp = zfPortsetInfo.m_uVportEopp;
    *p_start_port = zfPortsetInfo.m_uStartPort;
    *p_eg_node = SOC_SBX_P2L_NODE(unit, zfPortsetInfo.m_uEgNode);

    return SOC_E_NONE;
}

/***********************************************
 * write sysport map table for a sysport in all enabled inas
 */
int
soc_bm9600_ina_sysport_map_table_write_all(uint32 unit,
                       int32 sysport,
                       int32 portset,
                       int32 offset)
{
    uint32 ina_enabled[BM9600_NUM_LINKS];
    int ina;
    uint32 uData, ina_config;
    sbZfFabBm9600InaSysportMapEntry_t zfInaSysportMap;
    int bSuccess;
    int cached_ina;

    if ( (sysport < 0)                         ||
     (sysport >= BM9600_MAX_NUM_SYSPORTS)  ||
     (portset < 0)                         ||
     (offset < 0)                          ||
     (offset >= 16) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range, sysport(%d)"
               " portset(%d) offset(%d)\n"),
               FUNCTION_NAME(), sysport, portset, offset));
    return SOC_E_INTERNAL;
    }

    /* Find all enabled ina */
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    ina_enabled[ina] = FALSE;
    }

    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    uData = SAND_HAL_READ_STRIDE(unit, PL, INA, ina, INA_CONFIG);
    if (SAND_HAL_GET_FIELD(PL, INA_CONFIG, ENABLE, uData)) {
        ina_enabled[ina] = TRUE;
    }
    }

    /* determine if cached ina is enabled */
    cached_ina = SOC_SBX_CFG_BM9600(unit)->cached_ina;
    if (cached_ina != -1) {
        if (ina_enabled[cached_ina] == FALSE) {
            ina_config = SAND_HAL_READ_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG);
            ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 1);
            SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);
        }
    }

    /* Update the sysport mapping table for all enabled ina */
    zfInaSysportMap.m_uPortsetAddr = portset;
    zfInaSysportMap.m_uOffset = offset;
    sbZfFabBm9600InaSysportMapEntry_Pack(&zfInaSysportMap, (uint8*)&uData, 4);

    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    if ( (ina_enabled[ina]) || (ina == cached_ina) ) {
        bSuccess = soc_bm9600_InaSysportMapWrite(unit, sysport, ina, uData);
        if (bSuccess != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Error writing ina sysport map"
                   " for sysport %d\n"),
                   FUNCTION_NAME(), (uint32)sysport));
        goto err;
        }
    }
    }

    /* restore state */
    if (cached_ina != -1) {
        if (ina_enabled[cached_ina] == FALSE) {
            ina_config = SAND_HAL_READ_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG);
            ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 0);
            SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);
        }
    }

    return SOC_E_NONE;

err:
    /* restore state */
    if (cached_ina != -1) {
        if (ina_enabled[cached_ina] == FALSE) {
            ina_config = SAND_HAL_READ_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG);
            ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 0);
            SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);
        }
    }

    return bSuccess;

}

/***********************************************
 * read sysport map table for a sysport
 */
int
soc_bm9600_ina_sysport_map_table_read(uint32 unit,
                      int32 sysport,
                      int32 *portset,
                      int32 *offset)
{
    uint32 ina_enabled[BM9600_NUM_LINKS];
    int ina;
    uint32 uData;
    sbZfFabBm9600InaSysportMapEntry_t zfInaSysportMap;
    int bSuccess;

    if ( (sysport < 0)                         ||
     (sysport >= BM9600_MAX_NUM_SYSPORTS) ) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: Invalid params range, sysport(%d)\n"),
               FUNCTION_NAME(), sysport));
    return SOC_E_INTERNAL;
    }

    /* Find all enabled ina */
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    ina_enabled[ina] = FALSE;
    }

    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    uData = SAND_HAL_READ_STRIDE(unit, PL, INA, ina, INA_CONFIG);
    if (SAND_HAL_GET_FIELD(PL, INA_CONFIG, ENABLE, uData)) {
        ina_enabled[ina] = TRUE;
    }
    }

    /* Read the sysport mapping table from one of enabled ina, assuming all are same */
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
    if (ina_enabled[ina]) {
        bSuccess = soc_bm9600_InaSysportMapRead(unit, sysport, ina, &uData);
        if (bSuccess != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Error reading ina sysport map"
                   " for sysport %d on ina %d\n"),
                   FUNCTION_NAME(), (uint32)sysport,
                   (uint32)ina));
        return bSuccess;
        } else {
        sbZfFabBm9600InaSysportMapEntry_Unpack(&zfInaSysportMap, (uint8*)&uData, 4);
        *portset = zfInaSysportMap.m_uPortsetAddr;
        *offset = zfInaSysportMap.m_uOffset;
        return SOC_E_NONE;
        }
    }
    }

    /* retreive entry from cache */
    bSuccess = soc_bm9600_InaSysportMapCacheRead(unit, sysport, ina, &uData);
    if (bSuccess != SOC_E_NONE) {
        /* Only reach here when none of INA is enabled */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Error retreiving ina sysport map from cache, sysport %d\n"),
                   FUNCTION_NAME(), (uint32)sysport));
        return(bSuccess);
    }
    sbZfFabBm9600InaSysportMapEntry_Unpack(&zfInaSysportMap, (uint8*)&uData, 4);
    *portset = zfInaSysportMap.m_uPortsetAddr;
    *offset = zfInaSysportMap.m_uOffset;

    return(SOC_E_NONE);
}

/***********************************************
 * Atomic steps to move sysport from old portset/offset to a new portset/offset
 */
int
soc_bm9600_move_sysport_pri(uint32 unit,
                int32  sysport,
                            int32  old_portset,
                            int32  old_offset,
                            int32  new_portset,
                            int32  new_offset)
{
    
    uint32 uRegMove0, uRegMove1;
    uint32 uData, ina;
    uint32 ina_enabled[BM9600_NUM_LINKS];
    uint32 ina_done[BM9600_NUM_LINKS];
    uint32 bDone, count;
    int    rv = BCM_E_NONE;
    
    if ( (sysport < 0)                         ||
         (sysport >= BM9600_MAX_NUM_SYSPORTS)  ||
         (old_portset < 0)                     ||
         (old_portset >=  BM9600_MAX_PORTSETS) ||
         (old_offset < 0)                      ||
         (old_offset >= 16)                    ||
         (new_portset < 0)                     ||
         (new_portset >=  BM9600_MAX_PORTSETS) ||
         (new_offset < 0)                      ||
         (new_offset >= 16) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Invalid params range, sysport(%d),"
                   " old_ps(%d) old_os(%d) new_ps(%d),"
                   " new_os(%d)\n"),
                   FUNCTION_NAME(), sysport, old_portset, old_offset,
                   new_portset, new_offset));
        return SOC_E_INTERNAL;
    }
    
    /* start the move */
    uRegMove1  = SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE1, OLD_PORTSET_ADDR,  old_portset);
    uRegMove1 |= SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE1, OLD_OFFSET, old_offset);
    uRegMove1 |= SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE1, NEW_PORTSET_ADDR, new_portset);
    uRegMove1 |= SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE1, NEW_OFFSET, new_offset);
    
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_PORT_PRI_MOVE1, uRegMove1);
    
    uRegMove0  = SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE0, START, 1);
    uRegMove0 |= SAND_HAL_SET_FIELD(PL,INA_PORT_PRI_MOVE0, SYS_PORT_NUM, sysport);
    
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_PORT_PRI_MOVE0, uRegMove0);
    
    /* poll all enabled ina to make sure the move is done */
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
        ina_enabled[ina] = FALSE;
        ina_done[ina] = FALSE;
    }
    COMPILER_REFERENCE(ina_enabled);
    
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
        uData = SAND_HAL_READ_STRIDE_POLL(unit, PL, INA, ina, INA_CONFIG);
        if (SAND_HAL_GET_FIELD(PL, INA_CONFIG, ENABLE, uData)) {
            ina_enabled[ina] = TRUE;
        } else {
            ina_done[ina] = TRUE;
        }
    }

    count = 0;
    while (1) {
        for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
            if ( ina_done[ina] == FALSE ) {
                uData = SAND_HAL_READ_STRIDE_POLL(unit, PL, INA, ina, INA_PORT_PRI_MOVE0);
                if (SAND_HAL_GET_FIELD(PL, INA_PORT_PRI_MOVE0, DONE, uData)) {
                    /* Done for this ina */
                    ina_done[ina] = TRUE;
                    
                    /* Clear the done bit */
                    uData = 0;
                    SAND_HAL_WRITE_STRIDE(unit, PL, INA, ina, INA_PORT_PRI_MOVE0, uData);
                }
            }
        }
        
        bDone = TRUE;
        for (ina = 0; (ina < BM9600_NUM_LINKS) && (bDone == TRUE) ; ina++) {
            bDone &= ina_done[ina];
        }
        
        if (bDone) {

            rv = soc_bm9600_InaUpdateCachedSysportMap(unit,
                                                      SOC_SBX_CFG_BM9600(unit)->cached_ina,
                                                      sysport, 
                                                      new_portset, 
                                                      new_offset);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("Error updating INA cache for sysport(%d)\n"), sysport));
            }


            /* All enabled INA are done */
            return SOC_E_NONE;
        } else {
            /* Wait another round */
            count++;
            if (count > 100) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Error moving sysport %d\n"),
                           FUNCTION_NAME(), (uint32)sysport));
                return SOC_E_INTERNAL;
            }
        }
    }
}

/***********************************************
 * init one row of port priority table
 */
int
soc_bm9600_portpri_init(uint32 unit,
			int32  portset)
{

    uint32 uData, ina;
    uint32 ina_enabled[BM9600_NUM_LINKS];
    uint32 ina_done[BM9600_NUM_LINKS];
    uint32 bDone, count;

    if ( (portset < 0)                     ||
	 (portset >=  BM9600_MAX_PORTSETS) ) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
	          (BSL_META("%s: Invalid params range, portset(%d)\n"),
	           FUNCTION_NAME(), portset));
	return SOC_E_INTERNAL;
    }

    /* start the write */
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_DATA0, 0);
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_DATA1, 0);
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_DATA2, 0);
    SAND_HAL_WRITE_STRIDE(unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_DATA3, 0);

    SAND_HAL_WRITE_STRIDE( unit, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, portset)
			   );

    /* poll all enabled ina to make sure the write is done */
    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
	ina_enabled[ina] = FALSE;
	ina_done[ina] = FALSE;
    }
    COMPILER_REFERENCE(ina_enabled);

    for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
	uData = SAND_HAL_READ_STRIDE_POLL(unit, PL, INA, ina, INA_CONFIG);
	if (SAND_HAL_GET_FIELD(PL, INA_CONFIG, ENABLE, uData)) {
	    ina_enabled[ina] = TRUE;
	} else {
	    ina_done[ina] = TRUE;
	}
    }

    count = 0;
    while (1) {
	for (ina = 0; ina < BM9600_NUM_LINKS; ina++) {
	    if ( ina_done[ina] == FALSE ) {
		uData = SAND_HAL_READ_STRIDE_POLL(unit, PL, INA, ina, INA_MEM_ACC_CTRL);
		if (SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, uData)) {
		    /* Done for this ina */
		    ina_done[ina] = TRUE;
		}
	    }
	}

	bDone = TRUE;
	for (ina = 0; (ina < BM9600_NUM_LINKS) && (bDone == TRUE) ; ina++) {
	    bDone &= ina_done[ina];
	}

	if (bDone) {
	    /* All enabled INA are done */
	    return SOC_E_NONE;
	} else {
	    /* Wait another round */
	    count++;
	    if (count > 100) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
		          (BSL_META("%s: Error init portpri table for"
		           " row %d\n"),
		           FUNCTION_NAME(), (uint32)portset));
		return SOC_E_INTERNAL;
	    }
	}
    }

}

/***********************************************
 * Write to a HyperCore phy register.
 *  For registers above address location 0x001f this function will load the
 *  base address of the block containing the given register unless it is
 *  already set.
 *  For broadcast writes users should use the hwBm9600HcBroadcastWrite function.
 */
int soc_bm9600_mdio_hc_write(int unit, uint32 uPhyAddr, uint32 uLane, uint32 uRegAddr, uint32 uData) {
    uint32 uBlkBaseAddr;
    uint32 uRegOffset;
    int status;

    /*  HC register address space is only 16 bits. */
    if (uRegAddr > 0xffff) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Address must be less than or equal to 0xffff\n")));
	return SOC_E_PARAM;
    }
    /* 0x3ff is broadcast to all lanes */
    if (uLane > 0x3ff) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid lane (0-3 or 0x3ff for broadcast)\n")));
	return SOC_E_PARAM;
    }
    if (uPhyAddr == 0x1f) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Use soc_bm9600_mdio_broadcast_write for broadcast writes to HC's instead\n")));
	return SOC_E_PARAM;
    }

    /* Set the requested lane */
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, 0x1f, HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0xfff0);
    if (status) {
        return status;
    }
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, (HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0x000f) | (0x1<<4), uLane );
    if (status) {
        return status;
    }

#if 000
    if (uRegAddr <= 0x1f) { /* direct write */

	status = soc_bm9600_mdio_hc_cl22_write(unit, uPhyAddr, uRegAddr, uData);
	if (status) {
	    return status;
	}

    } else {  /* indirect write */
#endif

    {
	uBlkBaseAddr = uRegAddr & 0x0000fff0;

	/*  New block so set the block base address. */
	/*  This pointer to the phy block resides at location 0x1f within */
	/*  the HC phy.  Each block contains at most 16 registers (i.e. each block */
	/*  has a 4 bit address space). */
	status = soc_bm9600_mdio_hc_cl22_write(unit, uPhyAddr, 0x1f, uBlkBaseAddr);

	if (status) {
	    return status;
	}

	/*  The register offset within the block is given in */
	/*  lower 4 bits. */
	uRegOffset = uRegAddr & 0xf;

	if (uRegAddr > 0xf) {
	    /*  Set bit[4] to tell the HC to use the block base address */
	    /*  register (within the HC) to select the block for this register. */
	    uRegOffset = uRegOffset | (0x1<<4);
	}

	status = soc_bm9600_mdio_hc_cl22_write(unit, uPhyAddr, uRegOffset, uData);
	if (status) {
	    return status;
	}
    }
#if 000
    /* Go back to lane 0 */
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, 0x1f, HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0xfff0);
    if (status) {
        return status;
    }
    soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, (HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0x000f) | (0x1<<4), 0 );
    if (status) {
        return status;
    }
#endif

    return SOC_E_NONE;
}

/***********************************************
 *  Read from a HyperCore phy register.
 *  For registers above address location 0x001f this function will load the
 *  base address of the block containing the given register unless it is
 *  already set.
 */
int soc_bm9600_mdio_hc_read(int unit, uint32 uPhyAddr, uint32 uLane, uint32 uRegAddr, uint32 *pReadData) {
    uint32 uBlkBaseAddr;
    uint32 uRegOffset;
    int status;

    /*  HC register address space is only 16 bits. */
    if (uRegAddr > 0xffff) {
	return SOC_E_PARAM;
    }

    /* 0x3ff is broadcast to all lanes */
    if (uLane > 0x3ff) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid lane (0-3 or 0x3ff for broadcast)\n")));
	return SOC_E_PARAM;
    }

    if (pReadData == NULL) {
	return SOC_E_PARAM;
    }

    /* Set the requested lane */
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, 0x1f, HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0xfff0);
    if (status) {
        return status;
    }
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, (HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0x000f) | (0x1<<4), uLane );
    if (status) {
        return status;
    }
#if 000
    if (uRegAddr <= 0x1f) { /* direct read */

	status = soc_bm9600_mdio_hc_cl22_read(unit, uPhyAddr, uRegAddr, pReadData);
	if (status) {
	    return status;
	}

    } else { /* indirect read */
#endif
    {
	uBlkBaseAddr = uRegAddr & 0x0000fff0;

	/*  New block so set the block base address. */
	/*  This pointer to the phy block resides at location 0x1f within */
	/*  the HC phy.  Each block contains at most 16 registers (i.e. each block */
	/*  has a 4 bit address space). */
	status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, 0x1f, uBlkBaseAddr);

	if (status) {
	    return status;
	}

	/*  The register offset within the block is given in */
	/*  lower 4 bits. */
	uRegOffset = uRegAddr & 0xf;

	if (uRegAddr > 0xf) {
	    /*  Set bit[4] to tell the HC to use the block base address */
	    /*  register (within the HC) to select the block for this register. */
	    uRegOffset = uRegOffset | (0x1<<4);
	}

	status = soc_bm9600_mdio_hc_cl22_read(unit, uPhyAddr, uRegOffset, pReadData);
	if (status) {
	    return status;
	}
    }
#if 0000
    /* Go back to lane 0 */
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, 0x1f, HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0xfff0);
    if (status) {
        return status;
    }
    status = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, uPhyAddr, (HW_BM9600_HYPERCORE_AER_LANE_SELECT & 0x000f) | (0x1<<4), 0 );
    if (status) {
        return status;
    }
#endif
    return SOC_E_NONE;
}


/***********************************************
 *  Broadcast write to all enabled HyperCore's.
 *  For broadcast writes to indirect reg's we need to
 *  make sure that the base address register in EACH
 *  HC is set to the write address.
 */
int soc_bm9600_mdio_hc_broadcast_write(int unit, uint32 uRegAddr, uint32 uWrData) {
    uint32 uBlkBaseAddr;
    uint32 uRegOffset;
    const uint32 HC_BCAST_PHY_ID = 0x1f;
    int status;

    /*  HC register address space is only 16 bits. */
    if (uRegAddr > 0xffff) {
	return SOC_E_PARAM;
    }

    if (uRegAddr <= 0x1f) {
	/*
	 * direct broadcast write
	 */
	status = soc_bm9600_mdio_hc_cl22_write(unit, HC_BCAST_PHY_ID, uRegAddr, uWrData);

	if (status) {
	    return status;
	}

    } else {
	/*
	 * indirect broadcast write
	 */
	uBlkBaseAddr = uRegAddr & 0x0000fff0;

	/*  New block so broadcast the block base address. */
	status = soc_bm9600_mdio_hc_cl22_write(unit, HC_BCAST_PHY_ID, 0x1f, uBlkBaseAddr);

	if (status) {
	    return status;
	}
	/*  The register offset within the block is given in */
	/*  lower 4 bits. */
	uRegOffset = uRegAddr & 0xf;
	/*  Set bit[4] to tell the HC to use the block base address */
	/*  register (within the HC) to select the block for this register. */
	uRegOffset = uRegOffset | (0x1<<4);

	status =soc_bm9600_mdio_hc_cl22_write(unit, HC_BCAST_PHY_ID, uRegOffset, uWrData);

	if (status) {
	    return status;
	}

    }
    return SOC_E_NONE;
}

/***********************************************
 *  Write to a register in the HyperCore phy over the PI MDIO
 *  interface using clause 22 protocol.
 *  This is a base (low level) function for writing to the HC's.
 *  Users should typically use the soc_bm9600_mdio_hc_write function for normal writes.
 */
int soc_bm9600_mdio_hc_cl22_write(int unit, uint32 uPhyAddr, uint32 uRegAddr, uint32 uData) {
    uint32 uMemAccCtrl;
    uint32 uAck;
    uint32 uTimeOut;

    /*  write data is only 16 bits */
    assert (uData <= 0xffff);

    /*  setup pi.cmic_miim_param register for write transfer */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_PARAM, PHY_ID, uMemAccCtrl, uPhyAddr);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_PARAM, PHY_DATA, uMemAccCtrl, uData);
    SAND_HAL_WRITE(unit, PL, PI_CMIC_MIIM_PARAM, uMemAccCtrl);

    /*  set phy register address */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_ADDRESS, REGADR, uMemAccCtrl, uRegAddr);
    SAND_HAL_WRITE(unit, PL, PI_CMIC_MIIM_ADDRESS, uMemAccCtrl);

    /*  set write enable (self clearing) bit, and override external master control */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, MIIM_WR_START, uMemAccCtrl, 0x1);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, OVER_RIDE_EXT_MDIO_MSTR_CNTRL, uMemAccCtrl, 0x1);
    SAND_HAL_WRITE(unit, PL, PI_CMIC_MIIM_CTRL_STAT, uMemAccCtrl);

    /*  poll for write done          */
    uAck = 0;
    uTimeOut = 0;
    while ( !uAck && (150 > uTimeOut) ) {
	uAck = SAND_HAL_GET_FIELD(PL, PI_MIIM_INT_STATUS, MIIM_OP_DONE, SAND_HAL_READ_POLL(unit, PL, PI_MIIM_INT_STATUS));
	uTimeOut++;
    }
    if ( uTimeOut == 150 ) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "PI_MIIM_INT_STATUS-MIIM_OP_DONE, timeout waiting for ACK on MDIO write.\n")));
	return SOC_E_TIMEOUT;
    } else {
	/*clear interrupt status (W1TC) */
	SAND_HAL_WRITE(unit, PL, PI_MIIM_INT_STATUS, (1<<SAND_HAL_PL_PI_MIIM_INT_STATUS_MIIM_OP_DONE_SHIFT));
    }
    LOG_VERBOSE(BSL_LS_SOC_SOCMEM,
                (BSL_META_U(unit,
                            "soc_bm9600_mdio_hc_cl22_write. uPhyAddr 0x%x uRegAddr 0x%x, uData 0x%x\n"), uPhyAddr, uRegAddr, uData));

    return SOC_E_NONE;
}

/***********************************************
 *  Write to a register in the HyperCore phy over the PI MDIO
 *  interface using clause 22 protocol.
 *  This is a base (low level) function for writing to the HC's.
 *  Users should typically use the soc_bm9600_mdio_hc_write function for normal writes.
 *  This function was created for easy reload where you want to read indirect memory rather than directly.
 */
int soc_bm9600_mdio_hc_cl22_write_easy_reload(int unit, uint32 uPhyAddr, uint32 uRegAddr, uint32 uData) {
    uint32 uMemAccCtrl;
    uint32 uAck;
    uint32 uTimeOut;


    /*  write data is only 16 bits */
    assert (uData <= 0xffff);

    /*  setup pi.cmic_miim_param register for write transfer */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_PARAM, PHY_ID, uMemAccCtrl, uPhyAddr);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_PARAM, PHY_DATA, uMemAccCtrl, uData);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_PARAM, uMemAccCtrl);

    /*  set phy register address */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_ADDRESS, REGADR, uMemAccCtrl, uRegAddr);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_ADDRESS, uMemAccCtrl);

    /*  set write enable (self clearing) bit, and override external master control */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, MIIM_WR_START, uMemAccCtrl, 0x1);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, OVER_RIDE_EXT_MDIO_MSTR_CNTRL, uMemAccCtrl, 0x1);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_CTRL_STAT, uMemAccCtrl);

    /*  poll for write done          */
    uAck = 0;
    uTimeOut = 0;
    while ( !uAck && (150 > uTimeOut) ) {
	uAck = SAND_HAL_GET_FIELD(PL, PI_MIIM_INT_STATUS, MIIM_OP_DONE, SAND_HAL_READ(unit, PL, PI_MIIM_INT_STATUS));
	uTimeOut++;
    }
    if ( uTimeOut == 150 ) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "PI_MIIM_INT_STATUS-MIIM_OP_DONE, timeout waiting for ACK on MDIO write.\n")));
	return SOC_E_TIMEOUT;
    } else {
	/*clear interrupt status (W1TC) */
	SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_MIIM_INT_STATUS, (1<<SAND_HAL_PL_PI_MIIM_INT_STATUS_MIIM_OP_DONE_SHIFT));
    }
    LOG_VERBOSE(BSL_LS_SOC_SOCMEM,
                (BSL_META_U(unit,
                            "soc_bm9600_mdio_hc_cl22_write. uPhyAddr 0x%x uRegAddr 0x%x, uData 0x%x\n"), uPhyAddr, uRegAddr, uData));

    return SOC_E_NONE;
}

/***********************************************
 *  Read from a register in the HyperCore phy over the PI MDIO
 *  interface using clause 22 protocol.
 *  This is a base (low level) function for reading from the HC's.
 *  Users should typically use the soc_bm9600_mdio_hc_read function for normal reads.
 */
int soc_bm9600_mdio_hc_cl22_read(int unit, uint32 uPhyAddr, uint32 uRegAddr, uint32 *pReadData) {
    uint32 uMemAccCtrl;
    uint32 uAck;
    uint32 uTimeOut;

#if 0  /* this is no longer true when used by other PHY drivers */
    /*  phy address is only 5 bits */
    assert (uPhyAddr <= 0x1f);
    /*  reg address is only 5 bits */
    assert (uRegAddr <= 0x1f);
#endif


    /*  setup pi.cmic_miim_param register for read transfer */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_PARAM, PHY_ID, uMemAccCtrl, uPhyAddr);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_PARAM, uMemAccCtrl);

    /*  set phy register address */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_ADDRESS, REGADR, uMemAccCtrl, uRegAddr);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_ADDRESS, uMemAccCtrl);

    /* set read enable (self clearing) bit, and override external master control */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, MIIM_RD_START, uMemAccCtrl, 0x1);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(PL, PI_CMIC_MIIM_CTRL_STAT, OVER_RIDE_EXT_MDIO_MSTR_CNTRL, uMemAccCtrl, 0x1);
    SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_CMIC_MIIM_CTRL_STAT, uMemAccCtrl);

    /* poll for read done and grab read data */
    uAck = 0;
    uTimeOut = 0;
    while ( !uAck && (150 > uTimeOut) ) {
	uAck = SAND_HAL_GET_FIELD(PL, PI_MIIM_INT_STATUS, MIIM_OP_DONE, SAND_HAL_READ(unit, PL, PI_MIIM_INT_STATUS));
	uTimeOut++;
    }
    if ( uTimeOut == 150 ) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "PI_MIIM_INT_STATUS-MIIM_OP_DONE, timeout waiting for ACK on MDIO read.\n")));
	return SOC_E_TIMEOUT;
    } else {
	/* clear interrupt status (W1TC) */
	SAND_HAL_WRITE_EASY_RELOAD(unit, PL, PI_MIIM_INT_STATUS, (1<<SAND_HAL_PL_PI_MIIM_INT_STATUS_MIIM_OP_DONE_SHIFT));

	*pReadData = SAND_HAL_READ(unit, PL, PI_CMIC_MIIM_READ_DATA);
	LOG_VERBOSE(BSL_LS_SOC_SOCMEM,
                    (BSL_META_U(unit,
                                "soc_bm9600_mdio_hc_cl22_read uPhyAddr%d uRegAddr 0x%08x uData 0x%08x\n"),uPhyAddr, uRegAddr, *pReadData));

    }
    return SOC_E_NONE;
}
/* Always set the speed, then the encoding checks are done during encoding set */
int soc_bm9600_hc_speed_set(int unit, int32 nSi, uint32 uSerdesSpeed)
{
    return soc_phyctrl_speed_set(unit, nSi, uSerdesSpeed);
}


/* Always set the speed, then the encoding checks are done during encoding set */
int soc_bm9600_hc_encoding_set(int unit, int32 nSi, int32 bSerdesEncoding)
{
    int speed;
    soc_port_t port;
    uint32 si_config;
    int32 encoding;

    port = nSi;
    encoding = bSerdesEncoding;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_speed_get(unit, port, &speed));

    if (encoding) {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_ENCODING,
                                     phyControlEncoding8b10b));

	si_config  = SAND_HAL_READ_STRIDE(unit, PL, SI, nSi, SI_CONFIG0);
	si_config  = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, RX_BYTE_SWAP, 
                                         si_config, 1);
	si_config  = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, TX_BYTE_SWAP, 
                                         si_config, 1);
	SAND_HAL_WRITE_STRIDE(unit, PL, SI, nSi, SI_CONFIG0, si_config);

	if (speed == 6250) {
	    /* Enable scrambler */
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_set(unit, port, 
                                         SOC_PHY_CONTROL_SCRAMBLER, 1));
	} else {
	    /* Disable 8b10b scrambler only used at 6.25G */
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_set(unit, port, 
                                         SOC_PHY_CONTROL_SCRAMBLER, 0));
        }
    } else {

        si_config  = SAND_HAL_READ_STRIDE(unit, PL, SI, nSi, SI_CONFIG0);
        si_config  = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, RX_BYTE_SWAP, 
                                        si_config, 0);
        si_config  = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, TX_BYTE_SWAP, 
                                        si_config, 0);
        SAND_HAL_WRITE_STRIDE(unit, PL, SI, nSi, SI_CONFIG0, si_config);

        SOC_IF_ERROR_RETURN
            (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_ENCODING,
                                     phyControlEncoding64b66b));

        /* Disable 8b10b scrambler only used at 6.25G */
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_control_set(unit, port, 
                                     SOC_PHY_CONTROL_SCRAMBLER, 0));
        
    }
    return SOC_E_NONE;
}

int
soc_bm9600_xb_test_pkt_get(int unit, int egress, int xb_port, int *cnt)
{
    uint32 uAddress = (xb_port & 0xFF);
    uint64 nnCutoffTime = COMPILER_64_INIT(0,0);

    *cnt = -1;    
    
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_TEST_CNT_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, SELECT, egress) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, RD_WR_N, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ADDRESS, uAddress)
		    );

    while (1) {
	int nData = SAND_HAL_READ_POLL( (sbhandle)unit, PL, XB_TEST_CNT_ACC_CTRL);
	if (SAND_HAL_GET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ACK, nData) == 1 ) {
	    break;
	}
	COMPILER_64_ADD_32(nnCutoffTime,1);
	if( COMPILER_64_LO(nnCutoffTime) > 20 ) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s:  Error reading xb_test_cnt memory for uAddress=0x%08x\n"), FUNCTION_NAME(), (uint32)uAddress));
	    return SOC_E_TIMEOUT;
	}
    }

    *cnt = SAND_HAL_READ((sbhandle)unit, PL, XB_TEST_CNT_ACC_DATA);
    return SOC_E_NONE;
}

int
soc_bm9600_xb_test_pkt_clear(int unit, int egress, int xb_port)
{
    uint32 uAddress = (xb_port & 0xFF);
    uint64 nnCutoffTime = COMPILER_64_INIT(0,0);

    SAND_HAL_WRITE((sbhandle)unit, PL, XB_TEST_CNT_ACC_DATA, 0);

    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_TEST_CNT_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, SELECT, egress) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ADDRESS, uAddress)
		    );

    while (1) {
	int nData = SAND_HAL_READ_POLL( (sbhandle)unit, PL, XB_TEST_CNT_ACC_CTRL);
	if (SAND_HAL_GET_FIELD(PL, XB_TEST_CNT_ACC_CTRL, ACK, nData) == 1 ) {
	    break;
	}
	COMPILER_64_ADD_32(nnCutoffTime,1);
	if( COMPILER_64_LO(nnCutoffTime) > 20 ) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s:  Error writing xb_test_cnt memory for uAddress=0x%08x\n"), FUNCTION_NAME(), (uint32)uAddress));
	    return SOC_E_TIMEOUT;
	}
    }

    return SOC_E_NONE;
}

/* determines whether the BM9600 is capable of being an arbiter */
static int
soc_bm9600_arbiter_capable(int unit, uint8 *bArbiterCapable)
{
    *bArbiterCapable = TRUE;
    return BCM_E_NONE;
}

/* #define DEBUG_SDK30912 1 */
/* xb_iport_channel_ptr is passed in and is a pointer to an array of 6 32 bit words indicating which of 191 channels was reset */
int
soc_bm9600_port_check_policing_errors_reset_xb_port(int unit, uint32 *xb_iport_channel_ptr)
{
 
    int32 i, j;
    int enabled;
    bcm_port_t port;

    if (xb_iport_channel_ptr == 0) {
      return (SOC_E_PARAM);
    }
    
    xb_iport_channel_ptr[0]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR0);  
    xb_iport_channel_ptr[1]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR1);  
    xb_iport_channel_ptr[2]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR2);  
    xb_iport_channel_ptr[3]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR3);  
    xb_iport_channel_ptr[4]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR4);  
    xb_iport_channel_ptr[5]   = SAND_HAL_READ( (sbhandle)unit, PL, XB_NONEMPTY_ERROR5);  


#ifdef DEBUG_SDK30912
    for (j=0; j<6; i++) {
	if (xb_iport_channel_ptr[j]) {
	    LOG_CLI((BSL_META_U(unit,
                                "xb_nonempty_error%d =0x%08x\n"), j, xb_iport_channel_ptr[j]));
	}
    }
#endif

    /* first handle SFI_SCI ports */
    for (j=0; j<6; j++) {
	for (i=0;i<16; i++) {
	    if ( (xb_iport_channel_ptr[j]) & (0x3 << (2*i)) ) {
		
		port = i + (j *16);
		
		SOC_IF_ERROR_RETURN(soc_phyctrl_enable_get(unit, port, &enabled));

		if (enabled == TRUE) {
		    if (IS_SCI_PORT(unit, port)) {		
#ifdef DEBUG_SDK30912
			LOG_CLI((BSL_META_U(unit,
                                            "port(%d) xb error sfi_sci, disable and re-enable port\n"), port));
#endif
			SOC_IF_ERROR_RETURN(bcm_bm9600_port_enable_set(unit, port, FALSE));
			SOC_IF_ERROR_RETURN(bcm_bm9600_port_enable_set(unit, port, TRUE));
		    }
		}
	    }
	}
    }
    
    for (j=0; j<6; j++) {
	for (i=0;i<16; i++) {
	    if ( (xb_iport_channel_ptr[j]) & (0x3 << (2*i)) ) {
	    
		port = i + (j *16);

		SOC_IF_ERROR_RETURN(soc_phyctrl_enable_get(unit, port, &enabled));
	    
		if (enabled == TRUE) {
		    /*SFI_SCI handled above */
		    if (!IS_SCI_PORT(unit, port)) {	

#ifdef DEBUG_SDK30912
		      LOG_CLI((BSL_META_U(unit,
                                          "port(%d) xb error, disable and re-enable port\n"), port));
#endif
		      SOC_IF_ERROR_RETURN(bcm_bm9600_port_enable_set(unit, port, FALSE));
		      SOC_IF_ERROR_RETURN(bcm_bm9600_port_enable_set(unit, port, TRUE));
		    }
		}
	    }
	}
    }

    /* Clear errors until next check */
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR0, xb_iport_channel_ptr[0]);
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR1, xb_iport_channel_ptr[1]);
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR2, xb_iport_channel_ptr[2]);
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR3, xb_iport_channel_ptr[3]);
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR4, xb_iport_channel_ptr[4]);
    SAND_HAL_WRITE( (sbhandle)unit, PL, XB_NONEMPTY_ERROR5, xb_iport_channel_ptr[5]);
    
    return SOC_E_NONE;
}


#endif /* BCM_BM9600_SUPPORT */
