/*
 * $Id: switch.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * Common internal definitions for BCM switch module
 */

#ifndef _BCM_INT_SWITCH_H_
#define _BCM_INT_SWITCH_H_

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/dcmn/dcmn_crash_recovery.h>
#include <soc/dcmn/dcmn_crash_recovery_test.h>
#include <soc/dcmn/dcmn_crash_recovery_utils.h>
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/control.h>

#ifdef BCM_WARM_BOOT_API_TEST
#include <soc/defs.h>
extern char warmboot_api_function_name[SOC_MAX_NUM_DEVICES][100];
#endif

extern int _bcm_switch_state_sync(int unit, bcm_dtype_t dtype);

#if (defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)) && defined(BCM_WARM_BOOT_API_TEST)

#include <soc/dcmn/dcmn_wb.h>
#endif /* BCM_PETRA_SUPPORT && BCM_WARM_BOOT_API_TEST */

#define BCM_STATE_SYNC(_u) \
    /* next line actually call sync only in autosync mode */ \
    _bcm_switch_state_sync(_u, dtype);

#else /* !BCM_WARM_BOOT_SUPPORT */
#define BCM_STATE_SYNC(_u)
#endif /* BCM_WARM_BOOT_SUPPORT */


#if defined (BCM_WARM_BOOT_API_TEST) && !defined(CRASH_RECOVERY_SUPPORT)
#error "BCM_WARM_BOOT_API_TEST compilation is only allowed with CRASH_RECOVERY_SUPPORT"
#endif

#ifdef CRASH_RECOVERY_SUPPORT
#define BCM_CR_TRANSACTION_START(unit)\
    SOC_CR_DOCUMENT_API_NAME(unit);\
    BCM_CR_TEST_TRANSACTION_START(unit);\
    SOC_CR_DISP_ERR_CHECK(soc_dcmn_cr_dispatcher_transaction_start(unit));
#define BCM_CR_TRANSACTION_END(unit)\
    SOC_CR_DOCUMENT_API_NAME(unit);\
    SOC_CR_DISP_ERR_CHECK(soc_dcmn_cr_dispatcher_transaction_end(unit));\
    BCM_CR_TEST_TRANSACTION_END(unit);
#else /*CRASH_RECOVERY_SUPPORT*/
#define BCM_CR_TRANSACTION_START(unit)
#define BCM_CR_TRANSACTION_END(unit)
#endif /*CRASH_RECOVERY_SUPPORT*/

#endif /* _BCM_INT_SWITCH_H_ */
