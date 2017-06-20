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


#ifdef CRASH_RECOVERY_SUPPORT

#if defined(BCM_WARM_BOOT_API_TEST)
#include <soc/dnxc/legacy/dnxc_crash_recovery_test.h>
#endif

#include <soc/hwstate/hw_log.h>
#include <shared/swstate/sw_state_journal.h>
#include <soc/dnxc/legacy/dnxc_crash_recovery.h>
#include <soc/dnxc/legacy/dnxc_wb.h>
#include <soc/dnx/legacy/JER2_ARAD/jer2_arad_sim_em.h>
#include <soc/dnx/legacy/JER2_ARAD/jer2_arad_kbp.h>
#include <soc/dnx/legacy/JER2_JER/JER2_JER_PP/jer2_jer_pp_kaps.h>
#include <soc/scache.h>
#include <soc/dnx/legacy/drv.h>

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SHARED_SWSTATE

#define DNXC_CR_COULD_NOT_GUARANTEE 0
#define DNXC_CR_COMMITTED           1
#define DNXC_CR_ABORTED             2

#define DNXC_CR_UNIT_CHECK(unit) ((unit) >= 0 && (unit) < SOC_MAX_NUM_DEVICES)

#define DNXC_CR_DEFAULT_JOURNAL_SIZE (20000000) /*20MB*/

#if !defined(__KERNEL__) && defined (LINUX)
#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/ha.h>
#define CR_VERSION_1_0 1
#define CR_STRUCT_SIG 0
typedef enum {
    HA_CR_SUB_ID_0 = 0
} HA_CR_sub_id_tl;
#endif
#endif

uint8 dnxc_cr_suspect_crash[SOC_MAX_NUM_DEVICES];

/* INTERNAL FUNCTION DECLARATION */
static int soc_dnxc_cr_journaling_unfreeze(int unit);
static int soc_dnxc_cr_prepare_for_commit(int unit);

extern sw_state_journal_t *sw_state_journal[SOC_MAX_NUM_DEVICES];
soc_dnxc_cr_t *dnxc_cr_info[SOC_MAX_NUM_DEVICES];

int soc_dnxc_cr_init(int unit){

    uint32 hw_journal_size;
    uint32 sw_journal_size;
    uint32 mode;
    uint32 size;

    SOC_INIT_FUNC_DEFS;

    /* supported only for jer2_jericho */
    if (!SOC_IS_JER2_JERICHO(unit)) SOC_EXIT;
    /* assign the mode and exit if the CR journaling mode is disabled */
    if(!(mode = soc_property_get(unit, spn_HA_HW_JOURNAL_MODE, DNXC_CR_JOURNALING_MODE_DISABLED))) {
        SOC_EXIT;
    }

    _SOC_IF_ERR_EXIT(soc_stable_sanity(unit));

    /* allocate shared memory Crash Recovery struct */
    size = sizeof(soc_dnxc_cr_t);
    dnxc_cr_info[unit] = ha_mem_alloc(unit, HA_CR_Mem_Pool, HA_CR_SUB_ID_0, 
                           CR_VERSION_1_0, CR_STRUCT_SIG, &size);
    if (!dnxc_cr_info[unit])
    {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE,
                       (BSL_META_U(unit, "CR shared mem Allocation failed for unit %d.\n"), unit));
        return SOC_E_MEMORY;
    }

    /* set the journaling mode */
    soc_dnxc_cr_journaling_mode_set(unit, mode);

    _SOC_IF_ERR_EXIT(soc_dnxc_cr_utils_journaling_thread_set(unit));

    /* mark crash recovery as enabled */
    SOC_CR_ENABLE(unit);

    /* init HW Log */
    hw_journal_size = soc_property_get(unit, spn_HA_HW_JOURNAL_SIZE, 0);
    _SOC_IF_ERR_EXIT(soc_hw_log_init(unit, hw_journal_size));

    /* init SW State Journal */
    sw_journal_size = soc_property_get(unit, spn_HA_SW_JOURNAL_SIZE, 0);
    _SOC_IF_ERR_EXIT(sw_state_journal_init(unit, sw_journal_size));

    /* reset the logging flag */
    _SOC_IF_ERR_EXIT(soc_dnxc_cr_utils_logging_stop(unit));

    if (SOC_WARM_BOOT(unit)) {
        /* suspect a crash */
        dnxc_cr_suspect_crash[unit] = TRUE;
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_transaction_start(int unit) {
    
    SOC_INIT_FUNC_DEFS;

    /* supported only for jer2_jericho */
    if (!SOC_IS_JER2_JERICHO(unit)) SOC_EXIT;

    if (SOC_CR_ENABALED(unit)) {

#ifdef DRCM_CR_MULTIPLE_THREAD_JOURNALING
        /* set the transaction thread to the main thread */
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_utils_journaling_thread_set(unit));
#else
        /* exit if not the journaling thread */
        DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);
#endif /* DRCM_CR_MULTIPLE_THREAD_JOURNALING */

        /* exit if this is a nested API call and we are in commit per API mode*/
        if(soc_dnxc_cr_is_journaling_per_api(unit)) {
            if(!soc_dnxc_cr_api_is_top_level(unit)) {
                SOC_EXIT;
            }
        }

        /* ensure that hw log is not bypassed prior to a transaction start */
        _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));
        _SOC_IF_ERR_EXIT(soc_hw_log_ensure_not_suppressed(unit));

        if (SOC_DNX_CONFIG(unit)->jer2_arad->init.pp_enable && SOC_DNX_IS_LEM_SIM_ENABLE(unit)) {
            /* clean LEM transaction shadow */
            _SOC_IF_ERR_EXIT(chip_sim_em_delete_all(unit, JER2_ARAD_CHIP_SIM_LEM_CR_BASE, JER2_ARAD_CHIP_SIM_LEM_KEY, SOC_DNX_DEFS_GET(unit, lem_width), 
                                     JER2_ARAD_CHIP_SIM_LEM_PAYLOAD, -1, -1));
        }

        /* Mark transaction as started */
        dnxc_cr_info[unit]->transaction_mode = DNXC_TRANSACTION_MODE_LOGGING;

        /* set the logging flag */
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_utils_logging_start(unit));
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

static int 
soc_dnxc_cr_prepare_for_commit(int unit){
    SOC_INIT_FUNC_DEFS;

    /* stop the logging of the hw log and the sw state journal */
    _SOC_IF_ERR_EXIT(soc_dnxc_cr_utils_logging_stop(unit));

    /* raise ready for commit flag */
    dnxc_cr_info[unit]->transaction_mode = DNXC_TRANSACTION_MODE_COMMITTING;

    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_commit(int unit){

#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
    uint8                   kbp_dirty,kaps_dirty;
    uint32                  kbp_tbl_id,kaps_tbl_id;
#endif
    DNXC_TRANSACTION_MODE  transaction_mode;
    
    SOC_INIT_FUNC_DEFS;
    
    /* supported only for jer2_jericho */
    if (!SOC_IS_JER2_JERICHO(unit)) SOC_EXIT;

    /* sanity */
    if (!SOC_CR_ENABALED(unit)) {
        _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                       (BSL_META_U(unit,
                          "unit:%d Crash Recovery ERROR: trying to commit while Crash Recovery feature is disabled\n"), unit));
    }

    /* exit if not the journaling thread */
    DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* immediate hw access is a short term event. It should not be enabled during a commit */ 
    _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));

    /* perform some steps to insure all data is synced to NV memory */
    /* skip it if you are "re-committing after recovering from a crash */
    transaction_mode = dnxc_cr_info[unit]->transaction_mode;
    if (transaction_mode != DNXC_TRANSACTION_MODE_COMMITTING) {
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_prepare_for_commit(unit));
    }

    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.pp_enable && SOC_DNX_IS_LEM_SIM_ENABLE(unit)) {
        /* clean LEM transaction shadow */
        _SOC_IF_ERR_EXIT(chip_sim_em_delete_all(unit, JER2_ARAD_CHIP_SIM_LEM_CR_BASE, JER2_ARAD_CHIP_SIM_LEM_KEY, SOC_DNX_DEFS_GET(unit, lem_width), 
                                     JER2_ARAD_CHIP_SIM_LEM_PAYLOAD, -1, -1));
    }

    /* call kbp install if needed */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if (JER2_ARAD_KBP_IS_CR_MODE(unit)) {
        kbp_dirty = dnxc_cr_info[unit]->kbp_dirty;
        if (kbp_dirty) {
            kbp_tbl_id = dnxc_cr_info[unit]->kbp_tbl_id;
            if (jer2_arad_kbp_cr_db_commit(unit,kbp_tbl_id)) {
                _SOC_IF_ERR_EXIT(soc_dnxc_cr_abort(unit));
                _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                   (BSL_META_U(unit,
                      "unit:%d Crash Recovery ERROR: transaction commit failed to install KBP\n"), unit));
            }
            _SOC_IF_ERR_EXIT(jer2_arad_kbp_cr_transaction_cmd(unit,FALSE));
        }
        /* clean KBP dirty bit */
        dnxc_cr_info[unit]->kbp_dirty = 0;
    }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if (JER2_JER_KAPS_IS_CR_MODE(unit)) {
        /* call kaps install if needed */
        kaps_dirty = dnxc_cr_info[unit]->kaps_dirty;
        if (kaps_dirty) {
            kaps_tbl_id = dnxc_cr_info[unit]->kaps_tbl_id;
            if (jer2_jer_kaps_cr_db_commit(unit,kaps_tbl_id)) {
                _SOC_IF_ERR_EXIT(soc_dnxc_cr_abort(unit));
                _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                   (BSL_META_U(unit,
                      "unit:%d Crash Recovery ERROR: transaction commit failed to install KAPS\n"), unit));
            }
            _SOC_IF_ERR_EXIT(jer2_jer_kaps_cr_transaction_cmd(unit,FALSE));
        }
        /* clean KAPS dirty bit */
        dnxc_cr_info[unit]->kaps_dirty = 0;
    }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
#endif

    /* The following is not part of the logic of the commit.
     * It compiles only for CRASH RECOVERY api test.
     */
    DNXC_CR_TEST(_SOC_IF_ERR_EXIT(dnxc_bcm_crash_recovery_api_test_reset(unit, 1, NULL)));

    /* apply hw log */
    _SOC_IF_ERR_EXIT(soc_hw_log_commit(unit));

    /* lower ready for commit flag */
    dnxc_cr_info[unit]->transaction_mode = DNXC_TRANSACTION_MODE_IDLE;

    /* dismiss SW State roll back journal */
    _SOC_IF_ERR_EXIT(sw_state_journal_clear(unit));

    /* dismiss HW Log */
    soc_hw_log_purge(unit);

    /* unfreeze the journaling */
    _SOC_IF_ERR_EXIT(soc_dnxc_cr_journaling_unfreeze(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

static int soc_dnxc_cr_journaling_unfreeze(int unit){
    SOC_INIT_FUNC_DEFS;

    /* no point in unsuppressing if not enabled */
    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* if logging was suppressed, unsupress it after commit */
    _SOC_IF_ERR_EXIT(soc_hw_log_unsuppress(unit));

    /* reset the immediate access counter */
    _SOC_IF_ERR_EXIT(soc_hw_reset_immediate_hw_access_counter(unit));

    /* Inform that state is consistent and protected by Crash Recovery again */
    dnxc_cr_info[unit]->is_recovarable = TRUE;

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_dispatcher_commit(int unit){

    SOC_INIT_FUNC_DEFS;

    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* don't commit in case this is a nested API call */
    if(!soc_dnxc_cr_api_is_top_level(unit)) {
        SOC_EXIT;
    }

    _SOC_IF_ERR_EXIT(soc_dnxc_cr_commit(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_abort(int unit){
    SOC_INIT_FUNC_DEFS;

    /* apply roll back log */
    _SOC_IF_ERR_EXIT(sw_state_journal_roll_back(unit));

    /* dismiss SW State roll back journal */
    _SOC_IF_ERR_EXIT(sw_state_journal_clear(unit));

    /* clear kaps/kbp last transaction status */
#if defined(INCLUDE_KBP) && !defined(BCM_88030) 
    if (JER2_ARAD_KBP_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
        _SOC_IF_ERR_EXIT(jer2_arad_kbp_cr_clear_restore_status(unit));
    }
    if (JER2_JER_KAPS_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
        _SOC_IF_ERR_EXIT(jer2_jer_kaps_cr_clear_restore_status(unit));
    }
#endif

    /* dismiss HW Log */
    _SOC_IF_ERR_EXIT(soc_hw_log_purge(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_recover(int unit){
    DNXC_TRANSACTION_MODE commit_status;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
    uint8 kaps_commited = 0,kbp_commited = 0;
#endif
    int is_recovarable;
    SOC_INIT_FUNC_DEFS;

    /* if not suspecting a crash exit (e.g. we are in cold reboot) */
    if (!dnxc_cr_suspect_crash[unit]) SOC_EXIT;

    /* check if ready for commit flag is raised */
    commit_status = dnxc_cr_info[unit]->transaction_mode;
    if (commit_status == DNXC_TRANSACTION_MODE_COMMITTING) {
        /* check if KBP/KAPS committed */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
        if (JER2_ARAD_KBP_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
            kbp_commited = jer2_arad_kbp_cr_query_restore_status(unit);
        }
        if (JER2_JER_KAPS_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
            kaps_commited = jer2_jer_kaps_cr_query_restore_status(unit);
        }
		
        if (0 && (!kaps_commited || !kbp_commited)) {
		
            _SOC_IF_ERR_EXIT(soc_dnxc_cr_abort(unit));
            dnxc_cr_info[unit]->transaction_status = DNXC_CR_ABORTED;
        }
        else
#endif
        {
    
            dnxc_cr_info[unit]->perform_commit = TRUE;
    
        }
    } else if (commit_status == DNXC_TRANSACTION_MODE_LOGGING) {
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_abort(unit));
        dnxc_cr_info[unit]->transaction_status = DNXC_CR_ABORTED;
    } else if (commit_status == DNXC_TRANSACTION_MODE_IDLE) {
        dnxc_cr_info[unit]->transaction_status = DNXC_CR_COULD_NOT_GUARANTEE;
        dnxc_cr_info[unit]->not_recoverable_reason = dnxc_cr_no_support_not_in_api;
    }

    is_recovarable = dnxc_cr_info[unit]->is_recovarable;
    if (!is_recovarable) {
        dnxc_cr_info[unit]->transaction_status = DNXC_CR_COULD_NOT_GUARANTEE;
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_suppress(int unit, dnxc_cr_no_support reason){

    SOC_INIT_FUNC_DEFS;

    /* no point in suppressing if not enabled */
    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* make sure that we are currently in a middle of a transaction */
    if (!soc_dnxc_cr_utils_is_logging(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DNXC_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* immediate hw access should be disabled at the point of suppression */
    _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));

    /* Inform that state is not protected by Crash Recovery and may not be consistent after a crash */
    dnxc_cr_info[unit]->is_recovarable = FALSE;
    dnxc_cr_info[unit]->not_recoverable_reason = reason;


    _SOC_IF_ERR_EXIT(soc_dnxc_cr_commit(unit));
    _SOC_IF_ERR_EXIT(soc_hw_log_suppress(unit));
    
    dnxc_cr_info[unit]->is_recovarable = FALSE;
    
    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dnxc_cr_journaling_mode_get(int unit)
{
    return (SOC_CONTROL(unit)->journaling_mode);
}

int soc_dnxc_cr_is_journaling_per_api(int unit)
{
    return (SOC_CR_ENABALED(unit) &&
            (DNXC_CR_JOURNALING_MODE_AFTER_EACH_API == SOC_CONTROL(unit)->journaling_mode));
}

int soc_dnxc_cr_journaling_mode_set(int unit, DNXC_CR_JOURNALING_MODE mode)
{
    SOC_INIT_FUNC_DEFS;

    /* is the journaling mode is disabled, then disable the CR mechanism */
    if(DNXC_CR_JOURNALING_MODE_DISABLED == mode){
        SOC_CR_DISABLE(unit);
    }

    SOC_CONTROL(unit)->journaling_mode = mode;

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int dnxc_cr_perform_commit_after_recover(int unit) {
    SOC_INIT_FUNC_DEFS;
    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    _SOC_IF_ERR_EXIT(soc_dnxc_cr_api_counter_reset(unit));

    if (dnxc_cr_info[unit]->perform_commit == TRUE) {
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_commit(unit));
        dnxc_cr_info[unit]->perform_commit = FALSE;
        dnxc_cr_info[unit]->transaction_status = DNXC_CR_COMMITTED;
    }
    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;
}


int soc_dnxc_cr_dispatcher_transaction_start(int unit) {

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(soc_dnxc_cr_api_counter_increase(unit));

    if (soc_dnxc_cr_is_journaling_per_api(unit)) {
        _SOC_IF_ERR_EXIT(soc_dnxc_cr_transaction_start(unit));
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}


int soc_dnxc_cr_dispatcher_transaction_end(int unit) {

    int skip_commit = 0;
    SOC_INIT_FUNC_DEFS;

    /* following line performed only during CR regression */
    DNXC_CR_TEST(_SOC_IF_ERR_EXIT(dnxc_bcm_crash_recovery_api_test_reset(unit, 0, &skip_commit)));

    /* if crashed was simulated due to test mechanism skip the commit */
    if (soc_dnxc_cr_is_journaling_per_api(unit) && !skip_commit) {
        soc_dnxc_cr_dispatcher_commit(unit);
    }

    _SOC_IF_ERR_EXIT(soc_dnxc_cr_api_counter_decrease(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

#endif
