/*
 * $Id: c3hppc_etu.h,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_etu.h : ETU defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_ETU_H_
#define _C3HPPC_ETU_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>

#define C3HPPC_ETU_ESM_NUM                       (6)
#define C3HPPC_ETU_MASTER_ESM                    (1)
#define C3HPPC_ETU_TX_SERDES_REFCLK_SEL          (5)   /* Select any lane(4-7) within the master ESM */
#define C3HPPC_ETU_NL11K_TO_C3_LANE_NUM          (12)
#define C3HPPC_ETU_INTERFACE_BRINGUP_ATTEMPTS    (2)

#define C3HPPC_ETU_TABLE_NUM                     (64)
#define C3HPPC_ETU_UPDATE_QUEUE_SIZE             (16)

#define C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS         (322)
#define C3HPPC_ETU_NL11K_BLOCK_BASE_ADDRESS             (0x1000)
#define C3HPPC_ETU_NL11K_BLOCK_SET_OFFSET               (0x20)
#define C3HPPC_ETU_NL11K_BLOCK_CONFIG_OFFSET            (0x00)
#define C3HPPC_ETU_NL11K_BLOCK_ENTRY_NUM_IN_80b         (0x1000)
#define C3HPPC_ETU_NL11K_NATIVE_CONSTRUCT_WIDTH         (80)

#define C3HPPC_ETU_NL11K_DEVICE_ID_REGISTER_OFFSET      (0x0000)
#define C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTER_OFFSET   (0x0002)

#define C3HPPC_ETU_NL11K_LTR_BASE_ADDRESS               (0x4000)
#define C3HPPC_ETU_NL11K_LTR_SET_OFFSET                 (0x20)
#define C3HPPC_ETU_NL11K_LTR_BLOCK_SELECT0_OFFSET       (0x00)
#define C3HPPC_ETU_NL11K_LTR_BLOCK_SELECT1_OFFSET       (0x01)
#define C3HPPC_ETU_NL11K_LTR_SUPER_BLOCK_SELECT_OFFSET  (0x02)
#define C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH0_OFFSET    (0x03)
#define C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH1_OFFSET    (0x04)
#define C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH2_OFFSET    (0x05)
#define C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH3_OFFSET    (0x06)
#define C3HPPC_ETU_NL11K_LTR_MISC_OFFSET                (0x09)
#define C3HPPC_ETU_NL11K_LTR_KPU0_KEY_CONSTRUCT0_OFFSET (0x0b)
#define C3HPPC_ETU_NL11K_LTR_KPU0_KEY_CONSTRUCT1_OFFSET (0x0c)
#define C3HPPC_ETU_NL11K_LTR_KPU1_KEY_CONSTRUCT0_OFFSET (0x0d)
#define C3HPPC_ETU_NL11K_LTR_KPU1_KEY_CONSTRUCT1_OFFSET (0x0e)
#define C3HPPC_ETU_NL11K_LTR_KPU2_KEY_CONSTRUCT0_OFFSET (0x0f)
#define C3HPPC_ETU_NL11K_LTR_KPU2_KEY_CONSTRUCT1_OFFSET (0x10)
#define C3HPPC_ETU_NL11K_LTR_KPU3_KEY_CONSTRUCT0_OFFSET (0x11)
#define C3HPPC_ETU_NL11K_LTR_KPU3_KEY_CONSTRUCT1_OFFSET (0x12)

#define C3HPPC_ETU_LOOKUP__80                           (0)
#define C3HPPC_ETU_LOOKUP__160                          (1)
#define C3HPPC_ETU_LOOKUP__320                          (2)
#define C3HPPC_ETU_LOOKUP__640                          (3)
#define C3HPPC_ETU_LOOKUP__4x80                         (0x4 | C3HPPC_ETU_LOOKUP__80)

typedef struct c3hppc_etu_control_info_s {
  uint32                       blah;
} c3hppc_etu_control_info_t;


typedef struct c3hppc_etu_cp_fifo_entry_s {
  uint32                       Words[9];
} c3hppc_etu_cp_fifo_entry_t;


typedef struct c3hppc_etu_80b_data_s {
  uint64                       Words[2];
} c3hppc_etu_80b_data_t;

/*
  The following structures are organized with the requirement that the HW will do an endian byte swap 
  from how the command formats are documented in the NL11K specs.  From the regsfile:

  NOT_SEARCH => {
                MAXBIT  => 23, MINBIT => 23, RESETVAL => 0x0,
                DESC    => "Specify the raw access is not a search/lookup request. When 1, ETU do not flip endian-ness
                            of DWs specified in ETU_TX_RAW_REQ_DATA_WORD When 0, means the request is a search/lookup
                            request. ETU flip the endian-ness of DWs specified in ETU_TX_RAW_REQ_DATA_WORD"
*/

typedef union {
  uint32 value;
  struct {
    uint32 WRMODE:1,
           VBIT:1,
           Unused:4,
           AT:1,
           DeviceID:2,
           Reserved:3,
           Address:20;
  } bits;
} c3hppc_etu_nl11k_address_format_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Reserved:6,
           SearchResultNum:2,
           Placeholder1:24;
    uint32 Placeholder0:16,
           Unused3:1,
           BMR_Select3:3,
           Unused2:1,
           BMR_Select2:3,
           Unused1:1,
           BMR_Select1:3,
           Unused0:1,
           BMR_Select0:3;
  } bits;
} c3hppc_etu_nl11k_ltr_misc_format_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Reserved:9,
           NumBytes4:4,
           StartByte4:7,
           NumBytes3:4,
           StartByte3:7,
           NumBytes2_bit3:1;
    uint32 NumBytes2_bits2to0:3,
           StartByte2:7,
           NumBytes1:4,
           StartByte1:7,
           NumBytes0:4,
           StartByte0:7;
  } bits;
} c3hppc_etu_nl11k_ltr_key_construct0_format_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Reserved:9,
           NumBytes9:4,
           StartByte9:7,
           NumBytes8:4,
           StartByte8:7,
           NumBytes7_bit3:1;
    uint32 NumBytes7_bits2to0:3,
           StartByte7:7,
           NumBytes6:4,
           StartByte6:7,
           NumBytes5:4,
           StartByte5:7;
  } bits;
} c3hppc_etu_nl11k_ltr_key_construct1_format_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Reserved1:32;
    uint32 Reserved0:28,
           Width:3,
           Enable:1;
  } bits;
} c3hppc_etu_nl11k_block_config_format_ut;


typedef struct c3hppc_etu_table_parameters_s {
  uint32    uKeySizeIn80bSegments;
  int       nTableSize;
  uint8     bValid;
} c3hppc_etu_table_parameters_t;


typedef struct c3hppc_etu_update_command_info_s {
  int     nCommand;
  int     nTable;
  uint32  uStartingEntryIndex;
  int     nNumberOfEntries;
  int     nOptions;
  c3hppc_etu_80b_data_t  *pKeyData;
  c3hppc_etu_80b_data_t  *pKeyMask;
} c3hppc_etu_update_command_info_t;


typedef struct c3hppc_etu_update_manager_cb_s {
  int                              nUnit;
  uint8                            bExit;
  c3hppc_etu_update_command_info_t UpdateCmdQ[C3HPPC_ETU_UPDATE_QUEUE_SIZE];
  int                              nUpdateCmdQWrPtr;
  int                              nUpdateCmdQRdPtr;
  int                              nUpdateCmdQCount;
  c3hppc_etu_table_parameters_t    aTableParameters[C3HPPC_ETU_TABLE_NUM];
} c3hppc_etu_update_manager_cb_t;



int c3hppc_etu_hw_init( int nUnit, c3hppc_etu_control_info_t *pC3EtuControlInfo );
int c3hppc_etu_hw_cleanup( int nUnit );
int c3hppc_etu_esm_bringup( int nUnit );
int c3hppc_etu_nl11k_bringup( int nUnit );
int c3hppc_etu_interface_bringup( int nUnit );
int c3hppc_etu_prbs31_test( int nUnit );
int c3hppc_etu_wcl_reset_seq( int unit, unsigned master_wcl_num );
int c3hppc_etu_esm_aer_write( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr, uint16 uData );
uint16 c3hppc_etu_esm_aer_read( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr );
int c3hppc_etu_dsc_dump( int nUnit, int nPhyID );
int c3hppc_etu_display_error_state( int nUnit );
int c3hppc_etu_display_nl11k_error_state( int nUnit );
int c3hppc_etu_setup_nop_program( int nUnit, int nProgram );
int c3hppc_etu_setup_search_program( int nUnit, int nProgram, int nLTRsel, int nLayoutSelect );
int c3hppc_etu_read_nl11k_register( int nUnit, uint32 uAddress, uint64 *puuData79_64, uint64 *puuData63_0 );
int c3hppc_etu_write_nl11k_register( int nUnit, uint32 uAddress, uint64 uuData79_64, uint64 uuData63_0 );
int c3hppc_etu_populate_cpfifo_entry_for_database_write( int nUnit, uint32 uBlock, uint32 uRow, uint32 uVBIT,
                                                         uint64 *puuData, uint64 *puuMask,
                                                         uint32 uCapture, c3hppc_etu_cp_fifo_entry_t *pCPfifoEntry );
int c3hppc_etu_tcam_table_layout_setup( int nUnit, int nTable, int nLayoutSelect, int nMaxKeys );
uint32 c3hppc_etu_get_tcam_table_key_size( int nTable );
int c3hppc_etu_exit_update_manager_thread( void );
void c3hppc_etu_update_cmd_manager(void *pUpdateManagerCB_arg);
int c3hppc_etu_key_insert( int nTable, uint32 uStartingEntryIndex, int nNumberOfEntries,
                           c3hppc_etu_80b_data_t *pKeyData, c3hppc_etu_80b_data_t *pKeyMask, int nInsertOptions );
int c3hppc_etu_is_cmd_fifo_empty( void );
int c3hppc_etu_adjust_control_path_latency( int nUnit );


#endif /* _C3HPPC_ETU_H_ */
