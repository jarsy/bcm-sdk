/*
 * $Id: c3hppc_sws.c,v 1.69 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_sws.c
 * Purpose: Caladan3 SWS test driver for HPPC environment
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_sws.h>
#include <soc/drv.h>
#include <bcm/error.h>


static int g_anIlErrorRegisters[] = { IL_RX_ERRDET0_L2_INTRr,
                                      IL_RX_ERRDET1_L2_INTRr,
                                      IL_RX_ERRDET2_L2_INTRr,
                                      IL_RX_ERRDET3_L2_INTRr,
                                      IL_RX_ERRDET4_L2_INTRr,
                                      IL_RX_ERRDET5_L2_INTRr,
                                      IL_TX_ERRDET0_L2_INTRr,
                                      IL_FLOWCONTROL_L2_INTRr
                                    };
static int g_nIlErrorRegistersCount = COUNTOF(g_anIlErrorRegisters);
static int g_anIlErrorMaskRegisters[] = { IL_RX_ERRDET0_L2_INTR_MASKr,
                                          IL_RX_ERRDET1_L2_INTR_MASKr,
                                          IL_RX_ERRDET2_L2_INTR_MASKr,
                                          IL_RX_ERRDET3_L2_INTR_MASKr,
                                          IL_RX_ERRDET4_L2_INTR_MASKr,
                                          IL_RX_ERRDET5_L2_INTR_MASKr,
                                          IL_TX_ERRDET0_L2_INTR_MASKr,
                                          IL_FLOWCONTROL_L2_INTR_MASKr 
                                        };
static int g_nIlErrorMaskRegistersCount = COUNTOF(g_anIlErrorMaskRegisters);


static int g_anPpErrorRegisters[] = { PP_GLOBAL_EVENTr,
                                      PP_ECC_ERROR0r,
                                      PP_ECC_ERROR1r,
                                      PP_ECC_ERROR2r
                                    };
static int g_nPpErrorRegistersCount = COUNTOF(g_anPpErrorRegisters);
static int g_anPpErrorMaskRegisters[] = { PP_GLOBAL_EVENT_MASKr,
                                          PP_ECC_ERROR0_MASKr,
                                          PP_ECC_ERROR1_MASKr,
                                          PP_ECC_ERROR2_MASKr
                                        };
static int g_nPpErrorMaskRegistersCount = COUNTOF(g_anPpErrorMaskRegisters);


static int g_anPdErrorRegisters[] = { PD_EVENTr,
                                      PD_SER_EVENTr
                                    };
static int g_nPdErrorRegistersCount = COUNTOF(g_anPdErrorRegisters);
static int g_anPdErrorMaskRegisters[] = { PD_EVENT_MASKr,
                                          PD_SER_EVENT_MASKr
                                        };
static int g_nPdErrorMaskRegistersCount = COUNTOF(g_anPdErrorMaskRegisters);


static int g_anPbErrorRegisters[] = { PB_ECC_ERRORr,
                                      PB_ERROR0r,
                                      PB_PARITY_ERRORr
                                    };
static int g_nPbErrorRegistersCount = COUNTOF(g_anPbErrorRegisters);
static int g_anPbErrorMaskRegisters[] = { PB_ECC_ERROR_MASKr,
                                          PB_ERROR0_MASKr,
                                          PB_PARITY_ERROR_MASKr 
                                        };
static int g_nPbErrorMaskRegistersCount = COUNTOF(g_anPbErrorMaskRegisters);


static int g_anQmErrorRegisters[] = { QM_ECC_ERROR0r,
                                      QM_ECC_ERROR1r,
                                      QM_ECC_ERROR2r,
                                      QM_ECC_ERROR3r,
                                      QM_ERRORr,
                                      QM_PARITY_ERRORr
                                    };
static int g_nQmErrorRegistersCount = COUNTOF(g_anQmErrorRegisters);
static int g_anQmErrorMaskRegisters[] = { QM_ECC_ERROR0_MASKr,
                                          QM_ECC_ERROR1_MASKr,
                                          QM_ECC_ERROR2_MASKr,
                                          QM_ECC_ERROR3_MASKr,
                                          QM_ERROR_MASKr,
                                          QM_PARITY_ERROR_MASKr
                                        };
static int g_nQmErrorMaskRegistersCount = COUNTOF(g_anQmErrorMaskRegisters);


static int g_anPrErrorRegisters[] = { PR_HDP_ERROR_0r,
                                      PR_HDP_ERROR_1r,
                                      PR_ICC_ERROR_0r,
                                      PR_IDP_ERROR_0r,
                                      PR_IDP_ERROR_1r,
                                      PR_IDP_POLICER_ERRORr,
                                      PR_IPRE_ERROR_0r,
                                      PR_IPRE_ERROR_1r,
                                      PR_IPRE_ERROR_2r
                                    };
static int g_nPrErrorRegistersCount = COUNTOF(g_anPrErrorRegisters);
static int g_anPrErrorMaskRegisters[] = { PR_HDP_ERROR_0_MASKr,
                                          PR_HDP_ERROR_1_MASKr,
                                          PR_ICC_ERROR_0_MASKr,
                                          PR_IDP_ERROR_0_MASKr,
                                          PR_IDP_ERROR_1_MASKr,
                                          PR_IDP_POLICER_ERROR_MASKr,
                                          PR_IPRE_ERROR_0_MASKr,
                                          PR_IPRE_ERROR_1_MASKr,
                                          PR_IPRE_ERROR_2_MASKr
                                        };
static int g_nPrErrorMaskRegistersCount = COUNTOF(g_anPrErrorMaskRegisters);


static int g_anPtErrorRegisters[] = { PT_HPTE_DEBUG_ERROR0r,
                                      HPTE_ECC_ERROR0r,
                                      IPTE_ECC_ERROR0r,
                                      IPTE_DEBUG_ERROR0r,
                                      IPTE_DEBUG_ERROR1r,
                                      IPTE_DEBUG_ERROR2r
                                    };
static int g_nPtErrorRegistersCount = COUNTOF(g_anPtErrorRegisters);
static int g_anPtErrorMaskRegisters[] = { PT_HPTE_DEBUG_ERROR0_MASKr,
                                          HPTE_ECC_ERROR0_MASKr,
                                          IPTE_ECC_ERROR0_MASKr,
                                          IPTE_DEBUG_ERROR0_MASKr,
                                          IPTE_DEBUG_ERROR1_MASKr,
                                          IPTE_DEBUG_ERROR2_MASKr
                                        };
static int g_nPtErrorMaskRegistersCount = COUNTOF(g_anPtErrorMaskRegisters);

#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static int g_anPtClientCalendarMems[2] = { IPTE_CLIENT_CALm, IPTE_CLIENT_CAL1m };
static int g_anPtClientCalendarRegs[2] = { PT_IPTE_CLIENT_CAL_CFGr, PT_IPTE_CLIENT_CAL1_CFGr };
static int g_anPtPortCalendarMems[2] = { IPTE_PORT_CALm, IPTE_PORT_CAL1m };
static int g_anPtPortCalendarRegs[2] = { PT_IPTE_DS_PORT_CAL_CFGr, PT_IPTE_DS_PORT_CAL1_CFGr };
#endif

static c3hppc_sws_il_stats_t *g_pIlRxStats = NULL;
static c3hppc_sws_il_stats_t *g_pIlTxStats = NULL;
static uint32 *g_pIlDmaBuffer = NULL;
static uint8 g_bIlInstanceEnabled[C3HPPC_SWS_IL_INSTANCE_NUM];
static uint32 g_uIlCounterCompatMode = 0;
static uint8 g_bLrpLearningEnable = 0;
static uint8 g_bOamEnable = 0;
static uint8 g_bSwsOnlyTest = 0;
static int g_nTDM = 0;
static uint8 g_bSingleTrafficGenOnly = 0;
static uint8 g_bLineTrafficGenOnly = 0;
static uint8 g_bFabricTrafficGenOnly = 0;
static uint8 g_bXLportAndCmicPortsActive = 0;
static uint8 g_bHotSwapMix_10G_1G = 0;

static uint32 g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS_NUM][ILPKTGEN__BUILTIN_HEADERS_MAX_WORDS];
static int    g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS_NUM];

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;

#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static int g_anPortBlkNum[5] = { 17, 60, 61, 62, 63 };
#endif

/*
static int g_anWarpCore_PhyIDs[C3HPPC_SWS_IL_INSTANCE_NUM][3] = { {0x81, 0xa1, 0xc1}, {0x95, 0x99, 0xb5} };
*/





int c3hppc_sws_hw_init( int nUnit, c3hppc_sws_control_info_t *pC3SwsControlInfo ) {

  int nInstance;
  uint32 uRegisterValue;

  soc_cm_get_id( nUnit, &g_dev_id, &g_rev_id);

  g_bLrpLearningEnable = pC3SwsControlInfo->bLrpLearningEnable;
  g_bOamEnable = pC3SwsControlInfo->bOamEnable;
  g_bSwsOnlyTest = pC3SwsControlInfo->bSwsOnlyTest;
  g_nTDM = pC3SwsControlInfo->nTDM;
  g_bSingleTrafficGenOnly = pC3SwsControlInfo->bLineTrafficGenOnly | pC3SwsControlInfo->bFabricTrafficGenOnly;
  g_bLineTrafficGenOnly = pC3SwsControlInfo->bLineTrafficGenOnly;
  g_bFabricTrafficGenOnly = pC3SwsControlInfo->bFabricTrafficGenOnly;
  g_bXLportAndCmicPortsActive = pC3SwsControlInfo->bXLportAndCmicPortsActive;
  g_bHotSwapMix_10G_1G = ( pC3SwsControlInfo->nHotSwap & C3HPPC_SWS_HOTSWAP__10G_1G_MIX ) ? 1 : 0;

  c3hppc_sws_pp_bringup( nUnit, &(pC3SwsControlInfo->PpControlInfo) );
  c3hppc_sws_pd_bringup( nUnit, &(pC3SwsControlInfo->PdControlInfo) );
  c3hppc_sws_pb_bringup( nUnit, &(pC3SwsControlInfo->PbControlInfo) );
  c3hppc_sws_pr_bringup( nUnit, &(pC3SwsControlInfo->PrControlInfo) );
  c3hppc_sws_qm_bringup( nUnit, &(pC3SwsControlInfo->QmControlInfo) );
  c3hppc_sws_pt_bringup( nUnit, &(pC3SwsControlInfo->PtControlInfo), pC3SwsControlInfo->aIlControlInfo );



  READ_QM_FC_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_FC_CONFIG0r, &uRegisterValue, FC_ENABLEf, 1 );
  WRITE_QM_FC_CONFIG0r( nUnit, uRegisterValue );
  READ_QM_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_CONFIGr, &uRegisterValue, QM_ENABLEf, 1 );
  WRITE_QM_CONFIGr( nUnit, uRegisterValue );

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {
    soc_reg32_get( nUnit, PR_HDP_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_HDP_CONFIGr, &uRegisterValue, HDP_ENABLEf, 1 );
    soc_reg32_set( nUnit, PR_HDP_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_get( nUnit, PR_IDP_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IDP_CONFIG0r, &uRegisterValue, IDP_ENABLEf, 1 );
    soc_reg32_set( nUnit, PR_IDP_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }


  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    if ( pC3SwsControlInfo->aIlControlInfo[nInstance].bBringUp ) {
      g_bIlInstanceEnabled[nInstance] = 1; 
      if ( c3hppc_sws_il_bringup( nUnit, nInstance, &(pC3SwsControlInfo->aIlControlInfo[nInstance]) ) ) {
        return 1;
      }
    } else {
      g_bIlInstanceEnabled[nInstance] = 0; 
    }
  }

  if ( pC3SwsControlInfo->MacControlInfo.bBringUp ) {
    if ( c3hppc_sws_mac_bringup( nUnit, &(pC3SwsControlInfo->MacControlInfo) ) ) {
      return 1;
    }
  }

  /* Setup for packet tx/rx dma across CMIC */
  if ( soc_dma_init( nUnit ) ) {
    return 1;
  }

  return 0;
}


int c3hppc_sws_pp_bringup( int nUnit, c3hppc_sws_pp_control_info_t *pPpControlInfo ) {

  uint32 uRegisterValue, auInitialQueueStateEntry[6], uFieldValue;
  int nSourceQueue;
  c3hppc_sws_ppe_tcam_data_t TcamData, TcamMask; 
  uint32 auTcamEntry[16], auTcamRamEntry[8];
  uint32 uParseStage, uHdrType;
  int nTcamIndex, nTaskStreamNum;


  READ_PP_GLOBAL_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, PP_GLOBAL_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1 );
  WRITE_PP_GLOBAL_CONFIGr( nUnit, uRegisterValue );

  soc_reg_field_set( nUnit, PP_GLOBAL_CONFIGr, &uRegisterValue, INITf, 1 );
  WRITE_PP_GLOBAL_CONFIGr( nUnit, uRegisterValue );

  READ_PP_ASSEMBLER_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, PP_ASSEMBLER_CONFIGr, &uRegisterValue, LOAD_SEQNUMf, 1 );

  if ( pPpControlInfo->bBypassParsing == 1 ) { 

    for ( nSourceQueue = 0; nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nSourceQueue ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                        MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
      nTaskStreamNum = ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM && g_bLrpLearningEnable ) ? 11 : 12; 
      nTaskStreamNum = ( g_bSingleTrafficGenOnly ) ? 5 : nTaskStreamNum; 
      uFieldValue = ( pPpControlInfo->bMultiStreamUcode ) ? ( sal_rand() % nTaskStreamNum ) : 0;
      soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
  
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                         MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
    }

  } else {

    for ( nSourceQueue = 0, uParseStage = 0, uHdrType = 1;
          nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM;
          ++nSourceQueue ) {

      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_IQSMm, 
                                        MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
      uFieldValue = 0;
      soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_STREAMf, &uFieldValue );
      uFieldValue = nSourceQueue | ( uParseStage << 8 );
      soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, STATEf, &uFieldValue );
      uFieldValue = uHdrType;
      soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, INITIAL_TYPEf, &uFieldValue );
      uFieldValue = uHdrType;
      soc_mem_field_set( nUnit, PP_IQSMm, auInitialQueueStateEntry, SHIFTf, &uFieldValue );
  
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_IQSMm, 
                                         MEM_BLOCK_ANY, nSourceQueue, auInitialQueueStateEntry) );
    }

    for ( uParseStage = 0; uParseStage < 14; ++uParseStage, ++uHdrType ) {

      for ( nSourceQueue = 0; nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nSourceQueue ) {

        nTcamIndex = ( C3HPPC_SWS_SOURCE_QUEUE_NUM * uParseStage ) + nSourceQueue;
      
        sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );
        sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_ppe_tcam_data_t) );

        TcamData.Word6.bits.State = nSourceQueue | ( uParseStage << 8 );
        TcamMask.Word6.bits.State = 0xffffff;

        if ( uParseStage == 0 ) {

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x11 : 0x88;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
  
        } else if ( uParseStage == 1) {

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x22 : 0x99;
          TcamData.Word5.bits.HdrData_23to20 =
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x22000000 : 0x99000000;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
          TcamMask.Word5.bits.HdrData_23to20 = 0xff000000;

        } else if ( uParseStage == 2) { 

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x33 : 0xaa;
          TcamData.Word5.bits.HdrData_23to20 =
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x33330000 : 0xaaaa0000;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
          TcamMask.Word5.bits.HdrData_23to20 = 0xffff0000;

        } else if ( uParseStage == 3) { 

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x44 : 0xbb;
          TcamData.Word5.bits.HdrData_23to20 =
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x44444400 : 0xbbbbbb00;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
          TcamMask.Word5.bits.HdrData_23to20 = 0xffffff00;

        } else if ( uParseStage == 4) { 

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x55 : 0xcc;
          TcamData.Word5.bits.HdrData_23to20 =
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x55555555 : 0xcccccccc;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
          TcamMask.Word5.bits.HdrData_23to20 = 0xffffffff;

        } else if ( uParseStage == 5) { 

          TcamData.Word6.bits.HdrData_24to24 = 
            ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM || g_bSingleTrafficGenOnly ) ? 0x66 : 0xdd;
          TcamData.Word5.bits.HdrData_23to20 =
                  ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM ) ? 0x00000000 : 0x00000000;
          TcamData.Word4.bits.HdrData_19to16 =
                  ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM ) ? 0x00000000 : 0x00000000;
          TcamMask.Word6.bits.HdrData_24to24 = 0xff;
          /*
            To cause more transitions in the chip for power characterization the ucode modifies the packet data
            starting at byte 16.  In "SingleTrafficGenOnly" mode the classification beyond byte 16 need to
            become don't cares.
          */
          if ( g_bSingleTrafficGenOnly ) {
            TcamMask.Word5.bits.HdrData_23to20 = 0x00000000;
            TcamMask.Word4.bits.HdrData_19to16 = 0x00000000;
          } else {
            TcamMask.Word5.bits.HdrData_23to20 = 0xffffffff;
            TcamMask.Word4.bits.HdrData_19to16 = 0xff000000;
          }

        } else {

          TcamData.Word6.bits.HdrData_24to24 = 
                  ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM ) ? 0x00 : 0x00;
          TcamMask.Word6.bits.HdrData_24to24 = ( g_bSingleTrafficGenOnly ) ? 0x00 : 0xff;

        }

        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) ); 
        soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, DATAf, (uint32 *) &TcamData );
        soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, MASKf, (uint32 *) &TcamMask );
        uFieldValue = 3;
        soc_mem_field_set( nUnit, PP_TCAMm, auTcamEntry, VALIDf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_TCAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamEntry) );

        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
        uFieldValue = ( uParseStage == 13 ) ? 0x3f : ( uHdrType + 1 );
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, NXT_HDR_TYPEf, &uFieldValue );
        nTaskStreamNum = ( nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM && g_bLrpLearningEnable ) ? 11 : 12; 
        /*
          Ucode Streams >= 5 do packet content checks and will fail due to the ucode based packet modifications.
        */
        nTaskStreamNum = ( g_bSingleTrafficGenOnly ) ? 5 : nTaskStreamNum; 
        uFieldValue = ( uParseStage == 13 ) ? ( sal_rand() % nTaskStreamNum ) : 0;
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_VALUEf, &uFieldValue );
        uFieldValue = ( uParseStage == 13 ) ? 0xf : 0;
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STREAM_MASKf, &uFieldValue );
        uFieldValue = ( uHdrType <= 5 ) ? (uHdrType + 1) : 1;
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, SHIFTf, &uFieldValue );
        uFieldValue = nSourceQueue | ( (uParseStage+1) << 8 );
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATEf, &uFieldValue );
        uFieldValue = 0xffffff;
        soc_mem_field_set( nUnit, PP_CAM_RAMm, auTcamRamEntry, STATE_MASKf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PP_CAM_RAMm, MEM_BLOCK_ANY, nTcamIndex, auTcamRamEntry) );
      }
    }
  }

  return 0;
}


int c3hppc_sws_pd_bringup( int nUnit, c3hppc_sws_pd_control_info_t *pPdControlInfo ) {

  uint32 uRegisterValue, uFieldValue;
  int nIndex;


  READ_PD_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, PD_CONFIG0r, &uRegisterValue, SOFT_RESET_Nf, 1 );
  WRITE_PD_CONFIG0r( nUnit, uRegisterValue );

  uRegisterValue = 0xffffffff;
  for ( nIndex = 0; nIndex < 4; ++nIndex ) {
    WRITE_PD_VALID_DQUEUESr( nUnit, nIndex, uRegisterValue );
    WRITE_PD_VALID_SQUEUESr( nUnit, nIndex, uRegisterValue );
  }

  for ( nIndex = 0; nIndex < 63; ++nIndex ) {
    READ_PD_HDR_CONFIGr( nUnit, nIndex, &uRegisterValue );
    uFieldValue = ( nIndex <= 6 ) ? nIndex : 1;
    soc_reg_field_set( nUnit, PD_HDR_CONFIGr, &uRegisterValue, BASE_LENGTHf, uFieldValue );
    WRITE_PD_HDR_CONFIGr( nUnit, nIndex, uRegisterValue );
  }


  return 0;
}


int c3hppc_sws_pb_bringup( int nUnit, c3hppc_sws_pb_control_info_t *pPbControlInfo ) {

  uint32 uRegisterValue;


  READ_PB_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, PB_CONFIGr, &uRegisterValue, PB_SOFT_RESET_Nf, 1 );
  WRITE_PB_CONFIGr( nUnit, uRegisterValue );

  soc_reg_field_set( nUnit, PB_CONFIGr, &uRegisterValue, PB_DO_INITf, 1 );
  WRITE_PB_CONFIGr( nUnit, uRegisterValue );

  soc_reg_field_set( nUnit, PB_CONFIGr, &uRegisterValue, PB_ENABLEf, 1 );
  WRITE_PB_CONFIGr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_sws_qm_bringup( int nUnit, c3hppc_sws_qm_control_info_t *pQmControlInfo ) {

  uint32 uRegisterValue, auSourceQueueConfigEntry[3], auReplicationRefEntry[1], uFieldValue;
  int nSourceQueue;
  uint32 uInterfaceFlowControlThreshold, uQueueMinPages, uReservePageNumber;
  int nActiveLineSideQueueNum;


  READ_QM_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_CONFIGr, &uRegisterValue, QM_SOFT_RESET_Nf, 1 );
  WRITE_QM_CONFIGr( nUnit, uRegisterValue );

  uRegisterValue = 0x07ffffff;
  WRITE_QM_MEM_INITr( nUnit, uRegisterValue );

  READ_QM_FP_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_FP_CONFIG0r, &uRegisterValue, FP_INIT_START_PAGEf, 0 );
  soc_reg_field_set( nUnit, QM_FP_CONFIG0r, &uRegisterValue, FP_INIT_END_PAGEf, ( g_bLrpLearningEnable || g_bOamEnable ? 0x3ffe : 0x3fff) );
  soc_reg_field_set( nUnit, QM_FP_CONFIG0r, &uRegisterValue, FP_INITf, 1 );
  WRITE_QM_FP_CONFIG0r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG0r, &uRegisterValue, TOTAL_BUFF_MAX_PAGESf, 15384 );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG0r, &uRegisterValue, TOTAL_BUFF_HYSTERESIS_DELTAf, 1539 );
  WRITE_QM_BUFFER_CONFIG0r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG1r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG1r, &uRegisterValue, TOTAL_BUFF_DROP_THRESH_DE2f, 7692 );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG1r, &uRegisterValue, TOTAL_BUFF_DROP_THRESH_DE1f, 11538 );
  WRITE_QM_BUFFER_CONFIG1r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG2r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG2r, &uRegisterValue, INGRESS_MAX_PAGESf, C3HPPC_SWS_QM_INGRESS_MAX_PAGES );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG2r, &uRegisterValue, INGRESS_HYSTERESIS_DELTAf, 754 );
  WRITE_QM_BUFFER_CONFIG2r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG3r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG3r, &uRegisterValue, INGRESS_DROP_THRESH_DE2f, 3770 );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG3r, &uRegisterValue, INGRESS_DROP_THRESH_DE1f, 5655 );
  WRITE_QM_BUFFER_CONFIG3r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG4r, &uRegisterValue, EGRESS_MAX_PAGESf, C3HPPC_SWS_QM_EGRESS_MAX_PAGES );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG4r, &uRegisterValue, EGRESS_HYSTERESIS_DELTAf, 754 );
  WRITE_QM_BUFFER_CONFIG4r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG5r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG5r, &uRegisterValue, EGRESS_DROP_THRESH_DE2f, 3770 );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG5r, &uRegisterValue, EGRESS_DROP_THRESH_DE1f, 5655 );
  WRITE_QM_BUFFER_CONFIG5r( nUnit, uRegisterValue );

  READ_QM_BUFFER_CONFIG6r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG6r, &uRegisterValue, PER_QUEUE_DROP_HYSTERESIS_DELTAf, 34 );
  WRITE_QM_BUFFER_CONFIG6r( nUnit, uRegisterValue );

  uInterfaceFlowControlThreshold = ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ||
                                     g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) ? 7200 : 6715;

  READ_QM_FC_CONFIG1r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_FC_CONFIG1r, &uRegisterValue, FC_TOTAL_BUFFER_XOFF_THRESHf,
                                                             ((2*uInterfaceFlowControlThreshold) + 100) );
  WRITE_QM_FC_CONFIG1r( nUnit, uRegisterValue );

  READ_QM_FC_CONFIG2r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_FC_CONFIG2r, &uRegisterValue, FC_EGRESS_XOFF_THRESHf, uInterfaceFlowControlThreshold );
  soc_reg_field_set( nUnit, QM_FC_CONFIG2r, &uRegisterValue, FC_INGRESS_XOFF_THRESHf, uInterfaceFlowControlThreshold );
  WRITE_QM_FC_CONFIG2r( nUnit, uRegisterValue );

  READ_QM_DEBUGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_DEBUGr, &uRegisterValue, PT_PREFETCH_LIMITf, 0xf );
  WRITE_QM_DEBUGr( nUnit, uRegisterValue );

  switch ( g_nTDM ) {
    case C3HPPC_SWS_TDM__48x1G_BY_100IL:           nActiveLineSideQueueNum = 48; break;
    case C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS:   nActiveLineSideQueueNum = C3HPPC_SWS_IL_MAX_CHANNEL_NUM; break;
    case C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS:    nActiveLineSideQueueNum = 2; break;
    case C3HPPC_SWS_TDM__12x10G_BY_100IL:          nActiveLineSideQueueNum = 12; break;
    case C3HPPC_SWS_TDM__3x40G_BY_100IL:           nActiveLineSideQueueNum = 3; break;
    default:                                       nActiveLineSideQueueNum = 1;
  }

  if ( g_bXLportAndCmicPortsActive ) nActiveLineSideQueueNum += 2; 

  uQueueMinPages = ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL || g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS ||
                     g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) ? 819  : 12;
  if ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS ) uQueueMinPages /= 2;

  uReservePageNumber = uQueueMinPages * nActiveLineSideQueueNum * 2;   /* 2x factor is line and fabric interfaces */
  uReservePageNumber *= 2;    /* 2x factor is for header copy */ 

  READ_QM_BUFFER_CONFIG7r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_BUFFER_CONFIG7r, &uRegisterValue, NUM_PAGES_RESERVEDf, uReservePageNumber );
  WRITE_QM_BUFFER_CONFIG7r( nUnit, uRegisterValue );

  for ( nSourceQueue = 0; nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nSourceQueue ) {
    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                      MEM_BLOCK_ANY, nSourceQueue, auSourceQueueConfigEntry) );

    uFieldValue = C3HPPC_SWS_QM_INGRESS_MAX_PAGES / nActiveLineSideQueueNum;
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
    if ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL || g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) {
      uFieldValue = uInterfaceFlowControlThreshold - C3HPPC_SWS_QM_GT10G_FLOW_CTRL_THOLD_DELTA;
    } else if ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS ) {
      uFieldValue -= C3HPPC_SWS_QM_64_CHNL_FLOW_CTRL_THOLD_DELTA;
    } else {
      uFieldValue -= ( g_nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS || 
                       (g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL && 
                        nSourceQueue < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM) ) ? C3HPPC_SWS_QM_GT10G_FLOW_CTRL_THOLD_DELTA :
                                                                                C3HPPC_SWS_QM_LE10G_FLOW_CTRL_THOLD_DELTA;
    }
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, FLOW_CTRL_THRESHf, &uFieldValue );
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MIN_PAGES_DATAf, &uQueueMinPages );
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MIN_PAGES_HEADERf, &uQueueMinPages );
    uFieldValue = 258; 
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, DROP_THRESH_DE1f, &uFieldValue );
    uFieldValue = 172; 
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, DROP_THRESH_DE2f, &uFieldValue );
    uFieldValue = 1; 
    soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, ENABLEf, &uFieldValue );

    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                       MEM_BLOCK_ANY, nSourceQueue, auSourceQueueConfigEntry) );
  }

  /* Reserve a static page for non-packet based bubble work -- LRP based learning or OAM frame generation. */ 
  if ( g_bLrpLearningEnable || g_bOamEnable ) {

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, QM_REPLICATION_REFm, 
                                      MEM_BLOCK_ANY, 0x3fff, auReplicationRefEntry) );
    uFieldValue = 0xfff; 
    soc_mem_field_set( nUnit, QM_REPLICATION_REFm, auReplicationRefEntry, REF_CNTf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, QM_REPLICATION_REFm, 
                                       MEM_BLOCK_ANY, 0x3fff, auReplicationRefEntry) );
  }

  READ_QM_AGER_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, QM_AGER_CONFIGr, &uRegisterValue, QUEUE_AGE_THRESHOLDf, 1 );
  soc_reg_field_set( nUnit, QM_AGER_CONFIGr, &uRegisterValue, BUFFER_AGE_THRESHOLDf, 14 );
  soc_reg_field_set( nUnit, QM_AGER_CONFIGr, &uRegisterValue, AGER_CHECKER_ENABLEf, 1 );
  soc_reg_field_set( nUnit, QM_AGER_CONFIGr, &uRegisterValue, AGE_INCREMENTER_ENABLEf, 1 );
  WRITE_QM_AGER_CONFIGr( nUnit, uRegisterValue );


  return 0;
}


int c3hppc_sws_check_free_page_list( int nUnit ) {

  int nIndex, rc, nInstance, nPrCount, nMissingCount, nExpectedMissingCount;
  char *acFreePageScoreBoard;
  uint32 uRegisterValue, uFreePage;
  sal_time_t TimeStamp;
  int nLimitCounter;
  uint32 auSourceQueueState0[2], auSourceQueueState1[2], auDestQueueState[2];
  uint32 uField0, uField1, uField2;

  acFreePageScoreBoard = (char *) sal_alloc( C3HPPC_SWS_FREE_PAGE_NUM, "Scoreboard" );
  sal_memset( acFreePageScoreBoard, 0x00, C3HPPC_SWS_FREE_PAGE_NUM );
  rc = 0;

  /* It takes in real time a second per buffer to do the checks in emulation.  Can't wait 16K seconds. */
  if ( !SAL_BOOT_QUICKTURN ) {

    for ( nIndex = 0; nIndex < C3HPPC_SWS_FREE_PAGE_NUM; ++nIndex ) {
      uRegisterValue = 0;
      soc_reg_field_set( nUnit, QM_FP_CONFIG1r, &uRegisterValue, SW_FP_POP_UNDERFLOWf, 1 );
      soc_reg_field_set( nUnit, QM_FP_CONFIG1r, &uRegisterValue, SW_FP_OP_ACKf, 1 );
      soc_reg_field_set( nUnit, QM_FP_CONFIG1r, &uRegisterValue, SW_FP_OPf, 1 );
      soc_reg_field_set( nUnit, QM_FP_CONFIG1r, &uRegisterValue, SW_FP_GOf, 1 );
      WRITE_QM_FP_CONFIG1r( nUnit, uRegisterValue );

      if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, QM_FP_CONFIG1r, SW_FP_OP_ACKf, 1, 1, 0, &TimeStamp ) ) {
        sal_free( acFreePageScoreBoard );
        return -1;
      } else { 
        READ_QM_FP_CONFIG1r( nUnit, &uRegisterValue );
        if ( soc_reg_field_get( nUnit, QM_FP_CONFIG1r, uRegisterValue, SW_FP_POP_UNDERFLOWf ) == 0 ) {
          uFreePage = soc_reg_field_get( nUnit, QM_FP_CONFIG1r, uRegisterValue, SW_FP_PAGEf );
          ++acFreePageScoreBoard[uFreePage];
        }
      }
    }

    for ( nIndex = 0, nPrCount = 0; nIndex < C3HPPC_SWS_FREE_PAGE_NUM; ++nIndex ) {
      if ( acFreePageScoreBoard[nIndex] == 0 ) {
/*
        cli_out("<c3hppc_sws_check_free_page_list>  Checking PR for free page [%d]\n", nIndex );
*/
        ++nPrCount;

        for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, PR_IDP_BUFFER_SEEN_CHECK_RESPONSEr, &uRegisterValue, CHECK_COMPLETEf, 1 );
          soc_reg32_set( nUnit, PR_IDP_BUFFER_SEEN_CHECK_RESPONSEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, PR_IDP_BUFFER_SEEN_CHECKr, &uRegisterValue, INITIATE_CHECKf, 1 );
          soc_reg_field_set( nUnit, PR_IDP_BUFFER_SEEN_CHECKr, &uRegisterValue, BUFFER_PTRf, (uint32) nIndex );
          soc_reg32_set( nUnit, PR_IDP_BUFFER_SEEN_CHECKr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

          if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_BUFFER_SEEN_CHECK_RESPONSEr,
                                       CHECK_COMPLETEf, 1, 1, 0, &TimeStamp ) ) {
            sal_free( acFreePageScoreBoard );
            return -1;
          } else {
            soc_reg32_get( nUnit, PR_IDP_BUFFER_SEEN_CHECK_RESPONSEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
            if ( soc_reg_field_get( nUnit, PR_IDP_BUFFER_SEEN_CHECK_RESPONSEr, uRegisterValue, CHECK_STATUSf ) ) {
              ++acFreePageScoreBoard[nIndex];
/*
              cli_out("<c3hppc_sws_check_free_page_list>  Free page [%d] found in PR%d\n", nIndex, nInstance );
*/
            }
          }
        }
      }
    }

    for ( nIndex = 0, nMissingCount = 0; nIndex < C3HPPC_SWS_FREE_PAGE_NUM; ++nIndex ) {
      if ( acFreePageScoreBoard[nIndex] == 0 ) {
        ++nMissingCount;
      } else if ( acFreePageScoreBoard[nIndex] != 1 )  {
        cli_out("\n\n<c3hppc_sws_check_free_page_list>  Free page [%d] accounted for MULTIPLE times!\n", nIndex );
        rc = 1;
      }
    }

  /*
    Hi Dan,
    The 14 pages are in the HPRE free page cache, their page numbers are not accessible. Please see http://engjira.sj.broadcom.com/browse/CA3-1943
    Thanks,
    -Dominic
  */

    nExpectedMissingCount = ( g_bLrpLearningEnable == 1 || g_bOamEnable == 1 ) ? 15 : 14;

    if ( nMissingCount != nExpectedMissingCount ) {
      for ( nIndex = 0, nLimitCounter = 0; nIndex < C3HPPC_SWS_FREE_PAGE_NUM; ++nIndex ) {
        if ( acFreePageScoreBoard[nIndex] == 0 && nLimitCounter < 64 ) {
          cli_out("\n\n<c3hppc_sws_check_free_page_list>  Free page [%d] NOT accounted for!\n", nIndex );
          ++nLimitCounter;
          rc = 1;
        }
      }
      cli_out("\n\n<c3hppc_sws_check_free_page_list>  %d of %d PR(non QM) free pages are MISSING!\n", nMissingCount, nPrCount );
    }

  } /* if ( !SAL_BOOT_QUICKTURN )  */


  READ_QM_BUFFER_STATUS2r( nUnit, &uRegisterValue );
  uField0 = soc_reg_field_get( nUnit, QM_BUFFER_STATUS2r, uRegisterValue, INGRESS_IF_PAGES_ALLOCATEDf );
  if ( uField0 ) {
    cli_out("\n\n" );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), QM_BUFFER_STATUS2r );
  }

  READ_QM_BUFFER_STATUS3r( nUnit, &uRegisterValue );
  uField1 = soc_reg_field_get( nUnit, QM_BUFFER_STATUS3r, uRegisterValue, EGRESS_IF_PAGES_ALLOCATEDf );
  if ( uField1 ) {
    cli_out("\n\n" );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), QM_BUFFER_STATUS3r );
  }


  READ_QM_BUFFER_STATUS0r( nUnit, &uRegisterValue );
  if ( uRegisterValue || uField0 || uField1 ) {
    cli_out("\n\n" );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), QM_BUFFER_STATUS0r );
    cli_out("\n\n" );
    for ( nIndex = 0; nIndex < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nIndex ) {
      rc = soc_mem_read( nUnit, QM_SOURCE_QUEUE_STATE0m, MEM_BLOCK_ANY, nIndex, auSourceQueueState0);
      if (SOC_FAILURE(rc)) {
        sal_free( acFreePageScoreBoard );
        return (rc);
      }

      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE0m, auSourceQueueState0, PT_PREFETCH_STATEf, &uField0 );
      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE0m, auSourceQueueState0, PAGES_STOREDf, &uField1 );
      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE0m, auSourceQueueState0, AVAILABLEf, &uField2 );
      if ( uField0 || uField1 || uField2 ) {
        cli_out("<c3hppc_sws_check_free_page_list>  QM_SOURCE_QUEUE_STATE0[%d] PT_PREFETCH_STATE=%d PAGES_STORED=%d AVAILABLE=%d\n",
                nIndex, uField0, uField1, uField2 );
      }
      rc = soc_mem_read( nUnit, QM_SOURCE_QUEUE_STATE1m, MEM_BLOCK_ANY, nIndex, auSourceQueueState1);
      if (SOC_FAILURE(rc)) {
        sal_free( acFreePageScoreBoard );
        return (rc);
      }
      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE1m, auSourceQueueState1, OVERFLOWf, &uField0 );
      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE1m, auSourceQueueState1, HEADER_PAGES_ALLOCATEDf, &uField1 );
      soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_STATE1m, auSourceQueueState1, DATA_PAGES_ALLOCATEDf, &uField2 );
      if ( uField0 || uField1 || uField2 ) {
        cli_out("<c3hppc_sws_check_free_page_list>  QM_SOURCE_QUEUE_STATE1[%d] OVERFLOW=%d HEADER_PAGES_ALLOCATED=%d DATA_PAGES_ALLOCATED=%d\n",
                nIndex, uField0, uField1, uField2 );
      }

    }

    cli_out("\n\n" );
    for ( nIndex = 0; nIndex < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nIndex ) {
      rc = soc_mem_read( nUnit, QM_DEST_QUEUE_STATEm, MEM_BLOCK_ANY, nIndex, auDestQueueState);
      if (SOC_SUCCESS(rc)) {
        soc_mem_field_get( nUnit, QM_DEST_QUEUE_STATEm, auDestQueueState, PAGES_STOREDf, &uField2 );
        soc_mem_field_get( nUnit, QM_DEST_QUEUE_STATEm, auDestQueueState, QUEUE_AGEf, &uField1 );
        if ( uField1 || uField2 ) {
          cli_out("<c3hppc_sws_check_free_page_list>  QM_DEST_QUEUE_STATE[%d] PAGES_STORED=%d QUEUE_AGE=%d\n",
                  nIndex, uField2, uField1 );
        }
      }
    }

    rc = 1;
  }



  sal_free( acFreePageScoreBoard );

  return  rc;
}


int c3hppc_sws_pr_bringup( int nUnit, c3hppc_sws_pr_control_info_t *pPrControlInfo ) {

  uint32 uRegisterValue, auPortDefaultTableEntry[1], uFieldValue;
  int nInstance, nChannelorPort, nSourceQueue;
  uint32 uRxPortPageStart, uRxPerPortPageDepth, uLookupsRequired;
  c3hppc_sws_pr_tcam_data_t TcamData, TcamMask;
  c3hppc_sws_pr_tcam_data_first_lookup_t FirstTcamData, FirstTcamMask;
  uint32 auTcamEntry[16], auTcamRamEntry[2];
  uint32 uParseStage, uParseStageNum;
  int nTcamIndex, nRxPort, nRxPortNum, nRxPortIncrement;
  uint8 bIlPacketGeneratorBug = 0;
  uint8 bCommonConfig = 0;

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {

  
    soc_reg32_get( nUnit, PR_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_GLOBAL_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1 );
    soc_reg32_set( nUnit, PR_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg_field_set( nUnit, PR_GLOBAL_CONFIGr, &uRegisterValue, INITf, 1 );
    soc_reg32_set( nUnit, PR_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr, &uRegisterValue, PERIODf, 0x32 );
    soc_reg32_set( nUnit, PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, CL_CLIENT_IFf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, XT0_CLIENT_IFf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, XT1_CLIENT_IFf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, XT2_CLIENT_IFf, 1 );
    } else if ( (g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ||
                 g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, CL_CLIENT_IFf, 1 );
    } else {
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, IL_CLIENT_IFf, 1 );
    }
    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, CMIC_CLIENT_IFf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uRegisterValue, XL_CLIENT_IFf, 1 );
    }
    soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG1r, &uRegisterValue, IL_BASE_PORTf, 0 );
    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG1r, &uRegisterValue, XT0_BASE_PORTf, 12 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG1r, &uRegisterValue, XT1_BASE_PORTf, 24 );
    }
    soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG2r, &uRegisterValue, XT2_BASE_PORTf, 36 );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG2r, &uRegisterValue, XL_BASE_PORTf, C3HPPC_SWS_PR_XL_BASE_PORT );
      soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG2r, &uRegisterValue, CMIC_BASE_PORTf, 50 );
      soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    bCommonConfig = 0;
    if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
          g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {
        nRxPortNum = 48;
        uRxPerPortPageDepth = 12;
        nRxPortIncrement = 1;
      } else if ( g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL && g_bHotSwapMix_10G_1G ) {
        nRxPortNum = 12;
        uRxPerPortPageDepth = 32;
        nRxPortIncrement = 1;
      } else {  /* C3HPPC_SWS_TDM__3x40G_BY_100IL and C3HPPC_SWS_TDM__12x10G_BY_100IL share a common config. */
        nRxPortNum = 12;
        uRxPerPortPageDepth = 40;
        nRxPortIncrement = 1;
        bCommonConfig = 1;   /* For C3HPPC_SWS_TDM__3x40G_BY_100IL need an additional 32 */
      }
    } else {
        nRxPortNum = 1;
        uRxPerPortPageDepth = 0x240;
        nRxPortIncrement = 1;
    }

    for ( nRxPort = 0, uRxPortPageStart = 0; nRxPort < nRxPortNum; ++nRxPort ) {
      soc_reg32_get( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), (nRxPort*nRxPortIncrement), &uRegisterValue );
      soc_reg_field_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, &uRegisterValue, START_ADDRESSf, uRxPortPageStart );
      soc_reg_field_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, &uRegisterValue, DEPTHf,
                                                  ((bCommonConfig && !(nRxPort%4)) ? (uRxPerPortPageDepth + 32) : uRxPerPortPageDepth) );
      soc_reg32_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), (nRxPort*nRxPortIncrement), uRegisterValue );
      uRxPortPageStart += uRxPerPortPageDepth;
      if ( bCommonConfig && !(nRxPort%4) ) uRxPortPageStart += 32;
    }
    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      for ( nRxPort = ( g_bHotSwapMix_10G_1G ? 32 : 48), uRxPerPortPageDepth = 12; 
            nRxPort < 51;
            ++nRxPort, uRxPortPageStart += uRxPerPortPageDepth ) {
        soc_reg32_get( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nRxPort, &uRegisterValue );
        soc_reg_field_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, &uRegisterValue, START_ADDRESSf, uRxPortPageStart );
        soc_reg_field_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, &uRegisterValue, DEPTHf, uRxPerPortPageDepth );
        soc_reg32_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nRxPort, uRegisterValue );
      }
    }
    soc_reg32_get( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFG_COMPLETEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFG_COMPLETEr, &uRegisterValue, CONFIGURATION_COMPLETEf, 1 );
    soc_reg32_set( nUnit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFG_COMPLETEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    uRegisterValue = 0xffffffff;
    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg32_set( nUnit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_031_000r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
      soc_reg32_set( nUnit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_063_032r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    } else {
      soc_reg32_set( nUnit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_095_064r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
      soc_reg32_set( nUnit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_127_096r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    soc_reg32_get( nUnit, PR_IDP_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IDP_CONFIGr, &uRegisterValue, DE0_THRESHOLDf, 255 );
    soc_reg_field_set( nUnit, PR_IDP_CONFIGr, &uRegisterValue, DE1_THRESHOLDf, 192 );
    soc_reg_field_set( nUnit, PR_IDP_CONFIGr, &uRegisterValue, DE2_THRESHOLDf, 128 );
    soc_reg_field_set( nUnit, PR_IDP_CONFIGr, &uRegisterValue, DE3_THRESHOLDf, 0 );
    soc_reg32_set( nUnit, PR_IDP_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, &uRegisterValue, NFULL_THRESHOLDf,
                                                             ((nInstance == C3HPPC_SWS_LINE_INTERFACE) ? 207 : 228) );
    soc_reg32_set( nUnit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PR_IPRE_FC_TX_STATE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IPRE_FC_TX_STATE_CONFIG2r, &uRegisterValue, ENABLE_ENQ_REQ_FIFO_XOFFf, 1 );
    soc_reg_field_set( nUnit, PR_IPRE_FC_TX_STATE_CONFIG2r, &uRegisterValue, ENABLE_LINE_LLFCf,
                                                           (nInstance == C3HPPC_SWS_LINE_INTERFACE ? 1 : 0) );
    soc_reg_field_set( nUnit, PR_IPRE_FC_TX_STATE_CONFIG2r, &uRegisterValue, ENABLE_FABRIC_LLFCf,
                                                           (nInstance == C3HPPC_SWS_LINE_INTERFACE ? 0 : 1) );
    soc_reg32_set( nUnit, PR_IPRE_FC_TX_STATE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    /* Work-around for IL interface flow control polarity bug. */
    if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
          g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
    } else {
      soc_reg32_get( nUnit, PR_IPRE_IL_TX_LLFCr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PR_IPRE_IL_TX_LLFCr, &uRegisterValue, FC_LOGICAL_OR_MAPPINGf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_IL_TX_LLFCr, &uRegisterValue, SOFTWARE_CONTROLLED_XOFF_XONf, 0 );
      soc_reg32_set( nUnit, PR_IPRE_IL_TX_LLFCr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    nSourceQueue = ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 0x00 : 0x40;
    for ( nChannelorPort = 0; nChannelorPort < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannelorPort ) {
      uRegisterValue = 0;
      soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, SQUEUEf, (uint32) (nSourceQueue+nChannelorPort) );
      soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, FC_ENABLEf, 1 );
      soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, FC_LOGICAL_OR_MAPPINGf,
                                                             (nInstance == C3HPPC_SWS_LINE_INTERFACE ? 13 : 11) );
      if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
            g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL) && 
           nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
        /* Added for B0 */
        if ( g_rev_id == BCM88030_B0_REV_ID && g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ) {
          soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, SQUEUEf, (uint32) ((nSourceQueue+nChannelorPort)/4) );
        }
        soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, FC_INTERFACE_MAPPINGf, 1 );
      } else {
        if ( g_bHotSwapMix_10G_1G && nInstance == C3HPPC_SWS_FABRIC_INTERFACE && 
             nChannelorPort >= 12 && nChannelorPort < 28 ) {
          soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, SQUEUEf, (uint32) ((nSourceQueue+nChannelorPort)+20) );
        } else if ( g_bXLportAndCmicPortsActive && nInstance == C3HPPC_SWS_FABRIC_INTERFACE &&
                    g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL && (nChannelorPort == 12 || nChannelorPort == 13) ) {
          soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, SQUEUEf, (uint32) ((nSourceQueue+nChannelorPort)+49) );
        } else if ( g_bXLportAndCmicPortsActive && nInstance == C3HPPC_SWS_FABRIC_INTERFACE &&
                    g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && (nChannelorPort == 48 || nChannelorPort == 49) ) {
          soc_reg_field_set( nUnit, PR_IPRE_FC_CONFIGr, &uRegisterValue, SQUEUEf, (uint32) ((nSourceQueue+nChannelorPort)+13) );
        }
      }
      soc_reg32_set( nUnit, PR_IPRE_FC_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), nChannelorPort, uRegisterValue );
    }





    soc_reg32_get( nUnit, PR_IDP_POLICER_DISABLEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PR_IDP_POLICER_DISABLEr, &uRegisterValue, DISABLE_POLICERf, 1 );
    soc_reg32_set( nUnit, PR_IDP_POLICER_DISABLEr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    if ( pPrControlInfo->bBypassParsing ) {

      /* ICC bypass config ... */
      soc_reg32_get( nUnit, PR_ICC_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PR_ICC_CONFIG0r, &uRegisterValue, OPERATION_TYPEf, 1 );
      soc_reg32_set( nUnit, PR_ICC_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      soc_reg32_get( nUnit, PR_ICC_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PR_ICC_CONFIG1r, &uRegisterValue, QUEUE_BASEf, ((nInstance == C3HPPC_SWS_LINE_INTERFACE) ? 0x00 : 0x40) );
      soc_reg32_set( nUnit, PR_ICC_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    } else {

      /*
         Due to the IL packet generator bug the PR ICC look-ups will have a "catch all" filter approach.
         I am 100% sure that the "RxPort" the PR uses for the look-up is always 0.
      */
      if ( ((g_bLineTrafficGenOnly && nInstance == C3HPPC_SWS_LINE_INTERFACE) ||
            (g_bFabricTrafficGenOnly && nInstance == C3HPPC_SWS_FABRIC_INTERFACE) ||
            (g_bLineTrafficGenOnly == 0 && g_bFabricTrafficGenOnly == 0)) && 
           (g_rev_id == BCM88030_A0_REV_ID || g_rev_id == BCM88030_A1_REV_ID) ) {
        bIlPacketGeneratorBug = 1;
      } else {
        bIlPacketGeneratorBug = 0;
      }

      /* Defaulting "uLookupsRequired" to be max value of 3 as this is a design constraint. */
      uLookupsRequired = 3;
      /*
         For the BCM88034 B0 device: 
           The error "PR_ICC_ERROR_0.pr0.PKT_CTXT_DATA_BUFFER_WR_STATE_MACHINE_WRITE_ERROR" occurs if
           "uLookupsRequired" is greater than 2 due to JIRA CA3-3087.
      */
      if ( g_dev_id == BCM88034_DEVICE_ID && g_rev_id == BCM88030_B0_REV_ID && nInstance == C3HPPC_SWS_LINE_INTERFACE &&
           g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {
        uLookupsRequired = 2;
      }
      soc_reg32_get( nUnit, PR_ICC_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PR_ICC_CONFIG0r, &uRegisterValue, OPERATION_TYPEf, 0 );
      soc_reg_field_set( nUnit, PR_ICC_CONFIG0r, &uRegisterValue, LOOKUPS_REQUIREDf, uLookupsRequired );
      soc_reg32_set( nUnit, PR_ICC_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nRxPort = 0; nRxPort < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nRxPort ) {

        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, 
                                          PR_BLOCK(nUnit,nInstance), nRxPort, auPortDefaultTableEntry) );
        uFieldValue = 0;
        soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, auPortDefaultTableEntry, POLICER_BASEf, &uFieldValue );
        soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, auPortDefaultTableEntry, PROFILEf, &uFieldValue );
        uFieldValue = ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 0 : C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM;
        if ( bIlPacketGeneratorBug ) {
          /* A "QUEUE_ACTION" of 3(below) will use this default to resolve the QUEUE. */
          uFieldValue += nRxPort;
        }
        soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, auPortDefaultTableEntry, DEFAULT_QUEUEf, &uFieldValue );
  
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, 
                                           PR_BLOCK(nUnit,nInstance), nRxPort, auPortDefaultTableEntry) );
      }

      uParseStageNum = ( bIlPacketGeneratorBug == 1 ) ? 1 : uLookupsRequired;

      for ( uParseStage = 0; uParseStage < uParseStageNum; ++uParseStage ) {

        for ( nRxPort = 0; nRxPort < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nRxPort ) {
 
          nTcamIndex = ( C3HPPC_SWS_IL_MAX_CHANNEL_NUM * uParseStage ) + nRxPort;
      
          if ( uParseStage == 0 ) {

            sal_memset( &FirstTcamData, 0x00, sizeof(c3hppc_sws_pr_tcam_data_first_lookup_t) );
            sal_memset( &FirstTcamMask, 0x00, sizeof(c3hppc_sws_pr_tcam_data_first_lookup_t) );
  
            FirstTcamData.Word0.bits.RxPort = ( bIlPacketGeneratorBug == 1 ) ? 0 : nRxPort;
            FirstTcamMask.Word0.value = ( bIlPacketGeneratorBug == 1 ) ? 0x000000 : 0xffffff;
  
            if ( bIlPacketGeneratorBug == 0 ) {
              /* Packet bytes 0 to 3 --> 0x11222233 */ 
              FirstTcamData.Word7.bits.HdrData_199to190 = 0x044;
              FirstTcamMask.Word7.bits.HdrData_199to190 = 0x3ff;
              FirstTcamData.Word6.bits.HdrData_189to158 = 0x8888cc00; 
              FirstTcamMask.Word6.bits.HdrData_189to158 = 0xfffffc00;
            }

          } else {

            sal_memset( &TcamData, 0x00, sizeof(c3hppc_sws_pr_tcam_data_t) );
            sal_memset( &TcamMask, 0x00, sizeof(c3hppc_sws_pr_tcam_data_t) );

            TcamData.Word0.bits.State = nRxPort | ( uParseStage << 8 );
            TcamMask.Word0.bits.State = 0xffffff;

            if ( uParseStage == 1 ) {
         
              /* Packet bytes 4 to 7 --> 0x33334444 */ 
              TcamData.Word7.bits.HdrData_199to190 = 0x0cc;
              TcamMask.Word7.bits.HdrData_199to190 = 0x3ff;
              TcamData.Word6.bits.HdrData_189to158 = 0xcd111000; 
              TcamMask.Word6.bits.HdrData_189to158 = 0xfffffc00;
      
            } else if ( uParseStage == 2) {
      
              /* Packet bytes 8 to 11 --> 0x44445555 */ 
              TcamData.Word7.bits.HdrData_199to190 = 0x111;
              TcamMask.Word7.bits.HdrData_199to190 = 0x3ff;
              TcamData.Word6.bits.HdrData_189to158 = 0x11555400; 
              TcamMask.Word6.bits.HdrData_189to158 = 0xfffffc00;
      
            } else if ( uParseStage == 3) {
      
              /* Packet bytes 12 to 15 --> 0x55555566 */ 
              TcamData.Word7.bits.HdrData_199to190 = 0x155;
              TcamMask.Word7.bits.HdrData_199to190 = 0x3ff;
              TcamData.Word6.bits.HdrData_189to158 = 0x55559800; 
              TcamMask.Word6.bits.HdrData_189to158 = 0xfffffc00;
    
            } else {
  
/*
              TcamData.Word6.bits.HdrData_24to24 = 
                      ( nRxPort < C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM ) ? 0x55 : 0x00;
              TcamMask.Word6.bits.HdrData_24to24 = 0xff;
*/
  
            }

          }
  
          SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PR_ICC_LOOKUP_CORE_TCAM_TABLEm, 
                                            PR_BLOCK(nUnit,nInstance), nTcamIndex, auTcamEntry) ); 
          soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_TCAM_TABLEm, auTcamEntry, KEYf, 
                                            ( ( uParseStage == 0 ) ? (uint32 *) &FirstTcamData : (uint32 *) &TcamData ) );
          soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_TCAM_TABLEm, auTcamEntry, MSKf,
                                            ( ( uParseStage == 0 ) ? (uint32 *) &FirstTcamMask : (uint32 *) &TcamMask ) );
          uFieldValue = 3;
          soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_TCAM_TABLEm, auTcamEntry, VALIDf, &uFieldValue );
          SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PR_ICC_LOOKUP_CORE_TCAM_TABLEm,
                                             PR_BLOCK(nUnit,nInstance), nTcamIndex, auTcamEntry) );
    
          SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm,
                                            PR_BLOCK(nUnit,nInstance), nTcamIndex, auTcamRamEntry) );
          if ( (uParseStage+1) == uParseStageNum ) {
            uFieldValue = 1;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, LASTf, &uFieldValue );
            uFieldValue = 3;   /* 3 --> default_queue(from above) + match_queue */
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, QUEUE_ACTIONf, &uFieldValue );
            uFieldValue = nRxPort / ((g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE)  ? 4 : 1);
            /*
              To more closely achieve line rate from the fabric side IL packet generator keeping the channel space contiguous
              as to not introduce delay in finding the next available channel to transmit on.  In order to keep the rest of the
              SWS config the same the source queue is adjusted so the traffic goes out the desired lineside TX port.  For example,
              IL channels 12 and 13 drive XL ports 1(SrcQ61) and 2(SrcQ62) in the C3HPPC_SWS_TDM__12x10G_BY_100IL TDM.
            */  
            if ( g_bHotSwapMix_10G_1G && nInstance == C3HPPC_SWS_FABRIC_INTERFACE ) {
              if ( nRxPort >= 12 ) uFieldValue += 20; 
            } else if ( g_bXLportAndCmicPortsActive && g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL && 
                        nInstance == C3HPPC_SWS_FABRIC_INTERFACE ) {
              if ( nRxPort >= 12 ) uFieldValue += 49; 
            } else if ( g_bXLportAndCmicPortsActive && g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && 
                        nInstance == C3HPPC_SWS_FABRIC_INTERFACE ) {
              if ( nRxPort >= 48 ) uFieldValue += 13; 
            }
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, QUEUEf, &uFieldValue );
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, DROPf, &uFieldValue );
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, COSf, &uFieldValue );
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, SELECT_DEf, &uFieldValue );
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, DEFAULT_DEf, &uFieldValue );
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, DPf, &uFieldValue );
          } else {
            uFieldValue = 0;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, LASTf, &uFieldValue );
            uFieldValue = 4;
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, SHIFTf, &uFieldValue );
            uFieldValue = nRxPort | ( (uParseStage+1) << 8 );
            soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, STATEf, &uFieldValue );
          }
          SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm,
                                             PR_BLOCK(nUnit,nInstance), nTcamIndex, auTcamRamEntry) );
        }
      }

    }
    

  }

  return 0;
}


int c3hppc_sws_pt_bringup( int nUnit, c3hppc_sws_pt_control_info_t *pPtControlInfo, c3hppc_sws_il_control_info_t *pIlControlInfo ) {

  uint32 uRegisterValue, uFieldValue, uDQueueLookupEntry, uClientCalEntry, uPortCalEntry;
  int nInstance, nSourceQueue, nIndex, nField, nTxPortNum, nTxPortIncr;
  uint32 uQ2CmapEntry, uTxPort, uPortFifoBaseAddress, uSize, uThreshold;

  uQ2CmapEntry = nTxPortIncr = 0;

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PT_INSTANCE_NUM; ++nInstance ) {

    uFieldValue = 0;
    if ( pIlControlInfo[nInstance].bBringUp && !SAL_BOOT_QUICKTURN ) {
      soc_reg32_get( nUnit, IL_RX_CORE_STATUS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      uFieldValue = soc_reg_field_get( nUnit, IL_RX_CORE_STATUS1r, uRegisterValue, STAT_RX_ALIGNEDf );
    }

    if ( uFieldValue ) {

      continue;

    } else {

      nField = ( nInstance == C3HPPC_SWS_FABRIC_INTERFACE ) ? PT1_RESET_Nf : PT0_RESET_Nf;
      READ_CX_SOFT_RESET_0r( nUnit, &uRegisterValue );
      soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, nField, 0 );
      WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );
      soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, nField, 1 );
      WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );

    }
  
    soc_reg32_get( nUnit, PT_GEN_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_GEN_CONFIGr, &uRegisterValue, PT_SOFT_RESET_Nf, 1 );
    soc_reg32_set( nUnit, PT_GEN_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
    soc_reg32_get( nUnit, PT_MEM_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_MEM_INITr, &uRegisterValue, PT_DO_INITf, 1 );
    soc_reg32_set( nUnit, PT_MEM_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PT_GEN_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_GEN_CONFIGr, &uRegisterValue, PT_ENABLEf, 1 );
    soc_reg32_set( nUnit, PT_GEN_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    /* 
       Enable over-subscription
    */
    soc_reg32_set( nUnit, PT_IPTE_CONFIG3r, SOC_BLOCK_PORT(nUnit,nInstance), 0, 0x07ffffff );

    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      uRegisterValue = 0;
      soc_reg_field_set( nUnit, PT_HPTE_CONFIGr, &uRegisterValue, PT_HPTE_INGRESS_QUEUE_RANGEf, 0x40 );
      soc_reg_field_set( nUnit, PT_HPTE_CONFIGr, &uRegisterValue, PT_HPTE_INGRESS_BASE_QUEUEf, 0x00 );
      soc_reg_field_set( nUnit, PT_HPTE_CONFIGr, &uRegisterValue, PT_HPTE_EGRESS_QUEUE_RANGEf, 0x40 );
      soc_reg_field_set( nUnit, PT_HPTE_CONFIGr, &uRegisterValue, PT_HPTE_EGRESS_BASE_QUEUEf, 0x40 );
      soc_reg32_set( nUnit, PT_HPTE_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nSourceQueue = 0; nSourceQueue < C3HPPC_SWS_SOURCE_QUEUE_NUM; ++nSourceQueue ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, HPTE_DQUEUE_LOOKUPm,
                                          PT_BLOCK(nUnit,nInstance), nSourceQueue, &uDQueueLookupEntry) );
        uFieldValue = C3HPPC_SWS_SOURCE_QUEUE_NUM + nSourceQueue; 
        soc_mem_field_set( nUnit, HPTE_DQUEUE_LOOKUPm, &uDQueueLookupEntry, DQUEUEf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, HPTE_DQUEUE_LOOKUPm,
                                           PT_BLOCK(nUnit,nInstance), nSourceQueue, &uDQueueLookupEntry) );
      }

      if ( g_bOamEnable ) {
        soc_reg32_get( nUnit, PT_HPTE_EGRESS_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
        soc_reg_field_set( nUnit, PT_HPTE_EGRESS_CONFIGr, &uRegisterValue, PT_HPTE_EGRESS_REDIRECTION_QUEUE_EN0f, 1 );
        soc_reg_field_set( nUnit, PT_HPTE_EGRESS_CONFIGr, &uRegisterValue, PT_HPTE_EGRESS_REDIRECTION_QUEUE_ID0f, 0x3e );
        soc_reg32_set( nUnit, PT_HPTE_EGRESS_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

        soc_reg32_get( nUnit, PT_HPTE_INGRESS_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
        soc_reg_field_set( nUnit, PT_HPTE_INGRESS_CONFIGr, &uRegisterValue, PT_HPTE_INGRESS_REDIRECTION_QUEUE_EN0f, 1 );
        soc_reg_field_set( nUnit, PT_HPTE_INGRESS_CONFIGr, &uRegisterValue, PT_HPTE_INGRESS_REDIRECTION_QUEUE_ID0f, 0x3e );
        soc_reg32_set( nUnit, PT_HPTE_INGRESS_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
      }
    }

    soc_reg32_get( nUnit, PT_IPTE_QM_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_IPTE_QM_CONFIGr, &uRegisterValue, PT_IPTE_GRP0_QUEUE_RANGEf, 0x40 );
    soc_reg_field_set( nUnit, PT_IPTE_QM_CONFIGr, &uRegisterValue, PT_IPTE_GRP0_BASE_QUEUEf, 
                                         ((nInstance == C3HPPC_SWS_LINE_INTERFACE) ? 0xc0 : 0x80) );
    soc_reg32_set( nUnit, PT_IPTE_QM_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
          g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT0_BASE_QUEUEf, 0 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT0_QUEUE_RANGEf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT1_BASE_QUEUEf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT1_QUEUE_RANGEf, 1 );
      /* Added for B0 */
      if ( g_rev_id == BCM88030_B0_REV_ID ) {
        /*
           This disables the remap of PT ports 0,1,2 to CLPORT ports 0,4,8.  All internal
           40G port references (calendars, mapping tables etc.) must use 0,4,8 notation.   
        */
        soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT_REMAP_ENf, 0 );
      }
    } else {                  
      uFieldValue = ( nInstance == C3HPPC_SWS_LINE_INTERFACE && g_bXLportAndCmicPortsActive ) ? 0x3d : 0x40;
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG0r, &uRegisterValue, PT_IPTE_PORT0_QUEUE_RANGEf, uFieldValue ); 
    }
    soc_reg32_set( nUnit, PT_IPTE_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
          g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG1r, &uRegisterValue, PT_IPTE_PORT2_BASE_QUEUEf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG1r, &uRegisterValue, PT_IPTE_PORT2_QUEUE_RANGEf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG1r, &uRegisterValue, PT_IPTE_PORT3_PORT47_BASE_PORTf, 3 );
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG1r, &uRegisterValue, PT_IPTE_PORT3_PORT47_BASE_QUEUEf, 3 );
    } else {
      soc_reg_field_set( nUnit, PT_IPTE_CONFIG1r, &uRegisterValue, PT_IPTE_PORT3_PORT47_BASE_PORTf, 0 );
    }
    soc_reg32_set( nUnit, PT_IPTE_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    /* Added for B0 */
    if ( g_rev_id == BCM88030_B0_REV_ID ) {
      soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
        soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uRegisterValue, PT_IPTE_PORT12_PORT47_BASE_PORTf, 12 );
        soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uRegisterValue, PT_IPTE_PORT12_PORT47_BASE_QUEUEf, 12 );
      }
      soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    soc_reg32_get( nUnit, PT_IPTE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG2r, &uRegisterValue, PT_IPTE_PORT48_PORT50_BASE_QUEUEf,
                                                ((nInstance == C3HPPC_SWS_LINE_INTERFACE) ? 0x3d : 0) );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG2r, &uRegisterValue, PT_IPTE_PORT48_PORT50_BASE_PORTf,
                                                ((nInstance == C3HPPC_SWS_LINE_INTERFACE) ? 0x30 : 0) );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg32_get( nUnit, PT_IPTE_SCHEDULER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_IPTE_SCHEDULER_CFGr, &uRegisterValue, PT_IPTE_WDRR_QUANTUMf, 3 );
    soc_reg32_set( nUnit, PT_IPTE_SCHEDULER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    soc_reg_field_set( nUnit, PT_IPTE_FC_CHAN_CFG0r, &uRegisterValue, ENABLEf, 0xffffffff );
    soc_reg32_set( nUnit, PT_IPTE_FC_CHAN_CFG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg_field_set( nUnit, PT_IPTE_FC_CHAN_CFG1r, &uRegisterValue, ENABLEf, 0xffffffff );
    soc_reg32_set( nUnit, PT_IPTE_FC_CHAN_CFG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {

      for ( nIndex = 0; nIndex < 53; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_CLIENT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE0f, &uFieldValue );
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE1f, &uFieldValue );

        if ( nIndex == 0 || nIndex == 2 || nIndex == 4 || nIndex == 13 || nIndex == 15 ||
             nIndex == 17 || nIndex == 26 || nIndex == 35 || nIndex == 37 || nIndex == 39 ||
             nIndex == 48 ) {
          uFieldValue = 0;
        } else if ( nIndex == 7 || nIndex == 9 || nIndex == 11 || nIndex == 20 || nIndex == 22 || 
                    nIndex == 27 || nIndex == 29 || nIndex == 31 || nIndex == 40 || nIndex == 42 ||
                    nIndex == 44 || nIndex == 49 || nIndex == 51 ) {
          uFieldValue = 1; 
        } else if ( nIndex == 1 || nIndex == 3 || nIndex == 5 || nIndex == 14 || nIndex == 16 || 
                    nIndex == 18 || nIndex == 25 || nIndex == 34 || nIndex == 36 || nIndex == 38 ||
                    nIndex == 47 ) {
          uFieldValue = 2; 
        } else if ( nIndex == 8 || nIndex == 10 || nIndex == 12 || nIndex == 21 || nIndex == 23 || 
                    nIndex == 28 || nIndex == 30 || nIndex == 32 || nIndex == 41 || nIndex == 43 ||
                    nIndex == 45 || nIndex == 50 || nIndex == 52 ) {
          uFieldValue = 3; 
        } else if ( nIndex == 6 || nIndex == 19 || nIndex == 33 || nIndex == 46 ) { 
          uFieldValue = 4; 
        } else if ( nIndex == 24 ) { 
          uFieldValue = 5; 
        }
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT0f, &uFieldValue );

        if ( nIndex == 6 || nIndex == 8 || nIndex == 10 || nIndex == 19 || nIndex == 21 ||
             nIndex == 23 || nIndex == 28 || nIndex == 30 || nIndex == 32 || nIndex == 41 ||
             nIndex == 43 || nIndex == 45 || nIndex == 50 ) {
          uFieldValue = 0;
        } else if ( nIndex == 0 || nIndex == 2 || nIndex == 4 || nIndex == 13 || nIndex == 15 || 
                    nIndex == 17 || nIndex == 24 || nIndex == 33 || nIndex == 35 || nIndex == 37 ||
                    nIndex == 46 ) {
          uFieldValue = 1; 
        } else if ( nIndex == 7 || nIndex == 9 || nIndex == 11 || nIndex == 20 || nIndex == 22 || 
                    nIndex == 27 || nIndex == 29 || nIndex == 31 || nIndex == 40 || nIndex == 42 ||
                    nIndex == 44 || nIndex == 49 || nIndex == 51 ) {
          uFieldValue = 2; 
        } else if ( nIndex == 1 || nIndex == 3 || nIndex == 5 || nIndex == 14 || nIndex == 16 || 
                    nIndex == 18 || nIndex == 25 || nIndex == 34 || nIndex == 36 || nIndex == 38 ||
                    nIndex == 47 ) {
          uFieldValue = 3; 
        } else if ( nIndex == 12 || nIndex == 26 || nIndex == 39 || nIndex == 52 ) { 
          uFieldValue = 4; 
        } else if ( nIndex == 48 ) { 
          uFieldValue = 5; 
        }
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT1f, &uFieldValue );

        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_CLIENT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_END_ADDRf, 105 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nIndex = 0; nIndex < 51; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_PORT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, ACTIVEf, &uFieldValue );
        if ( nIndex < 48 ) uFieldValue = nIndex;
        else if ( nIndex == 48 ) uFieldValue = 50;
        else if ( nIndex == 49 ) uFieldValue = 48;
        else if ( nIndex == 50 ) uFieldValue = 49;
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, PORTf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_PORT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_END_ADDRf, 50 );
      soc_reg32_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );


    } else if ( g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {

      for ( nIndex = 0; nIndex < 41; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_CLIENT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE0f, &uFieldValue );
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE1f, &uFieldValue );

        uFieldValue = 0;
        if ( nIndex == 6 || nIndex == 9 || nIndex == 15 ) uFieldValue = 4;
        else if ( nIndex == 19 ) uFieldValue = 5;
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT0f, &uFieldValue );

        uFieldValue = 0;
        if ( nIndex == 12 || nIndex == 25 ) uFieldValue = 4;
        else if ( nIndex == 38 ) uFieldValue = 5;
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT1f, &uFieldValue );

        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_CLIENT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_END_ADDRf, 81 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nIndex = 0, uTxPort = 0; nIndex < 101; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_PORT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, ACTIVEf, &uFieldValue );

        if ( uTxPort == 12 && nIndex < 70 ) {
          if ( nIndex == 12 || nIndex == 51 ) uFieldValue = 48;
          else if ( nIndex == 25 || nIndex == 64 ) uFieldValue = 49;
          else if ( nIndex == 38 ) uFieldValue = 50;
          uTxPort = 0;
        } else {
          uFieldValue = uTxPort;
          ++uTxPort;
          if ( nIndex >= 70 && uTxPort == 12 ) uTxPort = 0;
        }
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, PORTf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_PORT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_END_ADDRf, 100 );
      soc_reg32_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    } else if ( g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {

      for ( nIndex = 0; nIndex < 35; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_CLIENT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE0f, &uFieldValue );
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE1f, &uFieldValue );

        uFieldValue = 0;
        if ( nIndex == 12 || nIndex == 22 || nIndex == 33 ) uFieldValue = 4;
        else if ( nIndex == 0 ) uFieldValue = 5;
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT0f, &uFieldValue );

        uFieldValue = 0;
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT1f, &uFieldValue );

        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_CLIENT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_END_ADDRf, 69 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nIndex = 0, uTxPort = 0; nIndex < 99; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_PORT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, ACTIVEf, &uFieldValue );

        uFieldValue = uTxPort++;
        if ( uTxPort == 3 ) uTxPort = 0;
        if ( nIndex == 3 ) {
          uFieldValue = 48;
          uTxPort = 0;
        } else if ( nIndex == 16 ) {
          uFieldValue = 49;
          uTxPort = 0;
        } else if ( nIndex == 29 ) {
          uFieldValue = 50;
          uTxPort = 0;
        }
        /* Added for B0 */
        if ( g_rev_id == BCM88030_B0_REV_ID ) {
          if ( uFieldValue < 3 ) uFieldValue *= 4;
        }
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, PORTf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_PORT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_END_ADDRf, 98 );
      soc_reg32_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );


    } else {

      for ( nIndex = 0; nIndex < 36; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_CLIENT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
        uFieldValue = ( nIndex == 4 || nIndex == 13 || nIndex == 22 ) ? 0 : 1; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE0f, &uFieldValue );
        uFieldValue = ( nIndex == 31 && nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 5 : 0; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT0f, &uFieldValue );
        uFieldValue = ( nIndex == 8 || nIndex == 17 || nIndex == 26 ) ? 0 : 1; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, ACTIVE1f, &uFieldValue );
        uFieldValue = ( nIndex == 35 && nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 5 : 0; 
        soc_mem_field_set( nUnit, IPTE_CLIENT_CALm, &uClientCalEntry, CLIENT1f, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_CLIENT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uClientCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, &uRegisterValue, CAL_END_ADDRf, 71 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      for ( nIndex = 0; nIndex < 108; ++nIndex ) {
        SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_PORT_CALm, 
                                          PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
        uFieldValue = 1; 
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, ACTIVEf, &uFieldValue );
        uFieldValue = 0; /* Port 0 as default */
        if ( g_bXLportAndCmicPortsActive ) {
          uFieldValue = ( (nIndex == 33 || nIndex == 67 || nIndex == 101) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 50 : 0;
        }
        soc_mem_field_set( nUnit, IPTE_PORT_CALm, &uPortCalEntry, PORTf, &uFieldValue );
        SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_PORT_CALm, 
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uPortCalEntry) );
      }
      soc_reg32_get( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, &uRegisterValue, PT_IPTE_PORT_CAL_END_ADDRf, 107 );
      soc_reg32_set( nUnit, PT_IPTE_DS_PORT_CAL_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_QUAD0_PORT3_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_QUAD0_PORT2_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_QUAD0_PORT1_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_QUAD0_PORT0_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_IDf, 1 );
    } else if ( (g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ||
                 g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_IDf, 1 );
    }
    soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_ENf, 1 );
    soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0r, &uRegisterValue, CLIENT_128B_SCHEDULE_QUANTUMf, 0 );
    soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  
    if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {

      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG0_1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD1_PORT0_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD1_PORT1_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD1_PORT2_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD1_PORT3_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD2_PORT0_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD2_PORT1_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD2_PORT2_START_ENf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, &uRegisterValue, CLIENT_QUAD2_PORT3_START_ENf, 2 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG0_1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG1r, &uRegisterValue, CLIENT_ENf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG2r, &uRegisterValue, CLIENT_ENf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG3r, &uRegisterValue, CLIENT_ENf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG4r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG4r, &uRegisterValue, CLIENT_GE_CLOCK_SPACINGf, 4 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG4r, &uRegisterValue, CLIENT_ENf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG4r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
      soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG5r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG5r, &uRegisterValue, PAD_ENf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG5r, &uRegisterValue, CLIENT_ENf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG5r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
    

    nTxPortIncr = 1;
    if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
          g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {

      if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {
        nTxPortNum = 48;
        uSize = 2;
        uThreshold = 1;
      } else if ( g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ) {
        nTxPortNum = 3; 
        /* Added for B0 */
        if ( g_rev_id == BCM88030_B0_REV_ID ) {
          nTxPortNum *= 4;
          nTxPortIncr *= 4;
        }
        uSize = 26;
        uThreshold = 7;
      } else {
        nTxPortNum = 12;
        uSize = 8;
        uThreshold = 2;
      } 

    } else {

       nTxPortNum = 1;
       uSize = 76;
       uThreshold = ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) ? 8 : 0x14;
    }

    for ( nIndex = 0, uPortFifoBaseAddress = 0; nIndex < nTxPortNum; nIndex += nTxPortIncr ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, SIZEf, uSize );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, PKT_SERVICE_PAGES_THRESHOLDf, uThreshold );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, BASE_ADDRESSf, uPortFifoBaseAddress );
      /* Actual depth is SIZE*8 */
      uPortFifoBaseAddress += (uSize * 8);
      soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, uRegisterValue );
    }

    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      uSize = 2;
      uThreshold = 1;
      for ( nIndex = 48; nIndex < 51; ++nIndex ) {
        soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, &uRegisterValue );
        soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, SIZEf, uSize );
        soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, PKT_SERVICE_PAGES_THRESHOLDf, uThreshold );
        soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uRegisterValue, BASE_ADDRESSf, uPortFifoBaseAddress ); 
        /* Actual depth is SIZE*8 */
        uPortFifoBaseAddress += (uSize * 8);
        soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, uRegisterValue );
      }
    }
    soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_INITr, &uRegisterValue, PT_PORT_FIFO_INITf, 1 );
    soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    if ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
      soc_reg32_get( nUnit, PT_HPTE_SCHEDULER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, PT_HPTE_SCHEDULER_CFGr, &uRegisterValue, PT_HPTE_IG_PORT_ENf, 1 );
      soc_reg_field_set( nUnit, PT_HPTE_SCHEDULER_CFGr, &uRegisterValue, PT_HPTE_EG_PORT_ENf, 1 );
      soc_reg_field_set( nUnit, PT_HPTE_SCHEDULER_CFGr, &uRegisterValue, PT_HPTE_WDRR_QUANTUMf, 3 );
      soc_reg32_set( nUnit, PT_HPTE_SCHEDULER_CFGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }

    /* Added for B0 */
    if ( g_rev_id == BCM88030_B0_REV_ID ) {
      if ( (g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL || g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
            g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL) && nInstance == C3HPPC_SWS_LINE_INTERFACE ) {
        for ( nIndex = 0; nIndex < 12; ++nIndex ) {
          SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_Q2C_MAPm,
                                            PT_BLOCK(nUnit,nInstance), nIndex, &uQ2CmapEntry) );
          uFieldValue = 0;
          soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, FC_INDEXf, &uFieldValue );
          if ( g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ) {
            uFieldValue = ( nIndex < 3 ) ? (4 * nIndex) : 0xf;
          } else {
            uFieldValue = nIndex;
          }
          soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, PORTf, &uFieldValue );
          SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_Q2C_MAPm,
                                             PT_BLOCK(nUnit,nInstance), nIndex, &uQ2CmapEntry) );
        }
      } else {
        for ( nIndex = 0; nIndex < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nIndex ) {
          SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_Q2C_MAPm,
                                            PT_BLOCK(nUnit,nInstance), nIndex, &uQ2CmapEntry) );
          uFieldValue = nIndex;
          soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, FC_INDEXf, &uFieldValue );
          uFieldValue = 0;
          soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, PORTf, &uFieldValue );
          SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_Q2C_MAPm,
                                           PT_BLOCK(nUnit,nInstance), nIndex, &uQ2CmapEntry) );
        }
      }
    }

  }

  return 0;
}

int c3hppc_sws_mac_bringup( int nUnit, c3hppc_sws_mac_control_info_t *pMacControlInfo ) {

  int nPort, nPortIncrement, nPortNum, nPortSpeed  COMPILER_ATTRIBUTE((unused));
  uint64 uReg64;
  uint32 uRegisterValue;
  int rv = SOC_E_NONE;

  nPortNum = nPortIncrement = 0;
  nPortSpeed = 0;


  if ( g_nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {

    nPortNum = 48;
    nPortIncrement = 12;
    nPortSpeed = 1000;
    cli_out("\n\nMAC 48x1G Bring-Up ... \n" );

  } else if ( g_nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ) {

    nPortNum = 12;
    nPortIncrement = 12;
    nPortSpeed = 10000;
    cli_out("\n\nMAC 12x10G Bring-Up ... \n" );

  } else if ( g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ) {

    nPortNum = 3;
    nPortIncrement = 3;
    nPortSpeed = 40000;
    cli_out("\n\nMAC 3x40G Bring-Up ... \n" );

  } else if ( g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) {

    nPortNum = 1;
    nPortIncrement = 1;
    nPortSpeed = 100000;
    cli_out("\n\nMAC 100G Bring-Up ... \n" );

  }

  if ( pMacControlInfo->bExternalLoopback == 0 ) {
    for ( nPort = 0; nPort < nPortNum; ++nPort ) {
#if (defined(LINUX))
      rv = bcm_port_loopback_set(nUnit, nPort, BCM_PORT_LOOPBACK_PHY);
      if (rv != BCM_E_NONE) {                                 
          cli_out("Setting loopback failed: %s\n",      
                  bcm_errmsg(rv));                    
          return rv;                                    
      }

#else
      rv = bcm_port_loopback_set(nUnit, nPort, BCM_PORT_LOOPBACK_MAC);
      if (rv != BCM_E_NONE) {                                 
          cli_out("Setting loopback failed: %s\n",      
                  bcm_errmsg(rv));                    
          return rv;                                    
      }

#endif
    }
  }

  for ( nPort = 56; nPort < 60; ++nPort ) {
#if (defined(LINUX))
    rv = bcm_port_loopback_set(nUnit, nPort, BCM_PORT_LOOPBACK_PHY);
    if (rv != BCM_E_NONE) {                                 
        cli_out("Setting loopback failed: %s\n",      
                bcm_errmsg(rv));                    
        return rv;                                    
    }

    rv = bcm_port_loopback_set(nUnit, nPort, BCM_PORT_LOOPBACK_MAC);
    if (rv != BCM_E_NONE) {                                 
      cli_out("Setting loopback failed: %s\n",      
              bcm_errmsg(rv));                    
      return rv;                                    
    }    
#else
    rv = bcm_port_loopback_set(nUnit, nPort, BCM_PORT_LOOPBACK_MAC);
    if (rv != BCM_E_NONE) {                                 
        cli_out("Setting loopback failed: %s\n",      
                bcm_errmsg(rv));                    
        return rv;                                    
    }

#endif
  }


/* Not necessary .... holding around in case needed in the future
   Note:  Doing "bcm_port_speed_set" had an adverse affect for only port 0 in 3x40G mode.

  for ( nPort = 0; nPort < nPortNum; ++nPort ) {
    if ( g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ) {
      bcm_port_interface_set( nUnit, nPort, BCM_PORT_IF_XLAUI );
    }
    bcm_port_speed_set( nUnit, nPort, nPortSpeed );
  }
*/

  for ( nPort = 0; nPort < nPortNum; nPort += nPortIncrement ) {
    if ( g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) {

      WRITE_PORT_ENABLE_REGr( nUnit, nPort, 0x000 );

      READ_PORT_SOFT_RESETr( nUnit, nPort, &uRegisterValue );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, CPORT_COREf, 1 );
      WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );

      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, CPORT_COREf, 0 );
      WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );

      WRITE_PORT_ENABLE_REGr( nUnit, nPort, 0x1 );

    } else {
      WRITE_PORT_ENABLE_REGr( nUnit, nPort, 0x000 );

      READ_PORT_SOFT_RESETr( nUnit, nPort, &uRegisterValue );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE0f, 1 );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE1f, 1 );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE2f, 1 );
      WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );

      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE0f, 0 );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE1f, 0 );
      soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE2f, 0 );
      WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );

      WRITE_PORT_ENABLE_REGr( nUnit, nPort, (g_nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ? 0x111 : 0xfff) );
    }
  }

  nPort = 56;
  WRITE_PORT_ENABLE_REGr( nUnit, nPort, 0x000 );
  READ_PORT_SOFT_RESETr( nUnit, nPort, &uRegisterValue );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE0f, 1 );
  WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uRegisterValue, XPORT_CORE0f, 0 );
  WRITE_PORT_SOFT_RESETr( nUnit, nPort, uRegisterValue );
  WRITE_PORT_ENABLE_REGr( nUnit, nPort, 0xf );


/*  LLFC config

  for ( nPort = 0; nPort < nPortNum; ++nPort ) {
    SOC_IF_ERROR_RETURN( READ_PORT_CONFIGr( nUnit, nPort, &uRegisterValue ) );
    soc_reg_field_set( nUnit, PORT_CONFIGr, &uRegisterValue, LLFC_ENf, 1 );
    SOC_IF_ERROR_RETURN( WRITE_PORT_CONFIGr( nUnit, nPort, uRegisterValue ) );

    SOC_IF_ERROR_RETURN( READ_XMAC_LLFC_CTRLr( nUnit, nPort, &uReg64 ) );
    soc_reg64_field32_set( nUnit, XMAC_LLFC_CTRLr, &uReg64, RX_LLFC_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_LLFC_CTRLr, &uReg64, TX_LLFC_ENf, 1 );
    SOC_IF_ERROR_RETURN( WRITE_XMAC_LLFC_CTRLr( nUnit, nPort, uReg64 ) );

    SOC_IF_ERROR_RETURN( READ_XMAC_PAUSE_CTRLr( nUnit, nPort, &uReg64 ) );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 0 );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 0 );
    SOC_IF_ERROR_RETURN( WRITE_XMAC_PAUSE_CTRLr( nUnit, nPort, uReg64 ) );
  }
  soc_reg32_get( nUnit, PT_IPTE_FC_SAFC_CFG0r, SOC_BLOCK_PORT(nUnit,0), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, PT_IPTE_FC_SAFC_CFG0r, &uRegisterValue, ENABLEf, 0xfff );
  soc_reg32_set( nUnit, PT_IPTE_FC_SAFC_CFG0r, SOC_BLOCK_PORT(nUnit,0), 0, uRegisterValue );

*/



  for ( nPort = 0; nPort < nPortNum; ++nPort ) {
    if ( g_nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) {

      SOC_IF_ERROR_RETURN( READ_CMAC_TX_CTRLr( nUnit, nPort, &uReg64 ) );
      soc_reg64_field32_set( nUnit, CMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 );  /* Replace */
      SOC_IF_ERROR_RETURN( WRITE_CMAC_TX_CTRLr( nUnit, nPort, uReg64 ) );

      SOC_IF_ERROR_RETURN( READ_CMAC_RX_CTRLr( nUnit, nPort, &uReg64 ) );
      if ( g_rev_id != BCM88030_B0_REV_ID ) {
        soc_reg64_field32_set( nUnit, CMAC_RX_CTRLr, &uReg64, RUNT_THRESHOLDf, 48 );
      }
      soc_reg64_field32_set( nUnit, CMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 );
      SOC_IF_ERROR_RETURN( WRITE_CMAC_RX_CTRLr( nUnit, nPort, uReg64 ) );

      if ( pMacControlInfo->bPauseDisable == 1 ) {
        SOC_IF_ERROR_RETURN( READ_CMAC_PAUSE_CTRLr( nUnit, nPort, &uReg64 ) );
        soc_reg64_field32_set( nUnit, CMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 0 );
        soc_reg64_field32_set( nUnit, CMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 0 );
        SOC_IF_ERROR_RETURN( WRITE_CMAC_PAUSE_CTRLr( nUnit, nPort, uReg64 ) );
      }

    } else {

      SOC_IF_ERROR_RETURN( READ_XMAC_TX_CTRLr( nUnit, nPort, &uReg64 ) );
      soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 );  /* Replace */
      SOC_IF_ERROR_RETURN( WRITE_XMAC_TX_CTRLr( nUnit, nPort, uReg64 ) );

      SOC_IF_ERROR_RETURN( READ_XMAC_RX_CTRLr( nUnit, nPort, &uReg64 ) );
      if ( g_rev_id != BCM88030_B0_REV_ID ) {
        soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, RUNT_THRESHOLDf, 48 );
      }
      soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 );
      SOC_IF_ERROR_RETURN( WRITE_XMAC_RX_CTRLr( nUnit, nPort, uReg64 ) );

      if ( pMacControlInfo->bPauseDisable == 1 ) {
        SOC_IF_ERROR_RETURN( READ_XMAC_PAUSE_CTRLr( nUnit, nPort, &uReg64 ) );
        soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 0 );
        soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 0 );
        SOC_IF_ERROR_RETURN( WRITE_XMAC_PAUSE_CTRLr( nUnit, nPort, uReg64 ) );
      }

    }
  }

  for ( nPort = 56; nPort < 60; ++nPort ) {
    SOC_IF_ERROR_RETURN( READ_XMAC_TX_CTRLr( nUnit, nPort, &uReg64 ) );
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 );  /* Replace */
    SOC_IF_ERROR_RETURN( WRITE_XMAC_TX_CTRLr( nUnit, nPort, uReg64 ) );

    SOC_IF_ERROR_RETURN( READ_XMAC_RX_CTRLr( nUnit, nPort, &uReg64 ) );
    if ( g_rev_id != BCM88030_B0_REV_ID ) {
      soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, RUNT_THRESHOLDf, 48 );
    }
    soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 );
    SOC_IF_ERROR_RETURN( WRITE_XMAC_RX_CTRLr( nUnit, nPort, uReg64 ) );

    if ( pMacControlInfo->bPauseDisable == 1 ) {
      SOC_IF_ERROR_RETURN( READ_XMAC_PAUSE_CTRLr( nUnit, nPort, &uReg64 ) );
      soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 0 );
      soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 0 );
      SOC_IF_ERROR_RETURN( WRITE_XMAC_PAUSE_CTRLr( nUnit, nPort, uReg64 ) );
    }

    /* Added for B0 */
    if ( g_rev_id == BCM88030_B0_REV_ID ) {
      SOC_IF_ERROR_RETURN( READ_XMAC_MODEr( nUnit, nPort, &uReg64 ) );
      soc_reg64_field32_set( nUnit, XMAC_MODEr, &uReg64, SPEED_MODEf, 3 );  /* LINK_2G5 */
      SOC_IF_ERROR_RETURN( WRITE_XMAC_MODEr( nUnit, nPort, uReg64 ) );
    }
  }

  /* The purpose of this reset is re-sync after establishing good clocks. */
  for ( nPort = 0; nPort < nPortNum; ++nPort ) {
    SOC_IF_ERROR_RETURN( READ_XMAC_CTRLr( nUnit, nPort, &uReg64 ) );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 1 );
    SOC_IF_ERROR_RETURN( WRITE_XMAC_CTRLr( nUnit, nPort, uReg64 ) );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
    SOC_IF_ERROR_RETURN( WRITE_XMAC_CTRLr( nUnit, nPort, uReg64 ) );
  }


  return 0;
}

int c3hppc_sws_il_bringup( int nUnit, int nInstance, c3hppc_sws_il_control_info_t *pIlControlInfo ) {

  uint32 uRegisterValue;
  int nField, nWC;
  sal_time_t TimeStamp;
  int nWCport;
  int rv = SOC_E_NONE;

  soc_reg32_get( nUnit, IL_RX_CORE_STATUS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  nField = soc_reg_field_get( nUnit, IL_RX_CORE_STATUS1r, uRegisterValue, STAT_RX_ALIGNEDf );
  if ( !SAL_BOOT_QUICKTURN && nField ) {
    cli_out("\n\nIL%d already up ... \n", nInstance );

    /****************************************************************************************************************************
     * Launch the stats memory initialization daemon ..
     *****************************************************************************************************************************/
    uRegisterValue = 0x1f;
    soc_reg32_set( nUnit, IL_MEMORY_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    /*
      Reset the override state so that back to back runs with and without the traffic generators 
      won't halt the traffic.  The following registers are set in "c3hppc_sws_il_pktgen_setup" for
      the following reason: 
        Do not allow SWS back-pressure destined for the packet generator to reach or come back into the SWS
        as the IL OOB interface is either in loop-back or cross-connected depending upon the TDM.
    */
    uRegisterValue = 0x00000000;
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    uRegisterValue = 0;
    soc_reg_field_set( nUnit, IL_FLOWCONTROL_CONFIGr, &uRegisterValue, IL_FLOWCONTROL_RXFC_OVERRIDE_ENABLEf, 0 );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

    return 0;

  } else {
    cli_out("\n\nIL%d Bring-Up ... \n", nInstance );

    nWCport = ( nInstance == C3HPPC_SWS_LINE_INTERFACE ) ?  0 : 60;

    /* Power down lanes ... */
    soc_phyctrl_enable_set(nUnit, nWCport, 0);

    /* coverity[check_return] */
    bcm_port_speed_set( nUnit, nWCport, ((g_dev_id != BCM88034_DEVICE_ID) ? 10937 : 10312) );

    if ( pIlControlInfo->bExternalLoopback == 0 && pIlControlInfo->bL1Loopback == 0 ) {
      rv = bcm_port_loopback_set(nUnit, nWCport, BCM_PORT_LOOPBACK_PHY);
      if (rv != BCM_E_NONE) {                                 
          cli_out("Setting loopback failed: %s\n",      
                  bcm_errmsg(rv));                    
          return rv;                                    
      }
    }

  }


  /****************************************************************************************************************************
   * Block hard reset ...
   *****************************************************************************************************************************/
  nField = ( nInstance == C3HPPC_SWS_FABRIC_INTERFACE ) ? IL1_RESET_Nf : IL0_RESET_Nf;
  READ_CX_SOFT_RESET_0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, nField, 0 );
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, nField, 1 );
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );

  /****************************************************************************************************************************
   * Place in soft reset ...
   *****************************************************************************************************************************/
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_GLOBAL_CONTROLr, &uRegisterValue, SOFT_RESETf, 1 );
  soc_reg32_set( nUnit, IL_GLOBAL_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  soc_reg32_get( nUnit, IL_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, IL_GLOBAL_CONFIGr, &uRegisterValue, XGS_COUNTER_COMPAT_MODEf, g_uIlCounterCompatMode );
  soc_reg_field_set( nUnit, IL_GLOBAL_CONFIGr, &uRegisterValue, CFG_RX_WC_BITFLIPf, 1 );
  soc_reg_field_set( nUnit, IL_GLOBAL_CONFIGr, &uRegisterValue, CFG_TX_WC_BITFLIPf, 1 );
  soc_reg32_set( nUnit, IL_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  /****************************************************************************************************************************
   * Config the IL serdes lane map registers.  This is just a placeholder for now.  Use the default reset config for this.
   *****************************************************************************************************************************/

  /****************************************************************************************************************************
   * In emulation enable the CDR lock ignore logic
   *****************************************************************************************************************************/
  if ( SAL_BOOT_QUICKTURN ) {
    uRegisterValue = 0;
    soc_reg_field_set( nUnit, IL_RX_CDR_LOCK_CONTROLr, &uRegisterValue, CDR_LOCK_MASKf, 0xfff );
    soc_reg32_set( nUnit, IL_RX_CDR_LOCK_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }

  /****************************************************************************************************************************
   * In L1 loopback enable the CDR lock ignore logic
   *****************************************************************************************************************************/
  if ( pIlControlInfo->bL1Loopback ) {
    soc_reg32_get( nUnit, IL_LOOPBACK_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, IL_LOOPBACK_CONFIGr, &uRegisterValue, IL_LOOPBACK_L1_ENABLEf, 1 );
    soc_reg32_set( nUnit, IL_LOOPBACK_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }

  if ( pIlControlInfo->bFCLoopback ) {
    soc_reg32_get( nUnit, IL_LOOPBACK_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, IL_LOOPBACK_CONFIGr, &uRegisterValue, IL_LOOPBACK_FC_ENABLEf, 1 );
    soc_reg32_set( nUnit, IL_LOOPBACK_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }


  /****************************************************************************************************************************
   * General block config ...
   *****************************************************************************************************************************/
  uRegisterValue = 0;
  soc_reg32_set( nUnit, IL_IEEE_CRC32_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0xffffffff;
  soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  /****************************************************************************************************************************
   * Allow 16K frames 
   *****************************************************************************************************************************/
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_RX_CONFIGr, &uRegisterValue, IL_RX_MAX_PACKET_SIZEf, 16383 );
  soc_reg32_set( nUnit, IL_RX_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_get( nUnit, IL_TX_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, IL_TX_CONFIGr, &uRegisterValue, IL_TX_MAX_PACKET_SIZEf, 16383 );
  soc_reg32_set( nUnit, IL_TX_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  /****************************************************************************************************************************
   * Enabling the enhanced scheduler and setting "BurstShort" to 64 (for Arad inter-op) otherwise set it to 32
   *****************************************************************************************************************************/
  soc_reg32_get( nUnit, IL_TX_CORE_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, IL_TX_CORE_CONFIG0r, &uRegisterValue, CTL_TX_BURSTSHORTf, 1 );
  soc_reg32_set( nUnit, IL_TX_CORE_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_get( nUnit, IL_TX_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, IL_TX_CONFIGr, &uRegisterValue, IL_TX_ENHANCED_SCHEDULING_ENf, 1 );
  soc_reg32_set( nUnit, IL_TX_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  if ( g_dev_id == BCM88034_DEVICE_ID ) {
    soc_reg32_get( nUnit, IL_TX_CORE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, IL_TX_CORE_CONFIG2r, &uRegisterValue, CTL_TX_LAST_LANEf, 5 );
    soc_reg32_set( nUnit, IL_TX_CORE_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_get( nUnit, IL_RX_CORE_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, IL_RX_CORE_CONFIG1r, &uRegisterValue, CTL_RX_LAST_LANEf, 5 );
    soc_reg32_set( nUnit, IL_RX_CORE_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }
  
  /****************************************************************************************************************************
   * Enable the instance ...
   *****************************************************************************************************************************/
  soc_reg32_get( nUnit, IL_GLOBAL_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  soc_reg_field_set( nUnit, IL_GLOBAL_CONTROLr, &uRegisterValue, SOFT_RESETf, 0 );
  soc_reg32_set( nUnit, IL_GLOBAL_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  /****************************************************************************************************************************
   * Launch the stats memory initialization daemon ..
   *****************************************************************************************************************************/
  uRegisterValue = 0x1f;
  soc_reg32_set( nUnit, IL_MEMORY_INITr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );


  /* This is here to allow "soc_miimc45_write" access to the WCs */
  /* Power up lanes ... */
  soc_phyctrl_enable_set(nUnit, nWCport, 1);

  /* Assert tx/rx asic reset across all lanes. */  
  for ( nWC = 0; nWC < 3; ++nWC ) {
/*
    soc_miimc45_write( nUnit, g_anWarpCore_PhyIDs[nInstance][nWC], 0x1, 0x810a, 0x00ff );
*/
  }
  /* De-assert tx asic reset across all lanes. */  
  for ( nWC = 0; nWC < 3; ++nWC ) {
/*
    soc_miimc45_write( nUnit, g_anWarpCore_PhyIDs[nInstance][nWC], 0x1, 0x810a, 0x000f );
*/
  }
  /* De-assert rx asic reset across all lanes. */  
  for ( nWC = 0; nWC < 3; ++nWC ) {
/*
    soc_miimc45_write( nUnit, g_anWarpCore_PhyIDs[nInstance][nWC], 0x1, 0x810a, 0x0000 );
*/
  }
  /* Do "tx1g" fifo reset via "tx_asic_reset" procedure via MDIO.  This is required due to CRC24 errors. */
  soc_phyctrl_notify( nUnit, nWCport, phyEventTxFifoReset, 0 );
  sal_usleep( 1000000 );

  /****************************************************************************************************************************
   * Force the block to re-synchronize with current configuration settings
   *****************************************************************************************************************************/
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_RX_CORE_CONTROL0r, &uRegisterValue, CTL_RX_FORCE_RESYNC_PULSEf, 1 );
  soc_reg32_set( nUnit, IL_RX_CORE_CONTROL0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg_field_set( nUnit, IL_RX_CORE_CONTROL0r, &uRegisterValue, CTL_RX_FORCE_RESYNC_PULSEf, 0 );
  soc_reg32_set( nUnit, IL_RX_CORE_CONTROL0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  sal_usleep( 1000000 );

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), 
                               IL_RX_CORE_STATUS1r, STAT_RX_ALIGNEDf, 1, 1, 0, &TimeStamp ) ) {
    cli_out("<c3hppc_sws_il_bringup> -- IL%d IL_RX_CORE_STATUS1 \"STAT_RX_ALIGNED\" event TIMEOUT!!!\n", nInstance);
    soc_reg32_get( nUnit, IL_RX_CORE_STATUS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    cli_out("<c3hppc_sws_il_bringup> -- IL%d IL_RX_CORE_STATUS1 --> 0x%08x\n", nInstance, uRegisterValue );
    return -1;
  } else {
    cli_out("<c3hppc_sws_il_bringup> -- IL%d achieved alignment!\n", nInstance);
  }

  /* Do "tx1g" fifo reset via "tx_asic_reset" procedure via MDIO.  This is required due to CRC24 errors. */
/*
  soc_phyctrl_notify( nUnit, nWCport, phyEventTxFifoReset, 0 );
*/


  return 0;
}


int c3hppc_sws_il_stats_open( int nUnit ) {

  int nSize;

  nSize = C3HPPC_SWS_IL_INSTANCE_NUM * sizeof(c3hppc_sws_il_stats_t);
  g_pIlRxStats = (c3hppc_sws_il_stats_t *) sal_alloc( nSize, "il_rx_stats" );
  sal_memset( g_pIlRxStats, 0x00, nSize );

  nSize = C3HPPC_SWS_IL_INSTANCE_NUM * sizeof(c3hppc_sws_il_stats_t);
  g_pIlTxStats = (c3hppc_sws_il_stats_t *) sal_alloc( nSize, "il_tx_stats" );
  sal_memset( g_pIlTxStats, 0x00, nSize );

  nSize = (soc_mem_index_max( nUnit, IL_STAT_MEM_4m ) + 1) * soc_mem_entry_words( nUnit, IL_STAT_MEM_4m ) * sizeof(uint32);
  g_pIlDmaBuffer = (uint32 *) soc_cm_salloc( nUnit, nSize, "il_stats_dma_buffer" );
  
  return 0;
}


int c3hppc_sws_il_stats_close( int nUnit ) {
  
  if ( g_pIlRxStats != NULL ) sal_free( g_pIlRxStats );
  if ( g_pIlTxStats != NULL ) sal_free( g_pIlTxStats );
  if ( g_pIlDmaBuffer != NULL ) soc_cm_sfree( nUnit, g_pIlDmaBuffer );

  return 0;
}


void   c3hppc_sws_il_stats_setrx( int nInstance, int nChannel, int nType, uint64 uuValue ) {

  g_pIlRxStats[nInstance].Array[nChannel][nType] = uuValue;

  return;
}


uint64 c3hppc_sws_il_stats_getrx( int nInstance, int nChannel, int nType ) {

  return g_pIlRxStats[nInstance].Array[nChannel][nType];

}


uint64 c3hppc_sws_il_stats_gettx( int nInstance, int nChannel, int nType ) {

  return g_pIlTxStats[nInstance].Array[nChannel][nType];

}

char *c3hppc_sws_il_stats_rxtype2string( int nType ) {
  static c3hppc_sws_il_stat_name_t acIlRxStatNames[] =
                  { {"RUND"}, {"R64"}, {"R127"}, {"R255"}, {"R511"}, {"R1023"}, {"R2047"}, {"R4095"},
                    {"R9216"}, {"R15999"}, {"RERR"}, {"RPKT"}, {"RBYT"}, {"ROVR"}, {"RMTU"}, {"RFCS"} };

  return acIlRxStatNames[nType].sName;
}

char *c3hppc_sws_il_stats_txtype2string( int nType ) {
  static c3hppc_sws_il_stat_name_t acIlTxStatNames[] =
                  { {"TUND"}, {"T64"}, {"T127"}, {"T255"}, {"T511"}, {"T1023"}, {"T1518"}, {"T2047"},
                    {"T4095"}, {"T9216"}, {"T16383"}, {"TOVR"}, {"TMTU"}, {"TERR"}, {"TPKT"}, {"TBYT"} };

  return acIlTxStatNames[nType].sName;
}


void c3hppc_sws_il_stats_display_nonzero( int nInstance ) {
  int nChannel, nType;
  uint64 uuStat;

  cli_out("\n\nDisplaying non-zero IL%d stats ... \n\n", nInstance );

  for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
    for ( nType = 0; nType < C3HPPC_SWS_IL_NUM_OF_STAT_TYPES; ++nType ) {
      uuStat = c3hppc_sws_il_stats_getrx( nInstance, nChannel, nType ); 
      if ( !COMPILER_64_IS_ZERO(uuStat) ) {
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)
        cli_out("    Channel[%d] %s --> 0x%x%08x\n", nChannel, c3hppc_sws_il_stats_rxtype2string(nType),
                COMPILER_64_HI(uuStat), COMPILER_64_LO(uuStat) );
#else
        cli_out("    Channel[%d] %s --> %lld\n", nChannel, c3hppc_sws_il_stats_rxtype2string(nType), uuStat );
#endif
      } 
    }
  }

  for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
    for ( nType = 0; nType < C3HPPC_SWS_IL_NUM_OF_STAT_TYPES; ++nType ) {
      uuStat = c3hppc_sws_il_stats_gettx( nInstance, nChannel, nType ); 
      if ( !COMPILER_64_IS_ZERO(uuStat) ) {
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)
        cli_out("    Channel[%d] %s --> 0x%x%08x\n", nChannel, c3hppc_sws_il_stats_txtype2string(nType), 
                COMPILER_64_HI(uuStat), COMPILER_64_LO(uuStat) );
#else
        cli_out("    Channel[%d] %s --> %lld\n", nChannel, c3hppc_sws_il_stats_txtype2string(nType), uuStat );
#endif
      } 
    }
  }

  return;
}


int c3hppc_sws_il_stats_collect( int nUnit ) {

  int nInstance, nStatArray, nEndRow, nRow, nField, nChannel, nChannelIndex, nStat, nStatTypeIndex;
  int nMemEntryWords, nDmaBufferIndex;
  uint32 auCount[2];
  uint64 uuCount;
  static int anIlStatArrayNames[5] =   { IL_STAT_MEM_0m, IL_STAT_MEM_1m, IL_STAT_MEM_2m, IL_STAT_MEM_3m, IL_STAT_MEM_4m };
  static int anIlFieldNumPerRow[5] =   { 3,              3,              1,              5,              1              };
  static int anIlRowNumPerChannel[5] = { 1,              1,              10,             1,              11             };
  static int anIlStatArrayFields[5][5] =
        { {RX_STAT_BYTE_COUNTf,            RX_STAT_PKT_COUNTf,       RX_STAT_BAD_PKT_ILERR_COUNTf, 0,                        0                       },
          {RX_STAT_IEEE_CRCERR_PKT_COUNTf, RX_STAT_EQMTU_PKT_COUNTf, RX_STAT_GTMTU_PKT_COUNTf,     0,                        0                       },
          {RX_STAT_PKT_COUNTf,             0,                        0,                            0,                        0                       },
          {TX_STAT_BYTE_COUNTf,            TX_STAT_PKT_COUNTf,       TX_STAT_BAD_PKT_PERR_COUNTf,  TX_STAT_EQMTU_PKT_COUNTf, TX_STAT_GTMTU_PKT_COUNTf},
          {TX_STAT_PKT_COUNTf,             0,                        0,                            0,                        0                       }
        };
  static int anIlStatMap[5][5] =
        { {ILRXSTAT__BYTE,                 ILRXSTAT__PKT,            ILRXSTAT__BAD_PKT_ILERR,      0,                        0                       },
          {ILRXSTAT__IEEE_CRCERR,          ILRXSTAT__EQMTU,          ILRXSTAT__GTMTU,              0,                        0                       },
          {0,                              0,                        0,                            0,                        0                       },
          {ILTXSTAT__BYTE,                 ILTXSTAT__PKT,            ILTXSTAT__BAD_PKT_PERR,       ILTXSTAT__EQMTU,          ILTXSTAT__GTMTU         },
          {0,                              0,                        0,                            0,                        0                       }
        };


  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {

    if ( g_bIlInstanceEnabled[nInstance] ) {

/*
      if ( g_bIlInstanceEnabled[0] ) {
        cli_out("\n" );
      }
      cli_out("<c3hppc_sws_il_stats_collect>  Collecting IL%d stats  ... \n", nInstance );
*/

      for ( nStatArray = 0; nStatArray < COUNTOF(anIlStatArrayNames); ++nStatArray ) {
        nEndRow = soc_mem_index_max( nUnit, anIlStatArrayNames[nStatArray] );
        if ( nEndRow == 127 ) nEndRow = 63;
        SOC_IF_ERROR_RETURN( soc_mem_read_range( nUnit, anIlStatArrayNames[nStatArray], IL_BLOCK(nUnit,nInstance), 0,
                                                 nEndRow, (void *) g_pIlDmaBuffer) );
        nMemEntryWords = soc_mem_entry_words( nUnit, anIlStatArrayNames[nStatArray] );
        for ( nRow = 0, nChannel = 0, nStat = 0, nDmaBufferIndex = 0; nRow <= nEndRow; ++nRow, nDmaBufferIndex += nMemEntryWords ) {
          for ( nField = 0; nField < anIlFieldNumPerRow[nStatArray]; ++nField ) {
            soc_mem_field_get( nUnit, anIlStatArrayNames[nStatArray], (g_pIlDmaBuffer+nDmaBufferIndex),
                               anIlStatArrayFields[nStatArray][nField], auCount );
            COMPILER_64_SET(uuCount,auCount[1],auCount[0]);
            nChannelIndex = nChannel;
            if ( anIlRowNumPerChannel[nStatArray] == 1 ) {
              nStatTypeIndex = anIlStatMap[nStatArray][nField];
              if ( (nField+1) == anIlFieldNumPerRow[nStatArray] ) {
                ++nChannel;
              } 
            } else {
              nStatTypeIndex = nStat++;
              if ( nStat == anIlRowNumPerChannel[nStatArray] ) {
                nStat = 0;
                ++nChannel;
              } 
            } 
            if ( g_uIlCounterCompatMode ) {
              if ( nStatArray < 3 ) {
                g_pIlRxStats[nInstance].Array[nChannelIndex][nStatTypeIndex] = uuCount;
              } else {
                g_pIlTxStats[nInstance].Array[nChannelIndex][nStatTypeIndex] = uuCount;
              }
            } else {
              if ( nStatArray < 3 ) {
                COMPILER_64_ADD_64(g_pIlRxStats[nInstance].Array[nChannelIndex][nStatTypeIndex],uuCount);
              } else {
                COMPILER_64_ADD_64(g_pIlTxStats[nInstance].Array[nChannelIndex][nStatTypeIndex],uuCount);
              }
            }
          }
        }
      }

    }

  }
  
  return 0;
}

int c3hppc_sws_il_pktgen_start( int nUnit, int nInstance ) {

  uint32 uRegisterValue;
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_STATUSr, &uRegisterValue, IL_PKTINJ_DONEf, 1 );
  soc_reg32_set( nUnit, IL_PKTINJ_STATUSr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONTROLr, &uRegisterValue, IL_PKTINJ_GOf, 0 );
  soc_reg32_set( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONTROLr, &uRegisterValue, IL_PKTINJ_GOf, 1 );
  soc_reg32_set( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  cli_out("\n\n<c3hppc_sws_il_pktgen_start>  Starting traffic on IL%d  ... \n", nInstance );
  
  return 0;
}

int c3hppc_sws_il_pktgen_stop( int nUnit, int nInstance ) {

  uint32 uRegisterValue;
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONTROLr, &uRegisterValue, IL_PKTINJ_GOf, 0 );
  soc_reg32_set( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  return 0;
}

int c3hppc_sws_il_pktgen_go_control( int nUnit, int nInstance, int nGo ) {

  uint32 uRegisterValue;
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONTROLr, &uRegisterValue, IL_PKTINJ_GOf, nGo );
  soc_reg32_set( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  return 0;
}

int c3hppc_sws_il_pktgen_wait_for_done( int nUnit, int nInstance, int nTimeoutInIterations ) {
  sal_time_t TimeStamp;
  uint32 uRegisterValue;
  uint32 uGo;

  soc_reg32_get( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  uGo = soc_reg_field_get( nUnit, IL_PKTINJ_CONTROLr, uRegisterValue, IL_PKTINJ_GOf );

  if ( uGo == 0 ) {
    return 1;
  } else {
/*
    cli_out("\n\n<c3hppc_sws_il_pktgen_wait_for_done> -- Waiting for IL%d \"IL_PKTINJ_DONE\" event ... \n", nInstance);
*/
    if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), 
                                 IL_PKTINJ_STATUSr, IL_PKTINJ_DONEf, 1, nTimeoutInIterations, 1, &TimeStamp ) ) {
      cli_out("<c3hppc_sws_il_pktgen_wait_for_done> -- IL%d IL_PKTINJ_STATUS \"IL_PKTINJ_DONE\" event TIMEOUT!!!\n", nInstance);
      return -1;
    }

    return 1;
  }
}

int c3hppc_sws_il_pktgen_is_done( int nUnit, int nInstance ) {
  uint32 uRegisterValue;
  uint32 uGo;

  soc_reg32_get( nUnit, IL_PKTINJ_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
  uGo = soc_reg_field_get( nUnit, IL_PKTINJ_CONTROLr, uRegisterValue, IL_PKTINJ_GOf );

  if ( uGo == 0 ) {
    return 1;
  } else {
/*
    cli_out("\n\n<c3hppc_sws_il_pktgen_is_done> -- Checking for IL%d \"IL_PKTINJ_DONE\" indication ... \n", nInstance);
*/
    soc_reg32_get( nUnit, IL_PKTINJ_STATUSr, SOC_BLOCK_PORT(nUnit,nInstance), 0, &uRegisterValue );
    return ( (int) soc_reg_field_get( nUnit, IL_PKTINJ_STATUSr, uRegisterValue, IL_PKTINJ_DONEf ) );
  }
}


int c3hppc_sws_il_pktgen_setup( int nUnit, int nInstance, int nInjectDirection, int nPacketCount,
                                uint64 uuTransmitDuration, int nPayloadPattern, int nIPG,
                                uint32 uMinChannel, uint32 uMaxChannel, int nChannelMode,
                                int nMinPacketLen, int nMaxPacketLen, int nPacketLenMode, 
                                uint32 *pPacketHeader, int nBuiltInHeaderSelect,
                                int nPacketHeaderLenInBytes ) {

#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uRegisterValue, uPayloadOffset, uRandomRange, uTransmitDurationLSBs, uTransmitDurationMSBs;
  int nWord, nByteCount, nDiff;
  static int anHeaderRegisterNames[64] = { IL_PKTINJ_PKTHDR_0r,  IL_PKTINJ_PKTHDR_1r,  IL_PKTINJ_PKTHDR_2r,  IL_PKTINJ_PKTHDR_3r,
                                           IL_PKTINJ_PKTHDR_4r,  IL_PKTINJ_PKTHDR_5r,  IL_PKTINJ_PKTHDR_6r,  IL_PKTINJ_PKTHDR_7r,
                                           IL_PKTINJ_PKTHDR_8r,  IL_PKTINJ_PKTHDR_9r,  IL_PKTINJ_PKTHDR_10r, IL_PKTINJ_PKTHDR_11r,
                                           IL_PKTINJ_PKTHDR_12r, IL_PKTINJ_PKTHDR_13r, IL_PKTINJ_PKTHDR_14r, IL_PKTINJ_PKTHDR_15r,
                                           IL_PKTINJ_PKTHDR_16r, IL_PKTINJ_PKTHDR_17r, IL_PKTINJ_PKTHDR_18r, IL_PKTINJ_PKTHDR_19r,
                                           IL_PKTINJ_PKTHDR_20r, IL_PKTINJ_PKTHDR_21r, IL_PKTINJ_PKTHDR_22r, IL_PKTINJ_PKTHDR_23r,
                                           IL_PKTINJ_PKTHDR_24r, IL_PKTINJ_PKTHDR_25r, IL_PKTINJ_PKTHDR_26r, IL_PKTINJ_PKTHDR_27r,
                                           IL_PKTINJ_PKTHDR_28r, IL_PKTINJ_PKTHDR_29r, IL_PKTINJ_PKTHDR_30r, IL_PKTINJ_PKTHDR_31r,
                                           IL_PKTINJ_PKTHDR_32r, IL_PKTINJ_PKTHDR_33r, IL_PKTINJ_PKTHDR_34r, IL_PKTINJ_PKTHDR_35r,
                                           IL_PKTINJ_PKTHDR_36r, IL_PKTINJ_PKTHDR_37r, IL_PKTINJ_PKTHDR_38r, IL_PKTINJ_PKTHDR_39r,
                                           IL_PKTINJ_PKTHDR_40r, IL_PKTINJ_PKTHDR_41r, IL_PKTINJ_PKTHDR_42r, IL_PKTINJ_PKTHDR_43r,
                                           IL_PKTINJ_PKTHDR_44r, IL_PKTINJ_PKTHDR_45r, IL_PKTINJ_PKTHDR_46r, IL_PKTINJ_PKTHDR_47r,
                                           IL_PKTINJ_PKTHDR_48r, IL_PKTINJ_PKTHDR_49r, IL_PKTINJ_PKTHDR_50r, IL_PKTINJ_PKTHDR_51r,
                                           IL_PKTINJ_PKTHDR_52r, IL_PKTINJ_PKTHDR_53r, IL_PKTINJ_PKTHDR_54r, IL_PKTINJ_PKTHDR_55r,
                                           IL_PKTINJ_PKTHDR_56r, IL_PKTINJ_PKTHDR_57r, IL_PKTINJ_PKTHDR_58r, IL_PKTINJ_PKTHDR_59r,
                                           IL_PKTINJ_PKTHDR_60r, IL_PKTINJ_PKTHDR_61r, IL_PKTINJ_PKTHDR_62r, IL_PKTINJ_PKTHDR_63r };

  uint32 uSwsPllNdivInteger, uSwsClkPeriodNumerator, uSwsClkPeriodDenominator;


  c3hppc_sws_il_pktgen_builtin_packet_header_init();

  if ( pPacketHeader == NULL && nBuiltInHeaderSelect < ILPKTGEN__BUILTIN_HEADERS_NUM ) {
    pPacketHeader = g_auBuiltInPacketHeaders[nBuiltInHeaderSelect];
    nPacketHeaderLenInBytes = g_anBuiltInPacketHeadersLength[nBuiltInHeaderSelect]; 
  } else {
    assert( pPacketHeader != NULL );
    return -1;
  }


  for ( nWord = 0, nByteCount = nPacketHeaderLenInBytes; nWord < COUNTOF(anHeaderRegisterNames); ++nWord, nByteCount -= 4 ) {
    uRegisterValue = ( nByteCount > 0 ) ? pPacketHeader[nWord] : 0;
    soc_reg32_set( nUnit, anHeaderRegisterNames[nWord], SOC_BLOCK_PORT(nUnit,nInstance), 0, pPacketHeader[nWord] );
  } 
  /* 
     Specifies the start of the HW generated payload.  The offset works at 16B boundaries (4LSB is ignored by the design).
     A 256B offset is specifed by writing all 0s to this field.
  */
  uPayloadOffset = (nPacketHeaderLenInBytes  + 15) & 0xf0;

  uRandomRange = 0;
  if ( nPacketLenMode == ILPKTGEN__RANDOM ) {
    nDiff = nMaxPacketLen - nMinPacketLen;
    if ( nDiff < 64 ) {
      uRandomRange = 0;
    } else if ( nDiff < 128 ) {
      uRandomRange = 1;
    } else if ( nDiff < 256 ) {
      uRandomRange = 2;
    } else if ( nDiff < 512 ) {
      uRandomRange = 3;
    } else if ( nDiff < 1024 ) {
      uRandomRange = 4;
    } else if ( nDiff < 2048 ) {
      uRandomRange = 5;
    } else if ( nDiff < 4096 ) {
      uRandomRange = 6;
    } else {
      uRandomRange = 7;
    }
  
    /* In random mode, "max_packet_len" holds the seed of the LFSR15 (only lower 15 bits used)" */
    nMaxPacketLen = sal_rand() & 0xffff; 
  }

  uTransmitDurationLSBs = uTransmitDurationMSBs = 0;
  uSwsPllNdivInteger = uSwsClkPeriodNumerator = uSwsClkPeriodDenominator = 0;
  if ( nPacketCount == 0 && !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {

    READ_CX_SWS_PLL_NDIV_INTEGERr( nUnit, &uSwsPllNdivInteger );
    switch ( uSwsPllNdivInteger ) {
      case 154:
        /* 641.67 MHz */
        uSwsClkPeriodNumerator = 155844;
        uSwsClkPeriodDenominator = 100000;
        break;
      case 130:
        /* 541.67 MHz */
        uSwsClkPeriodNumerator = 184615;
        uSwsClkPeriodDenominator = 100000;
        break;
      default:
        /* Divide by 1.67 by multiplying by 3/5 for 600MHz */
        uSwsClkPeriodNumerator = 5;
        uSwsClkPeriodDenominator = 3;
    }

    /* uuTransmitDuration represents milli-seconds when emulation and seconds for SV -  so need to convert to SWS clock ticks */
    /* First converting to nano-seconds */
    uuTransmitDuration *= ( SAL_BOOT_QUICKTURN ) ? 1000000 : 1000000000;
    uuTransmitDuration /= uSwsClkPeriodNumerator;
    uuTransmitDuration *= uSwsClkPeriodDenominator;
    uTransmitDurationLSBs = COMPILER_64_LO(uuTransmitDuration);
    uTransmitDurationMSBs = COMPILER_64_HI(uuTransmitDuration);
  }
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG0r, &uRegisterValue, IL_PKTINJ_ENABLEf, 1 );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG0r, &uRegisterValue, IL_PKTINJ_INCRMODE_PKTLEN_ENABLEf, (uint32) nPacketLenMode );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG0r, &uRegisterValue, IL_PKTINJ_INCRMODE_CHQID_ENABLEf, (uint32) nChannelMode );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG0r, &uRegisterValue, IL_PKTINJ_PKT_IPGf, (uint32) nIPG );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG1r, &uRegisterValue, IL_PKTINJ_PKTLENf, (uint32) nMinPacketLen );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG1r, &uRegisterValue, IL_PKTINJ_PKT_COUNTf, (uint32) nPacketCount );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG2r, &uRegisterValue, IL_PKTINJ_INCRMODE_MAX_PKTLENf, (uint32) nMaxPacketLen );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG2r, &uRegisterValue, IL_PKTINJ_CHQIDf, uMinChannel );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG2r, &uRegisterValue, IL_PKTINJ_INCRMODE_MAX_CHQIDf, uMaxChannel );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG2r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG3r, &uRegisterValue, IL_PKTINJ_SELECT_TX_PATHf, (uint32) nInjectDirection );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG3r, &uRegisterValue, IL_PKTINJ_RANDOM_RANGEf, uRandomRange );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG3r, &uRegisterValue, IL_PKTINJ_PAYLOAD_PATTERNf, (uint32) nPayloadPattern );
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG3r, &uRegisterValue, IL_PKTINJ_PAYLOAD_OFFSETf, uPayloadOffset );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG3r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG4r, &uRegisterValue, IL_PKTINJ_TIMER_VALUEf, uTransmitDurationLSBs );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG4r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTINJ_CONFIG5r, &uRegisterValue, IL_PKTINJ_TIMER_VALUE_MSBf, uTransmitDurationMSBs );
  soc_reg32_set( nUnit, IL_PKTINJ_CONFIG5r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  /*
    Do not allow SWS back-pressure destined for the packet generator to reach or come back into the SWS
    as the IL OOB interface is either in loop-back or cross-connected depending upon the TDM.
  */
  uRegisterValue = 0xffffffff;
  soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_FLOWCONTROL_CONFIGr, &uRegisterValue, IL_FLOWCONTROL_RXFC_OVERRIDE_ENABLEf, 1 );
  soc_reg32_set( nUnit, IL_FLOWCONTROL_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

/*
  uRegisterValue = 0xffffffff;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }
*/

  return 0;

#endif

}


int c3hppc_sws_il_pktgen_builtin_packet_header_init( void ) {
  int nIndex;

  g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN][0] = 0xdadadada;
  g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN][1] = 0xdada5a5a;
  g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN][2] = 0x5a5a5a5a;
  g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN][3] = 0x81000000;
  g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN][4] = 0xffff0000;
  g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN] = 18;

  /* Added for B0 */
  if ( g_rev_id != BCM88030_B0_REV_ID ) {

    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][8] = 0x11222233;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][9] = 0x33334444;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][10] = 0x44445555;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][11] = 0x55555566;
    g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__NULL0] = 48;

    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][8] = 0x889999aa;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][9] = 0xaaaabbbb;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][10] = 0xbbbbcccc;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][11] = 0xccccccdd;
    g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__NULL1] = 48;

  } else {

    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][0] = 0x11222233;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][1] = 0x33334444;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][2] = 0x44445555;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][3] = 0x55555566;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][4] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][5] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][6] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL0][7] = 0x00000000;
    g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__NULL0] = 32;

    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][0] = 0x889999aa;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][1] = 0xaaaabbbb;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][2] = 0xbbbbcccc;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][3] = 0xccccccdd;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][4] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][5] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][6] = 0x00000000;
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__NULL1][7] = 0x00000000;
    g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__NULL1] = 32;

  }

  for ( nIndex = 0; nIndex < ILPKTGEN__BUILTIN_HEADERS_MAX_WORDS; ++nIndex ) {
    g_auBuiltInPacketHeaders[ILPKTGEN__BUILTIN_HEADERS__PATTERN_256B][nIndex] = nIndex;
  }
  g_anBuiltInPacketHeadersLength[ILPKTGEN__BUILTIN_HEADERS__PATTERN_256B] = 256;

  return 0;
}

int c3hppc_sws_il_pktcap_arm( int nUnit, int nInstance ) {

  uint32 uRegisterValue;
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTCAP_STATUSr, &uRegisterValue, IL_PKTCAP_DONEf, 1 );
  soc_reg32_set( nUnit, IL_PKTCAP_STATUSr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTCAP_CONTROLr, &uRegisterValue, IL_PKTCAP_GOf, 0 );
  soc_reg32_set( nUnit, IL_PKTCAP_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  soc_reg_field_set( nUnit, IL_PKTCAP_CONTROLr, &uRegisterValue, IL_PKTCAP_GOf, 1 );
  soc_reg32_set( nUnit, IL_PKTCAP_CONTROLr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  
  return 0;
}

int c3hppc_sws_il_pktcap_wait_for_done( int nUnit, int nInstance, int nTimeoutInIterations ) {
  sal_time_t TimeStamp;

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), 
                               IL_PKTCAP_STATUSr, IL_PKTCAP_DONEf, 1, nTimeoutInIterations, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_sws_il_pktcap_wait_for_done> -- IL%d IL_PKTCAP_STATUS \"IL_PKTCAP_DONE\" event TIMEOUT!!!\n", nInstance);
    return -1;
  }

  return 1;
}

int c3hppc_sws_il_pktcap_setup( int nUnit, int nInstance, int nCaptureDirection, int nOneShot,
                                uint64 uuChannelMask ) {

  uint32 uRegisterValue, uChannelMaskMSBs, uChannelMaskLSBs;

  uChannelMaskLSBs = COMPILER_64_LO(uuChannelMask);
  uChannelMaskMSBs = COMPILER_64_HI(uuChannelMask);

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IL_PKTCAP_CONFIGr, &uRegisterValue, IL_PKTCAP_CAPTURE_MODEf, (uint32) (nOneShot ? 0 : 1) );
  soc_reg_field_set( nUnit, IL_PKTCAP_CONFIGr, &uRegisterValue, IL_PKTCAP_DATAPATH_SELf, (uint32) nCaptureDirection );
  soc_reg32_set( nUnit, IL_PKTCAP_CONFIGr, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );

  soc_reg32_set( nUnit, IL_PKTCAP_CHANNEL_SEL_1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uChannelMaskMSBs );
  soc_reg32_set( nUnit, IL_PKTCAP_CHANNEL_SEL_0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uChannelMaskLSBs );

  return 0;
}


int c3hppc_sws_il_pktcap_get( int nUnit, int nInstance, uint32 *pPacket, uint32 *pPacketLen,
                              uint32 *pSOP, uint32 *pEOP, uint32 *pERR, uint32 *pBV, uint32 *pChannel ) {

  uint32 uSideBandInfo;
  int nWord;
  static int anHeaderRegisterNames[64] = { IL_PKTCAP_PKTHDR_0r,  IL_PKTCAP_PKTHDR_1r,  IL_PKTCAP_PKTHDR_2r,  IL_PKTCAP_PKTHDR_3r,
                                           IL_PKTCAP_PKTHDR_4r,  IL_PKTCAP_PKTHDR_5r,  IL_PKTCAP_PKTHDR_6r,  IL_PKTCAP_PKTHDR_7r,
                                           IL_PKTCAP_PKTHDR_8r,  IL_PKTCAP_PKTHDR_9r,  IL_PKTCAP_PKTHDR_10r, IL_PKTCAP_PKTHDR_11r,
                                           IL_PKTCAP_PKTHDR_12r, IL_PKTCAP_PKTHDR_13r, IL_PKTCAP_PKTHDR_14r, IL_PKTCAP_PKTHDR_15r,
                                           IL_PKTCAP_PKTHDR_16r, IL_PKTCAP_PKTHDR_17r, IL_PKTCAP_PKTHDR_18r, IL_PKTCAP_PKTHDR_19r,
                                           IL_PKTCAP_PKTHDR_20r, IL_PKTCAP_PKTHDR_21r, IL_PKTCAP_PKTHDR_22r, IL_PKTCAP_PKTHDR_23r,
                                           IL_PKTCAP_PKTHDR_24r, IL_PKTCAP_PKTHDR_25r, IL_PKTCAP_PKTHDR_26r, IL_PKTCAP_PKTHDR_27r,
                                           IL_PKTCAP_PKTHDR_28r, IL_PKTCAP_PKTHDR_29r, IL_PKTCAP_PKTHDR_30r, IL_PKTCAP_PKTHDR_31r,
                                           IL_PKTCAP_PKTHDR_32r, IL_PKTCAP_PKTHDR_33r, IL_PKTCAP_PKTHDR_34r, IL_PKTCAP_PKTHDR_35r,
                                           IL_PKTCAP_PKTHDR_36r, IL_PKTCAP_PKTHDR_37r, IL_PKTCAP_PKTHDR_38r, IL_PKTCAP_PKTHDR_39r,
                                           IL_PKTCAP_PKTHDR_40r, IL_PKTCAP_PKTHDR_41r, IL_PKTCAP_PKTHDR_42r, IL_PKTCAP_PKTHDR_43r,
                                           IL_PKTCAP_PKTHDR_44r, IL_PKTCAP_PKTHDR_45r, IL_PKTCAP_PKTHDR_46r, IL_PKTCAP_PKTHDR_47r,
                                           IL_PKTCAP_PKTHDR_48r, IL_PKTCAP_PKTHDR_49r, IL_PKTCAP_PKTHDR_50r, IL_PKTCAP_PKTHDR_51r,
                                           IL_PKTCAP_PKTHDR_52r, IL_PKTCAP_PKTHDR_53r, IL_PKTCAP_PKTHDR_54r, IL_PKTCAP_PKTHDR_55r,
                                           IL_PKTCAP_PKTHDR_56r, IL_PKTCAP_PKTHDR_57r, IL_PKTCAP_PKTHDR_58r, IL_PKTCAP_PKTHDR_59r,
                                           IL_PKTCAP_PKTHDR_60r, IL_PKTCAP_PKTHDR_61r, IL_PKTCAP_PKTHDR_62r, IL_PKTCAP_PKTHDR_63r };
  static int anSideBandRegisterNames[8] = { IL_PKTCAP_PKTSB_0r,  IL_PKTCAP_PKTSB_1r,  IL_PKTCAP_PKTSB_2r,  IL_PKTCAP_PKTSB_3r,
                                            IL_PKTCAP_PKTSB_4r,  IL_PKTCAP_PKTSB_5r,  IL_PKTCAP_PKTSB_6r,  IL_PKTCAP_PKTSB_7r };

  /* Retrieve the contents of the IL_PKTCAP_PKTHDR_XX registers to assemble a packet */
  for ( nWord = 0; nWord < COUNTOF(anHeaderRegisterNames); ++nWord ) {
    soc_reg32_get( nUnit, anHeaderRegisterNames[nWord], SOC_BLOCK_PORT(nUnit,nInstance), 0, pPacket+nWord );
  }

  /* Retrieve the contents of the IL_PKTCAP_PKTSB_X registers to capture the sideband info */
  *pPacketLen = 0;
  *pERR = 0;
  *pSOP = 0;
  for ( nWord = 0; nWord < COUNTOF(anSideBandRegisterNames); ++nWord ) {
    soc_reg32_get( nUnit, anSideBandRegisterNames[nWord], SOC_BLOCK_PORT(nUnit,nInstance), 0, &uSideBandInfo );
    if ( nWord == 0 ) *pSOP = (uSideBandInfo >> 17) & 1;
    *pEOP = (uSideBandInfo >> 16) & 1;
    *pBV = (uSideBandInfo >> 8) & 0x3f;
    *pPacketLen += *pBV; 
    *pChannel = uSideBandInfo & 0x3f;
    if ( *pEOP == 1 ) {
      *pERR = (uSideBandInfo >> 18) & 1;
      break;
    }
  }

  return 0;
}





int c3hppc_sws_hw_cleanup( int nUnit ) {

  int nIndex, nInstance;
  uint32 uRegisterValue;

  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nIlErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anIlErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nIlErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nIlErrorMaskRegistersCount; ++nIndex ) {
      uRegisterValue = 0;
      soc_reg32_set( nUnit, g_anIlErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
  }


  for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nPrErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anPrErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nPrErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nPrErrorMaskRegistersCount; ++nIndex ) {
      uRegisterValue = 0;
      soc_reg32_set( nUnit, g_anPrErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
  }


  for ( nInstance = 0; nInstance < C3HPPC_SWS_PT_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nPtErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anPtErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nPtErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nPtErrorMaskRegistersCount; ++nIndex ) {
      uRegisterValue = 0;
      soc_reg32_set( nUnit, g_anPtErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
  }

  nInstance = 0;

  for ( nIndex = 0; nIndex < g_nQmErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anQmErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                              ((nIndex < g_nQmErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nQmErrorMaskRegistersCount; ++nIndex ) {
    uRegisterValue = 0;
    soc_reg32_set( nUnit, g_anQmErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }


  for ( nIndex = 0; nIndex < g_nPbErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anPbErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                              ((nIndex < g_nPbErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nPbErrorMaskRegistersCount; ++nIndex ) {
    uRegisterValue = 0;
    soc_reg32_set( nUnit, g_anPbErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }


  for ( nIndex = 0; nIndex < g_nPdErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anPdErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                              ((nIndex < g_nPdErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nPdErrorMaskRegistersCount; ++nIndex ) {
    uRegisterValue = 0;
    soc_reg32_set( nUnit, g_anPdErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }


  for ( nIndex = 0; nIndex < g_nPpErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anPpErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                              ((nIndex < g_nPpErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nPpErrorMaskRegistersCount; ++nIndex ) {
    uRegisterValue = 0;
    soc_reg32_set( nUnit, g_anPpErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }

  uRegisterValue = 0xffffffff;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_RXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS0r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, IL_FLOWCONTROL_TXFC_STS1r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }

  uRegisterValue = 0xffffffff;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_PT_INSTANCE_NUM; ++nInstance ) {
    soc_reg32_set( nUnit, PT_IPTE_FLOW_CONTROL_DEBUG6r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, PT_IPTE_FLOW_CONTROL_DEBUG7r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, PT_IPTE_FLOW_CONTROL_DEBUG8r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    soc_reg32_set( nUnit, PT_IPTE_FLOW_CONTROL_DEBUG9r, SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
  }

  return 0;
}



int c3hppc_sws_display_error_state( int nUnit ) {

#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int rc, nIndex, nInstance, nIndex1, nPortBlk, nPortIndex;
  uint32 uRegisterValue, uQmAllocatedWatermark, uPagesAllocatedWatermark;
  uint64 uReg64;

  for ( rc = 0, nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nIlErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anIlErrorRegisters[nIndex] );
    }
  }

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nPrErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anPrErrorRegisters[nIndex] );
    }
  }

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PT_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nPtErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anPtErrorRegisters[nIndex] );
    }
  }

  nInstance = 0;
  for ( nIndex = 0; nIndex < g_nQmErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anQmErrorRegisters[nIndex] );
  }

  for ( nIndex = 0; nIndex < g_nPbErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anPbErrorRegisters[nIndex] );
  }

  for ( nIndex = 0; nIndex < g_nPdErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anPdErrorRegisters[nIndex] );
  }

  for ( nIndex = 0; nIndex < g_nPpErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anPpErrorRegisters[nIndex] );
  }

  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_INGRESS_SQ_DROP_COUNTr );
  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_EGRESS_SQ_DROP_COUNTr );
  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_PED_INGRESS_DROP_COUNTr );
  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_PED_EGRESS_DROP_COUNTr );
  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_RESERVED_DROP_COUNTr );
  c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), QM_AGED_DROP_COUNTr );



  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), IL_FLOWCONTROL_RXFC_STS1r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), IL_FLOWCONTROL_RXFC_STS0r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), IL_FLOWCONTROL_TXFC_STS1r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), IL_FLOWCONTROL_TXFC_STS0r );
  }

  for ( nInstance = 0; nInstance < C3HPPC_SWS_PT_INSTANCE_NUM; ++nInstance ) {
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PT_IPTE_FLOW_CONTROL_DEBUG6r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PT_IPTE_FLOW_CONTROL_DEBUG7r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PT_IPTE_FLOW_CONTROL_DEBUG8r );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PT_IPTE_FLOW_CONTROL_DEBUG9r );
  }

  if ( g_bIlInstanceEnabled[C3HPPC_SWS_LINE_INTERFACE] == 0 ) {
    cli_out("\n");
    for ( nPortBlk = 0; nPortBlk < 5; ++nPortBlk ) {
      for ( nPortIndex = 0; nPortIndex < 12; ++nPortIndex ) {
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], TXPFr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("TXPF[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], RXPFr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("RXPF[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], TUFLr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("TUFL[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], TERRr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("TERR[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], RFCSr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("RFCS[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], RERPKTr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("RERPKT[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
        c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], RJBRr, nPortIndex, 0, &uReg64 );
        if ( uReg64 ) {
            cli_out("RJBR[client%d,port%d] -->  0x%lld  \n", nPortBlk, nPortIndex, uReg64 );
        }
      }
      cli_out("\n");
    }
  }


  for ( nInstance = 0; nInstance < C3HPPC_SWS_PR_INSTANCE_NUM; ++nInstance ) {
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_STATS_GLOBAL_DROP_PKTS_NO_FREE_PAGES_COUNTr );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_STATS_GLOBAL_DROP_PKTS_PB_ALMOST_FULL_COUNTr );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_STATS_GLOBAL_TOTAL_DROPPED_ENQUEUE_DONE_TO_QM_COUNTr );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_STATS_GLOBAL_TOTAL_PKTS_FULLY_DROPPED_COUNTr );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IDP_STATS_GLOBAL_CLIENT_IF_DROP_PKTS_COUNTr );
    c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), PR_IPRE_BUFFER_FULL_ERROR_PORTr );

    for ( nIndex = 0; nIndex < 128; ++nIndex ) {
      soc_reg32_get( nUnit, PR_HDP_STATS_DROP_PKT_COUNTr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, &uRegisterValue );
      if ( uRegisterValue ) {
        cli_out("Register[PR_HDP_STATS_DROP_PKT_COUNT(%d).pr%d] --> %d\n", nIndex, nInstance, uRegisterValue );
      }
    }
    for ( nIndex = 0; nIndex < 64; ++nIndex ) {
      soc_reg32_get( nUnit, PR_IDP_STATS_DROP_PKT_COUNTr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, &uRegisterValue );
      if ( uRegisterValue ) {
        cli_out("Register[PR_IDP_STATS_DROP_PKT_COUNT(%d).pr%d] --> %d\n", nIndex, nInstance, uRegisterValue );
      }
      soc_reg32_get( nUnit, PR_IDP_STATS_DROP_PKT_REASONSr, SOC_BLOCK_PORT(nUnit,nInstance), nIndex, &uRegisterValue );
      if ( uRegisterValue ) {
        cli_out("Register[PR_IDP_STATS_DROP_PKT_REASONS(%d).pr%d] --> %d\n", nIndex, nInstance, uRegisterValue );

        nIndex1 = (C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM * nInstance) + nIndex;
        if ( soc_mem_read( nUnit, QM_ALLOCATION_WATERMARKm, MEM_BLOCK_ANY, nIndex1, &uQmAllocatedWatermark) ) return -1;
        soc_mem_field_get( nUnit, QM_ALLOCATION_WATERMARKm, &uQmAllocatedWatermark, PAGES_ALLOCATED_WATERMARKf, &uPagesAllocatedWatermark );
        cli_out("Memory[QM_ALLOCATION_WATERMARK(%d)] --> 0x%x\n", nIndex1, uPagesAllocatedWatermark );
      }
    }
  }


  return rc;
#endif
}

/* Function content lifted from soc_reg_addr_get() in src/soc/common/reg.c */
uint32 c3hppc_sws_get_port_register_addr( int nUnit, soc_reg_t nReg, int nPort, int nIndex ) {

  uint32 uAddr;
  int    nGranularityShift;

  uAddr = SOC_REG_INFO(nUnit, nReg).offset | nPort;

  nGranularityShift = 8;
  if ( SOC_REG_IS_ARRAY(nUnit, nReg) ) {
    assert(nIndex >= 0 && nIndex < SOC_REG_NUMELS(nUnit, nReg));
    uAddr += nIndex*SOC_REG_ELEM_SKIP(nUnit, nReg);
  } else if ( nIndex && SOC_REG_ARRAY(nUnit, nReg) ) {
    assert(nIndex >= 0 && nIndex < SOC_REG_NUMELS(nUnit, nReg));
    if ( nIndex && SOC_REG_ARRAY2(nUnit, nReg) ) {
      uAddr += ((2*nIndex) << nGranularityShift);
    } else {
      uAddr += (nIndex << nGranularityShift);
    }
  }

  return uAddr;
}

/* Function content lifted from soc_reg_addr_get() in src/soc/common/reg.c */
uint8 c3hppc_sws_get_port_register_acc_type( int nUnit, soc_reg_t nReg ) {
  return (SOC_REG_INFO(nUnit, nReg).flags >> SOC_MEM_FLAG_ACC_TYPE_SHIFT) & SOC_MEM_FLAG_ACC_TYPE_MASK;
}

int c3hppc_sws_soc_reg32_get( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint32 *puData ) {

  _soc_reg32_get( nUnit, nBlk,
                  c3hppc_sws_get_port_register_acc_type( nUnit, nReg ),
                  c3hppc_sws_get_port_register_addr( nUnit, nReg, nPort, nIndex ),
                  puData );

  return 0;
}

int c3hppc_sws_soc_reg32_set( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint32 uData ) {

  _soc_reg32_set( nUnit, nBlk,
                  c3hppc_sws_get_port_register_acc_type( nUnit, nReg ),
                  c3hppc_sws_get_port_register_addr( nUnit, nReg, nPort, nIndex ),
                  uData );

  return 0;
}

int c3hppc_sws_soc_reg64_get( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint64 *puData ) {

  _soc_reg64_get( nUnit, nBlk,
                  c3hppc_sws_get_port_register_acc_type( nUnit, nReg ),
                  c3hppc_sws_get_port_register_addr( nUnit, nReg, nPort, nIndex ),
                  puData );

  return 0;
}

int c3hppc_sws_soc_reg64_set( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint64 uData ) {

  _soc_reg64_set( nUnit, nBlk,
                  c3hppc_sws_get_port_register_acc_type( nUnit, nReg ),
                  c3hppc_sws_get_port_register_addr( nUnit, nReg, nPort, nIndex ),
                  uData );

  return 0;
}



int c3hppc_sws_transition_to_1x40G_8x10G_from_12x10G( int nUnit ) {

#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg32;
  uint64 uReg64;
  int nPortBlk, nPortIndex, nIndex;
  uint32 uFieldValue, uQ2CmapEntry;
  uint32 uClientCalEntry, uPortCalEntry, uTxPort, uCalendarSelect;
  uint32 auSourceQueueConfigEntry[3];


  cli_out("\nINFO: Transitioning to a 1x40G_8x10G line side configuration ..... \n");

  /* 
     Force flow control on the fabric side source queues (72-75) associated with line side 10G ports 8 through 11
  */
  WRITE_QM_FC_DEBUG2r( nUnit, 0x00000f00 );
  WRITE_QM_FC_DEBUG5r( nUnit, 3 );

  /* 
     Wait for queues to drain associated with line side 10G ports 8 through 11
  */
  sal_usleep( 1000000 );

  /*
     Disable 10G ports 8 through 11
  */
  nPortBlk = 0;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f,  0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f,  0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  /* Assert reset to XPORT2 */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_XGXS2_CTRL_REGr, &uReg32, RSTB_HWf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, uReg32 );

  /* 
     Hold the PT0 interface logic with CLport ports 8-11 in reset.
     Changing the XPORT2_CORE_PORT_MODE value (below) caused bad things to happen in the PT.  Packet
     loss happened on XPORT0 port2 in the transmit direction and a QM FP_UNDERFLOW occurred in ingress path indicating
     some kind of rate increase was introduced across the 8 active ports.  Suspect a credit issue.

     The situation described above was due to changing the XPORT2_CORE_PORT_MODE value while running in the A0 mode
     of PT_IPTE_CONFIG0.pt_ipte_port_remap_en = 1.  This caused port 2 activity to come out on port 8 prematurely.
     By setting PT_IPTE_CONFIG0.pt_ipte_port_remap_en = 0 and adjusting 0,1,2 references to 0,4,8 for 40G ports in
     the various control structures the below reset is not necessary.

  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 1 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }
  */

  /* 
     Re-configure XPORT_CORE2 to be a 40G port
  */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_PHY_PORT_MODEf,  0 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );

  /* De-assert reset to XPORT2 */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  /* 
     Setup XMAC (xe8) to be a 40G port
  */
  nPortIndex = 8;
  c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, RX_ENf, 1 );
  soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, TX_ENf, 1 );
  soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, XLGMII_ALIGN_ENBf, 1 );
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );

  c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, &uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 ); /* Replace (testing purposes) */
  soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, TX_64BYTE_BUFFER_ENf, 1 );
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, uReg64 );

  c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, &uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 ); /* Strip (testing purposes) */
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, uReg64 );

  c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, &uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_RX_MAX_SIZEr, &uReg64, RX_MAX_SIZEf, 0x3fe8 );
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, uReg64 );

  c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, &uReg64 );
  soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 1 );
  soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 1 );
  c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, uReg64 );
  
  /* 
     Setup new PT0 client and port calendars and swap them in.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    uCalendarSelect = soc_reg_field_get( nUnit, PT_IPTE_CONFIG4r, uReg32, PT_IPTE_CLIENT_CAL_SELf );
    uCalendarSelect ^= 1;

    for ( nIndex = 0; nIndex < 39; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE0f, &uFieldValue );
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE1f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 6 ) uFieldValue = 4;
      else if ( nIndex == 19 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT0f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 12 || nIndex == 25 ) uFieldValue = 4;
      else if ( nIndex == 38 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT1f, &uFieldValue );

      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_ENf : CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_END_ADDRf : CAL_END_ADDRf), 77 );
    soc_reg32_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nIndex = 0, uTxPort = 0; nIndex < 99; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, ACTIVEf, &uFieldValue );

      if ( uTxPort == 12 && nIndex <= 38 ) {
        if ( nIndex == 12 ) uFieldValue = 48;
        else if ( nIndex == 25 ) uFieldValue = 49;
        else if ( nIndex == 38 ) uFieldValue = 50;
        uTxPort = 0;
      } else {
        uFieldValue = uTxPort;
        ++uTxPort;
        if ( nIndex > 38 && uTxPort == 12 ) uTxPort = 0;
      }
      if ( uFieldValue >= 9 && uFieldValue <= 11 ) uFieldValue = 8;
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, PORTf, &uFieldValue );
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32, 
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_ENf : PT_IPTE_PORT_CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32,
                                                    (uCalendarSelect ? PT_IPTE_PORT_CAL1_END_ADDRf : PT_IPTE_PORT_CAL_END_ADDRf), 98 );
    soc_reg32_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_CLIENT_CAL_SELf, uCalendarSelect );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT_CAL_SELf, uCalendarSelect );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  }
  

  /* 
     Size PORT_FIFO for port 8 to be 40G
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    nPortIndex = 8;
    soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, SIZEf, 26 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, PKT_SERVICE_PAGES_THRESHOLDf, 7 );
    soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, uReg32 );

    soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATE_PORTf, nPortIndex );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATEf, 1 );
    soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  /* 
     For PT1 give priority to the 40G port passing traffic on queue 8
  */
  soc_reg32_get( nUnit, PT_IPTE_PORT0_WDRR_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, &uReg32, PT_IPTE_WDRR_ENf, 1 );
  soc_reg_field_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, &uReg32, PT_IPTE_SPR0_ENf, 1 );
  soc_reg_field_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, &uReg32, PT_IPTE_SPR0_IDf, 8 );
  soc_reg32_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), 0, uReg32 );

  
  /* 
     Enable XPORT2 (port 8)
  */
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f,  1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );


  /* 
     Cleaning up the credits that were passed to PT0 with the above enable of XPORT2 via PORT_ENABLE_REG.
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_VALUEf, 16 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_PORTf, 8 );
    soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATEf, 1 );
    soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  /* 
     Funnel the destination queues for ports 8 through 11 to port 8 using the PT queue to port map.
  */
  for ( nIndex = 8; nIndex < 12; ++nIndex ) {
    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_Q2C_MAPm,
                                      PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uQ2CmapEntry) );
    uFieldValue = 8;
    soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, PORTf, &uFieldValue );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_Q2C_MAPm,
                                       PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uQ2CmapEntry) );
  }

  /* 
     Modify the ingress source queue configuration for queue 8 as the port rate has changed from 10G to 40G.
  */
  nIndex = 8; 
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                    MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );

  soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
  uFieldValue *= 4;
  soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
  uFieldValue -= C3HPPC_SWS_QM_GT10G_FLOW_CTRL_THOLD_DELTA;
  soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, FLOW_CTRL_THRESHf, &uFieldValue );

  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                     MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );

  /* 
     Release flow control on the fabric side source queues (72-75) associated with line side 40G port 8
  */
  WRITE_QM_FC_DEBUG5r( nUnit, 0 );
  WRITE_QM_FC_DEBUG2r( nUnit, 0x00000000 );


  return 0;
#endif
}





int c3hppc_sws_transition_to_12x10G_from_1x40G_8x10G( int nUnit ) {

#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg32;
  uint64 uReg64;
  int nPortBlk, nPortIndex, nIndex;
  uint32 uFieldValue, uQ2CmapEntry, uPortFifoBaseAddress;
  uint32 uClientCalEntry, uPortCalEntry, uTxPort, uCalendarSelect;
  uint32 auSourceQueueConfigEntry[3];



  cli_out("\nINFO: Transitioning to a 12x10G line side configuration ..... \n");


  /* 
     Force flow control on the fabric side source queues (72-75) associated with line side 40G port 8
  */
  WRITE_QM_FC_DEBUG2r( nUnit, 0x00000f00 );
  WRITE_QM_FC_DEBUG5r( nUnit, 3 );

  /* 
     Wait for queues to drain associated with line side 40G port 8
  */
  sal_usleep( 1000000 );

  /*
     Disable 40G port 8
  */
  nPortBlk = 0;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f,  0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  /* Assert reset to XPORT2 */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_XGXS2_CTRL_REGr, &uReg32, RSTB_HWf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, uReg32 );

  /* 
     To prevent a IPTE_CLIENT0_CLPORT_CREDIT_OVERFLOW_ERROR condition placing the PT0 QUAD2 interface in reset
     so that PT0 does not see the XPORT2_CORE_PORT_MODE change to 10G with a 40G port's quantity of credits.
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 1 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  /* 
     Re-configure XPORT_CORE2 to be 4x10G
  */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 2 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );

  /* De-assert reset to XPORT2 */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  /* 
     Setup XMACs (xe8,xe9,xe10,xe11) to be 10G ports
  */
  for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, RX_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, TX_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 ); /* Replace (testing purposes) */
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, TX_64BYTE_BUFFER_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 ); /* Strip (testing purposes) */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_MAX_SIZEr, &uReg64, RX_MAX_SIZEf, 0x3fe8 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, uReg64 );
  }
  
  /* 
     Setup new PT0 client and port calendars and swap them in.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    uCalendarSelect = soc_reg_field_get( nUnit, PT_IPTE_CONFIG4r, uReg32, PT_IPTE_CLIENT_CAL_SELf );
    uCalendarSelect ^= 1;

    for ( nIndex = 0; nIndex < 39; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE0f, &uFieldValue );
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE1f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 6 ) uFieldValue = 4;
      else if ( nIndex == 19 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT0f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 12 || nIndex == 25 ) uFieldValue = 4;
      else if ( nIndex == 38 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT1f, &uFieldValue );

      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_ENf : CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_END_ADDRf : CAL_END_ADDRf), 77 );
    soc_reg32_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nIndex = 0, uTxPort = 0; nIndex < 99; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, ACTIVEf, &uFieldValue );

      if ( uTxPort == 12 && nIndex <= 38 ) {
        if ( nIndex == 12 ) uFieldValue = 48;
        else if ( nIndex == 25 ) uFieldValue = 49;
        else if ( nIndex == 38 ) uFieldValue = 50;
        uTxPort = 0;
      } else {
        uFieldValue = uTxPort;
        ++uTxPort;
        if ( nIndex > 38 && uTxPort == 12 ) uTxPort = 0;
      }
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, PORTf, &uFieldValue );
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32, 
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_ENf : PT_IPTE_PORT_CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32,
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_END_ADDRf : PT_IPTE_PORT_CAL_END_ADDRf), 98 );
    soc_reg32_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_CLIENT_CAL_SELf, uCalendarSelect );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT_CAL_SELf, uCalendarSelect );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  }
  

  /* 
     Size PORT_FIFO for ports 8-11 to be 10G 
  */
  uPortFifoBaseAddress = 0;
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, SIZEf, 8 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, PKT_SERVICE_PAGES_THRESHOLDf, 2 );
      if ( nPortIndex == 8 ) {
        uPortFifoBaseAddress = soc_reg_field_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, uReg32, BASE_ADDRESSf );
      } else {
        soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, BASE_ADDRESSf, uPortFifoBaseAddress );
      }
      soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, uReg32 );
      /* Actual depth is SIZE*8 */
      uPortFifoBaseAddress += (8 * 8);

      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATE_PORTf, nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }




  /* 
     Enable XPORT2 (ports 8-11)
  */
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );


  /* 
     Initializing PT0 with the proper number of credits for the four 10G ports. 
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {

    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 0 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_VALUEf, 4 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_PORTf, nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }

  /* 
     Distribute the destination queues for ports 8 through 11 to the individual ports using the PT queue to port map.
  */
  for ( nIndex = 8; nIndex < 12; ++nIndex ) {
    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, IPTE_Q2C_MAPm,
                                      PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uQ2CmapEntry) );
    soc_mem_field_set( nUnit, IPTE_Q2C_MAPm, &uQ2CmapEntry, PORTf, (uint32 * ) &nIndex );
    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, IPTE_Q2C_MAPm,
                                       PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uQ2CmapEntry) );
  }

  /* 
     For PT1 remove the priority for the 40G port passing traffic on queue 8
  */
  soc_reg32_get( nUnit, PT_IPTE_PORT0_WDRR_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, &uReg32, PT_IPTE_WDRR_ENf, 0 );
  soc_reg_field_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, &uReg32, PT_IPTE_SPR0_ENf, 0 );
  soc_reg32_set( nUnit, PT_IPTE_PORT0_WDRR_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), 0, uReg32 );

  /* 
     Modify the ingress source queue configuration for queue 8 as the port rate has changed from 40G to 10G.
  */
  nIndex = 8; 
  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                    MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );

  soc_mem_field_get( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
  uFieldValue /= 4;
  soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
  uFieldValue -= C3HPPC_SWS_QM_LE10G_FLOW_CTRL_THOLD_DELTA;
  soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, FLOW_CTRL_THRESHf, &uFieldValue );

  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                     MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );

  /* 
     Release flow control on the fabric side source queues (72-75) associated with line side 40G port 8
  */
  WRITE_QM_FC_DEBUG5r( nUnit, 0 );
  WRITE_QM_FC_DEBUG2r( nUnit, 0x00000000 );


  return 0;
#endif
}



int c3hppc_sws_transition_to_16x1G_8x10G_from_12x10G( int nUnit ) {


#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg32;
  uint64 uReg64;
  int nPortBlk, nPortIndex, nIndex;
  uint32 uFieldValue, uPortFifoBaseAddress;
  uint32 uClientCalEntry, uPortCalEntry, uTxPort, uCalendarSelect;
  uint32 auSourceQueueConfigEntry[3];


  cli_out("\nINFO: Transitioning to a 16x1G_8x10G line side configuration ..... \n");

  /* 
     Force flow control on the fabric side source queues (72-75) associated with line side 10G ports 8 through 11
  */
  READ_QM_FC_DEBUG2r( nUnit, &uReg32 );
  uReg32 |= 0x00000f00;
  WRITE_QM_FC_DEBUG2r( nUnit, uReg32 );
  WRITE_QM_FC_DEBUG5r( nUnit, 3 );

  /* 
     Wait for queues to drain associated with line side 10G ports 8 through 11
  */
  sal_usleep( 1000000 );

  /*
     Disable 10G ports 8 through 11
  */
  nPortBlk = 0;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f,  0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f,  0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  /* 
     Assert reset to XPORT2
  */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  /*
     If the "TDM Optimize" mode is not done, then ports 0-7 underflow (TUFL counters) when the PT
     client and port calendars transition to the new TDM.
  */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 3 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_XGXS2_CTRL_REGr, &uReg32, RSTB_HWf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, uReg32 );

  /* 
     Hold the PT0 interface logic for XTPORT1/2 in reset.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 1 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_XT2f, 1 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_XT1f, 1 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  /* 
     Configure XTPORT1/2 to be 1G ports
  */
  nPortBlk = 2;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPC2_GMII_MII_ENABLEf, 1 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT1_CORE_PORT_MODEf, 3 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT0_CORE_PORT_MODEf, 3 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );
  nPortBlk = 3;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPC2_GMII_MII_ENABLEf, 1 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT1_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT1_CORE_PORT_MODEf, 2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPC1_GMII_MII_ENABLEf, 1 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT0_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT0_CORE_PORT_MODEf, 2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPC0_GMII_MII_ENABLEf, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );

  /* De-assert reset to XTPORT1/2 */
  for ( nPortBlk = 2; nPortBlk < 4; ++nPortBlk ) {
    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC0_RESETf, 0 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC1_RESETf, 0 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 0 );
    c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );
  }

  /* 
     Setup GMACs XTPORT1(ge32 - ge35) and XTPORT2(ge36 - ge47)
  */
  for ( nPortIndex = 8, nPortBlk = 2; nPortIndex < 12; ++nPortIndex ) {
    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, RX_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, TX_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 ); /* Replace (testing purposes) */
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, TX_64BYTE_BUFFER_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_MODEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_MODEr, &uReg64, SPEED_MODEf, 2 );  /* LINK_1G */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_MODEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_MAX_SIZEr, &uReg64, RX_MAX_SIZEf, 0x3fe8 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 ); /* Strip (testing purposes) */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, uReg64 );
  }
  for ( nPortIndex = 0, nPortBlk = 3; nPortIndex < 12; ++nPortIndex ) {
    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, RX_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, TX_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_MODEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_MODEr, &uReg64, SPEED_MODEf, 2 );  /* LINK_1G */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_MODEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 ); /* Replace (testing purposes) */
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, TX_64BYTE_BUFFER_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_MAX_SIZEr, &uReg64, RX_MAX_SIZEf, 0x3fe8 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 ); /* Strip (testing purposes) */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, uReg64 );
  }

  
  /* 
     Setup new PT0 client and port calendars and swap them in.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    uCalendarSelect = soc_reg_field_get( nUnit, PT_IPTE_CONFIG4r, uReg32, PT_IPTE_CLIENT_CAL_SELf );
    uCalendarSelect ^= 1;

    for ( nIndex = 0; nIndex < 13; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE0f, &uFieldValue );
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE1f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 1 ) uFieldValue = 2;
      else if ( nIndex == 6 ) uFieldValue = 3;
      else if ( nIndex == 12 ) uFieldValue = 2;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT0f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 3 || nIndex == 9 || nIndex == 12 ) uFieldValue = 3;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT1f, &uFieldValue );

      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_ENf : CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_END_ADDRf : CAL_END_ADDRf), 25 );
    soc_reg32_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nIndex = 0, uTxPort = 32; nIndex < 96; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, ACTIVEf, &uFieldValue );

      if ( nIndex < 80 ) {
        if ( (nIndex % 10) < 8 ) {
          uFieldValue = nIndex % 10;
        } else {
          uFieldValue = uTxPort;
          ++uTxPort;
        }
      } else {
        uFieldValue = nIndex % 8;
      }
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, PORTf, &uFieldValue );
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32, 
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_ENf : PT_IPTE_PORT_CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32,
                                                    (uCalendarSelect ? PT_IPTE_PORT_CAL1_END_ADDRf : PT_IPTE_PORT_CAL_END_ADDRf), 95 );
    soc_reg32_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_CLIENT_CAL_SELf, uCalendarSelect );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT_CAL_SELf, uCalendarSelect );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  }
  

  /* 
     Size PORT_FIFO for ports 32-47 to be 1G
  */
  soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 8, &uReg32 );
  uPortFifoBaseAddress = soc_reg_field_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, uReg32, BASE_ADDRESSf );
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    for ( nPortIndex = 32; nPortIndex < 48; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, SIZEf, 2 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, PKT_SERVICE_PAGES_THRESHOLDf, 1 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, BASE_ADDRESSf, uPortFifoBaseAddress );
      soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, uReg32 );
      /* Actual depth is SIZE*8 */
      uPortFifoBaseAddress += (2 * 8);

      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATE_PORTf, nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }

  /*
     Enable PT clients for XTPORTS 1/2
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT12_PORT47_BASE_PORTf, 12 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT12_PORT47_BASE_QUEUEf, 12 );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG2r, &uReg32, CLIENT_ENf, 1 );
  soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG3r, &uReg32, CLIENT_ENf, 1 );
  soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  /*
     Enable PR0 clients for XTPORTS 1/2
  */
  soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uReg32, XT1_CLIENT_IFf, 1 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uReg32, XT2_CLIENT_IFf, 1 );
  soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG1r, &uReg32, XT1_BASE_PORTf, 24 );
  soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG2r, &uReg32, XT2_BASE_PORTf, 36 );
  soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  
  


  /* 
     Enable XTPORT1/2 (port 8)
  */
  nPortBlk = 2;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );
  nPortBlk = 3;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 0 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE1f, 0 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE0f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  nPortBlk = 2;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  nPortBlk = 3;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT0f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT1f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT2f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT3f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT4f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT5f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT6f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT7f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );


  /* 
     Cleaning up the credits that were passed to PT0 with the above enable of XPORT1/2 via PORT_ENABLE_REG.
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {

    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_XT2f, 0 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_XT1f, 0 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nPortIndex = 32; nPortIndex < 48; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_VALUEf, 4 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_PORTf, (uint32) nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }

  /* 
     Configure the QM for 1G based ingress source queues 32-47 and 96-111
  */
  for ( nIndex = 32; nIndex < 112; ++nIndex ) {
    if ( (nIndex >= 32 && nIndex < 48) || (nIndex >= 96 && nIndex < 112) ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                        MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );
      uFieldValue = 160;  /* ~(C3HPPC_SWS_QM_INGRESS_MAX_PAGES/48) */
      soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, MAX_PAGESf, &uFieldValue );
      uFieldValue -= C3HPPC_SWS_QM_LE10G_FLOW_CTRL_THOLD_DELTA;
      soc_mem_field_set( nUnit, QM_SOURCE_QUEUE_CONFIGm, auSourceQueueConfigEntry, FLOW_CTRL_THRESHf, &uFieldValue );
    
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, QM_SOURCE_QUEUE_CONFIGm, 
                                         MEM_BLOCK_ANY, nIndex, auSourceQueueConfigEntry) );
    }
  }

  /* 
     The number of active queues will double, so adjusting the reserve page number accordingly.
  */
  READ_QM_BUFFER_CONFIG7r( nUnit, &uReg32 );
  uReg32 *= 2;
  WRITE_QM_BUFFER_CONFIG7r( nUnit, uReg32 );


  return 0;
#endif

}



int c3hppc_sws_transition_to_12x10G_from_16x1G_8x10G( int nUnit ) {


#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg32;
  uint64 uReg64;
  int nPortBlk, nPortIndex, nIndex;
  uint32 uFieldValue, uPortFifoBaseAddress;
  uint32 uClientCalEntry, uPortCalEntry, uTxPort, uCalendarSelect;


  cli_out("\nINFO: Transitioning to a 12x10G line side configuration ..... \n");

  /* 
     Force flow control on the fabric side source queues (96-111) associated with line side 1G ports 32 through 47
  */
  READ_QM_FC_DEBUG3r( nUnit, &uReg32 );
  uReg32 |= 0x0000ffff;
  WRITE_QM_FC_DEBUG3r( nUnit, uReg32 );
  WRITE_QM_FC_DEBUG5r( nUnit, 3 );

  /* 
     Wait for queues to drain associated with line side 1G ports 32 through 47
  */
  sal_usleep( 1000000 );

  /*
     Disable 1G ports 32 through 47
  */
  nPortBlk = 2;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  nPortBlk = 3;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT0f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT1f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT2f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT3f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT4f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT5f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT6f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT7f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 0 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  nPortBlk = 2;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );
  nPortBlk = 3;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 1 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE1f, 1 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE0f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  /* Assert reset to XTPORT1/2 */
  for ( nPortBlk = 2; nPortBlk < 4; ++nPortBlk ) {
    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC0_RESETf, 1 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC1_RESETf, 1 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 1 );
    c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );
  }

  for ( nPortBlk = 2; nPortBlk < 4; ++nPortBlk ) {
    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
    soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 3 );
    soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT1_CORE_PORT_MODEf, 3 );
    soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT0_CORE_PORT_MODEf, 3 );
    c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );
  }

  for ( nPortBlk = 2; nPortBlk < 4; ++nPortBlk ) {
    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, &uReg32 );
    soc_reg_field_set( nUnit, PORT_XGXS2_CTRL_REGr, &uReg32, RSTB_HWf, 0 );
    c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_XGXS2_CTRL_REGr, nPortIndex, 0, uReg32 );
  }

  /* 
     Hold the PT0 interface logic for client0 quad2 in reset.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 1 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
  }

  
  /* 
     Setup new PT0 client and port calendars and swap them in before changing the XPORT2_CORE_PORT_MODE value
     so that underflows do not occur on 10G ports 0-7.
  */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    uCalendarSelect = soc_reg_field_get( nUnit, PT_IPTE_CONFIG4r, uReg32, PT_IPTE_CLIENT_CAL_SELf );
    uCalendarSelect ^= 1;

    for ( nIndex = 0; nIndex < 39; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE0f, &uFieldValue );
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, ACTIVE1f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 6 ) uFieldValue = 4;
      else if ( nIndex == 19 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT0f, &uFieldValue );

      uFieldValue = 0;
      if ( nIndex == 12 || nIndex == 25 ) uFieldValue = 4;
      else if ( nIndex == 38 ) uFieldValue = 5;
      soc_mem_field_set( nUnit, g_anPtClientCalendarMems[uCalendarSelect], &uClientCalEntry, CLIENT1f, &uFieldValue );

      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtClientCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uClientCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_ENf : CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], &uReg32, (uCalendarSelect ? CAL1_END_ADDRf : CAL_END_ADDRf), 77 );
    soc_reg32_set( nUnit, g_anPtClientCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nIndex = 0, uTxPort = 0; nIndex < 99; ++nIndex ) {
      SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                        PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
      uFieldValue = 1;
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, ACTIVEf, &uFieldValue );

      if ( uTxPort == 12 && nIndex <= 38 ) {
        if ( nIndex == 12 ) uFieldValue = 48;
        else if ( nIndex == 25 ) uFieldValue = 49;
        else if ( nIndex == 38 ) uFieldValue = 50;
        uTxPort = 0;
      } else {
        uFieldValue = uTxPort;
        ++uTxPort;
        if ( nIndex > 38 && uTxPort == 12 ) uTxPort = 0;
      }
      soc_mem_field_set( nUnit, g_anPtPortCalendarMems[uCalendarSelect], &uPortCalEntry, PORTf, &uFieldValue );
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, g_anPtPortCalendarMems[uCalendarSelect],
                                         PT_BLOCK(nUnit,C3HPPC_SWS_LINE_INTERFACE), nIndex, &uPortCalEntry) );
    }
    soc_reg32_get( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32,
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_ENf : PT_IPTE_PORT_CAL_ENf), 1 );
    soc_reg_field_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], &uReg32,
                                                      (uCalendarSelect ? PT_IPTE_PORT_CAL1_END_ADDRf : PT_IPTE_PORT_CAL_END_ADDRf), 98 );
    soc_reg32_set( nUnit, g_anPtPortCalendarRegs[uCalendarSelect], SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    soc_reg32_get( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_CLIENT_CAL_SELf, uCalendarSelect );
    soc_reg_field_set( nUnit, PT_IPTE_CONFIG4r, &uReg32, PT_IPTE_PORT_CAL_SELf, uCalendarSelect );
    soc_reg32_set( nUnit, PT_IPTE_CONFIG4r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  }


  /*
     Configure XPORT_CORE2 to be 4x10G
  */
  nPortBlk = 0;
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_PHY_PORT_MODEf,  2 );
  soc_reg_field_set( nUnit, PORT_MODE_REGr, &uReg32, XPORT2_CORE_PORT_MODEf, 2 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MODE_REGr, nPortIndex, 0, uReg32 );

  /* 
     De-assert reset to XPORT2
  */
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );

  /*
     Setup XMACs (xe8,xe9,xe10,xe11) to be 10G ports
  */
  for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, RX_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, TX_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_CTRLr, &uReg64, SOFT_RESETf, 0 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, CRC_MODEf, 2 ); /* Replace (testing purposes) */
    soc_reg64_field32_set( nUnit, XMAC_TX_CTRLr, &uReg64, TX_64BYTE_BUFFER_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_TX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_CTRLr, &uReg64, STRIP_CRCf, 0 ); /* Strip (testing purposes) */
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_CTRLr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_RX_MAX_SIZEr, &uReg64, RX_MAX_SIZEf, 0x3fe8 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_RX_MAX_SIZEr, nPortIndex, 0, uReg64 );

    c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, &uReg64 );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, TX_PAUSE_ENf, 1 );
    soc_reg64_field32_set( nUnit, XMAC_PAUSE_CTRLr, &uReg64, RX_PAUSE_ENf, 1 );
    c3hppc_sws_soc_reg64_set( nUnit, g_anPortBlkNum[nPortBlk], XMAC_PAUSE_CTRLr, nPortIndex, 0, uReg64 );
  }
  
  /*
     Size PORT_FIFO for ports 8-11 to be 10G
  */
  uPortFifoBaseAddress = 0;
  if ( g_rev_id == BCM88030_B0_REV_ID ) {
    for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, SIZEf, 8 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, PKT_SERVICE_PAGES_THRESHOLDf, 2 );
      if ( nPortIndex == 8 ) {
        uPortFifoBaseAddress = soc_reg_field_get( nUnit, PT_IPTE_PORT_FIFO_CFGr, uReg32, BASE_ADDRESSf );
      } else {
        soc_reg_field_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, &uReg32, BASE_ADDRESSf, uPortFifoBaseAddress );
      }
      soc_reg32_set( nUnit, PT_IPTE_PORT_FIFO_CFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), nPortIndex, uReg32 );
      /* Actual depth is SIZE*8 */
      uPortFifoBaseAddress += (8 * 8);

      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATE_PORTf, nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, FIFO_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }

  /*
     Disable PT clients for XTPORTS 1/2
  */
  soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG2r, &uReg32, CLIENT_ENf, 0 );
  soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG2r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  soc_reg32_get( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PT_IPTE_CLIENT_CFG3r, &uReg32, CLIENT_ENf, 0 );
  soc_reg32_set( nUnit, PT_IPTE_CLIENT_CFG3r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  /*
     Disable PR0 clients for XTPORTS 1/2
  */
  soc_reg32_get( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uReg32, XT1_CLIENT_IFf, 0 );
  soc_reg_field_set( nUnit, PR_IPRE_CI_CONFIG0r, &uReg32, XT2_CLIENT_IFf, 0 );
  soc_reg32_set( nUnit, PR_IPRE_CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

  /*
     Enable XPORT2 (ports 8-11)
  */
  nPortIndex = 0;
  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_SOFT_RESETr, &uReg32, XPORT_CORE2f, 0 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, uReg32 );

  c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT8f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT9f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT10f, 1 );
  soc_reg_field_set( nUnit, PORT_ENABLE_REGr, &uReg32, PORT11f, 1 );
  c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, uReg32 );

  /*
     Initializing PT0 with the proper number of credits for the four 10G ports.
  */
  /* Added for B0 */
  if ( g_rev_id == BCM88030_B0_REV_ID ) {

    soc_reg32_get( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
    soc_reg_field_set( nUnit, PT_IPTE_MAC_RESETr, &uReg32, MAC_RST_CL_QUAD2f, 0 );
    soc_reg32_set( nUnit, PT_IPTE_MAC_RESETr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );

    for ( nPortIndex = 8; nPortIndex < 12; ++nPortIndex ) {
      soc_reg32_get( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, &uReg32 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_VALUEf, 4 );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATE_PORTf, nPortIndex );
      soc_reg_field_set( nUnit, PT_IPTE_PORT_RECFGr, &uReg32, CREDIT_UPDATEf, 1 );
      soc_reg32_set( nUnit, PT_IPTE_PORT_RECFGr, SOC_BLOCK_PORT(nUnit,C3HPPC_SWS_LINE_INTERFACE), 0, uReg32 );
    }
  }


  /* 
     The number of active queues will be cut in half, so adjusting the reserve page number accordingly.
  */
  READ_QM_BUFFER_CONFIG7r( nUnit, &uReg32 );
  uReg32 /= 2;
  WRITE_QM_BUFFER_CONFIG7r( nUnit, uReg32 );


  return 0;
#endif

}




#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
















/* Drop the incoming traffic on the queues associated with ports 8 through 11
  uint32 uLast, uQueue, uFieldValue, auTcamRamEntry[2];
  for ( nIndex = 0; nIndex < 256; ++nIndex ) {
    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm,
                                      PR_BLOCK(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), nIndex, auTcamRamEntry) );
    soc_mem_field_get( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, LASTf, &uLast );
    soc_mem_field_get( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, QUEUEf, &uQueue );
    if ( uLast && uQueue >= 8 && uQueue <= 11 ) {
      uFieldValue = 1;
      soc_mem_field_set( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm, auTcamRamEntry, DROPf, &uFieldValue );
      SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm,
                                         PR_BLOCK(nUnit,C3HPPC_SWS_FABRIC_INTERFACE), nIndex, auTcamRamEntry) );
      uQueue += C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM;
      cli_out("\n Dropping traffic on fabric queue --> %d \n", uQueue );
    }
  }
*/

/*
  nPortIndex = 0;
  for ( nPortBlk = 0; nPortBlk < 4; ++nPortBlk ) {
    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_SOFT_RESETr, nPortIndex, 0, &uReg32 );
    cli_out("\n PORT_SOFT_RESET[%d] -->  0x%08x  \n", nPortBlk, uReg32 );

    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_ENABLE_REGr, nPortIndex, 0, &uReg32 );
    cli_out("\n PORT_ENABLE_REG[%d] -->  0x%08x  \n", nPortBlk, uReg32 );

    c3hppc_sws_soc_reg32_get( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, &uReg32 );
    cli_out("\n PORT_MAC_CONTROL[%d] -->  0x%08x  \n", nPortBlk, uReg32 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC0_RESETf, 0 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC1_RESETf, 0 );
    soc_reg_field_set( nUnit, PORT_MAC_CONTROLr, &uReg32, XMAC2_RESETf, 0 );
    c3hppc_sws_soc_reg32_set( nUnit, g_anPortBlkNum[nPortBlk], PORT_MAC_CONTROLr, nPortIndex, 0, uReg32 );
  }
   
  for ( nPortBlk = 0; nPortBlk < 4; ++nPortBlk ) {
    for ( nPortIndex = 0; nPortIndex < 12; ++nPortIndex ) {
      c3hppc_sws_soc_reg64_get( nUnit, g_anPortBlkNum[nPortBlk], XMAC_CTRLr, nPortIndex, 0, &uReg64 );
      cli_out("\n XMAC_CTRLr[%d,%d] -->  0x%llx  \n", nPortBlk, nPortIndex, uReg64 );
    }
  }
*/

/* Force the queues associated with ports 8 through 11 to age in order to free the pages
  for ( uQueue = (C3HPPC_SWS_SOURCE_QUEUE_NUM+8);
        uQueue < (C3HPPC_SWS_SOURCE_QUEUE_NUM+12);
        ++uQueue ) {
    READ_QM_AGER_DEBUGr( nUnit, &uReg32 );
    soc_reg_field_set( nUnit, QM_AGER_DEBUGr, &uReg32, FORCE_AGED_QUEUEf, uQueue );
    soc_reg_field_set( nUnit, QM_AGER_DEBUGr, &uReg32, FORCE_AGED_ACKf, 1 );
    soc_reg_field_set( nUnit, QM_AGER_DEBUGr, &uReg32, FORCE_AGED_ENf, 1 );
    WRITE_QM_AGER_DEBUGr( nUnit, uReg32 );
    if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, QM_AGER_DEBUGr, FORCE_AGED_ACKf, 1, 10, 0, &TimeStamp ) ) {
      cli_out("\n QM aging of destination queue %d TIMED OUT! \n", uQueue );
    }
  }
*/
