/*
 * $Id: bme3200.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BME3200 (BCM83332) SOC Initialization
 */

#ifndef _BME3200_H
#define _BME3200_H

#include <soc/sbx/sbx_drv.h>
#include <bcm/types.h>
#include <bcm/port.h>


#define SB_FAB_DEVICE_BM3200_MAX_WRED_TEMPLATES          (256)
#define SB_FAB_DEVICE_BM3200_MAX_DS_IDS                  (128)
#define SB_FAB_DEVICE_BM3200_MAX_NODES                   (32)

#define SB_FAB_DEVICE_BM3200_MAX_TS                      (9) /* in micro seconds */

typedef enum soc_lcm_mode_e {
    lcmModeNormal,
    lcmModeConfigA,
    lcmModeConfigB,
    lcmModeHwSelect
} soc_lcm_mode_t;


/* New soc interfaces */

/* bw repository 0 only */
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT   0
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLOP  1
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_LTHR  2
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NPC2Q 3
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_Q2NPC 4
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP   5
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_R0    6

/* bw repository 0 and 1 */
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT   7
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_DST   8
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT   9

/* bw repository 1 only */
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WST  10
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT  11
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLT  12
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_R1   13

#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NUM_REPOSITORIES 14
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_MAX_REPOSITORY_SIZE 0x8000
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NUM_BANKS   2

/* Repository sizes. */
/* CMode */
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_DST_CMODE_SIZE  0x4000
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT_CMODE_SIZE   0x400
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT_CMODE_SIZE   0x280
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP_CMODE_SIZE   0x400
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_Q2NPC_CMODE_SIZE 0x200
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT_CMODE_SIZE   0x200
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLOP_CMODE_SIZE  0x280
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_LTHR_CMODE_SIZE  0x200
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NPC2Q_CMODE_SIZE 0x400

#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT_CMODE_SIZE   0x800
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WST_CMODE_SIZE   0x400
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLT_CMODE_SIZE   0x400

#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP_DMODE_SIZE   0x4000
#define SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT_DMODE_SIZE   0x1000


#define SB_FAB_DEVICE_BM3200_EPOCH_PRIME_PIPE                 6
#define SB_FAB_DEVICE_BM3200_EPOCH_WRED_KILL                  8
#define SB_FAB_DEVICE_BM3200_EPOCH_ALLOC_PRIME_PIPE           32

#define SB_FAB_DEVICE_BM3200_NUM_AI_PORTS                     32

extern int
soc_bm3200_init(int unit, soc_sbx_config_t *cfg);

extern void
soc_bm3200_fo_error(int unit, uint32 ignored);

extern void
soc_bm3200_bw_error(int unit, uint32 ignored);

extern void
soc_bm3200_xb_error(int unit, uint32 ignored);

extern void
soc_bm3200_ac_error(int unit, uint32 ignored);

extern void
soc_bm3200_isr(void *_unit);

extern soc_error_t
soc_bm3200_config_linkdriver(int unit, int nLink, sbLinkDriverConfig_t *pLinkDriverConfig);

extern int
soc_bm3200_eset_block_set(int unit, uint32 baseEset, uint32 nodesMasks[4]);

extern int
soc_bm3200_eset_block_get(int unit, uint32 baseEset, uint32 nodesMasks[4]);


extern int
soc_bm3200_eset_set(int unit, uint32 eset, uint32 nodesMask, uint32 uMcFullEvalMin);

extern int
soc_bm3200_eset_get(int unit, uint32 eset, uint32 *nodesMask, uint32 *pMcFullEvalMin);

extern int
soc_bm3200_xb_test_pkt_get(int unit, int egress, int xb_port, int *cnt);

extern int
soc_bm3200_xb_test_pkt_clear(int unit, int egress, int xb_port);

extern int
soc_bm3200_lcm_mode_set(int unit, soc_lcm_mode_t mode);

extern int
soc_bm3200_lcm_mode_get(int unit, soc_lcm_mode_t *mode);

extern int
soc_bm3200_lcm_fixed_config(int unit, int configAB,
                            bcm_port_t xcfg[], int num_xcfgs);

extern int
soc_bm3200_bwp_read(int unit, int32 queue, int32 *pgamma, int32 *psigma);

extern int
soc_bm3200_bwp_write(int unit, int32 queue,
                     int32 gamma, int32 sigma);

int
soc_bm3200_prt_read(int unit, int32 bw_group, int32 *psp_queues_in_bag, int32 *pqueues_in_bag,
		    int32 *pbase_queue, int32 *pbag_rate_in_bytes_per_epoch );

int
soc_bm3200_prt_write(int unit, int32 bw_group, int32 sp_queues_in_bag,
                     int32 queues_in_bag, int32 base_queue, int32 bag_rate_in_bytes_per_epoch);

extern int
soc_bm3200_bw_mem_read(int unit, int32 bw_table_id,
                       int32 repository, int32 addr, uint32 *pdata);

extern int
soc_bm3200_bw_mem_write(int unit, int32 bw_table_id,
                        int32 repository, int32 addr, uint32 write_data);

extern int
soc_bm3200_features(int unit, soc_feature_t feature);

extern int
soc_bm3200_port_info_config(int unit);

extern int
soc_bm3200_epoch_in_timeslot_config_get(int unit, int num_queues, uint32 *epoch_in_timeslots);

extern int
soc_bm3200_eset_init(int unit);

extern soc_driver_t soc_driver_bcm83332_a0;
extern soc_driver_t soc_driver_bcm83332_b0;

extern uint32
hwBm3200InaMemoryReadWrite(sbhandle userDeviceHandle,
                           uint32     port,
                           uint32     sel,
                           uint32     addr,
                           uint32*    pData0,
                           uint32*    pData1,
                           uint32*    pData2,
                           uint32*    pData3,
                           uint32*    pData4,
                           uint32    rd_wr_n);

extern uint32
hwBm3200NmEmapReadWrite(sbhandle userDeviceHandle,
                        uint32 addr,
                        uint32* pData0,
                        uint32* pData1,
                        uint32* pData2,
                        uint32* pData3,
                        uint32* pData4,
                        uint32  rd_wr_n);
#endif
