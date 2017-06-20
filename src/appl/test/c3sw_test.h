/*
 * $Id: c3sw_test.h,v 1.19 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        c3sw_test.h
 * Purpose:     Extern declarations for test functions.
 */


#if !defined(_TEST_C3SW_TEST_H)
#define _TEST_C3SW_TEST_H

#include <sal/core/time.h>
#include <sal/core/alloc.h>
#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/io.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <sal/appl/sal.h>
#include <sal/core/time.h>
#include <appl/diag/test.h> 
#include <sal/core/thread.h>

#define TEST_FAIL -1
#define TEST_OK    0

#define TEST_TOKEN                  "test"
#define SEED_TOKEN                  "seed"
#define ID_TOKEN                     "id"
#define CASE_TOKEN                 "case"
#define SUB_CASE_TOKEN          "subcase"
#define KEY_LEN_TOKEN         "keylength"
#define ROUTES_TOKEN             "routes"
#define TAPS_INST_TOKEN              "ti"
#define NUMDPFX_TOKEN               "ndp"
#define TAPS_VFLAG_TOKEN             "vf"
#define TAPS_L3FLUSH_TOKEN        "cache"
#define RUNNING_TIME               "time"
#define SEARCH_MODE_ID             "smode"
#define TAPS_SHARE                 "share"


#define TEST_DEF                      ""
#define SEED_DEF                      -1
#define ID_DEF                        -1
#define CASE_DEF                       0
#define SUB_CASE_DEF                  255
#define KEY_L_DEF                     32
#define ROUTES_DEF              (1024*64)
#define VFLAG_DEF                      0
#define NUM_DPFX_DEF                   7
#define TAPS_INST_DEF                  0
#define TAPS_VFLAG_DEF                 0
#define TAPS_L3FLUSH_DEF               0
#define RUNNING_TIME_DEF               0
#define SEARCH_MODE_ID_DEF             2
#define TAPS_SHARE_ID_DEF              0


/*
 * Test information structure -- contains test status/control and 
 * parsed command line arguments.
 */
typedef struct c3sw_test_info_s {
    int       unit;          /* Which unit to test on */
    /* Add test parameters here */
    char     *sTestName;      /* Test Name */
    int      nTestSeed;       /* Test Seed */
    int      nTestStatus;
    int      testid;
    int      testCase;        /* Test case */
    int      testSubCase;        /* Test sub case */
    int      testKeyLength;   /* Test key length */
    int      testNumRoutes;   /* Test number of routes */
    int      testTapsInst;   /* Test taps instance */
    int      testNumDramPrf;   /* Test number dram prefixes */
    int      testVerbFlag;     /* Test verbosity flag */
    int      testFlushCount;     /* Flush count for L3 caching */
    int      testRunningTime;     /* Test seconds*/
    int      testSearchModeId;     /* Test search mode*/
    int      testTapsShare;   /* Is taps share for multiple units*/
} c3sw_test_info_t;

typedef int     (*c3sw_test_func_t)(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

typedef struct c3sw_test_list_entry_s {
  char                *sTestName;                 /* Test Name */
  c3sw_test_func_t   pfuncTestInit;             /* Initialization routine */
  c3sw_test_func_t   pfuncTestRun;              /* Run routine */
  c3sw_test_func_t   pfuncTestDone;             /* Completion routine */
} c3sw_test_list_entry_t;

c3sw_test_list_entry_t *c3sw_find_test(char *sKey);
int  c3sw_is_test_done(int unit);

typedef int (*init_call_back_f)(int unit, void *testdata);
typedef int (*run_call_back_f)(int unit, void *testdata);
typedef int (*clean_call_back_f)(int unit, void *testdata);

typedef struct test_call_back_s {
    int                id;
    init_call_back_f   init;
    run_call_back_f    run;
    clean_call_back_f  clean;
} test_call_back_t;





/****************************************************************
 *                                                              *
 *                      Test Functions                          *
 *                                                              *
 * Each test function may have an optional corresponding        *
 * "init" function and "done" function.                         *
 *                                                              *
 ****************************************************************/
extern  int     c3_ut_ocm_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_ocm_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_ocm_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_ocm_dma_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_tmu_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_cmu_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cmu_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cmu_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_cmu_test2_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cmu_test2_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cmu_test2_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_cop_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cop_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_cop_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_tmu_hash_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_hash_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_hash_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_tmu_taps_trie_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_taps_trie_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_taps_trie_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_tmu_taps_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_taps_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_tmu_taps_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

extern  int     c3_ut_pkt_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_pkt_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData);
extern  int     c3_ut_pkt_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData);

#endif  /* _TEST_C3SW_TEST_H */

