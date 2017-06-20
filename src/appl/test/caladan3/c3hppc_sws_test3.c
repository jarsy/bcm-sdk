/*
 * $Id: c3hppc_sws_test3.c,v 1.4.30.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
#include <bcm/error.h>
#include <bcm/tx.h>
#include <bcm/rx.h>
#include <bcm/pkt.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/ppe.h>

 
#define C3HPPC_SWS_TEST3__STREAM_NUM                                (1)
#define C3HPPC_SWS_TEST3__IL_STATS_COLLECTION_INTERVAL              (2000)
#define C3HPPC_SWS_TEST3__CMIC_MAX_PACKET_SIZE                      (16384)
#define C3HPPC_SWS_TEST3__PACKET_CRC_SIZE                           (4)
#define C3HPPC_SWS_TEST3__ROUTE_HEADER_BASE_SIZE                    (16)
#define C3HPPC_SWS_TEST3__ROUTE_HEADER_TYPE                         (0x10)
#define C3HPPC_SWS_TEST3__L2_HEADER_SIZE                            (16)
#define C3HPPC_SWS_TEST3__L2_HEADER_TYPE                            (0x11)
#define C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE                          (20)
#define C3HPPC_SWS_TEST3__IPV4_HEADER_TYPE                          (0x12)
#define C3HPPC_SWS_TEST3__FRAGMENT_CONTROL_HEADER_TYPE              (0x13)


bcm_rx_t c3hppc_sws_test3__cmic_rx_cb( int unit, bcm_pkt_t *pkt, void *cookie );
void c3hppc_sws_test3__cmic_tx_setup( bcm_pkt_t *pPktInfo, uint8 *pPacketBuf );
int c3hppc_sws_test3__cmic_send_packet( int nUnit, bcm_pkt_t *pPktInfo, uint32 *pPacketBuf, int nPacketLength,
                                        uint32 uPacketID, int nL2ExtensionHdr );
int c3hppc_sws_test3__sws_pp_pd_modifications( int nUnit );
uint32 c3hppc_sws_test3__ipv4_checksum_calc( uint8 *pIpv4Header );

int g_HdrStackLength, g_nExpectRxPacketLength, g_nRxPacketDone;
uint8 *g_pExpectPacketBuf, *g_pReAssemblePacketBuf;
uint32 *g_pExpectFrag1HdrStack;


int
c3hppc_sws_test3__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance;
  

  
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
  }
  pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL;
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;
  pc3hppcTestInfo->BringUpControl.bSwsBypassPrParsing = 1;
  pc3hppcTestInfo->BringUpControl.bXLportAndCmicPortsActive = 1;
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
  /* coverity[secure_coding] */
  sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "sws_test3.oasm");

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  rc += c3hppc_sws_test3__sws_pp_pd_modifications( pc3hppcTestInfo->nUnit );

  return rc;
}

int
c3hppc_sws_test3__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nInstance, rc;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  int nMinPacketLen, nMaxPacketLen, nPacketLenMode;
  uint32 *pPacketBuf, uPacketID;
  int nPacketLength;
  bcm_pkt_t PktInfo;
  int nL2ExtensionHdr;
  int nRxPacketDoneLocal;
  uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;

  bcm_rx_queue_channel_set(pc3hppcTestInfo->nUnit, -1, 1);

  if ( (rc = bcm_rx_start(pc3hppcTestInfo->nUnit, NULL)) < 0 ) {
    cli_out("bcm_rx_start: Error: Cannot start RX: %s.\n", bcm_errmsg(rc) );
    return TEST_FAIL;
  }

  rc = bcm_rx_register( pc3hppcTestInfo->nUnit, "CMIC Rx CB",
                             c3hppc_sws_test3__cmic_rx_cb, 0x40, (void*)pc3hppcTestInfo, BCM_RCO_F_ALL_COS);
/*
                             c3hppc_sws_test3__cmic_rx_cb, 0x40, (void*)0x0c00c613, BCM_RCO_F_ALL_COS);
*/
  if ( rc < 0 ) {
    cli_out("ERROR: bcm_register(%d) failed:%d:%s\n",
            pc3hppcTestInfo->nUnit, rc, bcm_errmsg(rc));
    return TEST_FAIL;
  }
  pPacketBuf = (uint32 *) soc_cm_salloc( pc3hppcTestInfo->nUnit, C3HPPC_SWS_TEST3__CMIC_MAX_PACKET_SIZE, "packet buffer" );
  c3hppc_sws_test3__cmic_tx_setup( &PktInfo, (uint8 *)pPacketBuf );

  g_pExpectPacketBuf = (uint8 *) pPacketBuf; 
  g_pExpectFrag1HdrStack = (uint32 *) soc_cm_salloc( pc3hppcTestInfo->nUnit, (16 * sizeof(uint32)), "g_pExpectFrag1HdrStack" );
  g_pReAssemblePacketBuf = (uint8 *) soc_cm_salloc( pc3hppcTestInfo->nUnit, C3HPPC_SWS_TEST3__CMIC_MAX_PACKET_SIZE,
                                                    "reassemble packet buffer" );

  soc_sbx_caladan3_ppe_hc_control( pc3hppcTestInfo->nUnit, 0, 1, -1, -1, 0, 0 );
  soc_sbx_caladan3_ppe_hc_control( pc3hppcTestInfo->nUnit, 1, 0, 0x3f, -1, 0, 0 );

  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, continuous_epochs );




  uuTransmitDuration = pc3hppcTestInfo->uuIterations;
  nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;

  for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
    uint64 allOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
    if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
      c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1, allOnes);
      c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
    }
  }

  uPacketID = 0;
  g_nExpectRxPacketLength = 0;
  g_nRxPacketDone = 0;
  nRxPacketDoneLocal = g_nRxPacketDone;
  while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
    uint32 tmp = C3HPPC_MIN( C3HPPC_SWS_TEST3__IL_STATS_COLLECTION_INTERVAL, COMPILER_64_LO(uuTransmitDuration) );
    COMPILER_64_SET(uuMaxDurationB4StatsCollect, 0, tmp);

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
           (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {

/*
        nMinPacketLen = 64;
        nMaxPacketLen = 64; 
        nPacketLenMode = ILPKTGEN__FIXED;
*/
        nMinPacketLen = 64;
        nMaxPacketLen = 1024; 
        nPacketLenMode = ILPKTGEN__RANDOM;
        
        c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                    ( pc3hppcTestInfo->nSetupOptions ),
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
    nL2ExtensionHdr = 1;
    while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
             !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

      if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
        bShutDown = 1;
      }

      if ( g_nRxPacketDone == nRxPacketDoneLocal ) {
        ++nRxPacketDoneLocal;
        nPacketLength = sal_rand() % (C3HPPC_SWS_TEST3__CMIC_MAX_PACKET_SIZE-C3HPPC_SWS_TEST3__PACKET_CRC_SIZE);
        nPacketLength = C3HPPC_MAX( 64, nPacketLength ); 
  
/* Hang occurs waiting for interrupt if packet size is in the >15K range. */
 nPacketLength = C3HPPC_MIN( 14000, nPacketLength ); 
  
        g_nExpectRxPacketLength = nPacketLength + C3HPPC_SWS_TEST3__PACKET_CRC_SIZE;  /* CRC added */
        g_HdrStackLength = C3HPPC_SWS_TEST3__L2_HEADER_SIZE + nL2ExtensionHdr + C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE;
        if ( g_nExpectRxPacketLength >= 1024 ) {
          g_nExpectRxPacketLength += g_HdrStackLength;
        }
        c3hppc_sws_test3__cmic_send_packet( pc3hppcTestInfo->nUnit, &PktInfo, pPacketBuf, 
                                            nPacketLength, uPacketID, nL2ExtensionHdr );
        uPacketID = (uPacketID + 1) & 0xffff;
        ++nL2ExtensionHdr;
        if ( nL2ExtensionHdr == 9 ) nL2ExtensionHdr = 1;
      }

      sal_usleep( 100000 );

    }

    if ( bShutDown ) {
      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
      }
    }

    sal_sleep( 2 );
	/* coverity[ stack_use_overflow ] */
    c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );
    
    if ( bShutDown ) {
      COMPILER_64_SUB_64(uuTransmitDuration, uuTransmitDuration);
    } else {
      COMPILER_64_SUB_64(uuTransmitDuration, uuMaxDurationB4StatsCollect);
    }
  }

  soc_sbx_caladan3_ppe_hd( pc3hppcTestInfo->nUnit, 1, NULL );
  soc_sbx_caladan3_ped_hd(pc3hppcTestInfo->nUnit, 1, 1, 0, NULL, NULL);


  for ( nInstance = 0; ( nInstance < nIlInstanceNum ); ++nInstance ) {
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


  soc_cm_sfree( pc3hppcTestInfo->nUnit, pPacketBuf );
  soc_cm_sfree( pc3hppcTestInfo->nUnit, g_pExpectFrag1HdrStack );
  soc_cm_sfree( pc3hppcTestInfo->nUnit, g_pReAssemblePacketBuf );

  if ( g_nRxPacketDone != nRxPacketDoneLocal ) {
    cli_out("\n\nERROR:  Host packet NOT returned!\n\n");
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
  }


  return 0;
}




int
c3hppc_sws_test3__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc; 
  uint32 uRegisterValue;

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


  rc = bcm_rx_stop(pc3hppcTestInfo->nUnit, NULL);
  if ( rc < 0 ) {
    cli_out("ERROR:  bcm_rx_stop(%d) returns %d: %s\n",
            pc3hppcTestInfo->nUnit, rc, bcm_errmsg(rc));
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
  }

  uRegisterValue = 0;
  soc_reg_field_set( pc3hppcTestInfo->nUnit, PD_EVENTr, &uRegisterValue, COPY_OP_COMPLETEf, 1 );
  soc_reg_field_set( pc3hppcTestInfo->nUnit, PD_EVENTr, &uRegisterValue, COPY_OVERFLOWf, 1 );
  WRITE_PD_EVENTr( pc3hppcTestInfo->nUnit, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( pc3hppcTestInfo->nUnit, PP_GLOBAL_EVENTr, &uRegisterValue, COPY_OUT_INTERRUPTf, 1 );
  WRITE_PP_GLOBAL_EVENTr( pc3hppcTestInfo->nUnit, uRegisterValue );

  return 0;
}


bcm_rx_t c3hppc_sws_test3__cmic_rx_cb(int unit, bcm_pkt_t *pkt, void *cookie)
{

  int q_depth, nIndex;
  uint32 *pActualPacketBuf, *pExpectPacketBuf;
  static int nFrag0Length = 0;
  static int nFrag1Length = 0;
  static int nPacketID = 0;
  int nPacketAssembledLength;
  c3hppc_test_info_t *pc3hppcTestInfo;
  uint32 uIpv4HdrOffset, uFrag0Ipv4TotalLength, uIpv4Checksum;
  uint32 uFrag1Ipv4TotalLength, uFrag1Ipv4FragmentOffset, uOriginalIpv4TotalLength;

  pc3hppcTestInfo = (c3hppc_test_info_t *) cookie;

  bcm_rx_queue_packet_count_get(unit, 0, &q_depth);

/*
  cli_out("c3hppc_sws_test3__cmic_rx_cb: pkt len=%d rxUnit=%d rxPort=%d rxReason=%d qlen=%d id=0x%04x\n",
          pkt->pkt_len, pkt->rx_unit, pkt->rx_port,
          pkt->rx_reason, q_depth, nPacketID);
*/


  if ( nFrag0Length == 0 ) {
    nFrag0Length = pkt->pkt_len;
    sal_memcpy( g_pReAssemblePacketBuf, pkt->pkt_data->data, nFrag0Length );
    uIpv4HdrOffset = g_HdrStackLength - C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE; 
    if ( g_nExpectRxPacketLength >= 1024 ) {
      sal_memcpy( &uOriginalIpv4TotalLength, g_pExpectPacketBuf + uIpv4HdrOffset + 2, 2 ); 
      uOriginalIpv4TotalLength >>= 16;
      uFrag0Ipv4TotalLength = nFrag0Length - uIpv4HdrOffset;
      uFrag1Ipv4TotalLength = uOriginalIpv4TotalLength - uFrag0Ipv4TotalLength + C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE; 
      uFrag1Ipv4FragmentOffset = uFrag0Ipv4TotalLength / 8;
      uFrag0Ipv4TotalLength <<= 16;
      sal_memcpy( g_pExpectPacketBuf + uIpv4HdrOffset + 2, &uFrag0Ipv4TotalLength, 2 ); 
      sal_memset( g_pExpectPacketBuf + uIpv4HdrOffset + 6, 0x00, 2 ); 

      uFrag1Ipv4TotalLength <<= 16;
      sal_memcpy( ((uint8 *)g_pExpectFrag1HdrStack) + uIpv4HdrOffset + 2, &uFrag1Ipv4TotalLength, 2 ); 
      uFrag1Ipv4FragmentOffset <<= 16;
      sal_memcpy( ((uint8 *)g_pExpectFrag1HdrStack) + uIpv4HdrOffset + 6, &uFrag1Ipv4FragmentOffset, 2 ); 
      uIpv4Checksum = c3hppc_sws_test3__ipv4_checksum_calc( ((uint8 *) g_pExpectFrag1HdrStack) + uIpv4HdrOffset );
      sal_memcpy( ((uint8 *)g_pExpectFrag1HdrStack) + uIpv4HdrOffset + 10, &uIpv4Checksum, 2 ); 
    }
    uIpv4Checksum = c3hppc_sws_test3__ipv4_checksum_calc( g_pExpectPacketBuf + uIpv4HdrOffset );
    sal_memcpy( g_pExpectPacketBuf + uIpv4HdrOffset + 10, &uIpv4Checksum, 2 ); 
  } else {
    nFrag1Length = pkt->pkt_len;
    sal_memcpy( g_pReAssemblePacketBuf+nFrag0Length, pkt->pkt_data->data+g_HdrStackLength,
                (nFrag1Length - g_HdrStackLength) );

    if ( sal_memcmp( g_pExpectFrag1HdrStack, pkt->pkt_data->data, g_HdrStackLength ) ) {
      pActualPacketBuf = (uint32 *) pkt->pkt_data->data;
      pExpectPacketBuf = (uint32 *) g_pExpectFrag1HdrStack;
      for ( nIndex = 0;  nIndex < (g_HdrStackLength+3)/4; ++nIndex ) {
        if ( pActualPacketBuf[nIndex] != pExpectPacketBuf[nIndex] ) {
          cli_out("c3hppc_sws_test3__cmic_rx_cb: pkt_id[0x%04x] FRAG1 hdr_word[%d] MISCOMPARE --> Actual: 0x%08x  Expect: 0x%08x \n",
                  nPacketID, nIndex, pActualPacketBuf[nIndex], pExpectPacketBuf[nIndex] );
        }
      }
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }
  }

  if ( nFrag1Length != 0 || g_nExpectRxPacketLength < 1024 ) {
    nPacketAssembledLength = nFrag0Length + nFrag1Length; 
    nFrag0Length = 0;
    nFrag1Length = 0;
    if ( nPacketAssembledLength != g_nExpectRxPacketLength ) {
      cli_out("c3hppc_sws_test3__cmic_rx_cb: Rx packet length MISCOMPARE --> Actual: %d  Expect: %d\n",
              nPacketAssembledLength, g_nExpectRxPacketLength );
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }

    if ( sal_memcmp( g_pReAssemblePacketBuf, g_pExpectPacketBuf, g_nExpectRxPacketLength-g_HdrStackLength-4 ) ) {
      pActualPacketBuf = (uint32 *) g_pReAssemblePacketBuf;
      pExpectPacketBuf = (uint32 *) g_pExpectPacketBuf;
      for ( nIndex = 0; ( nIndex < (g_nExpectRxPacketLength-g_HdrStackLength-4+3)/4 ); ++nIndex ) {
        if ( pActualPacketBuf[nIndex] != pExpectPacketBuf[nIndex] ) {
          cli_out("c3hppc_sws_test3__cmic_rx_cb: pkt_id[0x%04x] word[%d] MISCOMPARE --> Actual: 0x%08x  Expect: 0x%08x \n",
                  nPacketID, nIndex, pActualPacketBuf[nIndex], pExpectPacketBuf[nIndex] );
        }
      }
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }

    nPacketID = (nPacketID + 1) & 0xffff;

    ++g_nRxPacketDone;

  } 


  bcm_rx_free(unit, pkt->pkt_data->data);
  pkt->pkt_data->data = NULL;

  return BCM_RX_HANDLED;
}

void c3hppc_sws_test3__cmic_tx_setup( bcm_pkt_t *pPktInfo, uint8 *pPacketBuf )
{

  sal_memset(pPktInfo, 0, sizeof(bcm_pkt_t));
  pPktInfo->pkt_data = &(pPktInfo->_pkt_data);

  pPktInfo->blk_count = 1;
  pPktInfo->_pkt_data.data = pPacketBuf;

  pPktInfo->dest_port = 50;
  pPktInfo->cos = 0;
  pPktInfo->flags = BCM_TX_CRC_ALLOC;
  pPktInfo->opcode = BCM_PKT_OPCODE_CPU;

  return;
}

int c3hppc_sws_test3__cmic_send_packet( int nUnit, bcm_pkt_t *pPktInfo, uint32 *pPacketBuf,
                                        int nPacketLength, uint32 uPacketID, int nL2ExtensionHdr )
{
  int nIndex, rc;
  uint32 uIpv4TotalLength;
  

  for ( nIndex = 0; nIndex < ((nPacketLength+3)/4); ++nIndex ) {
    pPacketBuf[nIndex] = (uPacketID << 16) | nIndex;
  }

  
  uIpv4TotalLength = nPacketLength - C3HPPC_SWS_TEST3__L2_HEADER_SIZE - nL2ExtensionHdr;

  pPacketBuf[0] = g_pExpectFrag1HdrStack[0] = 0xdadadada;
  pPacketBuf[1] = g_pExpectFrag1HdrStack[1] = 0xdada5a5a;
  pPacketBuf[2] = g_pExpectFrag1HdrStack[2] = 0x5a5a5a5a;
  pPacketBuf[3] = g_pExpectFrag1HdrStack[3] = nL2ExtensionHdr;
  if ( nL2ExtensionHdr == 1 ) {
    pPacketBuf[4] =  0x1145ff00 | (uIpv4TotalLength >> 8);
    pPacketBuf[5] =  0x00ffffff | (uIpv4TotalLength << 24);
    pPacketBuf[6] =  0xffffffff;
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xee000000;
    g_pExpectFrag1HdrStack[4] =  0x1145cccc;
    g_pExpectFrag1HdrStack[5] =  0xcccccccc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xee000000;
  } else if ( nL2ExtensionHdr == 2 ) {
    pPacketBuf[4] =  0x222245ff;
    pPacketBuf[5] =  0x0000ffff | (uIpv4TotalLength << 16);
    pPacketBuf[6] =  0xffffffff;
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffee0000;
    g_pExpectFrag1HdrStack[4] =  0x222245cc;
    g_pExpectFrag1HdrStack[5] =  0x0000cccc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xccee0000;
  } else if ( nL2ExtensionHdr == 3 ) {
    pPacketBuf[4] =  0x33333345;
    pPacketBuf[5] =  0xff0000ff | (uIpv4TotalLength << 8);
    pPacketBuf[6] =  0xffffffff;
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffee00;
    g_pExpectFrag1HdrStack[4] =  0x33333345;
    g_pExpectFrag1HdrStack[5] =  0xcccccccc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xccccee00;
  } else if ( nL2ExtensionHdr == 4 ) {
    pPacketBuf[4] =  0x44444444;
    pPacketBuf[5] =  0x45ff0000 | uIpv4TotalLength;
    pPacketBuf[6] =  0xffffffff;
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffffee;
    g_pExpectFrag1HdrStack[4] =  0x44444444;
    g_pExpectFrag1HdrStack[5] =  0x45cccccc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xccccccee;
  } else if ( nL2ExtensionHdr == 5 ) {
    pPacketBuf[4] =  0x55555555;
    pPacketBuf[5] =  0x5545ff00 | (uIpv4TotalLength >> 8);
    pPacketBuf[6] =  0x00ffffff | (uIpv4TotalLength << 24);
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffffff;
    pPacketBuf[10] = 0xee000000;
    g_pExpectFrag1HdrStack[4] =  0x55555555;
    g_pExpectFrag1HdrStack[5] =  0x5545cccc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xcccccccc;
    g_pExpectFrag1HdrStack[10] = 0xee000000;
  } else if ( nL2ExtensionHdr == 6 ) {
    pPacketBuf[4] =  0x66666666;
    pPacketBuf[5] =  0x666645ff;
    pPacketBuf[6] =  0x0000ffff | (uIpv4TotalLength << 16);
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffffff;
    pPacketBuf[10] = 0xffee0000;
    g_pExpectFrag1HdrStack[4] =  0x66666666;
    g_pExpectFrag1HdrStack[5] =  0x666645cc;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xcccccccc;
    g_pExpectFrag1HdrStack[10] = 0xccee0000;
  } else if ( nL2ExtensionHdr == 7 ) {
    pPacketBuf[4] =  0x77777777;
    pPacketBuf[5] =  0x77777745;
    pPacketBuf[6] =  0xff0000ff | (uIpv4TotalLength << 8);
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffffff;
    pPacketBuf[10] = 0xffffee00;
    g_pExpectFrag1HdrStack[4] =  0x77777777;
    g_pExpectFrag1HdrStack[5] =  0x77777745;
    g_pExpectFrag1HdrStack[6] =  0xcccccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xcccccccc;
    g_pExpectFrag1HdrStack[10] = 0xccccee00;
  } else if ( nL2ExtensionHdr == 8 ) {
    pPacketBuf[4] =  0x88888888;
    pPacketBuf[5] =  0x88888888;
    pPacketBuf[6] =  0x45ff0000 | uIpv4TotalLength;
    pPacketBuf[7] =  0xffffffff;
    pPacketBuf[8] =  0xffffffff;
    pPacketBuf[9] =  0xffffffff;
    pPacketBuf[10] = 0xffffffee;
    g_pExpectFrag1HdrStack[4] =  0x88888888;
    g_pExpectFrag1HdrStack[5] =  0x88888888;
    g_pExpectFrag1HdrStack[6] =  0x45cccccc;
    g_pExpectFrag1HdrStack[7] =  0xcccccccc;
    g_pExpectFrag1HdrStack[8] =  0xcccccccc;
    g_pExpectFrag1HdrStack[9] =  0xcccccccc;
    g_pExpectFrag1HdrStack[10] = 0xccccccee;
  }

  pPktInfo->_pkt_data.len = nPacketLength;

  rc = bcm_tx_pkt_setup( nUnit, pPktInfo );
  if ( BCM_FAILURE(rc) ) {
    cli_out("bcm_tx_pkt_setup error:%d %s\n", rc, bcm_errmsg(rc));
    return TEST_FAIL;
  }

/*
  cli_out("c3hppc_sws_test3__cmic_send_packet: len=%d  id=0x%04x\n", nPacketLength, uPacketID );
*/
  rc = bcm_tx( nUnit, pPktInfo, NULL );
  if ( BCM_FAILURE(rc) ) {
    cli_out("bcm_tx error:%d %s\n", rc, bcm_errmsg(rc));
    return TEST_FAIL;
  }


  return 0;
}


int c3hppc_sws_test3__sws_pp_pd_modifications( int nUnit )
{

  uint32 uRegisterValue, auInitialQueueStateEntry[6], uFieldValue;
  int nSourceQueue, nIndex, nEgressRuleOffset;
  c3hppc_sws_ppe_tcam_data_t TcamData, TcamMask; 
  uint32 auTcamEntry[16], auTcamRamEntry[8];
  uint32 uParseStage, uHdrType;
  int nTcamIndex;


  /* PED config for headers ranging from 1 to 8 bytes in length */
  for ( nIndex = 0; nIndex < 9; ++nIndex ) {
    READ_PD_HDR_CONFIGr( nUnit, nIndex, &uRegisterValue );
    uFieldValue = nIndex;
    soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, uFieldValue );
    WRITE_PD_HDR_CONFIGr( nUnit, nIndex, uRegisterValue );
  }

  /* PED config for variable length pseudo route header. */
  uHdrType = C3HPPC_SWS_TEST3__ROUTE_HEADER_TYPE;
  READ_PD_HDR_CONFIGr( nUnit, uHdrType, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, LEN_UNITSf, 0 );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, LEN_SIZEf, 7 );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, LEN_POSNf, 128 );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, LEN_MODf, 1 );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, 0 );
  WRITE_PD_HDR_CONFIGr( nUnit, uHdrType, uRegisterValue );

  /* PED config for pseudo L2 header. */
  uHdrType = C3HPPC_SWS_TEST3__L2_HEADER_TYPE;
  READ_PD_HDR_CONFIGr( nUnit, uHdrType, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, C3HPPC_SWS_TEST3__L2_HEADER_SIZE );
  WRITE_PD_HDR_CONFIGr( nUnit, uHdrType, uRegisterValue );

  /* PED config for IPV4 header. */
  uHdrType = C3HPPC_SWS_TEST3__IPV4_HEADER_TYPE;
  READ_PD_HDR_CONFIGr( nUnit, uHdrType, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE );
  WRITE_PD_HDR_CONFIGr( nUnit, uHdrType, uRegisterValue );

  READ_PD_IPV4_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_IPV4_CONFIGr, &uRegisterValue, ENBf, 1 );
  soc_reg_field_set( nUnit, PD_IPV4_CONFIGr, &uRegisterValue, CONDITIONAL_UPDATEf, 1 );
  soc_reg_field_set( nUnit, PD_IPV4_CONFIGr, &uRegisterValue, HDR_TYPEf, C3HPPC_SWS_TEST3__IPV4_HEADER_TYPE );
  WRITE_PD_IPV4_CONFIGr( nUnit, uRegisterValue );

  /* PED config for IPV4 Fragment Control header. */
  uHdrType = C3HPPC_SWS_TEST3__FRAGMENT_CONTROL_HEADER_TYPE;
  READ_PD_HDR_CONFIGr( nUnit, uHdrType, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, 4 );
  WRITE_PD_HDR_CONFIGr( nUnit, uHdrType, uRegisterValue );



  /* PPE config for ingress line traffic. */
  nSourceQueue = 0;
  uHdrType = 1;
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                    MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
  uFieldValue = 1;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_TYPEf, &uHdrType );
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, SHIFTf, &uHdrType );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                     MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );

  /* PPE config for egress line traffic. */
  nSourceQueue = 64;
  uHdrType = C3HPPC_SWS_TEST3__ROUTE_HEADER_TYPE;
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                    MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
  uFieldValue = 1;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_TYPEf, &uHdrType );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, SHIFTf, &uFieldValue );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_SHIFT_BITSf, &uFieldValue );
  uFieldValue = C3HPPC_SWS_TEST3__ROUTE_HEADER_BASE_SIZE;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_SHIFT_PTRf, &uFieldValue );
  uFieldValue = 6;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_MASKf, &uFieldValue );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_UNITSf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                     MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );

  /* PPE config for ingress host/fragmentation traffic. */
  nSourceQueue = 50;
  uHdrType = C3HPPC_SWS_TEST3__L2_HEADER_TYPE;
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                    MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_TYPEf, &uHdrType );
  uFieldValue = C3HPPC_SWS_TEST3__L2_HEADER_SIZE;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, SHIFTf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                     MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );

  /* PPE config for egress host/fragmentation traffic. */
  nSourceQueue = 64 + 47;
  uHdrType = C3HPPC_SWS_TEST3__ROUTE_HEADER_TYPE;
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                    MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_TYPEf, &uHdrType );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, SHIFTf, &uFieldValue );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_SHIFT_BITSf, &uFieldValue );
  uFieldValue = C3HPPC_SWS_TEST3__ROUTE_HEADER_BASE_SIZE;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_SHIFT_PTRf, &uFieldValue );
  uFieldValue = 6;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_MASKf, &uFieldValue );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, LENGTH_UNITSf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                     MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );




  /* Parse Stage(0) TCAM config for ingress host/fragmentation traffic. */
  for ( uHdrType = 1, uParseStage = 0; uHdrType <= 8; ++uHdrType ) {

    nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + uHdrType;
      
    sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
    sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

    TcamData.Word6.bits.HdrData_24to24 = 0xda; 
    TcamData.Word5.bits.HdrData_23to20 = 0xdadadada;
    TcamData.Word4.bits.HdrData_19to16 = 0xda5a5a5a;
    TcamData.Word3.bits.HdrData_15to12 = 0x5a5a5a00;
    TcamData.Word2.bits.HdrData_11to8 = ( uHdrType << 8 );
    TcamMask.Word6.bits.HdrData_24to24 = 0xff; 
    TcamMask.Word5.bits.HdrData_23to20 = 0xffffffff;
    TcamMask.Word4.bits.HdrData_19to16 = 0xffffffff;
    TcamMask.Word3.bits.HdrData_15to12 = 0xffffffff;
    TcamMask.Word2.bits.HdrData_11to8 =  0xffffff00;

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
    uFieldValue = 3;
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uHdrType );
    uFieldValue = 0x000001;
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATEf, &uFieldValue );
    uFieldValue = 0xffffff;
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATE_MASKf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  }

  /* Parse Stage(1) TCAM config for ingress host/fragmentation traffic. */
  ++uParseStage;
  nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage );

  sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
  sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

  TcamData.Word6.bits.State = 0x000001;
  TcamMask.Word6.bits.State = 0xffffff;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
  uFieldValue = 3;
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  uHdrType = C3HPPC_SWS_TEST3__IPV4_HEADER_TYPE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
  uFieldValue = C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uFieldValue );
  uFieldValue = 0x000002;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATEf, &uFieldValue );
  uFieldValue = 0xffffff;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATE_MASKf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );

  /* Parse Stage(2) TCAM config for ingress host/fragmentation traffic. */
  ++uParseStage;
  nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage );

  sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
  sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

  TcamData.Word6.bits.State = 0x000002;
  TcamMask.Word6.bits.State = 0xffffff;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
  uFieldValue = 3;
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  uHdrType = 0x3f;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_VALUEf, &uFieldValue );
  uFieldValue = 0xf;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_MASKf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );




  /* Parse Stage(0) TCAM config for egress host/fragmentation traffic. */
  nEgressRuleOffset = 16;
  uParseStage = 0;
  nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + uHdrType + nEgressRuleOffset;
      
  sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
  sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

  TcamData.Word6.bits.HdrData_24to24 = 0x5b; 
  TcamData.Word5.bits.HdrData_23to20 = 0x05b15b25;
  TcamData.Word4.bits.HdrData_19to16 = 0xb35b45b5;
  TcamData.Word3.bits.HdrData_15to12 = 0x5b65b75b;
  TcamData.Word2.bits.HdrData_11to8 =  0x85b95b00;
  TcamMask.Word6.bits.HdrData_24to24 = 0xff; 
  TcamMask.Word5.bits.HdrData_23to20 = 0xffffffff;
  TcamMask.Word4.bits.HdrData_19to16 = 0xffffffff;
  TcamMask.Word3.bits.HdrData_15to12 = 0xffffffff;
  TcamMask.Word2.bits.HdrData_11to8 =  0xffffff00;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
  uFieldValue = 3;
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  uHdrType = C3HPPC_SWS_TEST3__L2_HEADER_TYPE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
  uFieldValue = C3HPPC_SWS_TEST3__L2_HEADER_SIZE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );


  /* Parse Stage(1) TCAM config for egress host/fragmentation traffic. */
  ++uParseStage;
  for ( uHdrType = 1; uHdrType <= 8; ++uHdrType ) {

    nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + uHdrType + nEgressRuleOffset;
      
    sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
    sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

    TcamData.Word6.bits.HdrData_24to24 = 0xda; 
    TcamData.Word5.bits.HdrData_23to20 = 0xdadadada;
    TcamData.Word4.bits.HdrData_19to16 = 0xda5a5a5a;
    TcamData.Word3.bits.HdrData_15to12 = 0x5a5a5a00;
    TcamData.Word2.bits.HdrData_11to8 = ( uHdrType << 8 );
    TcamMask.Word6.bits.HdrData_24to24 = 0xff; 
    TcamMask.Word5.bits.HdrData_23to20 = 0xffffffff;
    TcamMask.Word4.bits.HdrData_19to16 = 0xffffffff;
    TcamMask.Word3.bits.HdrData_15to12 = 0xffffffff;
    TcamMask.Word2.bits.HdrData_11to8 =  0xffffff00;

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
    uFieldValue = 3;
    soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uHdrType );
    uFieldValue = 0x000001;
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATEf, &uFieldValue );
    uFieldValue = 0xffffff;
    soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATE_MASKf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  }

  /* Parse Stage(2) TCAM config for egress host/fragmentation traffic. */
  ++uParseStage;
  nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + nEgressRuleOffset;

  sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
  sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

  TcamData.Word6.bits.State = 0x000001;
  TcamMask.Word6.bits.State = 0xffffff;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
  uFieldValue = 3;
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  uHdrType = C3HPPC_SWS_TEST3__IPV4_HEADER_TYPE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
  uFieldValue = C3HPPC_SWS_TEST3__IPV4_HEADER_SIZE;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uFieldValue );
  uFieldValue = 0x000002;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATEf, &uFieldValue );
  uFieldValue = 0xffffff;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATE_MASKf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );

  /* Parse Stage(3) TCAM config for egress host/fragmentation traffic. */
  ++uParseStage;
  nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + nEgressRuleOffset;

  sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
  sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

  TcamData.Word6.bits.State = 0x000002;
  TcamMask.Word6.bits.State = 0xffffff;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
  uFieldValue = 3;
  soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
  uHdrType = 0x3f;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uHdrType );
  uFieldValue = 0;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_VALUEf, &uFieldValue );
  uFieldValue = 0xf;
  soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_MASKf, &uFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );

  return 0;
}

uint32 c3hppc_sws_test3__ipv4_checksum_calc( uint8 *pIpv4Header )
{
  uint32 uCheckSum, uTemp;
  int nIndex;

  for ( nIndex = 0, uCheckSum = 0; nIndex < 20; nIndex += 2 ) {
    if ( nIndex != 10 ) {
      sal_memcpy( &uTemp, pIpv4Header+nIndex, 2 );
      uTemp >>= 16;
    } else {
      uTemp = 0;
    }
    uCheckSum += uTemp;
  }

  uTemp = uCheckSum >> 16; 
  uCheckSum &= 0xffff;
  uCheckSum += uTemp;  
  uCheckSum ^= 0xffff;
  uCheckSum <<= 16;

  return uCheckSum;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
