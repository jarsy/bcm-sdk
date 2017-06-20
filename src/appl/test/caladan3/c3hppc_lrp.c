/*
 * $Id: c3hppc_lrp.c,v 1.36 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_lrp.c
 * Purpose: Caladan3 LRP test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>



#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_lrp.h>

static int g_naBank0InstMemTables[] = {LRA_INST_B0_MEM0m, LRA_INST_B0_MEM0m,
                                       LRA_INST_B0_MEM1m, LRA_INST_B0_MEM1m,
                                       LRA_INST_B0_MEM2m, LRA_INST_B0_MEM2m,
                                       LRA_INST_B0_MEM3m, LRA_INST_B0_MEM3m,
                                       LRA_INST_B0_MEM4m, LRA_INST_B0_MEM4m,
                                       LRA_INST_B0_MEM5m, LRA_INST_B0_MEM5m,
                                       LRA_INST_DEBUGm};
static int g_naBank1InstMemTables[] = {LRA_INST_B1_MEM0m, LRA_INST_B1_MEM0m,
                                       LRA_INST_B1_MEM1m, LRA_INST_B1_MEM1m,
                                       LRA_INST_B1_MEM2m, LRA_INST_B1_MEM2m,
                                       LRA_INST_B1_MEM3m, LRA_INST_B1_MEM3m,
                                       LRA_INST_B1_MEM4m, LRA_INST_B1_MEM4m,
                                       LRA_INST_B1_MEM5m, LRA_INST_B1_MEM5m,
                                       LRA_INST_DEBUGm};
#ifndef NO_FILEIO
static int g_nTaskMapTable = LRA_INST_TASK_MAPm;
#endif

static int g_anErrorRegisters[] = { LRA_ERRORr,
                                    LRB_ERRORr,
                                    LRA_SER_EVENTr,
                                    LRB_SER_EVENTr,
                                    LRA_PAR_ERR_STATUSr,
                                    LRA_ECC_ERR_CORR_STATUSr,
                                    LRA_ECC_ERR_UCOR_STATUSr,
                                    LRB_ECC_ERR_CORR_STATUSr,
                                    LRB_ECC_ERR_UCOR_STATUSr
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { LRA_ERROR_MASKr,
                                        LRB_ERROR_MASKr,
                                        LRA_SER_EVENT_MASKr,
                                        LRB_SER_EVENT_MASKr
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);

static sal_time_t  g_StartTimeStamp;


int c3hppc_lrp_hw_init( int nUnit, c3hppc_lrp_control_info_t *pC3LrpControlInfo ) {

  uint32 uRegisterValue, uStreamsOnline, uSVPlatency;
  int nAsmFileEpochLength = 0, nSVP;
  sal_time_t TimeStamp;

  /*
   * Default SVP semaphores to 128 corresponding to TMU Update FIFO size
  */
  for ( nSVP = 0; nSVP < 5; ++nSVP ) {
    READ_LRB_SVP_SEM_CONFIGr( nUnit, nSVP, &uRegisterValue ); 
    soc_reg_field_set( nUnit, LRB_SVP_SEM_CONFIGr, &uRegisterValue, INIT_VALUEf, 128 ); 
    WRITE_LRB_SVP_SEM_CONFIGr( nUnit, nSVP, uRegisterValue ); 
  }

  /*
   * Take block out of soft reset via LRP
  */
  READ_LRA_CONFIG0r( nUnit, &uRegisterValue ); 
  soc_reg_field_set( nUnit, LRA_CONFIG0r, &uRegisterValue, SOFT_RESET_Nf, 1 ); 
  WRITE_LRA_CONFIG0r( nUnit, uRegisterValue );
  READ_LRB_CONFIG0r( nUnit, &uRegisterValue ); 
  soc_reg_field_set( nUnit, LRB_CONFIG0r, &uRegisterValue, SOFT_RESET_Nf, 1 ); 
  WRITE_LRB_CONFIG0r( nUnit, uRegisterValue );

  WRITE_LRB_RESULT_RAM_INITr( nUnit, 0xff );

  c3hppc_lrp_load_ucode( nUnit,
                         pC3LrpControlInfo->sUcodeFileName,
                         pC3LrpControlInfo->nBankSelect,
                         pC3LrpControlInfo->bLoaderEnable,
                         pC3LrpControlInfo->bMaximizeActiveContexts,
                         &nAsmFileEpochLength );
/* Not sure what the C3HPPC_LRP__MIN_EPOCH value really is with the loader so commenting out the check.
  assert( nAsmFileEpochLength >= ((pC3LrpControlInfo->bLoaderEnable == 1) ? C3HPPC_LRP__MIN_EPOCH : 64) );
*/

  pC3LrpControlInfo->nEpochLength = ( !pC3LrpControlInfo->nEpochLength ) ? nAsmFileEpochLength :
                                                                           pC3LrpControlInfo->nEpochLength;
  if ( pC3LrpControlInfo->bLoaderEnable == 1 ) {
    pC3LrpControlInfo->nEpochLength = C3HPPC_MAX( C3HPPC_LRP__MIN_EPOCH, pC3LrpControlInfo->nEpochLength );
  }

  READ_LRA_CONFIG1r( nUnit, &uRegisterValue ); 
  soc_reg_field_set( nUnit, LRA_CONFIG1r, &uRegisterValue, EPOCH_LENGTHf, (pC3LrpControlInfo->nEpochLength - 17) ); 
  soc_reg_field_set( nUnit, LRA_CONFIG1r, &uRegisterValue, FRAMES_PER_CONTEXTf, 2*pC3LrpControlInfo->nNumberOfActivePEs ); 
  soc_reg_field_set( nUnit, LRA_CONFIG1r, &uRegisterValue, DUPLEXf, (int) pC3LrpControlInfo->bDuplex ); 
  soc_reg_field_set( nUnit, LRA_CONFIG1r, &uRegisterValue, UPDATEf, 1 ); 
  WRITE_LRA_CONFIG1r( nUnit, uRegisterValue );

  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, LRA_EVENTr, UPDATEf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_lrp_hw_init> -- LRA_EVENT UPDATE event TIMEOUT!!!\n");
    return -1;
  }

  if ( !pC3LrpControlInfo->bLoaderEnable ) {
    uRegisterValue = 0;
    soc_reg_field_set( nUnit, LRA_DEBUGr, &uRegisterValue, STREAM_BYPASSf, 1 ); 
    WRITE_LRA_DEBUGr( nUnit, uRegisterValue );
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRA_EPOCH_CONTROLr, &uRegisterValue, ENABLEf, 1 );
  WRITE_LRA_EPOCH_CONTROLr( nUnit, uRegisterValue );

  READ_LRA_CONFIG0r( nUnit, &uRegisterValue ); 
  soc_reg_field_set( nUnit, LRA_CONFIG0r, &uRegisterValue, LOAD_ENABLEf, (uint32) pC3LrpControlInfo->bLoaderEnable ); 
  soc_reg_field_set( nUnit, LRA_CONFIG0r, &uRegisterValue, BYPASSf, (uint32) pC3LrpControlInfo->bBypass ); 
  soc_reg_field_set( nUnit, LRA_CONFIG0r, &uRegisterValue, ENABLEf, 1 );
  WRITE_LRA_CONFIG0r( nUnit, uRegisterValue );

  READ_LRB_DEBUGr( nUnit, &uRegisterValue );
  uSVPlatency = soc_reg_field_get( nUnit, LRB_DEBUGr, uRegisterValue, SVP_LATENCYf ); 
  uSVPlatency = C3HPPC_MAX(15, uSVPlatency);
  soc_reg_field_set( nUnit, LRB_DEBUGr, &uRegisterValue, SVP_LATENCYf, uSVPlatency );
  WRITE_LRB_DEBUGr( nUnit, uRegisterValue );

  
  READ_LRA_CONFIG0r( nUnit, &uRegisterValue );
  uStreamsOnline = soc_reg_field_get( nUnit, LRA_CONFIG0r, uRegisterValue, ONLINEf ); 
  if ( uStreamsOnline ) {
    if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, LRA_EVENTr, ONLINEf, uStreamsOnline, 100, 1, &TimeStamp ) ) {
      cli_out("<c3hppc_lrp_hw_init> -- LRA_EVENT ONLINE event TIMEOUT!!!\n");
      return -1;
    }
  }

  return 0;
}



int c3hppc_lrp_bank_swap( int nUnit ) {
  uint32 uRegisterValue, uOffline;
  sal_time_t TimeStamp;

  READ_LRA_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, LRA_CONFIG0r, &uRegisterValue, BANK_SWAPf, 1 );
  WRITE_LRA_CONFIG0r( nUnit, uRegisterValue );

  uOffline = 0;
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, LRA_EVENTr, BANK_SWAP_DONEf, 1, (SAL_BOOT_QUICKTURN ? 100 : 1), 1, &TimeStamp ) ) {
    READ_LRA_EVENTr( nUnit, &uRegisterValue );
    uOffline = soc_reg_field_get( nUnit, LRA_EVENTr, uRegisterValue, OFFLINEf ); 
    if ( !uOffline ) {
      cli_out("<c3hppc_lrp_bank_swap> -- LRA_EVENT \"BANK_SWAP_DONE\" event TIMEOUT!!!\n");
      return -1;
    }
  }
  if ( !uOffline ) {
    cli_out("<c3hppc_lrp_bank_swap> -- LRA_EVENT \"BANK_SWAP_DONE\" event detected!\n");
  }

  return 0;
}



int c3hppc_lrp_set_results_timer( int nUnit, uint32 uTimer ) {

  WRITE_LRA_RESULT_TIMERr( nUnit, uTimer );

  return 0;
}



int c3hppc_lrp_hw_cleanup( int nUnit ) {

  int nIndex;

  for ( nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0, 
                              ((nIndex < g_nErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nErrorMaskRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0, 0x00000000 ); 
  }

  return 0;
}



int c3hppc_lrp_display_error_state( int nUnit ) {

  int rc, nIndex;

  for ( rc = 0, nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anErrorRegisters[nIndex] );
  }

  return rc;
}



int c3hppc_lrp_write_shared_register( int nUnit, int nRegIndex, uint32 uRegData ) {

  uint32 uRegisterValue;

  WRITE_LRB_SHARED_DATAr( nUnit, uRegData);
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, ACCf, 1 );
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, RW_Nf, 0 );
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, REG_IDf, (uint32) nRegIndex );
  WRITE_LRB_SHARED_DATA_ACCr( nUnit, uRegisterValue );

  return 0;
}



uint32 c3hppc_lrp_read_shared_register( int nUnit, int nRegIndex ) {

  uint32 uRegisterValue;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, ACCf, 1 );
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, RW_Nf, 1 );
  soc_reg_field_set( nUnit, LRB_SHARED_DATA_ACCr, &uRegisterValue, REG_IDf, (uint32) nRegIndex );
  WRITE_LRB_SHARED_DATA_ACCr( nUnit, uRegisterValue );

  READ_LRB_SHARED_DATAr( nUnit, &uRegisterValue);

  return uRegisterValue;
}



int c3hppc_lrp_start_control( int nUnit, uint64 uuNumberOfEpochsToRun ) {

  uint32 uRegisterValue, uFieldValue;
  uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;

  if ( COMPILER_64_NE(uuNumberOfEpochsToRun, continuous_epochs) ) {
    uRegisterValue = COMPILER_64_LO(uuNumberOfEpochsToRun);
    WRITE_LRA_EPOCH_COUNT0r( nUnit, uRegisterValue );
    uRegisterValue = COMPILER_64_HI(uuNumberOfEpochsToRun);
    WRITE_LRA_EPOCH_COUNT1r( nUnit, uRegisterValue );
  }

  uRegisterValue = 0;
  if ( COMPILER_64_EQ(uuNumberOfEpochsToRun, continuous_epochs) ) {
    uFieldValue = 0; 
  } else {
    uFieldValue = 1; 
  }
  soc_reg_field_set( nUnit, LRA_EPOCH_CONTROLr, &uRegisterValue, ENABLEf, uFieldValue );
  soc_reg_field_set( nUnit, LRA_EPOCH_CONTROLr, &uRegisterValue, STARTf, 1 );

  WRITE_LRA_EPOCH_CONTROLr( nUnit, uRegisterValue );
  g_StartTimeStamp = sal_time();
  if ( COMPILER_64_NE(uuNumberOfEpochsToRun, continuous_epochs) ) {
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)
    if (soc_property_get(nUnit, spn_LRP_BYPASS,0)) {
    cli_out("\nIssued LRP \"start\" command to run %x%08x epochs at start time %d!\n",
            COMPILER_64_HI(uuNumberOfEpochsToRun), 
            COMPILER_64_LO(uuNumberOfEpochsToRun),(int) g_StartTimeStamp);
    }
#else
    if (soc_property_get(nUnit, spn_LRP_BYPASS,0)) {
    cli_out("\nIssued LRP \"start\" command to run %lld epochs at start time %d!\n",
            uuNumberOfEpochsToRun, (int) g_StartTimeStamp);
    }
#endif
  } else {
    if (soc_property_get(nUnit, spn_LRP_BYPASS,0)) {
    cli_out("\nIssued LRP \"start\" command to run in continuous mode!\n");
    }
  }


  return 0;
}

sal_time_t  c3hppc_lrp_get_start_timestamp( void ) {
  return g_StartTimeStamp;
}



int c3hppc_lrp_setup_dm_segment_table( int nUnit, int nSegment, int nDm, uint32 uDmLookUp ) {

  uint32 uMemEntry;
  int nField;

  nField = 0;

  if ( nDm >= 0 && nDm < 3 ) {

    switch ( nDm ) {
      case 0:    nField = DM0f; break;
      case 1:    nField = DM1f; break;
      case 2:    nField = DM2f; break;
    }

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, LRB_DM_SEGMENT_TABLEm,
                                      MEM_BLOCK_ANY, nSegment, &uMemEntry) );
    soc_mem_field_set( nUnit, LRB_DM_SEGMENT_TABLEm,
                       &uMemEntry, nField, &uDmLookUp );
    SOC_IF_ERROR_RETURN(soc_mem_write(nUnit, LRB_DM_SEGMENT_TABLEm,
                                      MEM_BLOCK_ANY, nSegment, (void *) &uMemEntry) );
  }

  return 0;
}



int c3hppc_lrp_setup_tmu_program( int nUnit, int nTableIndex, uint32 uProgram, uint8 bSubKey0Valid,
                                  uint8 bSubKey1Valid ) {
  uint32 uMemEntry, uMemFieldValue;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, &uMemEntry) );
  uMemFieldValue =  ( bSubKey0Valid ) ? 1 : 0;
  uMemFieldValue |= ( bSubKey1Valid ) ? 2 : 0;
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, TMUf, &uMemFieldValue );
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, TPROGf, &uProgram );
  if ( bSubKey0Valid == 0 && bSubKey1Valid == 0 ) {
    uMemFieldValue = 1; 
    soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                       &uMemEntry, TMU_UPDATEf, &uMemFieldValue );
    soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, TMUf, &uMemFieldValue );
  }
  SOC_IF_ERROR_RETURN(soc_mem_write(nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, (void *) &uMemEntry) );

  return 0;
}


int c3hppc_lrp_setup_rce_program( int nUnit, int nTableIndex, uint32 uProgram ) {

  uint32 uMemEntry, uMemFieldValue;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, &uMemEntry) );
  uMemFieldValue = 1;
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, RCEf, &uMemFieldValue );
  uMemFieldValue = uProgram;
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, RPROGf, &uMemFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write(nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, (void *) &uMemEntry) );

  return 0;
}


int c3hppc_lrp_setup_etu_program( int nUnit, int nTableIndex, uint32 uProgram ) {

  uint32 uMemEntry, uMemFieldValue;

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, &uMemEntry) );
  uMemFieldValue = 1;
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, ETUf, &uMemFieldValue );
  uMemFieldValue = uProgram;
  soc_mem_field_set( nUnit, LRB_PROGRAM_TRANSLATIONm,
                     &uMemEntry, EPROGf, &uMemFieldValue );
  SOC_IF_ERROR_RETURN(soc_mem_write(nUnit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, nTableIndex, (void *) &uMemEntry) );

  return 0;
}


int c3hppc_lrp_bank_corrupt( int nUnit, int nBankSelect ) {

  stInstMemEntry_t *paInstMem;
  int nMemory, rc, nStream;

  paInstMem = (stInstMemEntry_t *) soc_cm_salloc(nUnit,
                                                 (soc_mem_index_max(nUnit, g_naBank0InstMemTables[0]) + 1) * sizeof(stInstMemEntry_t),
                                                 "imem_buffer");
  sal_memset( paInstMem, 0xff, ((soc_mem_index_max(nUnit, g_naBank0InstMemTables[0]) + 1) * sizeof(stInstMemEntry_t)) );

  for ( nStream = 0; nStream < (C3HPPC_LRP__STREAM_NUM / 2); ++nStream ) {
    nMemory = ( nBankSelect == 0 ) ? g_naBank0InstMemTables[nStream<<1] : g_naBank1InstMemTables[nStream<<1];

    /*    coverity[negative_returns : FALSE]    */
    rc = soc_mem_write_range(nUnit, nMemory, MEM_BLOCK_ANY, 0, 
                             soc_mem_index_max(nUnit, nMemory),
                             (void *) paInstMem[0].uaInstMemEntry);
    if ( rc ) {
        cli_out("soc_mem_write_range rc --> %d\n", rc);
    }
  }

  soc_cm_sfree(nUnit,paInstMem);

  return rc;
}


int c3hppc_lrp_setup_host_bubble( int nUnit, int nStream, int nTask, uint32 *uBubbleData ) {

  uint32 uRegisterValue;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRA_BUBBLEr, &uRegisterValue, INGRESSf, ( (nTask == 0) ? 1 : 0) );
  soc_reg_field_set( nUnit, LRA_BUBBLEr, &uRegisterValue, EGRESSf, ( (nTask == 1) ? 1 : 0) );
  soc_reg_field_set( nUnit, LRA_BUBBLEr, &uRegisterValue, STRf, (uint32) nStream );
  soc_reg_field_set( nUnit, LRA_BUBBLEr, &uRegisterValue, IDf, 0xbbbb );
  WRITE_LRA_BUBBLEr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_lrp_setup_pseudo_traffic_bubbles( int nUnit, uint8 bRandomizeStream, int nStream, int nTask ) {

  uint32 uRegisterValue, uBubbleTableSize, u512nsInterval;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  c3hppc_lrp_bubble_state_entry_ut BubbleState;
  int nOcmPort, nBubble, nIntervalIndex;
  uint32 auBubbleIntervalTableEntry[1];


  uBubbleTableSize = 1024;
  nIntervalIndex = 64;
  auBubbleIntervalTableEntry[0] = 0;
  u512nsInterval = 4;

  COMPILER_64_ZERO(BubbleState.value);
  BubbleState.bits.Init = 1;
  BubbleState.bits.Mode = C3HPPC_LRP_BUBBLE_UPDATE_MODE__CONTINUOUS;
  BubbleState.bits.IntervalIndex = nIntervalIndex;

  /* Initialize bubble table entries  */
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(nUnit,
                                                                uBubbleTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  for ( nBubble = 0; nBubble < (int) uBubbleTableSize; ++nBubble ) {
    BubbleState.bits.Stream = ( bRandomizeStream ) ? (sal_rand() % nStream) : nStream;
    BubbleState.bits.Task = ( nTask == C3HPPC_LRP__TASK_NUM ) ? (nBubble & 1) : nTask;
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

int c3hppc_lrp_disable_bubbles( int nUnit ) {
  uint32 uRegisterValue;

  READ_LRB_BUBBLE_TABLE_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uRegisterValue, ENABLEf, 0 );
  WRITE_LRB_BUBBLE_TABLE_CONFIGr( nUnit, uRegisterValue );

  return 0;
}

int c3hppc_lrp_enable_bubbles( int nUnit ) {
  uint32 uRegisterValue;

  READ_LRB_BUBBLE_TABLE_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uRegisterValue, ENABLEf, 1 );
  WRITE_LRB_BUBBLE_TABLE_CONFIGr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_lrp_setup_ring_wheel( int nUnit, int nSVP, int nLM, int nRingSize, int nEntrySize ) {
  uint32 uRegisterValue;
  int LMindex;

  LMindex = (4 * nSVP) + nLM;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG3_REGr, &uRegisterValue, READ_OFFSETf, 1 );
  WRITE_LRB_LIST_CONFIG3_REGr( nUnit, LMindex, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG4_REGr, &uRegisterValue, WRITE_OFFSETf, 0 );
  WRITE_LRB_LIST_CONFIG4_REGr( nUnit, LMindex, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENABLEf, 1 );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENABLE_UCODE_DEQf, 1 );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, SIZEf, (uint32) nEntrySize );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENTRIESf, (uint32) nRingSize );
  WRITE_LRB_LIST_CONFIG0_REGr( nUnit, LMindex, uRegisterValue );

  return 0;
}


int c3hppc_lrp_setup_host_producer_ring( int nUnit, int nSVP, int nLM, int nRingSize, int nEntrySize ) {
  uint32 uRegisterValue;
  int LMindex;

  LMindex = (4 * nSVP) + nLM;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG3_REGr, &uRegisterValue, READ_OFFSETf, 1 );
  WRITE_LRB_LIST_CONFIG3_REGr( nUnit, LMindex, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG4_REGr, &uRegisterValue, WRITE_OFFSETf, 1 );
  WRITE_LRB_LIST_CONFIG4_REGr( nUnit, LMindex, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENABLEf, 1 );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENABLE_UCODE_DEQf, 1 );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, SIZEf, (uint32) nEntrySize );
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG0_REGr, &uRegisterValue, ENTRIESf, (uint32) nRingSize );
  WRITE_LRB_LIST_CONFIG0_REGr( nUnit, LMindex, uRegisterValue );

  return 0;
}


int c3hppc_lrp_set_host_producer_ring_write_offset( int nUnit, int nSVP, int nLM, uint32 uOffset ) {
  uint32 uRegisterValue;
  int LMindex;

  LMindex = (4 * nSVP) + nLM;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, LRB_LIST_CONFIG4_REGr, &uRegisterValue, WRITE_OFFSETf, (uOffset + 1) );
  WRITE_LRB_LIST_CONFIG4_REGr( nUnit, LMindex, uRegisterValue );

  return 0;
}



int c3hppc_lrp_load_ucode(int nUnit, char *sUcodeFileName, int nBankSelect,
                          uint8 bLoaderEnable, uint8 bMaximizeActiveContexts, int *nAsmFileEpochLength) {

#ifndef NO_FILEIO
    /* defined(VXWORKS)*/

  stInstMemEntry_t *paInstMem;
   
  int nWordIndex, nRowIndex, rc;
  int nFileBufferSize, nPackageSize;
  char *psLine, *psLineEnd, *psTmp;
  void *pPackageBuffer;
  int nStream, nLineLen, nTask, nMaxTaskMemIndex;
  char sTmpBuf[9];
  uint32 uDataBuf, uContexts, uContextsOld, uSwitchMultipleSeen;
  uint32 uStreamsOnline, uStreamOnlineMask, uTaskMemEntry, uTaskMemEntryShift;
  int nOpenAttempts;
  int nIndex = 0;
  int nMemory;
  char sUcodeFile[256];
  char *psUcodeFile = sUcodeFile;
  FILE *pFileHandle;

  uTaskMemEntryShift = 0;
  nMaxTaskMemIndex = soc_mem_index_max( nUnit, g_nTaskMapTable );

  if ( (psTmp = sal_config_get("c3_ucode_path")) != NULL && sal_strlen(psTmp) < sizeof(sUcodeFile) ) {
    /* coverity[secure_coding] */
    sal_strcpy(sUcodeFile, psTmp );
    psUcodeFile = sUcodeFile + sal_strlen(sUcodeFile);
    /* coverity[secure_coding] */
    sal_strcpy(psUcodeFile, "/");
    psUcodeFile = sUcodeFile + sal_strlen(sUcodeFile);
    /* coverity[secure_coding] */
    sal_strcpy(psUcodeFile, sUcodeFileName);
    cli_out("\n\nLoading in microcode file --> %s\n\n", sUcodeFile);
  } else {
    cli_out("\"c3_ucode_path\" config variable NOT set or OVERSIZED!!!\n");
    return -1;
  }

  pFileHandle = NULL;
  nOpenAttempts = (SAL_BOOT_QUICKTURN ? 100 : 2);
  while ( (--nOpenAttempts) && pFileHandle == NULL ) {
    pFileHandle = sal_fopen(sUcodeFile, "r");
    if ( pFileHandle == NULL ) sal_sleep(1);
  }
  if ( pFileHandle == NULL ) {
    cli_out("Could not open file --> %s\n", sUcodeFile);
    return -1;
  }
  fseek(pFileHandle, 0, SEEK_END);
  nFileBufferSize = ftell(pFileHandle);
  if ( nFileBufferSize <= 0 ) {
    cli_out("ftell FAILED!!!\n");
    sal_fclose(pFileHandle);
    assert( nFileBufferSize > 0 );
    return -1;
  }
  rewind(pFileHandle);
  pPackageBuffer = (void *)sal_alloc(nFileBufferSize, "ucode_buffer");
  if ( pPackageBuffer == NULL ) {
    cli_out("ucode_buffer sal_alloc FAILED!!!\n");
    sal_fclose(pFileHandle);
    assert( pPackageBuffer != NULL ); 
    return -1;
  }
  /*    coverity[ +string_null_return ]    */
  /*    coverity[tainted_data_argument]    */
  /*    coverity[string_null_argument]     */
  nPackageSize = fread(pPackageBuffer, 1, nFileBufferSize, pFileHandle);
  if ( nPackageSize != nFileBufferSize ) {
    cli_out("fread of ucode file FAILED!!!\n");
    sal_fclose(pFileHandle);
    assert( nPackageSize == nFileBufferSize ); 
    /*    coverity[tainted_data]    */
    sal_free( pPackageBuffer );
    return -1;
  }
  sal_fclose(pFileHandle);


  nMemory = g_naBank0InstMemTables[0];
  paInstMem = (stInstMemEntry_t *) soc_cm_salloc(nUnit,
                                                 (soc_mem_index_max(nUnit, nMemory) + 1) * sizeof(stInstMemEntry_t),
                                                 "imem_buffer");
  sal_memset( paInstMem, 0, ((soc_mem_index_max(nUnit, nMemory) + 1) * sizeof(stInstMemEntry_t)) );
  nStream = 0xff;
  nTask = 0;
  psLine = (char *) pPackageBuffer;
  nRowIndex = 0;
  sTmpBuf[8] = '\0';
  uContexts = 3;
  uSwitchMultipleSeen = 0;
  uStreamOnlineMask = 0;
  while ( (psLineEnd = strchr(psLine, '\n')) != NULL ) {
    *psLineEnd = '\0';
    nLineLen = sal_strlen(psLine);

    if ( !nTask && strstr(psLine, "taskmap") ) {
      nTask = 1;
      uTaskMemEntry = 0;
      uTaskMemEntryShift = 0;
    }

    if ( nStream == 0xff && (psTmp = strstr(psLine, "stream")) ) {
      sscanf( (psTmp + sal_strlen("stream")), "%d", &nStream);
/*
      cli_out("  Processing instruction stream --> %d\n", nStream);
*/
      if ( nStream < C3HPPC_LRP__STREAM_NUM ) {
        uStreamOnlineMask |= (1 << nStream);
      }
    }

    if ( nStream != 0xff && nLineLen == 0 ) {
      if ( (nStream & 1) || strstr(psLineEnd+1, "stream") == NULL ) {
        nMemory = (nBankSelect == 0) ? g_naBank0InstMemTables[nStream] : g_naBank1InstMemTables[nStream];

        /*
         * When running with the loader disabled modify the inserted "switch 2"
         * instruction to be "switch 1" when the additional "switch" instruction count is
         * 0 or 2.
        */
        if ( bMaximizeActiveContexts && !bLoaderEnable && (uContexts == 3 || uContexts == 5) && uSwitchMultipleSeen == 0 ) {
          paInstMem[0].uaInstMemEntry[2] = 0xffc00000;
        }
  
#if 0
        for ( nRowIndex = 0; nRowIndex <= soc_mem_index_max(nUnit, nMemory); ++nRowIndex ) {
          rc = soc_mem_write(nUnit, nMemory, MEM_BLOCK_ANY, nRowIndex, (void *) paInstMem[nRowIndex].uaInstMemEntry);
          if ( rc ) {
              cli_out("soc_mem_write rc --> %d\n", rc);
          }
        }
#endif
  
        /*    coverity[negative_returns : FALSE]    */
        rc = soc_mem_write_range(nUnit, nMemory, MEM_BLOCK_ANY, 0, 
                                 soc_mem_index_max(nUnit, nMemory),
                                 (void *) paInstMem[0].uaInstMemEntry);
        if ( rc ) {
            cli_out("soc_mem_write_range rc --> %d\n", rc);
        }
      }
      nStream = 0xff;
      *nAsmFileEpochLength = nRowIndex;
      nRowIndex = 0;

    } else if ( nLineLen == 24 )  {

      nWordIndex = (nStream & 1) ? 5 : 2;
      for ( nIndex = 0; nIndex < 3; ++nIndex ) {
        /* coverity[secure_coding] */
        sal_strncpy(sTmpBuf, (psLine+(8*nIndex)), 8);
        sscanf(sTmpBuf, "%x", &uDataBuf);
        paInstMem[nRowIndex].uaInstMemEntry[nWordIndex] = uDataBuf; 
        --nWordIndex;
      }

      /*
       * Count the number of switch statements in the instruction stream so that
       * LRA_CONFIG1.CONTEXTS can be initialized.  The assembler automatically
       * inserts a "switch" for the first instruction.
       * 
      */
      if ( nStream == 0 && nRowIndex != 0 ) {
        if ( (paInstMem[nRowIndex].uaInstMemEntry[2] & 0xfc3fffff) == 0xfc000000 &&
             paInstMem[nRowIndex].uaInstMemEntry[1] == 0x00000000 &&
             paInstMem[nRowIndex].uaInstMemEntry[0] == 0x00000000 ) {
          uContextsOld = uContexts;
          uContexts += 0x10 - ((paInstMem[nRowIndex].uaInstMemEntry[2] >> 22) & 0xf);
          if ( uSwitchMultipleSeen == 0 ) {
            uSwitchMultipleSeen = ( (uContexts - uContextsOld) == 1 ) ? 0 : 1;
          }
        }
      } 

      ++nRowIndex;

    } else if ( nTask ) {

      if ( nLineLen == 2 ) {
        /* coverity[secure_coding] */
        sal_strncpy(sTmpBuf, psLine, 8);
        /* coverity[secure_coding] */
        sscanf(sTmpBuf, "%x", &uDataBuf);
        uTaskMemEntry |= (uDataBuf & 0xff) << uTaskMemEntryShift;
        uTaskMemEntryShift += 8;
      }

      if ( uTaskMemEntryShift == 32 || nLineLen == 0 ) {
        rc = soc_mem_write(nUnit, g_nTaskMapTable, MEM_BLOCK_ANY, nRowIndex, (void *) &uTaskMemEntry);
        if ( rc ) {
            cli_out("soc_mem_write rc --> %d\n", rc);
        }
        ++nRowIndex;
        uTaskMemEntry = 0;
        uTaskMemEntryShift = 0;
      }

      if ( nRowIndex > nMaxTaskMemIndex || nLineLen == 0 ) {
        nTask = 0;
        nRowIndex = 0;
      } 
    }
    
    psLine = psLineEnd + 1;
  }

  if ( bMaximizeActiveContexts && !bLoaderEnable && uSwitchMultipleSeen == 0 ) {
    switch ( uContexts ) {
      case 3:    uContexts = 8; break;
      case 4:    uContexts = 8; break;
      case 5:    uContexts = 7; break;
      case 6:    uContexts = 6; break;
      case 7:    uContexts = 7; break;
      case 8:    uContexts = 8; break;
    }
  }
  
  READ_LRA_CONFIG0r( nUnit, &uDataBuf );
  uStreamsOnline = soc_reg_field_get( nUnit, LRA_CONFIG0r, uDataBuf, ONLINEf ); 
  if ( !uStreamsOnline ) {
    soc_reg_field_set( nUnit, LRA_CONFIG0r, &uDataBuf, ONLINEf, uStreamOnlineMask ); 
    WRITE_LRA_CONFIG0r( nUnit, uDataBuf );
  
    READ_LRA_CONFIG1r( nUnit, &uDataBuf );
    soc_reg_field_set( nUnit, LRA_CONFIG1r, &uDataBuf, CONTEXTSf, uContexts ); 
    WRITE_LRA_CONFIG1r( nUnit, uDataBuf );
  }

  /*    coverity[tainted_data]    */
  sal_free(pPackageBuffer);
  soc_cm_sfree(nUnit, paInstMem);

#endif    /* if defined(VXWORKS) */

  return 0;
}

#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
