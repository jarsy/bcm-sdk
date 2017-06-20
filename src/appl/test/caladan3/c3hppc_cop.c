/*
 * $Id: c3hppc_cop.c,v 1.21 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_cop.c
 * Purpose: Caladan3 COP test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_cop.h>
#include <appl/test/caladan3/c3hppc_ocm.h>
#include <appl/test/caladan3/c3hppc_lrp.h>

static int g_anErrorRegisters[] = { CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUSr,
                                    CO_ERRORr,
                                    CO_REFRESH_OVERFLOW_ERRORr,
                                    CO_SEGMENT_DISABLE_ERRORr,
                                    CO_SEGMENT_RANGE_ERRORr,
                                    CO_ECC_ERRORr,
                                    CO_METER_BUCKET_OVERFLOW_STATUSr,
                                    CO_TIMER_EXPIRED_FIFO_ECC_STATUSr,
                                    CO_METER_PROFILE_ECC_STATUSr,
                                    CO_CACHE_DATA_ECC_STATUSr,
                                    CO_WB_BUFFER_ECC_STATUSr,
                                    CO_METER_MONITOR_COUNTER_ECC_STATUSr
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUS_MASKr,
                                        CO_ERROR_MASKr,
                                        CO_REFRESH_OVERFLOW_ERROR_MASKr,
                                        CO_SEGMENT_DISABLE_ERROR_MASKr,
                                        CO_SEGMENT_RANGE_ERROR_MASKr,
                                        CO_ECC_ERROR_MASKr,
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);


static uint64 g_auuMeterMonitorPktCounters[C3HPPC_COP_INSTANCE_NUM][C3HPPC_COP_METER_MONITOR_COUNTERS_NUM];
static uint64 g_auuMeterMonitorByteCounters[C3HPPC_COP_INSTANCE_NUM][C3HPPC_COP_METER_MONITOR_COUNTERS_NUM];

static sal_thread_t g_CopWatchDogTimerRingManagerThreadID = NULL;
static c3hppc_cop_watchdogtimer_manager_cb_t g_c3hppcCopWatchDogTimerRingManagerCB;

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;
static uint8 g_bScoreBoardMode = 0;

int c3hppc_cop_hw_init( int nUnit, int nCopInstance, c3hppc_cop_control_info_t *pC3CopControlInfo ) {

  uint32 u32bRegisterValue;
  sal_time_t TimeStamp;

  soc_cm_get_id( nUnit, &g_dev_id, &g_rev_id);
  g_bScoreBoardMode = pC3CopControlInfo->bWDT_ScoreBoardMode;

  /*
   * Take block out of soft reset
  */
  soc_reg32_get( nUnit, CO_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nCopInstance), 0, &u32bRegisterValue );
  soc_reg_field_set( nUnit, CO_GLOBAL_CONFIGr, &u32bRegisterValue, SOFT_RESETf, 1 );
  soc_reg32_set( nUnit, CO_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nCopInstance), 0, u32bRegisterValue);

  u32bRegisterValue = 0;
  soc_reg_field_set( nUnit, CO_MEMORY_INITr, &u32bRegisterValue, METER_PROFILE_INITf, 1 );
  soc_reg_field_set( nUnit, CO_MEMORY_INITr, &u32bRegisterValue, METER_MONITOR_COUNTER_INITf, 1 );
  soc_reg32_set( nUnit, CO_MEMORY_INITr, SOC_BLOCK_PORT(nUnit,nCopInstance), 0, u32bRegisterValue);

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                               CO_MEMORY_INIT_DONEr, METER_MONITOR_COUNTER_INIT_DONEf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_cop_hw_init> -- COP%d CO_MEMORY_INIT_DONE \"METER_MONITOR_COUNTER_INIT_DONE\" event TIMEOUT!!!\n", nCopInstance);
    return -1;
  }

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCopInstance),
                               CO_MEMORY_INIT_DONEr, METER_PROFILE_INIT_DONEf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_cop_hw_init> -- COP%d CO_MEMORY_INIT_DONE \"METER_PROFILE_INIT_DONE\" event TIMEOUT!!!\n", nCopInstance);
    return -1;
  }

  if ( nCopInstance == 0 ) {
    g_c3hppcCopWatchDogTimerRingManagerCB.nUnit = nUnit;
    g_c3hppcCopWatchDogTimerRingManagerCB.nNumberOfTimers = 0xffffffff;
    g_c3hppcCopWatchDogTimerRingManagerCB.nErrorCounter = nUnit;
    g_c3hppcCopWatchDogTimerRingManagerCB.uWDT_ListWriteOffset = 0;
    g_c3hppcCopWatchDogTimerRingManagerCB.bExit = 0;
    g_c3hppcCopWatchDogTimerRingManagerCB.bScoreBoardMode = pC3CopControlInfo->bWDT_ScoreBoardMode;
    g_CopWatchDogTimerRingManagerThreadID = sal_thread_create( "tCopWatchDogTimerRingManager",
                                                       SAL_THREAD_STKSZ,
                                                       100,
                                                       c3hppc_cop_watchdogtimer_ring_manager,
                                                       (void *) &g_c3hppcCopWatchDogTimerRingManagerCB);
    if ( g_CopWatchDogTimerRingManagerThreadID == NULL || g_CopWatchDogTimerRingManagerThreadID == SAL_THREAD_ERROR ) {
      cli_out("\nERROR: Can not create COP WatchDogTimer RING manager thread\n");
    }
  }

  if ( c3hppc_cop_segments_config( nUnit, nCopInstance, pC3CopControlInfo->pCopSegmentInfo ) ) {
    return -1;
  }

  if ( c3hppc_cop_profile_config( nUnit, nCopInstance, pC3CopControlInfo->pCopProfileInfo ) ) {
    return -1;
  }

  return 0;
}


int c3hppc_cop_segments_config( int nUnit, int nCopInstance, c3hppc_cop_segment_info_t *pCopSegmentInfo ) {

  int nSegment, nOcmPort;
  c3hppc_cop_segment_info_t *pSegmentInfo;

  for ( nSegment = 0; nSegment < C3HPPC_COP_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCopSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid ) {
      if ( c3hppc_cop_program_segment_table(nUnit, nCopInstance, nSegment, pSegmentInfo) ) {
      }
      nOcmPort = c3hppc_ocm_map_cop2ocm_port( nCopInstance );
      c3hppc_ocm_port_program_port_table( nUnit, nOcmPort, pSegmentInfo->uSegmentBase,
                                          pSegmentInfo->uSegmentOcmLimit, pSegmentInfo->uSegmentTransferSize,
                                          pSegmentInfo->nStartingPhysicalBlock );
    }
  }

  return 0;
}


int c3hppc_cop_segments_enable( int nUnit, int nCopInstance, c3hppc_cop_segment_info_t *pCopSegmentInfo ) {

  int nSegment;
  c3hppc_cop_segment_info_t *pSegmentInfo;

  for ( nSegment = 0; nSegment < C3HPPC_COP_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCopSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid ) {
      if ( c3hppc_cop_program_segment_enable( nUnit, nCopInstance, nSegment ) ) {
      }
    }
  }

  return 0;
}


int c3hppc_cop_program_segment_table( int nUnit, int nCopInstance, int nSegment,
                                      c3hppc_cop_segment_info_t *pCopSegmentInfo )
{
  uint32 u32bRegisterValue;
  uint32 uTimerInterval;

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_BASEr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, pCopSegmentInfo->uSegmentBase) );

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_LIMITr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, pCopSegmentInfo->uSegmentLimit) );

  u32bRegisterValue = 0;
  soc_reg_field_set( nUnit, CO_SEGMENT_CONFIGr, &u32bRegisterValue, Tf, pCopSegmentInfo->uSegmentType );
  if ( pCopSegmentInfo->uSegmentType == C3HPPC_COP_SEGMENT_TYPE__COHERENT_TABLE ) {
    soc_reg_field_set( nUnit, CO_SEGMENT_CONFIGr, &u32bRegisterValue, COHERENT_TABLE_FORMATf, 
                                                                pCopSegmentInfo->uSegmentTransferSize );
  } else if ( pCopSegmentInfo->uSegmentType == C3HPPC_COP_SEGMENT_TYPE__METER ) {
  } else if ( pCopSegmentInfo->uSegmentType == C3HPPC_COP_SEGMENT_TYPE__TIMER ) {
    soc_reg_field_set( nUnit, CO_SEGMENT_CONFIGr, &u32bRegisterValue, TIMER_MODE64f, 
                                                                pCopSegmentInfo->uMode64 );
  }
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_CONFIGr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, u32bRegisterValue) );

  u32bRegisterValue = 0;
  if ( pCopSegmentInfo->uSegmentType == C3HPPC_COP_SEGMENT_TYPE__TIMER ) {
    if ( g_bScoreBoardMode ) {
      /*
         "ScoreBoardMode" means pummel the expiration FIFO and to do that reducing the "uRefreshVisitInterval" to the
         minimum value possible.
      */
      pCopSegmentInfo->uRefreshVisitInterval = 2;
    } else {
      uTimerInterval = COMPILER_64_LO(pCopSegmentInfo->uuRefreshVisitPeriod);
      pCopSegmentInfo->uRefreshVisitInterval =  (2 * uTimerInterval) / 
                                              ((1 + pCopSegmentInfo->uMode64) * (pCopSegmentInfo->uSegmentLimit + 1)); 
    }
  }
  soc_reg_field_set( nUnit, CO_SEGMENT_VISIT_INTERVALr, &u32bRegisterValue, VISIT_INTERVALf, pCopSegmentInfo->uRefreshVisitInterval );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_VISIT_INTERVALr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, u32bRegisterValue) );

  u32bRegisterValue = 0;
  { uint64 uuTmp;
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(pCopSegmentInfo->uuRefreshVisitPeriod), COMPILER_64_LO(pCopSegmentInfo->uuRefreshVisitPeriod));
    COMPILER_64_SHR(uuTmp, 20);
    soc_reg_field_set( nUnit, CO_SEGMENT_MISC_CONFIG_HIr, &u32bRegisterValue, VISIT_PERIOD_39_20f, COMPILER_64_LO(uuTmp) );
  }
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_MISC_CONFIG_HIr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, u32bRegisterValue) );

  u32bRegisterValue = 0;
  soc_reg_field_set( nUnit, CO_SEGMENT_MISC_CONFIG_LOr, &u32bRegisterValue, VISIT_PERIOD_19_0f,
                           (COMPILER_64_LO(pCopSegmentInfo->uuRefreshVisitPeriod) & 0x000fffff) );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_MISC_CONFIG_LOr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nSegment, u32bRegisterValue) );

  return 0;
}


int c3hppc_cop_program_segment_enable( int nUnit, int nCopInstance, int nSegment )
{
  uint32 u32bRegisterValue;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CO_SEGMENT_ENABLEr, SOC_BLOCK_PORT(nUnit,nCopInstance),
                                      0, &u32bRegisterValue ) );

  u32bRegisterValue |= (1 << nSegment);

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_SEGMENT_ENABLEr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      0, u32bRegisterValue) );

  return 0;
}


int c3hppc_cop_profile_config( int nUnit, int nCopInstance, c3hppc_cop_profile_info_t *pCopProfileInfo ) {

  int nProfile;
  c3hppc_cop_profile_info_t *pProfileInfo;

  for ( nProfile = 0; nProfile < C3HPPC_COP_PROFILE_NUM; ++nProfile ) {
    pProfileInfo = pCopProfileInfo + nProfile;
    if ( pProfileInfo->bValid ) {
      if ( c3hppc_cop_program_profile_table(nUnit, nCopInstance, nProfile, pProfileInfo) ) {
      }
    }
  }

  return 0;
}


int c3hppc_cop_program_profile_table( int nUnit, int nCopInstance, int nProfile,
                                      c3hppc_cop_profile_info_t *pCopProfileInfo )
{
  uint32 auProfileTableEntry[3];
  uint32 uE, uM;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, CO_METER_PROFILEm, CO_BLOCK(nUnit,nCopInstance),
                                    nProfile, auProfileTableEntry) );

  c3hppc_cop_calc_bs_e_and_m( pCopProfileInfo->uCBSinBytes, &uE, &uM );
/* cli_out(" CBS c3hppc_cop_calc_bs_e_and_m %d %d %d \n", pCopProfileInfo->uCBSinBytes, uE, uM ); */
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, CBS_EXPONENTf, &uE );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, CBS_MANTISSAf, &uM );

  c3hppc_cop_calc_bs_e_and_m( pCopProfileInfo->uEBSinBytes, &uE, &uM );
/* cli_out(" EBS c3hppc_cop_calc_bs_e_and_m %d %d %d \n", pCopProfileInfo->uEBSinBytes,uE, uM ); */
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, EBS_EXPONENTf, &uE );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, EBS_MANTISSAf, &uM );

  c3hppc_cop_calc_ir_e_and_m( pCopProfileInfo->uCIRinKbps, &uE, &uM );
/* cli_out(" CIR c3hppc_cop_calc_bs_e_and_m %d %d %d \n", pCopProfileInfo->uCIRinKbps, uE, uM ); */ 
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, CIR_EXPONENTf, &uE );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, CIR_MANTISSAf, &uM );

  c3hppc_cop_calc_ir_e_and_m( pCopProfileInfo->uEIRinKbps, &uE, &uM );
/* cli_out(" EIR c3hppc_cop_calc_bs_e_and_m %d %d %d \n", pCopProfileInfo->uEIRinKbps, uE, uM ); */
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, EIR_EXPONENTf, &uE );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, EIR_MANTISSAf, &uM );

  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, BLINDf,       &(pCopProfileInfo->bBlind) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, DROP_ON_REDf, &(pCopProfileInfo->bDropOnRed) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, CP_FLAGf,     &(pCopProfileInfo->bCPflag) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, RFC2698f,     &(pCopProfileInfo->bRFC2698) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, BKTC_STRICTf, &(pCopProfileInfo->bBKTCstrict) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, BKTE_STRICTf, &(pCopProfileInfo->bBKTEstrict) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, BKTC_NODECf,  &(pCopProfileInfo->bBKTCnodec) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, BKTE_NODECf,  &(pCopProfileInfo->bBKTEnodec) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, PKT_MODEf,    &(pCopProfileInfo->bPktMode) );
  soc_mem_field_set( nUnit, CO_METER_PROFILEm, auProfileTableEntry, LEN_ADJf,     &(pCopProfileInfo->uPktLengthAdjust) );


  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, CO_METER_PROFILEm, CO_BLOCK(nUnit,nCopInstance),
                                     nProfile, auProfileTableEntry) );

  return 0;
}

void c3hppc_cop_calc_bs_e_and_m( uint32 uBurstSizeInBytes, uint32 *uE, uint32 *uM ) {

  int e, m;
  double d;

  e = (int) (c3hppcUtils_floor_power_of_2_exp(uBurstSizeInBytes) + 1);
  d = (double) uBurstSizeInBytes;
  d /= (double) (1 << (e - 1));
  d -= 1.0;
  d *= 128.0;
  m = (int) (d + 0.5);
  if ( m == 128 ) {
    e += 1;
    m = 0;
  }
  *uE = (uint32) e;
  *uM = (uint32) m;

  return;
}

void c3hppc_cop_calc_ir_e_and_m( uint32 uInformationRateInKbps, uint32 *uE, uint32 *uM ) {

  int e, m;
  double d, dBytesPerCycle;

  dBytesPerCycle = (double) uInformationRateInKbps;
  dBytesPerCycle *= 1000.0;
  dBytesPerCycle /= 8.0;
  dBytesPerCycle /= 1000000000.0;
  e = (int) (c3hppcUtils_floor_power_of_2_exp_real(dBytesPerCycle) + 27);
/*
   cli_out(" CIR c3hppc_cop_calc_bs_e_and_m  dBytesPerCycle %f  e %d  \n", dBytesPerCycle, e );   
*/
  d = dBytesPerCycle;
  d /= ( e >= 27 ) ? ( (double) (1 << (e - 27)) ) : ( (double) c3hppcUtils_negative_exp_to_decimal_value(e - 27) );
  d -= 1.0;
  d *= 2048.0;
  m = (int) (d + 0.5);
  if ( m == 2048 ) {
    e += 1;
    m = 0;
  }
  *uE = (uint32) e;
  *uM = (uint32) m;

  return;
}




int c3hppc_cop_hw_cleanup( int nUnit ) {

  int nIndex, nInstance;
  uint32 uRegisterValue;

  for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nErrorMaskRegistersCount; ++nIndex ) {
      uRegisterValue = 0;
      if ( g_anErrorMaskRegisters[nIndex] == CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUS_MASKr ) {
        soc_reg_field_set( nUnit, CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUSr, &uRegisterValue, NONEMPTYf, 1 );
      }
      soc_reg32_set( nUnit, g_anErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
  }

  return 0;
}



int c3hppc_cop_display_error_state( int nUnit ) {

  int rc, nIndex, nInstance;

  for ( rc = 0, nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anErrorRegisters[nIndex] );
    }
  }

  return rc;
}




int c3hppc_cop_coherent_table_read_write( int nUnit, int nCopInstance, int nSegment,     
                                          uint32 uOffset, uint8 bWrite, uint32 *puEntryData )
{
  uint32 uRegisterValue;
  sal_time_t TimeStamp;

  uRegisterValue = 0;
/*
  soc_reg_field_set( nUnit, CO_INJECT_DATA2r, &uRegisterValue, TOPf, (bWrite ? 2 : 0) );
  soc_reg_field_set( nUnit, CO_INJECT_DATA2r, &uRegisterValue, OFFSETf, uOffset );
*/
  uRegisterValue = ( bWrite ) ? 0x80000000 : 0x00000000;
  uRegisterValue |= uOffset;
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_INJECT_DATA2r, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      0, uRegisterValue) );
  if ( bWrite ) {
    SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_INJECT_DATA0r, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                        0, puEntryData[0]) );
    SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_INJECT_DATA1r, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                        0, puEntryData[1]) );
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, CO_INJECT_CTRLr, &uRegisterValue, REQf, 1 );
  soc_reg_field_set( nUnit, CO_INJECT_CTRLr, &uRegisterValue, SEGMENTf, (uint32) nSegment );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_INJECT_CTRLr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      0, uRegisterValue) );

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                               CO_INJECT_STATUSr, ACKf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_cop_coherent_table_read_write> -- COP%d CO_INJECT_STATUS \"ACK\" event TIMEOUT!!!\n", nCopInstance);
    return -1;
  }

  
  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CO_INJECT_DATA0r, SOC_BLOCK_PORT(nUnit,nCopInstance), 0, puEntryData+0 ) );
  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CO_INJECT_DATA1r, SOC_BLOCK_PORT(nUnit,nCopInstance), 0, puEntryData+1 ) );

  return 0;
}


int c3hppc_cop_meter_monitor_setup( int nUnit, int nCopInstance, int nGroup, uint32 uSegment, uint32 uMeterID ) {

  uint32 uRegisterValue;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CO_METER_MONITOR_CONFIGr, SOC_BLOCK_PORT(nUnit,nCopInstance), nGroup, &uRegisterValue ) );
  soc_reg_field_set( nUnit, CO_METER_MONITOR_CONFIGr, &uRegisterValue, ENABLEf, 1 );
  soc_reg_field_set( nUnit, CO_METER_MONITOR_CONFIGr, &uRegisterValue, SEGMENTf, uSegment );
  soc_reg_field_set( nUnit, CO_METER_MONITOR_CONFIGr, &uRegisterValue, OFFSETf, uMeterID );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CO_METER_MONITOR_CONFIGr, SOC_BLOCK_PORT(nUnit,nCopInstance), 
                                      nGroup, uRegisterValue) );

  return 0;
}


int c3hppc_cop_meter_monitor_dump_memory( int nUnit, int nInstance ) {

  uint32 *pDmaData, uDmaSize, uCount[2];
  int nIndex, nDmaIndex, nMemRowNum, nMemEntryWords;

  nMemEntryWords = soc_mem_entry_words( nUnit, CO_METER_MONITOR_COUNTERm );
  nMemRowNum = soc_mem_index_max( nUnit, CO_METER_MONITOR_COUNTERm ) + 1;
  uDmaSize = nMemRowNum * nMemEntryWords * sizeof(uint32);
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, uDmaSize, "meter monitor mem" );
  sal_memset( pDmaData, 0x00, uDmaSize );
  SOC_IF_ERROR_RETURN( soc_mem_read_range( nUnit, CO_METER_MONITOR_COUNTERm, CO_BLOCK(nUnit,nInstance), 0,
                                           (nMemRowNum - 1), (void *) pDmaData) );

  for ( nIndex = 0, nDmaIndex = 0; nIndex < nMemRowNum; ++nIndex, nDmaIndex += nMemEntryWords ) {
    soc_mem_field_get( nUnit, CO_METER_MONITOR_COUNTERm, (pDmaData+nDmaIndex), BYTE_COUNTf, uCount );
    COMPILER_64_SET(g_auuMeterMonitorByteCounters[nInstance][nIndex], uCount[1], uCount[0]);
    soc_mem_field_get( nUnit, CO_METER_MONITOR_COUNTERm, (pDmaData+nDmaIndex), PACKET_COUNTf, uCount );
    COMPILER_64_SET(g_auuMeterMonitorPktCounters[nInstance][nIndex], uCount[1], uCount[0]);
  }

  soc_cm_sfree( nUnit, pDmaData );

  return 0;
}

uint64 c3hppc_cop_meter_monitor_get_packet_count( int nIndex, int nInstance ) {
  return g_auuMeterMonitorPktCounters[nInstance][nIndex];
}

uint64 c3hppc_cop_meter_monitor_get_byte_count( int nIndex, int nInstance ) {
  return g_auuMeterMonitorByteCounters[nInstance][nIndex];
}


int c3hppc_cop_exit_watchdogtimer_ring_manager_thread( void ) {

  g_c3hppcCopWatchDogTimerRingManagerCB.bExit = 1;
  sal_sleep(1);

  return 0;
}

int c3hppc_cop_get_watchdogtimer_ring_manager_timer_count( int nCopInstance ) {
  return g_c3hppcCopWatchDogTimerRingManagerCB.anExpiredTimerCount[nCopInstance];
}

int c3hppc_cop_get_watchdogtimer_ring_manager_error_count( void ) {
  return g_c3hppcCopWatchDogTimerRingManagerCB.nErrorCounter;
}

void c3hppc_cop_set_watchdogtimer_ring_manager_timer_num( int nNumberOfTimers ) {
  g_c3hppcCopWatchDogTimerRingManagerCB.nNumberOfTimers = nNumberOfTimers;
}

void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_svp( int nSVP ) {
  g_c3hppcCopWatchDogTimerRingManagerCB.nLrpSVP = nSVP;
}

void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_lm( int nLM ) {
  g_c3hppcCopWatchDogTimerRingManagerCB.nLrpLM = nLM;
}

void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_size( int nEntryNum ) {
  g_c3hppcCopWatchDogTimerRingManagerCB.uWDT_ListEntryNum = nEntryNum;
}

void c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_base( uint32 uOcmBase ) {
  g_c3hppcCopWatchDogTimerRingManagerCB.uWDT_ListOcmBase = uOcmBase;
}


void c3hppc_cop_format_meterstate_in_ocm_entry( c3hppc_64b_ocm_entry_template_t *pOcmEntry, 
                                                uint32 uProfile, uint32 uBktE, uint32 uBktC ) {

  pOcmEntry->uData[1] = (uProfile << 22) | (uBktE >> 5);
  pOcmEntry->uData[0] = (uBktE << 27) | uBktC;

  return;
}


void c3hppc_cop_watchdogtimer_ring_manager( void *pWatchDogTimerRingManagerCB_arg )
{

  int nUnit, copyno;
  uint8 at;
  uint32 uRegisterValue;
  VOL uint32 *pWatchDogTimerRing[C3HPPC_COP_INSTANCE_NUM];
  VOL uint32 *pWatchDogTimerRingLimit[C3HPPC_COP_INSTANCE_NUM];
  VOL uint32 *pWatchDogTimerRingEntry[C3HPPC_COP_INSTANCE_NUM];
  uint32 uProcessedEntryNum, uTotalProcessedEntryNum[C3HPPC_COP_INSTANCE_NUM];
  uint32 uDataBeats, uEntryNum, uWatchDogTimerRingSize;
  c3hppc_cop_watchdogtimer_ring_entry_ut WatchDogTimerEntry;
  c3hppc_cop_watchdogtimer_ring_entry_ut ExpectedWatchDogTimerEntry[C3HPPC_COP_INSTANCE_NUM];
  schan_msg_t msg;
  c3hppc_cop_watchdogtimer_manager_cb_t *pWatchDogTimerRingManagerCB;
  soc_mem_t aCopWatchDogTimerFifoMem[] = { CO_WATCHDOG_TIMER_EXPIRED_FIFOm, CO_WATCHDOG_TIMER_EXPIRED_FIFOm };
  int nRing;
  c3hppc_cop_watchdogtimer_host2lrp_control_word_ut Host2Lrp_ControlWord;
  int nOcmPort, nTimer;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  char *acWDT_ScoreBoard[C3HPPC_COP_INSTANCE_NUM];
 
  

  cli_out("  Entering COP WatchDogTimer RING Manager thread .... \n\n");




  pWatchDogTimerRingManagerCB = (c3hppc_cop_watchdogtimer_manager_cb_t *) pWatchDogTimerRingManagerCB_arg;
  nUnit = pWatchDogTimerRingManagerCB->nUnit;

  if ( pWatchDogTimerRingManagerCB->bScoreBoardMode ) {
    for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) {
      acWDT_ScoreBoard[nRing] = (char *) sal_alloc( C3HPPC_COP_MAX_WATCHDOG_TIMERS, "Scoreboard" );
      sal_memset( acWDT_ScoreBoard[nRing], 0x00, C3HPPC_COP_MAX_WATCHDOG_TIMERS );
    }
  }


  COMPILER_64_ZERO(Host2Lrp_ControlWord.value);
  Host2Lrp_ControlWord.bits.Valid = 1;
  if ( g_rev_id == BCM88030_B0_REV_ID ) Host2Lrp_ControlWord.bits.Restart = 1;

  uWatchDogTimerRingSize = 4096;
/* Coverity
  switch (uWatchDogTimerRingSize) {
    case 64:    uEntryNum = 0; break;
    case 128:   uEntryNum = 1; break;
    case 256:   uEntryNum = 2; break;
    case 512:   uEntryNum = 3; break;
    case 1024:  uEntryNum = 4; break;
    case 2048:  uEntryNum = 5; break;
    case 4096:  uEntryNum = 6; break;
    case 8192:  uEntryNum = 7; break;
    case 16384: uEntryNum = 8; break;
    case 32768: uEntryNum = 9; break;
  }
*/
  uEntryNum = 6;
  uDataBeats = soc_mem_entry_words( nUnit, aCopWatchDogTimerFifoMem[0] );

  /*
   * Setup CMIC_CMC1_FIFO_CH0_RD_DMA and CMIC_CMC1_FIFO_CH1_RD_DMA for watchdog timer expiration ring processing
  */
  for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) {

    pWatchDogTimerRingManagerCB->anExpiredTimerCount[nRing] = 0;
    ExpectedWatchDogTimerEntry[nRing].value = 0;
    /* Added for B0 */
    ExpectedWatchDogTimerEntry[nRing].bits.Active = ( g_rev_id == BCM88030_B0_REV_ID ) ? 0 : 1;
    ExpectedWatchDogTimerEntry[nRing].bits.Segment = 1;

    copyno = CO_BLOCK(nUnit,nRing);
    uRegisterValue = soc_mem_addr_get( nUnit, aCopWatchDogTimerFifoMem[nRing], 0, copyno, 0, &at);
    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );

    schan_msg_clear(&msg);
    msg.readcmd.header.v3.opcode = FIFO_POP_CMD_MSG;
    msg.readcmd.header.v3.dst_blk = SOC_BLOCK2SCH( nUnit, copyno );
    msg.readcmd.header.v3.data_byte_len = uDataBeats * sizeof(uint32);
    /* Set 1st schan ctrl word as opcode */
    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );

    pWatchDogTimerRing[nRing] = (uint32 *) soc_cm_salloc( nUnit, (uWatchDogTimerRingSize * uDataBeats * sizeof(uint32)), "response_ring");
    pWatchDogTimerRingLimit[nRing] = pWatchDogTimerRing[nRing] + (uWatchDogTimerRingSize * uDataBeats);
    sal_memset( (void *)pWatchDogTimerRing[nRing], 0, (uWatchDogTimerRingSize * uDataBeats * sizeof(uint32)) );
    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pWatchDogTimerRing[nRing])) );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pWatchDogTimerRing[nRing])) );

    if ( nRing )
      READ_CMIC_CMC1_FIFO_CH1_RD_DMA_CFGr( nUnit, &uRegisterValue );
    else
      READ_CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, BEAT_COUNTf, uDataBeats );
    soc_reg_field_set( nUnit, CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, HOST_NUM_ENTRIES_SELf, uEntryNum );
    soc_reg_field_set( nUnit, CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ENDIANESSf, 1 );
    if ( nRing ) {
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uWatchDogTimerRingSize-64) );
    } else {
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uWatchDogTimerRingSize-64) );
    }

    soc_reg_field_set( nUnit, CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ENABLEf, 1 );
    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );

    pWatchDogTimerRingEntry[nRing] = pWatchDogTimerRing[nRing];

    uTotalProcessedEntryNum[nRing] = 0;
  }

  while ( !pWatchDogTimerRingManagerCB->bExit ) {

    for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) {
      uProcessedEntryNum = 0;

      while ( *pWatchDogTimerRingEntry[nRing] ) {

        assert( pWatchDogTimerRingEntry[nRing] < pWatchDogTimerRingLimit[nRing] );

        ++uProcessedEntryNum;

        WatchDogTimerEntry.value = *pWatchDogTimerRingEntry[nRing];
        if ( pWatchDogTimerRingManagerCB->bScoreBoardMode ) {
          ++acWDT_ScoreBoard[nRing][WatchDogTimerEntry.bits.Offset];

        } else if ( WatchDogTimerEntry.value != ExpectedWatchDogTimerEntry[nRing].value ) {
          cli_out(" \"RING%d\" MISCOMPARE --> Actual: 0x%08x  Expect: 0x%08x \n", nRing, WatchDogTimerEntry.value, 
                  ExpectedWatchDogTimerEntry[nRing].value ); 
          ++pWatchDogTimerRingManagerCB->nErrorCounter;
        }
        if ( pWatchDogTimerRingManagerCB->nErrorCounter > 16 ) {
          pWatchDogTimerRingManagerCB->bExit = 1;
          break; 
        } else {
          ++pWatchDogTimerRingManagerCB->anExpiredTimerCount[nRing];
          ++ExpectedWatchDogTimerEntry[nRing].bits.Offset;
          if ( ExpectedWatchDogTimerEntry[nRing].bits.Offset == pWatchDogTimerRingManagerCB->nNumberOfTimers && 
               !pWatchDogTimerRingManagerCB->bScoreBoardMode ) {
            ExpectedWatchDogTimerEntry[nRing].bits.Offset = 0;
          
            pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) 
                            soc_cm_salloc( nUnit,
                                           pWatchDogTimerRingManagerCB->nNumberOfTimers * sizeof(c3hppc_64b_ocm_entry_template_t),
                                           "ocm_block");
            nOcmPort = c3hppc_ocm_map_lrp2ocm_port( pWatchDogTimerRingManagerCB->nLrpSVP );
            for ( nTimer = 0; nTimer < pWatchDogTimerRingManagerCB->nNumberOfTimers; ++nTimer ) {
              Host2Lrp_ControlWord.bits.CopInstance = nRing;
              Host2Lrp_ControlWord.bits.Offset = nTimer;
              pOcmBlock[nTimer].uData[1] = COMPILER_64_HI(Host2Lrp_ControlWord.value);
              pOcmBlock[nTimer].uData[0] = COMPILER_64_LO(Host2Lrp_ControlWord.value);
            }
            c3hppc_ocm_dma_read_write( nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                       (pWatchDogTimerRingManagerCB->uWDT_ListOcmBase + pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset),
                                       (pWatchDogTimerRingManagerCB->uWDT_ListOcmBase + pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset +
                                        pWatchDogTimerRingManagerCB->nNumberOfTimers - 1), 1, pOcmBlock->uData );
            pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset += pWatchDogTimerRingManagerCB->nNumberOfTimers;
            if ( pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset == pWatchDogTimerRingManagerCB->uWDT_ListEntryNum ) {
              pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset = 0;
            }
            c3hppc_lrp_set_host_producer_ring_write_offset( nUnit, pWatchDogTimerRingManagerCB->nLrpSVP, pWatchDogTimerRingManagerCB->nLrpLM,
                                                            pWatchDogTimerRingManagerCB->uWDT_ListWriteOffset);
            soc_cm_sfree( nUnit, pOcmBlock );
          }
        }


        *pWatchDogTimerRingEntry[nRing] = 0;
#if (defined(LINUX))
        soc_cm_sinval( nUnit, (void *)pWatchDogTimerRingEntry[nRing], WORDS2BYTES(uDataBeats) );
#endif
        pWatchDogTimerRingEntry[nRing] += uDataBeats;

        if ( pWatchDogTimerRingEntry[nRing] == pWatchDogTimerRingLimit[nRing] ) {
          pWatchDogTimerRingEntry[nRing] = pWatchDogTimerRing[nRing];
/*
          cli_out("  RSP_FIFO%d ring wrapped!\n", nRing);
*/
        }

      }

      if ( uProcessedEntryNum ) {
        if ( nRing )
          WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
        else
          WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
        uTotalProcessedEntryNum[nRing] += uProcessedEntryNum;
/*
        cli_out("  Processed %d out of %d total WatchDogFifo ring%d elements.\n", uProcessedEntryNum,
                uTotalProcessedEntryNum[nRing], nRing );
*/
      }

    } /* for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) { */

#if (defined(LINUX))
    sal_usleep(1);
#else
    sal_sleep(0);
#endif
  }


  for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) {

    if ( pWatchDogTimerRingManagerCB->bScoreBoardMode ) {
      /* The following check assumes 32b WDTs.  The "nTimer & 2" factor accounts for the filler entries. */
      for ( nTimer = 0; nTimer < C3HPPC_COP_MAX_WATCHDOG_TIMERS; ++nTimer ) {
        if ( ((nTimer & 2)  && acWDT_ScoreBoard[nRing][nTimer] != 0 && nTimer < pWatchDogTimerRingManagerCB->nNumberOfTimers) ||
             (!(nTimer & 2) && acWDT_ScoreBoard[nRing][nTimer] != 1 && pWatchDogTimerRingManagerCB->bScoreBoardMode == 2 &&
              nTimer < pWatchDogTimerRingManagerCB->nNumberOfTimers) ||
             (acWDT_ScoreBoard[nRing][nTimer] != 0 && nTimer >= pWatchDogTimerRingManagerCB->nNumberOfTimers) ) {
          cli_out(" ERROR:  \"RING%d\" Timer %d scoreboard MISCOMPARE -->  Actual: %d \n",
                  nRing, nTimer, (int) acWDT_ScoreBoard[nRing][nTimer] );
          ++pWatchDogTimerRingManagerCB->nErrorCounter;
        }
      }

      sal_free( acWDT_ScoreBoard[nRing] );
    }


    soc_cm_sfree( nUnit, (void *)pWatchDogTimerRing[nRing] );

    soc_reg_field_set( nUnit, CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ABORTf, 1 );
    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );

    sal_usleep(1);

    if ( nRing )
      WRITE_CMIC_CMC1_FIFO_CH1_RD_DMA_CFGr( nUnit, 0 );
    else
      WRITE_CMIC_CMC1_FIFO_CH0_RD_DMA_CFGr( nUnit, 0 );
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUSr, &uRegisterValue, NONEMPTYf, 1 );
  for ( nRing = 0; nRing < C3HPPC_COP_INSTANCE_NUM; ++nRing ) {
    soc_reg32_set( nUnit, CO_WATCHDOG_TIMER_EXPIRED_FIFO_STATUSr, SOC_BLOCK_PORT(nUnit,nRing), 0, uRegisterValue);
  }

  cli_out("  Exiting COP WatchDogTimer RING Manager thread having processed %d and %d expired timers on rings 0 and 1 respectively.... \n\n",
          pWatchDogTimerRingManagerCB->anExpiredTimerCount[0], pWatchDogTimerRingManagerCB->anExpiredTimerCount[1] );

  sal_thread_exit(0); 

  return;
}

#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
