/*
 * $Id: arlmsg.c,v 1.61 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    arlmsg.c
 * Purpose: Keep a synchronized ARL shadow table.
 *      Provide a reliable stream of ARL insert/delete messages.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/time.h>


#include <soc/mem.h>
#include <soc/cm.h>

#include <soc/robo/mcm/driver.h>
#include <soc/robo/arl.h>
#ifdef BCM_TB_SUPPORT
#include "tbx/robo_tbx.h"
#endif

/****************************************************************************
 *
 * ARL Message Registration
 *
 ****************************************************************************/
#define ONESEC  1000000 /* 1 second = 1000000 us */
#define ARL_CB_MAX      3

typedef struct arl_cb_entry_s {
    soc_robo_arl_cb_fn   fn;
    void    *fn_data;
} arl_cb_entry_t;

typedef struct arl_data_s {
    arl_cb_entry_t  cb[ARL_CB_MAX];
    int                 cb_count;
} arl_data_t;

/* added_list : to log user inserted ARL/MARL first before this entry is sync
 * in ARL thread.
 *  - SW ARL for this ARL/MARL is existed already.
 *  - L2 registered callback is issued for that insert already.
 */
static  SHR_BITDCL   *added_list = NULL;
static  SHR_BITDCL   *delete_list = NULL;
STATIC arl_data_t arl_data[SOC_MAX_NUM_SWITCH_DEVICES];
STATIC void soc_arl_thread(void *unit_vp);

#ifdef BCM_TB_SUPPORT
STATIC void soc_arl_thread_tb(void *unit_vp);


/* TB B0 scan valid get 512 bits (256 entries) per index read*/
#define TB_ARL_SCAN_VALID_SIZE     512

#define TB_ARL_SCAN_VALID_ITER(exist_list, idx, count) \
    for((idx)=0; (idx) < (count); (idx)++)\
        if(SHR_BITGET(exist_list, idx))\
            if((idx)%2==0)

#endif

/*
 * Function:
 *  soc_robo_arl_register
 * Purpose:
 *  Register a callback routine to be notified of all inserts,
 *  deletes, and updates to the ARL.
 * Parameters:
 *  unit - RoboSwitch unit number
 *  fn - Callback function to register
 *  fn_data - Extra data passed to callback function
 * Returns:
 *  SOC_E_NONE - Success
 *  SOC_E_MEMORY - Too many callbacks registered
 */
int
soc_robo_arl_register(int unit, soc_robo_arl_cb_fn fn, void *fn_data)
{
    arl_data_t      *ad = &arl_data[unit];

    if (ad->cb_count >= ARL_CB_MAX) {
        return SOC_E_MEMORY;
    }

    ad->cb[ad->cb_count].fn = fn;
    ad->cb[ad->cb_count].fn_data = fn_data;

    ad->cb_count++;

    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_robo_arl_unregister
 * Purpose:
 *  Unregister a callback routine; requires same args as when registered
 * Parameters:
 *  unit - RoboSwitch unit number
 *  fn - Callback function to unregister; NULL to unregister all
 *  fn_data - Extra data passed to callback function;
 *  must match registered value unless fn is NULL
 * Returns:
 *  SOC_E_NONE - Success
 *  SOC_E_NOT_FOUND - Matching registered routine not found
 */
int
soc_robo_arl_unregister(int unit, soc_robo_arl_cb_fn fn, void *fn_data)
{
    arl_data_t      *ad = &arl_data[unit];
    int         i;

    if (fn == NULL) {
        ad->cb_count = 0;
        return SOC_E_NONE;
    }

    for (i = 0; i < ad->cb_count; i++) {
        if ((ad->cb[ad->cb_count].fn == fn &&
             ad->cb[ad->cb_count].fn_data == fn_data)) {

            for (ad->cb_count--; i < ad->cb_count; i++) {
                sal_memcpy(&ad->cb[i], &ad->cb[i + 1],
                    sizeof (arl_cb_entry_t));
            }

            return SOC_E_NONE;
        }
    }

    return SOC_E_NOT_FOUND;
}
static int arl_sw_entry_count = 0;
static int arl_sw_entry_count_prev = 0;
/*
 * Function:
 *  soc_arl_callback
 * Purpose:
 *  Routine to execute all callbacks on the list.
 * Parameters:
 *  unit - unit number.
 *  entry_del - deleted or updated entry, NULL if none.
 *  entry_add - added or updated entry, NULL if none.
 */
void
soc_robo_arl_callback(int unit, 
                 l2_arl_sw_entry_t *entry_del, l2_arl_sw_entry_t *entry_add)
{
    arl_data_t      *ad = &arl_data[unit];
    int         i;

    if( entry_del == NULL)
        arl_sw_entry_count++;
    if( entry_add == NULL)
        arl_sw_entry_count--;

    for (i = 0; i < ad->cb_count; i++) {
        (*ad->cb[i].fn)(unit, entry_del, entry_add, ad->cb[i].fn_data);
    }
    if (arl_sw_entry_count != arl_sw_entry_count_prev) {
        arl_sw_entry_count_prev = arl_sw_entry_count;
        LOG_INFO(BSL_LS_SOC_ARLMON,
                 (BSL_META_U(unit,
                             "ARL callback entry count %d\n"), 
                  arl_sw_entry_count));
    }

}

/****************************************************************************
 *
 * ARL Message Processing
 *
 ****************************************************************************/
extern int drv_robo_arlsync_cmp_info_build(int unit);

/*
 * Function:
 *  soc_arl_start (internal)
 * Purpose:
 *      Start ARL-related threads
 * Parameters:
 *  unit     - unit number.
 *  interval - time between resynchronization passes
 * Returns:
 *  BCM_E_MEMORY if can't create threads.
 */
STATIC int
soc_arl_start(int unit, sal_usecs_t interval)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    int index_count;
    void (*func)(void *);
#ifdef BCM_TB_SUPPORT
    int arl_mode;
#endif


    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "soc_arl_start: unit=%d interval=%d\n"),
              unit, interval));

    if (!soc_property_get(unit, "reg_write_log", FALSE)) {  
        sal_snprintf(soc->arl_name, sizeof(soc->arl_name), "bcmARL.%d", unit);
    }

    soc->arl_interval = interval;

    if (interval == 0) {
        return SOC_E_NONE;
    }

    if(soc->arl_table == NULL) {
        index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
        ARL_SW_TABLE_LOCK(soc);
        soc->arl_table = sal_alloc(index_count * sizeof (l2_arl_sw_entry_t),
            "arl_table");
        
        /* clear memory to 0 */
        sal_memset(&soc->arl_table[0], 0, sizeof (l2_arl_sw_entry_t) * index_count);
        ARL_SW_TABLE_UNLOCK(soc);
    }

    SOC_IF_ERROR_RETURN(drv_robo_arlsync_cmp_info_build(unit));
    
#ifdef BCM_TB_SUPPORT
    arl_mode = soc_property_get(unit, spn_L2XMSG_MODE, ARL_MODE_SEARCH_VALID);

    if (SOC_IS_TBX(unit) &&  !SOC_IS_TB_AX(unit)){
        arl_mode = soc_property_get(unit, spn_L2XMSG_MODE, ARL_MODE_SCAN_VALID);    
    }

    if (arl_mode == ARL_MODE_SCAN_VALID ){        
        func = soc_arl_thread_tb;
    } else 
#endif
    {
        func = soc_arl_thread;
    }

    if (soc->arl_pid == SAL_THREAD_ERROR) {
        soc->arl_pid = sal_thread_create(soc->arl_name,
                     SAL_THREAD_STKSZ,
                     50,
                     func,
                     INT_TO_PTR(unit));

        if (soc->arl_pid == SAL_THREAD_ERROR) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_arl_start: Could not start arl thread\n")));
            return SOC_E_MEMORY;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_arl_stop (internal)
 * Purpose:
 *  Stop ARL-related threads
 * Parameters:
 *  unit - unit number.
 * Returns:
 *  BCM_E_xxx
 */
STATIC int
soc_arl_stop(int unit)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int         rv = SOC_E_NONE;
    soc_timeout_t   to;

    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "soc_arl_stop: unit=%d\n"), unit));

    soc->arl_interval = 0;  /* Request exit */

    if (soc->arl_pid != SAL_THREAD_ERROR) {
        /* Wake up thread so it will check the exit flag */
        sal_sem_give(soc->arl_notify);

        /* Give thread a few seconds to wake up and exit */
        soc_timeout_init(&to, 5000000, 0);

        while (soc->arl_pid != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&to)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_arl_stop: thread will not exit\n")));
                rv = SOC_E_INTERNAL;
                break;
            }
        }
    }
    
    /* reset the L2 Frozen Sync status */
    soc_arl_frozen_sync_init(unit);

    if(soc->arl_table !=NULL) {
        ARL_SW_TABLE_LOCK(soc);
        sal_free(soc->arl_table);
        soc->arl_table = NULL;
        ARL_SW_TABLE_UNLOCK(soc);
    }

    return rv;
}

/* this_searched_index, frozen_search_status and frozen_action_l2_index are 
 *  the global variable to indicate the current ARL thread performed a 
 *  complete scan valid process during L2 frozen status.
 *
 *  Working Rule of 'this_searched_index' :
 *  1. this_searched_index can only be set in ARL thread to log the current
 *      operating seached valid L2 index.
 *  2. while L2 scan_valid operating SEARCH_START condition
 *      - this_searched_index = -1;
 *  3. For each valid entry GET, once 
 *      - this_searched_index = (this valid entry index)
 *
 *  Working Rule of 'frozen_search_status' and 'frozen_action_l2_index':
 *  1. while the SW ARL is inactive (sw_arl=NULL or interval=0):
 *      - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT
 *      - frozen_action_l2_index = -1
 *  2. L2 scan_valid operating SEARCH_START condition and the current 
 *      status is at SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT
 *      a. while L2 is frozen already : (this case might occurred when user
 *          forec L2 frozen and then restart ARL thread.)
 *          - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_START
 *          - frozen_action_l2_index = this_searched_index(-1 actually)
 *  3. while L2 is thawing or performing fast aging
 *      - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT
 *      - frozen_action_l2_index = -1
 *  4. while L2 is freezing from non frozen:
 *      - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_START
 *      - frozen_action_l2_index = this_searched_index
 *  5. while L2 scan_valid detects SEARCH_DONE condition and the current 
 *      status is at SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_START
 *      - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_CYCLE
 *  6. if (frozen_search_status == SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_CYCLE && 
 *      this_searched_index >= frozen_action_l2_index) : means sync already
 *      - frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_SYNC
 */
#define  SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT     0
#define  SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_START    1
#define  SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_CYCLE    2
#define  SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_SYNC     3

#define SOC_ROBO_ARL_FS_IS_INIT(_status)   \
            ((_status) == SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT)
#define SOC_ROBO_ARL_FS_IS_START(_status)   \
            ((_status) == SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_START)
#define SOC_ROBO_ARL_FS_IS_CYCLE(_status)   \
            ((_status) == SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_CYCLE)
#define SOC_ROBO_ARL_FS_IS_SYNC(_status)   \
            ((_status) == SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_SYNC)
            
static int this_searched_index = -1; 
static uint32 frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT;
/* frozen_action_l2_index:
 *  1. the first detected l2_index while L2 frozen sync is started.
 *  2. also used to report the sync count after status is SYNC
 */
static int frozen_action_l2_index = -1;

#ifdef BCM_TB_SUPPORT
/* frozen sync detect for scan valid feature :
 *  1. frozen_action_scan_index
 *      - To log the proceeding l2 index while L2 frozen is just detected 
 *          during Scan Valid operation.
 *      - Will be reset to -1 while L2 forzen status changed to thaw druing 
 *          Scan Valid operation.
 *      - Clear to 0 to be the Frozen Sync counter during Scan Valid operation
 *          and frozen_scan_status=SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_SYNC.
 *  2. frozen_scan_status : INIT -> START -> CYCLE -> SYNC
 *      - the working model is simular to 'frozen_search_status' in above.
 */
static int frozen_action_scan_index = -1;
static uint32 frozen_scan_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT;
#endif /* BCM_TB_SUPPORT */

/* Report the SYNC_STATUS.
 *  is_sync :   (OUT) TRUE/FALSE
 */
void 
soc_arl_frozen_sync_status(int unit, int *is_sync)
{
    if(SOC_ROBO_ARL_FS_IS_SYNC(frozen_search_status)) {
        *is_sync = TRUE;
    } else {
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)){
            *is_sync = (SOC_ROBO_ARL_FS_IS_SYNC(frozen_scan_status));
        }
#else /* BCM_TB_SUPPORT */
        *is_sync = FALSE;
#endif /* BCM_TB_SUPPORT */
    }
}

/* to reset sync status to INIT(0): 
 * >> three cases, no matter what the STATUS is 
 *  1. L2 not freezing 
 *  2. Fast aging
 *  3. SW_ARL inactive
 * >> Actions:
 *  - frozen_search_status = INIT
 *  - frozen_action_l2_index = -1
 */
void soc_arl_frozen_sync_init(int unit)
{
    frozen_search_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT;
    frozen_action_l2_index = -1;
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)){
        frozen_action_scan_index = -1;
        frozen_scan_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT;
    }
#endif /* BCM_TB_SUPPORT */
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "ARL FS status init!\n")));
}

/* _soc_arl_frozen_sync_status_update:
 *  - state machine on handling the L2 Frozen Sync status. 
 *
 *  Note:
 *  1. This routine can be called by ARL thread only.
 *      - Will be proceeded for the search op at {START, GET, DONE}
 */
STATIC void 
_soc_arl_frozen_sync_status_update(int unit, int search_op, int op_status)
{
    int     frozen = 0;
    
    soc_robo_arl_is_frozen(unit, &frozen);
    if (frozen) {
        if (SOC_ROBO_ARL_FS_IS_INIT(frozen_search_status)) {
            if (search_op == _ARL_SEARCH_VALID_OP_START || 
                    search_op == _ARL_SEARCH_VALID_OP_GET) {
                frozen_action_l2_index = this_searched_index;
                goto status_change;
            }
        } else if (SOC_ROBO_ARL_FS_IS_START(frozen_search_status)) {
            if (search_op == _ARL_SEARCH_VALID_OP_DONE && 
                    op_status == SOC_E_NONE){
                goto status_change;
            }
        } else if (SOC_ROBO_ARL_FS_IS_CYCLE(frozen_search_status)) {
            if (frozen_action_l2_index >= 0) {
                if (search_op == _ARL_SEARCH_VALID_OP_GET && 
                        op_status == SOC_E_EXISTS){
                    if (this_searched_index >= frozen_action_l2_index ){
                        frozen_action_l2_index = 0;
                        goto status_change;
                    }
                }
            } else {
                if (search_op == _ARL_SEARCH_VALID_OP_DONE && 
                        op_status == SOC_E_NONE){
                    frozen_action_l2_index = 0;
                    goto status_change;
                }
            }
        } else {
            /* means current status is SYNC :
             *  - increment the frozen_action_l2_index to log the SYNC count.
             */
            if (search_op == _ARL_SEARCH_VALID_OP_DONE && 
                        op_status == SOC_E_NONE){
                frozen_action_l2_index++ ;
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "%s,L2 FS SYNC count=%d\n"),
                          FUNCTION_NAME(), frozen_action_l2_index));
            }
        }
        return;
    } else {
        if (!SOC_ROBO_ARL_FS_IS_INIT(frozen_search_status)) {
            soc_arl_frozen_sync_init(unit);
        }
        return;
    }
    
status_change:
    frozen_search_status++;   
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "%s,L2 FS status set to %d!\n"), 
              FUNCTION_NAME(), frozen_search_status));
    return;   
}

#ifdef BCM_TB_SUPPORT
/* frozen sync status update on TBX devices(with Scan Valid feature) */
STATIC void 
_soc_arl_frozen_sync_status_update_tb(int unit, int scan_index)
{
    int     frozen = 0;
    int     max_index = (SOC_MEM_SIZE(unit, INDEX(L2_ARLm))*2/TB_ARL_SCAN_VALID_SIZE) - 1;
    
    soc_robo_arl_is_frozen(unit, &frozen);
    if (frozen) {
        if (SOC_ROBO_ARL_FS_IS_INIT(frozen_scan_status)) {
            frozen_action_scan_index = scan_index;
            goto status_change_tb;
        } else if (SOC_ROBO_ARL_FS_IS_START(frozen_scan_status)) {
            if (scan_index == max_index){
                goto status_change_tb;
            }
        } else if (SOC_ROBO_ARL_FS_IS_CYCLE(frozen_scan_status)) {
            if (scan_index > frozen_action_scan_index) {
                frozen_action_scan_index = 0;
                goto status_change_tb;
            }
        } else {
            /* means current status is SYNC :
             *  - increment the frozen_action_l2_index to log the SYNC count.
             */
            if (scan_index == max_index){
                frozen_action_scan_index++ ;
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "%s,L2 FS SYNC(Scan Valid) count=%d\n"),
                          FUNCTION_NAME(), frozen_action_scan_index));
            }
        }
        return;
    } else {
        if (!SOC_ROBO_ARL_FS_IS_INIT(frozen_scan_status)) {
            /* don't call soc_arl_frozen_sync_init() to prevent the unexpected 
             *  INIT process on 'frozen_search_status'(maintained by 
             *  search valid operation.
             */
            frozen_action_scan_index = -1;
            frozen_scan_status = SOC_ROBO_ARL_FROZEN_SEARCH_STATUS_INIT;
        }
        return;
    }
    
status_change_tb:
    frozen_scan_status++;   
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "%s,L2 FS status(Scan Valid) set to %d!\n"), 
              FUNCTION_NAME(), frozen_scan_status));
    return;   
}
#endif  /* BCM_TB_SUPPORT */

int soc_arl_scan_valid(int unit, int index, SHR_BITDCL *valid_list)
{
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)){
    
        /* frozen sync status update special for scan valid feature  */
        _soc_arl_frozen_sync_status_update_tb(unit, index);
    
        return _tb_arl_scan_valid(unit, index, valid_list);
    }
#endif
    return SOC_E_UNAVAIL;
}


int
soc_arl_search_valid(int unit, int op, void *index, void *entry, void *entry1)
{
    uint32 flags;
    int rv = SOC_E_PARAM;

    if (op == _ARL_SEARCH_VALID_OP_START) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "_ARL_SEARCH_VALID_OP_START \n")));
        flags = DRV_MEM_OP_SEARCH_VALID_START;
        rv = DRV_MEM_SEARCH
            (unit, DRV_MEM_ARL, NULL, NULL, NULL, flags);
        
        this_searched_index = -1;
        _soc_arl_frozen_sync_status_update(unit, op, rv);
        SOC_IF_ERROR_RETURN(rv);
    }

    if (op == _ARL_SEARCH_VALID_OP_NEXT) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "_ARL_SEARCH_VALID_OP_NEXT \n")));

        if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)){
            flags = DRV_MEM_OP_SEARCH_VALID_START | DRV_MEM_OP_SEARCH_VALID_GET;
            rv = DRV_MEM_SEARCH
                (unit, DRV_MEM_ARL, NULL, NULL, NULL, flags);
        } else {
            rv =SOC_E_NONE;
        }
        SOC_IF_ERROR_RETURN(rv);
    }

    if (op == _ARL_SEARCH_VALID_OP_GET) {        
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "_ARL_SEARCH_VALID_OP_GET \n")));   

        flags = DRV_MEM_OP_SEARCH_VALID_GET;
        rv  = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)index, 
                (uint32 *)entry, (uint32 *)entry1, flags);
        if (rv == SOC_E_EXISTS){
            this_searched_index = (int)(*(uint32 *)index);
        }
        _soc_arl_frozen_sync_status_update(unit, op, rv);
        SOC_IF_ERROR_RETURN(rv);
    }
    if (op == _ARL_SEARCH_VALID_OP_DONE) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "_ARL_SEARCH_VALID_OP_DONE \n")));           
        flags = DRV_MEM_OP_SEARCH_DONE;
        rv  = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, NULL, NULL, NULL, flags);
        _soc_arl_frozen_sync_status_update(unit, op, rv);
        SOC_IF_ERROR_RETURN(rv);
    }

    return rv;
}

/*
 * Function:
 *  soc_arl_thread (internal)
 * Purpose:
 *      Thread control for ARL message handling.
 * Parameters:
 *  unit_vp - StrataSwitch unit # (asa  void *).
 * Returns:
 *  Nothing
 * Notes:
 *  Exits when soc->arl_exit flag is set and semaphore is given.
 *  The table is processed one chunk at a time (spn_L2XMSG_CHUNKS chunks
 *  total) in order to reduce memory requirements for the temporary
 *  DMA buffer and to give the CPU a break from bursts of activity.
 */
STATIC void 
soc_arl_thread(void *unit_vp)
{
    int             unit = PTR_TO_INT(unit_vp);
    int             rv = SOC_E_NONE;
    int             index_min = 0, index_count = 0;
    int             interval = 0, sw_learn_cnt = 0;
    SHR_BITDCL      *exist_list, *current_list;
    uint32          index = 0, idx = 0;
    uint32          valid = 0;
    soc_control_t       *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t   *old_tab = NULL;
    l2_arl_sw_entry_t   sw_arl, null_entry, sw_arl1;
    int			chunk_count, chunk_size;
    int			bucket_index;
    int     valid_count = 0, default_count = 0;
    int         sync = 0;
#ifdef BCM_DINO16_SUPPORT
    uint32      flags = 0;
    l2_arl_sw_entry_t   output;
#endif /* BCM_DINO16_SUPPORT */

    index_min = SOC_MEM_BASE(unit, INDEX(L2_ARLm));
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));

    old_tab = (l2_arl_sw_entry_t *)(soc->arl_table);

    exist_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_exist_list");
    current_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_current_list");
    delete_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_delete_list");
    added_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_added_list");

    if ((exist_list == NULL) || (current_list == NULL) || 
        (delete_list == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_arl_thread: not enough memory, exiting\n")));
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                SOC_SWITCH_EVENT_THREAD_L2X, __LINE__, SOC_E_MEMORY);
        goto cleanup_exit;
    }

    /*
     * Start with initially empty shadow table.
     */
    sal_memset(old_tab, 0, index_count * sizeof (l2_arl_sw_entry_t));

    sal_memset(exist_list, 0, SHR_BITALLOCSIZE(index_count));
    sal_memset(current_list, 0, SHR_BITALLOCSIZE(index_count));
    sal_memset(delete_list, 0, SHR_BITALLOCSIZE(index_count));
    sal_memset(added_list, 0, SHR_BITALLOCSIZE(index_count));

    chunk_count = soc_property_get(unit, spn_L2XMSG_CHUNKS, 8);
    chunk_size = index_count / chunk_count;
    idx = index_min;
    sw_learn_cnt = 0;

    /* reset the L2 Frozen Sync status */
    soc_arl_frozen_sync_init(unit);

    while ((interval = soc->arl_interval / chunk_count) != 0) {
        rv =  soc_arl_search_valid(unit, 
            _ARL_SEARCH_VALID_OP_DONE, NULL, NULL, NULL);
        if (rv == SOC_E_NONE) {
            /* Sync software database */
            sal_memset(&sw_arl, 0, sizeof(l2_arl_sw_entry_t));
            sal_memset(&null_entry, 0, sizeof(l2_arl_sw_entry_t));                 

            for (bucket_index = 0; bucket_index < chunk_size; 
                            bucket_index ++) {
                /* Update the aged out and deleted entries */        

                if (SHR_BITGET(exist_list, idx) && 
                    !SHR_BITGET(current_list, idx)) {
                    DRV_ARL_SYNC(unit, (uint32 *)&idx, 
                        (l2_arl_sw_entry_t *)&old_tab[idx],
                        (l2_arl_sw_entry_t *)&sw_arl);

                    /* SDK-29989: 
                     *  - prevent the redundant arl_sync() been performed.
                     */
                    if (SHR_BITGET(delete_list, idx)){
                        SHR_BITCLR(delete_list, idx);
                    }
                }
                if (SHR_BITGET(delete_list, idx) && 
                    SHR_BITGET(current_list, idx)) {
                    DRV_ARL_SYNC(unit, (uint32 *)&idx, 
                        (l2_arl_sw_entry_t *)&old_tab[idx],
                        (l2_arl_sw_entry_t *)&null_entry);
                    SHR_BITCLR(delete_list, idx);

                    /* SDK-29989: 
                     *  - prevent the redundant arl_sync() been performed.
                     */
                    SHR_BITCLR(current_list, idx);
                }

                if (SHR_BITGET(current_list, idx)){
                    SHR_BITSET(exist_list, idx);
                } else {
                    SHR_BITCLR(exist_list, idx);
                }

                /* To fix the SDK-37297 about the ARL sync issue while 
                 *  fast-aging before the SW/HW ARL is syncronized.
                 */
                ARL_SW_TABLE_LOCK(soc);
                if (SHR_BITGET(added_list, idx)){
                    /* update the exist list for those entry bit on user   
                     * inserted already.
                     */
                    SHR_BITSET(exist_list, idx);
                    SHR_BITCLR(added_list, idx);
                }
                ARL_SW_TABLE_UNLOCK(soc);
                
                SHR_BITCLR(current_list, idx);
                if((++idx) >= index_count){
                    idx = index_min;
                }
            }

            VO_ARL_SEARCH_LOCK(unit,soc);

            rv =  soc_arl_search_valid(unit, 
                _ARL_SEARCH_VALID_OP_START, NULL, NULL, NULL);
            if(SOC_FAILURE(rv)){
                VO_ARL_SEARCH_UNLOCK(unit,soc);
                goto cleanup_exit;
            }
        } else if (rv == SOC_E_BUSY){
            VO_ARL_SEARCH_LOCK(unit,soc);

            rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                        NULL, NULL, NULL);
            if(SOC_FAILURE(rv)){
                VO_ARL_SEARCH_UNLOCK(unit,soc);
                goto cleanup_exit;
            }
        }
        /* defaul valid_count ideally is one chunk per second */
        default_count = (interval*chunk_size + ONESEC - 1)/ONESEC;
        valid_count = soc_property_get(unit, spn_ARL_RATE_LIMIT, default_count);
        LOG_VERBOSE(BSL_LS_SOC_ARL,
                    (BSL_META_U(unit,
                                "soc_arl_thread: valid_count %d \n"), valid_count));

        /* check the interval to prevent the timeout condition once the arl 
         * thead is going to be restarted or stopped.
         */
        while (valid_count && (soc->arl_interval > 0)) {
            sal_memset(&sw_arl, 0, sizeof(l2_arl_sw_entry_t));
            sal_memset(&sw_arl1, 0, sizeof(l2_arl_sw_entry_t));

            rv =  soc_arl_search_valid(unit, 
                _ARL_SEARCH_VALID_OP_GET, (uint32 *)&index, 
                (uint32 *)&sw_arl, (uint32 *)&sw_arl1);
            VO_ARL_SEARCH_UNLOCK(unit,soc);

            if (rv != SOC_E_EXISTS){
                if (rv != SOC_E_FAIL){
                    break;
                } else {
                    if (SOC_IS_TBX(unit)) {
                        /* rv=SOC_E_FAIL means searched valid but failed on  
                        * valid bit double check operation.
                        *
                        * The previous search valid op befor GET flow is 
                        *  >> Checked-Done -> OP-Started
                        *
                        * Fix : one more NEXT operation.
                        *
                        * Note : 
                        *  1. TB_A0 need special process to launch real NEXT 
                        *       action due to the formal NEXT operation in  
                        *       soc_arl_search_valid() is for TBX devices but 
                        *       is excluded for TB_A0. 
                        *  2. Formal NEXT op will be proceeded later in this 
                        *       while loop.
                        */
                        LOG_INFO(BSL_LS_SOC_ARL,
                                 (BSL_META_U(unit,
                                             "%s, valid bit double check mismatched!\n\t"\
                                             ">> Performinging NEXT op..(valid_cnt=%d)\n\t"),
                                  FUNCTION_NAME(), valid_count));
                        if (SOC_IS_TB_AX(unit)){
                            uint64  temp_data;

                            COMPILER_64_ZERO(temp_data);
                            /* TB_A0 to proceed NEXT op 
                             *
                             *  Dummy read on MEM_SRCH_KEY_2r to launch NEXT 
                             *  action in ARL search valid process.
                             */
                            rv = REG_READ_MEM_SRCH_KEY_2r(unit, &temp_data);
                            if (SOC_FAILURE(rv)){
                                goto cleanup_exit;
                            }
                        }
                    } else {
                        /* for non-TBX device got SOC_E_FAIL return */
                        break;
                    }
                }
            } else {
                if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO16(unit)) {
                    /* Search valid mechanism return 2 entries each time */
                    index = index * 2;
                }
                rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID,
                    (uint32 *)&sw_arl, &valid);
                if(SOC_FAILURE(rv)){
                    goto cleanup_exit;
                }
                    
#ifdef  BCM_TB_SUPPORT
                /* TB devices was designed to report the valid search result by 
                 *  one arl entry in each searched time. And the entry status may 
                 *  be with valid or pending status. Both status were treat as 
                 *  valid entry in ARL thread in current design.
                 */
                if (SOC_IS_TBX(unit)){
                    valid = (valid ==_TB_ARL_STATUS_PENDING || 
                            valid ==_TB_ARL_STATUS_VALID) ? 1 : 0;
                }
#endif  /* BCM_TB_SUPPORT */
                if (valid) {
                    SHR_BITSET(current_list, index);
                    ARL_SW_TABLE_LOCK(soc);
                    if (SHR_BITGET(added_list, index)) {
                        SHR_BITCLR(added_list, index);
                    }
                    ARL_SW_TABLE_UNLOCK(soc);
                    
                    /* If deletion is happen, issue arl callback first */
                    if (SHR_BITGET(delete_list, index)) {
                        /* Issue ARL callback first */
                        rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                             (l2_arl_sw_entry_t *)&old_tab[index],
                               (l2_arl_sw_entry_t *)&null_entry);
                        if(SOC_FAILURE(rv)){
                            goto cleanup_exit;
                        }
                        /* Clear delete and exist bit */
                        SHR_BITCLR(delete_list, index);
                        SHR_BITCLR(exist_list, index);
                    } else {
                        sync = 1;
#ifdef BCM_DINO16_SUPPORT
                        /* bcm5396 sw workaround
                          * ARL search valid might get entry 
                          * which did not exist in hw table.
                          * Check the entry again before sync
                          * to sw table */
                        if (SOC_IS_DINO16(unit)) {
                            flags = DRV_MEM_OP_BY_HASH_BY_MAC | 
                                    DRV_MEM_OP_BY_HASH_BY_VLANID;
                            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                            rv  = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, 
                                    (uint32 *)&sw_arl, (uint32 *)&output, 
                                    NULL, flags);
                            if (rv == SOC_E_NOT_FOUND) {
                                sync = 0;
                            }
                        }
#endif /* BCM_DINO16_SUPPORT */
                        /* Sync entry */
                        if (sync) {
                            /* sync for add or modify process */
                            rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                                    (l2_arl_sw_entry_t *)&old_tab[index],
                                    (l2_arl_sw_entry_t *)&sw_arl);
                            if (SOC_FAILURE(rv)) {
                                goto cleanup_exit;
                            }
    
                            /* SDK-29989: 
                             *  - prevent the redundant arl_sync().
                             */
                            if (!SHR_BITGET(exist_list,index)) {
                                SHR_BITSET(exist_list, index);
                            }
                        }
                    }
                }
                
                /* Deal the second search entry */
                if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO16(unit)) {
                    rv = DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID,
                        (uint32 *)&sw_arl1, &valid);
                    if (SOC_FAILURE(rv)){
                       goto cleanup_exit;
                    }

                    if (valid) {
                        index = index + 1;
                        SHR_BITSET(current_list, index);
                        ARL_SW_TABLE_LOCK(soc);
                        if (SHR_BITGET(added_list, index)) {
                            SHR_BITCLR(added_list, index);
                        }
                        ARL_SW_TABLE_UNLOCK(soc);
                        
                        /* If deletion is happen, issue arl callback first */
                        if (SHR_BITGET(delete_list, index)) {
                            /* Issue ARL callback first */
                            rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                                (l2_arl_sw_entry_t *)&old_tab[index],
                                   (l2_arl_sw_entry_t *)&null_entry);
                            if (SOC_FAILURE(rv)){
                                   goto cleanup_exit;
                            }
                            /* Clear delete and exist bit */
                            SHR_BITCLR(delete_list, index);
                            SHR_BITCLR(exist_list, index);
                        } else {
                            sync = 1;
#ifdef BCM_DINO16_SUPPORT
                            /* bcm5396 sw workaround
                              * ARL search valid might get entry 
                              * which did not exist in hw table.
                              * Check the entry again before sync
                              * to sw table */
                            if (SOC_IS_DINO16(unit)) {
                                flags = DRV_MEM_OP_BY_HASH_BY_MAC | 
                                        DRV_MEM_OP_BY_HASH_BY_VLANID;
                                sal_memset(&output, 0, 
                                    sizeof(l2_arl_sw_entry_t));
                                rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, 
                                        (uint32 *)&sw_arl1, (uint32 *)&output, 
                                        NULL, flags);
                                if (rv == SOC_E_NOT_FOUND) {
                                    sync = 0;
                                }
                            }
#endif /* BCM_DINO16_SUPPORT */
                            /* Sync entry */
                            if (sync) {
                                rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                                     (l2_arl_sw_entry_t *)&old_tab[index],
                                       (l2_arl_sw_entry_t *)&sw_arl1);
                                if (SOC_FAILURE(rv)) {
                                    goto cleanup_exit;
                                }
    
    
                                /* SDK-29989: 
                                 *  - prevent the redundant arl_sync().
                                 */
                                if (!SHR_BITGET(exist_list,index)) {
                                    SHR_BITSET(exist_list, index);
                                }
                            }
                        }
                    }
                }
            }
            
            valid_count-- ;
            if(valid_count) {
                VO_ARL_SEARCH_LOCK(unit,soc);

                rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                        NULL, NULL, NULL);
                if (SOC_FAILURE(rv)){
                    VO_ARL_SEARCH_UNLOCK(unit,soc);

                    goto cleanup_exit;
                }

            }

        }
        if (soc->arl_interval > 0) {
        /*
         * Implement the sleep using a semaphore timeout so if the task
         * is requested to exit, it can do so immediately.
         */
            /* update the latest SW ARL count */
            SHR_BITCOUNT_RANGE(exist_list, 
                    sw_learn_cnt, index_min, index_count);

            LOG_VERBOSE(BSL_LS_SOC_ARL,
                        (BSL_META_U(unit,
                                    "%s: SW ARL entry_cnt=%d, SLEEP..\n"),
                         FUNCTION_NAME(), sw_learn_cnt));
            sal_sem_take(soc->arl_notify, interval);
        }
    }

cleanup_exit:
    if (((soc->arl_exit) & _ROBO_SEARCH_LOCK) == 0) {
        VO_ARL_SEARCH_UNLOCK(unit,soc);
    }

    if (exist_list != NULL) {
        sal_free(exist_list);
    }
    if (current_list  != NULL) {
        sal_free(current_list);
    }
    if (delete_list  != NULL) {
        sal_free(delete_list);
    }
    if (added_list  != NULL) {
        sal_free(added_list);
    }

    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "soc_arl_thread: exiting\n")));

    soc->arl_pid = SAL_THREAD_ERROR;
    sal_thread_exit(0);
}

#ifdef BCM_TB_SUPPORT

/* the internal routine to count the SW existed l2 entry count on TB_B0.
 *  - the valid scan OP in TB_B0 reports the entry valid status bye 2 bits.
 *    the b00 is empty; b01 is pending; b11 is valid. (b10 is reserved)
 *
 *  >> pending and valid entry will be counted. (2nd bit can be ignored)
 */
STATIC void
_tb_b0_exist_list_count(SHR_BITDCL *bits, int first, int range, int *count)
{
    int i, temp_cnt = 0;

    for (i = first; i < (first + range); i+=2){
        if (SHR_BITGET(bits, i)){
            temp_cnt++;
        }
    }

    *count = temp_cnt;
}

STATIC void 
soc_arl_thread_tb(void *unit_vp)
{
    int unit = PTR_TO_INT(unit_vp);
    int i, scan_index, max_scan_index, addr_index, chunk, chunk_size;
    SHR_BITDCL  *exist_list;
    SHR_BITDCL  *valid_list, *result_list, *temp_list, scan_valid_diff;
    int scan_valid_chunk_size;
    int index_count = 0, index, sw_learn_cnt;
    int interval = 0, idx, rv;
    soc_control_t *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t   *old_tab = NULL;
    l2_arl_sw_entry_t   sw_arl;    
    int valid_count = 0, default_count = 0;
    int added_cnt = 0;
    
    old_tab = (l2_arl_sw_entry_t *)(soc->arl_table);

    /* 2 bits per entry {VALID, PENDING}*/
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm))*2;

    chunk = soc_property_get(unit, spn_L2XMSG_CHUNKS, 8);
    chunk_size = index_count / chunk;
    scan_valid_chunk_size = chunk_size /TB_ARL_SCAN_VALID_SIZE;
    scan_index = 0;
    max_scan_index = index_count/TB_ARL_SCAN_VALID_SIZE;

    exist_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_exist_list");
    valid_list = sal_alloc(SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE),"arl_valid_list");
    result_list = sal_alloc(SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE),"arl_result_list");        
    temp_list = sal_alloc(SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE),"arl_temp_list");        
    added_list = sal_alloc(SHR_BITALLOCSIZE(index_count), "arl_added_list");

    if ((exist_list == NULL) || (valid_list == NULL) || 
        (valid_list == NULL) || (temp_list == NULL) || 
        (added_list == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_arl_thread_tb: not enough memory, exiting\n")));
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                SOC_SWITCH_EVENT_THREAD_L2X, __LINE__, SOC_E_MEMORY);
        goto cleanup_exit;
    }
    /*
     * Start with initially empty shadow table.
     */
    ARL_SW_TABLE_LOCK(soc);
    sal_memset(old_tab, 0, SOC_MEM_SIZE(unit, INDEX(L2_ARLm)) * sizeof (l2_arl_sw_entry_t));
    ARL_SW_TABLE_UNLOCK(soc);
    
    sal_memset(exist_list, 0, SHR_BITALLOCSIZE(index_count));
    sal_memset(valid_list, 0, SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE));
    sal_memset(result_list, 0, SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE));
    sal_memset(temp_list, 0, SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE));
    sal_memset(added_list, 0, SHR_BITALLOCSIZE(index_count));

    sw_learn_cnt = 0;
    scan_valid_diff = 0;

    /* reset the L2 Frozen Sync status */
    soc_arl_frozen_sync_init(unit);

    while ((interval = (soc->arl_interval / chunk)) != 0) {

        /* 
         * Issue arl scan valid operation 
         * update one chunk (2k entries) 
         */
        for (i = 0; i < scan_valid_chunk_size; i++){        
            if (soc->arl_interval == 0){
                break;
            }   

            /* get valid_list */
            TBX_ARL_SCAN_LOCK(unit,soc);
            rv = soc_arl_scan_valid(unit, scan_index, valid_list);
            TBX_ARL_SCAN_UNLOCK(unit,soc);

            if (SOC_FAILURE(rv)){
                goto cleanup_exit;
            }
            addr_index = scan_index * TB_ARL_SCAN_VALID_SIZE;

            /* To fix the SDK-37297 about the ARL sync issue while 
             *  fast-aging before the SW/HW ARL is syncronized.
             */
            ARL_SW_TABLE_LOCK(soc);
            added_cnt = 0;
            SHR_BITCOUNT_RANGE(&added_list[_SHR_BITDCLSIZE(addr_index)], 
                    added_cnt, 0, TB_ARL_SCAN_VALID_SIZE);
            if (added_cnt > 0) {
                SHR_BITOR_RANGE(&exist_list[_SHR_BITDCLSIZE(addr_index)], 
                        &added_list[_SHR_BITDCLSIZE(addr_index)], 
                        0, TB_ARL_SCAN_VALID_SIZE, 
                        &exist_list[_SHR_BITDCLSIZE(addr_index)]);
                SHR_BITCLR_RANGE(&added_list[_SHR_BITDCLSIZE(addr_index)], 
                        0, TB_ARL_SCAN_VALID_SIZE);
            }
            ARL_SW_TABLE_UNLOCK(soc);

            sal_memcpy(temp_list, &exist_list[_SHR_BITDCLSIZE(addr_index)],
                SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE));
                
            /* issue the ADD/DELETE callback */                
            /* XOR the exist_list and valid_list to find the difference */
            SHR_BITXOR_RANGE(temp_list, valid_list, 0, 
                TB_ARL_SCAN_VALID_SIZE, result_list);

            SHR_BITTEST_RANGE(result_list, 0, TB_ARL_SCAN_VALID_SIZE, scan_valid_diff);
            if (soc->arl_interval == 0){
                break;
            }           
            if(scan_valid_diff){
                /* iterate the difference */
                TB_ARL_SCAN_VALID_ITER(result_list, idx, TB_ARL_SCAN_VALID_SIZE){
                    if (soc->arl_interval == 0){
                        break;
                    }

                    if(SHR_BITGET(valid_list, idx)){                    
                        index = (addr_index + idx)/2;
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "add idx %d addr_index %d index %d\n"),
                                     idx, addr_index, index));
                        rv = MEM_READ_L2_ARL_SWm(unit, index, (uint32 *)&sw_arl);
                        if (SOC_FAILURE(rv)){
                            goto cleanup_exit;
                        }
                                                
                        /* ADD callback */
                        rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                                (l2_arl_sw_entry_t *)&old_tab[index],
                                (l2_arl_sw_entry_t *)&sw_arl);
                        if (SOC_FAILURE(rv)){
                            goto cleanup_exit;
                        }
                    } else {
                        sal_memset(&sw_arl, 0, sizeof(l2_arl_sw_entry_t));
                        index = (addr_index + idx)/2;
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "delete idx %d addr_index %d index %d\n"),
                                     idx, addr_index, index));
                        /* DELETE callback */
                        rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                                (l2_arl_sw_entry_t *)&old_tab[index],
                                (l2_arl_sw_entry_t *)&sw_arl);
                        if (SOC_FAILURE(rv)){
                            goto cleanup_exit;
                        }
                    }
                }                
            }
            /* sync software database*/
            /* copy valid_list to exist_list */
            sal_memcpy(&exist_list[_SHR_BITDCLSIZE(addr_index)], valid_list, 
                SHR_BITALLOCSIZE(TB_ARL_SCAN_VALID_SIZE));
                            
            if(++scan_index >= max_scan_index){
                scan_index = 0;
                if (LOG_CHECK(BSL_LS_SOC_ARL | BSL_INFO)){
                    shr_bitop_range_count(exist_list,0, index_count,&valid_count);
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Total valid entry count %d \n"),
                                 valid_count/2));
                }
            }
        }

        if (soc->arl_interval == 0) {
            break;
        }

        /* 
         *  Search Valid part
         */
        /* defaul valid_count ideally is one chunk per second */
        default_count = ((interval*chunk_size + ONESEC - 1)/ONESEC)/2;
        valid_count = soc_property_get(unit, spn_ARL_RATE_LIMIT, default_count);
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_arl_thread_tb: valid_count %d \n"),
                   valid_count));


        VO_ARL_SEARCH_LOCK(unit,soc);

        rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_DONE, 
                NULL, NULL, NULL);

        if (SOC_SUCCESS(rv)) {
            rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_START,
                    NULL, NULL, NULL);
            if (SOC_FAILURE(rv)){
                VO_ARL_SEARCH_UNLOCK(unit,soc);
                goto cleanup_exit;
            }

        } else {
            if (rv == SOC_E_BUSY) {
                rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                        NULL, NULL, NULL);
                if (SOC_FAILURE(rv)){
                    VO_ARL_SEARCH_UNLOCK(unit,soc);
                    goto cleanup_exit;
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Error in search valid !!\n")));
                goto cleanup_exit;
            }
        }

        if (soc->arl_interval == 0) {
            break;
        }
        /* check the interval to prevent the timeout condition once the arl 
         * thead is going to be restarted or stopped.
         */
        while (valid_count && (soc->arl_interval > 0)) {
            sal_memset(&sw_arl, 0, sizeof(l2_arl_sw_entry_t));
            rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_GET,
                    &index, &sw_arl, NULL);

            VO_ARL_SEARCH_UNLOCK(unit,soc);
            if (rv == SOC_E_EXISTS) {
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "search valid index %d\n"),
                           index));
                rv = DRV_ARL_SYNC(unit, (uint32 *)&index, 
                        (l2_arl_sw_entry_t *)&old_tab[index],
                        (l2_arl_sw_entry_t *)&sw_arl);
    
                if (SOC_FAILURE(rv)){
                    goto cleanup_exit;
                }
    
                if (!SHR_BITGET(exist_list, index*2)) {
                    /* new entry*/
                    SHR_BITSET(exist_list, index*2);
                    SHR_BITSET(exist_list, index*2+1);            
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "search valid find new index %d\n"),
                                 index));
                }
    
                valid_count--;
            } else {
                if (rv != SOC_E_FAIL){
                    /* exit this loop if not {SOC_E_EXIST or SOC_E_FAIL} */ 
                    break;
                } else {
                    /* rv at SOC_E_FAIL means search valid but failed on  
                     * valid bit double check operation.
                     *
                     * The previous search valid op befor GET flow is 
                     *  >> Checked-Done -> OP-Started
                     *
                     * Fix : one more NEXT operation.
                     */
                    LOG_INFO(BSL_LS_SOC_ARL,
                             (BSL_META_U(unit,
                                         "%s, valid bit double check mismatched!\n\t"\
                                         ">> Performinging NEXT op..(valid_cnt=%d)\n\t"),
                              FUNCTION_NAME(), valid_count));
                }
            }

            if(valid_count && (soc->arl_interval > 0)) {
                VO_ARL_SEARCH_LOCK(unit,soc);
                rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                        NULL, NULL, NULL);
                if (SOC_FAILURE(rv)){
                    VO_ARL_SEARCH_UNLOCK(unit,soc);
                    goto cleanup_exit;
                }

            }
        }

        if (soc->arl_interval > 0) {
            /* update the latest SW ARL count */
            _tb_b0_exist_list_count(exist_list, 
                0, index_count, &sw_learn_cnt);

            /*
             * Implement the sleep using a semaphore timeout so if the task
             * is requested to exit, it can do so immediately.
             */
            
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                     "%s: SW ARL entry_cnt=%d, SLEEP..\n"),
                         FUNCTION_NAME(), sw_learn_cnt));
            sal_sem_take(soc->arl_notify, interval);
        }
    }
cleanup_exit:
    scan_valid_diff = 0;

    if (((soc->arl_exit) & _ROBO_SEARCH_LOCK) == 0) {
        VO_ARL_SEARCH_UNLOCK(unit,soc);
    }

    if (((soc->arl_exit) & _ROBO_SCAN_LOCK) == 0) {
        TBX_ARL_SCAN_UNLOCK(unit,soc);
    }


    if (exist_list != NULL) {
        sal_free(exist_list);
    }
    if (valid_list  != NULL) {
        sal_free(valid_list);
    }
    if (result_list  != NULL) {
        sal_free(result_list);
    }
    if (temp_list  != NULL) {
        sal_free(temp_list);
    }
    if (added_list  != NULL) {
        sal_free(added_list);
    }

    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "soc_arl_thread_tb: exiting\n")));

    soc->arl_pid = SAL_THREAD_ERROR;
    sal_thread_exit(0);
}
#endif

/*
 * Function:
 *  soc_robo_arl_mode_set
 * Purpose:  
 *  Configure ARL message processing mode.
 * Parameters:
 *  unit - unit number.
 *  mode - one of ARL_MODE_xxx
 * Returns:
 *  SOC_E_xxx
 * Notes:
 *  This starts and stops the ARL message thread.
 *  The ARL thread runs when the mode is not ARL_MODE_NONE.
 */

int
soc_robo_arl_mode_set(int unit, int mode)
{
    /*
     * Stop existing thread.
     * Set new mode and restart thread if necessary.
     */

    sal_usleep(MILLISECOND_USEC);

    SOC_IF_ERROR_RETURN(soc_arl_stop(unit));

    SOC_CONTROL(unit)->arlMode = mode;

    switch (mode) {
    case ARL_MODE_NONE:
        break;
    case ARL_MODE_ROBO_POLL:
    case ARL_MODE_SCAN_VALID:
        SOC_IF_ERROR_RETURN(soc_arl_start(unit, ARL_ROBO_POLL_INTERVAL));
        break;
    default:
        assert(0);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_robo_arl_mode_get
 * Purpose:  
 *  Return currently configured ARL mode (ARL_MODE_xxx)
 * Parameters:
 *  unit - unit number.
 *  mode - (OUT) current mode.
 * Returns:
 *  SOC_E_xxx
 */

int
soc_robo_arl_mode_get(int unit, int *mode)
{
    *mode = SOC_CONTROL(unit)->arlMode;

    return SOC_E_NONE;
}


/*
 * Function:
 *  soc_arl_database_dump
 * Purpose:  
 *  Dump an ARL entry from software shadow table
 * Parameters:
 *  unit - unit number.
 *    index - the entry index number
 *  entry - the pointer of the entry buffer
 * Returns:
 *  SOC_E_NONE
 * Notes:
 */

int
soc_arl_database_dump(int unit, uint32 index, l2_arl_sw_entry_t *entry)
{
    soc_control_t           *soc = SOC_CONTROL(unit);
    
    if (&(soc->arl_table[index]) != NULL) {
        sal_memcpy(entry, &(soc->arl_table[index]), sizeof(l2_arl_sw_entry_t));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_arl_database_delete
 * Purpose:  
 *  Set a delete bit to the corresponding ARL shadow table
 * Parameters:
 *  unit - unit number.
 *    index - the entry index number
 * Returns:
 *  SOC_E_NONE
 * Notes:
 */

int
soc_arl_database_delete(int unit, uint32 index)
{ 

    if (delete_list != NULL) {
        SHR_BITSET(delete_list, index);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_arl_database_add
 * Purpose:  
 *  Set the bit to represent the l2 inserted action on ARL shadow table
 * Parameters:
 *  unit - unit number.
 *    index - the entry index number
 * Returns:
 *  SOC_E_NONE
 * Notes:
 *  1. Special design for on 'index' by using bit 31 to indicate the 
 *      pending/valid status.
 *      >> bit31 : 0-valid; 1-pending
 *  2. for TBX scan valid feature support. There are two bits in added_list 
 *      to represent on entry status. 
 *  3. value of the valid status for TB's scan valid
 *      - b00 is empty; b01 is pending; b11 is valid. (b10 is reserved)
 */

int
soc_arl_database_add(int unit, uint32 index, int pending)
{ 
    soc_control_t *soc = SOC_CONTROL(unit);
    
#ifdef BCM_TB_SUPPORT
    int is_pending = 0, tb_index = 0, arl_mode;
    
    if (SOC_IS_TBX(unit) &&  !SOC_IS_TB_AX(unit)){
        arl_mode = soc_property_get(unit, spn_L2XMSG_MODE, 
                ARL_MODE_SCAN_VALID);    
    } else {
        arl_mode = soc_property_get(unit, spn_L2XMSG_MODE, 
                ARL_MODE_SEARCH_VALID);
    }

    if (arl_mode == ARL_MODE_SCAN_VALID ){        
        is_pending = (pending) ? TRUE : FALSE;
        
        tb_index = index * 2;
        if (added_list != NULL) {
            ARL_SW_TABLE_LOCK(soc);
            SHR_BITSET(added_list, tb_index);
            tb_index++;
            if (is_pending) {
                SHR_BITCLR(added_list, tb_index);
            } else {
                SHR_BITSET(added_list, tb_index);
            }
            ARL_SW_TABLE_UNLOCK(soc);
        }
    } else 
#endif
    {
        if (added_list != NULL) {
            ARL_SW_TABLE_LOCK(soc);
            SHR_BITSET(added_list, index);
            ARL_SW_TABLE_UNLOCK(soc);
        }
    }

    return SOC_E_NONE;
}



