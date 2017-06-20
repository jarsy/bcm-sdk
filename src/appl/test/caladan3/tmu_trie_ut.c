/* $Id: tmu_trie_ut.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/tmu/taps/trie.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <shared/alloc.h>
#include <sal/core/time.h>

extern int tmu_taps_trie_ut(int id, unsigned int seed);
extern int tmu_taps_kshift_ut(void);
extern int tmu_trie_split_ut(unsigned int seed);
extern int tmu_taps_bpm_trie_ut(int id, unsigned int seed);
extern int tmu_taps_util_get_bpm_pfx_ut(void);

/*********************/
/*     Test Runner   */
/*********************/
int c3_ut_tmu_taps_trie_test_init(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;
    return rv;
}

int
c3_ut_tmu_taps_trie_test_run(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE, index=0;

    if (testinfo->testid > 0) {
        if (testinfo->testid < 7) {
            rv = tmu_taps_trie_ut(testinfo->testid, testinfo->nTestSeed);
        } else if (testinfo->testid == 7) {
            rv = tmu_taps_kshift_ut();
        } else if (testinfo->testid == 8) {
            rv = tmu_trie_split_ut(testinfo->nTestSeed);
        } else if (testinfo->testid == 9) {
            rv = tmu_taps_bpm_trie_ut(testinfo->testid, testinfo->nTestSeed);
        } else if (testinfo->testid == 10) {
            rv = tmu_taps_util_get_bpm_pfx_ut();
        } else {
            cli_out("TEST ID: %d not supported !!!!\n", testinfo->testid);
            return SOC_E_PARAM;
        }
    } else { /* run all test */
        for (index=0; index < 7 && SOC_SUCCESS(rv); index++) {
            rv = tmu_taps_trie_ut(index, testinfo->nTestSeed);
        }
        if (SOC_SUCCESS(rv)) {
            rv = tmu_taps_kshift_ut();
        }
        if (SOC_SUCCESS(rv)) {
            rv = tmu_trie_split_ut(testinfo->nTestSeed);
        }
        if (SOC_SUCCESS(rv)) {
            rv = tmu_taps_bpm_trie_ut(9, testinfo->nTestSeed);
        }
        if (SOC_SUCCESS(rv)) {
            rv = tmu_taps_util_get_bpm_pfx_ut();
        }
        if (SOC_FAILURE(rv)) {
            cli_out("\n Unit Tests Failed !!!!!!!!!!\n");
        }
    }

    return rv;
}

int
c3_ut_tmu_taps_trie_test_done(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;
    return rv;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
