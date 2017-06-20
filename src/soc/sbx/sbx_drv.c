/*
 * $Id: sbx_drv.c,v 1.328.4.1.4.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * NOTE:
 * SOC driver infrastructure cleanup pending.
 */

#include <shared/bsl.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/types.h>
#include <soc/property.h>
#include <soc/drv.h>
#include <soc/ipoll.h>
#include <soc/i2c.h>
#include <soc/mcm/driver.h>     /* soc_base_driver_table */
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/phyctrl.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/wb_db_counter.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/sirius.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>

#include <soc/sbx/hal_pl_auto.h>

#include <soc/sbx/qe2000_mvt.h>

#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <sal/types.h>
#include <sal/compiler.h>
#include <sal/appl/io.h>
#include <sal/core/dpc.h>

#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

/*
 * The table of all known drivers
 * Indexed by supported chips
 */

#if  !defined(BCM_QE2000_A1) || !defined(BCM_QE2000_A2) \
   || !defined(BCM_QE2000_A3) || !defined(BCM_BME3200_B0) \
   || !defined(BCM_BME3200_A0) \
   || !defined(BCM_BM9600_A0) || !defined(BCM_BM9600_A1) \
   || !defined(BCM_BM9600_B0) || !defined(BCM_88230_A0) \
   || !defined(BCM_88230_B0)  || !defined(BCM_88230_C0) \
   || !defined(BCM_88030_A0)  || !defined(BCM_88030_A1) \
   || !defined(BCM_88030_B0)  || !defined(BCM_88030_B1)
static soc_driver_t soc_sbx_driver_none;
#endif

soc_driver_t *soc_sbx_driver_table[] = {
#if defined(BCM_QE2000_A1) || defined(BCM_QE2000_A2) || defined(BCM_QE2000_A3)
    &soc_driver_bcm83200_a3,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BME3200_A0)
    &soc_driver_bcm83332_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BME3200_B0)
    &soc_driver_bcm83332_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BM9600_A0) || defined(BCM_BM9600_A1)
    &soc_driver_bm9600_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BM9600_B0)
    &soc_driver_bm9600_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_A0)
    &soc_driver_bcm88230_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_B0)
    &soc_driver_bcm88230_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_C0)
    &soc_driver_bcm88230_c0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88030_A0)
    &soc_driver_bcm88030_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88030_A1)
    &soc_driver_bcm88030_a1,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88030_B0)
    &soc_driver_bcm88030_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88030_B1)
    &soc_driver_bcm88030_b0,
#else
    &soc_sbx_driver_none,
#endif

};

static char *_soc_module_names[] = SOC_MODULE_NAMES_INITIALIZER;

/* bank swap mutex used for locking pushdowns */
sal_mutex_t bankSwapLock[SOC_MAX_NUM_DEVICES];


char *soc_sbx_module_name(int unit, int module_num)
{
    if (sizeof(_soc_module_names) / sizeof(_soc_module_names[0])
                                                    != (SOC_SBX_WB_MODULE__COUNT - SOC_MODULE_COUNT_START)) {
        int i;

        i = sizeof(_soc_module_names) / sizeof(_soc_module_names[0]) - 1;

        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META(
            "soc_module_name: SOC_MODULE_NAMES_INITIALIZER(%d) and SOC_SBX_WB_MODULE__COUNT(%d) mis-match\n"), 
                i, (SOC_SBX_WB_MODULE__COUNT - SOC_MODULE_COUNT_START)));
        for(;i >= 0;i--) {
            LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META(
            "%2d. module_name %s module_num %d\n"), i, _soc_module_names[i], (i+SOC_MODULE_COUNT_START)));
        }
    }

    if ((module_num >= SOC_SBX_WB_MODULE__COUNT) ||
        (module_num < SOC_MODULE_COUNT_START)) {
        return "unknown";
    } 
    
    return _soc_module_names[(module_num - SOC_MODULE_COUNT_START)];
}


int
soc_sbx_misc_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_sbx_mmu_init(int unit)
{
    return SOC_E_NONE;
}

static soc_functions_t soc_sbx_drv_funs = {
    soc_sbx_misc_init,
    soc_sbx_mmu_init,
    NULL,
    NULL,
    NULL,
};

volatile int __soc_sbx_sirius_state_init = FALSE;

/*
 * Microcode version strings
 */
char *soc_sbx_ucode_versions[SOC_SBX_MAX_UCODE_TYPE] =
    {NULL, NULL, NULL};

static int const soc_sbx_connect_min_util[SBX_MAX_FABRIC_COS] =
{
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100
};

static int const soc_sbx_connect_max_age_time[SBX_MAX_FABRIC_COS] =
{
   32, 240, 240, 240, 240, 240, 240, 240,
  240, 240, 240, 240, 240, 240, 240, 240
};

extern void soc_sbx_intr(void *_unit);

#define UPPER32(x) ((uint32)((((uint64)(x))>>32) & 0xffffffffU))
#define LOWER32(x) ((uint32)(((uint64)(x)) & 0xffffffffU))

int soc_sbx_div64(uint64 x, uint32 y, uint32 *result)
{
    uint64 rem;
    uint64 b;
    uint64 res, d;
    uint32 high;
    uint32 low;

    COMPILER_REFERENCE(low);
    rem =  x;
    COMPILER_64_SET(b, 0, y);
    COMPILER_64_SET(d, 0, 1);

    high = COMPILER_64_HI(rem);
    low = COMPILER_64_LO(rem);

    COMPILER_64_ZERO(res);
    if (high >= y) {
        /* NOTE: Follow code is used to handle 64bits result
         *  high /= y;
         *  res = (uint64) (high << 32);
         *  rem -= (uint64)((high * y) << 32);
         */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_div64: result > 32bits\n")));
        return SOC_E_PARAM;
    }

    while ((!COMPILER_64_BITTEST(b, 63)) &&
	   (COMPILER_64_LT(b, rem)) ) {
	COMPILER_64_ADD_64(b,b);
	COMPILER_64_ADD_64(d,d);
    }

    do {
	if (COMPILER_64_GE(rem, b)) {
	    COMPILER_64_SUB_64(rem, b);
	    COMPILER_64_ADD_64(res, d);
	}
	COMPILER_64_SHR(b, 1);
	COMPILER_64_SHR(d, 1);
    } while (!COMPILER_64_IS_ZERO(d));

    *result = COMPILER_64_LO(res);

    /*
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("%s: divisor 0x%x%8.8x dividor 0x%x result 0x%x\n"),
                 FUNCTION_NAME(), high, low, y, *result));
    */
    return 0;
}

#if defined(BCM_CALADAN3_SUPPORT)
char const* soc_sbx_fte_segment_names[] = SOC_SBX_FTE_SEGMENT_STRINGS;
#endif

/* logic for node/port to qid macros added to ease testing config settings */

int map_np_to_qid(int unit, int node, int port, int numcos) {
   int qid = 0;

   if (SOC_SBX_CONTROL(unit)->ucode_erh == SOC_SBX_G3P1_ERH_ARAD) {
     /* when working with Arad, C3 uses "system port" forwarding, where COS is
	not a part of the destination. */
     numcos = 0;
   }
   
   /* NOTE: standalone feature must be global, so must be consistent for all units */
    if (soc_feature(0, soc_feature_standalone)) {
      if (SOC_IS_SBX_QE2000(unit)) {
	/* only QE2000 TME forced to be 16 queues per logical port */
	qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << 4 ));
      } else {
	qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << (_BITS(numcos))));
      }
    }
    else {
      /* for Polaris chasses - should be no-op otherwise */
       node = (node % 32);
           qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << (_BITS(numcos))));
       }
   return qid;
}

int map_qid_to_np(int unit, int qid, int *node, int *port, int numcos) {
   int qidoffs=0;

   if (SOC_SBX_CONTROL(unit)->ucode_erh == SOC_SBX_G3P1_ERH_ARAD) {
     /* when working with Arad, C3 uses "system port" forwarding, where COS is
	not a part of the destination. */
     numcos = 0;
   }

   /* NOTE: standalone feature must be global, so must be consistent for all units */
    if (soc_feature(0, soc_feature_standalone)) {
      if (SOC_IS_SBX_QE2000(unit)) {
	/* only QE2000 TME forced to be 16 queues per logical port */
	qidoffs =  (((qid) - SBX_UC_QID_BASE) >> 4);
      } else {
	qidoffs =  (((qid) - SBX_UC_QID_BASE) >> (_BITS(numcos)));
      }
    }
    else {
      qidoffs =  (((qid) - SBX_UC_QID_BASE) >> (_BITS(numcos)));
    }
    *node = qidoffs / SBX_MAX_PORTS;
    *port = qidoffs % SBX_MAX_PORTS;
   return 0;
}

int map_ds_id_to_qid(int unit, int ds_id, int numcos) {
    int qid;

    if (soc_feature(0, soc_feature_standalone)) {
        qid = SBX_MC_QID_BASE + (ds_id << 4);
    }
    else {
        qid = SBX_MC_QID_BASE + (ds_id << ((_BITS(numcos) > _BITS(SBX_MAX_COS) ? 3 : _BITS(numcos))));
    }
    return qid;
}

int map_qid_to_ds_id(int unit, int qid, int numcos) {
    int ds_id;

    if (soc_feature(0, soc_feature_standalone)) {
        ds_id = (qid >> 4) - SBX_MC_QID_BASE;
    }
    else {
        ds_id = (qid >> ((_BITS(numcos) > _BITS(SBX_MAX_COS) ? 3 : _BITS(numcos)))) - SBX_MC_QID_BASE;
    }
    return ds_id;
}

int
soc_sbx_hybrid_demarcation_qid_get(int unit)
{
    int qid;


    qid = (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule + 1) *
                           (SOC_SBX_CFG(unit)->cfg_num_nodes) *
                           soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    return(qid);
}

int
soc_sbx_translate_status(sbStatus_t s)
{
    int rv;

    switch (s) {
    case SB_OK:
        rv = SOC_E_NONE;
        break;
    case SB_MAC_DUP:
    case SB_SVID_DUP:
    case SB_LPM_DUPLICATE_ADDRESS:
        rv = SOC_E_EXISTS;
        break;
    case SB_MAC_FULL:
    case SB_MAC_NO_MEM:
    case SB_MAC_COL:
    case SB_SVID_TOO_MANY_ENTRIES:
    case SB_SVID_COL:
    case SB_LPM_OUT_OF_HOST_MEMORY:
    case SB_LPM_OUT_OF_DEVICE_MEMORY:
        rv = SOC_E_FULL;
        break;
    case SB_MAC_NOT_FOUND:
    case SB_SVID_KEY_NOT_FOUND:
    case SB_LPM_ADDRESS_NOT_FOUND:
        rv = SOC_E_NOT_FOUND;
        break;
    case SB_TIMEOUT_ERR_CODE:
        rv = SOC_E_TIMEOUT;
        break;
    default:
        rv = SOC_E_FAIL;
    }

    return rv;
}


int
soc_sbx_info_config(int unit, int dev_id, int drv_dev_id)
{
    soc_info_t          *si;
    soc_sbx_control_t   *sbx;
    soc_control_t       *soc;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    int                 blktype, blk, mem;
    int                 instance;
#endif
    int dev;

    sbx = SOC_SBX_CONTROL(unit);
    soc = SOC_CONTROL(unit);

    si  = &SOC_INFO(unit);
    sal_memset((void *)si, 0, sizeof(soc_info_t));
    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];

    si->fe.min          = si->fe.max          = -1;
    si->ge.min          = si->ge.max          = -1;
    si->xe.min          = si->xe.max          = -1;
    si->ce.min          = si->ce.max          = -1;
    si->cl.min          = si->cl.max          = -1;
    si->xl.min          = si->xl.max          = -1;
    si->xt.min          = si->xt.max          = -1;
    si->il.min          = si->il.max          = -1;
    si->hg.min          = si->hg.max          = -1;
    si->hg_subport.min  = si->hg_subport.max  = -1;
    si->hl.min          = si->hl.max          = -1;
    si->st.min          = si->st.max          = -1;
    si->gx.min          = si->gx.max          = -1;
    si->xg.min          = si->xg.max          = -1;
    si->spi.min         = si->spi.max         = -1;
    si->spi_subport.min = si->spi_subport.max = -1;
    si->sci.min         = si->sci.max         = -1;
    si->sfi.min         = si->sfi.max         = -1;
    si->port.min        = si->port.max        = -1;
    si->ether.min       = si->ether.max       = -1;
    si->all.min         = si->all.max         = -1;


    SOC_PBMP_CLEAR(si->cmic_bitmap);

    SOC_PBMP_CLEAR(si->fe.bitmap);
    SOC_PBMP_CLEAR(si->ge.bitmap);
    SOC_PBMP_CLEAR(si->ce.bitmap);
    SOC_PBMP_CLEAR(si->cl.bitmap);
    SOC_PBMP_CLEAR(si->xl.bitmap);
    SOC_PBMP_CLEAR(si->xt.bitmap);
    SOC_PBMP_CLEAR(si->il.bitmap);
    SOC_PBMP_CLEAR(si->xe.bitmap);
    SOC_PBMP_CLEAR(si->hg.bitmap);
    SOC_PBMP_CLEAR(si->hg_subport.bitmap);
    SOC_PBMP_CLEAR(si->hl.bitmap);
    SOC_PBMP_CLEAR(si->st.bitmap);
    SOC_PBMP_CLEAR(si->gx.bitmap);
    SOC_PBMP_CLEAR(si->xg.bitmap);
    SOC_PBMP_CLEAR(si->spi.bitmap);
    SOC_PBMP_CLEAR(si->spi_subport.bitmap);
    SOC_PBMP_CLEAR(si->sci.bitmap);
    SOC_PBMP_CLEAR(si->sfi.bitmap);
    SOC_PBMP_CLEAR(si->port.bitmap);
    SOC_PBMP_CLEAR(si->ether.bitmap);
    SOC_PBMP_CLEAR(si->all.bitmap);

#ifdef BCM_SBUSDMA_SUPPORT
    soc->max_sbusdma_channels = 0;
    soc->tdma_ch = -1;
    soc->tslam_ch = -1;
    soc->desc_ch = -1;
#endif

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (drv_dev_id) {
#if defined(BCM_QE2000_SUPPORT)
    case QE2000_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_QE2000;
        si->modid_max = 10031;
        sbx->fabtype = SB_FAB_DEVICE_QE2000;
        SOC_CHIP_STRING(unit) = "qe2000";
        sbx->modid_count = 1;
        sbx->init_func = soc_qe2000_init;
        sbx->detach = soc_sbx_qe2000_detach;
        sbx->isr = soc_qe2000_isr;

        soc_qe2000_port_info_config(unit);

        sbx->tx_ring_reg =
            SAND_HAL_KA_PC_TX_RING_PTR_OFFSET;
        sbx->tx_ring_size_reg =
            SAND_HAL_KA_PC_TX_RING_SIZE_OFFSET;
        sbx->tx_ring_producer_reg =
            SAND_HAL_KA_PC_TX_RING_PRODUCER_OFFSET;
        sbx->tx_ring_consumer_reg =
            SAND_HAL_KA_PC_TX_RING_CONSUMER_OFFSET;
        sbx->completion_ring_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_PTR_OFFSET;
        sbx->completion_ring_size_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_SIZE_OFFSET;
        sbx->completion_ring_producer_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_PRODUCER_OFFSET;
        sbx->completion_ring_consumer_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_CONSUMER_OFFSET;
        sbx->rxbuf_size_reg =
            SAND_HAL_KA_PC_RXBUF_SIZE_OFFSET;
        sbx->rxbuf_load_reg =
            SAND_HAL_KA_PC_RXBUF_LOAD0_OFFSET;
        sbx->rxbufs_pop_reg =
            SAND_HAL_KA_PC_RXBUF_FIFO_DEBUG_OFFSET;
        sbx->rxbufs_pop_bit =
            SAND_HAL_KA_PC_RXBUF_FIFO_DEBUG_POP_FIFO_MASK;
        break;
#endif /* BCM_QE2000_SUPPORT */

#if defined(BCM_BME3200_SUPPORT)
    case BME3200_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_BME3200;
        sbx->fabtype = SB_FAB_DEVICE_BM3200;
        SOC_CHIP_STRING(unit) = "bme3200";
        sbx->modid_count = 0;
        sbx->init_func = soc_bm3200_init;
        sbx->isr = soc_bm3200_isr;
        sbx->detach = NULL;

        soc_bm3200_port_info_config(unit);

        break;
#endif /* BCM_BME3200_SUPPORT */
/* MCM added bm9600 section */
#if defined(BCM_BM9600_SUPPORT)
    case BM9600_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_BM9600;
        sbx->fabtype = SB_FAB_DEVICE_BM9600;
        SOC_CHIP_STRING(unit) = "bm9600";
        sbx->modid_count = 0;
        sbx->init_func = soc_bm9600_init;
        sbx->isr = soc_bm9600_isr;
        sbx->detach = NULL;

        soc_bm9600_port_info_config(unit);

        break;
#endif /* BCM_BM9600_SUPPORT */

#if defined(BCM_CALADAN3_SUPPORT)
    case BCM88030_DEVICE_ID:
    case BCM88034_DEVICE_ID:
    case BCM88039_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_CALADAN3;
        si->chip      = 0; /*SOC_INFO_CHIP_TYPE_CALADAN3; - dont set until bitmap is associated*/
        si->modid_max = 31;
        si->num_time_interface = 1;

        /* TBD*/
        SOC_CHIP_STRING(unit) = "caladan3";
        sbx->modid_count = 1;
        sbx->fetype = SOC_SBX_FETYPE_CALADAN3;

        sbx->init_func = soc_sbx_caladan3_init;
        sbx->detach = soc_sbx_caladan3_detach;
        sbx->isr = soc_cmicm_intr;
        si->sci.num = si->sfi.num = 0;
        si->num_cpu_cosq = 48;
        si->num_ucs = 2;

        

        /* initialize port mapping */
	if((!SAL_BOOT_BCMSIM) && (SAL_BOOT_PLISIM)) {
	  /* skip port mapping init for pcid.sim, but not bcm.sim */
	  break;
	}
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_port_info_config(unit));

        break;
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    case BCM88230_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_SIRIUS;
        si->chip = SOC_INFO_CHIP_SIRIUS;
        si->modid_max = 10071;
        si->num_cpu_cosq = 48;
        sbx->fabtype = SB_FAB_DEVICE_SIRIUS;
        SOC_CHIP_STRING(unit) = "sirius";
        sbx->modid_count = 1;
        sbx->init_func = soc_sirius_init;
        sbx->detach = soc_sirius_detach;
        sbx->isr = soc_intr;

        
        switch (dev_id) {
        case BCM88235_DEVICE_ID:
          /* 80Gps/120Mps device, 4HG */
          break;
        case BCM88230_DEVICE_ID:
          /* 50Gps/75Mps device, 4HG */
          break;
        case BCM88239_DEVICE_ID:
          /* 20Gps/30Mps device, 2HG */
          break;
        case BCM56613_DEVICE_ID:
          /* 80Gps/120Mps device, 4HG with limited queue/hqos/buffer etc */
          break;
        default:
          break;
        }
        soc_sirius_port_info_config(unit, drv_dev_id, dev_id);

        break;
#endif /* BCM_SIRIUS_SUPPORT */

    default:
        si->chip_type = 0;
        SOC_CHIP_STRING(unit) = "???";
        sbx->modid_count = 0;
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "soc_sbx_info_config: driver device %04x unexpected\n"),
                  drv_dev_id));
        break;
    }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        /* config for reg/mem access through cmic interface */
        si->arl_block = -1;
        si->mmu_block = -1;
        si->mcu_block = -1;

        si->ipipe_block = -1;
        si->ipipe_hi_block = -1;
        si->epipe_block = -1;
        si->epipe_hi_block = -1;
        si->bsafe_block = -1;
        si->esm_block = -1;
        for (blk = 0; blk < SOC_MAX_NUM_OTPC_BLKS; blk++) {
            si->otpc_block[blk] = -1;
        }

        si->igr_block = -1;
        si->egr_block = -1;
        si->bse_block = -1;
        si->cse_block = -1;
        si->hse_block = -1;

        si->qma_block = -1;
        si->qmb_block = -1;
        si->qmc_block = -1;
        si->bp_block = -1;
        si->cs_block = -1;
        si->eb_block = -1;
        si->ep_block = -1;
        si->es_block = -1;
        si->fd_block = -1;
        si->ff_block = -1;
        si->fr_block = -1;
        si->tx_block = -1;
        si->qsa_block = -1;
        si->qsb_block = -1;
        si->rb_block = -1;
        si->sc_top_block = -1;
        si->sf_top_block = -1;
        si->ts_block = -1;

        sal_memset(si->has_block, 0, sizeof(soc_block_t) * COUNTOF(si->has_block));

        for (blk = 0; blk < SOC_MAX_NUM_CI_BLKS; blk++) {
            si->ci_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_XPORT_BLKS; blk++) {
            si->xport_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_CLPORT_BLKS; blk++) {
            si->clport_block[blk] = -1;
        }

        si->xlport_block = -1;

        for (blk = 0; blk < SOC_MAX_NUM_ILPORT_BLKS; blk++) {
            si->il_block[blk] = -1;
        }

        si->tma_block = -1;
        si->tmb_block = -1;

        for (blk = 0; blk < SOC_MAX_NUM_COP_BLKS; blk++) {
            si->co_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_PR_BLKS; blk++) {
            si->pr_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_PT_BLKS; blk++) {
            si->pt_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_TM_QE_BLKS; blk++) {
            si->tm_qe_block[blk] = -1;
        }


        for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
            instance = SOC_BLOCK_INFO(unit, blk).number;
            si->has_block[blk] = blktype;
            switch (blktype) {
                case SOC_BLK_CMIC:
                    si->cmic_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_BP:
                    si->bp_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_CI:
                    si->ci_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    if (SOC_IS_SBX_SIRIUS(unit)) {
                      SOC_PBMP_PORT_ADD(si->block_bitmap[blk], instance);
                    }
                    break;
                case SOC_BLK_CS:
                    si->cs_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_EB:
                    si->eb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_EP:
                    si->ep_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_ES:
                    si->es_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FD:
                    si->fd_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FF:
                    si->ff_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FR:
                    si->fr_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_GXPORT:
                    si->xport_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TX:
                    si->tx_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMA:
                    si->qma_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMB:
                    si->qmb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMC:
                    si->qmc_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QSA:
                    si->qsa_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QSB:
                    si->qsb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_RB:
                    si->rb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_SC_TOP:
                    si->sc_top_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_SF_TOP:
                    si->sf_top_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TS:
                    si->ts_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_OTPC:
                    si->otpc_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
#if defined(BCM_CALADAN3_SUPPORT)
                case SOC_BLK_CM:
                    si->cm_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_CO:
                    si->co_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    break;
                case SOC_BLK_CX:
                    si->cx_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_LRA:
                    si->lra_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_LRB:
                    si->lrb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_OC:
                    si->oc_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_PB:
                    si->pb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_PD:
                    si->pd_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_PP:
                    si->pp_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_PR:
                    si->pr_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    break;
                case SOC_BLK_PT:
                    si->pt_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    break;
                case SOC_BLK_QM:
                    si->qm_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_RC:
                    si->rc_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TMA:
                    si->tma_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TMB:
                    si->tmb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TP:
                    si->tp_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    break;
                case SOC_BLK_TM_QE:
                    si->tm_qe_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    /* overload instance into port so that reg access works.... */
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    break;
                case SOC_BLK_ETU:
                    si->etu_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_ETU_WRAP:
                    si->etu_wrap_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_XLPORT:
                    si->xlport_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_XTPORT:
                    si->xtport_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_CLPORT:
                    si->clport_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_IL:
                    si->il_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
#endif
            }
            sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                         "%s%d",
                         soc_block_name_lookup_ext(blktype, unit),
                         SOC_BLOCK_INFO(unit, blk).number);
        }
        si->block_num = blk;

        /*
         * Calculate the mem_block_any array for this configuration
         * The "any" block is just the first one enabled
         */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            si->mem_block_any[mem] = -1;
            if (SOC_MEM_IS_VALID(unit, mem)) {
                SOC_MEM_BLOCK_ITER(unit, mem, blk) {
                    si->mem_block_any[mem] = blk;
                    break;
                }
            }
        }

#if defined(BCM_CALADAN3_SUPPORT)
        if(SOC_IS_SBX_CALADAN3(unit)) {
            /* walk through memory list & fill in software defaults for null entry */
            for(mem=0; mem < NUM_SOC_MEM; mem++) {
                if((SOC_MEM_IS_VALID(unit, mem)) &&             \
                   (NULL == SOC_MEM_INFO(unit,mem).null_entry)) {
                    SOC_MEM_INFO(unit,mem).null_entry = _soc_mem_entry_null_zeroes;                
                }
            }
        }
#endif

    }
#ifdef BCM_SBUSDMA_SUPPORT
#if defined(BCM_CALADAN3_SUPPORT)
    if(SOC_IS_SBX_CALADAN3(unit)) {
        soc->max_sbusdma_channels = 3;
        soc->tdma_ch = 0;
        soc->tslam_ch = 1;
        soc->desc_ch = 2;
    }
#endif
#endif

#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT */

    /* elect master unit */
    sbx->master_unit = -1;    
    sbx->num_slaves = 0;
    for (dev = 0; dev < SOC_MAX_NUM_DEVICES; dev++) {
	sbx->slave_units[dev] = -1;
    }

    return SOC_E_NONE;
}


soc_driver_t *
soc_sbx_chip_driver_find(uint16 pci_dev_id, uint8 pci_rev_id)
{
    int                 i;
    soc_driver_t        *d;

    /*
     * Find driver in table.  In theory any IDs returned by
     * soc_cm_id_to_driver_id() should have a driver in the table.
     */
    for (i = 0; i < SOC_SBX_NUM_SUPPORTED_CHIPS; i++) {
         d = soc_sbx_driver_table[i];
         if ((d != NULL) &&
             (d->pci_device == pci_dev_id) &&
             (d->pci_revision == pci_rev_id)) {
             return d;
         }
    }

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("soc_sbx_chip_driver_find: driver in devid table "
                        "not in soc_sbx_driver_table\n")));

    return NULL;
}

void
soc_sbx_get_default(int unit, soc_sbx_config_t *cfg)
{
    int                   spi, port=0, node, intf;
    soc_sbx_control_t    *sbx;
    int                   cos;
    int                   level;
    uint16                dev_id;
    uint8                 rev_id;

    assert(SOC_CONTROL(unit) != NULL);
    assert(SOC_SBX_CONTROL(unit) != NULL);
    assert(SOC_SBX_CFG(unit) != NULL);

    sbx = SOC_SBX_CONTROL(unit);

    soc_cm_get_id(unit, &dev_id, &rev_id);

    /* Global config */
    SOC_SBX_CFG(unit)->DeviceHandle        = sbx->sbhdl;
    SOC_SBX_CFG(unit)->reset_ul            = 1;           /* default to reset */
    SOC_SBX_CFG(unit)->uClockSpeedInMHz    = 0;           /* default to 0 */
    SOC_SBX_CFG(unit)->uFabricConfig       = SOC_SBX_SYSTEM_CFG_INVALID;
                                                          /* default to invalid */
    SOC_SBX_CFG(unit)->bHalfBus            = FALSE;       /* default to full bus mode */
    SOC_SBX_CFG(unit)->bRunSelfTest        = TRUE;        /* default to run self test */
    SOC_SBX_CFG(unit)->uActiveScId         = 0;           /* default to 0 ACTIVE_SWITCH_CONTROLLER */
    SOC_SBX_CFG(unit)->uLinkThresholdIndex = 89;
    SOC_SBX_CFG(unit)->uRedMode            = 0;           /* default to be manual */
    SOC_SBX_CFG(unit)->uMaxFailedLinks     = 2;           /* default to allow 2 links fail */
    SOC_SBX_CFG(unit)->bHybridMode         = FALSE;       /* default to non-hybrid mode */
    SOC_SBX_CFG(unit)->bUcqResourceAllocationMode = FALSE;/* default resource creation mode */
    SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
    SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
    SOC_SBX_CFG(unit)->nDiscardTemplates   = 0;
    SOC_SBX_CFG(unit)->discard_probability_mtu = SOC_SBX_DISCARD_PROBABILITY_MTU_SZ;
    SOC_SBX_CFG(unit)->discard_queue_size = SOC_SBX_DISCARD_QUEUE_SZ;
    SOC_SBX_CFG(unit)->nShaperCount        = 0;           /* default to no Shapers, override if available */
    SOC_SBX_CFG(unit)->bcm_cosq_init       = FALSE;        /* default to initialize VOQs */
    SOC_SBX_CFG(unit)->bTmeMode = FALSE;                  /* Non-TME by default */
    SOC_SBX_CFG(unit)->fabric_egress_setup = TRUE;        /* default creation of egress groups to True */
    SOC_SBX_CFG(unit)->erh_type = 0;
    SOC_SBX_CFG(unit)->epoch_length_in_timeslots = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS;
    COMPILER_64_ZERO(SOC_SBX_CFG(unit)->xbar_link_en); /* fabric disabled */
    SOC_SBX_CFG(unit)->mcgroup_local_start_index = 0;
    SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = 0;
    SOC_SBX_CFG(unit)->max_pkt_len_adj_value = 0;
    SOC_SBX_CFG(unit)->enable_all_egress_nodes = FALSE;
    if (SOC_IS_CALADAN3(unit)) {
        SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule = 53;     
    } else {
        SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule = 32;     /* hybrid mode & FE FTE configuration */
    }
    SOC_SBX_CFG(unit)->is_demand_scale_fixed = FALSE;    /* default no SOC property */
    SOC_SBX_CFG(unit)->fixed_demand_scale = -1;
    SOC_SBX_CFG(unit)->demand_scale = 0;                 /* default to demand scale 0 bits */
    SOC_SBX_CFG(unit)->sp_mode = SOC_SBX_SP_MODE_IN_BAG;
    SOC_SBX_CFG(unit)->local_template_id = SOC_SBX_QOS_TEMPLATE_TYPE0;
    SOC_SBX_CFG(unit)->node_template_id = SOC_SBX_NODE_QOS_TEMPLATE_TYPE0;
    SOC_SBX_CFG(unit)->v4_ena = 1;
    SOC_SBX_CFG(unit)->oam_rx_ena = 0;
    SOC_SBX_CFG(unit)->oam_tx_ena = 0;
    SOC_SBX_CFG(unit)->oam_spi_lb_port = 0;
    SOC_SBX_CFG(unit)->oam_spi_lb_queue = 0;
    SOC_SBX_CFG(unit)->v4mc_str_sel = 0;
    SOC_SBX_CFG(unit)->v4uc_str_sel = 0;
    SOC_SBX_CFG(unit)->uInterfaceProtocol = SOC_SBX_IF_PROTOCOL_XGS;
    SOC_SBX_CFG(unit)->arbitration_port_allocation = SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1;


    /* These values are system maximums for fabric devices.
     * Default this maximums to those in DMode. Later on,
     * these maximums will be adjusted based on the system mode
     */
    SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
    SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
    SOC_SBX_CFG(unit)->num_internal_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
    SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;

    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
    SOC_SBX_CFG(unit)->num_bw_groups = HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS;
    SOC_SBX_CFG(unit)->num_sysports = 0;
    SOC_SBX_CFG(unit)->use_extended_esets = 0;
    SOC_SBX_CFG(unit)->max_ports = SB_FAB_DEVICE_QE2000_MAX_PORT;

    for (cos=0; cos<SBX_MAX_FABRIC_COS; cos++) {
        SOC_SBX_CFG(unit)->bcm_cosq_priority_mode[cos] = BCM_COSQ_BE; /* best effort */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_min_bw_kbps[cos] = 0; /* no bw_group (bag) rate */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_max_bw_kbps[cos] = 0; /* no shape rate */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_weight[cos] = 0; /* no weight */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_min_depth_bytes[cos] = (25*1024);  /* 25kbytes */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_max_depth_bytes[cos] = (1024*1024); /* 1Mbyte */
	SOC_SBX_CFG(unit)->bcm_cosq_priority_group[cos] = 0; /* default mapped to pg 0 */
    }
    SOC_SBX_CFG(unit)->pfc_cos_enable = 0;
    SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps = 1000000; /* 1G bag rate */

    SOC_SBX_CFG(unit)->hold_pri_num_timeslots = -1;  /* default hold depends on queue region */

    SOC_SBX_CFG(unit)->num_ingress_scheduler = 0;
    SOC_SBX_CFG(unit)->num_egress_scheduler = 0;
    SOC_SBX_CFG(unit)->num_ingress_multipath = 0;
    SOC_SBX_CFG(unit)->num_egress_multipath = 0;

    for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
        SOC_SBX_CFG(unit)->connect_min_util[cos] = soc_sbx_connect_min_util[cos];
	if (SOC_IS_SIRIUS(unit)) {
	    SOC_SBX_CFG(unit)->connect_min_util[cos] = SOC_SBX_CFG(unit)->connect_min_util[cos] * 2;
	}
        SOC_SBX_CFG(unit)->connect_max_age_time[cos] = soc_sbx_connect_max_age_time[cos];
    }

    SOC_SBX_CFG(unit)->diag_qe_revid = -1;

    for (cos = 0; cos < SBX_FAB_MAX_MC_FIFOS; cos++) {
        SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] = -1;
    }
    SOC_SBX_CFG(unit)->is_mc_ef_cos[2] = TRUE;
    SOC_SBX_CFG(unit)->is_mc_ef_cos[3] = FALSE;

    SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap = FALSE; /* no overlap, reference errata */

    SOC_SBX_CFG(unit)->module_custom1_in_system = FALSE;
    SOC_SBX_CFG(unit)->module_custom1_links = 0;

    if (SOC_IS_SBX_QE2000(unit)) {

        SOC_SBX_CFG(unit)->uSerdesSpeed = 3125;        /* 3.125G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;       /* 8B10B by default */

        /* QE2000 only config */
        SOC_SBX_CFG_QE2000(unit)->n2one_ul = 0;
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 250;  /* full speed by default */
        SOC_SBX_CFG_QE2000(unit)->bHalfBus = FALSE;        /* half bus mode for the qe device only */
        SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest = FALSE;
        SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2 = TRUE;
        SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;          /* By default 48k mvt entries */
        SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull = TRUE;
        SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat = SBX_MVT_FORMAT0;
        SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout = SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT;
        SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout = SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT;
        SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout = 500;
        SOC_SBX_CFG_QE2000(unit)->uScGrantoffset = 0xE;

        if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
            SOC_SBX_CFG_QE2000(unit)->uScGrantDelay = 0x3E; /* default for qe2000/sirius interop */
        } else {
            SOC_SBX_CFG_QE2000(unit)->uScGrantDelay = 0x0; /* default for QE2000 only  */
        }
        SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes = 20;  /* by default, adjust length for ethernet IPG */
        SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs = 20000;
        SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows = TRUE;
        SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0x96;
        SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = -1;
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0] = SOC_SBX_PORT_MODE_BURST_IL;
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1] = SOC_SBX_PORT_MODE_BURST_IL;
        SOC_SBX_CFG_QE2000(unit)->uEiLines[0] = 768;
        SOC_SBX_CFG_QE2000(unit)->uEiLines[1] = 768;
        SOC_SBX_CFG_QE2000(unit)->nodeNum_ul = 0;          /* Node 0 by default */
        SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes = SB_FAB_USER_MAX_NUM_NODES;
        SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress = 1;  /* Default to be per queue shaping */
        SOC_SBX_CFG_QE2000(unit)->uSfiDataLinkInitMask = 0;     /* No QE1k links supported */
        SOC_SBX_CFG_QE2000(unit)->bEpDisable = FALSE;
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0] = 0;         /* Default, Power On Reset value */
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1] = 0;         /* Default, Power on Reset value */
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1] = 0;

        for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {

            SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = -1;
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = -1;
            SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[port] = FALSE;
            SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[port] = FALSE;
        }

        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1] = 0;
        COMPILER_64_ZERO(SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0]);
        COMPILER_64_ZERO(SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1]);


        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_SEL;
        SOC_SBX_CFG(unit)->max_pkt_len_adj_value = SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_VALUE;

        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            for(port=0; port<SB_FAB_DEVICE_QE2000_MAX_SPI_SUBPORTs; port++){
                SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[spi][port] = 0;
            }
        }

        for(port=0; port<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port]));
        }

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_QE2000_MAX_WRED_TEMPLATES;
        SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
        SOC_SBX_CFG(unit)->nShaperCount = SB_FAB_DEVICE_QE2000_MAX_EGRESS_SHAPERS;


        SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness = 1;

        SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = 0;

    } else if (SOC_IS_SBX_BME3200(unit)) {
        /* BM3200 only config */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 200;  
        SOC_SBX_CFG_BM3200(unit)->uDeviceMode = 0;
        SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId = 0;
        SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId = 0;
        SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG(unit)->uSerdesSpeed = 3125;          /* 3.125G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;       /* 8B10B by default */

        for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port]));
            SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
        }

        /*
         * Default to SCIs on 0 & 1 because the BME3200 requires SCI ports
         * starting at port 0
         */
        SOC_SBX_CFG_BM3200(unit)->nNumLinks = 0;
        SOC_SBX_CFG_BM3200(unit)->uSerializerMask = 0;
        SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0;          /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0;          /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;             /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;             /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed = FALSE;
        sal_memset(SOC_SBX_CFG_BM3200(unit)->uLcmXcfg, 0,            /* Unused for BM3200 */
                   (size_t)(SB_FAB_MAX_NUM_DATA_PLANES *  \
                            SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS *      \
                            sizeof(uint32)));

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_BM3200_MAX_WRED_TEMPLATES;
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /* BM9600 only config */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 200;  /* full speed by default */
        SOC_SBX_CFG_BM9600(unit)->uDeviceMode = 0;
        SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = FALSE;
        SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId = 0;
        SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId = 0;
        SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG(unit)->uSerdesSpeed = 6250;         /* 6.250G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;      /* 8b10b by default */
        SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit = -1;      /* 8b10b by default */

        for (port=0; port< SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++) {
            SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] = SOC_PORT_ABILITY_SFI;
        }

        for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port]));
            SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
        }


        /*
         * Default to SCIs on 0 & 1 because the BME3200 requires SCI ports
         * starting at port 0
         */
        SOC_SBX_CFG_BM9600(unit)->nNumLinks = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;
        SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed = FALSE;
        sal_memset(SOC_SBX_CFG_BM9600(unit)->uLcmXcfg, 0,
                   (size_t)(SB_FAB_MAX_NUM_DATA_PLANES *  \
                            SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS *      \
                            sizeof(uint32)));

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_BM9600_MAX_WRED_TEMPLATES;
        SOC_SBX_CFG_BM9600(unit)->cached_ina = -1;
        SOC_SBX_CFG_BM9600(unit)->arbiterUnusedLinkForSfiSciXcfgRemap = soc_property_get(unit, "unused_link_for_sfi_sci", 0);

        if (SOC_SBX_CFG_BM9600(unit)->arbiterUnusedLinkForSfiSciXcfgRemap >= SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS) {
            /* Override invalid value */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_get_default: unit %d unused_link_for_sfi_sci(%d) out of range max(%d)\n"),
                       unit, SOC_SBX_CFG_BM9600(unit)->arbiterUnusedLinkForSfiSciXcfgRemap, SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS));
        }

    } else if (SOC_IS_SBX_SIRIUS(unit)) {
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 405;        /* default to 405MhZ for 80G */

        /* default to 1k col, 8k rol, 1G memory */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns = 1024;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows = 8192;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks = 8;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories = 10; /* default to 10 ddr devices */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz = 667;   /* 667 Mhz */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade = 0;     /* 9-9-9 grade */
        SOC_SBX_CFG_SIRIUS(unit)->uMaxBuffers = 256 * 1024;     /* match other ddr settings */

        SOC_SBX_CFG_SIRIUS(unit)->ucast_cos_map = _SIRIUS_I_COS_PROFILE_GENERAL;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_cos_map = _SIRIUS_I_COS_PROFILE_GENERAL;

        SOC_SBX_CFG(unit)->uSerdesSpeed = 6250;           /* 6.250G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;        /* 8b10b by default */

        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_SEL;
        SOC_SBX_CFG(unit)->max_pkt_len_adj_value = SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_VALUE;

        for (port=0; port< SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++) {
            SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[port] = SOC_PORT_ABILITY_DUAL_SFI;
        }

        for(port=0; port<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port]));
        }

        /* Limited by the SIRIUS to 16K for now, sirius max is 32k VOQs */
        SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = BM9600_BW_MAX_VOQ_NUM;
        /* In BCMSIM, DualLocalGrants is not supported */
        SOC_SBX_CFG_SIRIUS(unit)->bDualLocalGrants = (SAL_BOOT_BCMSIM? FALSE : TRUE);

        /* default to 4 fifos for each fifo group as what hardware naturally supports */
        SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize =  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE;
        for(intf = 0; intf < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; intf++) {
            SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf] = 0;
            SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf] = 0;
        }
        sal_memset(SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed, 0, sizeof(SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed));
        SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts = 0;
	SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = FALSE;
        SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit = -1;
        SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports = 0;
        SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo = SB_FAB_XCORE_COS_FIFO_UNICAST_EF;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo = SB_FAB_XCORE_COS_FIFO_MULTICAST_EF;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo = SB_FAB_XCORE_COS_FIFO_MULTICAST_NEF;

        /* by default no ingress scheduler is used, all egress schedulers are used */
        for(level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] = 0;
        }
        SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] = 1; /* root only has 1 scheduler */

        /* default 0 - root_relay parent may have single child */
        SOC_SBX_CFG_SIRIUS(unit)->tsChildPassThroughDisable = soc_property_get(unit, "ts_child_passthrough_disable", 0);


        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[0] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[2] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[3] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[4] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L4;
        /* support level 1/2 egress schedulers need to be allocated */
        SOC_SBX_CFG(unit)->num_egress_scheduler = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 +
	    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
        SOC_SBX_CFG(unit)->num_egress_multipath = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER;

	SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = -1; /* unknown mode at default */

        SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = -1; /* unknown mode at default */

        

        SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs = 80000;
        SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos = 8;            /* default to 8 cos levels */
        SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize = FALSE;  /* default to optimize level usage */

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_SIRIUS_MAX_WRED_TEMPLATES;

        for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; port++) {
            
            /* by default, adjust length for ethernet interframe gap */
	    if (soc_property_get(unit, spn_IF_PROTOCOL, 1) == 1) {
		SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[port] = 10;
	    } else { /* in XGS mode, default shapingBusLengthAdj to 0 */
		SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[port] = 5;
	    }
        }

        level = soc_property_get(unit, spn_QE_MVR_MAX_SIZE, 5);
        if ((1 > level) || (5 < level)) {
            
            level = 5;
        }
        SOC_SBX_CFG_SIRIUS(unit)->mvrMaxSize = level;

        level = soc_property_get(unit, spn_QE_MC_DUAL_LOOKUP, 0);
        if (level) {
            SOC_SBX_CFG_SIRIUS(unit)->dualLookup = TRUE;
        } else {
            SOC_SBX_CFG_SIRIUS(unit)->dualLookup = FALSE;
        }

        level = soc_property_get(unit, spn_QE_LAG_UC_REDIST, 0);
        if (level) {
            SOC_SBX_CFG_SIRIUS(unit)->redirectUcLag = TRUE;
        } else {
            SOC_SBX_CFG_SIRIUS(unit)->redirectUcLag = FALSE;
        }
    } else if (SOC_IS_CALADAN3(unit)) {
        if (dev_id == BCM88034_DEVICE_ID) {
            SOC_SBX_CFG(unit)->uClockSpeedInMHz = 742;        /* default to 742MhZ */
        } else if (dev_id == BCM88039_DEVICE_ID) {
            SOC_SBX_CFG(unit)->uClockSpeedInMHz = 1100;        /* default to 1100MhZ */
        } else {
            SOC_SBX_CFG(unit)->uClockSpeedInMHz = 1000;        /* default to 1000MhZ */
        }
    }

    for (node = 0; node < SBX_MAXIMUM_NODES; node++) {
        /* setup one to one mapping for logical node to physical node */
        SOC_SBX_CFG(unit)->l2p_node[node] = node;
        SOC_SBX_CFG(unit)->p2l_node[node] = node;
    }

    SOC_SBX_CFG(unit)->uRateClockSpeed = SIRIUS_RATE_CLOCK;
    SOC_SBX_CFG(unit)->uMaxClocksInEpoch = SIRIUS_MAX_CLOCKS_PER_EPOCH;

    /* all lxbars below 18 must be the interop xbars shared between QE and SS */
    SOC_SBX_CFG(unit)->max_interop_xbar = SB_FAB_DEVICE_QE2000_SFI_LINKS;
}

static void
soc_sbx_intr_disable(int unit, uint16 dev_id, uint32 mask)
{

}

int
soc_sbx_attach(int unit)
{
    soc_sbx_control_t   *sbx;
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;
    soc_control_t       *soc;
    soc_persist_t       *sop;
    soc_port_t          port;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    int                 mem;
    int                 ix;
    int                 rv = SOC_E_MEMORY;
    int                 cmc;
#endif
#ifdef BCM_CMICM_SUPPORT
    int                 arm, i;
#endif

    LOG_INFO(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "soc_attach: unit %d\n"), unit));

    /*
     * Allocate soc_control and soc_persist if not already.
     */

    soc = SOC_CONTROL(unit);
    if (SOC_CONTROL(unit) == NULL) {
        SOC_CONTROL(unit) =
            sal_alloc(sizeof (soc_control_t), "soc_control");
        if (SOC_CONTROL(unit) == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_CONTROL(unit), 0, sizeof (soc_control_t));
    } else {
        if (soc->soc_flags & SOC_F_ATTACHED) {
            return(SOC_E_NONE);
        }
    }

    SOC_PERSIST(unit) = sal_alloc(sizeof (soc_persist_t), "soc_persist");
    if (NULL == SOC_PERSIST(unit)) {
        return SOC_E_MEMORY;
    }

    sal_memset(SOC_PERSIST(unit), 0, sizeof (soc_persist_t));

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);
    sop->version = 1;

    /* Sbx control structure
     */
    if (SOC_SBX_CONTROL(unit) == NULL) {
        SOC_CONTROL(unit)->drv = (soc_sbx_control_t *)
                                 sal_alloc(sizeof(soc_sbx_control_t),
                                           "soc_sbx_control");

        if (SOC_SBX_CONTROL(unit) == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_SBX_CONTROL(unit), 0, sizeof (soc_sbx_control_t));
        sbx = SOC_SBX_CONTROL(unit);
        sbx->cfg = NULL;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    /* make sure state information is correctly initialised */
    if (SOC_IS_SBX_SIRIUS(unit)) {
        if (!__soc_sbx_sirius_state_init) {
            sal_memset(&(_soc_sbx_sirius_state[0]),
                       0x00,
                       sizeof(_soc_sbx_sirius_state[0]) * SOC_MAX_NUM_DEVICES);
            __soc_sbx_sirius_state_init = TRUE;
        }
    }

#endif /* defined(BCM_SIRIUS_SUPPORT) */

    sbx = SOC_SBX_CONTROL(unit);
    sbx->sbhdl = (sbhandle)unit;

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

    soc->chip_driver = soc_sbx_chip_driver_find(dev_id_driver, rev_id_driver);

    
    if (soc->chip_driver == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d has no driver "
                              "(device 0x%04x rev 0x%02x)\n"),
                   unit, dev_id, rev_id));
        return SOC_E_UNAVAIL;
    }

    soc->soc_functions = &soc_sbx_drv_funs;

#if defined(BCM_SIRIUS_SUPPORT)
    
    sbx->sbx_functions.sirius_ddr_clear = soc_sbx_sirius_ddr23_clear;
#endif

    /*
     * Default config
     */
    if (sbx->cfg == NULL) {
        sbx->cfg = (soc_sbx_config_t *)
                   sal_alloc(sizeof(soc_sbx_config_t),
                             "soc_sbx_config");
        if (sbx->cfg == NULL) {
             return SOC_E_MEMORY;
        }
        sal_memset(sbx->cfg, 0, sizeof (soc_sbx_config_t));
    }

    SOC_SBX_CFG(unit)->bTmeMode = soc_property_get(unit, spn_QE_TME_MODE, 0);

    /* Set feature cache, since used by mutex creation */
    if (SOC_DRIVER(unit)->feature) {
        soc_feature_init(unit);
    }

    /* Create MIIM mutex */
    if ((soc->miimMutex = sal_mutex_create("MIIM")) == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d unable to create miimMutex\n"),
                   unit));
        return SOC_E_MEMORY;
    }

    if (soc_sbx_info_config(unit, dev_id, dev_id_driver) != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d unable to init config\n"),
                   unit));
        return SOC_E_INTERNAL;
    }

    sbx->dma_sem = sal_sem_create("DMA done semaphore", sal_sem_BINARY, 0);
    if (!sbx->dma_sem) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d unable to create dma_sem\n"),
                   unit));
        return SOC_E_MEMORY;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    
    if (SOC_IS_SBX_SIRIUS(unit)) {
        soc_intr_disable(unit, ~0);

        /* get rid of old copy if this exists */
        soc_sirius_state_cleanup(unit);
        /* allocate local state information if needed */
        SOC_SBX_SIRIUS_STATE(unit) = sal_alloc(sizeof(*SOC_SBX_SIRIUS_STATE(unit)),
                                               "Sirius SOC state");
        if (SOC_SBX_SIRIUS_STATE(unit)) {
            sal_memset(SOC_SBX_SIRIUS_STATE(unit),
                       0x00,
                       sizeof(*SOC_SBX_SIRIUS_STATE(unit)));
        } else {
            return SOC_E_MEMORY;
        }
    } /* if (SOC_IS_SBX_SIRIUS(unit)) */
#endif


    /* Install the Interrupt Handler */
    /* Make sure interrupts are masked before connecting line. */
    SOC_PCI_CMCS_NUM(unit) = soc_property_uc_get(unit, 0, spn_PCI_CMCS_NUM, 1);
#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
        SOC_PCI_CMC(unit) = soc_property_uc_get(unit, 0, spn_CMC, 0);
	SOC_CMCS_NUM(unit) = 3;
        arm = 0;
        for (i = 0; i < SOC_CMCS_NUM(unit); i++) {
            
            if (SOC_PCI_CMC(unit) != i) {
                SOC_ARM_CMC(unit, arm) = i;
                arm++;
            }
        }

	/* disable interrupts */
        soc_cmicm_intr0_disable(unit, ~0);
        soc_cmicm_intr1_disable(unit, ~0);
        soc_cmicm_intr2_disable(unit, ~0);
        soc_cmicm_intr3_disable(unit, ~0);
        soc_cmicm_intr4_disable(unit, ~0);
        soc_cmicm_cmcx_intr0_disable(unit, SOC_ARM_CMC(unit, 0), ~0);
        soc_cmicm_cmcx_intr0_disable(unit, SOC_ARM_CMC(unit, 1), ~0);
    } else
#endif
    {
        /* disable interrupts before connecting isr */
        soc_sbx_intr_disable(unit, dev_id, ~0);
    }

    if (sbx->isr != NULL) {
#if defined(BCM_SIRIUS_SUPPORT)
        if (soc_property_get(unit, spn_POLLED_IRQ_MODE, 0)) {
            if (soc_ipoll_connect(unit, sbx->isr, INT_TO_PTR(unit)) < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_attach: soc_attach: unit %d"
                                      " polled interrupt connect failed\n"),
                           unit));
                return SOC_E_INTERNAL;
            }
            soc->soc_flags |= SOC_F_POLLED;
        }
        else
#endif
        {
            /* unit # is ISR arg */
            if (soc_cm_interrupt_connect(unit, sbx->isr,
                                         INT_TO_PTR(unit)) < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_attach: unit %d interrupt"
                                      " connect failed\n"), unit));
                return SOC_E_INTERNAL;
            }
        }
    }

    if (soc->soc_flags & SOC_F_ATTACHED) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d already attached\n"),
                   unit));
        return(SOC_E_NONE);
    }

    /* Initialize information in soc persistent structure */
    sop = SOC_PERSIST(unit);
    SOC_PBMP_CLEAR(sop->link_fwd);

    /*
     * Configure nominal IPG register settings.
     * By default the IPG should be 96 bit-times.
     */
    PBMP_ALL_ITER(unit, port) {
        sop->ipg[port].hd_10    = 96;
        sop->ipg[port].hd_100   = 96;
        sop->ipg[port].hd_1000  = 96;
        sop->ipg[port].hd_2500  = 96;

        sop->ipg[port].fd_10    = 96;
        sop->ipg[port].fd_100   = 96;
        sop->ipg[port].fd_1000  = 96;
        sop->ipg[port].fd_2500  = 96;
        sop->ipg[port].fd_10000 = 96;
        sop->ipg[port].fd_xe    = 96;
        sop->ipg[port].fd_hg    = 64;

    }

    soc_sbx_get_default(unit, sbx->cfg);

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        if ((soc->socControlMutex = sal_mutex_create("SOC_CONTROL")) == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d unable to create schanMutex\n"),
                       unit));
            return SOC_E_MEMORY;
        }
 
        if ((soc->schanMutex = sal_mutex_create("SCHAN")) == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d unable to create schanMutex\n"),
                       unit));
            return SOC_E_MEMORY;
        }

#ifdef BCM_CMICM_SUPPORT
	if (soc_property_get(unit, spn_FSCHAN_ENABLE, 0)) {
	    if ((soc->fschanMutex = sal_mutex_create("FSCHAN")) == NULL) {
		return SOC_E_MEMORY;
	    }
	} else {
	    soc->fschanMutex = NULL;
	}
#endif

        /* create mutex for memory access */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            /*
             * should only create mutexes for valid memories, but at least
             * PTABLE's mutex is used (in PORT_LOCK)
             */
            if ((soc->memState[mem].lock =
                 sal_mutex_create(SOC_MEM_NAME(unit, mem))) == NULL) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_attach: unit %d unable to create Mutex for mem %s\n"),
                           unit, SOC_MEM_NAME(unit, mem)));
            }

            if (!SOC_MEM_IS_VALID(unit, mem)) {
                continue;
            }

            /* Set cache copy pointers to NULL */

            sal_memset(soc->memState[mem].cache,
                       0,
                       sizeof (soc->memState[mem].cache));
        }

#ifdef INCLUDE_MEM_SCAN
        if ((soc->mem_scan_notify =
             sal_sem_create("memscan timer", sal_sem_BINARY, 0)) == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d failed\n"), unit));
            /* coverity[stack_use_overflow] */
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        soc->mem_scan_pid = SAL_THREAD_ERROR;
        soc->mem_scan_interval = 0;
#endif

        /* dma */

        rv = soc_sbusdma_lock_init(unit);
        if (rv != SOC_E_NONE) {
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

#ifdef BCM_SBUSDMA_SUPPORT
    if (soc_feature(unit, soc_feature_sbusdma)) {
        if (soc_sbusdma_init(unit, soc_property_get(unit, spn_DMA_DESC_TIMEOUT_USEC, 0),
                                  soc_property_get(unit, spn_DMA_DESC_INTR_ENABLE, 0))) {
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }
    }

#endif /* BCM_SBUSDMA_SUPPORT */

#ifdef BCM_CMICM_SUPPORT
        for (cmc = 0; cmc < SOC_CMCS_NUM_MAX; cmc++) {
            soc->ccmDmaMutex[cmc] = NULL;
            soc->ccmDmaIntr[cmc] = NULL;
        }
        if (soc_feature(unit, soc_feature_cmicm) &&
            soc_property_get(unit, spn_CCM_DMA_ENABLE, 1)) {
            if (SAL_BOOT_QUICKTURN) {
                soc->ccmDmaTimeout = CCMDMA_TIMEOUT_QT;
            } else {
                soc->ccmDmaTimeout = CCMDMA_TIMEOUT;
            }
            soc->ccmDmaTimeout = soc_property_get(unit, spn_CCMDMA_TIMEOUT_USEC,
                                                  soc->ccmDmaTimeout);
            if (soc->ccmDmaTimeout) {
                for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit); cmc++) {
                    soc->ccmDmaMutex[cmc] = sal_mutex_create("ccmDMA");
                    if (soc->ccmDmaMutex[cmc] == NULL) {
                        return SOC_E_MEMORY;
                    }
                    soc->ccmDmaIntr[cmc] = sal_sem_create("CCMDMA interrupt",
                                                     sal_sem_BINARY, 0);
                    if (soc->ccmDmaIntr[cmc] == NULL) {
                        return SOC_E_MEMORY;
                    }
                    soc->ccmDmaIntrEnb = soc_property_get(unit,
                                                          spn_CCMDMA_INTR_ENABLE, 1);
                }
            }
        }
#endif /* CMICM Support */

        /* mem command */
        if (soc_feature(unit, soc_feature_mem_cmd)) {
            soc->memCmdTimeout = soc_property_get(unit,
                                                  spn_MEMCMD_TIMEOUT_USEC,
                                                  1000000);
            soc->memCmdIntrEnb = soc_property_get(unit,
                                                  spn_MEMCMD_INTR_ENABLE, 0);
            for (ix = 0; ix < 3; ix++) {
                if ((soc->memCmdIntr[ix] =
                     sal_sem_create("MemCmd interrupt", sal_sem_BINARY, 0)) == NULL) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_attach: unit %d failed\n"), unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
            }
        } else {
            for (ix = 0; ix < 3; ix++) {
                soc->memCmdIntr[ix] = 0;
            }
        }

        /* fifo pop dma */
        
        if (soc_feature(unit, soc_feature_fifo_dma)) {
            soc->l2modDmaIntrEnb =
                soc_property_get(unit, spn_L2MOD_DMA_INTR_ENABLE, 0);
        }

        if ((soc->counterMutex = sal_mutex_create("Counter")) == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d failed\n"), unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        if (soc_ndev_attached++ == 0) {
            int                     chip;

            /* Work to be done before the first SOC device is attached. */
            for (chip = 0; chip < SOC_NUM_SUPPORTED_CHIPS; chip++) {
                /* Call each chip driver's init function */
                if (soc_base_driver_table[chip]->init) {
                    (soc_base_driver_table[chip]->init)();
                }
            }
        }

        /*
         * Set up port bitmaps.  They are also set up on each soc_init so
         * they can be changed from a CLI without rebooting.
         */
#if defined(BCM_SIRIUS_SUPPORT) 
        if (SOC_IS_SBX_SIRIUS(unit)) {
            soc_sbx_info_config(unit, dev_id, dev_id_driver);
        }
#endif

        soc_dcb_unit_init(unit);

        /*
         * Initialize memory index_maxes. Chip specific overrides follow.
         */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                sop->memState[mem].index_max = SOC_MEM_INFO(unit, mem).index_max;
            } else {
                sop->memState[mem].index_max = -1;
            }
        }

        

        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                uint32      max;
                uint32      max_adj;
                char        mem_name[100];
                char        *mptr;

                max = sop->memState[mem].index_max;
		/* coverity[secure_coding] */
                sal_strcpy(mem_name, "memmax_");
                mptr = &mem_name[sal_strlen(mem_name)];
		/* coverity[secure_coding] */
                sal_strcpy(mptr, SOC_MEM_NAME(unit, mem));
                max_adj = soc_property_get(unit, mem_name, max);
                if (max_adj == max) {
		  /* coverity[secure_coding] */
                    sal_strcpy(mptr, SOC_MEM_UFNAME(unit, mem));
                    max_adj = soc_property_get(unit, mem_name, max);
                }
                if (max_adj == max) {
		  /* coverity[secure_coding] */
                    sal_strcpy(mptr, SOC_MEM_UFALIAS(unit, mem));
                    max_adj = soc_property_get(unit, mem_name, max);
                }
                sop->memState[mem].index_max = max_adj;
            }
        }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        
        if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
            /* Allocate counter module resources */
            if (soc_counter_attach(unit)) {

                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_attach: unit %d failed\n"), unit));

                soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                return rv;
            }
        }
#endif /* BCM_SIRIUS_SUPPORT or BCM_CALADAN3_SUPPORT */
        
        /*
         * Create binary semaphores for interrupt signals, initially empty
         * making us block when we try to "take" on them.  In soc_intr(),
         * when we receive the interrupt, a "give" is performed, which will
         * wake us back up.
         */

        for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++) {
            if ((soc->schanIntr[cmc] =
                sal_sem_create("SCHAN interrupt", sal_sem_BINARY, 0)) == NULL) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d failed\n"), unit));
                soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                return rv;
            }
        }

        if ((soc->miimIntr =
             sal_sem_create("MIIM interrupt", sal_sem_BINARY, 0)) == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_attach: unit %d failed\n"), unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        /* Initialize target device */
        if (SAL_BOOT_QUICKTURN) {
            soc->schanTimeout = SCHAN_TIMEOUT_QT;
            soc->miimTimeout = MIIM_TIMEOUT_QT;
            soc->bistTimeout = BIST_TIMEOUT_QT;
        } else if (SAL_BOOT_PLISIM) {
            soc->schanTimeout = SCHAN_TIMEOUT_PLI;
            soc->miimTimeout = MIIM_TIMEOUT_PLI;
            soc->bistTimeout = BIST_TIMEOUT_PLI;
        } else {
            soc->schanTimeout = SCHAN_TIMEOUT;
            soc->miimTimeout = MIIM_TIMEOUT;
            soc->bistTimeout = BIST_TIMEOUT;
        }

        soc->schanTimeout = soc_property_get(unit, spn_SCHAN_TIMEOUT_USEC,
                                             soc->schanTimeout);
        soc->miimTimeout = soc_property_get(unit, spn_MIIM_TIMEOUT_USEC,
                                            soc->miimTimeout);
        soc->bistTimeout = soc_property_get(unit, spn_BIST_TIMEOUT_MSEC,
                                            soc->bistTimeout);

        soc->schanIntrEnb = soc_property_get(unit, spn_SCHAN_INTR_ENABLE, 1);
        soc->schanIntrBlk = soc_property_get(unit, spn_SCHAN_ERROR_BLOCK_USEC,
                                             250000);

        /* soc->miimIntrEnb = soc_property_get(unit, spn_MIIM_INTR_ENABLE, 1); */
        soc->miimIntrEnb = soc_property_get(unit, spn_MIIM_INTR_ENABLE, 0);
    }

    /* Initialize SCHAN */
    rv = soc_schan_init(unit);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d failed\n"), unit));
        soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
        return rv;
    }

#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */

    soc->soc_flags |= SOC_F_ATTACHED;

    return SOC_E_NONE;
}


int
soc_sbx_detach(int unit)
{
    soc_control_t       *soc;
    soc_sbx_control_t   *sbx;
    uint32              dev_type;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    soc_mem_t           mem;
    int                 ix;
    int                 cmc;
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    if (NULL == soc) {
        return SOC_E_NONE;
    }

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        return SOC_E_UNIT;
    }

    if (bankSwapLock[unit] != NULL) {
        sal_mutex_destroy(bankSwapLock[unit]);
        bankSwapLock[unit] = NULL;
    }

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

    if (SOC_IS_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        

        if (0 == (soc->soc_flags & SOC_F_ATTACHED)) {
            return SOC_E_NONE;
        }

        /* Free up any memory used by the I2C driver */

#ifdef  INCLUDE_I2C
        (void)soc_i2c_detach(unit);
#endif

        /* Clear all outstanding DPCs owned by this unit */
        sal_dpc_cancel(INT_TO_PTR(unit));

        /*
         * Call soc_init to cancel link scan task, counter DMA task,
         * outstanding DMAs, interrupt generation, and anything else the
         * driver or chip may be doing.
         */

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        SOC_DETACH(unit, TRUE);
    }
#endif
#if !defined(PLISIM)
        soc_sbx_reset_init(unit);
#endif
#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        SOC_DETACH(unit, FALSE);
    }
#endif

        /*
         * PHY drivers and ID map
         */
        SOC_IF_ERROR_RETURN(soc_phyctrl_software_deinit(unit));

        /* Free up DMA memory */
        (void)soc_dma_detach(unit);

        /* Shutdown polled interrupt mode if active */
        soc_ipoll_disconnect(unit);
        soc->soc_flags &= ~SOC_F_POLLED;

        /* Detach interrupt handler, if we installed one */
        /* unit # is ISR arg */
        if (soc_cm_interrupt_disconnect(unit) < 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_detach: could not disconnect interrupt line\n")));
            return SOC_E_INTERNAL;
        }

        /* Terminate counter module; frees allocated space */
        if (SOC_IS_SIRIUS(unit)) {
            soc_sbx_counter_detach(unit);
        }
        soc_counter_detach(unit);

        if (soc->counterMutex) {
            sal_mutex_destroy(soc->counterMutex);
            soc->counterMutex = NULL;
        }

        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                /* Deallocate table cache memory, if caching enabled */
                (void)soc_mem_cache_set(unit, mem, COPYNO_ALL, FALSE);
            }
            if (soc->memState[mem].lock != NULL) {
                sal_mutex_destroy(soc->memState[mem].lock);
                soc->memState[mem].lock = NULL;
            }
        }

#ifdef INCLUDE_MEM_SCAN
        (void)soc_mem_scan_stop(unit);         /* Stop memory scanner */
        if (soc->mem_scan_notify) {
            sal_sem_destroy(soc->mem_scan_notify);
        }
#endif

        if (soc->schanMutex) {
            sal_mutex_destroy(soc->schanMutex);
            soc->schanMutex = NULL;
        }

        for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++) {
            if (soc->schanIntr[cmc]) {
                sal_sem_destroy(soc->schanIntr[cmc]);
                soc->schanIntr[cmc] = NULL;
            }
        }

        if (soc->miimMutex) {
            sal_mutex_destroy(soc->miimMutex);
            soc->miimMutex = NULL;
        }

        (void)soc_sbusdma_lock_deinit(unit);

        if (soc->miimIntr) {
            sal_sem_destroy(soc->miimIntr);
            soc->miimIntr = NULL;
        }

        for (ix = 0; ix < 3; ix++) {
            if (soc->memCmdIntr[ix]) {
                sal_sem_destroy(soc->memCmdIntr[ix]);
                soc->memCmdIntr[ix] = NULL;
            }
        }

        if (soc->ipfixIntr) {
            sal_sem_destroy(soc->ipfixIntr);
            soc->ipfixIntr = NULL;
        }

        if (soc->socControlMutex) {
            sal_mutex_destroy(soc->socControlMutex);
            soc->socControlMutex = NULL;
        }

        if (soc->egressMeteringMutex) {
            sal_mutex_destroy(soc->egressMeteringMutex);
            soc->egressMeteringMutex = NULL;
        }

        /* disable interrupts before detaching */
        soc_sbx_intr_disable(unit, dev_id, ~0);

        /*
         * Detach interrupt handler
         */
        dev_type = soc_cm_get_dev_type(unit);
        if (!(dev_type & SAL_EB_DEV_TYPE)) {
            
            if (soc_cm_interrupt_disconnect(unit) < 0) {
                return SOC_E_INTERNAL;
            }
        }

        /* Terminate software counter collection module; free allocated space */
        soc_sbx_counter_detach(unit);

        /* Destroy MIIM mutex */
        if (soc->miimMutex) {
            sal_mutex_destroy(soc->miimMutex);
            soc->miimMutex = NULL;
        }

        sbx = SOC_SBX_CONTROL(unit);

        if (sbx != NULL) {
            /*
             * Detach resources allocated by device specific initialization
             */
            if (sbx->dma_sem) {
                sal_sem_destroy(sbx->dma_sem);
                sbx->dma_sem = 0;
            }

            if (sbx->detach) {
                sbx->detach(unit);
            }

            if (sbx->cfg) {
                sal_free(sbx->cfg);
                sbx->cfg = NULL;
            }

            sal_free(sbx);
            sbx = soc->drv = NULL;

         }

        if (soc) {
            soc->soc_flags &= ~SOC_F_ATTACHED;
        }

        sal_free(SOC_PERSIST(unit));
        SOC_PERSIST(unit) = NULL;

        sal_free(SOC_CONTROL(unit));
        SOC_CONTROL(unit) = NULL;

        if (--soc_ndev_attached == 0) {
            /* Work done after the last SOC device is detached. */
            /* (currently nothing) */
        }
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
    } else {

        sbx = SOC_SBX_CONTROL(unit);

        /* disable interrupts before detaching */
        soc_sbx_intr_disable(unit, dev_id, ~0);

        /*
         * Detach interrupt handler
         */
        dev_type = soc_cm_get_dev_type(unit);
        if (!(dev_type & SAL_EB_DEV_TYPE)) {
            
            if (soc_cm_interrupt_disconnect(unit) < 0) {
                return SOC_E_INTERNAL;
            }
        }

        /* Terminate software counter collection module; free allocated space */
        soc_sbx_counter_detach(unit);

        /* Destroy MIIM mutex */
        if (soc->miimMutex) {
            sal_mutex_destroy(soc->miimMutex);
            soc->miimMutex = NULL;
        }

        if (sbx != NULL) {
            /*
             * Detach resources allocated by device specific initialization
             */
            if (sbx->dma_sem) {
                sal_sem_destroy(sbx->dma_sem);
                sbx->dma_sem = 0;
            }

            if (sbx->detach) {
                sbx->detach(unit);
            }

            if (sbx->cfg) {
                sal_free(sbx->cfg);
                sbx->cfg = NULL;
            }

            sal_free(sbx);
            sbx = soc->drv = NULL;

            sal_free(SOC_PERSIST(unit));
            SOC_PERSIST(unit) = NULL;

            sal_free(SOC_CONTROL(unit));
            soc = SOC_CONTROL(unit) = NULL;
         }

        if (soc) {
            soc->soc_flags &= ~SOC_F_ATTACHED;
        }
    }


    return SOC_E_NONE;
}



#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0


/*
 * Function:
 *      soc_wb_state_alloc_and_check
 * Purpose:
 *      Allocate an scache handle during a cold boot, or check
 *      version during a warm boot
 * Parameters:
 *     (in)     unit      - bcm device number
 *     (in)     hdl       - scache handle to alloc/validate
 *     (in/out) size      - size to allocate for cold boot
 *                          or size validated for warm boot
 *     (in)     current_version  - current version to store on cold boot
 *                                 or version to verify
 *     (out)    upgrade   - during a warm boot, returns upgrade status
 *                          == 0 : versions match
 *                          >  0 : upgrade detected
 *                          <  0 : downgrade detected
 * Returns:
 *      SOC_E_XXX
 */
int
soc_wb_state_alloc_and_check(int unit, soc_scache_handle_t hdl,
                             uint32 *size, uint32 current_version, 
                             int *upgrade)
{
    int                rv = BCM_E_NONE;
    int                scache_size;
    soc_wb_cache_t    *wbc;

    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, &scache_size);
    if (scache_size == 0) {
        return SOC_E_UNAVAIL;
    }
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    /* Allocate a new chunk of scache during a cold boot */
    if (SOC_WARM_BOOT(unit) == FALSE) {
        rv = soc_scache_alloc(unit, hdl, *size);
        if (rv == BCM_E_EXISTS) {
            rv = BCM_E_NONE;
        }
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    /* Get the pointer for the Level 2 cache */
    wbc = NULL;
    SOC_IF_ERROR_RETURN(
        soc_scache_ptr_get(unit, hdl, (uint8**)&wbc, size));

    if (wbc == NULL) {
        return SOC_E_NOT_FOUND;
    }

    if (SOC_WARM_BOOT(unit) == FALSE) {
        soc_scache_handle_lock(unit, hdl);
        wbc->version = current_version;
        soc_scache_handle_unlock(unit, hdl);
    }

    if (upgrade) {
        *upgrade = current_version - wbc->version;
    }

    return rv;
}


/*
 * Function:
 *      soc_sbx_shutdown
 * Purpose:
 *      Free up SOC resources without touching hardware
 * Parameters:
 *      unit - SBX unit #
 * Returns:
 *      SOC_E_XXX
 */
int
soc_sbx_shutdown(int unit)
{
    return soc_sbx_detach(unit);
}

int
soc_sbx_wb_init(int unit)
{
  int                 rv = SOC_E_NONE;
  soc_scache_handle_t scache_handle;
  uint32              scache_len=0;
  uint8               *scache_ptr=NULL;
  uint8               *ptr, *end_ptr;
  uint16              default_ver = BCM_WB_DEFAULT_VERSION;
  uint16              recovered_ver = BCM_WB_DEFAULT_VERSION;
  int a,b,stable_size;

  /* check to see if an scache table has been configured */
  rv = soc_stable_size_get(unit, &stable_size);
  if (SOC_FAILURE(rv) || stable_size <= 0) {
      return rv;
  }

  SOC_SCACHE_HANDLE_SET(scache_handle, unit, SOC_SBX_WB_MODULE_SOC, 0);

  if (SOC_WARM_BOOT(unit)) {
    /* If device is during warm-boot, recover the state from scache */
    scache_len = 0;
    rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					  FALSE, &scache_len, &scache_ptr,
					  default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        return rv;
    }
  }

  ptr = scache_ptr;
  end_ptr = scache_ptr + scache_len; /* used for overrun checks*/
  
  /* now decompress */
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->module_id0);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->node_id);
  for (a=0;a<SBX_MAX_MODIDS;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->modport[a][b]);
  for (a=0;a<SBX_MAX_NODES;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->fabnodeport2feport[a][b]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_DECOMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->fabric_units[a]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_DECOMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->forwarding_units[a]);

  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->reset_ul);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uRedMode);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);

  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
  __WB_DECOMPRESS_SCALAR(uint64, SOC_SBX_CFG(unit)->xbar_link_en);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->demand_scale);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
  __WB_DECOMPRESS_SCALAR(sbBool_t, SOC_SBX_CFG(unit)->uInterfaceProtocol);
  __WB_DECOMPRESS_SCALAR(sbBool_t, SOC_SBX_CFG(unit)->bTmeMode);
  if (SOC_IS_SBX_QE2000(unit)){
#ifdef BCM_QE2000_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BME3200(unit)){
#ifdef BCM_BME3200_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BM9600(unit)){
#ifdef BCM_BM9600_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverEqualization);
    }
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit);
#endif
  }else if (SOC_IS_SBX_SIRIUS(unit)){
#ifdef BCM_SIRIUS_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[a].uDriverEqualization);
    }	
    for (a=0; a < PRED_TYPE_MAX; a++) {
        for (b=0; b < SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS; b++) {
            __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->type_res_flags[a][b]);
        }
    }
    for (a=0; a < 4; a++) {
        for (b=0; b < SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES; b++) {
            __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->pred_info_flags[a][b]);
        }
    }
#endif
  }else if (SOC_IS_CALADAN3(unit)){
#ifdef BCM_WARM_BOOT_SUPPORT
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: WARM_BOOT_TODO: SOC WB init code needs to be hooked in here\n"), FUNCTION_NAME()));
#endif /* BCM_WARM_BOOT_SUPPORT */

  }else {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: Missing SOC WB init code\n"), FUNCTION_NAME()));
  }

  rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

  if (!SOC_WARM_BOOT(unit)){
    /* scache_len now has the size needed */
    rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					  TRUE, &scache_len, &scache_ptr,
					  default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        return rv;
    }
  }
  return rv;
}

int
soc_sbx_wb_sync(int unit, int sync)
{
  uint8                   *scache_ptr = NULL;
  uint8                   *ptr, *end_ptr;
  uint32                  scache_len=0;
  int                     rv;
  soc_scache_handle_t     scache_handle;
  int a,b;
  uint16               default_ver = BCM_WB_DEFAULT_VERSION;
  uint16               recovered_ver = BCM_WB_DEFAULT_VERSION;

  if (SOC_WARM_BOOT(unit)){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Cannot write to scache during WarmBoot\n")));
    return SOC_E_INTERNAL;
  }

  SOC_SCACHE_HANDLE_SET(scache_handle, unit, SOC_SBX_WB_MODULE_SOC, 0);
  rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                        FALSE, &scache_len, &scache_ptr,
                                        default_ver, &recovered_ver);
  if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: Error(%s) reading scache. scache_ptr:0x%x and len:%d\n"),
               FUNCTION_NAME(), soc_errmsg(rv), (uint32) scache_ptr, scache_len));
    return rv;
  }

  /* cancel any ongoing dma */
  (void)soc_dma_abort(unit);
          
  ptr = scache_ptr;
  end_ptr = scache_ptr+scache_len;

  /* now compress */
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->module_id0);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->node_id);
  for (a=0;a<SBX_MAX_MODIDS;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->modport[a][b]);
  for (a=0;a<SBX_MAX_NODES;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->fabnodeport2feport[a][b]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_COMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->fabric_units[a]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_COMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->forwarding_units[a]);

  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->reset_ul);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uRedMode);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);

  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
  __WB_COMPRESS_SCALAR(uint64, SOC_SBX_CFG(unit)->xbar_link_en);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->demand_scale);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
  __WB_COMPRESS_SCALAR(sbBool_t, SOC_SBX_CFG(unit)->uInterfaceProtocol);  
  __WB_COMPRESS_SCALAR(sbBool_t, SOC_SBX_CFG(unit)->bTmeMode);
  if (SOC_IS_SBX_QE2000(unit)){
#ifdef BCM_QE2000_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BME3200(unit)){
#ifdef BCM_BME3200_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BM9600(unit)){
#ifdef BCM_BM9600_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverEqualization);
    }
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit);
#endif
  }else if (SOC_IS_SBX_SIRIUS(unit)){
#ifdef BCM_SIRIUS_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[a].uDriverEqualization);
    }	
    for (a=0; a < PRED_TYPE_MAX; a++) {
        for (b=0; b < SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS; b++) {
            __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->type_res_flags[a][b]);
        }
    }
    for (a=0; a < 4; a++) {
        for (b=0; b < SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES; b++) {
            __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->pred_info_flags[a][b]);
        }
    }
#endif
  }else if (SOC_IS_CALADAN3(unit)){
#ifdef BCM_CALADAN3_SUPPORT      
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: WARM_BOOT_TODO: SOC WB init code\n"), FUNCTION_NAME()));
#endif /* BCM_CALADAN3_SUPPORT */    
  }else {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: Missing SOC WB init code\n"), FUNCTION_NAME()));
  }
  
  rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

  if (sync) {
      if (!SOC_IS_CALADAN3(unit)){
          /* Caladan3 commit is performed at the bcm layer - do not do it twice */
          rv = soc_scache_commit(unit);
          if (rv != SOC_E_NONE) {
              LOG_ERROR(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: Error(%s) sync'ing scache to Persistent memory. \n"),
                         FUNCTION_NAME(), soc_errmsg(rv)));
              return rv;
          }
      }
  }
  return BCM_E_NONE;
}

#endif /* WARMBOOT */


int
soc_sbx_module_custom1_init(int unit)
{
    int      lgl_link;
    int      found_first_lgl_link = FALSE, found_last_contg_lgl_link = FALSE, warning = FALSE;


    SOC_SBX_CFG(unit)->module_custom1_in_system = soc_property_get(unit, spn_SYSTEM_IS_MODULE_CUSTOM1_IN_SYSTEM, SOC_SBX_CFG(unit)->module_custom1_in_system);

    /* consistency check - configuration value */
    if ( (SOC_SBX_CFG(unit)->module_custom1_in_system != FALSE) &&
                                       (SOC_SBX_CFG(unit)->module_custom1_in_system != TRUE) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "unit(%d), Invalid Module-Custom1 Configuration(%d), Marking module not present\n"), unit, SOC_SBX_CFG(unit)->module_custom1_in_system));
        SOC_SBX_CFG(unit)->module_custom1_in_system = FALSE;
    }

    /* consistency check - supported system configuration */
    if (SOC_SBX_CFG(unit)->module_custom1_in_system == TRUE) {
        if ( (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) &&
               (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "unit(%d), Module-Custom1 Configuration not supported for FabricConfiguration(%d), Marking module not present\n"), unit, SOC_SBX_CFG(unit)->uFabricConfig));
            SOC_SBX_CFG(unit)->module_custom1_in_system = FALSE;
        }
    }

    /* no further processing if module not configured in system */
    if (SOC_SBX_CFG(unit)->module_custom1_in_system == FALSE) {
        return(BCM_E_NONE);
    }    

    SOC_SBX_CFG(unit)->module_custom1_links = soc_property_get(unit, spn_MODULE_CUSTOM1_LINKS, SOC_SBX_CFG(unit)->module_custom1_links);

    /* consistency check - xbar links need to be configured  */
    if (SOC_SBX_CFG(unit)->module_custom1_links == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "unit(%d), Module-Custom1 has no links(%d)\n"), unit, SOC_SBX_CFG(unit)->module_custom1_links));
        return(BCM_E_NONE);
    }

    /* consistency check - Warning if module-custom1 links are not contiguous */
    for (lgl_link = 0; lgl_link < 32; lgl_link++) {
        if (SOC_SBX_CFG(unit)->module_custom1_links & (1 << lgl_link)) {
            if (found_first_lgl_link == FALSE) {
                found_first_lgl_link = TRUE;
            }
          
            if ( (warning == FALSE) && (found_last_contg_lgl_link == TRUE) ) {
                warning = TRUE;
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "unit(%d), Unconventional Module-Custom1 links(%d)\n"), unit, SOC_SBX_CFG(unit)->module_custom1_links));
            }
        }
        else {
            if ((found_first_lgl_link == TRUE) && (found_last_contg_lgl_link == FALSE)) {
                found_last_contg_lgl_link = TRUE;
            }
        }
    } 

    return(BCM_E_NONE);
}

int
soc_sbx_init(int unit)
{
    soc_control_t        *soc;
    soc_sbx_control_t    *sbx;
    int                   rv;
    uint32                property = 0;
    uint32                port_default_speed = 0, port_speed = 0;
    uint32                max_nodes = 0;
    int                   cos;
    int                   i;
    uint32                requeue_mode;
    uint32                node, modid;
    int                   port=0, spi, hg, intf, level;
    int                   fifoset;
    char                 *portStr = NULL;
    int                   nbr_nodes_arbitration_port_allocation;
    int                   flow_control_mode, nbr_eset_resource = 0;
    uint16                dev_id, dev_id_driver;
    uint8                 rev_id, rev_id_driver;


    if (!SOC_UNIT_VALID(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_init: unit %d not valid\n"), unit));
        return SOC_E_UNIT;
    }

    if (SOC_WARM_BOOT(unit)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d: warm booting.\n"), unit));
    }

    soc = SOC_CONTROL(unit);
    sbx = SOC_SBX_CONTROL(unit);
    assert(soc != NULL);
    assert(sbx != NULL);
    assert(SOC_SBX_CFG(unit) != NULL);

#if defined(BCM_CALADAN3_SUPPORT)
    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_init: unit %d not attached\n"), unit));
        return SOC_E_UNIT;
    }

    if (soc->soc_flags & SOC_F_INITED) {
        if (!SOC_IS_RCPU_ONLY(unit)) {
            SOC_IF_ERROR_RETURN(soc_dma_abort(unit)); /* Turns off/clean up DMA */
        }
        SOC_IF_ERROR_RETURN(soc_counter_stop(unit)); /* Stop counter collection */
#ifdef INCLUDE_MEM_SCAN
        SOC_IF_ERROR_RETURN(soc_mem_scan_stop(unit)); /* Stop memory scanner */
#endif
#ifdef  INCLUDE_I2C
        SOC_IF_ERROR_RETURN(soc_i2c_detach(unit)); /* Free up I2C driver mem */
#endif
#ifdef BCM_SBUSDMA_SUPPORT
            /*SOC_IF_ERROR_RETURN(soc_sbusdma_desc_detach(unit));*/
#endif

        soc->soc_flags &= ~SOC_F_INITED;
    }
#endif /* BCM_CALADAN3_SUPPORT */


    if (sbx->sbhdl != (sbhandle)unit) {
        sbx->sbhdl = (sbhandle)unit;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    /* Recover stored Level 2 Warm Boot cache */
    /* The stable and stable size must be selected first */
    if (SOC_WARM_BOOT(unit)) {
        rv = soc_scache_recover(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "scache recover failed: %s\n"),
                       soc_errmsg(rv)));
            return rv;
        }
    } else {
        /* if calling soc_init again after bcm_init, 
         * then we need to clear scache_state
         */
#ifdef BCM_CALADAN3_SUPPORT
        if (!SOC_RECONFIG_TDM) {
#endif
            rv = soc_scache_detach(unit);
            if ((rv != SOC_E_NONE) && (rv != SOC_E_CONFIG)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "scache re-init failed: %s\n"),
                           soc_errmsg(rv)));
                return rv;
            }
#ifdef BCM_CALADAN3_SUPPORT
        }
#endif
    }
#endif


    /* If Warmboot defined and Caladan3 - Init warmboot functionality */
#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit)) {
        rv = soc_sbx_caladan3_wb_counter_state_init(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON, 
                      (BSL_META_U(unit,
                                  "warmboot counter error: unit %d failed (%s)\n"), 
                       unit, soc_errmsg(rv)));
                      return rv;
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (SOC_IS_SIRIUS(unit)) {
        /* reset local state (fail now if not allocated) */
        
        /* get rid of old copy if this exists */
        soc_sirius_state_cleanup(unit);
        /* allocate local state information if needed */
        SOC_SBX_SIRIUS_STATE(unit) = sal_alloc(sizeof(*SOC_SBX_SIRIUS_STATE(unit)),
                                               "Sirius SOC state");
        if (SOC_SBX_SIRIUS_STATE(unit)) {
            sal_memset(SOC_SBX_SIRIUS_STATE(unit),
                       0x00,
                       sizeof(*SOC_SBX_SIRIUS_STATE(unit)));
        } else {
            return SOC_E_MEMORY;
        }
    }


    /* Re-initialize state upon init */
    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);
    
    if (soc_sbx_info_config(unit, dev_id, dev_id_driver) != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d unable to init config\n"),
                   unit));
        return SOC_E_INTERNAL;
    }


    SOC_SBX_CFG(unit)->parse_rx_erh = soc_property_get(unit, spn_RX_PARSE_ERH, 1);
    NUM_COS(unit) = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    /* Default cfg was setup when device was attached
     * here process user defines to modify the default cfg
     */
    SOC_SBX_CFG(unit)->uFabricConfig       = soc_property_get(unit, spn_FABRIC_CONFIGURATION,
                                                              SOC_SBX_CFG(unit)->uFabricConfig);
    SOC_SBX_CFG(unit)->bHalfBus            = soc_property_get(unit, spn_HALF_BUS_MODE,
                                                              SOC_SBX_CFG(unit)->bHalfBus);
    SOC_SBX_CFG(unit)->bRunSelfTest        = soc_property_get(unit, spn_BIST_ENABLE,
                                                              SOC_SBX_CFG(unit)->bRunSelfTest);
    SOC_SBX_CFG(unit)->uLinkThresholdIndex = soc_property_get(unit, spn_LINK_THRESHOLD,
                                                              SOC_SBX_CFG(unit)->uLinkThresholdIndex);
    SOC_SBX_CFG(unit)->uRedMode            = soc_property_get(unit, spn_REDUNDANCY_MODE,
                                                              SOC_SBX_CFG(unit)->uRedMode);
    SOC_SBX_CFG(unit)->uMaxFailedLinks     = soc_property_get(unit, spn_MAX_FAILED_LINKS,
                                                              SOC_SBX_CFG(unit)->uMaxFailedLinks);
    SOC_SBX_CFG(unit)->bHybridMode         = soc_property_get(unit, spn_HYBRID_MODE,
                                                              SOC_SBX_CFG(unit)->bHybridMode);
    SOC_SBX_CFG(unit)->bUcqResourceAllocationMode = soc_property_get(unit, spn_UNICAST_QUEUE_RESOURCE_ALLOCATION_MODE,
								     SOC_SBX_CFG(unit)->bUcqResourceAllocationMode);

    /* flow control mode */
    flow_control_mode = soc_property_get(unit, spn_EGRESS_FIFO_INDEPENDENT_FC, FALSE);

    /* additional consistency checks - flow control mode */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if (flow_control_mode != SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid FlowCtrlMode %d unit %d fabricMode:%d, setting it to default-0\n"),
                       flow_control_mode, unit, SOC_SBX_CFG(unit)->uFabricConfig));
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC;
        }
    }
    else if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                       (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
        if ((flow_control_mode != SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC) &&
                          (flow_control_mode != SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid FlowCtrlMode %d unit %d fabricMode:%d, setting it to default-0\n"),
                       flow_control_mode, unit, SOC_SBX_CFG(unit)->uFabricConfig));
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC;
        }

        
        if (flow_control_mode == SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC) {
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_NIFC;
        }
    }

    /* flow control mode - setting */
    switch (flow_control_mode) {
        case SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = TRUE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_IFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = TRUE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_NIFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid FlowCtrlMode %d unit %d, setting it to default-0\n"),
                       flow_control_mode, unit));
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
    }

    SOC_SBX_CFG(unit)->bTmeMode            = soc_property_get(unit, spn_QE_TME_MODE,
                                                              SOC_SBX_CFG(unit)->bTmeMode);
    SOC_SBX_CFG(unit)->fabric_egress_setup = soc_property_get(unit, spn_FABRIC_EGRESS_SETUP,
                                                              SOC_SBX_CFG(unit)->fabric_egress_setup);
    SOC_SBX_CFG(unit)->sp_mode             = soc_property_get(unit, spn_BCM_COSQ_SP_MODE,
							      SOC_SBX_CFG(unit)->sp_mode);
    SOC_SBX_CFG(unit)->local_template_id   = soc_property_get(unit, spn_TM_LOCAL_SCHED_DISCIPLINE_TEMPLATE,
							      SOC_SBX_CFG(unit)->local_template_id);
    SOC_SBX_CFG(unit)->node_template_id    = soc_property_get(unit, spn_TM_SCHED_DISCIPLINE_TEMPLATE,
							      SOC_SBX_CFG(unit)->node_template_id);

    SOC_SBX_CFG(unit)->erh_type = soc_property_get(unit, spn_QE_ERH_TYPE, 0);

    if ( (SOC_SBX_CFG(unit)->sp_mode != SOC_SBX_SP_MODE_IN_BAG) &&
         (SOC_SBX_CFG(unit)->sp_mode != SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid SP mode %d unit %d\n"),
                   SOC_SBX_CFG(unit)->sp_mode, unit));

        /* set it to a default value */
        SOC_SBX_CFG(unit)->sp_mode = SOC_SBX_SP_MODE_IN_BAG;
    }

    SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule = soc_property_get(unit,
                 (spn_NUM_MAX_FABRIC_PORTS_ON_MODULE), SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule);

    SOC_SBX_CFG(unit)->fixed_demand_scale = soc_property_get(unit,
                           (spn_DEMAND_SCALE), SOC_SBX_CFG(unit)->fixed_demand_scale);
    if (SOC_SBX_CFG(unit)->fixed_demand_scale != -1) {
        SOC_SBX_CFG(unit)->is_demand_scale_fixed = TRUE;

        if (SOC_SBX_CFG(unit)->fixed_demand_scale > 7) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid Demand Scale %d unit %d\n"),
                       SOC_SBX_CFG(unit)->fixed_demand_scale, unit));
            /* set it to a default value */
            SOC_SBX_CFG(unit)->fixed_demand_scale = 3;
        }

        SOC_SBX_CFG(unit)->demand_scale = SOC_SBX_CFG(unit)->fixed_demand_scale;
    }

    SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap = soc_property_get(unit,
                           (spn_BCM8823X_ALLOW_UCAST_MCAST_RESOURCE_OVERLAP),
                            SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap);
    if ( (SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap != FALSE) &&
                     (SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap != TRUE) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid ucast_mcast_overlap mode %d unit %d, Setting it to FALSE\n"),
                   SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap, unit));
        SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap = FALSE;
    }


#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
      SOC_SBX_CFG_CALADAN3(unit)->l2_age_cycles =  soc_property_get(unit, spn_L2_AGE_CYCLES,
								  L2_AGE_CYCLES_INTERVAL_DEFAULT);

      SOC_SBX_CFG_CALADAN3(unit)->l2_cache_max_idx = soc_property_get(unit, spn_L2CACHE_MAX,
								    L2CACHE_MAX_IDX_DEFAULT);

      /* Default front panel higig interface with oob flow control on interlaken */
      SOC_SBX_CFG_CALADAN3(unit)->fc_type[0] = soc_property_get(unit, "fc_type_il_line", SOC_SBX_CALADAN3_FC_TYPE_NONE);
      /* Default backplane oob interlaken flow control */
      SOC_SBX_CFG_CALADAN3(unit)->fc_type[1] = soc_property_get(unit, "fc_type_il_fabric", SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB);

      SOC_SBX_CFG(unit)->uClockSpeedInMHz = soc_property_get(unit, spn_CORE_CLOCK_SPEED,
							     SOC_SBX_CFG(unit)->uClockSpeedInMHz);

      SOC_SBX_CFG_CALADAN3(unit)->include_lss_faults =  soc_property_get(unit, "include_lss_faults", 0);
    }
#endif

    SOC_SBX_CFG(unit)->v4_ena = soc_property_get(unit, spn_L3_ENABLE, 0);
    SOC_SBX_CFG(unit)->v6_ena = 0;
    SOC_SBX_CFG(unit)->mim_ena = 0;
    SOC_SBX_CFG(unit)->dscp_ena = 0;
    SOC_SBX_CFG(unit)->mplstp_ena = 0;

    if (SOC_IS_CALADAN3(unit)) {
        /* Enable mplstp by default for Caladan3 */
        SOC_SBX_CFG(unit)->mplstp_ena = 1;
    }
    SOC_SBX_CFG(unit)->oam_spi_lb_port = soc_property_get(unit, spn_OAM_SPI_LB_PORT,
                                           SBX_OAM_SPI_LB_PORT);
    SOC_SBX_CFG(unit)->oam_spi_lb_queue = soc_property_get(unit, spn_OAM_SPI_LB_QUEUE,
                                            SBX_OAM_SPI_LB_QUEUE);
    SOC_SBX_CFG(unit)->v4mc_str_sel = soc_property_get(unit, spn_V4MC_STR_SEL, 0);
    SOC_SBX_CFG(unit)->v4uc_str_sel = soc_property_get(unit, spn_V4UC_STR_SEL, 0);

    bankSwapLock[unit] = sal_mutex_create("bank_swap_mutex");
    if(bankSwapLock[unit] == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_attach: unit %d create mutex for bank swap failed\n"),
                   unit));
        return SOC_E_MEMORY;
    }
    /*
     * Consistency checks nbrResourcePerEset SOC property
     * On consistency checks failures or if there are no resources an error is logged and
     * SOC property is set to the default configuration
     */
    SOC_SBX_CFG(unit)->num_res_per_eset_spec = soc_property_get(unit, spn_BME_NUM_RESOURCES_PER_ESET, FALSE);

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {

        if ( (SOC_SBX_CFG(unit)->bHybridMode != FALSE) || (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_FIC) ) {
            if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Invalid nbrResourcePerEset %d unit %d, setting it to default mode (0), Onl valid in FIC mode\n"),
                           SOC_SBX_CFG(unit)->num_res_per_eset_spec, unit));
                SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;
            }
        }
        else {
            if (soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol) == SOC_SBX_IF_PROTOCOL_XGS) {
                if ( (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) &&
                                          (SOC_SBX_CFG(unit)->num_res_per_eset_spec != 4) ) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid nbrResourcePerEset %d unit %d, setting it to default mode (0)\n"),
                               SOC_SBX_CFG(unit)->num_res_per_eset_spec, unit));
                    SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;
                }
                else {
                    if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                        nbr_eset_resource = SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                        if ( (soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS) * nbr_eset_resource) > BM9600_MAX_MULTICAST_EXTENDED_ESETS) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "Not enough resources to support nbrResourcePerEset %d unit %d, setting it to default mode (0)\n"),
                                       SOC_SBX_CFG(unit)->num_res_per_eset_spec, unit));
                            SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;
                        }
                    }
                    if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                        SOC_SBX_CFG(unit)->is_mc_ef_cos[0] = TRUE;
                        SOC_SBX_CFG(unit)->is_mc_ef_cos[1] = FALSE;
                        SOC_SBX_CFG(unit)->is_mc_ef_cos[2] = FALSE;
                        SOC_SBX_CFG(unit)->is_mc_ef_cos[3] = FALSE;
                    }
                }
            }
            else {
                if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid nbrResourcePerEset %d unit %d valid only when Interface mode is XGS , setting it to default mode (0)\n"),
                               SOC_SBX_CFG(unit)->num_res_per_eset_spec, unit));
                }
                SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;
            }
        }
    }
    else {
        if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid nbrResourcePerEset %d unit valid only for Fabric Mode VPORT %d, setting it to default mode (0)\n"),
                       SOC_SBX_CFG(unit)->num_res_per_eset_spec, unit));
        }
        SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;
    }

    /* These values are system maximums for fabric devices.
     * Initialize these on all devices to the maximum system wide value.
     */
    switch (SOC_SBX_CFG(unit)->uFabricConfig) {
        case SOC_SBX_SYSTEM_CFG_DMODE: /* DMode (Default) - Bm3200 + Qe2000 */
        default:
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
            SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
            SOC_SBX_CFG(unit)->num_bw_groups = HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS;
            if (SOC_SBX_CFG(unit)->bHybridMode == FALSE) {
                SOC_SBX_CFG(unit)->num_sysports = 0;
            } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
                SOC_SBX_CFG(unit)->num_sysports =
                         SB_FAB_DEVICE_BM3200_MAX_NODES * SB_FAB_DEVICE_QE2000_MAX_PORT;
            }
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;

            /* update num_ds_ids to account for epoch optimizations */
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SOC_SBX_CFG(unit)->num_ds_ids);
            if (SOC_SBX_CFG(unit)->num_ds_ids > SB_FAB_DEVICE_BM3200_MAX_DS_IDS) {
                SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
            }
            SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;

            SOC_SBX_CFG(unit)->num_res_per_eset_spec = FALSE;

            break;

        case SOC_SBX_SYSTEM_CFG_VPORT:        /* Vport - Bm9600 + Qe4000 */
            
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 0;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;

                /* check if non-default eset resource mode */
                if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                    SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                    if (SOC_SBX_CFG(unit)->num_internal_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                        SOC_SBX_CFG(unit)->use_extended_esets = 0;
                    }
                }
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Invalid ds_ids %d unit %d, setting it to %d\n"),
                                   SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 0;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;

                /* check if non-default eset resource mpde */
                if (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                    SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                    if (SOC_SBX_CFG(unit)->num_internal_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                        SOC_SBX_CFG(unit)->use_extended_esets = 0;
                    }
                }
            }
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }

            if ((SOC_SBX_CFG(unit)->bHybridMode) && (SOC_IS_SIRIUS(unit))) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            max_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            break;
        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* Vport Legacy - Bm9600 + Qe2000 */
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Invalid ds_ids %d unit %d, setting it to %d\n"),
                                   SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;
            }
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                if (SOC_IS_SBX_QE2000(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
                } else {
		    SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
		}
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }
            if (SOC_SBX_CFG(unit)->bHybridMode) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;

            break;
        case SOC_SBX_SYSTEM_CFG_VPORT_MIX: /* Vport Legacy - Bm9600 + Qe2000 + Qe4000 */
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			/* only complain when user is tring to config this way, otherwise quietly change it */
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Invalid ds_ids %d unit %d, setting it to %d\n"),
                                   SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;
            }
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                if (SOC_IS_SBX_QE2000(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
                } else if (SOC_IS_SIRIUS(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
                } else {
		    SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
		}
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
            if ((SOC_SBX_CFG(unit)->bHybridMode) && (SOC_IS_SIRIUS(unit))) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            break;
    }

    soc_sbx_module_custom1_init(unit);

    if (SOC_IS_SBX_SIRIUS(unit) && soc_feature(unit, soc_feature_standalone)) {

        SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
        SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit,
                                               spn_BME_NUM_ESETS,
                                               SB_FAB_DEVICE_BM9600_MAX_DS_IDS);

        
        if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
            SOC_SBX_CFG(unit)->use_extended_esets = 1;
            SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
        }
        if ((SOC_SBX_CFG(unit)->bHybridMode) ||
	    (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
	    (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
            SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
	    SOC_SBX_CFG(unit)->num_bw_groups = SOC_SBX_CFG(unit)->num_queues / NUM_COS(unit);
        } else {
            SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
	    SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
        }

        SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
        max_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
    }

    SOC_SBX_CFG(unit)->cfg_num_nodes = soc_property_get(unit, spn_NUM_MODULES, max_nodes);

    /* fill in the default node mask */
    SOC_SBX_CFG(unit)->cfg_node_00_31_mask = SOC_SBX_CFG(unit)->cfg_node_32_63_mask =
                                                 SOC_SBX_CFG(unit)->cfg_node_64_95_mask = 0;
    for (i = 0; i < SOC_SBX_CFG(unit)->cfg_num_nodes; i++) {
        if (i < (sizeof(uint32) * 8)) {
            SOC_SBX_CFG(unit)->cfg_node_00_31_mask |= (1 << i);
        }
        else if (i < ((sizeof(uint32) * 8) * 2)) {
            SOC_SBX_CFG(unit)->cfg_node_32_63_mask |= (1 << (i - (sizeof(uint32) * 8)));
        }
        else {
            SOC_SBX_CFG(unit)->cfg_node_64_95_mask |= (1 << (i - ((sizeof(uint32) * 8) * 2)));
        }
    }
    /* update node mask from the configuration file */
    /* NOTE: currently no consistency checks are done */
    SOC_SBX_CFG(unit)->cfg_node_00_31_mask = soc_property_get(unit, spn_NUM_MODULES_00_31_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_00_31_mask);
    SOC_SBX_CFG(unit)->cfg_node_32_63_mask = soc_property_get(unit, spn_NUM_MODULES_32_63_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_32_63_mask);
    SOC_SBX_CFG(unit)->cfg_node_64_95_mask = soc_property_get(unit, spn_NUM_MODULES_64_95_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_64_95_mask);

    SOC_SBX_CFG(unit)->uActiveScId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                              SOC_SBX_CFG(unit)->uActiveScId);
    /* discard configuration parameters */
    SOC_SBX_CFG(unit)->discard_probability_mtu = soc_property_get(unit, spn_DISCARD_MTU_SIZE,
                                                      SOC_SBX_CFG(unit)->discard_probability_mtu);
    SOC_SBX_CFG(unit)->discard_queue_size = soc_property_get(unit, spn_DISCARD_QUEUE_SIZE,
                                                      SOC_SBX_CFG(unit)->discard_queue_size);

    soc_sbx_set_epoch_length(unit);

    SOC_SBX_CFG(unit)->hold_pri_num_timeslots = soc_property_get(unit, spn_HOLD_PRI_NUM_TIMESLOTS, SOC_SBX_CFG(unit)->hold_pri_num_timeslots);

    SOC_SBX_CFG(unit)->uSerdesSpeed           = soc_property_get(unit, spn_BACKPLANE_SERDES_SPEED,
                                                                 SOC_SBX_CFG(unit)->uSerdesSpeed);
    SOC_SBX_CFG(unit)->bSerdesEncoding        = soc_property_get(unit, spn_BACKPLANE_SERDES_ENCODING,
                                                                 SOC_SBX_CFG(unit)->bSerdesEncoding);

    nbr_nodes_arbitration_port_allocation = soc_property_get(unit, spn_ARBITRATION_PORT_MAX_NODES, -1);
    if (nbr_nodes_arbitration_port_allocation != -1) {
        if ( (nbr_nodes_arbitration_port_allocation != SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION1) &&
                (nbr_nodes_arbitration_port_allocation != SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION2) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid Max Nodes for Arbitration Port Allocation Scheme %d unit %d, resetting it to default\n"),
                       nbr_nodes_arbitration_port_allocation, unit));
            nbr_nodes_arbitration_port_allocation = -1;
        }
        else {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
              (nbr_nodes_arbitration_port_allocation ==
                             SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION1) ?
                               SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1 :
                               SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2;
        }
    }
    if (nbr_nodes_arbitration_port_allocation == -1) {
        /* currently the default is SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1 */
#if 0
        if (SOC_SBX_CFG(unit)->cfg_num_nodes > 32) {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
                                                SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1;
        }
        else {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
                                                SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2;
        }
#endif /* 0 */
    }

    SOC_SBX_CFG(unit)->diag_qe_revid = soc_property_get(unit, "diag_qe_revid",
							SOC_SBX_CFG(unit)->diag_qe_revid);

    if ( SOC_IS_SBX_QE2000(unit) || SOC_IS_SBX_SIRIUS(unit) ) {
        for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
            SOC_SBX_CFG(unit)->connect_min_util[cos] = soc_property_cos_get(unit, cos,
                  spn_FABRIC_CONNECT_MIN_UTILIZATION, SOC_SBX_CFG(unit)->connect_min_util[cos]);
            SOC_SBX_CFG(unit)->connect_max_age_time[cos] = soc_property_cos_get(unit, cos,
                  spn_FABRIC_CONNECT_MAX_AGE_TIME, SOC_SBX_CFG(unit)->connect_max_age_time[cos]);
        }
    }

    if (SOC_IS_SBX_QE2000(unit)) {
        /* Fill in QE2000 specific configuration */
        /* modify the configuration set earlier */
        if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
            if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
                SOC_SBX_CFG(unit)->num_sysports = 0;
            } else if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
                SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0xc0;
            }
        } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
	    SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0xfa;
	}

        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_QE2000(unit)->bHalfBus               = soc_property_get(unit, spn_QE_HALF_BUS_MODE,
                                                                            SOC_SBX_CFG_QE2000(unit)->bHalfBus);
        SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest  = soc_property_get(unit, spn_QE_LONG_DDR_MEMTEST,
                                                                            SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest);
        SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2           = soc_property_get(unit, spn_QE_MEMORY_PART,
                                                                            SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2);
        SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks);
        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize             = soc_property_get(unit, spn_QE_EG_MVT_SIZE,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMVTSize);

        switch (SOC_SBX_CFG_QE2000(unit)->uEgMVTSize) {
            case 0:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_12K_LOCAL_BASE;
                break;

            case 1:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_24K_LOCAL_BASE;
                break;

            case 2:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                break;

            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Invalid MVT size %d unit %d\n"),
                           SOC_SBX_CFG_QE2000(unit)->uEgMVTSize, unit));
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;
                break;
        }

        SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness = soc_property_get(unit,"subscriber_qos_excess_mode", SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness);


        SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull        = soc_property_get(unit, spn_QE_EG_MC_DROP_ON_FULL,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull);

        SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat           = soc_property_get(unit, spn_QE_MVT_FORMAT,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat);
        if ((SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat != SBX_MVT_FORMAT0) && (SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat != SBX_MVT_FORMAT1) ) {
            SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat = SBX_MVT_FORMAT0;
        }

        SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout = soc_property_get(unit, spn_EGRESS_MC_EF_TIMEOUT,
                                                                       SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout);
        SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout = soc_property_get(unit, spn_EGRESS_MC_NEF_TIMEOUT,
                                                                       SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout);

        SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout = soc_property_get(unit, spn_QE_EI_PORT_TIMEOUT,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout);
        SOC_SBX_CFG_QE2000(unit)->uScGrantoffset          = soc_property_get(unit, spn_QE_GRANT_OFFSET,
                                                                            SOC_SBX_CFG_QE2000(unit)->uScGrantoffset);
        SOC_SBX_CFG_QE2000(unit)->uScGrantDelay          = soc_property_get(unit, "qe_grant_delay",
                                                                            SOC_SBX_CFG_QE2000(unit)->uScGrantDelay);
        SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes = soc_property_get(unit, spn_QE_GLOBAL_SHAPING_ADJUST,
                                                                                 SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes);
        SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs   = soc_property_get(unit, spn_QE_MAX_ARRIVAL_RATE,
                                                                            SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs);
        SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows = soc_property_get(unit, spn_QE_MIX_HIGH_LOW_RATE_FLOWS,
                                                                             SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows);
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0] = soc_property_get(unit, spn_QE_SPI_0_FULL_PACKET_MODE,
                                                                             SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0]);
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1] = soc_property_get(unit, spn_QE_SPI_1_FULL_PACKET_MODE,
                                                                             SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1]);
        SOC_SBX_CFG_QE2000(unit)->uEiLines[0]
            = soc_property_get(unit, spn_QE_SPI_0_EI_LINES,
                               SOC_SBX_CFG_QE2000(unit)->uEiLines[0]);
        SOC_SBX_CFG_QE2000(unit)->uEiLines[1]
            = soc_property_get(unit, spn_QE_SPI_1_EI_LINES,
                               SOC_SBX_CFG_QE2000(unit)->uEiLines[1]);

        SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes             = soc_property_get(unit, spn_NUM_MODULES,
                                                                             SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes);
        SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress = soc_property_get(unit, spn_QE_QUEUES_PER_INGRESS_SHAPER,
                                                                             SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress);

        SOC_SBX_CFG_QE2000(unit)->bEpDisable = soc_property_get(unit, spn_QE_EP_DISABLE, SOC_SBX_CFG_QE2000(unit)->bEpDisable);
        if ((SOC_SBX_CFG_QE2000(unit)->bEpDisable != FALSE) && (SOC_SBX_CFG_QE2000(unit)->bEpDisable != TRUE)) {
            SOC_SBX_CFG_QE2000(unit)->bEpDisable = FALSE;
        }

        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0] = soc_property_get(unit, spn_SPI_0_REF_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0]);
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0] = soc_property_get(unit, spn_SPI_0_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0]);
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1] = soc_property_get(unit, spn_SPI_1_REF_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1]);
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1] = soc_property_get(unit, spn_SPI_1_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]      = soc_property_get(unit, spn_QE_SPI_0_SUBPORTS,
                                                                             SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]      = soc_property_get(unit, spn_QE_SPI_1_SUBPORTS,
                                                                             SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);

        /* NOTE: currently it is expeted that spn_QE_SPI_0_SUBPORTS/spn_QE_SPI_1_SUBPORTS will  */
        /*       always be specified when Tx/Rx properties are specified individually. The      */
        /*       alternative is to set it from the Tx properties. The port bitmap is populated  */
        /*       with spn_QE_SPI_0_SUBPORTS/spn_QE_SPI_1_SUBPORTS values and there is no        */
        /*       consistency check currently present when a queue group is added for a physical */
        /*       port. These checks can be later added.                                         */
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[0] = soc_property_get(unit,
             spn_SPI_0_NUM_TX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[0] = soc_property_get(unit,
             spn_SPI_0_NUM_RX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[1] = soc_property_get(unit,
             spn_SPI_1_NUM_TX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[1] = soc_property_get(unit,
             spn_SPI_1_NUM_RX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);

        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0] = soc_property_get(unit,
             spn_SPI_0_TX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0] = soc_property_get(unit,
             spn_SPI_0_RX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1] = soc_property_get(unit,
             spn_SPI_1_TX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1] = soc_property_get(unit,
             spn_SPI_1_RX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1]);

        SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = soc_property_get(unit, spn_QE_SC_TXDMA_SOT_DELAY_CLOCKS,
                                                                              SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks);
        SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = soc_property_get(unit, spn_QE_SFI_TIMESLOT_OFFSET_CLOCKS,
                                                                                SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks);

        SOC_INFO(unit).modid_max = BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->num_nodes - 1;

        /* init default spi subport speed based on number of ports on the spi interface */
        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            SOC_SBX_CFG_QE2000(unit)->uInterleaveBurstSize[spi] =
                soc_property_port_get(unit, spi, spn_SPI_INTERLEAVE_BURST_SIZE, HW_QE2000_PORT_MODE_PKT_IL_BURST_SIZE);
            if (SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi] > 1) {
                /* default to 1 Gbps */
                port_default_speed = 1000;
            } else {
                /* default to 10 Gbps */
                port_default_speed = 10000;
            }
            for(port=0; port<(SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi]); port++){
                if (spi == 0) {
                    port_speed = soc_property_port_get(unit, port, spn_QE_SPI_0_SUBPORT_SPEED, port_default_speed);
                } else {
                    port_speed = soc_property_port_get(unit, port, spn_QE_SPI_1_SUBPORT_SPEED, port_default_speed);
                }
                SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[spi][port] = port_speed;
                if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
                    uint64 uuTmp;
                    if (spi == 0) {
                        requeue_mode = soc_property_port_get(unit, port, spn_QE_SPI_0_SUBPORT_IS_REQUEUE, 0);
                    } else {
                        requeue_mode = soc_property_port_get(unit, port, spn_QE_SPI_1_SUBPORT_IS_REQUEUE, 0);
                    }
                    COMPILER_64_SET(uuTmp, 0, requeue_mode);
                    COMPILER_64_SHL(uuTmp, port);
                    COMPILER_64_OR(SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[spi], uuTmp);
                }
            }
        }

        for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = soc_property_port_get(unit, port, spn_EGRESS_MCAST_EF_DESC_SZ,
                                                                                               SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port]);
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = soc_property_port_get(unit, port, spn_EGRESS_MCAST_NEF_DESC_SZ,
                                                                                               SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port]);
        }

        fifoset=0;
        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            for (port=0; port <(SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi]); port++) {
                /* 2 multicast FIFOs per port, EF, non EF */
                /* EF is 0-49, with PCI at 49, NEF is 50-99 with PCI at 99 */
                SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[fifoset] = TRUE;
                SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[fifoset] = TRUE;
                fifoset++;
            }
        }
        /* Reserve last 2 FIFOs after all SPI for PCI port */
        SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[49] = TRUE;
        SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[49] = TRUE;

        for(port=0; port<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; port++){
            SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverStrength);
            SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverEqualization);
        }
        /* Following config's are either hardcoded or configured after init:
           SOC_SBX_CFG_QE2000(unit)->uSfiDataLinkInitMask  = 0; QE1000 related, no use
           SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0] = 0; requeue configuration done by API
           SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1] = 0; requeue configuration done by API
           SOC_SBX_CFG_QE2000(unit)->nodeNum_ul            = 0; node id is unknown at init time, configured by API
        */

        SOC_SBX_CFG(unit)->mcgroup_local_start_index =
                soc_property_get(unit, spn_MC_GROUP_LOCAL_START_INDEX, SOC_SBX_CFG(unit)->mcgroup_local_start_index);

        switch (SOC_SBX_CFG_QE2000(unit)->uEgMVTSize) {
            case 0:
                if ((SBX_MVT_ID_12K_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Disabling local MVT entries on unit %d\n"),
                              unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_12K_DYNAMIC_END) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid MVT local Index %d unit %d\n"),
                               SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_12K_LOCAL_BASE;
                }
                break;

            case 1:
                if ((SBX_MVT_ID_24K_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Disabling local MVT entries on unit %d\n"),
                              unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_24K_DYNAMIC_END) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid MVT local Index %d unit %d\n"),
                               SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_24K_LOCAL_BASE;
                }
                break;

            case 2:
                if ((SBX_MVT_ID_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Disabling local MVT entries on unit %d\n"),
                              unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_DYNAMIC_END) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid MVT local Index %d unit %d\n"),
                               SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                }
                break;

            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Invalid MVT size %d unit %d\n"),
                           SOC_SBX_CFG_QE2000(unit)->uEgMVTSize, unit));
                if ((SBX_MVT_ID_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Disabling local MVT entries on unit %d\n"),
                              unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_DYNAMIC_END) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Invalid MVT local Index %d unit %d\n"),
                               SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                }
                SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;
                break;
        }

        SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = soc_property_get(unit, spn_PACKET_ADJUST_FORMAT, SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat);
        if ( (SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat != 0) &&
                             (SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat != 1) ) {
            SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = 0;
        }

    } else if (SOC_IS_SBX_BME3200(unit)) {
        /* Fill in BM3200 specific configuration */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId           = soc_property_get(unit, spn_BME_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId);
        SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId);
        SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks);

        /* Bm device mode
         */
        switch (SOC_SBX_CFG_BM3200(unit)->uDeviceMode = soc_property_get(unit, spn_BM_DEVICE_MODE, SOC_SBX_CFG_BM3200(unit)->uDeviceMode)) {
            case SOC_SBX_BME_ARBITER_MODE:
                /* BME only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_USER_MAX_NUM_NODES;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_XBAR_MODE:
                /* SE only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_ARBITER_XBAR_MODE:
                /* BME+SE */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_LCM_MODE:
                /* LCM only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                /*
                 * Find out LCM Xcfg and link usage from the user defined lcm_dataplane_x_map
                 */
                if( soc_property_get(unit, spn_LCM_PASSTHROUGH_MODE, 0)) {
                    /* Default to be non-passthrough */
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;
                    for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                } else {
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 1;
                    for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_1_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane1_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                }
                /* Serializer usage mask */
                for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                    if ( SOC_SBX_CFG_BM3200(unit)->linkState[port].nState == SB_FAB_DEVICE_SERIALIZER_STATE_DATA ) {
                        if (port < 32) {
                            SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo |= 0x1 << port;
                        } else {
                            SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi |= 0x1 << (port-32);
                        }
                    }
                }
                /* bLcmXcfgABInputPolarityReversed */
                SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed = soc_property_get(unit, spn_LCM_XCFG_AB_INPUT_POLARITY_REVERSED,
                                                                              SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed );
                break;
            default :
                /* Default to BME+SE */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
        }

        /* Get Link config from user defined config
         */
        for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverEqualization);

            if (port < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                /* User defines for SCIs */
                if(port < 32 && soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
                    /* Links for bandwidth manager */
                    SOC_SBX_CFG_BM3200(unit)->uSerializerMask |= 1 << port;
                    SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_CONTROL;
                } else {
                    /* Links for crossbar */
                    SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                }
            } else {
                /* Reserved links */
                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
            }
        }
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /* Fill in Bm9600 specific configuration */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId           = soc_property_get(unit, spn_BME_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId);
        SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId);
        SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks);

        SOC_SBX_CFG(unit)->enable_all_egress_nodes = TRUE;
        SOC_SBX_CFG(unit)->enable_all_egress_nodes = soc_property_get(unit, spn_ENABLE_ALL_MODULE_ARBITRATION,
                                                                            SOC_SBX_CFG(unit)->enable_all_egress_nodes);
        SOC_SBX_CFG(unit)->enable_all_egress_nodes =
               (SOC_SBX_CFG(unit)->enable_all_egress_nodes != FALSE) ? TRUE : FALSE;

	SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = FALSE;

        /* Bm device mode
         */
        switch (SOC_SBX_CFG_BM9600(unit)->uDeviceMode = soc_property_get(unit, spn_BM_DEVICE_MODE, SOC_SBX_CFG_BM9600(unit)->uDeviceMode)) {
            case SOC_SBX_BME_ARBITER_MODE:
                /* BME only */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_USER_MAX_NUM_NODES;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_XBAR_MODE:
                /* SE only */
	        SAND_HAL_WRITE(unit, PL, PI_CLOCK_GATE_CONFIG1, 0);
	        SAND_HAL_WRITE(unit, PL, PI_CLOCK_GATE_CONFIG2, 2);

                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_ARBITER_XBAR_MODE:
                /* BME+SE */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_LCM_MODE:
                /* LCM only */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                /*
                 * Find out LCM Xcfg and link usage from the user defined lcm_dataplane_x_map
                 */
                if( soc_property_get(unit, spn_LCM_PASSTHROUGH_MODE, 0)) {
                    /* Default to be non-passthrough */
                    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;
                    for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                } else {
		    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 1;
		    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 1;
		    for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
			if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
			    if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
				SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[0][port] = property;
				SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
				SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
			    } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_1_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "soc_sbx_init: unit %d lcm_dataplane1_map_port%d value %d out of range [0, %d]"),
                                           unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                }
                /* Serializer usage mask */
                for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
                    if ( SOC_SBX_CFG_BM9600(unit)->linkState[port].nState == SB_FAB_DEVICE_SERIALIZER_STATE_DATA ) {
                        if (port < 32) {
                            SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo |= 0x1 << port;
                        } else {
                            SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi |= 0x1 << (port-32);
                        }
                    }
                }
                /* bLcmXcfgABInputPolarityReversed */
                SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed = soc_property_get(unit, spn_LCM_XCFG_AB_INPUT_POLARITY_REVERSED,
                                                                              SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed );
                break;
            default :
                /* Default to BME+SE */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
        }

        /* Get Link config from user defined config
         */
        for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverEqualization);

            if (port < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
		  if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
		      (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {

		      /* User defines for SCIs */
		      if( soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
			  /* Links for bandwidth manager */
              /* coverity[large_shift : FALSE] */
			  SOC_SBX_CFG_BM9600(unit)->uSerializerMask |= 1 << port;
			  SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_CONTROL;
		      }
		  } else {
		      /* Links for crossbar */
		      SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
		  }
            } else {
                /* Reserved links */
                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
            }
	}
        for (node = 0; node < SBX_MAXIMUM_NODES; node++) {

	    modid = soc_property_port_get(unit, node, spn_SCI_PORT_MODID, -1);

            if (modid != -1 && node != BCM_STK_MOD_TO_NODE(modid)) {
                SOC_SBX_CFG(unit)->l2p_node[BCM_STK_MOD_TO_NODE(modid)] = node;
                SOC_SBX_CFG(unit)->p2l_node[node] = BCM_STK_MOD_TO_NODE(modid);

                if (SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)]
                                            == BCM_STK_MOD_TO_NODE(modid)) {
                    SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)] = SBX_MAXIMUM_NODES - 1;
                }
            }
        }

	SOC_SBX_CFG(unit)->uInterfaceProtocol = soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol);
  	if (SOC_SBX_CFG(unit)->uInterfaceProtocol >= 2) {
	  LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_sbx_init: unit %d if_protocol value %d out of range [0, 1]"),
                     unit, soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol)));
	  return SOC_E_PARAM;
	}

	/* All levels and all nodes in each level are used for egress scheduler */
	SOC_SBX_CFG(unit)->num_egress_group = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;

    } else if (SOC_IS_SBX_SIRIUS(unit)) {
        /* Fill in Sirius specific configuration */
        if (soc_property_get(unit, spn_IF_SUBPORTS_CREATE, 0)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, IF_SUBPORTS_CREATE);
        }
        if (soc_property_get(unit, spn_TM_FABRIC_PORT_HIERARCHY_SETUP, 1)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, TM_FABRIC_PORT_HIERARCHY_SETUP);
        }
        if (soc_property_get(unit, spn_ES_FABRIC_PORT_HIERARCHY_SETUP, 1)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP);
        } else {
	    /* when fabric port (level 2) was not setup, fabric egress (level 1/0) setup could not be done */
	    SOC_SBX_CFG(unit)->fabric_egress_setup = FALSE;
	}

        SOC_SBX_CFG(unit)->uClockSpeedInMHz = soc_property_get(unit, spn_CORE_CLOCK_SPEED,
							       SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns = soc_property_get(unit,spn_EXT_RAM_COLUMNS,
								  SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows = soc_property_get(unit,spn_EXT_RAM_ROWS,
							       SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks = soc_property_get(unit,spn_EXT_RAM_BANKS,
								SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories = soc_property_get(unit,spn_EXT_RAM_PRESENT,
								       SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz = soc_property_get(unit, spn_SIRIUS_DDR3_CLOCK_MHZ,
								    SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade = soc_property_get(unit, spn_SIRIUS_DDR3_MEM_GRADE,
								    SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade);

	/* Sanity check DDR config */
	if (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks != 8) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_init: unit %d ext_mem_banks value %d unsupported"),
                       unit, SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks));
	    return SOC_E_PARAM;
	}
	switch (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories) {
	    case 10:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 8:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 6:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 5:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 4:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 3:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 2:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 8192) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 2048)) {
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		    return SOC_E_PARAM;
		}
		break;
	    default:
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_init: unit %d invalid ext mem column/row config"), unit));
		return SOC_E_PARAM;
	}

    /* default 0 - root_relay parent may have single child */
    SOC_SBX_CFG_SIRIUS(unit)->tsChildPassThroughDisable = soc_property_get(unit, "ts_child_passthrough_disable", 0);
        SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = BM9600_BW_MAX_VOQ_NUM;

        if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME ||
            SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) {
            SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = 0;
        }

        if ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) || 
            (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) || 
            SOC_SBX_CFG(unit)->bHybridMode) {
            /* Local grants required, config TS */

            /* All levels and all nodes in each level are used for ingress scheduler */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[0] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L0;          /* 64K leaf nodes */
            if ( soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT) <= 4 ) {
                /* each level 1 node has 4 children */
                SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
                SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1;      /* 16K level 1 nodes */
            } else {
                /* each level 1 node has 8 children */
                SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = TRUE;
                SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1/2;    /* 8K level 1 nodes */
            }

            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[2] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L2;          /* 4K level 2 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[3] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L3;          /* 1K level 3 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[4] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L4;          /* 264 level 4 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[5] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L5;          /* 132 level 5 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[6] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L6;          /* 56 level 6 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[7] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L7;          /* 7 level 7nodes */

            SOC_SBX_CFG(unit)->num_ingress_scheduler = 0;
            SOC_SBX_CFG(unit)->num_ingress_multipath = 0;
            for (level = 1; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
		/* memory footprint optimization: no need to reserve logical scheduler for level 0 nodes */
                SOC_SBX_CFG(unit)->num_ingress_scheduler += SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level];
		if ((level >= 2) && (level <= 6)) {
		    /* only level 2 to 6 support ingress multipath shaper */
		    SOC_SBX_CFG(unit)->num_ingress_multipath += SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level];
		}
            }
	    /* multipath shaper need at least 2 members each, assume half of all nodes should be enough */
	    SOC_SBX_CFG(unit)->num_ingress_multipath = SOC_SBX_CFG(unit)->num_ingress_multipath / 2;
        }

	SOC_SBX_CFG(unit)->uInterfaceProtocol = soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol);
	if (SOC_SBX_CFG(unit)->uInterfaceProtocol >= 2) {
	  LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_sbx_init: unit %d if_protocol value %d out of range [0, 1]"),
                     unit, soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol)));
	  return SOC_E_PARAM;
	}

        /*
         *  Set split point in ep_oi2qb_map table.  Below this point is space
         *  multicast will use for repId to OI translation.  At and above this
         *  point is space that will be used for requeueing.
         *
         *  Valid range for this is 0x080..0x100 in XGS mode,
         *  and 0x080..0x0F0 in SBX mode.  Values below 0x080 may cause some
         *  issues with multicast.  Values above 0x0F0 may cause problems with
         *  SBX mode requeue behaviour.  The value 0x100 indicates that the
         *  entire space goes to multicast, and can only be used in XGS mode.
         */
        if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
            /* Use all of ep_oi2qb_map for multicast in XGS mode */
            SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage = 0x100;
        } else {
            /* Use high 8K of ep_oi2qb_map for requeue in SBX mode */
            SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage = 0x0F0;
        }

	/* All levels and all nodes in each level are used for egress scheduler */
	SOC_SBX_CFG(unit)->num_egress_group = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;

        for(port=0; port<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverEqualization);
        }

        /* Local grant config */
        if (SOC_SBX_CFG_SIRIUS(unit)->bDualLocalGrants && !SOC_SBX_CFG(unit)->bHybridMode) {
            /*
             * when dual grant, 1st grant goes to plane A interfaces while 2nd grant goes to
             * plane B interfaces. Evenly distribute mc/higig interfaces on plane A and plane B
             * assign cpu interface to plane A. When a port is disabled, set the child nodes
             * of the interface to always report priority 0
	     plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	     for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
	       intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
	       if (soc_property_port_get(unit, hg, spn_IF_SUBPORTS, SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf])) {
	         SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = plane;
	         if (plane == SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A) {
	           plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	         } else {
	           plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	         }
	       }
	     }
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    */
	    /* hardcode plane for higig interfaces for easier NULL grants management */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG2_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG3_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;	    
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
        } else {
            /*
             * When not dual grant, all enabled interfaces are at plane A.
             * all disabled interfaces are at plane B
             */
            for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
                intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
                if (soc_property_port_get(unit, hg, spn_IF_SUBPORTS, SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf])) {
                    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
                } else {
                    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
                }
            }
            /* MC0, MC1, CPU are assumed to be always enabled and on plane A */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
        }

	/* count total number of configed higig subports */
	for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
	    intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
            SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts += soc_property_port_get(unit, hg, spn_IF_SUBPORTS,
                                                                                 SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf]);
	}
        if (SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts > 128) {
	    /* support this feature only under TME or INLINE mode for now */
	    if ((SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME) &&
		(SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME_BYPASS)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_init: unit %d only TME or TME_BYPASS mode supports more than 128 ports"), unit));
		return SOC_E_PARAM;
	    }

	    /* when more than 128 subports, has to use independent fc mode */
	    SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = TRUE;
	} else {
	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = FALSE;
	}

        for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; intf++) {
            /* don't assign weight to interfaces */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfaceWeight[intf] = 0;

            /* try to configure the interface length adjustment */
            port = soc_property_port_get(unit, intf, spn_QE_EGR_SHAPING_ADJUST, -1);
            if ((0 > port) || (31 < port)) {
                port = soc_property_get(unit, spn_QE_EGR_SHAPING_ADJUST, -1);
            }
            if ((0 <= port) && (31 >= port)) {
                SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[intf] = port;
            }

            /* collect diagnostic mode settings */
            port = soc_property_port_get(unit, intf, spn_PORT_DIAG_MODE, 0);
            if (port) {
                portStr = soc_property_port_get_str(unit, intf, spn_PORT_DIAG_HEADER);
                port_default_speed = 0;
                port_speed = 0;
                if ((!portStr) || (sal_strlen(portStr) < 16)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_init: unit %d port"
                                          " %d property %s requires a"
                                          " string of sixteen hexadecimal"
                                          " digits (and must be included"
                                          " if %s is true)\n"),
                               unit,
                               intf,
                               spn_PORT_DIAG_HEADER,
                               spn_PORT_DIAG_MODE));
                    return SOC_E_PARAM;
                }
                for (i = 0; i < 16; i++) {
                    if (('0' <= portStr[i]) &&
                        ('9' >= portStr[i])) {
                        property = portStr[i] - '0';
                    } else if (('a' <= portStr[i]) &&
                               ('f' >= portStr[i])) {
                        property = portStr[i] - ('a' - 10);
                    } else if (('A' <= portStr[i]) &&
                               ('F' >= portStr[i])) {
                        property = portStr[i] - ('A' - 10);
                    } else {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "soc_sbx_init: unit %d"
                                              " port %d property %s"
                                              " requires a string of"
                                              " sixteen hexadecimal"
                                              " digits.  %c is not a"
                                              " hexadecimal digit.\n"),
                                   unit,
                                   intf,
                                   spn_PORT_DIAG_HEADER,
                                   portStr[i]));
                        return SOC_E_PARAM;
                    }
                    if (i < 8) {
                        port_speed = (port_speed << 4) | (property & 0xF);
                    } else {
                        port_default_speed = (port_default_speed << 4) |
                                             (property & 0xF);
                    }
                }
                /* mark interface as in diagnostic mode */
                SOC_SBX_CFG_SIRIUS(unit)->if_diag_mode[intf] = 1;
                /* now header is loaded, parse it */
                /* K.SOP not adjustable, so ignore it in the setting */
                /* ECT */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_ECTf,
                                  (port_speed >> 23) & 0x1);
                /* ECN */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_ECNf,
                                  (port_speed >> 22) & 0x1);
                /* Test */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_Tf,
                                  (port_speed >> 21) & 0x1);
                /* Multicast */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_MCf,
                                  (port_speed >> 20) & 0x1);
                /* LEN_ADJ_IDX (also used for TC in some cases) */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_LEN_ADJ_IDXf,
                                  (port_speed >> 16) & 0xF);
                /* Destination */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_1r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][1]),
                                  NOHEAD_QUEUEf,
                                  port_speed & 0xFFFF);
                /* Source */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_SIDf,
                                  (port_default_speed >> 16) & 0xFFFF);
                /* LBID */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_0r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][0]),
                                  NOHEAD_LBIDf,
                                  (port_default_speed >> 8) & 0xFF);
                /* DP */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_1r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][1]),
                                  NOHEAD_DPf,
                                  (port_default_speed >> 6) & 0x3);
                /* Overlay type */
                soc_reg_field_set(unit,
                                  RB_IF0_NOHEAD_FIELDS_1r,
                                  &(SOC_SBX_CFG_SIRIUS(unit)->if_diag_nohead[intf][1]),
                                  NOHEAD_TYPEf,
                                  port_default_speed & 0x3F);
            } else {
                /* first char indicates FALSE, disable */
                SOC_SBX_CFG_SIRIUS(unit)->if_diag_mode[intf] = 0;
            }
        }

        SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs = soc_property_get(unit,
                        spn_QE_MAX_ARRIVAL_RATE, SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs);
        SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit = soc_property_get(unit,
                        spn_QE_THRESH_DROP_LIMIT, SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit);
    }


#ifdef BCM_WARM_BOOT_SUPPORT
    /*
     * Currently, Warmboot or LCMs
     */
    if (!(SOC_IS_SBX_BME3200(unit) && (SOC_SBX_CFG_BM3200(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE)))
    {
 	SOC_IF_ERROR_RETURN(soc_sbx_wb_init(unit));
    }
#endif


    /* Set feature cache */
    if (SOC_DRIVER(unit)->feature) {
        soc_feature_init(unit);
    }

    /*
     * PHY drivers and ID map
     */
    SOC_IF_ERROR_RETURN(soc_phyctrl_software_init(unit));

    rv = sbx->init_func(unit, SOC_SBX_CFG(unit));
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_init: unit %d "
                              "init_func failed e=%d\n"), unit, rv));
        return rv;
    }

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
        int                 i, j;
        int                 numq = 0, cmc_numq;

        for (i = 0; i < SOC_CMCS_NUM(unit); i++) {
            /* Clear the CPU & ARM queues */
            SHR_BITCLR_RANGE(CPU_ARM_QUEUE_BITMAP(unit, i),
                             0, NUM_CPU_COSQ(unit));
            if (i == SOC_PCI_CMC(unit)) {
                NUM_CPU_ARM_COSQ(unit, i) =
                    soc_property_uc_get(unit, 0, spn_NUM_QUEUES,
                                        NUM_CPU_COSQ(unit));
            } else {
                /* Properties presume it is PCI for first CMC, then the UC's */
                j = (i < SOC_PCI_CMC(unit)) ? (i + 1) : i;
                NUM_CPU_ARM_COSQ(unit, i) =
                    soc_property_uc_get(unit, j, spn_NUM_QUEUES, 0);
            }

            cmc_numq = NUM_CPU_ARM_COSQ(unit, i);
            SHR_BITSET_RANGE(CPU_ARM_QUEUE_BITMAP(unit, i),
                             numq, cmc_numq);
            numq += cmc_numq;

            if (numq > NUM_CPU_COSQ(unit)) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "soc_do_init: total cpu and arm cosq %04x unexpected\n"),
                          numq));
            }
        }
    }
#endif /* CMICM Support */
 
    /*
     * Reset fusion core on HG XE
     */



    sbx->init = 1;  /* HW Initialized */
    if (soc->soc_flags & SOC_F_INITED) {
        soc->soc_flags |= SOC_F_RE_INITED;
    }
    soc->soc_flags |= SOC_F_INITED;

    return rv;
}


int
soc_sbx_reset_init(int unit) {
    /* force the reset */
    SOC_SBX_CFG(unit)->reset_ul = 1;

    return soc_sbx_init(unit);
}


/*
 * Function:
 *     soc_sbx_block_find
 * Purpose:
 *     Find block and returns index to SOC block info array.
 * Parameters:
 *     unit   - Device number
 *     type   - Block type to match SOC_BLK_xxx
 *     number - Block instance to match
 * Returns:
 *     >= 0  Index to soc_block_info array in SOC driver
 *     -1    If block type/instance was not found
 */
int
soc_sbx_block_find(int unit, int type, int number)
{
    int blk;

    for (blk = 0; ; blk++) {

        if (SOC_BLOCK_INFO(unit, blk).type < 0) { /* End of list */
            return -1;
        }

        if ((SOC_BLOCK_INFO(unit, blk).type   == type) &&
            (SOC_BLOCK_INFO(unit, blk).number == number)) { /* Found */
            return blk;
        }
    }

    return -1;
}

int
soc_sbx_node_port_get(int unit, int module_id, int port,
                      int *fab_unit, int *node_id, int *fabric_port)
{
    int modport;
    int mymod;
    int node;

    if (SOC_IS_SBX_FE(unit)) {
        mymod = SOC_SBX_CONTROL(unit)->module_id0;
    } else if (SOC_IS_SBX_QE(unit)) {
        mymod = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id);
    } else {
        return SOC_E_UNIT;
    }

    if (port >= SBX_MAX_PORTS) {
        return SOC_E_PARAM;
    }

    modport = SOC_SBX_CONTROL(unit)->modport[module_id][port];
    if (BCM_GPORT_IS_CHILD(modport)) {
        node = BCM_GPORT_CHILD_MODID_GET(modport) - SBX_QE_BASE_MODID;
        *fabric_port = BCM_GPORT_CHILD_PORT_GET(modport);
    } else {
        node = ((modport >> 16) & 0xffff) - SBX_QE_BASE_MODID;
        *fabric_port = modport & 0xffff;
    }

    if (node < 0 || node > SBX_MAX_NODES) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "invalid modport found on unit %d "
                              " module %d port %d:\n  (fabric_node=%d "
                              "fabric_modid=%d fabric_port=%d)\n"),
                   unit, module_id, port, node,
                   modport >> 16, modport & 0xffff));
        return SOC_E_PARAM;
    }

    *node_id = node;

    if (module_id == mymod) {
        *fab_unit = SOC_SBX_QE_FROM_FE(unit, port);
    } else {
        *fab_unit = SBX_INVALID_UNIT;
    }

    return SOC_E_NONE;
}


int
soc_sbx_modid_get(int unit, int f_node, int f_port, int *module)
{
    if (!(SOC_SBX_NODE_VALID(unit, f_node)))
        return SOC_E_NOT_FOUND;

        SOC_SBX_MODID_FROM_NODE(f_node, *module);
    

    return SOC_E_NONE;
}

int
soc_sbx_register(int unit, const soc_sbx_functions_t *sbx_functions)
{
    soc_control_t *drv;
    soc_sbx_control_t *sbx_drv;

    drv = SOC_CONTROL(unit);
    if (!drv->soc_flags & SOC_F_ATTACHED) {
        return SOC_E_FAIL;
    }

    sbx_drv = SOC_SBX_CONTROL(unit);
    if (sbx_drv == NULL) {
        return SOC_E_FAIL;
    }

    sbx_drv->sbx_functions.sram_init = sbx_functions->sram_init;
    sbx_drv->sbx_functions.ddr_train = sbx_functions->ddr_train;
    return SOC_E_NONE;
}

int
soc_sbx_unregister(int unit)
{
    soc_control_t *drv;
    soc_sbx_control_t *sbx_drv;

    drv = SOC_CONTROL(unit);
    if (!drv->soc_flags & SOC_F_ATTACHED) {
        return SOC_E_FAIL;
    }

    sbx_drv = SOC_SBX_CONTROL(unit);
    if (sbx_drv == NULL) {
        return SOC_E_FAIL;
    }

    sbx_drv->sbx_functions.sram_init = NULL;

    return SOC_E_NONE;
}

void
soc_sbx_xport_type_update(int unit, soc_port_t port, int to_hg_port)
{
    soc_info_t          *si;
    soc_port_t          it_port;

    si = &SOC_INFO(unit);

    /* We need to lock the SOC structures until we finish the update */
    /* SOC_CONTROL_LOCK(unit); */

    if (to_hg_port) {
        /* SOC_PBMP_PORT_ADD(si->st.bitmap, port); */
        SOC_PBMP_PORT_ADD(si->hg.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
    } else {
        SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
        SOC_PBMP_PORT_ADD(si->xe.bitmap, port);
        /* SOC_PBMP_PORT_REMOVE(si->st.bitmap, port); */
        SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
    }

    /* Local _ptype_str is required below to avoid hardcoded NULL in
     * printf format, which otherwise may cause a compiler warning.
     */
#undef RECONFIGURE_PORT_TYPE_INFO
#define RECONFIGURE_PORT_TYPE_INFO(ptype, ptype_str) \
    si->ptype.num = 0; \
    si->ptype.min = si->ptype.max = -1; \
    PBMP_ITER(si->ptype.bitmap, it_port) { \
        char *_ptype_str = ptype_str; \
        if (_ptype_str != NULL) { \
            sal_snprintf(si->port_name[it_port], \
                         sizeof(si->port_name[it_port]), \
                         "%s%d", _ptype_str, si->ptype.num); \
        } \
        si->ptype.port[si->ptype.num++] = it_port; \
        if (si->ptype.min < 0) { \
            si->ptype.min = it_port; \
        } \
        if (it_port > si->ptype.max) { \
            si->ptype.max = it_port; \
        } \
    }

    /* Recalculate port type data */
    RECONFIGURE_PORT_TYPE_INFO(ether, NULL);
    RECONFIGURE_PORT_TYPE_INFO(st, NULL);
    RECONFIGURE_PORT_TYPE_INFO(hg, "hg");
    RECONFIGURE_PORT_TYPE_INFO(xe, "xe");
#undef  RECONFIGURE_PORT_TYPE_INFO

    /* Release SOC structures lock */
    /* SOC_CONTROL_UNLOCK(unit); */
}

int
soc_sbx_set_epoch_length(int unit)
{
    int     rc = SOC_E_NONE, num_queues, num_cos;
    uint32  epoch_length;

    if ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) &&
        !(SOC_SBX_CFG(unit)->bHybridMode)) {
        epoch_length = SB_FAB_TME_EPOCH_IN_TIMESLOTS;
    } else {
        epoch_length = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS;
    }
    /* check if "optimize QOS" is set. Applicable only for FIC mode */
    if ((soc_property_get(unit, spn_FABRIC_QOS_OPTIMIZE, FALSE) == TRUE) &&
        (SOC_SBX_CFG(unit)->bHybridMode == FALSE) &&
        (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC)) {

        num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);
        /* unicast and multicast queues */
        num_queues = (SOC_SBX_CFG(unit)->cfg_num_nodes *
                      SOC_SBX_CFG(unit)->max_ports * num_cos) +
                     (SOC_SBX_CFG(unit)->num_ds_ids * num_cos);
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit: %d, NumQueues: %d(0x%x)\n"),
                     unit, num_queues, num_queues));

        switch (SOC_SBX_CFG(unit)->uFabricConfig) {
#ifdef BCM_BME3200_SUPPORT
        case SOC_SBX_SYSTEM_CFG_DMODE: /* BM3200 + QE2000 */
            SOC_SBX_CFG(unit)->num_queues = num_queues;
            rc = soc_bm3200_epoch_in_timeslot_config_get(unit, num_queues,
                                                         &epoch_length);
            break;
#endif
        case SOC_SBX_SYSTEM_CFG_VPORT:        /* Vport - Bm9600 + Qe4000 */
        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* Vport Legacy - Bm9600 + Qe2000 */
#ifdef BCM_BM9600_SUPPORT
        case SOC_SBX_SYSTEM_CFG_VPORT_MIX:    /* Vport Mix - Bm9600 + Qe2000 + Qe4000 */
            SOC_SBX_CFG(unit)->num_queues = num_queues;
            rc = soc_bm9600_epoch_in_timeslot_config_get(unit, num_queues,
                                                         &epoch_length);
#endif
        default:
            /* default epoch length assigned above */
            break;
        }
    }

    SOC_SBX_CFG(unit)->epoch_length_in_timeslots = epoch_length;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "unit-%d: epochLen: %d(0x%x)\n"),
                 unit, epoch_length, epoch_length));
    return rc;
}

soc_sbx_ucode_erh_t
soc_sbx_configured_ucode_erh_get(int unit)
{
    char *ucodetype;

    if (SOC_IS_SBX_CALADAN3(unit)) {
        ucodetype = soc_property_get_str(unit, spn_BCM88030_UCODE);
        if (ucodetype == NULL) {
            ucodetype = "g3p1";
        }
    } else {
        ucodetype = NULL;
    }

    if (!ucodetype) {
        return SOC_SBX_G2P3_ERH_DEFAULT;
    } else if (strstr(ucodetype, "g3p1a") || strstr(ucodetype, "t3p1a")) {
        return SOC_SBX_G3P1_ERH_ARAD;
    } else if (strstr(ucodetype, "g3p1") || strstr(ucodetype, "t3p1")) {
        return SOC_SBX_G3P1_ERH_SIRIUS;
    } else if (strstr(ucodetype, "ss")) {
        return SOC_SBX_G2P3_ERH_SIRIUS;
    } else if (strstr(ucodetype, "qess")) {
        return SOC_SBX_G2P3_ERH_QESS;
    } else {
        return SOC_SBX_G2P3_ERH_DEFAULT;
    }
}

soc_sbx_ucode_type_t
soc_sbx_configured_ucode_get(int unit)
{
    char *ucodetype;

    if (SOC_IS_SBX_CALADAN3(unit)) {
        ucodetype = soc_property_get_str(unit, spn_BCM88030_UCODE);
        if (ucodetype == NULL) {
            ucodetype = "g3p1";
        }
    } else {
        ucodetype = NULL;
    }

    /* default to g2p3 */
    if (!ucodetype || strstr(ucodetype, "g2p3")) {
        return SOC_SBX_UCODE_TYPE_G2P3;
    } else if (strstr(ucodetype, "g3p1")) {
        return SOC_SBX_UCODE_TYPE_G3P1;
    } else if (strstr(ucodetype, "t3p1")) {
        return SOC_SBX_UCODE_TYPE_T3P1;
    } else {
        /* else its customer ucode */
        return SOC_SBX_UCODE_TYPE_G2XX;
    }
}


#if defined(BROADCOM_DEBUG)

#define P bsl_printf
int
soc_sbx_dump(int unit, const char *pfx)
{
    soc_control_t       *soc;
    uint16              dev_id;
    uint8               rev_id;

    if (!SOC_UNIT_VALID(unit)) {
        return(SOC_E_UNIT);
    }

    soc = SOC_CONTROL(unit);

    P("%sUnit %d Driver Control Structure:\n", pfx, unit);

    soc_cm_get_id(unit, &dev_id, &rev_id);

    P("%sChip=%s Rev=0x%02x Driver=%s\n",
      pfx,
      soc_dev_name(unit),
      rev_id,
      SOC_CHIP_NAME(soc->chip_driver->type));
    P("%sFlags=0x%x:",
      pfx, soc->soc_flags);
    if (soc->soc_flags & SOC_F_ATTACHED)        P(" attached");
    if (soc->soc_flags & SOC_F_INITED)          P(" initialized");
    if (soc->soc_flags & SOC_F_LSE)             P(" link-scan");
    if (soc->soc_flags & SOC_F_SL_MODE)         P(" sl-mode");
    if (soc->soc_flags & SOC_F_POLLED)          P(" polled");
    if (soc->soc_flags & SOC_F_URPF_ENABLED)    P(" urpf");
    if (soc->soc_flags & SOC_F_MEM_CLEAR_USE_DMA) P(" mem-clear-use-dma");
    if (soc->soc_flags & SOC_F_IPMCREPLSHR)     P(" ipmc-repl-shared");
    if (soc->remote_cpu)                P(" rcpu");
    P("; board type 0x%x", soc->board_type);
    P("\n");
    P("%s", pfx);
    soc_cm_dump(unit);

    return(0);
}
#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *     soc_sbx_chip_dump
 * Purpose:
 *     Display SBX driver and chip information.
 * Parameters:
 *      unit - Device unit number
 *      d    - Device driver
 * Notes:
 *      Pass unit -1 to avoid referencing unit number.
 */
void
soc_sbx_chip_dump(int unit, soc_driver_t *d)
{
#ifdef BROADCOM_DEBUG

    soc_info_t          *si;
    int                 i, count = 0;
    soc_port_t          port;
    char                pfmt[SOC_PBMP_FMT_LEN];
    uint16              dev_id;
    uint8               rev_id;
    int                 blk, blk_number, blk_index;
    soc_block_t         blk_type;
    char                *bname;
    soc_sbx_control_t   *sbx;

    if (d == NULL) {
        P("unit %d: no driver attached\n", unit);
        return;
    }

    if (((unit) < 0) || ((unit) >= SOC_MAX_NUM_DEVICES)) {
        P("invalid unit:%d\n", unit);
        return;
    }

    sbx = SOC_SBX_CONTROL(unit);

    P("driver %s (%s)\n", SOC_CHIP_NAME(d->type), d->chip_string);
    P("\tregsfile\t\t%s\n", d->origin);
    P("\tpci identifier\t\tvendor 0x%04x device 0x%04x rev 0x%02x\n",
      d->pci_vendor, d->pci_device, d->pci_revision);
    P("\tclasses of service\t%d\n", d->num_cos);
    P("\tmaximums\t\tblock %d ports %d mem_bytes %d\n",
      SOC_MAX_NUM_BLKS, SOC_MAX_NUM_PORTS, SOC_MAX_MEM_BYTES);

    /* Device Block Information */
    if (d->block_info) {
        for (blk = 0; d->block_info[blk].type >= 0; blk++) {
            bname = soc_block_name_lookup_ext(d->block_info[blk].type, unit);
            P("\tblk %d\t\t%s%d\tschan %d cmic %d\n",
              blk,
              bname,
              d->block_info[blk].number,
              d->block_info[blk].schan,
              d->block_info[blk].cmic);
        }

        /* Device Block Port Information */
        if (d->port_info) {

            for (port = 0; ; port++) {

                /* Front-panel port information */
                blk        = SOC_PORT_BLOCK(unit, port);
                blk_type   = SOC_PORT_BLOCK_TYPE(unit, port);
                blk_number = SOC_PORT_BLOCK_NUMBER(unit, port);
                blk_index  = SOC_PORT_BLOCK_INDEX(unit, port);

                if (blk < 0 && blk_index < 0) {    /* end of list */
                    break;
                }
                if (blk < 0) {                     /* empty slot */
                    continue;
                }

                P("\tport %d\t\t%s\tblk %d %s%d",
                  port,
                  (blk_type == SOC_BLK_GXPORT) ?
                  (SOC_IS_SIRIUS(unit) ? "hg" : ((SOC_IS_SBX_BM9600(unit) ? (IS_SFI_PORT(unit, port) ? "sfi" : "sci") : "xe"))) :
                  ((blk_type == SOC_BLK_BSE) || (blk_type == SOC_BLK_CSE)) ?
                  (IS_SFI_PORT(unit, port) ? "sfi" : "sci") :
                  soc_block_port_name_lookup_ext(blk_type, unit),
                  blk, soc_block_name_lookup_ext(blk_type, unit),
                  blk_number);

                if (blk_index >= 0) {
                    P(".%d", blk_index);
                }

                /* System-side port information */
                if (sbx && sbx->system_port_info) {
                    blk = SOC_SBX_SYSTEM_PORT_BLOCK(unit, port);
                    if (blk >= 0) {
                        blk_type   = SOC_SBX_SYSTEM_PORT_BLOCK_TYPE(unit,
                                                                    port);
                        blk_number = SOC_SBX_SYSTEM_PORT_BLOCK_NUMBER(unit,
                                                                      port);
                        blk_index  = SOC_SBX_SYSTEM_PORT_BLOCK_INDEX(unit,
                                                                     port);
                        P("\t- system blk %d %s%d.%d",
                          blk, soc_block_name_lookup_ext(blk_type, unit),
                          blk_number, blk_index);
                    }
                }

                P("\n");
            }
        }
    }

    si = &SOC_INFO(unit);
    soc_cm_get_id(unit, &dev_id, &rev_id);

    P("unit %d:\n", unit);
    P("\tpci\t\t\tdevice %04x rev %02x\n", dev_id, rev_id);
    P("\tdriver\t\t\ttype %d (%s) group %d (%s)\n",
      si->driver_type, SOC_CHIP_NAME(si->driver_type),
      si->driver_group, soc_chip_group_names[si->driver_group]);
    P("\tchip\t\t\t%s%s%s%s%s\n",
      SOC_IS_SBX_QE2000(unit) ? "qe2000 " : "",
      SOC_IS_SBX_BME3200(unit) ? "bme3200 " : "",
      SOC_IS_SBX_BM9600(unit) ? "bm9600 " : "",
      SOC_IS_SBX_SIRIUS(unit) ? "sirius " : "",
      SOC_IS_SBX(unit) ? "sbx " : "");
    P("\tmax modid\t\t%d\n", si->modid_max);
    P("\tnum ports\t\t%d\n", si->port_num);
    P("\tnum modids\t\t%d\n", si->modid_count);
    P("\tnum blocks\t\t%d\n", si->block_num);

    if (si->ge.num)
      P("\tGE ports\t%d\t%s (%d:%d)\n",
        si->ge.num, SOC_PBMP_FMT(si->ge.bitmap, pfmt),
        si->ge.min, si->ge.max);
    if (si->xe.num)
      P("\tXE ports\t%d\t%s (%d:%d)\n",
        si->xe.num, SOC_PBMP_FMT(si->xe.bitmap, pfmt),
        si->xe.min, si->xe.max);
    if (si->hg.num)
      P("\tHG ports\t%d\t%s (%d:%d)\n",
        si->hg.num, SOC_PBMP_FMT(si->hg.bitmap, pfmt),
        si->hg.min, si->hg.max);
    if (si->spi.num)
      P("\tSPI ports\t%d\t%s (%d:%d)\n",
        si->spi.num, SOC_PBMP_FMT(si->spi.bitmap, pfmt),
        si->spi.min, si->spi.max);
    if (si->spi_subport.num)
      P("\tSPI subports\t%d\t%s (%d:%d)\n",
        si->spi_subport.num, SOC_PBMP_FMT(si->spi_subport.bitmap, pfmt),
        si->spi_subport.min, si->spi_subport.max);
    if (si->sci.num)
      P("\tSCI ports\t%d\t%s (%d:%d)\n",
        si->sci.num, SOC_PBMP_FMT(si->sci.bitmap, pfmt),
        si->sci.min, si->sci.max);
    if (si->sfi.num)
      P("\tSFI ports\t%d\t%s (%d:%d)\n",
        si->sfi.num, SOC_PBMP_FMT(si->sfi.bitmap, pfmt),
        si->sfi.min, si->sfi.max);
    if (si->ether.num)
      P("\tETHER ports\t%d\t%s (%d:%d)\n",
        si->ether.num, SOC_PBMP_FMT(si->ether.bitmap, pfmt),
        si->ether.min, si->ether.max);
    if (si->port.num)
      P("\tPORT ports\t%d\t%s (%d:%d)\n",
        si->port.num, SOC_PBMP_FMT(si->port.bitmap, pfmt),
        si->port.min, si->port.max);
    if (si->all.num)
      P("\tALL ports\t%d\t%s (%d:%d)\n",
        si->all.num, SOC_PBMP_FMT(si->all.bitmap, pfmt),
        si->all.min, si->all.max);
    SOC_PBMP_COUNT(si->cmic_bitmap, i);
    if (i)
      P("\tCPU port\t%d\t%s (%d)\n",
        i,
        SOC_PBMP_FMT(si->cmic_bitmap, pfmt), si->cmic_port);

    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            count++;
        }
    }
    P("\thas blocks\t%d\t", count);
    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            P("%s ", soc_block_name_lookup_ext(si->has_block[i], unit));
            if ((i) && !(i%6)) {
                P("\n\t\t\t\t");
            }
        }
    }
    P("\n");
    P("\tport names\t\t");
    for (port = 0; port < si->port_num; port++) {
        if (port > 0 && (port % 4) == 0) {
            P("\n\t\t\t\t");
        }
        P("%2d=%-8s ",
          port,
          (si->port_name[port][0] == '?') ? " " : si->port_name[port]);
    }
    P("\n");

    if (d->block_info) {
        i = 0;
        for (blk = 0; d->block_info[blk].type >= 0; blk++) {
            if (SOC_PBMP_IS_NULL(si->block_bitmap[blk])) {
                continue;
            }
            if (++i == 1) {
                P("\tblock bitmap\t");
            } else {
                P("\n\t\t\t");
            }
            P("%d\t%s\t%s (%d ports)",
              blk,
              si->block_name[blk],
              SOC_PBMP_FMT(si->block_bitmap[blk], pfmt),
              si->block_valid[blk]);
        }
        if (i > 0) {
            P("\n");
        }
    }

    {
        soc_feature_t f;

        P("\tfeatures\t");
        i = 0;
        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                if (++i > 3) {
                    P("\n\t\t\t");
                    i = 1;
                }
                P("%s ", soc_feature_name[f]);
            }
        }
        P("\n");
    }

#endif /* BROADCOM_DEBUG */
}

static uint32
_soc_sbx_control_modport_signature(soc_sbx_control_t *drv) {
    int i,j;
    uint32 accum = 0;

    for (i=0; i<SBX_MAX_MODIDS; i++) {
        for (j=0; j<SBX_MAX_PORTS; j++) {
            accum ^= drv->modport[i][j];
        }
    }
    return accum;
}

static uint32
_soc_sbx_control_fabnodeport2feport_signature(soc_sbx_control_t *drv) {
    int i,j;
    uint32 accum = 0;

    for (i=0; i<SBX_MAX_NODES; i++) {
        for (j=0; j<SBX_MAX_PORTS; j++) {
            accum ^= drv->fabnodeport2feport[i][j];
        }
    }
    return accum;
}


static uint32
_soc_sbx_control_trunk_ete_signature(soc_sbx_control_t *drv) {
    return _shr_crc32(0, (unsigned char *)drv->trunk_ete, sizeof(uint32) * SBX_MAX_TRUNKS); 
}

static uint32 
_rotate32(uint32 val, int shift) {
    return ((val << shift) | (val >> (32 - shift)));
}


static uint32
_soc_sbx_control_signature(soc_sbx_control_t *drv) {
    int i;
    uint32 accum = 0;

    accum ^= (uint32)drv->sbhdl;
    accum ^= _rotate32(drv->fabtype, 1);
    accum ^= _rotate32(drv->fetype, 2);
    accum ^= _rotate32(drv->module_id0, 3);
    accum ^= _rotate32(drv->modid_count, 4);
    accum ^= drv->node_id;
    accum ^= _soc_sbx_control_modport_signature(drv); 
    accum ^= _soc_sbx_control_fabnodeport2feport_signature(drv); 
    accum ^= _soc_sbx_control_trunk_ete_signature(drv); 
    accum ^= drv->invalid_l2ete;
    accum ^= _rotate32(drv->stpforward, 5);
    accum ^= _rotate32(drv->stpblock, 6);
    accum ^= _rotate32(drv->stplearn, 7);
    accum ^= _rotate32((uint32)drv->system_port_info, 8);
    accum ^= _rotate32((uint32)drv->init, 9);
    accum ^= _rotate32((uint32)drv->libInit, 10);
    accum ^= _rotate32((uint32)drv->libver, 11);
    accum ^= _rotate32(drv->uver_maj, 12);
    accum ^= _rotate32(drv->uver_min, 13);
    accum ^= _rotate32(drv->uver_patch, 14);
    accum ^= _rotate32(drv->numQueues, 15);
    accum ^= _rotate32(drv->master_unit, 16);
    accum ^= _rotate32(drv->num_slaves, 17);
    accum ^= _rotate32(drv->numQueues, 18);
    for (i=0; i<SBX_MAX_PORTS; i++) {
        accum ^= _rotate32((uint32)drv->fabric_units[i], i%32);
    }
    for (i=0; i<SBX_MAX_PORTS; i++) {
        accum ^= _rotate32((uint32)drv->forwarding_units[i], i%32);
    }
    for (i=0; i<SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS; i++) {
        accum ^= _rotate32(drv->hg_subport_count[i], i%32);
    }
    for (i=0; i<SBX_MAX_PORTS; i++) {
        accum ^= _rotate32(drv->port_ete[i], i%32);
    }
    for (i=0; i<SBX_MAX_PORTS; i++) {
        accum ^= _rotate32(drv->port_ut_ete[i], i%32);
    }
    for (i=0; i<SOC_MAX_NUM_DEVICES; i++) {
        accum ^= _rotate32(drv->slave_units[i], i%32);
    }
    for (i=strlen(drv->uver_name); i>0; i--) {
        accum ^= (uint32)drv->uver_name[i-1] << ((i%4) * 8);
    }
    for (i=strlen(drv->uver_str); i>0; i--) {
        accum ^= (uint32)drv->uver_str[i-1] << ((i%4) * 8);
    }
    return accum;
}


void 
soc_sbx_drv_signature_show(int unit) 
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc drv 0x%08x\n"), 
                 _soc_sbx_control_signature(SOC_SBX_CONTROL(unit))));
}

void
soc_sbx_control_dump(int unit)
{

    soc_sbx_control_t *drv = SOC_SBX_CONTROL(unit);

    cli_out("soc_sbx_control for unit %d, signature 0x%x\n", 
                 unit, _soc_sbx_control_signature(SOC_SBX_CONTROL(unit)));

    cli_out("sbhandle    0x%08x  ", (uint32)drv->sbhdl);
    cli_out("fabtype     0x%08x  ", drv->fabtype);
    cli_out("fetype      0x%08x\n", drv->fetype);

    cli_out("module_id0  0x%08x  ", drv->module_id0);
    cli_out("modid_count 0x%08x  ", drv->modid_count);
    cli_out("node_id     0x%08x  ", drv->node_id);
    cli_out("inval_l2ete 0x%08x\n", drv->invalid_l2ete);

    cli_out("stpforward  0x%08x  ", drv->stpforward);
    cli_out("stpblock    0x%08x  ", drv->stpblock);
    cli_out("stplearn    0x%08x\n", drv->stplearn);

    cli_out("system_port 0x%08x  ", (uint32)drv->system_port_info);
    cli_out("init        0x%08x  ", (uint32)drv->init);
    cli_out("libInit     0x%08x  ", (uint32)drv->libInit);
    cli_out("libver      0x%08x\n", (uint32)drv->libver);

    cli_out("uver_maj    0x%08x  ", drv->uver_maj);
    cli_out("uver_min    0x%08x  ", drv->uver_min);
    cli_out("uver_patch  0x%08x  ", drv->uver_patch);
    cli_out("numQueues   0x%08x\n", drv->numQueues);

    cli_out("master_unit 0x%08x  ", drv->master_unit);
    cli_out("num_slaves  0x%08x  ", drv->num_slaves);
    cli_out("numQueues   0x%08x\n", drv->numQueues);

    cli_out("\nsignatures:             " );
    cli_out("modport     0x%08x  ", _soc_sbx_control_modport_signature(drv)); 
    cli_out("fnodep2fep  0x%08x\n", _soc_sbx_control_fabnodeport2feport_signature(drv)); 
    cli_out("                        trunk_ete   0x%08x\n", _soc_sbx_control_trunk_ete_signature(drv)); 

    cli_out("fabric_units %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->fabric_units[0], (uint32)drv->fabric_units[1], 
                 (uint32)drv->fabric_units[2], (uint32)drv->fabric_units[3], 
                 (uint32)drv->fabric_units[4], (uint32)drv->fabric_units[5], 
                 (uint32)drv->fabric_units[6], (uint32)drv->fabric_units[7], 
                 (uint32)drv->fabric_units[8], (uint32)drv->fabric_units[9]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->fabric_units[10], (uint32)drv->fabric_units[11], 
                 (uint32)drv->fabric_units[12], (uint32)drv->fabric_units[13], 
                 (uint32)drv->fabric_units[14], (uint32)drv->fabric_units[15], 
                 (uint32)drv->fabric_units[16], (uint32)drv->fabric_units[17], 
                 (uint32)drv->fabric_units[18], (uint32)drv->fabric_units[19]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->fabric_units[20], (uint32)drv->fabric_units[21], 
                 (uint32)drv->fabric_units[22], (uint32)drv->fabric_units[23], 
                 (uint32)drv->fabric_units[24], (uint32)drv->fabric_units[25], 
                 (uint32)drv->fabric_units[26], (uint32)drv->fabric_units[27], 
                 (uint32)drv->fabric_units[28], (uint32)drv->fabric_units[29]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->fabric_units[30], (uint32)drv->fabric_units[31], 
                 (uint32)drv->fabric_units[32], (uint32)drv->fabric_units[33], 
                 (uint32)drv->fabric_units[34], (uint32)drv->fabric_units[35], 
                 (uint32)drv->fabric_units[36], (uint32)drv->fabric_units[37], 
                 (uint32)drv->fabric_units[38], (uint32)drv->fabric_units[39]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->fabric_units[40], (uint32)drv->fabric_units[41], 
                 (uint32)drv->fabric_units[42], (uint32)drv->fabric_units[43], 
                 (uint32)drv->fabric_units[44], (uint32)drv->fabric_units[45], 
                 (uint32)drv->fabric_units[46], (uint32)drv->fabric_units[47], 
                 (uint32)drv->fabric_units[48], (uint32)drv->fabric_units[49]);
  
    cli_out("fwd_units    %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->forwarding_units[0], (uint32)drv->forwarding_units[1], 
                 (uint32)drv->forwarding_units[2], (uint32)drv->forwarding_units[3], 
                 (uint32)drv->forwarding_units[4], (uint32)drv->forwarding_units[5], 
                 (uint32)drv->forwarding_units[6], (uint32)drv->forwarding_units[7], 
                 (uint32)drv->forwarding_units[8], (uint32)drv->forwarding_units[9]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->forwarding_units[10], (uint32)drv->forwarding_units[11], 
                 (uint32)drv->forwarding_units[12], (uint32)drv->forwarding_units[13], 
                 (uint32)drv->forwarding_units[14], (uint32)drv->forwarding_units[15], 
                 (uint32)drv->forwarding_units[16], (uint32)drv->forwarding_units[17], 
                 (uint32)drv->forwarding_units[18], (uint32)drv->forwarding_units[19]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->forwarding_units[20], (uint32)drv->forwarding_units[21], 
                 (uint32)drv->forwarding_units[22], (uint32)drv->forwarding_units[23], 
                 (uint32)drv->forwarding_units[24], (uint32)drv->forwarding_units[25], 
                 (uint32)drv->forwarding_units[26], (uint32)drv->forwarding_units[27], 
                 (uint32)drv->forwarding_units[28], (uint32)drv->forwarding_units[29]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->forwarding_units[30], (uint32)drv->forwarding_units[31], 
                 (uint32)drv->forwarding_units[32], (uint32)drv->forwarding_units[33], 
                 (uint32)drv->forwarding_units[34], (uint32)drv->forwarding_units[35], 
                 (uint32)drv->forwarding_units[36], (uint32)drv->forwarding_units[37], 
                 (uint32)drv->forwarding_units[38], (uint32)drv->forwarding_units[39]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->forwarding_units[40], (uint32)drv->forwarding_units[41], 
                 (uint32)drv->forwarding_units[42], (uint32)drv->forwarding_units[43], 
                 (uint32)drv->forwarding_units[44], (uint32)drv->forwarding_units[45], 
                 (uint32)drv->forwarding_units[46], (uint32)drv->forwarding_units[47], 
                 (uint32)drv->forwarding_units[48], (uint32)drv->forwarding_units[49]);
  
    cli_out("hg_subport_count     %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->hg_subport_count[0], (uint32)drv->hg_subport_count[1], 
                 (uint32)drv->hg_subport_count[2], (uint32)drv->hg_subport_count[3], 
                 (uint32)drv->hg_subport_count[4], (uint32)drv->hg_subport_count[5], 
                 (uint32)drv->hg_subport_count[6], (uint32)drv->hg_subport_count[7]);

    cli_out("port_ete     %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->port_ete[0], (uint32)drv->port_ete[1], 
                 (uint32)drv->port_ete[2], (uint32)drv->port_ete[3], 
                 (uint32)drv->port_ete[4], (uint32)drv->port_ete[5], 
                 (uint32)drv->port_ete[6], (uint32)drv->port_ete[7], 
                 (uint32)drv->port_ete[8], (uint32)drv->port_ete[9]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ete[10], (uint32)drv->port_ete[11], 
                 (uint32)drv->port_ete[12], (uint32)drv->port_ete[13], 
                 (uint32)drv->port_ete[14], (uint32)drv->port_ete[15], 
                 (uint32)drv->port_ete[16], (uint32)drv->port_ete[17], 
                 (uint32)drv->port_ete[18], (uint32)drv->port_ete[19]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->port_ete[20], (uint32)drv->port_ete[21], 
                 (uint32)drv->port_ete[22], (uint32)drv->port_ete[23], 
                 (uint32)drv->port_ete[24], (uint32)drv->port_ete[25], 
                 (uint32)drv->port_ete[26], (uint32)drv->port_ete[27], 
                 (uint32)drv->port_ete[28], (uint32)drv->port_ete[29]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ete[30], (uint32)drv->port_ete[31], 
                 (uint32)drv->port_ete[32], (uint32)drv->port_ete[33], 
                 (uint32)drv->port_ete[34], (uint32)drv->port_ete[35], 
                 (uint32)drv->port_ete[36], (uint32)drv->port_ete[37], 
                 (uint32)drv->port_ete[38], (uint32)drv->port_ete[39]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ete[40], (uint32)drv->port_ete[41], 
                 (uint32)drv->port_ete[42], (uint32)drv->port_ete[43], 
                 (uint32)drv->port_ete[44], (uint32)drv->port_ete[45], 
                 (uint32)drv->port_ete[46], (uint32)drv->port_ete[47], 
                 (uint32)drv->port_ete[48], (uint32)drv->port_ete[49]);
  
    cli_out("port_ut_ete  %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->port_ut_ete[0], (uint32)drv->port_ut_ete[1], 
                 (uint32)drv->port_ut_ete[2], (uint32)drv->port_ut_ete[3], 
                 (uint32)drv->port_ut_ete[4], (uint32)drv->port_ut_ete[5], 
                 (uint32)drv->port_ut_ete[6], (uint32)drv->port_ut_ete[7], 
                 (uint32)drv->port_ut_ete[8], (uint32)drv->port_ut_ete[9]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ut_ete[10], (uint32)drv->port_ut_ete[11], 
                 (uint32)drv->port_ut_ete[12], (uint32)drv->port_ut_ete[13], 
                 (uint32)drv->port_ut_ete[14], (uint32)drv->port_ut_ete[15], 
                 (uint32)drv->port_ut_ete[16], (uint32)drv->port_ut_ete[17], 
                 (uint32)drv->port_ut_ete[18], (uint32)drv->port_ut_ete[19]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d - ",
                 (uint32)drv->port_ut_ete[20], (uint32)drv->port_ut_ete[21], 
                 (uint32)drv->port_ut_ete[22], (uint32)drv->port_ut_ete[23], 
                 (uint32)drv->port_ut_ete[24], (uint32)drv->port_ut_ete[25], 
                 (uint32)drv->port_ut_ete[26], (uint32)drv->port_ut_ete[27], 
                 (uint32)drv->port_ut_ete[28], (uint32)drv->port_ut_ete[29]);
    cli_out("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ut_ete[30], (uint32)drv->port_ut_ete[31], 
                 (uint32)drv->port_ut_ete[32], (uint32)drv->port_ut_ete[33], 
                 (uint32)drv->port_ut_ete[34], (uint32)drv->port_ut_ete[35], 
                 (uint32)drv->port_ut_ete[36], (uint32)drv->port_ut_ete[37], 
                 (uint32)drv->port_ut_ete[38], (uint32)drv->port_ut_ete[39]);
    cli_out("             %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                 (uint32)drv->port_ut_ete[40], (uint32)drv->port_ut_ete[41], 
                 (uint32)drv->port_ut_ete[42], (uint32)drv->port_ut_ete[43], 
                 (uint32)drv->port_ut_ete[44], (uint32)drv->port_ut_ete[45], 
                 (uint32)drv->port_ut_ete[46], (uint32)drv->port_ut_ete[47], 
                 (uint32)drv->port_ut_ete[48], (uint32)drv->port_ut_ete[49]);
  
    cli_out("slave_units  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n             0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
                 (uint32)drv->slave_units[0], (uint32)drv->slave_units[1], 
                 (uint32)drv->slave_units[2], (uint32)drv->slave_units[3], 
                 (uint32)drv->slave_units[4], (uint32)drv->slave_units[5], 
                 (uint32)drv->slave_units[6], (uint32)drv->slave_units[7], 
                 (uint32)drv->slave_units[8], (uint32)drv->slave_units[9]);
    cli_out("             0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n             0x%08x 0x%08x 0x%08x\n",
                 (uint32)drv->slave_units[10], (uint32)drv->slave_units[11], 
                 (uint32)drv->slave_units[12], (uint32)drv->slave_units[13], 
                 (uint32)drv->slave_units[14], (uint32)drv->slave_units[15], 
                 (uint32)drv->slave_units[16], (uint32)drv->slave_units[17]);

    cli_out("uver_name   \"%s\"     ", drv->uver_name);
    cli_out("uver_str    \"%s\" \n", drv->uver_str);
}


#ifdef P
#undef P
#endif /* P */



