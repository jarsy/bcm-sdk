/*
 * $Id: c3hppc_cop.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_cop.h : COP defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_COP_H_
#define _C3HPPC_COP_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <appl/test/caladan3/c3hppc_ocm.h>
#include <soc/types.h>
#include <soc/drv.h>


#define C3HPPC_COP_INSTANCE_NUM                       (2)
#define C3HPPC_COP_SEGMENT_NUM                        (32) 
#define C3HPPC_COP_PROFILE_NUM                        (1024) 
#define C3HPPC_COP_METER_MONITOR_GROUP_NUM            (8) 
#define C3HPPC_COP_METER_MONITOR_COUNTERS_PER_GROUP   (16) 
#define C3HPPC_COP_METER_MONITOR_COUNTERS_NUM         (C3HPPC_COP_METER_MONITOR_GROUP_NUM*\
                                                       C3HPPC_COP_METER_MONITOR_COUNTERS_PER_GROUP) 
#define C3HPPC_COP_SEGMENT_TYPE__METER                (0) 
#define C3HPPC_COP_SEGMENT_TYPE__TIMER                (1) 
#define C3HPPC_COP_SEGMENT_TYPE__CHECKER              (2) 
#define C3HPPC_COP_SEGMENT_TYPE__COHERENT_TABLE       (3)
#define C3HPPC_COP_MAX_WATCHDOG_TIMERS                (0x100000)

typedef struct c3hppc_cop_watchdogtimer_manager_cb_s {
  int               nUnit;
  int               nErrorCounter;
  int               nNumberOfTimers;
  int               nLrpSVP;
  int               nLrpLM;
  uint32            uWDT_ListWriteOffset;
  uint32            uWDT_ListOcmBase;
  uint32            uWDT_ListEntryNum;
  int               anExpiredTimerCount[C3HPPC_COP_INSTANCE_NUM];
  uint8             bExit;
  uint8             bScoreBoardMode;
} c3hppc_cop_watchdogtimer_manager_cb_t;

typedef union {
  uint64 value;
  struct {
    uint32 Valid:1,
           CopInstance:1,
           Unused0:30;
    uint32 Unused1:1,
           Restart:1,
           Unused2:9,
           Offset:21;
  } bits;
} c3hppc_cop_watchdogtimer_host2lrp_control_word_ut;


typedef union {
  uint32 value;
  struct {
    uint32 Unused:3,
           EccError:1,
           Forced:1,
           Active:1,
           Segment:5,
           Offset:21;
  } bits;
} c3hppc_cop_watchdogtimer_ring_entry_ut;


typedef struct c3hppc_cop_segment_info_s {
  uint32   uProfile;
  uint32   uSegmentBase;
  uint32   uSegmentLimit;
  uint32   uSegmentOcmLimit;
  uint32   uSegmentType;
  uint32   uSegmentTransferSize;
  uint32   uRefreshVisitInterval;
  uint32   uMode64;
  uint64   uuRefreshVisitPeriod;
  int      nStartingPhysicalBlock;
  char     bValid;
} c3hppc_cop_segment_info_t;

typedef struct c3hppc_cop_profile_info_s {
  uint32   uCIRinKbps;
  uint32   uEIRinKbps;
  uint32   uCBSinBytes;
  uint32   uEBSinBytes;
  uint32   uPktLengthAdjust;
  uint32   bBlind;
  uint32   bDropOnRed;
  uint32   bCPflag;
  uint32   bRFC2698;
  uint32   bBKTCstrict;
  uint32   bBKTEstrict;
  uint32   bBKTCnodec;
  uint32   bBKTEnodec;
  uint32   bPktMode;
  char     bValid;
} c3hppc_cop_profile_info_t;

typedef struct c3hppc_cop_control_info_s {
  c3hppc_cop_segment_info_t    *pCopSegmentInfo;
  c3hppc_cop_profile_info_t    *pCopProfileInfo;
  uint8                        bWDT_ScoreBoardMode;
} c3hppc_cop_control_info_t;


typedef union {
  uint64 value;
  struct {
    uint32 Unused:2,
           Timeout:30;
    uint32 Started:1,
           Interrupt:1,
           Timer:30;
  } bits;
} c3hppc_cop_watchdogtimer_state64b_entry_ut;

typedef union {
  uint64 value;
  struct {
    uint32 Started_1:1,
           Interrupt_1:1,
           Timer_1:30;
    uint32 Started_0:1,
           Interrupt_0:1,
           Timer_0:30;
  } bits;
} c3hppc_cop_watchdogtimer_state32b_entry_ut;



int c3hppc_cop_program_segment_table( int nUnit, int nCopInstance, int nSegment,
                                      c3hppc_cop_segment_info_t *pCopSegmentInfo );
int c3hppc_cop_program_segment_enable( int nUnit, int nCopInstance, int nSegment ); 
int c3hppc_cop_program_profile_table( int nUnit, int nCopInstance, int nProfile,
                                      c3hppc_cop_profile_info_t *pCopProfileInfo );
int c3hppc_cop_meter_monitor_dump_memory( int nUnit, int nInstance );
uint64 c3hppc_cop_meter_monitor_get_packet_count( int nIndex, int nInstance );
uint64 c3hppc_cop_meter_monitor_get_byte_count( int nIndex, int nInstance );
int c3hppc_cop_meter_monitor_setup( int nUnit, int nCopInstance, int nGroup, uint32 uSegment, uint32 uMeterID );

void c3hppc_cop_watchdogtimer_ring_manager(void *pWatchDogTimerRingManagerCB_arg);
int c3hppc_cop_exit_watchdogtimer_ring_manager_thread( void );
int c3hppc_cop_get_watchdogtimer_ring_manager_error_count( void );
void c3hppc_cop_set_watchdogtimer_ring_manager_timer_num( int nNumberOfTimers );
void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_svp( int nSVP );
void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_lm( int nLM );
void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_size( int nEntryNum );
void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_base( uint32 uOcmBase );
int c3hppc_cop_get_watchdogtimer_ring_manager_timer_count( int nCopInstance );


int c3hppc_cop_hw_init( int nUnit, int nCopInstance, c3hppc_cop_control_info_t *pC3CopControlInfo );
int c3hppc_cop_profile_config( int nUnit, int nCopInstance, c3hppc_cop_profile_info_t *pCopProfileInfo );
int c3hppc_cop_segments_config( int nUnit, int nCopInstance, c3hppc_cop_segment_info_t *pCopSegmentInfo );
int c3hppc_cop_segments_enable( int nUnit, int nCopInstance, c3hppc_cop_segment_info_t *pCopSegmentInfo );
int c3hppc_cop_coherent_table_read_write( int nUnit, int nCopInstance, int nSegment, 
                                          uint32 uOffset, uint8 bWrite, uint32 *puEntryData );
int c3hppc_cop_hw_cleanup( int nUnit );
int c3hppc_cop_display_error_state( int nUnit );
void c3hppc_cop_calc_bs_e_and_m( uint32 uBurstSizeInBytes, uint32 *uE, uint32 *uM );
void c3hppc_cop_calc_ir_e_and_m( uint32 uInformationRateInKbps, uint32 *uE, uint32 *uM );
void c3hppc_cop_format_meterstate_in_ocm_entry( c3hppc_64b_ocm_entry_template_t *pOcmEntry,
                                                uint32 uProfile, uint32 uBktE, uint32 uBktC );


#endif /* _C3HPPC_COP_H_ */
