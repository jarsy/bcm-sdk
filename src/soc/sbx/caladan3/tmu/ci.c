/*
 * $Id: ci.c,v 1.13.14.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    tmu.c
 * Purpose: Caladan3 on TMU drivers
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/ci.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <soc/shmoo_ddr40.h>

/*#define TIME_STAMP_DBG*/

#ifdef TIME_STAMP_DBG 
sal_usecs_t        start, end;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    end = sal_time_usecs();                                         \
    LOG_CLI((BSL_META("\n %s: Time Stamp: [%u]"), msg, SAL_USECS_SUB(end, start))); \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#endif

/* Have it run a little faster ~(7800ns / 1.666666667ns) */
#define SOC_SBX_CALADAN3_TMU_7800NS_REFRESH_INTERVAL   (4672)
#define SOC_SBX_CALADAN3_TMU_REFRESH_WHEEL_INTERVAL    (SOC_SBX_CALADAN3_TMU_7800NS_REFRESH_INTERVAL /\
                                                        SOC_SBX_CALADAN3_TMU_CI_INSTANCE_NUM)
#define SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_0_RBUS_START         (0x0200)
#define SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_1_RBUS_START         (0x0400)
#define SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_READ_DATA_DLY  (352)
#define SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL  (420)

 int _soc_sbx_caladan3_tmu_ci_phy_read_write( int unit, int instance, uint8 write, 
                                            uint32 address, uint32 *entry_data )
{
  uint32 regval;


  if ( write ) {
    SOC_IF_ERROR_RETURN( soc_reg32_set( unit, DDR_PHY_REG_DATAr, SOC_BLOCK_PORT(unit,instance), 
                                        0, entry_data[0]) );
  }

  regval = 0;
  soc_reg_field_set( unit, DDR_PHY_REG_CTRLr, &regval, ACKf, 1 );
  soc_reg_field_set( unit, DDR_PHY_REG_CTRLr, &regval, REQf, 1 );
  soc_reg_field_set( unit, DDR_PHY_REG_CTRLr, &regval, ADDRf,address );
  soc_reg_field_set( unit, DDR_PHY_REG_CTRLr, &regval, RD_WR_Nf, (write ? 0 : 1) );
  SOC_IF_ERROR_RETURN(soc_reg32_set( unit, DDR_PHY_REG_CTRLr, SOC_BLOCK_PORT(unit,instance), 
                                      0, regval));

  SOC_IF_ERROR_RETURN(soc_sbx_caladan3_reg32_expect_field_timeout(unit, DDR_PHY_REG_CTRLr,
                                                                  -1, -1,
                                                                  SOC_BLOCK_PORT(unit,instance),
                                                                  ACKf, 1, 100));
  if (!write) {
    SOC_IF_ERROR_RETURN(soc_reg32_get( unit, DDR_PHY_REG_DATAr, SOC_BLOCK_PORT(unit,instance), 0, entry_data+0));
  }

  return 0;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_ci_memory_pattern_init
 * Purpose:
 *   Initialize pattern into all DDR3 memories. 
 *   EML root table pattern is initialized by default
 */
int soc_sbx_caladan3_tmu_ci_memory_pattern_init(int unit) {

  int instance, dataregs;
  uint32 regval;
  int ci_ddr_test_data_reg[] = {CI_DDR_TEST_DATA0r, CI_DDR_TEST_DATA1r,
                                CI_DDR_TEST_DATA2r, CI_DDR_TEST_DATA3r,
                                CI_DDR_TEST_DATA4r, CI_DDR_TEST_DATA5r,
                                CI_DDR_TEST_DATA6r, CI_DDR_TEST_DATA7r};

  int ci_ddr_test_alt_data_reg[] = {CI_DDR_TEST_ALT_DATA0r, CI_DDR_TEST_ALT_DATA1r,
                                    CI_DDR_TEST_ALT_DATA2r, CI_DDR_TEST_ALT_DATA3r,
                                    CI_DDR_TEST_ALT_DATA4r, CI_DDR_TEST_ALT_DATA5r,
                                    CI_DDR_TEST_ALT_DATA6r, CI_DDR_TEST_ALT_DATA7r};
  int dram_size_in_gbits;

  if (soc_property_get(unit, spn_LRP_BYPASS, 0)) {
     return SOC_E_NONE;
  }

  dram_size_in_gbits = soc_property_get(unit, spn_EXT_RAM_TOTAL_SIZE, 
                                        2*MEM_1GBITS_IN_MBYTES);
  /* size of dram part */
  if (dram_size_in_gbits % MEM_1GBITS_IN_MBYTES) {
      /* not multiple of 1G bits */
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d DDR memory part not in multiple of Gbits!!!\n"), 
                 FUNCTION_NAME(), unit));
      return SOC_E_PARAM;          
  }
  dram_size_in_gbits = dram_size_in_gbits/MEM_1GBITS_IN_MBYTES;

  TIME_STAMP_START

  for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance++) {
      for (dataregs = 0; dataregs < COUNTOF(ci_ddr_test_data_reg); dataregs++) {

	  if (( dataregs == 0 ) ||  ( dataregs == 4 )) regval = 0xf8000000;
          else if (( dataregs == 1 ) || ( dataregs == 5 )) regval = 0x00000007;
          else regval = 0;
          soc_reg32_set( unit, ci_ddr_test_data_reg[dataregs], SOC_BLOCK_PORT(unit,instance), 0, regval );
          soc_reg32_set( unit, ci_ddr_test_alt_data_reg[dataregs], SOC_BLOCK_PORT(unit,instance), 0, regval );
      }
      /* For each 1Gb, with a 256b burst size, 2^22 bursts are required. */ 
      soc_reg32_set( unit, CI_DDR_BURSTr, SOC_BLOCK_PORT(unit,instance), 0, ((0x400000 * (dram_size_in_gbits))-1) );
      SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, &regval ));
      soc_reg_field_set( unit, CI_CONFIG3r, &regval, DISABLE_TMU_SCHEDULE_REFRESHf, 1 );
      soc_reg_field_set( unit, CI_CONFIG3r, &regval, REFRESH_OVERRIDEf, 1 );
      soc_reg32_set( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, regval);

      regval = 0;
      soc_reg_field_set( unit, CI_DDR_TESTr, &regval, MODEf, 0 );
      soc_reg_field_set( unit, CI_DDR_TESTr, &regval, RAM_TEST_FAILf, 1 );
      soc_reg_field_set( unit, CI_DDR_TESTr, &regval, WRITE_ONLYf, 1 );
      soc_reg_field_set( unit, CI_DDR_TESTr, &regval, RAM_DONEf, 1 );
      soc_reg32_set( unit, CI_DDR_TESTr, SOC_BLOCK_PORT(unit,instance), 0, regval);
  }
  
  for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance ) {
      SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_DDR_TESTr, SOC_BLOCK_PORT(unit,instance), 0, &regval ));
      soc_reg_field_set( unit, CI_DDR_TESTr, &regval, RAM_TESTf, 1 );
      soc_reg32_set( unit, CI_DDR_TESTr, SOC_BLOCK_PORT(unit,instance), 0, regval);
  }

  TIME_STAMP("#### CI-DDR Pattern Init Time:")

  return SOC_E_NONE;
}

int soc_sbx_caladan3_tmu_is_ci_memory_init_done(int unit) 
{

  int instance, timeout;
  uint32 regval=0, ramdone=0;

  timeout = ( SAL_BOOT_QUICKTURN ) ? 600 : 100;

  if (soc_property_get(unit, spn_LRP_BYPASS, 0)) {
     return SOC_E_NONE;
  }

  for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit) && timeout; ++instance) {
      while (TRUE) {
          SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_DDR_TESTr, SOC_BLOCK_PORT(unit,instance), 0, &regval ));
          ramdone = soc_reg_field_get( unit, CI_DDR_TESTr, regval, RAM_DONEf );
          if ( ramdone ) {
              SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, &regval ));
              soc_reg_field_set( unit, CI_CONFIG3r, &regval, DISABLE_TMU_SCHEDULE_REFRESHf, 0 );
              soc_reg_field_set( unit, CI_CONFIG3r, &regval, REFRESH_OVERRIDEf, 0);
              soc_reg32_set( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, regval);
              break;
          } else {
              sal_sleep(1);
              if ( !(--timeout) ) break;
          }
      }
  }
  
  return ((ramdone)?SOC_E_NONE:SOC_E_TIMEOUT);
}

/*
 * Function:
 *     _soc_sbx_caladan3_ci_init
 * Purpose:
 *     Caladan3 CI Interfaces init
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
_soc_sbx_caladan3_ci_init(int unit, uint32 ddr_config) 
{

#ifdef BCM_DDR3_SUPPORT
    int rv = SOC_E_NONE, ci = 0;
    if (soc_feature(unit, soc_feature_ddr3)) {
        /* DDR Tune */
        if (soc_property_get(unit, spn_DDR3_AUTO_TUNE, TRUE)) {
            LOG_CLI((BSL_META_U(unit,
                                "Please wait while DDR3 Memories are tuned...\n")));
            LOG_CLI((BSL_META_U(unit,
                                "Save the config to avoid re-tuning\n")));
        }
        for (ci = 0; ((1 << ci) & ddr_config); ci += 2)
        {
            if (soc_property_get(unit, spn_DDR3_AUTO_TUNE, TRUE)) {
                LOG_CLI((BSL_META_U(unit,
                                    "Unit %d tuning ci %d,%d\n"), unit, ci, ci+1));
                rv = soc_ddr40_shmoo_ctl(unit, ci, DDR_PHYTYPE_AND, DDR_CTLR_T2, 0, 0);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_soc_sbx_caladan3_ci_init: ddr tune on unit %d failed at soc_ddr40_shmoo_ctl() %d"), unit, rv));
                    return rv;
                }
                if (soc_ddr40_shmoo_savecfg(unit, ci)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_soc_sbx_caladan3_ci_init: ddr tune on unit %d failed at soc_ddr40_shmoo_savecfg() %d"), unit, rv));
                    return rv;
                }
            } else {
                rv = soc_ddr40_shmoo_restorecfg(unit, ci);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_soc_sbx_caladan3_ci_init: on unit %d failed at soc_ddr40_shmoo_restorecfg() %d"), unit, rv));
                    return rv;
                }
            }
        }
        if (SOC_SUCCESS(rv)) {
            if (soc_property_get(unit, spn_DDR3_AUTO_TUNE, TRUE)) {
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "DDR3 Tuning Completed\n")));
                /* No need to tune again for this session */
#if 0 /* sal_config_set should not be used in SOC/BCM */
                sal_config_set("ddr3_auto_tune","0");
#endif
            } else {
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "DDR3 Tune Values Restored\n")));
            }
        }
    }
    return rv;
#else
    return SOC_E_UNAVAIL;
#endif
}


/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_ci_init
 * Purpose:
 *     Bring up TMU CI
 * This code is intended and purely oriented towards emulator. 
 * This is port of code from QT SV test
 */
int soc_sbx_caladan3_tmu_ci_init(int unit) 
{

#ifndef CI_USE_OLD_INIT

    uint32 ddr_config = 0;

    ddr_config = (1 << SOC_DDR3_NUM_MEMORIES(unit)) - 1;
    return (_soc_sbx_caladan3_ci_init(unit, ddr_config));

#else   
    uint32 regval=0;
    int instance;

    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance ) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_RESETr, SOC_BLOCK_PORT(unit, instance), 0, &regval));
        soc_reg_field_set(unit, CI_RESETr, &regval, SW_RESET_Nf, 1);
        soc_reg_field_set(unit, CI_RESETr, &regval, PHY_SW_INITf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_RESETr, SOC_BLOCK_PORT(unit, instance), 0, regval));
    }

    regval = 0;
    soc_reg_field_set(unit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &regval, TFAWf, 4);
    soc_reg_field_set(unit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &regval, TRFCf, 96 );
    soc_reg_field_set(unit, TMB_DISTRIBUTOR_CI_PARAMETERSr, &regval, TRCf, 29 );
    SOC_IF_ERROR_RETURN(WRITE_TMB_DISTRIBUTOR_CI_PARAMETERSr(unit,regval));
    
    regval = 0;
    soc_reg_field_set(unit, TMB_DISTRIBUTOR_REFRESH_CONFIGr, &regval,
                      REFRESH_TIMER_WINDOWf, SOC_SBX_CALADAN3_TMU_7800NS_REFRESH_INTERVAL);
    SOC_IF_ERROR_RETURN(WRITE_TMB_DISTRIBUTOR_REFRESH_CONFIGr(unit,regval));

    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance ) {

        SOC_IF_ERROR_RETURN(READ_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr(unit, instance, &regval));
        soc_reg_field_set(unit, TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr, &regval, REFRESH_THRESHOLDf,
                           (instance * SOC_SBX_CALADAN3_TMU_REFRESH_WHEEL_INTERVAL) );
        SOC_IF_ERROR_RETURN(WRITE_TMB_DISTRIBUTOR_REFRESH_CI_CONFIGr(unit, instance, regval));
        
        READ_TMB_PER_CI_REFRESH_DELAYr(unit, instance, &regval );
        soc_reg_field_set(unit, TMB_PER_CI_REFRESH_DELAYr, &regval, CI_REFRESH_DELAYf, 6);
        WRITE_TMB_PER_CI_REFRESH_DELAYr(unit, instance, regval);

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_CONFIG0r, SOC_BLOCK_PORT(unit, instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG0r, &regval, TRP_READf, ((instance%2==1) ? 19 : 20) );
        soc_reg_field_set(unit, CI_CONFIG0r, &regval, TRRDf, 3);
        soc_reg_field_set(unit, CI_CONFIG0r, &regval, TRCf, 23);
        soc_reg_field_set(unit, CI_CONFIG0r, &regval, TFAWf, 17);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_CONFIG0r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_CONFIG1r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG1r, &regval, TRDTWRf, 10);
        soc_reg_field_set(unit, CI_CONFIG1r, &regval, TWRTRDf, 21);
        soc_reg_field_set(unit, CI_CONFIG1r, &regval, TWLf, (18 - (instance%2)));
        soc_reg_field_set(unit, CI_CONFIG1r, &regval, TREAD_ENBf, (21 - (instance%2)));
        SOC_IF_ERROR_RETURN(soc_reg32_set( unit, CI_CONFIG1r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_CONFIG5r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG5r, &regval, TZQCSf, 38);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_CONFIG5r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_CONFIG2r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG2r, &regval, TRFCf, 75);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_CONFIG2r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        /*  Disables refresh
            soc_reg32_get( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, &regval );
            soc_reg_field_set( unit, CI_CONFIG3r, &regval, TREFIf, 0 );
            soc_reg32_set( unit, CI_CONFIG3r, SOC_BLOCK_PORT(unit,instance), 0, regval);
        */

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_CONFIG5r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG5r, &regval, TZQINITf, 299 );
        soc_reg_field_set(unit, CI_CONFIG5r, &regval, TZQOPERf, 0x96 );
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_CONFIG5r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_CONFIG6r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_CONFIG6r, &regval, TREF_HOLDOFFf, 0x4000);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_CONFIG6r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get( unit, CI_PHY_STRAPS0r, SOC_BLOCK_PORT(unit,instance), 0, &regval ));
        soc_reg_field_set(unit, CI_PHY_STRAPS0r, &regval, BUS16f, 0);
        soc_reg_field_set(unit, CI_PHY_STRAPS0r, &regval, CLf, 14);
        soc_reg_field_set(unit, CI_PHY_STRAPS0r, &regval, CWLf, 9);
        soc_reg_field_set(unit, CI_PHY_STRAPS0r, &regval, WRf, 14);
        soc_reg_field_set(unit, CI_PHY_STRAPS0r, &regval, CHIP_SIZEf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_PHY_STRAPS0r, SOC_BLOCK_PORT(unit,instance), 0, regval));

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CI_PHY_STRAPS1r, SOC_BLOCK_PORT(unit,instance), 0, &regval));
        soc_reg_field_set(unit, CI_PHY_STRAPS1r, &regval, JEDECf, 25);
        soc_reg_field_set(unit, CI_PHY_STRAPS1r, &regval, MHZf, 933);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, CI_PHY_STRAPS1r, SOC_BLOCK_PORT(unit,instance), 0, regval));
    }

    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_reg32_expect_field_timeout(unit, CI_PHY_CONTROLr,
                                                                        -1, -1,
                                                                        SOC_BLOCK_PORT(unit,instance),
                                                                        PWRUP_RSBf, 1, 100));
    }
  
    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance) {
        soc_reg32_get(unit, CI_RESETr, SOC_BLOCK_PORT(unit,instance), 0, &regval);
        soc_reg_field_set(unit, CI_RESETr, &regval, DDR_RESET_Nf, 0);
        soc_reg_field_set(unit, CI_RESETr, &regval, PHY_SW_INITf, 0);
        soc_reg32_set(unit, CI_RESETr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
  
    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance) {
        soc_reg32_get(unit, CI_MR0r, SOC_BLOCK_PORT(unit,instance), 0, &regval);
        regval |= 0x0001;
        regval &= 0xff8f;
        regval |= 0x0010;
        regval |= 0x0004;
        regval &= 0xf1ff;
        regval |= 0x0e00;
        soc_reg32_set(unit, CI_MR0r, SOC_BLOCK_PORT(unit,instance), 0, regval);

        soc_reg32_get(unit, CI_MR2r, SOC_BLOCK_PORT(unit,instance), 0, &regval);
        regval &= 0xffc7;
        regval |= 0x0020;
        soc_reg32_set(unit, CI_MR2r, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
  
    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance) {
        soc_reg32_get(unit, CI_RESETr, SOC_BLOCK_PORT(unit,instance), 0, &regval);
        soc_reg_field_set(unit, CI_RESETr, &regval, DDR_RESET_Nf, 1);
        soc_reg32_set(unit, CI_RESETr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance ) {
        soc_reg32_get( unit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(unit,instance), 0, &regval );
        soc_reg_field_set( unit, CI_PHY_CONTROLr, &regval, DDR_MHZf, 933 );
        soc_reg_field_set( unit, CI_PHY_CONTROLr, &regval, RST_Nf, 3 );
        soc_reg_field_set( unit, CI_PHY_CONTROLr, &regval, AUTO_INITf, 0 );
        soc_reg_field_set( unit, CI_PHY_CONTROLr, &regval, CKEf, 0 );
        soc_reg32_set( unit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }

    regval = 3;
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 1, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_0_RBUS_START + 
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_READ_DATA_DLY),
                                      &regval);
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 1, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_1_RBUS_START +
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_READ_DATA_DLY),
                                      &regval);
    }
    
    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2) {
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 0, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_0_RBUS_START + 
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL),
                                      &regval);
        regval &= 0xfbff;
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 1, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_0_RBUS_START + 
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL),
                                      &regval);
        
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 0, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_1_RBUS_START + 
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL),
                                      &regval);
        regval &= 0xfbff;
        _soc_sbx_caladan3_tmu_ci_phy_read_write( unit, instance, 1, 
                                      (SOC_SBX_CALADAN3_TMU_PHY_WORD_LANE_1_RBUS_START + 
                                       SOC_SBX_CALADAN3_TMU_DDR40_PHY_WORD_LANE_DRIVE_PAD_CTL),
                                      &regval);
    }
  
    for (instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance) {
        soc_reg32_get( unit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(unit,instance), 0, &regval );
        soc_reg_field_set( unit, CI_PHY_CONTROLr, &regval, CKEf, 3 );
        soc_reg32_set( unit, CI_PHY_CONTROLr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    
    regval = 0;
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, DONEf, 1 );
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, REQf, 1 );
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, DEVICE_SELf, 3 );
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, MR_SELf, 2 );
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        soc_reg32_set( unit, CI_MR_CMDr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, MR_SELf, 3 );
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        soc_reg32_set( unit, CI_MR_CMDr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, MR_SELf, 1 );
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        soc_reg32_set( unit, CI_MR_CMDr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    soc_reg_field_set( unit, CI_MR_CMDr, &regval, MR_SELf, 0 );
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        soc_reg32_set( unit, CI_MR_CMDr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    regval = 0;
    soc_reg_field_set( unit, CI_ZQ_CMDr, &regval, DONEf, 1 );
    soc_reg_field_set( unit, CI_ZQ_CMDr, &regval, REQf, 1 );
    soc_reg_field_set( unit, CI_ZQ_CMDr, &regval, DEVICE_SELf, 3 );
    soc_reg_field_set( unit, CI_ZQ_CMDr, &regval, CMDf, 3 );
    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); instance += 2 ) {
        soc_reg32_set( unit, CI_ZQ_CMDr, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }

    for ( instance = 0; instance < SOC_DDR3_NUM_MEMORIES(unit); ++instance ) {
        soc_reg32_get( unit, CI_CONFIG8r, SOC_BLOCK_PORT(unit,instance), 0, &regval );
        soc_reg_field_set( unit, CI_CONFIG8r, &regval, RD_PARITY_ENf, 3 );
        soc_reg_field_set( unit, CI_CONFIG8r, &regval, WR_PARITY_ENf, 7 );
        soc_reg_field_set( unit, CI_CONFIG8r, &regval, RFIFO_PARITY_ENf, 1 );
        soc_reg32_set( unit, CI_CONFIG8r, SOC_BLOCK_PORT(unit,instance), 0, regval);
    }
    
    return SOC_E_NONE;
#endif
}



#endif
