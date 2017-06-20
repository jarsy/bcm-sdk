/*
 * $Id: c3hppc_tmu.c,v 1.73 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_tmu.c
 * Purpose: Caladan3 TMU test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_tmu.h>

extern int _soc_caladan3_mem_reset_and_init_after_shmoo_addr(int unit, int ci);

static uint64 C3HPPC_TMU_SKIP_RSP_COMPARE_EML_ROOT           = COMPILER_64_INIT(0xfaceface,0xfacefac0);
static uint64 C3HPPC_TMU_SKIP_RSP_COMPARE_EML_CHAIN          = COMPILER_64_INIT(0xfaceface,0xfacefac1);
#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static uint64 C3HPPC_TMU_SKIP_RSP_COMPARE                    = COMPILER_64_INIT(0xfaceface,0xfacefac0);
static uint64 C3HPPC_TMU_SKIP_RSP_COMPARE_MASK               = COMPILER_64_INIT(0xffffffff,0xfffffff0);
#endif

static int g_anTmErrorRegisters[] = { TMA_DIRECT_ERROR0r,
                                      TMA_DIRECT_ERROR1r,
                                      TMA_DIRECT_ERROR2r,
                                      TMA_DIRECT_ERROR3r,
                                      TMA_KEYPLODER_ERRORr,
                                      TMB_DISTRIBUTOR_SCHEDULER_ERRORr,
                                      TMB_DISTRIBUTOR_REFRESH_ERRORr,
                                      TMB_DISTRIBUTOR_INTERFACE_FIFO_ERRORr,
                                      TMB_DISTRIBUTOR_FUNNEL_FIFO_ERRORr,
                                      TMB_UPDATER_ERRORr,
                                      TMB_COMPLETION_ERROR0r,
                                      TMB_COMPLETION_ERROR1r,
                                      TMB_COMPLETION_PARITY_ERRORr
                                    };
static int g_nTmErrorRegistersCount = COUNTOF(g_anTmErrorRegisters);
static int g_anTmErrorMaskRegisters[] = { TMA_DIRECT_ERROR0_MASKr,
                                          TMA_DIRECT_ERROR1_MASKr,
                                          TMA_DIRECT_ERROR2_MASKr,
                                          TMA_DIRECT_ERROR3_MASKr,
                                          TMA_KEYPLODER_ERROR_MASKr,
                                          TMB_DISTRIBUTOR_SCHEDULER_ERROR_MASKr,
                                          TMB_DISTRIBUTOR_REFRESH_ERROR_MASKr,
                                          TMB_DISTRIBUTOR_INTERFACE_FIFO_ERROR_MASKr,
                                          TMB_DISTRIBUTOR_FUNNEL_FIFO_ERROR_MASKr,
                                          TMB_UPDATER_ERROR_MASKr,
                                          TMB_COMPLETION_ERROR0_MASKr,
                                          TMB_COMPLETION_ERROR1_MASKr,
                                          TMB_COMPLETION_PARITY_ERROR_MASKr
                                        };
static int g_nTmErrorMaskRegistersCount = COUNTOF(g_anTmErrorMaskRegisters);

static int g_anCiErrorRegisters[] = { CI_ERRORr
                                    };
static int g_nCiErrorRegistersCount = COUNTOF(g_anCiErrorRegisters);
static int g_anCiErrorMaskRegisters[] = { CI_ERROR_MASKr
                                        };
static int g_nCiErrorMaskRegistersCount = COUNTOF(g_anCiErrorMaskRegisters);

static int g_anTmTpErrorRegisters[] = { TP_GLOBAL_EVENTr,
                                        TP_ECC_ERRORr,
                                        TP_TCAM_SCAN_ERRORr,
                                        TP_ECC_STATUS0r,
                                        TP_ECC_STATUS1r,
                                        
                                      };
static int g_nTmTpErrorRegistersCount = COUNTOF(g_anTmTpErrorRegisters);
static int g_anTmTpErrorMaskRegisters[] = { TP_GLOBAL_EVENT_MASKr,
                                            TP_ECC_ERROR_MASKr,
                                            TP_TCAM_SCAN_ERROR_MASKr
                                          };
static int g_nTmTpErrorMaskRegistersCount = COUNTOF(g_anTmTpErrorMaskRegisters);

static int g_anTmQeErrorRegisters[] = { TM_QE_ERRORr,
                                        TM_QE_FIFO_OVERFLOW_ERRORr,
                                        TM_QE_FIFO_UNDERFLOW_ERRORr,
                                        TM_QE_FIFO_PARITY_ERRORr,
                                        TM_QE_ECC_DEBUGr
                                      };
static int g_nTmQeErrorRegistersCount = COUNTOF(g_anTmQeErrorRegisters);
static int g_anTmQeErrorMaskRegisters[] = { TM_QE_ERROR_MASKr,
                                            TM_QE_FIFO_OVERFLOW_ERROR_MASKr,
                                            TM_QE_FIFO_UNDERFLOW_ERROR_MASKr,
                                            TM_QE_FIFO_PARITY_ERROR_MASKr

                                          };
static int g_nTmQeErrorMaskRegistersCount = COUNTOF(g_anTmQeErrorMaskRegisters);




static int g_anTmPmMemories[] = {TMA_PM_HIST0m, TMA_PM_HIST1m,
                                 TMA_PM_HIST2m, TMA_PM_HIST3m,
                                 TMA_PM_HIST4m, TMA_PM_HIST5m,
                                 TMA_PM_HIST6m, TMA_PM_HIST7m};

static int g_anTmaHash0Memories[] = {TMA_HASH0_RANDTABLE0m, TMA_HASH0_RANDTABLE1m,
                                     TMA_HASH0_RANDTABLE2m, TMA_HASH0_RANDTABLE3m,
                                     TMA_HASH0_RANDTABLE4m, TMA_HASH0_RANDTABLE5m,
                                     TMA_HASH0_RANDTABLE6m, TMA_HASH0_RANDTABLE7m};
static int g_anTmaHash1Memories[] = {TMA_HASH1_RANDTABLE0m, TMA_HASH1_RANDTABLE1m,
                                     TMA_HASH1_RANDTABLE2m, TMA_HASH1_RANDTABLE3m,
                                     TMA_HASH1_RANDTABLE4m, TMA_HASH1_RANDTABLE5m,
                                     TMA_HASH1_RANDTABLE6m, TMA_HASH1_RANDTABLE7m};
static int g_anTmbHash0Memories[] = {TMB_HASH0_RANDTABLE0m, TMB_HASH0_RANDTABLE1m,
                                     TMB_HASH0_RANDTABLE2m, TMB_HASH0_RANDTABLE3m,
                                     TMB_HASH0_RANDTABLE4m, TMB_HASH0_RANDTABLE5m,
                                     TMB_HASH0_RANDTABLE6m, TMB_HASH0_RANDTABLE7m};

static int g_anCiDDRtestDataRegisters[] = {CI_DDR_TEST_DATA0r, CI_DDR_TEST_DATA1r,
                                           CI_DDR_TEST_DATA2r, CI_DDR_TEST_DATA3r,
                                           CI_DDR_TEST_DATA4r, CI_DDR_TEST_DATA5r,
                                           CI_DDR_TEST_DATA6r, CI_DDR_TEST_DATA7r};
static int g_anCiDDRtestAltDataRegisters[] = {CI_DDR_TEST_ALT_DATA0r, CI_DDR_TEST_ALT_DATA1r,
                                              CI_DDR_TEST_ALT_DATA2r, CI_DDR_TEST_ALT_DATA3r,
                                              CI_DDR_TEST_ALT_DATA4r, CI_DDR_TEST_ALT_DATA5r,
                                              CI_DDR_TEST_ALT_DATA6r, CI_DDR_TEST_ALT_DATA7r};
static int g_anCiMemAccDataRegisters[] = {CI_MEM_ACC_DATA0r, CI_MEM_ACC_DATA1r,
                                          CI_MEM_ACC_DATA2r, CI_MEM_ACC_DATA3r,
                                          CI_MEM_ACC_DATA4r, CI_MEM_ACC_DATA5r,
                                          CI_MEM_ACC_DATA6r, CI_MEM_ACC_DATA7r};
static int g_anCiDDRtestFailedDataRegisters[] = {CI_DDR_TEST_FAILED_DATA0r, CI_DDR_TEST_FAILED_DATA1r,
                                                 CI_DDR_TEST_FAILED_DATA2r, CI_DDR_TEST_FAILED_DATA3r,
                                                 CI_DDR_TEST_FAILED_DATA4r, CI_DDR_TEST_FAILED_DATA5r,
                                                 CI_DDR_TEST_FAILED_DATA6r, CI_DDR_TEST_FAILED_DATA7r};

static int g_anCmicDmaStateRegisters[] = { CMIC_CMC0_SCHAN_ACK_DATA_BEAT_COUNTr,
                                           CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr,
                                           CMIC_CMC0_FIFO_CH1_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH1_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH1_RD_DMA_STATr,
                                           CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr,
                                           CMIC_CMC0_FIFO_CH2_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH2_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH2_RD_DMA_STATr,
                                           CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr,
                                           CMIC_CMC0_FIFO_CH3_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH3_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEMr,
                                           CMIC_CMC0_FIFO_CH3_RD_DMA_STATr,
                                           CMIC_CMC0_DMA_STATr,
                                           CMIC_CMC0_CCM_DMA_STATr,
                                           CMIC_CMC0_IRQ_STAT0r,
                                           CMIC_CMC0_IRQ_STAT1r,
                                           CMIC_CMC0_IRQ_STAT2r,
                                           CMIC_CMC0_IRQ_STAT3r,
                                           CMIC_CMC0_IRQ_STAT4r,
                                           CMIC_CMC1_DMA_STATr,
                                           CMIC_CMC1_CCM_DMA_STATr,
                                           CMIC_CMC1_IRQ_STAT0r,
                                           CMIC_CMC1_IRQ_STAT1r,
                                           CMIC_CMC1_IRQ_STAT2r,
                                           CMIC_CMC1_IRQ_STAT3r,
                                           CMIC_CMC1_IRQ_STAT4r,
                                           CMIC_CMC2_DMA_STATr,
                                           CMIC_CMC2_CCM_DMA_STATr,
                                           CMIC_CMC2_IRQ_STAT0r,
                                           CMIC_CMC2_IRQ_STAT1r,
                                           CMIC_CMC2_IRQ_STAT2r,
                                           CMIC_CMC2_IRQ_STAT3r,
                                           CMIC_CMC2_IRQ_STAT4r
                                         };
static int g_anCmicDmaStateRegistersCount = COUNTOF(g_anCmicDmaStateRegisters);


static sal_thread_t g_TmuUpdateCmdManagerThreadID = NULL;
static sal_thread_t g_TmuUpdateRspManagerThreadID = NULL;
static sal_thread_t g_TmuUpdateFreeListManagerThreadID = NULL;
static c3hppc_tmu_update_manager_cb_t g_c3hppcTmuUpdateManagerCB;

static uint32 g_auHashTables[C3HPPC_TMU_HASH_TABLE_NUM][C3HPPC_TMU_HASH_TABLE_ENTRY_NUM];

static uint64 g_auuPmMemContents[C3HPPC_TMU_PM_MEMORY_ROW_NUM];
static uint64 g_uuCacheMissCount, g_uuCacheHitCount, g_uuCacheHitPendingCount;

static uint64 g_auuHashAdjustValues[C3HPPC_TMU_HASH_ADJUST_SELECT_NUM];

static uint64 g_uuQeHashAdjustValue;

static uint32 g_uQeEMC128Mode = 0;

static uint32 g_uNumberOfCIs = C3HPPC_TMU_CI_INSTANCE_NUM;
static uint32 g_uNumberOfQEs = C3HPPC_TMU_QE_INSTANCE_NUM;

static uint32 g_uEmlMaxProvisionedKey = 0x10000;

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;

int c3hppc_tmu_hw_init( int nUnit, c3hppc_tmu_control_info_t *pC3TmuControlInfo ) {

  uint32 uRegisterValue;
  c3hppc_tmu_ci_control_info_t  c3hppcTmuCiControlInfo;
  int nQE, nCmdFifo, nTP, rc;
  int nHashTableIndex, nHashTable, nHashTableSize, nHashTableEntrySizeIn32b;
  uint32 *puHashTable, uIndex, uRandNum;

  soc_cm_get_id( nUnit, &g_dev_id, &g_rev_id);

  sal_memset( g_auuHashAdjustValues, 0x00, sizeof(g_auuHashAdjustValues) );
  COMPILER_64_SET(g_uuQeHashAdjustValue,0,1);
  g_uNumberOfCIs = pC3TmuControlInfo->uNumberOfCIs;
  g_uNumberOfQEs = pC3TmuControlInfo->uNumberOfCIs;
  rc = 0;

  /*
   * Take blocks out of soft reset
  */
  READ_TMA_CONTROLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_CONTROLr, &uRegisterValue, TMA_SOFT_RESET_Nf, 1 );
  WRITE_TMA_CONTROLr( nUnit, uRegisterValue );

  READ_TMB_CONTROLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_CONTROLr, &uRegisterValue, TMB_SOFT_RESET_Nf, 1 );
  WRITE_TMB_CONTROLr( nUnit, uRegisterValue );



  READ_TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &uRegisterValue, TRC_COST_WEIGHTf, 0 );
  soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &uRegisterValue, TFAW_COST_WEIGHTf, 0 );
  soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &uRegisterValue, TREF_COST_WEIGHTf, 0 );
  soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &uRegisterValue, TWR_COST_WEIGHTf, 0 );
  WRITE_TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr( nUnit, uRegisterValue );
  


  c3hppcTmuCiControlInfo.bSkipDramSelfTest = pC3TmuControlInfo->bSkipCiDramSelfTest;
  c3hppcTmuCiControlInfo.nDramFreq = pC3TmuControlInfo->nDramFreq;
  if ( c3hppc_tmu_ci_hw_init( nUnit, &c3hppcTmuCiControlInfo ) ) {
    return 1;
  }

  if ( pC3TmuControlInfo->bHwEmlChainManagement && pC3TmuControlInfo->bSkipCiDramInit == 0 ) {
    c3hppc_tmu_ci_memory_init( nUnit, &c3hppcTmuCiControlInfo );
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_CONFIGr, &uRegisterValue, REGION_MAPPER_INITf, 1 );
  WRITE_TMB_DISTRIBUTOR_CONFIGr( nUnit, uRegisterValue );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, TMB_UPDATER_LR_EML_CONFIGr, &uRegisterValue, SOFTWARE_LOCK_WAITf, 1 );
  WRITE_TMB_UPDATER_LR_EML_CONFIGr( nUnit, uRegisterValue );

  if ( pC3TmuControlInfo->bBypassScrambler ) {

    READ_TMB_DISTRIBUTOR_DEBUGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_DEBUGr, &uRegisterValue, BYPASS_SCRAMBLERf, 1 );
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_DEBUGr, &uRegisterValue, BYPASS_SCRAMBLER_CRCf, 1 );
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_DEBUGr, &uRegisterValue, CONFIG_RAND_LB_MODEf, 1 );
    WRITE_TMB_DISTRIBUTOR_DEBUGr( nUnit, uRegisterValue );

  } else {

    c3hppc_tmu_init_scrambler_table( nUnit, TMB_DISTRIBUTOR_SCRAMBLE_TABLE0m );
    c3hppc_tmu_init_scrambler_table( nUnit, TMB_DISTRIBUTOR_SCRAMBLE_TABLE1m );
    c3hppc_tmu_init_scrambler_table( nUnit, TMB_DISTRIBUTOR_SCRAMBLE_TABLE2m );

    for ( uIndex = 0; uIndex < 64; ++uIndex ) {
      uRandNum = sal_rand();
      WRITE_TMB_GLOBAL_TABLE_SCRAMBLER_CONFIGr( nUnit, (int) uIndex, uRandNum );
    }

  }

  if ( pC3TmuControlInfo->bBypassHash ) {
    READ_TMA_HASH_BYPASS_DEBUGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, TMA_HASH_BYPASS_DEBUGr, &uRegisterValue, BYPASS_HASHf, 1 );
    WRITE_TMA_HASH_BYPASS_DEBUGr( nUnit, uRegisterValue );
    READ_TMB_HASH_BYPASS_DEBUGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, TMB_HASH_BYPASS_DEBUGr, &uRegisterValue, BYPASS_HASHf, 1 );
    WRITE_TMB_HASH_BYPASS_DEBUGr( nUnit, uRegisterValue );
  } else {

    nHashTableSize = soc_mem_index_max( nUnit, g_anTmbHash0Memories[0] ) + 1;
    nHashTableEntrySizeIn32b = soc_mem_entry_words( nUnit, g_anTmbHash0Memories[0] );
    puHashTable = (uint32 *) soc_cm_salloc( nUnit, (nHashTableSize * nHashTableEntrySizeIn32b * sizeof(uint32)), "hash table" ); 
    sal_memset( puHashTable, 0x00, (nHashTableSize * nHashTableEntrySizeIn32b * sizeof(uint32)) );

    for ( nHashTable = 0; nHashTable < COUNTOF(g_anTmbHash0Memories); ++nHashTable ) {
      for ( nHashTableIndex = 0; nHashTableIndex < nHashTableSize; ++nHashTableIndex ) {
        g_auHashTables[nHashTable][nHashTableIndex] = sal_rand();
        soc_mem_field_set( nUnit, g_anTmbHash0Memories[0], (puHashTable + (nHashTableEntrySizeIn32b*nHashTableIndex)),
                           RAND_NUMf, &(g_auHashTables[nHashTable][nHashTableIndex]) );
      }
      /*    coverity[negative_returns : FALSE]    */
      rc += soc_mem_write_range( nUnit, g_anTmbHash0Memories[nHashTable], MEM_BLOCK_ANY, 0,
                                 (nHashTableSize - 1), (void *) puHashTable);
      /*    coverity[negative_returns : FALSE]    */
      rc += soc_mem_write_range( nUnit, g_anTmaHash0Memories[nHashTable], MEM_BLOCK_ANY, 0,
                                 (nHashTableSize - 1), (void *) puHashTable);
      /*    coverity[negative_returns : FALSE]    */
      rc += soc_mem_write_range( nUnit, g_anTmaHash1Memories[nHashTable], MEM_BLOCK_ANY, 0,
                                 (nHashTableSize - 1), (void *) puHashTable);
    }
    soc_cm_sfree( nUnit, puHashTable );

  }

  
  for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
    soc_reg32_get( nUnit, TP_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nTP), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, TP_GLOBAL_CONFIGr, &uRegisterValue, MEM_INITf, 1 );
    soc_reg32_set( nUnit, TP_GLOBAL_CONFIGr, SOC_BLOCK_PORT(nUnit,nTP), 0, uRegisterValue);
  }

  g_uQeEMC128Mode = (uint32) pC3TmuControlInfo->bEMC128Mode;
  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, TM_QE_CONFIGr, &uRegisterValue, CACHE_ENABLEf, (uint32) pC3TmuControlInfo->bCacheEnable );
    soc_reg_field_set( nUnit, TM_QE_CONFIGr, &uRegisterValue, EMC_128MODEf, g_uQeEMC128Mode );
    soc_reg32_set( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, uRegisterValue);
    if ( pC3TmuControlInfo->bBypassHash ) {
      soc_reg32_get( nUnit, TM_QE_DEBUGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, TM_QE_DEBUGr, &uRegisterValue, BYPASS_CRC32Cf, 1 );
      soc_reg32_set( nUnit, TM_QE_DEBUGr, SOC_BLOCK_PORT(nUnit,nQE), 0, uRegisterValue);
    }
  }

  
  for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
    soc_reg32_get( nUnit, TP_TCAM_SCAN_DEBUG1r, SOC_BLOCK_PORT(nUnit,nTP), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, TP_TCAM_SCAN_DEBUG1r, &uRegisterValue, TCAM_SCAN_WINDOWf, 0x1000 );
    soc_reg32_set( nUnit, TP_TCAM_SCAN_DEBUG1r, SOC_BLOCK_PORT(nUnit,nTP), 0, uRegisterValue);
  }

  /* This snippet throttles the Updater to 1 access per 100G epoch, ~(500 tmu clks to 860 lrp) */
/*
  READ_TMB_UPDATER_DS_ARB_CREDIT_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_DS_ARB_CREDIT_CONFIGr, &uRegisterValue, MAX_CREDITSf, 512 );
  WRITE_TMB_UPDATER_DS_ARB_CREDIT_CONFIGr( nUnit, uRegisterValue );

  READ_TMB_UPDATER_DS_ARB_RD_CREDIT_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_DS_ARB_RD_CREDIT_CONFIGr, &uRegisterValue, FIXED_CREDITSf, 512 );
  WRITE_TMB_UPDATER_DS_ARB_RD_CREDIT_CONFIGr( nUnit, uRegisterValue );

  READ_TMB_UPDATER_DS_ARB_WR_CREDIT_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_DS_ARB_WR_CREDIT_CONFIGr, &uRegisterValue, FIXED_CREDITSf, 512 );
  WRITE_TMB_UPDATER_DS_ARB_WR_CREDIT_CONFIGr( nUnit, uRegisterValue );
*/


  COMPILER_64_ZERO(g_uuCacheMissCount);
  COMPILER_64_ZERO(g_uuCacheHitCount); 
  COMPILER_64_ZERO(g_uuCacheHitPendingCount);
  g_c3hppcTmuUpdateManagerCB.nUnit = nUnit;
  g_c3hppcTmuUpdateManagerCB.bExit = 0;
  g_c3hppcTmuUpdateManagerCB.bPopulateFreeListFifos = 0;
  for ( nCmdFifo = 0; nCmdFifo < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifo ) {
    g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifo] = 0;
    g_c3hppcTmuUpdateManagerCB.nUpdateCmdQRdPtr[nCmdFifo] = 0;
    g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifo] = 0;
    g_c3hppcTmuUpdateManagerCB.nExpectRspRingWrPtr[nCmdFifo] = 0;
    g_c3hppcTmuUpdateManagerCB.nExpectRspRingRdPtr[nCmdFifo] = 0;
  }
  for ( uIndex = 0; uIndex < C3HPPC_TMU_TABLE_NUM; ++uIndex ) {
    g_c3hppcTmuUpdateManagerCB.aTableParameters[uIndex].bValid = 0;
    g_c3hppcTmuUpdateManagerCB.aTableParameters[uIndex].FreeList = NULL;
  }
  g_c3hppcTmuUpdateManagerCB.nErrorCounter = 0;
  g_uEmlMaxProvisionedKey = pC3TmuControlInfo->uEmlMaxProvisionedKey;
  g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryNum = g_uEmlMaxProvisionedKey * C3HPPC_TMU_EML424_ROOT_TABLE_ENTRY_SIZE_IN_64b;
  g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer = 
         (uint64 *) sal_alloc( (g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryNum * sizeof(uint64)), "table dump buffer" );
  g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount = 0;
  g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryNum = 
                               C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM * g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryNum;
  g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer = 
         (uint64 *) sal_alloc( (g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryNum * sizeof(uint64)), "table dump buffer" );
  g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryCount = 0;


  /* Flow control for the 2 Updater Command FIFOs */ 
  READ_CMIC_CMC0_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC0_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 );
  soc_reg_field_set( nUnit, CMIC_CMC0_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH1_FLOW_CONTROLf, 1 );
  WRITE_CMIC_CMC0_CONFIGr( nUnit, uRegisterValue );

  /* Flow control for the 4 Free Chain FIFOs */ 
  READ_CMIC_CMC0_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC0_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH2_FLOW_CONTROLf, 1 );
  WRITE_CMIC_CMC0_CONFIGr( nUnit, uRegisterValue );
  READ_CMIC_CMC1_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 );
  soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH1_FLOW_CONTROLf, 1 );
  soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH2_FLOW_CONTROLf, 1 );
  WRITE_CMIC_CMC1_CONFIGr( nUnit, uRegisterValue );

  g_TmuUpdateCmdManagerThreadID = sal_thread_create( "tTmuUpdateCmdManager",
                                                     SAL_THREAD_STKSZ,
                                                     100,
                                                     c3hppc_tmu_update_cmd_manager,
                                                     (void *) &g_c3hppcTmuUpdateManagerCB);
  if ( g_TmuUpdateCmdManagerThreadID == NULL || g_TmuUpdateCmdManagerThreadID == SAL_THREAD_ERROR ) {
    cli_out("\nERROR: Can not create TMU update CMD manager thread\n");
  }

  g_TmuUpdateRspManagerThreadID = sal_thread_create( "tTmuUpdateRspManager",
                                                     SAL_THREAD_STKSZ,
                                                     100,
                                                     c3hppc_tmu_update_rsp_manager,
                                                     (void *) &g_c3hppcTmuUpdateManagerCB);
  if ( g_TmuUpdateRspManagerThreadID == NULL || g_TmuUpdateRspManagerThreadID == SAL_THREAD_ERROR ) {
    cli_out("\nERROR: Can not create TMU update RSP manager thread\n");
  }

  if ( pC3TmuControlInfo->bHwEmlChainManagement ) {
    g_TmuUpdateFreeListManagerThreadID = sal_thread_create( "tTmuUpdateFreeListManager",
                                                            SAL_THREAD_STKSZ,
                                                            100,
                                                            c3hppc_tmu_update_freelist_manager,
                                                            (void *) &g_c3hppcTmuUpdateManagerCB);
    if ( g_TmuUpdateFreeListManagerThreadID == NULL || g_TmuUpdateRspManagerThreadID == SAL_THREAD_ERROR ) {
      cli_out("\nERROR: Can not create TMU update FREELIST manager thread\n");
    }
  }

  for ( nCmdFifo = 0; nCmdFifo < C3HPPC_TMU_UPDATE_RSP_FIFO_NUM; ++nCmdFifo ) {
    /* Provision for 2us */
    WRITE_TMB_UPDATER_RSP_FIFO_INACTIVITY_THRESHr( nUnit, nCmdFifo, 1200 );
  }

  return rc;
}

int c3hppc_tmu_init_scrambler_table( int nUnit, int nScrambleTableMemory ) {

  int nScrambleTableIndex, nScrambleSubTableCount, rc;
  uint32 *auRandTable, *auScrambleTable, uScrambleTableSize, nScrambleTableEntrySizeIn32b, uIndex, uRandNum, uScrambleSubTableSize;

  uScrambleTableSize = soc_mem_index_max( nUnit, nScrambleTableMemory ) + 1;
  nScrambleTableEntrySizeIn32b = soc_mem_entry_words( nUnit, nScrambleTableMemory );
  auScrambleTable = (uint32 *) soc_cm_salloc( nUnit, (uScrambleTableSize * nScrambleTableEntrySizeIn32b * sizeof(uint32)), "scramble table" ); 
  uScrambleSubTableSize = 512;
  auRandTable = (uint32 *) sal_alloc( (uScrambleSubTableSize * sizeof(uint32)), "rand table" ); 

  nScrambleTableIndex = 0;
  for ( nScrambleSubTableCount = 0; nScrambleSubTableCount < 9; ++nScrambleSubTableCount ) {
    for ( uIndex = 0; uIndex < uScrambleSubTableSize; ++uIndex ) {
      auRandTable[uIndex] = 0x80000000 | uIndex;
    }
    for ( uIndex = 0; uIndex < uScrambleSubTableSize; ++uIndex ) {
      while ( 1 ) {
        uRandNum = sal_rand() % uScrambleSubTableSize;
        if ( auRandTable[uRandNum] & 0x80000000 ) {
          auRandTable[uRandNum] &= 0x7fffffff;
          break;
        }
      }
      auScrambleTable[nScrambleTableIndex++] = auRandTable[uRandNum];
    }
    uScrambleSubTableSize >>= 1;
  }

  /*    coverity[negative_returns : FALSE]    */
  rc = soc_mem_write_range( nUnit, nScrambleTableMemory, MEM_BLOCK_ANY, 0,
                            (uScrambleTableSize - 1), (void *) auScrambleTable);

  soc_cm_sfree( nUnit, auScrambleTable );
  sal_free( auRandTable );

  return rc;
}


int c3hppc_tmu_ci_memory_init( int nUnit, c3hppc_tmu_ci_control_info_t *pC3TmuCiControlInfo ) {

  int nCiInstance, nDataRegs;
  uint32 uRegisterValue;
  
  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    for ( nDataRegs = 0; nDataRegs < COUNTOF(g_anCiDDRtestDataRegisters); ++nDataRegs ) {
      if ( nDataRegs == 0 || nDataRegs == 4 ) uRegisterValue = 0xf8000000;
      else if ( nDataRegs == 1 || nDataRegs == 5 ) uRegisterValue = 0x00000007;
      else uRegisterValue = 0;
      soc_reg32_set( nUnit, g_anCiDDRtestDataRegisters[nDataRegs], SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue );
      soc_reg32_set( nUnit, g_anCiDDRtestAltDataRegisters[nDataRegs], SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue );
    }
    /* For 2Gb parts, with a 256b burst size, 2^23 bursts are required. */ 
    soc_reg32_set( nUnit, CI_DDR_BURSTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, (0x800000-1) );

    soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 1 );
    soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

    uRegisterValue = 0;
    soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, 0 );
    soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
    soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, WRITE_ONLYf, 1 );
    soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
    soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  }

  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    soc_reg32_get( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
    soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  }

  return 0;
}


int c3hppc_tmu_is_ci_memory_init_done( int nUnit ) {

  int nCiInstance, nTimeOut;
  uint32 uRegisterValue, uRamDone;
  
  nTimeOut = ( SAL_BOOT_QUICKTURN ) ? 600 : 10;
  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs && nTimeOut; ++nCiInstance ) {
    while ( 1 ) {
      soc_reg32_get( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      uRamDone = soc_reg_field_get( nUnit, CI_DDR_TESTr, uRegisterValue, RAM_DONEf );
      if ( uRamDone ) {
        soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 0 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 0 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 0 );
        soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

        break;

      } else {
        sal_sleep(1);
        if ( !(--nTimeOut) ) break;
      }
    }
  }


  return nTimeOut;
}


int c3hppc_tmu_ci_deassert_ci_reset( int nUnit ) {

  int nCiInstance;
  uint32 uRegisterValue;
  
  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, TREX2_DEBUG_ENABLEf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, PHY_SW_INITf, 1 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, SW_RESET_Nf, 1 );
    soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  }

  return 0;
}


int c3hppc_tmu_ci_deassert_phy_reset( int nUnit ) {

  int nCiInstance;
  uint32 uRegisterValue;
  
  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, TREX2_DEBUG_ENABLEf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, PHY_SW_INITf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, SW_RESET_Nf, 1 );
    soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  }

  return 0;
}


int c3hppc_tmu_ci_deassert_ddr_reset( int nUnit ) {

  int nCiInstance;
  uint32 uRegisterValue;
  
  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, TREX2_DEBUG_ENABLEf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 1 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, PHY_SW_INITf, 0 );
    soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, SW_RESET_Nf, 1 );
    soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  }

  return 0;
}


int c3hppc_tmu_ci_poll_phy_pwrup_rsb( int nUnit ) {
  int nCiInstance;
  sal_time_t TimeStamp;

  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
    if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                 CI_PHY_CONTROLr, PWRUP_RSBf, 1, 1000, 0, &TimeStamp ) ) {
      cli_out("<c3hppc_tmu_ci_poll_phy_pwrup_rsb> -- CI%d CI_PHY_CONTROL \"PWRUP_RSB\" event TIMEOUT!!!\n", nCiInstance);
      return -1;
    }
  }

  return 0;
}

int c3hppc_tmu_ci_clear_alarms( int nUnit ) {
  int nCiInstance, nIndex, nQe;
  uint32 uRegisterValue, uDisableTmuScheduleRefresh;

  for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
    for ( nIndex = 0; nIndex < g_nCiErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anCiErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nCiInstance), 0,
                                ((nIndex < g_nCiErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nCiErrorMaskRegistersCount; ++nIndex ) {
      soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      uDisableTmuScheduleRefresh = soc_reg_field_get( nUnit, CI_CONFIG3r, uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf );
      uRegisterValue = 0;
      if ( g_anCiErrorMaskRegisters[nIndex] == CI_ERROR_MASKr ) {
        if ( uDisableTmuScheduleRefresh ) {
          /* These errors need to be masked because the TMU mechanism to request refresh has been disabled. */
          soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, REFRESH_ERROR_DISINTf, 1 );
          soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, ZQCAL_ERROR_DISINTf, 1 );
        }
        /* These errors need to be masked due to a clock crossing boundary issue that errantly fires this error. (JIRA CA3-2791) */
        soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, WR_CMD_FORMAT_ERR_DISINTf, 1 );
        soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, WR_QUEUE_OVF_DISINTf, 1 );
      }
      soc_reg32_set( nUnit, g_anCiErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue );
    }
  }

  /*  The "c3hppc_ddr_test_suite" takes the TMA/B out of reset.  Need to mask errors */
  for ( nQe = 0; nQe < C3HPPC_TMU_QE_INSTANCE_NUM; ++nQe ) {
    soc_reg32_set( nUnit, TM_QE_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,nQe), 0, 0xffffffff );
    soc_reg32_set( nUnit, TM_QE_ERRORr, SOC_BLOCK_PORT(nUnit,nQe), 0, 0xffffffff );
  }
  soc_reg32_set( nUnit, TMA_KEYPLODER_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,0), 0, 0xffffffff );
  soc_reg32_set( nUnit, TMA_KEYPLODER_ERRORr, SOC_BLOCK_PORT(nUnit,0), 0, 0xffffffff );
  soc_reg32_set( nUnit, TMB_DISTRIBUTOR_REFRESH_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,0), 0, 0xffffffff );
  soc_reg32_set( nUnit, TMB_DISTRIBUTOR_REFRESH_ERRORr, SOC_BLOCK_PORT(nUnit,0), 0, 0xffffffff );

  return 0;
}

uint32 c3hppc_tmu_ci_phy_set_register_field( uint32 uRegisterValue, uint32 uFieldMask, uint32 uFieldValue ) {
  uint32 uRegisterOut;
  int nStartBit;

  uRegisterOut = uRegisterValue & (uFieldMask ^ 0xffffffff);
  nStartBit = c3hppcUtils_first_bit_set( uFieldMask );
  uRegisterOut |= ((uFieldValue << nStartBit) & uFieldMask);

  return uRegisterOut;
}


int c3hppc_tmu_ci_hw_init( int nUnit, c3hppc_tmu_ci_control_info_t *pC3TmuCiControlInfo ) {

  int nCiInstance, rc;
  uint32 uRegisterValue, uDDRburst, uAddr, uTestMode, uModeIndex;
  sal_time_t TimeStamp;
  int nDramGrade, nIndex, nAttempts;
  uint32 auFailedData[8];
  c3hppc_tmu_ci_mem_acc_addr_ut CiMemAccAddr;
  uint32 uTestModes[4] = { 3, 2, 1, 0 };

  rc = 0;

  /*
   * If "c3hppc_tmu_ci_hw_init()" is called directly this ensures TMU is out of reset for DDR refresh purposes.
  */
  READ_TMA_CONTROLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_CONTROLr, &uRegisterValue, TMA_SOFT_RESET_Nf, 1 );
  WRITE_TMA_CONTROLr( nUnit, uRegisterValue );

  READ_TMB_CONTROLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_CONTROLr, &uRegisterValue, TMB_SOFT_RESET_Nf, 1 );
  WRITE_TMB_CONTROLr( nUnit, uRegisterValue );

  switch ( pC3TmuCiControlInfo->nDramFreq ) {
    case DDR_FREQ_800:    nDramGrade = MEM_GRADE_111111; break;
    case DDR_FREQ_933:    nDramGrade = MEM_GRADE_131313; break;
    case DDR_FREQ_1066:   nDramGrade = MEM_GRADE_141414; break;
    default: nDramGrade = 0;
  }

  if ( 0 && SAL_BOOT_QUICKTURN ) {

  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, SW_RESET_Nf, 1 );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, PHY_SW_INITf, 1 );
      soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    uRegisterValue = 0;
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &uRegisterValue, TFAWf, 4 );
    /* 160ns / 1.666667 ~= 96 */
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &uRegisterValue, TRFCf, 96 );
    /* tRC(47.91ns) / 1.666667 ~= 29 */
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &uRegisterValue, TRCf, 29 );
    WRITE_TMB_DISTRIBUTOR_CI_PARAMETERSr( nUnit, uRegisterValue );

    uRegisterValue = 0;
    soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_REFRESH_CONFIGr, &uRegisterValue, REFRESH_TIMER_WINDOWf,
                       C3HPPC_TMU_7800NS_REFRESH_INTERVAL );
    WRITE_TMB_DISTRIBUTOR_REFRESH_CONFIGr( nUnit, uRegisterValue );

    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {

      READ_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, &uRegisterValue );
      soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr, &uRegisterValue, REFRESH_THRESHOLDf,
                         (nCiInstance * C3HPPC_TMU_REFRESH_WHEEL_INTERVAL) );
      WRITE_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, uRegisterValue );

      READ_TMB_PER_CI_REFRESH_DELAYr( nUnit, nCiInstance, &uRegisterValue );
      soc_reg_field_set( nUnit, TMB_PER_CI_REFRESH_DELAYr, &uRegisterValue, CI_REFRESH_DELAYf, 6 );
      WRITE_TMB_PER_CI_REFRESH_DELAYr( nUnit, nCiInstance, uRegisterValue );

      soc_reg32_get( nUnit, CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG0r, &uRegisterValue, TRP_READf, ((nCiInstance%2==1) ? 19 : 20) );
      soc_reg_field_set( nUnit, CI_CONFIG0r, &uRegisterValue, TRRDf, 3 );
      soc_reg_field_set( nUnit, CI_CONFIG0r, &uRegisterValue, TRCf, 23 );
      soc_reg_field_set( nUnit, CI_CONFIG0r, &uRegisterValue, TFAWf, 17 );
      soc_reg32_set( nUnit, CI_CONFIG0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG1r, &uRegisterValue, TRDTWRf, 10 );
      soc_reg_field_set( nUnit, CI_CONFIG1r, &uRegisterValue, TWRTRDf, 21 );
      soc_reg_field_set( nUnit, CI_CONFIG1r, &uRegisterValue, TWLf, (18 - (nCiInstance%2)) );
      soc_reg_field_set( nUnit, CI_CONFIG1r, &uRegisterValue, TREAD_ENBf, (21 - (nCiInstance%2)) );
      soc_reg32_set( nUnit, CI_CONFIG1r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_CONFIG5r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG5r, &uRegisterValue, TZQCSf, 38 );
      soc_reg32_set( nUnit, CI_CONFIG5r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG2r, &uRegisterValue, TRFCf, 75 );
      soc_reg32_set( nUnit, CI_CONFIG2r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

/*  Disables refresh
soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, TREFIf, 0 );
soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
*/

      soc_reg32_get( nUnit, CI_CONFIG5r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG5r, &uRegisterValue, TZQINITf, 299 );
      soc_reg_field_set( nUnit, CI_CONFIG5r, &uRegisterValue, TZQOPERf, 0x96 );
      soc_reg32_set( nUnit, CI_CONFIG5r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_CONFIG6r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG6r, &uRegisterValue, TREF_HOLDOFFf, 0x4000 );
      soc_reg32_set( nUnit, CI_CONFIG6r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_PHY_STRAPS0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS0r, &uRegisterValue, BUS16f, 0 );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS0r, &uRegisterValue, CLf, 13 );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS0r, &uRegisterValue, CWLf, 9 );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS0r, &uRegisterValue, WRf, 14 );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS0r, &uRegisterValue, CHIP_SIZEf, 1 );
      soc_reg32_set( nUnit, CI_PHY_STRAPS0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_PHY_STRAPS1r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS1r, &uRegisterValue, JEDECf, 25 );
      soc_reg_field_set( nUnit, CI_PHY_STRAPS1r, &uRegisterValue, MHZf, 933 );
      soc_reg32_set( nUnit, CI_PHY_STRAPS1r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                   CI_PHY_CONTROLr, PWRUP_RSBf, 1, 1000, 0, &TimeStamp ) ) {
        cli_out("<c3hppc_tmu_ci_hw_init> -- CI%d CI_PHY_CONTROL \"PWRUP_RSB\" event TIMEOUT!!!\n", nCiInstance);
        return -1;
      }
    }
  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 0 );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, PHY_SW_INITf, 0 );
      soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }
  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_MR0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      uRegisterValue |= 0x0001;
      uRegisterValue &= 0xff8f;
      uRegisterValue |= 0x0010;
      uRegisterValue |= 0x0004;
      uRegisterValue &= 0xf1ff;
      uRegisterValue |= 0x0e00;
      soc_reg32_set( nUnit, CI_MR0r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  
      soc_reg32_get( nUnit, CI_MR2r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      uRegisterValue &= 0xffc7;
      uRegisterValue |= 0x0020;
      soc_reg32_set( nUnit, CI_MR2r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }
  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 1 );
      soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }
  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, DDR_MHZf, 933 );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, RST_Nf, 3 );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, AUTO_INITf, 0 );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, CKEf, 0 );
      soc_reg32_set( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    uRegisterValue = 3;
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, 
                                    (C3HPPC_PHY_WORD_LANE_0_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_READ_DATA_DLY__OFFSET),
                                    &uRegisterValue);
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, 
                                    (C3HPPC_PHY_WORD_LANE_1_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_READ_DATA_DLY__OFFSET),
                                    &uRegisterValue);
    }

    /* Turn OFF "dqs_always_on" in ddr40 phy instances */
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 0, 
                                    (C3HPPC_PHY_WORD_LANE_0_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET),
                                    &uRegisterValue);
      uRegisterValue = c3hppc_tmu_ci_phy_set_register_field( uRegisterValue, 
                                                             C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__DQS_ALWAYS_ON,
                                                             0 );
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, 
                                    (C3HPPC_PHY_WORD_LANE_0_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET),
                                    &uRegisterValue);

      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 0, 
                                    (C3HPPC_PHY_WORD_LANE_1_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET),
                                    &uRegisterValue);
      uRegisterValue = c3hppc_tmu_ci_phy_set_register_field( uRegisterValue, 
                                                             C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__DQS_ALWAYS_ON,
                                                             0 );
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, 
                                    (C3HPPC_PHY_WORD_LANE_1_RBUS_START__OFFSET + C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET),
                                    &uRegisterValue);
    }
  
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, CKEf, 3 );
      soc_reg32_set( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }
  
  
    uRegisterValue = 0;
    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DONEf, 1 );
    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, REQf, 1 );
    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DEVICE_SELf, 3 );
    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 2 );
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 3 );
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 1 );
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 0 );
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    uRegisterValue = 0;
    soc_reg_field_set( nUnit, CI_ZQ_CMDr, &uRegisterValue, DONEf, 1 );
    soc_reg_field_set( nUnit, CI_ZQ_CMDr, &uRegisterValue, REQf, 1 );
    soc_reg_field_set( nUnit, CI_ZQ_CMDr, &uRegisterValue, DEVICE_SELf, 3 );
    soc_reg_field_set( nUnit, CI_ZQ_CMDr, &uRegisterValue, CMDf, 3 );
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; nCiInstance += 2 ) {
      soc_reg32_set( nUnit, CI_ZQ_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      soc_reg32_get( nUnit, CI_CONFIG8r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_CONFIG8r, &uRegisterValue, RD_PARITY_ENf, 3 );
      soc_reg_field_set( nUnit, CI_CONFIG8r, &uRegisterValue, WR_PARITY_ENf, 7 );
      soc_reg_field_set( nUnit, CI_CONFIG8r, &uRegisterValue, RFIFO_PARITY_ENf, 1 );
      soc_reg32_set( nUnit, CI_CONFIG8r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

  } else {    /* if (SAL_BOOT_QUICKTURN)  */

    cli_out("<c3hppc_tmu_ci_hw_init> -- Hardware bring-up sequence of CI/PHY/DDR ....\n");

    /*
       Turn off TMB refreshes by default so that when the soc_ddr40 routines get control only 
       active CIs will get issued refreshes.
    */
    for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
      READ_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, &uRegisterValue );
      soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr, &uRegisterValue, REFRESH_THRESHOLDf, 0xffff );
      WRITE_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, uRegisterValue );
    }

    /*
       Hynix parts have bug in "DLL Reset" logic so turning this off.
    for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
      READ_CI_MR0r( nUnit, nCiInstance, &uRegisterValue );
      uRegisterValue &= 0xfeff;
      WRITE_CI_MR0r( nUnit, nCiInstance, uRegisterValue );
    }
    */

/*
g_uNumberOfCIs = 1;
*/

    SOC_DDR3_NUM_COLUMNS(nUnit) = soc_property_get(nUnit,spn_EXT_RAM_COLUMNS, 1024);
    SOC_DDR3_NUM_BANKS(nUnit) = soc_property_get(nUnit,spn_EXT_RAM_BANKS, 8);
    SOC_DDR3_NUM_MEMORIES(nUnit) = soc_property_get(nUnit,spn_EXT_RAM_PRESENT, g_uNumberOfCIs);
    SOC_DDR3_NUM_ROWS(nUnit) = soc_property_get(nUnit,spn_EXT_RAM_ROWS, 16384);
    SOC_DDR3_CLOCK_MHZ(nUnit) = soc_property_get(nUnit, spn_DDR3_CLOCK_MHZ, pC3TmuCiControlInfo->nDramFreq );
    SOC_DDR3_MEM_GRADE(nUnit) = soc_property_get(nUnit, spn_DDR3_MEM_GRADE, nDramGrade );
  
    cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_set_shmoo_dram_config\" ....\n");
    soc_ddr40_set_shmoo_dram_config(nUnit, c3hppcUtils_generate_32b_mask( (g_uNumberOfCIs-1), 0 ) );
    cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_phy_pll_ctl\" ....\n");
    soc_ddr40_phy_pll_ctl( nUnit, 0, SOC_DDR3_CLOCK_MHZ(nUnit), DDR_PHYTYPE_AND, 0 );
    cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_ctlr_ctl\" ....\n");
    soc_ddr40_ctlr_ctl( nUnit, 0, DDR_CTLR_T2, 0 );
    cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_phy_calibrate\" ....\n");
    soc_ddr40_phy_calibrate( nUnit, 0, DDR_PHYTYPE_AND, 0 );

    for ( nCiInstance = g_uNumberOfCIs; nCiInstance <  C3HPPC_TMU_CI_INSTANCE_NUM; nCiInstance += 2 ) {
      soc_reg32_get( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_RESETr, &uRegisterValue, DDR_RESET_Nf, 1 );
      soc_reg32_set( nUnit, CI_RESETr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

      soc_reg32_get( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
      soc_reg_field_set( nUnit, CI_PHY_CONTROLr, &uRegisterValue, PLL_PWRDNf, 1 );
      soc_reg32_set( nUnit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    }

    if ( !SAL_BOOT_QUICKTURN ) {
      for ( nCiInstance = 0; nCiInstance <  g_uNumberOfCIs; nCiInstance += 2 ) {
        if ( soc_property_get( nUnit, spn_DDR3_AUTO_TUNE, FALSE ) ) {
          cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_shmoo_ctl\" for CI%d ....\n", nCiInstance);
          soc_ddr40_shmoo_ctl( nUnit, nCiInstance, DDR_PHYTYPE_AND, DDR_CTLR_T2, 0, 0 );
          soc_ddr40_shmoo_savecfg( nUnit, nCiInstance );
        } else {
          cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_shmoo_restorecfg\" for CI%d ....\n", nCiInstance);
          rc = soc_ddr40_shmoo_restorecfg( nUnit, nCiInstance );
          if ( rc ) {
            cli_out("<c3hppc_tmu_ci_hw_init> -- \"soc_ddr40_shmoo_restorecfg\" FAILED for CI%d ....\n", nCiInstance);
            return rc;
          }
        }
      }
    }
    cli_out("<c3hppc_tmu_ci_hw_init> -- Calling routine --> \"soc_ddr40_ctlr_zqcal_ctl\" ....\n");
    soc_ddr40_ctlr_zqcal_ctl( nUnit, 0, DDR_CTLR_T2, 0 );

    /*
      The DDR config and shmoo does not result with a PASS/FAIL indication that the memory is operable.  An actual
      write/read memory test is required for this.  Using the built CI memory self test logic to accomplish this.
    */
    if ( pC3TmuCiControlInfo->bSkipDramSelfTest ) {

      cli_out("\n\n<c3hppc_tmu_ci_hw_init> -- Skipping Memory Self Test .... \n");

    } else {

      cli_out("\n\n<c3hppc_tmu_ci_hw_init> -- Running Memory Self Test .... \n\n");

      /*
         The following example provides the logic behind the "CI_DDR_BURST" setting.
           For 2Gb parts, with a 256b burst size, 2^23 bursts are required.
      */
      uDDRburst = ( (SOC_DDR3_NUM_COLUMNS(nUnit) * SOC_DDR3_NUM_ROWS(nUnit)) >> 1 ) - 1;

      for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
        soc_reg32_set( nUnit, CI_DDR_STARTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0 );
        soc_reg32_set( nUnit, CI_DDR_BURSTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uDDRburst );
        soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );
        soc_reg32_set( nUnit, CI_DDR_STEPr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );

        soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 1 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 1 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 1 );
        soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
      }

      nAttempts = ( pC3TmuCiControlInfo->nDramFreq == 1066 ) ? 1 : 3;
      while ( nAttempts ) {
        rc = 0;
        for ( uModeIndex = 0; uModeIndex < 1; ++uModeIndex ) {
          uTestMode = uTestModes[uModeIndex];
          for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
            uRegisterValue = 0;
            soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, uTestMode );
            soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
            soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
            soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    
            soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
            soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
          }

          for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
            if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                         CI_DDR_TESTr, RAM_DONEf, 1, 10, 0, &TimeStamp ) ) {
              cli_out("<c3hppc_tmu_ci_hw_init> -- CI%d CI_DDR_TEST \"RAM_DONE\" event TIMEOUT!!!\n", nCiInstance);
              return 1;
            } else {
              soc_reg32_get( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
              if ( soc_reg_field_get( nUnit, CI_DDR_TESTr, uRegisterValue, RAM_TEST_FAILf ) ) {
                soc_reg32_get( nUnit, CI_FAILED_ADDRr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
                cli_out("<c3hppc_tmu_ci_hw_init> -- CI%d self test mode [%d] FAILED on ADDRESS[0x%08x] with DATA PATTERN[255:0] --> \n",
                        nCiInstance, uTestMode, uRegisterValue);
                for ( nIndex = 0; nIndex < 8; ++nIndex ) {
                  soc_reg32_get( nUnit, g_anCiDDRtestFailedDataRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nCiInstance), 0, auFailedData+nIndex );
                }
                cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                        auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                        auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
                rc = 1;
  
                for ( uAddr = 0; uAddr < 64; ++uAddr ) {
                  CiMemAccAddr.value = 0;
                  CiMemAccAddr.bits.Bank = uAddr & 0x7;
                  /* Column offset is the 256b chunk within the 2KB(256b * 64chunks) row. */
                  /* The "4 *" represents the 256b multiplier and the 6 address bits represents the chunk number. */
                  CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
                  CiMemAccAddr.bits.Row = (uAddr >> 9);
                  c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
                  cli_out("<c3hppc_tmu_ci_hw_init> -- Failed CI%d Address[0x%08x] Data Pattern[255:0] --> \n", nCiInstance, uAddr );
                  cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                          auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                          auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
                }

                _soc_caladan3_mem_reset_and_init_after_shmoo_addr( nUnit, (nCiInstance & 0xe) );
              }
            }
          }
        }

        nAttempts = ( rc == 0 ) ? 0 : nAttempts - 1;

      }  /* while ( nAttempts ) */


      for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
        soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 0 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 0 );
        soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 0 );
        soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
      }

    }  /* else ( pC3TmuCiControlInfo->bSkipDramSelfTest ) */

    /*
       A0/A1 --> Lower threshold (from default of 8) as a way to detect refresh request build-up in the CI and reset counters.
                 Keep in mind that there is a clock crossing bug between the CI and TMU that causes the MAX_INFLIGHT_REFRESHES
                 counter to be over-decremented so the count is not accurate.
       B0  -->  With the clock crossing bug fixed and an accurate MAX_INFLIGHT_REFRESHES counter the system requires a value of 13
                that hasn't been explained.
    */
    for ( nCiInstance = 0; nCiInstance < g_uNumberOfCIs; ++nCiInstance ) {
      SOC_IF_ERROR_RETURN( READ_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, &uRegisterValue ) );
      soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr, &uRegisterValue, MAX_INFLIGHT_REFRESHESf, 
                                                                     ((g_rev_id == BCM88030_B0_REV_ID) ? 14 : 4) );
      SOC_IF_ERROR_RETURN( WRITE_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr( nUnit, nCiInstance, uRegisterValue ) );

      uRegisterValue = 0;
      soc_reg_field_set( nUnit, TMB_DISTRIBUTOR_REFRESH_DEBUGr, &uRegisterValue, CLEAR_MAX_REFRESHES_ISSUEDf, 1 );
      SOC_IF_ERROR_RETURN( WRITE_TMB_DISTRIBUTOR_REFRESH_DEBUGr( nUnit, nCiInstance, uRegisterValue ) );
    }
  
  }   /* else (SAL_BOOT_QUICKTURN)  */

  return rc;
}


int c3hppc_tmu_ci_do_shmoo( int nUnit, int nCiInstance ) {

  cli_out("<c3hppc_tmu_ci_do_shmoo> -- Calling routine --> \"soc_ddr40_shmoo_ctl\" for CI%d ....\n", nCiInstance);
  soc_ddr40_shmoo_ctl( nUnit, nCiInstance, DDR_PHYTYPE_AND, DDR_CTLR_T2, 0, 0 );
  soc_ddr40_shmoo_savecfg( nUnit, nCiInstance );

  return 0;
}

int c3hppc_tmu_pm_filter_setup( int nUnit, int nInstance, uint32 uInterface, uint8 bSubKey0, uint8 bOddTags,
                                uint32 uProgram ) {
  int rc;

/* Write-up from Daniel

1) Write 0 to each TMA_PM_HIST memory entry (8 memories, 512 entries each)
2) Each of the 8 performance monitor slices has a filter that specifies which requests to monitor.  (TMA_PM_FILTER_CONFIG)
     a. intf       -- which interface to monitor (0 -- DM0; 1 -- DM1; 2 -- DM2; 3 -- DM3; 4 -- Key)
     b. subkey     -- which subkey to monitor (only applies when intf = 4)
     c. oddtag     -- when set, monitor requests with odd tags.  When clear, monitor requests with even tags.
     d. prog_mask  -- bitmask indicating which programs to collect data on (only applies when intf = 4)
3) Write to TMA_PM_CONFIG
     a. active     = 1
     b. ctr_shift  = 0
     c. dt_offset  = 0
     d. dt_shift   = 2
4) Sometime later (after lots of requests) read the TMA_PM_HIST entries

Note:  which histogram entry will increment is computed as follows (just for clarification -- you can ignore this)
t0_ctr = Request arrival time
t1_ctr = Response time

dt = (t0_ctr[19:0] >> ctr_shift) - (t1_ctr[19:0] >> ctr_shift)
if (dt > dt_offset)
     dt = dt - dt_offset
     else
           dt = 0
     
dt = dt >> dt_shift
dt[8:0] is the histogram bucket we increment

*/


  uint32 *pDmaData, uDmaSize;
  uint32 uRegisterValue;

  uDmaSize = (soc_mem_index_max(nUnit, g_anTmPmMemories[nInstance]) + 1) * 
             soc_mem_entry_words( nUnit, g_anTmPmMemories[nInstance] ) * sizeof(uint32); 
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, uDmaSize, "pm hist mem" );
  sal_memset( pDmaData, 0x00, uDmaSize );
  /*    coverity[negative_returns : FALSE]    */
  rc = soc_mem_write_range( nUnit, g_anTmPmMemories[nInstance], MEM_BLOCK_ANY, 0,
                            soc_mem_index_max(nUnit, g_anTmPmMemories[nInstance]), (void *) pDmaData);

  READ_TMA_PM_FILTER_CONFIGr( nUnit, nInstance, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_PM_FILTER_CONFIGr, &uRegisterValue, INTFf, uInterface );
  soc_reg_field_set( nUnit, TMA_PM_FILTER_CONFIGr, &uRegisterValue, SUBKEYf, ((bSubKey0 == 1) ? 0 : 1) );
  soc_reg_field_set( nUnit, TMA_PM_FILTER_CONFIGr, &uRegisterValue, ODD_TAGf, ((bOddTags == 1) ? 1 : 0) );
  soc_reg_field_set( nUnit, TMA_PM_FILTER_CONFIGr, &uRegisterValue, PROG_MASKf, (1 << uProgram) );
  WRITE_TMA_PM_FILTER_CONFIGr( nUnit, nInstance, uRegisterValue );

  soc_cm_sfree( nUnit, pDmaData );

  return rc;
}

int c3hppc_tmu_pm_activate( int nUnit, uint32 uTimeStampShift, uint32 uBucketShift, uint32 uBucketOffset ) {

  uint32 uRegisterValue;

  READ_TMA_PM_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_PM_CONFIGr, &uRegisterValue, ACTIVEf, 1 );
  soc_reg_field_set( nUnit, TMA_PM_CONFIGr, &uRegisterValue, CTR_SHIFTf, uTimeStampShift );
  soc_reg_field_set( nUnit, TMA_PM_CONFIGr, &uRegisterValue, DT_OFFSETf, uBucketOffset );
  soc_reg_field_set( nUnit, TMA_PM_CONFIGr, &uRegisterValue, DT_SHIFTf, uBucketShift );
  WRITE_TMA_PM_CONFIGr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_tmu_pm_dump_memory( int nUnit, int nInstance ) {

  uint32 *pDmaData, uDmaSize;
  int nIndex, nDmaIndex, nMemRowNum;

  uDmaSize = (soc_mem_index_max( nUnit, g_anTmPmMemories[nInstance] ) + 1) * 
             soc_mem_entry_words( nUnit, g_anTmPmMemories[nInstance] ) * sizeof(uint32);
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, uDmaSize, "pm hist mem" );
  sal_memset( pDmaData, 0x00, uDmaSize );
  SOC_IF_ERROR_RETURN(  soc_mem_read_range( nUnit, g_anTmPmMemories[nInstance], MEM_BLOCK_ANY, 0,
                                            soc_mem_index_max(nUnit, g_anTmPmMemories[nInstance]), (void *) pDmaData) );

  nMemRowNum = soc_mem_index_max( nUnit, g_anTmPmMemories[nInstance] ) + 1;
  for ( nIndex = 0, nDmaIndex = 0; nIndex < nMemRowNum; ++nIndex ) {
    COMPILER_64_SET(g_auuPmMemContents[nIndex], pDmaData[nDmaIndex+1], (pDmaData[nDmaIndex]));
    nDmaIndex += 2;
  }

  soc_cm_sfree( nUnit, pDmaData );

  return 0;
}

uint64 c3hppc_tmu_pm_get_count( int nIndex ) {
  return g_auuPmMemContents[nIndex]; 
}

uint32 c3hppc_tmu_get_emc128mode( void ) {
  return g_uQeEMC128Mode; 
}


int c3hppc_tmu_get_number_of_regions_per_bank( void ) {
  int n, nNumberOfBaseBanks;

  nNumberOfBaseBanks = (g_uNumberOfCIs * C3HPPC_TMU_BANK_NUM) / C3HPPC_TMU_DRAM_BANK_PAIR_NUM;
  n = C3HPPC_TMU_REGION_NUM / nNumberOfBaseBanks;
  /* Need ensure that the product of "number_of_regions_per_bank" * "number_of_banks" is >= 4K (number of regions) */
  if ( (C3HPPC_TMU_REGION_NUM % nNumberOfBaseBanks) ) ++n;
  
  return n;
}


int c3hppc_tmu_get_number_of_rows_per_region_in_a_bank( void ) {
  int n;

  /* For 2Gb parts there are (32MB / 2KB) == 16K rows per bank */
  n = 16384 / c3hppc_tmu_get_number_of_regions_per_bank();

  
  return n;
}


int c3hppc_tmu_region_map_setup( int nUnit, int nRegionLayout ) {
  int rc;

  rc = __c3hppc_tmu_region_map_setup( nUnit, nRegionLayout );

  return rc;
}


int __c3hppc_tmu_region_map_setup( int nUnit, int nRegionLayout ) {

  int nRegion, nDramBankPair, rc COMPILER_ATTRIBUTE((unused));
  uint32 uRegionbase, uDram, uBank, uDramPool, uInitialDramPool;
  uint32 auRegionTableEntry[2];
  int anDramFields[] = {DRAM0f, DRAM1f, DRAM2f, DRAM3f};
  int anBankFields[] = {BANK0f, BANK1f, BANK2f, BANK3f};
  uint32 *pDmaData, nDmaIndex;
  char *apDramBankRegionbaseVectors[C3HPPC_TMU_DRAM_BANK_PAIR_NUM];
  uint32 auAvailableRegionbaseList[C3HPPC_TMU_BANK_NUM * C3HPPC_TMU_DRAM_BANK_PAIR_NUM];
  uint32 uAvailableRegionbaseListLength;
  uint32 auDram[C3HPPC_TMU_DRAM_BANK_PAIR_NUM], auBank[C3HPPC_TMU_DRAM_BANK_PAIR_NUM];
  uint32 uDramBankRegionbaseVectorIndex, uIndex;
  uint32 uDramBank;
  int nNumberOfRegionsPerBank, nNumberOfDramsPerPool;


  
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, (C3HPPC_TMU_REGION_NUM * sizeof(auRegionTableEntry)), "region map" );
  nDmaIndex = 0;

  nNumberOfDramsPerPool = g_uNumberOfCIs / C3HPPC_TMU_DRAM_BANK_PAIR_NUM; 
  nNumberOfRegionsPerBank = c3hppc_tmu_get_number_of_regions_per_bank();
  

  if ( nRegionLayout == C3HPPC_TMU_REGION_LAYOUT__RANDOM ) {
    for ( uDramPool = 0; uDramPool < C3HPPC_TMU_DRAM_BANK_PAIR_NUM; ++uDramPool ) {
      apDramBankRegionbaseVectors[uDramPool] = (char *) sal_alloc( C3HPPC_TMU_REGION_NUM, "vector" );
      sal_memset( apDramBankRegionbaseVectors[uDramPool], 0x01, C3HPPC_TMU_REGION_NUM );
    }
  }

  for ( nRegion = 0; nRegion < C3HPPC_TMU_REGION_NUM; ++nRegion ) {

    auRegionTableEntry[0] = auRegionTableEntry[1] = 0;

    if ( nRegionLayout != C3HPPC_TMU_REGION_LAYOUT__RANDOM ) {

      if ( nRegionLayout == C3HPPC_TMU_REGION_LAYOUT__SEQUENTIAL ) {
        uRegionbase = (nRegion % nNumberOfRegionsPerBank) * c3hppc_tmu_get_number_of_rows_per_region_in_a_bank();
        uDram =  nRegion / (nNumberOfRegionsPerBank * C3HPPC_TMU_BANK_NUM);
        uBank = (nRegion % (nNumberOfRegionsPerBank * C3HPPC_TMU_BANK_NUM)) / nNumberOfRegionsPerBank;
      }
    
      for ( nDramBankPair = 0; nDramBankPair < C3HPPC_TMU_DRAM_BANK_PAIR_NUM; ++nDramBankPair, uDram += nNumberOfDramsPerPool ) {
        soc_mem_field_set( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
                           auRegionTableEntry, anDramFields[nDramBankPair], &uDram );
        soc_mem_field_set( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
                           auRegionTableEntry, anBankFields[nDramBankPair], &uBank );
      }

    } else {

      uInitialDramPool = sal_rand() % C3HPPC_TMU_DRAM_BANK_PAIR_NUM;

      while ( 1 ) {
        uDramBankRegionbaseVectorIndex = sal_rand() % C3HPPC_TMU_REGION_NUM;
        if ( apDramBankRegionbaseVectors[uInitialDramPool][uDramBankRegionbaseVectorIndex] ) {
          apDramBankRegionbaseVectors[uInitialDramPool][uDramBankRegionbaseVectorIndex] = 0;
          break;
        }
      }
      
      uDramBank = uDramBankRegionbaseVectorIndex / nNumberOfRegionsPerBank;
      uRegionbase = uDramBankRegionbaseVectorIndex % nNumberOfRegionsPerBank;
      auBank[uInitialDramPool] = uDramBank % C3HPPC_TMU_BANK_NUM;
      auDram[uInitialDramPool] = (nNumberOfDramsPerPool * uInitialDramPool) + (uDramBank / C3HPPC_TMU_BANK_NUM);

      for ( uDramPool = 0; uDramPool < C3HPPC_TMU_DRAM_BANK_PAIR_NUM; ++uDramPool ) {
        if ( uDramPool != uInitialDramPool ) {

          for ( uDram = 0, uAvailableRegionbaseListLength = 0; uDram < nNumberOfDramsPerPool; ++uDram ) {
            for ( uBank = 0; uBank < C3HPPC_TMU_BANK_NUM; ++uBank ) {
              uIndex = (uDram * nNumberOfRegionsPerBank * C3HPPC_TMU_BANK_NUM) + 
                       (uBank * nNumberOfRegionsPerBank) + uRegionbase;
              if ( uIndex < C3HPPC_TMU_REGION_NUM && apDramBankRegionbaseVectors[uDramPool][uIndex] ) {
                auAvailableRegionbaseList[uAvailableRegionbaseListLength++] = (uDram << 4) | uBank;
              }
            }
          }

          if ( uAvailableRegionbaseListLength == 0 ) {
            cli_out("uAvailableRegionbaseListLength %d Region %d\n", uAvailableRegionbaseListLength, nRegion);
            cli_out("uRegionbase %d uDramBankRegionbaseVectorIndex %d\n", uRegionbase, uDramBankRegionbaseVectorIndex);
            assert( 0 ); 
          }

          /* coverity[divide_by_zero] */
          uIndex  = sal_rand() % uAvailableRegionbaseListLength; 
          uDram = auAvailableRegionbaseList[uIndex] >> 4;
          auBank[uDramPool] =  auAvailableRegionbaseList[uIndex] & 0xf;
          auDram[uDramPool] = (nNumberOfDramsPerPool * uDramPool) + uDram; 
          uDramBankRegionbaseVectorIndex = (uDram * nNumberOfRegionsPerBank * C3HPPC_TMU_BANK_NUM) +
                                           (auBank[uDramPool] * nNumberOfRegionsPerBank) + uRegionbase;
          apDramBankRegionbaseVectors[uDramPool][uDramBankRegionbaseVectorIndex] = 0;

        }
      }

      for ( nDramBankPair = 0; nDramBankPair < C3HPPC_TMU_DRAM_BANK_PAIR_NUM; ++nDramBankPair ) {
        soc_mem_field_set( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
                           auRegionTableEntry, anDramFields[nDramBankPair], &auDram[nDramBankPair] );
        soc_mem_field_set( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
                           auRegionTableEntry, anBankFields[nDramBankPair], &auBank[nDramBankPair] );
      }

      uRegionbase *= c3hppc_tmu_get_number_of_rows_per_region_in_a_bank();

    }

    soc_mem_field_set( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
                       auRegionTableEntry, REGION_BASEf, &uRegionbase );

    pDmaData[nDmaIndex++] = auRegionTableEntry[0];
    pDmaData[nDmaIndex++] = auRegionTableEntry[1];
  }

  /* coverity[unchecked_value] */
  /*    coverity[negative_returns : FALSE]    */
  rc = soc_mem_write_range( nUnit, TMB_DISTRIBUTOR_REGION_DEFINITIONm, MEM_BLOCK_ANY, 0,
                                   (C3HPPC_TMU_REGION_NUM - 1), (void *) pDmaData);
  soc_cm_sfree( nUnit, pDmaData );
  if ( nRegionLayout == C3HPPC_TMU_REGION_LAYOUT__RANDOM ) {
    for ( uDramPool = 0; uDramPool < C3HPPC_TMU_DRAM_BANK_PAIR_NUM; ++uDramPool ) {
      sal_free( apDramBankRegionbaseVectors[uDramPool] );
    }
  }

  return 0;
}


int c3hppc_tmu_table_setup( int nUnit, int nTableIndex, uint32 uLookup, uint32 uNumEntries,   
                            uint32 uReplicationFactor, uint32 uRegionOffset, uint32 uRowOffset,  
                            uint32 uColumnOffset, uint32 uNumEntriesPerRow,  
                            uint32 uDeadlineOffset, uint32 uNextTable, uint32 uUpChainHw,  
                            uint32 uUpChainSplitMode, uint32 uUpChainLimit_BucketPrefixNum, uint32 uUpChainPool,
                            uint32 uEmDefault, int nHashAdjustSelect, uint32 *puNewRegionRowOffset )
{
  uint32 uRegisterValue, uEntrySizeInBytes, uIndex, uNumRowsPerRegion;

  uEntrySizeInBytes = 0;

  uNumEntries = c3hppcUtils_ceil_power_of_2_exp( uNumEntries );
  
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:            uEntrySizeInBytes = (C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b * sizeof(uint64));
                                                  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uKeySizeInBytes =
                                                                                     C3HPPC_TMU_EML64_KEY_SIZE_IN_64b * sizeof(uint64); break;
    case C3HPPC_TMU_LOOKUP__2ND_EML64:            uEntrySizeInBytes = (C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * sizeof(uint64) * 
                                                                                                  (uUpChainLimit_BucketPrefixNum+1)); break; 
    case C3HPPC_TMU_LOOKUP__1ST_EML176:           uEntrySizeInBytes = (C3HPPC_TMU_EML176_ROOT_TABLE_ENTRY_SIZE_IN_64b * sizeof(uint64));
                                                  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uKeySizeInBytes =
                                                                                     C3HPPC_TMU_EML176_KEY_SIZE_IN_64b * sizeof(uint64); break;
    case C3HPPC_TMU_LOOKUP__2ND_EML176:           uEntrySizeInBytes = (C3HPPC_TMU_EML176_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * sizeof(uint64) * 
                                                                                                  (uUpChainLimit_BucketPrefixNum+1)); break; 
    case C3HPPC_TMU_LOOKUP__1ST_EML304:           uEntrySizeInBytes = (C3HPPC_TMU_EML304_ROOT_TABLE_ENTRY_SIZE_IN_64b * sizeof(uint64));
                                                  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uKeySizeInBytes =
                                                                                     C3HPPC_TMU_EML304_KEY_SIZE_IN_64b * sizeof(uint64); break;
    case C3HPPC_TMU_LOOKUP__2ND_EML304:           uEntrySizeInBytes = (C3HPPC_TMU_EML304_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * sizeof(uint64) * 
                                                                                                  (uUpChainLimit_BucketPrefixNum+1)); break; 
    case C3HPPC_TMU_LOOKUP__1ST_EML424:           uEntrySizeInBytes = (C3HPPC_TMU_EML424_ROOT_TABLE_ENTRY_SIZE_IN_64b * sizeof(uint64));
                                                  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uKeySizeInBytes =
                                                                                     C3HPPC_TMU_EML424_KEY_SIZE_IN_64b * sizeof(uint64); break;
    case C3HPPC_TMU_LOOKUP__2ND_EML424:           uEntrySizeInBytes = (C3HPPC_TMU_EML424_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * sizeof(uint64) * 
                                                                                                  (uUpChainLimit_BucketPrefixNum+1)); break; 
    case C3HPPC_TMU_LOOKUP__1ST_EMC64:            uEntrySizeInBytes = ( g_uQeEMC128Mode ) ? 16 : 24; break;
    case C3HPPC_TMU_LOOKUP__2ND_EMC64:            uEntrySizeInBytes = ( g_uQeEMC128Mode ) ? 16 : 24; break;
    case C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET:     uEntrySizeInBytes = ( sizeof(uint64) * 
                                                      c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( uUpChainLimit_BucketPrefixNum ) );
                                                                      uUpChainLimit_BucketPrefixNum = 0;  break;
    case C3HPPC_TMU_LOOKUP__TAPS_IPV4_ASSOC_DATA: uEntrySizeInBytes = 16; break;
    case C3HPPC_TMU_LOOKUP__TAPS_IPV6_BUCKET:     uEntrySizeInBytes = ( sizeof(uint64) *
                                                      c3hppc_tmu_calc_ipv6_bucket_table_entry_size_in_64b( uUpChainLimit_BucketPrefixNum ) );
                                                                      uUpChainLimit_BucketPrefixNum = 0; break; 
    case C3HPPC_TMU_LOOKUP__TAPS_IPV6_ASSOC_DATA: uEntrySizeInBytes = 32; break;
    case C3HPPC_TMU_LOOKUP__DM119:                uEntrySizeInBytes = 16; break;
    case C3HPPC_TMU_LOOKUP__DM247:                uEntrySizeInBytes = 32; break;
    case C3HPPC_TMU_LOOKUP__DM366:                uEntrySizeInBytes = 48; break;
    case C3HPPC_TMU_LOOKUP__DM494:                uEntrySizeInBytes = 64; break;
  }

  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].bValid = 1;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].nTableSizePowerOf2 = (int) uNumEntries;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].nHashAdjustSelect = nHashAdjustSelect;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEntrySizeInBytes = uEntrySizeInBytes;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEntrySizeIn64b = uEntrySizeInBytes / sizeof(uint64);
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uLookup = uLookup;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEMLchainLimit = uUpChainLimit_BucketPrefixNum;
  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].bEMLchainTable = 0;

  if ( uLookup >= C3HPPC_TMU_LOOKUP__2ND_EML64 && uLookup <= C3HPPC_TMU_LOOKUP__2ND_EML424 ) {
    g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].bEMLchainTable = 1;

    g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEMLchainElementSizeIn64b =
         c3hppc_tmu_get_eml_chain_table_chain_element_entry_size_in_64b( uLookup - C3HPPC_TMU_LOOKUP__2ND_EML64 );                         

    switch ( uLookup ) {
      case C3HPPC_TMU_LOOKUP__2ND_EML64:
           g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit = 5; break;
      case C3HPPC_TMU_LOOKUP__2ND_EML176:
           g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit = 3; break;
      case C3HPPC_TMU_LOOKUP__2ND_EML304:
           g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit = 2; break;
      case C3HPPC_TMU_LOOKUP__2ND_EML424:
           g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit = 1; break;
    }

    g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbCommandsPerChainEntry = 
        ( (uUpChainLimit_BucketPrefixNum+1) + (g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit - 1) ) /
                                  g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEML1KbChainLimit;

    if ( uUpChainHw ) {
      g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uEMLchainPoolMask = uUpChainPool; 
      g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uFreeListSize = 1 << uNumEntries;
      g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].FreeList = 
       (uint32 *) sal_alloc( (g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uFreeListSize * sizeof(uint32)),
                             "Chain Table Free List" );
      g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uFreeListNextEntry = 0;
      for ( uIndex = 0; uIndex < g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].uFreeListSize; ++uIndex ) {
        g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].FreeList[uIndex] = uIndex + 1; 
      }
      g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].FreeList[uIndex-1] = C3HPPC_TMU_UPDATE_FREELIST_EMPTY; 
      g_c3hppcTmuUpdateManagerCB.bPopulateFreeListFifos = 1;
    }
  }

  READ_TMA_GLOBAL_TABLE_ENTRY_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, ENTRY_SIZEf, (uEntrySizeInBytes >> 3) );
  soc_reg_field_set( nUnit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, NUM_ENTRIESf, uNumEntries );
  soc_reg_field_set( nUnit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, NEXT_TABLEf, uNextTable );
  soc_reg_field_set( nUnit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, HASH_ADJUST_SELf, (uint32) nHashAdjustSelect );
  WRITE_TMA_GLOBAL_TABLE_ENTRY_CONFIGr( nUnit, nTableIndex, uRegisterValue );

  READ_TMB_GLOBAL_TABLE_ENTRY_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, ENTRY_SIZEf, (uEntrySizeInBytes >> 3) );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, NUM_ENTRIESf, uNumEntries );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, NEXT_TABLEf, uNextTable );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, EM_DEFAULTf, uEmDefault );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &uRegisterValue, HASH_ADJUST_SELf, (uint32) nHashAdjustSelect );
  WRITE_TMB_GLOBAL_TABLE_ENTRY_CONFIGr( nUnit, nTableIndex, uRegisterValue );

  uNumEntriesPerRow = c3hppcUtils_ceil_power_of_2_exp( uNumEntriesPerRow );

  uNumRowsPerRegion = (1 << g_c3hppcTmuUpdateManagerCB.aTableParameters[nTableIndex].nTableSizePowerOf2) /
                                                                                (1 << uNumEntriesPerRow) / 
                                                                                    C3HPPC_TMU_REGION_NUM; 

  if ( uReplicationFactor == 2 ) uNumRowsPerRegion = C3HPPC_MAX( 2, uNumRowsPerRegion );
  if ( uReplicationFactor == 1 ) uNumRowsPerRegion = C3HPPC_MAX( 4, uNumRowsPerRegion );
  if ( puNewRegionRowOffset != NULL ) {
    *puNewRegionRowOffset += c3hppcUtils_ceil_power_of_2( uNumRowsPerRegion );
  }
  uNumRowsPerRegion = c3hppcUtils_ceil_power_of_2_exp( uNumRowsPerRegion ); 


  READ_TMB_GLOBAL_TABLE_LAYOUT_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, REPLICATION_FACTORf, (uReplicationFactor >> 1) );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, NUM_ROWS_PER_REGIONf, uNumRowsPerRegion );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, NUM_ENTRIES_PER_ROWf, uNumEntriesPerRow );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, COLUMN_OFFSETf, uColumnOffset );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, ROW_OFFSETf, uRowOffset );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &uRegisterValue, REGION_OFFSETf, uRegionOffset );
  WRITE_TMB_GLOBAL_TABLE_LAYOUT_CONFIGr( nUnit, nTableIndex, uRegisterValue );

  READ_TMA_GLOBAL_TABLE_DEADLINE_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMA_GLOBAL_TABLE_DEADLINE_CONFIGr, &uRegisterValue, DEADLINEf, uDeadlineOffset );
  WRITE_TMA_GLOBAL_TABLE_DEADLINE_CONFIGr( nUnit, nTableIndex, uRegisterValue );

  READ_TMB_GLOBAL_TABLE_DEADLINE_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_DEADLINE_CONFIGr, &uRegisterValue, DEADLINEf, uDeadlineOffset );
  WRITE_TMB_GLOBAL_TABLE_DEADLINE_CONFIGr( nUnit, nTableIndex, uRegisterValue );

  READ_TMB_GLOBAL_TABLE_CHAIN_CONFIGr( nUnit, nTableIndex, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &uRegisterValue, UP_CHAIN_LIMITf, uUpChainLimit_BucketPrefixNum );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &uRegisterValue, UP_CHAIN_POOLf, uUpChainPool );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &uRegisterValue, UP_CHAIN_SPLIT_MODEf, uUpChainSplitMode );
  soc_reg_field_set( nUnit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &uRegisterValue, UP_CHAIN_HWf, uUpChainHw );
  WRITE_TMB_GLOBAL_TABLE_CHAIN_CONFIGr( nUnit, nTableIndex, uRegisterValue );


  return 0;
}

int c3hppc_tmu_get_hash_adjust_select( int nTable ) {
  return g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].nHashAdjustSelect;
}

int c3hppc_tmu_get_table_size( int nTable ) {
  return g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].nTableSizePowerOf2;
}

uint32 c3hppc_tmu_get_table_entry_size_in_64b( int nTable ) {
  return g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uEntrySizeIn64b;
}


int c3hppc_tmu_taps_write( int nCmdFifoSelect, int nCommandNum, uint32 *pCommandData )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;
  int nCmdSizeInBytes;

  nCmdSizeInBytes = C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_BYTES;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);

  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__TAPS;
  pActiveUpdateCmdInfo->nTapsOperation = C3HPPC_TMU_TAPS_OPCODE__WRITE;
  pActiveUpdateCmdInfo->nNumberOfEntries = nCommandNum;
  pActiveUpdateCmdInfo->pTableData = 
     (uint32 *) sal_alloc( (nCommandNum * nCmdSizeInBytes), "Taps Command" );
  sal_memcpy( pActiveUpdateCmdInfo->pTableData, pCommandData,
              (nCommandNum * C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_BYTES) );

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];


  return 0;
}


int c3hppc_tmu_eml_insert( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pInsertData, int nInsertOptions )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;
  uint32 uEntrySizeInBytes;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);

  uEntrySizeInBytes = g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uKeySizeInBytes + C3HPPC_TMU_ASSOC_DATA_SIZE_IN_BYTES; 

  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT;
  pActiveUpdateCmdInfo->nOptions = nInsertOptions;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nNumberOfEntries = nNumberOfEntries;
  pActiveUpdateCmdInfo->pTableData = (uint32 *) sal_alloc( (nNumberOfEntries * uEntrySizeInBytes), "Insert Data" ); 
  sal_memcpy( pActiveUpdateCmdInfo->pTableData, pInsertData, (nNumberOfEntries * uEntrySizeInBytes) );

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];


  return 0;
}



int c3hppc_tmu_eml_verify_delete( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pInsertData )
{
  return c3hppc_tmu_eml_delete( nCmdFifoSelect, nTable, nNumberOfEntries, pInsertData,
                                C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_NOT_FOUND );
}

int c3hppc_tmu_eml_verify_insert( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pInsertData )
{
  return c3hppc_tmu_eml_delete( nCmdFifoSelect, nTable, nNumberOfEntries, pInsertData,
                                C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_FOUND );
}


int c3hppc_tmu_eml_delete( int nCmdFifoSelect, int nTable, int nNumberOfEntries, uint32 *pKeyData, int nDeleteOptions )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;
  uint32 uEntrySizeInBytes;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);

  uEntrySizeInBytes = g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uKeySizeInBytes;

  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__EML_DELETE;
  pActiveUpdateCmdInfo->nOptions = nDeleteOptions;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nNumberOfEntries = nNumberOfEntries;
  pActiveUpdateCmdInfo->pTableData = (uint32 *) sal_alloc( (nNumberOfEntries * uEntrySizeInBytes), "Key Data" ); 
  sal_memcpy( pActiveUpdateCmdInfo->pTableData, pKeyData, (nNumberOfEntries * uEntrySizeInBytes) );

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];


  return 0;
}


int c3hppc_tmu_xl_write( int nCmdFifoSelect, int nTable, int nStartingEntryIndex,
                         int nNumberOfEntries, uint32 uOffset, uint32 *pTableData )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);

  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__XL_WRITE;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nStartingEntryIndex = nStartingEntryIndex;
  pActiveUpdateCmdInfo->nNumberOfEntries = nNumberOfEntries;
  pActiveUpdateCmdInfo->uOffset = uOffset;
  pActiveUpdateCmdInfo->pTableData = 
     (uint32 *) sal_alloc( (nNumberOfEntries * g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uEntrySizeInBytes), "XL Write Data" );
  sal_memcpy( pActiveUpdateCmdInfo->pTableData, pTableData,
              (nNumberOfEntries * g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uEntrySizeInBytes) );

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];


  return 0;
}


int c3hppc_tmu_xl_read( int nCmdFifoSelect, int nTable, int nStartingEntryIndex,
                        int nNumberOfEntries, uint32 uOffset, uint32 *pTableData )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);

  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__XL_READ;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nStartingEntryIndex = nStartingEntryIndex;
  pActiveUpdateCmdInfo->nNumberOfEntries = nNumberOfEntries;
  pActiveUpdateCmdInfo->uOffset = uOffset;
  if ( pTableData != NULL ) {
    pActiveUpdateCmdInfo->pTableData = 
       (uint32 *) sal_alloc( (nNumberOfEntries * g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uEntrySizeInBytes), "XL Read Data" );
    sal_memcpy( pActiveUpdateCmdInfo->pTableData, pTableData,
                (nNumberOfEntries * g_c3hppcTmuUpdateManagerCB.aTableParameters[nTable].uEntrySizeInBytes) );
  } else {
    pActiveUpdateCmdInfo->pTableData = NULL;
  }

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];


  return 0;
}


int c3hppc_tmu_lock( int nCmdFifoSelect, int nTable, uint32 uEntryNumber )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);


  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__LOCK;
  pActiveUpdateCmdInfo->nOptions = ( uEntryNumber == C3HPPC_TMU_UPDATE_GLOBAL_LOCK ) ? 1 : 0;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nNumberOfEntries = ( uEntryNumber == C3HPPC_TMU_UPDATE_GLOBAL_LOCK ) ? 0 : ((int) uEntryNumber);
  pActiveUpdateCmdInfo->pTableData = NULL;

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];

  return 0;
}


int c3hppc_tmu_release( int nCmdFifoSelect, int nTable )
{
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo;

  pActiveUpdateCmdInfo = 
      &(g_c3hppcTmuUpdateManagerCB.UpdateCmdQ[nCmdFifoSelect][g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]]);


  pActiveUpdateCmdInfo->nCommand = C3HPPC_TMU_UPDATE_COMMAND__RELEASE;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->nOptions = 0;
  pActiveUpdateCmdInfo->nNumberOfEntries = 0;
  pActiveUpdateCmdInfo->pTableData = NULL;

  g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect] = 
    (g_c3hppcTmuUpdateManagerCB.nUpdateCmdQWrPtr[nCmdFifoSelect]+1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect];

  return 0;
}

/* Added for B0 */
int c3hppc_tmu_enable_ipv6_3rd_probe( int nUnit ) {
  int nQE;
  uint32 uRegisterValue;

  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    if ( g_rev_id == BCM88030_B0_REV_ID ) soc_reg_field_set( nUnit, TM_QE_CONFIGr, &uRegisterValue, V6_ENABLE_3RD_PROBEf, 1 );
    soc_reg32_set( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, uRegisterValue);

    soc_reg32_get( nUnit, TM_QE_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    if ( g_rev_id == BCM88030_B0_REV_ID ) soc_reg_field_set( nUnit, TM_QE_ERROR_MASKr, &uRegisterValue, V6_COLLISION_DISINTf, 1 );
    soc_reg32_set( nUnit, TM_QE_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,nQE), 0, uRegisterValue);
  }

  return 0;
}

/* Added for B0 */
int c3hppc_tmu_enable_eml_144_mode( int nUnit ) {
  int nQE;
  uint32 uRegisterValue;

  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, TM_QE_CONFIGr, &uRegisterValue, EML_144_MODEf, 1 );
    soc_reg32_set( nUnit, TM_QE_CONFIGr, SOC_BLOCK_PORT(nUnit,nQE), 0, uRegisterValue);
  }

  READ_TMB_CONTROLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_CONTROLr, &uRegisterValue, EML_144_MODEf, 1 );
  WRITE_TMB_CONTROLr( nUnit, uRegisterValue );

  return 0;
}

int c3hppc_tmu_hw_cleanup( int nUnit ) {

  int nIndex, nInstance;
  uint32 uRegisterValue;

  for ( nInstance = 0; nInstance < C3HPPC_TMU_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nTmErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nTmErrorMaskRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, 0x00000000 );
    }
  }

  for ( nInstance = 0; nInstance < g_uNumberOfQEs; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmQeErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmQeErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nTmQeErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nTmQeErrorMaskRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmQeErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, 0x00000000 );
    }
  }

  for ( nInstance = 0; nInstance < g_uNumberOfCIs; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nCiErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anCiErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nCiErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nCiErrorMaskRegistersCount; ++nIndex ) {
      uRegisterValue = 0;
      if ( g_anCiErrorMaskRegisters[nIndex] == CI_ERROR_MASKr ) {
        /* These errors needs to be masked due to a clock crossing boundary issue that errantly fires this error. (JIRA CA3-2791) */
        soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, WR_CMD_FORMAT_ERR_DISINTf, 1 );
        soc_reg_field_set( nUnit, CI_ERROR_MASKr, &uRegisterValue, WR_QUEUE_OVF_DISINTf, 1 );
      }
      soc_reg32_set( nUnit, g_anCiErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
    }
  }

  for ( nInstance = 0; nInstance < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmTpErrorRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmTpErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0,
                                ((nIndex < g_nTmTpErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( nIndex = 0; nIndex < g_nTmTpErrorMaskRegistersCount; ++nIndex ) {
      soc_reg32_set( nUnit, g_anTmTpErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, 0x00000000 );
    }
  }

  return 0;
}



int c3hppc_tmu_display_error_state( int nUnit ) {

  int rc, nIndex, nInstance;
  uint32 uRegisterValue;

  /* Fred said bits 15:0 being set is not an error condition. */
  uRegisterValue = 0x0000ffff;
  WRITE_TMB_DISTRIBUTOR_INTERFACE_FIFO_ERRORr( nUnit, uRegisterValue );

  for ( rc = 0, nInstance = 0; nInstance < C3HPPC_TMU_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anTmErrorRegisters[nIndex] );
    }
  }

  for ( nInstance = 0; nInstance < g_uNumberOfQEs; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmQeErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anTmQeErrorRegisters[nIndex] );
    }
  }

  for ( nInstance = 0; nInstance < g_uNumberOfCIs; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nCiErrorRegistersCount; ++nIndex ) {
      if ( g_anCiErrorRegisters[nIndex] == CI_ERRORr ) {
        uRegisterValue = 0;
        /* This error needs to be cleared due to a clock crossing boundary issue that errantly fires this error. */
        soc_reg_field_set( nUnit, CI_ERRORr, &uRegisterValue, WR_CMD_FORMAT_ERRf, 1 );
        soc_reg32_set( nUnit, g_anCiErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,nInstance), 0, uRegisterValue );
      }
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anCiErrorRegisters[nIndex] );
    }
  }

  for ( nInstance = 0; nInstance < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nInstance ) {
    for ( nIndex = 0; nIndex < g_nTmTpErrorRegistersCount; ++nIndex ) {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,nInstance), g_anTmTpErrorRegisters[nIndex] );
    }
  }

  return rc;
}




int c3hppc_tmu_ci_read_write( int nUnit, int nCiInstance, int nBurstSizeInBytes,
                              uint8 bWrite, uint32 uAddress, uint32 *puEntryData )
{
  uint32 uRegisterValue;
  int nDataReg;
  sal_time_t TimeStamp;


  if ( bWrite ) {
    for ( nDataReg = 0; nDataReg < COUNTOF(g_anCiMemAccDataRegisters); ++nDataReg ) {
      SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, g_anCiMemAccDataRegisters[nDataReg],
                                          SOC_BLOCK_PORT(nUnit,nCiInstance), 0, puEntryData[nDataReg]) );
    }
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, CI_MEM_ACC_CTRLr, &uRegisterValue, MEM_ACC_ACKf, 1 );
  soc_reg_field_set( nUnit, CI_MEM_ACC_CTRLr, &uRegisterValue, MEM_ACC_REQf, 1 );
  soc_reg_field_set( nUnit, CI_MEM_ACC_CTRLr, &uRegisterValue, MEM_ACC_SIZEf, ((nBurstSizeInBytes/8) - 1) );
  soc_reg_field_set( nUnit, CI_MEM_ACC_CTRLr, &uRegisterValue, MEM_ACC_ADDRf, uAddress );
  soc_reg_field_set( nUnit, CI_MEM_ACC_CTRLr, &uRegisterValue, MEM_ACC_RD_WR_Nf, (bWrite ? 0 : 1) );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CI_MEM_ACC_CTRLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 
                                      0, uRegisterValue) );

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance), 
                               CI_MEM_ACC_CTRLr, MEM_ACC_ACKf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_tmu_ci_read_write> -- CI%d CI_MEM_ACC_CTRL \"MEM_ACC_ACK\" event TIMEOUT!!!\n", nCiInstance);
    return -1;
  }

  
  if ( !bWrite ) {
    for ( nDataReg = 0; nDataReg < COUNTOF(g_anCiMemAccDataRegisters); ++nDataReg ) {
      SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, g_anCiMemAccDataRegisters[nDataReg],
                                          SOC_BLOCK_PORT(nUnit,nCiInstance), 0, puEntryData+nDataReg ) );
    }
  }

  return 0;
}



int c3hppc_tmu_ci_phy_read_write( int nUnit, int nCiInstance, uint8 bWrite, uint32 uAddress, uint32 *puEntryData )
{
  uint32 uRegisterValue;
  sal_time_t TimeStamp;


  if ( bWrite ) {
    SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, DDR_PHY_REG_DATAr, SOC_BLOCK_PORT(nUnit,nCiInstance), 
                                        0, puEntryData[0]) );
  }

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, DDR_PHY_REG_CTRLr, &uRegisterValue, ACKf, 1 );
  soc_reg_field_set( nUnit, DDR_PHY_REG_CTRLr, &uRegisterValue, REQf, 1 );
  soc_reg_field_set( nUnit, DDR_PHY_REG_CTRLr, &uRegisterValue, ADDRf, uAddress );
  soc_reg_field_set( nUnit, DDR_PHY_REG_CTRLr, &uRegisterValue, RD_WR_Nf, (bWrite ? 0 : 1) );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, DDR_PHY_REG_CTRLr, SOC_BLOCK_PORT(nUnit,nCiInstance), 
                                      0, uRegisterValue) );

  if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance), 
                               DDR_PHY_REG_CTRLr, ACKf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_tmu_ci_read_write> -- CI%d DDR_PHY_REG_CTRL \"ACK\" event TIMEOUT!!!\n", nCiInstance);
    return -1;
  }

  
  if ( !bWrite ) {
    SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, DDR_PHY_REG_DATAr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, puEntryData+0 ) );
  }

  return 0;
}

uint32 c3hppc_tmu_1stLookup_hash( c3hppc_tmu_key_t auuKey, int nHashAdjustSelect, int nTableSizePowerOf2 ) {
  int nKeyWord, nHashTableIndex, nHashTable;
  uint32 uAccumulator32, uKeyHi, uKeyLo;
  uint64 uuAccumulator64;
  uint64 uuKeyBits;

  assert(nTableSizePowerOf2<=32);

  uAccumulator32 = 0;
  COMPILER_64_ZERO(uuAccumulator64);

  /* Adjust key and compute CRCs... */
  for ( nKeyWord = 6; nKeyWord >= 0; --nKeyWord ) {
    uint64 uuTmp;
    COMPILER_64_SET(uuKeyBits, COMPILER_64_HI(auuKey[nKeyWord]), COMPILER_64_LO(auuKey[nKeyWord]));
    COMPILER_64_ADD_64(uuKeyBits, g_auuHashAdjustValues[nHashAdjustSelect]);
    uKeyHi = COMPILER_64_HI(uuKeyBits);
    uKeyLo = COMPILER_64_LO(uuKeyBits);
    uAccumulator32 = c3hppc_tmu_crc32_ieee802( uAccumulator32 ^ uKeyHi );
    uAccumulator32 = c3hppc_tmu_crc32_ieee802( uAccumulator32 ^ uKeyLo );
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuAccumulator64), COMPILER_64_LO(uuAccumulator64));
    COMPILER_64_XOR(uuTmp, uuKeyBits);
    uuAccumulator64 = c3hppc_tmu_crc64( uuTmp );
  }

  /* Permute...   */
  for ( nHashTable = 0; nHashTable < C3HPPC_TMU_HASH_TABLE_NUM; nHashTable++ ) {
    uint64 uuTmp;
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuAccumulator64), COMPILER_64_LO(uuAccumulator64));
    COMPILER_64_SHR(uuTmp, (8 * nHashTable));
    nHashTableIndex = COMPILER_64_LO(uuTmp) & (C3HPPC_TMU_HASH_TABLE_ENTRY_NUM-1); 
    uAccumulator32 ^= g_auHashTables[nHashTable][nHashTableIndex];
  }

  /* Fold and reduce to table size ...  */
  if ( nTableSizePowerOf2 != 32 ) {
    uAccumulator32 ^= (uAccumulator32 >> nTableSizePowerOf2);
    uAccumulator32 &= (1 << nTableSizePowerOf2) - 1;
  }

  return uAccumulator32;
}

uint32 c3hppc_tmu_2ndEmcLookup_hash( c3hppc_tmu_key_t auuKey, int nTableSizePowerOf2 ) {
  uint64 uuKeyBits;
  uint32 uAccumulator32, uKeyHi, uKeyLo;

  assert(nTableSizePowerOf2<=32);

  uAccumulator32 = 0;

  /* Adjust key and compute CRCs... */
  COMPILER_64_SET(uuKeyBits, COMPILER_64_HI(auuKey[0]), COMPILER_64_LO(auuKey[0]));
  COMPILER_64_ADD_64(uuKeyBits, g_uuQeHashAdjustValue);
  uKeyHi = COMPILER_64_HI(uuKeyBits);
  uKeyLo = COMPILER_64_LO(uuKeyBits);
  uAccumulator32 = c3hppc_tmu_crc32_qe( uAccumulator32 ^ uKeyHi );
  uAccumulator32 = c3hppc_tmu_crc32_qe( uAccumulator32 ^ uKeyLo );

  /* Reduce to table size ... */
  if ( nTableSizePowerOf2 != 32 ) {
    uAccumulator32 &= (1 << nTableSizePowerOf2) - 1;
  }

  return uAccumulator32;
}

uint32 c3hppc_tmu_crc32_ieee802( uint32 uDataIn ) {
  const uint32 uPoly = 0x04c11db7;
  int n;
  uint32 uResult;

  uResult = uDataIn;
  for ( n = 0; n < 32; n++ ) {
    if (uResult & 0x80000000)
      uResult = (uResult << 1) ^ uPoly;
    else
      uResult <<= 1;
  }

  return uResult;
}

uint32 c3hppc_tmu_crc32_qe( uint32 uDataIn ) {
  const uint32 uPoly = 0x1edc6f41;
  int n;
  uint32 uResult;

  uResult = uDataIn;
  for ( n = 0; n < 32; n++ ) {
    if (uResult & 0x80000000)
      uResult = (uResult << 1) ^ uPoly;
    else
      uResult <<= 1;
  }

  return uResult;
}

uint64 c3hppc_tmu_crc64( uint64 uuDataIn ) {
  uint64 uuPoly;
  int n;
  uint64 uuResult;

  COMPILER_64_SET(uuPoly, 0x42F0E1EB, 0xA9EA3693);
  COMPILER_64_SET(uuResult, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  for ( n = 0; n < 64; n++ ) {
    if (COMPILER_64_BITTEST(uuResult,63)) {
      COMPILER_64_SHL(uuResult,1);
      COMPILER_64_XOR(uuResult, uuPoly);
    } else
      COMPILER_64_SHL(uuResult,1);
  }

  return uuResult;
}




int c3hppc_tmu_exit_update_manager_thread( void ) {

  g_c3hppcTmuUpdateManagerCB.bExit = 1;
  sal_sleep(1);

  return 0;
}

int c3hppc_tmu_are_free_chain_fifos_empty( int nUnit ) {
  uint32 uFreeChainFifo, uSum, uFullSpace;

  for ( uFreeChainFifo = 0, uSum = 0; uFreeChainFifo < C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM; ++uFreeChainFifo ) {
    READ_TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr( nUnit, uFreeChainFifo, &uFullSpace );
    uSum += uFullSpace;
  }

  return ( uSum ? 0 : 1 );
}

int c3hppc_tmu_are_rsp_fifos_empty( void ) {
  if ( c3hppc_tmu_get_expect_rsp_ring_count(g_c3hppcTmuUpdateManagerCB.nExpectRspRingWrPtr[0],
                                            g_c3hppcTmuUpdateManagerCB.nExpectRspRingRdPtr[0]) ||
       c3hppc_tmu_get_expect_rsp_ring_count(g_c3hppcTmuUpdateManagerCB.nExpectRspRingWrPtr[1],
                                            g_c3hppcTmuUpdateManagerCB.nExpectRspRingRdPtr[1]) ) return 0; 
  else return 1;
}

int c3hppc_tmu_are_cmd_fifos_empty( void ) {
  if ( g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[0] || g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[1] ) return 0;
  else return 1;
}

int c3hppc_tmu_is_cmd_fifo_empty( int nCmdFifoSelect ) {
  if ( g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect] ) return 0;
  else return 1;
}

int c3hppc_tmu_cmd_fifo_count( int nCmdFifoSelect ) {
  return ( g_c3hppcTmuUpdateManagerCB.nUpdateCmdQCount[nCmdFifoSelect] );
}

int c3hppc_tmu_rsp_fifo_count( int nRspFifoSelect ) {
  return ( c3hppc_tmu_get_expect_rsp_ring_count(g_c3hppcTmuUpdateManagerCB.nExpectRspRingWrPtr[nRspFifoSelect],
                                                g_c3hppcTmuUpdateManagerCB.nExpectRspRingRdPtr[nRspFifoSelect]) );
}

int c3hppc_tmu_get_rsp_fifos_error_count( void ) {
  return g_c3hppcTmuUpdateManagerCB.nErrorCounter;
}

uint64 c3hppc_tmu_get_cache_hit_count( void ) {
  return g_uuCacheHitCount;
}

uint64 c3hppc_tmu_get_cache_miss_count( void ) {
  return g_uuCacheMissCount;
}

uint64 c3hppc_tmu_get_cache_hit_for_pending_count( void ) {
  return g_uuCacheHitPendingCount;
}

uint64 c3hppc_tmu_collect_cache_hit_counts( int nUnit, uint8 bClear ) {
  uint32 uRegisterValue;
  int nQE;

  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_CACHE_HIT_CNT_DEBUGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    if ( bClear ) COMPILER_64_ZERO(g_uuCacheHitCount); 
    else COMPILER_64_ADD_32(g_uuCacheHitCount, uRegisterValue); 
  }
  return g_uuCacheHitCount;
}


uint64 c3hppc_tmu_collect_cache_miss_counts( int nUnit, uint8 bClear ) {
  uint32 uRegisterValue;
  int nQE;

  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_DRAM_REQ_CNT_DEBUGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    if ( bClear ) COMPILER_64_ZERO(g_uuCacheMissCount); 
    else COMPILER_64_ADD_32(g_uuCacheMissCount, uRegisterValue); 
  }
  return g_uuCacheMissCount;
}


uint64 c3hppc_tmu_collect_cache_hit_for_pending_counts( int nUnit, uint8 bClear ) {
  uint32 uRegisterValue;
  int nQE;

  for ( nQE = 0; nQE < g_uNumberOfQEs; ++nQE ) {
    soc_reg32_get( nUnit, TM_QE_CACHE_PENDING_CNT_DEBUGr, SOC_BLOCK_PORT(nUnit,nQE), 0, &uRegisterValue );
    if ( bClear ) COMPILER_64_ZERO(g_uuCacheHitPendingCount);
    else COMPILER_64_ADD_32(g_uuCacheHitPendingCount, uRegisterValue); 
  }
  return g_uuCacheHitPendingCount;
}

int c3hppc_tmu_keyploder_setup( int nUnit, int nSubKeyInstance, int nProgram, uint32 uLookup, uint32 uTable,
                                uint32 uTapsSegment, uint32 uShiftS0, uint32 uMaskS0, uint32 uShiftS1, uint32 uMaskS1,
                                uint32 uShiftLeftS0 ) {

  uint32 uShiftConfig, uLookupConfig;

  if ( nSubKeyInstance == 0 ) {
    READ_TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr( nUnit, nProgram, &uShiftConfig ); 
    READ_TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr( nUnit, nProgram, &uLookupConfig ); 
  } else {
    READ_TMA_KEYPLODER_SUBKEY1_SHIFT_CONFIGr( nUnit, nProgram, &uShiftConfig ); 
    READ_TMA_KEYPLODER_SUBKEY1_LOOKUP_CONFIGr( nUnit, nProgram, &uLookupConfig ); 
  }

  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &uShiftConfig, KEY_SHIFT_S0f, uShiftS0 );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &uShiftConfig, KEY_MASK_S0f, uMaskS0 );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &uShiftConfig, KEY_SHIFT_S1f, uShiftS1 );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &uShiftConfig, KEY_MASK_S1f, uMaskS1 );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &uShiftConfig, SUB_KEY_SHIFT_S0f, uShiftLeftS0 );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &uLookupConfig, LOOKUPf, uLookup );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &uLookupConfig, TABLE0f, uTable );
  soc_reg_field_set( nUnit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &uLookupConfig, TAPS_SEGf, uTapsSegment );

  if ( nSubKeyInstance == 0 ) {
    WRITE_TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr( nUnit, nProgram, uShiftConfig ); 
    WRITE_TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr( nUnit, nProgram, uLookupConfig ); 
  } else {
    WRITE_TMA_KEYPLODER_SUBKEY1_SHIFT_CONFIGr( nUnit, nProgram, uShiftConfig ); 
    WRITE_TMA_KEYPLODER_SUBKEY1_LOOKUP_CONFIGr( nUnit, nProgram, uLookupConfig ); 
  }

  return 0;
}


int c3hppc_tmu_taps_segment_setup( int nUnit, int nInstance, int nSegment, int nKeySize, int nRootPivotNum, uint32 uBase,
                                   int nBBXbucketPrefixSize, int nPrefixesPerBucket, uint8 bUnified, uint32 uMode ) {

  uint32 uRegisterValue, uLimit, uMaxPrefixesPerBaseUnit, uFormat;

  uLimit = 0;
  uMaxPrefixesPerBaseUnit = 0;
  uFormat = 0;

  if ( nKeySize <= 48 ) {
    uLimit = ((nRootPivotNum / 4) - 1) >> 2;
    uFormat = 1;
  } else if ( nKeySize == 144 ) {
    uLimit = (nRootPivotNum - 1) >> 2;
    uFormat = 3;
  }

  uBase >>= 2;

  /* TAPS uarch Table 47 */
  if ( nBBXbucketPrefixSize <= 32 ) uMaxPrefixesPerBaseUnit= 12; 
  else if ( nBBXbucketPrefixSize <= 48 ) uMaxPrefixesPerBaseUnit = 8; 
  else if ( nBBXbucketPrefixSize <= 64 ) uMaxPrefixesPerBaseUnit = 6; 
  else if ( nBBXbucketPrefixSize <= 96 ) uMaxPrefixesPerBaseUnit = 4; 
  else if ( nBBXbucketPrefixSize <= 128 ) uMaxPrefixesPerBaseUnit = 3; 
  else nBBXbucketPrefixSize = 2;

  soc_reg32_get( nUnit, TP_SEGMENT_CONFIG0_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, &uRegisterValue );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG0_Sr, &uRegisterValue, KEY_SIZEf, (uint32) nKeySize );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG0_Sr, &uRegisterValue, LIMITf, uLimit );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG0_Sr, &uRegisterValue, BASEf, uBase );
  soc_reg32_set( nUnit, TP_SEGMENT_CONFIG0_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, uRegisterValue);

  soc_reg32_get( nUnit, TP_SEGMENT_CONFIG1_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, &uRegisterValue );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG1_Sr, &uRegisterValue, PREFIXES_PER_BUCKETf, (uint32) nPrefixesPerBucket );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG1_Sr, &uRegisterValue, MAX_PREFIXES_PER_BASE_UNITf, uMaxPrefixesPerBaseUnit );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG1_Sr, &uRegisterValue, FORMATf, uFormat );
  soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG1_Sr, &uRegisterValue, CTRLf, uMode );
  soc_reg32_set( nUnit, TP_SEGMENT_CONFIG1_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, uRegisterValue);

  if ( bUnified && nInstance == 1 ) {
    soc_reg32_get( nUnit, TP_SEGMENT_CONFIG2_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, &uRegisterValue );
    soc_reg_field_set( nUnit, TP_SEGMENT_CONFIG2_Sr, &uRegisterValue, GLOBAL_BASE_POINTERf, (uint32) nRootPivotNum );
    soc_reg32_set( nUnit, TP_SEGMENT_CONFIG2_Sr, SOC_BLOCK_PORT(nUnit,nInstance), nSegment, uRegisterValue);
  }


  return 0;
}


/* crc40gsm = x40 + x26 + x23 + x17 + x3 + 1 */
uint64 c3hppc_tmu_psig_hash_calc( uint32 *puPrefixData, uint32 uHashAdjust ) {
  uint32  uAdjustedValue, uPrefixData;
  uint64  b;
  uint64  b40;
  uint64  crc40;
  uint64  crc40_bit39;
  uint64  uuTmp;
  uint32  auAdjustedPrefix[4];
  int     nWord, nShift;

  COMPILER_64_ZERO(crc40);

  for ( nWord = 0; nWord < 4; nWord++ ) {
    auAdjustedPrefix[nWord] = 0;
    uPrefixData = puPrefixData[nWord];
    for ( nShift = 0; nShift < 32; nShift += 8 ) {
      uAdjustedValue = (((uPrefixData >> nShift) & 0xff) + uHashAdjust) & 0xff;
      if ( nWord == 3 && nShift == 24 ) uAdjustedValue &= 0x7f;
      auAdjustedPrefix[nWord] |= uAdjustedValue << nShift;
    }
  }

  for ( nWord = 3; nWord >= 0; --nWord ) {
    for ( nShift = ((nWord == 3) ? 30 : 31); nShift >= 0; --nShift ) {
      COMPILER_64_SET(b,0,((auAdjustedPrefix[nWord] >> nShift) & 1)); 
      COMPILER_64_ZERO(b40);
      COMPILER_64_ZERO(crc40_bit39);
      if (COMPILER_64_BITTEST(crc40, 39))
        COMPILER_64_SET(crc40_bit39, 0, 1);
      COMPILER_64_SET(uuTmp, COMPILER_64_HI(b), COMPILER_64_LO(b));
      COMPILER_64_XOR(uuTmp, crc40_bit39); /* <<  0 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 3);           /* <<  3 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 14);          /* << 17 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 6 );          /* << 23 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 3 );          /* << 26 */
      COMPILER_64_OR(b40, uuTmp);

      COMPILER_64_SHL(crc40,1);
      COMPILER_64_SET(uuTmp, 0x000000ff,0xffffffff);
      COMPILER_64_AND(crc40, uuTmp);
      COMPILER_64_XOR(crc40, b40);
    }
  }

  return crc40;
}


uint64 c3hppc_tmu_get_root_table_dump_buffer_entry( int nEntryIndex ) {
  uint64 uuAllOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
 if ( nEntryIndex < g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount ) 
   return g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer[nEntryIndex];
 else
   return uuAllOnes;

}

int c3hppc_tmu_get_root_table_dump_buffer_count( void ) {
  return g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount;
}


uint64 c3hppc_tmu_get_chain_table_dump_buffer_entry( int nEntryIndex ) {
  uint64 uuAllOnes = COMPILER_64_INIT(0xffffffff,0xffffffff);
 if ( nEntryIndex < g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryCount ) 
   return g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer[nEntryIndex];
 else
   return uuAllOnes;

}

int c3hppc_tmu_get_chain_table_dump_buffer_count( void ) {
  return g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryCount;
}


int c3hppc_tmu_get_eml_tables( int nUnit, int nCmdFifoSelect, int nRootTable, int nChainTable,
                               int nRootTableNumOfEntries, int nTimeOut ) {

  g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount = 0;
  g_c3hppcTmuUpdateManagerCB.nEmlChainTableDumpBufferEntryCount = 0;
  c3hppc_tmu_xl_read( nCmdFifoSelect, nRootTable, 0, nRootTableNumOfEntries, 0, NULL );
  c3hppc_tmu_xl_read( nCmdFifoSelect, nChainTable,0, nRootTableNumOfEntries, 0, NULL );

  while ( !c3hppc_tmu_are_cmd_fifos_empty() || !c3hppc_tmu_are_rsp_fifos_empty() ) {
    sal_sleep(1);
    if ( !(--nTimeOut) ) break;
  }

  return ( (!nTimeOut) ? 1 : 0  );
}



int c3hppc_tmu_display_andor_scoreboard_eml_tables( int nUnit, uint8 bDisplay, char *pFileName, int nRootTable,
                                                    int nChainTable, uint8 bScoreboard, int nMaxKey ) {
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  char *acKeyScoreBoard;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nKVpair, nRootTableIndex, nChainTableIndex, nWord, nIndex;
  uint64 uuResponseWord;
  uint32 uKey;
  int nRootTableEntrySizeIn64b;
  int nChainEntrySizeIn64b;
  int rc;
  uint32 uChainAllocatedCount, uFreeChainFifoFullSpace, uTotalFreeChains;
  char sEmlDumpFile[256];
  char *psEmlDumpFile = sEmlDumpFile;
  char *psTmp;
  uint32 uNL_LE;

  
  acKeyScoreBoard = NULL;
  rc = 0;
  uChainAllocatedCount = 0;
  uNL_LE = 0;

  if ( bDisplay ) {
    if ( (psTmp = sal_config_get("c3_eml_dump_path")) != NULL && sal_strlen(psTmp) < sizeof(sEmlDumpFile) ) {
      /* coverity[secure_coding] */
      sal_strcpy(sEmlDumpFile, psTmp );
      psEmlDumpFile = sEmlDumpFile + sal_strlen(sEmlDumpFile);
      /* coverity[secure_coding] */
      sal_strcpy(psEmlDumpFile, "/");
      psEmlDumpFile = sEmlDumpFile + sal_strlen(sEmlDumpFile);
      /* coverity[secure_coding] */
      sal_strcpy(psEmlDumpFile, pFileName); 
      c3hppcUtils_enable_output_to_file( sEmlDumpFile );
    } else {
      cli_out("\"c3_eml_dump_path\" config variable NOT set or OVERSIZED!!!\n");
      return -1;
    }
  }

  if ( bScoreboard ) {
    acKeyScoreBoard = (char *) sal_alloc( g_uEmlMaxProvisionedKey, "Scoreboard" ); 
    sal_memset( acKeyScoreBoard, 0x00, g_uEmlMaxProvisionedKey );
  }

  nChainEntrySizeIn64b = g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainElementSizeIn64b *
                             g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainLimit;

  nRootTableEntrySizeIn64b = g_c3hppcTmuUpdateManagerCB.aTableParameters[nRootTable].uEntrySizeIn64b;

  if ( nRootTableEntrySizeIn64b == 4 ) {
    for ( nIndex = 0; nIndex < g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount; ++nIndex ) {
      nRootTableIndex = nIndex / nRootTableEntrySizeIn64b;
      uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer[nIndex];
      if ( (nIndex % nRootTableEntrySizeIn64b) == 0 ) {
        EMLcontrolWord.value = uuResponseWord;
        uNL_LE = (EMLcontrolWord.bits.NL_LE_bits3to1 << 1) | EMLcontrolWord.bits.NL_LE_bit0;
        if ( bDisplay ) {
            cli_out("\n EML RootTable Entry 0x%07x: NextTable[%d] NextEntry[0x%06x] NL_LE[%d] NL_GT[%d] Splitter[0x%07x]\n",
                    nRootTableIndex, (uint32) EMLcontrolWord.bits.NextTable, (uint32) EMLcontrolWord.bits.NextEntry,
                    uNL_LE, (uint32) EMLcontrolWord.bits.NL_GT,
                    (uint32) EMLcontrolWord.bits.Splitter );
        }
        if ( uNL_LE != 0xf ) ++uChainAllocatedCount;
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 1 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
        if ( EMLcontrolWord.bits.Splitter != 0 ) {
          uKey = COMPILER_64_LO(uuResponseWord);
          assert( uKey < nMaxKey );  
          if ( bScoreboard ) ++acKeyScoreBoard[uKey];
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 2 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 3 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[119:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
  
        if ( EMLcontrolWord.bits.Splitter != 0 && uNL_LE != 0 && uNL_LE != 0xf ) {
          nChainTableIndex = (int) EMLcontrolWord.bits.NextEntry * nChainEntrySizeIn64b;
          if ( bDisplay ) {
              cli_out("\n");
          }
          for ( nKVpair = 0; nKVpair < (int) uNL_LE; ++nKVpair ) {
            for ( nWord = 0; nWord < g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainElementSizeIn64b; ++nWord ) {
              uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer[nChainTableIndex++];
              if ( nWord == 0 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
                uKey = COMPILER_64_LO(uuResponseWord);
                assert( uKey < nMaxKey );  
                if ( bScoreboard ) ++acKeyScoreBoard[uKey];
              } else if ( nWord == 1 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 2 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[119:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              }
            }
          }
        }
      }
    }
  }

  if ( nRootTableEntrySizeIn64b == 6 ) {
    for ( nIndex = 0; nIndex < g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount; ++nIndex ) {
      nRootTableIndex = nIndex / nRootTableEntrySizeIn64b;
      uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer[nIndex];
      if ( (nIndex % nRootTableEntrySizeIn64b) == 0 ) {
        EMLcontrolWord.value = uuResponseWord;
        uNL_LE = (EMLcontrolWord.bits.NL_LE_bits3to1 << 1) | EMLcontrolWord.bits.NL_LE_bit0;
        if ( bDisplay ) {
            cli_out("\n EML RootTable Entry 0x%07x: NextTable[%d] NextEntry[0x%06x] NL_LE[%d] NL_GT[%d] Splitter[0x%07x]\n",
                    nRootTableIndex, (uint32) EMLcontrolWord.bits.NextTable, (uint32) EMLcontrolWord.bits.NextEntry,
                    uNL_LE, (uint32) EMLcontrolWord.bits.NL_GT,
                    (uint32) EMLcontrolWord.bits.Splitter );
        }
        if ( uNL_LE != 0xf ) ++uChainAllocatedCount;
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 1 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
        if ( EMLcontrolWord.bits.Splitter != 0 ) {
          uKey = COMPILER_64_LO(uuResponseWord);
          assert( uKey < nMaxKey );  
          if ( bScoreboard ) ++acKeyScoreBoard[uKey];
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 2 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[127:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 3 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[175:128] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 4 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 5 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[119:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
  
        if ( EMLcontrolWord.bits.Splitter != 0 && uNL_LE != 0 && uNL_LE != 0xf ) {
          nChainTableIndex = (int) EMLcontrolWord.bits.NextEntry * nChainEntrySizeIn64b;
          if ( bDisplay ) {
              cli_out("\n");
          }
          for ( nKVpair = 0; nKVpair < (int) uNL_LE; ++nKVpair ) {
            for ( nWord = 0; nWord < g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainElementSizeIn64b; ++nWord ) {
              uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer[nChainTableIndex++];
              if ( nWord == 0 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
                uKey = COMPILER_64_LO(uuResponseWord);
                assert( uKey < nMaxKey );  
                if ( bScoreboard ) ++acKeyScoreBoard[uKey];
              } else if ( nWord == 1 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[127:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 2 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[175:128] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 3 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 4 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[119:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              }
            }
          }
        }
      }
    }
  }

  if ( nRootTableEntrySizeIn64b == 8 ) {
    for ( nIndex = 0; nIndex < g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount; ++nIndex ) {
      nRootTableIndex = nIndex / nRootTableEntrySizeIn64b;
      uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer[nIndex];
      if ( (nIndex % nRootTableEntrySizeIn64b) == 0 ) {
        EMLcontrolWord.value = uuResponseWord;
        uNL_LE = (EMLcontrolWord.bits.NL_LE_bits3to1 << 1) | EMLcontrolWord.bits.NL_LE_bit0;
        if ( bDisplay ) {
            cli_out("\n EML RootTable Entry 0x%07x: NextTable[%d] NextEntry[0x%06x] NL_LE[%d] NL_GT[%d] Splitter[0x%07x]\n",
                    nRootTableIndex, (uint32) EMLcontrolWord.bits.NextTable, (uint32) EMLcontrolWord.bits.NextEntry,
                    uNL_LE, (uint32) EMLcontrolWord.bits.NL_GT,
                    (uint32) EMLcontrolWord.bits.Splitter );
        }
        if ( uNL_LE != 0xf ) ++uChainAllocatedCount;
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 1 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
        if ( EMLcontrolWord.bits.Splitter != 0 ) {
          uKey = COMPILER_64_LO(uuResponseWord);
          assert( uKey < nMaxKey );  
          if ( bScoreboard ) ++acKeyScoreBoard[uKey];
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 2 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[127:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 3 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[182:128] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 4 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[246:183] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 5 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[303:247] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 6 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 7 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[119:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
  
        if ( EMLcontrolWord.bits.Splitter != 0 && uNL_LE != 0 && uNL_LE != 0xf ) {
          nChainTableIndex = (int) EMLcontrolWord.bits.NextEntry * nChainEntrySizeIn64b;
          if ( bDisplay ) {
              cli_out("\n");
          }
          for ( nKVpair = 0; nKVpair < (int) uNL_LE; ++nKVpair ) {
            for ( nWord = 0; nWord < g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainElementSizeIn64b; ++nWord ) {
              uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer[nChainTableIndex++];
              if ( nWord == 0 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
                uKey = COMPILER_64_LO(uuResponseWord);
                assert( uKey < nMaxKey );  
                if ( bScoreboard ) ++acKeyScoreBoard[uKey];
              } else if ( nWord == 1 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[127:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 2 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[182:128] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 3 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[246:183] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 4 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[303:247] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 5 ) {
                 if ( bDisplay ) {
                     cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[63:0] --> 0x%016llx\n",
                             (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                 }
              } else if ( nWord == 6 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[119:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              }
            }
          }
        }
      }
    }
  }

  if ( nRootTableEntrySizeIn64b == 10 ) {
    for ( nIndex = 0; nIndex < g_c3hppcTmuUpdateManagerCB.nEmlRootTableDumpBufferEntryCount; ++nIndex ) {
      nRootTableIndex = nIndex / nRootTableEntrySizeIn64b;
      uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlRootTableDumpBuffer[nIndex];
      if ( (nIndex % nRootTableEntrySizeIn64b) == 0 ) {
        EMLcontrolWord.value = uuResponseWord;
        uNL_LE = (EMLcontrolWord.bits.NL_LE_bits3to1 << 1) | EMLcontrolWord.bits.NL_LE_bit0;
        if ( bDisplay ) {
            cli_out("\n EML RootTable Entry 0x%07x: NextTable[%d] NextEntry[0x%06x] NL_LE[%d] NL_GT[%d] Splitter[0x%07x]\n",
                    nRootTableIndex, (uint32) EMLcontrolWord.bits.NextTable, (uint32) EMLcontrolWord.bits.NextEntry,
                    uNL_LE, (uint32) EMLcontrolWord.bits.NL_GT,
                    (uint32) EMLcontrolWord.bits.Splitter );
        }
        if ( uNL_LE != 0xf ) ++uChainAllocatedCount;
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 1 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
        if ( EMLcontrolWord.bits.Splitter != 0 ) {
          uKey = COMPILER_64_LO(uuResponseWord);
          assert( uKey < nMaxKey );  
          if ( bScoreboard ) ++acKeyScoreBoard[uKey];
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 2 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[127:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 3 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[182:128] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 4 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[246:183] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 5 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[310:247] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 6 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[374:311] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 7 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Key[423:375] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 8 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[63:0] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
      } else if ( (nIndex % nRootTableEntrySizeIn64b) == 9 ) {
        if ( bDisplay ) {
            cli_out(" EML RootTable Entry 0x%07x: Data[119:64] --> 0x%016llx\n", nRootTableIndex, uuResponseWord );
        }
  
        if ( EMLcontrolWord.bits.Splitter != 0 && uNL_LE != 0 && uNL_LE != 0xf ) {
          nChainTableIndex = (int) EMLcontrolWord.bits.NextEntry * nChainEntrySizeIn64b;
          if ( bDisplay ) {
              cli_out("\n");
          }
          for ( nKVpair = 0; nKVpair < (int) uNL_LE; ++nKVpair ) {
            for ( nWord = 0; nWord < g_c3hppcTmuUpdateManagerCB.aTableParameters[nChainTable].uEMLchainElementSizeIn64b; ++nWord ) {
              uuResponseWord = g_c3hppcTmuUpdateManagerCB.pEmlChainTableDumpBuffer[nChainTableIndex++];
              if ( nWord == 0 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
                uKey = COMPILER_64_LO(uuResponseWord);
                assert( uKey < nMaxKey );  
                if ( bScoreboard ) ++acKeyScoreBoard[uKey];
              } else if ( nWord == 1 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[127:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 2 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[182:128] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 3 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[246:183] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 4 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[310:247] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 5 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[374:311] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 6 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Key[423:375] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 7 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[63:0] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              } else if ( nWord == 8 ) {
                if ( bDisplay ) {
                    cli_out(" EML ChainTable Entry 0x%06x: KVpair[%d] Data[119:64] --> 0x%016llx\n",
                            (uint32) EMLcontrolWord.bits.NextEntry, nKVpair, uuResponseWord );
                }
              }
            }
          }
        }
      }
    }
  }

  if ( bDisplay ) c3hppcUtils_disable_output_to_file();

  if ( bScoreboard ) {
    for ( uKey = 0; uKey < nMaxKey; ++uKey ) {
      if ( acKeyScoreBoard[uKey] != 1 ) {
        cli_out("ERROR:  \"c3hppc_tmu_scoreboard\" --> Key 0x%04x had a hit count of %d\n", uKey, (int) acKeyScoreBoard[uKey] );
        rc = 1;
      }
    }
    sal_free( acKeyScoreBoard );

    uFreeChainFifoFullSpace = c3hppc_tmu_get_free_chain_fifos_full_space( nUnit );
    uTotalFreeChains = (C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM * C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_ENTRY_NUM);
    if ( (uFreeChainFifoFullSpace + uChainAllocatedCount) != uTotalFreeChains ) {
      cli_out("WARNING:  \"c3hppc_tmu_scoreboard\" --> Free Chain INCONSISTENCY:  FreeChainFifoFullSpace %d  ChainAllocatedCount %d  TotalFreeChains %d\n",
              uFreeChainFifoFullSpace, uChainAllocatedCount, uTotalFreeChains );
    } 
  }


  return rc;
#endif
}


int c3hppc_tmu_bulk_delete_setup( int nUnit, int nRootTable, uint32 *pKeyFilter, uint32 *pKeyFilterMask ) {

  uint32 uRegisterValue, uKeySizeField;

  uKeySizeField = 
      c3hppc_tmu_get_insert_delete_cmd_size_field( g_c3hppcTmuUpdateManagerCB.aTableParameters[nRootTable].uLookup );

  WRITE_TMB_UPDATER_BULK_DELETE_KEY_31_0r( nUnit, *pKeyFilter );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_63_32r( nUnit, 0x00000000 );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_95_64r( nUnit, 0x00000000 );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_127_96r( nUnit, 0x00000000 );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_159_128r( nUnit, 0x00000000 );
  /* I was lazy about this.  Hard coded the MSB of the key to be asserted in both the database setup and ucode.
     Hard coding it here as well.
  */
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_175_160r( nUnit, 0x00008000 );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_31_0r( nUnit, *pKeyFilterMask );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_63_32r( nUnit, 0xffffffff );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_95_64r( nUnit, 0xffffffff );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_127_96r( nUnit, 0xffffffff );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_159_128r( nUnit, 0xffffffff );
  WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_175_160r( nUnit, 0xffff );

/*  This is here because there is a bug in the QE in that it over-writes data bits 15:0
  WRITE_TMB_UPDATER_BULK_DELETE_DATA_31_0r( nUnit, *pKeyFilter );
  WRITE_TMB_UPDATER_BULK_DELETE_DATA_MASK_31_0r( nUnit, *pKeyFilterMask );
*/

  READ_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_BULK_DELETE_CONFIGr, &uRegisterValue, ROOT_TABLEf, (uint32) nRootTable );
  soc_reg_field_set( nUnit, TMB_UPDATER_BULK_DELETE_CONFIGr, &uRegisterValue, KEY_SIZEf, uKeySizeField );
  soc_reg_field_set( nUnit, TMB_UPDATER_BULK_DELETE_CONFIGr, &uRegisterValue, FILTER_ENf, 1 );
  WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, uRegisterValue );


  return 0;
}


int c3hppc_tmu_bulk_delete_start_scanner( int nUnit ) {

  uint32 uRegisterValue;

  READ_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_BULK_DELETE_CONFIGr, &uRegisterValue, GOf, 1 );
  WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_tmu_bulk_delete_cancel( int nUnit ) {

  uint32 uRegisterValue;

  READ_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_BULK_DELETE_CONFIGr, &uRegisterValue, FILTER_ENf, 0 );
  WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr( nUnit, uRegisterValue );

  return 0;
}


int c3hppc_tmu_wait_for_bulk_delete_done( int nUnit, int nTimeOutInSeconds ) {
  int rc;
  sal_time_t TimeStamp;

  rc = 0;
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, TMB_UPDATER_BULK_DELETE_EVENTr, DONEf, 1, 
                               nTimeOutInSeconds, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_tmu_wait_for_bulk_delete_done> -- TMB_UPDATER_BULK_DELETE_EVENT DONE event TIMEOUT!!!\n");
    rc = 1;
  }

  return rc;
}


uint32 c3hppc_tmu_get_free_chain_fifos_full_space( int nUnit ) {

  uint32 uFreeChainFifo, uFullSpace, uTotalFullSpace;

  for ( uFreeChainFifo = 0, uTotalFullSpace = 0; uFreeChainFifo < C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM; ++uFreeChainFifo ) {
    READ_TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr( nUnit, uFreeChainFifo, &uFullSpace );
    uTotalFullSpace += uFullSpace;
  }

  return uTotalFullSpace;
}


uint32 c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( int nNumberOfBucketPrefixes ) {
  uint32 uIPV4BucketEntrySizeIn64b;

  uIPV4BucketEntrySizeIn64b = ( nNumberOfBucketPrefixes / C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM ) *
                                 C3HPPC_TMU_IPV4_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b;
  if ( (nNumberOfBucketPrefixes % C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM) ) {
    uIPV4BucketEntrySizeIn64b += C3HPPC_TMU_IPV4_128b_BUCKET_TABLE_ENTRY_SIZE_IN_64b;
  }

  return uIPV4BucketEntrySizeIn64b;
}


uint32 c3hppc_tmu_calc_ipv6_bucket_table_entry_size_in_64b( int nNumberOfBucketPrefixes ) {
  uint32 uIPV6BucketEntrySizeIn64b;

  uIPV6BucketEntrySizeIn64b = ( nNumberOfBucketPrefixes / C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM ) *
                                 C3HPPC_TMU_IPV6_256b_BUCKET_TABLE_ENTRY_SIZE_IN_64b;
  if ( (nNumberOfBucketPrefixes % C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM) ) {
    uIPV6BucketEntrySizeIn64b += C3HPPC_TMU_IPV6_128b_BUCKET_TABLE_ENTRY_SIZE_IN_64b;
  }

  return uIPV6BucketEntrySizeIn64b;
}

int c3hppc_tmu__dump_cmic_rd_dma_state( int nUnit ) {
  int nIndex;
  uint32 uRegisterValue;
  char sPrintBuf[1024];
  char *pRegName;

  for ( nIndex = 0; nIndex < g_anCmicDmaStateRegistersCount; ++nIndex ) {
    soc_pci_getreg(nUnit, soc_reg_addr(nUnit, g_anCmicDmaStateRegisters[nIndex], REG_PORT_ANY, 0), &uRegisterValue);
    pRegName = SOC_REG_NAME(nUnit,g_anCmicDmaStateRegisters[nIndex]);
    soc_reg_sprint_data( nUnit, sPrintBuf, ",  ", g_anCmicDmaStateRegisters[nIndex], uRegisterValue );
    cli_out("\nRegister[%s] --> %s \n", pRegName, sPrintBuf);
  }

  for ( nIndex = 0; nIndex < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nIndex ) {
    READ_TMB_UPDATER_CMD_FIFO_DEPTHr( nUnit, nIndex, &uRegisterValue );
    cli_out("\n TMB_UPDATER_CMD_FIFO_DEPTH instance[%d] --> %d \n", nIndex, uRegisterValue);
  }

  for ( nIndex = 0; nIndex < C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM; ++nIndex ) {
    READ_TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr( nUnit, nIndex, &uRegisterValue );
    cli_out("\n TMB_UPDATER_FREE_CHAIN_FIFO_DEPTH instance[%d] --> %d \n", nIndex, uRegisterValue);
  }

  return 0;
}



void c3hppc_tmu_update_freelist_manager( void *pUpdateManagerCB_arg ) 
{

  int rc, nUnit, copyno;
  uint8 at;
  uint32 uRegisterValue;
  VOL uint32 *pRecycleRing;
  VOL uint32 *pRecycleRingLimit;
  VOL uint32 *pRecycleRingEntry;
  uint32 uProcessedEntryNum, uTotalProcessedEntryNum;
  uint32 uDataBeats, uEntryNum, uRecycleRingSize, uTable, uTableEntryIndex;
  uint32 uFreeChainFifo, uFullSpace, uNextTableToService, uNextTableToService_temp;
  uint32 *pDmaData, nDmaIndex, uFreeListNextEntry, uDmaEntryNum;
  int nFreeSpace;
  schan_msg_t msg;
  c3hppc_tmu_update_manager_cb_t *pFreeListRingManagerCB;
  soc_mem_t TmuUpdaterRecycleFifoMem = TMB_UPDATER_RECYCLE_CHAIN_FIFOm;
  soc_mem_t aTmuUpdaterFreeChainFifoMem[] = { TMB_UPDATER_FREE_CHAIN_FIFO0m, TMB_UPDATER_FREE_CHAIN_FIFO1m,
                                              TMB_UPDATER_FREE_CHAIN_FIFO2m, TMB_UPDATER_FREE_CHAIN_FIFO3m };

 
  cli_out("  Entering TMU Updater FREELIST Manager thread .... \n\n");
  rc = 0;


  pFreeListRingManagerCB = (c3hppc_tmu_update_manager_cb_t *) pUpdateManagerCB_arg;
  nUnit = pFreeListRingManagerCB->nUnit;
  uNextTableToService = C3HPPC_TMU_TABLE_NUM;


  READ_TMB_UPDATER_FIFO_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_FIFO_CONFIGr, &uRegisterValue, FREE_CHAIN_FIFO_AEMPTY_THRESHf, 0 );
  soc_reg_field_set( nUnit, TMB_UPDATER_FIFO_CONFIGr, &uRegisterValue, FREE_CHAIN_FIFO_AFULL_THRESHf, 0x3ff );
  WRITE_TMB_UPDATER_FIFO_CONFIGr( nUnit, uRegisterValue );



  uRecycleRingSize = 1024;
/* Coverity
  switch (uRecycleRingSize) {
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
  uEntryNum = 4;
  uDataBeats = soc_mem_entry_words( nUnit, TmuUpdaterRecycleFifoMem );

  /*
   * Setup CMIC_CMC0_FIFO_CH1_RD_DMA for recycle ring processing
  */

  copyno = SOC_MEM_BLOCK_ANY(nUnit, TmuUpdaterRecycleFifoMem );
  uRegisterValue = soc_mem_addr_get( nUnit, TmuUpdaterRecycleFifoMem, 0, copyno, 0, &at);
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );

  schan_msg_clear(&msg);
  msg.readcmd.header.v3.opcode = FIFO_POP_CMD_MSG;
  msg.readcmd.header.v3.dst_blk = SOC_BLOCK2SCH( nUnit, copyno );
  msg.readcmd.header.v3.data_byte_len = uDataBeats * sizeof(uint32);
  /* Set 1st schan ctrl word as opcode */
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );

  pRecycleRing = (uint32 *) soc_cm_salloc( nUnit, (uRecycleRingSize * uDataBeats * sizeof(uint32)), "recycle_ring");
  pRecycleRingLimit = pRecycleRing + (uRecycleRingSize * uDataBeats);
  sal_memset( (void *)pRecycleRing, 0, (uRecycleRingSize * uDataBeats * sizeof(uint32)) );
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pRecycleRing)) );

  READ_CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr, &uRegisterValue, BEAT_COUNTf, uDataBeats );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr, &uRegisterValue, HOST_NUM_ENTRIES_SELf, uEntryNum );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr, &uRegisterValue, ENDIANESSf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uRecycleRingSize-64) );

  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr, &uRegisterValue, ENABLEf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );

  pRecycleRingEntry = pRecycleRing;

  uTotalProcessedEntryNum = 0;

  uDmaEntryNum = C3HPPC_TMU_UPDATE_FREECHAIN_DMA_ENTRY_NUM;
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, (uDmaEntryNum * sizeof(uint32)), "free chain dma buffer" );

  while ( !pFreeListRingManagerCB->bExit ) {

    uProcessedEntryNum = 0;

#if (defined(LINUX))
    soc_cm_sinval( nUnit, (void *)pRecycleRingEntry, WORDS2BYTES(uDataBeats) );
#endif

    while ( *pRecycleRingEntry ) {

      assert( pRecycleRingEntry < pRecycleRingLimit );

      ++uProcessedEntryNum;


      soc_mem_field_get( nUnit, TmuUpdaterRecycleFifoMem, (void *)pRecycleRingEntry, N_TABLEf, &uTable );
      soc_mem_field_get( nUnit, TmuUpdaterRecycleFifoMem, (void *)pRecycleRingEntry, N_ENTRYf, &uTableEntryIndex );
      pFreeListRingManagerCB->aTableParameters[uTable].FreeList[uTableEntryIndex] = 
                                       pFreeListRingManagerCB->aTableParameters[uTable].uFreeListNextEntry;
      pFreeListRingManagerCB->aTableParameters[uTable].uFreeListNextEntry = uTableEntryIndex;
      if ( uNextTableToService == C3HPPC_TMU_TABLE_NUM ) uNextTableToService = uTable;

      *pRecycleRingEntry = 0;
      pRecycleRingEntry += uDataBeats;

      if ( pRecycleRingEntry == pRecycleRingLimit ) {
        pRecycleRingEntry = pRecycleRing;
/*
        cli_out("  RSP_FIFO%d ring wrapped!\n", nRspFifo);
*/
      }
#if (defined(LINUX))
      soc_cm_sinval( nUnit, (void *)pRecycleRingEntry, WORDS2BYTES(uDataBeats) );
#endif

    }

    if ( uProcessedEntryNum ) {
      WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
      uTotalProcessedEntryNum += uProcessedEntryNum;

/*
      cli_out("  Processed %d out of %d total RECYCLE ring elements.\n", uProcessedEntryNum, uTotalProcessedEntryNum );
*/

    }

    if ( pFreeListRingManagerCB->bPopulateFreeListFifos ) {
      pFreeListRingManagerCB->bPopulateFreeListFifos = 0;
      for ( uNextTableToService = 0; uNextTableToService < C3HPPC_TMU_TABLE_NUM; ++uNextTableToService ) {
        if ( pFreeListRingManagerCB->aTableParameters[uNextTableToService].FreeList &&
             pFreeListRingManagerCB->aTableParameters[uNextTableToService].uFreeListNextEntry != C3HPPC_TMU_UPDATE_FREELIST_EMPTY ) {
          break;
        }
      }
    }


    if ( uNextTableToService != C3HPPC_TMU_TABLE_NUM ) {
      for ( uFreeChainFifo = 0; uFreeChainFifo < C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_NUM; ++uFreeChainFifo ) {
        READ_TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr( nUnit, uFreeChainFifo, &uFullSpace );
#if 0
        if ( uFullSpace != 1024 ) {
            cli_out("  FREECHAIN Fifo%d FULLSPACE %d  uNextTableToService %d\n", uFreeChainFifo, uFullSpace, uNextTableToService);
        }
#endif
        nFreeSpace = C3HPPC_TMU_UPDATE_FREECHAIN_FIFO_ENTRY_NUM - uFullSpace;
        if ( nFreeSpace < 0 ) {
          c3hppc_tmu__dump_cmic_rd_dma_state( nUnit );
          ++pFreeListRingManagerCB->nErrorCounter;
        }
        if ( nFreeSpace > 0 && (uNextTableToService_temp = c3hppc_tmu_are_buffers_available_for_this_free_chain_fifo(uFreeChainFifo)) != 0 ) {
          uNextTableToService = uNextTableToService_temp;
          nDmaIndex = 0;
          for ( nDmaIndex = 0; 
                (nDmaIndex < C3HPPC_MIN( nFreeSpace, uDmaEntryNum ) &&
                 pFreeListRingManagerCB->aTableParameters[uNextTableToService].uFreeListNextEntry != C3HPPC_TMU_UPDATE_FREELIST_EMPTY);
                ++nDmaIndex ) {
            uFreeListNextEntry = pFreeListRingManagerCB->aTableParameters[uNextTableToService].uFreeListNextEntry;
            soc_mem_field_set( nUnit, aTmuUpdaterFreeChainFifoMem[uFreeChainFifo],
                               (pDmaData + nDmaIndex), N_TABLEf, &uNextTableToService );
            soc_mem_field_set( nUnit, aTmuUpdaterFreeChainFifoMem[uFreeChainFifo],
                               (pDmaData + nDmaIndex), N_ENTRYf, &uFreeListNextEntry );
            pFreeListRingManagerCB->aTableParameters[uNextTableToService].uFreeListNextEntry =
                 pFreeListRingManagerCB->aTableParameters[uNextTableToService].FreeList[uFreeListNextEntry];
          }
          /*    coverity[negative_returns : FALSE]    */
          rc = soc_mem_write_range( nUnit, aTmuUpdaterFreeChainFifoMem[uFreeChainFifo], MEM_BLOCK_ANY, 0,
                                    (nDmaIndex - 1), (void *) pDmaData);
          if ( rc ) {
            c3hppc_tmu__dump_cmic_rd_dma_state( nUnit );
            ++pFreeListRingManagerCB->nErrorCounter;
          }
/*
          cli_out("  %d %s written to FREECHAIN Fifo%d for table %d.\n", nDmaIndex, ((nDmaIndex == 1) ? "entry was" : "entries were"),
                  uFreeChainFifo, uNextTableToService );
*/

          for ( uTable = 0; uTable < C3HPPC_TMU_TABLE_NUM; ++uTable ) {
            uNextTableToService = (uNextTableToService + 1) & (C3HPPC_TMU_TABLE_NUM - 1);
            if ( pFreeListRingManagerCB->aTableParameters[uNextTableToService].FreeList &&
                 pFreeListRingManagerCB->aTableParameters[uNextTableToService].uFreeListNextEntry != C3HPPC_TMU_UPDATE_FREELIST_EMPTY ) {
              break;
            }
          }
          if ( uTable == C3HPPC_TMU_TABLE_NUM ) {
            uNextTableToService = C3HPPC_TMU_TABLE_NUM;
            break;
          }
        }
      }
    }

#if (defined(LINUX))
    sal_usleep(1);
#else
    sal_sleep(0);
#endif
  }


  soc_cm_sfree( nUnit, (void *)pRecycleRing );
  for ( uTable = 0; uTable < C3HPPC_TMU_TABLE_NUM; ++uTable ) {
    if ( pFreeListRingManagerCB->aTableParameters[uTable].FreeList ) { 
      sal_free( pFreeListRingManagerCB->aTableParameters[uTable].FreeList );
    }
  }
  soc_cm_sfree( nUnit, pDmaData );

  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr, &uRegisterValue, ABORTf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr( nUnit, uRegisterValue );

  sal_usleep(1);

  WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_CFGr( nUnit, 0 );

  cli_out("  Exiting TMU Updater FREELIST Manager thread .... \n\n");

  sal_thread_exit(0); 

  return;
}


uint32 c3hppc_tmu_are_buffers_available_for_this_free_chain_fifo( uint32 uFreeChainFifo )
{
  uint32 uTable = 0;

  for ( uTable = 0; uTable < C3HPPC_TMU_TABLE_NUM; ++uTable ) {
    if ( g_c3hppcTmuUpdateManagerCB.aTableParameters[uTable].FreeList &&
         g_c3hppcTmuUpdateManagerCB.aTableParameters[uTable].uFreeListNextEntry != C3HPPC_TMU_UPDATE_FREELIST_EMPTY &&
         (g_c3hppcTmuUpdateManagerCB.aTableParameters[uTable].uEMLchainPoolMask & (1 << uFreeChainFifo)) ) {
      break;
    }
  }

  return ( uTable != C3HPPC_TMU_TABLE_NUM ? uTable : 0);
}


void c3hppc_tmu_update_rsp_manager( void *pUpdateManagerCB_arg ) 
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return;

#else
  int nUnit, copyno, nCount;
  uint8 at;
  uint32 uRegisterValue;
  VOL uint32 *pRspRing[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  VOL uint32 *pRspRingLimit[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  VOL uint32 *pRspRing_31to0Entry[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  VOL uint32 *pRspRing_LookAheadForNextRsp[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  VOL uint32 *pRspRing_63to32Entry[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  uint32 uProcessedEntryNum, uTotalProcessedEntryNum[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  uint32 uTotalTrailersNum[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  uint32 uReadRspInProgress[C3HPPC_TMU_UPDATE_RSP_FIFO_NUM];
  uint32 uDataBeats, uEntryNum, uRspRingSize;
  c3hppc_tmu_updater_response_ut ResponseWord;
  schan_msg_t msg;
  c3hppc_tmu_update_manager_cb_t *pRspRingManagerCB;
  soc_mem_t aTmuUpdaterRspFifoMem[] = { TMB_UPDATER_RSP_FIFO0m, TMB_UPDATER_RSP_FIFO1m };
  int nRspFifo;
  uint64 uuExpectResponseWord;
 
  

  cli_out("  Entering TMU Updater RESPONSE Manager thread .... \n\n");



  pRspRingManagerCB = (c3hppc_tmu_update_manager_cb_t *) pUpdateManagerCB_arg;
  nUnit = pRspRingManagerCB->nUnit;


  uRspRingSize = 4096;
/* Coverity
  switch (uRspRingSize) {
    case 64:    uEntryNum = 0; break;
    case 128:   uEntryNum = 1; break;
    case 256:   uEntryNum = 2; break;
    case 512:   uEntryNum = 3; break;
    case 1024:  uEntryNum = 4; break;
    case 2048:  uEntryNum = 5; break;
    case 4096:  uEntryNum = 6; break;
    case 8192:  uEntryNum = 7; break;
    case 16384: uEntryNum = 8; break;
  }
*/
  uEntryNum = 6;
  uDataBeats = soc_mem_entry_words( nUnit, aTmuUpdaterRspFifoMem[0] );

  /*
   * Setup CMIC_CMC0_FIFO_CH2_RD_DMA and CMIC_CMC0_FIFO_CH3_RD_DMA for update response ring processing
  */
  for ( nRspFifo = 0; nRspFifo < C3HPPC_TMU_UPDATE_RSP_FIFO_NUM; ++nRspFifo ) {

    copyno = SOC_MEM_BLOCK_ANY(nUnit, aTmuUpdaterRspFifoMem[nRspFifo] );
    uRegisterValue = soc_mem_addr_get( nUnit, aTmuUpdaterRspFifoMem[nRspFifo], 0, copyno, 0, &at);
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );

    schan_msg_clear(&msg);
    msg.readcmd.header.v3.opcode = FIFO_POP_CMD_MSG;
    msg.readcmd.header.v3.dst_blk = SOC_BLOCK2SCH( nUnit, copyno );
    msg.readcmd.header.v3.data_byte_len = uDataBeats * sizeof(uint32);
    /* Set 1st schan ctrl word as opcode */
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );

    pRspRing[nRspFifo] = (uint32 *) soc_cm_salloc( nUnit, (uRspRingSize * uDataBeats * sizeof(uint32)), "response_ring");
    pRspRingLimit[nRspFifo] = pRspRing[nRspFifo] + (uRspRingSize * uDataBeats);
    sal_memset( (void *)pRspRing[nRspFifo], 0, (uRspRingSize * uDataBeats * sizeof(uint32)) );
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pRspRing[nRspFifo])) );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pRspRing[nRspFifo])) );

    if ( nRspFifo )
      READ_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, &uRegisterValue );
    else
      READ_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, BEAT_COUNTf, uDataBeats );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, HOST_NUM_ENTRIES_SELf, uEntryNum );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, ENDIANESSf, 1 );
    if ( nRspFifo ) {
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uRspRingSize-64) );
    } else {
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uRspRingSize-64) );
    }

    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, ENABLEf, 1 );
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, uRegisterValue );

    pRspRing_31to0Entry[nRspFifo] = pRspRing[nRspFifo];

    uTotalProcessedEntryNum[nRspFifo] = 0;
    uTotalTrailersNum[nRspFifo] = 0;

    uReadRspInProgress[nRspFifo] = 0;

    pRspRing_LookAheadForNextRsp[nRspFifo] = NULL;
  }

  while ( !pRspRingManagerCB->bExit ) {

    for ( nRspFifo = 0; nRspFifo < C3HPPC_TMU_UPDATE_RSP_FIFO_NUM; ++nRspFifo ) {
      uProcessedEntryNum = 0;

#if (defined(LINUX))
      soc_cm_sinval( nUnit, (void *)pRspRing_31to0Entry[nRspFifo], WORDS2BYTES(uDataBeats) );
      if ( uReadRspInProgress[nRspFifo] ) {
        soc_cm_sinval( nUnit, (void *)pRspRing_LookAheadForNextRsp[nRspFifo], WORDS2BYTES(uDataBeats) );
      }
#endif

      while ( *pRspRing_31to0Entry[nRspFifo] || *(pRspRing_31to0Entry[nRspFifo] + 1) ||
              (uReadRspInProgress[nRspFifo] && 
               (*pRspRing_LookAheadForNextRsp[nRspFifo] || *(pRspRing_LookAheadForNextRsp[nRspFifo] + 1))) ) {

        assert( pRspRingManagerCB->nExpectRspRingRdPtr[nRspFifo] < C3HPPC_TMU_EXPECT_RSP_RING_SIZE );
        assert( pRspRing_31to0Entry[nRspFifo] < pRspRingLimit[nRspFifo] );
        nCount =  c3hppc_tmu_get_expect_rsp_ring_count( pRspRingManagerCB->nExpectRspRingWrPtr[nRspFifo],
                                                        pRspRingManagerCB->nExpectRspRingRdPtr[nRspFifo] );
        assert( nCount >= 0 && nCount <= C3HPPC_TMU_EXPECT_RSP_RING_SIZE );


        pRspRing_63to32Entry[nRspFifo] = pRspRing_31to0Entry[nRspFifo] + 1;
      
        ++uProcessedEntryNum;

        COMPILER_64_SET(ResponseWord.value, *pRspRing_63to32Entry[nRspFifo], *pRspRing_31to0Entry[nRspFifo]);

        if ( ResponseWord.bits.Op != C3HPPC_TMU_UPDATE_COMMAND__TRAILER || uReadRspInProgress[nRspFifo] ) {
          uint64 uuTmp;
          assert( nCount != 0 );
          uuExpectResponseWord = pRspRingManagerCB->ExpectRspRing[nRspFifo][pRspRingManagerCB->nExpectRspRingRdPtr[nRspFifo]];
          COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuExpectResponseWord), COMPILER_64_LO(uuExpectResponseWord));
          COMPILER_64_AND(uuTmp, C3HPPC_TMU_SKIP_RSP_COMPARE_MASK);
          if ( COMPILER_64_EQ(uuTmp, C3HPPC_TMU_SKIP_RSP_COMPARE) ) {
  
            if ( COMPILER_64_EQ(uuExpectResponseWord, C3HPPC_TMU_SKIP_RSP_COMPARE_EML_ROOT) ) {
              if ( pRspRingManagerCB->nEmlRootTableDumpBufferEntryCount < pRspRingManagerCB->nEmlRootTableDumpBufferEntryNum ) {
                COMPILER_64_SET(pRspRingManagerCB->pEmlRootTableDumpBuffer[pRspRingManagerCB->nEmlRootTableDumpBufferEntryCount++],
                                COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
              }
            } else if ( COMPILER_64_EQ(uuExpectResponseWord, C3HPPC_TMU_SKIP_RSP_COMPARE_EML_CHAIN) ) {
              if ( pRspRingManagerCB->nEmlChainTableDumpBufferEntryCount < pRspRingManagerCB->nEmlChainTableDumpBufferEntryNum ) {
                COMPILER_64_SET(pRspRingManagerCB->pEmlChainTableDumpBuffer[pRspRingManagerCB->nEmlChainTableDumpBufferEntryCount++],
                                COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
              }
            }
           
          } else if ( COMPILER_64_NE(ResponseWord.value, uuExpectResponseWord) ) {

            if ( pRspRingManagerCB->nErrorCounter < 64 ) {
              cli_out(" \"RSP_FIFO%d\" MISCOMPARE --> Actual: 0x%016llx  Expect: 0x%016llx \n", nRspFifo, ResponseWord.value,
                      uuExpectResponseWord );
            }

            ++pRspRingManagerCB->nErrorCounter;

            if ( SAL_BOOT_QUICKTURN ) {
              pRspRingManagerCB->bExit = 1;
              break;
            }
          } 

          pRspRingManagerCB->nExpectRspRingRdPtr[nRspFifo] =
            (pRspRingManagerCB->nExpectRspRingRdPtr[nRspFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 
  
        } else if ( !uReadRspInProgress[nRspFifo] ) {
          ++uTotalTrailersNum[nRspFifo];
        }

        *pRspRing_31to0Entry[nRspFifo] = 0;
        *pRspRing_63to32Entry[nRspFifo] = 0;

        pRspRing_31to0Entry[nRspFifo] += uDataBeats;

        if ( pRspRing_31to0Entry[nRspFifo] == pRspRingLimit[nRspFifo] ) {
          pRspRing_31to0Entry[nRspFifo] = pRspRing[nRspFifo];
/*
          cli_out("  RSP_FIFO%d ring wrapped!\n", nRspFifo);
*/
        }

#if (defined(LINUX))
        soc_cm_sinval( nUnit, (void *)pRspRing_31to0Entry[nRspFifo], WORDS2BYTES(uDataBeats) );
#endif

        if ( uReadRspInProgress[nRspFifo] ) {
          --uReadRspInProgress[nRspFifo];
        } else if ( ResponseWord.bits.Op == C3HPPC_TMU_UPDATE_COMMAND__XL_READ ) {
          uReadRspInProgress[nRspFifo] = (uint32) ResponseWord.bits.Size_Val01;
          pRspRing_LookAheadForNextRsp[nRspFifo] = pRspRing_31to0Entry[nRspFifo] + (uReadRspInProgress[nRspFifo] * uDataBeats);
          if ( pRspRing_LookAheadForNextRsp[nRspFifo] >= pRspRingLimit[nRspFifo] ) {
            pRspRing_LookAheadForNextRsp[nRspFifo] = pRspRing[nRspFifo] +
                          (pRspRing_LookAheadForNextRsp[nRspFifo] - pRspRingLimit[nRspFifo]); 
          }
#if (defined(LINUX))
          soc_cm_sinval( nUnit, (void *)pRspRing_LookAheadForNextRsp[nRspFifo], WORDS2BYTES(uDataBeats) );
#endif
        }

      }

      if ( uProcessedEntryNum ) {
        if ( nRspFifo )
          WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
        else
          WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
        uTotalProcessedEntryNum[nRspFifo] += uProcessedEntryNum;

/*
        cli_out("  Processed %d out of %d total RSP%d ring elements with %d trailers.\n", uProcessedEntryNum,
                uTotalProcessedEntryNum[nRspFifo], nRspFifo, uTotalTrailersNum[nRspFifo] );
*/
      }

    } /* for ( nRspFifo = 0; nRspFifo < C3HPPC_TMU_UPDATE_RSP_FIFO_NUM; ++nRspFifo ) { */

#if (defined(LINUX))
    sal_usleep(1);
#else
    sal_sleep(0);
#endif
  }


  for ( nRspFifo = 0; nRspFifo < C3HPPC_TMU_UPDATE_RSP_FIFO_NUM; ++nRspFifo ) {
    soc_cm_sfree( nUnit, (void *)pRspRing[nRspFifo] );

    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, ABORTf, 1 );
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, uRegisterValue );

    sal_usleep(1);

    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, 0 );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, 0 );

  }

  sal_free( pRspRingManagerCB->pEmlRootTableDumpBuffer );
  sal_free( pRspRingManagerCB->pEmlChainTableDumpBuffer );

  cli_out("  Exiting TMU Updater RESPONSE Manager thread .... \n\n");

  sal_thread_exit(0); 

  return;
#endif
}

int c3hppc_tmu_get_expect_rsp_ring_count( int nWrPtr, int nRdPtr ) {
  int nCount;

  if ( nWrPtr >= nRdPtr ) {
    nCount = nWrPtr - nRdPtr;
  } else {
    nCount = (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - nRdPtr) + nWrPtr;
  }

  return nCount;
}

uint32 c3hppc_tmu_get_insert_delete_cmd_size_field( uint32 uLookup ) {
  uint32 uSize;

/*  Left this here as a comment ...
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = 0; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = 1; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = 2; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = 3; break;
  }
*/
  uSize = uLookup;

  return uSize;
}

uint32 c3hppc_tmu_get_eml_root_table_entry_size_in_64b( uint32 uLookup ) {
  uint32 uSize;

  uSize = 0;
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = C3HPPC_TMU_EML176_ROOT_TABLE_ENTRY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = C3HPPC_TMU_EML304_ROOT_TABLE_ENTRY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = C3HPPC_TMU_EML424_ROOT_TABLE_ENTRY_SIZE_IN_64b; break;
  }

  return uSize;
}

uint32 c3hppc_tmu_get_eml_chain_table_chain_element_entry_size_in_64b( uint32 uLookup ) {
  uint32 uSize;

  uSize = 0;
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = C3HPPC_TMU_EML176_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = C3HPPC_TMU_EML304_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = C3HPPC_TMU_EML424_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b; break;
  }

  return uSize;
}

uint32 c3hppc_tmu_get_eml_insert_cmd_entry_size_in_64b( uint32 uLookup ) {
  uint32 uSize;

  uSize = 0;
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = C3HPPC_TMU_EML176_INSERT_COMMAND_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = C3HPPC_TMU_EML304_INSERT_COMMAND_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = C3HPPC_TMU_EML424_INSERT_COMMAND_SIZE_IN_64b; break;
  }

  return uSize;
}

uint32 c3hppc_tmu_get_eml_key_size_in_bytes( uint32 uLookup ) {
  uint32 uSize;

  uSize = 0;
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = C3HPPC_TMU_EML64_KEY_SIZE_IN_BYTES; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = C3HPPC_TMU_EML176_KEY_SIZE_IN_BYTES; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = C3HPPC_TMU_EML304_KEY_SIZE_IN_BYTES; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = C3HPPC_TMU_EML424_KEY_SIZE_IN_BYTES; break;
  }

  return uSize;
}

uint32 c3hppc_tmu_get_eml_key_size_in_64b( uint32 uLookup ) {
  uint32 uSize;

  uSize = 0;
  switch ( uLookup ) {
    case C3HPPC_TMU_LOOKUP__1ST_EML64:  uSize = C3HPPC_TMU_EML64_KEY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML176: uSize = C3HPPC_TMU_EML176_KEY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML304: uSize = C3HPPC_TMU_EML304_KEY_SIZE_IN_64b; break;
    case C3HPPC_TMU_LOOKUP__1ST_EML424: uSize = C3HPPC_TMU_EML424_KEY_SIZE_IN_64b; break;
  }

  return uSize;
}

void c3hppc_tmu_update_cmd_manager( void *pUpdateManagerCB_arg ) 
{

  int rc, nUnit, nCount;
  c3hppc_tmu_update_manager_cb_t *pUpdateManagerCB;
  uint32 auSequenceNumber[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  soc_mem_t aTmuUpdaterCmdFifoMem[] = { TMB_UPDATER_CMD_FIFO0m, TMB_UPDATER_CMD_FIFO1m };
  c3hppc_tmu_updater_xl_write_command_ut xlWriteCmd;
  c3hppc_tmu_updater_xl_read_command_ut xlReadCmd;
  c3hppc_tmu_updater_nop_command_ut xlNopCmd;
  c3hppc_tmu_updater_taps_command_ut TapsCmd;
  c3hppc_tmu_updater_eml_insert_command_ut emlInsertCmd;
  c3hppc_tmu_updater_eml_delete_command_ut emlDeleteCmd;
  c3hppc_tmu_updater_lock_release_command_ut LockReleaseCmd;
  c3hppc_tmu_updater_response_ut ResponseWord; 
  int nIndex, nCmdFifo;
  c3hppc_tmu_update_command_info_t *pActiveUpdateCmdInfo[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 *pDmaData, nDmaIndex;
  int nCmdFifoFreeSpace, nExpectRspRingFreeSpace;
  int anSingleEntryCmdSize[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM] = {0};
  int anOperationSize[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM] = {0};
  uint32 auTableDataDmaIndex[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM] = {0};
  int  anRspSize[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  int    nNopCount, nNop, nReadCount;
  int anCommandsPerEntry[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM], nCmd;
  uint32 au1KbLimitSizeIn64b[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM] = {C3HPPC_TMU_UPDATE_OPERATION_LIMIT_IN_64b_WORDS,C3HPPC_TMU_UPDATE_OPERATION_LIMIT_IN_64b_WORDS};
  uint32 auEntrySizeIn64b_RemainingCount[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM] = {0}, uRemainingCount, uOffset;
  uint32 uEntrySizeIn64b;
  sal_time_t  now, PreviousTime;
 

/*
  sal_usecs_t start_time;
  int diff_time, total_diff_times, total_dma_ops;
  int first_delete_command;
  int init_average;
  int thread_wakeup_average;
  sal_usecs_t last_wakeup, wakeup;
  int first_wakeup;
  int thread_wakeups;
  int dma_size_average;
  int total_dma_size;

  dma_size_average = 0;
  total_dma_size = 0;
  total_diff_times = total_dma_ops = 0;
  first_delete_command = 0;
  init_average = 0;

  first_wakeup = 1;
  thread_wakeups = 0;
  thread_wakeup_average = 0;
  last_wakeup = 0;
*/
  

  cli_out("  Entering TMU Updater COMMAND Manager thread .... \n\n");
  rc = 0;

  PreviousTime = sal_time();

  pUpdateManagerCB = (c3hppc_tmu_update_manager_cb_t *) pUpdateManagerCB_arg;
  nUnit = pUpdateManagerCB->nUnit;

/*
  READ_TMB_UPDATER_FIFO_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_FIFO_CONFIGr, &uRegisterValue, CMD_FIFO_AFULL_THRESHf, 1 );
  WRITE_TMB_UPDATER_FIFO_CONFIGr( nUnit, uRegisterValue );
*/

  for ( nCmdFifo = 0; nCmdFifo < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifo ) {
    pActiveUpdateCmdInfo[nCmdFifo] = NULL;
    auSequenceNumber[nCmdFifo] = 0;
    anRspSize[nCmdFifo] = 0;
    anCommandsPerEntry[nCmdFifo] = 0;
  }

  COMPILER_64_ZERO(LockReleaseCmd.value);
  COMPILER_64_ZERO(xlNopCmd.value);
  COMPILER_64_ZERO(TapsCmd.value);
  TapsCmd.bits.Op = C3HPPC_TMU_UPDATE_COMMAND__TAPS;
  COMPILER_64_ZERO(emlInsertCmd.value);
  emlInsertCmd.bits.Op = C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT_BEGIN;
  COMPILER_64_ZERO(emlDeleteCmd.value);
  emlDeleteCmd.bits.Op = C3HPPC_TMU_UPDATE_COMMAND__EML_DELETE;
  COMPILER_64_ZERO(xlWriteCmd.value);
  xlWriteCmd.bits.Op = C3HPPC_TMU_UPDATE_COMMAND__XL_WRITE;
  COMPILER_64_ZERO(xlReadCmd.value);
  xlReadCmd.bits.Op = C3HPPC_TMU_UPDATE_COMMAND__XL_READ;

  pDmaData = (uint32 *) soc_cm_salloc( nUnit, (C3HPPC_TMU_UPDATE_CMD_FIFO_SIZE_IN_64b_WORDS * sizeof(uint64)), "cmd fifo" );


  while ( !pUpdateManagerCB->bExit ) {

/*
    if ( first_wakeup ) {
      first_wakeup = 0;
      last_wakeup = sal_time_usecs(); 
    } else {
      wakeup = sal_time_usecs();
      thread_wakeup_average += SAL_USECS_SUB(wakeup, last_wakeup); 
      ++thread_wakeups;  
      last_wakeup = wakeup;
    }
*/

    /*  TMU cache count capture daemon.  Collect stats every 1 second.  */
    now = sal_time();
    if ( (now - PreviousTime) != 0 ) {
      PreviousTime = now;
      c3hppc_tmu_collect_cache_hit_counts( nUnit, 0 );
      c3hppc_tmu_collect_cache_miss_counts( nUnit, 0 );
      c3hppc_tmu_collect_cache_hit_for_pending_counts( nUnit, 0 );
    }  

    for ( nCmdFifo = 0; nCmdFifo < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifo ) {

      if ( pUpdateManagerCB->nUpdateCmdQCount[nCmdFifo] || pActiveUpdateCmdInfo[nCmdFifo] != NULL ) { 

        if ( pActiveUpdateCmdInfo[nCmdFifo] == NULL ) {
          pActiveUpdateCmdInfo[nCmdFifo] = &(pUpdateManagerCB->UpdateCmdQ[nCmdFifo][pUpdateManagerCB->nUpdateCmdQRdPtr[nCmdFifo]]);
          anSingleEntryCmdSize[nCmdFifo] = 1;
          anRspSize[nCmdFifo] = 1;
          anCommandsPerEntry[nCmdFifo] = 1;

          if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__XL_WRITE ) {

            if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
              anCommandsPerEntry[nCmdFifo] = 
                         pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEML1KbCommandsPerChainEntry;
              anRspSize[nCmdFifo] = anCommandsPerEntry[nCmdFifo];
              anSingleEntryCmdSize[nCmdFifo] = anCommandsPerEntry[nCmdFifo];
              au1KbLimitSizeIn64b[nCmdFifo] = 
                              pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEML1KbChainLimit *
                              pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainElementSizeIn64b;
            } else {
              au1KbLimitSizeIn64b[nCmdFifo] = C3HPPC_TMU_UPDATE_OPERATION_LIMIT_IN_64b_WORDS;
              uEntrySizeIn64b = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEntrySizeIn64b;
              if ( uEntrySizeIn64b > au1KbLimitSizeIn64b[nCmdFifo] ) {
                anCommandsPerEntry[nCmdFifo] = ( uEntrySizeIn64b + (au1KbLimitSizeIn64b[nCmdFifo] - 1) ) /
                                                                                au1KbLimitSizeIn64b[nCmdFifo]; 
                anSingleEntryCmdSize[nCmdFifo] = anCommandsPerEntry[nCmdFifo];
              }
            }
            anSingleEntryCmdSize[nCmdFifo] +=
                    pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEntrySizeIn64b;
            auEntrySizeIn64b_RemainingCount[nCmdFifo] = 
                    pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEntrySizeIn64b;

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__XL_READ ) {

            if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
              anRspSize[nCmdFifo] += pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainLimit *
                              pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainElementSizeIn64b;
            } else {
              anRspSize[nCmdFifo] +=
                    pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEntrySizeIn64b;
            }

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__TAPS ) {

            anSingleEntryCmdSize[nCmdFifo] += C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_64B_WORDS;

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT ) {

            anSingleEntryCmdSize[nCmdFifo] += (C3HPPC_TMU_ASSOC_DATA_SIZE_IN_BYTES + 
                pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uKeySizeInBytes) / sizeof(uint64);

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__EML_DELETE ) {
/*
            if ( first_delete_command == 0 ) first_delete_command = 1;
*/

            anSingleEntryCmdSize[nCmdFifo] +=
                pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uKeySizeInBytes / sizeof(uint64);
          }
                      

          anOperationSize[nCmdFifo] = anSingleEntryCmdSize[nCmdFifo];
          if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand != C3HPPC_TMU_UPDATE_COMMAND__LOCK &&
               pActiveUpdateCmdInfo[nCmdFifo]->nCommand != C3HPPC_TMU_UPDATE_COMMAND__RELEASE ) {
            anOperationSize[nCmdFifo] *= pActiveUpdateCmdInfo[nCmdFifo]->nNumberOfEntries;
          }
          auTableDataDmaIndex[nCmdFifo] = 0;
        }

        nCmdFifoFreeSpace = C3HPPC_TMU_UPDATE_CMD_FIFO_SIZE_IN_64b_WORDS;  
        nExpectRspRingFreeSpace = C3HPPC_TMU_EXPECT_RSP_RING_SIZE -
                                  c3hppc_tmu_get_expect_rsp_ring_count( pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo],
                                                                        pUpdateManagerCB->nExpectRspRingRdPtr[nCmdFifo] );
        --nExpectRspRingFreeSpace;  /* Do not allow this FIFO to become completely full as without using a semaphore 
                                       it is not possible to detect the difference between totally full and empty.  The 
                                       write and read pointers of this FIFO span threads. In place of a semaphore
                                       just limiting the free space by 1.
                                    */ 

        nDmaIndex = 0;
        while ( anOperationSize[nCmdFifo] != 0 && 
                (nCmdFifoFreeSpace - (nDmaIndex/2)) >= anSingleEntryCmdSize[nCmdFifo] && 
                (nCmdFifoFreeSpace - (nDmaIndex/2)) >= 4 &&
                nExpectRspRingFreeSpace >= anRspSize[nCmdFifo] ) {

          nCount =  c3hppc_tmu_get_expect_rsp_ring_count( pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo],
                                                          pUpdateManagerCB->nExpectRspRingRdPtr[nCmdFifo] );
          assert( nCount >= 0 && nCount <= C3HPPC_TMU_EXPECT_RSP_RING_SIZE );
          assert( pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] < C3HPPC_TMU_EXPECT_RSP_RING_SIZE );
          assert( nDmaIndex < (2*C3HPPC_TMU_UPDATE_CMD_FIFO_SIZE_IN_64b_WORDS) );

          COMPILER_64_ZERO(ResponseWord.value);

          if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__XL_WRITE ) {

            xlWriteCmd.bits.EntryNum = pActiveUpdateCmdInfo[nCmdFifo]->nStartingEntryIndex++;
            xlWriteCmd.bits.Lookup = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uLookup;
            xlWriteCmd.bits.Table = pActiveUpdateCmdInfo[nCmdFifo]->nTable;
            uRemainingCount = auEntrySizeIn64b_RemainingCount[nCmdFifo];
            uOffset = pActiveUpdateCmdInfo[nCmdFifo]->uOffset;

            for ( nCmd = 0; nCmd < anCommandsPerEntry[nCmdFifo]; ++nCmd ) { 

              xlWriteCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
              xlWriteCmd.bits.Offset = uOffset;
              xlWriteCmd.bits.Size = C3HPPC_MIN( uRemainingCount, au1KbLimitSizeIn64b[nCmdFifo] );
#if 0
              cli_out("CMD %d anCommandsPerEntry %d Size %d auEntrySizeIn64b_RemainingCount %d au1KbLimitSizeIn64b %d\n",
                      nCmdFifo, anCommandsPerEntry[nCmdFifo], xlWriteCmd.bits.Size, auEntrySizeIn64b_RemainingCount[nCmdFifo],
                      au1KbLimitSizeIn64b[nCmdFifo] );
#endif
  
              pDmaData[nDmaIndex++] = COMPILER_64_LO( xlWriteCmd.value);
              pDmaData[nDmaIndex++] = COMPILER_64_HI( xlWriteCmd.value);
              for ( nIndex = 0; nIndex < (2 * xlWriteCmd.bits.Size); ++nIndex ) {
                pDmaData[nDmaIndex++] = pActiveUpdateCmdInfo[nCmdFifo]->pTableData[auTableDataDmaIndex[nCmdFifo]++]; 
              }

              ResponseWord.bits.Op = xlWriteCmd.bits.Op;
              ResponseWord.bits.SeqNum = xlWriteCmd.bits.SeqNum;
              COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                              COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
              pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 

              uRemainingCount -= xlWriteCmd.bits.Size;
              uOffset += xlWriteCmd.bits.Size;

            }

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__XL_READ ) {

            xlReadCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
            xlReadCmd.bits.Table = pActiveUpdateCmdInfo[nCmdFifo]->nTable;
            xlReadCmd.bits.Lookup = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uLookup;
            if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
              xlReadCmd.bits.KVpairs = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainLimit;
            }
            xlReadCmd.bits.EntryNum = pActiveUpdateCmdInfo[nCmdFifo]->nStartingEntryIndex++;

            pDmaData[nDmaIndex++] = COMPILER_64_LO( xlReadCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( xlReadCmd.value);

            ResponseWord.bits.Op = xlReadCmd.bits.Op;
            ResponseWord.bits.SeqNum = xlReadCmd.bits.SeqNum;
            if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
              ResponseWord.bits.Size_Val01 = xlReadCmd.bits.KVpairs *
                              pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainElementSizeIn64b;
            } else {
              ResponseWord.bits.Size_Val01 = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEntrySizeIn64b;
            }
            nReadCount = (int) ResponseWord.bits.Size_Val01;
            COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                            COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
            pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
              (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1);

            for ( nIndex = 0; nIndex < nReadCount; ++nIndex ) {
              if ( pActiveUpdateCmdInfo[nCmdFifo]->pTableData == NULL ) {
                if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
                  COMPILER_64_SET(ResponseWord.value, COMPILER_64_HI(C3HPPC_TMU_SKIP_RSP_COMPARE_EML_CHAIN), COMPILER_64_LO(C3HPPC_TMU_SKIP_RSP_COMPARE_EML_CHAIN));
                } else {
                  COMPILER_64_SET(ResponseWord.value, COMPILER_64_HI(C3HPPC_TMU_SKIP_RSP_COMPARE_EML_ROOT), COMPILER_64_LO(C3HPPC_TMU_SKIP_RSP_COMPARE_EML_ROOT));
                }
              } else {
                COMPILER_64_SET(ResponseWord.value, 
                                pActiveUpdateCmdInfo[nCmdFifo]->pTableData[(auTableDataDmaIndex[nCmdFifo]+1)],
                                pActiveUpdateCmdInfo[nCmdFifo]->pTableData[auTableDataDmaIndex[nCmdFifo]]);
                auTableDataDmaIndex[nCmdFifo] += 2;
              }
              COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                              COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
              pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1);
            }
            if ( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].bEMLchainTable ) {
              auTableDataDmaIndex[nCmdFifo] += 2 *
                         pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uEMLchainElementSizeIn64b;
            }

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__TAPS ) {

            TapsCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
  
            pDmaData[nDmaIndex++] = COMPILER_64_LO( TapsCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( TapsCmd.value);
            for ( nIndex = 0; nIndex < C3HPPC_TMU_TAPS_COMMAND_SIZE_IN_32B_WORDS; ++nIndex ) {
              pDmaData[nDmaIndex++] = pActiveUpdateCmdInfo[nCmdFifo]->pTableData[auTableDataDmaIndex[nCmdFifo]++]; 
            }

            ResponseWord.bits.Op = TapsCmd.bits.Op;
            ResponseWord.bits.SeqNum = TapsCmd.bits.SeqNum;
            COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                            COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
            pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                        (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__LOCK ||
                      pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__RELEASE ) {

            LockReleaseCmd.bits.Op = pActiveUpdateCmdInfo[nCmdFifo]->nCommand;
            LockReleaseCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
            LockReleaseCmd.bits.Table = pActiveUpdateCmdInfo[nCmdFifo]->nTable;
            LockReleaseCmd.bits.Global = pActiveUpdateCmdInfo[nCmdFifo]->nOptions;
            LockReleaseCmd.bits.EntryNum = pActiveUpdateCmdInfo[nCmdFifo]->nNumberOfEntries;
  
            pDmaData[nDmaIndex++] = COMPILER_64_LO( LockReleaseCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( LockReleaseCmd.value);

            ResponseWord.bits.Op = LockReleaseCmd.bits.Op;
            ResponseWord.bits.SeqNum = LockReleaseCmd.bits.SeqNum;
            COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                            COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
            pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                        (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__EML_INSERT ) {

            emlInsertCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
            emlInsertCmd.bits.Table = pActiveUpdateCmdInfo[nCmdFifo]->nTable;
            emlInsertCmd.bits.Filter = 1;
            emlInsertCmd.bits.Size =
              c3hppc_tmu_get_insert_delete_cmd_size_field( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uLookup );
  
            pDmaData[nDmaIndex++] = COMPILER_64_LO( emlInsertCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( emlInsertCmd.value);
            for ( nIndex = 0; nIndex < ( 2 * (anSingleEntryCmdSize[nCmdFifo] - 1) ); ++nIndex ) { 
              pDmaData[nDmaIndex++] = pActiveUpdateCmdInfo[nCmdFifo]->pTableData[auTableDataDmaIndex[nCmdFifo]++]; 
            }

            ResponseWord.bits.Op = emlInsertCmd.bits.Op;
            ResponseWord.bits.SeqNum = emlInsertCmd.bits.SeqNum;
            ResponseWord.bits.ErrCode_01 = 0;
            COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                            COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
            pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                        (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 

          } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nCommand == C3HPPC_TMU_UPDATE_COMMAND__EML_DELETE ) {

            emlDeleteCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
            emlDeleteCmd.bits.Table = pActiveUpdateCmdInfo[nCmdFifo]->nTable;
            emlDeleteCmd.bits.BulkDelete = ( pActiveUpdateCmdInfo[nCmdFifo]->nOptions ) ? 1 : 0;
            emlDeleteCmd.bits.Size =
              c3hppc_tmu_get_insert_delete_cmd_size_field( pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo[nCmdFifo]->nTable].uLookup );
  
            pDmaData[nDmaIndex++] = COMPILER_64_LO( emlDeleteCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( emlDeleteCmd.value);
            for ( nIndex = 0; nIndex < ( 2 * (anSingleEntryCmdSize[nCmdFifo] - 1) ); ++nIndex ) { 
              pDmaData[nDmaIndex++] = pActiveUpdateCmdInfo[nCmdFifo]->pTableData[auTableDataDmaIndex[nCmdFifo]++]; 
            }

            ResponseWord.bits.Op = emlDeleteCmd.bits.Op;
            ResponseWord.bits.SeqNum = emlDeleteCmd.bits.SeqNum;
            if ( pActiveUpdateCmdInfo[nCmdFifo]->nOptions == C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_NOT_FOUND ) {
              ResponseWord.bits.ErrCode_01 = C3HPPC_TMU_HOST_RSP__ERRCODE__EML_KEY_NOT_FOUND;
            } else if ( pActiveUpdateCmdInfo[nCmdFifo]->nOptions == C3HPPC_TMU_UPDATE_DELETE_OPTIONS__EXPECT_FOUND ) {
              ResponseWord.bits.ErrCode_01 = C3HPPC_TMU_HOST_RSP__ERRCODE__EML_NO_BD_MATCH;
            } else {
              ResponseWord.bits.ErrCode_01 = 0;
            }
            COMPILER_64_SET(pUpdateManagerCB->ExpectRspRing[nCmdFifo][pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]],
                            COMPILER_64_HI(ResponseWord.value), COMPILER_64_LO(ResponseWord.value));
            pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo] =
                        (pUpdateManagerCB->nExpectRspRingWrPtr[nCmdFifo]+1) & (C3HPPC_TMU_EXPECT_RSP_RING_SIZE - 1); 

          }

          nExpectRspRingFreeSpace -= anRspSize[nCmdFifo];
          anOperationSize[nCmdFifo] -= anSingleEntryCmdSize[nCmdFifo];
        }
  
        if ( nDmaIndex ) {
          nNopCount = (nDmaIndex/2) % 4;
          if ( nNopCount ) nNopCount = 4 - nNopCount;
          for ( nNop = 0; nNop < nNopCount; ++nNop ) {
            xlNopCmd.bits.SeqNum = auSequenceNumber[nCmdFifo]++;
            pDmaData[nDmaIndex++] = COMPILER_64_LO( xlNopCmd.value);
            pDmaData[nDmaIndex++] = COMPILER_64_HI( xlNopCmd.value);
          }

          /*    coverity[negative_returns : FALSE]    */
          rc = soc_mem_write_range( nUnit, aTmuUpdaterCmdFifoMem[nCmdFifo], MEM_BLOCK_ANY, 0,
                                    ((nDmaIndex/sizeof(uint64)) - 1), (void *) pDmaData);
          if ( rc ) {
            c3hppc_tmu__dump_cmic_rd_dma_state( nUnit );
            ++pUpdateManagerCB->nErrorCounter;
          }

/*
          start_time = sal_time_usecs();
          coverity[negative_returns : FALSE]
          soc_mem_write_range( nUnit, aTmuUpdaterCmdFifoMem[nCmdFifo], MEM_BLOCK_ANY, 0,
                               ((nDmaIndex/sizeof(uint64)) - 1), (void *) pDmaData);
          diff_time = SAL_USECS_SUB(sal_time_usecs(), start_time);

          total_diff_times += diff_time;
          ++total_dma_ops;

          total_dma_size += nDmaIndex;

          if ( first_delete_command == 1 ) {
            first_delete_command = 2;
            total_diff_times -= diff_time;
            --total_dma_ops;
            init_average = total_diff_times / total_dma_ops;
            total_dma_size -= nDmaIndex;
            dma_size_average = total_dma_size / total_dma_ops;
            cli_out("\n\n  Init DMA Perf --> %d    Init DMA Average Size %d \n\n", init_average, dma_size_average );
            
            total_diff_times = diff_time;
            total_dma_ops = 1;
            total_dma_size = nDmaIndex;
          }
*/

          if ( anOperationSize[nCmdFifo] == 0 ) {
            pUpdateManagerCB->nUpdateCmdQRdPtr[nCmdFifo] = 
              (pUpdateManagerCB->nUpdateCmdQRdPtr[nCmdFifo] + 1) & (C3HPPC_TMU_UPDATE_QUEUE_SIZE - 1); 
            --pUpdateManagerCB->nUpdateCmdQCount[nCmdFifo];
            if ( pActiveUpdateCmdInfo[nCmdFifo]->pTableData != NULL ) sal_free( pActiveUpdateCmdInfo[nCmdFifo]->pTableData );
            pActiveUpdateCmdInfo[nCmdFifo] = NULL;
          }
        }

      }

    }  /* for ( nCmdFifo = 0; nCmdFifo < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifo ) { */

#if (defined(LINUX))
    sal_usleep(1);
#else
    sal_sleep(0);
#endif

  }  /* while ( !pUpdateManagerCB->bExit ) */

  soc_cm_sfree( nUnit, pDmaData );

  cli_out("  Exiting TMU Updater COMMAND Manager thread .... \n\n");

/*
  cli_out("\n\n  Init DMA Perf --> %d     RunTime DMA Perf --> %d \n\n", init_average, (total_diff_times / total_dma_ops) );
  cli_out("\n\n  Average thread wakeup --> %d \n\n", (thread_wakeup_average / thread_wakeups) );
  cli_out("\n\n  DMA size average --> %d \n\n", (total_dma_size / total_dma_ops) );
*/

  sal_thread_exit(0);

  return;

}


#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
