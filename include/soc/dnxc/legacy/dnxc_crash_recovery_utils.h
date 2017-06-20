/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * INFO: this is a utility module that is shared between the other Crash Recovery modules.
 * 
 */

#ifndef _SOC_DNXC_CRASH_RECOVERY_UTILITY_H
#define _SOC_DNXC_CRASH_RECOVERY_UTILITY_H

#include <soc/types.h>
#include <sal/core/thread.h>

/*#define DRCM_CR_MULTIPLE_THREAD_JOURNALING 1*/

#ifdef CRASH_RECOVERY_SUPPORT 

#ifdef BCM_WARM_BOOT_API_TEST
#define DNXC_CR_TEST(_x_) _x_
#else
#define DNXC_CR_TEST(_x_)
#endif

#define DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit) \
if (!soc_dnxc_cr_utils_is_journaling_thread(unit)) {SOC_EXIT;}

#define DNXC_CR_EXIT_IF_NOT_MAIN_THREAD \
if (sal_thread_self() != sal_thread_main_get()) {SOC_EXIT;}

typedef enum
{
    dnxc_cr_no_support_invalid = 0,
    dnxc_cr_no_support_unknown,
    dnxc_cr_no_support_not_in_api,
    dnxc_cr_no_support_legacy_wb,
    dnxc_cr_no_support_em_tables,
    dnxc_cr_no_support_tcam_tables,
    dnxc_cr_no_support_dma,
    dnxc_cr_no_support_kaps_kbp,
    dnxc_cr_no_support_count,
    dnxc_cr_no_support_wide_mem,
    dnxc_cr_no_support_portmod,
    dnxc_cr_no_support_sat,
    dnxc_cr_no_support_sw_state_alloc,
    dnxc_cr_no_support_multicast,
/*  The problem exists only in the CR test.
 *  If the API has in-out parameters, then, there is a problem
 *  with the second run of the API (after roll back) because
 *  this variable has changed and the second run differs from the first one.
 */
#ifdef BCM_WARM_BOOT_API_TEST
    dnxc_cr_no_support_in_out_parameter
#endif
} dnxc_cr_no_support;

typedef enum
{
    DNXC_TRANSACTION_MODE_INVALID = 0,
    DNXC_TRANSACTION_MODE_IDLE,
    DNXC_TRANSACTION_MODE_LOGGING,
    DNXC_TRANSACTION_MODE_COMMITTING,
    DNXC_TRANSACTION_MODE_COUNT
} DNXC_TRANSACTION_MODE;

typedef struct soc_dnxc_cr_s {
    DNXC_TRANSACTION_MODE  transaction_mode;
    int                    is_recovarable;
    int                    transaction_status;
    dnxc_cr_no_support     not_recoverable_reason;
    uint32                 nested_api_cnt; /* nested api counter */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
    uint8                  kbp_dirty;
    uint8                  kaps_dirty;
    uint32                 kbp_tbl_id;
    uint32                 kaps_tbl_id;
#endif
	
    uint8                  perform_commit;
	
} soc_dnxc_cr_t;

typedef struct soc_dnxc_cr_utils_s {
    sal_thread_t tid;        /* transaction thread id */
    uint8        is_logging; /* is journaling flag */
} soc_dcmmn_cr_utils_t;

/* define one cr utility structure instance per unit */
soc_dcmmn_cr_utils_t dnxc_cr_utils[SOC_MAX_NUM_DEVICES];

extern uint8 soc_dnxc_cr_utils_is_journaling_thread(int unit);
extern int soc_dnxc_cr_utils_journaling_thread_set(int unit);

extern uint8 soc_dnxc_cr_utils_is_logging(int unit);
extern int soc_dnxc_cr_utils_logging_start(int unit);
extern int soc_dnxc_cr_utils_logging_stop(int unit);

extern int soc_dnxc_cr_api_counter_increase(int unit);
extern int soc_dnxc_cr_api_counter_decrease(int unit);
extern int soc_dnxc_cr_api_counter_count_get(int unit);
extern int soc_dnxc_cr_api_counter_reset(int unit);
extern uint8 soc_dnxc_cr_api_is_top_level(int unit);

#endif /*CRASH_RECOVERY_SUPPORT*/
#endif  /* _SOC_DNXC_CRASH_RECOVERY_UTILITY_H */
