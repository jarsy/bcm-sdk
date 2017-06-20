/* $Id: c3hppc_ddr_test_suite.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"

#if (defined(LINUX))
extern int bb_caladan3_current(int unit, int bx);
extern int soc_sbx_caladan3_temperature_monitor(int unit, int nsamples, int format, int celsius);
#endif

extern int _soc_caladan3_mem_reset_and_init_after_shmoo_addr(int unit, int ci);


static int g_anCiDDRtestFailedDataRegisters[] = {CI_DDR_TEST_FAILED_DATA0r, CI_DDR_TEST_FAILED_DATA1r,
                                                 CI_DDR_TEST_FAILED_DATA2r, CI_DDR_TEST_FAILED_DATA3r,
                                                 CI_DDR_TEST_FAILED_DATA4r, CI_DDR_TEST_FAILED_DATA5r,
                                                 CI_DDR_TEST_FAILED_DATA6r, CI_DDR_TEST_FAILED_DATA7r};
 
static int  c3hppc_ddr_test_suite__t002( int nUnit );
static int  c3hppc_ddr_test_suite__t000( c3hppc_test_info_t *pc3hppcTestInfo );
static int  c3hppc_ddr_test_suite__t001( c3hppc_test_info_t *pc3hppcTestInfo );

int
c3hppc_ddr_test_suite__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  return 0;
}

int
c3hppc_ddr_test_suite__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  c3hppc_tmu_ci_control_info_t C3TmuCiControlInfo;
  int nESM;

  if ( pc3hppcTestInfo->nProgramSelect == 2 ) {
    pc3hppcTestInfo->nTestStatus = c3hppc_ddr_test_suite__t002( pc3hppcTestInfo->nUnit );

  } else if ( pc3hppcTestInfo->nProgramSelect == 0 || pc3hppcTestInfo->nProgramSelect == 1 )  {

#if (defined(LINUX))
    if ( pc3hppcTestInfo->nShowTemp ) {
      cli_out("\n");
      soc_sbx_caladan3_temperature_monitor( pc3hppcTestInfo->nUnit, 1, 0, 1 );
      cli_out("\n");
    }
#endif

    C3TmuCiControlInfo.nDramFreq = pc3hppcTestInfo->nDramFreq;
    C3TmuCiControlInfo.bSkipDramSelfTest = 1;
    if ( c3hppc_tmu_ci_hw_init( pc3hppcTestInfo->nUnit, &C3TmuCiControlInfo) ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL; 

    } else {

      if ( COMPILER_64_IS_ZERO(pc3hppcTestInfo->uuIterations) ) COMPILER_64_SET(pc3hppcTestInfo->uuIterations,0, 1);
      pc3hppcTestInfo->nTestStatus = ( pc3hppcTestInfo->nProgramSelect == 0 ) ? c3hppc_ddr_test_suite__t000( pc3hppcTestInfo ) :
                                                                                c3hppc_ddr_test_suite__t001( pc3hppcTestInfo );
    }

#if (defined(LINUX))
    if ( pc3hppcTestInfo->nShowTemp ) {
      cli_out("\n");
      soc_sbx_caladan3_temperature_monitor( pc3hppcTestInfo->nUnit, 1, 0, 1 );
      cli_out("\n");
    }
#endif

  } else {

    for ( nESM = 0; nESM < 3; ++nESM ) {
      c3hppc_etu_dsc_dump( pc3hppcTestInfo->nUnit, nESM );
    }

  }

/*
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, 
                            (pc3hppcTestInfo->uuEpochCount + pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition) );

  while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
    sal_sleep(5);
    assert( !c3hppc_cmu_display_error_state(pc3hppcTestInfo->nUnit) );
  }
*/


  return 0;
}

int
c3hppc_ddr_test_suite__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  return 0;
}






int  c3hppc_ddr_test_suite__t002( int nUnit )
{
  int rc, nCiInstance, nPhyLane;
  uint32 uRegisterValue, uAddress, uExpectRegisterValue;

  cli_out("\n\n<c3hppc_ddr_test_suite> -- Running ported DV test:  \"c3_ci_phy_wrap_t002\" [RBUS Access Test] \n");

  rc = TEST_OK;

  c3hppc_tmu_ci_deassert_ci_reset( nUnit );
  c3hppc_tmu_ci_deassert_phy_reset( nUnit );
  c3hppc_tmu_ci_deassert_ddr_reset( nUnit );

  if ( c3hppc_tmu_ci_poll_phy_pwrup_rsb(nUnit) ) {
    rc = TEST_FAIL;
  } else {

    c3hppc_tmu_ci_clear_alarms( nUnit );

    for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; nCiInstance += 2 ) {

      uRegisterValue = sal_rand() & 0x7fffffff;
      uExpectRegisterValue = uRegisterValue;
      uAddress = C3HPPC_DDR40_PHY_PLL_CONTROL__OFFSET;
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, uAddress, &uRegisterValue );
      uRegisterValue = 0xffffffff;
      c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 0, uAddress, &uRegisterValue );
      if ( uRegisterValue != uExpectRegisterValue ) {
        cli_out("<c3hppc_ddr_test_suite__t002> -- CI%d DDR40_PHY_PLL_CONTROL register MISCOMPARE!   Actual: 0x%08x   Expect: 0x%08x \n",
                nCiInstance, uRegisterValue, uExpectRegisterValue );
        rc = TEST_FAIL;
      }
      

      for ( nPhyLane = 0; nPhyLane < 2; ++nPhyLane ) {
        uRegisterValue = sal_rand() & 0x1f;
        uExpectRegisterValue = uRegisterValue;
        uAddress = ( nPhyLane == 0 ) ? C3HPPC_PHY_WORD_LANE_0_RBUS_START__OFFSET : C3HPPC_PHY_WORD_LANE_1_RBUS_START__OFFSET;
        uAddress += C3HPPC_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL__OFFSET;
        c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 1, uAddress, &uRegisterValue );
        uRegisterValue = 0xffffffff;
        c3hppc_tmu_ci_phy_read_write( nUnit, nCiInstance, 0, uAddress, &uRegisterValue );
        if ( uRegisterValue != uExpectRegisterValue ) {
          cli_out("<c3hppc_ddr_test_suite__t002> -- CI%d DDR40_PHY_WORD_LANE%d_DRIVE_PAD_CTL register MISCOMPARE!   Actual: 0x%08x   Expect: 0x%08x \n",
                  nCiInstance, nPhyLane, uRegisterValue, uExpectRegisterValue );
          rc = TEST_FAIL;
        }
      }
    }


  }


  return rc;
}



int  c3hppc_ddr_test_suite__t001( c3hppc_test_info_t *pc3hppcTestInfo )
{
  int rc, nCiInstance, nIndex, nFailingCiInstance COMPILER_ATTRIBUTE((unused)), nUnit, nCiTarget, nCiAll;
  uint32 uRegisterValue, uModeIndex, uTestMode, uAddr, uBurstLength, uReadOutBurstLength;
  sal_time_t TimeStamp;
  uint32 auFailedData[8];
  uint32 uTestModes[4] = { 3, 2, 1, 0 };
  c3hppc_tmu_ci_mem_acc_addr_ut CiMemAccAddr;
  int nDDRinFailedState;
  
  nCiTarget = 14;
  nCiAll = 1;
  uReadOutBurstLength = 0x40;
  uBurstLength = uReadOutBurstLength;
  uBurstLength = 0x800000 - 1;

  nUnit = pc3hppcTestInfo->nUnit;

  cli_out("\n\n<c3hppc_ddr_test_suite> -- Running Memory Self Test .... \n");

  rc = TEST_OK;
  nFailingCiInstance = 0;


  /* For 2Gb parts, with a 256b burst size, 2^23 bursts are required. */
  for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
    soc_reg32_set( nUnit, CI_DDR_BURSTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uBurstLength );
    soc_reg32_set( nUnit, CI_DDR_STARTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0x00000000 );
    soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );
    soc_reg32_set( nUnit, CI_DDR_STEPr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );

    soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 1 );
    soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

    soc_reg32_set( nUnit, TM_QE_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0xffffffff );
  }

  c3hppc_tmu_ci_clear_alarms( nUnit );

  for ( nCiInstance = 0; ( 0 && nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM ); ++nCiInstance ) {
    nDDRinFailedState = 0;
    if ( nCiInstance == nCiTarget || nCiAll ) {
      for ( uAddr = 0; ( uAddr <= uReadOutBurstLength ); ++uAddr ) {
        CiMemAccAddr.value = 0;
        CiMemAccAddr.bits.Bank = uAddr & 0x7;
        /* Column is the number of 64b accesses. */
        CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
        CiMemAccAddr.bits.Row = (uAddr >> 9);
        c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
        cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d Address[0x%08x] PhyAddress[0x%08x] Data Pattern[255:0] --> \n",
                nCiInstance, uAddr, CiMemAccAddr.value );
        cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
        if ( auFailedData[0] != uAddr ) nDDRinFailedState = 1;
      }
      if ( 0 && nDDRinFailedState ) {
        cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d DDR is in a failed state, doing MR2 re-write.\n", nCiInstance );
        soc_reg32_set( nUnit, CI_MR3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0);
        uRegisterValue = 0;
        soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DONEf, 1 );
        soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, REQf, 1 );
        soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DEVICE_SELf, 3 );
        soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 2 );
        soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
      }
    }
  }

  for ( uModeIndex = 0; ( uModeIndex < 4 && rc == TEST_OK ); ++uModeIndex ) {

    uTestMode = uTestModes[uModeIndex];

    for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
      if ( nCiInstance == nCiTarget || nCiAll ) {
        uRegisterValue = 0;
        soc_reg_field_set( nUnit, CI_DDR_ITERr, &uRegisterValue, DDR_ITERf, COMPILER_64_LO(pc3hppcTestInfo->uuIterations) );
        soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

        uRegisterValue = 0;
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, uTestMode );
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
        soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
        soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
      }
    }

    for ( nCiInstance = 0; ( nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM && rc == TEST_OK ); ++nCiInstance ) {
      if ( nCiInstance == nCiTarget || nCiAll ) {
        if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                     CI_DDR_TESTr, RAM_DONEf, 1, ( (int) (2*((int) COMPILER_64_LO(pc3hppcTestInfo->uuIterations))) ), 0, &TimeStamp ) ) {
          cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d CI_DDR_TEST \"RAM_DONE\" event TIMEOUT!!!\n", nCiInstance);
          rc = TEST_FAIL;
        } else {
          nFailingCiInstance = nCiInstance;
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
            rc = TEST_FAIL;

            for ( uAddr = 0; uAddr <= uReadOutBurstLength; ++uAddr ) {
              CiMemAccAddr.value = 0;
              CiMemAccAddr.bits.Bank = uAddr & 0x7;
              /* Column is the number of 64b accesses. */
              CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
              CiMemAccAddr.bits.Row = (uAddr >> 9);
              c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
              cli_out("<c3hppc_ddr_test_suite__t000> -- Failed CI%d Address[0x%08x] Data Pattern[255:0] --> \n", nCiInstance, uAddr );
              cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                      auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                      auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
            }
          }
        }
  
        for ( uAddr = 0; ( 0 && uAddr <= uReadOutBurstLength ); ++uAddr ) {
          CiMemAccAddr.value = 0;
          CiMemAccAddr.bits.Bank = uAddr & 0x7;
          /* Column is the number of 64b accesses. */
          CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
          CiMemAccAddr.bits.Row = (uAddr >> 9);
          c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
          cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d Address[0x%08x] Data Pattern[255:0] --> \n", nCiInstance, uAddr );
          cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                  auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                  auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
        }
  
      }
    }

  }

  if ( 0 && rc == TEST_OK ) {
    uTestMode = 2;

    for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
      if ( nCiInstance == nCiTarget || nCiAll ) {
        uRegisterValue = 0;
        soc_reg_field_set( nUnit, CI_DDR_ITERr, &uRegisterValue, DDR_ITERf, 1 );
        soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

        uRegisterValue = 0;
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, uTestMode );
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, WRITE_ONLYf, 1 );
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
        soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  
        soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
        soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

        c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                     CI_DDR_TESTr, RAM_DONEf, 1, 2, 0, &TimeStamp );
  
      }
    }
  }

/*
  if ( rc == TEST_FAIL ) assert(0);
*/

  
  return rc;
}


int  c3hppc_ddr_test_suite__t000( c3hppc_test_info_t *pc3hppcTestInfo )
{
  int rc, nCiInstance, nIndex, nFailingCiInstance COMPILER_ATTRIBUTE((unused)), nUnit, nCiTarget, nCiAll;
  uint32 uRegisterValue, uModeIndex, uTestMode, uAddr, uBurstLength, uReadOutBurstLength;
  sal_time_t TimeStamp;
  uint32 auFailedData[8];
  uint32 uTestModes[4] = { 3, 2, 1, 0 };
  c3hppc_tmu_ci_mem_acc_addr_ut CiMemAccAddr;
  int nDDRinFailedState;
  uint64 uuIterationCount;
  
  nCiTarget = 14;
  nCiAll = 0;
  uReadOutBurstLength = 0x40;
  uBurstLength = 0x800000 - 1;
  uBurstLength = uReadOutBurstLength;

  nUnit = pc3hppcTestInfo->nUnit;

  cli_out("\n\n<c3hppc_ddr_test_suite> -- Running Memory Self Test .... \n");

  rc = TEST_OK;
  nFailingCiInstance = 0;


  /* For 2Gb parts, with a 256b burst size, 2^23 bursts are required. */
  for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
    soc_reg32_set( nUnit, CI_DDR_BURSTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uBurstLength );
    soc_reg32_set( nUnit, CI_DDR_STARTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0x00000000 );
    soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );
    soc_reg32_set( nUnit, CI_DDR_STEPr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 1 );

    soc_reg32_get( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, &uRegisterValue );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, DISABLE_TMU_SCHEDULE_REFRESHf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, REFRESH_OVERRIDEf, 1 );
    soc_reg_field_set( nUnit, CI_CONFIG3r, &uRegisterValue, ZQCAL_OVERRIDEf, 1 );
    soc_reg32_set( nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);

    soc_reg32_set( nUnit, TM_QE_ERROR_MASKr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0xffffffff );
  }

  c3hppc_tmu_ci_clear_alarms( nUnit );

  COMPILER_64_ZERO(uuIterationCount);
  do {
    COMPILER_64_ADD_32(uuIterationCount, 1);
    for ( nCiInstance = 0; ( nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM ); ++nCiInstance ) {
      nDDRinFailedState = 0;
      if ( nCiInstance == nCiTarget || nCiAll ) {
        for ( uAddr = 0; ( uAddr <= uReadOutBurstLength ); ++uAddr ) {
          CiMemAccAddr.value = 0;
          CiMemAccAddr.bits.Bank = uAddr & 0x7;
          /* Column is the number of 64b accesses. */
          CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
          CiMemAccAddr.bits.Row = (uAddr >> 9);
          c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
          cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d Address[0x%08x] PhyAddress[0x%08x] Data Pattern[255:0] --> \n",
                  nCiInstance, uAddr, CiMemAccAddr.value );
          cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                  auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                  auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
          if ( auFailedData[0] != uAddr ) nDDRinFailedState = 1;
        }
        if ( nDDRinFailedState ) {
/*
          cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d DDR is in a failed state, doing MR2 re-write.\n", nCiInstance );
          soc_reg32_set( nUnit, CI_MR3r, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, 0);
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DONEf, 1 );
          soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, REQf, 1 );
          soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, DEVICE_SELf, 3 );
          soc_reg_field_set( nUnit, CI_MR_CMDr, &uRegisterValue, MR_SELf, 2 );
          soc_reg32_set( nUnit, CI_MR_CMDr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
*/

          cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d DDR is in a failed state, doing DDR reset and config!\n", nCiInstance );
          _soc_caladan3_mem_reset_and_init_after_shmoo_addr( nUnit, nCiInstance );
        }
      }
    }

    for ( uModeIndex = 0; ( uModeIndex < 1 && rc == TEST_OK ); ++uModeIndex ) {

      uTestMode = uTestModes[uModeIndex];

      for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
        if ( nCiInstance == nCiTarget || nCiAll ) {
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, uTestMode );
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
          soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
          soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
        }
      }

      for ( nCiInstance = 0; ( nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM && rc == TEST_OK ); ++nCiInstance ) {
        if ( nCiInstance == nCiTarget || nCiAll ) {
          if ( c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                       CI_DDR_TESTr, RAM_DONEf, 1, ( (int) (2*(COMPILER_64_LO(pc3hppcTestInfo->uuIterations))) ), 0, &TimeStamp ) ) {
            cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d CI_DDR_TEST \"RAM_DONE\" event TIMEOUT!!!\n", nCiInstance);
            rc = TEST_FAIL;
          } else {
            nFailingCiInstance = nCiInstance;
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
              rc = TEST_FAIL;
  
              for ( uAddr = 0; uAddr <= uReadOutBurstLength; ++uAddr ) {
                CiMemAccAddr.value = 0;
                CiMemAccAddr.bits.Bank = uAddr & 0x7;
                /* Column is the number of 64b accesses. */
                CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
                CiMemAccAddr.bits.Row = (uAddr >> 9);
                c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
                cli_out("<c3hppc_ddr_test_suite__t000> -- Failed CI%d Address[0x%08x] Data Pattern[255:0] --> \n", nCiInstance, uAddr );
                cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                        auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                        auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
              }
            }
          }
    
          for ( uAddr = 0; ( 0 && uAddr <= uReadOutBurstLength ); ++uAddr ) {
            CiMemAccAddr.value = 0;
            CiMemAccAddr.bits.Bank = uAddr & 0x7;
            /* Column is the number of 64b accesses. */
            CiMemAccAddr.bits.Column = 4 * ((uAddr >> 3) & 0x3f);
            CiMemAccAddr.bits.Row = (uAddr >> 9);
            c3hppc_tmu_ci_read_write( nUnit, nCiInstance, 32, 0, CiMemAccAddr.value, auFailedData );
            cli_out("<c3hppc_ddr_test_suite__t000> -- CI%d Address[0x%08x] Data Pattern[255:0] --> \n", nCiInstance, uAddr );
            cli_out("     0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x_0x%08x\n\n",
                    auFailedData[7],auFailedData[6],auFailedData[5],auFailedData[4],
                    auFailedData[3],auFailedData[2],auFailedData[1],auFailedData[0] );
          }
    
        }
      }
  
    }
  
    if ( 0 && rc == TEST_OK ) {
      uTestMode = 2;
  
      for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
        if ( nCiInstance == nCiTarget || nCiAll ) {
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, CI_DDR_ITERr, &uRegisterValue, DDR_ITERf, 1 );
          soc_reg32_set( nUnit, CI_DDR_ITERr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  
          uRegisterValue = 0;
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, MODEf, uTestMode );
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, WRITE_ONLYf, 1 );
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TEST_FAILf, 1 );
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_DONEf, 1 );
          soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
    
          soc_reg_field_set( nUnit, CI_DDR_TESTr, &uRegisterValue, RAM_TESTf, 1 );
          soc_reg32_set( nUnit, CI_DDR_TESTr, SOC_BLOCK_PORT(nUnit,nCiInstance), 0, uRegisterValue);
  
          c3hppcUtils_poll_field( nUnit, SOC_BLOCK_PORT(nUnit,nCiInstance),
                                       CI_DDR_TESTr, RAM_DONEf, 1, 2, 0, &TimeStamp );
    
        }
      }
    }
  
    if ( rc == TEST_FAIL ) {

      assert(0);

    } else if ( 0 ) {

      for ( nCiInstance = 0; nCiInstance < C3HPPC_TMU_CI_INSTANCE_NUM; ++nCiInstance ) {
        if ( nCiInstance == nCiTarget || nCiAll ) {
          _soc_caladan3_mem_reset_and_init_after_shmoo_addr( nUnit, nCiInstance );
        }
      }
      
    }

  } while (COMPILER_64_LT(uuIterationCount, pc3hppcTestInfo->uuIterations) && (rc == TEST_OK ));
  
  return rc;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
