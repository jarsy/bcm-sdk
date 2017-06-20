/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * INFO: this module is the entry poit for Crash Recovery feature
 *
 */

#ifndef _SOC_DCMN_CRASH_RECOVERY_TEST_H
#define _SOC_DCMN_CRASH_RECOVERY_TEST_H

#if defined (CRASH_RECOVERY_SUPPORT) && defined (BCM_WARM_BOOT_API_TEST)
#include <soc/dcmn/dcmn_crash_recovery_utils.h>
#include <soc/dcmn/dcmn_wb.h>

#ifdef SW_STATE_DIFF_DEBUG
#define CR_DEBUG(x) x
#else
#define CR_DEBUG(x)
#endif

int dcmn_cr_test_prepare(int unit);
int dcmn_cr_test_keep_looping(int unit);
int dcmn_cr_test_clean(int unit);
int dcmn_bcm_crash_recovery_api_test_reset(int unit, int iteration, int *was_reset);


#define BCM_CR_TEST_TRANSACTION_START(unit) \
    r_rv = BCM_E_NONE; /* initialization of the api returned value in order to avoid build error with optimization */\
    dcmn_wb_api_counter_increase(unit);\
    dcmn_cr_test_prepare(unit);\
    do {

/* the API logic comes between BCM_CR_TRANSACTION_START and BCM_CR_TRANSACTION_END*/

#define BCM_CR_TEST_TRANSACTION_END(unit) \
    } while (dcmn_cr_test_keep_looping(unit));\
    dcmn_cr_test_clean(unit);\
    SOC_CR_DISP_ERR_CHECK(dcmn_bcm_warm_boot_api_test_reset(unit));\
    dcmn_wb_api_counter_decrease(unit);

#else /* CRASH_RECOVERY_SUPPORT && BCM_WARM_BOOT_API_TEST */
#define BCM_CR_TEST_TRANSACTION_START(unit)
#define BCM_CR_TEST_TRANSACTION_END(unit)
#endif /* CRASH_RECOVERY_SUPPORT && BCM_WARM_BOOT_API_TEST */

#endif  /* _SOC_DCMN_CRASH_RECOVERY_TEST_H */
