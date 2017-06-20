/*
 * $Id: c3hppc_test.c,v 1.133 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        c3hppc_test.c
 * Purpose:     Run a Caladan3 HPP Cluster test
 *
 *
 * Notes:       
 *             
 *
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "c3hppc_test.h"


#if (defined(LINUX))
extern int bb_caladan3_current(int unit, int bx);
extern int soc_sbx_caladan3_temperature_monitor(int unit, int nsamples, int format, int celsius);
#endif


static c3hppc_test_list_entry_t 
       g_c3hppcTestList[] = {
                             {"c3hppc_ocm_store_test1", c3hppc_ocm_store_test1__init, 
                              c3hppc_ocm_store_test1__run, c3hppc_ocm_store_test1__done},
                             {"c3hppc_ocm_load_test1", c3hppc_ocm_load_test1__init, 
                              c3hppc_ocm_load_test1__run, c3hppc_ocm_load_test1__done},
                             {"c3hppc_ocm_store_load_test1", c3hppc_ocm_store_load_test1__init, 
                              c3hppc_ocm_store_load_test1__run, c3hppc_ocm_store_load_test1__done},
                             {"c3hppc_cmu_test1", c3hppc_cmu_test1__init, 
                              c3hppc_cmu_test1__run, c3hppc_cmu_test1__done},
                             {"c3hppc_cop_test1", c3hppc_cop_test1__init, 
                              c3hppc_cop_test1__run, c3hppc_cop_test1__done},
                             {"c3hppc_cop_test2", c3hppc_cop_test2__init, 
                              c3hppc_cop_test2__run, c3hppc_cop_test2__done},
                             {"c3hppc_cop_test3", c3hppc_cop_test3__init, 
                              c3hppc_cop_test3__run, c3hppc_cop_test3__done},
                             {"c3hppc_cop_test4", c3hppc_cop_test4__init, 
                              c3hppc_cop_test4__run, c3hppc_cop_test4__done},
                             {"c3hppc_sws_test1", c3hppc_sws_test1__init, 
                              c3hppc_sws_test1__run, c3hppc_sws_test1__done},
                             {"c3hppc_sws_test2", c3hppc_sws_test2__init, 
                              c3hppc_sws_test2__run, c3hppc_sws_test2__done},
                             {"c3hppc_sws_test3", c3hppc_sws_test3__init, 
                              c3hppc_sws_test3__run, c3hppc_sws_test3__done},
                             {"c3hppc_sws_test4", c3hppc_sws_test4__init, 
                              c3hppc_sws_test4__run, c3hppc_sws_test4__done},
                             {"c3hppc_sws_test5", c3hppc_sws_test5__init, 
                              c3hppc_sws_test5__run, c3hppc_sws_test5__done},
                             {"c3hppc_rce_test1", c3hppc_rce_test1__init,
                              c3hppc_rce_test1__run, c3hppc_rce_test1__done},
                             {"c3hppc_rce_test2", c3hppc_rce_test2__init,
                              c3hppc_rce_test2__run, c3hppc_rce_test2__done},
                             {"c3hppc_tmu_test1", c3hppc_tmu_test1__init,
                              c3hppc_tmu_test1__run, c3hppc_tmu_test1__done},
                             {"c3hppc_ddr_test_suite", c3hppc_ddr_test_suite__init,
                              c3hppc_ddr_test_suite__run, c3hppc_ddr_test_suite__done},
                             {"c3hppc_etu_test1", c3hppc_etu_test1__init,
                              c3hppc_etu_test1__run, c3hppc_etu_test1__done},
                             {"c3hppc_exerciser_test1", c3hppc_exerciser_test1__init,
                              c3hppc_exerciser_test1__run, c3hppc_exerciser_test1__done},
                             {"c3_exerciser_test1", c3_exerciser_test1__init,
                              c3_exerciser_test1__run, c3_exerciser_test1__done},
                             {"c3_ut_ocm_test1", c3_ut_ocm_test1_init, 
                              c3_ut_ocm_test1_run, c3_ut_ocm_test1_done}
                            };
static int g_nc3hppcTestListCnt = COUNTOF(g_c3hppcTestList);
static c3hppc_test_list_entry_t *g_pc3hppcCurrentTest = NULL;


static sal_thread_t g_CmuCounterRingManagerThreadID = NULL;
static c3hppc_counter_ring_manager_cb_t g_c3hppcCmuCounterRingManagerCB;

static int g_OcmNextPhysicalBlock_0to63, g_OcmNextPhysicalBlock_64to127;

static int g_IterationFromLastReboot = 0;

static int g_nTestDuration;

static int g_nChipConfigFailed;

static int g_nShowCurrent;
static int g_nShowTemp;

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;





static c3hppc_test_info_t  *g_c3hppcTestInfo[SOC_MAX_NUM_DEVICES];
/*
 * Function:
 *      c3hppc_print_param
 *
 * Purpose:
 *      Print parameter structure
 *
 * Parameters:
 *      pc3hppcTestInfo             - parameter structure
 *
 * Returns:
 *      Allocated structure or NULL if alloc failed.
 *
 */

STATIC void
c3hppc_print_param(c3hppc_test_info_t *pc3hppcTestInfo)
{
  if (pc3hppcTestInfo != NULL) {
    if ( pc3hppcTestInfo->sTestName != NULL ) { 
      cli_out("\n\n\n**************************************************************\n");
      cli_out("  Running TEST=%s with SEED=%d\n",  pc3hppcTestInfo->sTestName, pc3hppcTestInfo->nTestSeed);
      cli_out("**************************************************************\n\n\n");
    }
  } else {
    cli_out("pc3hppcTestInfo has no test parameters!!!)\n");
  }
}


/*
 * Function:
 *      c3hppc_alloc
 *
 * Purpose:
 *      Allocate and initialize test parameter structure
 *
 * Parameters:
 *      unit            - Unit these parameters are for
 *
 * Returns:
 *      Allocated structure or NULL if alloc failed.
 *
 */

STATIC c3hppc_test_info_t *
c3hppc_alloc(int unit)
{
  int ucode_num_ports;
  c3hppc_test_info_t *pc3hppcTestInfo;
  char *psTmp;

  soc_cm_get_id( unit, &g_dev_id, &g_rev_id);

  pc3hppcTestInfo = (c3hppc_test_info_t *)sal_alloc(sizeof(c3hppc_test_info_t), "c3hppc test info");

  if (pc3hppcTestInfo != NULL) {
    sal_memset( pc3hppcTestInfo, 0x00, sizeof(c3hppc_test_info_t) );
    pc3hppcTestInfo->nUnit        = unit;
    pc3hppcTestInfo->sTestName    = NULL;
    pc3hppcTestInfo->nTestStatus  = TEST_OK;
    ucode_num_ports = soc_property_get(unit, spn_UCODE_NUM_PORTS, 0);
    if ( ucode_num_ports == 1 && (psTmp = soc_property_port_get_str(unit, 0, spn_UCODE_PORT)) != NULL &&
         strstr(psTmp, "clport0") ) {

      pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100G_BY_100IL; 

    } else {

      switch ( ucode_num_ports ) {
        case 1:    pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS; break;
        case 3:    pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__3x40G_BY_100IL; break;
        case 12:   pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__12x10G_BY_100IL; break;
        case 48:   pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__48x1G_BY_100IL; break;
        default:   pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL;
      }

    }
/*
pc3hppcTestInfo->nTDM = C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL;
*/
    COMPILER_64_ZERO(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
    sal_memset( &(pc3hppcTestInfo->BringUpControl), 0, sizeof(c3hppc_bringup_control_t) );
    pc3hppcTestInfo->BringUpControl.uDefaultPhysicalSegmentSizeIn16kBlocks = 1;
    pc3hppcTestInfo->BringUpControl.uDefaultCmuPhysicalSegmentSizeIn16kBlocks = 1;
  }

  g_c3hppcCmuCounterRingManagerCB.nUnit = unit;
  g_c3hppcCmuCounterRingManagerCB.bExit = 0;
  g_c3hppcCmuCounterRingManagerCB.pBringUpControl = &(pc3hppcTestInfo->BringUpControl);
  g_c3hppcCmuCounterRingManagerCB.uPciCounterBase = 0;
  g_c3hppcCmuCounterRingManagerCB.pPciCounterLoLimit = NULL;
  g_c3hppcCmuCounterRingManagerCB.pPciCounterHiLimit = NULL;
  g_OcmNextPhysicalBlock_0to63 = g_OcmNextPhysicalBlock_64to127 = 0;
  ++g_IterationFromLastReboot;
  g_nTestDuration = 0;
  g_nChipConfigFailed = 0;

  return pc3hppcTestInfo;
}

/*
 * Function:
 *      c3hppc_free
 *
 * Purpose:
 *      Free test parameter structure
 *
 * Parameters:
 *      pc3hppcTestInfo             - parameter structure pointer
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

STATIC int c3hppc_free(c3hppc_test_info_t *pc3hppcTestInfo)
{
  int rv = TEST_FAIL;

  if ( pc3hppcTestInfo == NULL ) {
    return rv;
  }
  if ( pc3hppcTestInfo->sTestName != NULL ) sal_free(pc3hppcTestInfo->sTestName);

  if ( g_c3hppcCmuCounterRingManagerCB.uPciCounterBase ) {
    sal_free( (void *) g_c3hppcCmuCounterRingManagerCB.uPciCounterBase );
  }

  sal_free(pc3hppcTestInfo);

  return TEST_OK;
}

/*
 * Function:
 *      c3hppc_test_init
 *
 * Purpose:
 *      Parse test arguments and save parameter structure locally
 *
 * Parameters:
 *      unit            - unit to test
 *      args            - test arguments
 *      pa              - test cookie (not used)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
c3hppc_test_init(int unit, args_t *args, void **pa)
{
  parse_table_t pt;
  char *psTr172Tr173Test;
  int rc = 0;

  c3hppc_test_info_t *pc3hppcTestInfo = c3hppc_alloc(unit);


  if (pc3hppcTestInfo == NULL) {
    cli_out("%s: out of memory\n", ARG_CMD(args));
    return(TEST_FAIL);
  }

  parse_table_init(unit, &pt);

  parse_table_add(&pt, "TEST", (PQ_STRING|PQ_DFL|PQ_STATIC), "",
                  &(pc3hppcTestInfo->sTestName), NULL);
  parse_table_add(&pt, "SEED", (PQ_INT), (void *)-1,
                  &(pc3hppcTestInfo->nTestSeed), NULL);
  parse_table_add(&pt, "TransferSize", (PQ_INT), (void *)64,
                  &(pc3hppcTestInfo->nTransferSize), NULL);
  parse_table_add(&pt, "BankSwap", (PQ_INT), (void *)1,
                  &(pc3hppcTestInfo->bBankSwap), NULL);
  parse_table_add(&pt, "HotSwap", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nHotSwap), NULL);
  parse_table_add(&pt, "SwitchCount", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nSwitchCount), NULL);
  parse_table_add(&pt, "ProgramSelect", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nProgramSelect), NULL);
  parse_table_add(&pt, "ActiveDmNum", (PQ_INT), (void *)1,
                  &(pc3hppcTestInfo->nActiveDmNum), NULL);
  parse_table_add(&pt, "DmType", (PQ_INT), (void *)494,
                  &(pc3hppcTestInfo->nDmType), NULL);
  parse_table_add(&pt, "EML64Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEML64Enable), NULL);
  parse_table_add(&pt, "EML176Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEML176Enable), NULL);
  parse_table_add(&pt, "EML304Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEML304Enable), NULL);
  parse_table_add(&pt, "EML424Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEML424Enable), NULL);
  parse_table_add(&pt, "EMC64Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEMC64Enable), NULL);
  parse_table_add(&pt, "IPV4Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bIPV4Enable), NULL);
  parse_table_add(&pt, "IPV6Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bIPV6Enable), NULL);
  parse_table_add(&pt, "IPV4BucketPrefixNum", (PQ_INT), (void *)C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM,
                  &(pc3hppcTestInfo->nIPV4BucketPrefixNum), NULL);
  parse_table_add(&pt, "IPV6BucketPrefixNum", (PQ_INT), (void *)C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM,
                  &(pc3hppcTestInfo->nIPV6BucketPrefixNum), NULL);
  parse_table_add(&pt, "TapsUnified", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bTapsUnified), NULL);
  parse_table_add(&pt, "BulkDeleteEnable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bBulkDeleteEnable), NULL);
  parse_table_add(&pt, "EMLInsertEnable", (PQ_INT), (void *)(pc3hppcTestInfo->bEML64Enable | 
                                                             pc3hppcTestInfo->bEML176Enable | 
                                                             pc3hppcTestInfo->bEML304Enable |
                                                             pc3hppcTestInfo->bEML424Enable),
                  &(pc3hppcTestInfo->bEMLInsertEnable), NULL);
  parse_table_add(&pt, "LrpLearningEnable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bLrpLearningEnable), NULL);
  parse_table_add(&pt, "OamEnable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bOamEnable), NULL);
  parse_table_add(&pt, "EML144Enable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bEML144Enable), NULL);
  parse_table_add(&pt, "IPV6CollisionEnable", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bIPV6CollisionEnable), NULL);
  parse_table_add(&pt, "CacheEnable", (PQ_INT), (void *)1,
                  &(pc3hppcTestInfo->bCacheEnable), NULL);
  parse_table_add(&pt, "PerformanceTest", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bPerformanceTest), NULL);
  parse_table_add(&pt, "BypassScrambler", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bBypassScrambler), NULL);
  parse_table_add(&pt, "BypassHash", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bBypassHash), NULL);
  parse_table_add(&pt, "SkipCiDramInit", (PQ_INT), (void *)((SAL_BOOT_QUICKTURN) ? 1 : 0),
                  &(pc3hppcTestInfo->bSkipCiDramInit), NULL);
  parse_table_add(&pt, "SkipCiDramTest", PQ_INT, (void *)((SAL_BOOT_QUICKTURN) ? 1 : 0),
                  &(pc3hppcTestInfo->bSkipCiDramSelfTest), NULL);
  parse_table_add(&pt, "DramFreq", PQ_INT, (void *)((g_dev_id == BCM88034_DEVICE_ID) ? 800 : DDR_FREQ_933),
                  &(pc3hppcTestInfo->nDramFreq), NULL);
  parse_table_add(&pt, "LrpFreq", PQ_INT, (void *)((g_dev_id == BCM88039_DEVICE_ID) ? 1100 : ((g_dev_id == BCM88034_DEVICE_ID) ? 742 : 1000)),
                  &(pc3hppcTestInfo->nLrpFreq), NULL);
  parse_table_add(&pt, "SwsFreq", PQ_INT, (void *)641,
                  &(pc3hppcTestInfo->nSwsFreq), NULL);
  parse_table_add(&pt, "TmuFreq", PQ_INT, (void *)((g_dev_id == BCM88034_DEVICE_ID) ? 495 : 0),
                  &(pc3hppcTestInfo->nTmuFreq), NULL);
  parse_table_add(&pt, "NL11KFreq", PQ_INT, (void *)312,
                  &(pc3hppcTestInfo->nNL11KFreq), NULL);
  parse_table_add(&pt, "PauseDisable", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bPauseDisable), NULL);
  parse_table_add(&pt, "XLportEnable", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bXLportEnable), NULL);
  parse_table_add(&pt, "LineExternalLoopBack", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bLineExternalLoopBack), NULL);
  parse_table_add(&pt, "FabricExternalLoopBack", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bFabricExternalLoopBack), NULL);
  parse_table_add(&pt, "InternalLoopBack", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bInternalLoopBack), NULL);
  parse_table_add(&pt, "LineTrafficGenOnly", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bLineTrafficGenOnly), NULL);
  parse_table_add(&pt, "FabricTrafficGenOnly", PQ_INT, (void *)0,
                  &(pc3hppcTestInfo->bFabricTrafficGenOnly), NULL);
  parse_table_add(&pt, "HostActivity", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nHostActivityControl), NULL);
  parse_table_add(&pt, "MaxKey", (PQ_INT), (void *)((SAL_BOOT_QUICKTURN) ? 0x01000 : 0x10000),
                  &(pc3hppcTestInfo->nMaxKey), NULL);
  parse_table_add(&pt, "CounterNum", (PQ_INT), (void *)1024,
                  &(pc3hppcTestInfo->nCounterNum), NULL);
  parse_table_add(&pt, "CiNum", (PQ_INT), (void *)((g_dev_id == BCM88034_DEVICE_ID) ? 8 : C3HPPC_TMU_CI_INSTANCE_NUM),
                  &(pc3hppcTestInfo->nNumberOfCIs), NULL);
  parse_table_add(&pt, "ReplicationFactor", (PQ_INT), (void *)4,
                  &(pc3hppcTestInfo->uReplicationFactor), NULL);
  parse_table_add(&pt, "Periodic", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nPeriodic), NULL);
  parse_table_add(&pt, "SetupOptions", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nSetupOptions), NULL);
  parse_table_add(&pt, "NoTmu", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bNoTmu), NULL);
  parse_table_add(&pt, "NoRce", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bNoRce), NULL);
  parse_table_add(&pt, "NoEtu", (PQ_INT), (void *)((g_dev_id == BCM88034_DEVICE_ID) ? 1 : 0),
                  &(pc3hppcTestInfo->bNoEtu), NULL);
  parse_table_add(&pt, "NoPpe", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bNoPpe), NULL);
  parse_table_add(&pt, "ShowCurrent", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nShowCurrent), NULL);
  parse_table_add(&pt, "ShowTemp", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nShowTemp), NULL);
  parse_table_add(&pt, "ModulateTimer", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nModulateTimer), NULL);
  parse_table_add(&pt, "ErrorInject", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nErrorInject), NULL);
  parse_table_add(&pt, "EpochCount", (PQ_INT64), (void *)0,
                  &(pc3hppcTestInfo->uuEpochCount), NULL);
  parse_table_add(&pt, "Iterations", (PQ_INT64), (void *)0,
                  &(pc3hppcTestInfo->uuIterations), NULL);
  /* Added for B0 */
  parse_table_add(&pt, "PacketSize", (PQ_INT), (void *)((g_rev_id == BCM88030_B0_REV_ID) ? 64 : C3HPPC_TEST_150MPPS_PACKET_SIZE),
                  &(pc3hppcTestInfo->nPacketSize), NULL);
  parse_table_add(&pt, "IPG", (PQ_INT), (void *)(void *)((g_rev_id == BCM88030_B0_REV_ID) ? 2 : 1),
                  &(pc3hppcTestInfo->nIPG), NULL);
  parse_table_add(&pt, "ShmooUnderLoad", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->bShmooUnderLoad), NULL);
  parse_table_add(&pt, "LinePortNum", (PQ_INT), (void *)0,
                  &(pc3hppcTestInfo->nLineSidePortNum), NULL);



  /* Parse arguments */
  if (0 > parse_arg_eq(args, &pt)) {
    test_error(unit,
               "%s: Invalid option: %s\n",
               ARG_CMD(args),
               ARG_CUR(args) ? ARG_CUR(args) : "*");
    parse_arg_eq_done(&pt);
    return(TEST_FAIL);
  }

  if ( pc3hppcTestInfo->bShmooUnderLoad == 1 ) pc3hppcTestInfo->bNoTmu = 1;

  if ( pc3hppcTestInfo->nTestSeed == -1 ) {
    sal_srand( (int)sal_time() );
    pc3hppcTestInfo->nTestSeed = sal_rand();
    pc3hppcTestInfo->BringUpControl.uSTACEseed = (uint32) (pc3hppcTestInfo->nTestSeed * 17);
  }
  sal_srand(pc3hppcTestInfo->nTestSeed);

  switch (pc3hppcTestInfo->nTransferSize) {
    case 64:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
      break;
    case 32:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_LONGWORD;
      break;
    case 16:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_WORD;
      break;
    case 8:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_BYTE;
      break;
    case 4:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_NIBBLE;
      break;
    case 2:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_DBIT;
      break;
    case 1:
      pc3hppcTestInfo->nTransferSize = C3HPPC_DATUM_SIZE_BIT;
      break;
    case 0:
      pc3hppcTestInfo->nTransferSize = sal_rand() % C3HPPC_OCM_NUM_OF_TRANSFER_SIZES;
      break;
    default:
      test_error(unit, "%d -> is an invalid transfer size!\n", pc3hppcTestInfo->nTransferSize );
      parse_arg_eq_done(&pt);
      return(TEST_FAIL);
  } 

  if ( !((!(pc3hppcTestInfo->nIPV4BucketPrefixNum % C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM) ||
          !((pc3hppcTestInfo->nIPV4BucketPrefixNum - C3HPPC_TMU_IPV4_128b_BUCKET_PREFIX_NUM) % C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM)) &&
         pc3hppcTestInfo->nIPV4BucketPrefixNum <= 56) ) {
    test_error(unit, "%d -> is an invalid IPV4 Bucket Prefix Number!\n", pc3hppcTestInfo->nIPV4BucketPrefixNum );
    parse_arg_eq_done(&pt);
    return(TEST_FAIL);
  }
  if ( !((!(pc3hppcTestInfo->nIPV6BucketPrefixNum % C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM) ||
          !((pc3hppcTestInfo->nIPV6BucketPrefixNum - C3HPPC_TMU_IPV6_128b_BUCKET_PREFIX_NUM) % C3HPPC_TMU_IPV6_256b_BUCKET_PREFIX_NUM)) &&
         pc3hppcTestInfo->nIPV6BucketPrefixNum <= 40) ) {
    test_error(unit, "%d -> is an invalid IPV6 Bucket Prefix Number!\n", pc3hppcTestInfo->nIPV6BucketPrefixNum );
    parse_arg_eq_done(&pt);
    return(TEST_FAIL);
  }

  parse_arg_eq_done(&pt);

/*
  if (pc3hppcTestInfo->pkt_size >  0) {
    test_error(unit,
               "%s: Error: packet size > alloc size\n",
               ARG_CMD(args));
    parse_arg_eq_done(&pt);
    return(TEST_FAIL);
  }
*/

  c3hppc_print_param( pc3hppcTestInfo );

  g_c3hppcTestInfo[unit] = pc3hppcTestInfo;

  g_nShowCurrent = pc3hppcTestInfo->nShowCurrent;
  g_nShowTemp = pc3hppcTestInfo->nShowTemp;


  if ( (psTr172Tr173Test = sal_config_get("c3_ucode_test")) != NULL ) {

    cli_out("\n\ntr172/tr173 invocation ...\n\n" );

    if (pc3hppcTestInfo->sTestName == NULL) {
        pc3hppcTestInfo->sTestName = sal_alloc(64, "testname");
      if (pc3hppcTestInfo->sTestName) {
        sal_sprintf(pc3hppcTestInfo->sTestName, "%s", "c3_exerciser_test1");
      } else {
        return(TEST_FAIL);
      }
    }

    if (COMPILER_64_IS_ZERO(pc3hppcTestInfo->uuIterations) ) {
      COMPILER_64_SET(pc3hppcTestInfo->uuIterations,0,1);
    }
    if ( !sal_strcmp(psTr172Tr173Test,"172_48G") ) {
      pc3hppcTestInfo->bLineExternalLoopBack = 0;
      pc3hppcTestInfo->bFabricExternalLoopBack = 1;
      pc3hppcTestInfo->bLineTrafficGenOnly = 1;
      pc3hppcTestInfo->bFabricTrafficGenOnly = 0;
    } else if ( !sal_strcmp(psTr172Tr173Test,"172_100G") ) {
      pc3hppcTestInfo->bLineExternalLoopBack = 1;
      pc3hppcTestInfo->bFabricExternalLoopBack = 1;
      pc3hppcTestInfo->bLineTrafficGenOnly = 1;
      pc3hppcTestInfo->bFabricTrafficGenOnly = 0;
    } else if ( !sal_strcmp(psTr172Tr173Test,"173_48G") ||
                !sal_strcmp(psTr172Tr173Test,"173_100G") ) {
      pc3hppcTestInfo->bLineExternalLoopBack = 1;
      pc3hppcTestInfo->bFabricExternalLoopBack = 1;
      pc3hppcTestInfo->bLineTrafficGenOnly = 0;
      pc3hppcTestInfo->bFabricTrafficGenOnly = 1;
    } else {
      test_error(unit, "%s -> is an invalid \"c3_ucode_test\" setting!\n", psTr172Tr173Test );
      parse_arg_eq_done(&pt);
      return(TEST_FAIL);
    }

    if ( pc3hppcTestInfo->bInternalLoopBack ) {
      pc3hppcTestInfo->bLineExternalLoopBack = 0;
      pc3hppcTestInfo->bFabricExternalLoopBack = 0;
    }
  }


  if ( (g_pc3hppcCurrentTest = c3hppc_find_test(pc3hppcTestInfo->sTestName)) == NULL ) {
    test_error(unit,
               "Test %s NOT found!\n", pc3hppcTestInfo->sTestName);
    return(TEST_FAIL);
  }

  /*
   * Invoke test specific init routine
   */
  g_nChipConfigFailed = g_pc3hppcCurrentTest->pfuncTestInit(g_c3hppcTestInfo[unit], NULL);
  if ( g_nChipConfigFailed == 0 ) {

    c3hppc_lrp_hw_cleanup( unit );
    c3hppc_cmu_hw_cleanup( unit );
    c3hppc_ocm_hw_cleanup( unit );
    c3hppc_cop_hw_cleanup( unit );
    c3hppc_rce_hw_cleanup( unit );
    c3hppc_tmu_hw_cleanup( unit );
    if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp ) {
      rc =  c3hppc_etu_hw_cleanup( unit );
      if ( rc != 0 ) {
        c3hppc_etu_display_error_state( unit );
        g_nChipConfigFailed = 1;
        test_error(unit,
                   "Test %s ETU clean-up FAILED!\n", pc3hppcTestInfo->sTestName);
      }
    }
    if ( pc3hppcTestInfo->BringUpControl.uSwsBringUp ) c3hppc_sws_hw_cleanup( unit ); 

  /* Only do this when an Interrupt Handler is ready.
    WRITE_CMIC_CMC0_PCIE_IRQ_MASK3r( unit, 0xffffffff );
    WRITE_CMIC_CMC0_PCIE_IRQ_MASK4r( unit, 0xffffffff );
  */

    if ( pc3hppcTestInfo->bIPV6CollisionEnable ) {
      c3hppc_tmu_enable_ipv6_3rd_probe( unit );
    }

    if ( pc3hppcTestInfo->bEML144Enable ) {
      c3hppc_tmu_enable_eml_144_mode( unit );
    }

  }

  return TEST_OK;
}

/*
 * Function:
 *      c3hppc_test_run
 *
 * Purpose:
 *      Runs TX test
 *
 * Parameters:
 *      unit            - unit to test
 *      a               - test arguments (ignored)
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
c3hppc_test_run(int unit, args_t *a, void *pa)
{

  if ( g_nChipConfigFailed == 0 ) {
    g_pc3hppcCurrentTest->pfuncTestRun(g_c3hppcTestInfo[unit], NULL);
  }

  return 0;
}


/*
 * Function:
 *      c3hppc_test_done
 *
 * Purpose:
 *      Cleans up after tx or rx test completes
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
c3hppc_test_done(int unit, void *pa)
{
  int nTestStatus, rc;
  uint32 uRegisterValue, uCmicStat3, uCmicStat4;
  char *psStopOnError;

  if ( g_nChipConfigFailed == 1 ) {
    cli_out("\n\n\n##############################################################\n");
    test_error(unit, "c3hppc test [%s] using seed[%d] FAILED!          {iteration %d}  RESULT[2]\n",
                     g_c3hppcTestInfo[unit]->sTestName,  g_c3hppcTestInfo[unit]->nTestSeed, g_IterationFromLastReboot );
    cli_out("##############################################################\n\n\n");
    c3hppc_free( g_c3hppcTestInfo[unit] );
    if ( (psStopOnError = sal_config_get("stop_on_error")) != NULL ) {
      if ( !sal_strcmp(psStopOnError,"1") ) assert(0);
    }
    return -1;
  }

  /*
   * Perform CMU flush operations to get accurate view of counters.
  */
  if ( g_c3hppcTestInfo[unit]->BringUpControl.uCmuBringUp ) {
    SOC_IF_ERROR_RETURN( WRITE_CM_BACKGROUND_EJECT_ENABLEr( unit, 0 ) );
    if ( c3hppc_cmu_segments_flush( unit, g_c3hppcTestInfo[unit]->BringUpControl.aCmuSegmentInfo ) ) {
      g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
    }

    if ( g_c3hppcTestInfo[unit]->BringUpControl.auIlBringUp[0] || 
         g_c3hppcTestInfo[unit]->BringUpControl.auIlBringUp[1] ) {
      c3hppc_sws_il_stats_collect( unit );
    }
  }

  if ( g_c3hppcTestInfo[unit]->BringUpControl.uTmuBringUp && c3hppc_tmu_get_rsp_fifos_error_count() ) {
    g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }


  g_pc3hppcCurrentTest->pfuncTestDone(g_c3hppcTestInfo[unit], NULL);


  if ( g_CmuCounterRingManagerThreadID != NULL ) { 
    g_c3hppcCmuCounterRingManagerCB.bExit = 1; 
  }
  if ( g_c3hppcTestInfo[unit]->BringUpControl.uTmuBringUp ) {
    c3hppc_tmu_exit_update_manager_thread();
  }
  if ( g_c3hppcTestInfo[unit]->BringUpControl.uEtuBringUp ) {
    if ( c3hppc_etu_exit_update_manager_thread() ) g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }
  c3hppc_cop_exit_watchdogtimer_ring_manager_thread();

  sal_sleep(1);


  if ( c3hppc_cop_get_watchdogtimer_ring_manager_error_count() ) {
    g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }

  /* Ensure the test was not aborted via a CTRL-C to avoid giving false "PASS" status. */
  if ( sal_strcmp( g_c3hppcTestInfo[unit]->sTestName, "c3hppc_ddr_test_suite") ) {
    READ_LRA_EVENTr( unit, &uRegisterValue );
    if ( soc_reg_field_get(unit, LRA_EVENTr, uRegisterValue, OFFLINEf) ) {
      uRegisterValue = 0;
      soc_reg_field_set(unit, LRA_EVENTr, &uRegisterValue, OFFLINEf, 1);
      WRITE_LRA_EVENTr( unit, uRegisterValue );
    } else if ( g_c3hppcTestInfo[unit]->BringUpControl.uSwsBringUp == 0 && COMPILER_64_LO(g_c3hppcTestInfo[unit]->uuIterations) ) {
      cli_out("\n\nERROR --> LRP did NOT finish test execution!\n\n" );
      g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
    }
  }

  soc_pci_getreg(unit, soc_reg_addr(unit, CMIC_CMC0_FIFO_CH2_RD_DMA_STATr, REG_PORT_ANY, 0), &uRegisterValue);
  if ( soc_reg_field_get(unit, CMIC_CMC0_FIFO_CH2_RD_DMA_STATr, uRegisterValue, SBUSACK_NACKf) ) {
    c3hppc_tmu__dump_cmic_rd_dma_state( unit );
    g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }

  READ_CMIC_CMC0_IRQ_STAT3r( unit, &uCmicStat3 );
  if ( uCmicStat3 ) {
    cli_out("\nRegister[CMIC_CMC0_IRQ_STAT3] --> 0x%08x\n", uCmicStat3 );
    g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }
  READ_CMIC_CMC0_IRQ_STAT4r( unit, &uCmicStat4 );
  if ( uCmicStat4 ) {
    cli_out("\nRegister[CMIC_CMC0_IRQ_STAT4] --> 0x%08x\n", uCmicStat4 );
    g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
  }


  if ( uCmicStat3 || uCmicStat4 || g_c3hppcTestInfo[unit]->nTestStatus == TEST_FAIL ) {
    rc = c3hppc_lrp_display_error_state(unit) +
         c3hppc_cmu_display_error_state(unit) +
         c3hppc_ocm_display_error_state(unit) +
         c3hppc_cop_display_error_state(unit) +
         ( (g_c3hppcTestInfo[unit]->BringUpControl.uRceBringUp == 1) ? c3hppc_rce_display_error_state(unit) : 0 ) +
         ( (g_c3hppcTestInfo[unit]->BringUpControl.uTmuBringUp == 1) ? c3hppc_tmu_display_error_state(unit) : 0 ) +
         ( (g_c3hppcTestInfo[unit]->BringUpControl.uEtuBringUp == 1) ? c3hppc_etu_display_error_state(unit) : 0 ) +
         ( (g_c3hppcTestInfo[unit]->BringUpControl.uSwsBringUp == 1) ? c3hppc_sws_display_error_state(unit) : 0 );
    if ( rc ) g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL; 
  }

  if ( g_c3hppcTestInfo[unit]->BringUpControl.uSwsBringUp == 1 ) {
    c3hppc_sws_il_stats_close( unit );
    if ( c3hppc_sws_check_free_page_list( unit ) ) {
      g_c3hppcTestInfo[unit]->nTestStatus = TEST_FAIL;
      cli_out("\n\nERROR --> SWS free page list check failed!\n\n" );
    }
  }


  /*
   * Setup this register so that OCM memory can be viewed via BCM.0> dump commands.
   * LRP P0 has a global view of OCM.  Setup LRP P0 Ocm Port so that there is a 
   * 1 to 1 mapping of logical to physical addresses.
  */
  c3hppc_ocm_port_program_port_table( unit, c3hppc_ocm_map_lrp2ocm_port(0), 0,
                                      (C3HPPC_OCM_MAX_PHY_BLK * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS - 1),
                                      C3HPPC_DATUM_SIZE_QUADWORD, 0);
  uRegisterValue = 0;
  soc_reg_field_set(unit, OC_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1);
  soc_reg_field_set(unit, OC_CONFIGr, &uRegisterValue, PROC_PORT_SEGMENTf, C3HPPC_DATUM_SIZE_QUADWORD);
  soc_reg_field_set(unit, OC_CONFIGr, &uRegisterValue, PROC_PORT_IDf, 0);
  SOC_IF_ERROR_RETURN( WRITE_OC_CONFIGr(unit, uRegisterValue) );


  if ( g_c3hppcTestInfo[unit]->nTestStatus == TEST_OK ) {
    cli_out("\n\n\n**************************************************************\n");
    cli_out("c3hppc test [%s] PASSED!       {iteration %d}  RESULT[1]\n", g_c3hppcTestInfo[unit]->sTestName, g_IterationFromLastReboot );
    cli_out("**************************************************************\n\n\n");
  } else {
    cli_out("\n\n\n##############################################################\n");
    test_error(unit, "c3hppc test [%s] using seed[%d] FAILED!          {iteration %d}  RESULT[0]\n",
                     g_c3hppcTestInfo[unit]->sTestName,  g_c3hppcTestInfo[unit]->nTestSeed, g_IterationFromLastReboot );
    cli_out("##############################################################\n\n\n");
  }

  

  nTestStatus = g_c3hppcTestInfo[unit]->nTestStatus;


  if ( c3hppc_free(g_c3hppcTestInfo[unit]) || nTestStatus ) {
    nTestStatus = -1;
    if ( (psStopOnError = sal_config_get("stop_on_error")) != NULL ) {
      if ( !sal_strcmp(psStopOnError,"1") ) assert(0);
    }
  }

  return nTestStatus;
}


uint32 
c3hppc_test__get_interrupt_summary( int nUnit )
{
  uint32 uCmicStat3, uCmicStat4, uQeError, uQeDebug, uQeMask, uCorrectableError, uUnCorrectableError;
  uint32 uCorrectableMemId, uUnCorrectableMemId, uRegisterValue;
  int nQe;

  READ_CMIC_CMC0_IRQ_STAT3r( nUnit, &uCmicStat3 );
  READ_CMIC_CMC0_IRQ_STAT4r( nUnit, &uCmicStat4 );

  /*
     The following QE correctable/uncorrectable errors need to be ignored due to (JIRA CA3-2615)
  */
  if ( (uCmicStat4 & 0x3fffc000) ) {
    for ( nQe = 0; nQe < C3HPPC_TMU_QE_INSTANCE_NUM; ++nQe ) {
      uQeMask = 1 << (29 - nQe);
      if ( (uCmicStat4 & uQeMask) ) {
        soc_reg32_get( nUnit, TM_QE_ERRORr, SOC_BLOCK_PORT(nUnit,nQe), 0, &uQeError );
        uCorrectableError = soc_reg_field_get( nUnit, TM_QE_ERRORr, uQeError, CORRECTED_ERRORf );
        uUnCorrectableError = soc_reg_field_get( nUnit, TM_QE_ERRORr, uQeError, UNCORRECTED_ERRORf );
        if ( uCorrectableError || uUnCorrectableError ) {
          soc_reg32_get( nUnit, TM_QE_ECC_DEBUGr, SOC_BLOCK_PORT(nUnit,nQe), 0, &uQeDebug );
          uCorrectableMemId = soc_reg_field_get( nUnit, TM_QE_ECC_DEBUGr, uQeDebug, CORRECTABLE_ECC_ERROR_MEM_IDf );
          uUnCorrectableMemId = soc_reg_field_get( nUnit, TM_QE_ECC_DEBUGr, uQeDebug, UNCORRECTABLE_ECC_ERROR_MEM_IDf );
          if ( uCorrectableError && uCorrectableMemId == 4 ) {
            uRegisterValue = 0;
            soc_reg_field_set( nUnit, TM_QE_ERRORr, &uRegisterValue, CORRECTED_ERRORf, 1 );
            soc_reg32_set( nUnit, TM_QE_ERRORr, SOC_BLOCK_PORT(nUnit,nQe), 0, uRegisterValue );
          }
          if ( uUnCorrectableError && uUnCorrectableMemId == 0 ) {
            uRegisterValue = 0;
            soc_reg_field_set( nUnit, TM_QE_ERRORr, &uRegisterValue, UNCORRECTED_ERRORf, 1 );
            soc_reg32_set( nUnit, TM_QE_ERRORr, SOC_BLOCK_PORT(nUnit,nQe), 0, uRegisterValue );
          }
          soc_reg32_set( nUnit, TM_QE_ECC_DEBUGr, SOC_BLOCK_PORT(nUnit,nQe), 0, 0 );
          uCmicStat4 ^= uQeMask;
        }
      }
    }
  }

  return ( uCmicStat3 | uCmicStat4 );
}




/*
 * Function:
 *      c3hppc_find_test
 *
 * Purpose:
 *      
 *
 * Parameters:
 *      sKey            - test name search key
 *
 * Returns:
 *      Pointer to test entry
 */

c3hppc_test_list_entry_t *
c3hppc_find_test(char *sKey)
{
  int nKeyLen, i;
  c3hppc_test_list_entry_t *pc3hppcTestListEntry = NULL;

  nKeyLen = sal_strlen(sKey);
  for (i = 0; (pc3hppcTestListEntry == NULL && i < g_nc3hppcTestListCnt); i++) {
    if (!sal_strncasecmp(sKey, g_c3hppcTestList[i].sTestName, nKeyLen)) {
      pc3hppcTestListEntry = &g_c3hppcTestList[i];
    }
  }
  return pc3hppcTestListEntry;
}


/*
 * Function:
 *      c3hppc_is_test_done
 *
 * Purpose:
 *      Check to see if LRP is offline
 *
 * Parameters:
 *      nUnit           - device unit
 *
 * Returns:
 *      1 --> means test complete, 0 --> not complete
 */

int
c3hppc_is_test_done(int nUnit)
{
  uint32 uRegisterValue, uOffline;
  sal_time_t  now;

  READ_LRA_EVENTr( nUnit, &uRegisterValue );
  uOffline = soc_reg_field_get( nUnit, LRA_EVENTr, uRegisterValue, OFFLINEf );

  if ( uOffline ) {
    now = sal_time();
    cli_out("\nDetected LRP \"offline\" event at time %d!\n", (int) now );
  }
   

  return uOffline;
}


/*
 * Function:
 *      c3hppc_wait_for_test_done
 *
 * Purpose:
 *      Check to see if LRP is offline
 *
 * Parameters:
 *      nUnit           - device unit
 *
 * Returns:
 *      0 --> means test complete, -1 --> timeout / not complete
 */

int
c3hppc_wait_for_test_done(int nUnit)
{
  uint32 rc, nTimeout;
  sal_time_t StartTimeStamp, EndTimeStamp;
  uint64 uuNumberOfEpochsToRun;
  uint32 uRegisterValue1, uRegisterValue2;

  READ_LRA_EPOCH_COUNT0r( nUnit, &uRegisterValue1 );
  READ_LRA_EPOCH_COUNT1r( nUnit, &uRegisterValue2 );
  COMPILER_64_SET(uuNumberOfEpochsToRun,uRegisterValue2, uRegisterValue1);
  if ( SAL_BOOT_QUICKTURN ) {
    /* To determine the duration (in seconds) assuming 10ms per epoch */
    if (soc_sbx_div64(uuNumberOfEpochsToRun, 100, &nTimeout) == SOC_E_PARAM) return SOC_E_INTERNAL;
  } else {
    /* Should get the epoch length to determine the duration (in seconds) but for now assuming 2.0us per epoch */
    if (soc_sbx_div64(uuNumberOfEpochsToRun, 500000, &nTimeout) == SOC_E_PARAM) return SOC_E_INTERNAL;
  }
  nTimeout = C3HPPC_MAX( 1, nTimeout );


  rc = c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, LRA_EVENTr, OFFLINEf, 1, nTimeout, 0, &EndTimeStamp );

  if ( rc ) {
    cli_out("\n\n<c3hppc_wait_for_test_done>   ERROR:  TIMEOUT detected!  \n\n");
    g_c3hppcTestInfo[nUnit]->nTestStatus = TEST_FAIL;
  } else {
    cli_out("\n\nDetected the LRP \"offline\" event ...\n");
  }

  StartTimeStamp = c3hppc_lrp_get_start_timestamp();

  g_nTestDuration = (int) (EndTimeStamp - StartTimeStamp);

  cli_out("\n\nTest Duration ---> %d seconds  \n\n", g_nTestDuration);

  return rc;
}


int
c3hppc_test__get_test_duration( void )
{
  return g_nTestDuration;
}

/*
 * Function:
 *      c3hppc_bringup
 *
 * Purpose:
 *      Calls block drivers for device bringup
 *
 * Parameters:
 *      pBringUpControl    - c3hppc_bringup_control_t control structure
 *
 * Returns:
 *      Bring-up status
 */
int
c3hppc_bringup(int nUnit, c3hppc_bringup_control_t *pBringUpControl) {

  int nCopInstance, nDramInitDone, nInstance;
  uint32 uRegisterValue, uFieldValue;
  c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
  c3hppc_ocm_control_info_t   c3hppcOcmControlInfo;
  c3hppc_cop_control_info_t   c3hppcCopControlInfo;
  c3hppc_cmu_control_info_t   c3hppcCmuControlInfo;
  c3hppc_rce_control_info_t   c3hppcRceControlInfo;
  c3hppc_tmu_control_info_t   c3hppcTmuControlInfo;
  c3hppc_etu_control_info_t   c3hppcEtuControlInfo;
  c3hppc_sws_control_info_t   c3hppcSwsControlInfo;
  int nSegment, nCounters, nTotalCounterNum, nDramFreq;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;

  /*
   * Take block out of soft reset via CX
  */
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, CX_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1 );
  WRITE_CX_CONFIGr( nUnit, uRegisterValue );

  /*
   * Alter CMIC/SBUS clock
  READ_CX_MAC_PLL_CHANNEL_1r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_MAC_PLL_CHANNEL_1r, &uRegisterValue, MDIVf, 7 );
  WRITE_CX_MAC_PLL_CHANNEL_1r( nUnit, uRegisterValue );
  READ_CX_PLL_CTRLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_PLL_CTRLr, &uRegisterValue, MAC_BOND_OVERRIDEf, 1 );
  WRITE_CX_PLL_CTRLr( nUnit, uRegisterValue );
  READ_CX_MAC_PLL_CHANNEL_1r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_MAC_PLL_CHANNEL_1r, &uRegisterValue, LOAD_ENf, 1 );
  WRITE_CX_MAC_PLL_CHANNEL_1r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_MAC_PLL_CHANNEL_1r, &uRegisterValue, LOAD_ENf, 0 );
  WRITE_CX_MAC_PLL_CHANNEL_1r( nUnit, uRegisterValue );
  */

  READ_CX_SOFT_RESET_0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, LR_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, OC_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, RC_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CO0_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CO1_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CM_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CI_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, ET_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PB_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PR0_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PR1_RESET_Nf, 0 );
/* This is done in the PT bring-up routine
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PT0_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PT1_RESET_Nf, 0 );
*/
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, QM_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PD_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PP_RESET_Nf, 0 );
/* This is done in the IL bring-up routine
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, IL0_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, IL1_RESET_Nf, 0 );
*/
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, LR_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, OC_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, RC_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CO0_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CO1_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CM_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, CI_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, ET_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PB_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PR0_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PR1_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PT0_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PT1_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, QM_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PD_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, PP_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, IL0_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, IL1_RESET_Nf, 1 );
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );

  READ_CX_SOFT_RESET_1r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TMA_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TMB_RESET_Nf, 0 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TIMESTAMP_RESET_Nf, 0 );
  WRITE_CX_SOFT_RESET_1r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TMA_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TMB_RESET_Nf, 1 );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_1r, &uRegisterValue, TIMESTAMP_RESET_Nf, 1 );
  WRITE_CX_SOFT_RESET_1r( nUnit, uRegisterValue );


  /* Modify PLL for TOD to drive a 125MHz clock instead of the default 250MHz
  READ_CX_TS_PLL_CHANNEL_3r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_TS_PLL_CHANNEL_3r, &uRegisterValue, HOLDf, 1 );
  WRITE_CX_TS_PLL_CHANNEL_3r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_TS_PLL_CHANNEL_3r, &uRegisterValue, MDIVf, 24 );
  WRITE_CX_TS_PLL_CHANNEL_3r( nUnit, uRegisterValue );
  READ_CX_PLL_CTRLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_PLL_CTRLr, &uRegisterValue, TS_BOND_OVERRIDEf, 1 );
  WRITE_CX_PLL_CTRLr( nUnit, uRegisterValue );
  READ_CX_TS_PLL_CHANNEL_3r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_TS_PLL_CHANNEL_3r, &uRegisterValue, HOLDf, 0 );
  WRITE_CX_TS_PLL_CHANNEL_3r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_TS_PLL_CHANNEL_3r, &uRegisterValue, LOAD_ENf, 1 );
  WRITE_CX_TS_PLL_CHANNEL_3r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_TS_PLL_CHANNEL_3r, &uRegisterValue, LOAD_ENf, 0 );
  WRITE_CX_TS_PLL_CHANNEL_3r( nUnit, uRegisterValue );

  WRITE_IEEE1588_TIME_FREQ_CONTROLr(nUnit, 0x20000000 );     8ns increment with 125MHz ts clock
  */

  WRITE_IEEE1588_TIME_FREQ_CONTROLr(nUnit, 0x10000000 );  /* 8ns increment with 250MHz ts clock */
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, IEEE1588_TIME_CONTROLr, &uRegisterValue, LOAD_ENABLEf, 0x1f );
  WRITE_IEEE1588_TIME_CONTROLr( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, IEEE1588_TIME_CONTROLr, &uRegisterValue, LOAD_ENABLEf, 0 );
  WRITE_IEEE1588_TIME_CONTROLr( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, IEEE1588_TIME_CONTROLr, &uRegisterValue, COUNT_ENABLEf, 1 );
  WRITE_IEEE1588_TIME_CONTROLr( nUnit, uRegisterValue );



  if ( g_c3hppcTestInfo[nUnit]->nTmuFreq ) {
    switch ( g_c3hppcTestInfo[nUnit]->nTmuFreq ) {
      case 660:    uFieldValue = 132; break;
      case 600:    uFieldValue = 120; break;
      case 495:    uFieldValue = 99;  break;
      case 400:    uFieldValue = 80;  break;
      default:     uFieldValue = 120;
    }
  } else {
    /* DDR 2133 operation requires TMU to be clocked at 660MHz -- (25 * 132(ndiv)) / 5(mdiv) */
    nDramFreq = soc_property_get(nUnit, spn_DDR3_CLOCK_MHZ, pBringUpControl->nTmuDramFreq );
    uFieldValue = ( nDramFreq == 1066 ) ? 132 : 120;
  }
  READ_CX_TMU_PLL_NDIV_INTEGERr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_TMU_PLL_NDIV_INTEGERr, &uRegisterValue, NDIV_INTf, uFieldValue );
  WRITE_CX_TMU_PLL_NDIV_INTEGERr( nUnit, uRegisterValue );

  /* SWS 641.67MHz operation  -- (25 * 154(ndiv)) / 6(mdiv) */
  switch ( g_c3hppcTestInfo[nUnit]->nSwsFreq ) {
    case 641:    uFieldValue = 154; break;
    case 541:    uFieldValue = 130; break;
    default:     uFieldValue = 144;
  }
  READ_CX_SWS_PLL_NDIV_INTEGERr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_SWS_PLL_NDIV_INTEGERr, &uRegisterValue, NDIV_INTf, uFieldValue );
  WRITE_CX_SWS_PLL_NDIV_INTEGERr( nUnit, uRegisterValue );


  /* LRP 1100MHz operation  -- (25 * 132(ndiv)) / 3(mdiv) */
  /* Note: RCE frequency is adjusted proportionally       */
  switch ( g_c3hppcTestInfo[nUnit]->nLrpFreq ) {
    case 1100:    uFieldValue = 132; break;
    case 900:     uFieldValue = 108; break;
    case 742:     uFieldValue = 89; break;
    case 660:     uFieldValue = 80; break;
    case 600:     uFieldValue = 72; break;
    case 500:     uFieldValue = 60; break;
    default:      uFieldValue = 120;
  }
  READ_CX_HPP_PLL_NDIV_INTEGERr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_HPP_PLL_NDIV_INTEGERr, &uRegisterValue, NDIV_INTf, uFieldValue );
  WRITE_CX_HPP_PLL_NDIV_INTEGERr( nUnit, uRegisterValue );

  if ( pBringUpControl->uSwsBringUp ) {
    cli_out("\nSWS Bring-Up .... \n");
    
    for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
      c3hppcSwsControlInfo.aIlControlInfo[nInstance].bBringUp = pBringUpControl->auIlBringUp[nInstance];
      c3hppcSwsControlInfo.aIlControlInfo[nInstance].bL1Loopback = 0;
      c3hppcSwsControlInfo.aIlControlInfo[nInstance].bExternalLoopback = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) ?  
                                                         g_c3hppcTestInfo[nUnit]->bLineExternalLoopBack :
                                                         g_c3hppcTestInfo[nUnit]->bFabricExternalLoopBack;
      c3hppcSwsControlInfo.aIlControlInfo[nInstance].bFCLoopback = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) ?  
                                                         ( g_c3hppcTestInfo[nUnit]->bLineExternalLoopBack ? 0 : 1) :
                                                         ( g_c3hppcTestInfo[nUnit]->bFabricExternalLoopBack ? 0 : 1);
      c3hppcSwsControlInfo.aIlControlInfo[nInstance].uOperationalMode = C3HPPC_SWS_IL_OP_MODE__100G_NARROW;
    }

    c3hppcSwsControlInfo.MacControlInfo.bExternalLoopback = g_c3hppcTestInfo[nUnit]->bLineExternalLoopBack;
    c3hppcSwsControlInfo.MacControlInfo.bBringUp =
      ( g_c3hppcTestInfo[nUnit]->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ||
        g_c3hppcTestInfo[nUnit]->nTDM == C3HPPC_SWS_TDM__12x10G_BY_100IL ||
        g_c3hppcTestInfo[nUnit]->nTDM == C3HPPC_SWS_TDM__3x40G_BY_100IL ||
        g_c3hppcTestInfo[nUnit]->nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) ? 1 : 0;
    c3hppcSwsControlInfo.MacControlInfo.bPauseDisable = g_c3hppcTestInfo[nUnit]->bPauseDisable;

    c3hppcSwsControlInfo.bLrpLearningEnable = pBringUpControl->bLrpLearningEnable;
    c3hppcSwsControlInfo.bOamEnable = pBringUpControl->bOamEnable;
    c3hppcSwsControlInfo.nTDM = g_c3hppcTestInfo[nUnit]->nTDM;
    c3hppcSwsControlInfo.bSwsOnlyTest = pBringUpControl->bSwsOnlyTest;
    c3hppcSwsControlInfo.bXLportAndCmicPortsActive = pBringUpControl->bXLportAndCmicPortsActive;
    c3hppcSwsControlInfo.nHotSwap = g_c3hppcTestInfo[nUnit]->nHotSwap;
    c3hppcSwsControlInfo.bLineTrafficGenOnly = g_c3hppcTestInfo[nUnit]->bLineTrafficGenOnly;
    c3hppcSwsControlInfo.bFabricTrafficGenOnly = g_c3hppcTestInfo[nUnit]->bFabricTrafficGenOnly;
    c3hppcSwsControlInfo.PpControlInfo.bBypassParsing = pBringUpControl->bSwsBypassPpParsing;
    c3hppcSwsControlInfo.PpControlInfo.bMultiStreamUcode = pBringUpControl->bMultiStreamUcode;
    c3hppcSwsControlInfo.PrControlInfo.bBypassParsing = pBringUpControl->bSwsBypassPrParsing;
    if ( c3hppc_sws_hw_init( nUnit, &c3hppcSwsControlInfo ) ) {
      cli_out("\n\nERROR: \"c3hppc_sws_hw_init\" FAILED!!!!! ...\n\n" );
      return 1;
    }

    if ( pBringUpControl->auIlBringUp[0] || pBringUpControl->auIlBringUp[1] ) {
      c3hppc_sws_il_stats_open( nUnit );
    }
  }


  if ( pBringUpControl->uTmuBringUp ) {
    cli_out("\nTMU Bring-Up .... \n");
    c3hppcTmuControlInfo.bCacheEnable = pBringUpControl->bTmuCacheEnable;
    c3hppcTmuControlInfo.bBypassScrambler = pBringUpControl->bTmuBypassScrambler;
    c3hppcTmuControlInfo.bBypassHash = pBringUpControl->bTmuBypassHash;
    c3hppcTmuControlInfo.bHwEmlChainManagement = pBringUpControl->bTmuHwEmlChainManagement;
    c3hppcTmuControlInfo.bSkipCiDramInit = pBringUpControl->bTmuSkipCiDramInit;
    c3hppcTmuControlInfo.bSkipCiDramSelfTest = pBringUpControl->bTmuSkipCiDramSelfTest;
    c3hppcTmuControlInfo.nDramFreq = pBringUpControl->nTmuDramFreq;
    c3hppcTmuControlInfo.bEMC128Mode = pBringUpControl->bTmuEMC128Mode;
    c3hppcTmuControlInfo.uEmlMaxProvisionedKey = pBringUpControl->uTmuEmlMaxProvisionedKey;
    c3hppcTmuControlInfo.uNumberOfCIs = pBringUpControl->uTmuNumberOfCIs;
    if ( c3hppc_tmu_hw_init( nUnit, &c3hppcTmuControlInfo ) ) {
      cli_out("\n\nERROR: \"c3hppc_tmu_hw_init\" FAILED!!!!! ...\n\n" );
      return 1;
    }
  }

  if ( pBringUpControl->uOcmBringUp ) {
    cli_out("\nOCM Bring-Up .... \n");
    c3hppcOcmControlInfo.pOcmPortInfo = pBringUpControl->aOcmPortInfo;
    c3hppc_ocm_hw_init( nUnit, &c3hppcOcmControlInfo );
  }

  if ( pBringUpControl->uLrpBringUp ) {
    cli_out("\nLRP Bring-Up .... \n");

    c3hppcLrpControlInfo.nBankSelect = 0;
    if ( pBringUpControl->u100Gduplex ) {
      c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
      c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
      c3hppcLrpControlInfo.bDuplex = 1;
    }
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = pBringUpControl->bLrpLoaderEnable;
    c3hppcLrpControlInfo.bMaximizeActiveContexts = pBringUpControl->bLrpMaximizeActiveContexts;
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, pBringUpControl->sUcodeFileName );

    if ( c3hppc_lrp_hw_init( nUnit, &c3hppcLrpControlInfo ) ) {
      cli_out("\n\nERROR: \"c3hppc_lrp_hw_init\" FAILED!!!!! ...\n\n" );
      return 1;
    }
  }

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    if ( pBringUpControl->uCopBringUp[nCopInstance] ) {
      cli_out("\nCOP%d Bring-Up .... \n", nCopInstance);
      c3hppcCopControlInfo.pCopSegmentInfo = pBringUpControl->aCopSegmentInfo[nCopInstance];
      c3hppcCopControlInfo.pCopProfileInfo = pBringUpControl->aCopProfileInfo[nCopInstance];
      c3hppcCopControlInfo.bWDT_ScoreBoardMode = pBringUpControl->bCopWDT_ScoreBoardMode;
      c3hppc_cop_hw_init( nUnit, nCopInstance, &c3hppcCopControlInfo );
    }
  }

  if ( pBringUpControl->uCmuBringUp ) {
    cli_out("\nCMU Bring-Up .... \n\n");

    /*
     *  Allocate PCI/Host memory for counters.  This will be a three step process.
     *  First calculate the total space needed by all segments, alloc the block and
     *  align to an 8 bytes boundary, then set each individual segment's "pSegmentPciBase".
     *  By allocating a single block simple bounds checks can be made in
     *  "c3hppc_cmu_counter_ring_manager".
    */
    nTotalCounterNum = 0;
    for ( nTotalCounterNum = 0, nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
      pCmuSegmentInfo = pBringUpControl->aCmuSegmentInfo + nSegment;
      if ( pCmuSegmentInfo->bValid ) {
        nCounters = pCmuSegmentInfo->uSegmentLimit + 1;
	if ( pCmuSegmentInfo->uSegmentType != C3HPPC_CMU_SEGMENT_TYPE__SIMPLE_64b ) {
          nCounters *= 2;
        } 
        nTotalCounterNum += nCounters;
      }
    }

    g_c3hppcCmuCounterRingManagerCB.uPciCounterBase = 
                             (uint32) sal_alloc( ((nTotalCounterNum+1) * sizeof(uint64)), "total_counter_space");
    g_c3hppcCmuCounterRingManagerCB.pPciCounterLoLimit = 
                             (uint64 *) ((g_c3hppcCmuCounterRingManagerCB.uPciCounterBase + 7) & 0xfffffff8);
    g_c3hppcCmuCounterRingManagerCB.pPciCounterHiLimit = g_c3hppcCmuCounterRingManagerCB.pPciCounterLoLimit +
                                                         nTotalCounterNum; 
    sal_memset( g_c3hppcCmuCounterRingManagerCB.pPciCounterLoLimit, 0x00, (nTotalCounterNum * sizeof(uint64)) );

    for ( nTotalCounterNum = 0, nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
      pCmuSegmentInfo = pBringUpControl->aCmuSegmentInfo + nSegment;
      if ( pCmuSegmentInfo->bValid ) {
        nCounters = pCmuSegmentInfo->uSegmentLimit + 1;
	if ( pCmuSegmentInfo->uSegmentType != C3HPPC_CMU_SEGMENT_TYPE__SIMPLE_64b ) {
          nCounters *= 2;
        } 
        pCmuSegmentInfo->pSegmentPciBase = g_c3hppcCmuCounterRingManagerCB.pPciCounterLoLimit + nTotalCounterNum;
        nTotalCounterNum += nCounters;
      }
    }

    c3hppcCmuControlInfo.pCmuSegmentInfo = pBringUpControl->aCmuSegmentInfo;
    c3hppcCmuControlInfo.uLFSRseed = pBringUpControl->uSTACEseed;
    c3hppc_cmu_hw_init( nUnit, &c3hppcCmuControlInfo );


    g_CmuCounterRingManagerThreadID = sal_thread_create( "tCmuCounterRingManager",
                                                         SAL_THREAD_STKSZ,
                                                         100,
                                                         c3hppc_cmu_counter_ring_manager,
                                                         (void *) &g_c3hppcCmuCounterRingManagerCB);
    if ( g_CmuCounterRingManagerThreadID == NULL || g_CmuCounterRingManagerThreadID == SAL_THREAD_ERROR ) {
      cli_out("\nERROR: Can not create CMU counter manager thread\n");
    }
                                                        
    
  }

  if ( pBringUpControl->uRceBringUp ) {
    cli_out("\nRCE Bring-Up .... \n");
    c3hppc_rce_hw_init( nUnit, &c3hppcRceControlInfo );
  } else {
    READ_CX_CONFIGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CX_CONFIGr, &uRegisterValue, RCE_CLK_DISABLEf, 1 );
    WRITE_CX_CONFIGr( nUnit, uRegisterValue );
  }

  if ( pBringUpControl->uEtuBringUp ) {
    cli_out("\nETU Bring-Up .... \n");
    if ( c3hppc_etu_hw_init( nUnit, &c3hppcEtuControlInfo ) ) {
      cli_out("\n\nERROR: \"c3hppc_etu_hw_init\" FAILED!!!!! ...\n\n" );
      return 1;
    }
  } else {
    READ_CX_CONFIGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CX_CONFIGr, &uRegisterValue, ETU_CLK_DISABLEf, 1 );
    WRITE_CX_CONFIGr( nUnit, uRegisterValue );
  }

  if ( pBringUpControl->uTmuBringUp && pBringUpControl->bTmuHwEmlChainManagement ) {
    nDramInitDone = ( pBringUpControl->bTmuSkipCiDramInit ) ?  1 : c3hppc_tmu_is_ci_memory_init_done( nUnit );
    if ( !nDramInitDone ) {
      cli_out("\n\nERROR:  DRAM initialization did NOT finish !!!!! ...\n\n" );
    }
  }


  return 0;
}


/*
 * Function:
 *      c3hppc_populate_with_defaults
 *
 * Purpose:
 *      Populates bring-up control structure with default settings.
 *
 * Parameters:
 *      pBringUpControl    - c3hppc_bringup_control_t control structure
 *
 * Returns:
 *      None 
 */
void
c3hppc_populate_with_defaults( int nUnit, c3hppc_bringup_control_t *pBringUpControl ) {

  int nCopInstance, nPort, nOcmPort;
  int nSegment, nInstance;
  uint32 uSegmentLogicalSize;
  c3hppc_ocm_port_info_t *pOcmPortInfo;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;

  pBringUpControl->uLrpBringUp = 1;
  pBringUpControl->bLrpLoaderEnable = 0;
  pBringUpControl->bLrpMaximizeActiveContexts = 1;
  pBringUpControl->uOcmBringUp = 1;
  pBringUpControl->uCmuBringUp = 1;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    pBringUpControl->uCopBringUp[nCopInstance] = 1;
  }
  pBringUpControl->uRceBringUp = 0;
  pBringUpControl->uTmuBringUp = 0;
  pBringUpControl->uEtuBringUp = 0;
  pBringUpControl->u100Gduplex = 1;
  pBringUpControl->bTmuCacheEnable = g_c3hppcTestInfo[nUnit]->bCacheEnable;
  pBringUpControl->bTmuBypassScrambler = g_c3hppcTestInfo[nUnit]->bBypassScrambler;
  pBringUpControl->bTmuBypassHash = g_c3hppcTestInfo[nUnit]->bBypassHash;
  pBringUpControl->uTmuNumberOfCIs = g_c3hppcTestInfo[nUnit]->nNumberOfCIs;
  pBringUpControl->nTmuDramFreq = g_c3hppcTestInfo[nUnit]->nDramFreq;
  pBringUpControl->bTmuSkipCiDramInit = g_c3hppcTestInfo[nUnit]->bSkipCiDramInit;
  pBringUpControl->bTmuSkipCiDramSelfTest = g_c3hppcTestInfo[nUnit]->bSkipCiDramSelfTest;
  pBringUpControl->bTmuHwEmlChainManagement = g_c3hppcTestInfo[nUnit]->bEMLInsertEnable;
  pBringUpControl->uSwsBringUp = 0;
  pBringUpControl->bMultiStreamUcode = 0;
  pBringUpControl->bCopWDT_ScoreBoardMode = 0;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pBringUpControl->auIlBringUp[nInstance] = 0;
  }


  for ( nPort = 0; nPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nPort ) {
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port( nPort );
    pOcmPortInfo = &(pBringUpControl->aOcmPortInfo[nOcmPort]);
    pOcmPortInfo->bValid = 1;
    pOcmPortInfo->nStartingSegment = 0;
    pOcmPortInfo->uSegmentTransferSize = C3HPPC_OCM_ALL_TRANSFER_SIZES;
    pOcmPortInfo->uSegmentBase = 0;
    uSegmentLogicalSize = ( nPort == 0 ) ? C3HPPC_OCM_MAX_PHY_BLK :
                                           pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    uSegmentLogicalSize *= C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
    pOcmPortInfo->uSegmentLimit = uSegmentLogicalSize - 1;
    /* LRP ports 6 through 9 are tied to memory blocks 64-127 */
    if ( nPort >= 6 ) {
      pOcmPortInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_64to127;
      g_OcmNextPhysicalBlock_64to127 += pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    } else {
      pOcmPortInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_0to63;
      g_OcmNextPhysicalBlock_0to63 += pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    } 
    pOcmPortInfo->bSegmentProtected = 1;
  }

  for ( nSegment = 0; nSegment < C3HPPC_CMU_OCM_PORT_NUM; ++nSegment ) {
    pCmuSegmentInfo = &(pBringUpControl->aCmuSegmentInfo[nSegment]);
    pCmuSegmentInfo->bValid = 1;
    pCmuSegmentInfo->uSegment = (uint32) nSegment;
    pCmuSegmentInfo->uSegmentOcmBase = 0;
    pCmuSegmentInfo->uSegmentType = ( SAL_BOOT_QUICKTURN ) ? C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b :
                                                             C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
    pCmuSegmentInfo->uSegmentLimit = 3*C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE - 1;
    pCmuSegmentInfo->uSegmentPort = nSegment;
    if ( nSegment ) {
      pCmuSegmentInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_64to127;
      g_OcmNextPhysicalBlock_64to127 += pBringUpControl->uDefaultCmuPhysicalSegmentSizeIn16kBlocks;
    } else {
      pCmuSegmentInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_0to63;
      g_OcmNextPhysicalBlock_0to63 += pBringUpControl->uDefaultCmuPhysicalSegmentSizeIn16kBlocks;
    }
  }

  for ( nCopInstance = 0, nSegment = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    pCopSegmentInfo = &(pBringUpControl->aCopSegmentInfo[nCopInstance][nSegment]);
    pCopSegmentInfo->bValid = 1;
    pCopSegmentInfo->uSegmentBase = 0;
    pCopSegmentInfo->uSegmentLimit = 
              (pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) - 1;
    pCopSegmentInfo->uSegmentOcmLimit = pCopSegmentInfo->uSegmentLimit;
    pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__COHERENT_TABLE;
    pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    if ( nCopInstance ) {
      pCopSegmentInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_64to127;
      g_OcmNextPhysicalBlock_64to127 += pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    } else {
      pCopSegmentInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_0to63;
      g_OcmNextPhysicalBlock_0to63 += pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    }
  }

  for ( nPort = 0; nPort < C3HPPC_NUM_OF_OCM_BUBBLE_PORTS; ++nPort ) {
    nOcmPort = c3hppc_ocm_map_bubble2ocm_port( nPort );
    pOcmPortInfo = &(pBringUpControl->aOcmPortInfo[nOcmPort]);
    pOcmPortInfo->bValid = 1;
    pOcmPortInfo->nStartingSegment = 0;
    pOcmPortInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    pOcmPortInfo->uSegmentBase = 0;
    uSegmentLogicalSize = pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    uSegmentLogicalSize *= C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
    pOcmPortInfo->uSegmentLimit = uSegmentLogicalSize - 1;
    pOcmPortInfo->nStartingPhysicalBlock = g_OcmNextPhysicalBlock_0to63;
    g_OcmNextPhysicalBlock_0to63 += pBringUpControl->uDefaultPhysicalSegmentSizeIn16kBlocks;
    pOcmPortInfo->bSegmentProtected = 1;
  }

  return;
}

int    c3hppc_test__get_ocm_next_physical_block_0to63( void ) {
  return g_OcmNextPhysicalBlock_0to63;
}

int    c3hppc_test__get_ocm_next_physical_block_64to127( void ) {
  return g_OcmNextPhysicalBlock_64to127;
}

int    c3hppc_test__get_error_state_summary( int nUnit ) {
  uint32 uRegisterValue0, uRegisterValue1;

  uRegisterValue0 = uRegisterValue1 = 0;

  if ( c3hppc_test__get_interrupt_summary( nUnit ) ) {
    READ_CMIC_CMC0_IRQ_STAT3r( nUnit, &uRegisterValue0 );
    READ_CMIC_CMC0_IRQ_STAT4r( nUnit, &uRegisterValue1 );
  } 

  if ( uRegisterValue0 || uRegisterValue1 ) {
    cli_out("<c3hppc_test__get_error_state_summary>  CMIC_CMC0_IRQ_STAT3[0x%08x]   CMIC_CMC0_IRQ_STAT4[0x%08x] \n",
            uRegisterValue0, uRegisterValue1 );
  }

  return ( (uRegisterValue0 || uRegisterValue1) ? 1 : 0 );
}


int    c3hppc_check_stats( int nUnit, int nSrcInterface, int nDstInterface, int nPacketSizeChange ) {

  int nChannel, nExpectInterface, nActualInterface, rc;
  uint64 uuExpect, uuActual, uuPktCount, uuTmp;

  rc = 0;
  nExpectInterface = ( nSrcInterface == C3HPPC_TEST__LINE_INTERFACE ) ? 0 : 1;
  nActualInterface = ( nDstInterface == C3HPPC_TEST__LINE_INTERFACE ) ? 0 : 1;

  for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) { 
    uuExpect = c3hppc_sws_il_stats_getrx( nExpectInterface, nChannel, ILRXSTAT__PKT );
    uuActual = c3hppc_sws_il_stats_gettx( nActualInterface, nChannel, ILTXSTAT__PKT );
#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
    if ( COMPILER_64_NE(uuExpect, uuActual) ) {
      rc = 1;
      cli_out("<c3hppc_check_stats> -- Channel[%d] \"PKT\" count MISCOMPARE {IL%d}Actual: %lld   {IL%d}Expect: %lld\n",
              nChannel, nActualInterface, uuActual, nExpectInterface, uuExpect );
    }
#endif
    uuPktCount = uuExpect;
    uuExpect = c3hppc_sws_il_stats_getrx( nExpectInterface, nChannel, ILRXSTAT__BYTE );
    uuActual = c3hppc_sws_il_stats_gettx( nActualInterface, nChannel, ILTXSTAT__BYTE );
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuPktCount), COMPILER_64_LO(uuPktCount));
    COMPILER_64_UMUL_32(uuTmp, nPacketSizeChange);
    COMPILER_64_ADD_64(uuExpect, uuTmp);
#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
    if ( COMPILER_64_NE(uuExpect, uuActual) ) {
      rc = 1;
      cli_out("<c3hppc_check_stats> -- Channel[%d] \"BYTE\" count MISCOMPARE {IL%d}Actual: %lld   {IL%d}Expect: %lld\n",
              nChannel, nActualInterface, uuActual, nExpectInterface, uuExpect );
    }
#endif
  }

  return rc;
}




void  c3hppc_test__setup_lpmipv4_table_contents( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                 int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                 int nBucketPopulatedPrefixNumber,
                                                 uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                 uint32 *pIPV4BucketTableContents, uint32 **apIPV4AssocDataTableContents ) {

  int nRPBpivot, nBBXpivot, nBucketPrefix, nBucketPairIndex, nPair, nIndex;
  c3hppc_tmu_taps_rpb_ccmd_t *pRPBcommand;
  c3hppc_tmu_taps_bbx_ccmd_t *pBBXcommand;
  c3hppc_tmu_taps_brr_ccmd_t *pBRRcommand;
  uint32 uKeyBits_43_24;
  uint32 uBucketTableEntrySizeIn32b;



  if ( nSetupOptions == C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH ) { 
    uBucketTableEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( (int) uBucketTable );
  } else {
    uBucketTableEntrySizeIn32b = 0;
  }

  pRPBcommand = (c3hppc_tmu_taps_rpb_ccmd_t *) pRPBcommands;
  pBBXcommand = (c3hppc_tmu_taps_bbx_ccmd_t *) pBBXcommands;
  pBRRcommand = (c3hppc_tmu_taps_brr_ccmd_t *) pBRRcommands;

  for ( nRPBpivot = 0; nRPBpivot < nRPBpivotNumber; ++nRPBpivot, ++pRPBcommand ) {

    pRPBcommand->Word0.bits.TapsI = nTapsInstance + 1;
    pRPBcommand->Word0.bits.Block = 1;
    pRPBcommand->Word0.bits.Opcode = 2;
    pRPBcommand->Word0.bits.Target = 7;
    pRPBcommand->Word0.bits.Segment = nSegment;
    pRPBcommand->Word0.bits.Offset = nRPBpivot;
    pRPBcommand->Word1.bits.Bucket = nRPBpivot;
    pRPBcommand->Word3.bits.Plength = 16;
    pRPBcommand->Word3.bits.Pdata143_128 = nRPBpivot;

    for ( nBBXpivot = 0; nBBXpivot < nBBXpopulatedPivotNumber; ++nBBXpivot, ++pBBXcommand, ++pBRRcommand ) { 

      pBBXcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBBXcommand->Word0.bits.Block = 2;
      pBBXcommand->Word0.bits.Opcode = 2;
      pBBXcommand->Word0.bits.Format = 3;
      pBBXcommand->Word0.bits.Segment = nSegment;
      pBBXcommand->Word0.bits.Offset = nRPBpivot;
      pBBXcommand->Word1.bits.Pnumber = nBBXpivot;
      pBBXcommand->Word3.bits.Plength = 21;
      pBBXcommand->Word3.bits.Pdata143_128 = nRPBpivot;
      pBBXcommand->Word4.bits.Pdata127_96 = nBBXpivot << 27;

      pBRRcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBRRcommand->Word0.bits.Block = 3;
      pBRRcommand->Word0.bits.Opcode = 2;
      pBRRcommand->Word0.bits.Format = 3;
      pBRRcommand->Word0.bits.Segment = nSegment;
      pBRRcommand->Word0.bits.Offset = nRPBpivot;
      pBRRcommand->Word1.bits.Pnumber = nBBXpivot;
      if ( nSetupOptions == C3HPPC_TMU_TAPS_MODE__FULLY_ON_CHIP || nSetupOptions == C3HPPC_TMU_TAPS_MODE__SEARCH_ON_CHIP ) { 

        pBRRcommand->Word3.bits.Adata = (nRPBpivot << 8) | (nBBXpivot << 3);

        if ( nSetupOptions == C3HPPC_TMU_TAPS_MODE__SEARCH_ON_CHIP ) {

          nIndex = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b * ((nRPBpivot * nBBXmaxPivotNumber) + nBBXpivot);  
          uKeyBits_43_24 = pBRRcommand->Word3.bits.Adata; 

          if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
            apIPV4AssocDataTableContents[0][nIndex+0] = 0x11000000 | (0 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[0][nIndex+1] = 0x22000000 | (0 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[0][nIndex+2] = 0x33000000 | (0 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[0][nIndex+3] = 0x00400000 | uKeyBits_43_24;
          }
          if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
            apIPV4AssocDataTableContents[1][nIndex+0] = 0x11000000 | (1 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[1][nIndex+1] = 0x22000000 | (1 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[1][nIndex+2] = 0x33000000 | (1 << 20) | uKeyBits_43_24;
            apIPV4AssocDataTableContents[1][nIndex+3] = 0x00400000 | uKeyBits_43_24;
          }

        }

      } else if ( nSetupOptions == C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH ) {
        /*
           The local best match pointer will point to the key/associated data linked to the first unpopulated psig
           in the dram bucket of the first pair.  The following index is local to the BBX bucket.   
           The BBX bucket is the row of pivots in SRAM.  This local best match pointer is made global by
           using the "prefixes_per_bucket" value contained in TP_SEGMENT_CONFIG1_S in conjunction with the Root Pivot.  
        */ 

        nPair = 0;
        pBRRcommand->Word3.bits.Adata = (2 * nBucketPrefixNumber * nBBXpivot) +  (nPair * nBucketPrefixNumber) + 
                                                                                     nBucketPopulatedPrefixNumber;

        nBucketPairIndex = 2 * ((nRPBpivot * nBBXmaxPivotNumber) + nBBXpivot);  
        for ( nPair = 0; nPair < 2; ++nPair ) { 

          for ( nBucketPrefix = 0; nBucketPrefix < nBucketPrefixNumber; ++nBucketPrefix ) {

            nIndex = ((int)uBucketTableEntrySizeIn32b * (nBucketPairIndex + nPair)) + nBucketPrefix;
            /* Adjust "index" to account for the reserved slots contained within the 
               IPv4 256b Bucket data structure.
            */
            nIndex += (nBucketPrefix / C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM);
            if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) { 
              pIPV4BucketTableContents[nIndex] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
            } else {
              pIPV4BucketTableContents[nIndex] = 0;
            }

          
            nIndex = ((C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b * nBucketPrefixNumber) * (nBucketPairIndex + nPair)) +
                                                                       (C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b * nBucketPrefix);
            if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) {
              uKeyBits_43_24 = (nRPBpivot << 8) | (nBBXpivot << 3) | (nPair << 2) | nBucketPrefix;

              if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
                apIPV4AssocDataTableContents[0][nIndex+0] = 0x11000000 | (0 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[0][nIndex+1] = 0x22000000 | (0 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[0][nIndex+2] = 0x33000000 | (0 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[0][nIndex+3] = 0x00400000 | uKeyBits_43_24;
              }
              if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
                apIPV4AssocDataTableContents[1][nIndex+0] = 0x11000000 | (1 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[1][nIndex+1] = 0x22000000 | (1 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[1][nIndex+2] = 0x33000000 | (1 << 20) | uKeyBits_43_24;
                apIPV4AssocDataTableContents[1][nIndex+3] = 0x00400000 | uKeyBits_43_24;
              }
  
            } else {
  
              if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
                apIPV4AssocDataTableContents[0][nIndex+0] = 0xdeadbeef;
                apIPV4AssocDataTableContents[0][nIndex+1] = 0xdeadbeef;
                apIPV4AssocDataTableContents[0][nIndex+2] = 0xdeadbeef;
                apIPV4AssocDataTableContents[0][nIndex+3] = 0x004dbeef;
              }
              if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
                apIPV4AssocDataTableContents[1][nIndex+0] = 0xdeadbeef;
                apIPV4AssocDataTableContents[1][nIndex+1] = 0xdeadbeef;
                apIPV4AssocDataTableContents[1][nIndex+2] = 0xdeadbeef;
                apIPV4AssocDataTableContents[1][nIndex+3] = 0x004dbeef;
              }

            }

          }

        }

      }

    }

  }

  return;
}




void  c3hppc_test__setup_unified_lpmipv4_table_contents( int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                         int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                         int nBucketPopulatedPrefixNumber,
                                                         uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                         uint32 *pIPV4BucketTableContents, uint32 *pIPV4AssocDataTableContents ) {

  int nRPBpivot, nBBXpivot, nBucketPrefix, nBucketPairIndex, nPair, nIndex;
  c3hppc_tmu_taps_rpb_ccmd_t *pRPBcommand;
  c3hppc_tmu_taps_bbx_ccmd_t *pBBXcommand;
  c3hppc_tmu_taps_brr_ccmd_t *pBRRcommand;
  uint32 uKeyBits_43_24;
  uint32 uBucketTableEntrySizeIn32b;



  uBucketTableEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( (int) uBucketTable );

  pRPBcommand = (c3hppc_tmu_taps_rpb_ccmd_t *) pRPBcommands;
  pBBXcommand = (c3hppc_tmu_taps_bbx_ccmd_t *) pBBXcommands;
  pBRRcommand = (c3hppc_tmu_taps_brr_ccmd_t *) pBRRcommands;

  for ( nRPBpivot = 0; nRPBpivot < nRPBpivotNumber; ++nRPBpivot, ++pRPBcommand ) {

    pRPBcommand->Word0.bits.TapsI = (nRPBpivot / (nRPBpivotNumber/2)) + 1;
    pRPBcommand->Word0.bits.Block = 1;
    pRPBcommand->Word0.bits.Opcode = 2;
    pRPBcommand->Word0.bits.Target = 7;
    pRPBcommand->Word0.bits.Segment = nSegment;
    pRPBcommand->Word0.bits.Offset = nRPBpivot & ((nRPBpivotNumber/2)-1);
    pRPBcommand->Word1.bits.Bucket = pRPBcommand->Word0.bits.Offset;
    pRPBcommand->Word3.bits.Plength = 16;
    pRPBcommand->Word3.bits.Pdata143_128 = nRPBpivot;

    for ( nBBXpivot = 0; nBBXpivot < nBBXpopulatedPivotNumber; ++nBBXpivot, ++pBBXcommand, ++pBRRcommand ) { 

      pBBXcommand->Word0.bits.TapsI = pRPBcommand->Word0.bits.TapsI;
      pBBXcommand->Word0.bits.Block = 2;
      pBBXcommand->Word0.bits.Opcode = 2;
      pBBXcommand->Word0.bits.Format = 3;
      pBBXcommand->Word0.bits.Segment = nSegment;
      pBBXcommand->Word0.bits.Offset = pRPBcommand->Word1.bits.Bucket;
      pBBXcommand->Word1.bits.Pnumber = nBBXpivot;
      pBBXcommand->Word3.bits.Plength = 21;
      pBBXcommand->Word3.bits.Pdata143_128 = nRPBpivot;
      pBBXcommand->Word4.bits.Pdata127_96 = nBBXpivot << 27;

      pBRRcommand->Word0.bits.TapsI = pRPBcommand->Word0.bits.TapsI;
      pBRRcommand->Word0.bits.Block = 3;
      pBRRcommand->Word0.bits.Opcode = 2;
      pBRRcommand->Word0.bits.Format = 3;
      pBRRcommand->Word0.bits.Segment = nSegment;
      pBRRcommand->Word0.bits.Offset = pRPBcommand->Word1.bits.Bucket;
      pBRRcommand->Word1.bits.Pnumber = nBBXpivot;
      /*
         The local best match pointer will point to the key/associated data linked to the first unpopulated psig
         in the dram bucket of the first pair.  The following index is local to the BBX bucket.   
         The BBX bucket is the row of pivots in SRAM.  This local best match pointer is made global by
         using the "prefixes_per_bucket" value contained in TP_SEGMENT_CONFIG1_S in conjunction with the Root Pivot.  
      */ 
      nPair = 0;
      pBRRcommand->Word3.bits.Adata = (2 * nBucketPrefixNumber * nBBXpivot) +  (nPair * nBucketPrefixNumber) + 
                                                                                     nBucketPopulatedPrefixNumber;

      nBucketPairIndex = 2 * ((nRPBpivot * nBBXmaxPivotNumber) + nBBXpivot);  
      for ( nPair = 0; nPair < 2; ++nPair ) { 

        for ( nBucketPrefix = 0; nBucketPrefix < nBucketPrefixNumber; ++nBucketPrefix ) {

          nIndex = ((int)uBucketTableEntrySizeIn32b * (nBucketPairIndex + nPair)) + nBucketPrefix;
          /* Adjust "index" to account for the reserved slots contained within the 
             IPv4 256b Bucket data structure.
          */
          nIndex += (nBucketPrefix / C3HPPC_TMU_IPV4_256b_BUCKET_PREFIX_NUM);
          if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) { 
            pIPV4BucketTableContents[nIndex] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
          } else {
            pIPV4BucketTableContents[nIndex] = 0;
          }

          
          nIndex = ((C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b * nBucketPrefixNumber) * (nBucketPairIndex + nPair)) +
                                                                       (C3HPPC_TMU_ASSOC_DATA_SIZE_IN_32b * nBucketPrefix);
          if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) {
            uKeyBits_43_24 = (nRPBpivot << 8) | (nBBXpivot << 3) | (nPair << 2) | nBucketPrefix;

            pIPV4AssocDataTableContents[nIndex+0] = 0x11000000 | (0 << 20) | uKeyBits_43_24;
            pIPV4AssocDataTableContents[nIndex+1] = 0x22000000 | (0 << 20) | uKeyBits_43_24;
            pIPV4AssocDataTableContents[nIndex+2] = 0x33000000 | (0 << 20) | uKeyBits_43_24;
            pIPV4AssocDataTableContents[nIndex+3] = 0x00400000 | uKeyBits_43_24;

          } else {

            pIPV4AssocDataTableContents[nIndex+0] = 0xdeadbeef;
            pIPV4AssocDataTableContents[nIndex+1] = 0xdeadbeef;
            pIPV4AssocDataTableContents[nIndex+2] = 0xdeadbeef;
            pIPV4AssocDataTableContents[nIndex+3] = 0x004dbeef;

          }

        }

      }

    }

  }

  return;
}





void  c3hppc_test__setup_lpmipv6_table_contents( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                 int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                 int nBucketPopulatedPrefixNumber,
                                                 uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                 uint32 *pIPV6BucketTableContents, uint32 **apIPV6KeyAndAssocDataTableContents ) {

  int nRPBpivot, nBBXpivot, nBucketPrefix, nBucketPairIndex, nPair, nIndex, nSubIndex;
  c3hppc_tmu_taps_rpb_ccmd_t *pRPBcommand;
  c3hppc_tmu_taps_bbx_ccmd_t *pBBXcommand;
  c3hppc_tmu_taps_brr_ccmd_t *pBRRcommand;
  uint32 uKeyBits_139_120;
  c3hppc_tmu_psig_ut PSig;
  uint32 auPrefixData[4];
  uint32 uBucketTableEntrySizeIn32b;
  uint64 uuCrc40;
 

  uBucketTableEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( (int) uBucketTable );

  pRPBcommand = (c3hppc_tmu_taps_rpb_ccmd_t *) pRPBcommands;
  pBBXcommand = (c3hppc_tmu_taps_bbx_ccmd_t *) pBBXcommands;
  pBRRcommand = (c3hppc_tmu_taps_brr_ccmd_t *) pBRRcommands;

  for ( nRPBpivot = 0; nRPBpivot < nRPBpivotNumber; ++nRPBpivot, ++pRPBcommand ) {

    pRPBcommand->Word0.bits.TapsI = nTapsInstance + 1;
    pRPBcommand->Word0.bits.Block = 1;
    pRPBcommand->Word0.bits.Opcode = 2;
    pRPBcommand->Word0.bits.Target = 7;
    pRPBcommand->Word0.bits.Segment = nSegment;
    pRPBcommand->Word0.bits.Offset = nRPBpivot;
    pRPBcommand->Word1.bits.Bucket = nRPBpivot;
    pRPBcommand->Word1.bits.Kshift = 16;
    pRPBcommand->Word3.bits.Plength = 16;
    pRPBcommand->Word3.bits.Pdata143_128 = nRPBpivot;

    for ( nBBXpivot = 0; nBBXpivot < nBBXpopulatedPivotNumber; ++nBBXpivot, ++pBBXcommand, ++pBRRcommand ) { 

      pBBXcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBBXcommand->Word0.bits.Block = 2;
      pBBXcommand->Word0.bits.Opcode = 2;
      pBBXcommand->Word0.bits.Format = 8;
      pBBXcommand->Word0.bits.Segment = nSegment;
      pBBXcommand->Word0.bits.Offset = nRPBpivot;
      pBBXcommand->Word1.bits.Pnumber = nBBXpivot;
      pBBXcommand->Word3.bits.Plength = 5;
      pBBXcommand->Word3.bits.Pdata143_128 = nBBXpivot << 11;

      pBRRcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBRRcommand->Word0.bits.Block = 3;
      pBRRcommand->Word0.bits.Opcode = 2;
      pBRRcommand->Word0.bits.Format = 8;
      pBRRcommand->Word0.bits.Segment = nSegment;
      pBRRcommand->Word0.bits.Offset = nRPBpivot;
      pBRRcommand->Word1.bits.Pnumber = nBBXpivot;
      /*
         The local best match pointer will point to the key/associated data linked to the first unpopulated psig
         in the dram bucket of the first pair.  The following index is local to the BBX bucket.   
         The BBX bucket is the row of pivots in SRAM.  This local best match pointer is made global by
         using the "prefixes_per_bucket" value contained in TP_SEGMENT_CONFIG1_S in conjunction with the Root Pivot.  
      */ 
      nPair = 0;
      pBRRcommand->Word3.bits.Adata = (2 * nBucketPrefixNumber * nBBXpivot) +  (nPair * nBucketPrefixNumber) + 
                                                                                     nBucketPopulatedPrefixNumber;

      nBucketPairIndex = 2 * ((nRPBpivot * nBBXmaxPivotNumber) + nBBXpivot);  
      for ( nPair = 0; nPair < 2; ++nPair ) { 

        for ( nBucketPrefix = 0; nBucketPrefix < nBucketPrefixNumber; ++nBucketPrefix ) { 

          nIndex = ((int)uBucketTableEntrySizeIn32b * (nBucketPairIndex + nPair));

          COMPILER_64_ZERO(PSig.value); 
          sal_memset( auPrefixData, 0x00, sizeof(auPrefixData) );
          if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) { 
            auPrefixData[3] = ((nBBXpivot & 0xf) << 27) | (nPair << 26) | (nBucketPrefix << 24);
            uuCrc40 = c3hppc_tmu_psig_hash_calc( auPrefixData, 0 );
            PSig.bits.HashedPrefix_bits39to24  = (COMPILER_64_LO(uuCrc40) >> 24);
            PSig.bits.HashedPrefix_bits39to24 |= ((COMPILER_64_HI(uuCrc40) & 0x000000FF) << 8);
            PSig.bits.HashedPrefix_bits23to0 = (COMPILER_64_LO(uuCrc40) & 0x00ffffff);
            PSig.bits.Length = 127;
            PSig.bits.Valid = 1;
            if ( nBucketPrefix == 0 ) {
              pIPV6BucketTableContents[nIndex+0] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+1] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff);
            } else if ( nBucketPrefix == 1 ) {
              pIPV6BucketTableContents[nIndex+1] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff) << 16;
              pIPV6BucketTableContents[nIndex+2] |= (COMPILER_64_LO( PSig.value ) >> 16);
              pIPV6BucketTableContents[nIndex+2] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff) << 16;
            } else if ( nBucketPrefix == 2 ) {
              pIPV6BucketTableContents[nIndex+3] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+4] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff);
            } else if ( nBucketPrefix == 3 ) {
              pIPV6BucketTableContents[nIndex+4] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff) << 16;
              pIPV6BucketTableContents[nIndex+5] |= (COMPILER_64_LO( PSig.value ) >> 16);
              pIPV6BucketTableContents[nIndex+5] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff) << 16;
            } else {
              pIPV6BucketTableContents[nIndex+6] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+7] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff);
            }
          }

          nIndex = ((C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b * nBucketPrefixNumber) * (nBucketPairIndex + nPair)) +
                                                           (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b * nBucketPrefix);
          if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) { 
            uKeyBits_139_120 = (nRPBpivot << 8) | (nBBXpivot << 3) | (nPair << 2) | nBucketPrefix; 

            if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              apIPV6KeyAndAssocDataTableContents[0][nIndex+0] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+1] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+2] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+3] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+4] = 0x11000000 | (0 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+5] = 0x22000000 | (0 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+6] = 0x33000000 | (0 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[0][nIndex+7] = 0x00400000 | uKeyBits_139_120;
            }
            if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              apIPV6KeyAndAssocDataTableContents[1][nIndex+0] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+1] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+2] = 0x00000000;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+3] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+4] = 0x11000000 | (1 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+5] = 0x22000000 | (1 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+6] = 0x33000000 | (1 << 20) | uKeyBits_139_120;
              apIPV6KeyAndAssocDataTableContents[1][nIndex+7] = 0x00400000 | uKeyBits_139_120;
            }

          } else {

            if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              for ( nSubIndex = 0; nSubIndex < C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b; ++nSubIndex ) {
                apIPV6KeyAndAssocDataTableContents[0][nIndex+nSubIndex] = 
                    ( nSubIndex < (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b - 1) ) ? 0xdeadbeef : 0x004dbeef;
              }
            }
            if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              for ( nSubIndex = 0; nSubIndex < C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b; ++nSubIndex ) {
                apIPV6KeyAndAssocDataTableContents[1][nIndex+nSubIndex] = 
                    ( nSubIndex < (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b - 1) ) ? 0xdeadbeef : 0x004dbeef;
              }
            }

          }

        }

      }

    }

  }

  return;
}





void  c3hppc_test__setup_lpmipv6_table_contents_with_collisions( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                                 int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                                 int nBucketPopulatedPrefixNumber,
                                                                 uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                                 uint32 *pIPV6BucketTableContents, uint32 **apIPV6KeyAndAssocDataTableContents,
                                                                 int nCollisionControl ) {

  int nRPBpivot, nBBXpivot, nBucketPrefix, nBucketPairIndex, nPair, nIndex, nSubIndex, nBucketPrefix0Index;
  c3hppc_tmu_taps_rpb_ccmd_t *pRPBcommand;
  c3hppc_tmu_taps_bbx_ccmd_t *pBBXcommand;
  c3hppc_tmu_taps_brr_ccmd_t *pBRRcommand;
  uint32 uKeyBits_139_120;
  c3hppc_tmu_psig_ut PSig;
  uint32 auPrefixData[4];
  uint32 uBucketTableEntrySizeIn32b;
  uint64 uuCrc40;
 

  uBucketTableEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( (int) uBucketTable );

  pRPBcommand = (c3hppc_tmu_taps_rpb_ccmd_t *) pRPBcommands;
  pBBXcommand = (c3hppc_tmu_taps_bbx_ccmd_t *) pBBXcommands;
  pBRRcommand = (c3hppc_tmu_taps_brr_ccmd_t *) pBRRcommands;

  for ( nRPBpivot = 0; nRPBpivot < nRPBpivotNumber; ++nRPBpivot, ++pRPBcommand ) {

    pRPBcommand->Word0.bits.TapsI = nTapsInstance + 1;
    pRPBcommand->Word0.bits.Block = 1;
    pRPBcommand->Word0.bits.Opcode = 2;
    pRPBcommand->Word0.bits.Target = 7;
    pRPBcommand->Word0.bits.Segment = nSegment;
    pRPBcommand->Word0.bits.Offset = nRPBpivot;
    pRPBcommand->Word1.bits.Bucket = nRPBpivot;
    pRPBcommand->Word1.bits.Kshift = 16;
    pRPBcommand->Word3.bits.Plength = 16;
    pRPBcommand->Word3.bits.Pdata143_128 = nRPBpivot;

    for ( nBBXpivot = 0; nBBXpivot < nBBXpopulatedPivotNumber; ++nBBXpivot, ++pBBXcommand, ++pBRRcommand ) { 

      pBBXcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBBXcommand->Word0.bits.Block = 2;
      pBBXcommand->Word0.bits.Opcode = 2;
      pBBXcommand->Word0.bits.Format = 8;
      pBBXcommand->Word0.bits.Segment = nSegment;
      pBBXcommand->Word0.bits.Offset = nRPBpivot;
      pBBXcommand->Word1.bits.Pnumber = nBBXpivot;
      pBBXcommand->Word3.bits.Plength = 5;
      pBBXcommand->Word3.bits.Pdata143_128 = nBBXpivot << 11;

      pBRRcommand->Word0.bits.TapsI = nTapsInstance + 1;
      pBRRcommand->Word0.bits.Block = 3;
      pBRRcommand->Word0.bits.Opcode = 2;
      pBRRcommand->Word0.bits.Format = 8;
      pBRRcommand->Word0.bits.Segment = nSegment;
      pBRRcommand->Word0.bits.Offset = nRPBpivot;
      pBRRcommand->Word1.bits.Pnumber = nBBXpivot;
      /*
         The local best match pointer will point to the key/associated data linked to the first unpopulated psig
         in the dram bucket of the first pair.  The following index is local to the BBX bucket.   
         The BBX bucket is the row of pivots in SRAM.  This local best match pointer is made global by
         using the "prefixes_per_bucket" value contained in TP_SEGMENT_CONFIG1_S in conjunction with the Root Pivot.  
      */ 
      nPair = 0;
      pBRRcommand->Word3.bits.Adata = (2 * nBucketPrefixNumber * nBBXpivot) +  (nPair * nBucketPrefixNumber) + 
                                                                                     nBucketPopulatedPrefixNumber;

      nBucketPairIndex = 2 * ((nRPBpivot * nBBXmaxPivotNumber) + nBBXpivot);  
      for ( nPair = 0; nPair < 2; ++nPair ) { 

        for ( nBucketPrefix = 0; nBucketPrefix < nBucketPrefixNumber; ++nBucketPrefix ) { 

          nIndex = ((int)uBucketTableEntrySizeIn32b * (nBucketPairIndex + nPair));

          COMPILER_64_ZERO(PSig.value); 
          sal_memset( auPrefixData, 0x00, sizeof(auPrefixData) );
          if ( nBucketPrefix < nBucketPopulatedPrefixNumber ) { 
            auPrefixData[3] = ((nBBXpivot & 0xf) << 27) | (nPair << 26) | (nBucketPrefix << 24);
            uuCrc40 = c3hppc_tmu_psig_hash_calc( auPrefixData, 0 );
            PSig.bits.HashedPrefix_bits39to24  = (COMPILER_64_LO(uuCrc40) >> 24);
            PSig.bits.HashedPrefix_bits39to24 |= ((COMPILER_64_HI(uuCrc40) & 0x000000FF) << 8);
            PSig.bits.HashedPrefix_bits23to0 = (COMPILER_64_LO(uuCrc40) & 0x00ffffff);
            PSig.bits.Length = 127;
            PSig.bits.Valid = 1;
            if ( nBucketPrefix == 0 ) {
              pIPV6BucketTableContents[nIndex+0] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+1] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff);
            } else if ( nBucketPrefix == 1 ) {
              pIPV6BucketTableContents[nIndex+1] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff) << 16;
              pIPV6BucketTableContents[nIndex+2] |= (COMPILER_64_LO( PSig.value ) >> 16);
              pIPV6BucketTableContents[nIndex+2] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff) << 16;
            } else if ( nBucketPrefix == 2 ) {
              pIPV6BucketTableContents[nIndex+3] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+4] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff);
            } else if ( nBucketPrefix == 3 ) {
              pIPV6BucketTableContents[nIndex+4] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff) << 16;
              pIPV6BucketTableContents[nIndex+5] |= (COMPILER_64_LO( PSig.value ) >> 16);
              pIPV6BucketTableContents[nIndex+5] |= (COMPILER_64_HI( PSig.value ) & 0x0000ffff) << 16;
            } else {
              pIPV6BucketTableContents[nIndex+6] |= (COMPILER_64_LO( PSig.value ));
              pIPV6BucketTableContents[nIndex+7] |= (COMPILER_64_LO( PSig.value ) & 0x0000ffff);
            }
          }

          nBucketPrefix0Index = ((C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b * nBucketPrefixNumber) * (nBucketPairIndex + nPair));
          nIndex = nBucketPrefix0Index + (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b * nBucketPrefix);

          if ( nBucketPrefix < (nBucketPopulatedPrefixNumber+1) ) { 
            uKeyBits_139_120 = (nRPBpivot << 8) | (nBBXpivot << 3) | (nPair << 2) | nBucketPrefix; 

            if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              if ( nBucketPrefix == nBucketPopulatedPrefixNumber && nPair == 0 ) {
                /* 3rd probe entry for collision */
                apIPV6KeyAndAssocDataTableContents[0][nIndex+0] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+1] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+1];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+2] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+2];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+3] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+3];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+4] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+4];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+5] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+5];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+6] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+6];
                apIPV6KeyAndAssocDataTableContents[0][nIndex+7] = apIPV6KeyAndAssocDataTableContents[0][nBucketPrefix0Index+7];
              } else if ( nBucketPrefix != nBucketPopulatedPrefixNumber ) {
                /* The "0x00000001" value causes a collision. */
                apIPV6KeyAndAssocDataTableContents[0][nIndex+0] = ( nPair == 0 && nBucketPrefix == 0 ) ? 0x00000001 : 0x00000000;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+1] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+2] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+3] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+4] = 0x11000000 | (0 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+5] = 0x22000000 | (0 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+6] = 0x33000000 | (0 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[0][nIndex+7] = 0x00400000 | uKeyBits_139_120;
              }
            }
            if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              if ( nBucketPrefix == nBucketPopulatedPrefixNumber && nPair == 0 ) {
                /* 3rd probe entry for collision */
                apIPV6KeyAndAssocDataTableContents[1][nIndex+0] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+1] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+1];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+2] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+2];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+3] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+3];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+4] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+4];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+5] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+5];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+6] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+6];
                apIPV6KeyAndAssocDataTableContents[1][nIndex+7] = apIPV6KeyAndAssocDataTableContents[1][nBucketPrefix0Index+7];
              } else if ( nBucketPrefix != nBucketPopulatedPrefixNumber ) {
                /* The "0x00000001" value causes a collision. */
                apIPV6KeyAndAssocDataTableContents[1][nIndex+0] = ( nPair == 0 && nBucketPrefix == 0 ) ? 0x00000001 : 0x00000000;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+1] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+2] = 0x00000000;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+3] = (nBBXpivot << 28) | (nPair << 27) | (nBucketPrefix << 25) | 0x01000000;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+4] = 0x11000000 | (1 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+5] = 0x22000000 | (1 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+6] = 0x33000000 | (1 << 20) | uKeyBits_139_120;
                apIPV6KeyAndAssocDataTableContents[1][nIndex+7] = 0x00400000 | uKeyBits_139_120;
              }
            }

          } else {

            if ( nTapsInstance == 0 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              for ( nSubIndex = 0; nSubIndex < C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b; ++nSubIndex ) {
                apIPV6KeyAndAssocDataTableContents[0][nIndex+nSubIndex] = 
                    ( nSubIndex < (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b - 1) ) ? 0xdeadbeef : 0x004dbeef;
              }
            }
            if ( nTapsInstance == 1 || nTapsInstance == C3HPPC_TMU_TAPS_INSTANCE_NUM ) {
              for ( nSubIndex = 0; nSubIndex < C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b; ++nSubIndex ) {
                apIPV6KeyAndAssocDataTableContents[1][nIndex+nSubIndex] = 
                    ( nSubIndex < (C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_32b - 1) ) ? 0xdeadbeef : 0x004dbeef;
              }
            }

          }

        }

      }

    }

  }

  return;
}



void  c3hppc_test__setup_eml64_key_list_entry( uint32 nInsertIndex, uint32 nEntryKey, uint32 *pList ) {

  pList[nInsertIndex+0] = nEntryKey;
  pList[nInsertIndex+1] = 0x00000000;
  pList[nInsertIndex+2] = 0x11100000 | nEntryKey;
  pList[nInsertIndex+3] = 0x22200000 | nEntryKey;
  pList[nInsertIndex+4] = 0x33300000 | nEntryKey;
  pList[nInsertIndex+5] = 0x00700000 | nEntryKey;

  return;
}

void  c3hppc_test__setup_eml_table_contents( int nSetupOptions, int nMaxKey, int nRootTable,
                                             uint32 *pRootTableContents, uint32 *pEML_InsertList ) {

  int n, nEntry, nIndex, nNumEntries;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nRootTableEntrySizeIn32b, nInsertListEntrySizeIn32b;

  nNumEntries = 1 << c3hppc_tmu_get_table_size( nRootTable );
  nRootTableEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( nRootTable );
  nInsertListEntrySizeIn32b = 2 * C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b;

  COMPILER_64_ZERO(EMLcontrolWord.value); 
  EMLcontrolWord.bits.NL_GT = 15;
  EMLcontrolWord.bits.NL_LE_bits3to1 = 7;
  EMLcontrolWord.bits.NL_LE_bit0 = 1;

  for ( nEntry = 0; nEntry < nMaxKey; ++nEntry ) {
    c3hppc_test__setup_eml64_key_list_entry( (nInsertListEntrySizeIn32b * nEntry), nEntry, pEML_InsertList );
  }

  for ( nEntry = 0; nEntry < nNumEntries; ++nEntry ) {
    nIndex = nRootTableEntrySizeIn32b * nEntry;

    pRootTableContents[nIndex+0] = COMPILER_64_LO( EMLcontrolWord.value );
    pRootTableContents[nIndex+1] = COMPILER_64_HI( EMLcontrolWord.value );
    for ( n = 2; n <= (nRootTableEntrySizeIn32b-1); ++n ) {
      pRootTableContents[nIndex+n] = 0;
    }
  }

  return;
}



void  c3hppc_test__setup_emc64_table_entry( uint32 u128Mode, uint32 uEntryKey, uint32 uIndex, uint32 *pTable ) {

  pTable[uIndex+0] = uEntryKey;
  pTable[uIndex+1] = 0x00000000;
  pTable[uIndex+2] = 0x888c0000 | uEntryKey;
  if ( u128Mode ) {
    pTable[uIndex+3] = 0x007c0000 | uEntryKey;
  } else {
    pTable[uIndex+3] = 0x999c0000 | uEntryKey;
    pTable[uIndex+4] = 0xaaac0000 | uEntryKey;
    pTable[uIndex+5] = 0x007c0000 | uEntryKey;
  }

  return;
}


void  c3hppc_test__setup_emc_table_contents( int nSetupOptions, uint32 *pRootTableContents, uint32 *pNextTableContents,
                                             int nRootTable, int nNextTable ) {

  uint32 uEntry, uEntryKey, uEntrySizeIn32b, uRootIndex, uNextIndex, u128Mode;
  int nRootTableSizePowerOf2, nRootTableHashAdjustSelect, nNextTableSizePowerOf2, nCuckooFlag, nNumEntries;
  c3hppc_tmu_key_t auuKey;


  nNumEntries = 1 << c3hppc_tmu_get_table_size( nRootTable );
  uEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( nRootTable );
  u128Mode = c3hppc_tmu_get_emc128mode();

  sal_memset( pRootTableContents, 0xff, (nNumEntries * uEntrySizeIn32b * sizeof(uint32)) ); 
  sal_memset( pNextTableContents, 0xff, (nNumEntries * uEntrySizeIn32b * sizeof(uint32)) );

  nRootTableSizePowerOf2 = c3hppc_tmu_get_table_size( nRootTable );
  nRootTableHashAdjustSelect = c3hppc_tmu_get_hash_adjust_select( nRootTable );
  nNextTableSizePowerOf2 = c3hppc_tmu_get_table_size( nNextTable );
  sal_memset( auuKey, 0x00, sizeof(c3hppc_tmu_key_t) );


  for ( uEntry = 0; uEntry < nNumEntries; ++uEntry ) {

    uEntryKey = uEntry; 
    COMPILER_64_SET(auuKey[0], 0, uEntryKey);
    nCuckooFlag = 0;

    while ( 1 ) {
      uRootIndex = c3hppc_tmu_1stLookup_hash( auuKey, nRootTableHashAdjustSelect, nRootTableSizePowerOf2 ) * uEntrySizeIn32b; 
      uNextIndex = c3hppc_tmu_2ndEmcLookup_hash( auuKey, nNextTableSizePowerOf2 ) * uEntrySizeIn32b;
      if ( pRootTableContents[uRootIndex+1] ) {

        c3hppc_test__setup_emc64_table_entry( u128Mode, uEntryKey, uRootIndex, pRootTableContents );
        break;

      } else if ( pNextTableContents[uNextIndex+1] ) {

        c3hppc_test__setup_emc64_table_entry( u128Mode, uEntryKey, uNextIndex, pNextTableContents );
        break;

      } else {

/*
        cli_out("CUCKOO%d   CUCKOO%d   CUCKOO%d   CUCKOO%d   CUCKOO%d  \n", 
                nCuckooFlag, nCuckooFlag, nCuckooFlag, nCuckooFlag, nCuckooFlag );
*/
        if ( (nCuckooFlag & 1) == 0 ) {
          COMPILER_64_SET(auuKey[0], 0, pRootTableContents[uRootIndex+0]);
          c3hppc_test__setup_emc64_table_entry( u128Mode, uEntryKey, uRootIndex, pRootTableContents );
        } else {
          COMPILER_64_SET(auuKey[0], 0, pNextTableContents[uNextIndex+0]);
          c3hppc_test__setup_emc64_table_entry( u128Mode, uEntryKey, uNextIndex, pNextTableContents );
        }
        uEntryKey = COMPILER_64_LO(auuKey[0]);
        ++nCuckooFlag;

      } 
    } 

  }

  return;
}

uint64  c3hppc_test__display_pm_stats( int nUnit, int nPmInstance, int nStartIndex, uint32 uPmBucketShift, char *pStatName ) {

  int nIndex;
  uint64 uuPmCount, uuTotalPmCount;

  c3hppc_tmu_pm_dump_memory( nUnit, nPmInstance );
  COMPILER_64_ZERO(uuTotalPmCount);
  for ( nIndex = 0; nIndex < C3HPPC_TMU_PM_MEMORY_ROW_NUM; ++nIndex ) {
    uuPmCount = c3hppc_tmu_pm_get_count( nIndex );
    COMPILER_64_ADD_64(uuTotalPmCount,uuPmCount);
#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
    if ( !COMPILER_64_IS_ZERO(uuPmCount) && nIndex >= nStartIndex ) {
      cli_out("INFO:  The %s tag %d clock bucket saw %lld hits\n", pStatName, (nIndex << uPmBucketShift), uuPmCount );
    }
#endif
  }

  return uuTotalPmCount;
}


int  c3hppc_test__wait_for_updaters_to_be_idle( int nUnit, int nTimeOut ) {
  sal_time_t TimeStamp;
  uint32 uRegisterValue;
  uint8 bFirstTime;

  bFirstTime = 1;
  nTimeOut *= 1000000;
  while ( (g_c3hppcTestInfo[nUnit]->BringUpControl.uTmuBringUp && (!c3hppc_tmu_are_cmd_fifos_empty() || !c3hppc_tmu_are_rsp_fifos_empty())) ||
          (g_c3hppcTestInfo[nUnit]->BringUpControl.uEtuBringUp && !c3hppc_etu_is_cmd_fifo_empty()) ) {

    if ( g_c3hppcTestInfo[nUnit]->BringUpControl.uEtuBringUp && !c3hppc_etu_is_cmd_fifo_empty() ) {
      READ_ETU_GLOBAL_INTR_STSr( nUnit, &uRegisterValue );
      soc_reg_field_set(nUnit, ETU_GLOBAL_INTR_STSr, &uRegisterValue, CP_FIFO_CAPTURE_DONEf, 0);
      soc_reg_field_set(nUnit, ETU_GLOBAL_INTR_STSr, &uRegisterValue, TX_RAW_REQ_DONEf, 0);
      if ( uRegisterValue ) {
        cli_out("ERROR:   Detected ETU/NL11K interface issues during table update!  ETU_GLOBAL_INTR_STS[0x%08x]\n", uRegisterValue );
        READ_ILAMAC_RX_INTF_STATE0r( nUnit, &uRegisterValue );
        if ( uRegisterValue != 0x1cffffff ) {
          cli_out("\nERROR-4: ETU/NL11K interface went DOWN!!!\n");
          cli_out("  XON_TX_CH1=%d XON_TX_CH0=%d RX_WORD_SYNC=0x%03x RX_SYNCED=0x%03x RX_MISALIGNED=%d RX_ALIGNED_ERR=%d RX_ALIGNED=%d\n",
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, XON_TX_CH1f ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, XON_TX_CH0f ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_WORD_SYNCf ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_SYNCEDf ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_MISALIGNEDf ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_ALIGNED_ERRf ),
                  soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_ALIGNEDf ) );

          c3hppc_etu_display_nl11k_error_state( nUnit );
          break;
        }
        return 0;
      }
    }

    if ( !(nTimeOut % 1000000) ) {
      if ( bFirstTime ) {
        bFirstTime = 0;
        cli_out("\n\nINFO:  Waiting for table update to finish ...");
      } else {
        cli_out(".");
      }
    }

    sal_usleep(1);

    if ( !(--nTimeOut) ) break;
  }

  cli_out("\n\n");

  if ( g_c3hppcTestInfo[nUnit]->BringUpControl.uEtuBringUp && nTimeOut ) {
    if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, ETU_CP_FIFO_STSr, EMPTYf, 1, 500, 0, &TimeStamp ) ) {
      cli_out("\n\n<c3hppc_test__wait_for_updaters_to_be_idle>   TIMEOUT detected waiting for ETU_CP_FIFO_STS !  \n\n");
    }
  }

  if ( nTimeOut == 0 ) {
    cli_out("\n ERROR:   Updater TIMEOUT failure:  c3hppc_tmu_are_cmd_fifos_empty %d  c3hppc_tmu_are_rsp_fifos_empty %d  c3hppc_etu_is_cmd_fifo_empty %d\n",
            c3hppc_tmu_are_cmd_fifos_empty(), c3hppc_tmu_are_rsp_fifos_empty(), c3hppc_etu_is_cmd_fifo_empty() );
    cli_out("\n ERROR:   Updater TIMEOUT failure:    cmd_fifo0_count %d    cmd_fifo1_count %d    rsp_fifo0_count %d   rsp_fifo0_count %d\n",
            c3hppc_tmu_cmd_fifo_count(0), c3hppc_tmu_cmd_fifo_count(1), c3hppc_tmu_rsp_fifo_count(0), c3hppc_tmu_rsp_fifo_count(1) );

    c3hppc_tmu__dump_cmic_rd_dma_state( nUnit );
  }


  return nTimeOut;
}


uint64 c3hppc_test__get_current_epoch_count( int nUnit ) {
  uint64 uuEpochCounter;
  c3hppc_64b_ocm_entry_template_t OcmEntry;

  c3hppc_cop_coherent_table_read_write( nUnit, 1, 0, 0, 0, OcmEntry.uData );
  COMPILER_64_SET(uuEpochCounter, OcmEntry.uData[1], OcmEntry.uData[0]);

  return uuEpochCounter;
}



void c3hppc_test__setup_dm_table_contents( uint32 uDmLookUp, int nTableNumber, int nTableEntryNum, 
                                           uint32 uFirst4Bytes, uint32 *pBuffer ) { 

  int nEntry, nIndex;

  for ( nEntry = 0; nEntry < nTableEntryNum; ++nEntry ) {
    if ( uDmLookUp == C3HPPC_TMU_LOOKUP__DM119 ) {
      nIndex = 4 * nEntry;
      pBuffer[nIndex+0] = uFirst4Bytes | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+1] = 0x22200000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+2] = 0x33300000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+3] = 0x00400000 | (nTableNumber << 16) | nEntry;
    } else if ( uDmLookUp == C3HPPC_TMU_LOOKUP__DM247 ) {
      nIndex = 8 * nEntry;
      pBuffer[nIndex+0] = uFirst4Bytes | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+1] = 0x22200000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+2] = 0x33300000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+3] = 0x44400000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+4] = 0x55500000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+5] = 0x66600000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+6] = 0x77700000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+7] = 0x00700000 | (nTableNumber << 16) | nEntry;
    } else if ( uDmLookUp == C3HPPC_TMU_LOOKUP__DM366 ) {
      nIndex = 12 * nEntry;
      pBuffer[nIndex+0] = uFirst4Bytes | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+1] = 0x22200000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+2] = 0x33300000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+3] = 0x44400000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+4] = 0x55500000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+5] = 0x66600000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+6] = 0x77700000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+7] = 0x00700000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+8] = 0x99900000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+9] = 0xaaa00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+10] = 0xbbb00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+11] = 0x00700000 | (nTableNumber << 16) | nEntry;
    } else {
      nIndex = 16 * nEntry;
      pBuffer[nIndex+0] = uFirst4Bytes | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+1] = 0x22200000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+2] = 0x33300000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+3] = 0x44400000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+4] = 0x55500000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+5] = 0x66600000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+6] = 0x77700000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+7] = 0x00700000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+8] = 0x99900000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+9] = 0xaaa00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+10] = 0xbbb00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+11] = 0xccc00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+12] = 0xddd00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+13] = 0xeee00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+14] = 0xfff00000 | (nTableNumber << 16) | nEntry;
      pBuffer[nIndex+15] = 0x00700000 | (nTableNumber << 16) | nEntry;
    }
  }

  return;
}


/*
 * Function:
 *      c3hppc_cmu_counter_ring_manager
 *
 * Purpose:
 *      Manages host based counter ring  
 *
 * Parameters:
 *      pCounterRingManagerCB_arg    - c3hppc_counter_ring_manager_cb_t control structure
 *
 * Returns:
 *      None 
 */
void
c3hppc_cmu_counter_ring_manager (void *pCounterRingManagerCB_arg) {

  int nUnit, copyno;
  uint8 at;
  uint32 uRegisterValue;
  VOL uint32 *pCounterRing, *pCounterRingLimit, *pCounterRing_AddressEntry, *pCounterRing_CountEntry;
  uint32 uProcessedEntryNum, uTotalProcessedEntryNum;
  uint32 uDataBeats, uEntryNum, uCounterRingSize;
  uint64 *pCounter;
  schan_msg_t msg;
  c3hppc_counter_ring_manager_cb_t *pCounterRingManagerCB;
  sal_time_t  now, PreviousTime;

 
  cli_out("  Entering CMU counter ring thread .... \n\n");

  PreviousTime = sal_time();

  pCounterRingManagerCB = (c3hppc_counter_ring_manager_cb_t *) pCounterRingManagerCB_arg;
  nUnit = pCounterRingManagerCB->nUnit;

#if (defined(LINUX))
  if ( g_nShowCurrent ) {
    cli_out("\n");
    bb_caladan3_current( nUnit, 0 );
  }
  if ( g_nShowTemp ) {
    cli_out("\n");
    soc_sbx_caladan3_temperature_monitor( nUnit, 1, 0, 1 );
  }
#endif

  /*
   * Setup CMIC_CMC0_FIFO_CH0_RD_DMA for counter ring processing
  */

  uDataBeats = soc_mem_entry_words( nUnit, CM_EJECTION_FIFOm );

  uCounterRingSize = 4096;
/* Coverity
  switch (uCounterRingSize) {
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

  copyno = SOC_MEM_BLOCK_ANY(nUnit, CM_EJECTION_FIFOm);
  uRegisterValue = soc_mem_addr_get( nUnit, CM_EJECTION_FIFOm, 0, copyno, 0, &at);
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );

  schan_msg_clear(&msg);
  msg.readcmd.header.v3.opcode = FIFO_POP_CMD_MSG;
  msg.readcmd.header.v3.dst_blk = SOC_BLOCK2SCH( nUnit, copyno );
  msg.readcmd.header.v3.data_byte_len= uDataBeats * sizeof(uint32);
  /* Set 1st schan ctrl word as opcode */
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );

  pCounterRing = (uint32 *) soc_cm_salloc( nUnit, (uCounterRingSize * uDataBeats * sizeof(uint32)), "counter_ring");
  pCounterRingLimit = pCounterRing + (uCounterRingSize * uDataBeats);
  sal_memset( (void *)pCounterRing, 0, (uCounterRingSize * uDataBeats * sizeof(uint32)) );
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, (uint32) (soc_cm_l2p(nUnit, (void *)pCounterRing)) );

  READ_CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, BEAT_COUNTf, uDataBeats );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, HOST_NUM_ENTRIES_SELf, uEntryNum );
  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ENDIANESSf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );

  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uCounterRingSize-256) );

  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ENABLEf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );

  pCounterRing_AddressEntry = pCounterRing;
  uProcessedEntryNum = 0;
  uTotalProcessedEntryNum = 0;



  while ( !pCounterRingManagerCB->bExit ) {

    /*  IL stats capture daemon.  Collect stats every 10 seconds.  */
    if ( pCounterRingManagerCB->pBringUpControl->auIlBringUp[0] ||
         pCounterRingManagerCB->pBringUpControl->auIlBringUp[1] ) {
      now = sal_time();
      if ( (now - PreviousTime) >= 15 ) {

#if (defined(LINUX))
        if ( g_nShowCurrent ) {
          cli_out("\n");
          bb_caladan3_current( nUnit, 0 );
        }
        if ( g_nShowTemp ) {
          cli_out("\n");
          soc_sbx_caladan3_temperature_monitor( nUnit, 1, 0, 1 );
        }
#endif
        /* Added for B0 */
        if ( g_rev_id != BCM88030_A0_REV_ID ) {
          c3hppc_sws_il_stats_collect( nUnit );
        }

        PreviousTime = now;
      }
    }


    if ( *pCounterRing_AddressEntry ) {

      pCounterRing_CountEntry = pCounterRing_AddressEntry + 1;

      assert( ((uint32) *pCounterRing_AddressEntry) >= ((uint32) pCounterRingManagerCB->pPciCounterLoLimit) &&
              ((uint32) *pCounterRing_AddressEntry) <  ((uint32) pCounterRingManagerCB->pPciCounterHiLimit) ); 
  
/*
if ( *pCounterRing_CountEntry && ((uint32) *pCounterRing_AddressEntry) < ((uint32) pCounterRingManagerCB->pPciCounterLoLimit + 1024) ) {
  cli_out("   pCounterRing_AddressEntry 0x%08x -- pCounterRing_CountEntry  0x%08x\n", *pCounterRing_AddressEntry, *pCounterRing_CountEntry);
}
*/
  
      ++uProcessedEntryNum;
      pCounter = (uint64 *) (*pCounterRing_AddressEntry & 0xfffffff8);
      COMPILER_64_ADD_32(*pCounter, *pCounterRing_CountEntry);

      *pCounterRing_AddressEntry = 0;
#if (defined(LINUX))
      soc_cm_sinval( nUnit, (void *)pCounterRing_AddressEntry, WORDS2BYTES(uDataBeats) );
#endif
      pCounterRing_AddressEntry += uDataBeats;
      if ( pCounterRing_AddressEntry == pCounterRingLimit ) {
        pCounterRing_AddressEntry = pCounterRing;
/*
        cli_out("  Counter ring wrapped!\n");
*/
      }

    } else {
      if ( uProcessedEntryNum ) {
        WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr( nUnit, uProcessedEntryNum ); 
        uTotalProcessedEntryNum += uProcessedEntryNum;
        if ( (uTotalProcessedEntryNum % 100) < 4 ) {
/*
          cli_out("  Processed %d out of %d total counter elements.\n", uProcessedEntryNum, uTotalProcessedEntryNum );
*/
        }
        uProcessedEntryNum = 0;
      }
#if (defined(LINUX))
      sal_usleep(1);
#else
      sal_sleep(0);
#endif
    }
  }



  soc_cm_sfree( nUnit, (void *)pCounterRing );

  soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &uRegisterValue, ABORTf, 1 );
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr( nUnit, uRegisterValue );
  sal_usleep(1);
  WRITE_CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr( nUnit, 0 );

  cli_out("  Exiting CMU counter ring thread .... \n\n");

  sal_thread_exit(0); 

  return;
}


#endif /* #ifdef BCM_CALADAN3_SUPPORT */

