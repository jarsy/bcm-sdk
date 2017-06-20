/*
 * $Id: c3hppc_sws_test5.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT


#include "../c3hppc_test.h"

#include <soc/sbx/caladan3/ped.h>
#include <soc/mem.h>

int c3hppc_sws_test5__bubble_generator( int nUnit );
 
#define C3HPPC_SWS_TEST5__STREAM_NUM                                (1)
#define C3HPPC_SWS_TEST5__IL_STATS_COLLECTION_INTERVAL              (2000)

static uint64 g_auuQmSourceQueueDropCounts[C3HPPC_SWS_IL_INSTANCE_NUM];
static int g_anQmSourceQueueDropRegisters[] = { QM_INGRESS_SQ_DROP_COUNTr, QM_EGRESS_SQ_DROP_COUNTr };

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;
static uint8 g_bDoDoubleRedirect = 0;
static uint8 g_bDoAlternatingUnload = 0;
static uint8 g_bDoChangeDirection = 0;
static uint8 g_bDoReplication = 0;
static uint64 g_uuBubbleCount = COMPILER_64_INIT(0x00000000, 0x00000000);


int
c3hppc_sws_test5__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance;
  
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    COMPILER_64_ZERO(g_auuQmSourceQueueDropCounts[nInstance]);
  }
  /* Added to appease pedantic compiler build warning/errors */
  COMPILER_64_ZERO(g_uuBubbleCount);
  g_nFlowTableSize = 0;
  g_pFlowTable = NULL;
  g_anQmSourceQueueDropRegisters[0] += 0;
  g_anQmSourceQueueDropRegisters[1] += 0;
  g_bDoDoubleRedirect = 0;
  g_bDoAlternatingUnload = 0;
  g_bDoChangeDirection = 0;
  g_bDoReplication = 0;
  if ( pc3hppcTestInfo->bOamEnable && pc3hppcTestInfo->nSetupOptions == 1 ) {
    g_bDoDoubleRedirect = 1;
  }
  if ( pc3hppcTestInfo->nSetupOptions == 2 || pc3hppcTestInfo->nSetupOptions == 5 ) {
    g_bDoAlternatingUnload = 1;
    if ( pc3hppcTestInfo->nSetupOptions == 5 ) {
      g_bDoChangeDirection = 1;
      pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS;
    }
  }
  if ( pc3hppcTestInfo->nSetupOptions == 3 || pc3hppcTestInfo->nSetupOptions == 4 ) {
    g_bDoChangeDirection = 1;
    if ( pc3hppcTestInfo->nSetupOptions == 4 ) g_bDoReplication = 1;
    pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS;
/*
    pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS;
*/
  }
  

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
  }
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPrParsing = 1;
  pc3hppcTestInfo->BringUpControl.bOamEnable = pc3hppcTestInfo->bOamEnable;
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
  if ( g_bDoDoubleRedirect ) {
    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5a.oasm");
  } else if ( g_bDoAlternatingUnload == 1 ) {
   if ( g_bDoChangeDirection == 1 ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5e.oasm");
   } else {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5b.oasm");
   }
  } else if ( g_bDoChangeDirection == 1 ) {
    if ( g_bDoReplication == 0 ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5c.oasm");
    } else {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5d.oasm");
    }
  } else {
    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test5.oasm");
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nInstance] );
  }

  return rc;
}

int
c3hppc_sws_test5__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int  nIndex, nInstance, nCopInstance, nOcmPort;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  int nMinPacketLen, nMaxPacketLen, nPacketLenMode;
  uint32 uRegisterValue, uMaxChannel;
  uint64 uuContinuousEpochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;
  uint64 uuIlStatsCollectionInterval;
  uint64 uuChannelMask;
  int nSecondsCounter;

  uint16 dev_id;
  uint8 rev_id;
  soc_cm_get_id( pc3hppcTestInfo->nUnit, &dev_id, &rev_id);


/*
  WRITE_PD_VALID_SQUEUESr( pc3hppcTestInfo->nUnit, 0, 0xfffffffe );
  WRITE_PD_CHECKSr( pc3hppcTestInfo->nUnit, 0x00000080 );
*/

  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the packet iteration count which needs to be clear at the start.
   *****************************************************************************************************************************/
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    nOcmPort = c3hppc_ocm_map_cop2ocm_port( nCopInstance );
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                               0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );
  } 

  /****************************************************************************************************************************
   * Using the flow table to hold the per source queue iteration value.
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nIndex ) {
    g_pFlowTable[nIndex].uData[0] = 1 + (uint32) g_bDoAlternatingUnload;
    if ( g_bDoDoubleRedirect == 0 ) {
      if ( g_bDoAlternatingUnload == 0 ) {
        g_pFlowTable[nIndex].uData[0] += (sal_rand() % 4);
      } else {
        g_pFlowTable[nIndex].uData[0] += (sal_rand() % 8);
        g_pFlowTable[nIndex].uData[0] &= 0xe;
      }
    }
  }
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port( 0 );
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                             0, (C3HPPC_SWS_SOURCE_QUEUE_NUM-1), 1, g_pFlowTable[0].uData );



  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuContinuousEpochs );

/* Used to turn off PEs
  c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 0, 11, C3HPPC_LRP__TASK_NUM );
  sal_usleep( 2000000 );
  c3hppc_lrp_disable_bubbles( pc3hppcTestInfo->nUnit );
*/


  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;
  /* Added for B0 */
  uuIlStatsCollectionInterval = ( rev_id != BCM88030_A0_REV_ID ) ? uuTransmitDuration : 
                                                                   C3HPPC_SWS_TEST5__IL_STATS_COLLECTION_INTERVAL; 

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    if ( g_bDoDoubleRedirect == 0 ) {
      uuChannelMask = ( pc3hppcTestInfo->bOamEnable == 1 ) ? COMPILER_64_INIT(0x00010000,0x00000000) :
                                                             COMPILER_64_INIT(0xffffffff,0xffffffff);
    } else {
      uuChannelMask = ( nInstance == 1 ) ? COMPILER_64_INIT(0x0000ffff,0x00000000) :
                                           COMPILER_64_INIT(0xffffffff,0xffffffff);
    }
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1,
                                  uuChannelMask );
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }


  nSecondsCounter = 0;
  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
    uuMaxDurationB4StatsCollect = C3HPPC_MIN( uuIlStatsCollectionInterval, uuTransmitDuration );

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

        if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
          nMinPacketLen = pc3hppcTestInfo->nPacketSize;
          nMaxPacketLen = pc3hppcTestInfo->nPacketSize;
          nPacketLenMode = ILPKTGEN__FIXED;
        } else {
          nMinPacketLen = pc3hppcTestInfo->nPacketSize + 1;
          nMaxPacketLen = pc3hppcTestInfo->nPacketSize + 1;
          nPacketLenMode = ILPKTGEN__FIXED;
        }

        if ( g_bDoDoubleRedirect == 1 && nInstance == C3HPPC_TEST__LINE_INTERFACE ) uMaxChannel = 15; 
        else if ( pc3hppcTestInfo->bOamEnable == 1 ) uMaxChannel = 47; 
        else if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS ) uMaxChannel = 1; 
        else uMaxChannel = 63;
        
        c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                     0,
                                    uuMaxDurationB4StatsCollect, 
                                    ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD :
                                                                                 ILPKTGEN__PAYLOAD_PATTEN__SSO ),
                                    1,
                                    0,
                                    ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ? 0 : uMaxChannel ),
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

    if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 ) {
      c3hppc_sws_test5__bubble_generator( pc3hppcTestInfo->nUnit );
    }

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {
        c3hppc_sws_il_pktgen_start( pc3hppcTestInfo->nUnit, nInstance );
      }
    }

    bShutDown = 0;
    while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
             !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        soc_reg32_get( pc3hppcTestInfo->nUnit, g_anQmSourceQueueDropRegisters[nInstance], 
                       SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nInstance), 0, &uRegisterValue );
        COMPILER_64_ADD_32(g_auuQmSourceQueueDropCounts[nInstance], uRegisterValue);
      }

      if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
        bShutDown = 1;
      }

      READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uRegisterValue );
      g_uuBubbleCount += uRegisterValue;


      sal_usleep( 1000000 );

      ++nSecondsCounter;

      if ( !(nSecondsCounter % 60) ) {
        cli_out("\nINFO: INGRESS channel[0] current \"RPKT\" count -->   %lld\n", c3hppc_sws_il_stats_getrx( 0, 0, ILRXSTAT__PKT ) );
        cli_out("\nINFO: EGRESS channel[0] current \"RPKT\" count -->   %lld\n", c3hppc_sws_il_stats_getrx( 1, 0, ILRXSTAT__PKT ) );
      }
    }

    if ( bShutDown ) {
      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
      }
    }

    if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 ) {
      c3hppc_lrp_disable_bubbles( pc3hppcTestInfo->nUnit );
    }

    sal_sleep( 2 );
    c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );

    READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uRegisterValue );
    g_uuBubbleCount += uRegisterValue;

    if ( bShutDown ) {
      COMPILER_64_SUB_64(uuTransmitDuration, uuTransmitDuration);
    } else {
      COMPILER_64_SUB_64(uuTransmitDuration, uuMaxDurationB4StatsCollect);
    }
  }

/* Used for debug ...
  soc_sbx_caladan3_ped_hd(pc3hppcTestInfo->nUnit, 1, 1, 0, NULL, NULL);
*/

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

    soc_reg32_get( pc3hppcTestInfo->nUnit, g_anQmSourceQueueDropRegisters[nInstance], 
                   SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nInstance), 0, &uRegisterValue );
    COMPILER_64_ADD_32(g_auuQmSourceQueueDropCounts[nInstance], uRegisterValue);
  }

#endif

  return 0;
}




int
c3hppc_sws_test5__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else
  
  int nSourceInterface, nDestinationInterface, nChannel, nBaseChannel, nRedirectChannelOffset;
  int nInstance, nIlInstanceNum;
  char sMessage[16];
  uint64 uuSourceCount, uuDestinationCount, uuTotalPktCount, uuTotalByteCount, uuTemp;
  double dBWinMpps;
  uint64 uuChannels48Count;
  uint32 uReplicationCount;

  uuChannels48Count = 0;

  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  if ( g_bDoChangeDirection == 1 ) {

    uuTotalPktCount = 0;
    uuTotalByteCount = 0;
 
    for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {

      uReplicationCount = g_pFlowTable[(0+nChannel)].uData[0];
      if ( g_bDoAlternatingUnload == 1 ) uReplicationCount /= 2; 
      uuSourceCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__LINE_INTERFACE, nChannel, ILRXSTAT__PKT );
      COMPILER_64_UMUL_32(uuSourceCount,uReplicationCount);

      uReplicationCount = g_pFlowTable[(64+nChannel)].uData[0];
      if ( g_bDoAlternatingUnload == 1 ) uReplicationCount /= 2; 
      uuTemp = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__PKT );
      COMPILER_64_UMUL_32(uuTemp,uReplicationCount);
      uuSourceCount += uuTemp;

      uuDestinationCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__PKT );
      uuDestinationCount += c3hppc_sws_il_stats_gettx( C3HPPC_TEST__LINE_INTERFACE,  nChannel, ILTXSTAT__PKT );
      uuTotalPktCount += uuDestinationCount;
      if ( COMPILER_64_NE(uuDestinationCount, uuSourceCount) ) {
        if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 && nChannel == 48 ) {
          uuChannels48Count += uuDestinationCount;
        } else {
          cli_out("\nERROR: Channels[%d and %d] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                  nChannel, (64+nChannel), uuDestinationCount, uuSourceCount );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      }

      uReplicationCount = g_pFlowTable[(0+nChannel)].uData[0];
      if ( g_bDoAlternatingUnload == 1 ) uReplicationCount /= 2; 
      uuSourceCount = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__LINE_INTERFACE, nChannel, ILRXSTAT__BYTE );
      COMPILER_64_UMUL_32(uuSourceCount,uReplicationCount);

      uReplicationCount = g_pFlowTable[(64+nChannel)].uData[0];
      if ( g_bDoAlternatingUnload == 1 ) uReplicationCount /= 2; 
      uuTemp = c3hppc_sws_il_stats_getrx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILRXSTAT__BYTE );
      COMPILER_64_UMUL_32(uuTemp,uReplicationCount);
      uuSourceCount += uuTemp;

      uuDestinationCount = c3hppc_sws_il_stats_gettx( C3HPPC_TEST__FABRIC_INTERFACE, nChannel, ILTXSTAT__BYTE );
      uuDestinationCount += c3hppc_sws_il_stats_gettx( C3HPPC_TEST__LINE_INTERFACE,  nChannel, ILTXSTAT__BYTE );
      uuTotalByteCount += uuDestinationCount;
      if ( COMPILER_64_NE(uuDestinationCount, uuSourceCount) ) {
        if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 && nChannel == 48 ) {
          uuChannels48Count += uuDestinationCount;
        } else {
          cli_out("\nERROR: Channels[%d and %d] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                  nChannel, (64+nChannel), uuDestinationCount, uuSourceCount );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      }

    }

    dBWinMpps = (double) ( ((double) uuTotalPktCount) / 
                               ((double) (pc3hppcTestInfo->uuIterations / (( SAL_BOOT_QUICKTURN ) ? 1000.0 : 1.0))) );
    dBWinMpps /= 1000000.0;
    cli_out("INFO: Aggregate bandwidth --> %.2fMpps with a total PKT/BYTE counts of --> %lld/%lld \n\n",
            dBWinMpps, uuTotalPktCount, uuTotalByteCount );

  } else {

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {

      nRedirectChannelOffset = 0;

      if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
        /* coverity[secure_coding] */
        sal_strcpy( sMessage, "INGRESS" );
        nSourceInterface = C3HPPC_TEST__LINE_INTERFACE;
        nDestinationInterface = C3HPPC_TEST__FABRIC_INTERFACE;
        nBaseChannel = 0;
        if ( g_bDoDoubleRedirect == 1 ) nRedirectChannelOffset = 32;
      } else { 
        /* coverity[secure_coding] */
        sal_strcpy( sMessage, "EGRESS" );
        nSourceInterface = C3HPPC_TEST__FABRIC_INTERFACE;
        nDestinationInterface = C3HPPC_TEST__LINE_INTERFACE;
        nBaseChannel = C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM;
      }
  
      uuTotalPktCount = 0;
      uuTotalByteCount = 0;
  
      for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
        if ( nRedirectChannelOffset && nChannel == 16 ) break;
  
        uReplicationCount = g_pFlowTable[(nBaseChannel+nChannel)].uData[0];
        if ( g_bDoAlternatingUnload == 1 ) uReplicationCount /= 2; 
  
        uuSourceCount = c3hppc_sws_il_stats_getrx( nSourceInterface, nChannel, ILRXSTAT__PKT );
        COMPILER_64_UMUL_32(uuSourceCount,uReplicationCount);
        uuDestinationCount = c3hppc_sws_il_stats_gettx( nDestinationInterface, (nRedirectChannelOffset + nChannel), ILTXSTAT__PKT );
        uuTotalPktCount += uuDestinationCount;
        if ( COMPILER_64_NE(uuDestinationCount, uuSourceCount) ) {
          if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 && nChannel == 48 ) {
            uuChannels48Count += uuDestinationCount;
          } else {
            cli_out("\nERROR: %s Channel[%d] \"PKT\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                    sMessage, nChannel, uuDestinationCount, uuSourceCount );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
        }
  
        uuSourceCount = c3hppc_sws_il_stats_getrx( nSourceInterface, nChannel, ILRXSTAT__BYTE );
        COMPILER_64_UMUL_32(uuSourceCount,uReplicationCount);
        uuDestinationCount = c3hppc_sws_il_stats_gettx( nDestinationInterface, (nRedirectChannelOffset + nChannel), ILTXSTAT__BYTE );
        uuTotalByteCount += uuDestinationCount;
        if ( COMPILER_64_NE(uuDestinationCount, uuSourceCount) ) {
          if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 && nChannel == 48 ) {
          } else {
            cli_out("\nERROR: %s Channel[%d] \"BYTE\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
                    sMessage, nChannel, uuDestinationCount, uuSourceCount );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
        }
      }
  
      dBWinMpps = (double) ( ((double) uuTotalPktCount) / 
                                 ((double) (pc3hppcTestInfo->uuIterations / (( SAL_BOOT_QUICKTURN ) ? 1000.0 : 1.0))) );
      dBWinMpps /= 1000000.0;
      cli_out("INFO: %s bandwidth --> %.2fMpps with a total PKT/BYTE counts of --> %lld/%lld \n\n",
              sMessage, dBWinMpps, uuTotalPktCount, uuTotalByteCount );
      
    }

  }

  if ( g_bDoDoubleRedirect == 0 && pc3hppcTestInfo->bOamEnable == 1 ) {
    if ( g_uuBubbleCount != uuChannels48Count ) { 
      cli_out("\nERROR: Redirection \"PACKET\" count MISCOMPARE  -->  Actual: %lld   Expect: %lld\n",
              uuChannels48Count, g_uuBubbleCount );
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }
  }


  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
#endif
}


int c3hppc_sws_test5__bubble_generator( int nUnit ) {

  uint32 uRegisterValue, uBubbleTableSize, u512nsInterval;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  c3hppc_lrp_bubble_state_entry_ut BubbleState;
  int nOcmPort, nBubble, nIntervalIndex;
  uint32 auBubbleIntervalTableEntry[1];


  uBubbleTableSize = 128;
  nIntervalIndex = 64;
  auBubbleIntervalTableEntry[0] = 0;
  u512nsInterval = 1000;

  COMPILER_64_ZERO(BubbleState.value);
  BubbleState.bits.Init = 1;
  BubbleState.bits.Mode = C3HPPC_LRP_BUBBLE_UPDATE_MODE__CONTINUOUS;
  BubbleState.bits.IntervalIndex = nIntervalIndex;

  /* Initialize bubble table entries  */
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(nUnit,
                                                                uBubbleTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  for ( nBubble = 0; nBubble < (int) uBubbleTableSize; ++nBubble ) {
    BubbleState.bits.Task = sal_rand() % C3HPPC_LRP__TASK_NUM;
    BubbleState.bits.Stream = 1 + BubbleState.bits.Task; 
    pOcmBlock[nBubble].uData[1] = COMPILER_64_HI(BubbleState.value);
    pOcmBlock[nBubble].uData[0] = COMPILER_64_LO(BubbleState.value);
  }
  nOcmPort = c3hppc_ocm_map_bubble2ocm_port(0);
  c3hppc_ocm_dma_read_write( nUnit, nOcmPort, 0, 0, (uBubbleTableSize-1), 1, pOcmBlock->uData );
  soc_cm_sfree( nUnit, pOcmBlock );

  soc_mem_field_set( nUnit, LRB_BUBBLE_INTERVAL_TABLEm, auBubbleIntervalTableEntry, INTERVALf,  &u512nsInterval );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, LRB_BUBBLE_INTERVAL_TABLEm, MEM_BLOCK_ANY,
                                     nIntervalIndex, auBubbleIntervalTableEntry) );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uRegisterValue, ENABLEf, 1 );
  soc_reg_field_set( nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uRegisterValue, SIZEf, uBubbleTableSize );
  WRITE_LRB_BUBBLE_TABLE_CONFIGr( nUnit, uRegisterValue );

  return 0;
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
