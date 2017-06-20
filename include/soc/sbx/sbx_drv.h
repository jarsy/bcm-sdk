/*
 * $Id: sbx_drv.h,v 1.280.8.1.4.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much.
 */
#ifndef _SOC_SBX_DRV_H
#define _SOC_SBX_DRV_H

#include <assert.h>

#include <shared/avl.h>
#include <shared/bitop.h>

#include <soc/util.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/scache.h>
#include <soc/feature.h>
#include <soc/property.h>
#include <soc/devids.h>

#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbFabCommon.h>
#include <soc/sbx/sbx_common.h>
#include <soc/sbx/mbCommon.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/lrp.h>
#endif



/*
 * SOC_IS_* Macros.  If support for the chip is defined out of the
 * build, they are defined as zero to let the optimizer remove
 * code.
 */
#ifdef  BCM_QE2000_SUPPORT
#define SOC_IS_SBX_QE2000(unit) \
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_QE2000)
#else
#define SOC_IS_SBX_QE2000(unit)    0
#endif /* BCM_QE2000_SUPPORT */

#ifdef  BCM_BME3200_SUPPORT
#define SOC_IS_SBX_BME3200(unit) \
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_BME3200)
#else
#define SOC_IS_SBX_BME3200(unit)        0
#endif /* BCM_BME3200_SUPPORT */

#ifdef  BCM_BM9600_SUPPORT
#define SOC_IS_SBX_BM9600(unit) \
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_BM9600)
#else
#define SOC_IS_SBX_BM9600(unit)        0
#endif /* BCM_BM9600_SUPPORT */

#define SOC_IS_SBX_FE2000(unit)    0
#define SOC_IS_SBX_FE2KXT(unit)    0

#ifdef  BCM_SIRIUS_SUPPORT
#define SOC_IS_SBX_SIRIUS(unit) \
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_SIRIUS)
#else
#define SOC_IS_SBX_SIRIUS(unit)    0
#endif /* BCM_SIRIUS_SUPPORT */

#ifdef  BCM_CALADAN3_SUPPORT
#define SOC_IS_SBX_CALADAN3(unit) \
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_CALADAN3)
#define SOC_IS_CALADAN3_REVB(unit) \
        (SOC_INFO(unit).driver_type == SOC_CHIP_BCM88030_B0)
#else
#define SOC_IS_SBX_CALADAN3(unit)    0
#endif /* BCM_CALADAN3_SUPPORT */

#define SBX_BCMSIM_PORT_MAX 25

#define SBX_INVALID_MVT_INDEX (-1)
#define SBX_INVALID_NODE      (-1)
#define SBX_INVALID_MODID     (-1)
#define SBX_INVALID_PORT      (-1)
#define SBX_INVALID_TRUNK     (-1)
#define SBX_INVALID_ENCAP_ID  (-1)
#define SBX_INVALID_FTE       (-1)
#define SBX_INVALID_UNIT      (-1)

/**
 * Max defines
 */
#define SBX_MAX_TRUNKS      128
#define SBX_MAX_GPORTS      (8*1024)
#define SBX_MAX_COS         8
#define SBX_MAX_TRUNK_SIZE  8
#define SBX_MAX_USER_OHI    (32 * 1024)
#define SBX_MAX_RXLOAD_FIFO 256

/*
 * BM9600 support upto 72 nodes
 * Assuming each Node can have up to 2 FE,
 * so max modid = 72 * 2 = 144
 */
#ifdef BCM_BM9600_SUPPORT
#if defined(SBX_MAX_ESET) && (1024 > SBX_MAX_ESET)
#undef SBX_MAX_ESET
#define SBX_MAX_ESET        1024
#endif
#endif /* BCM_BM9600_SUPPORT */

#ifdef BCM_BME3200_SUPPORT
#if  defined(SBX_MAX_ESET) && (128 > SBX_MAX_ESET)
#undef SBX_MAX_ESET
#define SBX_MAX_ESET        128
#endif
#endif /* BCM_BME3200_SUPPORT */

#define SBX_MAX_MODIDS      64
#define SBX_MAX_NODES       32

#define SBX_MAXIMUM_NODES   72  /* BCM_STK_MAX_MODULES */

#if defined(BCM_CALADAN3_SUPPORT)
/* Fab ports are included, need API to understand ucode ports and phy ports */


/* 
 * Note these values correlate with max number of uCode ports defined
 * in g3p1 app (uCode application code).
 * This value also correlates to the value SOC_MAX_NUM_PORTS 
 * that is auto-generated. SOC_MAX_NUM_PORTS is 2*SBX_MAX_PORTS.
 */
/*#define BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE 1*/

#if defined(BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE)
#define SBX_MAX_PORTS       60
#else
#define SBX_MAX_PORTS       55
#endif

#define STATIC_ASSERT(cond, msg) \
    typedef char msg[(cond) ? 1 : -1]

STATIC_ASSERT(SOC_MAX_NUM_PORTS >= SBX_MAX_PORTS*2, Static_Assert_For_Soc_Sbx_Max_Num_Check_Failed);

#elif defined(BCM_QE2000_SUPPORT)
#define SBX_MAX_PORTS       50
#else
#define SBX_MAX_PORTS       50
#endif


/*
 * Flag to allow EXT_DDR to be tested with TR50
 */
#ifdef BCM_SIRIUS_SUPPORT
#define SBX_EXT_DDR_FLAG (-1)
#endif

/*
 * For partial builds: when BM32 or BM96 is not defined
 */
#ifndef SBX_MAX_ESET
#define SBX_MAX_ESET        128
#endif

#define SBX_MAX_SID         (1 << 12)
#define SBX_MAX_EXC_QID     1
#define SBX_MAX_QID         (1 << 14)
#define SBX_DEFAULT_MTU_SIZE    9216

typedef enum {
    SOC_SBX_G2P3_ERH_DEFAULT=0,
    SOC_SBX_G2P3_ERH_SIRIUS=1,
    SOC_SBX_G2P3_ERH_QESS=2,
    SOC_SBX_G3P1_ERH_SIRIUS=3,
    SOC_SBX_G3P1_ERH_ARAD=4
}soc_sbx_ucode_erh_t;

typedef enum soc_sbx_ucode_type_e {
    SOC_SBX_UCODE_TYPE_NONE,
    SOC_SBX_UCODE_TYPE_G2P3,
    SOC_SBX_UCODE_TYPE_G2XX,
    SOC_SBX_UCODE_TYPE_G3P1,
    SOC_SBX_UCODE_TYPE_T3P1,
    SOC_SBX_MAX_UCODE_TYPE
} soc_sbx_ucode_type_t;

#define SOC_SBX_G2P3_ERH_LEN_DEFAULT 12
#define SOC_SBX_G2P3_ERH_LEN_SIRIUS  12
#define SOC_SBX_G2P3_ERH_LEN_QESS    14
#define SOC_SBX_G3P1_ERH_LEN_SIRIUS  12
#define SOC_SBX_G3P1_ERH_LEN_ARAD    16

extern char *soc_sbx_ucode_versions[SOC_SBX_MAX_UCODE_TYPE];
extern int sbx_num_cosq[];

/* get the configured, or erh format */
extern soc_sbx_ucode_erh_t  soc_sbx_configured_ucode_erh_get(int unit);

/* get the configured, or default ucode */
extern soc_sbx_ucode_type_t  soc_sbx_configured_ucode_get(int unit);

typedef enum soc_sbx_fetype_e {
    SOC_SBX_FETYPE_FE2K = 0,
    SOC_SBX_FETYPE_FE2KXT,
    SOC_SBX_FETYPE_CALADAN3
} soc_sbx_fetype_t;

#define SBX_PROTOCOL_3_4_SFI_PBMP_OFFSET   9

/*
 * SBX Properties Block Types
 *
 * Values for config properties as defined in 'property.h':
 *
 *     ucode_port_N = 0xbbpp
 * where,
 *     <N>    is the ucode port number
 *     <0xbb> is the block number
 *     <0xpp> is the port within the block
 */
#define SBX_SPN_BLK_CPU    0
#define SBX_SPN_BLK_GE     1
#define SBX_SPN_BLK_XE     2
#define SBX_SPN_BLK_SPI0   3
#define SBX_SPN_BLK_SPI1   4

#define SBX_SPN_BLK_SHIFT  8
#define SBX_SPN_BLK_MASK   0xff00
#define SBX_SPN_PORT_MASK  0x00ff

#define SBX_MAX_FABRIC_COS  16

#define CS_NUM_SEGMENTS     32
#define CS_NUM_BRICKS       32
#define CS_NUM_GROUPS        4
#define SBX_FAB_ERRORS       5

typedef enum fd_drop_type_s {
  FD_ALL,
  FD_GREEN,
  FD_YELLOW,
  FD_RED,
  FD_MC,
  FD_DROP_TYPE_COUNT
} fd_drop_type_t;



#define CS_DMA_CNTR_MSG_LAST          0x1
#define CS_DMA_CNTR_MSG_DUMMY         0x2
#define CS_DMA_CNTR_MSG_UNUSED        0xC
#define CS_DMA_CNTR_ADDR_MASK        ~(CS_DMA_CNTR_MSG_LAST  | \
                                       CS_DMA_CNTR_MSG_DUMMY | \
                                   CS_DMA_CNTR_MSG_UNUSED)

#define CS_DMA_FIFO_MSG_SIZE           (sizeof(soc_sbx_cs_rbuf_entry_t)/4)
#define CS_DMA_FIFO_MEM_ADDR(mem_addr) ((mem_addr) & CS_DMA_CNTR_ADDR_MASK)
#define CS_CFG_GRP_MASK                0xFFFFFC00
#define CS_CFG_GRP_SHIFT               10
#define CS_GRP_SIZE                    (1 << CS_CFG_GRP_SHIFT)
#define CS_SLQ_ENABLE                  0x02
#define CS_GBL_ENABLE                  0x04
#define CS_FLUSHING                    0x08

typedef enum cs_grp_state_s {
  CS_GROUP_FREE,
  CS_GROUP_PROVISIONING,
  CS_GROUP_ACTIVE
} cs_grp_state_t;

typedef enum gbl_stats_s {
  gbl_accepted_dp0,
  gbl_accepted_dp1,
  gbl_accepted_dp2,
  gbl_accepted_dp3,
  gbl_wred_drop_dp0,
  gbl_wred_drop_dp1,
  gbl_wred_drop_dp2,
  gbl_wred_drop_dp3,
  gbl_wred_mark_dp0,
  gbl_wred_mark_dp1,
  gbl_wred_mark_dp2,
  gbl_wred_mark_dp3,
  gbl_nwred_drop_dp0,
  gbl_nwred_drop_dp1,
  gbl_nwred_drop_dp2,
  gbl_nwred_drop_dp3,
  gbl_dequeue
} gbl_stats_t;

typedef enum slq_stats_s {
  slq_accepted_dp0,
  slq_marked_dp0,
  slq_dropped_dp0,
  slq_accepted_dp1,
  slq_marked_dp1,
  slq_dropped_dp1,
  slq_accepted_dp2,
  slq_marked_dp2,
  slq_dropped_dp2,
  slq_accepted_dp3,
  slq_marked_dp3,
  slq_dropped_dp3,
  slq_tail_drop,
  slq_oversubscribetotal,
  slq_oversubscriptguarantee,
  slq_dequeue
} slq_stats_t;

typedef enum dp_color_s {
  ADD_GREEN,
  ADD_YELLOW,
  ADD_RED,
  ADD_BLACK,
  DROP_GREEN,
  DROP_YELLOW,
  DROP_RED,
  DROP_BLACK
} dp_color_t;

typedef struct soc_sbx_cs_rbuf_entry_s {
    uint32           entry_pkts;
    uint32           entry_bytes;
    uint32           mem_addr;
} soc_sbx_cs_rbuf_entry_t;

typedef struct soc_sbx_cs_update_s {
    uint64           bytes64;
    uint64           pkts64;
} soc_sbx_cs_update_t;

/* Track the configurable base addresses of the FTE address space.  The 
 * addresses are arranged in the same order as the enum below.  They are 
 * computed during soc init after a ucode package is loaded and may be 
 * overridden with soc properties.
 */
typedef enum soc_sbx_fte_segment_e {
    SOC_SBX_FSEG_PORT_MESH,  /* per ucode port, per node */
    SOC_SBX_FSEG_DROP,
    SOC_SBX_FSEG_CPU,
    SOC_SBX_FSEG_TRUNK,
    SOC_SBX_FSEG_HG_PORT,     /* required for HiGig support */
    SOC_SBX_FSEG_VPLS_COLOR,  /* vpls split horizon group color */
    SOC_SBX_FSEG_LGPORT,      /* local gports */
    SOC_SBX_FSEG_GGPORT,      /* global gports */
    SOC_SBX_FSEG_VID_VSI,     /* VID Flood & unknown mc flood FTEs */
    SOC_SBX_FSEG_DYN_VSI,     /* Dynamic Flood & unknown mc flood FTEs */
    SOC_SBX_FSEG_VPWS_UNI,    /* vpws uni FTEs */
    SOC_SBX_FSEG_UMC,         /* unknown mc flood FTEs */
    SOC_SBX_FSEG_PW_FO,       /* FTEs for PW Redundancy */
    SOC_SBX_FSEG_DYNAMIC,     /* remaining non-priviledged FTEs */
    SOC_SBX_FSEG_EXTRA,       /* Additional  non-priviledged FTEs */
    SOC_SBX_FSEG_END,         /* last valid FTE + 1 */
    SOC_SBX_FSEG_MAX
} soc_sbx_fte_segment_t;

#if defined(BCM_FE2000_A0) || defined(BCM_CALADAN3_SUPPORT)
extern char const* soc_sbx_fte_segment_names[];
#endif

#define SOC_SBX_FTE_SEGMENT_STRINGS  {     \
    "SOC_SBX_FSEG_PORT_MESH",              \
    "SOC_SBX_FSEG_DROP",                   \
    "SOC_SBX_FSEG_CPU",                    \
    "SOC_SBX_FSEG_TRUNK",                  \
    "SOC_SBX_FSEG_HG_PORT",                \
    "SOC_SBX_FSEG_VPLS_COLOR",             \
    "SOC_SBX_FSEG_LGPORT",                 \
    "SOC_SBX_FSEG_GGPORT",                 \
    "SOC_SBX_FSEG_VID_VSI",                \
    "SOC_SBX_FSEG_DYN_VSI",                \
    "SOC_SBX_FSEG_VPWS_UNI",               \
    "SOC_SBX_FSEG_UMC",                    \
    "SOC_SBX_FSEG_PW_FO",                  \
    "SOC_SBX_FSEG_DYNAMIC",                \
    "SOC_SBX_FSEG_EXTRA",                  \
    "SOC_SBX_FSEG_END",                    \
    "SOC_SBX_FSEG_MAX"                     \
}
    
/*
 * soc_sbx_qe2000_config_t
 * soc_sbx_bm3200_config_t
 *
 * Control parameters for device initialization
 * Values for parameters depend on the board (topology)
 */

#define SOC_SBX_CFG_FE2000_IF_IS_HIGIG(u, i) (SOC_SBX_CFG_FE2000(u)->ifmaskhigig & (1 << (i)))
#define SOC_SBX_CFG_FE2000_IF_IS_XG(u, i) (SOC_SBX_CFG_FE2000(u)->ifmaskxg & (1 << (i)))

typedef struct soc_sbx_qe2000_config_s {
    /* qe2000 specific config */
    uint32             nodeNum_ul;
    uint32             n2one_ul;
    sbBool_t           bHalfBus;
    sbBool_t           bRunLongDdrMemoryTest;
    sbBool_t           bQm512MbDdr2;
    sbBool_t           bSv2_5GbpsLinks;
    uint32             uEgMVTSize;
    uint32             uEgMcDropOnFull;
    uint32             uEgMvtFormat;
    uint32             uEgMcEfTimeout;
    uint32             uEgMcNefTimeout;
    uint32             uEiPortInactiveTimeout;
    uint32             uScGrantoffset;
    uint32             uScGrantDelay;
    uint32             nGlobalShapingAdjustInBytes;
    uint32             uSfiTimeslotOffsetInClocks;
    uint32             uQmMaxArrivalRateMbs;
    sbBool_t           bMixHighAndLowRateFlows;
    uint32             uScTxdmaSotDelayInClocks;
    uint32             uQsMaxNodes;
    uint32             nQueuesPerShaperIngress;
    uint32             uSfiDataLinkInitMask;
    uint32             SpiRefClockSpeed[2]; /* In KHz */
    uint32             SpiClockSpeed[2];    /* In KHz */
    uint32             uNumPhySpiPorts[2];
    uint32             uNumSpiTxPorts[2];
    uint32             uNumSpiRxPorts[2];
    uint32             uNumSpiTxStatusRepCnt[2]; /* tx cal_m value */
    uint32             uNumSpiRxStatusRepCnt[2]; /* rx cal_m value */
    uint64             uuRequeuePortsMask[2];
    uint32             bEiSpiFullPacketMode[2];
    uint32             uEiLines[SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES];
    uint32             uInterleaveBurstSize[SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES];
    sbBool_t           bEpDisable;
    sbLinkDriverConfig_t linkDriverConfig[SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS];
    uint32             uSpiSubportSpeed[SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES][SB_FAB_DEVICE_QE2000_MAX_SPI_SUBPORTs];
    uint32             bVirtualPortFairness;
    uint32             uEgressMcastEfDescFifoSize[SB_FAB_DEVICE_QE2000_MAX_PHYSICAL_PORTS - 1];
    uint32             bEgressMcastEfDescFifoInUse[SB_FAB_DEVICE_QE2000_MAX_PHYSICAL_PORTS - 1];
    uint32             uEgressMcastNefDescFifoSize[SB_FAB_DEVICE_QE2000_MAX_PHYSICAL_PORTS - 1];
    uint32             bEgressMcastNefDescFifoInUse[SB_FAB_DEVICE_QE2000_MAX_PHYSICAL_PORTS - 1];
    uint32             uPacketAdjustFormat;
} soc_sbx_qe2000_config_t;

typedef struct soc_sbx_bm3200_config_s {
    /* bm3200 specific config */
    uint32             uDeviceMode;
    sbBool_t           bLcmXcfgABInputPolarityReversed;
    sbLinkDriverConfig_t linkDriverConfig[SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS];

    uint32             uBmLocalBmId;
    uint32             uBmDefaultBmId;
    sbBool_t           bSv2_5GbpsLinks;
    uint32             uSerializerMask;       /* Serializer used for BM */
    uint32             uSeSerializerMaskHi;   /* Serializer used for SE */
    uint32             uSeSerializerMaskLo;   /* Serializer used for SE */
    uint32             uLcmSerializerMaskHi;  /* Serializer used for LCM */
    uint32             uLcmSerializerMaskLo;  /* Serializer used for LCM */
    uint32             uLcmXcfg[SB_FAB_MAX_NUM_DATA_PLANES][SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS];
    uint32             uLcmPlaneValid[SB_FAB_MAX_NUM_DATA_PLANES];

    uint32             nNumLinks;
    sbLinkState_t      linkState[SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS];
} soc_sbx_bm3200_config_t;


typedef struct soc_sbx_bm9600_config_s {
    /* bm9600 specific config */
    uint32             uDeviceMode;
    sbBool_t           bElectArbiterReconfig; /* Elected as new arbiter device after being crossbar only or elected as crossbar only */
    sbBool_t           bLcmXcfgABInputPolarityReversed;
    sbLinkDriverConfig_t linkDriverConfig[SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS];

    uint32             uBmLocalBmId;
    uint32             uBmDefaultBmId;
    sbBool_t           bSv2_5GbpsLinks;
    uint32             uSerdesAbility[SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS];
    uint32             uSerializerMask;       /* Serializer used for BM */
    uint32             uSeSerializerMaskHi;   /* Serializer used for SE */
    uint32             uSeSerializerMaskLo;   /* Serializer used for SE */
    uint32             uLcmSerializerMaskHi;  /* Serializer used for LCM */
    uint32             uLcmSerializerMaskLo;  /* Serializer used for LCM */
    uint32             uLcmXcfg[SB_FAB_MAX_NUM_DATA_PLANES][SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS];
    uint32             uLcmPlaneValid[SB_FAB_MAX_NUM_DATA_PLANES];

    uint32             nNumLinks;
    sbLinkState_t      linkState[SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS];
    uint32           BackupDeviceUnit;
    int32            cached_ina;
    uint32             arbiterUnusedLinkForSfiSciXcfgRemap; 
} soc_sbx_bm9600_config_t;

typedef struct group_config_s {
    uint8              cu_num;
    uint8              group_used;
    uint16             segment;
    uint32             offset;
    uint16             usage;
} cs_grp_cfg_t;

typedef struct cs_segment_s {
    uint64             cntrIdMap;
    uint64             *ullCount;
    uint64             *ullCountOrig;
    uint32             segmentSize;
    uint32             group[64];
    uint32             cu_num;
    uint32             shared;
    uint32             flushTimeDelta[64];
} cs_segment_t;

typedef struct soc_sbx_sirius_cs_s {
    uint64             *gbl_stats;
    uint64             *slq_stats;
    uint64             *fd_drop[FD_DROP_TYPE_COUNT];
    uint32             *fifo_dma_rbuf_begin;
    uint32             *fifo_dma_rbuf_read_ptr;
    uint32             *fifo_dma_rbuf_end;
    VOL sal_sem_t      CsFifoSem;
    uint32             segmentInUse;
    cs_segment_t       segment[CS_NUM_SEGMENTS];
    cs_grp_cfg_t       groupConfig[CS_NUM_BRICKS][CS_NUM_GROUPS];
    uint16             slq_q_active;
    uint32             flags;
    uint16             ep_type_use[16][2];
    uint16             statscfg_use[128][3];
    uint8              slq_usage;
} soc_sbx_sirius_cs_t;


#ifdef BCM_CALADAN3_SUPPORT
typedef struct soc_sbx_caladan3_cs_s {
    uint64             *gbl_stats;
    uint64             *slq_stats;
    uint64             *fd_drop[FD_DROP_TYPE_COUNT];
    uint32             *fifo_dma_rbuf_begin;
    uint32             *fifo_dma_rbuf_read_ptr;
    uint32             *fifo_dma_rbuf_end;
    VOL sal_sem_t      CsFifoSem;
    uint32             segmentInUse;
    cs_segment_t       segment[CS_NUM_SEGMENTS];
    cs_grp_cfg_t       groupConfig[CS_NUM_BRICKS][CS_NUM_GROUPS];
    uint16             slq_q_active;
    uint32             flags;
    uint16             ep_type_use[16][2];
    uint16             statscfg_use[128][3];
    uint8              slq_usage;
} soc_sbx_caladan3_cs_t;


typedef struct soc_sbx_caladan3_config_s {
    soc_sbx_caladan3_sws_config_t sws_cfg;
    soc_sbx_caladan3_port_map_t  *port_map;
    /*soc_sbx_caladan3_ppe_config_t *ppe_cfg;*/
    soc_sbx_caladan3_pd_config_t  ped_cfg;
    soc_sbx_caladan3_cmu_config_t cmu_cfg;
    soc_sbx_caladan3_cop_config_t cop_cfg;
    soc_sbx_caladan3_lrp_t lrp_cfg;
    uint32                          numUcodePorts;
    uint32                          fteMap[SOC_SBX_FSEG_MAX];
    uint32                          vsiEnd;
    uint32                          l2_age_cycles; /* number of cycles per age interval */
    uint32                          l2_cache_max_idx;
    soc_sbx_caladan3_cs_t           cs; /* Stats related */
    uint32                          c3_64bit_pc; /* 64bit print counter support*/
#define SOC_SBX_CALADAN3_FC_NUM_INTERFACES          2
    uint8                           fc_type[SOC_SBX_CALADAN3_FC_NUM_INTERFACES];
    uint8                           include_lss_faults;  /* Factor in MAC RX_LSS_STATUS in link status */
} soc_sbx_caladan3_config_t;
#endif /* BCM_CALADAN3_SUPPORT */


typedef enum soc_sbx_property_type_e {
    IF_SUBPORTS_CREATE,
    BCM_COSQ_INIT,
    DIAG_ASSIGN_SYSPORT,
    TM_FABRIC_PORT_HIERARCHY_SETUP,
    ES_FABRIC_PORT_HIERARCHY_SETUP,
    SOC_SBX_MAX_PROPERTY_TYPE,
    DIAG_EMULATOR_PARTIAL_INIT
} soc_sbx_property_type_t;

/*
 *  These things are kept in a list ordered not by ID but by priority.  The
 *  head of the list points to the highest priority (higher numerical priority)
 *  entries, and the tail points to the lowest (lower numerical priority)
 *  entries.
 *
 *  A new entry will be added as lower priority than all existing entries of
 *  the same priority as the new entry.  This means that the first entry at a
 *  particular priority effectively has higher priority than any added
 *  afterward at the same priority.
 */
typedef struct soc_sbx_sirius_predicate_parser_rule_s {
    uint8   ruleId;
    uint8   priority;
    uint16  predState;
    uint16  predMask;
    uint16  parser; /* gratuitous, but keep alignment */
    struct soc_sbx_sirius_predicate_parser_rule_s *next;
    struct soc_sbx_sirius_predicate_parser_rule_s *prev;
} soc_sbx_sirius_predicate_parser_rule_t;

/*
 *  type_res_flags below is an array for ingres and egress of the 16 frame
 *  parsers.  It contains a number of flags (PRED_TYPE_FLAGS_UC et al) that
 *  indicate information about what kind of frames this parser handles.  It
 *  seems to be used by the statistics code.
 *
 *  parserPredicates defines the predicate values that are used by the parsers.
 *  It has up to two fields, and each field supports mask, min, max.  There
 *  must always be one more than is normally used, for certain internal
 *  functions that manage port-specific parsers.
 */
/* frame parser is on receive block */
#define PRED_TYPE_RB               0
/* frame parser is on egress processor */
#define PRED_TYPE_EP               1
#define PRED_TYPE_MAX              2
/* frame parser gets unicast frames */
#define PRED_TYPE_FLAGS_UC         0x001
/* frame parser gets multicast frames */
#define PRED_TYPE_FLAGS_MC         0x002
/* frame parser gets CPU interface frames */
#define PRED_TYPE_FLAGS_INTF_CPU   0x010
/* frame parser gets HIGIG0 frames */
#define PRED_TYPE_FLAGS_INTF_0     0x020
/* frame parser gets HIGIG1 frames */
#define PRED_TYPE_FLAGS_INTF_1     0x040
/* frame parser gets HIGIG2 frames */
#define PRED_TYPE_FLAGS_INTF_2     0x080
/* frame parser gets HIGIG3 frames */
#define PRED_TYPE_FLAGS_INTF_3     0x100
/* frame parser gets REQUEUE0 frames */
#define PRED_TYPE_FLAGS_INTF_RQ0   0x200
/* frame parser gets REQUEUE1 frames */
#define PRED_TYPE_FLAGS_INTF_RQ1   0x400
/* frame parser gets any (non-requeue) interface's frames */
#define PRED_TYPE_FLAGS_INTF       0x1F0
/* frame parser gets any (including requeue) interface's frames */
#define PRED_TYPE_FLAGS_INTF_ANY   0x7F0
/* frame parser descriptive flags all of above bits set */
#define PRED_TYPE_FLAGS_ALL_BITS   0x7F3
/* shift distance for interfaces */
#define PRED_TYPE_FLAGS_IF_SHIFT   4

#define SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES 10
#define SB_FAB_DEVICE_SIRIUS_CONFIG_PARSER_PRED_FIELDS 3
#define SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS 16
#define SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS 7
#define SB_FAB_DEVICE_SIRIUS_CONFIG_COS_MAP_BLOCKS 4
#define SB_FAB_DEVICE_SIRIUS_CONFIG_PRED_PARSER_MAPS 256

#define SBX_FAB_MAX_MC_FIFOS    4
#define SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)    (SOC_SBX_CFG(unit)->num_res_per_eset_spec == 0)

/* Ingress predicates that are 'fixed' use by SDK */
/* NOTE: ALL = all modes, XGS = XGS mode, SBX = SBX mode */
#define _SIRIUS_I_PRED_ALL_IF0  0
#define _SIRIUS_I_PRED_ALL_IF1  1
#define _SIRIUS_I_PRED_ALL_IF2  2
#define _SIRIUS_I_PRED_ALL_MC   3
#define _SIRIUS_I_PRED_XGS_RCPU 4
#define _SIRIUS_I_PRED_XGS_EHV  5
#define _SIRIUS_I_PRED_SBX_DP0  6
#define _SIRIUS_I_PRED_SBX_DP1  7
#define _SIRIUS_I_PRED_SBX_NTB  8
/* Ingress predicate derived bitmaps for predicate->parser map rules */
#define _SIRIUS_I_PRED_ALL_IF0_BIT  (1 << _SIRIUS_I_PRED_ALL_IF0)
#define _SIRIUS_I_PRED_ALL_IF1_BIT  (1 << _SIRIUS_I_PRED_ALL_IF1)
#define _SIRIUS_I_PRED_ALL_IF2_BIT  (1 << _SIRIUS_I_PRED_ALL_IF2)
#define _SIRIUS_I_PRED_ALL_MC_BIT   (1 << _SIRIUS_I_PRED_ALL_MC)
#define _SIRIUS_I_PRED_XGS_RCPU_BIT (1 << _SIRIUS_I_PRED_XGS_RCPU)
#define _SIRIUS_I_PRED_XGS_EHV_BIT  (1 << _SIRIUS_I_PRED_XGS_EHV)
#define _SIRIUS_I_PRED_SBX_DP0_BIT  (1 << _SIRIUS_I_PRED_SBX_DP0)
#define _SIRIUS_I_PRED_SBX_DP1_BIT  (1 << _SIRIUS_I_PRED_SBX_DP1)
#define _SIRIUS_I_PRED_SBX_NTB_BIT  (1 << _SIRIUS_I_PRED_SBX_NTB)
#define _SIRIUS_I_PRED_ALL_IF_BITS  (_SIRIUS_I_PRED_ALL_IF0_BIT | _SIRIUS_I_PRED_ALL_IF1_BIT | _SIRIUS_I_PRED_ALL_IF2_BIT)
#define _SIRIUS_I_PRED_ALL_BITS     (_SIRIUS_I_PRED_ALL_IF_BITS | _SIRIUS_I_PRED_ALL_MC_BIT)
#define _SIRIUS_I_PRED_XGS_BITS     (_SIRIUS_I_PRED_XGS_RCPU_BIT | _SIRIUS_I_PRED_XGS_EHV_BIT)
#define _SIRIUS_I_PRED_SBX_DP_BITS  (_SIRIUS_I_PRED_SBX_DP0_BIT | _SIRIUS_I_PRED_SBX_DP1_BIT)
#define _SIRIUS_I_PRED_SBX_BITS     (_SIRIUS_I_PRED_SBX_DP0_BIT | _SIRIUS_I_PRED_SBX_DP1_BIT | _SIRIUS_I_PRED_SBX_NTB_BIT)

/* Egress predicates that are 'fixed' use by SDK */
/* NOTE: ALL = all modes, XGS = XGS mode, SBX = SBX mode */
#define _SIRIUS_E_PRED_ALL_IF0  0
#define _SIRIUS_E_PRED_ALL_IF1  1
#define _SIRIUS_E_PRED_ALL_IF2  2
#define _SIRIUS_E_PRED_ALL_MC   3
#define _SIRIUS_E_PRED_XGS_RREP 4
#define _SIRIUS_E_PRED_XGS_OP23 5
#define _SIRIUS_E_PRED_XGS_OP4  6
#define _SIRIUS_E_PRED_XGS_PPD2 7
#define _SIRIUS_E_PRED_SBX_IPMC 4
#define _SIRIUS_E_PRED_SBX_LB   5
#define _SIRIUS_E_PRED_ALL_IF0_BIT  (1 << _SIRIUS_E_PRED_ALL_IF0)
#define _SIRIUS_E_PRED_ALL_IF1_BIT  (1 << _SIRIUS_E_PRED_ALL_IF1)
#define _SIRIUS_E_PRED_ALL_IF2_BIT  (1 << _SIRIUS_E_PRED_ALL_IF2)
#define _SIRIUS_E_PRED_ALL_MC_BIT   (1 << _SIRIUS_E_PRED_ALL_MC)
#define _SIRIUS_E_PRED_XGS_RREP_BIT (1 << _SIRIUS_E_PRED_XGS_RREP)
#define _SIRIUS_E_PRED_XGS_OP23_BIT (1 << _SIRIUS_E_PRED_XGS_OP23)
#define _SIRIUS_E_PRED_XGS_OP4_BIT  (1 << _SIRIUS_E_PRED_XGS_OP4)
#define _SIRIUS_E_PRED_XGS_PPD2_BIT (1 << _SIRIUS_E_PRED_XGS_PPD2)
#define _SIRIUS_E_PRED_SBX_IPMC_BIT (1 << _SIRIUS_E_PRED_SBX_IPMC)
#define _SIRIUS_E_PRED_SBX_LB_BIT   (1 << _SIRIUS_E_PRED_SBX_LB)
#define _SIRIUS_E_PRED_ALL_IF_BITS  (_SIRIUS_I_PRED_ALL_IF0_BIT | _SIRIUS_I_PRED_ALL_IF1_BIT | _SIRIUS_I_PRED_ALL_IF2_BIT)
#define _SIRIUS_E_PRED_ALL_BITS     (_SIRIUS_I_PRED_ALL_IF_BITS | _SIRIUS_I_PRED_ALL_MC_BIT)
#define _SIRIUS_E_PRED_XGS_BITS     (_SIRIUS_E_PRED_XGS_RREP_BIT | _SIRIUS_E_PRED_XGS_OP23_BIT | _SIRIUS_E_PRED_XGS_OP4_BIT | _SIRIUS_E_PRED_XGS_PPD2_BIT)
#define _SIRIUS_E_PRED_SBX_BITS     (_SIRIUS_E_PRED_SBX_IPMC_BIT | _SIRIUS_E_PRED_SBX_LB_BIT)

/* Ingress parsers that are 'fixed' use by SDK */
/* NOTE: XGS = XGS mode, SBX = SBX mode */
#define _SIRIUS_I_PARSER_XGS_UNICAST         0
#define _SIRIUS_I_PARSER_XGS_MULTICAST       1
#define _SIRIUS_I_PARSER_XGS_REMOTE_CPU      2
#define _SIRIUS_I_PARSER_SBX_INGRESS_DP0     0
#define _SIRIUS_I_PARSER_SBX_INGRESS_DP1     1
#define _SIRIUS_I_PARSER_SBX_INGRESS_DP2     2
#define _SIRIUS_I_PARSER_SBX_INGRESS_DP3     3
#define _SIRIUS_I_PARSER_SBX_TB_REQ_DP0      4
#define _SIRIUS_I_PARSER_SBX_TB_REQ_DP1      5
#define _SIRIUS_I_PARSER_SBX_TB_REQ_DP2      6
#define _SIRIUS_I_PARSER_SBX_TB_REQ_DP3      7
#define _SIRIUS_I_PARSER_SBX_LB_REQ_DP0      8
#define _SIRIUS_I_PARSER_SBX_LB_REQ_DP1      9
#define _SIRIUS_I_PARSER_SBX_LB_REQ_DP2     10
#define _SIRIUS_I_PARSER_SBX_LB_REQ_DP3     11
#define _SIRIUS_I_PARSER_ALL_INVALID        15

/* Egress parsers that are 'fixed' use by SDK */
/* NOTE: ALL = all modes, XGS = XGS mode, SBX = SBX mode */
#define _SIRIUS_E_PARSER_XGS_UC        0
#define _SIRIUS_E_PARSER_XGS_RAW       1
#define _SIRIUS_E_PARSER_XGS_MC_L2     2
#define _SIRIUS_E_PARSER_XGS_MC_L2R    3
#define _SIRIUS_E_PARSER_XGS_MC_L3     4
#define _SIRIUS_E_PARSER_XGS_MC_DVP    5
#define _SIRIUS_E_PARSER_SBX_PASSTHRU  0
#define _SIRIUS_E_PARSER_SBX_TB        1
#define _SIRIUS_E_PARSER_SBX_TB_IPMC   2
#define _SIRIUS_E_PARSER_SBX_LB_UC     3
#define _SIRIUS_E_PARSER_SBX_LB_MC     4
#define _SIRIUS_E_PARSER_ALL_DIAG     14
#define _SIRIUS_E_PARSER_ALL_INVALID  15

/* Ingress QUEUE_MAP segments (qsel) 'fixed' use by SDK */
/* ..._XGS_UNICAST for unicast traffic queue selection */
/* ..._XGS_MULTICAST for multicast traffic queue selection */
/* ..._XGS_CPU for CPU directed traffic queue selection */
/* ..._SBX_RQ for SBX mode requeue queue lookup */
#define _SIRIUS_I_QUEUE_MAP_SEG_XGS_UNICAST   1
#define _SIRIUS_I_QUEUE_MAP_SEG_XGS_MULTICAST 2
#define _SIRIUS_I_QUEUE_MAP_SEG_SBX_RQ        4

/* Ingress COS_PROFILE (COS_MAP block / qsel_offset) 'fixed' use by SDK */
/* ..._GENERAL for general use */
/* ..._ENCAPID used only for BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID */
#define _SIRIUS_I_COS_PROFILE_GENERAL 0
#define _SIRIUS_I_COS_PROFILE_ENCAPID 1

/*
 *  This struct contains state (not configuration) for Sirius.  It is used
 *  for operations.  Early in init, this must be created if it does not exist.
 *  Even if it does exist, it must be cleared early in the init process.
 *
 *  It is possible that as things become more complex, there will be parts that
 *  do not take well to a blind overwrite (resource leakage or similar).  Code
 *  to create those parts must occur after the clearing, and code to destroy
 *  those parts must occur before the clearing (if not creating) and probably
 *  in the detach code (which should also destroy this).
 */
typedef struct soc_sbx_sirius_state_s {
    uint16             ingressPreds;                                               /* ingress predicates in use */
    uint16             ingressPredsSdk;                                            /* ingress predicates in use by SDK */
    uint16             ingressParsers;                                             /* ingress parsers in use */
    uint16             ingressParsersSdk;                                          /* ingress parsers in use by SDK */
    uint16             egressPreds;                                                /* egress predicates in use */
    uint16             egressPredsSdk;                                             /* egress predicates in use by SDK */
    uint16             egressParsers;                                              /* egress parsers in use */
    uint16             egressParsersSdk;                                           /* egress parsers in use by SDK */
    int                ingressPredRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES];    /* ingress predicate reference counts */
    int                ingressParserRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS];     /* ingress action reference counts */
    int                egressPredRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES];     /* egress predicate reference counts */
    int                egressParserRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS];      /* egress action reference counts */
    uint8              ingressCosMaps;                                             /* ingress COS_MAP blocks in use */
    uint8              ingressCosMapsSdk;                                          /* ingress COS_MAP blocks in use by SDK */
    uint8              ingressQueueMaps;                                           /* ingress QUEUE_MAP blocks in use */
    uint8              ingressQueueMapsSdk;                                        /* ingress QUEUE_MAP blocks in use by SDK */
    int                ingressCosMapRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_COS_MAP_BLOCKS]; /* ingress COS_MAP block references */
    int                ingressQueueMapRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS]; /* ingress QUEUE_MAP block references */
    int                ingressQueueMapIntRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS]; /* ingress QUEUE_MAP internal references */
    int                ingressQueueMapCosMapRefs[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS][SB_FAB_DEVICE_SIRIUS_CONFIG_COS_MAP_BLOCKS]; /* queue map block references to each COS_MAP */
    uint32             ingressQueueMapSize[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS]; /* ingress QUEUE_MAP block sizes */
    uint16             ingressQueueMapBase[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS]; /* ingress QUEUE_MAP block bases */
    uint16             ingressQueueMapAddr[SB_FAB_DEVICE_SIRIUS_CONFIG_QUEUE_SEGMENTS]; /* ingress QUEUE_MAP block physical addresses */
    uint16             ingressRules;                                               /* number of ingress rules below */
    uint16             egressRules;                                                /* number of egress rules below */
    soc_sbx_sirius_predicate_parser_rule_t *ingressRuleHead;                       /* ingress predicate->parser mapping rule list head */
    soc_sbx_sirius_predicate_parser_rule_t *ingressRuleTail;                       /* ingress predicate->parser mapping rule list tail */
    soc_sbx_sirius_predicate_parser_rule_t *egressRuleHead;                        /* egress predicate->parser mapping rule list head */
    soc_sbx_sirius_predicate_parser_rule_t *egressRuleTail;                        /* egress predicate->parser mapping rule list tail */
    SHR_BITDCL         ingressRuleIds[_SHR_BITDCLSIZE(SB_FAB_DEVICE_SIRIUS_CONFIG_PRED_PARSER_MAPS)]; /* ingress rule IDs in use */
    SHR_BITDCL         egressRuleIds[_SHR_BITDCLSIZE(SB_FAB_DEVICE_SIRIUS_CONFIG_PRED_PARSER_MAPS)]; /* egress rule IDs in use */
    SHR_BITDCL         inhibitQsel[_SHR_BITDCLSIZE(SB_FAB_DEVICE_SIRIUS_NUM_QUEUES)]; /* don't commit queue to SDK qsel */
    int32              nMaxFabricPorts;                                            /* max number of internal/external fabric ports */
    uint32             uNumExternalSubports[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];  /* indexed by interface ID */
    uint32             uNumInternalSubports[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];  /* indexed by interface ID */
    uint32             uTotalInternalSubports;
} soc_sbx_sirius_state_t;
extern soc_sbx_sirius_state_t *_soc_sbx_sirius_state[SOC_MAX_NUM_DEVICES];
extern volatile int __soc_sbx_sirius_state_init;
#define SOC_SBX_SIRIUS_STATE(_unit) _soc_sbx_sirius_state[_unit]

typedef struct soc_sbx_sirius_config_s {
    /* sirius specific config */
    SHR_BITDCL         property[1];
    uint32             uDdr3NumColumns;
    uint32             uDdr3NumRows;
    uint32             uDdr3NumBanks;
    uint32             uDdr3NumMemories;
    uint32             uDdr3ClockMhz;
    uint32             uDdr3MemGrade;
    int32              nMaxVoq;
    uint16             uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE+1];
    uint32             uInterfacePlane[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];       /* interface planes */
    uint32             uInterfaceWeight[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];      /* interface weights */
    int                nNodeUserManagementMode;                                    /* 0: SDK manage TS nodes, 1: application manage TS nodes, -1 unknown */
    uint8              shapingBusLengthAdj[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];   /* shaping bus length adjust values */
    sbBool_t           b8kNodes;                                                   /* level 1 has 8K instead of 16K nodes */
    sbBool_t           bDualLocalGrants;
    sbBool_t           bSv2_5GbpsLinks;
    uint32             uNumTsNode[SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS+1];           /* number of TS nodes of the level */
    uint32             uNumEsNode[SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS];             /* number of ES nodes of the level */
    uint32             uFifoGroupSize;                                             /* size of egress fifo group */
    uint32             type_res_flags[PRED_TYPE_MAX][SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS]; /* information about frame parsers */
    uint32             pred_info_flags[2*PRED_TYPE_MAX][SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES]; /* predicate information */
    uint32             parserPredicate[SB_FAB_DEVICE_SIRIUS_CONFIG_PARSERS][SB_FAB_DEVICE_SIRIUS_CONFIG_PARSER_PRED_FIELDS][3]; /* predicate masks & ranges for parsers */
    int                userPredicate[SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES];      /* track user predicates */
    int                userAction[SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES];         /* track user parsers */
    int                userPredAction[SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES];     /* track user predicate->action map rules */
    soc_sbx_sirius_cs_t cs;                                                        /* central statistics related structures */
    sbBool_t           bSubscriberNodeOptimize;                                    /* optimize the node usage by skip a level */
    int8               egrGlobalAdjust;                                            /* emulated EP global frame size adjust */
    int8               mvrMaxSize;                                                 /* maximum MVR size, in MDB elements */
    uint32             uSerdesAbility[SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS];
    sbLinkDriverConfig_t linkDriverConfig[SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS];
    sal_mutex_t        lMcAggrLock;                                                /* multicast/aggregation shared lock */
    uint32             bForce16Queue;                                              
    uint32             uQmMaxArrivalRateMbs;
    uint32             uSubscriberMaxCos;                                          /* max level of cos for subscriber queues */
    uint16             requeueMinPage;                                             /* lowest ep_oi2qb_map page for requeue */
    uint16             queueOffset;                                                /* ep_oi2qb_map queue offset for dual lookup mode */
    sbBool_t           dualLookup;                                                 /* whether dual lookup mode is enabled */
    sbBool_t           redirectUcLag;                                              /* whether Sirius redirects unicast frames on aggregates */
    uint32             uMaxBuffers;
    sbBool_t           bExtendedPortMode;                                          /* extended number of fabric port support */
    uint16             thresh_drop_limit;
    uint16             fifo_thresh[SB_FAB_DEVICE_SIRIUS_FIFO_NUM];
    uint8              ucast_cos_map; /* default unicast cos map */
    uint8              mcast_cos_map; /* default multicast cos map */
    uint8              ucast_ef_fifo; /* default   unicast  ef fifo */
    uint8              mcast_ef_fifo; /* default multicast  ef fifo */
    uint8              mcast_nef_fifo;/* default multicast nef fifo */
    uint8              if_diag_mode[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES]; /* bits: 0 = config in diag mode, 1 = temp in diag mode */
    uint16             if_diag_queue[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES]; /* when temporarily in diag mode, stores default queue */
    uint32             if_diag_nohead[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES][2]; /* nohead fields for each port in diag mode */
    int8               is_static_es;  /* fixed egress scheduler specified at 
                                         init time via soc property */
    int8               tsChildPassThroughDisable; /* parents with a single child cannot be root_relay if non-zero */
} soc_sbx_sirius_config_t;

typedef struct soc_sbx_dbg_cntr_s {
    uint32          trigger;
    SHR_BITDCL      ports[SBX_FAB_ERRORS];
} soc_sbx_dbg_cntr_t;

typedef struct soc_sbx_config_s {
    sbhandle           DeviceHandle;
    uint32             reset_ul;
    uint32             uClockSpeedInMHz;
    uint32             uFabricConfig;
    sbBool_t           bHalfBus;
    sbBool_t           bRunSelfTest;
    sbBool_t           bTmeMode;
    sbBool_t           fabric_egress_setup;
    sbBool_t           uInterfaceProtocol;                                         /* XGS or SBX */
    uint32             erh_type;
    uint32             uActiveScId;
    uint32             uLinkThresholdIndex;
    uint32             uRedMode;
    uint32             uMaxFailedLinks;
    sbBool_t           bHybridMode;
    sbBool_t           bUcqResourceAllocationMode;
    sbBool_t           bEgressFifoIndependentFlowControl;
    sbBool_t           bEgressMulticastFifoIndependentFlowControl;
    uint32             bcm_cosq_init; /* if set, VOQs are initialized at bcm_cosq_init() */
    uint32             bcm_cosq_priority_mode[SBX_MAX_FABRIC_COS]; /* BCM_COSQ_SP, BCM_COSQ_WEIGHTED_FAIR_QUEUING, BCM_COSQ_BE, */
                                                            /* BCM_COSQ_AF, BCM_COSQ_SP_GLOBAL */
    uint32             bcm_cosq_priority_min_bw_kbps[SBX_MAX_FABRIC_COS]; /* CIR, only valide for AF, guaranteed rate */
    uint32             bcm_cosq_priority_max_bw_kbps[SBX_MAX_FABRIC_COS]; /* PIR shape rate */
    uint32             bcm_cosq_priority_weight[SBX_MAX_FABRIC_COS]; /* only valid if mode is WFQ */
    uint32             bcm_cosq_priority_min_depth_bytes[SBX_MAX_FABRIC_COS]; /* queue memory allocation min */
    uint32             bcm_cosq_priority_max_depth_bytes[SBX_MAX_FABRIC_COS]; /* queue memory allocation max */
    int32              bcm_cosq_priority_group[SBX_MAX_FABRIC_COS]; /* Priority flow control priority group */
    uint32             pfc_cos_enable; /* pfc priority group enable */
    uint32             bcm_cosq_all_min_bw_kbps; /* this is the rate of the BAG */
    uint32             hold_pri_num_timeslots;
    uint32             epoch_length_in_timeslots;
    uint64             xbar_link_en;
    int                is_demand_scale_fixed;
    uint32             fixed_demand_scale;
    uint32             demand_scale;
    uint32             nMaxFabricPortsOnModule;  /* hybrid mode configuration */
    uint32             *custom_stats;
    soc_sbx_dbg_cntr_t soc_sbx_dbg_cntr_rx[9];
    uint32             arbitration_port_allocation;

    /* configuration */
    uint32             nDiscardTemplates; /* WRED/Discard Templates */
    uint32             discard_probability_mtu;
    uint32             discard_queue_size;
    uint32             nShaperCount;      /* Number of Egress Shapers to manage */
    uint32             num_ds_ids;
    uint32             num_internal_ds_ids;
    uint32             num_res_per_eset_spec; /* 0: not specified, !(0): specified(only 4 valid) */
    int8               is_mc_ef_cos[SBX_FAB_MAX_MC_FIFOS]; /* TRUE: ef, FALSE: nef, -1: Invalid */
    uint32             num_nodes;
    uint32             cfg_num_nodes;
    uint32             cfg_node_00_31_mask;
    uint32             cfg_node_32_63_mask;
    uint32             cfg_node_64_95_mask;
    uint32             max_ports;
    uint32             num_queues;
    uint32             num_bw_groups;
    uint32             num_sysports;
    uint32             use_extended_esets;
    uint32             mcgroup_local_start_index;
    sbBool_t           v4_ena;
    sbBool_t           oam_rx_ena;
    sbBool_t           oam_tx_ena;
    uint32             oam_spi_lb_port;
    uint32             oam_spi_lb_queue;
    sbBool_t           v4mc_str_sel;
    sbBool_t           v4uc_str_sel;
    sbBool_t           mim_ena;
    sbBool_t           v6_ena;
    sbBool_t           dscp_ena;
    sbBool_t           mplstp_ena;
    uint8              parse_rx_erh;
    uint8              max_pkt_len_adj_sel;
    uint8              max_pkt_len_adj_value;
    sbBool_t           enable_all_egress_nodes; /* enable grants on nodes not present */
                                                /* Helps in VoQ draining              */
    uint8              sp_mode;
    uint8              local_template_id;        /* priority template id */
    uint8              node_template_id;         /* node scheduler template id */

    uint8              l2p_node[SBX_MAXIMUM_NODES];
    uint8              p2l_node[SBX_MAXIMUM_NODES];
    uint32             num_ingress_scheduler;    /* number of ingress scheduler */
    uint32             num_egress_scheduler;     /* number of egress scheduler */
    uint32             num_egress_group;         /* number of egress group */
    uint32             num_ingress_multipath;    /* number of ingress multipath */
    uint32             num_egress_multipath;     /* number of egress multipath */

    uint32             uSerdesSpeed;
    sbBool_t           bSerdesEncoding;
    uint32             connect_min_util[SBX_MAX_FABRIC_COS];      /* specified as percentage */
    uint32             connect_max_age_time[SBX_MAX_FABRIC_COS];  /* specified in usecs */
    uint8              connect_min_util_template[SBX_MAX_FABRIC_COS]; /* local min_util */
    uint8              connect_min_util_remote_template[SBX_MAX_FABRIC_COS];
    uint8              connect_max_age_time_template[SBX_MAX_FABRIC_COS];
    uint8              connect_min_util_tdm_calendar_template;
    uint8              connect_max_age_time_tdm_calendar_template;

    uint32             uRateClockSpeed;       /* in MHz */       
    uint32             uMaxClocksInEpoch;
    uint32             diag_qe_revid;         /* qe revision in the system, internal use only */
    uint8              max_interop_xbar;
    sbBool_t           allow_ucast_mcast_resource_overlap;

    sbBool_t           module_custom1_in_system; /* chassis has this module type */
    uint32             module_custom1_links;     /* logical crossbar links for this module */

    union {
#ifdef BCM_CALADAN3_SUPPORT
        soc_sbx_caladan3_config_t caladan3_cfg;
#endif
        soc_sbx_qe2000_config_t qe2000_cfg;
        soc_sbx_bm3200_config_t bm3200_cfg;
        soc_sbx_bm9600_config_t bm9600_cfg;
        soc_sbx_sirius_config_t sirius_cfg;
    }chip_u;
} soc_sbx_config_t;

typedef struct soc_sbx_txrx_active_s *soc_sbx_txrx_active_p_t;

typedef int (*soc_sbx_init_f)(int, soc_sbx_config_t *);
typedef int (*soc_sbx_detach_f)(int unit);
typedef int (*soc_sbx_ucode_update_f)(int unit);
typedef void (*soc_sbx_isr_f)(void *);
typedef void (*soc_sbx_lib_isr_f)(void *, uint32 ireg);
typedef void (*soc_sbx_txrx_done_f)(int unit, soc_sbx_txrx_active_p_t done);

typedef struct soc_sbx_txrx_active_s {
    struct soc_sbx_txrx_active_s *next;   /* must be first */
    int status;
    void *cookie;
    int rxlen;                        /* RX only */
    soc_sbx_txrx_done_f donecb;
    /* TXRX internal state */
    int entries;
} soc_sbx_txrx_active_t;

/*
 * Typedef: soc_sbx_functions_t
 * Purpose: Chip driver functions that are not automatically generated.
 */

typedef void (*soc_sbx_sram_dll_init_f)(sbhandle sbh);
typedef void (*soc_sbx_ddr_train_f)(sbhandle sbh);

#ifdef BCM_SIRIUS_SUPPORT
typedef int (*soc_sirius_ddr_clear_f)(int unit);
#endif

typedef struct soc_sbx_functions_s {

    soc_sbx_sram_dll_init_f     sram_init;
    soc_sbx_ddr_train_f         ddr_train;
#ifdef BCM_SIRIUS_SUPPORT
    soc_sirius_ddr_clear_f      sirius_ddr_clear;
#endif
    /*
     * Add new ones here
     */
} soc_sbx_functions_t;

/*
 * Translate an sbStatus_t to a soc error code
 */
extern int
soc_sbx_translate_status(sbStatus_t s);

/*
 * Register APIs
 */
extern int
soc_sbx_register(int unit, const soc_sbx_functions_t *sbx_functions);

extern int
soc_sbx_unregister(int unit);

#define SOC_SBX_LIB_GU2   2               /* Gu 2k */

/*
 * Typedef: soc_sbx_control_t
 * Purpose: SBX SOC Control Structure.  All info about a device instance.
 */
typedef struct soc_sbx_control_s {
    /*
     * Context for FE/QE lib
     */
    void                  *drv;
    sbhandle              sbhdl;
    int                   fabtype;        /* Valid on QE, BME */
    soc_sbx_fetype_t      fetype;         /* Valid on FE */

    uint32                module_id0;     /* Module Id of FE0 */
    int                   modid_count;
    int                   node_id;        /* QE Node number */
    /* [fab-unit][fab-port] will give feport */
    uint32                modport[SBX_MAX_MODIDS][SBX_MAX_PORTS];
    uint32                fabnodeport2feport[SBX_MAX_NODES][SBX_MAX_PORTS];
    sbhandle              fabric_units[SBX_MAX_PORTS];
    sbhandle              forwarding_units[SBX_MAX_PORTS];
    int                   spi_subport_count[SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES];
    int                   hg_subport_count[SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS];
    soc_port_info_t       *system_port_info;

    soc_sbx_init_f        init_func;
    soc_sbx_detach_f      detach;         /* Deallocate driver resources */
    soc_sbx_isr_f         isr;
    soc_sbx_lib_isr_f     lib_isr;        /* ISR for SBX library (e.g ILib) */

    int                   init;           /* HW Initialization Status */
    int                   libInit;        /* Library/ucode Init status */

    soc_sbx_ucode_erh_t   ucode_erh;
    soc_sbx_ucode_type_t  ucodetype;
    soc_sbx_ucode_update_f ucode_update_func;
    void                  *ucode;
    int                   libver;
    uint32                uver_maj;
    uint32                uver_min;
    uint32                uver_patch;
    char                  uver_name[32];
    char                  uver_str[45];

    int                   numQueues;      /* Valid for QE only */

    VOL sal_sem_t         dma_sem;

    soc_sbx_config_t      *cfg;

    /*** TX & RX state ***/
    /* RX & TX configuration parameters */
    int tx_ring_entries;        /* 0: defaulted */
    int completion_ring_entries; /* 0: defaulted */
    int rx_buffer_size;         /* 0: defaulted */
    int max_rx_buffers;         /* 0: defaulted */
    int rx_debug_fill;          /* 0: defaulted */
    uint8 rx_debug_fill_val;
    int max_tx_buffers;         /* 0: defaulted */

    /* RX & TX register offsets */
    int tx_ring_reg;
    int tx_ring_size_reg;
    int tx_ring_producer_reg;
    int tx_ring_consumer_reg;
    int completion_ring_reg;
    int completion_ring_size_reg;
    int completion_ring_producer_reg;
    int completion_ring_consumer_reg;
    int rxbuf_size_reg;
    int rxbuf_load_reg;
    int rxbufs_pop_reg;
    uint32 rxbufs_pop_bit;

    /* RX & TX internal state */
    VOL sal_mutex_t txrx_mutex;
    VOL sal_sem_t txrx_sync_sem;
    soc_sbx_txrx_active_t *txrx_active_mem;
    soc_sbx_txrx_active_t *txrx_active_free;
    uint8 *completion_ring_mem;
    uint32 *completion_ring;
    int completion_ring_consumer;

    int rx_active;
    soc_sbx_txrx_active_t *rx_active_head;
    soc_sbx_txrx_active_t *rx_active_tail;

    uint8 *tx_ring_mem;
    uint32 *tx_ring;
    int tx_ring_producer;
    int tx_ring_entries_active;
    int tx_active;
    soc_sbx_txrx_active_t *tx_active_head;
    soc_sbx_txrx_active_t *tx_active_tail;

    /* Allocated port-ete */
    uint32  port_ete[SBX_MAX_PORTS];
    uint32  port_ut_ete[SBX_MAX_PORTS];
    /* Allocated trunk-ete */
    uint32  trunk_ete[SBX_MAX_TRUNKS];
    /* invalid ete, this is of type L2 */
    uint32  invalid_l2ete;

    /* g2p3 state */
    uint32 stpforward;
    uint32 stpblock;
    uint32 stplearn;

    void *state; /* bcm state */
    soc_sbx_functions_t sbx_functions;
    int32  master_unit;
    int32  num_slaves;
    int32  slave_units[SOC_MAX_NUM_DEVICES];
} soc_sbx_control_t;

/*
 * SOC_SBX_* control driver macros
 */
#define SOC_SBX_CONTROL(unit) \
        ((soc_sbx_control_t *)(SOC_CONTROL(unit)->drv))

#define SOC_SBX_STATE(unit) \
        ((bcm_sbx_state_t *)(SOC_SBX_CONTROL(unit)->state))

#define SOC_SBX_SBHANDLE(unit)    (SOC_SBX_CONTROL(unit)->sbhdl)

#define SOC_SBX_CFG(unit)         (SOC_SBX_CONTROL(unit)->cfg)
#define SOC_SBX_CFG_CALADAN3(unit)  (&(SOC_SBX_CONTROL(unit)->cfg->chip_u.caladan3_cfg))
#define SOC_SBX_CFG_QE2000(unit)  (&(SOC_SBX_CONTROL(unit)->cfg->chip_u.qe2000_cfg))
#define SOC_SBX_CFG_BM3200(unit)  (&(SOC_SBX_CONTROL(unit)->cfg->chip_u.bm3200_cfg))
#define SOC_SBX_CFG_BM9600(unit)  (&(SOC_SBX_CONTROL(unit)->cfg->chip_u.bm9600_cfg))
#define SOC_SBX_CFG_SIRIUS(unit)  (&(SOC_SBX_CONTROL(unit)->cfg->chip_u.sirius_cfg))


/*
 * Macros to get the device block type, number (instance) and index
 * within block for a given port
 */
#define SOC_SBX_SYSTEM_PORT_INFO(unit, port)        \
    (SOC_SBX_CONTROL(unit)->system_port_info[port])

#define SOC_SBX_SYSTEM_PORT_BLOCK(unit, port)       \
    (SOC_SBX_SYSTEM_PORT_INFO(unit, port).blk)

#define SOC_SBX_SYSTEM_PORT_BLOCK_TYPE(unit, port)   \
    (SOC_BLOCK_TYPE(unit, SOC_SBX_SYSTEM_PORT_BLOCK(unit, port)))

#define SOC_SBX_SYSTEM_PORT_BLOCK_NUMBER(unit, port) \
    (SOC_BLOCK_NUMBER(unit, SOC_SBX_SYSTEM_PORT_BLOCK(unit, port)))

#define SOC_SBX_SYSTEM_PORT_BLOCK_INDEX(unit, port)  \
    (SOC_SBX_SYSTEM_PORT_INFO(unit, port).bindex)


/*
 * More SOC_IS_* macros
 */
#define SOC_IS_SBX_FE(unit) \
        (SOC_IS_SBX_CALADAN3(unit))
#define SOC_IS_SBX_QE(unit) \
        ((SOC_IS_SBX_QE2000(unit)) || (SOC_IS_SBX_SIRIUS(unit)))
#define SOC_IS_SBX_BME(unit) \
        (SOC_IS_SBX_BME3200(unit) || SOC_IS_SBX_BM9600(unit))
#define SOC_IS_SBX(unit) \
        (SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit) || SOC_IS_SBX_BME(unit))
#define SOC_IS_SBX_NODE_ARBITER(unit) \
        (soc_feature(unit, soc_feature_node) || soc_feature(unit, soc_feature_arbiter))

/*
 * Return true if it's a master unit (which might control
 *   other units that have same device ID and managed by the same
 *   SDK). There could only be one master unit
 */
#define SOC_IS_SBX_MASTER(u)   \
        (SOC_SBX_CONTROL(u)->master_unit < 0)

#define SOC_SBX_MASTER(u)   \
        (SOC_SBX_CONTROL(u)->master_unit)

/*
 * To be used in FE2000 implementations only
 */
#define SOC_IS_SBX_G2P3(unit) \
        (SOC_SBX_CONTROL(unit)->ucodetype  == SOC_SBX_UCODE_TYPE_G2P3)
#define SOC_IS_SBX_G2XX(unit) \
        (SOC_SBX_CONTROL(unit)->ucodetype  == SOC_SBX_UCODE_TYPE_G2XX)

/*
 *  For Caladan3
 */
#define SOC_IS_SBX_G3P1(unit) \
        (SOC_SBX_CONTROL(unit)->ucodetype  == SOC_SBX_UCODE_TYPE_G3P1)

#define SOC_IS_SBX_T3P1(unit) \
        (SOC_SBX_CONTROL(unit)->ucodetype  == SOC_SBX_UCODE_TYPE_T3P1)

#define SOC_IS_SBX_TYPE(dev_type)        \
  (((dev_type) == QE2000_DEVICE_ID)      \
   || ((dev_type) == BME3200_DEVICE_ID)  \
   || ((dev_type) == BM9600_DEVICE_ID)   \
   || ((dev_type) == BCM88020_DEVICE_ID) \
   || ((dev_type) == BCM88025_DEVICE_ID) \
   || ((dev_type) == BCM88030_DEVICE_ID) \
   || ((dev_type) == BCM88034_DEVICE_ID) \
   || ((dev_type) == BCM88039_DEVICE_ID) \
   || ((dev_type) == BCM88230_DEVICE_ID) \
   || ((dev_type) == BCM88231_DEVICE_ID) \
   || ((dev_type) == BCM88235_DEVICE_ID) \
   || ((dev_type) == BCM88236_DEVICE_ID) \
   || ((dev_type) == BCM88239_DEVICE_ID) \
   || ((dev_type) == BCM56931_DEVICE_ID) \
   || ((dev_type) == BCM56936_DEVICE_ID) \
   || ((dev_type) == BCM56613_DEVICE_ID))

#define SOC_SBX_INIT(unit)     (SOC_SBX_CONTROL(unit)->init)
#define SOC_SBX_LIB_INIT(unit) (SOC_SBX_CONTROL(unit)->libInit)

#define SOC_SBX_G2FE_FROM_UNIT(unit)            \
    (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv



/* Can this macro be moved to common area? (have nothing specific to SBX) */
#define SBX_PBMP_VALID(unit, pbmp, status) \
do {                                 \
  bcm_port_t   __p;                  \
  status  = BCM_E_NONE;              \
  BCM_PBMP_ITER((pbmp), __p) {       \
    if (!SOC_PORT_VALID((unit), __p)) {\
        status = BCM_E_PORT;    \
        break;                    \
    }\
  }\
} while(0)

#define SBX_TRUNK_VALID(tid) (((tid) >= 0) && ((tid) < SBX_MAX_TRUNKS))

#define SBX_PORT_VALID(unit,port)     \
    ((SAL_BOOT_BCMSIM) ? ((port) >= 0 && (port) <= SBX_BCMSIM_PORT_MAX) : \
                         (SOC_PORT_VALID_RANGE(unit,port)) \
                       /* && SBX_PORT_TYPE(unit, port) != 0*/)

/* use PORT_MIN/_MAX to be more efficient than PBMP_ITER */
#define _SBX_PBMP_ITER(_u,_pt,_p)       \
        for ((_p) = SOC_PORT_MIN(_u,_pt); \
             (_p) >= 0 && (_p) <= ((SAL_BOOT_BCMSIM) ? SBX_BCMSIM_PORT_MAX : \
                                                       SOC_PORT_MAX(_u,_pt)); \
             (_p)++) \
                if (_SHR_PBMP_MEMBER(SOC_PORT_BITMAP(_u,_pt), (_p)))

#define SBX_PBMP_ALL_ITER(_u, _p)           _SBX_PBMP_ITER(_u,all,_p)

#define SBX_ADD_PORT(ptype, nport) \
            si->ptype.num++; \
            if ( (si->ptype.min < 0) || (si->ptype.min > nport) ) {	\
                si->ptype.min = nport; \
            } \
            if (nport > si->ptype.max) { \
                si->ptype.max = nport; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.bitmap, nport);

#define SBX_REMOVE_PORT(ptype, nport) \
            si->ptype.num--; \
            SOC_PBMP_PORT_REMOVE(si->ptype.bitmap, nport);


/*
 * Discard related
 */
#define SOC_SBX_DISCARD_PROBABILITY_MTU_SZ  1518
#define SOC_SBX_DISCARD_QUEUE_SZ             0 /* 0 => actual queue size */
                                               /* non zero (2 * 1024 * 1024) 2 MB) => queue size */

/*
 * BME related
 */
#define SOC_SBX_BME_ARBITER_MODE              0
#define SOC_SBX_BME_XBAR_MODE                 1
#define SOC_SBX_BME_ARBITER_XBAR_MODE         2
#define SOC_SBX_BME_LCM_MODE                  3
/* This future mode is for a device which may be capable of becoming an arb/xbar */
/* which is configured as an xbar but gets arb config commands and sets them up  */
/* on the hardware - for example, sysports, etc....                              */
#define SOC_SBX_BME_XBAR_ARBITER_CAPABLE_MODE 4

/*
 * System Configuration
 */
#define SOC_SBX_SYSTEM_CFG_DMODE            0 /* Bm3200 + Qe2000 */
#define SOC_SBX_SYSTEM_CFG_VPORT            1 /* Bm9600 + Qe4000 */
#define SOC_SBX_SYSTEM_CFG_VPORT_LEGACY     2 /* Bm9600 + Qe2000 */
#define SOC_SBX_SYSTEM_CFG_VPORT_MIX        3 /* Bm9600 + Qe2000 + Qe4000 */
#define SOC_SBX_SYSTEM_CFG_INVALID          ~0

#define SOC_SBX_SP_MODE_IN_BAG              0
#define SOC_SBX_SP_MODE_ACCOUNT_IN_BAG      1

#define SOC_SBX_QOS_TEMPLATE_TYPE0          0
#define SOC_SBX_QOS_TEMPLATE_TYPE1          1
#define SOC_SBX_QOS_TEMPLATE_TYPE2          2
#define SOC_SBX_QOS_TEMPLATE_TYPE3          3
#define SOC_SBX_QOS_TEMPLATE_TYPE4          4

#define SOC_SBX_NODE_QOS_TEMPLATE_TYPE0     0
#define SOC_SBX_NODE_QOS_TEMPLATE_TYPE1     1

#define SOC_SBX_QE_MODE_FIC                 0
#define SOC_SBX_QE_MODE_TME                 1
#define SOC_SBX_QE_MODE_HYBRID              2
#define SOC_SBX_QE_MODE_TME_BYPASS          3 /* Sportster mode */

#define SOC_SBX_PORT_MODE_BURST_IL          0 /* Burst Interleaved */
#define SOC_SBX_PORT_MODE_PKT_IL_N_1        1 /* Packet Interleaved, N:1 channel sharing mode */
#define SOC_SBX_PORT_MODE_PKT_IL_FULL       2 /* Packet interleaved, full packet mode */


#define SOC_SBX_IF_PROTOCOL_XGS             0
#define SOC_SBX_IF_PROTOCOL_SBX             1

#define SOC_SBX_API_PARAM_NO_CHANGE         (-1)
/*
 * Different arbitration port allocation schemes. Relevant when independent ef/nef flow
 * control is enabled. this functionality is enabled via the following SOC property setting
 * "egress_fifo_independent_fc=1".
 */
#define SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1    0
                                     /* odd buckets reserved for nef */
#define SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2    1
                                     /* odd sysport from same bucket reserved for nef */

#define SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION1    64
#define SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION2    32


#define SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC    0
#define SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC      1
#define SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_IFC     2
#define SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_NIFC     3

/* On QE2K and SIRIUS */
#define SBX_QE_CPU_PORT        49

#define SBX_MAX_LAGS        128
#define SBX_MAX_LAG_SIZE    8
#define SBX_MAX_VRF         1024 
#define SBX_MAX_VID         (4*1024)
#define SBX_MAX_L2_EGRESS_OHI (8*1024)

#define SBX_VRF_BASE           0
#define SBX_VRF_END            (SBX_VRF_BASE + SBX_MAX_VRF - 1)

#define SBX_QOS_DEFAULT_PROFILE_IDX    0
#define SBX_QOS_DEFAULT_REMARK_IDX     0

#define SBX_QOS_PROFILE_BASE      1
#define SBX_QOS_EGR_REMARK_BASE   1

#define SBX_PROTECTION_BASE   1
#define SBX_PROTECTION_END    1023

#define SBX_VSI_FROM_VID(vid)  (vid)
#define SBX_VSI_TO_VID(vid)    (vid)

#define SBX_OAM_SPI_LB_PORT   0x1e
#define SBX_OAM_SPI_LB_QUEUE  0x38

#define SBX_MAX_MODULES_FP_ON_HIGIG 5

#define SBX_MAX_VPLS_COLORS (1)

/*
 * Convenience macros for obtaining the vairious FTE segment addresses.
 * All FTE macros return FTE addresses, no offset need be applied
 * to find the segment.
 */
#ifdef BCM_CALADAN3_SUPPORT
#define CFG_FTE_BASE(u,g) \
    (SOC_SBX_CFG_CALADAN3(u)->fteMap[(g)])
#endif

#define SBX_FTE_x_BASE(u, x)    CFG_FTE_BASE(u, x)
#define SBX_FTE_x_END(u, x)    (CFG_FTE_BASE(u, x + 1) - 1)

/* _BASE & _END are inclusive.
 */
#define SBX_PORT_FTE_BASE(u)         SBX_FTE_x_BASE(u, SOC_SBX_FSEG_PORT_MESH)
#define SBX_PORT_FTE_END(u)          SBX_FTE_x_END (u, SOC_SBX_FSEG_PORT_MESH)
#define SBX_DROP_FTE(u)              SBX_FTE_x_BASE(u, SOC_SBX_FSEG_DROP)
#define SBX_CPU_FTE(u)               SBX_FTE_x_BASE(u, SOC_SBX_FSEG_CPU)
#define SBX_TRUNK_FTE_BASE(u)        SBX_FTE_x_BASE(u, SOC_SBX_FSEG_TRUNK)
#define SBX_TRUNK_FTE_END(u)         SBX_FTE_x_END (u, SOC_SBX_FSEG_TRUNK)
#define SBX_HIGIG_FTE_BASE(u)        SBX_FTE_x_BASE(u, SOC_SBX_FSEG_HG_PORT)
#define SBX_HIGIG_FTE_END(u)         SBX_FTE_x_END (u, SOC_SBX_FSEG_HG_PORT)
#define SBX_VPLS_COLOR_FTE_BASE(u)   SBX_FTE_x_BASE(u, SOC_SBX_FSEG_VPLS_COLOR)
#define SBX_VPLS_COLOR_FTE_END(u)    SBX_FTE_x_END (u, SOC_SBX_FSEG_VPLS_COLOR)
#define SBX_LOCAL_GPORT_FTE_BASE(u)  SBX_FTE_x_BASE(u, SOC_SBX_FSEG_LGPORT)
#define SBX_LOCAL_GPORT_FTE_END(u)   SBX_FTE_x_END (u, SOC_SBX_FSEG_LGPORT)
#define SBX_GLOBAL_GPORT_FTE_BASE(u) SBX_FTE_x_BASE(u, SOC_SBX_FSEG_GGPORT)
#define SBX_GLOBAL_GPORT_FTE_END(u)  SBX_FTE_x_END (u, SOC_SBX_FSEG_GGPORT)
#define SBX_VID_VSI_FTE_BASE(u)      SBX_FTE_x_BASE(u, SOC_SBX_FSEG_VID_VSI)
#define SBX_VID_VSI_FTE_END(u)       SBX_FTE_x_END (u, SOC_SBX_FSEG_VID_VSI)
#define SBX_DYNAMIC_VSI_FTE_BASE(u)  SBX_FTE_x_BASE(u, SOC_SBX_FSEG_DYN_VSI)
#define SBX_DYNAMIC_VSI_FTE_END(u)   SBX_FTE_x_END (u, SOC_SBX_FSEG_DYN_VSI)
#define SBX_VPWS_UNI_FTE_BASE(u)      SBX_FTE_x_BASE(u, SOC_SBX_FSEG_VPWS_UNI)
#define SBX_VPWS_UNI_FTE_END(u)       SBX_FTE_x_END (u, SOC_SBX_FSEG_VPWS_UNI)
#define SBX_PW_FO_FTE_BASE(u)        SBX_FTE_x_BASE(u, SOC_SBX_FSEG_PW_FO)
#define SBX_PW_FO_FTE_END(u)         SBX_FTE_x_END (u, SOC_SBX_FSEG_PW_FO)
#define SBX_DYNAMIC_FTE_BASE(u)      SBX_FTE_x_BASE(u, SOC_SBX_FSEG_DYNAMIC)
#define SBX_DYNAMIC_FTE_END(u)       SBX_FTE_x_END (u, SOC_SBX_FSEG_DYNAMIC)
#define SBX_EXTRA_FTE_BASE(u)        SBX_FTE_x_BASE(u, SOC_SBX_FSEG_EXTRA)
#define SBX_EXTRA_FTE_END(u)         SBX_FTE_x_END (u, SOC_SBX_FSEG_EXTRA)

/*
 * Dynamic VSI refers to the available VSI values in the system above and
 * beyond the VLAN(VID) VSIs.  When a DYNAMIC VSI is allocated, an implied
 * FTE is also allocated and is located at 'SBX_VID_VSI_FTE_BASE(fe) + vsi'
 */
#define SBX_DYNAMIC_VSI_BASE(u)   SBX_MAX_VID
#if defined(BCM_CALADAN3_SUPPORT)
#define SBX_DYNAMIC_VSI_END(u)  (SOC_SBX_CFG_CALADAN3(u)->vsiEnd)
#endif

/**
 * Unicast, Multicast and Exception QID ranges.
 * Max is SB_G2_FE_MAX_QID
 * Note:
 *  With BM9600, UC and MC queues preallocation no longer able to cover
 *  all possible node/port/cos. We cover same node/port/cos as BM3200,
 *  32 nodes/ 50 ports, 128 esets
 */
#define SBX_UC_QID_BASE          0
#define SBX_UC_QID_END           (SBX_UC_QID_BASE +                      \
                                  ((SOC_SBX_CFG(unit)->cfg_num_nodes) *  \
                                    SBX_MAX_PORTS * SBX_MAX_COS) - 1)
#define SBX_MC_QID_BASE          (SBX_UC_QID_END + 1)
#define SBX_MC_QID_END           (SBX_MC_QID_BASE +                      \
                                  ((SOC_SBX_CFG(unit)->num_ds_ids) *     \
                                          SBX_MAX_COS) - 1)
/*
 * In QE2000 all exceptions go to CPU cos queues
 * Next generation QE will allow us to map exceptions to a
 * dedicated queue group to CPU
 */
#define SBX_EXC_QID_BASE(unit)    SOC_IS_CALADAN3(unit) ?                  \
                                      SBX_EXC_C3_QID_BASE(unit) :          \
                                          SBX_EXC_FE_QID_BASE(unit)

#define SBX_EXC_FE_QID_BASE(unit)  (SBX_UC_QID_BASE +                      \
                                       (SBX_QE_CPU_PORT * NUM_COS(unit)))
#define SBX_EXC_FE_QID_END(unit)   (SBX_EXC_FE_QID_BASE +                  \
                                       (SBX_MAX_EXC_QID * NUM_COS(unit))-1)

#ifdef BCM_SIRIUS_SUPPORT
#define SBX_EXC_C3_QID_PORT         54
#else

#define SBX_EXC_C3_QID_PORT         54
#endif
                                     

#define SBX_EXC_C3_QID_BASE(unit)  (SBX_UC_QID_BASE +                      \
                                      (SBX_EXC_C3_QID_PORT * NUM_COS(unit)))
#define SBX_EXC_C3_QID_END(unit)   (SBX_EXC_C3_QID_BASE +                   \
                                      (SBX_MAX_EXC_QID * NUM_COS(unit)) - 1)

#define SBX_QID_END              (SBX_MAX_QID - 1)

#define SBX_PORT_SID_BASE(u)        SBX_PORT_FTE_BASE(u)
#define SBX_PORT_SID_END(u)         SBX_PORT_FTE_END(u)
#define SBX_TRUNK_SID_BASE(u)       SBX_TRUNK_FTE_BASE(u)
#define SBX_TRUNK_SID_END(u)        SBX_TRUNK_FTE_END(u)

#define SBX_TB_VID_OHI_BASE      0
#define SBX_TB_VID_OHI_END       ((4 * 1024) - 1)
#define SBX_TB_IPMC_OHI_BASE     (SBX_TB_VID_OHI_END + 1)
#define SBX_TB_IPMC_OHI_END      ((8 * 1024) - 1)
#define SBX_RAW_OHI_BASE         (8 * 1024)
#define SBX_RAW_OHI_END          (SBX_RAW_OHI_BASE + 2)
#define SBX_PORT_OHI_BASE        (SBX_RAW_OHI_END + 1)
#define SBX_PORT_OHI_END         (SBX_PORT_OHI_BASE + SBX_MAX_PORTS - 1)
#define SBX_L2_EGRESS_OHI_BASE   (SBX_PORT_OHI_END + 1)
#define SBX_L2_EGRESS_OHI_END    (SBX_L2_EGRESS_OHI_BASE + SBX_MAX_L2_EGRESS_OHI - 1)
#define SBX_TRUNK_OHI_BASE       (SBX_L2_EGRESS_OHI_END + 1)
#define SBX_TRUNK_OHI_END        (SBX_TRUNK_OHI_BASE + SBX_MAX_TRUNKS - 1)
#define SBX_DYNAMIC_OHI_BASE     (SBX_TRUNK_OHI_END + 1)
#define SBX_DYNAMIC_OHI_END      #error Please use sbG2FeOutHdrIndexRangeGet()

#define SBX_LPORT_BASE           (SBX_MAX_PORTS)
#define SBX_LPORT_END            ((16 * 1024) - 1)

#if 0
#define SBX_VPLS_COLOR_INVALID  0
#define SBX_VPLS_COLOR_BASE     1
#define SBX_VPLS_COLOR_END      ((16 * 1024) - 1)
#endif

#define SBX_FE2K_NUM_MPLS_LBLS   0x0FFFF
#define SBX_MPLS_LPORT_BASE      (SBX_LPORT_END + 1)
#define SBX_MPLS_LPORT_END       (SBX_MPLS_LPORT_BASE + SBX_FE2K_NUM_MPLS_LBLS -1)

#define SBX_QE_BASE_MODID        10000

/**
 * OHI Macros
 */
#define SOC_SBX_TRUNK_TO_OHI(tid) \
    (SBX_TRUNK_OHI_BASE + (tid))

#define _SBX_OHI_ENCAP_ID_SIGNATURE   0xFE000000
#define _SBX_L2_ENCAP_ID_SIGNATURE    0xDD000000

#define SOC_SBX_ENCAP_ID_FROM_OHI(_ohi) \
    (_SBX_OHI_ENCAP_ID_SIGNATURE | (_ohi))

#define SOC_SBX_OHI_FROM_ENCAP_ID(_encap_id) \
    ((_encap_id) & ~_SBX_OHI_ENCAP_ID_SIGNATURE)

#define SOC_SBX_IS_VALID_ENCAP_ID(_encap_id) \
    (((_encap_id) & _SBX_OHI_ENCAP_ID_SIGNATURE) == _SBX_OHI_ENCAP_ID_SIGNATURE)

#define SOC_SBX_L2_ENCAP_ID_FROM_OHI(_ohi) \
    (_SBX_L2_ENCAP_ID_SIGNATURE | ((_ohi) - (SBX_L2_EGRESS_OHI_BASE)))

#define SOC_SBX_OHI_FROM_L2_ENCAP_ID(_encap_id) \
    ((SBX_L2_EGRESS_OHI_BASE) + ((_encap_id) & ~_SBX_L2_ENCAP_ID_SIGNATURE))

#define SOC_SBX_IS_VALID_L2_ENCAP_ID(_encap_id) \
    (((_encap_id) & _SBX_L2_ENCAP_ID_SIGNATURE) == _SBX_L2_ENCAP_ID_SIGNATURE)

#define SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(off) \
       ((off) | _SBX_L2_ENCAP_ID_SIGNATURE)

#define SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(_encap_id) \
        ((_encap_id) & ~_SBX_L2_ENCAP_ID_SIGNATURE)

#define SOC_SBX_OHI_FROM_ANY_ENCAP_ID(_ohi, _any_encap_id)      \
    if (SOC_SBX_IS_VALID_L2_ENCAP_ID((_any_encap_id))) {         \
        _ohi = SOC_SBX_OHI_FROM_L2_ENCAP_ID(_any_encap_id);     \
    } else if (SOC_SBX_IS_VALID_ENCAP_ID((_any_encap_id))) {    \
        _ohi = SOC_SBX_OHI_FROM_ENCAP_ID((_any_encap_id));      \
    } else {                                                    \
        _ohi = (_any_encap_id) & 0x3FFFF;                       \
    }

/*
 * QID Macros
 *
 * For unicast, QID in the FTE is the base QID for a given system
 * port; Queues are allocated for each system port based on
 * maximum number of COS levels supported
 * If the destination port is a LAG, QID is a union of LagIndex
 * and LagSize;
 *
 * For multicast, QID in the FTE represents the base QID of an
 * ESET; Multiple MC groups may use a single ESET in which case
 * packets hitting the MC group end up sharing the same set of
 * queues (for that ESET)
 *
 */

#define _BITS(numcos)                                       \
          ((numcos) <= 1 ? 3 : ((numcos) <= 2 ? 1 :         \
				((numcos) <= 4 ? 2 : ((numcos) <= 8 ? 3 : 4))))


#define SOC_SBX_TRUNK_TO_QID(tid) \
          ((tid) << SBX_MAX_TRUNK_SIZE)

/* change node/port - qid macros to call functions to look at config params */
extern int map_np_to_qid(int unit,int node, int port, int numcos);
extern int map_qid_to_np(int unit, int qid, int *node, int *port, int numcos);
extern int map_ds_id_to_qid(int unit, int ds_id, int numcos);
extern int map_qid_to_ds_id(int unit, int qid, int numcos);
extern int soc_sbx_hybrid_demarcation_qid_get(int unit);

#define SOC_SBX_NODE_PORT_TO_QID(unit,node_id, port, numcos)	\
         map_np_to_qid(unit,node_id, port, numcos)

#define SOC_SBX_NODE_PORT_FROM_QID(unit,qid, node, port, numcos)	\
         map_qid_to_np(unit, qid, &node, &port, numcos)


#define SOC_SBX_DS_ID_TO_QID(unit, ds_id, numcos)           \
        map_ds_id_to_qid(unit, ds_id,  numcos)

#define SOC_SBX_DS_ID_FROM_QID(unit, qid, numcos)           \
        map_qid_to_ds_id(unit, qid,  numcos)


#define SOC_SBX_TRUNK_FROM_QID(qid, tid)                    \
    (tid = (((qid) >> SBX_MAX_TRUNK_SIZE) &                 \
                   ((1 << SBX_MAX_TRUNK_SIZE) - 1)))

#define SOC_SBX_HYBRID_DEMARCATION_QID(unit)                \
         soc_sbx_hybrid_demarcation_qid_get(unit)

/*
 * When the <unit> is an FE, given a QE <node, port>, find the module ID of
 * the FE device that owns the port; the FE could be local or remote.
 * When the <unit> is a QE, given a QE <node, port>, find the module ID of
 * the QE.  The QE <node, port> can be local or remote.
 */
int soc_sbx_modid_get(int unit, int f_node, int f_port, int *module);

/*
 * Given a FE <unit, modid, port>, find QE <unit, nodeid, port>
 * Note that module ID being passed can be a remote module ID, in
 * which case fab_unit is bogus
 */
int soc_sbx_node_port_get(int unit, int module_id, int port,
                          int *fab_uint, int *node_id, int *fabric_port);

/*
 * Given a QE node id, get the corresponding QE module id
 */
#define SOC_SBX_MODID_FROM_NODE(nodeid, modid) \
  ((modid) = (nodeid) + SBX_QE_BASE_MODID)

/*
 * Given a QE module id, get the corresponding QE Node id
 */
#define SOC_SBX_NODE_FROM_MODID(modid, nodeid)  \
  ((nodeid) = (modid) - SBX_QE_BASE_MODID)


/*
 * FTE Macros
 *
 */
#define SOC_SBX_NODE_VALID(unit, node) ((node) >= 0 && (node) <= 31)
#define SOC_SBX_NODE_GET(unit) \
          (SOC_SBX_CONTROL(unit)->node_id)
#define SOC_SBX_REMOTE_NODE_GET(unit, module_id, port) \
          (((SOC_SBX_CONTROL(unit)->modport[(module_id)][(port)] >> 16)      \
             - SBX_QE_BASE_MODID) & 0x1f)

#define SOC_SBX_PORT_FTE(_unit, _node, _port) \
          (SBX_PORT_FTE_BASE(_unit) +                                        \
           ((_node) * SOC_SBX_CFG(_unit)->nMaxFabricPortsOnModule) + (_port))

#define SOC_SBX_TRUNK_FTE(u, tid) \
    (SBX_TRUNK_VALID(tid) ? \
     SBX_TRUNK_FTE_BASE(u) + (tid) : SBX_INVALID_FTE)

#define SOC_SBX_HIGIG_FTE(u, port) \
          (SBX_HIGIG_FTE_BASE(u) + port)

/* Converse FTE macros */
#define SOC_SBX_PORT_TGID_GET(u, fte_idx, node, port, tgid)                  \
          do {                                                               \
              uint32 _num_ports_ =                                           \
                         SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule;         \
              (node) = SBX_INVALID_NODE;                                     \
              (port) = SBX_INVALID_PORT;                                     \
              (tgid) = SBX_INVALID_TRUNK;                                    \
              if (fte_idx >= SBX_PORT_FTE_BASE(u) &&                         \
                  fte_idx < SBX_TRUNK_FTE_BASE(u)) {                         \
                  (node) = (fte_idx - SBX_PORT_FTE_BASE(u)) / _num_ports_;   \
                  (port) = (fte_idx - SBX_PORT_FTE_BASE(u)) % _num_ports_;   \
              } else if (fte_idx < SBX_HIGIG_FTE_BASE(u)) {                  \
                  (tgid) = fte_idx - SBX_TRUNK_FTE_BASE(u);                  \
              }                                                              \
          } while(0)

/**
 * ETE/OHI Macros
 * Parameter validation is assumed to be done by caller
 */
#define SOC_SBX_PORT_ETE(_unit, _port)  \
    SOC_SBX_CONTROL(_unit)->port_ete[(_port)]

#define SOC_SBX_PORT_UT_ETE(_unit, _port)  \
    SOC_SBX_CONTROL(_unit)->port_ut_ete[(_port)]

#define SOC_SBX_TRUNK_ETE(_unit, _tid) \
    SOC_SBX_CONTROL(_unit)->trunk_ete[(_tid)]

#define SOC_SBX_INVALID_L2ETE(_unit)  \
    (SOC_SBX_CONTROL(_unit)->invalid_l2ete)

/* XXX: Below macro needs work, Suresh ?? */
#define SOC_SBX_PORT_OHI(port)  (SBX_PORT_OHI_BASE + (port))
#define SOC_SBX_TRUNK_OHI(tid)  (SBX_TRUNK_OHI_BASE + (tid))

/*
 * SID Macros
 */
#define SOC_SBX_TRUNK_TO_SID(u, tid) \
    (SBX_TRUNK_SID_BASE(u) + (tid))

#define SOC_SBX_PORT_SID(u, node, port) \
    (SBX_PORT_SID_BASE(u) +              \
        (((node) * SOC_SBX_CFG(u)->nMaxFabricPortsOnModule) + (port)))

#define SOC_SBX_PORT_FROM_SID(u_, node_, sid_) \
    ((sid_) - ((node_) * (SOC_SBX_CFG(u_)->nMaxFabricPortsOnModule) - \
       SBX_PORT_SID_BASE(u_)))

#define SOC_SBX_TRUNK_SID(u, tid) \
            (SBX_TRUNK_SID_BASE(u) + (tid))

#define SOC_SBX_SID_IS_TRUNK(u, sid) \
            ((sid) >= SBX_TRUNK_SID_BASE(u) && (sid) <= SBX_TRUNK_SID_END(u))

#define SOC_SBX_PORT_ADDRESSABLE(unit,port) \
            (((port) >= 0) && ((port) < (SBX_MAX_PORTS)))
#define SOC_SBX_MODID_ADDRESSABLE(unit,mod) \
            (((mod) >= 0) && ((mod) < (SBX_MAX_MODIDS)))
#define SOC_SBX_NODE_ADDRESSABLE(unit,node) \
            (((node) >= 0) && ((node) < (SBX_MAX_NODES)))

/*
 * QE that owns a local (FE) port
 */
#define SOC_SBX_QE_FROM_FE(unit, port)                \
    ((int)(SOC_SBX_CONTROL(unit)->fabric_units[(port)]))

/*
 * FE that owns a local (FE) port on a given QE
 */
#define SOC_SBX_FE_FROM_QE(unit, port)                \
    ((int)(SOC_SBX_CONTROL(unit)->forwarding_units[(port)]))

/* Macros to get Physical Node given a logical Node */
#define SOC_SBX_L2P_NODE(unit, lgl_node)              \
                          (SOC_SBX_CFG(unit)->l2p_node[lgl_node])

#define SOC_SBX_P2L_NODE(unit, phy_node)              \
                          (SOC_SBX_CFG(unit)->p2l_node[phy_node])

#define SOC_SBX_V4_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->v4_ena)
#define SOC_SBX_OAM_RX_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->oam_rx_ena)
#define SOC_SBX_OAM_TX_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->oam_tx_ena)
#define SOC_SBX_OAM_SPI_LB_PORT(unit)                     \
                          (SOC_SBX_CFG(unit)->oam_spi_lb_port)
#define SOC_SBX_OAM_SPI_LB_QUEUE(unit)                    \
                          (SOC_SBX_CFG(unit)->oam_spi_lb_queue)
#define SOC_SBX_V4MC_STR_SEL(unit)                       \
                          (SOC_SBX_CFG(unit)->v4mc_str_sel)
#define SOC_SBX_V4UC_STR_SEL(unit)                        \
                          (SOC_SBX_CFG(unit)->v4uc_str_sel)
#define SOC_SBX_MIM_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->mim_ena)
#define SOC_SBX_V6_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->v6_ena)
#define SOC_SBX_MPLSTP_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->mplstp_ena)
#define SOC_SBX_DSCP_ENABLE(unit)                       \
                          (SOC_SBX_CFG(unit)->dscp_ena)

/*
 * Externs
 */
#define SOC_SBX_NUM_SUPPORTED_CHIPS 14
extern soc_driver_t *soc_sbx_driver_table[SOC_SBX_NUM_SUPPORTED_CHIPS];
#define SOC_SBX_DRIVER_ACTIVE(i) (soc_sbx_driver_table[i]->block_info)

/* 
 * Used for creating a unique SCACHE handle 
 * We cannot use BCM_MODULE__COUNT in include/bcm/module.h as that is the starting 
 * for C3 specific modules,  the last BCM module C3 specific is 
 * BCM_CALADAN3_MODULE__COUNT defined in include/bcm_int/sbx/caladan3/bcm_sw_db.h
 *
 * 
 */
#define SOC_MODULE_COUNT_START 100

enum {
    SOC_SBX_WB_MODULE_CONNECT = SOC_MODULE_COUNT_START,     
    SOC_SBX_WB_MODULE_COMMON,    
    SOC_SBX_WB_MODULE_SOC,        
    SOC_SBX_WB_MODULE_PPE,        
    SOC_SBX_WB_MODULE_LINK,       
    SOC_SBX_WB_MODULE_CMU,        
    SOC_SBX_WB_MODULE_COP,        
    SOC_SBX_WB_MODULE_TMU,        
    SOC_SBX_WB_MODULE_RCE,
    SOC_SBX_WB_MODULE_COUNTER,
    /* must be last in list */
    SOC_SBX_WB_MODULE__COUNT 
};

#define SOC_MODULE_NAMES_INITIALIZER                        \
{                                                           \
    "soc connect",                                          \
    "soc common",                                           \
    "soc sbx",                                              \
    "soc ppe",                                              \
    "soc link",                                             \
    "soc cmu",                                              \
    "soc cop",                                              \
    "soc tmu",                                              \
    "soc rce",                                              \
    "soc counter"                                           \
} 

extern void soc_sbx_control_dump(int unit);

extern char *soc_sbx_module_name(int unit, int module_num);

#if defined(BCM_WARM_BOOT_SUPPORT)
extern int soc_sbx_shutdown(int unit);

extern int soc_wb_state_alloc_and_check(int unit, soc_scache_handle_t hdl,
                                        uint32 *size, uint32 current_version, 
                                        int *upgrade);

/* Convenience macros for disabling/enabling warm boot; useful for 
 * accessing indirect memories during warmboot
 */
#define SOC_SBX_WARM_BOOT_DECLARE(v)         v
#define SOC_SBX_WARM_BOOT_IGNORE(u_, m_)                        \
  (m_) =  SOC_WARM_BOOT((u_));                                  \
  if ((m_)) {   SOC_WARM_BOOT_DONE((u_));    } 

#define SOC_SBX_WARM_BOOT_OBSERVE(u_, m_)                       \
  if ((m_)) { SOC_WARM_BOOT_START((u_)); } 

#else /* BCM_WARM_BOOT_SUPPORT */

#define SOC_SBX_WARM_BOOT_DECLARE(v)
#define SOC_SBX_WARM_BOOT_IGNORE(u_, m_)
#define SOC_SBX_WARM_BOOT_OBSERVE(u_, m_)

#endif /* BCM_WARM_BOOT_SUPPORT */

extern int soc_sbx_attach(int unit);
extern int soc_sbx_detach(int unit);
extern int soc_sbx_init(int unit);
extern int soc_sbx_reset_init(int unit);

extern int soc_sbx_block_find(int unit, int type, int number);
extern soc_driver_t *soc_sbx_chip_driver_find(uint16 dev_id, uint8 rev_id);
extern int soc_sbx_dump(int unit, const char *pfx);
extern void soc_sbx_chip_dump(int unit, soc_driver_t *d);

extern void soc_sbx_xport_type_update(int unit, soc_port_t port, int to_hg_port);
extern int soc_sbx_info_config(int unit, int dev_id, int drv_dev_id);

extern int
soc_sbx_set_epoch_length(int unit);

#ifdef  BCM_SIRIUS_SUPPORT
extern int soc_sbx_sirius_process_cs_dma_fifo(int unit);
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
extern int soc_sbx_wb_sync(int unit, int sync);
extern int soc_sbx_wb_connect_state_sync(int unit, int sync);
#endif
extern int soc_sbx_div64(uint64 x, uint32 y, uint32 *result);
extern void  soc_sbx_drv_signature_show(int unit);
#endif  /* !_SOC_SBX_DRV_H */
