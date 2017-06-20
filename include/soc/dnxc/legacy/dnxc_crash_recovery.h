/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * INFO: this module is the entry poit for Crash Recovery feature
 * 
 */

#ifndef _SOC_DNXC_CRASH_RECOVERY_H
#define _SOC_DNXC_CRASH_RECOVERY_H

#include <soc/types.h>
#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/error.h>
#include <soc/drv.h>
#endif

#include <soc/dnxc/legacy/dnxc_crash_recovery_utils.h>
#include <shared/swstate/sw_state.h>
#ifdef CRASH_RECOVERY_SUPPORT
#if defined(__KERNEL__) || !defined(__linux__)
#error "cmpl flags err: CRASH_RECOVERY_SUPPORT cannot be used w/o LINUX or with __KERNEL__"
#endif
#ifndef BCM_WARM_BOOT_SUPPORT
#error "cmpl flags err: CRASH_RECOVERY_SUPPORT cannot be used w/o BCM_WARM_BOOT_SUPPORT"
#endif
#endif

typedef enum
{
    DNXC_CR_JOURNALING_MODE_DISABLED=0,
    DNXC_CR_JOURNALING_MODE_AFTER_EACH_API,
    DNXC_CR_JOURNALING_MODE_ON_DEMAND
} DNXC_CR_JOURNALING_MODE;

#ifdef CRASH_RECOVERY_SUPPORT

#ifdef BCM_WARM_BOOT_API_TEST
#define SOC_CR_DOCUMENT_API_NAME(unit)\
    sal_strcpy(warmboot_api_function_name[unit],__FUNCTION__);
#else
#define SOC_CR_DOCUMENT_API_NAME(unit)
#endif

#define SOC_CR_DISP_ERR_CHECK(disp_rv)\
    if (disp_rv != SOC_E_NONE) {return SOC_E_INTERNAL;}

/* 
 * used to allocate/retrieve shared memory\ 
 * use str as a unique identifier for the memory segment 
 * use flags to indicate if the memory should be allocated or fetched 
 * returns a pointer to the shared memory. 
 * return NULL on failure.* 
 */
typedef void* (*dnxc_ha_mem_get)(unsigned int size, char *str, int flags);

/* 
 * used to free shared memory
 */
typedef void* (*dnxc_ha_mem_release)(unsigned int size, char *str);

int soc_dnxc_cr_transaction_start(int unit);
int soc_dnxc_cr_init(int unit);
int soc_dnxc_cr_commit(int unit);
int soc_dnxc_cr_dispatcher_commit(int unit);
int soc_dnxc_cr_abort(int unit);
int soc_dnxc_cr_recover(int unit);
int soc_dnxc_cr_suppress(int unit, dnxc_cr_no_support reason);
int soc_dnxc_cr_unsuppress(int unit);
int soc_dnxc_cr_journaling_mode_get(int unit);
int soc_dnxc_cr_is_journaling_per_api(int unit);
int soc_dnxc_cr_journaling_mode_set(int unit, DNXC_CR_JOURNALING_MODE mode);
int soc_dnxc_cr_dispatcher_transaction_start(int unit);
int soc_dnxc_cr_dispatcher_transaction_end(int unit);
int dnxc_cr_perform_commit_after_recover(int unit);
#endif /* CRASH_RECOVERY_SUPPORT */
#endif  /* _SOC_DNXC_CRASH_RECOVERY_H */

