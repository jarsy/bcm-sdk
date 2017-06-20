/*
 * $Id: bm3200_init.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ============================================================
 * == bm3200_init.h - BM3200 Initialization defines          ==
 * ============================================================
 */

#ifndef _BM3200_INIT_H
#define _BM3200_INIT_H

#include "sbTypesGlue.h"

/*
** BM3200 Initialization Structure
*/
typedef struct HW_BM3200_INIT_s {
    sbhandle      userDeviceHandle;       /* sbhandle */
    uint32      reset;                  /* reset BM3200 device if non-zero */
    uint32      bBmRunSelfTest;          /* run BM3200 device selftest if non-zero */

    /* BM3200 Parameters */
    uint32      bmModeLatency_ul;       /* bm3200 only */
    uint32      bmSerializerMask_ul;    /* mask of bm serializers used */
    uint32      bmLocalBmId_ul;         /* BM3200 identifier for failover */
    uint32      bmDefaultBmId_ul;       /* active BM3200 identifier for failover */
    uint32      bmCmode_ul;             /* if set, indicates cmode operation */
    uint32      bmEpochLength_ul;
    uint32      bmTimeslotSizeInNs_ul;
    uint32      bmEnableAutoFailover;   /* enable failover */
    uint32      bmEnableAutoLinkDisable;/* enable auto switchover */
    uint32      bmMaxFailedLinks;       /* max failed links */
    uint32      spMode;

    /* SE4000 Parameters */
    uint64      seSerializerMask_ull;   /* mask of se serializers used */

    /* LCM4000 Parameters */

#define HW_BM3200_PT_NUM_SERIALIZERS           (40)
#define HW_BM3200_MAX_NUM_PLANES                (2)

    uint32      lcmXcfgABInputPolarityReversed_ul; /* indicates that xcfg_switch_a_b_n polarity is reversed from the QE */
    uint64      lcmSerializerMask_ull;  /* mask of lcm serializers used */
    uint32      lcmXcfg_ul[HW_BM3200_MAX_NUM_PLANES][HW_BM3200_PT_NUM_SERIALIZERS];
    uint32      lcmPlaneValid_ul[HW_BM3200_MAX_NUM_PLANES];
    uint32      siLsThreshold_ul;
    uint32      siLsWindow_ul;

} HW_BM3200_INIT_ST;

uint32  hwBm3200Init(HW_BM3200_INIT_ST *hwBm3200Init_sp);

/* status */
#define HW_BM3200_STATUS_OK_K                            (0)
#define HW_BM3200_STATUS_INIT_BM3200_BAD_CHIP_REV_K      (1)
#define HW_BM3200_STATUS_INIT_BIST_TIMEOUT_K             (2)
#define HW_BM3200_STATUS_INIT_BM3200_BIST_BW_UNREPAIR_K  (3)
#define HW_BM3200_STATUS_INIT_BM3200_BIST_BW_TIMEOUT_K   (4)
#define HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K       (5)
#define HW_BM3200_STATUS_INIT_BM3200_SER_TIMEOUT_K       (6)
#define HW_BM3200_STATUS_INIT_BM3200_BW_TIMEOUT_K        (7)
#define HW_BM3200_STATUS_INIT_EPOCH_LENGTH_INVALID_K     (8)
#define HW_BM3200_STATUS_INIT_PARAM_ERROR_K              (9)
#define HW_BM3200_STATUS_INIT_SCHED_ACTIVE_TIMEOUT_K     (10)

/* Fix for bug 23436, resolution was wrong */
#define HW_BM3200_10_USEC_K       (10000)
#define HW_BM3200_100_USEC_K     (100000)
#define HW_BM3200_1_MSEC_K      (1000000)
#define HW_BM3200_10_MSEC_K    (10000000)
#define HW_BM3200_100_MSEC_K  (100000000)
#define HW_BM3200_500_MSEC_K  (500000000)
#define HW_BM3200_1_SEC_K    (1000000000)

/* These defines are the timeouts for acks on all BM3200 init code */
/* these can be updated to increase timeout.                       */
#define HW_BM3200_TIMEOUT_GENERAL           5
#define HW_BM3200_POLL_GENERAL              10000

#define HW_BM3200_PT_NUM_NODES                 (32)
#define HW_BM3200_PT_ID                    (0x0280)
#define HW_BM3200_PT_REV0                    (0x00)
#define HW_BM3200_PT_REV1                    (0x01)

#define HW_BM3200_PT_RANDOM_ARRAY_SIZE         (55)
#define HW_BM3200_CLOCK_SPEED_IN_HZ     (250000000)
#define HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS (760)

#define HW_BM3200_PT_EMAP_MEM_SIZE             (32)

#define HW_BM3200_PT_MAX_CMODE_VIRTUAL_PORTS    (640)
#define HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS    (4096)
#define HW_BM3200_PT_MAX_CMODE_QUEUES   (1024)
#define HW_BM3200_PT_MAX_DMODE_QUEUES   (16384)

#define HW_BM3200_PT_MAX_ESETS (128)

#define HW_BM3200_NULL_CYCLE_COUNT             (4) /* conservative value */

#endif /* _BM3200_INIT_H */
