/*
 * $Id: dfe_port.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DCMN WB
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

#include <soc/defs.h>
#include <soc/error.h>
#include <soc/drv.h>


#if (defined(BCM_DFE_SUPPORT) || defined(BCM_DPP_SUPPORT)) && defined(BCM_WARM_BOOT_SUPPORT)

#include <soc/dcmn/dcmn_wb.h>
#include <soc/dcmn/dcmn_crash_recovery.h>
#include <shared/bsl.h>
#include <appl/diag/shell.h>

#ifdef BCM_WARM_BOOT_API_TEST

/* general flag, indication whether we are in WB test mode or not */
int    _soc_dcmn_wb_warmboot_test_mode[SOC_MAX_NUM_DEVICES];
/* temporary disabling WB test mode */
int    _soc_dcmn_wb_override_wb_test[SOC_MAX_NUM_DEVICES];
/* temporary disabling WB test mode due to temporarily field unstability */
int    _soc_dcmn_wb_field_test_mode_stop[SOC_MAX_NUM_DEVICES][_dcmnWbFieldFlagNOF];
/* disabling WB test mode for one BCM API call */
int    _soc_dcmn_wb_disable_once_wb_test[SOC_MAX_NUM_DEVICES];
/* counter, counting the number of warmboot test performed */
int    _soc_dcmn_wb_warmboot_test_counter[SOC_MAX_NUM_DEVICES];
/* counter, counting the number of APIs in current stack */
int    _soc_dcmn_wb_nested_api_counter[SOC_MAX_NUM_DEVICES];

char warmboot_api_function_name[SOC_MAX_NUM_DEVICES][100];

/* 
 * this is intended to be used by all utilities that participate in wb/cr
 * reset test, checking sanity stuff like unit=0, API in API, main thread
 * not during warmboot or detach
 */
int dcmn_wb_cr_all_reset_test_utils_preconditions (int unit) {

    /* some basic sanity conditions */
    if (!SOC_UNIT_VALID(unit)) 
    {
        return 0;
    }
    if (!SOC_IS_DONE_INIT(unit)) 
    {
        return 0;
    }
    if (SOC_WARM_BOOT(unit)) 
    {
        return 0;
    }
    if (SOC_IS_DETACHING(unit)) 
    {
        return 0;
    }

    /* must be main thread */
    if (sal_thread_self() != sal_thread_main_get()) {
        return 0;
    }

    /* check that we are not in nested API (API in API) */
    if (!dcmn_wb_api_counter_is_top_level(unit)) {
        return 0;
    }

    return 1;
}
/* 
 * check pre-conditions that apply for both WB and CR regression
 * 0 - don't hold
 * 1 - preconditions hold
 */
int dcmn_wb_cr_common_api_preconditions (int unit) {
    int no_wb_test;
    int field_wb_stop;
    int disable_once;

    if (!dcmn_wb_cr_all_reset_test_utils_preconditions(unit)) {
        return 0;
    }

    
    soc_dcmn_wb_field_test_mode_get(unit, _dcmnWbFieldFlagDirectExtraction, &field_wb_stop);
    if (field_wb_stop) {
        return 0;
    }
    soc_dcmn_wb_field_test_mode_get(unit, _dcmnWbFieldFlagDataQual, &field_wb_stop);
    if (field_wb_stop) {
        return 0;
    }

    /* check if we were signaled not to perform any resets */
    soc_dcmn_wb_no_wb_test_get(unit, &no_wb_test);
    if (no_wb_test) {
        return 0;
    }

    /* check if we were signaled not to perform any resets */
    soc_dcmn_wb_disable_once_get(unit, &disable_once);
    if (disable_once) {
        return 0;
    }

    /* all good! */
    return 1;
}

/* in case WB sequence was skipped once, */
/* setting the flag to '0' in order that next time WB sequence will occur  */
void dcmn_bcm_warm_boot_api_test_enable_if_disabled_once(int unit) {
    int disable_once;

    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));

    soc_dcmn_wb_disable_once_get(unit, &disable_once);
    if (1 == disable_once) {
        soc_dcmn_wb_disable_once_set(unit, 0);
    }
    return;
}

/*!
 * \brief dcmn_bcm_warm_boot_api_test_reset
 *
 * This function simulate warm boot via calling tr 141
 * only at the end of API when running in warm boot mode regression.
 *
 * \param [in] unit
 *
 * \retval 0  - tr 141 finished successfully
 * \retval -1  - Warm boot test failed
 *
 */
int dcmn_bcm_warm_boot_api_test_reset(int unit) {

    int test_counter;
    int warmboot_test_mode_enable;
    int nof_skip = 0;

    if (!dcmn_wb_cr_all_reset_test_utils_preconditions(unit)) {
        return 0;
    }

    /* if not wb test mode, just exit without error */
    soc_dcmn_wb_test_mode_get(unit, &warmboot_test_mode_enable);
    if (_DCMN_BCM_WARM_BOOT_API_TEST_MODE_AFTER_EVERY_API != warmboot_test_mode_enable)
    {
        return 0;
    }

    /* if preconditions for reset don't hold just ack and exit */
    if (!dcmn_wb_cr_common_api_preconditions(unit)) {
        dcmn_bcm_warm_boot_api_test_enable_if_disabled_once(unit);
        return 0;
    }

    /* for now don't allow wb test mode regression if CR is enabled */
    assert(!SOC_CR_ENABALED(unit));

    /* increase reset test counter */
    /* note that counter is being increased only if preconditions hold */
    soc_dcmn_wb_test_counter_plus_plus(unit);

    /* do exponential backoff, decrease frequency of reset tests */
    /* don't back off for FE */
    soc_dcmn_wb_test_counter_get(unit, &test_counter);
    if (!SOC_IS_DFE(unit) && !dcmn_wb_exp_backoff(test_counter, &nof_skip)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "  --> *** WB BCM API %s *** skipping %d warm reboot tests for faster regression\n"),
                  warmboot_api_function_name[unit] , nof_skip));
        return 0;
    }

    LOG_WARN(BSL_LS_BCM_COMMON,
             (BSL_META_U(unit,
              "**** WB BCM API %s **** (test counter: %d) ****\n"),
              warmboot_api_function_name[unit], test_counter));


    LOG_DEBUG(BSL_LS_BCM_COMMON, (BSL_META_U(unit,
        "Unit:%d Starting warm reboot test\n"), unit));

    /* perform the reset test */
    if (sh_process_command(unit, TR_141_COMMAND(FALSE)) != 0) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
            (BSL_META_U(unit, "Unit:%d Warm reboot test failed\n"), unit));
        return -1;
    } else {
        LOG_WARN(BSL_LS_BCM_COMMON,
            (BSL_META_U(unit, "Unit:%d Warm reboot test finish successfully\n"), unit));
    }

    return 0;
}

/* 
 * allowed values for warmboot_test_mode_set/get: 
 * ---------------------------------------------- 
 * 0:WB_TEST_MODE_NONE
 * 1:WB_TEST_MODE_AFTER_EVERY_API
 * 2:WB_TEST_MODE_END_OF_DVAPIS 
 * 3:WB_TEST_MODE_CRASH_RECOVERY
 */

void
soc_dcmn_wb_test_mode_set(int unit, int enable)
{
    _soc_dcmn_wb_warmboot_test_mode[unit] = enable;
}

void
soc_dcmn_wb_test_mode_get(int unit, int *enable)
{
    *enable =  _soc_dcmn_wb_warmboot_test_mode[unit];
}

void
soc_dcmn_wb_test_counter_set(int unit, int counter)
{
    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));
    _soc_dcmn_wb_warmboot_test_counter[unit] = counter;
}

void
soc_dcmn_wb_test_counter_get(int unit, int *counter)
{
    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));
    *counter =  _soc_dcmn_wb_warmboot_test_counter[unit];
}

void
soc_dcmn_wb_test_counter_plus_plus(int unit)
{
    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));
    _soc_dcmn_wb_warmboot_test_counter[unit]++;
}

void
soc_dcmn_wb_test_counter_reset(int unit)
{
    _soc_dcmn_wb_warmboot_test_counter[unit] = 0;
}


/* the two functions below set\get a flag that override the wb test mode
 * i.e if test mode is on (perform warm rebooot at the end of APIs) turning this
 * flag on will instruct the driver to not perform the reboots  */ 
void
soc_dcmn_wb_no_wb_test_set(int unit, int wb_flag)
{
    if (wb_flag == 0)
    {
        _soc_dcmn_wb_override_wb_test[unit]--;
    }
    else
    {
        _soc_dcmn_wb_override_wb_test[unit]++;
    }
}

void
soc_dcmn_wb_no_wb_test_get(int unit, int *wb_flag)
{
    *wb_flag =  _soc_dcmn_wb_override_wb_test[unit];
}


void soc_dcmn_wb_field_test_mode_set(int unit, _dcmn_wb_field_falgs_e type, int wb_flag)
{
    /* only verify it's not called from other thread */
    assert(sal_thread_self() == sal_thread_main_get());

    _soc_dcmn_wb_field_test_mode_stop[unit][type] = wb_flag;
}  
void soc_dcmn_wb_field_test_mode_get(int unit, _dcmn_wb_field_falgs_e type, int *wb_flag)
{
    /* only verify it's not called from other thread */
    assert(sal_thread_self() == sal_thread_main_get());

    *wb_flag =  _soc_dcmn_wb_field_test_mode_stop[unit][type];
} 

/* the two functions below set\get a flag that disable the wb test mode
 * i.e if test mode is on (perform warm rebooot at the end of APIs) turning this
 * flag on will instruct the driver to not perform the when the BCM API finish to run, 
 * should be used by APIs that
 * create a mismatch btween SW state and HW state, for example: field instructions saved to SW
 * but not yet commited to HW should turn on this flag. */ 
void
soc_dcmn_wb_disable_once_set(int unit, int wb_flag)
{
    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));
    _soc_dcmn_wb_disable_once_wb_test[unit] = wb_flag;
}

void
soc_dcmn_wb_disable_once_get(int unit, int *wb_flag)
{
    assert(dcmn_wb_cr_all_reset_test_utils_preconditions(unit));
    *wb_flag = _soc_dcmn_wb_disable_once_wb_test[unit];
}

int dcmn_bcm_warm_boot_api_test_mode_skip_wb_sequence(int unit) {
    int rv = BCM_E_NONE;
    if (dcmn_wb_cr_all_reset_test_utils_preconditions(unit)) {
        soc_dcmn_wb_disable_once_set(unit, 1);
    }
    return rv;
}

/* return 1 if test should be performed or 0 if test should be masked */
/* return in nof_skip the number of tests we are going to skip (if it's the first of them) */
int dcmn_wb_exp_backoff(int test_counter, int *nof_skip) {
    int i = DCMN_WARM_EXP_PARAM;
    int skip = 0;

    while (i < test_counter) {
        i *= DCMN_WARM_EXP_PARAM;
    }
    if (test_counter % (i/DCMN_WARM_EXP_PARAM) == 0 || test_counter == 0) {
      return 1;
    }
    skip = i / DCMN_WARM_EXP_PARAM - test_counter % (i/DCMN_WARM_EXP_PARAM);
    if (nof_skip != NULL && skip + 1 == i/DCMN_WARM_EXP_PARAM) *nof_skip = skip;
    else if (nof_skip != NULL) *nof_skip = 0;
    return 0;
}

int dcmn_wb_api_counter_increase(int unit)
{
    if (!(SOC_UNIT_VALID(unit) &&
          SOC_IS_DONE_INIT(unit)   &&
          (!SOC_WARM_BOOT(unit))   && 
          (!SOC_IS_DETACHING(unit))))
    {
        return 0;
    }

    /* do only for main thread */
    if (sal_thread_self() != sal_thread_main_get()) {
        return 0;
    }

    _soc_dcmn_wb_nested_api_counter[unit]++;

    assert(_soc_dcmn_wb_nested_api_counter[unit] >= 0);
    assert(_soc_dcmn_wb_nested_api_counter[unit] < 4);

    return 0;
}

int dcmn_wb_api_counter_decrease(int unit)
{
    if (!(SOC_UNIT_VALID(unit) &&
          SOC_IS_DONE_INIT(unit)   &&
          (!SOC_WARM_BOOT(unit))   && 
          (!SOC_IS_DETACHING(unit))))
    {
        return 0;
    }

    /* do only for main thread */
    if (sal_thread_self() != sal_thread_main_get()) {
        return 0;
    }

    /* return an error if outside a transaction */
    assert(_soc_dcmn_wb_nested_api_counter[unit]>0);

    _soc_dcmn_wb_nested_api_counter[unit]--;

    assert(_soc_dcmn_wb_nested_api_counter[unit] >= 0);
    assert(_soc_dcmn_wb_nested_api_counter[unit] < 4);

    return 0;
}

int dcmn_wb_api_counter_get(int unit)
{
    if (!(SOC_UNIT_VALID(unit) &&
         SOC_IS_DONE_INIT(unit)   &&
         (!SOC_WARM_BOOT(unit))   && 
         (!SOC_IS_DETACHING(unit))))
    {
        return 1;
    }

    if (sal_thread_self() != sal_thread_main_get()) {
        return 1;
    }

    assert(_soc_dcmn_wb_nested_api_counter[unit] >= 0);
    assert(_soc_dcmn_wb_nested_api_counter[unit] < 4);

    return _soc_dcmn_wb_nested_api_counter[unit];
}

int dcmn_wb_api_counter_reset(int unit)
{
    if (!(SOC_UNIT_VALID(unit) &&
         SOC_IS_DONE_INIT(unit)   &&
         (!SOC_WARM_BOOT(unit))   && 
         (!SOC_IS_DETACHING(unit))))
    {
        return 0;
    }

    /* do only for main thread */
    if (sal_thread_self() != sal_thread_main_get()) {
        return 0;
    }

    _soc_dcmn_wb_nested_api_counter[unit] = 0;

    return 0;
}

int dcmn_wb_api_counter_is_top_level(int unit)
{
    /* we don't expect counter to be zero - should always be at least inside one API */
    if(!SOC_CR_ENABALED(unit) || soc_dcmn_cr_is_journaling_per_api(unit)) {
        assert(dcmn_wb_api_counter_get(unit));
    }

    /* if used not in crash recovery context */
    if (!SOC_CR_ENABALED(unit)) {
        return (dcmn_wb_api_counter_get(unit) == 1);
    }
    else {
        return ((!soc_dcmn_cr_is_journaling_per_api(unit)) || /* always return true if not in API commit mode */
                (dcmn_wb_api_counter_get(unit) == 1));
    }
}

#endif /* BCM_WARM_BOOT_API_TEST */
#endif /*(defined(BCM_DFE_SUPPORT) || defined(BCM_DPP_SUPPORT)) && defined(BCM_WARM_BOOT_SUPPORT)*/
