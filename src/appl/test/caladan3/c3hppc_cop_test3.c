/* $Id: c3hppc_cop_test3.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_COP_TEST3__SEGMENT_NUM  2
#define C3HPPC_COP_TEST3__STREAM_NUM   1

#define C3HPPC_COP_TEST3__TASK_NUM  2


#define C3HPPC_COP_TEST3__BASIC_WDT                             0
#define C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO            2
#define C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO_AND_REARM  3

static int nCopSegment;

int
c3hppc_cop_test3__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  int nOcmPort, nStartingPhysicalBlock1, nStartingPhysicalBlock0;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize, nTimer, nMode32;
  c3hppc_cop_watchdogtimer_state32b_entry_ut WatchDogTimer32bStateEntry, WatchDogTimer32bStateFillerEntry;
#if C3HPPC_COP_WDT_STATE64b_ENTRY
  c3hppc_cop_watchdogtimer_state64b_entry_ut WatchDogTimer64bStateEntry;
#endif
  uint64 uuWatchDogTimerStateEntry, uuTimerValue;

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );


  strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test3.oasm");
  nCopSegment = 1;

  if ( pc3hppcTestInfo->nSetupOptions & C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO ) {
    /* B0 bug:
       The fix for B0 added a 6-bit depth counter which is used to generate a full signal into the COP read-modify-write pipeline,
       indicating that an expiration on the next cycle should result in a transition to the pending state (i.e. no eject).
       Specifically, the depth counter is used to detect the condition when the depth is 63 (i.e. full-1). The bug is that the
       depth counter is decremented by 1 whenever the FIFO is read, EVEN IF it is being written on the same cycle.

       The logic only looks at the "depth equals 63" signal when there was an ejection on the immediately preceding cycle.
       In this case, since the instantaneous depth is 63 and there is another push in progress, the FIFO is effectively full. However,
       if there is no ejection on the immediately preceding cycle, the depth is ignored and the instantaneous full signal is used,
       which pre-existed in A0 and does NOT have a bug in its calculation. 

       Therefore, the goal of a workaround is to eliminate the possibility of the read-modify-write pipeline processing 2 timer
       expirations in consecutive clock cycles. Currently, the only way I can think of to guarantee this is to ensure that every
       OTHER 64-bit timer location (or every OTHER PAIR of 32-bit timers) in the segment is disabled (i.e. unused). Also, there can
       only be 1 timer segment configured to push expiration events into the FIFO. The 2nd condition arises because each segment
       vies independently for entrance into the read-modify-write pipeline. Even if the 1st condition is met, you could have a
       situation where timers from 2 different segments enter the pipeline on back-to-back cycles. You are only safe in no more
       than 1 segment is allowed to push expiration events.
    */
    pc3hppcTestInfo->nCounterNum *= 2;
    pc3hppcTestInfo->BringUpControl.bCopWDT_ScoreBoardMode = (uint8) pc3hppcTestInfo->nSetupOptions; 
    if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO_AND_REARM ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test3_expirefifo.oasm");
    }
  }

  nStartingPhysicalBlock0 = 32;
  nStartingPhysicalBlock1 = 32;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
    pCopSegmentInfo->uMode64 = 0;
    pCopSegmentInfo->bValid = 1;
    pCopSegmentInfo->uSegmentBase = 0;
    pCopSegmentInfo->uSegmentLimit = pc3hppcTestInfo->nCounterNum - 1;
    pCopSegmentInfo->uSegmentOcmLimit = (((1 + pCopSegmentInfo->uMode64) * pc3hppcTestInfo->nCounterNum) / 2) - 1;
    pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__TIMER;
    pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    pCopSegmentInfo->nStartingPhysicalBlock = ( nCopInstance ) ? nStartingPhysicalBlock1++ :
                                                                 nStartingPhysicalBlock0++;
    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod, 0, 1000000);  /* timer_interval = 1ms = 1000000 clock cycles at 1 GHz */
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;

  COMPILER_64_ZERO(WatchDogTimer32bStateEntry.value);
  WatchDogTimer32bStateEntry.bits.Started_1 = 1;
  WatchDogTimer32bStateEntry.bits.Started_0 = 1;
  WatchDogTimer32bStateEntry.bits.Interrupt_1 = 1;
  WatchDogTimer32bStateEntry.bits.Interrupt_0 = 1;
  COMPILER_64_ZERO(WatchDogTimer32bStateFillerEntry.value);
  WatchDogTimer32bStateFillerEntry.bits.Started_1 = 1;
  WatchDogTimer32bStateFillerEntry.bits.Started_0 = 1;
#if C3HPPC_COP_WDT_STATE64b_ENTRY
  WatchDogTimer64bStateEntry.bits.Started = 1;
  WatchDogTimer64bStateEntry.bits.Interrupt = 0;
#endif
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {

    /* Initialize timer state */
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
    nMode32 = ( pCopSegmentInfo->uMode64 ) ? 0 : 1;
    nDmaBlockSize = pc3hppcTestInfo->nCounterNum / (1 + nMode32);
    pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                  nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                  "ocm_block");
    COMPILER_64_SET(uuTimerValue, 0, 5);
    for ( nTimer = 0; nTimer <= (int) pCopSegmentInfo->uSegmentLimit; nTimer += (1+nMode32) ) {
      if ( nMode32 ) {
        WatchDogTimer32bStateEntry.bits.Timer_1 = COMPILER_64_LO(uuTimerValue) * ( SAL_BOOT_QUICKTURN ? 1 : 1000 );
        if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_COP_TEST3__BASIC_WDT ) COMPILER_64_ADD_32(uuTimerValue, 5);
        WatchDogTimer32bStateEntry.bits.Timer_0 = COMPILER_64_LO(uuTimerValue) * ( SAL_BOOT_QUICKTURN ? 1 : 1000 );
        if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_COP_TEST3__BASIC_WDT ) COMPILER_64_ADD_32(uuTimerValue, 5);

        if ( (nTimer & 0x2) && pc3hppcTestInfo->nSetupOptions & C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO )
          /* Every other entry is a filler. */
          COMPILER_64_SET(uuWatchDogTimerStateEntry, COMPILER_64_HI(WatchDogTimer32bStateFillerEntry.value), 
                                                     COMPILER_64_LO(WatchDogTimer32bStateFillerEntry.value) );
        else
          COMPILER_64_SET(uuWatchDogTimerStateEntry, COMPILER_64_HI(WatchDogTimer32bStateEntry.value), 
                                                     COMPILER_64_LO(WatchDogTimer32bStateEntry.value) );

      } else {
        COMPILER_64_ZERO(uuWatchDogTimerStateEntry);
      } 
      pOcmBlock[nTimer>>nMode32].uData[1] = COMPILER_64_HI(uuWatchDogTimerStateEntry);
      pOcmBlock[nTimer>>nMode32].uData[0] = COMPILER_64_LO(uuWatchDogTimerStateEntry);
    } 
    nOcmPort = c3hppc_ocm_map_cop2ocm_port(nCopInstance);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (nDmaBlockSize-1), 1, pOcmBlock->uData );
    soc_cm_sfree(pc3hppcTestInfo->nUnit,pOcmBlock);

  }

  c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                              pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                       pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );

  return 0;
}

int
c3hppc_cop_test3__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nCopInstance;
  int anExpiredTimerCount[C3HPPC_COP_INSTANCE_NUM];
  uint64 uuVal;

  c3hppc_cop_set_watchdogtimer_ring_manager_timer_num( pc3hppcTestInfo->nCounterNum );

  c3hppc_lrp_setup_ring_wheel( pc3hppcTestInfo->nUnit, 0, 0, pc3hppcTestInfo->nCounterNum, 0 );

  /****************************************************************************************************************************
   * Enable COP segments to enable timers.
   *****************************************************************************************************************************/
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nCopInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance] );
  }

  COMPILER_64_SET(uuVal, 0, 0); /* C3HPPC_LRP__CONTINUOUS_EPOCHS */
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuVal );

  COMPILER_64_SET(pc3hppcTestInfo->uuIterations, 0, 
                  (pc3hppcTestInfo->nSetupOptions & C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO) ? 10 : pc3hppcTestInfo->nCounterNum);
  COMPILER_64_ADD_32(pc3hppcTestInfo->uuIterations, 1);
  while ( !COMPILER_64_IS_ZERO(pc3hppcTestInfo->uuIterations) ) {
    sal_sleep( pc3hppcTestInfo->nSetupOptions & C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO ? 1 : 5 );
    for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
      anExpiredTimerCount[nCopInstance] = c3hppc_cop_get_watchdogtimer_ring_manager_timer_count( nCopInstance );
      cli_out(" COP%d current expiration counter --> %d \n", nCopInstance, anExpiredTimerCount[nCopInstance] );
/*
      c3hppcUtils_display_register_notzero( pc3hppcTestInfo->nUnit, SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nCopInstance),
                                            CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUSr );
*/
    }
    if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_COP_TEST3__BASIC_WDT ) {
      if ( (COMPILER_64_HI(pc3hppcTestInfo->uuIterations) == 0) && (COMPILER_64_LO(pc3hppcTestInfo->uuIterations) == 1)  ) {
        for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
          if ( anExpiredTimerCount[nCopInstance] != pc3hppcTestInfo->nCounterNum ) {
            cli_out("\nERROR:  COP%d expiration counter [%d] does not match the expected final value of [%d] ! \n", nCopInstance,
                    anExpiredTimerCount[nCopInstance], pc3hppcTestInfo->nCounterNum );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
        }
      } else {
        for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
          if ( anExpiredTimerCount[nCopInstance] == pc3hppcTestInfo->nCounterNum ) {
            cli_out("\nERROR:  COP%d expiration counter [%d] reached the expected final value [%d] too soon! \n", nCopInstance,
                    anExpiredTimerCount[nCopInstance], pc3hppcTestInfo->nCounterNum );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
        }
      }
    }
    
    COMPILER_64_SUB_32(pc3hppcTestInfo->uuIterations,1);
  }

  if ( pc3hppcTestInfo->nSetupOptions != C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO_AND_REARM ) {
    sal_sleep( SAL_BOOT_QUICKTURN ? 10 : 1 );
    if ( pc3hppcTestInfo->nSetupOptions & C3HPPC_COP_TEST3__PUMMEL_WDT_EXPIRATION_FIFO ) pc3hppcTestInfo->nCounterNum /= 2;
    for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
      anExpiredTimerCount[nCopInstance] = c3hppc_cop_get_watchdogtimer_ring_manager_timer_count( nCopInstance );
      if ( anExpiredTimerCount[nCopInstance] != pc3hppcTestInfo->nCounterNum ) {
        cli_out("\nERROR:  COP%d expiration counter [%d] does not match the expected final value of [%d] ! \n", nCopInstance,
                anExpiredTimerCount[nCopInstance], pc3hppcTestInfo->nCounterNum );
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }
  }

  return 0;
}

int
c3hppc_cop_test3__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
