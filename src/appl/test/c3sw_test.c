/*
 * $Id: c3sw_test.c,v 1.26 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        c3sw_test.c
 * Purpose:     Run a Caladan3 Software driver Drivers tests
 *
 *
 * Notes:       
 *             
 *
 */


#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "c3sw_test.h"

static c3sw_test_list_entry_t 
       g_c3swTestList[] = {
                             {"ocm_ut1", c3_ut_ocm_test1_init, 
                              c3_ut_ocm_test1_run, c3_ut_ocm_test1_done},
                             {"ocm_ut2", c3_ut_ocm_test1_init, 
                              c3_ut_ocm_dma_test_run, c3_ut_ocm_test1_done},
                             {"tmu", c3_ut_tmu_test_init, 
                              c3_ut_tmu_test_run, c3_ut_tmu_test_done},
                             {"cmu_ut1", c3_ut_cmu_test1_init, 
                              c3_ut_cmu_test1_run, c3_ut_cmu_test1_done},
                             {"cmu_ut2", c3_ut_cmu_test2_init, 
                              c3_ut_cmu_test2_run, c3_ut_cmu_test2_done},
                             {"cop_ut1", c3_ut_cop_test1_init, 
                              c3_ut_cop_test1_run, c3_ut_cop_test1_done},
                             {"tmu_hash_ut", c3_ut_tmu_hash_test_init, 
                              c3_ut_tmu_hash_test_run, c3_ut_tmu_hash_test_done},
                             {"tmu_trie_ut", c3_ut_tmu_taps_trie_test_init, 
                              c3_ut_tmu_taps_trie_test_run, c3_ut_tmu_taps_trie_test_done},
                             {"tmu_taps_ut",  c3_ut_tmu_taps_test_init, 
                              c3_ut_tmu_taps_test_run, c3_ut_tmu_taps_test_done},
#ifdef BCM_FE2000_SUPPORT
                             {"sbx_pkt_ut",  c3_ut_pkt_test_init,
                              c3_ut_pkt_test_run, c3_ut_pkt_test_done},
#endif
                            };

static int g_nc3swTestListCnt = COUNTOF(g_c3swTestList);
static c3sw_test_list_entry_t *g_pc3swCurrentTest = NULL;

static c3sw_test_info_t  *g_c3swTestInfo[SOC_MAX_NUM_DEVICES];
/*
 * Function:
 *      c3sw_print_param
 *
 * Purpose:
 *      Print parameter structure
 *
 * Parameters:
 *      pc3swTestInfo             - parameter structure
 *
 * Returns:
 *      Allocated structure or NULL if alloc failed.
 *
 */

STATIC void
c3sw_print_param(c3sw_test_info_t *pc3swTestInfo)
{
  if (pc3swTestInfo != NULL) {
    if ( pc3swTestInfo->sTestName != NULL ) { 
      cli_out("\n\n\n**************************************************************\n");
      cli_out("  Running TEST=%s with SEED=%d TestID=%d TestCase=%d\n", 
              pc3swTestInfo->sTestName, pc3swTestInfo->nTestSeed,
              pc3swTestInfo->testid, pc3swTestInfo->testCase);
      cli_out("**************************************************************\n\n\n");
    }
  } else {
    cli_out("pc3swTestInfo has no test parameters!!!)\n");
  }
}


/*
 * Function:
 *      c3sw_alloc
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

STATIC c3sw_test_info_t *
c3sw_alloc(int unit)
{
  c3sw_test_info_t *pc3swTestInfo =
      (c3sw_test_info_t *)sal_alloc(sizeof(c3sw_test_info_t),
                                    "c3sw test info");

  if (pc3swTestInfo != NULL) {
    pc3swTestInfo->unit        = unit;
    pc3swTestInfo->sTestName    = NULL;
    pc3swTestInfo->nTestStatus  = TEST_OK;
  }

  return pc3swTestInfo;
}

/*
 * Function:
 *      c3sw_free
 *
 * Purpose:
 *      Free test parameter structure
 *
 * Parameters:
 *      pc3swTestInfo             - parameter structure pointer
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

STATIC int c3sw_free(c3sw_test_info_t *pc3swTestInfo)
{
  int rv = TEST_FAIL;

  if ( pc3swTestInfo == NULL ) {
    return rv;
  }
  if ( pc3swTestInfo->sTestName != NULL ) sal_free(pc3swTestInfo->sTestName);

  sal_free(pc3swTestInfo);

  return TEST_OK;
}

/*
 * Function:
 *      c3sw_test_init
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
c3sw_test_init(int unit, args_t *args, void **pa)
{
  parse_table_t pt;
  c3sw_test_info_t *pc3swTestInfo = c3sw_alloc(unit);


  if (pc3swTestInfo == NULL) {
    cli_out("%s: out of memory\n", ARG_CMD(args));
    return(TEST_FAIL);
  }

  parse_table_init(unit, &pt);

  /*  Pull out parameters for tests*/
  parse_table_add(&pt, TEST_TOKEN, (PQ_STRING|PQ_DFL|PQ_STATIC), 
          TEST_DEF, &(pc3swTestInfo->sTestName), NULL);
  parse_table_add(&pt, SEED_TOKEN, (PQ_INT), (void *)SEED_DEF,
                  &(pc3swTestInfo->nTestSeed), NULL);
  parse_table_add(&pt, ID_TOKEN, (PQ_INT), (void*)ID_DEF, 
          &(pc3swTestInfo->testid), NULL);
  parse_table_add(&pt, CASE_TOKEN, (PQ_INT), (void*)CASE_DEF, &(pc3swTestInfo->testCase), NULL);
  parse_table_add(&pt, SUB_CASE_TOKEN, (PQ_INT), (void*)SUB_CASE_DEF, &(pc3swTestInfo->testSubCase), NULL);
  parse_table_add(&pt, KEY_LEN_TOKEN, (PQ_INT), (void*)KEY_L_DEF, &(pc3swTestInfo->testKeyLength), NULL);
  parse_table_add(&pt, ROUTES_TOKEN, (PQ_INT), (void*)ROUTES_DEF, &(pc3swTestInfo->testNumRoutes), NULL);
  parse_table_add(&pt, TAPS_INST_TOKEN, (PQ_INT), (void*)TAPS_INST_DEF, &(pc3swTestInfo->testTapsInst), NULL);
  parse_table_add(&pt, NUMDPFX_TOKEN, (PQ_INT), (void*)NUM_DPFX_DEF, &(pc3swTestInfo->testNumDramPrf), NULL);
  parse_table_add(&pt, TAPS_VFLAG_TOKEN, (PQ_INT), (void*)TAPS_VFLAG_DEF, &(pc3swTestInfo->testVerbFlag), NULL);
  parse_table_add(&pt, TAPS_L3FLUSH_TOKEN, (PQ_INT), (void*)TAPS_L3FLUSH_DEF, &(pc3swTestInfo->testFlushCount), NULL);
  parse_table_add(&pt, RUNNING_TIME, (PQ_INT), (void*)RUNNING_TIME_DEF, &(pc3swTestInfo->testRunningTime), NULL);
  parse_table_add(&pt, SEARCH_MODE_ID, (PQ_INT), (void*)SEARCH_MODE_ID_DEF, &(pc3swTestInfo->testSearchModeId), NULL);
  parse_table_add(&pt, TAPS_SHARE, (PQ_INT), (void*)TAPS_SHARE_ID_DEF, &(pc3swTestInfo->testTapsShare), NULL);

  /* Parse arguments */
  if (0 > parse_arg_eq(args, &pt)) {
    test_error(unit,
               "%s: Invalid option: %s\n",
               ARG_CMD(args),
               ARG_CUR(args) ? ARG_CUR(args) : "*");
    parse_arg_eq_done(&pt);
    c3sw_free(pc3swTestInfo);
    return(TEST_FAIL);
  }

  if ( pc3swTestInfo->nTestSeed == -1 ) {
    sal_srand( (int)sal_time() );
    pc3swTestInfo->nTestSeed = sal_rand();
  }
  sal_srand(pc3swTestInfo->nTestSeed);

  parse_arg_eq_done(&pt);

  c3sw_print_param( pc3swTestInfo );

  g_c3swTestInfo[unit] = pc3swTestInfo;
  if ( (g_pc3swCurrentTest = c3sw_find_test(pc3swTestInfo->sTestName)) == NULL ) {
    test_error(unit,
               "Test %s NOT found!\n", pc3swTestInfo->sTestName);
    return(TEST_FAIL);
  }

  /*
   * Invoke test specific init routine
   */
  if (g_pc3swCurrentTest->pfuncTestInit(g_c3swTestInfo[unit], (void*)args) < 0) {
      return TEST_FAIL;
  } else {
      return TEST_OK;
  }
}

/*
 * Function:
 *      c3sw_test_run
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
c3sw_test_run(int unit, args_t *a, void *pa)
{
  return g_pc3swCurrentTest->pfuncTestRun(g_c3swTestInfo[unit], (void *)a);
}


/*
 * Function:
 *      c3sw_test_done
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
c3sw_test_done(int unit, void *pa)
{
    int status;
  g_c3swTestInfo[unit]->nTestStatus = g_pc3swCurrentTest->pfuncTestDone(g_c3swTestInfo[unit], NULL);

  if ( g_c3swTestInfo[unit]->nTestStatus == TEST_OK ) {
    cli_out("\n\n\n**************************************************************\n");
    cli_out("c3sw test [%s] PASSED!\n", g_c3swTestInfo[unit]->sTestName);
    cli_out("**************************************************************\n\n\n");
  } else {
    cli_out("\n\n\n##############################################################\n");
    test_error(unit, "c3sw test [%s] using seed[%d] ID[%d] FAILED!\n",
               g_c3swTestInfo[unit]->sTestName, g_c3swTestInfo[unit]->nTestSeed,
               g_c3swTestInfo[unit]->testid);
    cli_out("##############################################################\n\n\n");
  }

  status = g_c3swTestInfo[unit]->nTestStatus;
  c3sw_free(g_c3swTestInfo[unit]);
  return status;
}


/*
 * Function:
 *      c3sw_find_test
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

c3sw_test_list_entry_t *
c3sw_find_test(char *sKey)
{
  int nKeyLen, i;
  c3sw_test_list_entry_t *pc3swTestListEntry = NULL;

  nKeyLen = sal_strlen(sKey);
  for (i = 0; (pc3swTestListEntry == NULL && i < g_nc3swTestListCnt); i++) {
    if (!sal_strncasecmp(sKey, g_c3swTestList[i].sTestName, nKeyLen)) {
      pc3swTestListEntry = &g_c3swTestList[i];
    }
  }
  return pc3swTestListEntry;
}


/*
 * Function:
 *      c3sw_is_test_done
 *
 * Purpose:
 *      Check to see if LRP is offline
 *
 * Parameters:
 *      unit           - device unit
 *
 * Returns:
 *      1 --> means test complete, 0 --> not complete
 */

int
c3sw_is_test_done(int unit)
{
    return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */

