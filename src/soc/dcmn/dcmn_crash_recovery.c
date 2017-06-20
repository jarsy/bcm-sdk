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
#include <soc/dcmn/dcmn_crash_recovery_test.h>
#endif

#include <soc/hwstate/hw_log.h>
#include <shared/swstate/sw_state_journal.h>
#include <soc/dcmn/dcmn_crash_recovery.h>
#include <soc/dcmn/dcmn_wb.h>
#include <soc/dpp/ARAD/arad_sim_em.h>
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/scache.h>
#include <soc/dpp/drv.h>

#include <bcm_int/common/sat.h>
#include <soc/dpp/ARAD/arad_tcam_access_crash_recovery.h>

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SHARED_SWSTATE

#define DCMN_CR_COULD_NOT_GUARANTEE 0
#define DCMN_CR_COMMITTED           1
#define DCMN_CR_ABORTED             2

#define DCMN_CR_UNIT_CHECK(unit) ((unit) >= 0 && (unit) < SOC_MAX_NUM_DEVICES)

#define DCMN_CR_DEFAULT_JOURNAL_SIZE (20000000) /*20MB*/

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

uint8 dcmn_cr_suspect_crash[SOC_MAX_NUM_DEVICES];

/* INTERNAL FUNCTION DECLARATION */
STATIC int soc_dcmn_cr_journaling_unfreeze(int unit);
STATIC int soc_dcmn_cr_prepare_for_commit(int unit);

extern sw_state_journal_t *sw_state_journal[SOC_MAX_NUM_DEVICES];
soc_dcmn_cr_t *dcmn_cr_info[SOC_MAX_NUM_DEVICES];

int soc_dcmn_cr_init(int unit){

    uint32 hw_journal_size;
    uint32 sw_journal_size;
    uint32 mode;
    uint32 size;

    SOC_INIT_FUNC_DEFS;

    /* supported only for jericho */
    if (!SOC_IS_JERICHO(unit)) SOC_EXIT;
    /* assign the mode and exit if the CR journaling mode is disabled */
    if(!(mode = soc_property_get(unit, spn_HA_HW_JOURNAL_MODE, DCMN_CR_JOURNALING_MODE_DISABLED))) {
        SOC_EXIT;
    }

    _SOC_IF_ERR_EXIT(soc_stable_sanity(unit));

    /* allocate shared memory Crash Recovery struct */
    size = sizeof(soc_dcmn_cr_t);
    dcmn_cr_info[unit] = ha_mem_alloc(unit, HA_CR_Mem_Pool, HA_CR_SUB_ID_0, 
                           CR_VERSION_1_0, CR_STRUCT_SIG, &size);
    if (!dcmn_cr_info[unit])
    {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE,
                       (BSL_META_U(unit, "CR shared mem Allocation failed for unit %d.\n"), unit));
        return SOC_E_MEMORY;
    }

    /* set the journaling mode */
    soc_dcmn_cr_journaling_mode_set(unit, mode);

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_utils_journaling_thread_set(unit));

    /* mark crash recovery as enabled */
    SOC_CR_ENABLE(unit);

    /* init HW Log */
    hw_journal_size = soc_property_get(unit, spn_HA_HW_JOURNAL_SIZE, 0);
    _SOC_IF_ERR_EXIT(soc_hw_log_init(unit, hw_journal_size));

    /* init SW State Journal */
    sw_journal_size = soc_property_get(unit, spn_HA_SW_JOURNAL_SIZE, 0);
    _SOC_IF_ERR_EXIT(sw_state_journal_init(unit, sw_journal_size));

    /* reset the logging flag */
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_utils_logging_stop(unit));

    if (SOC_WARM_BOOT(unit)) {
        /* suspect a crash */
        dcmn_cr_suspect_crash[unit] = TRUE;
    }

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_transaction_api_count_reset(unit));
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_traverse_flag_reset(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_transaction_start(int unit) {
    
    SOC_INIT_FUNC_DEFS;

    /* supported only for jericho */
    if (!SOC_IS_JERICHO(unit)) SOC_EXIT;

    if (SOC_CR_ENABALED(unit)) {

#ifdef DRCM_CR_MULTIPLE_THREAD_JOURNALING
        /* set the transaction thread to the main thread */
        _SOC_IF_ERR_EXIT(soc_dcmn_cr_utils_journaling_thread_set(unit));
#else
        /* exit if not the journaling thread */
        DCMN_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);
#endif /* DRCM_CR_MULTIPLE_THREAD_JOURNALING */

        /* exit if this is a nested API call and we are in commit per API mode*/
        if(soc_dcmn_cr_is_journaling_per_api(unit)) {
            if(!soc_dcmn_cr_api_is_top_level(unit)) {
                SOC_EXIT;
            }
        }

        /* ensure that hw log is not bypassed prior to a transaction start */
        _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));
        _SOC_IF_ERR_EXIT(soc_hw_log_ensure_not_suppressed(unit));

        if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable && SOC_DPP_IS_EM_SIM_ENABLE(unit)) {
            /* clean LEM transaction shadow */
            _SOC_IF_ERR_EXIT(chip_sim_exact_match_cr_shadow_clear(unit));

        }

        /* Mark transaction as started */
        dcmn_cr_info[unit]->transaction_mode = DCMN_TRANSACTION_MODE_LOGGING;

        /* set the logging flag */
        _SOC_IF_ERR_EXIT(soc_dcmn_cr_utils_logging_start(unit));

        /* clean TCAM transaction shadow */
        arad_tcam_access_cr_dictionary_clear(unit);
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

STATIC int 
soc_dcmn_cr_prepare_for_commit(int unit){
    SOC_INIT_FUNC_DEFS;

    /* stop the logging of the hw log and the sw state journal */
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_utils_logging_stop(unit));

    /* raise ready for commit flag */
    dcmn_cr_info[unit]->transaction_mode = DCMN_TRANSACTION_MODE_COMMITTING;

    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_commit(int unit){

#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
    uint8                   kbp_dirty,kaps_dirty;
    uint32                  kbp_tbl_id,kaps_tbl_id;
#endif
    DCMN_TRANSACTION_MODE  transaction_mode;
    
    SOC_INIT_FUNC_DEFS;
    
    /* supported only for jericho */
    if (!SOC_IS_JERICHO(unit)) SOC_EXIT;

    /* sanity */
    if (!SOC_CR_ENABALED(unit)) {
        _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                       (BSL_META_U(unit,
                          "unit:%d Crash Recovery ERROR: trying to commit while Crash Recovery feature is disabled\n"), unit));
    }

    /* exit if not the journaling thread */
    DCMN_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* immediate hw access is a short term event. It should not be enabled during a commit */ 
    _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));

    /* perform some steps to insure all data is synced to NV memory */
    /* skip it if you are "re-committing after recovering from a crash */
    transaction_mode = dcmn_cr_info[unit]->transaction_mode;
    if (transaction_mode != DCMN_TRANSACTION_MODE_COMMITTING) {
        _SOC_IF_ERR_EXIT(soc_dcmn_cr_prepare_for_commit(unit));
    }

    
    bcm_common_sat_wb_sync(unit, TRUE);

    if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable && SOC_DPP_IS_EM_SIM_ENABLE(unit)) {
        /* clean LEM transaction shadow */
        _SOC_IF_ERR_EXIT(chip_sim_exact_match_cr_shadow_clear(unit));
    }

    /* clean TCAM transaction shadow */
    arad_tcam_access_cr_dictionary_clear(unit);

    /* call kbp install if needed */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if (ARAD_KBP_IS_CR_MODE(unit)) {
        kbp_dirty = dcmn_cr_info[unit]->kbp_dirty;
        if (kbp_dirty) {
            kbp_tbl_id = dcmn_cr_info[unit]->kbp_tbl_id;
            if (arad_kbp_cr_db_commit(unit,kbp_tbl_id)) {
                _SOC_IF_ERR_EXIT(soc_dcmn_cr_abort(unit));
                _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                   (BSL_META_U(unit,
                      "unit:%d Crash Recovery ERROR: transaction commit failed to install KBP\n"), unit));
            }
            _SOC_IF_ERR_EXIT(arad_kbp_cr_transaction_cmd(unit,FALSE));
        }
        /* clean KBP dirty bit */
        dcmn_cr_info[unit]->kbp_dirty = 0;
    }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if (JER_KAPS_IS_CR_MODE(unit)) {
        /* call kaps install if needed */
        kaps_dirty = dcmn_cr_info[unit]->kaps_dirty;
        if (kaps_dirty) {
            kaps_tbl_id = dcmn_cr_info[unit]->kaps_tbl_id;
            if (jer_kaps_cr_db_commit(unit,kaps_tbl_id)) {
                _SOC_IF_ERR_EXIT(soc_dcmn_cr_abort(unit));
                _SOC_EXIT_WITH_ERR(SOC_E_INTERNAL,
                   (BSL_META_U(unit,
                      "unit:%d Crash Recovery ERROR: transaction commit failed to install KAPS\n"), unit));
            }
            _SOC_IF_ERR_EXIT(jer_kaps_cr_transaction_cmd(unit,FALSE));
        }
        /* clean KAPS dirty bit */
        dcmn_cr_info[unit]->kaps_dirty = 0;
    }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
#endif

    /* The following is not part of the logic of the commit.
     * It compiles only for CRASH RECOVERY api test.
     */
    DCMN_CR_TEST(_SOC_IF_ERR_EXIT(dcmn_bcm_crash_recovery_api_test_reset(unit, 1, NULL)));

    /* apply hw log */
    _SOC_IF_ERR_EXIT(soc_hw_log_commit(unit));

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_transaction_api_count_reset(unit));
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_traverse_flag_reset(unit));

    /* lower ready for commit flag */
    dcmn_cr_info[unit]->transaction_mode = DCMN_TRANSACTION_MODE_IDLE;

    /* dismiss SW State roll back journal */
    _SOC_IF_ERR_EXIT(sw_state_journal_clear(unit));

    /* dismiss HW Log */
    soc_hw_log_purge(unit);

    /* unfreeze the journaling */
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_journaling_unfreeze(unit));

    /* if journaling mode change has occurred but was delayed until after the commit, do it now */
    if (dcmn_cr_info[unit]->set_journaling_mode_after_commit) {
        soc_dcmn_cr_journaling_mode_set(unit,dcmn_cr_info[unit]->journaling_mode_after_commit);
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

STATIC int soc_dcmn_cr_journaling_unfreeze(int unit){
    SOC_INIT_FUNC_DEFS;

    /* no point in unsuppressing if not enabled */
    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DCMN_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* if logging was suppressed, unsupress it after commit */
    _SOC_IF_ERR_EXIT(soc_hw_log_unsuppress(unit));

    /* reset the immediate access counter */
    _SOC_IF_ERR_EXIT(soc_hw_reset_immediate_hw_access_counter(unit));

    /* Inform that state is consistent and protected by Crash Recovery again */
    dcmn_cr_info[unit]->is_recovarable = TRUE;

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_dispatcher_commit(int unit){

    SOC_INIT_FUNC_DEFS;

    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DCMN_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* don't commit in case this is a nested API call */
    if(!soc_dcmn_cr_api_is_top_level(unit)) {
        SOC_EXIT;
    }

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_commit(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_abort(int unit){
    SOC_INIT_FUNC_DEFS;

    /* apply roll back log */
    _SOC_IF_ERR_EXIT(sw_state_journal_roll_back(unit));

    /* dismiss SW State roll back journal */
    _SOC_IF_ERR_EXIT(sw_state_journal_clear(unit));

    /* clear kaps/kbp last transaction status */
#if defined(INCLUDE_KBP) && !defined(BCM_88030) 
    if (SOC_DPP_IS_ELK_ENABLE(unit) && ARAD_KBP_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
        _SOC_IF_ERR_EXIT(arad_kbp_cr_clear_restore_status(unit));
    }
    if (JER_KAPS_ENABLE(unit) && JER_KAPS_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
        _SOC_IF_ERR_EXIT(jer_kaps_cr_clear_restore_status(unit));
    }
#endif

    /* dismiss HW Log */
    _SOC_IF_ERR_EXIT(soc_hw_log_purge(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_recover(int unit){
    DCMN_TRANSACTION_MODE commit_status;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
    uint8 kaps_commited = FALSE, kbp_commited = FALSE;
    uint8 need_abort = FALSE;
#endif
    int is_recovarable;
    SOC_INIT_FUNC_DEFS;

    /* if not suspecting a crash exit (e.g. we are in cold reboot) */
    if (!dcmn_cr_suspect_crash[unit]) SOC_EXIT;

    /* check if ready for commit flag is raised */
    commit_status = dcmn_cr_info[unit]->transaction_mode;
    if (commit_status == DCMN_TRANSACTION_MODE_COMMITTING) {
        /* check if KBP/KAPS committed */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)        
        if (SOC_DPP_IS_ELK_ENABLE(unit) && ARAD_KBP_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
            kbp_commited = arad_kbp_cr_query_restore_status(unit);
            if (!kbp_commited) need_abort = TRUE;
        }
        if (JER_KAPS_ENABLE(unit) && JER_KAPS_IS_CR_MODE(unit) && SOC_IS_DONE_INIT(unit)) {
            kaps_commited = jer_kaps_cr_query_restore_status(unit);
            if (!kaps_commited) need_abort = TRUE;
        }
        if (need_abort) {
            _SOC_IF_ERR_EXIT(soc_dcmn_cr_abort(unit));
            dcmn_cr_info[unit]->transaction_status = DCMN_CR_ABORTED;
        }
        else
#endif
        {
            _SOC_IF_ERR_EXIT(soc_dcmn_cr_api_counter_reset(unit));
            _SOC_IF_ERR_EXIT(soc_dcmn_cr_commit(unit));
        }
    } else if (commit_status == DCMN_TRANSACTION_MODE_LOGGING) {
        _SOC_IF_ERR_EXIT(soc_dcmn_cr_abort(unit));
        dcmn_cr_info[unit]->transaction_status = DCMN_CR_ABORTED;
    } else if (commit_status == DCMN_TRANSACTION_MODE_IDLE) {
        dcmn_cr_info[unit]->transaction_status = DCMN_CR_COULD_NOT_GUARANTEE;
        dcmn_cr_info[unit]->not_recoverable_reason = dcmn_cr_no_support_not_in_api;
    }

    is_recovarable = dcmn_cr_info[unit]->is_recovarable;
    if (!is_recovarable) {
        dcmn_cr_info[unit]->transaction_status = DCMN_CR_COULD_NOT_GUARANTEE;
    }

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_suppress(int unit, dcmn_cr_no_support reason){

    SOC_INIT_FUNC_DEFS;

    /* no point in suppressing if not enabled */
    if (!SOC_CR_ENABALED(unit)) SOC_EXIT;

    /* make sure that we are currently in a middle of a transaction */
    if (!soc_dcmn_cr_utils_is_logging(unit)) SOC_EXIT;

    /* exit if not the journaling thread */
    DCMN_CR_EXIT_IF_NOT_JOURNALING_THREAD(unit);

    /* immediate hw access should be disabled at the point of suppression */
    _SOC_IF_ERR_EXIT(soc_hw_ensure_immediate_hw_access_disabled(unit));

    /* Inform that state is not protected by Crash Recovery and may not be consistent after a crash */
    dcmn_cr_info[unit]->is_recovarable = FALSE;
    dcmn_cr_info[unit]->not_recoverable_reason = reason;


    _SOC_IF_ERR_EXIT(soc_dcmn_cr_commit(unit));
    _SOC_IF_ERR_EXIT(soc_hw_log_suppress(unit));
    
    dcmn_cr_info[unit]->is_recovarable = FALSE;
    
    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_journaling_mode_get(int unit)
{
    return (SOC_CONTROL(unit)->journaling_mode);
}

int soc_dcmn_cr_is_journaling_per_api(int unit)
{
    return (SOC_CR_ENABALED(unit) &&
            (DCMN_CR_JOURNALING_MODE_AFTER_EACH_API == SOC_CONTROL(unit)->journaling_mode));
}

int soc_dcmn_cr_is_journaling_on_demand(int unit)
{
    return (SOC_CR_ENABALED(unit) &&
            (DCMN_CR_JOURNALING_MODE_ON_DEMAND == SOC_CONTROL(unit)->journaling_mode));
}

int soc_dcmn_cr_journaling_mode_set(int unit, DCMN_CR_JOURNALING_MODE mode)
{
    SOC_INIT_FUNC_DEFS;

    /* is the journaling mode is disabled, then disable the CR mechanism */
    if(DCMN_CR_JOURNALING_MODE_DISABLED == mode){
        SOC_CR_DISABLE(unit);
    }

    /*  if we are currently logging then we delay the change in journaling mode until after the next commit */
	if(!soc_dcmn_cr_utils_is_logging(unit)){
           SOC_CONTROL(unit)->journaling_mode = mode;

           if (SOC_CR_ENABALED(unit)){
              dcmn_cr_info[unit]->set_journaling_mode_after_commit = FALSE;
           }

        SOC_EXIT;
	}

    /* sets the flags that indicates to change the jornaling mode after commit */
    dcmn_cr_info[unit]->set_journaling_mode_after_commit = TRUE;
    dcmn_cr_info[unit]->journaling_mode_after_commit = mode;

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_dispatcher_transaction_start(int unit) {

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_api_counter_increase(unit));

    if (soc_dcmn_cr_is_journaling_per_api(unit)) {
        _SOC_IF_ERR_EXIT(soc_dcmn_cr_transaction_start(unit));
    }

    /* first perform the samity check and then increment the API counter */
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_ondemand_sanity_check(unit));
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_transaction_api_count_increase(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}


int soc_dcmn_cr_dispatcher_transaction_end(int unit) {

    int skip_commit = 0;
    SOC_INIT_FUNC_DEFS;

    /* following line performed only during CR regression */
    DCMN_CR_TEST(_SOC_IF_ERR_EXIT(dcmn_bcm_crash_recovery_api_test_reset(unit, 0, &skip_commit)));

    /* if crashed was simulated due to test mechanism skip the commit */
    if (soc_dcmn_cr_is_journaling_per_api(unit) && !skip_commit) {
        soc_dcmn_cr_dispatcher_commit(unit);
    }

    _SOC_IF_ERR_EXIT(soc_dcmn_cr_api_counter_decrease(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int soc_dcmn_cr_ondemand_em_traverse_hit(int unit)
{
    SOC_INIT_FUNC_DEFS;

    /* should be relevant only for on-demand transaction handling */
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_traverse_flag_set(unit));
    _SOC_IF_ERR_EXIT(soc_dcmn_cr_ondemand_sanity_check(unit));

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;

}

int soc_dcmn_cr_ondemand_sanity_check(int unit)
{
    SOC_INIT_FUNC_DEFS;

    if (!SOC_CR_ENABALED(unit) || !soc_dcmn_cr_utils_is_logging(unit)) SOC_EXIT;

    /* if there is more than one api in the transaction and there exists such that contains an EM traverse */
    if((soc_dcmn_cr_transaction_api_count_get(unit) > 1) &&
        soc_dcmn_cr_traverse_flag_get(unit)) {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE,
                       (BSL_META_U(unit, "CR ON-DEMAND: APIs containg EM traverse operations must be in a dadicated transaction, unit %d.\n"), unit));
        return SOC_E_FAIL;
    }
    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

#endif
