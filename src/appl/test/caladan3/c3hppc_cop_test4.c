/*
 * $Id: c3hppc_cop_test4.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"

 
#define C3HPPC_COP_TEST4__STREAM_NUM                                (1)
#define C3HPPC_COP_TEST4__IL_STATS_COLLECTION_INTERVAL              (2000)


#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static int g_nCopSegmentNum;
#endif

int
c3hppc_cop_test4__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int rc, nInstance, nSegment;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_cop_profile_info_t *pCopProfileInfo;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nOcmPort, nDmaBlockSize, nMeter, nStartMeter, nCopSegment, nCopInstance;
  int anStartingPhysicalBlock[C3HPPC_COP_INSTANCE_NUM];
  uint32 uBktE, uBktC;
  

  
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL;
  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS ||
         pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ||
         nInstance == C3HPPC_TEST__FABRIC_INTERFACE ) {  
      pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
    }
  }
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;


  g_nCopSegmentNum = ( pc3hppcTestInfo->nCounterNum == 1 || pc3hppcTestInfo->nCounterNum == 0x100000 ) ? 1 : 2;

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    anStartingPhysicalBlock[nCopInstance] = 0;

    for ( nCopSegment = 0; nCopSegment < g_nCopSegmentNum; ++nCopSegment ) {
      pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
      pCopSegmentInfo->bValid = 1;
      pCopSegmentInfo->uProfile = ( nCopSegment == 0 ) ? 0 : (C3HPPC_COP_PROFILE_NUM - 1);
      pCopSegmentInfo->uSegmentLimit = ( pc3hppcTestInfo->nCounterNum / g_nCopSegmentNum ) - 1;
      pCopSegmentInfo->uSegmentBase = anStartingPhysicalBlock[nCopInstance] << C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT;
      pCopSegmentInfo->uSegmentOcmLimit = pCopSegmentInfo->uSegmentLimit;
      pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__METER;
      pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
      pCopSegmentInfo->nStartingPhysicalBlock = anStartingPhysicalBlock[nCopInstance]; 
      anStartingPhysicalBlock[nCopInstance] += 
         C3HPPC_MAX( 1, ((pCopSegmentInfo->uSegmentLimit+1) / C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) );

      switch ( pc3hppcTestInfo->nCounterNum ) {
        case 0x00000001:    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod,0, 1048576);   break;
        case 0x00000002:    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod,0,2*1048576);  break; /* The 2* factor is due to the segment division done below */
        case 0x00080000:    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod, 0x80, 0x0);   break; /* The 2* factor is due to the segment division done below */
        case 0x00100000:    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod,0, 20971520);  break; /* Divide by 2 required to prevent BKTC_OVERFLOW when CIR=1G */
        default: COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod,0,1048576);
      }
      pCopSegmentInfo->uuRefreshVisitPeriod /= (uint64) g_nCopSegmentNum;
      pCopSegmentInfo->uRefreshVisitInterval = (uint32) ( pCopSegmentInfo->uuRefreshVisitPeriod /
                                                          ((uint64) (pCopSegmentInfo->uSegmentLimit + 1)) );

      if ( pCopSegmentInfo->uRefreshVisitInterval >= 1048576 ) pCopSegmentInfo->uRefreshVisitInterval = 0;

      pCopProfileInfo = &(pc3hppcTestInfo->BringUpControl.aCopProfileInfo[nCopInstance][pCopSegmentInfo->uProfile]);
      if ( !pCopProfileInfo->bValid ) {
        if ( pCopSegmentInfo->uProfile == 0 ) {
          switch ( pc3hppcTestInfo->nCounterNum ) {
            case 0x00000001:    pCopProfileInfo->uCIRinKbps = 10000000; break;
            case 0x00000002:    pCopProfileInfo->uCIRinKbps = 50000000; break;
            case 0x00080000:    pCopProfileInfo->uCIRinKbps = 80;       break;
            case 0x00100000:    pCopProfileInfo->uCIRinKbps = 50000000; break;
            default: pCopProfileInfo->uCIRinKbps = 1000000;
          }
        } else if ( pCopSegmentInfo->uProfile == (C3HPPC_COP_PROFILE_NUM - 1) ) {
          switch ( pc3hppcTestInfo->nCounterNum ) {
            case 0x00000002:    pCopProfileInfo->uCIRinKbps = 64;       break;
            case 0x00080000:    pCopProfileInfo->uCIRinKbps = 64;       break;
            default: pCopProfileInfo->uCIRinKbps = 1000000;
          }
        }
        pCopProfileInfo->bValid = 1;
        pCopProfileInfo->uCBSinBytes = 10000;  /* Roughly a MTU */
        if ( pCopProfileInfo->uCIRinKbps > 100000 ) pCopProfileInfo->uCBSinBytes *= 3;  /* Keep burst low for fast rate convergence. */
        pCopProfileInfo->uEBSinBytes = pCopProfileInfo->uCBSinBytes;
        pCopProfileInfo->uEIRinKbps = pCopProfileInfo->uCIRinKbps;
        pCopProfileInfo->bDropOnRed = 1;
        pCopProfileInfo->bRFC2698 = 1;
        pCopProfileInfo->bBKTCstrict = pc3hppcTestInfo->nSetupOptions;
        pCopProfileInfo->bBKTEstrict = pc3hppcTestInfo->nSetupOptions;
      }
    }
  }

  /*
    The CMU will be setup to maintain "green" packet counters per meter.  Therefore the max number of meters
    that the CMU can be used to maintain counts for is 512K.
  */
  if ( pc3hppcTestInfo->nCounterNum <= 0x00080000 ) {
    for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
      nSegment = nCopInstance;
      pCmuSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[nSegment]);
      pCmuSegmentInfo->bValid = 1;
      pCmuSegmentInfo->uSegment = (uint32) nSegment;
      pCmuSegmentInfo->uSegmentOcmBase = 0;
      pCmuSegmentInfo->uSegmentType = C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
      pCmuSegmentInfo->uSegmentLimit = ( pc3hppcTestInfo->nCounterNum == 1 ) ? 2 : (pc3hppcTestInfo->nCounterNum - 1);
      pCmuSegmentInfo->uSegmentPort = nSegment;
      pCmuSegmentInfo->nStartingPhysicalBlock = anStartingPhysicalBlock[nCopInstance];
    }
  }

  
  switch ( pc3hppcTestInfo->nCounterNum ) {
    case 0x00000001:  
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test4_dualsize.oasm");     
      break;
    case 0x00000002:  
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test4_2_meters.oasm");     
      break;
    case 0x00080000:  
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test4_512K_meters.oasm");  
      break;
    case 0x00100000:  
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test4_1M_meters.oasm");  
      break;
    default: 
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "");
  }
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition, 0, 3);


  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;



  nDmaBlockSize = C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  --nDmaBlockSize;
  assert( pOcmBlock != NULL );

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {

    for ( nCopSegment = 0; nCopSegment < g_nCopSegmentNum; ++nCopSegment ) {
      /* Initialize meter state */
      pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
      nOcmPort = c3hppc_ocm_map_cop2ocm_port(nCopInstance);
      uBktE = 0x7ffffff;
      uBktC = 0x7ffffff;
      for ( nMeter = 0, nStartMeter = 0; nMeter <= (int) pCopSegmentInfo->uSegmentLimit; ++nMeter ) {
        c3hppc_cop_format_meterstate_in_ocm_entry( &(pOcmBlock[nMeter & nDmaBlockSize]), 
                                                   pCopSegmentInfo->uProfile, uBktE, uBktC );
        if ( nMeter == pCopSegmentInfo->uSegmentLimit ||
             (nMeter & nDmaBlockSize) == nDmaBlockSize ) {
          c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, (pCopSegmentInfo->uSegmentBase + nStartMeter),
                                     (pCopSegmentInfo->uSegmentBase + nMeter), 1, pOcmBlock->uData );
          nStartMeter = nMeter + 1;
        }
      }

      c3hppc_cop_meter_monitor_setup( pc3hppcTestInfo->nUnit, nCopInstance, nCopSegment, nCopSegment, 0 );
    }

    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nCopInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance] );
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit,pOcmBlock);


  if ( pc3hppcTestInfo->nCounterNum <= 0x00080000 ) {
    c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                                pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
    c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                         pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  }


  return 0;
#endif
}

int
c3hppc_cop_test4__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nInstance;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  int nMinPacketLen, nMaxPacketLen, nPacketLenMode;
  uint64 uuEpochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;



  
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuEpochs );

  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      uint64 nTmp = COMPILER_64_INIT(0xffffffff,0xffffffff);
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1, nTmp);
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }

  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
    uint32 nTmp = C3HPPC_MIN( C3HPPC_COP_TEST4__IL_STATS_COLLECTION_INTERVAL, COMPILER_64_LO(uuTransmitDuration));
    COMPILER_64_SET(uuMaxDurationB4StatsCollect, 0, nTmp);

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

        if ( pc3hppcTestInfo->nCounterNum == 1 ) {
          nMinPacketLen = 64;
          nMaxPacketLen = 8192;
          nPacketLenMode = ILPKTGEN__PINGPONG;
        } else {
          nMinPacketLen = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? 48  :
                                                    ((pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL) ? 64 : 48) );
          nMaxPacketLen = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? 48  : 
                                                    ((pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL) ? 64 : 48) );
          nPacketLenMode = ILPKTGEN__FIXED;
        }
        
        c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                    0,
                                    uuMaxDurationB4StatsCollect, 
                                    ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD :
                                                                                 ILPKTGEN__PAYLOAD_PATTEN__SSO ),
                                    1,
                                    0,
                                    ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ? 0 : 47 ),
                                    ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ? ILPKTGEN__FIXED :
                                                                                                      ILPKTGEN__INCREMENT ),
                                    nMinPacketLen, nMaxPacketLen, nPacketLenMode,
                                    NULL,
                                    ( (nInstance == C3HPPC_TEST__LINE_INTERFACE || pc3hppcTestInfo->bLineTrafficGenOnly ||
                                       pc3hppcTestInfo->bFabricTrafficGenOnly) ? ILPKTGEN__BUILTIN_HEADERS__NULL0 :
                                                                                 ILPKTGEN__BUILTIN_HEADERS__NULL1 ),
                                    0 ); 
      }
    }

    for ( nIndex = 0; nIndex < nIlInstanceNum; ++nIndex ) {
      if ( nIndex == 0 ) {
        nInstance = sal_rand() % nIlInstanceNum;
      } else {
        nInstance = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) ? C3HPPC_TEST__FABRIC_INTERFACE : C3HPPC_TEST__LINE_INTERFACE; 
      }
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {
        c3hppc_sws_il_pktgen_start( pc3hppcTestInfo->nUnit, nInstance );
      }
    }

    bShutDown = 0;
    while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
             !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

      if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
        bShutDown = 1;
      }

      sal_usleep( 1000000 );

    }

    if ( bShutDown ) {
      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
      }
    }

    sal_sleep( 2 );
    c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );

    if (bShutDown) {
      COMPILER_64_SUB_64(uuTransmitDuration, uuTransmitDuration);
    } else {
      COMPILER_64_SUB_64(uuTransmitDuration, uuMaxDurationB4StatsCollect);
    }
  }



  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      c3hppc_sws_il_pktcap_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, 1 );
      c3hppc_sws_il_pktcap_get( pc3hppcTestInfo->nUnit, nInstance, uPacketCaptureBuf, &uPacketCaptureLen,
                                &uPacketCaptureSOP, &uPacketCaptureEOP, &uPacketCaptureERR,
                                &uPacketCaptureBV, &uPacketCaptureChannel );
      cli_out("\nPacket of length [%d] captured on channel [%d] coming from PT%d:\n",
              uPacketCaptureLen, uPacketCaptureChannel, nInstance );
      for ( nIndex = 0; nIndex < ((uPacketCaptureLen + 3) / 4); ++nIndex ) {
        cli_out("Word[%d] --> 0x%08x\n", nIndex, uPacketCaptureBuf[nIndex] );
      }
    }
  }


  return 0;
}




int
c3hppc_cop_test4__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int nErrorCnt;
  uint64 uuPktCount COMPILER_ATTRIBUTE((unused)), uuByteCount, uuSmallPacketCount, uuLargePacketCount;
  int nMeterID, nIndex, nCopInstance, nCmuSegment, nCopSegment;
  uint64 uuActualCIR, uuExpectCIR;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_cop_profile_info_t *pCopProfileInfo;
  double dActualPercentDiff = 0, dExpectPercentDiff = 0;
  

  if ( pc3hppcTestInfo->bLineTrafficGenOnly ) {

    if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE, C3HPPC_TEST__LINE_INTERFACE, 0 ) )
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else if ( pc3hppcTestInfo->bFabricTrafficGenOnly ) {

    if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE, C3HPPC_TEST__FABRIC_INTERFACE, 0 ) )
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE, C3HPPC_TEST__FABRIC_INTERFACE, 0 ) ||
              c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE, C3HPPC_TEST__LINE_INTERFACE, 0 ) ) {

    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
  }

  /* Tolerances based on 100s run duration */
  switch ( pc3hppcTestInfo->nCounterNum ) {
    case 0x00000002:    dExpectPercentDiff = 1.5;  break;
    case 0x00080000:    dExpectPercentDiff = 2.5;  break;
    case 0x00100000:    dExpectPercentDiff = 0.1;  break;
    default:            dExpectPercentDiff = 1.0;
  }

  nErrorCnt = 0;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {

    c3hppc_cop_meter_monitor_dump_memory( pc3hppcTestInfo->nUnit, nCopInstance );
    
    for ( nIndex = 0; nIndex < C3HPPC_COP_METER_MONITOR_COUNTERS_PER_GROUP; ++nIndex ) {
      uuPktCount = c3hppc_cop_meter_monitor_get_packet_count( nIndex, nCopInstance );
      uuByteCount = c3hppc_cop_meter_monitor_get_byte_count( nIndex, nCopInstance );
      uuActualCIR = (8 * uuByteCount) / pc3hppcTestInfo->uuIterations;
      cli_out("<c3hppc_cop_test4__done> -- COP[%d] Meter[0] Counter[%d] -- Rate --> %lld bps \n", 
              nCopInstance, nIndex, uuActualCIR );
      if ( pc3hppcTestInfo->nCounterNum == 0x100000 && nIndex == 0 ) {
        COMPILER_64_SET(uuExpectCIR, 0xb, 0xA43B7400); /*  = 50000000000LL */
        if ( uuActualCIR >= uuExpectCIR ) dActualPercentDiff = (double) ( uuActualCIR - uuExpectCIR ); 
        else dActualPercentDiff = (double) ( uuExpectCIR - uuActualCIR );
        dActualPercentDiff = ( dActualPercentDiff / ((double) uuExpectCIR) ) * 100.0;
        if ( dActualPercentDiff > dExpectPercentDiff || COMPILER_64_IS_ZERO(uuByteCount) ) {
          if ( nErrorCnt < 64 ) {
            cli_out("<c3hppc_cop_test4__done> -- COP%d meter[0] CIR MISCOMPARE -->  Actual: %lld bps   Expect: %lld bps   PercentDiff: %f  BYTES: %lld\n",
                    nCopInstance, uuActualCIR, uuExpectCIR, dActualPercentDiff, uuByteCount );
          }
          ++nErrorCnt;
        }
      }
    }

  }

  for ( nCopInstance = 0; ( nCopInstance < C3HPPC_COP_INSTANCE_NUM && pc3hppcTestInfo->nCounterNum != 0x100000 ); ++nCopInstance ) {
    nCmuSegment = nCopInstance;
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + nCmuSegment;
    for ( nMeterID = 0; nMeterID < pc3hppcTestInfo->nCounterNum; ++nMeterID ) {
      nCopSegment = ( nMeterID < (pc3hppcTestInfo->nCounterNum / g_nCopSegmentNum) ) ? 0 : 1; 
      pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
      pCopProfileInfo = &(pc3hppcTestInfo->BringUpControl.aCopProfileInfo[nCopInstance][pCopSegmentInfo->uProfile]);
      uuByteCount = *(pCmuSegmentInfo->pSegmentPciBase + (2 * nMeterID));
      if ( pc3hppcTestInfo->nCounterNum == 1 ) {
        uuSmallPacketCount = *(pCmuSegmentInfo->pSegmentPciBase + 1);
        uuLargePacketCount = *(pCmuSegmentInfo->pSegmentPciBase + 3);
        COMPILER_64_ADD_64(uuByteCount, *(pCmuSegmentInfo->pSegmentPciBase + 2)); 
        if ( uuSmallPacketCount >= uuLargePacketCount ) dActualPercentDiff = (double) ( uuSmallPacketCount / uuLargePacketCount );
        else dActualPercentDiff = (double) ( uuLargePacketCount / uuSmallPacketCount );
        cli_out("<c3hppc_cop_test4__done> -- COP%d meter[%d] -->  Small Packet: %lld    Large Packet: %lld   Ratio: %f\n",
                nCopInstance, nMeterID, uuSmallPacketCount, uuLargePacketCount, dActualPercentDiff );
      }
      uuActualCIR = (8 * uuByteCount) / pc3hppcTestInfo->uuIterations;
      COMPILER_64_SET(uuExpectCIR, 0, pCopProfileInfo->uCIRinKbps);
      COMPILER_64_UMUL_32(uuExpectCIR, 1000);
      if ( uuActualCIR >= uuExpectCIR ) dActualPercentDiff = (double) ( uuActualCIR - uuExpectCIR ); 
      else dActualPercentDiff = (double) ( uuExpectCIR - uuActualCIR );
      dActualPercentDiff = ( dActualPercentDiff / ((double) uuExpectCIR) ) * 100.0;
      if ( dActualPercentDiff > dExpectPercentDiff || COMPILER_64_IS_ZERO(uuByteCount) ) {
        if ( nErrorCnt < 64 ) {
          cli_out("<c3hppc_cop_test4__done> -- COP%d meter[%d] CIR MISCOMPARE -->  Actual: %lld bps   Expect: %lld bps   PercentDiff: %f  BYTES: %lld\n",
                  nCopInstance, nMeterID, uuActualCIR, uuExpectCIR, dActualPercentDiff, uuByteCount );
        }
        ++nErrorCnt;
      }
    }
  }


  if ( nErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_cop_test4__done> -- Total error count --> %d\n", nErrorCnt );
  }


  return 0;

#endif
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
