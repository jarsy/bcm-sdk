/*
 * $Id: c3hppc_sws_test4.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"

 
#define C3HPPC_SWS_TEST4__STREAM_NUM                                (1)
#define C3HPPC_SWS_TEST4__IL_STATS_COLLECTION_INTERVAL              (2000)


#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static uint64 g_auuPrPktDropCounts[C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM];
static uint64 g_auuPrByteDropCounts[C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM];
#endif

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;

int
c3hppc_sws_test4__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance;
  
  soc_cm_get_id( pc3hppcTestInfo->nUnit, &g_dev_id, &g_rev_id);
  
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
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
  pc3hppcTestInfo->BringUpControl.bSwsBypassPrParsing = 0;
  pc3hppcTestInfo->BringUpControl.bSwsOnlyTest = 1;
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);

  /* This does replication to ensure the line-side is sufficiently fed.  Not necessary at this time. */ 
 /* coverity[secure_coding] */
  sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test4.oasm");
 /* coverity[secure_coding] */
  sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "NOP.oasm");

  pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive = pc3hppcTestInfo->bXLportEnable;

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  return rc;
}

int
c3hppc_sws_test4__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int  nIndex, nInstance, nActiveChannelNum;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect, uuIlStatsCollectionInterval;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown, bDoOnce;
  uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;
  uint32 uReg32;
  
  for ( nIndex = 0; nIndex < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM; ++nIndex ) {
    g_auuPrPktDropCounts[nIndex] = 0;
    g_auuPrByteDropCounts[nIndex] = 0;
  }

  switch ( pc3hppcTestInfo->nTDM ) {
    case C3HPPC_SWS_TDM__100G_BY_100IL:
    case C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL:  nActiveChannelNum = 1;  break;
    case C3HPPC_SWS_TDM__48x1G_BY_100IL:        nActiveChannelNum = 48; break;
    case C3HPPC_SWS_TDM__12x10G_BY_100IL:       nActiveChannelNum = 12; break;
    case C3HPPC_SWS_TDM__3x40G_BY_100IL:        nActiveChannelNum = 3; break;
    default:                                    nActiveChannelNum = C3HPPC_SWS_IL_MAX_CHANNEL_NUM;
  }

  if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) nActiveChannelNum += 2;

  if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_1G_MIX ) {
    /* The maximum IL channel that traffic should traverse for the 1G hot swap flavors is 27. */
    nActiveChannelNum = 28;
    /* 
      Using the QM flow control debug feature to throttle unused IL 1G channels 12 to 27
      which map to QM source queues 96 to 111.
    */
    READ_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, &uReg32 );
    uReg32 |= 0x0000ffff;
    WRITE_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, uReg32 );
    WRITE_QM_FC_DEBUG5r( pc3hppcTestInfo->nUnit, 3 );
  }

  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, continuous_epochs );

  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;
  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  if ( g_rev_id != BCM88030_A0_REV_ID ) {
    uuIlStatsCollectionInterval = uuTransmitDuration;
  } else {
    COMPILER_64_SET( uuIlStatsCollectionInterval, 0, C3HPPC_SWS_TEST4__IL_STATS_COLLECTION_INTERVAL );
  }

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      uint64 uuOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1,
                                  uuOnes );
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }


  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {

    COMPILER_64_SET(uuMaxDurationB4StatsCollect,0,
                      C3HPPC_MIN( COMPILER_64_LO(uuIlStatsCollectionInterval), COMPILER_64_LO(uuTransmitDuration) ));

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

        c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                    0,
                                    uuMaxDurationB4StatsCollect, 
                                    ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD :
                                                                                 ILPKTGEN__PAYLOAD_PATTEN__SSO ),
                                    pc3hppcTestInfo->nIPG,
                                    0, (nActiveChannelNum-1), ( nActiveChannelNum == 1 ? ILPKTGEN__FIXED : ILPKTGEN__INCREMENT ), 
                                    pc3hppcTestInfo->nPacketSize, pc3hppcTestInfo->nPacketSize, ILPKTGEN__FIXED,
/*
                                    64, 8192, ILPKTGEN__RANDOM,
*/
                                    NULL,
                                    ( (nInstance == C3HPPC_TEST__LINE_INTERFACE || pc3hppcTestInfo->bLineTrafficGenOnly ||
                                       pc3hppcTestInfo->bFabricTrafficGenOnly) ? ILPKTGEN__BUILTIN_HEADERS__NULL0 :
                                                                                 ILPKTGEN__BUILTIN_HEADERS__NULL1 ),
                                    0 ); 
      }
    }

/*
    if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_40G_MIX ) {
      c3hppc_sws_transition_to_1x40G_8x10G_from_12x10G( pc3hppcTestInfo->nUnit );
    }

    if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_1G_MIX ) {
      c3hppc_sws_transition_to_16x1G_8x10G_from_12x10G( pc3hppcTestInfo->nUnit );
      READ_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, &uReg32 );
      uReg32 &= 0xffff0000;
      WRITE_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, uReg32 );
    }
*/

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {
        c3hppc_sws_il_pktgen_start( pc3hppcTestInfo->nUnit, nInstance );
      }
    }


    bShutDown = bDoOnce = 0;
    while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
             !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

      if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
        bShutDown = 1;
      }

      sal_usleep( SAL_BOOT_QUICKTURN ? 10000000 : 1000000 );

      if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) {
        for ( nIndex = 0; nIndex < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM; ++nIndex ) {
          soc_reg32_get( pc3hppcTestInfo->nUnit, PR_IDP_STATS_DROP_PKT_COUNTr,
                         SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uReg32 );
          g_auuPrPktDropCounts[nIndex] += uReg32;
          soc_reg32_get( pc3hppcTestInfo->nUnit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr,
                         SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uReg32 );
          g_auuPrByteDropCounts[nIndex] += uReg32;
        }
      }


/* Used to test PT flow control clear mechanism when LLFC/SAFC is enabled.

      sal_usleep( SAL_BOOT_QUICKTURN ? 10000 : 1000000 );
      for ( nIndex = 0; nIndex < 12; ++nIndex ) {
        soc_reg32_get( pc3hppcTestInfo->nUnit, PT_IPTE_MAC_RESETr, 
                                               SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
        soc_reg_field_set( pc3hppcTestInfo->nUnit, PT_IPTE_MAC_RESETr, &uReg32, RESET_BKP_STATE_PORTf, (uint32) nIndex );
        soc_reg_field_set( pc3hppcTestInfo->nUnit, PT_IPTE_MAC_RESETr, &uReg32, RESET_BKP_STATEf, 1 );
        soc_reg32_set( pc3hppcTestInfo->nUnit, PT_IPTE_MAC_RESETr, 
                                               SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
      }
*/
      

      if ( pc3hppcTestInfo->nHotSwap && !bDoOnce ) {

        if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_40G_MIX ) {

          c3hppc_sws_transition_to_1x40G_8x10G_from_12x10G( pc3hppcTestInfo->nUnit );
          sal_usleep( SAL_BOOT_QUICKTURN ? 10000000 : 1000000 );
          c3hppc_sws_transition_to_12x10G_from_1x40G_8x10G( pc3hppcTestInfo->nUnit );

        } else {

          c3hppc_sws_transition_to_16x1G_8x10G_from_12x10G( pc3hppcTestInfo->nUnit );
          READ_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, &uReg32 );
          uReg32 &= 0xffff0000;
          WRITE_QM_FC_DEBUG3r( pc3hppcTestInfo->nUnit, uReg32 );

          sal_usleep( SAL_BOOT_QUICKTURN ? 10000000 : 1000000 );

          c3hppc_sws_transition_to_12x10G_from_16x1G_8x10G( pc3hppcTestInfo->nUnit );
          READ_QM_FC_DEBUG2r( pc3hppcTestInfo->nUnit, &uReg32 );
          uReg32 &= 0xfffff0ff;
          WRITE_QM_FC_DEBUG2r( pc3hppcTestInfo->nUnit, uReg32 );

        }
/*
        bDoOnce = 1;
*/
      }

    }

    if ( bShutDown ) {
      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
      }
    }

    sal_usleep( 1000 ); 
    c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );

    if ( bShutDown ) {
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
#endif
}




int
c3hppc_sws_test4__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else


  int nChannel, nDestChannel;
  double dBWinGbps, dBWinMpps;
  uint64 uuSourcePktCount, uuDestinationPktCount, uuSourceByteCount, uuDestinationByteCount;

  if ( !pc3hppcTestInfo->nHotSwap && !pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) {

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

  } else if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_40G_MIX ) {

    for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
      if ( nChannel < 8 || nChannel > 11 ) {
        uuSourcePktCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__PKT );
        uuDestinationPktCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__PKT );
        if ( uuSourcePktCount != uuDestinationPktCount ) {
          cli_out("\nERROR: Channel[%d] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                  nChannel, uuDestinationPktCount, uuSourcePktCount );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        } 
        uuSourceByteCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__BYTE );
        uuDestinationByteCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__BYTE );
        if ( uuSourceByteCount != uuDestinationByteCount ) {
          cli_out("\nERROR: Channel[%d] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                  nChannel, uuDestinationByteCount, uuSourceByteCount );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        } 
      } 
    }

    uuSourcePktCount = uuDestinationPktCount = 0;
    uuSourceByteCount = uuDestinationByteCount = 0;
    for ( nChannel = 8; nChannel < 12; ++nChannel ) {
      uuSourcePktCount += c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__PKT );
      uuDestinationPktCount += c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__PKT );
      uuSourceByteCount += c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__BYTE );
      uuDestinationByteCount += c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__BYTE );
    }
    if ( uuSourcePktCount != uuDestinationPktCount ) {
      cli_out("\nERROR: Channels[8-11] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
              uuDestinationPktCount, uuSourcePktCount );
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    } 
    if ( uuSourceByteCount != uuDestinationByteCount ) {
      cli_out("\nERROR: Channels[8-11] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
              uuDestinationByteCount, uuSourceByteCount );
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    } 

  } else if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL &&
              pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) {

    for ( nChannel = 0; nChannel < 50; ++nChannel ) {
      uuSourcePktCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__PKT );
      nDestChannel = nChannel;
      if ( nChannel >= 48 ) {
        nDestChannel = ( nChannel == 48 ) ? C3HPPC_SWS_PR_XL_BASE_PORT : C3HPPC_SWS_PR_XL_BASE_PORT + 2;
      }
      uuDestinationPktCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nDestChannel, ILTXSTAT__PKT );
      if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) uuDestinationPktCount += g_auuPrPktDropCounts[nDestChannel];
      if ( uuSourcePktCount != uuDestinationPktCount ) {
        cli_out("\nERROR: Channel[%d] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                nDestChannel, uuDestinationPktCount, uuSourcePktCount );
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
      uuSourceByteCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__BYTE );
      uuDestinationByteCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nDestChannel, ILTXSTAT__BYTE );
      if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) uuDestinationByteCount += g_auuPrByteDropCounts[nDestChannel];
      if ( uuSourceByteCount != uuDestinationByteCount ) {
        cli_out("\nERROR: Channel[%d] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                nDestChannel, uuDestinationByteCount, uuSourceByteCount );
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }

  } else if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_1G_MIX ||
              pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) {

    for ( nChannel = 0; nChannel < (pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_1G_MIX ? 28 : 14); ++nChannel ) {
      uuSourcePktCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__PKT );
      nDestChannel = nChannel;
      if ( nChannel >= 12 ) {
        if ( pc3hppcTestInfo->nHotSwap == C3HPPC_SWS_HOTSWAP__10G_1G_MIX ) {
          nDestChannel += 20;
        } else {
          nDestChannel = ( nChannel == 12 ) ? C3HPPC_SWS_PR_XL_BASE_PORT : C3HPPC_SWS_PR_XL_BASE_PORT + 2;
        }
      }
      uuDestinationPktCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nDestChannel, ILTXSTAT__PKT );
      if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) uuDestinationPktCount += g_auuPrPktDropCounts[nDestChannel];
      if ( uuSourcePktCount != uuDestinationPktCount ) {
        cli_out("\nERROR: Channel[%d] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                nDestChannel, uuDestinationPktCount, uuSourcePktCount );
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
      uuSourceByteCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__BYTE );
      uuDestinationByteCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nDestChannel, ILTXSTAT__BYTE );
      if ( pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive ) uuDestinationByteCount += g_auuPrByteDropCounts[nDestChannel];
      if ( uuSourceByteCount != uuDestinationByteCount ) {
        cli_out("\nERROR: Channel[%d] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                nDestChannel, uuDestinationByteCount, uuSourceByteCount );
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }

  }

  cli_out("\n\n");

  for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
    uuDestinationPktCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__PKT ) +
                            g_auuPrPktDropCounts[nChannel];
    uuDestinationByteCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__BYTE ) +
                            g_auuPrByteDropCounts[nChannel];
    if ( uuDestinationByteCount ) {
      dBWinGbps = (double) ( (8.0 * ((double) uuDestinationByteCount)) /
                               ((double) (pc3hppcTestInfo->uuIterations / (( SAL_BOOT_QUICKTURN ) ? 1000.0 : 1.0))) );
      dBWinGbps /= 1000000000.0;
      dBWinMpps = (double) ( ((double) uuDestinationPktCount) /
                               ((double) (pc3hppcTestInfo->uuIterations / (( SAL_BOOT_QUICKTURN ) ? 1000.0 : 1.0))) );
      dBWinMpps /= 1000000.0;
      cli_out("INFO: Channel[%d] bandwidth --> %.3fGbps/%.2fMpps with PKT/BYTE counts of --> %lld/%lld \n\n",
              nChannel, dBWinGbps, dBWinMpps, uuDestinationPktCount, uuDestinationByteCount );
    }
  }



  return 0;
#endif
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
