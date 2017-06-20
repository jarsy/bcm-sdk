/*
 * $Id: sbFabCommon.h,v 1.55 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Common defines for QE, BME Link config
 */

#ifndef _SB_FAB_COMMON_H_
#define _SB_FAB_COMMON_H_

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbFabStatus.h>

/* Some compilers do not support 'packed' attribute */
#if defined(VXWORKS) && (CPU == PPC603) && (VX_VERSION == 54) || defined(__DCC__) && (__DCC__ == 1)
#define PACK_STRUCT
#else
#define PACK_STRUCT __attribute__ ((__packed__))
#endif

typedef struct sbLinkSpecificDriverConfigRambusRs3214_s{
    int32 nHiDrv;
    int32 nLoDrv;
    int32 nDtx;
    int32 nDeq;
} sbLinkSpecificDriverConfigRambusRs3214_t;

typedef struct sbLinkSpecificDriverConfigHypercoreStrength_s {
    int32 nIDriver;  /* strength main tx amplitude control 4 bits */
    int32 nIPreDriver; /* strength 4 bits */
} sbLinkSpecificDriverConfigHypercoreStrength_t;

typedef struct sbLinkSpecificDriverConfigHypercoreEqualization_s {
    int32 nPremphasisPost; /* equalization 0-66%  4 bits */
    int32 nPreemphasisPre; /* equalization 0-33% 4 bits */
} sbLinkSpecificDriverConfigHypercoreEqualization_t;

typedef struct sbLinkSpecificDriverConfigHypercore_s {
  sbLinkSpecificDriverConfigHypercoreStrength_t strength;
  sbLinkSpecificDriverConfigHypercoreEqualization_t equalization;
} sbLinkSpecificDriverConfigHypercore_t;

typedef struct sbLinkSpecificDriverConfig_s {
    int32 nLinkType;

    union {
        sbLinkSpecificDriverConfigHypercore_t hypercore;
        sbLinkSpecificDriverConfigRambusRs3214_t rambusRs2314;
    } u;
} sbLinkSpecificDriverConfig_t;

typedef struct sbLinkThresholdConfig_s {
    uint32 uLsThreshold;            /* range 1-255 */
    uint32 uLsWindowIn256Clocks;    /* range 1-255 on a QE1000, this is
                                         adjusted to be uLsThreshold*255 clocks
                                         to match BM3200/QE2000
                                       */
} sbLinkThresholdConfig_t;

typedef struct PACK_STRUCT sbLinkDriverConfig_s {
    uint32 uDriverStrength;     /* Value in the range of [0,100] */
    uint32 uDriverEqualization; /* Value in the range of [0,100] */
} sbLinkDriverConfig_t;


typedef struct sbLinkStatus_s {
    sbBool_t bLinkEnabled;
    sbBool_t bLinkUp;              /*  bTimeAligned & bByteAligned &
                                    *  !bRxLossOfSyncEvent & !bRxLosEvent &
                                    *  !bTxFifoOverflowEvent &
                                    *  !bRxFifoOverFlowEvent
                                    */
    sbBool_t bTimeAligned;
    sbBool_t bByteAligned;
    sbBool_t bLossOfSyncEvent;
    sbBool_t bRxLosEvent;
    sbBool_t bTxFifoOverflowEvent;
    sbBool_t bRxFifoOverflowEvent;
    uint32 nSerializerStatusRegister;
    uint32 nSerializerErrorRegister;
    uint32 nSerializerStickyStateRegister;
} sbLinkStatus_t;

typedef struct sbLinkState_s {
  int32 nState; /* unused, data, control, etc */
    int32 nRemoteDeviceType; /* logical crossbar, lcm, switch controller */
    int32 nRemoteDeviceId;
    
    int32 nLogicalCrossbarId; /* data links: Always track logical crossbar, invalid if device type is switch controller */
    int32 nAssociatedNodeId;  /* data links: Always track a node associated with the link, esp. for pass through LCMs */
    int32 nPlane; /* 0 or 1, control/data plane */
    sbLinkStatus_t linkStatus; /* state information about health of the link */
} sbLinkState_t;

/* QE2000 defines */
#define SB_FAB_DEVICE_QE2000_SFI_LINKS               (18)
#define SB_FAB_DEVICE_QE2000_SCI_LINKS                (2)
#define SB_FAB_DEVICE_QE2000_SFI_PBMP_BASE           (50) /* if sfi pbmp changes, this must change */
#define SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS         (SB_FAB_DEVICE_QE2000_SFI_LINKS + SB_FAB_DEVICE_QE2000_SCI_LINKS)
#define SB_FAB_DEVICE_QE2000_NUM_XCFG_REMAP_ENTRIES (256)
#define SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES       (2)
#define SB_FAB_DEVICE_QE2000_NUM_QUEUES           (16384)
#define SB_FAB_DEVICE_QE2000_MAX_PHYSICAL_PORTS      (51) /* includes multicast */
#define SB_FAB_DEVICE_QE2000_MAX_FABRIC_PORTS       (102)
#define SB_FAB_DEVICE_QE2000_MAX_SPI_SUBPORTs        (49)

#define SB_FAB_DEVICE_QE2000_PCI_PORT                (49)
#define SB_FAB_DEVICE_QE2000_MULTICAST_PORT          (50)

#define SB_FAB_DEVICE_QE2000_QUEUE_BUFFER_SIZE      (4096)

#define SB_FAB_DEVICE_QE2000_MAX_EGRESS_SHAPERS     (150)

/* Caladan3 defines */
#define SB_FAB_DEVICE_CALADAN3_NUM_QUEUES            (65536)

/* Sirius defines */
#define SB_FAB_DEVICE_SIRIUS_SFI_LINKS             (22)
#define SB_FAB_DEVICE_SIRIUS_SCI_LINKS             (2)
#define SB_FAB_DEVICE_SIRIUS_SFI_PBMP_BASE         (9) /* if sfi pbmp changes, this must change */
#define SB_FAB_DEVICE_SIRIUS_SCI_PBMP_BASE         (31) /* if sci pbmp changes, this must change */

#define SB_FAB_DEVICE_SIRIUS_LINKS                 (SB_FAB_DEVICE_SIRIUS_SFI_LINKS + SB_FAB_DEVICE_SIRIUS_SCI_LINKS)
#define SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS       (SB_FAB_DEVICE_SIRIUS_LINKS)
#define SB_FAB_DEVICE_SIRIUS_INTERNAL_LINKS        (32)
#define SB_FAB_DEVICE_SIRIUS_SC_SFI_PORTS          (22)
#define SB_FAB_DEVICE_SIRIUS_SF_SFI_PORTS          (24)
#define SB_FAB_DEVICE_SIRIUS_SFI_PORTS             (SB_FAB_DEVICE_SIRIUS_SC_SFI_PORTS + SB_FAB_DEVICE_SIRIUS_SF_SFI_PORTS)
#define SB_FAB_DEVICE_SIRIUS_NUM_QUEUES            (65536)
#define SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET        (65536)
#define SB_FAB_DEVICE_SIRIUS_MAX_VOQ               (32768-1)
#define SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS    (132)
#define SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS      (264)

#define SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0         (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS - 4)
#define SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE1         (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS - 3)
#define SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE2         (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS - 2)
#define SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3         (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS - 1)
#define SB_FAB_DEVICE_SIRIUS_IS_MCAST_HANDLE(handle)                                           \
             ((handle >= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0) &&                                \
                             (handle <= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3))

#define SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE_HG_GET(handle)  (handle - SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0)
                             


#define SB_FAB_DEVICE_SIRIUS_CPU_HANDLE            (128)
#define SB_FAB_DEVICE_SIRIUS_INGRESS_MCAST_HANDLE  (SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0 - 1)
#define SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS        (265)
#define SB_FAB_DEVICE_SIRIUS_INGRESS_MCAST_FC_DOMAIN  (132)
#define SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS     (4)
#define SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS   (4)
#define SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS          (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS + SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS)
#define SB_FAB_DEVICE_SIRIUS_NUM_OOB_HCFC_PORTS    (2)
#define SB_FAB_DEVICE_SIRIUS_CPU_PORT              (SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS) /* Assuming CPU port right after all higig ports and at fix offset */
#define SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE    (4)
#define SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE   (16)
#define SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX        (16)
#define SB_FAB_DEVICE_SIRIUS_FIFO_NUM              (528)
#define SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS   (64)
#define SB_FAB_DEVICE_SIRIUS_MAX_HG_SUBPORTS       (128)
#define SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES  (7)
#define SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A         (0)
#define SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B         (1)
#define SB_FAB_DEVICE_SIRIUS_NUM_REQUEUE_PORTS     (2)
#define SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS         (8)
#define SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS         (5)
#define SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_SEL   (16)
#define SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_VALUE (254)

#define SB_FAB_DEVICE_SIRIUS_NUM_TDM_CALENDAR_ENTRIES_PER_PLANE 2048

#define SB_FAB_DEVICE_GROUP_SHAPER_LEVEL           (15)

/*
 * Fabric defines
 */
/* 4 FIFO model currently */

#define SB_FAB_XCORE_COS_FIFO_UNICAST_EF     0
#define SB_FAB_XCORE_COS_FIFO_UNICAST_NEF    1
#define SB_FAB_XCORE_COS_FIFO_MULTICAST_EF   2
#define SB_FAB_XCORE_COS_FIFO_MULTICAST_NEF  3

#define SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS 32
#define SB_FAB_DEVICE_INVALID_CROSSBAR      0xff
#define SB_FAB_DEVICE_MAX_NODE_CROSSBARS 24 /* sirius has max 24 link enable mapping to channels masks */
#define SB_FAB_DEVICE_MAX_PHYSICAL_SERDES 96 /* maximum number of serdes is 96 on BM9600 */

#define SB_FAB_DEVICE_MAX_FABRIC_PORTS             (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)

#define SB_FAB_DEVICE_UNKNOWN          (0x45450000)
#define SB_FAB_DEVICE_SE4000           (0x45450001)
#define SB_FAB_DEVICE_BM3200           (0x45450002)
#define SB_FAB_DEVICE_LCM4000          (0x45450003)
#define SB_FAB_DEVICE_QE1000           (0x45450004)
#define SB_FAB_DEVICE_QE2000           (0x45450005)
#define SB_FAB_DEVICE_TME2000          (0x45450006)
#define SB_FAB_DEVICE_LOGICAL_CROSSBAR (0x45450007) /* We need a logical crossbar device for link state of QE SFI's */
#define SB_FAB_DEVICE_QE               (0x45450008) /* Used only in determining ownership of a port, abstract device type */
#define SB_FAB_DEVICE_TME              (0x45450009) /* Used only in determining ownership of a port, abstract device type */
#define SB_FAB_DEVICE_LCM_GENERIC      (0x4545000A)
#define SB_FAB_DEVICE_BM9600           (0x4545000B)
#define SB_FAB_DEVICE_SIRIUS           (0x4545000C)

#define SB_FAB_DEVICE_SERIALIZER_STATE_UNUSED  (-1)
#define SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED (1)
#define SB_FAB_DEVICE_SERIALIZER_STATE_DATA     (2)
#define SB_FAB_DEVICE_SERIALIZER_STATE_CONTROL  (8)

/* Set to BM3200 maximum number for initial value GNATS 22135 */
#define SB_FAB_USER_MAX_NUM_NODES      32
/* Maximum number of nodes across all current sbx devices */
#define SB_FAB_DEVICE_MAX_NODES      72
#define SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS    (46)
#define SB_FAB_DEVICE_INVALID_DATA_CHANNEL               (255)
#define SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS    (46)
#define SB_FAB_MAX_NODE_TYPES                   (4)
#define SB_FAB_NODE_TYPE_SIRIUS_FIC             (0)
#define SB_FAB_NODE_TYPE_QE2K                   (1)
#define SB_FAB_NODE_TYPE_SIRIUS_HYBRID          (2)
#define SB_FAB_NODE_TYPE_RESERVED               (3)

#define SB_FAB_MAX_NUM_DATA_PLANES               2
#define SB_FAB_MAX_NUM_GRANT_PLANES              2
#define SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS     40

#define SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS    (96)
#define SB_FAB_USER_MAX_NUM_SERIALIZERS         (SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS)

/* 12 -> 4B Timeslot Hdr + 8B QE40 Header(includes 1B padding) */
#define SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH            (12)

/* Pre-calculated TIMESLOT values */ 
#define SB_FAB_DEVICE_BM3200_MIN_TIMESLOT_IN_NS_DMODE        (760)
#define SB_FAB_TME_MIN_TIMESLOT_IN_NS                        (760)
#define SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_LEGACY (760)
#define SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_MIX    (920)
#define SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT        (955)
/* AC_TIMESLOT0/1 fields are 11bits wide */
#define SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS              \
            (0x7ff * BM9600_BW_CLOCK_PERIOD_200MHZ)

#define SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT              (35) /* GBPS */

/* NOTE: The following value should be consistent with the values specified */
/*       in sbFab.c. It is the maximum timeslot value that could be         */
/*       configured in the system.                                          */
#define SB_FAB_MAX_TIMESLOT_IN_NS   8192


#define SB_FAB_TME_EPOCH_IN_NS     (1000000)
#define SB_FAB_DMODE_EPOCH_IN_TIMESLOTS     16433 /* increase this value from 16401 due to epoch calc in bm3200_init.c */
#define SB_FAB_TME_EPOCH_IN_TIMESLOTS       ( SB_FAB_TME_EPOCH_IN_NS /* 1ms */ / SB_FAB_TME_MIN_TIMESLOT_IN_NS)
                                                 /* The calculation of this number in TME mode            */
                                                 /* really has nothing to do with timeslots               */
                                                 /* This number was calculated based on                   */
                                                 /* demand time=4 clocks/queue * 4ns * 16K queues=256K ns */
                                                 /* plus ~384ns for BAA = ~1ms see Scott/Kathryn          */

/*
 * These values should be the maximum of any Switch Controller
 */
#define SB_FAB_SWITCH_CONFIG_MAX_LOGICAL_PORTS  (4200)
#define SB_FAB_SWITCH_CONFIG_MAX_VOQS	        (16384)
#define SB_FAB_SWITCH_CONFIG_MAX_NUM_ESETS      (128)
#define SB_FAB_SWITCH_CONFIG_REF_COUNT_SINGLE   (1)
#define SB_FAB_SWITCH_CONFIG_MAX_WRED_PROFILES  (256)


/* The maximum equalization/drive strength value allowed for hypercore serdes */
#define LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX 16
#define LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX 256
#define LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_MAX 100 /* maximum for all serdes types */
#define LINK_SPECIFIC_CONFIG_EQUALIZATION_MAX   256 /* maximum for all serdes types */

/*
 * Externs
 */
extern sbFabStatus_t
GetLinkThresholdConfig(uint32 uLinkThresholdIndex,
		       sbLinkThresholdConfig_t *pLinkThresholdConfig);

extern void
GetDefaultLinkDriverConfig(sbFabUserDeviceHandle_t sbHandle, int32 nLink,
                           sbLinkDriverConfig_t *pLinkDriverConfig);

extern sbFabStatus_t
GetLinkSpecificConfig(sbFabUserDeviceHandle_t devHandle, int32 nLink,
                      sbLinkDriverConfig_t *pLinkDriverConfig,
                      sbLinkSpecificDriverConfig_t *pSpecificConfig);

int32
soc_sbx_fabric_get_timeslot_size(int unit, int32 nTotalLogicalCrossbars,
                                 int32 bSetTimeslotSizeForHalfBus,
                                 int32 bHybrid);

int32
soc_sbx_fabric_util_num_to_mant_exp(int unit,
				    soc_mem_t mem,
				    soc_field_t field,
				    uint32 num,
				    uint32 *mant,
				    uint32 *exp);

extern int
soc_sbx_fabric_get_timeslot_optimized(int unit, uint32 *ts_opt, 
                                      uint32 *opt_packlets);

#endif /* _SB_FAB_COMMON_H_ */
