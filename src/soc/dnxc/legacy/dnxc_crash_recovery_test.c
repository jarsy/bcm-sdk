/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * INFO: this module is the entry poit for Crash Recovery feature
 *
 * some design details:
 * - transaction starts at the beginning of an API and ends at the end of an API
 *
 */

#include <soc/types.h>
#include <soc/error.h>
#include <shared/bsl.h>

#if defined(CRASH_RECOVERY_SUPPORT)
#include <soc/dnxc/legacy/dnxc_crash_recovery.h>
#include <soc/dnxc/legacy/dnxc_crash_recovery_test.h>
#endif

#if defined(CRASH_RECOVERY_SUPPORT) && defined(BCM_WARM_BOOT_API_TEST)
#include <soc/dnxc/legacy/dnxc_wb.h>
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SHARED_SWSTATE

extern soc_dnxc_cr_t *dnxc_cr_info[SOC_MAX_NUM_DEVICES];

int iteration_occured[SOC_MAX_NUM_DEVICES][2];
int cr_test_done[SOC_MAX_NUM_DEVICES] = { 1 }; /* only unit 0 is initialized, but it's the only one tested */
int cr_test_ttl_counter[SOC_MAX_NUM_DEVICES];

extern char warmboot_api_function_name[SOC_MAX_NUM_DEVICES][100];

int dnxc_cr_test_prepare(int unit) {
    /* verify that test session is done before starting a new one */

    if (!dnxc_wb_cr_all_reset_test_utils_preconditions(unit))
    {
        return 0;
    }

    if (!soc_dnxc_cr_is_journaling_per_api(unit)) {
        return 0;
    }

    /* sanity check - we expect test to be marked as done before preparing a new test */
    assert(cr_test_done[unit]);

    cr_test_done[unit] = 0;
    cr_test_ttl_counter[unit] = 0;
    iteration_occured[unit][0] = 0;
    iteration_occured[unit][1] = 0;

    return 0;
}

int dnxc_cr_test_keep_looping(int unit) {

    int test_mode;

    if (!dnxc_wb_cr_all_reset_test_utils_preconditions(unit))
    {
        return 0;
    }

    if (!soc_dnxc_cr_is_journaling_per_api(unit)) {
        return 0;
    }

    /* verify we are not looping more than two times */
    /* we may loop up to two times - (1)roll-back & (2)re-commit */
    cr_test_ttl_counter[unit]++;
    assert(cr_test_ttl_counter[unit] < 3);

    /* if not CR test mode, don't loop, API only called once */
    soc_dnxc_wb_test_mode_get(unit, &test_mode);
    if (_DNXC_BCM_WARM_BOOT_API_TEST_MODE_CRASH_RECOVERY != test_mode)
    {
        return 0;
    }

    /* stop looping if both iterations occured */
    return !(iteration_occured[unit][0] && iteration_occured[unit][1]);
}

int dnxc_cr_test_clean(int unit) {

    if (!dnxc_wb_cr_all_reset_test_utils_preconditions(unit))
    {
        return 0;
    }

    /* we clean the test also if not in journaling mode
       to work with switch_control API that toggle the journaling flag */

    cr_test_done[unit] = 1;
    return 0;
}


int dnxc_bcm_crash_recovery_api_test_reset(int unit, int iteration, int *was_reset) {

    int test_counter;
    int warmboot_test_mode_enable;
    int nof_skip = 0;

    /* assume reset did not occur until proven otherwise */
    if (was_reset) {
        *was_reset = 0;
    }

    if (!dnxc_wb_cr_all_reset_test_utils_preconditions(unit)) {
        return 0;
    }

    /* if not cr test mode, just exit without error */
    soc_dnxc_wb_test_mode_get(unit, &warmboot_test_mode_enable);
    if (_DNXC_BCM_WARM_BOOT_API_TEST_MODE_CRASH_RECOVERY != warmboot_test_mode_enable)
    {
        return 0;
    }

    /* if iteration ID was already performed for this API, just exit without error */
    if (iteration_occured[unit][iteration])
    {
        return 0;
    }
    else
    {
        /* mark that iteration ID is being performed */
        iteration_occured[unit][iteration] = TRUE;
    }

    /* if preconditions for reset don't hold just ack and exit */
    if (!dnxc_wb_cr_common_api_preconditions(unit)) {
        if (iteration == 1) {
            /* re-enable only afer second iteration */
            dnxc_bcm_warm_boot_api_test_enable_if_disabled_once(unit);
        }
        return 0;
    }

    /* CR must be enabled if we are to perform CR regression */
    assert(SOC_CR_ENABALED(unit));

    /* specific Crash-recovery precondition for reset test */
    if (soc_dnxc_cr_is_journaling_per_api(unit) && !dnxc_cr_info[unit]->is_recovarable) {
        return 0;
    }

    /* increase reset test counter only in first iteration*/
    /* note that counter is being increased only if preconditions hold */
    if (iteration == 0) {
        soc_dnxc_wb_test_counter_plus_plus(unit);
    }

    /* do exponential backoff, decrease frequency of reset tests */
    soc_dnxc_wb_test_counter_get(unit, &test_counter);
    if (!dnxc_wb_exp_backoff(test_counter, &nof_skip)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "\n  --> *** CR BCM API %s *** skipping %d crash tests for faster regression\n"),
                  warmboot_api_function_name[unit] , nof_skip));
        return 0;
    }

    LOG_WARN(BSL_LS_BCM_COMMON,
             (BSL_META_U(unit,
              "\n**** Crash Recovery BCM API %s **** (test counter: %d, iteration: [%d]%s) ****\n"),
              warmboot_api_function_name[unit], test_counter, iteration,
              (iteration==0)? "roll-back" : (iteration==1)? "commit" : "value not expected (ERROR)!!!"));

    LOG_DEBUG(BSL_LS_BCM_COMMON, (BSL_META_U(unit,
        "Unit:%d Starting warm reboot test\n"), unit));

    /* perform the reset test */
    if (was_reset) {
        *was_reset = 1;
    }
    if (sh_process_command(unit, TR_141_COMMAND(TRUE)) != 0) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
            (BSL_META_U(unit, "Unit:%d Warm reboot test failed\n"), unit));
        return -1;
    } else {
        LOG_WARN(BSL_LS_BCM_COMMON,
            (BSL_META_U(unit, "Unit:%d Warm reboot test finish successfully\n"), unit));
    }

    return 0;
}

#endif
