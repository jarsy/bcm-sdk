/*
 * $Id: c3hppc_rce.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_rce.h : RCE defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_RCE_H_
#define _C3HPPC_RCE_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>

#define C3HPPC_RCE_INSTRUCTION_NUM                         (12288)
#define C3HPPC_RCE_SBLOCK_NUM                              (24)
#define C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK                  (32)
#define C3HPPC_RCE_TOTAL_COLUMN_NUM                        (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK * C3HPPC_RCE_SBLOCK_NUM)
#define C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK              (8)
#define C3HPPC_RCE_DMA_BLOCK_PATTERN_START_WORD            (64)  
#define C3HPPC_RCE_DMA_BLOCK_SIZE_IN_WORDS                 (8 * 32)  
#define C3HPPC_RCE_NUM_DMA_BLOCKS                          (C3HPPC_RCE_INSTRUCTION_NUM / C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK)  
#define C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK     (32)
#define C3HPPC_RCE_SBLOCK_PATTEN_COLUMN_DEPTH_IN_64B_WORDS (C3HPPC_RCE_INSTRUCTION_NUM / 64)
#define C3HPPC_RCE_RESULTS_COUNTER_NUM                     (16)
#define C3HPPC_RCE_PROGRAM_NUM                             (16)

#define C3HPPC_RCE_RESULT_REGS_USE_ALL                     (4)

typedef struct c3hppc_rce_port_info_s {
  int      nStartingSegment;
  uint32   uSegmentTransferSize;
  uint32   uSegmentBase;
  uint32   uSegmentLimit;
  int      nStartingPhysicalBlock;
  char     bSegmentProtected;
  char     bValid;
} c3hppc_rce_port_info_t;

typedef struct c3hppc_rce_control_info_s {
  c3hppc_rce_port_info_t    *pRcePortInfo;
} c3hppc_rce_control_info_t;

typedef struct c3hppc_rce_sblk_pattern_column_s {
  uint64   uuColumnData[C3HPPC_RCE_SBLOCK_PATTEN_COLUMN_DEPTH_IN_64B_WORDS];
} c3hppc_rce_sblk_pattern_column_t;


typedef union {
  uint64 value;
  struct {
    uint32 null:21, 
           Opcode:3,            /* Bits 42:40 */
           EndF:1,
           POp:2,
           Word:5;
    uint32 Hi:16,
           Lo:16;
  } bits;
} c3hppc_rce_range_instruction_ut;

typedef union {
  uint64 value;
  struct {
    uint32 null:21,
           Opcode:3,            /* Bits 42:40 */
           EndF:1,
           POp:2,
           KeyBitIndex_bits8to4:5;
    uint32 KeyBitIndex_bits3to0:4,
           B2:4,
           B1:4,
           B0:4,
           LUT:16;
  } bits;
} c3hppc_rce_lpm_instruction_ut;

typedef union {
  uint64 value;
  struct {
    uint32 null:21,
           Opcode:3,            /* Bits 42:40 */
           EndF:1,
           Unused1:7;
    uint32 Unused0:32;
  } bits;
} c3hppc_rce_nop_instruction_ut;

typedef union {
  uint64 value;
  struct {
    uint32 null:21,
           Opcode:3,            /* Bits 42:40 */
           Unused1:8;
    uint32 Unused0:32;
  } bits;
} c3hppc_rce_prefix_instruction_ut;

typedef union {
  uint64 value;
  struct {
    uint32 null:21, 
           Opcode:3,            /* Bits 42:40 */
           Link_bits11to4:8;
    uint32 Link_bits3to0:4,
           Res:8,
           Base:20;
  } bits;
} c3hppc_rce_start_instruction_ut;



int c3hppc_rce_hw_init( int nUnit, c3hppc_rce_control_info_t *pC3RceControlInfo );
int c3hppc_rce_hw_cleanup( int nUnit );
int c3hppc_rce_create_program_for_lpm_exact_match( int nUnit, int nProgramNumber, uint32 uProgramBaseAddress,
                                                   uint32 uNumberOfFilterSets, uint32 uFilterLength,
                                                   uint32 uKeyLength, uint32 uKeyStartIndex,
                                                   int nResultRegsSelect );
int c3hppc_rce_add_filter_for_lpm_exact_match( uint32 uFilterLength, uint32 uKeyLength, uint32 uFilterSetIndex,
                                               uint32 uSBlkIndex, uint32 uColumnIndex,
                                               uint32 uProgramBaseAddress, uint64 *puuPatternData );
int c3hppc_rce_move_filter_set( int nUnit, int nProgramNumber, uint32 uFilterLength,
                                uint32 uFilterSetToMove, uint32 uNewLocation );
void c3hppc_rce_set_pattern( uint32 uSBlk, uint32 uColumn, uint32 uFilterSetBitOffset,
                             uint32 uStartBit, uint32 uEndBit, uint64 uuData);
uint32 c3hppc_rce_get_pattern_bit( uint32 uSBlk, uint32 uColumn, uint32 uRow);
void c3hppc_rce_get_dma_block( uint32 uBlockNum, uint32 *puDmaData );
int c3hppc_rce_dma_image( int nUnit );
void c3hppc_rce_dump_pattern_array( void );
int c3hppc_rce_display_error_state( int nUnit );
int c3hppc_rce_conifg_results_counters(int nUnit, int nCounterId, uint32 uRuleLo, uint32 uRuleHi );
int c3hppc_rce_read_results_counters( int nUnit );
uint64 c3hppc_rce_get_results_counter(int nCounterId );
int c3hppc_rce_mem_read_write( int nUnit, uint32 uOffset,
                               uint8 bWrite, uint32 *puEntryData );
int c3hppc_rce_dma_read_write( int nUnit, uint32 uStartOffset, uint32 uEndOffset,
                               uint8 bWrite, uint32 *puDmaData );
#endif /* _C3HPPC_RCE_H_ */
