/*
 * $Id: c3hppc_sws_test1.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"

 
#define C3HPPC_SWS_TEST1__STREAM_NUM                                (1)
#define C3HPPC_SWS_TEST1__IL_STATS_COLLECTION_INTERVAL              (2000)


static int  wait_for_flowcontrol_status_bit( int nUnit, int nIlInstance, uint32 uBackPressureQueue );


int
c3hppc_sws_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance;
  

  
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
  }
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);

  if ( pc3hppcTestInfo->bLineTrafficGenOnly && !pc3hppcTestInfo->bFabricTrafficGenOnly ) {
    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test1_egress.oasm");
  } else if ( pc3hppcTestInfo->bFabricTrafficGenOnly && !pc3hppcTestInfo->bLineTrafficGenOnly ) {
    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test1_ingress.oasm");
  } else {
    cli_out("\n ERROR:  BAD TEST PARAMETERS  .... \n");
    return 1;
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  return rc;
}

int
c3hppc_sws_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nInstance;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  uint32 uBackPressureQueue;
  uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;


  /* Initialize ucode backpressure control register before traffic starts. */
  uBackPressureQueue = ( pc3hppcTestInfo->bLineTrafficGenOnly ) ? 0 : 0x7f;
  c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 0, uBackPressureQueue );


  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, continuous_epochs );

  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      uint64 uuOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1,
                                  uuOnes );
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }

  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
    uint32 tmp =  C3HPPC_MIN( C3HPPC_SWS_TEST1__IL_STATS_COLLECTION_INTERVAL, COMPILER_64_LO(uuTransmitDuration) );
    COMPILER_64_SET(uuMaxDurationB4StatsCollect, 0, tmp);

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

        c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                     0,
                                    uuMaxDurationB4StatsCollect, 
                                    ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD :
                                                                                 ILPKTGEN__PAYLOAD_PATTEN__SSO ),
                                    1,
                                    0, (C3HPPC_SWS_IL_MAX_CHANNEL_NUM - 1), ILPKTGEN__INCREMENT,
                                    64, 64, ILPKTGEN__FIXED,
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
    nInstance = ( pc3hppcTestInfo->bLineTrafficGenOnly ) ? C3HPPC_TEST__FABRIC_INTERFACE : C3HPPC_TEST__LINE_INTERFACE;
    while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
             !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

      if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
        bShutDown = 1;
      }

      uBackPressureQueue = sal_rand() % C3HPPC_SWS_SOURCE_QUEUE_NUM;
      if ( nInstance == C3HPPC_TEST__FABRIC_INTERFACE ) uBackPressureQueue |= 0x40;
      else uBackPressureQueue &= 0x3f;

      c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 0, uBackPressureQueue );

      if ( !wait_for_flowcontrol_status_bit( pc3hppcTestInfo->nUnit, nInstance, uBackPressureQueue ) ) {
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        cli_out("\nERROR:  Flowcontrol NOT seen for Source Queue --> %d\n", uBackPressureQueue );
        bShutDown = 1;
      } else {
        cli_out("\nINFO:  Flowcontrol seen for Source Queue --> %d\n", uBackPressureQueue );
      }
      

      uBackPressureQueue = ( pc3hppcTestInfo->bLineTrafficGenOnly ) ? 0 : 0x7f;
      c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 0, uBackPressureQueue );

      if ( !bShutDown ) {
        sal_usleep( 1000000 );
        soc_reg32_set( pc3hppcTestInfo->nUnit, IL_FLOWCONTROL_RXFC_STS0r, SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nInstance), 0, 0xffffffff );
        soc_reg32_set( pc3hppcTestInfo->nUnit, IL_FLOWCONTROL_RXFC_STS1r, SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nInstance), 0, 0xffffffff );
      }


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
c3hppc_sws_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
/*
  int nInstance, nIlInstanceNum;


  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;
  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    c3hppc_sws_il_stats_display_nonzero( nInstance );
  }
*/

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


  return 0;
}


int  wait_for_flowcontrol_status_bit( int nUnit, int nIlInstance, uint32 uBackPressureQueue ) {
 
  int nTimeOut;
  uint32 uRegisterValue;
  uint64 uuBackPressureQueueMask = COMPILER_64_INIT(0x00000000,0x00000001), uuRxfcMask;

  if ( nIlInstance == 1 ) uBackPressureQueue &= 0x3f;
  COMPILER_64_SHL(uuBackPressureQueueMask, uBackPressureQueue);

/*
  cli_out("\n uuBackPressureQueueMask 0x%llx   uBackPressureQueue %d  nIlInstance %d \n", uuBackPressureQueueMask, uBackPressureQueue, nIlInstance );
*/
  
  nTimeOut = 1000;
  while ( (--nTimeOut) ) {
    uint32 uRegisterValue2;
    soc_reg32_get( nUnit, IL_FLOWCONTROL_RXFC_STS0r, SOC_BLOCK_PORT(nUnit,nIlInstance), 0, &uRegisterValue );
    soc_reg32_get( nUnit, IL_FLOWCONTROL_RXFC_STS1r, SOC_BLOCK_PORT(nUnit,nIlInstance), 0, &uRegisterValue2 );
    COMPILER_64_SET(uuRxfcMask, uRegisterValue2, uRegisterValue);
    if ( COMPILER_64_EQ(uuBackPressureQueueMask, uuRxfcMask) ) break; 
    sal_usleep( 1000 );
  }

/*
  cli_out("\n  uuRxfcMask  0x%llx  \n", uuRxfcMask );
*/

  return nTimeOut;
}


#endif /* #ifdef BCM_CALADAN3_SUPPORT */
