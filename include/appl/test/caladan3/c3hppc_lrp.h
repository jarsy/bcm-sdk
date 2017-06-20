/*
 * $Id: c3hppc_lrp.h,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * lrp.h : LRP defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_LRP_H_
#define _C3HPPC_LRP_H_

#include <sal/types.h>
#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/appl/sal.h>

#include <soc/types.h>
#include <soc/drv.h>

#include <appl/test/caladan3/c3hppc_utils.h>
#include <appl/test/caladan3/c3hppc_ocm.h>

#define C3HPPC_LRP__CONTINUOUS_EPOCHS   COMPILER_64_INIT(0x00000000,0x00000000)
#define C3HPPC_LRP__MIN_EPOCH           (426 - 2)  /* Jira CA3-2685 */
#define C3HPPC_LRP__STREAM_NUM          12
#define C3HPPC_LRP__MAX_PE_NUM          64
#define C3HPPC_LRP__BARREL_BUDDY_NUM    2
#define C3HPPC_LRP__TASK_NUM            2

#define C3HPPC_LRP_LOOKUP__DM119        (0)
#define C3HPPC_LRP_LOOKUP__DM247        (1)
#define C3HPPC_LRP_LOOKUP__DM366        (2)
#define C3HPPC_LRP_LOOKUP__DM494        (3)

#define C3HPPC_LRP_BUBBLE_UPDATE_MODE__CONTINUOUS        (3)

typedef struct c3hppc_lrp_control_info_s {
  int         nEpochLength;
  int         nNumberOfActivePEs;
  int         nBankSelect;
  char        bBypass;
  char        bDuplex;
  uint8       bLoaderEnable;
  uint8       bMaximizeActiveContexts;
  char        sUcodeFileName[64];
} c3hppc_lrp_control_info_t;

typedef struct _stInstMemEntry {
    uint32   uaInstMemEntry[7];
} stInstMemEntry_t;


typedef union {
  uint64 value;
  struct {
    uint32 Timeout:32;
    uint32 Count:8,
           Reserved:8,
           IntervalIndex:7,
           Task:1,
           Stream:4,
           Init:1,
           JitterEnable:1,
           Mode:2;
  } bits;
} c3hppc_lrp_bubble_state_entry_ut;


int c3hppc_lrp_hw_init( int nUnit, c3hppc_lrp_control_info_t *pC3LrpControlInfo );
int c3hppc_lrp_hw_cleanup( int nUnit );
int c3hppc_lrp_start_control( int nUnit, uint64 uuNumberOfEpochsToRun );
int c3hppc_lrp_write_shared_register( int nUnit, int nRegIndex, uint32 uRegData );
uint32 c3hppc_lrp_read_shared_register( int nUnit, int nRegIndex );
int c3hppc_lrp_load_ucode( int nUnit, char *sUcodeFileName, int nBankSelect,
                           uint8 bLoaderEnable, uint8 bMaximizeActiveContexts, int *nAsmFileEpochLength );
int c3hppc_lrp_bank_corrupt( int nUnit, int nBankSelect );
int c3hppc_lrp_set_results_timer( int nUnit, uint32 uTimer );
int c3hppc_lrp_bank_swap( int nUnit );
int c3hppc_lrp_setup_dm_segment_table( int nUnit, int nSegment, int nDm, uint32 uDmLookUp );
int c3hppc_lrp_setup_tmu_program( int nUnit, int nTableIndex, uint32 uProgram, uint8 bSubKey0Valid, 
                                  uint8 bSubKey1Valid );
int c3hppc_lrp_setup_rce_program( int nUnit, int nTableIndex, uint32 uProgram );
int c3hppc_lrp_setup_etu_program( int nUnit, int nTableIndex, uint32 uProgram );
int c3hppc_lrp_display_error_state( int nUnit );
int c3hppc_lrp_setup_pseudo_traffic_bubbles( int nUnit, uint8 bRandomizeStream, int nStream, int nTask ); 
int c3hppc_lrp_disable_bubbles( int nUnit ); 
int c3hppc_lrp_enable_bubbles( int nUnit ); 
int c3hppc_lrp_setup_host_bubble( int nUnit, int nStream, int nTask, uint32 *uBubbleData ); 
int c3hppc_lrp_setup_ring_wheel( int nUnit, int nSVP, int nLM, int nRingSize, int nEntrySize );
int c3hppc_lrp_setup_host_producer_ring( int nUnit, int nSVP, int nLM, int nRingSize, int nEntrySize );
int c3hppc_lrp_set_host_producer_ring_write_offset( int nUnit, int nSVP, int nLM, uint32 uOffset );
sal_time_t  c3hppc_lrp_get_start_timestamp( void ); 

#endif /* _C3HPPC_LRP_H_ */
