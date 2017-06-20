/*
 * $Id: c3hppc_sws_test2.c,v 1.5.14.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT


#include "../c3hppc_test.h"

#include <soc/sbx/caladan3/ped.h>

 
#define C3HPPC_SWS_TEST2__STREAM_NUM                                (1)
#define C3HPPC_SWS_TEST2__IL_STATS_COLLECTION_INTERVAL              (2000)

static uint64 g_auuQmSourceQueueDropCounts[C3HPPC_SWS_IL_INSTANCE_NUM];
static int g_anQmSourceQueueDropRegisters[] = { QM_INGRESS_SQ_DROP_COUNTr, QM_EGRESS_SQ_DROP_COUNTr };


int
c3hppc_sws_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance;
  
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    COMPILER_64_ZERO(g_auuQmSourceQueueDropCounts[nInstance]);
  }

  
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
  }
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
  /* coverity[secure_coding] */
  sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test2.oasm");

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  return rc;
}

int
c3hppc_sws_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nInstance;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  int nMinPacketLen, nMaxPacketLen, nPacketLenMode;
  uint32 auSourceQueueConfigEntry[3], uFieldValue;
  int nSourceQueue;
  uint32 uRegisterValue;
  uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;

/*
  WRITE_PD_VALID_SQUEUESr( pc3hppcTestInfo->nUnit, 0, 0xfffffffe );
  WRITE_PD_CHECKSr( pc3hppcTestInfo->nUnit, 0x00000080 );
*/

  for ( nSourceQueue = 0; nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nSourceQueue ) {
    SOC_IF_ERROR_RETURN(soc_mem_read( pc3hppcTestInfo->nUnit, QM_SOURCE_QUEUE_CONFIGm,
                                      MEM_BLOCK_ANY, nSourceQueue, auSourceQueueConfigEntry) );

    uFieldValue = ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ) ? 7083 : 40;
    soc_mem_field_set( pc3hppcTestInfo->nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );

    SOC_IF_ERROR_RETURN(soc_mem_write( pc3hppcTestInfo->nUnit, QM_SOURCE_QUEUE_CONFIGm,
                                       MEM_BLOCK_ANY, nSourceQueue, auSourceQueueConfigEntry) );
  }
  


  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, continuous_epochs );

  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    uint64 allOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1,
                                  allOnes );
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }

  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
    uint32 tmp = C3HPPC_MIN( C3HPPC_SWS_TEST2__IL_STATS_COLLECTION_INTERVAL, COMPILER_64_LO(uuTransmitDuration) );
    COMPILER_64_SET(uuMaxDurationB4StatsCollect,0,tmp);

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

        if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
          nMinPacketLen = 257;
          nMaxPacketLen = 513;
          nPacketLenMode = ILPKTGEN__PINGPONG;
        } else {
          nMinPacketLen = 48;
          nMaxPacketLen = 8192;
          nPacketLenMode = ILPKTGEN__RANDOM;
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

      sal_usleep( 1000000 );

    }

    if ( bShutDown ) {
      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
      }
    }

    sal_sleep( 2 );
    c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );

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


  return 0;
}




int
c3hppc_sws_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else
  
  int nSourceInterface, nDestinationInterface, nChannel;
  int nInstance, nIlInstanceNum;
  char sMessage[16];
  uint64 uuSourcePktCount, uuDestinationPktCount, uuTmp;


  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {

    if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
      /* coverity[secure_coding] */
      sal_strcpy( sMessage, "INGRESS" );
      nSourceInterface = C3HPPC_TEST__LINE_INTERFACE;
      nDestinationInterface = C3HPPC_TEST__FABRIC_INTERFACE;
    } else { 
      /* coverity[secure_coding] */
      sal_strcpy( sMessage, "EGRESS" );
      nSourceInterface = C3HPPC_TEST__FABRIC_INTERFACE;
      nDestinationInterface = C3HPPC_TEST__LINE_INTERFACE;
    }

    COMPILER_64_ZERO(uuSourcePktCount);
    COMPILER_64_ZERO(uuDestinationPktCount);

    for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
      uuTmp = c3hppc_sws_il_stats_getrx( nSourceInterface, nChannel, ILRXSTAT__PKT );
      COMPILER_64_ADD_64(uuSourcePktCount,uuTmp);
      uuTmp = c3hppc_sws_il_stats_gettx( nDestinationInterface, nChannel, ILTXSTAT__PKT );
      COMPILER_64_ADD_64(uuDestinationPktCount, uuTmp);
    }
    
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDestinationPktCount), COMPILER_64_LO(uuDestinationPktCount));
    COMPILER_64_ADD_64(uuTmp, g_auuQmSourceQueueDropCounts[nInstance]);
    cli_out("\nINFO: %s src[%lld] drop[%lld] dest[%lld] dest+drop[%lld]counts \n\n", sMessage, 
            uuSourcePktCount, g_auuQmSourceQueueDropCounts[nInstance], uuDestinationPktCount, uuTmp);

    if ( COMPILER_64_LT(uuDestinationPktCount, g_auuQmSourceQueueDropCounts[nInstance]) ) {
      cli_out("\nERROR: %s src[%lld] count LESS THAN dest[%lld] count! \n\n", sMessage, 
              uuSourcePktCount, uuDestinationPktCount ); 
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }

    if ( COMPILER_64_IS_ZERO(g_auuQmSourceQueueDropCounts[nInstance]) ) {
      cli_out("\nERROR: %s drop[%lld] count is ZERO! \n\n", sMessage, g_auuQmSourceQueueDropCounts[nInstance] ); 
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }

  }

  return 0;
#endif
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
