/*
 * $Id: c3hppc_tmu.h,v 1.28 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_tmu.h : COP defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_TMU_H_
#define _C3HPPC_TMU_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <sal/core/thread.h>
#include <sal/appl/sal.h>
#include <soc/shmoo_ddr40.h>
#include <soc/phy/ddr40.h>


#define C3HPPC_TMU_INSTANCE_NUM                        (1)
#define C3HPPC_TMU_REGION_NUM                          (4096) 
#define C3HPPC_TMU_BANK_NUM                            (8) 
#define C3HPPC_TMU_PER_BANK_REGION_BASE_OFFSET_NUM     (128) 
#define C3HPPC_TMU_ROWS_PER_REGION_PER_BANK            (128) 
#define C3HPPC_TMU_TABLE_NUM                           (64) 
#define C3HPPC_TMU_HASH_TABLE_NUM                      (8) 
#define C3HPPC_TMU_HASH_TABLE_ENTRY_NUM                (256) 
#define C3HPPC_TMU_HASH_ADJUST_SELECT_NUM              (8) 
#define C3HPPC_TMU_KEY_SIZE_IN_64b_WORDS               (7) 
#define C3HPPC_TMU_SUBKEY_NUM                          (2) 
#define C3HPPC_TMU_UPDATE_CMD_FIFO_NUM                 (2)
#define C3HPPC_TMU_UPDATE_RSP_FIFO_NUM                 (2)
#define C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM           (4)
#define C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_ENTRY_NUM     (1024)
#if (defined(LINUX))
#define C3HPPC_TMU_UPDATE_FREECHAIN_DMA_ENTRY_NUM      (512)
#else
#define C3HPPC_TMU_UPDATE_FREECHAIN_DMA_ENTRY_NUM      (128)
#endif
#define C3HPPC_TMU_UPDATE_CMD_FIFO_SIZE_IN_64b_WORDS   (1024)
#define C3HPPC_TMU_UPDATE_COMMAND__XL_READ             (1) 
#define C3HPPC_TMU_UPDATE_COMMAND__XL_WRITE            (2) 
#define C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT_BEGIN    (5) 
#define C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT_END      (6) 
#define C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT          C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT_END
#define C3HPPC_TMU_UPDATE_COMMAND__EML_DELETE          (7) 
#define C3HPPC_TMU_UPDATE_COMMAND__TAPS                (8) 
#define C3HPPC_TMU_UPDATE_COMMAND__LOCK                (9) 
#define C3HPPC_TMU_UPDATE_COMMAND__RELEASE             (10) 
#define C3HPPC_TMU_UPDATE_COMMAND__TRAILER             (15) 
#define C3HPPC_TMU_UPDATE_OPERATION_LIMIT_IN_64b_WORDS (16) 
#define C3HPPC_TMU_UPDATE_GLOBAL_LOCK                  (0xffffffff) 
#define C3HPPC_TMU_HOST_RSP__ERRCODE__SUCCESS                (0x0)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_NO_HARDWARE_CHAIN  (0x1)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_NO_FREE_CHAIN      (0x2)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_CHAIN0_FULL        (0x3)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_CHAIN1_FULL        (0x4)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_KEY_NOT_FOUND      (0x5)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_LOCK_FAIL          (0x6)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_FILTER_NO_UPDATE   (0x7)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__EML_NO_BD_MATCH        (0x8)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__RAW_STRADDLES_ROW      (0x10)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__BAD_WRITE_SIZE         (0x11)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__KV_PAIRS_IS_0          (0x12)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__SRAM_ECC_ERROR         (0xfe)
#define C3HPPC_TMU_HOST_RSP__ERRCODE__DRAM_ECC_ERROR         (0xff)


#define C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE             (0) 
#define C3HPPC_TMU_UPDATE_DELETE_OPTIONS__NONE             (0) 
#define C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_NOT_FOUND (1) 
#define C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_FOUND     (2) 

#define C3HPPC_TMU_TAPS_INSTANCE_NUM                   (2) 
#define C3HPPC_TMU_TAPS_OPCODE__WRITE                  (2) 
#define C3HPPC_TMU_TAPS_OPCODE__READ                   (1) 
#define C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_BYTES          (32)
#define C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_64B_WORDS      (C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_BYTES/8)
#define C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_32B_WORDS      (C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_BYTES/4) 
#define C3HPPC_TMU_TAPS_MODE__FULLY_ON_CHIP            (0) 
#define C3HPPC_TMU_TAPS_MODE__SEARCH_ON_CHIP           (1) 
#define C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH           (2) 

#define C3HPPC_TMU_CI_INSTANCE_NUM                     (16)
#define C3HPPC_TMU_QE_INSTANCE_NUM                     (16)
#define C3HPPC_TMU_DRAM_INSTANCE_NUM                   (16)
#define C3HPPC_TMU_DRAM_BANK_PAIR_NUM                  4 

#define C3HPPC_PHY_WORD_LANE_0_RBUS_START__OFFSET                        (0x0200)
#define C3HPPC_PHY_WORD_LANE_1_RBUS_START__OFFSET                        (0x0400)
#define C3HPPC_DDR40_PHY_PLL_CONTROL__OFFSET                             (24)
#define C3HPPC_DDR40_PHY_WORD_LANE_READ_DATA_DLY__OFFSET                 (352)
#define C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET                 (420)
#define C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__DQS_ALWAYS_ON          (0x00000400)
#define C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__VDDO_VOLTS             (0x00000018)
#define C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__RT120B_G               (0x00000002)
#define C3HPPC_DDR40_PHY_PLL_DIVIDERS__OFFSET                            (28)
#define C3HPPC_DDR40_PHY_PLL_DIVIDERS__NDIV                              (0x000000ff)
#define C3HPPC_DDR40_PHY_PLL_DIVIDERS__POST_DIV                          (0x00003800)
#define C3HPPC_DDR40_PHY_PLL_CONFIG__OFFSET                              (20)
#define C3HPPC_DDR40_PHY_PLL_CONFIG__RESET                               (0x00000002)
#define C3HPPC_DDR40_PHY_PLL_STATUS__OFFSET                              (16)
#define C3HPPC_DDR40_PHY_PLL_STATUS__LOCK                                (0x00000001)
#define C3HPPC_DDR40_PHY_PLL_STATUS__LOCK_LOST                           (0x04000000)
#define C3HPPC_DDR40_PHY_VDL_CALIBRATE__OFFSET                           (72)
#define C3HPPC_DDR40_PHY_VDL_CALIBRATE__CALIB_FAST                       (0x00000001)
#define C3HPPC_DDR40_PHY_VDL_CALIBRATE__CALIB_ONCE                       (0x00000002)
#define C3HPPC_DDR40_PHY_VDL_CALIBRATE__CALIB_AUTO                       (0x00000100)

#define C3HPPC_TMU_LOOKUP__1ST_EML64                   (0) 
#define C3HPPC_TMU_LOOKUP__1ST_EML176                  (1) 
#define C3HPPC_TMU_LOOKUP__1ST_EML304                  (2) 
#define C3HPPC_TMU_LOOKUP__1ST_EML424                  (3) 
#define C3HPPC_TMU_LOOKUP__2ND_EML64                   (4) 
#define C3HPPC_TMU_LOOKUP__2ND_EML176                  (5) 
#define C3HPPC_TMU_LOOKUP__2ND_EML304                  (6) 
#define C3HPPC_TMU_LOOKUP__2ND_EML424                  (7) 
#define C3HPPC_TMU_LOOKUP__1ST_EMC64                   (8) 
#define C3HPPC_TMU_LOOKUP__2ND_EMC64                   (9) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV4                   (12) 
#define C3HPPC_TMU_LOOKUP__TAPS_UNIFIED_IPV4           (14) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET            (16) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV4_ASSOC_DATA        (18) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV6                   (13) 
#define C3HPPC_TMU_LOOKUP__TAPS_UNIFIED_IPV6           (15) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV6_BUCKET            (17) 
#define C3HPPC_TMU_LOOKUP__TAPS_IPV6_ASSOC_DATA        (19) 
#define C3HPPC_TMU_LOOKUP__EML_INSERT_DELETE           (28) 
#define C3HPPC_TMU_LOOKUP__DM119                       (32) 
#define C3HPPC_TMU_LOOKUP__DM247                       (33) 
#define C3HPPC_TMU_LOOKUP__DM366                       (34) 
#define C3HPPC_TMU_LOOKUP__DM494                       (35) 
#define C3HPPC_TMU_LOOKUP__DO_NOTHING                  (63) 
#define C3HPPC_TMU_MAX_DM_INTERFACES                   (4) 

#define C3HPPC_TMU_PM_MEMORY_ROW_NUM                   (512) 
#define C3HPPC_TMU_PM_INTF__DM0                        (0)
#define C3HPPC_TMU_PM_INTF__DM1                        (1)
#define C3HPPC_TMU_PM_INTF__DM2                        (2)
#define C3HPPC_TMU_PM_INTF__DM3                        (3)
#define C3HPPC_TMU_PM_INTF__KEY                        (4)

#define C3HPPC_TMU_UPDATE_FREELIST_EMPTY               (0xffffffff) 

#define C3HPPC_TMU_UPDATE_QUEUE_SIZE                   (512) 
#define C3HPPC_TMU_EXPECT_RSP_RING_SIZE                (2048) 

#define C3HPPC_TMU_REGION_LAYOUT__SEQUENTIAL           (0)
#define C3HPPC_TMU_REGION_LAYOUT__DRAM_ROUND_ROBIN     (1)
#define C3HPPC_TMU_REGION_LAYOUT__RANDOM               (2)

#define C3HPPC_TMU_DM494_DATA_SIZE_IN_64b                          (8) 

#define C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b                          (2) 
#define C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b                          (2*C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b) 
#define C3HPPC_TMU_ASSOC_DATA_SIZE_IN_BYTES                        (sizeof(uint64) * C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b) 
#define C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM                           (14)
#define C3HPPC_TMU_EML64_KEY_SIZE_IN_64b                           (1)
#define C3HPPC_TMU_EML64_KEY_SIZE_IN_BYTES                         (8)
#define C3HPPC_TMU_EML64_ROOT_CONTROL_WORD_SIZE_IN_64b             (1)
#define C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b              (C3HPPC_TMU_EML64_ROOT_CONTROL_WORD_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_EML64_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b     (C3HPPC_TMU_EML64_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b                (C3HPPC_TMU_EML64_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML176_KEY_SIZE_IN_64b                          (3)
#define C3HPPC_TMU_EML176_KEY_SIZE_IN_BYTES                        (22)
#define C3HPPC_TMU_EML176_ROOT_CONTROL_WORD_SIZE_IN_64b            (1)
#define C3HPPC_TMU_EML176_ROOT_TABLE_ENTRY_SIZE_IN_64b             (C3HPPC_TMU_EML176_ROOT_CONTROL_WORD_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_EML176_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML176_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b    (C3HPPC_TMU_EML176_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML176_INSERT_COMMAND_SIZE_IN_64b               (C3HPPC_TMU_EML176_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML304_KEY_SIZE_IN_64b                          (5)
#define C3HPPC_TMU_EML304_KEY_SIZE_IN_BYTES                        (38)
#define C3HPPC_TMU_EML304_ROOT_CONTROL_WORD_SIZE_IN_64b            (1)
#define C3HPPC_TMU_EML304_ROOT_TABLE_ENTRY_SIZE_IN_64b             (C3HPPC_TMU_EML304_ROOT_CONTROL_WORD_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_EML304_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML304_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b    (C3HPPC_TMU_EML304_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML304_INSERT_COMMAND_SIZE_IN_64b               (C3HPPC_TMU_EML304_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML424_KEY_SIZE_IN_64b                          (7)
#define C3HPPC_TMU_EML424_KEY_SIZE_IN_BYTES                        (53)
#define C3HPPC_TMU_EML424_ROOT_CONTROL_WORD_SIZE_IN_64b            (1)
#define C3HPPC_TMU_EML424_ROOT_TABLE_ENTRY_SIZE_IN_64b             (C3HPPC_TMU_EML424_ROOT_CONTROL_WORD_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_EML424_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML424_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b    (C3HPPC_TMU_EML424_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EML424_INSERT_COMMAND_SIZE_IN_64b               (C3HPPC_TMU_EML424_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_EMC64_KEY_SIZE_IN_64b                           (1)
#define C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b                   (C3HPPC_TMU_EMC64_KEY_SIZE_IN_64b +\
                                                                    C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b)
#define C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM                     (7)
#define C3HPPC_TMU_IPV4_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b        (4)
#define C3HPPC_TMU_IPV4_128b_BUCKET_PREFIX_NUM                     (3)
#define C3HPPC_TMU_IPV4_128b_BUCKET_TABLE_ENTRY_SIZE_IN_64b        (2)
#define C3HPPC_TMU_IPV4_256b_BUCKET_TABLE_ENTRY_SIZE_IN_32b        (2*C3HPPC_TMU_IPV4_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b)
#define C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM                     (5)
#define C3HPPC_TMU_IPV6_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b        (4)
#define C3HPPC_TMU_IPV6_128b_BUCKET_PREFIX_NUM                     (2)
#define C3HPPC_TMU_IPV6_128b_BUCKET_TABLE_ENTRY_SIZE_IN_64b        (2)
#define C3HPPC_TMU_IPV6_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b        (4)
#define C3HPPC_TMU_IPV6_256b_BUCKET_TABLE_ENTRY_SIZE_IN_32b        (2*C3HPPC_TMU_IPV6_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b)
#define C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_64b (4)
#define C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b (2*C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_64b)


/* Have it run a little faster ~(7800ns / 1.666666667ns) */
#define C3HPPC_TMU_7800NS_REFRESH_INTERVAL             (4672)
#define C3HPPC_TMU_REFRESH_WHEEL_INTERVAL              (C3HPPC_TMU_7800NS_REFRESH_INTERVAL / C3HPPC_TMU_CI_INSTANCE_NUM)

typedef struct c3hppc_tmu_control_info_s {
  uint8    bCacheEnable;
  uint8    bBypassScrambler;
  uint8    bBypassHash;
  uint8    bHwEmlChainManagement;
  uint8    bSkipCiDramInit;
  uint8    bSkipCiDramSelfTest;
  uint8    bEMC128Mode;
  uint32   uEmlMaxProvisionedKey;
  uint32   uNumberOfCIs;
  int      nDramFreq;
} c3hppc_tmu_control_info_t;

typedef struct c3hppc_tmu_ci_control_info_s {
  uint8    bSkipDramSelfTest;
  int      nDramFreq;
} c3hppc_tmu_ci_control_info_t;


typedef struct c3hppc_tmu_table_parameters_s {
  uint32    uEntrySizeInBytes;
  uint32    uEntrySizeIn64b;
  uint32    uKeySizeInBytes;
  uint32    uLookup;
  uint32    uEML1KbCommandsPerChainEntry;
  uint32    uEML1KbChainLimit;
  uint32    uEMLchainLimit;
  uint32    uEMLchainElementSizeIn64b;
  uint32    uEMLchainPoolMask;
  uint32    *FreeList;
  uint32    uFreeListSize;
  uint32    uFreeListNextEntry;
  int       nHashAdjustSelect;
  int       nTableSizePowerOf2;
  uint8     bEMLchainTable;
  uint8     bValid;
} c3hppc_tmu_table_parameters_t;

typedef struct c3hppc_tmu_update_command_info_s {
  int     nCommand;
  int     nTapsOperation;
  int     nTable;
  int     nStartingEntryIndex;
  int     nNumberOfEntries;
  int     nOptions;
  uint32  uOffset;
  uint32  *pTableData;
} c3hppc_tmu_update_command_info_t;

typedef struct c3hppc_tmu_update_manager_cb_s {
  int                              nUnit;
  uint8                            bExit;
  uint8                            bPopulateFreeListFifos;
  c3hppc_tmu_update_command_info_t UpdateCmdQ[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM][C3HPPC_TMU_UPDATE_QUEUE_SIZE]; 
  int                              nUpdateCmdQWrPtr[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM]; 
  int                              nUpdateCmdQRdPtr[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM]; 
  int                              nUpdateCmdQCount[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM]; 
  uint64                           ExpectRspRing[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM][C3HPPC_TMU_EXPECT_RSP_RING_SIZE]; 
  int                              nExpectRspRingWrPtr[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM]; 
  int                              nExpectRspRingRdPtr[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM]; 
  int                              nExpectRspRingCount[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM]; 
  int                              nErrorCounter; 
  uint64                           *pEmlRootTableDumpBuffer; 
  int                              nEmlRootTableDumpBufferEntryCount; 
  int                              nEmlRootTableDumpBufferEntryNum; 
  uint64                           *pEmlChainTableDumpBuffer; 
  int                              nEmlChainTableDumpBufferEntryCount; 
  int                              nEmlChainTableDumpBufferEntryNum; 
  c3hppc_tmu_table_parameters_t    aTableParameters[C3HPPC_TMU_TABLE_NUM];
} c3hppc_tmu_update_manager_cb_t;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           X:3,
           Offset:7,
           Lookup:6,
           Table:6;
    uint32 Size:5,
           EntryNum:27;
  } bits;
} c3hppc_tmu_updater_xl_write_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           X1:10,
           Lookup:6,
           Table:6;
    uint32 KVpairs:4,
           X0:1,
           EntryNum:27;
  } bits;
} c3hppc_tmu_updater_xl_read_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           Echo:1,
           Reserved1:21;
    uint32 Reserved0:32;
  } bits;
} c3hppc_tmu_updater_nop_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           Spacer2:16,
           Table:6;
    uint32 Spacer1:15,
           Filter:1,
           Spacer0:14,
           Size:2;
  } bits;
} c3hppc_tmu_updater_eml_insert_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           Spacer2:16,
           Table:6;
    uint32 Spacer1:15,
           BulkDelete:1,
           Spacer0:14,
           Size:2;
  } bits;
} c3hppc_tmu_updater_eml_delete_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           Spacer2:13,
           Global:1,
           Spacer1:2,
           Table:6;
    uint32 Spacer0:5,
           EntryNum:27;
  } bits;
} c3hppc_tmu_updater_lock_release_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           Reserved1:22;
    uint32 Reserved0:32;
  } bits;
} c3hppc_tmu_updater_taps_command_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Op:4,
           SeqNum:6,
           spacer1:14,
           ErrCode_01:8;
    uint32 spacer0:24,
           Size_Val01:8;
  } bits;
} c3hppc_tmu_updater_response_ut;

typedef union {
  uint64 value;
  struct {
    uint32 NextTable:5,
           NextEntry:24,
           NL_LE_bits3to1:3;
    uint32 NL_LE_bit0:1,
           NL_GT:4,
           Splitter:27;
  } bits;
} c3hppc_tmu_eml_table_control_word_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer:5,
           Row:16,
           Column:8,
           Bank:3;
  } bits;
} c3hppc_tmu_ci_mem_acc_addr_ut;




typedef union {
  uint32 value;
  struct {
    uint32 TapsI:2,
           Block:2,
           Opcode:4,
           Target:4,
           spacer1:1,
           Segment:3,
           spacer0:4,
           Offset:12;
  } bits;
} c3hppc_tmu_taps_rpb_ccmd_word0_ut;

typedef union {
  uint32 value;
  struct {
    uint32 BPMlength:8,
           Kshift:8,
           spacer:4,
           Bucket:12;
  } bits;
} c3hppc_tmu_taps_rpb_ccmd_word1_ut;

typedef union {
  uint32 value;
  struct {
    uint32 BestMatch:32;
  } bits;
} c3hppc_tmu_taps_rpb_ccmd_word2_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer1:3,
           A:1,
           spacer0:3,
           G:1,
           Plength:8,
           Pdata143_128:16;
  } bits;
} c3hppc_tmu_taps_rpb_ccmd_word3_ut;

typedef union {
  uint32 value;
  struct {
    uint32 Pdata127_96:32;
  } bits;
} c3hppc_tmu_taps_ccmd_word4_ut;

typedef union {
  uint32 value;
  struct {
    uint32 Pdata95_64:32;
  } bits;
} c3hppc_tmu_taps_ccmd_word5_ut;

typedef union {
  uint32 value;
  struct {
    uint32 Pdata63_32:32;
  } bits;
} c3hppc_tmu_taps_ccmd_word6_ut;

typedef union {
  uint32 value;
  struct {
    uint32 Pdata31_0:32;
  } bits;
} c3hppc_tmu_taps_ccmd_word7_ut;


/*
 * The following word order results in:
 *
 * struct -->    7,6     5,4      3,2       1,0 
 * sbus   -->    6,7     4,5      2,3       0,1
 * ccmd   -->  [63:0] [127:64] [191:128] [255:192]
 *
 *-----------------------------------------------------------------------------*/
typedef struct c3hppc_tmu_taps_rpb_ccmd_s {
  c3hppc_tmu_taps_ccmd_word7_ut      Word7;
  c3hppc_tmu_taps_ccmd_word6_ut      Word6;
  c3hppc_tmu_taps_ccmd_word5_ut      Word5;
  c3hppc_tmu_taps_ccmd_word4_ut      Word4;
  c3hppc_tmu_taps_rpb_ccmd_word3_ut  Word3;
  c3hppc_tmu_taps_rpb_ccmd_word2_ut  Word2;
  c3hppc_tmu_taps_rpb_ccmd_word1_ut  Word1;
  c3hppc_tmu_taps_rpb_ccmd_word0_ut  Word0;
} c3hppc_tmu_taps_rpb_ccmd_t;



typedef union {
  uint32 value;
  struct {
    uint32 TapsI:2,
           Block:2,
           Opcode:4,
           Format:4,
           spacer1:1,
           Segment:3,
           spacer0:4,
           Offset:12;
  } bits;
} c3hppc_tmu_taps_bbx_ccmd_word0_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer1:8,
           Kshift:8,
           spacer0:7,
           Pnumber:9;
  } bits;
} c3hppc_tmu_taps_bbx_ccmd_word1_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer:32;
  } bits;
} c3hppc_tmu_taps_bbx_ccmd_word2_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer1:3,
           A:1,
           spacer0:3,
           G:1,
           Plength:8,
           Pdata143_128:16;
  } bits;
} c3hppc_tmu_taps_bbx_ccmd_word3_ut;

typedef struct c3hppc_tmu_taps_bbx_ccmd_s {
  c3hppc_tmu_taps_ccmd_word7_ut      Word7;
  c3hppc_tmu_taps_ccmd_word6_ut      Word6;
  c3hppc_tmu_taps_ccmd_word5_ut      Word5;
  c3hppc_tmu_taps_ccmd_word4_ut      Word4;
  c3hppc_tmu_taps_rpb_ccmd_word3_ut  Word3;
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word2;
  c3hppc_tmu_taps_bbx_ccmd_word1_ut  Word1;
  c3hppc_tmu_taps_bbx_ccmd_word0_ut  Word0;
} c3hppc_tmu_taps_bbx_ccmd_t;




typedef union {
  uint32 value;
  struct {
    uint32 spacer0:23,
           Pnumber:9;
  } bits;
} c3hppc_tmu_taps_brr_ccmd_word1_ut;

typedef union {
  uint32 value;
  struct {
    uint32 spacer0:15,
           Adata:17;
  } bits;
} c3hppc_tmu_taps_brr_ccmd_word3_ut;


typedef struct c3hppc_tmu_taps_brr_ccmd_s {
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word7;
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word6;
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word5;
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word4;
  c3hppc_tmu_taps_brr_ccmd_word3_ut  Word3;
  c3hppc_tmu_taps_bbx_ccmd_word2_ut  Word2;
  c3hppc_tmu_taps_brr_ccmd_word1_ut  Word1;
  c3hppc_tmu_taps_bbx_ccmd_word0_ut  Word0;
} c3hppc_tmu_taps_brr_ccmd_t;



typedef union {
  uint64 value;
  struct {
    uint32 spacer:16,
           HashedPrefix_bits39to24:16;
    uint32 HashedPrefix_bits23to0:24,
           Length:7,
           Valid:1;
  } bits;
} c3hppc_tmu_psig_ut;

typedef uint64 (c3hppc_tmu_key_t)[C3HPPC_TMU_KEY_SIZE_IN_64b_WORDS];





int c3hppc_tmu_hw_init( int nUnit, c3hppc_tmu_control_info_t *pC3TmuControlInfo );
int c3hppc_tmu_init_scrambler_table( int nUnit, int nScrambleTableMemory ); 
int c3hppc_tmu_ci_hw_init( int nUnit, c3hppc_tmu_ci_control_info_t *pC3TmuCiControlInfo );
int c3hppc_tmu_ci_memory_init( int nUnit, c3hppc_tmu_ci_control_info_t *pC3TmuCiControlInfo );
int c3hppc_tmu_is_ci_memory_init_done( int nUnit );
int c3hppc_tmu_ci_read_write( int nUnit, int nCiInstance, int nBurstSizeInBytes, 
                              uint8 bWrite, uint32 uAddress, uint32 *puEntryData );
int c3hppc_tmu_ci_phy_read_write( int nUnit, int nCiInstance, uint8 bWrite, uint32 uAddress, uint32 *puEntryData );
int c3hppc_tmu_ci_deassert_ci_reset( int nUnit );
int c3hppc_tmu_ci_deassert_phy_reset( int nUnit );
int c3hppc_tmu_ci_deassert_ddr_reset( int nUnit );
int c3hppc_tmu_ci_poll_phy_pwrup_rsb( int nUnit );
int c3hppc_tmu_ci_do_shmoo( int nUnit, int nCiInstance );
uint32 c3hppc_tmu_ci_phy_set_register_field( uint32 uRegisterValue, uint32 uFieldMask, uint32 uFieldValue );
int c3hppc_tmu_ci_clear_alarms( int nUnit );
int c3hppc_tmu_region_map_setup( int nUnit, int nRegionLayout );
int __c3hppc_tmu_region_map_setup( int nUnit, int nRegionLayout );
int c3hppc_tmu_table_setup( int nUnit, int nTableIndex, uint32 uLookup, uint32 uNumEntries, 
                            uint32 uReplicationFactor, uint32 uRegionOffset, uint32 uRowOffset,
                            uint32 uColumnOffset, uint32 uNumEntriesPerRow,
                            uint32 uDeadlineOffset, uint32 uNextTable, uint32 uUpChainHw,
                            uint32 uUpChainSplitMode, uint32 uUpChainLimit_BucketPrefixNum, uint32 uUpChainPool,
                            uint32 uEmDefault, int nHashAdjustSelect, uint32 *puNewRegionRowOffset );
int c3hppc_tmu_get_hash_adjust_select( int nTable );
int c3hppc_tmu_get_table_size( int nTable );
int c3hppc_tmu_get_number_of_regions_per_bank( void );
int c3hppc_tmu_get_number_of_rows_per_region_in_a_bank( void );
uint32 c3hppc_tmu_get_table_entry_size_in_64b( int nTable );
int c3hppc_tmu_keyploder_setup( int nUnit, int nSubKeyInstance, int nProgram, uint32 uLookup, uint32 uTable, 
                                uint32 uTapsSegment, uint32 uShiftS0, uint32 uMaskS0, uint32 uShiftS1,
                                uint32 uMaskS1, uint32 uShiftLeftS0 );

int c3hppc_tmu_taps_segment_setup( int nUnit, int nInstance, int nSegment, int nKeySize, int nRootPivotNum,
                                   uint32 uBase, int nBBXbucketPrefixSize, int nPrefixesPerBucket,
                                   uint8 bUnified, uint32 uMode );
uint32 c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( int nNumberOfBucketPrefixes );
uint32 c3hppc_tmu_calc_ipv6_bucket_table_entry_size_in_64b( int nNumberOfBucketPrefixes );

int c3hppc_tmu_xl_write( int nCmdFifoSelect, int nTable, int nStartingEntryIndex,
                         int nNumberOfEntries, uint32 uOffset, uint32 *pTableData );
int c3hppc_tmu_xl_read( int nCmdFifoSelect, int nTable, int nStartingEntryIndex,
                        int nNumberOfEntries, uint32 uOffset, uint32 *pTableData );
int c3hppc_tmu_taps_write( int nCmdFifoSelect, int nCommandNum, uint32 *pCommandData );
int c3hppc_tmu_eml_insert( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pInsertData, 
                           int nInsertOptions );
uint32 c3hppc_tmu_get_insert_delete_cmd_size_field( uint32 uLookup );
int c3hppc_tmu_lock( int nCmdFifoSelect, int nTable, uint32 uEntryNumber ); 
int c3hppc_tmu_release( int nCmdFifoSelect, int nTable ); 
int c3hppc_tmu_eml_verify_insert( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pInsertData ); 
int c3hppc_tmu_eml_delete( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pKeyData,
                           int nDeleteOptions );
int c3hppc_tmu_eml_verify_delete( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pKeyData );
int c3hppc_tmu_bulk_delete_setup( int nUnit, int nRootTable, uint32 *pKeyFilter, uint32 *pKeyFilterMask );
int c3hppc_tmu_bulk_delete_start_scanner( int nUnit );
int c3hppc_tmu_bulk_delete_cancel( int nUnit );
int c3hppc_tmu_wait_for_bulk_delete_done( int nUnit, int nTimeOutInSeconds );

int c3hppc_tmu_pm_filter_setup( int nUnit, int nInstance, uint32 uInterface, uint8 bSubKey0, uint8 bOddTags, 
                                uint32 uProgram ); 
int c3hppc_tmu_pm_activate( int nUnit, uint32 uTimeStampShift, uint32 uBucketShift, uint32 uBucketOffset );
int c3hppc_tmu_pm_dump_memory( int nUnit, int nInstance ); 
uint64 c3hppc_tmu_pm_get_count( int nIndex ); 
uint64 c3hppc_tmu_psig_hash_calc( uint32 *puPrefixData, uint32 uHashAdjust );

void c3hppc_tmu_update_cmd_manager(void *pUpdateManagerCB_arg);
void c3hppc_tmu_update_rsp_manager(void *pUpdateManagerCB_arg);
void c3hppc_tmu_update_freelist_manager(void *pUpdateManagerCB_arg);
int c3hppc_tmu_exit_update_manager_thread( void );
int c3hppc_tmu_are_rsp_fifos_empty( void );
int c3hppc_tmu_get_expect_rsp_ring_count( int nWrPtr, int nRdPtr );
int c3hppc_tmu_are_cmd_fifos_empty(void );
int c3hppc_tmu_is_cmd_fifo_empty( int nCmdFifoSelect );
int c3hppc_tmu_rsp_fifo_count( int nRspFifoSelect );
int c3hppc_tmu_cmd_fifo_count( int nCmdFifoSelect );
int c3hppc_tmu_get_rsp_fifos_error_count( void );
int c3hppc_tmu_hw_cleanup( int nUnit );
int c3hppc_tmu_display_error_state( int nUnit );
int c3hppc_tmu_are_free_chain_fifos_empty( int nUnit );
uint32 c3hppc_tmu_get_eml_root_table_entry_size_in_64b( uint32 uLookup );
uint32 c3hppc_tmu_get_eml_chain_table_chain_element_entry_size_in_64b( uint32 uLookup );
uint32 c3hppc_tmu_get_eml_insert_cmd_entry_size_in_64b( uint32 uLookup );
uint32 c3hppc_tmu_get_eml_key_size_in_bytes( uint32 uLookup );
uint32 c3hppc_tmu_get_eml_key_size_in_64b( uint32 uLookup );
uint64 c3hppc_tmu_get_root_table_dump_buffer_entry( int nEntryIndex );
int c3hppc_tmu_get_root_table_dump_buffer_count( void );
uint64 c3hppc_tmu_get_chain_table_dump_buffer_entry( int nEntryIndex );
int c3hppc_tmu_get_chain_table_dump_buffer_count( void );
int c3hppc_tmu_get_eml_tables( int nUnit, int nCmdFifoSelect, int nRootTable, int nChainTable,
                               int nRootTableNumOfEntries, int nTimeOut );
int c3hppc_tmu_display_eml_tables( char *pFileName, int nRootTable, int nChainTable );
/*
int c3hppc_tmu_scoreboard_eml_tables( int nMaxKey );
*/
int c3hppc_tmu_display_andor_scoreboard_eml_tables( int nUnit, uint8 bDisplay, char *pFileName, int nRootTable,
                                                    int nChainTable, uint8 bScoreboard, int nMaxKey );

uint32 c3hppc_tmu_1stLookup_hash( c3hppc_tmu_key_t auuKey, int nHashAdjustSelect, int nTableSizePowerOf2 );
uint32 c3hppc_tmu_2ndEmcLookup_hash( c3hppc_tmu_key_t auuKey, int nTableSizePowerOf2 );
uint32 c3hppc_tmu_crc32_ieee802( uint32 uDataIn );
uint32 c3hppc_tmu_crc32_qe( uint32 uDataIn );
uint64 c3hppc_tmu_crc64( uint64 uuDataIn );
uint32 c3hppc_tmu_get_free_chain_fifos_full_space( int nUnit );
uint32 c3hppc_tmu_are_buffers_available_for_this_free_chain_fifo( uint32 uFreeChainFifo );
uint32 c3hppc_tmu_get_emc128mode( void );
uint64 c3hppc_tmu_get_cache_hit_count( void );
uint64 c3hppc_tmu_get_cache_miss_count( void );
uint64 c3hppc_tmu_get_cache_hit_for_pending_count( void );
uint64 c3hppc_tmu_collect_cache_hit_counts( int nUnit, uint8 bClear );
uint64 c3hppc_tmu_collect_cache_miss_counts( int nUnit, uint8 bClear );
uint64 c3hppc_tmu_collect_cache_hit_for_pending_counts( int nUnit, uint8 bClear );
int    c3hppc_tmu__dump_cmic_rd_dma_state( int nUnit );
int    c3hppc_tmu_enable_ipv6_3rd_probe( int nUnit );
int    c3hppc_tmu_enable_eml_144_mode( int nUnit );


#endif /* _C3HPPC_TMU_H_ */
