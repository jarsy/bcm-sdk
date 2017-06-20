/*
 * $Id: taps.c,v 1.96.6.1.6.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    taps.c
 * Purpose: Caladan3 TAPS driver
 * Requires:
 */
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>


#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/taps/tcam.h>
#include <soc/sbx/caladan3/tmu/taps/sbucket.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>

uint8 mem_pool_enable = FALSE;
soc_heap_sl_mem_chunk_t *g_trie_node_hpcm;
soc_heap_sl_mem_chunk_t *g_dpfx_hpcm;
static taps_work_type_e_t work_type[] = {
    TAPS_TCAM_WORK, 
    TAPS_SBUCKET_WORK, 
    TAPS_DBUCKET_DATA_WORK,
    TAPS_DBUCKET_WORK,
    TAPS_TCAM_PROPAGATION_WORK,
    TAPS_SBUCKET_PROPAGATION_WORK,
    TAPS_REDISTRIBUTE_STAGE1_WORK,
    TAPS_REDISTRIBUTE_STAGE2_WORK,
    TAPS_REDISTRIBUTE_STAGE3_WORK
};

/* Implemenation notes:
 * Fully supported feature:
 * TAPS_OFFCHIP_ALL - 3 level search only
 * Taps instances except TAPS_INST_SEQ sequential mode
 * IPV4 lookup
 */
taps_container_t *taps_state[SOC_MAX_NUM_DEVICES];

typedef  dq_t taps_cmd_cache;

taps_handle_t ipv6da_taps[SOC_MAX_NUM_DEVICES] = {NULL};


/* Supports caching of commands */
taps_cmd_cache taps_cached_cmds[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM];


int _cache_taps_work[SOC_MAX_NUM_DEVICES] = {1};
int taps_used_as_em = FALSE;

typedef struct _taps_obj_s {
    taps_wgroup_handle_t wgroup[SOC_MAX_NUM_DEVICES];
    taps_tcam_pivot_handle_t tph;
    taps_spivot_handle_t sph;
    taps_dprefix_handle_t dph;
    taps_dbucket_handle_t ndbh;
    taps_dbucket_handle_t wdbh; /* wild spivot dbucket */
    taps_spivot_handle_t  nsph;
    taps_sbucket_handle_t nsbh;
    taps_tcam_pivot_handle_t ntph;
    unsigned int domain_id;
    unsigned int sph_id;
	int tcam_entry_id;
    /* true is the operation is associated with default route on a vrf*/
    uint8  vrf_def_route; 
} taps_obj_t;

typedef struct taps_cookie_s {
    int unit;
    taps_arg_t arg;
    taps_obj_t obj;
    int state; /* enum based on processing */
    int oldstate;
    /* Following params used for error handling */
    taps_spivot_handle_t *remove_sph;
    taps_spivot_handle_t *added_sph;
    taps_dprefix_handle_t *redisted_dph; 
    uint16 remove_sph_count;
    uint16 added_sph_count;
    uint16 redisted_dph_count;
    unsigned int tcam_pivot_len;
} taps_cookie_t;

#define TRANSITION_STATE(cookie,_newstate) \
 (cookie).oldstate = (cookie).state; (cookie).state = _newstate;

#define TRANSITION_ERROR_STATE(cookie,suffix)      \
      TRANSITION_STATE(cookie,(suffix##_ST_ERROR));

#define TRANSITION_ERROR_HANDLE(cookie,suffix)      \
      TRANSITION_STATE(cookie,(suffix##_ST_ERROR)); break;

#define _TAPS_MAX_KEY_WORDS_ (5)

/* insert states */
typedef enum _taps_ins_state_e {
    INS_ST_INIT = 0,
    INST_ST_VRF_DEF_ROUTE,
    INS_ST_DBKT_PFX,
    INS_ST_DBKT_PROPAGATE,
    INS_ST_DBKT_SPLIT,
    INS_ST_DBKT_PROPAGATE_SPLIT,
    INS_ST_SBKT_PIVOT,
    INS_ST_DOMAIN_SPLIT,
    INS_ST_DOMAIN_PROPAGATE_SPLIT,
    INS_ST_TCAM_PIVOT,
    INS_ST_BUCKET_REDIST,
    INS_ST_ERROR,
    INS_ST_DONE,
    INS_ST_MAX
} taps_ins_state_t;

/* delete states */
typedef enum _taps_del_state_e {
    DEL_ST_INIT = 0,
    DEL_ST_DBKT_PFX,
    DEL_ST_WILD_SPIVOT,
    DEL_ST_SBKT_PIVOT,
    DEL_ST_TCAM_PIVOT,
    DEL_ST_ERROR,
    DEL_ST_DONE,
    DEL_ST_MAX
} taps_del_state_t;

/* update states */
typedef enum _taps_upd_state_e {
    UPD_ST_INIT = 0,
    UPD_ST_ERROR,
    UPD_ST_DONE,
    UPD_ST_MAX
} taps_upd_state_t;

typedef struct _taps_redistribute_info_s {    
    taps_cookie_t old_cookie;                      /* old cookie for original operations */
    taps_cookie_t cookie;                          /* cookie for the redistribution operations */
    unsigned int pivot[TAPS_MAX_KEY_SIZE_WORDS];   /* pivot for new tcam domain (sbucket) */
    unsigned int pivot_len;                        /* pivot length for new tcam domain (sbucket) */
    unsigned int pfx_cnt;                          /* count of prefixes that need to be moved */
    taps_dprefix_handle_t *pdph;                   /* array of dph pointer that need to be moved */
} _taps_redistribute_info_t;

typedef struct _taps_bucket_redist_s {
    int unit;
    taps_handle_t taps;
    taps_wgroup_handle_t *wgroup;
    uint32 tph_cnt;
    uint32 sph_cnt;
    uint32 dph_cnt;
    taps_tcam_pivot_handle_t *tph;
    taps_spivot_handle_t *sph;
    taps_dprefix_handle_t *dph;
} _taps_bucket_redist_t;

typedef struct taps_share_cache_cookie_data_s {
    int master_unit;
    int num_slaves;
    int slave_units[SOC_MAX_NUM_SLAVE];
} taps_share_cache_cookie_data_t;

taps_share_cache_cookie_data_t g_share_cache_info;

#define TAPS_CACHED_CMD_POST_PFX_CHECK
#define _TAPS_COMMIT_FLAG_DRAM (0x1)
#define _TAPS_COMMIT_FLAG_SRAM (0x2)
#define _TAPS_COMMIT_FLAG_TCAM (0x4)
#define _TAPS_COMMIT_FLAG_ALL (_TAPS_COMMIT_FLAG_DRAM | \
                               _TAPS_COMMIT_FLAG_SRAM | \
                               _TAPS_COMMIT_FLAG_TCAM)

#ifdef TIME_STAMP_DBG 
sal_usecs_t        start, end;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    end = sal_time_usecs();                                         \
    LOG_CLI((BSL_META("\n %s: Time Stamp: [%u]"), msg, SAL_USECS_SUB(end, start))); \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#endif



static uint _taps_cache_count[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM];
static uint _taps_cache_size[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM];

#define TAPS_MAX_CACHE                    255

#define TAPS_AUTOCACHE_DEF_INTERVAL     10000  /* Autocache flush interval in microseconds */
#define TAPS_AUTOCACHE_NAME         "taps_ac"  /* Name of the autocache thread */
#define TAPS_AUTOCACHE_THREAD_PRIO         15  /* Priority for the autocache thread */


static volatile sal_thread_t _taps_autocache[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM] = {NULL};
sal_mutex_t _taps_autocache_mutex[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM] = {NULL};

/* #define TAPS_DEBUG_BACKGROUND_INSERT */
#ifdef TAPS_DEBUG_BACKGROUND_INSERT
static volatile sal_thread_t _taps_background_insert[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM][16] = {{NULL}};
static sal_sem_t _taps_background_insert_sem[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM][16] = {{NULL}};
#endif

#undef TAPS_USE_NANO_TIMER

#undef BCM_CALADAN3_TAPS_USE_SPINLOCK

#ifdef BCM_CALADAN3_TAPS_USE_SPINLOCK
#define TAPS_USE_SPINLOCK
#else
#undef TAPS_USE_SPINLOCK
#endif

#ifndef TAPS_USE_SPINLOCK
static sal_mutex_t _taps_sal_mutex[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM] = {NULL};
#define TAPS_CACHE_LOCK_GET(unit)     sal_mutex_take(_taps_sal_mutex[(unit)],sal_mutex_FOREVER);
#define TAPS_CACHE_LOCK_FREE(unit)    sal_mutex_give(_taps_sal_mutex[(unit)]);
#else
#include <pthread.h>
static pthread_spinlock_t _taps_cache_lock[SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM];
#define TAPS_CACHE_LOCK_GET(unit)     pthread_spin_lock(&_taps_cache_lock[(unit)]);
#define TAPS_CACHE_LOCK_FREE(unit)    pthread_spin_unlock(&_taps_cache_lock[(unit)]);
#endif

static int taps_share_flush_cache_without_cache_lock(unsigned int unit);
static int taps_flush_cache_without_cache_lock(unsigned int unit);


#if (defined(LINUX) && !defined(__KERNEL__)) || defined(UNIX)
#define TAPS_USE_NANO_TIMER
#include <time.h>
#endif 

/*
 *
 * Function:
 *     _taps_autocache_thread
 * Purpose:
 *     Thread that periodically flushes any entries in the cache
 */
static void _taps_autocache_thread(int unit) {
    unsigned int last_cache_count=0;

#ifdef TAPS_USE_NANO_TIMER
    struct timespec sleep_time, sleep_rep;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = TAPS_AUTOCACHE_DEF_INTERVAL * 1000;
#endif

    /* Flush the cache then sleep - repeat */
    while(1) {
    /* Check if the current route count is the same as last time to 
    avoid butting on on an ongoing fast load */

    if((last_cache_count == _taps_cache_count[unit]) && (_taps_cache_size[unit] != 0)) {
        taps_flush_cache(unit);
    } else {
        last_cache_count = _taps_cache_count[unit];
    }

#ifdef TAPS_USE_NANO_TIMER
    clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, &sleep_rep);
#else
    sal_mutex_take( _taps_autocache_mutex[unit],TAPS_AUTOCACHE_DEF_INTERVAL );
#endif

    }
}

static void _taps_autocache_init(int unit) {

#ifndef TAPS_USE_NANO_TIMER
  /* Create the mutex we will use to implement the wakeup  */
   _taps_autocache_mutex[unit] = sal_mutex_create("Autocache Timing Mutex");
 
  /* We will take the mutex here and use the timeout for driving the thread interval */
   sal_mutex_take( _taps_autocache_mutex[unit],sal_mutex_FOREVER);
#endif
   /* Lock for syncing autocache thread with route insertion thread for flushing cache */
#ifndef TAPS_USE_SPINLOCK
   _taps_sal_mutex[unit] = sal_mutex_create("Cache Mutex");
#else
   pthread_spin_init(&_taps_cache_lock[unit], PTHREAD_PROCESS_PRIVATE);
#endif    
  /* Start up the autocache service thread */
  _taps_autocache[unit] =  sal_thread_create(TAPS_AUTOCACHE_NAME,
                                       SAL_THREAD_STKSZ, TAPS_AUTOCACHE_THREAD_PRIO,
                                       (void (*)(void*))_taps_autocache_thread,
                                       INT_TO_PTR(unit));

  if (!SOC_IS_SBX_MASTER(unit)) {
        if(g_share_cache_info.num_slaves >= SOC_MAX_NUM_SLAVE) {
            return;
        }
        g_share_cache_info.master_unit = SOC_SBX_MASTER(unit);
        g_share_cache_info.slave_units[g_share_cache_info.num_slaves] = unit;
        g_share_cache_info.num_slaves++;
        
        DQ_INIT(&taps_cached_cmds[SOC_TAPS_SHARE_CACHE_UNIT]);
        
#ifndef TAPS_USE_NANO_TIMER
        /* Create the mutex we will use to implement the wakeup  */
        _taps_autocache_mutex[SOC_TAPS_SHARE_CACHE_UNIT] = 
                                sal_mutex_create("Share cache autocache Timing Mutex");

        /* We will take the mutex here and use the timeout for driving the thread interval */
        sal_mutex_take( _taps_autocache_mutex[SOC_TAPS_SHARE_CACHE_UNIT],sal_mutex_FOREVER);
#endif

#ifndef TAPS_USE_SPINLOCK
       _taps_sal_mutex[SOC_TAPS_SHARE_CACHE_UNIT] = sal_mutex_create("Cache Master unit Mutex");
#else
       pthread_spin_init(&_taps_cache_lock[SOC_TAPS_SHARE_CACHE_UNIT], PTHREAD_PROCESS_PRIVATE);
#endif
      _taps_autocache[SOC_TAPS_SHARE_CACHE_UNIT] =  sal_thread_create("Share cache",
                                                SAL_THREAD_STKSZ, TAPS_AUTOCACHE_THREAD_PRIO,
                                                (void (*)(void*))_taps_autocache_thread,
                                                INT_TO_PTR(SOC_TAPS_SHARE_CACHE_UNIT));

  } 
  
}


/* #define TAPS_DEBUG_CACHE */



/*
 *
 * Function:
 *     taps_set_caching
 * Purpose:
 *     turns taps work item caching on or off
 *     Deprecated: Does nothing
 */
void taps_set_caching(int unit,int cache)
{
  _cache_taps_work[unit] = cache;
}

/*
 *
 * Function:
 *     _taps_get_caching
 * Purpose:
 *     returns value indicating whether internal caching is on or off
 */
int _taps_get_caching(int unit)
{
    return _cache_taps_work[unit];
}



/*
 *
 * Function:
 *     taps_cache_work
 * Purpose:
 *     Cache up the work items for later flushing
 */
int taps_cache_work(int unit, taps_wgroup_handle_t wgroup, taps_work_type_e_t *work_type,
        uint32 work_type_count)
{

    int index;
    int rv = SOC_E_NONE;
    dq_p_t work_item;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;

    TAPS_CACHE_LOCK_GET(unit)

    for (index=0; index < work_type_count; index++) {
        do {
            rv = taps_work_dequeue(unit, wgroup, work_type[index],
                                   &work_item, _WQ_DEQUEUE_DEFAULT_);
            if (SOC_SUCCESS(rv)) {
                _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);

                /* put it on the cache for sending and checking */
                DQ_INSERT_TAIL(&taps_cached_cmds[unit],&cmd->tc_list_elem);
                /* increment cache count and size*/
                _taps_cache_count[unit]++;
                _taps_cache_size[unit]++;
                if(_taps_cache_size[unit] >= TAPS_MAX_CACHE) {
                    /* Avoid to excess max response buffer */
                    taps_flush_cache_without_cache_lock(unit);
                }
            }
        } while (SOC_SUCCESS(rv));
    }

    TAPS_CACHE_LOCK_FREE(unit)

    return SOC_E_NONE;
}

static void taps_commit_force_worktype_set(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup,
                                                    taps_work_type_e_t type)
{
    int slave_idx, slave_unit;
    if (type >= TAPS_REDISTRIBUTE_STAGE1_WORK && type <= TAPS_REDISTRIBUTE_STAGE3_WORK) {
        wgroup[unit]->force_work_type_enable = TRUE;
        wgroup[unit]->forced_work_type = type;
        if (_IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table) 
            && !_taps_get_caching(unit)) {
            for (slave_idx = 0; slave_idx < taps->num_slaves; slave_idx++) {
                slave_unit = taps->slave_units[slave_idx];
                wgroup[slave_unit]->force_work_type_enable = TRUE;
                wgroup[slave_unit]->forced_work_type = type;
            }
        }
    }
}

static void taps_commit_force_worktype_unset(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup)
{
    int slave_idx, slave_unit;
    wgroup[unit]->force_work_type_enable = FALSE;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table) 
        && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < taps->num_slaves; slave_idx++) {
            slave_unit = taps->slave_units[slave_idx];
            wgroup[slave_unit]->force_work_type_enable = FALSE;
        }
    }
}

/*
 *
 * Function:
 *     taps_cmd_print
 * Purpose:
 *     Wrapper function for using linked list macro to dump command cache
 */
void taps_cmd_print(void * p_elem, int unit)
{
    tmu_cmd_printf(unit, DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t *,p_elem,tc_list_elem));
}

/*
 *
 * Function:
 *     taps_cmd_cache_print
 * Purpose:
 *     Prints the command cache
 */
void taps_cmd_cache_print(int unit, taps_cmd_cache * p_cmd_cache)
{
    /* Use the linked list macro to walk the list calling the print for each element */
    DQ_MAP(p_cmd_cache, taps_cmd_print, unit);
}

static int taps_cached_cmd_access_same_entry(int unit, soc_sbx_caladan3_tmu_cmd_t *p_cmd, int *no_post)
{   
    int rv = SOC_E_NONE;
#ifdef TAPS_CACHED_CMD_POST_PFX_CHECK
    dq_p_t elem;
    soc_sbx_caladan3_tmu_cmd_t *next_cmd;
    if (p_cmd->post_dpfx_index != POST_PFX_CHECK_DISABLE) {
        DQ_BACK_TRAVERSE(&taps_cached_cmds[unit], elem) {
            next_cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*, elem, tc_list_elem);
            if (next_cmd != p_cmd) {
                if(next_cmd->post_dpfx_index == p_cmd->post_dpfx_index
                && next_cmd->cmd.xlwrite.entry_num == p_cmd->cmd.xlwrite.entry_num    
                && next_cmd->cmd.xlwrite.table == p_cmd->cmd.xlwrite.table) {
                    *no_post = TRUE;   
                    break;
                }   
            } else {
                 break;
            }
        } DQ_TRAVERSE_END(&taps_cached_cmds[unit], elem);
    }
#endif
    return rv;
}
/*
 *
 * Function:
 *     taps_cached_cmd_dma
 * Purpose:
 *    Send cached commands to DMA
 */
void taps_cached_cmd_dma(void * p_elem, int unit)
{
    int rv;
    int no_post = FALSE;
    soc_sbx_caladan3_tmu_cmd_t *p_cmd;
   
    p_cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t *,p_elem, tc_list_elem);
    
    rv = taps_cached_cmd_access_same_entry(unit, p_cmd, &no_post);
    if(SOC_SUCCESS(rv)) {
        if (no_post) {
            p_cmd->opcode = SOC_SBX_TMU_CMD_MAX;
            return;
        } 
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d Failed to get flush state %d\n"),
                 FUNCTION_NAME(), unit, rv));
    }
    
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
            p_cmd, SOC_SBX_TMU_CMD_POST_CACHE);

    if(SOC_FAILURE(rv)) {
        if (rv == SOC_E_FULL) {
            rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                         NULL, SOC_SBX_TMU_CMD_POST_FLUSH);
            if (SOC_SUCCESS(rv)) {
                 /* Send out the command that got rejected */
                rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                         p_cmd, SOC_SBX_TMU_CMD_POST_CACHE);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d Failed command send on entry%d\n"),
                         FUNCTION_NAME(), unit, rv));
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                            "%s: unit %d Failed to flush cmds %d\n"),
                    FUNCTION_NAME(), unit, rv));
            }
        }else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                            "%s: unit %d Failed to cache cmds%d\n"),
                    FUNCTION_NAME(), unit, rv));
        }
    }
}

/*
 *
 * Function:
 *     taps_cached_cmd_share_dma
 * Purpose:
 *    Send cached commands to DMA
 */
void taps_cached_cmd_share_dma(void * p_elem, taps_share_cache_cookie_data_t *cookie)
{
    int rv;
    int no_post = FALSE;
    soc_sbx_caladan3_tmu_cmd_t * p_cmd;

    p_cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t *,p_elem, tc_list_elem);
    
    rv = taps_cached_cmd_access_same_entry(SOC_TAPS_SHARE_CACHE_UNIT, p_cmd, &no_post);
    if(SOC_SUCCESS(rv)) {
        if (no_post) {
            p_cmd->opcode = SOC_SBX_TMU_CMD_MAX;
            return;
        } 
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(cookie->master_unit,
                            "%s: unit %d Failed to get flush state %d\n"),
                 FUNCTION_NAME(), cookie->master_unit, rv));
    }
    
    rv = soc_sbx_caladan3_tmu_master_cache_cmd(cookie->master_unit, &cookie->slave_units[0], cookie->num_slaves,
                            SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, p_cmd);
    if(SOC_FAILURE(rv)) {
        if (rv == SOC_E_FULL) {
            rv = soc_sbx_caladan3_tmu_master_flush_cmd(cookie->master_unit, &cookie->slave_units[0], 
                            cookie->num_slaves, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO);
            if (SOC_SUCCESS(rv)) {
                 /* Send out the command that got rejected */
                rv = soc_sbx_caladan3_tmu_master_cache_cmd(cookie->master_unit, &cookie->slave_units[0], cookie->num_slaves, 
                                SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, p_cmd);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                        (BSL_META_U(cookie->master_unit,
                                    "%s: unit %d Failed command send on entry%d\n"),
                         FUNCTION_NAME(), cookie->master_unit, rv));
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(cookie->master_unit,
                            "%s: unit %d Failed to flush cmds for host_share mode%d\n"),
                    FUNCTION_NAME(), cookie->master_unit, rv));
            }
        }else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(cookie->master_unit,
                            "%s: unit %d Failed to cache cmdsfor host_share mode %d\n"),
                    FUNCTION_NAME(), cookie->master_unit, rv));
        }
    }
}

/*
 *
 * Function:
 *     taps_cached_cmd_resp
 * Purpose:
 *     Process cached responses
 */
INLINE void taps_cached_cmd_resp(void * p_elem, int unit)
{
    int rv;
    soc_sbx_caladan3_tmu_cmd_t * p_cmd;

    p_cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t *,p_elem, tc_list_elem);
    if (p_cmd->opcode != SOC_SBX_TMU_CMD_MAX) {
        rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                    p_cmd, NULL,0);
        
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid response !!!\n"),
                       FUNCTION_NAME(), unit));
        }
    } else {
        p_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    }
    
    tmu_cmd_free(unit, p_cmd);
}

/*
 *
 * Function:
 *     taps_cached_cmd_share_resp
 * Purpose:
 *     Process cached responses
 */
INLINE void taps_cached_cmd_share_resp(void * p_elem, taps_share_cache_cookie_data_t *cookie)
{
    int rv;
    int slave_idx = 0;
    soc_sbx_caladan3_tmu_cmd_t * p_cmd;

    p_cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t *,p_elem, tc_list_elem);
    if (p_cmd->opcode != SOC_SBX_TMU_CMD_MAX) {
        rv = soc_sbx_caladan3_tmu_get_resp(cookie->master_unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
            p_cmd, NULL,0);

        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Invalid response !!!\n"),
                       FUNCTION_NAME(), cookie->master_unit));
        }
        while(slave_idx < cookie->num_slaves) {
            p_cmd->seqnum = p_cmd->slave_seqnum[slave_idx];
            soc_sbx_caladan3_tmu_slave_cache_cmd(cookie->master_unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, p_cmd);
            
            rv = soc_sbx_caladan3_tmu_slave_get_resp(cookie->slave_units[slave_idx], cookie->master_unit, 
                                    SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, p_cmd, NULL,0);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: unit %d Invalid response !!!\n"),
                           FUNCTION_NAME(), cookie->master_unit));
            }
            ++slave_idx;
        }
    } else {
        p_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    }
    tmu_cmd_free(cookie->master_unit, p_cmd);
}

static int _taps_flush_cache(unsigned int unit)
{
    int rv = SOC_E_NONE;
    TMU_LOCK(unit);    /* We need to lock out any other TMU commands while sending the cache */

    if(_taps_cache_size[unit] != 0)
      {
    /* Send all the commands */
    DQ_MAP(&taps_cached_cmds[unit], taps_cached_cmd_dma, unit);

    /* Flush out any existing commands */
    /* coverity[unchecked_value] */
    /* coverity[check_return] */
    soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
            NULL, SOC_SBX_TMU_CMD_POST_FLUSH);

    /* Process the responses */
    DQ_MAP(&taps_cached_cmds[unit], taps_cached_cmd_resp, unit);

    /* Get ready for next batch */
    DQ_INIT(&taps_cached_cmds[unit]);

        _taps_cache_size[unit] = 0;

      }
    TMU_UNLOCK(unit);    
    return rv;
}

/*
 *
 * Function:
 *     taps_flush_cache
 * Purpose:
 *     Flush all cached work items to hardware
 */
int taps_flush_cache(unsigned int unit)
{
    int rv = SOC_E_NONE;
#ifdef TAPS_DEBUG_CACHE
    LOG_CLI((BSL_META_U(unit,
                        "%s: Cached Commands\n"),FUNCTION_NAME()));
    taps_cmd_cache_print(unit, &taps_cached_cmds);
#endif
    if (unit == SOC_TAPS_SHARE_CACHE_UNIT) {
        return taps_share_flush_cache(unit);
    }
        
    TAPS_CACHE_LOCK_GET(unit)
        
    rv = _taps_flush_cache(unit);
    
    TAPS_CACHE_LOCK_FREE(unit)
        
    return rv;
}

static int taps_flush_cache_without_cache_lock(unsigned int unit)
{
    int rv = SOC_E_NONE;
#ifdef TAPS_DEBUG_CACHE
    LOG_CLI((BSL_META_U(unit,
                        "%s: Cached Commands\n"),FUNCTION_NAME()));
    taps_cmd_cache_print(unit, &taps_cached_cmds);
#endif
    if (unit == SOC_TAPS_SHARE_CACHE_UNIT) {
        return taps_share_flush_cache_without_cache_lock(unit);
    }

    rv = _taps_flush_cache(unit);
    return rv;
}

static int _taps_share_flush_cache(unsigned int unit)
{
    int rv = SOC_E_NONE;
    int slave_idx = 0;
    assert(unit >= SOC_TAPS_SHARE_CACHE_UNIT);
    if(_taps_cache_size[unit] != 0) {
        /* We need to lock out any other TMU commands while sending the cache
               * Lock all units */
        TMU_LOCK(g_share_cache_info.master_unit);    
        while(slave_idx < g_share_cache_info.num_slaves) {
            TMU_LOCK(g_share_cache_info.slave_units[slave_idx]);
            ++slave_idx;
        }
        /* Cache all the commands */
        DQ_MAP(&taps_cached_cmds[unit], taps_cached_cmd_share_dma, &g_share_cache_info);

        /* Flush out any existing commands */
        soc_sbx_caladan3_tmu_master_flush_cmd(g_share_cache_info.master_unit, &g_share_cache_info.slave_units[0],
                    g_share_cache_info.num_slaves, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO);

        /* Process the responses */
        DQ_MAP(&taps_cached_cmds[unit], taps_cached_cmd_share_resp, &g_share_cache_info);

        /* Lock all units */
        slave_idx = 0;
        while(slave_idx < g_share_cache_info.num_slaves) {
            TMU_UNLOCK(g_share_cache_info.slave_units[slave_idx]);
            ++slave_idx;
        }
        TMU_UNLOCK(g_share_cache_info.master_unit);
    }
    
     DQ_INIT(&taps_cached_cmds[unit]);
    _taps_cache_size[unit] = 0;
    return rv;
}


/*
 *
 * Function:
 *     taps_share_flush_cache
 * Purpose:
 *     Cache & flush commands for all units
 */
int taps_share_flush_cache(unsigned int unit) 
{   
    int rv = SOC_E_NONE;
    TAPS_CACHE_LOCK_GET(unit)
        
    rv = _taps_share_flush_cache(unit);
    
    TAPS_CACHE_LOCK_FREE(unit)
    return rv;
}

static int taps_share_flush_cache_without_cache_lock(unsigned int unit) 
{
    return _taps_share_flush_cache(unit); 
}

/*
 *
 * Function:
 *     taps_host_flush_cache
 * Purpose:
 *     To do share flush or single flush, decided by whether the taps is shared.
 */
int taps_host_flush_cache(unsigned int unit, int host_share) 
{   
    int rv;
    if (host_share) {
       rv = taps_share_flush_cache(SOC_TAPS_SHARE_CACHE_UNIT);
    } else {
       rv = taps_flush_cache(unit);
    }

    return rv;
}

static INLINE int hpcm_trie_node_init(int unit)
{
    int rv;
    rv = hpcm_sl_init(unit, TAPS_MEM_POOL_CHUNK_SIZE * 4, 
                sizeof(trie_node_t), 
                &g_trie_node_hpcm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to allocate hpcm memory for trie\n"),
                   FUNCTION_NAME(), unit));
    }
    return rv;
}

static INLINE int hpcm_dprefix_init(int unit)
{
    int rv;
    rv = hpcm_sl_init(unit, TAPS_MEM_POOL_CHUNK_SIZE, 
                sizeof(taps_dprefix_t) + \
                4 * sizeof(unsigned int), 
                &g_dpfx_hpcm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to allocate hpcm memory for dpfx\n"),
                   FUNCTION_NAME(), unit));
    }
    return rv;
}

/* #define REDISTRIBUTE_DBG */

/* internal functions to calculate prefix global/local pointer */
static int taps_calculate_prefix_global_pointer(int unit, 
                        taps_handle_t taps,
                        taps_tcam_pivot_handle_t tph,
                        taps_spivot_handle_t sph,
                        taps_dprefix_handle_t dph,
                        uint32 *bpm_global_index);

/* internal callback functions for prefix redistribution */
static int _taps_redistribute_dbucket_after_split_cb(taps_spivot_handle_t sph,
                             void *user_data);

static int _taps_redistribute_prefix_after_split_cb(taps_dprefix_handle_t dph,
                            void* user_data);

static int _taps_redistribute_prefix_after_split(taps_dprefix_handle_t dph, _taps_redistribute_info_t *info, taps_cookie_t *out_cookie);

static int _taps_redistributed_prefix_move(taps_dprefix_handle_t dph, _taps_redistribute_info_t* info);

static int _taps_splitted_prefix_move(taps_dprefix_handle_t dph, taps_dbucket_handle_t dbh,
                                _taps_redistribute_info_t* info);

static int _taps_calculate_prefix_pointer(int unit, 
                      taps_handle_t taps,
                      taps_sbucket_handle_t sbh,
                      taps_dbucket_handle_t dbh,
                      void * p_prefix_handle,
                      uint32 *pointer, uint8 global);

/*
 *
 * Function:
 *     taps_work_commit
 * Purpose:
 *     issues bulk/sequential commit & process response all issued command
 */
int taps_work_commit(int unit, 
                     taps_wgroup_handle_t wgroup,
                     taps_work_type_e_t *work_type,
                     uint32 work_type_count,
                     taps_work_commit_model_e_t commit)
{
    int rv = SOC_E_NONE, index=0;
    dq_p_t work_item;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;
#ifdef _TAPS_PERFORMANCE_PROFILE
    sal_usecs_t start, end;
    uint32 time=0;
    static uint32 count, total;
    commit = _TAPS_SEQ_COMMIT;
#endif

    if (work_type_count == 0) return SOC_E_NONE;
    if (!wgroup || !work_type) return SOC_E_PARAM;
    if (!_TAPS_VALID_COMMIT(commit)) return SOC_E_PARAM;

    /* validate work type */
    for (index=0; index < work_type_count; index++) {
        if (!(_TAPS_VALID_WORK_TYPE_(work_type[index]))) {
            LOG_CLI((BSL_META_U(unit,
                                " Bad work type %d !! \n"), work_type[index]));
            return SOC_E_PARAM;
        }
    }

    /* Check if we are caching up entries. If so put them on the work cache */
    if(_taps_get_caching(unit)) {
        return taps_cache_work(wgroup->host_share?SOC_TAPS_SHARE_CACHE_UNIT:unit, wgroup, work_type, work_type_count);
    }

    if (_TAPS_SEQ_COMMIT == commit) {
    /* post dma and wait for response one command a time */
        for (index=0; index < work_type_count && SOC_SUCCESS(rv); index++) {
            do {
                rv = taps_work_dequeue(unit, wgroup, work_type[index],
                                       &work_item, _WQ_DEQUEUE_DEFAULT_); 
                if (SOC_SUCCESS(rv)) {
                    _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);

#ifdef _TAPS_PERFORMANCE_PROFILE
                    start = sal_time_usecs();
#endif

            TMU_LOCK(unit);
            
                    /* post the command to hardware */
                    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                       cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
                    if (SOC_SUCCESS(rv)) {
                        rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                           cmd, NULL, 0);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d Invalid response !!!\n"), 
                                       FUNCTION_NAME(), unit));
                        }
                        tmu_cmd_free(unit, cmd);

#ifdef _TAPS_PERFORMANCE_PROFILE
                        end = sal_time_usecs();
                        time = SAL_USECS_SUB(end, start);
                        count++;
                        total += time;
                        if (count && ((count%1000) == 0)) {
                            LOG_CLI((BSL_META_U(unit,
                                                "Taps %d cmds total %d usecs average %d usecs\n"), count, total, total/count)); 
                        }
#endif
                    }  
                    TMU_UNLOCK(unit);
                    
                }
            } while (SOC_SUCCESS(rv));

            if (rv == SOC_E_EMPTY) rv = SOC_E_NONE;
        }

    } else if (_TAPS_BULK_COMMIT == commit) {
    /* bulk commit, post dma for multiple commands at a time */
        for (index=0; index < work_type_count; index++) {
            /* cache commit model */
            rv = taps_work_queue_stats(unit, wgroup, work_type[index]);
            if (SOC_SUCCESS(rv)) {

                rv = taps_work_queue_iter_first(unit, wgroup,
                                                work_type[index],
                                                &work_item);
                TMU_LOCK(unit);
                
                if (SOC_SUCCESS(rv)) {
                    while(SOC_SUCCESS(rv)) {

                        _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);

                        rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                           cmd, SOC_SBX_TMU_CMD_POST_CACHE);
                        if (rv == SOC_E_FULL) { /* flush commit */
                            rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                               NULL, SOC_SBX_TMU_CMD_POST_FLUSH);
                            if (rv == SOC_E_EMPTY) {
                                rv = SOC_E_NONE;
                            } else if (SOC_FAILURE(rv)) {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Failed to flush cache commands %d !!!\n"), 
                                           FUNCTION_NAME(), unit, rv));
                            }

                rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                   cmd, SOC_SBX_TMU_CMD_POST_CACHE);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to cache commands after flush %d !!!\n"), 
                           FUNCTION_NAME(), unit, rv));
                }               
                        } else if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d Failed to cache commands %d !!!\n"), 
                                       FUNCTION_NAME(), unit, rv));
                        }
                        
                        if (SOC_SUCCESS(rv)) {
                            rv = taps_work_queue_iter_get_next(unit, wgroup,
                                                               work_type[index],
                                                               &work_item);
                        }
                    }
                    
                    /* flush commit */
                    if (rv == SOC_E_LIMIT) {
                        rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                           NULL, SOC_SBX_TMU_CMD_POST_FLUSH);
                        if (rv == SOC_E_EMPTY) {
                            rv = SOC_E_NONE;
                        } else if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d Failed to flush cache commands %d !!!\n"), 
                                       FUNCTION_NAME(), unit, rv));
                        }
                    }
                } 

                /* process response */
                while (SOC_SUCCESS(rv)) {
                    rv = taps_work_dequeue(unit, wgroup, work_type[index],
                                           &work_item, _WQ_DEQUEUE_DEFAULT_); 
                    if (SOC_SUCCESS(rv)) {
                        _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);
                        
                        /* post the command to hardware */
                        rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                           cmd, NULL, 0);
                        if (SOC_FAILURE(rv)) {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Invalid response !!!\n"), 
                                           FUNCTION_NAME(), unit));
                        }
                        tmu_cmd_free(unit, cmd);
                    }
                }
                TMU_UNLOCK(unit);
            }
        }
    } else {
        assert(0);
    }

    if (rv == SOC_E_EMPTY) rv = SOC_E_NONE;

    return rv;
}


int _taps_commit(int unit, taps_cookie_t *cookie, unsigned int flags)
{
    taps_obj_t *obj=NULL;
    int rv = SOC_E_NONE;
    int slave_idx = 0;
    
    if (!cookie) return SOC_E_PARAM;
    if (flags & ~_TAPS_COMMIT_FLAG_ALL) return SOC_E_PARAM;

    obj = &cookie->obj;

    /* commit the work item to hardware */
    if (flags & _TAPS_COMMIT_FLAG_DRAM) {
        rv = taps_dbucket_commit(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to commit dram %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (SOC_SUCCESS(rv) && (flags & _TAPS_COMMIT_FLAG_SRAM)) {
        rv = taps_sbucket_commit(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to commit sram %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (SOC_SUCCESS(rv) && (flags & _TAPS_COMMIT_FLAG_TCAM)) {
        rv = taps_tcam_commit(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to commit tcam %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (!_taps_get_caching(unit)
        && _IS_MASTER_SHARE_LPM_TABLE(unit, cookie->arg.taps->master_unit,
                                    cookie->arg.taps->param.host_share_table)
        && SOC_SUCCESS(rv)) {
		/* Commit command for slave unit */
        while (slave_idx <  cookie->arg.taps->num_slaves) {
            rv = _taps_commit(cookie->arg.taps->slave_units[slave_idx], cookie, flags);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit cmmmands for "
                                      "slave unit %d:%s !!!\n"), 
                           FUNCTION_NAME(), cookie->arg.taps->slave_units[slave_idx],
                           rv, soc_errmsg(rv)));
                break;
            }
            ++slave_idx;
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_free_work_queue
 * Purpose:
 *     flushes out entire work queue. Does not commit it to hardware,
 *     rather frees up all heap resources
 */
int taps_free_work_queue(int unit, 
                         taps_wgroup_handle_t wgroup,
                         taps_work_type_e_t *work_type,
                         uint32 work_type_count)
{
    int rv = SOC_E_NONE, index=0;
    dq_p_t work_item;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;

    if (work_type_count == 0) return SOC_E_NONE;
    if (!wgroup || !work_type) return SOC_E_PARAM;

    /* validate work type */
    for (index=0; index < work_type_count; index++) {
        if (!(_TAPS_VALID_WORK_TYPE_(work_type[index]))) {
            LOG_CLI((BSL_META_U(unit,
                                " Bad work type %d !! \n"), work_type[index]));
            return SOC_E_PARAM;
        }
    }

    do {
        rv = taps_work_dequeue(unit, wgroup, work_type[index],
                               &work_item, _WQ_DEQUEUE_DEFAULT_); 
        if (SOC_SUCCESS(rv)) {
            _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);
            /* free the command */
            tmu_cmd_free(unit, cmd);
        }
    } while (SOC_SUCCESS(rv));

    if (rv == SOC_E_EMPTY) rv = SOC_E_NONE;

    return rv;
}


int _taps_free_work_queue(int unit, taps_cookie_t *cookie, unsigned int flags)
{
    taps_obj_t *obj=NULL;
    int rv = SOC_E_NONE;

    if (!cookie) return SOC_E_PARAM;
    if (flags & ~_TAPS_COMMIT_FLAG_ALL) return SOC_E_PARAM;

    obj = &cookie->obj;

    /* free work queue */
    if (flags & _TAPS_COMMIT_FLAG_DRAM) {
        rv = taps_dbucket_free_work_queue(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free dram work queue %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (SOC_SUCCESS(rv) && (flags & _TAPS_COMMIT_FLAG_SRAM)) {
        rv = taps_sbucket_free_work_queue(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free sram work queue %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (SOC_SUCCESS(rv) && (flags & _TAPS_COMMIT_FLAG_TCAM)) {
        rv = taps_tcam_free_work_queue(unit, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free tcam work queue %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     _taps_parallel_segment_alloc
 * Purpose:
 *     allocate a taps segment id for parallel mode
 */
static int _taps_parallel_segment_alloc(int unit, taps_instance_e_t instance, unsigned int *segment)
{
    int rv = SOC_E_NONE, index=0;

    if (!segment) return SOC_E_PARAM;

    rv = SOC_E_MEMORY;

    for (index=0; index < _TAPS_MAX_SEGMENT_; index++) {
        if (!(taps_state[unit]->seg_allocator[instance] & (1<<index))) {
            *segment = index;
            rv = SOC_E_NONE;
            taps_state[unit]->seg_allocator[instance] |= (1<<index);
            break;
        }
    }
    return rv;
}

/*
 *
 * Function:
 *     _taps_unified_segment_alloc
 * Purpose:
 *     allocate a taps segment id for unified mode
 */
static int _taps_unified_segment_alloc(int unit, unsigned int *segment)
{
    int rv = SOC_E_NONE, index=0;

    if (!segment) return SOC_E_PARAM;

    rv = SOC_E_MEMORY;

    for (index=0; index < _TAPS_MAX_SEGMENT_; index++) {
        if ((!(taps_state[unit]->seg_allocator[TAPS_INST_0] & (1<<index))) 
            && (!(taps_state[unit]->seg_allocator[TAPS_INST_1] & (1<<index)))) {
            *segment = index;
            rv = SOC_E_NONE;
            taps_state[unit]->seg_allocator[TAPS_INST_0] |= (1<<index);
            taps_state[unit]->seg_allocator[TAPS_INST_1] |= (1<<index);
            break;
        }
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_segment_alloc
 * Purpose:
 *     allocate a taps segment id
 */
int taps_segment_alloc(int unit, taps_handle_t taps)
{
    int rv = SOC_E_NONE;

    if (!taps) {
        return SOC_E_PARAM;
    }

    taps->segment = _TAPS_MAX_SEGMENT_;
    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
        rv = _taps_parallel_segment_alloc(unit, taps->param.instance, &taps->segment);
    } else {
        rv = _taps_unified_segment_alloc(unit, &taps->segment);
    }
    
    return rv;
}
/*
 *
 * Function:
 *     _taps_parallel_segment_free
 * Purpose:
 *     free a taps segment id for parallel mode
 */
static int _taps_parallel_segment_free(int unit, taps_instance_e_t instance, unsigned int segment)
{
    if (segment >= _TAPS_MAX_SEGMENT_) {
        return SOC_E_PARAM;
    }

    taps_state[unit]->seg_allocator[instance] &= (~(1 << segment));
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_unified_segment_free
 * Purpose:
 *     free a taps segment id for unified mode
 */
static int _taps_unified_segment_free(int unit, unsigned int segment)
{
    if (segment >= _TAPS_MAX_SEGMENT_) {
        return SOC_E_PARAM;
    }

    taps_state[unit]->seg_allocator[TAPS_INST_0] &= (~(1 << segment));
    taps_state[unit]->seg_allocator[TAPS_INST_1] &= (~(1 << segment));
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     taps_segment_free
 * Purpose:
 *     free a taps segment id
 */
int taps_segment_free(int unit, taps_instance_e_t instance, uint32 segment)
{
    int rv = SOC_E_NONE;

    if (_TAPS_IS_PARALLEL_MODE_(instance)) {
        rv = _taps_parallel_segment_free(unit, instance, segment);
    } else {
        rv = _taps_unified_segment_free(unit, segment);
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_bucket_alloc
 * Purpose:
 *     allocate a sram bucket or tcam domain 
 */
int taps_bucket_alloc(int unit,taps_handle_t taps, int tcam_entry, unsigned int *domain_id)
{
    int rv = SOC_E_NONE;
    int index = 0;
    int start_alloc_id = 0;

    if (!taps || !domain_id) return SOC_E_PARAM;
    if (!taps->allocator) return SOC_E_INIT;
    rv = SOC_E_MEMORY;

    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
        /* Parallel mode
         * Allocate bucket id from zero 
         */
        start_alloc_id = 0;
    } else {
        /* Unified mode */
        if (tcam_entry < taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry) {
            /* tcam entry is in TAPS0. Reserve [0, TAPS_INST_0.num_entry) for TAPS0 */
            start_alloc_id = 0;
        } else {
            /* tcam_entry is in TAPS1. Reserve [TAPS_INST_0.num_entry, TAPS_INST_SEQ.num_entry) for TAPS1 */
            start_alloc_id = taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
        }
    }
    
    /* allocate tcam domain (different from actual tcam entry ID) using bitmask allocator */
    for (index = start_alloc_id; index < taps->param.seg_attr.seginfo[taps->param.instance].num_entry && \
         SOC_FAILURE(rv); index++) {
        if (!(SHR_BITGET(taps->allocator, index))) {
            *domain_id = index;
            rv = SOC_E_NONE;
            SHR_BITSET(taps->allocator, index);
        }
    }

    /* all index are local within segment */
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_free
 * Purpose:
 *     free a sram bucket or tcam domain 
 */
int taps_bucket_free(int unit,taps_handle_t taps, unsigned int domain_id)
{
    if (!taps) return SOC_E_PARAM;
    if (domain_id >= taps->param.seg_attr.seginfo[taps->param.instance].num_entry)
    return SOC_E_PARAM;
    if (!taps->allocator) return SOC_E_INIT;

    /* free tcam domain (different from actual tcam entry ID) using bitmask allocator */
    SHR_BITCLR(taps->allocator,domain_id);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_hw_init
 * Purpose:
 *     Bring up TAPS drivers
 */
int soc_sbx_caladan3_tmu_taps_hw_init(int unit) 
{
    taps_instance_e_t inst;
    uint32 regval;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
        /* only init hardware if not warm boot mode */
#endif /* def BCM_WARM_BOOT_SUPPORT */
        for (inst=0; inst < TAPS_INST_SEQ; inst++) {
        /* init mem */
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, TP_GLOBAL_CONFIGr, SOC_BLOCK_PORT(unit,inst), 0, &regval ));
            soc_reg_field_set(unit, TP_GLOBAL_CONFIGr, &regval, MEM_INITf, 1 );
            soc_reg32_set(unit, TP_GLOBAL_CONFIGr, SOC_BLOCK_PORT(unit,inst), 0, regval);

        /* turn on tcam scruber, single bit correct, multi-bit invalidate */
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, TP_TCAM_SCAN_DEBUG0r, SOC_BLOCK_PORT(unit,inst), 0, &regval));
            soc_reg_field_set(unit, TP_TCAM_SCAN_DEBUG0r, &regval, TCAM_SCAN_CORRECTf, 1);
            soc_reg32_set(unit, TP_TCAM_SCAN_DEBUG0r, SOC_BLOCK_PORT(unit,inst), 0, regval);

            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, TP_TCAM_SCAN_DEBUG1r, SOC_BLOCK_PORT(unit,inst), 0, &regval));
            soc_reg_field_set(unit, TP_TCAM_SCAN_DEBUG1r, &regval, TCAM_SCAN_WINDOWf, 0);
            soc_reg32_set(unit, TP_TCAM_SCAN_DEBUG1r, SOC_BLOCK_PORT(unit,inst), 0, regval);
        }
#ifdef BCM_WARM_BOOT_SUPPORT
    } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* def BCM_WARM_BOOT_SUPPORT */

    /* Init the taps command caches */
    DQ_INIT(&taps_cached_cmds[unit]);

    /* Initialize autocaching for route insertion */
    _taps_autocache_init(unit); 


    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_driver_uninit
 * Purpose:
 *     clean up
 */
int soc_sbx_caladan3_tmu_taps_driver_uninit(int unit)
{
    sal_free(taps_state[unit]);
    taps_state[unit] = NULL;
    return SOC_E_NONE;
}


#ifdef TAPS_DEBUG_BACKGROUND_INSERT
static void _taps_background_insert_thread(int unit)
{
    int route, rv = SOC_E_NONE;
    uint32 key[2], count = 0, total = 500000;
    uint32 payload[4];
    taps_arg_t arg;
    taps_handle_t taps = NULL;
    dq_p_t elem;
    int default_vrf_done = FALSE;
    int default_vrf = unit & 0xF;
    
    unit = (unit &0xF0)>>4;

    key[0] = 0;
    key[1] = 0;
    payload[0] = 0xdead;
    payload[1] = 0xbeef;
    payload[2] = 0x0;
    payload[3] = 0x0;

    /* we need to make sure each thread use different vrf */

    while (1) {
        sal_sem_take(_taps_background_insert_sem[unit][default_vrf], 10 * MILLISECOND_USEC);
        
        /* check if there is a v4 taps */
        if (taps == NULL) {
            if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)) {
                DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
                    taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
                    if ((taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) &&
                        (taps->param.mode == 2) &&
                        (taps->param.max_capacity_limit >= 500000)) {
                        LOG_CLI((BSL_META_U(unit,
                                            " Found valid taps 0x%p on unit %d !!! \n"), taps, unit));
                        if (total == 0) {
                            /* not specified total, calculate it */
                            total = taps->param.max_capacity_limit/2;
                        }
                        break;
                    } else {
                        taps = NULL;
                    }
                } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);
            }        
        } else {
            if (default_vrf_done == FALSE) {
                arg.taps = taps;
                key[1] = default_vrf;
                arg.key = &key[0];
                arg.length = 16;
                arg.payload = &payload[0];
                arg.cookie = NULL;
                
                /* insert vrf route */
                rv = taps_insert_route(unit, &arg);
                if (SOC_FAILURE(rv)) {
                    LOG_CLI((BSL_META_U(unit,
                                        " Failed to insert vrf route on unit %d, rv %d !!! \n"), unit, rv));                    
                } else {
                    default_vrf_done = TRUE;
                }
            }

            /* insert 100 routes */
            for (route = 0; route < 100; route++) {                
                arg.taps = taps;
                key[0] = default_vrf;
                arg.key = &key[0];                
                arg.length = 48;
                arg.payload = &payload[0];
                arg.cookie = NULL;
                
                if ((count % (total*2)) >= total) {
                    key[1] = count % total;
                    /* update route */
                    rv = taps_delete_route(unit, &arg);
                    if (SOC_FAILURE(rv)) {
                        LOG_CLI((BSL_META_U(unit,
                                            " Failed to delete route 0x%x on unit %d, rv %d !!! \n"), key[1], unit, rv));
                    }
                } else {
                    key[1] = count % total;
                    /* insert route */
                    rv = taps_insert_route(unit, &arg);
                    if (SOC_FAILURE(rv)) {
                        LOG_CLI((BSL_META_U(unit,
                                            " Failed to insert route on unit %d, rv %d !!! \n"), unit, rv));                    
                    }
                }

                count++;
                payload[3]++;
            }
        }
    }
}
#endif

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_driver_init
 * Purpose:
 *     Bring up TAPS drivers
 */
int soc_sbx_caladan3_tmu_taps_driver_init(int unit) 
{
    int rv = SOC_E_NONE;
    static int init_memory_pool = FALSE;
    if (taps_state[unit]) return SOC_E_EXISTS;

    taps_state[unit] = sal_alloc(sizeof(taps_container_t), "taps-container");
    if (!taps_state[unit]) {
        rv = SOC_E_MEMORY;

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to Init Taps driver!!! %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    } else {
        sal_memset(taps_state[unit], 0, sizeof(taps_container_t));

        /* allocate bit map to manage the pool */
        taps_state[unit]->seg_allocator[TAPS_INST_0] = 0;
        taps_state[unit]->seg_allocator[TAPS_INST_1] = 0;
        taps_state[unit]->seg_allocator[TAPS_INST_SEQ] = 0;
        DQ_INIT(&taps_state[unit]->taps_object_list);
    }
    if (soc_property_get(unit, "taps_memory_pool", FALSE)) {
        mem_pool_enable = TRUE;
    }
    if (mem_pool_enable && (!init_memory_pool)) {
        rv = hpcm_trie_node_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to alloc hpcm mem for trie :%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        
        rv = hpcm_dprefix_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to alloc hpcm mem for dprefix:%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        init_memory_pool = TRUE;
    }

#ifdef TAPS_DEBUG_BACKGROUND_INSERT
    /* JOEY debug, create 2 threads that keep insert/delete taps routes */
    _taps_background_insert_sem[unit][0] = sal_sem_create("background insert semphore for vrf 0", sal_sem_BINARY, 0);    

    _taps_background_insert[unit][0] =  sal_thread_create("taps background insert thread for vrf 0",
                                                          SAL_THREAD_STKSZ, TAPS_AUTOCACHE_THREAD_PRIO,
                                                          (void (*)(void*))_taps_background_insert_thread,
                                                          INT_TO_PTR(((unit&0xF)<<4) + 0));

    _taps_background_insert_sem[unit][1] = sal_sem_create("background insert semphore for vrf 1", sal_sem_BINARY, 0);    

    _taps_background_insert[unit][1] =  sal_thread_create("taps background insert thread for vrf 1",
                                                          SAL_THREAD_STKSZ, TAPS_AUTOCACHE_THREAD_PRIO,
                                                          (void (*)(void*))_taps_background_insert_thread,
                                                          INT_TO_PTR(((unit&0xF)<<4) + 1));
#endif

    return rv;
}

int _taps_validate_segment_attr(int unit, 
                                taps_instance_e_t instance, 
                                taps_tcam_layout_e_t tcam_layout,
                                taps_segment_attr_t *seg_attr)
{
    taps_instance_e_t inst;
    dq_p_t elem;
    taps_handle_t taps;

    /* mutex ... */
    /* look for a free segment id */
    /* segment attribute validation */
    /* verify if the offset/entries do not overlap with pre-allocated segments */
    inst = (instance != TAPS_INST_SEQ)?instance:TAPS_INST_0;

    for (;inst < TAPS_INST_SEQ; inst++) {
        if (seg_attr->seginfo[inst].num_entry == 0) {
            continue;
        }

        if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)){
            DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
                taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
                if (taps->param.instance == inst ||
                    taps->param.instance == TAPS_INST_SEQ) {
                    /* verify for no overlap */
                    if ((seg_attr->seginfo[inst].offset >=
                         taps->param.seg_attr.seginfo[inst].offset) &&
                        (seg_attr->seginfo[inst].offset <=
                         (taps->param.seg_attr.seginfo[inst].offset +
                          taps->param.seg_attr.seginfo[inst].num_entry - 1))) {
                        /* verify start address of the segment doesn't fall into
                         * any of existing segments
                         */
                        return SOC_E_PARAM;
                    }
                    
                    if (((seg_attr->seginfo[inst].offset+
                          seg_attr->seginfo[inst].num_entry  - 1) >=
                         taps->param.seg_attr.seginfo[inst].offset) &&
                        ((seg_attr->seginfo[inst].offset+
                          seg_attr->seginfo[inst].num_entry - 1) <=
                         (taps->param.seg_attr.seginfo[inst].offset +
                          taps->param.seg_attr.seginfo[inst].num_entry - 1))) {
                        /* verify end address of the segment doesn't fall into 
                         * any of existing segments
                         */
                        return SOC_E_PARAM;
                    }

                    if ((seg_attr->seginfo[inst].offset + seg_attr->seginfo[inst].num_entry) >=
                        (1<<(soc_reg_field_length(unit, TP_SEGMENT_CONFIG0_Sr, BASEf)+2))) {
                        /* verify end address of the segment doesn't go beyond hardware limitation */
                        return SOC_E_PARAM;
                    }

		    if ((seg_attr->seginfo[inst].offset % (_TAPS_MIN_TCAM_ENTRY_SEG_ / 4)) ||
			(seg_attr->seginfo[inst].num_entry % (_TAPS_MIN_TCAM_ENTRY_SEG_ / 4))) {
			/* verify offset and num_entry are in multiple of minimum allocation units */			
			return SOC_E_PARAM;
		    }
                }
            } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);            
        }
    }
    
    return SOC_E_NONE;
}

/*
 * Allocate a chunk of entries and make sure segment doesn't overlap
 * with existing segments. Return SOC_E_FULL when can not find such
 * chunk.
 */
int _taps_segment_alloc_entries(int unit, 
                                taps_instance_e_t instance, 
                                taps_tcam_layout_e_t tcam_layout,
                                taps_segment_attr_t *seg_attr)
{
    int rv = SOC_E_NONE;
    taps_instance_e_t inst;
    dq_p_t elem;
    taps_handle_t taps;

    inst = (instance != TAPS_INST_SEQ)?instance:TAPS_INST_0;

    for (;inst < TAPS_INST_SEQ; inst++) {
        /* Set to 0 as initial value */
        seg_attr->seginfo[inst].offset = 0;
        if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)){
            /* TAPS object queue traversing */
            DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
                taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
                if (taps->param.instance == inst ||
                    taps->param.instance == TAPS_INST_SEQ) {
                    /* allocate at end of this segment and verify if it will fit */
		    seg_attr->seginfo[inst].offset = (taps->param.seg_attr.seginfo[inst].offset +
						      taps->param.seg_attr.seginfo[inst].num_entry);
                    /* Jump out of the queue traversing */
                    break;
                }
            } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);         
        }
    }
    
    if (SOC_FAILURE(_taps_validate_segment_attr(unit, instance, tcam_layout, seg_attr))) {
        seg_attr->seginfo[instance].offset = -1;  
        rv = SOC_E_FULL;
    }

        return rv;
}
/*
 * Allocate a chunk of entries and make sure segment doesn't overlap
 * with existing segments. Return SOC_E_FULL when can not find such
 * chunk.
 */
int _taps_segment_offset_get_from_master(int unit, 
                                int master_unit,
                                taps_instance_e_t instance, 
                                unsigned int segment,
                                taps_segment_attr_t *seg_attr)
{
    int rv = SOC_E_NONE;
    taps_instance_e_t inst;
    dq_p_t elem;
    taps_handle_t taps;

    if (!DQ_EMPTY(&taps_state[master_unit]->taps_object_list)) {
        /* TAPS object queue traversing */
        DQ_TRAVERSE(&taps_state[master_unit]->taps_object_list, elem) {
            taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
            if (taps->param.instance == instance && taps->segment == segment) {
                /* Corresponding segment on master unit */
                if (taps->param.instance != TAPS_INST_SEQ) {
                    seg_attr->seginfo[instance].offset = taps->param.seg_attr.seginfo[instance].offset;
                } else {
                    for (inst = 0; inst < TAPS_INST_SEQ; inst++) {
                        seg_attr->seginfo[inst].offset = taps->param.seg_attr.seginfo[inst].offset;
                    }
                }
                /* Jump out of the queue traversing */
                break;
            }
        } DQ_TRAVERSE_END(&taps_state[master_unit]->taps_object_list, elem);         
    }
        
    return rv;
}

int _taps_validate_handle(int unit, taps_handle_t handle) 
{
    int rv = SOC_E_NONE;

    if (!taps_state[unit]) return SOC_E_INIT;
    if (!handle) return SOC_E_PARAM;

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, handle->master_unit, handle->param.host_share_table)) {
        /* Taps handle exist in the master unit */
        unit = handle->master_unit;
    }
        
    /* verify handle exist in our taps object handle list */
    rv = SOC_E_NOT_FOUND;
    if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)){
        dq_p_t elem;
        DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
            if (handle ==                                               \
                DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node)) {
                rv = SOC_E_NONE;
                break;
            }
        } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);            
    }
    
    if (SOC_FAILURE(rv)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TAPS Handle Validate Failed %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    return rv;
}

int _taps_validate_init_params(int unit, taps_init_params_t *param)
{
    int rv = SOC_E_PARAM, index=0;

    if (!param) return rv;

    if (!_TAPS_SUPPORTED_INSTANCE_(param->instance)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TAPS Instance not supported %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    /* validate key attributes */
    if(!_TAPS_SUPPORTED_KEY_TYPE_(param->key_attr.type)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TAPS key type not supported %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    if (!_TAPS_SUPPORTED_KEY_SIZE_(param->key_attr.lookup_length) ||
        param->key_attr.vrf_length > TAPS_IPV4_MAX_VRF_SIZE) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d bad key length %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    if (!_TAPS_SUPPORTED_SEARCH_MODE_(param->mode)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d bad search mode %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    if (!_TAPS_SUPPORTED_TCAM_LAYOUT_(param->tcam_layout)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d bad tcam layout %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    if (!_TAPS_SUPPORTED_BBX_FORMAT_(param->sbucket_attr.format)) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d bad sram BBX format %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }


    if ((param->mode == TAPS_OFFCHIP_ALL) && 
        !((_TAPS_SUPPORTED_IPV4_NUM_DDR_PFX_(param->key_attr.type, param->dbucket_attr.num_dbucket_pfx)) ||
      (_TAPS_SUPPORTED_IPV6_NUM_DDR_PFX_(param->key_attr.type, param->dbucket_attr.num_dbucket_pfx)))) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d bad number of dram prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }

    for (index=0; index < TAPS_DDR_TABLE_MAX && param->mode != TAPS_ONCHIP_ALL; index++) {
        int state=0;
        if (!_TAPS_SUPPORTED_DDR_TBL_FLAGS_(param->dbucket_attr.flags[index])) {
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d bad ddr table flag %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            return rv;
        }
        if (param->dbucket_attr.flags[index] == _TAPS_DDR_TBL_FLAG_WITH_ID) {
            if (param->dbucket_attr.table_id[index] >= SOC_SBX_CALADAN3_TMU_MAX_TABLE) {
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d bad tmu table id to reserve %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                return rv;
            }
            soc_sbx_caladan3_tmu_get_table_state(unit, 
                                                 param->dbucket_attr.table_id[index],
                                                 &state); 
            if (state) {
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Table already allocated %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                return rv;
            }
        }

        if (param->mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) break;
    }

    /* validate num prefix on bucket */
    if (param->mode == TAPS_OFFCHIP_ALL) {
        if (param->key_attr.type == TAPS_IPV4_KEY_TYPE) {
        /* V4 */
            if (param->dbucket_attr.num_dbucket_pfx % _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX) {
                if ((param->dbucket_attr.num_dbucket_pfx % 
                     _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX) /
                    _TAPS_DBUCKET_IPV4_128BIT_DBKT_NUM_PFX > 1) {
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Dram bucket prefix must be combination "
                                              " of multiples of 256b:%d entry +"
                                              "128b:%d entry padding %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit,
                                   _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX,
                                   _TAPS_DBUCKET_IPV4_128BIT_DBKT_NUM_PFX,
                                   rv, soc_errmsg(rv)));
                    }
            return rv;
        }
            }
        } else {
            /* V6 */
            if (param->dbucket_attr.num_dbucket_pfx % _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX) {
                if ((param->dbucket_attr.num_dbucket_pfx % 
                     _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX) /
                    _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX > 1) {
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Dram bucket prefix must be combination "
                                              " of multiples of 256b:%d entry +"
                                              "128b:%d entry padding %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit,
                                   _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX,
                                   _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX,
                                   rv, soc_errmsg(rv)));
                    }
            return rv;
        }
            }
        }
    }

    /* segment attribute validation */
    /* verify if the offset/entries do not overlap with pre-allocated segments */
    if (SOC_FAILURE(_taps_validate_segment_attr(unit, param->instance,
                                                param->tcam_layout, &param->seg_attr))) {
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Bad segment attribute %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        return rv;
    }
        
        /* Verify divide_ratio param */
    if (!_TAPS_IS_PARALLEL_MODE_(param->instance)) {
        if (!_TAPS_SEG_DIVIDE_RATIO_CHECK(param->divide_ratio)) {
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d TAPS segment divide ration is incorrect %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            return rv;
        }
    }
    
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_instance_seg_init
 * Purpose:
 *     Segment configuration for specified instance
 */
static int _taps_instance_seg_init(int unit, taps_handle_t taps, 
                                    taps_instance_e_t instance)
{
    uint32 regval = 0;
    uint32 field = 0;

    
    /* TAPS uarch Table 47 */
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, TP_SEGMENT_CONFIG0_Sr, SOC_BLOCK_PORT(unit, instance),
                           taps->segment, &regval));
    soc_reg_field_set( unit, TP_SEGMENT_CONFIG0_Sr, &regval, KEY_SIZEf, 
                       taps->param.key_attr.lookup_length);

    field = ((taps->param.seg_attr.seginfo[instance].num_entry/
              (4 / (1<<taps->param.tcam_layout))) - 1) >> 2;
    soc_reg_field_set( unit, TP_SEGMENT_CONFIG0_Sr, &regval, LIMITf, field);

    field = (taps->param.seg_attr.seginfo[instance].offset) >> 2;
    soc_reg_field_set( unit, TP_SEGMENT_CONFIG0_Sr, &regval, BASEf, field);

    soc_reg32_set( unit, TP_SEGMENT_CONFIG0_Sr, SOC_BLOCK_PORT(unit, instance), 
                   taps->segment, regval);

    SOC_IF_ERROR_RETURN(soc_reg32_get( unit, TP_SEGMENT_CONFIG1_Sr, SOC_BLOCK_PORT(unit, instance),
                                       taps->segment, &regval));

    switch (taps->param.tcam_layout) {
    case TAPS_TCAM_SINGLE_ENTRY:
        field = 1;
        break;
    case TAPS_TCAM_DOUBLE_ENTRY:
        field = 2;
        break;
    case TAPS_TCAM_QUAD_ENTRY:
        field = 3;
        break;
    default:
        field = 0;
        break;
    }

    soc_reg_field_set(unit, TP_SEGMENT_CONFIG1_Sr, &regval, FORMATf, field);
    if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS
        || taps->param.mode == TAPS_ONCHIP_ALL) {
        soc_reg_field_set(unit, TP_SEGMENT_CONFIG1_Sr, 
			  &regval, MAX_PREFIXES_PER_BASE_UNITf, 
			  BASE_UNIT_PREFIX_NUMBER(SOC_SBX_TMU_TAPS_BB_FORMAT_2ENTRIES));
    } else {
        soc_reg_field_set(unit, TP_SEGMENT_CONFIG1_Sr, 
			  &regval, MAX_PREFIXES_PER_BASE_UNITf, 0);
    }
    soc_reg_field_set(unit, TP_SEGMENT_CONFIG1_Sr, &regval, PREFIXES_PER_BUCKETf, 
                      taps->param.dbucket_attr.num_dbucket_pfx *  
                      _TAPS_DBUCKET_BSEL_MULTIPLY_FACTOR *
                      taps->param.sbucket_attr.max_pivot_number);
    soc_reg_field_set( unit, TP_SEGMENT_CONFIG1_Sr, &regval, CTRLf, taps->param.mode);
    soc_reg32_set( unit, TP_SEGMENT_CONFIG1_Sr, SOC_BLOCK_PORT(unit, instance),
                   taps->segment, regval);
    
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_segment_global_base_pointer_set
 * Purpose:
 *     Just for unified mode
 *     Set global_base_pointer in TP_SEGMENT_CONFIG2_S.     
 */
static int _taps_segment_global_base_pointer_set(int unit, taps_handle_t taps, 
                                    taps_instance_e_t instance, uint32 global_base)
{
    uint32 regval = 0;
    
    SOC_IF_ERROR_RETURN(soc_reg32_get( unit, TP_SEGMENT_CONFIG2_Sr, SOC_BLOCK_PORT(unit, instance),
                                           taps->segment, &regval));
    soc_reg_field_set(unit, TP_SEGMENT_CONFIG2_Sr, &regval, GLOBAL_BASE_POINTERf, global_base);
    soc_reg32_set(unit, TP_SEGMENT_CONFIG2_Sr, SOC_BLOCK_PORT(unit, instance),
                taps->segment, regval);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_segment_hw_init
 * Purpose:
 *     Segment definition, hardware configuration
 */
int _taps_segment_hw_init(int unit, taps_handle_t taps)
{
    int rv;
    taps_instance_e_t instance;
    
    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
        /* Parallel mode */
        rv = _taps_instance_seg_init(unit, taps, taps->param.instance);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to configure segment in taps %d :%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, taps->param.instance, rv, soc_errmsg(rv)));
        }
    } else {
        /* Unified mode 
                 * Both of two instances need to be initilized */
        for (instance = TAPS_INST_0; instance < TAPS_INST_SEQ; instance++) {
            rv = _taps_instance_seg_init(unit, taps, instance);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to configure segment in taps %d :%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, instance, rv, soc_errmsg(rv)));
                break;
            }
        }
        if (SOC_SUCCESS(rv)) {
            rv = _taps_segment_global_base_pointer_set(unit, taps, 
                                                TAPS_INST_0, 0);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to configure global base pointer :%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            rv = _taps_segment_global_base_pointer_set(unit, taps, 
                                                TAPS_INST_1, taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to configure global base pointer :%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }
    }
    return rv;
}

int _taps_create_default_entry(int unit, taps_handle_t taps)
{
    int rv=SOC_E_NONE, rv2=SOC_E_NONE;
    unsigned int key[TAPS_MAX_KEY_SIZE_WORDS];
    taps_cookie_t cookie;
    taps_obj_t *obj;

    if (!taps) return SOC_E_PARAM;
    cookie.arg.taps = taps;
    obj = &cookie.obj;
    sal_memset(obj, 0, sizeof(taps_obj_t));
    obj->domain_id = _TAPS_INV_ID_;

    sal_memset(key, 0, sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);

    taps->defpayload = sal_alloc(sizeof(uint32)*_TAPS_PAYLOAD_WORDS_,
                                 "taps-def-entry");
    if (taps->defpayload) {
        if (taps->param.mode == TAPS_ONCHIP_ALL) {
            *taps->defpayload = taps->param.defpayload[0] 
                            & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        } else {
            sal_memcpy(taps->defpayload, taps->param.defpayload,
                   sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);
        }
    } else {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d failed to create def entry:%d :%s !!!\n"), 
                  FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
       goto _def_error;
    }

    /* Setup segment in hardware */
    rv = _taps_segment_hw_init(unit, taps);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to init segment in hardware :%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    }
        
    /* allocate a tcam domain, insert default entry onto the tph */
    rv = taps_tcam_create(unit, taps, &taps->tcam_hdl);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to create tcam handle:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    } 
    
    /* create work group */
    rv = taps_work_group_create(unit, taps->wqueue[unit], 
                                _TAPS_DEFAULT_WGROUP_,
                                &obj->wgroup[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to create work group:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    }             
    
    /*=== trie insert done, find a tcam free entry to insert */
    rv = taps_tcam_entry_alloc(unit, taps->tcam_hdl, obj->wgroup, 
                   0, &obj->tcam_entry_id);
    if (SOC_FAILURE(rv)) {
        /* TCAM full, we should prevent this from happening by checking TCAM use counters */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam segment %d failed to alloc entry for length %d\n"),
                   FUNCTION_NAME(), unit, taps->tcam_hdl->segment, 0));
    }

    /* alloc sbucket domain */
    rv = taps_bucket_alloc(unit, taps, obj->tcam_entry_id, &obj->domain_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate tcam domain:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    } 
    
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* only need to insert prefix in dbucket in mode 2*/
        /* create a wild char spviot, dbucket */
        rv = taps_dbucket_create(unit, taps, obj->wgroup[unit],
                                 obj->domain_id, 
                                 _SBKT_WILD_CHAR_RSVD_POS_,
                                 &obj->wdbh); 
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate dram bucket:%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _def_error;
        }
        
        obj->ndbh = obj->wdbh;
        
        /* insert wildcard (*) prefix into dbucket */
        rv = taps_dbucket_insert_prefix(unit, taps, obj->wgroup,
                                        obj->wdbh, &key[0], 0, 
                                        taps->param.defpayload, NULL, 0, &obj->dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate prefix:%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _def_error;
        }
    }
    
    /* create sbucket */
    rv = taps_sbucket_create(unit, taps, obj->wgroup,
                             obj->domain_id, obj->wdbh,
                             TRUE, 0, &obj->nsbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate sbucket:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    } 

    /* insert wildcard (*) entry into tcam */
    rv = taps_tcam_insert_pivot(unit, taps->tcam_hdl,
                                obj->wgroup, &key[0], 0, &key[0],
                                obj->nsbh, &obj->tph, obj->tcam_entry_id, TRUE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate tcam pivot:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _def_error;
    } 

    /* all previous calls generate hardware commands, commit all of those commands
     * into hardware.
     */
    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_ALL);
    if (SOC_FAILURE(rv)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d failed to commit :%d :%s !!!\n"), 
                  FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
       goto _def_error;
    }
    
 _def_error: 
    if (SOC_FAILURE(rv)) {

        if (taps->defpayload) {
            sal_free(taps->defpayload);
            taps->defpayload = NULL;
        }

        if ((obj->domain_id != _TAPS_INV_ID_) && (SOC_SUCCESS(rv2))) {
            rv2 = taps_bucket_free(unit, taps, obj->domain_id);     
        }
        if ((obj->dph) && (SOC_SUCCESS(rv2))) {
            rv2 = taps_dbucket_delete_prefix(unit, taps, obj->wgroup,
                                       obj->ndbh, obj->dph);
        }
        if ((obj->wdbh) && (SOC_SUCCESS(rv2))) {
            rv2 = taps_dbucket_destroy(unit, taps, 
                                 obj->wgroup[unit], obj->wdbh);
        }
        if ((obj->tph) && (SOC_SUCCESS(rv2))) {
            rv2 = taps_tcam_delete_pivot(unit, taps->tcam_hdl,
                                   obj->wgroup, obj->tph);
        }
        if ((taps->tcam_hdl) && (SOC_SUCCESS(rv2))) {
            taps_tcam_destroy(unit, taps->tcam_hdl);
        }

    }

    /* destroy work group */
    if (obj->wgroup[unit]) taps_work_group_destroy(unit, obj->wgroup[unit]);

    if (SOC_FAILURE(rv2)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d failed to clean up after error :%d :%s !!!\n"), 
                  FUNCTION_NAME(), unit, rv, soc_errmsg(rv2)));
    }
    return rv;
}

/* Allocate DDR tables based on search modes */
int _taps_ddr_table_manage(int unit, taps_handle_t taps, uint8 alloc)
{
    soc_sbx_caladan3_table_attr_t attr[TAPS_DDR_TABLE_MAX];
    taps_instance_segment_attr_t *seginfo;
    int index=0, rv=SOC_E_NONE;
    uint32 regval=0;

    if (!taps) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_ONCHIP_ALL) return SOC_E_NONE; /* nop */

    /* no support for sequential mode, add when required */
    if (!_TAPS_SUPPORTED_INSTANCE_(taps->param.instance)) return SOC_E_PARAM;

    sal_memset(attr, 0, sizeof(soc_sbx_caladan3_table_attr_t) * TAPS_DDR_TABLE_MAX);
    seginfo = &taps->param.seg_attr.seginfo[taps->param.instance];

    if (alloc) {
        for (index=0; index < TAPS_DDR_TABLE_MAX &&
                 taps->param.dbucket_attr.flags[index] == _TAPS_DDR_TBL_FLAG_WITH_ID; index++) {
            attr[index].id = taps->param.dbucket_attr.table_id[index];
            attr[index].flags = SOC_SBX_TMU_TABLE_FLAG_WITH_ID;
            if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) break;
        }

        /* allocate dbucket prefix table based on search mode */
        if (taps->param.mode == TAPS_OFFCHIP_ALL) {
            /* off chip prefix and payload */
            if (taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
                /* IPV4 */
                /* dbucket prefix table */
                if (taps->param.dbucket_attr.num_dbucket_pfx / _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX) {
                    attr[TAPS_DDR_PREFIX_TABLE].entry_size_bits =       \
                        (taps->param.dbucket_attr.num_dbucket_pfx / 
                         _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX) * 256;
                }
                
                if (taps->param.dbucket_attr.num_dbucket_pfx % _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX) {
                    assert(taps->param.dbucket_attr.num_dbucket_pfx % 
                           _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX ==
                           _TAPS_DBUCKET_IPV4_128BIT_DBKT_NUM_PFX);
                    attr[TAPS_DDR_PREFIX_TABLE].entry_size_bits += 128;
                }
                
                /* table size is factor of number of tcam & sram pivots */
                attr[TAPS_DDR_PREFIX_TABLE].num_entries =  seginfo->num_entry * \
                    taps->param.sbucket_attr.max_pivot_number *         \
                    _TAPS_DBUCKET_BSEL_MULTIPLY_FACTOR;
                
                attr[TAPS_DDR_PREFIX_TABLE].lookup =  SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY;
                
                /* dbucket payload table */
                attr[TAPS_DDR_PAYLOAD_TABLE].entry_size_bits = _TAPS_DDR_V4_PAYLOAD_SIZE;
                attr[TAPS_DDR_PAYLOAD_TABLE].lookup = SOC_SBX_TMU_LKUP_DM_119;      
                attr[TAPS_DDR_PAYLOAD_TABLE].num_entries = taps->param.dbucket_attr.num_dbucket_pfx *
                    attr[TAPS_DDR_PREFIX_TABLE].num_entries;
                
                rv = soc_sbx_caladan3_tmu_table_alloc(unit, &attr[TAPS_DDR_PREFIX_TABLE]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed allocate dram bucket table :%d :%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                } 
            } else {
                /* IPV6 */
                /* dbucket prefix table */
                if (taps->param.dbucket_attr.num_dbucket_pfx / _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX) {
                    attr[TAPS_DDR_PREFIX_TABLE].entry_size_bits =       \
                        (taps->param.dbucket_attr.num_dbucket_pfx / 
                         _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX) * 256;
                }
                
                if (taps->param.dbucket_attr.num_dbucket_pfx % _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX) {
                    assert(taps->param.dbucket_attr.num_dbucket_pfx % 
                           _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX ==
                           _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX);
                    attr[TAPS_DDR_PREFIX_TABLE].entry_size_bits += 128;
                }

                /* table size is factor of number of tcam & sram pivots */
                attr[TAPS_DDR_PREFIX_TABLE].num_entries =  seginfo->num_entry *  \
                                                          taps->param.sbucket_attr.max_pivot_number *   \
                                                          _TAPS_DBUCKET_BSEL_MULTIPLY_FACTOR;

                attr[TAPS_DDR_PREFIX_TABLE].lookup =  SOC_SBX_TMU_LKUP_TAPS_IPV6_SUB_KEY;

                /* dbucket payload table */
                attr[TAPS_DDR_PAYLOAD_TABLE].entry_size_bits = _TAPS_DDR_V6_PAYLOAD_SIZE;
                attr[TAPS_DDR_PAYLOAD_TABLE].lookup = SOC_SBX_TMU_LKUP_DM_247;      
                attr[TAPS_DDR_PAYLOAD_TABLE].num_entries = taps->param.dbucket_attr.num_dbucket_pfx *
                    attr[TAPS_DDR_PREFIX_TABLE].num_entries;
                
                rv = soc_sbx_caladan3_tmu_table_alloc(unit, &attr[TAPS_DDR_PREFIX_TABLE]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed allocate dram bucket table :%d :%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }       
            }
        } else if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
            /* on-chip prefix, off-chip payload */
            if (taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
                attr[TAPS_DDR_PAYLOAD_TABLE].entry_size_bits = _TAPS_DDR_V4_PAYLOAD_SIZE;
                attr[TAPS_DDR_PAYLOAD_TABLE].lookup = SOC_SBX_TMU_LKUP_DM_119;
                attr[TAPS_DDR_PAYLOAD_TABLE].num_entries = seginfo->num_entry * \
                    _MAX_SBUCKET_PIVOT_PER_BB_ *                        \
                    (1 << (taps->param.tcam_layout));
            } else {
                attr[TAPS_DDR_PAYLOAD_TABLE].entry_size_bits = _TAPS_DDR_V6_PAYLOAD_SIZE;
                attr[TAPS_DDR_PAYLOAD_TABLE].lookup = SOC_SBX_TMU_LKUP_DM_247;
                attr[TAPS_DDR_PAYLOAD_TABLE].num_entries = seginfo->num_entry * \
                    _MAX_SBUCKET_PIVOT_PER_BB_ *                        \
                    (1 << (taps->param.tcam_layout));
            }  
        } else {
            /* on-chip prefix and payload */
            rv = SOC_E_PARAM;
        }
        
        /* allocate dbucket payload table and connect the prefix table with paylod table (NEXT_TABLE field) */
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_tmu_table_alloc(unit, &attr[TAPS_DDR_PAYLOAD_TABLE]);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed allocate dram payload table :%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                
                /* free back bucket table */
                if (taps->param.mode == TAPS_OFFCHIP_ALL) {
                    soc_sbx_caladan3_tmu_table_free(unit, attr[TAPS_DDR_PREFIX_TABLE].id);
                }
            }
#ifdef BCM_WARM_BOOT_SUPPORT
            if (!SOC_WARM_BOOT(unit)) {
                /* only manipulate hardware if not warm boot mode */
#endif /* def BCM_WARM_BOOT_SUPPORT */
                if (taps->param.mode == TAPS_OFFCHIP_ALL) {
                    SOC_IF_ERROR_RETURN(READ_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit,
                                                                            attr[TAPS_DDR_PREFIX_TABLE].id, &regval));
                    soc_reg_field_set(unit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr,
                                      &regval, NEXT_TABLEf,
                                      attr[TAPS_DDR_PAYLOAD_TABLE].id);
                    SOC_IF_ERROR_RETURN(WRITE_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr[TAPS_DDR_PREFIX_TABLE].id, regval));
                    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit,
                                                                            attr[TAPS_DDR_PREFIX_TABLE].id, &regval));
                    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr,
                                      &regval, NEXT_TABLEf,
                                      attr[TAPS_DDR_PAYLOAD_TABLE].id);
                    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr[TAPS_DDR_PREFIX_TABLE].id, regval));

                } else {
                    SOC_IF_ERROR_RETURN(READ_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit,
                                                                            attr[TAPS_DDR_PAYLOAD_TABLE].id, &regval));
                    soc_reg_field_set(unit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr,
                                      &regval, NEXT_TABLEf,
                                      attr[TAPS_DDR_PAYLOAD_TABLE].id);
                    SOC_IF_ERROR_RETURN(WRITE_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr[TAPS_DDR_PAYLOAD_TABLE].id, regval));
                    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit,
                                                                            attr[TAPS_DDR_PAYLOAD_TABLE].id, &regval));
                    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr,
                                      &regval, NEXT_TABLEf,
                                      attr[TAPS_DDR_PAYLOAD_TABLE].id);
                    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr[TAPS_DDR_PAYLOAD_TABLE].id, regval));
                }
#ifdef BCM_WARM_BOOT_SUPPORT
            } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* def BCM_WARM_BOOT_SUPPORT */
            
        }

        if (SOC_SUCCESS(rv)) {
            for (index=0; index < TAPS_DDR_TABLE_MAX; index++) {
                taps->param.dbucket_attr.table_id[index] = attr[index].id;
                if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) break;
            }
        }
    } else {
	/* free both dbucket prefix and payload tables */
        for (index=0; index < TAPS_DDR_TABLE_MAX && SOC_SUCCESS(rv); index++) {
            rv = soc_sbx_caladan3_tmu_table_free(unit, 
                                                 taps->param.dbucket_attr.table_id[index]);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed free dram-type[%d]"
                                      " table-id:%d  :%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, 
                           index, taps->param.dbucket_attr.table_id[index],
                           rv, soc_errmsg(rv)));
            }
            if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) break;
        }        
    }

    return rv;
}

/*
 *
 * Function:
 *     _taps_tcam_num_entry_calculate
 * Purpose:
 *     Init the number of tcam entry
 */
int _taps_tcam_num_entry_calculate(int unit,
                                taps_handle_t taps,
                                taps_init_params_t *param) 
{
    int rv = SOC_E_NONE;
    int max_tcam_num_entries;
    uint32 entry_units;
    uint32 avg_num_pfx_per_domain;
    taps_instance_segment_attr_t *seginfo;

    if (!taps || !param) {
        return SOC_E_PARAM;
    }

    if (_TAPS_IS_PARALLEL_MODE_(param->instance)) {
        /* Parallel mode */
        max_tcam_num_entries = TAPS_TCAM_NUM_ENTRIES;
    } else {
        /* Unified mode 
	 * max capacity should be doubled 
	 */
        max_tcam_num_entries = TAPS_TCAM_NUM_ENTRIES * 2;
    }
    
    seginfo = &param->seg_attr.seginfo[param->instance];
    
    entry_units = _TAPS_MIN_TCAM_ENTRY_SEG_ / (1 << param->tcam_layout);

    if (seginfo->num_entry != 0) {
        /* align to min entries required */
        if (seginfo->num_entry < entry_units) {
            seginfo->num_entry = entry_units;
        } else if (seginfo->num_entry % entry_units) {
            seginfo->num_entry += (entry_units - (seginfo->num_entry % entry_units));
        }
        if (seginfo->num_entry > max_tcam_num_entries) {       
            seginfo->num_entry = max_tcam_num_entries;
        }
    } else {
        /* determine the num_entry based on capacity.
         * assuming 50% efficiency in both dbucket and sbucket 
         * NOTE: sbucket size need to be adjusted when key_shift is supported
         */
        if (param->mode == TAPS_OFFCHIP_ALL) {
            avg_num_pfx_per_domain = ((param->dbucket_attr.num_dbucket_pfx * 2) *
				      taps->param.sbucket_attr.max_pivot_number) / 4;
        } else {
            /* Key_shift is supportted for mode 0/1, 
	     * avg_num_pfx_per_domain is the max pivot number with 100% efficiency.
	     * Min value is max_pivot_number, and max value is _MAX_SBUCKET_PIVOT_PER_BB_
	     * use the mean number of these two values since key_shift is supportted
	     */
            avg_num_pfx_per_domain = (param->sbucket_attr.max_pivot_number + _MAX_SBUCKET_PIVOT_PER_BB_ * (1 << (param->tcam_layout))) / 2;
        }
        
        seginfo->num_entry = param->max_capacity_limit / avg_num_pfx_per_domain;
        
        /* align to min entries required */
        if (seginfo->num_entry < entry_units) {
            seginfo->num_entry = entry_units;
        } else if (seginfo->num_entry % entry_units) {
            seginfo->num_entry += (entry_units - (seginfo->num_entry % entry_units));
        }
        if (seginfo->num_entry > max_tcam_num_entries) {       
            seginfo->num_entry = max_tcam_num_entries;
        }
    }
    
    if (!_TAPS_IS_PARALLEL_MODE_(param->instance)) {
        /* Unified mode. Split entries to two taps*/
        param->seg_attr.seginfo[TAPS_INST_0].num_entry = 
                        param->seg_attr.seginfo[TAPS_INST_SEQ].num_entry * 
                        param->divide_ratio / ceiling_ratio;
	if (param->seg_attr.seginfo[TAPS_INST_0].num_entry % entry_units) {
	    param->seg_attr.seginfo[TAPS_INST_0].num_entry += (entry_units - param->seg_attr.seginfo[TAPS_INST_0].num_entry % entry_units);
	}

        param->seg_attr.seginfo[TAPS_INST_1].num_entry = 
                        param->seg_attr.seginfo[TAPS_INST_SEQ].num_entry
                        - param->seg_attr.seginfo[TAPS_INST_0].num_entry;
        if (param->seg_attr.seginfo[TAPS_INST_0].num_entry > TAPS_TCAM_NUM_ENTRIES) {
            param->seg_attr.seginfo[TAPS_INST_0].num_entry = TAPS_TCAM_NUM_ENTRIES;
        }
        
        if (param->seg_attr.seginfo[TAPS_INST_1].num_entry > TAPS_TCAM_NUM_ENTRIES) {
            param->seg_attr.seginfo[TAPS_INST_1].num_entry = TAPS_TCAM_NUM_ENTRIES;
        }
    } 

    return rv;
}

/*
 *
 * Function:
 *     taps_insert_default_entry_for_slave_unit
 * Purpose:
 *     Insert default entry(0/0) in the LPM table of slave unit. 
 *	   The taps handle have created by the master unit
 */
int taps_insert_default_entry_for_slave_unit(int unit, taps_handle_t taps)
{
    int rv;
    taps_cookie_t cookie;
    taps_obj_t *obj;

    if (taps == NULL) {
        return SOC_E_PARAM;
    }

    taps->slave_units[taps->num_slaves] = unit;
    taps->num_slaves++;
    cookie.arg.taps = taps;
    obj = &cookie.obj;
    sal_memset(obj, 0, sizeof(taps_obj_t));
    
	/* Setup segment in hardware */
    rv = _taps_segment_hw_init(unit, taps);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to init segment in hardware :%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto error_insert;
    }

	/* Allocate work queue */
    rv = taps_work_queue_init(unit, &taps->wqueue[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to initialize tcam instance %d, %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, taps->param.instance, rv, soc_errmsg(rv)));
        goto error_insert;     
    }

    /* create work group */
    rv = taps_work_group_create(unit, taps->wqueue[unit],
                                _TAPS_DEFAULT_WGROUP_,
                                &obj->wgroup[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to create work group:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto error_insert;
    }
    
	/* Insert default entry in dbucket */
    if (taps->param.mode != TAPS_ONCHIP_ALL) {
        
        /* Allocate dram tables */
        rv = _taps_ddr_table_manage(unit, taps, TRUE);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate dram tables :%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto error_insert;
        }
        
        rv = taps_dbucket_insert_default_entry(unit, taps, obj->wgroup[unit]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to insert default entry in dram:%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto error_insert;
        }
    } 

	/* Insert default entry in sbucket */
    rv = taps_sbucket_insert_default_entry(unit, taps, obj->wgroup[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to insert default entry in sram:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto error_insert;
    }

	/* Insert default entry in tcam */
    rv = taps_tcam_insert_default_entry(unit, taps, obj->wgroup[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to insert default entry in tcam:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto error_insert;
    }
    
    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_ALL);
    if (SOC_FAILURE(rv)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d failed to commit :%d :%s !!!\n"), 
                  FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
       goto error_insert;
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        rv = taps_work_group_destroy(unit, obj->wgroup[unit]);
    }

error_insert:
    if (SOC_FAILURE(rv)) {
        if (obj->wgroup[unit]) taps_work_group_destroy(unit, obj->wgroup[unit]);
        if (taps->wqueue[unit]) taps_work_queue_destroy(unit, taps->wqueue[unit]);
        if (taps->param.mode != TAPS_ONCHIP_ALL) _taps_ddr_table_manage(unit, taps, FALSE);
    }
    return rv;
}

int taps_handle_get_from_master_unit(int unit, int master_unit, taps_init_params_t *param, 
                                    taps_handle_t *handle) 
{
    int rv;
    taps_t taps;
    taps_handle_t master_taps;
    dq_p_t elem;
    
    if (!handle) {
        return SOC_E_PARAM;
    }
	sal_memset(&taps, 0, sizeof(taps_t));
    sal_memcpy(&taps.param, param, sizeof(taps_init_params_t));

    /* allocate a taps segment */
    rv = taps_segment_alloc(unit, &taps);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to allocate taps segment :%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    rv = SOC_E_NOT_FOUND;
    if (!DQ_EMPTY(&taps_state[master_unit]->taps_object_list)){
        /* TAPS object queue traversing */
        DQ_TRAVERSE(&taps_state[master_unit]->taps_object_list, elem) {
            master_taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
            if ((master_taps->param.instance == param->instance)
                 && (master_taps->segment == taps.segment)) {
                /* The segment of master and slave should be the same */
    		    *handle = master_taps;
                rv = SOC_E_NONE;
                /* Jump out of the queue traversing */
                break;
            }
        } DQ_TRAVERSE_END(&taps_state[master_unit]->taps_object_list, elem);         
    }

    if (SOC_FAILURE(rv)) {
        rv = taps_segment_free(unit, param->instance, taps.segment);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to free taps segment %d :%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, taps.segment, rv, soc_errmsg(rv)));
        }
    }
    return rv;
}
/*
 *
 * Function:
 *     taps_create
 * Purpose:
 *     allocate a taps instance
 */
int taps_create(int unit, taps_init_params_t *param, taps_handle_t *handle)
{
    int rv = SOC_E_NONE;
    taps_handle_t taps=NULL;
    int master_unit = SOC_SBX_MASTER(unit);

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, master_unit, param->host_share_table)) {
        /* Get master's taps handle*/
        rv = taps_handle_get_from_master_unit(unit, master_unit, param, handle);  
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to get master's taps hanele %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } else {
            /* Sync cache status with master */
            taps_set_caching(unit, _taps_get_caching(master_unit));
            /* Insert default entry in hardware for slave unit */
#ifdef BCM_WARM_BOOT_SUPPORT
            if (!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */
                rv = taps_insert_default_entry_for_slave_unit(unit, *handle);
#ifdef BCM_WARM_BOOT_SUPPORT
            } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* BCM_WARM_BOOT_SUPPORT */
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to insert default entry for slave %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            } 
        }
        /* End for slave unit */
        return rv;
    } 
    
    /* Forcing tcam_layout to be QUAD entry */
    param->tcam_layout = TAPS_TCAM_QUAD_ENTRY;

    /* validate the init params */
    /* on the process walk through the taps list to validate segment allocation */

    /* driver decides the bbx format for ipv4/v6 key type, instead of allow user input
     * we might revisit this when driver is flexible enough to handle all cases.
     */
    if (param->key_attr.length == TAPS_32BITS_KEY_SIZE) {
        param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_2ENTRIES;
    } else if (param->key_attr.length == TAPS_IPV4_KEY_SIZE) {
        param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES;
    } else if (param->key_attr.length == TAPS_64BITS_KEY_SIZE) {
        param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_4ENTRIES;
    } else if (param->key_attr.length == TAPS_96BITS_KEY_SIZE) {
        param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_6ENTRIES;
    } else if (param->key_attr.length == TAPS_IPV6_KEY_SIZE) {
        param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES;
    }
    param->sbucket_attr.max_pivot_number = BB_PREFIX_NUMBER(param->sbucket_attr.format) * (1 << (param->tcam_layout));

    if (param->key_attr.length <= TAPS_IPV4_KEY_SIZE) {
        param->key_attr.lookup_length = TAPS_IPV4_KEY_SIZE;
    } else { 
        param->key_attr.lookup_length = TAPS_IPV6_KEY_SIZE;
    }

    if (param->seg_attr.seginfo[param->instance].num_entry <= TAPS_TCAM_NUM_ENTRIES) {
        rv = _taps_validate_init_params(unit, param);
        if (SOC_FAILURE(rv)) return rv;
    }

    /* create a taps instance */
    taps = sal_alloc(sizeof(taps_t),"taps-mgr");

    if (taps) {
        sal_memset(taps, 0, sizeof(taps_t));
        sal_memcpy(&taps->param, param, sizeof(taps_init_params_t));
        /* allocate work queue manager */
        rv = taps_work_queue_init(unit, &taps->wqueue[unit]);
    } else {
        rv = SOC_E_MEMORY;
    }

    if (SOC_FAILURE(rv)) {
        goto taps_create_error;
    }

    taps->master_unit = unit;
    taps->num_slaves = 0;

    if (param->instance == TAPS_INST_0) {
        taps->hwinstance = 1;
    } else if (param->instance == TAPS_INST_1) {
        taps->hwinstance = 2;
    } else {
        taps->hwinstance = 3;
    }

    /* allocate a taps segment */
    rv = taps_segment_alloc(unit, taps);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            " Failed to allocate taps segment !!! \n")));
        goto taps_create_error;
    }
    
    rv = _taps_tcam_num_entry_calculate(unit, taps, param);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to initialize tcam instance %d, %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, param->instance, rv, soc_errmsg(rv)));
        goto taps_create_error;     
    }

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, master_unit, TRUE)) {
        /* Slave unit, get segment offset from master */
        rv = _taps_segment_offset_get_from_master(unit, master_unit, param->instance, taps->segment, &param->seg_attr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate taps segment tcam entries %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto taps_create_error;    
        }
    } else {
        /* allocate the num_entry using seginfo of existing segments, calcuate the offset */
        rv = _taps_segment_alloc_entries(unit, param->instance, param->tcam_layout, &param->seg_attr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate taps segment tcam entries %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto taps_create_error;    
        }
    }

    sal_memcpy(&taps->param.seg_attr, &(param->seg_attr),
           sizeof(taps_segment_attr_t));    

    /* allocate bit map to manage the pool */
    taps->allocator = sal_alloc(SHR_BITALLOCSIZE(param->seg_attr.seginfo[param->instance].num_entry),
                            "taps-domain-bmap");
    if (taps->allocator) {
        sal_memset(taps->allocator, 0, 
                SHR_BITALLOCSIZE(param->seg_attr.seginfo[param->instance].num_entry));
    } else {
        goto taps_create_error;
    } 

    if ((param->mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) 
        || (param->mode == TAPS_OFFCHIP_ALL)) {
        /* allocate dram tables */
        rv = _taps_ddr_table_manage(unit, taps, TRUE);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate dram tables :%d :%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto taps_create_error;
        }
    }

    taps->taps_mutex = sal_mutex_create("TAPS MUTEX");
    if (taps->taps_mutex == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d failed to create taps mutex :%d :%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto taps_create_error;
    }

    
    /* initialize default entry 0/0 */
#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */
        rv = _taps_create_default_entry(unit, taps);
#ifdef BCM_WARM_BOOT_SUPPORT
    } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* BCM_WARM_BOOT_SUPPORT */

 taps_create_error:
    if (SOC_FAILURE(rv)) {
        if (taps) {
            if (taps->wqueue[unit]) taps_work_queue_destroy(unit, taps->wqueue[unit]);
            if (taps->defpayload) sal_free(taps->defpayload);
            if (taps->allocator) sal_free(taps->allocator);
            if (taps->segment != _TAPS_MAX_SEGMENT_) taps_segment_free(unit, taps->param.instance, taps->segment);
            if ((param->mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) 
                || (param->mode == TAPS_OFFCHIP_ALL)) {
                _taps_ddr_table_manage(unit, taps, FALSE);
            }
            if (taps->taps_mutex != NULL) {
                sal_mutex_destroy(taps->taps_mutex);
            }
            sal_free(taps);
        }
    } else {
        *handle = taps;
        /* hang it to taps list on global cfg */
        DQ_INSERT_HEAD(&taps_state[unit]->taps_object_list, &taps->taps_list_node);
    } 

    if (param->key_attr.type == TAPS_IPV6_KEY_TYPE) 
      if(param->instance == 0)
    ipv6da_taps[unit] = taps;

    return rv;
}

/* call back function for walk through dbucket & destroy all prefix */
static int _taps_dbucket_destroy(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie;

    if (!dph || !user_data) return SOC_E_PARAM;
    cookie = (taps_cookie_t*)user_data;

    rv =  taps_dbucket_delete_prefix(cookie->unit, 
                                     cookie->arg.taps, cookie->obj.wgroup,
                                     cookie->obj.sph->dbh, dph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to destroy dram prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
    } 

    return rv;
}

/* call back function for walk through the sram bucket and destroy all pivots */
static int _taps_sbucket_destroy(taps_spivot_handle_t sph, void *user_data)
{
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie;
    unsigned int pivot_id=0;

    if (!sph || !user_data) return SOC_E_PARAM;
    cookie = (taps_cookie_t*)user_data;

    cookie->obj.sph = sph;
    if (cookie->arg.taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* destroy dbucket */
        rv = taps_dbucket_destroy_traverse(cookie->unit, 
                               cookie->arg.taps, cookie->obj.wgroup,
                               sph->dbh, cookie, 
                               _taps_dbucket_destroy);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Error destroying dbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        } else {
            rv = taps_dbucket_destroy(cookie->unit, 
                                      cookie->arg.taps, cookie->obj.wgroup[cookie->unit],
                                      cookie->obj.sph->dbh);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: unit %d Error destroying dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
            }
        }
    }
#if 0
        _taps_free_work_queue(cookie->unit, cookie, _TAPS_COMMIT_FLAG_DRAM);
#endif
    /* Note: DRAM can be skipped from hardware update since it will be initalized
     * when allocated later */
    /* skip wild * pivot since they are destroyed when sbucket is destroyed &
     * managed internally within sbucket object */
    if (sph != cookie->obj.tph->sbucket->wsph) {
        pivot_id = sph->index;
        rv = taps_sbucket_delete_pivot(cookie->unit, 
                                       cookie->arg.taps, cookie->obj.wgroup,
                                       cookie->obj.tph->sbucket, sph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Error destroying spivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        }

        /* free back pivot id */
        rv = taps_sbucket_pivot_id_free(cookie->unit, cookie->arg.taps, 
                    cookie->obj.tph->sbucket, pivot_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Error freeing spivot id %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        }
    }
    return rv;
}

/* call back function for through the tcam entries and destroy all objects in the tcam domain */
static int _taps_tcam_destroy(taps_tcam_pivot_handle_t tph, void *user_data)
{
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie;

    if (!tph || !user_data) return SOC_E_PARAM;

    cookie = (taps_cookie_t*)user_data;
    cookie->obj.tph = tph;

    rv = taps_sbucket_destroy_traverse(cookie->unit, cookie->arg.taps, 
                               cookie->obj.wgroup,
                               cookie->obj.tph->sbucket,
                               cookie, _taps_sbucket_destroy);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d Error traversing sbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
    } else {
        
        rv = taps_sbucket_destroy(cookie->unit, cookie->arg.taps, 
                                  cookie->obj.wgroup,
                                  cookie->obj.tph->sbucket);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Error destroying sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        }

        rv = taps_tcam_delete_pivot(cookie->unit, cookie->arg.taps->tcam_hdl,
                                    cookie->obj.wgroup, tph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Error destroying tph %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        }

        rv = _taps_commit(cookie->unit, cookie, 
                          _TAPS_COMMIT_FLAG_DRAM|
                          _TAPS_COMMIT_FLAG_SRAM|
                          _TAPS_COMMIT_FLAG_TCAM);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to commit %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        }

    }

    return rv;
}

/*
 *
 * Function:
 *     taps_destroy
 * Purpose:
 *     destroys taps instance
 */

int taps_destroy(int unit, taps_handle_t taps)
{
    int rv = SOC_E_NONE;
    int slave_idx = 0, slave_unit;
    taps_cookie_t cookie;

    if (!taps) {
        return SOC_E_PARAM;
    }

    sal_memset(&cookie, 0, sizeof(cookie));
    cookie.arg.taps = taps;
    cookie.unit = unit;

    SOC_IF_ERROR_RETURN(taps_work_group_create_for_all_devices(unit, taps, taps->wqueue, 
                                            _TAPS_DEFAULT_WGROUP_,
                                            taps->param.host_share_table,
                                            cookie.obj.wgroup));

    /* traverse tcam trie & destroy all associated objects */
    rv = taps_tcam_destroy_traverse(unit, taps, taps->tcam_hdl, 
                            &cookie, _taps_tcam_destroy);

    taps_tcam_destroy(unit, taps->tcam_hdl); 


    /* Make sure all commands sent to the hardware */
        
    SOC_IF_ERROR_RETURN(taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                              taps->master_unit, taps->param.host_share_table)));

    taps_segment_free(unit, taps->param.instance, taps->segment);
    if (taps->param.mode == TAPS_OFFCHIP_ALL 
        || taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        _taps_ddr_table_manage(unit, taps, FALSE);
    }

    taps_work_group_destroy(unit, cookie.obj.wgroup[unit]);
    taps_work_queue_destroy(unit, taps->wqueue[unit]);
    
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table)) {
        while (slave_idx < taps->num_slaves) {
            slave_unit = taps->slave_units[slave_idx];            
			/* Release DRAM table */
            if (taps->param.mode == TAPS_OFFCHIP_ALL 
                || taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
                _taps_ddr_table_manage(slave_unit, taps, FALSE);
            }

			/* Destroy work group and work queue */
            taps_work_group_destroy(slave_unit, cookie.obj.wgroup[slave_unit]);
            taps_work_queue_destroy(slave_unit, taps->wqueue[slave_unit]);
            ++slave_idx;
        }
    }
    
    /* remove from taps list */
    DQ_REMOVE(&taps->taps_list_node);
    if (taps->defpayload) sal_free(taps->defpayload);
    if (taps->allocator) sal_free(taps->allocator);
    if (taps->taps_mutex != NULL) {
        sal_mutex_destroy(taps->taps_mutex);
    }

    sal_free(taps);
    return rv;
}

int _taps_validate_args(int unit, taps_arg_t *arg)
{
    if (!arg) return SOC_E_PARAM;
    if (!arg->taps || !arg->key) return SOC_E_PARAM;

    SOC_IF_ERROR_RETURN(_taps_validate_handle(unit, arg->taps));

    if (arg->length > arg->taps->param.key_attr.lookup_length) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Invalid key length:%d Supported:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, arg->length,
                   arg->taps->param.key_attr.lookup_length, soc_errmsg(SOC_E_PARAM)));
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_find_prefix_objects
 * Purpose:
 *     finds all associated objects for a given prefix
 * Note:
 *     user can skip tcam trie search if specify obj->tph
 *     user can skip sbucket trie search if specify obj->sph
 */
int _taps_find_prefix_objects(int unit, taps_arg_t *arg, taps_obj_t *obj)
{
    int rv = SOC_E_NONE;

    if (!arg || !obj) return SOC_E_PARAM;
    if (!arg->taps || !arg->key) return SOC_E_PARAM;
    
    /* search tcam trie, skip this step if user specify obj->tph */
    if (obj->tph == NULL) {
        rv = taps_tcam_lookup_prefix(unit, arg->taps->tcam_hdl, arg->key, arg->length, &obj->tph);
        if (SOC_FAILURE(rv) || (obj->tph == NULL)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Prefix has no associated TCAM pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }           
    }
    
    if (SOC_SUCCESS(rv)) {
        obj->domain_id = obj->tph->sbucket->domain;
        
        /* search sram bucket trie, skip this step if user specify obj->sph */
        if (obj->sph == NULL) {
            rv = taps_sbucket_prefix_pivot_find(unit, obj->tph->sbucket, 
                            arg->key, arg->length, arg->taps->param.mode, &obj->sph);
        }
    }
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        if (SOC_FAILURE(rv) || (obj->sph == NULL)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Prefix has no associated SRAM pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } else {
            /* search dram bucket trie, doesn't make sense to allow user to skip this step */
            assert(obj->sph);
            rv = taps_dbucket_find_prefix(unit, obj->sph->dbh, 
                                         arg->key, arg->length, &obj->dph);
        }
    }
    
    return rv;
}

/*
 *   Function
 *      taps_find_bpm_asso_data
 *   Purpose
 *      Find Best Prefix Match (BPM) length  for a given
 *      key/length in the tcam database. Return the corresponding bpm's payload
 *      Note: Just used by mode zero
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) key    : key buffer of the prefix
 *      (IN) length : key length of the prefix
 *      (OUT) bpm_length      : bpm key length of the prefix
 *      (OUT) asso_data: payload data
 *   Returns
 *       SOC_E_NONE - found (we should always find one assuming * is in system)
 *       SOC_E_* as appropriate otherwise
 */
int taps_find_bpm_asso_data(int unit, taps_tcam_t *handle,
                            uint32 *key, uint32 length, uint32 *bpm_mask, unsigned int **asso_data) 
{
    int rv = SOC_E_NONE, word_idx;
    uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];
    uint32 bpm_length = 0;
    taps_arg_t arg;
    taps_obj_t obj;

    if (handle == NULL || key == NULL || asso_data == NULL) {
        return SOC_E_PARAM;
    }
    if (bpm_mask != NULL) {
        /* get the bpm_length from bpm_mask */
        rv = taps_get_bpm_pfx(bpm_mask, length, handle->key_size, &bpm_length);
        if (SOC_FAILURE(rv)) {
        	LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d segment %d Failed to find proper bpm in bpm_mask passed in\n"),
                           FUNCTION_NAME(), unit, handle->segment));
        	(void)taps_show_prefix(handle->key_size, bpm_mask, handle->key_size);	
        	return SOC_E_INTERNAL;
        } 
    } 
    
    if ((bpm_length == 0) && (length != 0)) {
        rv = trie_find_prefix_bpm(handle->trie, key, length, &bpm_length);
        if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find BPM for following key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
    	(void)taps_show_prefix(handle->key_size, key, length);
    	return rv;	
        }
    }

    if (bpm_length == 0) {
        /* bpm use default payload */
        *asso_data = handle->taps->defpayload;
    } else {
        for (word_idx = 0; word_idx < BITS2WORDS(handle->key_size); word_idx++) {
            bpm_key[word_idx]=key[word_idx];
            }
        
        /* right shift key to construct bpm_key */
        rv = taps_key_shift(handle->key_size, bpm_key, length, (length-bpm_length));
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to construct BPM key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
        return rv;  
        }
        
        sal_memset(&obj, 0, sizeof(obj));
        arg.taps = handle->taps;
        arg.key  = bpm_key;
        arg.length = bpm_length;
        
        rv = _taps_find_prefix_objects(unit, &arg, &obj);
        assert(obj.tph);
        if (handle->taps->param.mode == TAPS_OFFCHIP_ALL) {
            if(SOC_SUCCESS(rv) && obj.dph) {
                *asso_data = obj.dph->payload;
            } 
        } else {
            if(SOC_SUCCESS(rv)) {
                *asso_data = obj.sph->payload;
            }
        }
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Fail to find bpm key 0x%x 0x%x 0x%x "
                                  "0x%x 0x%x, bpm_length %d \n"),
                       FUNCTION_NAME(), unit, bpm_key[0], bpm_key[1], bpm_key[2], bpm_key[3],
                       bpm_key[4], bpm_length));
        }
    }
    
    return rv;
}

int taps_tcam_propagate_prefix_for_onchip(int unit, taps_tcam_handle_t handle,
			                        taps_wgroup_handle_t *wgroup,
                                    taps_tcam_pivot_handle_t tph, int add,
                                    int32 global_index_added,
                                    uint32 *key, uint32 length, unsigned int *payload,
                                    uint8 *isbpm)
{
    int rv; 
    if (handle->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        /* For mode one, we need to do propagation in tcam */
        rv = taps_tcam_propagate_prefix(unit, handle, wgroup, tph,
                                            add, global_index_added, key, 
                                            length, isbpm);
    } else if (handle->taps->param.mode == TAPS_ONCHIP_ALL){
        /* For mode zero, just update the asso data */
        rv = taps_tcam_propagate_prefix_for_modezero(unit, handle, wgroup, tph,
                                            add, key, length, payload);
    } else {
        rv = SOC_E_PARAM;
    }
    return rv;
}

int _dbucket_prefix_collect(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    _taps_redistribute_info_t *redistribute_info = NULL;

    if (!dph || !user_data) return SOC_E_PARAM;

    redistribute_info = (_taps_redistribute_info_t *)user_data;
    redistribute_info->pdph[redistribute_info->pfx_cnt] = dph;
    redistribute_info->pfx_cnt++;
    
    return rv;
}

/* callback function for walk through the split dram bucket & propagate all applicable BPM prefix */
int _dbucket_split_propagate(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie=NULL;
    taps_tcam_pivot_handle_t tph=NULL;
    uint8 bpm=0; 
    int global_pointer;

    if (!dph || !user_data) return SOC_E_PARAM;

    cookie = (taps_cookie_t*)user_data;
    
    if (_TAPS_KEY_IPV4(cookie->arg.taps) && (dph->length >= 46)) {
    /* any dph longer than maximum sph length not possible be bpm of other routes */
    return rv;
    }

    
    /* find prefix info required by hardware propagation */
    rv = taps_tcam_lookup_prefix(cookie->unit, cookie->arg.taps->tcam_hdl,
                 &(dph->pfx[0]), dph->length, &tph);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: unit %d Prefix has no associated TCAM pivot %d:%s !!!\n"), 
               FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
    }   

    /* skip dbucket propagation for domain split since the dram bucket pointers are local
     * in nature and does not affect any spivot's bpm pointers
     */
    /* propagate in Sram bucket */
    if (SOC_SUCCESS(rv) && 
    (cookie->state != INS_ST_DOMAIN_PROPAGATE_SPLIT)) {
    rv = taps_sbucket_propagate_prefix(cookie->unit, cookie->arg.taps, 
                       tph->sbucket, cookie->obj.wgroup,
                       &(dph->pfx[0]), dph->length, 
                       _BRR_INVALID_CPE_, TRUE, &bpm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d SRAM failed to propagate split prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
    }
    }
    
    /* propagate in Tcam */
    if (SOC_SUCCESS(rv)) {
    if (cookie->state == INS_ST_DOMAIN_PROPAGATE_SPLIT) {
        /* calculate the global pointer since new tcam pivot not
         * inserted into the tcam trie yet.
         */
        rv = _taps_calculate_prefix_pointer(cookie->unit, cookie->arg.taps,
                        cookie->obj.nsbh, cookie->obj.sph->dbh, dph,
                        (uint32*)&global_pointer, TRUE);
    } else {
        /* lookup the global pointer */
        global_pointer = -1;
    }

    if (SOC_SUCCESS(rv)) {
        rv =  taps_tcam_propagate_prefix(cookie->unit, cookie->arg.taps->tcam_hdl, 
                         cookie->obj.wgroup, tph, TRUE,
                         global_pointer, &(dph->pfx[0]), dph->length, &bpm);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d TCAM failed to propagate split prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        } 
    }
    }

    return rv;
}

/*
 *   Function
 *       _sbucket_split_propagate_for_onchip_mode
 *   Purpose
 *       call back for walk through the split sram bucket
 *   Parameters
 *      (IN) sph             : sbucket prefix handler
 *      (IN) user_data   : use it as cookie
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */

static int _sbucket_split_propagate_for_onchip_mode(taps_spivot_handle_t sph, void *user_data)
{   
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie=NULL;
    taps_tcam_pivot_handle_t tph=NULL;
    int global_pointer;
    uint8 bpm=0;

    if (!sph || !user_data) return SOC_E_PARAM;

    /* if the dram prefix is a payload, propagate */
    cookie = (taps_cookie_t*)user_data;

    /* find prefix info required by hardware propagation */
    rv = taps_tcam_lookup_prefix(cookie->unit, cookie->arg.taps->tcam_hdl,
                 &(sph->pivot[0]), sph->length, &tph);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: unit %d Prefix has no associated TCAM pivot %d:%s !!!\n"), 
               FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
    }  
    
    /* propagate in Tcam */
    if (cookie->state == INS_ST_DOMAIN_PROPAGATE_SPLIT) {
        /* calculate the global pointer since new tcam pivot not
         * inserted into the tcam trie yet.
         */
        rv = _taps_calculate_prefix_pointer(cookie->unit, cookie->arg.taps,
                        cookie->obj.nsbh, NULL, sph,
                        (uint32*)&global_pointer, TRUE);
    } else {
        /* lookup the global pointer */
        global_pointer = -1;
    }

    if (SOC_SUCCESS(rv)) {
        rv =  taps_tcam_propagate_prefix_for_onchip(cookie->unit, cookie->arg.taps->tcam_hdl, 
                         cookie->obj.wgroup, tph, TRUE,
                         global_pointer, &(sph->pivot[0]), sph->length, sph->payload, &bpm);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d TCAM failed to propagate split prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        } 
    }
    return rv;
}

/* call back for walk through the split sram bucket,
 * relocate all applicable dram bucket & propagate BPM prefix
 */
int _sbucket_split_propagate(taps_spivot_handle_t sph, void *user_data)
{
    int rv = SOC_E_NONE;
    taps_cookie_t *cookie=NULL;
    taps_spivot_handle_t tmp_sph = NULL;

    if (!sph || !user_data) return SOC_E_PARAM;

    /* if the dram prefix is a payload, propagate */
    cookie = (taps_cookie_t*)user_data;

    /* copy the dbucket to the new domain (different address in Dram) */    
    rv = taps_relocate_dbucket(cookie->unit, cookie->arg.taps, cookie->obj.wgroup,
                               cookie->obj.domain_id, sph->dbh);

    /* traverse dbucket and propagate each prefix */
    if (SOC_SUCCESS(rv)) {
    /* pass in dbh for calculation of global index for tcam propagation */
        tmp_sph = cookie->obj.sph;
        cookie->obj.sph = sph;
        rv = taps_dbucket_traverse(cookie->unit, 
                                   cookie->arg.taps, cookie->obj.wgroup,
                                   sph->dbh, cookie, 
                                   _dbucket_split_propagate);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to propagate split dram bucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
        } 
        cookie->obj.sph = tmp_sph;
    }

    return rv;
}

/*
 *
 * Function:
 *     _taps_dbucket_redist_dpfx_get
 * Purpose:
 *     Get pfx which need to be redisted
 */
static int _taps_dbucket_redist_dpfx_get(taps_dprefix_handle_t dph, void *user_data)
{
    int rv=SOC_E_NONE;
    _taps_bucket_redist_t *cbdata = (_taps_bucket_redist_t *)user_data;
    
    if (!dph || !user_data) return SOC_E_PARAM;
    
    cbdata->dph[cbdata->dph_cnt] = dph;
    cbdata->dph_cnt++;

    assert(cbdata->dph_cnt <= TAPS_DBUCKET_DIST_THRESHOLD(cbdata->taps->param.dbucket_attr.num_dbucket_pfx*2));

    return rv;
}

/*
 *
 * Function:
 *     _taps_dbucket_redist_spivot_get
 * Purpose:
 *     Get spivot which need to be redisted
 * Note:
 *     There are three kind of spivot we need to skip
 *     1. Wild spivot
 *     2. The spivot's pfx number is more than TAPS_DBUCKET_DIST_THRESHOLD 
 *     3. The spivot is the vrf default pfx
 */
static int _taps_dbucket_redist_spivot_get(taps_spivot_handle_t sph, void *user_data)
{
    int rv=SOC_E_NONE, redist = TRUE;
    int trie0_count = 0, trie1_count = 0;
    _taps_bucket_redist_t *cbdata = (_taps_bucket_redist_t*)user_data;
    
    if (!sph || !user_data) return SOC_E_PARAM;

    SHR_BITCOUNT_RANGE(sph->dbh->pfx_bmap0, trie0_count, 0, cbdata->taps->param.dbucket_attr.num_dbucket_pfx);
    SHR_BITCOUNT_RANGE(sph->dbh->pfx_bmap1, trie1_count, 0, cbdata->taps->param.dbucket_attr.num_dbucket_pfx);
    
    if((sph == sph->sbh->wsph) 
      || (trie0_count + trie1_count > TAPS_DBUCKET_DIST_THRESHOLD(cbdata->taps->param.dbucket_attr.num_dbucket_pfx*2))
      || (sph->length == cbdata->taps->param.key_attr.vrf_length)) {
        redist = FALSE;
    } 
    if (redist) {
        cbdata->sph[cbdata->sph_cnt] = sph;
        cbdata->sph_cnt++;
    }
    
    return rv;
}

/*
 *
 * Function:
 *     _taps_sbucket_redist_check
 * Purpose:
 *     Check if this sbucket need to be redisted. If if would trigger another split, then don't redist
 */
int _taps_dbucket_redist_check(int unit, taps_handle_t taps, 
                                taps_wgroup_handle_t *wgroup, 
                                taps_spivot_handle_t sph, 
                                int redist_dph_cnt,
                                int *split)
{
    int rv = SOC_E_NONE, count, msb;
    taps_arg_t arg;
    taps_obj_t obj;
    unsigned int key[TAPS_MAX_KEY_SIZE_WORDS];
    taps_tcam_pivot_t tmp_tp;
    SHR_BITDCL *bmap = NULL;

    sal_memcpy(&key[0], &sph->pivot[0], sizeof(unsigned int) * BITS2WORDS(taps->param.key_attr.lookup_length));
    sal_memset(&obj, 0, sizeof(obj));

    /* Check if redist would trigger another split, if so, don't redist it if the target sph is not wild sph */
    /* Construct a key which length is sph->length -1 */
    rv = taps_key_shift(taps->param.key_attr.lookup_length, key, 
                sph->length, 1); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to construct search key\n"),
                   FUNCTION_NAME(), unit));
        return rv;  
    }
    obj.tph = &tmp_tp;
    obj.tph->sbucket = sph->sbh;
    obj.sph = NULL;
    obj.dph = NULL;
    
    arg.taps = taps;
    arg.key  = &key[0];
    arg.length = sph->length - 1;

    /* Find the longest pivot match of this sph */
    rv = _taps_find_prefix_objects(unit, &arg, &obj);
    assert(obj.sph);

    /* coverity[var_deref_op : FALSE] */
    if (obj.sph->length == sph->length - 1) {
        msb = 0;
    } else {
        msb = _TAPS_GET_KEY_BIT(key, (sph->length - 1) - obj.sph->length - 1 , 
            taps->param.key_attr.lookup_length)?1:0;
    }
    
    if (msb) {
        bmap = &(obj.sph->dbh->pfx_bmap1[0]);
    } else {
        bmap = &(obj.sph->dbh->pfx_bmap0[0]);
    }
    SHR_BITCOUNT_RANGE(bmap, count, 0, taps->param.dbucket_attr.num_dbucket_pfx);
     /* Check if there is enough capacity in the sub-trie */
    if (count + redist_dph_cnt >= taps->param.dbucket_attr.num_dbucket_pfx) {
        /* Will trigger split if merge the two sbucket */
        *split = TRUE;
    } else {
        *split = FALSE;
    }
    
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_dbucket_redist_cb
 * Purpose:
 *     dbucket redisst 
 */
static int _taps_dbucket_redist_dbk(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup, 
                            taps_sbucket_handle_t sbh, taps_dbucket_handle_t dbh, taps_spivot_handle_t sph,
                            taps_dprefix_handle_t *dph_array, int dph_cnt)
{
    int rv = SOC_E_NONE, dbucket_split = FALSE;
    taps_cookie_t cookie;
    taps_arg_t arg;
    taps_obj_t *obj = NULL;
    taps_dprefix_handle_t dph = NULL;
    uint8 isbpm;
    taps_bucket_stat_e_t bkt_stat;
    uint32 local_pointer, global_pointer, pivot_len;
	uint32 del_pivot_id = 0, dph_index = 0;
    taps_work_type_e_t forced_work_type;
    unsigned int bpm[_TAPS_MAX_KEY_WORDS_], pivot[_TAPS_MAX_KEY_WORDS_];
    /* Use for error handling */
    taps_spivot_t eh_sph;
    taps_dprefix_t eh_dph[TAPS_DBUCKET_DIST_THRESHOLD(63*2)];
    unsigned int eh_bpm[_TAPS_MAX_KEY_WORDS_];
    int eh_dph_index;
    
    if (taps == NULL || wgroup == NULL || sbh == NULL || dbh == NULL || sph == NULL
        || dph_array == NULL) {
        return SOC_E_PARAM;
    }

    /* Get sph's bpm_mask for error handling */
    sal_memcpy(&eh_sph, sph, 
        sizeof(taps_spivot_t) + BITS2WORDS(taps->param.key_attr.lookup_length) * sizeof(unsigned int));
    sal_memset(&eh_bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
    rv = trie_bpm_mask_get(sbh->trie, sph->pivot, sph->length, &eh_bpm[0]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to find bpm %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 
    
    sal_memset(&cookie, 0, sizeof(taps_cookie_t));
    sal_memset(&arg, 0, sizeof(taps_arg_t));
    sal_memset(&pivot[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));

    arg.taps = taps;
    sal_memcpy(&cookie.arg, &arg, sizeof(taps_arg_t));
    
    cookie.unit = unit;
    obj = &cookie.obj;
    sal_memcpy(&obj->wgroup[0], wgroup, sizeof(taps_wgroup_handle_t) * SOC_MAX_NUM_DEVICES);
    rv = _taps_dbucket_redist_check(unit, taps, wgroup, sph, dph_cnt, &dbucket_split);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to check whether redist this dph %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 
    if (dbucket_split) {
        return rv;
    }
    /* To avoid to re-use the same spivot index, get a free spivot index before delete sph */
    rv = taps_sbucket_pivot_id_alloc(unit, taps, sbh, &obj->sph_id);
    if (SOC_FAILURE(rv)) {
        /* This should never happen */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to allocate spivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _dkt_redist_error;
    }
    
    /* Delete sph */
    /* Use STAGE3 to commit delete operation at last */
    taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);

    del_pivot_id = sph->index;
    
    rv = taps_sbucket_delete_pivot(unit, taps, wgroup, sbh, sph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d  failed to delete sram pivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _dkt_redist_error;
    } 

    rv = taps_sbucket_pivot_id_free(unit, taps, sbh, del_pivot_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, del_pivot_id, rv, soc_errmsg(rv)));
        goto _dkt_redist_error;
    }

    for (dph_index = 0; dph_index < dph_cnt; dph_index++) {
        obj->tph = NULL;
        obj->sph = NULL;
        obj->dph = NULL;
        dph = dph_array[dph_index];
        arg.key  = dph->pfx;
        arg.length = dph->length;
             
        rv = _taps_find_prefix_objects(unit, &arg, obj);
        if(rv != SOC_E_NOT_FOUND || obj->sph == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d  failed to get target spivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        } 
        /* Insert dph */
        /* Use STAGE1 to commit insert operation at first */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE1_WORK);

        rv = taps_dbucket_insert_prefix(unit, taps, wgroup,
                                     obj->sph->dbh, dph->pfx, dph->length,
					                 dph->payload, dph->cookie,
                                     obj->sph->length, &obj->dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to insert prefix to dram bucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error; 
        }
        /* Propagation */
        /* Use STAGE2 to commit propagate operation at secend */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE2_WORK);
        
        obj->dph->bpm = dph->bpm;

        rv = taps_calculate_prefix_local_pointer(unit, taps, obj->sph, obj->dph, &local_pointer);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to calculate local point for inserted prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        } 
        rv = taps_sbucket_propagate_prefix(unit, taps, obj->tph->sbucket, wgroup,
                       dph->pfx, dph->length, local_pointer,
                       TRUE, &isbpm);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to propagate inserted prefix in SRAM %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        }

        rv = taps_calculate_prefix_global_pointer(unit, taps, obj->tph, obj->sph,
                          obj->dph, &global_pointer);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to calculate global point for inserted prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        }
        rv = taps_tcam_propagate_prefix(unit, taps->tcam_hdl, wgroup,
                            obj->tph, TRUE, global_pointer, dph->pfx, dph->length, &isbpm);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to propagate inserted prefix in TCAM %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        } 
        
        /* check if we need to split the dbucket */
        rv = taps_dbucket_stat_get(unit, taps, obj->sph->dbh, &bkt_stat);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to compute dbucket vacancy %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        }
        if (_TAPS_BKT_FULL == bkt_stat) {
            if (obj->sph_id == _TAPS_INV_ID_) {
                /* It means we delete one dbucket but create two more. This is not acceptable. 
                            * So just goto the error handling step to roll back the modification
                            */
                goto _dkt_redist_error;
            }
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE1_WORK);
            
            sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
    
            rv = taps_dbucket_split(unit, taps, wgroup, obj->sph_id,
                        obj->sph->dbh, &pivot[0], &pivot_len,
                        &bpm[0], &obj->ndbh);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to split dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _dkt_redist_error;
            }
            rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                       obj->sph_id, obj->tph->sbucket,
                       &pivot[0], pivot_len,
		               FALSE, NULL, NULL,
                       obj->ndbh, &bpm[0], &obj->nsph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to Insert sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _dkt_redist_error;
            }
            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _dkt_redist_error;
            }
            if (_TAPS_BKT_FULL == bkt_stat) {
                /* very rare case. Sbucket split during redistribution
                         * not handled for now.
                         */
                goto _dkt_redist_error;
            } 
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE2_WORK);
            /* Meet the requirement of _dbucket_split_propagate() function */
            cookie.state = INS_ST_DBKT_PROPAGATE_SPLIT;
            rv = taps_dbucket_traverse(unit, taps, wgroup,
                                   obj->ndbh, (void *)&cookie, 
                                   _dbucket_split_propagate);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to propagate split dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _dkt_redist_error;
            }
            /* Set sph_id to invalid id to indicate that we have split one time*/
            obj->sph_id = _TAPS_INV_ID_;
        }
        /* Delete dph */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
        sal_memcpy(&eh_dph[dph_index], dph, 
            sizeof(taps_dprefix_t) + \
            (BITS2WORDS(taps->param.key_attr.lookup_length)-1) * sizeof(unsigned int));
        rv = taps_dbucket_delete_prefix(unit, taps, wgroup, dbh, dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to delete dram prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _dkt_redist_error;
        }
    }
    
    rv = taps_dbucket_destroy(unit, taps, wgroup[unit], dbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to free dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        goto _dkt_redist_error;
    }
    
    for (forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK; 
        forced_work_type <= TAPS_REDISTRIBUTE_STAGE3_WORK;
        forced_work_type++) {
        rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
        }
    }
_dkt_redist_error:
    if (SOC_FAILURE(rv)) {
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
        if(obj->ndbh && obj->sph) {
            rv = taps_dbucket_merge(unit, taps, wgroup, 
                            obj->sph->length, obj->sph->dbh,
                            obj->ndbh, NULL);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to merged dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }
        if(obj->nsph) {
            rv = taps_sbucket_delete_pivot(unit, taps, wgroup, sbh, obj->nsph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d  failed to delete sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            rv = taps_sbucket_pivot_id_free(unit, taps, sbh, obj->sph_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, del_pivot_id, rv, soc_errmsg(rv)));
            }
        }
        for (eh_dph_index = 0; eh_dph_index < dph_index; eh_dph_index++) {
            dph = &eh_dph[eh_dph_index];
            rv = taps_dbucket_insert_prefix(unit, taps, wgroup,
                                     dbh, dph->pfx, dph->length,
					                 dph->payload, dph->cookie,
                                     eh_sph.length, &obj->dph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to insert dpx back :%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, del_pivot_id, rv, soc_errmsg(rv)));
            }
        }
        rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                       del_pivot_id, sbh,
                       eh_sph.pivot, eh_sph.length,
		               eh_sph.is_prefix, NULL, NULL,
                       dbh, &eh_bpm[0], &obj->nsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to insert dpx back :%u %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, del_pivot_id, rv, soc_errmsg(rv)));
        }
        forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
        rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
        }
        forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
        rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
        }
        forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
        taps_command_destory(unit, taps, wgroup, &forced_work_type, 1);
    }

    taps_commit_force_worktype_unset(unit, taps, wgroup);
    return rv;
}

/*
 *
 * Function:
 *     _taps_dbucket_redist_cb
 * Purpose:
 *     dbucket redisst call back function
 */
static int _taps_dbucket_redist_cb(taps_tcam_pivot_handle_t tph, void *user_data)
{
    int rv=SOC_E_NONE;
    int sph_index;
    _taps_bucket_redist_t *cbdata = (_taps_bucket_redist_t*)user_data;
        
    if (!tph || !user_data) return SOC_E_PARAM;

    /* Init sph infor */
    cbdata->sph_cnt = 0;
    sal_memset(cbdata->sph, 0, sizeof(taps_spivot_handle_t) * _MAX_SBUCKET_PIVOT_);
    
    /* Traverse sbucket to find the sph & dph which need to be redisted*/
    taps_sbucket_traverse(cbdata->unit, cbdata->taps, cbdata->wgroup,
                               tph->sbucket, cbdata, _taps_dbucket_redist_spivot_get);
    
    for(sph_index = 0; sph_index < cbdata->sph_cnt; sph_index++) {
        /* Traverse dbucket to find the dph which need to be redisted */
        cbdata->dph_cnt = 0;
        taps_dbucket_traverse(cbdata->unit, cbdata->taps, cbdata->wgroup, 
                        cbdata->sph[sph_index]->dbh, user_data, _taps_dbucket_redist_dpfx_get);
        
        rv = _taps_dbucket_redist_dbk(cbdata->unit, cbdata->taps, cbdata->wgroup, cbdata->sph[sph_index]->sbh,
                        cbdata->sph[sph_index]->dbh, cbdata->sph[sph_index], cbdata->dph, cbdata->dph_cnt);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(cbdata->unit,
                                  "%s: unit %d redistribute failed to redist dbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), cbdata->unit, rv, soc_errmsg(rv)));
            break;
        }
    } 
    
    return rv;
}

/*
 *
 * Function:
 *     taps_dbucket_redist
 * Purpose:
 *     Redist dbucket 
 */
int taps_dbucket_redist(int unit, _taps_bucket_redist_t *p_bucket_redist_info)
{
    int rv;
    rv = taps_tcam_traverse(unit, p_bucket_redist_info->taps, 
                p_bucket_redist_info->taps->tcam_hdl, p_bucket_redist_info, _taps_dbucket_redist_cb);
    return rv;
}

/*
 *
 * Function:
 *     _taps_sbucket_redist_spivot_get
 * Purpose:
 *     Get spivot which need to be redisted.
 */
static int _taps_sbucket_redist_spivot_get(taps_spivot_handle_t sph, void *user_data)
{
    int rv=SOC_E_NONE, redist = TRUE;
    taps_bucket_stat_e_t bkt_stat;
    _taps_bucket_redist_t *cbdata = (_taps_bucket_redist_t*)user_data;
    
    if (!sph || !user_data) return SOC_E_PARAM;

    if (cbdata->taps->param.mode == TAPS_OFFCHIP_ALL) {
        if(sph == sph->sbh->wsph) {
            /* For mode 2, skip wild sph if there is no pfx in wild dbucket*/
            rv = taps_dbucket_stat_get(cbdata->unit, cbdata->taps, sph->dbh, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(cbdata->unit,
                                      "%s: failed to compute dbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), rv, soc_errmsg(rv)));
            }
            if (_TAPS_BKT_EMPTY == bkt_stat) {
                /* Wild spivot and there is no pfx in its dbucket */
                redist = FALSE;
            }
        } 
    } else if (cbdata->taps->param.mode == TAPS_ONCHIP_ALL){
        /* For mode 0, we reserved a wild sph, but this sph could be updated */
        if (sph->length == 0) {
            redist = FALSE;
        }
    } else {
        /* For mode 1, there is no wild sph*/
    }
    

    if (redist) {
        cbdata->sph[cbdata->sph_cnt] = sph;
        cbdata->sph_cnt++;
    }

    return rv;
}

/*
 *
 * Function:
 *     _taps_sbucket_redist_tpivot_get
 * Purpose:
 *     Check if this tph need to be redisted.
 */
static int _taps_sbucket_redist_tpivot_get(taps_tcam_pivot_handle_t tph, void *user_data) 
{
    int rv=SOC_E_NONE;
    int count;
    _taps_bucket_redist_t *cbdata = (_taps_bucket_redist_t*)user_data;
    
    if (!tph || !user_data) return SOC_E_PARAM;

    /* Skip wild tph */
    if(tph->length == 0) return SOC_E_NONE;
    
    SHR_BITCOUNT_RANGE(tph->sbucket->pivot_bmap, count, 0, tph->sbucket->prefix_number);

    if(count <= TAPS_SBUCKET_DIST_THRESHOLD(tph->sbucket->prefix_number)) {
        cbdata->tph[cbdata->tph_cnt] = tph;
        cbdata->tph_cnt++;
    }
    
    return rv;
}

/*
 *
 * Function:
 *     _taps_sbucket_redist_check
 * Purpose:
 *     Check if this sbucket need to be redisted. If if would trigger another split, then don't redist
 */
int _taps_sbucket_redist_check(int unit, taps_handle_t taps, 
                                taps_wgroup_handle_t *wgroup, 
                                taps_tcam_pivot_handle_t tph, 
                                int redist_sph_cnt,
                                int *split,
                                taps_tcam_pivot_handle_t *dst_tph)
{
    int rv = SOC_E_NONE, count;
    taps_arg_t arg;
    taps_obj_t obj;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS];

    sal_memcpy(&key[0], &tph->key[0], sizeof(uint32) * BITS2WORDS(taps->param.key_attr.lookup_length));
    sal_memset(&obj, 0, sizeof(obj));

    /* Check if redist would trigger another split, if so, don't redist */
    /* Construct a key which length is sph->length -1 */
    rv = taps_key_shift(taps->param.key_attr.lookup_length, key, 
                tph->length, 1); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to construct search key\n"),
                   FUNCTION_NAME(), unit));
        return rv;  
    }
    
    arg.taps = taps;
    arg.key  = &key[0];
    arg.length = tph->length - 1;

    /* Find the longest pivot match of this sph */
    rv = _taps_find_prefix_objects(unit, &arg, &obj);
    assert(obj.tph);
    /* Which sub-trie should be used */
    SHR_BITCOUNT_RANGE(obj.tph->sbucket->pivot_bmap, count, 0, obj.tph->sbucket->prefix_number);
    
    /* Check if there is enough capacity in the sub-trie */
    if (count + redist_sph_cnt >= obj.tph->sbucket->prefix_number) {
        /* Will trigger split if merge the two sbucket */
        *split = TRUE;
    } else {
        *split = FALSE;
        *dst_tph = obj.tph;
    }
    
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _taps_sbucket_redist_sbk
 * Purpose:
 *     Redist sbucket 
 */
static int _taps_sbucket_redist_sbk_for_offchip(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup, 
                            taps_tcam_pivot_handle_t tph, taps_spivot_handle_t *sph_array, int sph_cnt)
{
    int rv = SOC_E_NONE;
    int sph_index = 0, sbucket_split = FALSE, del_pivot_id = 0, 
        bpm_word = 0, bpm_word_idx = 0, tph_bpm_len = 0, shift_len = 0;
    taps_cookie_t cookie;
    taps_arg_t arg;
    taps_obj_t *obj = NULL;
    taps_sbucket_handle_t old_sbh;
    taps_bucket_stat_e_t bkt_stat;
    taps_work_type_e_t forced_work_type;
    unsigned int bpm[_TAPS_MAX_KEY_WORDS_], tpivot_bpm[_TAPS_MAX_KEY_WORDS_], spivot_bpm[_TAPS_MAX_KEY_WORDS_];
    unsigned int *spivot_key;
    uint32 tpivot_key[_TAPS_MAX_KEY_WORDS_];
    uint32 spivot_length, tpivot_length;
    /* Use for error handling */
    taps_spivot_handle_t eh_added_sph[TAPS_SBUCKET_DIST_THRESHOLD(_MAX_SBUCKET_PIVOT_)];  
    taps_spivot_t eh_deleted_sph[TAPS_SBUCKET_DIST_THRESHOLD(_MAX_SBUCKET_PIVOT_)];  
    int eh_add_cnt = 0, eh_del_cnt = 0;
    
    sal_memset(&cookie, 0, sizeof(taps_cookie_t));
    sal_memset(&arg, 0, sizeof(taps_arg_t));
    sal_memset(&tpivot_bpm[0], 0, sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);
    sal_memset(&eh_added_sph[0], 0, 
         TAPS_SBUCKET_DIST_THRESHOLD(_MAX_SBUCKET_PIVOT_) * sizeof(taps_spivot_handle_t));
    sal_memset(&eh_deleted_sph[0], 0, 
         TAPS_SBUCKET_DIST_THRESHOLD(_MAX_SBUCKET_PIVOT_) * sizeof(taps_spivot_t));
    sal_memset(&spivot_bpm[0], 0, sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);
    
    arg.taps = taps;
    sal_memcpy(&cookie.arg, &arg, sizeof(taps_arg_t));
    
    cookie.unit = unit;
    obj = &cookie.obj;
    sal_memcpy(&obj->wgroup[0], wgroup, sizeof(taps_wgroup_handle_t) * SOC_MAX_NUM_DEVICES);
    /* Check if we really need to redist this sbucket */
    rv = _taps_sbucket_redist_check(unit, taps, wgroup, tph, sph_cnt, &sbucket_split, &obj->tph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to check whether redist this spivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    if (!sbucket_split) {
        obj->nsbh = obj->tph->sbucket;
        old_sbh = tph->sbucket;
        /* Backup tpivot length&key */
        if (_TAPS_KEY_IPV4(taps)) {
            sal_memcpy(tpivot_key, tph->key, sizeof(uint32) * BITS2WORDS(TAPS_IPV4_KEY_SIZE));
        } else {
            /* coverity[overrun-buffer-arg] */
            sal_memcpy(tpivot_key, tph->key, sizeof(uint32) * BITS2WORDS(TAPS_IPV6_KEY_SIZE));
        }
        tpivot_length = tph->length;
        /* Get bpm mask of the deleted tpivot */
        trie_bpm_mask_get(taps->tcam_hdl->trie, tph->key, tph->length, &tpivot_bpm[0]);

        /* Mask up the higher bits, just care the difference bits between tph and obj->tph */
        if (obj->tph->length == 0) {
            tph_bpm_len = tph->length;
        } else {
            tph_bpm_len = tph->length - obj->tph->length + 1;
        }
        bpm_word = ((BITS2WORDS(taps->param.key_attr.lookup_length)*32) - tph_bpm_len)/32;
        /* Clear higher words of tpivot_bpm */
        for (bpm_word_idx = 0; bpm_word_idx < bpm_word; bpm_word_idx++) {
            tpivot_bpm[bpm_word_idx] = 0;
        }
        /* clear higher bits on bpm_word of tpivot_bpm */
        tpivot_bpm[bpm_word] &= TP_MASK(tph_bpm_len % 32);
            
        /* Delete tph by first, to avoid to search bpm failed */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
        rv = taps_tcam_delete_pivot(unit, taps->tcam_hdl, wgroup, tph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }
        tph = NULL;
        for (sph_index = 0; sph_index < sph_cnt; sph_index++) {
            /* Use STAGE1 to commit spivot insert operation */
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE1_WORK);
            obj->sph = sph_array[sph_index];
            sal_memcpy(&bpm[0], &tpivot_bpm[0], sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);
            sal_memset(&spivot_bpm[0], 0, sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);
            if (obj->sph == old_sbh->wsph) {
                /* For wild spivot, we need to use tph's key and length as a new
                            * spivot, and insert this spivot into the dest sbucket
                            */
                spivot_key = &tpivot_key[0];
                spivot_length = tpivot_length;
            } else {
                spivot_key = &obj->sph->pivot[0];
                spivot_length = obj->sph->length;
                /* Get bpm mask from sbucket trie */
                rv = trie_bpm_mask_get(old_sbh->trie, spivot_key, spivot_length, spivot_bpm);
                if (SOC_FAILURE(rv)) {
                    /* failed to get bpm mask */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to get bpm_mask %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    goto _sbk_redist_error;
                }
            }
            /* Left shift tpivot_bpm, leave shift bits for sbucket bpm */
            shift_len = tpivot_length - spivot_length;
            if (shift_len != 0) {
                rv = taps_key_shift(taps->param.key_attr.lookup_length, bpm, 
                                tpivot_length, shift_len); 
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to construct search key\n"),
                               FUNCTION_NAME(), unit));
                    goto _sbk_redist_error; 
                }
                
                /* Conject tpivot_bpm and spivot_bpm */
                bpm_word = ((BITS2WORDS(taps->param.key_attr.lookup_length)*32) - spivot_length)/32;
                for (bpm_word_idx = bpm_word; 
                    bpm_word_idx < BITS2WORDS(taps->param.key_attr.lookup_length); 
                    bpm_word_idx++) {
                    bpm[bpm_word_idx] |= spivot_bpm[bpm_word_idx];
                }
            }
            /* Get a free spivot index */
            rv = taps_sbucket_pivot_id_alloc(unit, taps, obj->nsbh, &obj->sph_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d dram bucket split failed to allocate spivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _sbk_redist_error;
            }
            
            /* Alloc a new dbucket */
            rv = taps_dbucket_create(unit, taps, wgroup[unit], obj->nsbh->domain,
                            obj->sph_id, &obj->ndbh);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d new Dbucket creation Failed %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        	    goto _sbk_redist_error;
            }
            rv = taps_dbucket_merge(unit, taps, wgroup, 
                                   spivot_length, obj->ndbh, obj->sph->dbh, NULL);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to merged dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            /* Insert the sph to target sbucket */
            rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                                           obj->sph_id, obj->nsbh,
                                           spivot_key, spivot_length,
                                           obj->sph->is_prefix, NULL, NULL,
                                           obj->ndbh, &bpm[0], &obj->nsph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to insert new spivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _sbk_redist_error;
            }
            eh_added_sph[eh_add_cnt] = obj->nsph;
            eh_add_cnt++;
            
            /* Do not need to do sbucket propagate. The reason is obvious */
            /* Tcam propagate */
            /* Use STAGE2 to commit propagate operation */
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE2_WORK);
            cookie.state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
            /* obj->sph and obj->nsbh used in _dbucket_split_propagate() */
            taps_dbucket_traverse(unit, taps, wgroup,
                                   obj->ndbh, &cookie, 
                                   _dbucket_split_propagate);
            /* We have guranteed the sbucket would not be full after insertting,
                    * So don't need to check it again 
                    */
            /* Destory old dbucket */
            rv = taps_dbucket_destroy(unit, taps, wgroup[unit], obj->sph->dbh);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _sbk_redist_error;
            }
            obj->sph->dbh = NULL;
            if (obj->sph != old_sbh->wsph) {
                /* Remove spivot */
                /* Use STAGE3 to commit spivot delete operation */
                taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
                sal_memcpy(&eh_deleted_sph[eh_del_cnt], obj->nsph, sizeof(taps_spivot_t));
                eh_del_cnt++;
                del_pivot_id = obj->sph->index;
                rv = taps_sbucket_delete_pivot(unit, taps, wgroup,  old_sbh, obj->sph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to delete spivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    goto _sbk_redist_error;
                }
                
                rv = taps_sbucket_pivot_id_free(unit, taps, old_sbh, del_pivot_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, del_pivot_id, rv, soc_errmsg(rv)));
                    goto _sbk_redist_error;
                }
            }
        }
        
        rv = taps_sbucket_stat_get(unit, old_sbh, &bkt_stat);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        assert(_TAPS_BKT_WILD == bkt_stat);
        /* Use STAGE3 to commit tpivot delete operation */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
        /* Release domain id */ 
        rv = taps_bucket_free(unit, taps, old_sbh->domain);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, old_sbh->domain,
                       rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }
        if(old_sbh->wsph->dbh) {
            rv = taps_dbucket_destroy(unit, taps, wgroup[unit], old_sbh->wsph->dbh);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free wild dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                goto _sbk_redist_error;
            }
        }
        /* destroy sbucket */
        rv = taps_sbucket_destroy(unit, taps, wgroup, old_sbh);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to destroy sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }
        
        
        /* Commit cmds */
        for (forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK; 
            forced_work_type <= TAPS_REDISTRIBUTE_STAGE3_WORK;
            forced_work_type++) {
            rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
            }
        }
        taps_commit_force_worktype_unset(unit, taps, wgroup);
_sbk_redist_error:
        if (SOC_FAILURE(rv)) {
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
            for (sph_index = 0; sph_index < eh_add_cnt; sph_index++) {
                del_pivot_id = eh_added_sph[sph_index]->index;
                rv = taps_sbucket_delete_pivot(unit, taps, wgroup, obj->nsbh, eh_added_sph[sph_index]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to delete spivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                rv = taps_sbucket_pivot_id_free(unit, taps, obj->nsbh, del_pivot_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to free spivot id %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            for (sph_index = 0; sph_index < eh_del_cnt; sph_index++) {
                eh_deleted_sph[sph_index].dbh->bucket = eh_deleted_sph[sph_index].index;
                eh_deleted_sph[sph_index].dbh->domain = old_sbh->domain;
                rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                                   eh_deleted_sph[sph_index].index, old_sbh,
                                   eh_deleted_sph[sph_index].pivot, eh_deleted_sph[sph_index].length,
                                   eh_deleted_sph[sph_index].is_prefix, NULL, NULL,
                                   eh_deleted_sph[sph_index].dbh, &bpm[0], &obj->nsph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to insert spivot id %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            if (tph == NULL) {
                rv = taps_tcam_insert_pivot(unit, taps->tcam_hdl, wgroup,
                                    &tpivot_key[0], tpivot_length, &tpivot_bpm[0],
                                    old_sbh, &obj->ntph, 
                                    0, FALSE);
                 if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to insert tpivot id %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            /* Commit cmds */
            forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
            rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
            }
            forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
            rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
            }
            forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
            taps_command_destory(unit, taps, wgroup, &forced_work_type, 1);
            taps_commit_force_worktype_unset(unit, taps, wgroup);
        }
    }
    
    return rv;
}
static int _taps_sbucket_redist_sbk_for_onchip(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup, 
                            taps_tcam_pivot_handle_t tph, taps_spivot_handle_t *sph_array, int sph_cnt)
{
    int rv = SOC_E_NONE;
    int sbucket_split = FALSE;
    taps_cookie_t cookie;
    taps_arg_t arg;
    taps_obj_t *obj = NULL;
    taps_work_type_e_t forced_work_type;
    taps_sbucket_handle_t old_sbh;
    /* Following use for error handling */
    uint32 eh_tpivot_key[_TAPS_MAX_KEY_WORDS_];
    uint32 eh_tpivot_length;
    unsigned int eh_bpm[_TAPS_MAX_KEY_WORDS_];
    
    sal_memset(&cookie, 0, sizeof(taps_cookie_t));
    sal_memset(&arg, 0, sizeof(taps_arg_t));

    arg.taps = taps;
    sal_memcpy(&cookie.arg, &arg, sizeof(taps_arg_t));
    
    cookie.unit = unit;
    obj = &cookie.obj;
    cookie.state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
    sal_memcpy(&obj->wgroup[0], wgroup, sizeof(taps_wgroup_handle_t) * SOC_MAX_NUM_DEVICES);
    rv = _taps_sbucket_redist_check(unit, taps, wgroup, tph, sph_cnt, &sbucket_split, &obj->tph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to check whether redist this spivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    if (!sbucket_split) {
        obj->nsbh = obj->tph->sbucket;
        old_sbh = tph->sbucket;
        if (_TAPS_KEY_IPV4(taps)) {
            sal_memcpy(eh_tpivot_key, tph->key, sizeof(uint32) * BITS2WORDS(TAPS_IPV4_KEY_SIZE));
        } else {
            /* coverity[overrun-buffer-arg] */
            sal_memcpy(eh_tpivot_key, tph->key, sizeof(uint32) * BITS2WORDS(TAPS_IPV6_KEY_SIZE));
        }
        eh_tpivot_length = tph->length;
        trie_bpm_mask_get(taps->tcam_hdl->trie, eh_tpivot_key, eh_tpivot_length, &eh_bpm[0]);
         
        /* Delete tph by first, to avoid to search bpm failed */
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE2_WORK);
        rv = taps_tcam_delete_pivot(unit, taps->tcam_hdl, wgroup, tph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }
        tph = NULL;
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE1_WORK);
        rv = taps_sbucket_merge(unit, taps, wgroup,  obj->nsbh, old_sbh, NULL);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to merge sram bucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }
        taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE2_WORK);
        rv = taps_sbucket_traverse(unit, taps, wgroup,  obj->nsbh, &cookie, _sbucket_split_propagate_for_onchip_mode);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to propagate back split sram bucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            goto _sbk_redist_error;
        }

        /* Release domain id */ 
        rv = taps_bucket_free(unit, taps, old_sbh->domain);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, old_sbh->domain,
                       rv, soc_errmsg(rv)));
        }
        trie_destroy(old_sbh->trie);
        sal_free(old_sbh);
        /* Commit cmds */
        for (forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK; 
            forced_work_type <= TAPS_REDISTRIBUTE_STAGE3_WORK;
            forced_work_type++) {
            rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
            }
        }
        taps_commit_force_worktype_unset(unit, taps, wgroup);
_sbk_redist_error:
        if (SOC_FAILURE(rv)) {
            taps_commit_force_worktype_set(unit, taps, wgroup, TAPS_REDISTRIBUTE_STAGE3_WORK);
            if (old_sbh != NULL) {
                rv = taps_sbucket_merge(unit, taps, wgroup, old_sbh, obj->nsbh, NULL);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to merge sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            if (tph == NULL) {
                rv = taps_tcam_insert_pivot(unit, taps->tcam_hdl, wgroup,
                                    &eh_tpivot_key[0], eh_tpivot_length, &eh_bpm[0],
                                    old_sbh, &obj->ntph, 
                                    0, FALSE);
                 if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to insert tpivot id %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            /* Commit cmds */
            for (forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK; 
                forced_work_type <= TAPS_REDISTRIBUTE_STAGE3_WORK;
                forced_work_type++) {
                rv = taps_work_commit(unit, wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d redistribute failed to commit STAGE %d works %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, forced_work_type, rv, soc_errmsg(rv)));
                }
            }
            taps_commit_force_worktype_unset(unit, taps, wgroup);
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_redist
 * Purpose:
 *     Redist sbucket 
 */
int taps_sbucket_redist(int unit, _taps_bucket_redist_t *p_bucket_redist_info)
{
    int rv = SOC_E_NONE;
    int tph_index;
    p_bucket_redist_info->tph_cnt = 0;
    taps_tcam_traverse(unit, p_bucket_redist_info->taps, 
                p_bucket_redist_info->taps->tcam_hdl, p_bucket_redist_info, 
                _taps_sbucket_redist_tpivot_get);
    for (tph_index = 0; tph_index < p_bucket_redist_info->tph_cnt; tph_index++) {
        p_bucket_redist_info->sph_cnt = 0;
        /* Get all sph in this sbucket */
        rv = taps_sbucket_traverse(unit, p_bucket_redist_info->taps, p_bucket_redist_info->wgroup,
                               p_bucket_redist_info->tph[tph_index]->sbucket, p_bucket_redist_info, 
                               _taps_sbucket_redist_spivot_get);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to get spivot to redist %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        /* Insert those sph into the target sbucket */
        if (p_bucket_redist_info->taps->param.mode == TAPS_OFFCHIP_ALL) {
            rv = _taps_sbucket_redist_sbk_for_offchip(unit, p_bucket_redist_info->taps, 
                                p_bucket_redist_info->wgroup,
                                p_bucket_redist_info->tph[tph_index],
                                p_bucket_redist_info->sph,
                                p_bucket_redist_info->sph_cnt);
        } else {
            rv = _taps_sbucket_redist_sbk_for_onchip(unit, p_bucket_redist_info->taps, 
                                p_bucket_redist_info->wgroup,
                                p_bucket_redist_info->tph[tph_index],
                                p_bucket_redist_info->sph,
                                p_bucket_redist_info->sph_cnt);
        }
        
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to redist sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
    }
    
    return rv;
}

static int taps_bucket_redist_infor_init(int unit, 
                                        taps_handle_t taps, 
                                        taps_wgroup_handle_t *wgroup, 
                                        _taps_bucket_redist_t *bucket_redist_info)
{
    bucket_redist_info->unit = unit;
    bucket_redist_info->taps = taps;
    bucket_redist_info->wgroup = wgroup;
    bucket_redist_info->tph = sal_alloc(sizeof(taps_tcam_pivot_handle_t) * taps->tcam_hdl->use_count, 
                                            "bucket redist tph info");
    bucket_redist_info->sph = sal_alloc(sizeof(taps_spivot_handle_t) * _MAX_SBUCKET_PIVOT_,
                                            "bucket redist sph info");
    if(bucket_redist_info->tph == NULL
     || bucket_redist_info->sph == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(bucket_redist_info->tph, 0, sizeof(taps_tcam_pivot_handle_t) * taps->tcam_hdl->use_count);
    sal_memset(bucket_redist_info->sph, 0, sizeof(taps_spivot_handle_t) * _MAX_SBUCKET_PIVOT_);
    
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* Alloc dph for offchip mode */
        bucket_redist_info->dph = sal_alloc(sizeof(taps_dprefix_handle_t) * 
                               TAPS_DBUCKET_DIST_THRESHOLD(taps->param.dbucket_attr.num_dbucket_pfx*2),
                               "bucket redist dph info");
        if(bucket_redist_info->dph == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(bucket_redist_info->dph, 0, 
                sizeof(taps_dprefix_handle_t) * TAPS_DBUCKET_DIST_THRESHOLD(taps->param.dbucket_attr.num_dbucket_pfx*2));
    }

    return SOC_E_NONE;
}

static void taps_bucket_redist_infor_uninit(int unit, _taps_bucket_redist_t *bucket_redist_info)
{
    sal_free(bucket_redist_info->tph);
    sal_free(bucket_redist_info->sph);
    if (bucket_redist_info->taps->param.mode == TAPS_OFFCHIP_ALL) {
        sal_free(bucket_redist_info->dph);
    }
}

int taps_bucket_redist(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup)
{
    int rv=SOC_E_NONE;
    _taps_bucket_redist_t bucket_redist_info;

    /* Init call back data information */
    rv = taps_bucket_redist_infor_init(unit, taps, wgroup, &bucket_redist_info);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to init bucket redist infor %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    /* Redist dbucket */
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = taps_dbucket_redist(unit, &bucket_redist_info);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to redist dbucket redist %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            taps_bucket_redist_infor_uninit(unit, &bucket_redist_info);
            return rv;
        }
    }
    
    /* Redist sbucket */
    rv = taps_sbucket_redist(unit, &bucket_redist_info);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to redist sbucket redist %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        taps_bucket_redist_infor_uninit(unit, &bucket_redist_info);
        return rv;
    }

    /* Release resource */
    taps_bucket_redist_infor_uninit(unit, &bucket_redist_info);
    return rv;
}

int taps_host_bucket_redist(int unit, taps_handle_t taps)
{
    int rv = SOC_E_NONE;
    taps_obj_t obj;
    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table)) {
        unit = taps->master_unit;
    }
    
    /* create work group */
    rv = taps_work_group_create_for_all_devices(unit, taps, taps->wqueue, 
                                _TAPS_DEFAULT_WGROUP_,
                                taps->param.host_share_table,
                                obj.wgroup);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }    

    rv = taps_bucket_redist(unit, taps, obj.wgroup);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to redist buckdt %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    /* destory work group */
    rv = taps_work_group_destroy_for_all_devices(unit, taps, 
                                obj.wgroup, taps->param.host_share_table);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to destory work group %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_insert_error_cleanup_for_onchip_mode
 * Purpose:
 *     error handling state for taps insertion
 */
int taps_insert_error_cleanup_for_onchip_mode(int unit, taps_cookie_t *cookie)
{
    taps_ins_state_t state;
    taps_obj_t *obj;
    int rv=SOC_E_NONE;

    if (!cookie) return SOC_E_PARAM;

    if (cookie->state > INS_ST_ERROR) return SOC_E_PARAM;

    state = cookie->oldstate;
    obj = &cookie->obj;

    while(state != INS_ST_DONE) {
        switch(state) {
        case INS_ST_INIT:
            /* free back prefix id if allocated */
            /* destroy work group */
            if (obj->wgroup[unit]) {
        		/* commit everything at end */
        		rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_ALL);
        		if (SOC_FAILURE(rv)) {
        		    LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to commit during onchip error cleanup %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        		} 

                rv = taps_work_group_destroy_for_all_devices(unit, cookie->arg.taps,
                                obj->wgroup,  cookie->arg.taps->param.host_share_table);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy work queue group %d:%s !!!\n"), 
                               FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
                }
                obj->wgroup[unit] = NULL;
            }

            state = INS_ST_DONE; /* clean up done */
            break;
            
        case INS_ST_SBKT_PIVOT:
            if (obj->nsph) {
                rv = taps_sbucket_delete_pivot(unit, cookie->arg.taps, cookie->obj.wgroup,
                                               obj->tph->sbucket, obj->nsph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free sph %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            if (obj->sph_id != _TAPS_INV_ID_ && SHR_BITGET(obj->tph->sbucket->pivot_bmap, obj->sph_id)){
                rv = taps_sbucket_pivot_id_free(unit, cookie->arg.taps, 
                                               obj->tph->sbucket, 
                                               obj->sph_id);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, obj->sph_id, rv, soc_errmsg(rv)));
                }
            }
            
            state = INS_ST_INIT;
            break;

        case INS_ST_DOMAIN_SPLIT:
        case INS_ST_DOMAIN_PROPAGATE_SPLIT:
            if (!_TAPS_IS_PARALLEL_MODE_(cookie->arg.taps->param.instance)) {
                if (obj->tcam_entry_id != _TAPS_INV_ID_) {
                    taps_tcam_entry_bitmap_clear(unit, cookie->arg.taps->tcam_hdl, 
                                            cookie->tcam_pivot_len, obj->tcam_entry_id);
                }
            } 
            
            if (obj->domain_id != _TAPS_INV_ID_) {
                rv = taps_bucket_free(unit, cookie->arg.taps, obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->domain_id,
                               rv, soc_errmsg(rv)));
                }
            }
            
            if (obj->nsbh) {
                cookie->obj.domain_id = obj->tph->sbucket->domain;
                /* if new sbucket was created, fuse it with old sbucket and free the new sbucket */
                rv = taps_sbucket_merge(unit, cookie->arg.taps, obj->wgroup, 
                                        obj->tph->sbucket, obj->nsbh, &obj->nsph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to merge sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                obj->sph_id = obj->nsph->index;
                rv = taps_sbucket_traverse(unit, cookie->arg.taps, obj->wgroup,
                                       obj->tph->sbucket, cookie, _sbucket_split_propagate_for_onchip_mode);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to propagate back split sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                trie_destroy(obj->nsbh->trie);
                sal_free(obj->nsbh);
                rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_ALL);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            state = INS_ST_SBKT_PIVOT;
            break;
#if 0
        case INS_ST_DOMAIN_PROPAGATE_SPLIT:
            assert(obj->nsbh);
            assert(obj->nsbh->domain != obj->tph->sbucket->domain);
            obj->nsbh->domain = obj->tph->sbucket->domain;

            /* traverse through split domain, relocate dram buckets, propagate */
            rv = taps_sbucket_traverse(unit, cookie->arg.taps, obj->wgroup,
                                       obj->nsbh, cookie, _sbucket_split_propagate_for_onchip_mode);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            } 

            rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit & propagate"
                                      " relocated dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            } 

            state = INS_ST_DOMAIN_SPLIT;
            break;
#endif
        case INS_ST_TCAM_PIVOT:
            if (obj->ntph) {
                rv = taps_tcam_delete_pivot(unit, cookie->arg.taps->tcam_hdl,
                                            obj->wgroup, obj->ntph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                obj->tcam_entry_id = _TAPS_INV_ID_;
                rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_TCAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit tcam pivot delete %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }

            state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
            break;

        default:
            assert(0);
            break;
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_insert_error_cleanup_for_offchip_mode
 * Purpose:
 *     error handling state for taps insertion
 */
int taps_insert_error_cleanup_for_offchip_mode(int unit, taps_cookie_t *cookie)
{
    taps_ins_state_t state;
    taps_obj_t *obj;
    int rv=SOC_E_NONE;
    uint8 isbpm = FALSE;
    int pfx_idx, sph_idx, pivot_id;
    taps_dbucket_handle_t dbh = NULL;
    _taps_redistribute_info_t redistribute_info;
    taps_work_type_e_t forced_work_type;

    if (!cookie) return SOC_E_PARAM;

    if (cookie->state > INS_ST_ERROR) return SOC_E_PARAM;

    state = cookie->oldstate;
    obj = &cookie->obj;

    while(state != INS_ST_DONE) {
        switch(state) {
        case INS_ST_INIT:
            /* free back prefix id if allocated */
            /* destroy work group */
            if (obj->wgroup[unit]) {
        		/* commit everything at end */
        		rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_ALL);
        		if (SOC_FAILURE(rv)) {
        		    LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to commit during offchip error cleanup %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        		} 

                rv = taps_work_group_destroy_for_all_devices(unit, cookie->arg.taps, 
                    obj->wgroup, cookie->arg.taps->param.host_share_table);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy work queue group %d:%s !!!\n"), 
                               FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
                }
                obj->wgroup[unit] = NULL;
            }

            state = INS_ST_DONE; /* clean up done */
            break;

        case INST_ST_VRF_DEF_ROUTE:
            if (obj->sph_id != _TAPS_INV_ID_ && SHR_BITGET(obj->tph->sbucket->pivot_bmap, obj->sph_id)) {
                rv = taps_sbucket_pivot_id_free(unit,  cookie->arg.taps, 
                                                obj->tph->sbucket, 
                                                obj->sph_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->sph->index, rv, soc_errmsg(rv)));
                }
            }

            if (obj->ndbh) {
                rv = taps_dbucket_destroy(unit, cookie->arg.taps,
                                          obj->wgroup[unit], obj->ndbh);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free dbucket id:%u  %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->ndbh->bucket,
                               rv, soc_errmsg(rv)));
                }
            }
            state = INS_ST_INIT;
            break;

        case INS_ST_DBKT_PFX:
            if (obj->dph) {
                if (obj->vrf_def_route) {
                    rv = taps_dbucket_delete_prefix(unit, cookie->arg.taps, obj->wgroup,
                                                obj->ndbh, obj->dph);
                } else {
                    rv = taps_dbucket_delete_prefix(unit, cookie->arg.taps, obj->wgroup,
                                                obj->sph->dbh, obj->dph);
                }
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy prefix DPH %d:%s !!!\n"), 
                               FUNCTION_NAME(), cookie->unit, rv, soc_errmsg(rv)));
                }
            }

            if (obj->vrf_def_route) {
                state = INST_ST_VRF_DEF_ROUTE;
            } else {
                state = INS_ST_INIT;
            }
            break;

        case INS_ST_DBKT_PROPAGATE:
            rv = taps_sbucket_propagate_prefix(unit, cookie->arg.taps, 
                                               obj->tph->sbucket, obj->wgroup,
                                               cookie->arg.key, cookie->arg.length, _BRR_INVALID_CPE_,
                                               FALSE, &obj->dph->bpm);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d SRAM failed to undo propagate inserted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }

            rv =  taps_tcam_propagate_prefix(unit, cookie->arg.taps->tcam_hdl, obj->wgroup,
                                             obj->tph, FALSE, -1, cookie->arg.key, cookie->arg.length,
                                             &isbpm);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d TCAM failed to propagate inserted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            
            state = INS_ST_DBKT_PFX;
            break;

        case INS_ST_DBKT_SPLIT:
            if (obj->sph_id != _TAPS_INV_ID_ && SHR_BITGET(obj->tph->sbucket->pivot_bmap, obj->sph_id)) {
                rv = taps_sbucket_pivot_id_free(unit, cookie->arg.taps,
                                                obj->tph->sbucket, obj->sph_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free spivot id:%u %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->sph_id, 
                               rv, soc_errmsg(rv)));
                }
            }

            if (obj->ndbh) {
                if (obj->sph != NULL) {
                    /* Old dbh is not null */
                    /* set the new dram bucket id same as the split dbucket 
                                * so prefix pointers could be re-updated */
                    assert( obj->ndbh->bucket != obj->sph->dbh->bucket);

                    /* fuse it with old dbucket & free the new bucket */
                    rv = taps_dbucket_merge(unit, cookie->arg.taps, obj->wgroup, 
                                        obj->sph->length, obj->sph->dbh,
                                        obj->ndbh, &obj->dph);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to merged dram bucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    }
                    /* Re-propagate dbucket */
                    rv = taps_dbucket_traverse(cookie->unit, 
                                       cookie->arg.taps, cookie->obj.wgroup,
                                       obj->sph->dbh, cookie, 
                                       _dbucket_split_propagate);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to propagate dram bucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    } else {
                        rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_ALL);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to commit propagation %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        }
                    }
                    rv = taps_dbucket_destroy(unit, cookie->arg.taps,
                                          obj->wgroup[unit], obj->ndbh);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to free dbucket id:%u  %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, obj->ndbh->bucket,
                                   rv, soc_errmsg(rv)));
                    }
                } else {
                    /* Only if there is no node in old dbh */
                    obj->sph = obj->nsph;
                }
            }

            state = INS_ST_DBKT_PFX;
            break;
#if 0
        case INS_ST_DBKT_PROPAGATE_SPLIT:

            assert(obj->nsbh);
            if (obj->nsbh == NULL) {
                rv = SOC_E_INTERNAL;
                break;
            }
            assert( obj->ndbh->bucket != obj->sph->dbh->bucket);

            /* change bucket id to parent bucket so propagation adjusts 
             * bpm to old pointer */
            obj->ndbh->bucket = obj->sph->dbh->bucket;

            /* verify: set cookie domain id to old domain id */
            cookie->obj.domain_id = cookie->obj.tph->sbucket->domain;

            /* re-propagate the dram bucket bpm */
            rv = taps_dbucket_traverse(cookie->unit, 
                                       cookie->arg.taps, cookie->obj.wgroup,
                                       obj->ndbh, cookie, 
                                       _dbucket_split_propagate);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate merged dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            } else {
                rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit propagation %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            state = INS_ST_DBKT_SPLIT;
            break;
#endif
        case INS_ST_SBKT_PIVOT:
        case INS_ST_DBKT_PROPAGATE_SPLIT:
            if (obj->nsph) {
                if (obj->sph != NULL) {
                    rv = taps_sbucket_delete_pivot(unit, cookie->arg.taps, cookie->obj.wgroup,
                                               obj->tph->sbucket, obj->nsph);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to free sph %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    }
                    rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_SRAM);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit sram pivot delete in roll back %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    }
                } else {
                    obj->sph = obj->nsph;
                }
            }

            if (obj->vrf_def_route) {
                state = INS_ST_DBKT_PFX;
            } else {
                state = INS_ST_DBKT_SPLIT;
            }
            break;

        case INS_ST_DOMAIN_SPLIT:
            if (!_TAPS_IS_PARALLEL_MODE_(cookie->arg.taps->param.instance)) {
                if (obj->tcam_entry_id != _TAPS_INV_ID_) {
                    taps_tcam_entry_bitmap_clear(unit, cookie->arg.taps->tcam_hdl, 
                                            cookie->tcam_pivot_len, obj->tcam_entry_id);
                }
            } 
            
            if (obj->domain_id != _TAPS_INV_ID_) {
                rv = taps_bucket_free(unit, cookie->arg.taps, obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->domain_id,
                               rv, soc_errmsg(rv)));
                }
            }
            
            if (obj->wdbh) {
                rv = taps_dbucket_destroy(unit, cookie->arg.taps,
                                          obj->wgroup[unit], obj->wdbh);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free dbucket id:%u  %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, obj->wdbh->bucket,
                               rv, soc_errmsg(rv)));
                }
            }

            if (obj->nsbh) {
                cookie->obj.domain_id = obj->tph->sbucket->domain;
                /* if new sbucket was created, fuse it with old sbucket and free the new sbucket */
                rv = taps_sbucket_merge(unit, cookie->arg.taps, obj->wgroup, 
                                        obj->tph->sbucket, obj->nsbh, NULL);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to merge sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                rv = taps_sbucket_traverse(unit, cookie->arg.taps, obj->wgroup,
                                       obj->tph->sbucket, cookie, _sbucket_split_propagate);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                trie_destroy(obj->nsbh->trie);
                sal_free(obj->nsbh);
                
                rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_ALL);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit sram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                
                forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
                rv = taps_work_commit(unit, obj->wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d redistribute failed to commit STAGE 3 works %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            state = INS_ST_SBKT_PIVOT;
            break;

        case INS_ST_DOMAIN_PROPAGATE_SPLIT:
            assert(obj->nsbh);
            sal_memset(&redistribute_info, 0, sizeof(redistribute_info));
            sal_memcpy(&(redistribute_info.old_cookie), cookie, sizeof(*cookie));
            redistribute_info.cookie.unit = unit;
            sal_memcpy(redistribute_info.cookie.obj.wgroup, obj->wgroup, sizeof(obj->wgroup));
            redistribute_info.cookie.arg.taps = cookie->arg.taps;
            redistribute_info.pfx_cnt = cookie->redisted_dph_count;
            redistribute_info.pdph = cookie->redisted_dph;

            /* Insert prefix back to old sbucket prefix which have been redistributed*/
            for (pfx_idx = 0; pfx_idx < redistribute_info.pfx_cnt; pfx_idx++) {
                rv = _taps_redistributed_prefix_move(redistribute_info.pdph[pfx_idx], &redistribute_info);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to insert back redisted prefix %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }

            /* Delete prefix which have been splitted to the new dbucket in new sbucket,
                    * Insert those prefix back to the old dbucket in the new sbucket
                    */
            for (sph_idx = 0; sph_idx < cookie->added_sph_count; sph_idx++) {
                /* Re-init redistribute_info.pdph, and re-use it to save memory */
                sal_memset(redistribute_info.pdph, 0, sizeof(taps_dprefix_handle_t) *
                   cookie->arg.taps->param.sbucket_attr.max_pivot_number*
                   cookie->arg.taps->param.dbucket_attr.num_dbucket_pfx * 2);
                redistribute_info.pfx_cnt = 0;
                
                dbh = cookie->added_sph[sph_idx]->dbh;

                /* Delete sph which have been insertted in the new sbucket */
                pivot_id = cookie->added_sph[sph_idx]->index;
                
                rv = taps_sbucket_delete_pivot(unit, cookie->arg.taps, cookie->obj.wgroup,
                                            obj->nsbh, cookie->added_sph[sph_idx]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to delete spivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                rv = taps_sbucket_pivot_id_free(unit, cookie->arg.taps, 
						                obj->nsbh, pivot_id);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free spivot id %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                /* Collocet prefix which need to be insertted back*/
                rv = taps_dbucket_traverse(unit, cookie->arg.taps, obj->wgroup,
                                       dbh, &redistribute_info, 
                                       _dbucket_prefix_collect);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to insert back redisted prefix %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                /* Move back those dph to original dbucket */
                for (pfx_idx = 0; pfx_idx < redistribute_info.pfx_cnt; pfx_idx++) {
                    rv = _taps_splitted_prefix_move(redistribute_info.pdph[pfx_idx], dbh,
                                        &redistribute_info);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to insert back splitted prefix %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    }
                }

                /* Destory dbh */
                rv = taps_dbucket_destroy(unit, cookie->arg.taps, obj->wgroup[unit], dbh);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free dbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            state = INS_ST_DOMAIN_SPLIT;
            break;

        case INS_ST_TCAM_PIVOT:
            if (obj->ntph) {
                rv = taps_tcam_delete_pivot(unit, cookie->arg.taps->tcam_hdl,
                                            obj->wgroup, obj->ntph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                obj->tcam_entry_id = _TAPS_INV_ID_;
                rv = _taps_commit(unit, cookie, _TAPS_COMMIT_FLAG_TCAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit tcam pivot delete %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }  
            }

            state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
            break;

        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d unsupported error clean up state %d !!!\n"), 
                       FUNCTION_NAME(), unit, state));
            break;
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     _taps_insert_route_for_onchip_mode
 * Purpose:
 *     insert a route in onchip all mode
 */
static int _taps_insert_route_for_onchip_mode(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE, rv_in;
    taps_cookie_t cookie;
    taps_obj_t  *obj = NULL;
    unsigned int pivot_len = 0;
    uint8 isbpm=0, ispfxpivot = 0;
    taps_bucket_stat_e_t bkt_stat;
    trie_node_t  *split_trie_root = NULL;
    unsigned int bpm[_TAPS_MAX_KEY_WORDS_], pivot[_TAPS_MAX_KEY_WORDS_];
    uint32 *bpm_payload = NULL;
    
    /* validate */
    if (!arg->payload) return SOC_E_PARAM;
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    if (!TAPS_CAPACITY_LIMIT_DISABLED(&arg->taps->param) && 
        (arg->taps->capacity >= arg->taps->param.max_capacity_limit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d FIB Full length:%d Max:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, arg->taps->capacity,
                   arg->taps->param.max_capacity_limit, soc_errmsg(SOC_E_FULL)));
        return SOC_E_FULL;
    }

    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    sal_memset(&pivot[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
    cookie.state = cookie.oldstate = INS_ST_INIT;
    cookie.unit = unit;
    obj = &cookie.obj;
    obj->tcam_entry_id = _TAPS_INV_ID_;
    obj->domain_id = _TAPS_INV_ID_;

    while(cookie.state != INS_ST_DONE) {
        switch(cookie.state) {
        case INS_ST_INIT:
TIME_STAMP_START
            /*****
             * TAPS insert begin 
             *****/
            rv = _taps_find_prefix_objects(unit, arg, obj);
            if (SOC_SUCCESS(rv)) { 
                if (obj->sph) { /* Duplicate prefix */
                    rv = SOC_E_EXISTS;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "%s: unit %d Duplicate Prefix insertion %d:%s !!!\n"), 
                                 FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_STATE(cookie,INS_ST_DONE);
                    break;
                }
            }

            if (!(obj->tph && !obj->sph && rv == SOC_E_NOT_FOUND)) {
                TRANSITION_ERROR_HANDLE(cookie,INS);
            }

            /* create work group */
            rv = taps_work_group_create_for_all_devices(unit, arg->taps, arg->taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        arg->taps->param.host_share_table,
                                        obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);
            }
            ispfxpivot = TRUE;
            sal_memcpy(&pivot[0], arg->key, 
                       BITS2WORDS(arg->taps->param.key_attr.lookup_length) *
                       sizeof(unsigned int));
            sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
            _TAPS_SET_KEY_BIT(bpm, 0, arg->taps->param.key_attr.lookup_length);
            pivot_len = arg->length;
            
            /* advance state */
            TRANSITION_STATE(cookie,INS_ST_SBKT_PIVOT);

TIME_STAMP("# INS_ST_INIT state")
            break;
        case INS_ST_SBKT_PIVOT:
            /* sram bucket update due to dram bucket split */
TIME_STAMP_START
            rv = taps_sbucket_pivot_id_alloc(unit, arg->taps,
                                            obj->tph->sbucket, 
                                            &obj->sph_id);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_ONCHIP_WP_1, rv);
            if (SOC_FAILURE(rv)) {
               obj->sph_id = _TAPS_INV_ID_;
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s: unit %d failed to allocate spivot %d:%s !!!\n"), 
                          FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
               TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = taps_sbucket_insert_pivot(unit, arg->taps, obj->wgroup, 
                                           obj->sph_id, obj->tph->sbucket,
                                           &pivot[0], pivot_len,
                                           ispfxpivot, arg->payload, arg->cookie,
                                           NULL, &bpm[0], &obj->nsph);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_ONCHIP_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to Insert sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            rv =  taps_tcam_propagate_prefix_for_onchip(unit, arg->taps->tcam_hdl, obj->wgroup,
                                             obj->tph, TRUE, -1, arg->key, arg->length, arg->payload,
                                             &isbpm);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_ONCHIP_WP_3, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate for tcam %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            /* commit the propagation work item to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM | _TAPS_COMMIT_FLAG_TCAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_ONCHIP_WP_4, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit sram pivot insert %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);
            }

            if (_TAPS_BKT_FULL == bkt_stat) {
                /* pre-emptive split when the bucket size is 1 */
                TRANSITION_STATE(cookie,INS_ST_DOMAIN_SPLIT);
            } else {
                TRANSITION_STATE(cookie,INS_ST_DONE);
            } 
            
TIME_STAMP("# INS_ST_SBKT_PIVOT state")
            break;
        case INS_ST_DOMAIN_SPLIT:
        /* sram bucket split when full */
TIME_STAMP_START

            sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
            if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                /* Allocate a sbucket id */
                rv = taps_bucket_alloc(unit, arg->taps, 0, &obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    obj->domain_id = _TAPS_INV_ID_;
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate domain %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                }
            } else {
                /* coverity[var_deref_op] */
                rv = taps_sbucket_pivot_len_get(unit, arg->taps, obj->tph->sbucket,
                                                &pivot_len);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to get pivot len of split sbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS); 
                }
                
                 /* find a tcam free entry to insert */
                rv = taps_tcam_entry_alloc(unit, arg->taps->tcam_hdl, obj->wgroup, 
                                    pivot_len, &obj->tcam_entry_id);
                if (SOC_FAILURE(rv)) {
                    /* TCAM full, we should prevent this from happening by checking TCAM use counters */
                    obj->tcam_entry_id = _TAPS_INV_ID_;
                    /* Commit the tcam cmds since it is possible that some of tcam entries have been moved by  
                             * calling taps_tcam_entry_alloc()
                             */
                    rv_in = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
                    if (SOC_FAILURE(rv_in)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv_in, soc_errmsg(rv_in)));
                    }
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Tcam segment %d failed to alloc entry for length %d\n"),
                               FUNCTION_NAME(), unit, arg->taps->tcam_hdl->segment, pivot_len));
                    TRANSITION_ERROR_HANDLE(cookie,INS);
                }
                cookie.tcam_pivot_len = pivot_len;            
                            /* Allocate a sbucket id */
                rv = taps_bucket_alloc(unit, arg->taps, obj->tcam_entry_id, &obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    obj->domain_id = _TAPS_INV_ID_;
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate domain %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                }
            }
                        /* To support unified mode, seperate domain split process to 2 stages
                      * Stage1: Split trie, return root node of sub_trie which is splitted from original trie 
                      * Stage2: Move entried from original trie to new trie, and set to configure hw
                      *
                      * Note: Stage 1 and Stage 2 MUST NOT be used separately */
                      
                        /* Stage 1 */
            rv = taps_sbucket_split_stage1(unit, arg->taps, obj->tph->sbucket,
                                        &pivot[0], &pivot_len, 
                                        &bpm[0], &split_trie_root);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to split sbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS); 
            }

            cookie.tcam_pivot_len = pivot_len;
                        /* Stage 2 */
            rv = taps_sbucket_split_onchip_stage2(unit, arg->taps, obj->wgroup, obj->tph->sbucket,
                                    obj->domain_id, split_trie_root, pivot_len, &obj->nsbh, &obj->nsph);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_SPLIT_ONCHIP_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to do sbucket move %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            /* obj->nsph may have been insertted to the new sbucket with new spivot index, so update sph_id */
            obj->sph_id = obj->nsph->index;
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_SPLIT_ONCHIP_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            if (taps_used_as_em) {
            /* TAPS used as EM search in onchip mode, 
                        so dont't need to do tcam propagation*/
                TRANSITION_STATE(cookie,INS_ST_TCAM_PIVOT);
            } else {
                TRANSITION_STATE(cookie,INS_ST_DOMAIN_PROPAGATE_SPLIT);
            }
                        
TIME_STAMP("# INS_ST_DOMAIN_SPLIT state")
            break;

        case INS_ST_DOMAIN_PROPAGATE_SPLIT:
        /* sram/tcam propagation due to sram bucket split */
TIME_STAMP_START

            /* traverse through split domain,  propagate */
            rv = taps_sbucket_traverse(unit, arg->taps, obj->wgroup,
                                       obj->nsbh, &cookie, _sbucket_split_propagate_for_onchip_mode);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_ONCHIP_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            } 

            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_ONCHIP_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit & propagate"
                                      " relocated dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            } 
            

            TRANSITION_STATE(cookie,INS_ST_TCAM_PIVOT);

TIME_STAMP("# INS_ST_DOMAIN_PROPAGATE_SPLIT state")
            break;

        case INS_ST_TCAM_PIVOT:
        /* install tcam entry for sbucket created by sram bucket split
         * NOTE: it need to be done last to avoid packet goes to wrong destination
         */
TIME_STAMP_START
            if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                 rv = taps_tcam_insert_pivot(unit, arg->taps->tcam_hdl, obj->wgroup,
                                        &pivot[0], pivot_len, &bpm[0],
                                        obj->nsbh, &obj->ntph, 
                                        obj->tcam_entry_id, FALSE);
            } else {
                 assert(obj->tcam_entry_id >= 0);
                 /* coverity[negative_returns : FALSE] */
                 rv = taps_tcam_insert_pivot(unit, arg->taps->tcam_hdl, obj->wgroup,
                                        &pivot[0], pivot_len, &bpm[0],
                                        obj->nsbh, &obj->ntph, 
                                        obj->tcam_entry_id, TRUE);
            }
            
            if (SOC_FAILURE(rv)) {
                obj->tcam_entry_id = _TAPS_INV_ID_;
                /* Commit the tcam cmds since it is possible that some tcam entry have been moved by  
                             * calling taps_tcam_entry_alloc()
                             */
                if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                    rv_in = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
                    if (SOC_FAILURE(rv_in)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv_in, soc_errmsg(rv_in)));
                    }
                } 
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to Insert tcam pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            /* commit tph to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_ONCHIP_WP_1, rv);
            TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_ONCHIP_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }            

            if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
                /* Update the asso data for wsph */
                rv = taps_find_bpm_asso_data(unit, arg->taps->tcam_hdl, 
                                    &pivot[0], pivot_len, &bpm[0], &bpm_payload);
                if (SOC_SUCCESS(rv)) {
                    rv = taps_sbucket_enqueue_update_assodata_work(unit, 
                                                 arg->taps,
                                                 obj->nsbh,
                                                 obj->nsbh->wsph,
                                                 obj->wgroup,
                                                 bpm_payload[0] 
                                                 & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_));
                    TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_1, rv);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to update asso data for mode zero %d:%s!!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,INS);
                    } 
                    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                    TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_2, rv);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit sram bucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,INS);            
                    }
                } 
                
            }
            
            if (arg->taps->tcam_hdl->use_count < arg->taps->tcam_hdl->size) {
                TRANSITION_STATE(cookie,INS_ST_DONE);
            } else {
                TRANSITION_STATE(cookie,INS_ST_BUCKET_REDIST);
            }
TIME_STAMP("# INS_ST_TCAM_PIVOT state")
            break;
       case INS_ST_BUCKET_REDIST:
           rv = taps_bucket_redist(unit, arg->taps, obj->wgroup);
           if (SOC_FAILURE(rv)) {
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s: unit %d redistribute failed to redist bucket %d:%s !!!\n"), 
                          FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));

           }
           TRANSITION_STATE(cookie,INS_ST_DONE);

           /* error handling & cleanup */
        case INS_ST_ERROR:
TIME_STAMP_START
#ifdef TAPS_ERROR_COUNT
            arg->taps->error_count++;
#endif
            LOG_ERROR(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d failed to insert a route %d:%s !!!\n"), 
                         FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            taps_show_prefix(arg->taps->param.key_attr.lookup_length, arg->key, arg->length);
            LOG_CLI((BSL_META_U(unit,
                "taps 0x%x, tph 0x%x, sph 0x%x, nsph 0x%x, nsbh 0x%x, ntph 0x%x,\n"
                "domain_id %d, sph_id %d,  tcam_entry_id %d, vrf_def_route %d\n"),
                (unsigned int)arg->taps, (unsigned int)obj->tph, (unsigned int)obj->sph, 
                (unsigned int)obj->nsph,(unsigned int)obj->nsbh, (unsigned int)obj->ntph,
                obj->domain_id, obj->sph_id, obj->tcam_entry_id, obj->vrf_def_route));
            taps_command_destory(unit, arg->taps, obj->wgroup, work_type, COUNTOF(work_type));
            taps_insert_error_cleanup_for_onchip_mode(unit, &cookie);
            TRANSITION_STATE(cookie,INS_ST_DONE);
TIME_STAMP("# INS_ST_ERROR state")
            break;

        default:
            assert(0);
        }
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        (void)taps_work_group_destroy_for_all_devices(unit, arg->taps,
                            obj->wgroup, arg->taps->param.host_share_table);
    }
     
    /*TIME_STAMP("#### TAPS insert")*/
    return rv;
}

/*
 *
 * Function:
 *     _taps_insert_route_for_offchip_mode
 * Purpose:
 *     insert a route in offchip all mode
 */
static int _taps_insert_route_for_offchip_mode(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE, rv_in, pfx, pivot_id, slave_unit, slave_idx;
    taps_cookie_t cookie;
    taps_obj_t  *obj=NULL;
    unsigned int pivot_len=0;
    uint8 isbpm=0, ispfxpivot=0;
    taps_bucket_stat_e_t bkt_stat;
    trie_node_t  *split_trie_root = NULL;
    unsigned int bpm[_TAPS_MAX_KEY_WORDS_], pivot[_TAPS_MAX_KEY_WORDS_];
    _taps_redistribute_info_t redistribute_info;
    taps_work_type_e_t forced_work_type;
    
    /*TIME_STAMP_START*/
    
    /* validate */
    if (!arg->payload) return SOC_E_PARAM;
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    if (!TAPS_CAPACITY_LIMIT_DISABLED(&arg->taps->param) && 
        (arg->taps->capacity >= arg->taps->param.max_capacity_limit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d FIB Full length:%d Max:%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, arg->taps->capacity,
                   arg->taps->param.max_capacity_limit, soc_errmsg(SOC_E_FULL)));
        return SOC_E_FULL;
    }

    sal_memset(&redistribute_info, 0, sizeof(_taps_redistribute_info_t));
    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    sal_memset(&pivot[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
    cookie.state = cookie.oldstate = INS_ST_INIT;
    cookie.unit = unit;
    obj = &cookie.obj;
    obj->tcam_entry_id = _TAPS_INV_ID_;
    obj->domain_id = _TAPS_INV_ID_;

    while(cookie.state != INS_ST_DONE) {
        switch(cookie.state) {
        case INS_ST_INIT:
TIME_STAMP_START
            /*****
             * TAPS insert begin 
             *****/
            rv = _taps_find_prefix_objects(unit, arg, obj);
            if (SOC_SUCCESS(rv)) { 
                if (obj->dph) { /* Duplicate prefix */
                    rv = SOC_E_EXISTS;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "%s: unit %d Duplicate Prefix insertion %d:%s !!!\n"), 
                                 FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_STATE(cookie,INS_ST_DONE);
                    break;
                }
            }

            if (!(obj->tph && obj->sph && !obj->dph && rv == SOC_E_NOT_FOUND)) {
                TRANSITION_ERROR_HANDLE(cookie,INS);
            }

            /* create work group */
            rv = taps_work_group_create_for_all_devices(unit, arg->taps, arg->taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        arg->taps->param.host_share_table,
                                        obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);
            }
            /* if wild spivot is hit and tph length is shorter than vrf length, 
             * it means VRF is not created. if inserted entry is default entry (vrf/0)
             * insert else fail
             */
            if ((obj->sph->length == 0) && /* * spivot */
                (obj->tph->length < arg->taps->param.key_attr.vrf_length)) {
                /* vrf default route does not exist, validate provided routed */
                if (arg->length != arg->taps->param.key_attr.vrf_length) {
                    rv = SOC_E_PARAM;
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Default VRF route has"
                                          "to be created before adding routes within"
                                          " vrf table %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);
                } else {
                    obj->vrf_def_route = TRUE;
                    ispfxpivot = TRUE;
                    sal_memcpy(&pivot[0], arg->key, 
                               BITS2WORDS(arg->taps->param.key_attr.lookup_length) *
                               sizeof(unsigned int));
                    sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
                    _TAPS_SET_KEY_BIT(bpm, 0, arg->taps->param.key_attr.lookup_length);
                    pivot_len = arg->length;
                }
            }

            /* advance state */
            if (obj->vrf_def_route) {
                TRANSITION_STATE(cookie,INST_ST_VRF_DEF_ROUTE);
            } else {
                TRANSITION_STATE(cookie,INS_ST_DBKT_PFX);
            }
TIME_STAMP("# INS_ST_INIT state")
            break;
            
        case INST_ST_VRF_DEF_ROUTE:
        /* Insert VRF default route */
TIME_STAMP_START
            rv = taps_sbucket_pivot_id_alloc(unit, arg->taps,
                                             obj->tph->sbucket, 
                                             &obj->sph_id);
            if (SOC_FAILURE(rv)) {
                obj->sph_id = _TAPS_INV_ID_;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d vrf route failed to allocate spivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            /* create a dbucket to populate vrf route since there will 
             * never be a spivot associated */
            rv = taps_dbucket_create(unit, arg->taps, obj->wgroup[unit],
                                     obj->domain_id,  obj->sph_id, &obj->ndbh); 
            if (SOC_FAILURE(rv)) {
                obj->ndbh = NULL;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate dram bucket:%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);   
            }

            TRANSITION_STATE(cookie,INS_ST_DBKT_PFX);

TIME_STAMP("# INST_ST_VRF_DEF_ROUTE state")
            break;

        case INS_ST_DBKT_PFX:
            /* Insert & commit dram prefix on to dram bucket */
TIME_STAMP_START
            if (obj->vrf_def_route) {
                rv = taps_dbucket_insert_prefix(unit, arg->taps, obj->wgroup,
                                            obj->ndbh, arg->key, arg->length,
					    arg->payload, arg->cookie, arg->length, &obj->dph);
            } else {
                /* coverity[var_deref_op] */
                rv = taps_dbucket_insert_prefix(unit, arg->taps, obj->wgroup,
                                            obj->sph->dbh, arg->key, arg->length,
					    arg->payload, arg->cookie,
                                            obj->sph->length, 
                                            &obj->dph);
            }
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PFX_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to insert dprefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            } 
            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PFX_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit prefix to dram %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            obj->dph->bpm = FALSE;

            /* default routes do not require propagation */
            if (obj->vrf_def_route) {
                /* the vrf pivot has to be populated so advance states appropriately */
                TRANSITION_STATE(cookie,INS_ST_SBKT_PIVOT);
            }
#if 0
            else {
                rv = taps_dbucket_stat_get(unit, arg->taps, obj->sph->dbh, &bkt_stat);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to compute dbucket vacancy %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                }
                
                if (_TAPS_BKT_FULL == bkt_stat) {
                    /* pre-emptive split when the bucket size is 1 */
                    TRANSITION_STATE(cookie,INS_ST_DBKT_SPLIT);
                } else {
                    TRANSITION_STATE(cookie,INS_ST_DBKT_PROPAGATE);
                }
            }
#else
            else {
                TRANSITION_STATE(cookie,INS_ST_DBKT_PROPAGATE);
            }
#endif
TIME_STAMP("# INS_ST_DBKT_PFX state")
            break;

        case INS_ST_DBKT_PROPAGATE:
        /* propagate the inserted prefix in sbucket and tcam */
TIME_STAMP_START
            rv = taps_sbucket_propagate_prefix(unit, arg->taps, 
                                               obj->tph->sbucket, obj->wgroup,
                                               arg->key, arg->length, _BRR_INVALID_CPE_,
                                               TRUE, &obj->dph->bpm);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PROPAGATE_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d SRAM failed to propagate inserted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            rv =  taps_tcam_propagate_prefix(unit, arg->taps->tcam_hdl, obj->wgroup,
                                             obj->tph, TRUE, -1, arg->key, arg->length,
                                             &isbpm);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PROPAGATE_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d TCAM failed to propagate inserted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            obj->dph->bpm |= isbpm;

            /* commit the propagation work item to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PROPAGATE_WP_3, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit propagation %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = taps_dbucket_stat_get(unit, arg->taps, obj->sph->dbh, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute dbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            
            if (_TAPS_BKT_FULL == bkt_stat) {
                /* pre-emptive split when the bucket size is 1 */
                TRANSITION_STATE(cookie,INS_ST_DBKT_SPLIT);
            } else {
                TRANSITION_STATE(cookie,INS_ST_DONE);
            }

            /*TRANSITION_STATE(cookie,INS_ST_DONE);*/
TIME_STAMP("# INS_ST_DBKT_PROPAGATE state")
            break;

        case INS_ST_DBKT_SPLIT:
            /* dram bucket split when full */
TIME_STAMP_START

            /* before we do the real split, check if this split will cause 
             * fib to be full
             */
            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);
            } 
            if ((bkt_stat == _TAPS_BKT_ALMOST_FULL) &&
                (arg->taps->tcam_hdl->use_count == arg->taps->tcam_hdl->size)) {
                /* sbucket will be full after dbucket split, and there is no
                 * more tcam entry available. We better return here instead
                 * of split and clean up later.
                 */
                rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                                                obj->sph->dbh, obj->dph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to clean up when taps 0x%x is full %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, (uint32)arg->taps, rv, soc_errmsg(rv)));
                }               

                /* commit the DRAM cmds */
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit split dram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }

                return SOC_E_FULL;
            }

            /* invalidation if any will be queued by dbucket creation */
            rv = taps_sbucket_pivot_id_alloc(unit, arg->taps,
                                             obj->tph->sbucket, &obj->sph_id);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_SPLIT_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                obj->sph_id = _TAPS_INV_ID_;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d dram bucket split failed to allocate spivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));

            rv = taps_dbucket_split(unit, arg->taps, obj->wgroup, obj->sph_id,
                                    obj->sph->dbh, &pivot[0], &pivot_len,
                                    &bpm[0], &obj->ndbh);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_SPLIT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to split dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_SPLIT_WP_3, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit split dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS); 
            }

            TRANSITION_STATE(cookie, INS_ST_SBKT_PIVOT);
TIME_STAMP("# INS_ST_DBKT_SPLIT state")
            break;

        case INS_ST_SBKT_PIVOT:
            /* sram bucket update due to dram bucket split */
TIME_STAMP_START

            if ((obj->sph->dbh->trie0->trie == NULL) &&
		(obj->sph->dbh->trie1->trie == NULL) &&
		(obj->sph != obj->tph->sbucket->wsph)) {
		/* dbucket split empty out the old dbucket */
		rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], obj->sph->dbh); 
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy old dbucket emptied out by dbucket split %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		}

		/* delete the old spivot */
		pivot_id = obj->sph->index;
		rv = taps_sbucket_delete_pivot(unit, arg->taps, obj->wgroup, 
					       obj->tph->sbucket, obj->sph);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to clean up sram pivot when dbucket split empty old dbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		}
        
        obj->sph = NULL;

		/* free back pivot id */
		rv = taps_sbucket_pivot_id_free(unit, arg->taps, 
						obj->tph->sbucket, 
						pivot_id);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, pivot_id, rv, soc_errmsg(rv)));
		}
	    }

            rv = taps_sbucket_insert_pivot(unit, arg->taps, obj->wgroup, 
                                           obj->sph_id, obj->tph->sbucket,
                                           &pivot[0], pivot_len,
                                           ispfxpivot, NULL, NULL,
                                           obj->ndbh, &bpm[0], &obj->nsph);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to Insert sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            /* commit the propagation work item to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_SBKT_PIVOT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit sram pivot insert %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            if (_TAPS_BKT_FULL == bkt_stat) {
                /* pre-emptive split when the bucket size is 1 */
                TRANSITION_STATE(cookie,INS_ST_DOMAIN_SPLIT);
            } else {
                if (obj->vrf_def_route) { /* verify */
                    if ((bkt_stat == _TAPS_BKT_ALMOST_FULL) &&
                        (arg->taps->tcam_hdl->use_count == arg->taps->tcam_hdl->size)) {
                        /* sbucket will be full after insert, and there is no
                         * more tcam entry available. We better return here instead
                         * of split and clean up later.
                         */

                        /* remove the dph */
                        rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                                                        obj->ndbh, obj->dph);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to clean up when taps 0x%x is full %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, (uint32)arg->taps, rv, soc_errmsg(rv)));
                        }               

                        /* remove the dbucket */
                        rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], obj->ndbh); 
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to clean up dram bucket when taps 0x%x is full :%d :%s !!!\n"), 
                                       FUNCTION_NAME(), unit, (uint32)arg->taps, rv, soc_errmsg(rv)));
                        }

                        /* clean up the spivot */
                        rv = taps_sbucket_delete_pivot(unit, arg->taps, obj->wgroup, 
                                                       obj->tph->sbucket, obj->nsph);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to clean up sram pivot when taps 0x%x is full %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, (uint32)arg->taps, rv, soc_errmsg(rv)));
                        }

                        rv = taps_sbucket_pivot_id_free(unit, arg->taps, 
                                                        obj->tph->sbucket, 
                                                        obj->sph_id);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, obj->sph_id, rv, soc_errmsg(rv)));
                        }

                        return SOC_E_FULL;
                    }

                    TRANSITION_STATE(cookie,INS_ST_DONE);
                } else {
                    TRANSITION_STATE(cookie, INS_ST_DBKT_PROPAGATE_SPLIT);
                }
            } 
TIME_STAMP("# INS_ST_SBKT_PIVOT state")
            break;

        case INS_ST_DBKT_PROPAGATE_SPLIT:
            /* sram/tcam propagation due to dram bucket split */
TIME_STAMP_START
            assert(obj->ndbh != NULL);
            /* coverity[var_deref_op : FALSE] */
            rv = taps_dbucket_traverse(unit, arg->taps, obj->wgroup,
                                       obj->ndbh, &cookie, 
                                       _dbucket_split_propagate);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PROPAGATE_SPLIT_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate split dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }            

            /* commit the propagation work item to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DBKT_PROPAGATE_SPLIT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit propagation %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            TRANSITION_STATE(cookie,INS_ST_DONE);
TIME_STAMP("# INS_ST_DBKT_PROPAGATE_SPLIT state")
            break;

        case INS_ST_DOMAIN_SPLIT:
        /* sram bucket split when full */
TIME_STAMP_START
            sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
            /* To support unified mode, seperate domain split process to 2 stages
              * Stage1: Split trie, return root node of sub_trie which is splitted from original trie 
              * Stage2: Move entried from original trie to new trie, and set to configure hw
              *
              * Note: Stage 1 and Stage 2 MUST NOT be used separately */
                      
                        /* Stage 1 */
            if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                rv = taps_bucket_alloc(unit, arg->taps, 0, &obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    obj->domain_id = _TAPS_INV_ID_;
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate domain id  %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                }
            } else {
                /* coverity[var_deref_op] */
                rv = taps_sbucket_pivot_len_get(unit, arg->taps, obj->tph->sbucket,
                                                &pivot_len);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to get pivot len of split sbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS); 
                }
                
                 /* find a tcam free entry to insert */
                rv = taps_tcam_entry_alloc(unit, arg->taps->tcam_hdl, obj->wgroup, 
                                    pivot_len, &obj->tcam_entry_id);
                if (SOC_FAILURE(rv)) {
                    obj->tcam_entry_id = _TAPS_INV_ID_;
                    /* TCAM full, we should prevent this from happening by checking TCAM use counters */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Tcam segment %d failed to alloc entry for length %d\n"),
                               FUNCTION_NAME(), unit, arg->taps->tcam_hdl->segment, pivot_len));

                    /* Commit the tcam cmds since it is possible that some of tcam entries have been moved by  
                             * calling taps_tcam_entry_alloc()
                             */
                    rv_in = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
                    if (SOC_FAILURE(rv_in)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv_in, soc_errmsg(rv_in)));
                    }
                    TRANSITION_ERROR_HANDLE(cookie,INS);
                }

                cookie.tcam_pivot_len = pivot_len;
                            /* Allocate a sbucket id */
                rv = taps_bucket_alloc(unit, arg->taps, obj->tcam_entry_id, &obj->domain_id);
                if (SOC_FAILURE(rv)) {
                    obj->domain_id = _TAPS_INV_ID_;
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate domain %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                }
            }
            
            /* create a wild char spviot, dbucket */
            rv = taps_dbucket_create(unit, arg->taps, 
                                     obj->wgroup[unit], obj->domain_id, 
                                     _SBKT_WILD_CHAR_RSVD_POS_,
                                     &obj->wdbh);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_SPLIT_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate dram bucket:%d :%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);   
            }         
            rv = taps_sbucket_split_stage1(unit, arg->taps, obj->tph->sbucket,
                                        &pivot[0], &pivot_len, 
                                        &bpm[0], &split_trie_root);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to split sbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS); 
            }
            
            cookie.tcam_pivot_len = pivot_len;
            /* Stage2 */
            rv = taps_sbucket_split_offchip_stage2(unit, arg->taps, obj->wgroup, obj->tph->sbucket,
                                    obj->domain_id, split_trie_root,
                                    obj->wdbh, &obj->nsbh);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_SPLIT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to move domain entry %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM | _TAPS_COMMIT_FLAG_DRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_SPLIT_WP_3, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }

            TRANSITION_STATE(cookie,INS_ST_DOMAIN_PROPAGATE_SPLIT);
TIME_STAMP("# INS_ST_DOMAIN_SPLIT state")
            break;

        case INS_ST_DOMAIN_PROPAGATE_SPLIT:
            /* sram/tcam propagation due to sram bucket split */
TIME_STAMP_START

            /* traverse through split domain, relocate dram buckets, propagate */
            rv = taps_sbucket_traverse(unit, arg->taps, obj->wgroup,
                                       obj->nsbh, &cookie, _sbucket_split_propagate);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_1, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            } 

            /* dont issue propagate work item if dram relocate fails */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit & propagate"
                                      " relocated dram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS); 
            } else {
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit & propagate"
                                          " relocated dram bucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,INS);            
                } 
            }

            cookie.remove_sph = sal_alloc(sizeof(taps_spivot_handle_t) * 
                                        obj->tph->sbucket->prefix_number, "taps remove sph alloc");
            cookie.added_sph = sal_alloc(sizeof(taps_spivot_handle_t) * 
                                        obj->nsbh->prefix_number, "taps added sph alloc");
            sal_memset(cookie.remove_sph, 0, 
                        sizeof(taps_spivot_handle_t) * obj->tph->sbucket->prefix_number);
            sal_memset(cookie.added_sph, 0, 
                        sizeof(taps_spivot_handle_t) * obj->nsbh->prefix_number);
            /* scan through the old sbucket and try to find any prefix 
             * covered by the new tcam pivot. Move those prefixes from
             * the old sbucket to new sbucket.
             */
            sal_memcpy(&(redistribute_info.old_cookie), &cookie, sizeof(cookie));
            redistribute_info.cookie.unit = unit;
            sal_memcpy(redistribute_info.cookie.obj.wgroup, obj->wgroup, sizeof(obj->wgroup));
            redistribute_info.cookie.arg.taps = arg->taps;
            sal_memcpy(redistribute_info.pivot, &pivot[0], BITS2WORDS(arg->taps->param.key_attr.lookup_length) *
                       sizeof(unsigned int));
            redistribute_info.pivot_len = pivot_len;
            redistribute_info.pfx_cnt = 0;
            redistribute_info.pdph = sal_alloc(sizeof(taps_dprefix_handle_t)*
                                               arg->taps->param.sbucket_attr.max_pivot_number*
                                               arg->taps->param.dbucket_attr.num_dbucket_pfx * 2,
                                               "redistribution dph handles");
            if (redistribute_info.pdph == NULL) {
                rv = SOC_E_MEMORY;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to alloate redistribution cache %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            sal_memset(redistribute_info.pdph, 0, (sizeof(taps_dprefix_handle_t)* 
                                                   arg->taps->param.sbucket_attr.max_pivot_number*
                                                   arg->taps->param.dbucket_attr.num_dbucket_pfx * 2));
            
            rv = taps_sbucket_pm_traverse(unit, arg->taps, obj->wgroup,
                                          obj->tph->sbucket, &pivot[0], pivot_len,
                                          (void *)(&redistribute_info),
                                          _taps_redistribute_dbucket_after_split_cb);
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_3, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to find redistribute prefixes in "
                                      "old sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                #if 0
                if (redistribute_info.pdph) {
                    sal_free(redistribute_info.pdph);
                    redistribute_info.pdph = NULL;
                }
                #endif
                TRANSITION_ERROR_HANDLE(cookie,INS);        
            }
            cookie.redisted_dph = redistribute_info.pdph;
                
            /* go through the found prefixes and move to new sbucket */
            for (pfx = 0; pfx < redistribute_info.pfx_cnt; pfx++) {
                rv = _taps_redistribute_prefix_after_split(redistribute_info.pdph[pfx], &redistribute_info, &cookie);
                if (SOC_FAILURE(rv)) {
                    cookie.redisted_dph_count = pfx + 1;
                    break;       
                }
            }
            TAPS_ERROR_WATCH_POINT(INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_4, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to move redistribute prefixes in "
                                      "old sram bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                #if 0
                if (redistribute_info.pdph) {
                    sal_free(redistribute_info.pdph);
                    redistribute_info.pdph = NULL;
                }
                #endif
                TRANSITION_ERROR_HANDLE(cookie,INS); 
            }
#if 0
            if (redistribute_info.pdph) {
                sal_free(redistribute_info.pdph);
                redistribute_info.pdph = NULL;
            }
#endif            
            
            TRANSITION_STATE(cookie,INS_ST_TCAM_PIVOT);

TIME_STAMP("# INS_ST_DOMAIN_PROPAGATE_SPLIT state")
            break;

        case INS_ST_TCAM_PIVOT:
            /* install tcam entry for sbucket created by sram bucket split
             * NOTE: it need to be done last to avoid packet goes to wrong destination
             */
TIME_STAMP_START
            if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                rv = taps_tcam_insert_pivot(unit, arg->taps->tcam_hdl, obj->wgroup,
                                        &pivot[0], pivot_len, &bpm[0],
                                        obj->nsbh, &obj->ntph, 
                                        obj->tcam_entry_id, FALSE);
            } else {
                assert(obj->tcam_entry_id >= 0);
                /* coverity[negative_returns : FALSE] */
                rv = taps_tcam_insert_pivot(unit, arg->taps->tcam_hdl, obj->wgroup,
                                        &pivot[0], pivot_len, &bpm[0],
                                        obj->nsbh, &obj->ntph, 
                                        obj->tcam_entry_id, TRUE);
            }
            if (SOC_FAILURE(rv)) {
                obj->tcam_entry_id = _TAPS_INV_ID_;
                /* Commit the tcam cmds since it is possible that some tcam entry have been moved by  
                             * calling taps_tcam_entry_alloc()
                             */
                if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
                    rv_in = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
                    if (SOC_FAILURE(rv_in)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv_in, soc_errmsg(rv_in)));
                    }
                }      
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to Insert tcam pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }
            /* Remove redundant sph */
            if (cookie.remove_sph_count != 0) {
                obj->wgroup[unit]->force_work_type_enable = TRUE;
                obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
                if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table) 
                    && !_taps_get_caching(unit)) {
                    for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
                        slave_unit = arg->taps->slave_units[slave_idx];
                        obj->wgroup[slave_unit]->force_work_type_enable = TRUE;
                        obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
                    }
                }
                rv = taps_sbucket_redundant_pivot_clear(unit, arg->taps, obj->wgroup, 
                                        cookie.remove_sph[0]->sbh, cookie.remove_sph, cookie.remove_sph_count);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to remove redundant spivot %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                    
                obj->wgroup[unit]->force_work_type_enable = FALSE;
                if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit,  arg->taps->param.host_share_table)
                     && !_taps_get_caching(unit)) {
                    for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
                        slave_unit = arg->taps->slave_units[slave_idx];
                        obj->wgroup[slave_unit]->force_work_type_enable = FALSE;
                    }
                }
            }
            
            /* commit STAGE 1 and STAGE 2. STAGE 3 work need to be commited after the tcam insert */
            forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
            rv = taps_work_commit(unit, obj->wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE 1 works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);        
            }
            forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
            rv = taps_work_commit(unit, obj->wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE 2 works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);        
            }
            /* commit tph to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit tcam pivot insert %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);            
            }            
            
            /* commit redistribution STAGE 3 work */
            forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
            rv = taps_work_commit(unit, obj->wgroup[unit], &forced_work_type, 1, _TAPS_BULK_COMMIT);
            TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_WP_1, rv);
            TAPS_ERROR_WATCH_POINT(INS_ST_TCAM_PIVOT_WP_2, rv);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE 3 works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,INS);        
            }
#ifdef REDISTRIBUTE_DBG
#if 0
        LOG_CLI((BSL_META_U(unit,
                            "%s: Redistribution Result unit %d Taps handle 0x%x old TPH 0x%x new TPH 0x%x\n"),
                 FUNCTION_NAME(), unit, (uint32)(arg->taps), (uint32)(obj->tph), (uint32)(obj->ntph)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Redistribution Result unit %d old sram bucket\n"), FUNCTION_NAME(), unit));
        taps_tcam_pivot_dump(unit, arg->taps->tcam_hdl, obj->tph, TAPS_DUMP_TCAM_ALL | \
                 TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL);
        LOG_CLI((BSL_META_U(unit,
                            "\n\n%s: Redistribution Result unit %d new sram bucket\n"), FUNCTION_NAME(), unit));
        taps_tcam_pivot_dump(unit, arg->taps->tcam_hdl, obj->ntph, TAPS_DUMP_TCAM_ALL | \
                 TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL);
#endif
#endif
            if (arg->taps->tcam_hdl->use_count < arg->taps->tcam_hdl->size) {
                TRANSITION_STATE(cookie,INS_ST_DONE);
            } else {
                TRANSITION_STATE(cookie,INS_ST_BUCKET_REDIST);
            }
            
TIME_STAMP("# INS_ST_TCAM_PIVOT state")
            break;
        case INS_ST_BUCKET_REDIST:
            
            rv = taps_bucket_redist(unit, arg->taps, obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to redist bucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));

            }

            TRANSITION_STATE(cookie,INS_ST_DONE);
            break;
           /* error handling & cleanup */
        case INS_ST_ERROR:
TIME_STAMP_START
#ifdef TAPS_ERROR_COUNT
            arg->taps->error_count++;
#endif
            LOG_ERROR(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d failed to insert a route %d:%s !!!\n"), 
                         FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            taps_show_prefix(arg->taps->param.key_attr.lookup_length, arg->key, arg->length);
            LOG_CLI((BSL_META_U(unit,
                "taps 0x%x, tph 0x%x, sph 0x%x, dph 0x%x, ndbh 0x%x, nsph 0x%x, nsbh 0x%x, ntph 0x%x,\n"
                "domain_id %d, sph_id %d,  tcam_entry_id %d, vrf_def_route %d\n"),
                (unsigned int)arg->taps, (unsigned int)obj->tph, (unsigned int)obj->sph, 
                (unsigned int)obj->dph, (unsigned int)obj->ndbh, (unsigned int)obj->nsph, 
                (unsigned int)obj->nsbh, (unsigned int)obj->ntph, obj->domain_id, obj->sph_id, 
                obj->tcam_entry_id, obj->vrf_def_route));
            taps_command_destory(unit, arg->taps, obj->wgroup, work_type, COUNTOF(work_type));
            taps_insert_error_cleanup_for_offchip_mode(unit, &cookie);
            TRANSITION_STATE(cookie,INS_ST_DONE);
TIME_STAMP("# INS_ST_ERROR state")
            break;

        default:
            assert(0);
        }
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        (void)taps_work_group_destroy_for_all_devices(unit, arg->taps, 
                                obj->wgroup, arg->taps->param.host_share_table);
    }

    if (redistribute_info.pdph) {
        sal_free(redistribute_info.pdph);
    }
    if (cookie.remove_sph) {
        sal_free(cookie.remove_sph);
    }
    if(cookie.added_sph) {
        sal_free(cookie.added_sph);
    }
    /*TIME_STAMP("#### TAPS insert")*/
    return rv;
}

/*
 *
 * Function:
 *     taps_insert_route
 * Purpose:
 *     insert a route into taps
 */
int taps_insert_route(int unit, taps_arg_t *arg) 
{
    int rv;
    
    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }

    sal_mutex_take(arg->taps->taps_mutex, sal_mutex_FOREVER);
    
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = _taps_insert_route_for_offchip_mode(unit, arg);
    } else {
        rv = _taps_insert_route_for_onchip_mode(unit, arg);
    }
    
    sal_mutex_give(arg->taps->taps_mutex);
    return rv;
}

static int _taps_delete_route_for_onchip_mode(int unit,  taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_cookie_t cookie;
    taps_obj_t  *obj=NULL;
    taps_bucket_stat_e_t bkt_stat;
    uint8 isbpm = 0;
    unsigned int prefix_id=0;

    /* validate */
    if (!arg->payload) return SOC_E_PARAM;
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    cookie.state = cookie.oldstate = DEL_ST_INIT;
    cookie.unit = unit;
    obj = &cookie.obj;
    while(cookie.state != DEL_ST_DONE) {
        switch(cookie.state) {
        case DEL_ST_INIT:
            /*****
             * TAPS delete begin 
             *****/
            rv = _taps_find_prefix_objects(unit, arg, obj);
            if (SOC_FAILURE(rv) || !obj->sph) { 
                /* prefix not found */
                rv = SOC_E_NOT_FOUND;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "%s: unit %d Prefix not found %d:%s !!!\n"), 
                             FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);
            }

            /* create work group */
            rv = taps_work_group_create_for_all_devices(unit, arg->taps, arg->taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        arg->taps->param.host_share_table,
                                        obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);
            }

            /* advance state */
            TRANSITION_STATE(cookie,DEL_ST_SBKT_PIVOT);
            break;

        case DEL_ST_SBKT_PIVOT:
            /* delete sram pivot when corresponding dram bucket is deleted */ 

            /* destroy sram pivot */
            /* coverity doesn't understand state machines */
            /* coverity[var_deref_op] */
            prefix_id = obj->sph->index;
            
            rv = taps_sbucket_delete_pivot(unit, arg->taps, obj->wgroup,
                                           obj->tph->sbucket, 
                                           obj->sph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);     
            } 


            /* free back pivot id */
            rv = taps_sbucket_pivot_id_free(unit, arg->taps, 
                                            obj->tph->sbucket, 
                                            prefix_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, prefix_id, rv, soc_errmsg(rv)));
            }
            rv =  taps_tcam_propagate_prefix_for_onchip(unit, arg->taps->tcam_hdl, obj->wgroup,
                                                        obj->tph, FALSE, -1, arg->key, arg->length,
                                                        NULL, &isbpm);
            if (SOC_FAILURE(rv)) {
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s: unit %d TCAM failed to propagate deleted prefix %d:%s !!!\n"), 
                          FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
               TRANSITION_ERROR_HANDLE(cookie, DEL);            
            }

            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM | _TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit prefix to dram %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);           
            } 
              
            /* destroy the sbucket & unlink */
            if ((arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS && bkt_stat == _TAPS_BKT_EMPTY)
                || (arg->taps->param.mode == TAPS_ONCHIP_ALL && bkt_stat == _TAPS_BKT_WILD 
                && obj->tph->sbucket->wsph->length == 0 && obj->tph->length != 0)) {
                /* Reserve Domain Id before delete this bucket*/
                obj->domain_id = obj->tph->sbucket->domain;
                rv = taps_sbucket_destroy(unit, arg->taps,
                                          obj->wgroup, obj->tph->sbucket);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy sbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);           
                }

                /* Commit the generated SRAM commands */
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit to sram %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);            
                }                

                obj->tph->sbucket = NULL; /* unlink tcam pivot & sram bucket */
                TRANSITION_STATE(cookie,DEL_ST_TCAM_PIVOT);
            } else {
                TRANSITION_STATE(cookie,DEL_ST_DONE);
            }
            break;

        case DEL_ST_TCAM_PIVOT:
            /* Destory domain ID when corresponding sram bucket is deleted */ 
            rv = taps_bucket_free(unit, cookie.arg.taps, obj->domain_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, obj->tph->entry,
                           rv, soc_errmsg(rv)));
            }

            rv = taps_tcam_delete_pivot(unit, arg->taps->tcam_hdl,
                                        obj->wgroup, obj->tph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }

            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit tcam pivot delete %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            TRANSITION_STATE(cookie,DEL_ST_DONE);
            break;

        case DEL_ST_ERROR:
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d failed to delete route %d:%s !!!\n"), 
                         FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            /* LOG_CLI((BSL_META_U(unit,
                                   "\n ADD SUPPORT FOR DELETE ERROR CLEANUP !!!!! \n"))); */
            TRANSITION_STATE(cookie,DEL_ST_DONE);
            break;

        default:
            assert(0);
            break;
        }
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        rv = taps_work_group_destroy_for_all_devices(unit, arg->taps, 
                                    obj->wgroup, arg->taps->param.host_share_table);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to destroy groups %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } 
    }

    return rv;
}


static int _taps_delete_route_for_offchip_mode(int unit,  taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_cookie_t cookie;
    taps_obj_t  *obj=NULL;
    taps_bucket_stat_e_t bkt_stat;
    uint8 isbpm=0;
    uint8 pivot_id=0;

    /* validate */
    if (!arg->payload) return SOC_E_PARAM;
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    cookie.state = cookie.oldstate = DEL_ST_INIT;
    cookie.unit = unit;
    obj = &cookie.obj;

    while(cookie.state != DEL_ST_DONE) {
        switch(cookie.state) {
        case DEL_ST_INIT:
            /*****
             * TAPS delete begin 
             *****/
            rv = _taps_find_prefix_objects(unit, arg, obj);
            if (SOC_FAILURE(rv) || (!obj->dph)) { 
                rv = SOC_E_NOT_FOUND;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "%s: unit %d Prefix not found %d:%s !!!\n"), 
                             FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);
            }

            /* create work group */
            rv = taps_work_group_create_for_all_devices(unit, arg->taps, arg->taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        arg->taps->param.host_share_table,
                                        obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);
            }

            /* advance state */
            TRANSITION_STATE(cookie,DEL_ST_DBKT_PFX);
            break;

        case DEL_ST_DBKT_PFX:
            /* delete the prefix from the dram bucket */
            rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                                            obj->sph->dbh, obj->dph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to delete dram prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            /* Propagate & commit route delete */
            rv = taps_sbucket_propagate_prefix(unit, arg->taps, 
                                               obj->tph->sbucket, obj->wgroup,
                                               arg->key, arg->length, _BRR_INVALID_CPE_,
                                               FALSE, &obj->dph->bpm);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d SRAM failed to propagate deleted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            rv =  taps_tcam_propagate_prefix(unit, arg->taps->tcam_hdl, obj->wgroup,
                                             obj->tph, FALSE, -1, arg->key, arg->length,
                                             &isbpm);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d TCAM failed to propagate deleted prefix %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie, DEL);            
            }

            /* commit the propagation work item to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit propagation %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit prefix to dram %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            rv = taps_dbucket_stat_get(unit, arg->taps, obj->sph->dbh, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute dbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            /* Never free wild bucket */
            if (bkt_stat == _TAPS_BKT_EMPTY) {
                /* if wild pivot, free it only if there are no more spivots on the 
                 * sbucket */
                if (obj->sph != obj->tph->sbucket->wsph) {
                    rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], obj->sph->dbh);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to free dbucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,DEL);            
                    }
                    obj->sph->dbh = NULL; /* unlink dbh & sram pivot */
                    TRANSITION_STATE(cookie,DEL_ST_SBKT_PIVOT);
                } else {
                    TRANSITION_STATE(cookie,DEL_ST_WILD_SPIVOT);
                }
            } else {
                TRANSITION_STATE(cookie,DEL_ST_DONE);
            }
            break;

        case DEL_ST_WILD_SPIVOT:
            /* delete wild (*) pivot in sram bucket */
            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);           
            } 

            /* if only wild pivot exists then destroy this sbucket & wild dbucket */
            if (_TAPS_BKT_WILD == bkt_stat) {
                taps_dbucket_handle_t wdbh = obj->sph->dbh;

                /* destroy sbucket */
                rv = taps_sbucket_destroy(unit, arg->taps,
                                          obj->wgroup, obj->tph->sbucket);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to destroy sbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);           
                }

                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit wild sbucket delete %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);            
                }

                obj->tph->sbucket = NULL; /* unlink tcam pivot & sram bucket */

                /* destroy wild dbucket */
                rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], wdbh);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free dbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);            
                }
                obj->sph = NULL; /* unlink dbh & sram pivot */

                /* optimize: commit is not really required for dbucket since spivot should take care ?*/
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to commit wild dbucket delete %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);            
                }
                TRANSITION_STATE(cookie,DEL_ST_TCAM_PIVOT);
            } else {
                TRANSITION_STATE(cookie,DEL_ST_DONE);
            }
            break;

        case DEL_ST_SBKT_PIVOT:
            /* delete sram pivot when corresponding dram bucket is deleted */ 

            /* destroy sram pivot */
            /* coverity[var_deref_op : FALSE] */
            pivot_id = obj->sph->index;
            
            rv = taps_sbucket_delete_pivot(unit, arg->taps, obj->wgroup,
                                           obj->tph->sbucket, 
                                           obj->sph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);     
            } 

            assert(pivot_id != 0);
            
            /* free back pivot id */
            rv = taps_sbucket_pivot_id_free(unit, arg->taps, 
                                            obj->tph->sbucket, 
                                            pivot_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free sram pivot id:%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, pivot_id, rv, soc_errmsg(rv)));
            }

            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit prefix to dram %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);           
            } 
              
            /* destroy the sbucket & unlink */
            if (bkt_stat == _TAPS_BKT_WILD) {
                taps_dbucket_handle_t wdbh = obj->tph->sbucket->wsph->dbh;

                /* check if wild spivot associated dbucket is empty */
                rv = taps_dbucket_stat_get(unit, arg->taps, wdbh, &bkt_stat);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to compute dbucket vacancy %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    TRANSITION_ERROR_HANDLE(cookie,DEL);            
                }

                if (bkt_stat == _TAPS_BKT_EMPTY) {
                    /* Reserve Domain Id before delete this bucket*/
                    obj->domain_id = obj->tph->sbucket->domain;
                    
                    rv = taps_sbucket_destroy(unit, arg->taps,
                                              obj->wgroup, obj->tph->sbucket);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to destroy sbucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,DEL);           
                    }
                    
                    /* Commit the generated SRAM commands */
                    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit to sram %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,DEL);            
                    }                
                    
                    obj->tph->sbucket = NULL; /* unlink tcam pivot & sram bucket */

                    /* destroy wild dbucket */
                    rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], wdbh);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to free dbucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,DEL);            
                    }
                    obj->sph = NULL; /* unlink dbh & sram pivot */

                    /* optimize: commit is not really required for dbucket since spivot should take care ?*/
                    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to commit wild dbucket delete %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,DEL);            
                    }
                    TRANSITION_STATE(cookie,DEL_ST_TCAM_PIVOT);
                } else {
                    TRANSITION_STATE(cookie,DEL_ST_DONE);
                }
            } else {
                TRANSITION_STATE(cookie,DEL_ST_DONE);
            }
            break;
        case DEL_ST_TCAM_PIVOT:
            /* delete tcam entry when corresponding sram bucket is deleted */ 
            rv = taps_bucket_free(unit, cookie.arg.taps, obj->domain_id);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to free tcam domain id:%u  %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, obj->tph->entry,
                           rv, soc_errmsg(rv)));
            }

            rv = taps_tcam_delete_pivot(unit, arg->taps->tcam_hdl,
                                        obj->wgroup, obj->tph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to delete tcam pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }

            /* commit the prefix to hardware */
            rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_TCAM);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit tcam pivot delete %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,DEL);            
            }

            TRANSITION_STATE(cookie,DEL_ST_DONE);
            break;

        case DEL_ST_ERROR:
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d failed to delete route %d:%s !!!\n"), 
                         FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            /* LOG_CLI((BSL_META_U(unit,
                                   "\n ADD SUPPORT FOR DELETE ERROR CLEANUP !!!!! \n"))); */
            TRANSITION_STATE(cookie,DEL_ST_DONE);
            break;

        default:
            assert(0);
            break;
        }
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        rv = taps_work_group_destroy_for_all_devices(unit, arg->taps, 
                                obj->wgroup, arg->taps->param.host_share_table);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to destroy groups %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } 
    }

    return rv;
}


int taps_delete_route(int unit,  taps_arg_t *arg)
{
    int rv;
    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }
    
    sal_mutex_take(arg->taps->taps_mutex, sal_mutex_FOREVER);
    
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = _taps_delete_route_for_offchip_mode(unit, arg);
    } else {
        rv = _taps_delete_route_for_onchip_mode(unit, arg);
    }

    sal_mutex_give(arg->taps->taps_mutex);
    return rv;

}

/*
 *
 * Function:
 *     taps_update_route
 * Purpose:
 *     update a route in taps
 */
int taps_update_route(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_cookie_t cookie;
    taps_obj_t  *obj=NULL;

    /* validate */
    if (!arg->payload) return SOC_E_PARAM;

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }
    sal_mutex_take(arg->taps->taps_mutex, sal_mutex_FOREVER);
    
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    cookie.state = cookie.oldstate = UPD_ST_INIT;
    cookie.unit = unit;
    obj = &cookie.obj;

    while(cookie.state != UPD_ST_DONE) {
        switch(cookie.state) {
        case UPD_ST_INIT:
            /*****
             * TAPS update begin 
             *****/
            rv = _taps_find_prefix_objects(unit, arg, obj);
            if (SOC_SUCCESS(rv)) {
                if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
                    if (!obj->dph) { /* prefix not found */
                        rv = SOC_E_NOT_FOUND;
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Prefix not found %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,UPD);
                    }
                } else {
                    if (!obj->sph) { /* prefix not found */
                        rv = SOC_E_NOT_FOUND;
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Prefix not found %d:%s !!!\n"), 
                                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,UPD);
                    }
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Prefix lookup error %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,UPD);                
            }

            /* create work group */
            rv = taps_work_group_create_for_all_devices(unit, arg->taps, arg->taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        arg->taps->param.host_share_table,
                                        obj->wgroup);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                TRANSITION_ERROR_HANDLE(cookie,UPD);
            }
            if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
                 /* update payload in dph */
                sal_memcpy(&obj->dph->payload[0], 
                            arg->payload, 
			   sizeof(uint32)*_TAPS_PAYLOAD_WORDS_);
		obj->dph->cookie = arg->cookie;

                /* update payload */
                rv = taps_dbucket_enqueue_update_payload_work(unit, arg->taps,
                                      obj->wgroup,
                                      obj->sph->dbh,
                                      obj->dph);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to update payload for dph 0x%x %d:%s !!!\n"), 
                                   FUNCTION_NAME(), 
                                   unit, 
                                   (uint32)obj->dph, 
                                   rv, 
                                   soc_errmsg(rv)));
                        TRANSITION_ERROR_HANDLE(cookie,UPD);
                    }

                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit update for mode 2 %d:%s!!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                } 
            }
            else if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS){
                 /* update payload in sph */
                sal_memcpy(&obj->sph->payload[0], 
			   arg->payload, 
			   sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);
		obj->sph->cookie = arg->cookie;

                rv = taps_update_offchip_payload_work(unit, arg->taps, 
                                                    obj->wgroup, obj->sph);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to update payload %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit update for mode 1 %d:%s!!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                } 
            } else {
                /* update payload in sph */
                obj->sph->payload[0] = arg->payload[0] & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
		obj->sph->cookie = arg->cookie;

                rv = taps_sbucket_enqueue_update_assodata_work(unit, 
                                     arg->taps,
                                     obj->tph->sbucket,
                                     obj->sph,
                                     obj->wgroup,
                                     obj->sph->payload[0]);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to update asso data for mode 0 %d:%s!!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                } 
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM);
                if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to commit update %d:%s!!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                } 
            }
       

            /* advance state */
            TRANSITION_STATE(cookie,UPD_ST_DONE);
            break;

        case UPD_ST_ERROR:
            LOG_CLI((BSL_META_U(unit,
                                "\n ADD SUPPORT FOR UPDATE ERROR CLEANUP !!!!! \n")));
            TRANSITION_STATE(cookie,UPD_ST_DONE);
            break;

        default:
            assert(0);
            break;
        }
    }

    /* destroy work group */
    if (obj->wgroup[unit]) {
        rv = taps_work_group_destroy_for_all_devices(unit, arg->taps, 
                            obj->wgroup, arg->taps->param.host_share_table);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to destroy groups %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } 
    }

    sal_mutex_give(arg->taps->taps_mutex);
    return rv;
}

/*
 *
 * Function:
 *     taps_get_route
 * Purpose:
 *     get a route information
 */
int taps_get_route(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_cookie_t cookie;
    taps_obj_t  *obj=NULL;


    /* validate */
    if (!arg->payload) return SOC_E_PARAM;

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }
    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));

    /* initialize cookie */
    sal_memset(&cookie, 0, sizeof(cookie));
    sal_memcpy(&cookie.arg, arg, sizeof(taps_arg_t));
    cookie.unit = unit;
    obj = &cookie.obj;

    /* search software database to get associated objects */
    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_SUCCESS(rv)) {
        if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
            if (obj->dph) { /* prefix found */
		sal_memcpy(arg->payload, obj->dph->payload,
			   _TAPS_PAYLOAD_WORDS_ * sizeof(unsigned int ));
		arg->cookie = obj->dph->cookie;	    
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        } else {
            if (obj->sph) {
                if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
                    sal_memcpy(arg->payload, obj->sph->payload,
                       _TAPS_PAYLOAD_WORDS_ * sizeof(unsigned int ));
                } else {
                    arg->payload[0] = obj->sph->payload[0] 
                                    & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
                }
		arg->cookie = obj->sph->cookie;
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        }
        
    } else {
        rv = SOC_E_NOT_FOUND;
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_find_bpm
 * Purpose:
 *     find best prefix match for given route
 */
int taps_find_bpm(int unit, taps_arg_t *arg, unsigned int *bpm_length)
{
    int rv = SOC_E_NONE;
    taps_obj_t obj;

    if (!arg || !bpm_length) return SOC_E_PARAM;

    SOC_IF_ERROR_RETURN(_taps_validate_args(unit, arg));
    sal_memset(&obj, 0, sizeof(obj));
    *bpm_length = 0;

    rv = _taps_find_prefix_objects(unit, arg, &obj);
    if (!obj.dph && rv == SOC_E_NOT_FOUND) rv = SOC_E_NONE;

    if (SOC_SUCCESS(rv)) {
        if (!obj.tph || !obj.sph) return SOC_E_INTERNAL;

        /* try to find bpm within sbucket */
        rv = taps_sbucket_find_bpm(unit, arg->taps,
                                   obj.tph->sbucket,
                                   arg->key, arg->length,
                                   bpm_length);
        if (SOC_SUCCESS(rv)) {
            /* try to find bpm on tcam if not found within sbucket domain */
            if (*bpm_length == 0) {
                rv = taps_tcam_find_bpm(unit, arg->taps->tcam_hdl,
                                        arg->key, arg->length,
                                        bpm_length);
            }
        }
    }

    return rv;
}

int _taps_find_prefix_pointer(int unit, 
                              taps_handle_t taps,
                              uint32 *key, uint32 length,
                              uint32 *pointer, uint8 global)
{
    int rv = SOC_E_NONE;
    taps_arg_t arg;
    taps_obj_t obj;

    if (!key || !taps || !pointer) return SOC_E_PARAM;

    sal_memset(&obj, 0, sizeof(obj));
    arg.taps = taps;
    arg.key  = key;
    arg.length = length;
    *pointer = _TAPS_INV_ID_;
    
    rv = _taps_find_prefix_objects(unit, &arg, &obj);
    assert(obj.tph);
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        if(SOC_SUCCESS(rv) && obj.dph) {
            *pointer = obj.sph->dbh->bucket * 
               taps->param.dbucket_attr.num_dbucket_pfx * 2+ 
               obj.dph->index;
            if (global) {
                *pointer += (obj.tph->sbucket->domain * taps->param.sbucket_attr.max_pivot_number *
                             taps->param.dbucket_attr.num_dbucket_pfx * 2);
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to find bpm prefix pointer in sbucket %d"
                              "dbucket %d %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, 
                   obj.tph->sbucket->domain, obj.sph==NULL?-1:obj.sph->index,
                   rv, soc_errmsg(rv)));
            taps_show_prefix(taps->param.key_attr.lookup_length, key, length);
        }
    } else {
        if (obj.sph && !taps_used_as_em) {
            /* Find a route and don't use taps as em search*/
            *pointer = obj.tph->sbucket->domain * (1 << (taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_
                        + obj.sph->index;
        } else {
            /* no prefix found, so return wild tcam entry */
            if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
                /* Parallel mode, wildcard entry's index is 0, in taps0 */
                *pointer = 0;
            } else {
                /* Unified mode, wildcard entry's index equal to global_base, in taps1 */  
                *pointer = taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry *
                         (1 << (taps->param.tcam_layout)) * 
                         _MAX_SBUCKET_PIVOT_PER_BB_;
            }
            rv = SOC_E_NONE;
        }
    }      
    return rv;
}

/*
 *
 * Function:
 *     taps_find_prefix_global_pointer
 * Purpose:
 *     given a prefix return the global bucket pointer
 */
int taps_find_prefix_global_pointer(int unit, 
                    taps_handle_t taps,
                    uint32 *key, uint32 length,
                    uint32 *bpm_global_index)
{
    int rv = SOC_E_NONE;
    
    rv = _taps_find_prefix_pointer(unit, taps, key,
                                   length, bpm_global_index, TRUE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to find global pointer %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
   
    return rv;
}

/*
 *
 * Function:
 *     taps_find_prefix_local_pointer
 * Purpose:
 *     given a prefix return the local bucket pointer
 */
int taps_find_prefix_local_pointer(int unit, 
                   taps_handle_t taps,
                   uint32 *key, uint32 length,
                   uint32 *bpm_local_index)
{
    int rv = SOC_E_NONE;

    rv =  _taps_find_prefix_pointer(unit, taps, key,
                                    length, bpm_local_index, FALSE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to find local pointer %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}


static int _taps_calculate_prefix_pointer(int unit, 
                      taps_handle_t taps,
                      taps_sbucket_handle_t sbh,
                      taps_dbucket_handle_t dbh,
                      void                 *p_prefix_handle,
                      uint32 *pointer, uint8 global)
{
    /* both local and global pointer requires sph and dph */
    if (!taps) return SOC_E_PARAM;

    /* global pointer also requires tph */
    if (global && !sbh) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        taps_dprefix_handle_t dph = (taps_dprefix_handle_t)p_prefix_handle;
        *pointer = dbh->bucket * taps->param.dbucket_attr.num_dbucket_pfx * 2 + 
                dph->index;
        if (global) {
            *pointer += (sbh->domain * taps->param.sbucket_attr.max_pivot_number *
                         taps->param.dbucket_attr.num_dbucket_pfx * 2);
        }
        
    } else {
        taps_spivot_handle_t sph = (taps_spivot_handle_t)p_prefix_handle;
        *pointer = sbh->domain * (1 << (taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_ + sph->index;
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     taps_calculate_prefix_global_pointer
 * Purpose:
 *     given software object handles return the global bucket pointer
 */
static int taps_calculate_prefix_global_pointer(int unit, 
                        taps_handle_t taps,
                        taps_tcam_pivot_handle_t tph,
                        taps_spivot_handle_t sph,
                        taps_dprefix_handle_t dph,
                        uint32 *bpm_global_index)
{
    int rv = SOC_E_NONE;
    
    rv = _taps_calculate_prefix_pointer(unit, taps, tph->sbucket,
                    sph->dbh, dph, bpm_global_index, TRUE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to calculate global pointer %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_calculate_prefix_local_pointer
 * Purpose:
 *     given software object handles return the local bucket pointer
 */
int taps_calculate_prefix_local_pointer(int unit, 
                    taps_handle_t taps,
                    taps_spivot_handle_t sph,
                    taps_dprefix_handle_t dph,
                    uint32 *bpm_local_index)
{
    int rv = SOC_E_NONE;

    rv =  _taps_calculate_prefix_pointer(unit, taps, NULL, sph->dbh, dph,
                     bpm_local_index, FALSE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to calculate local pointer %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_instance_and_entry_id_get
 * Purpose:
 *     Get hardware instance, tcam_entry_id, sbucket_id
 */
int taps_instance_and_entry_id_get(int unit, const taps_handle_t taps, 
                                int tcam_entry, int sbucket_id,
                                int *hwinstance, int *hw_tcam_entry, int *hw_sbucket_id)
{
    if (hwinstance == NULL
        || hw_tcam_entry == NULL
        || hw_sbucket_id == NULL) {
        return SOC_E_PARAM;
    }
    
    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
        /* Parallel mode */
        /* Hardware configuration infor is the same with what we have in software */
       *hwinstance = taps->hwinstance;
       *hw_tcam_entry = tcam_entry;
       *hw_sbucket_id = sbucket_id;
    } else {
        /* Unified mode */
        if (tcam_entry < taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry) {
            /* If tcam_entry is in taps0, the hardware configuration infor is also the same
                     * with what we have in software */
            *hwinstance = SW_INST_CONVERT_TO_HW_INST(TAPS_INST_0);
            *hw_tcam_entry = tcam_entry;
            *hw_sbucket_id = sbucket_id;
        } else {
            /* If tcam_entry is in taps1, should minus the offset which equal to 
                      * the number of entry we have in taps0 */
            *hwinstance = SW_INST_CONVERT_TO_HW_INST(TAPS_INST_1);
            *hw_tcam_entry = tcam_entry 
                        - taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
            *hw_sbucket_id = sbucket_id 
                        - taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
        }
    }

    return SOC_E_NONE;
}


typedef struct _taps_dump_datum_s {
    taps_handle_t taps;
    int unit;
    uint32 flags;
} taps_dump_datum_t;


int _taps_dbucket_dump(taps_spivot_handle_t sph, void *user_data)
{
    int rv=SOC_E_NONE;
    taps_dump_datum_t *cbdata = (taps_dump_datum_t*)user_data;
    
    if (!sph || !user_data) return SOC_E_PARAM;

    if (TAPS_DUMP_DRAM_FLAGS(cbdata->flags)) {
        
        /* dump spivot again to coordiante with dbucket dump */
        rv =  taps_sbucket_pivot_dump(cbdata->unit, cbdata->taps,
                                      sph, TAPS_DUMP_SRAM_SW_PIVOT);

        rv = taps_dbucket_dump(cbdata->unit, cbdata->taps,
                               sph->dbh,
                               cbdata->flags);

    }

    return rv;
}

int _taps_sbucket_dump(taps_tcam_pivot_handle_t tph, void *user_data)
{
    int rv=SOC_E_NONE;
    taps_dump_datum_t *cbdata = (taps_dump_datum_t*)user_data;

    if (!tph || !user_data) return SOC_E_PARAM;

    rv = taps_sbucket_dump(cbdata->unit, cbdata->taps,
                      tph->sbucket,
                      cbdata->flags);

    if (TAPS_DUMP_DRAM_FLAGS(cbdata->flags)) {
        rv = taps_sbucket_traverse(cbdata->unit, cbdata->taps, NULL,
                                   tph->sbucket, cbdata,
                                   _taps_dbucket_dump);
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_dump
 * Purpose:
 *     taps dump utility
 */
int taps_dump(int unit, taps_handle_t taps, uint32 flags)
{
    int rv = SOC_E_NONE;
    dq_p_t elem;
    taps_dump_datum_t cbdata;

    if (!(TAPS_DUMP_FLAG_VALID(flags))) return SOC_E_PARAM;

    if (!taps) {
        /* dump a list of all taps objects and their parameters */
        if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)) {
            DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
                taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
                
                /* dumps the taps handle and the associated taps parameters */
                LOG_CLI((BSL_META_U(unit,
                                    "%s:====================================================\n"),
                         FUNCTION_NAME()));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS handle %p info\n"),
                         FUNCTION_NAME(), unit, taps));
                LOG_CLI((BSL_META_U(unit,
                                    "%s:====================================================\n"),
                         FUNCTION_NAME()));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: instance %d segment %d search mode %d divide_ratio %d \n"),
                         FUNCTION_NAME(), unit, taps->param.instance, taps->segment, taps->param.mode, taps->param.divide_ratio));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: host_share_table %s master_unit %d num_slaves %d, slave 0 unit %d, slave 1 unit %d \n"),
                         FUNCTION_NAME(), unit, taps->param.host_share_table?"TRUE":"FALSE", taps->master_unit, taps->num_slaves, taps->slave_units[0], taps->slave_units[1]));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: max_capacity_limit %d tcam_layout %d default payload 0x%08x 0x%08x 0x%08x 0x%08x\n"),
                         FUNCTION_NAME(), unit, taps->param.max_capacity_limit, taps->param.tcam_layout, taps->param.defpayload[0],
                         taps->param.defpayload[1], taps->param.defpayload[2], taps->param.defpayload[3]));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: key type %d lookup length %d length %d vrf_length %d\n"),
                         FUNCTION_NAME(), unit, taps->param.key_attr.type, taps->param.key_attr.lookup_length, taps->param.key_attr.length,
                         taps->param.key_attr.vrf_length));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: segment Taps 0 offset %d num_entry %d\n"),
                         FUNCTION_NAME(), unit, taps->param.seg_attr.seginfo[0].offset, taps->param.seg_attr.seginfo[0].num_entry));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: segment Taps 0 offset %d num_entry %d\n"),
                         FUNCTION_NAME(), unit, taps->param.seg_attr.seginfo[1].offset, taps->param.seg_attr.seginfo[1].num_entry));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: segment Taps 0/1 offset %d num_entry %d\n"),
                         FUNCTION_NAME(), unit, taps->param.seg_attr.seginfo[2].offset, taps->param.seg_attr.seginfo[2].num_entry));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: sbucket format %d max_pivot_number %d\n"),
                         FUNCTION_NAME(), unit, taps->param.sbucket_attr.format, taps->param.sbucket_attr.max_pivot_number));
                LOG_CLI((BSL_META_U(unit,
                                    "%s: unit %d TAPS init parameters: dbucket num_dbucket_pfx %d payload table_id %d prefix table_id %d\n"),
                         FUNCTION_NAME(), unit, taps->param.dbucket_attr.num_dbucket_pfx, taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE],
                         taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE]));
            } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);
        }        
    } else {
        /* dump the specfied taps */
        SOC_IF_ERROR_RETURN(_taps_validate_handle(unit, taps));
        sal_memset(&cbdata, 0, sizeof(taps_dump_datum_t));
        cbdata.taps  = taps;
        cbdata.flags = flags;
        cbdata.unit  = unit;
        
#ifdef TAPS_ERROR_COUNT        
        LOG_CLI((BSL_META_U(unit,
                     "%s: unit %d TAPS handle %p insert route error count %u\n"),
                     FUNCTION_NAME(), unit, taps, taps->error_count));
#endif        

#ifdef TAPS_V6_COLLISION_COUNT        
        LOG_CLI((BSL_META_U(unit,
                     "%s: unit %d v6 collision error count %u\n"),
                     FUNCTION_NAME(), unit, taps->v6_collision_count));
#endif

        if (TAPS_DUMP_TCAM_FLAGS(flags)) {
            taps_tcam_dump(unit, taps->tcam_hdl, flags);
        } else if (TAPS_DUMP_SRAM_FLAGS(flags) || TAPS_DUMP_DRAM_FLAGS(flags)) {
            /* If set tcam flags, the sbucket and dbucket have been dumped */
            taps_tcam_traverse(unit, taps, taps->tcam_hdl,
                               &cbdata, _taps_sbucket_dump);
        }
        
    }
    return rv;
}

/* callback to redistribute a dbucket */
static int _taps_redistribute_dbucket_after_split_cb(taps_spivot_handle_t sph, void *user_data)
{
    int rv = SOC_E_NONE;
    _taps_redistribute_info_t *info;
    taps_cookie_t *old_cookie;
    
    info = (_taps_redistribute_info_t*)user_data;
    old_cookie = &(info->old_cookie);

#ifdef REDISTRIBUTE_DBG
    LOG_CLI((BSL_META("%s: Redistributing unit %d Taps handle 0x%x DBH 0x%x\n"),
             FUNCTION_NAME(), old_cookie->unit, (uint32)old_cookie->arg.taps, (uint32)sph->dbh));
    taps_dbucket_dump(old_cookie->unit, old_cookie->arg.taps, sph->dbh, TAPS_DUMP_DRAM_SW_BKT);
#endif

    /* go through every prefixes in the dbucket trie, we need
     * to use destroy_traverse since we might delete prefix from trie during
     * traverse.
     */
    rv = taps_dbucket_traverse(old_cookie->unit, old_cookie->arg.taps, old_cookie->obj.wgroup,
                   sph->dbh, user_data, _taps_redistribute_prefix_after_split_cb);

    return rv;    
}

/* callback to redistribute a prefix in dbucket */
static int _taps_redistribute_prefix_after_split_cb(taps_dprefix_handle_t dph, void* user_data)
{
    int rv = SOC_E_NONE;
    _taps_redistribute_info_t *info;
    taps_cookie_t *cookie;
    taps_arg_t *arg;

    /* construct pointers and validate */
    info = (_taps_redistribute_info_t*)user_data;
    cookie = &(info->cookie);
    arg = &(cookie->arg);

    /*
     * Do nothing if the prefix in dbucket is shorter than the new tcam pivot.
     */
    if (dph->length < info->pivot_len) {
    return SOC_E_NONE;
    }

    /*
     * Do nothing if the prefix doesn't match the new tcam pivot.
     */
    if (!taps_key_match(arg->taps->param.key_attr.lookup_length, dph->pfx, dph->length,
            info->pivot, info->pivot_len)) {
    return SOC_E_NONE;
    }

    /* accumulate the dph pointers in the info */
    info->pdph[info->pfx_cnt] = dph;
    info->pfx_cnt++;

    return rv;
}

static int _taps_redistribute_prefix_after_split(taps_dprefix_handle_t dph, _taps_redistribute_info_t* info, 
                                                taps_cookie_t *out_cookie)
{
    int rv = SOC_E_NONE;
    int unit;
    int slave_idx, slave_unit;
    uint32 global_pointer = 0, local_pointer = 0;
    unsigned int pivot_len=0;
    uint8 isbpm=0, ispfxpivot=0;
    taps_bucket_stat_e_t bkt_stat;
    unsigned int bpm[_TAPS_MAX_KEY_WORDS_], pivot[_TAPS_MAX_KEY_WORDS_];
    taps_cookie_t *cookie;
    taps_cookie_t *old_cookie;
    taps_obj_t *obj;
    taps_arg_t *arg;
    taps_tcam_pivot_t tmp_tp;

    /* construct pointers and validate */
    cookie = &(info->cookie);
    old_cookie = &(info->old_cookie);
    obj = &(cookie->obj);
    arg = &(cookie->arg);
    unit = cookie->unit;

#ifdef REDISTRIBUTE_DBG
    LOG_CLI((BSL_META_U(unit,
                        "%s: Redistributing unit %d Taps handle 0x%x DBH 0x%x DPH 0x%x \n"),
             FUNCTION_NAME(), unit, (uint32)arg->taps, (uint32)old_cookie->obj.sph->dbh, (uint32)dph));
    taps_dbucket_prefix_dump(unit, arg->taps, old_cookie->obj.sph->dbh, dph, TAPS_DUMP_DRAM_SW_PFX);
#endif

    /*
     * move these prefixes from the original sbucket to the new sbucket. We need to do following
     * sequence to prevent lookup failure of the moved prefixes when we are doing
     * the move.
     *
     * (1) insert the prefix into the new sbucket
     *       * might need to update local pointers of the existing pivots in the new sbucket
     *       * might do dbucket split due to insertion
     *       * add ASSERT to check that we never do sbucket split due to insertion
     * (2) insert the tph for the new sbucket
     *       * because we insert the tph into software trie before we allocate
     *         the tcam entry, all the tcam entry moved will use the new location of the moved
     *         prefixes in case the prefixes are BPM of the moved tcam entry
     * (3) delete the prefix from the old sbucket
     *       * the deleted prefix should not be the BPM (local pointer) of the other
     *         dbucket in the old sbucket. This is because the moved prefixes should be
     *         equal or longer than the tph of the new sbucket, and all the dbh remaining
     *         in the old sbucket will be either shorter than the tph of the new sbucket
     *         (thus shorter than the moved prefix) or not matching the moved prefix at all
     *       * we can not start search from tcam trie since it will always hit the 
     *         new sbucket. 
     *       * We need to delete the dbucket if the dbucket becomes empty
     *         due to the deletion.
     *
     * NOTE:       
     *      for now, we implement a new state machine instead of reuse the state machine
     *      in the insert_route/delete_route code since we have too much special handling
     *      when we are moving prefixes.
     */

    /* NOTE: hardware command sequence 
     *      (1) commit all dram/sram commands generated by insertion in any order
     *          since the tcam entry is not installed yet.
     *      (2) commit all tcam commands generated by insertion. these commands are
     *          propagation commands to point other tcam entries bpm global_index to
     *          the prefixes in the new sbucket
     *      (3) repeat (1)/(2) for all prefixes moved
     *      (4) install the tcam insertion command
     *      (5) commit all dram/sram commands generated by deletion in any order
     *          since traffic will never hit those prefixes once the tcam entry
     *          for the new sbucket is installed.
     */


    /*=====================================================================
     * state machine for inserting the prefix to new sbucket
     * NOTE: (1) we don't handle nested sbucket split case for now
     *       (2) no need to handle VRF default since we are assuming VRF 
     *           will be created first (upper layer guarrante it)
     *====================================================================*/

    /* Similar to state INS_ST_INIT
     * initialize the arg with the prefix being moved (dph)
     * initialize obj with the new sbucket (destination), skip tcam lookup
     */
    cookie->state = INS_ST_INIT;
    arg->key = dph->pfx;                  /* we lookup the moved prefix */
    arg->length = dph->length;
    arg->payload = dph->payload;
    arg->cookie = dph->cookie;
    obj->wgroup[unit] = old_cookie->obj.wgroup[unit]; /* use the original work group */
    obj->wgroup[unit]->force_work_type_enable = TRUE;
    obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)
         && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit] = old_cookie->obj.wgroup[slave_unit]; /* use the original work group */
            obj->wgroup[slave_unit]->force_work_type_enable = TRUE;
            obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
        }
    }    
    /* new tph object doesn't exist yet. fake one here and make
     * it point to the new sbucket instead. and obj->tph->sbucket should be 
     * the only thing used below. Everything else is clear to 0 on purpose.
     * (NOTE: _taps_find_prefix_objects and taps_calculate_prefix_global_pointer
     * only use obj->tph->sbucket for now).
     */
    sal_memset(&tmp_tp, 0, sizeof(taps_tcam_pivot_t));
    obj->tph = &tmp_tp;
    obj->tph->sbucket = old_cookie->obj.nsbh;
    obj->sph = NULL;
    obj->dph = NULL;

    /* find where to insert in the new sbucket, we should not have 
     * a duplicate here and we should always be able to find a slot
     * for insertion.
     */
    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_SUCCESS(rv) && (obj->dph != NULL)) { 
    /* this should not happen unless there is some software bug */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution found duplicate in new sbucket !!!\n"), 
               FUNCTION_NAME(), unit));
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution FATAL ERROR CODE 1 !!!\n"), 
               FUNCTION_NAME(), unit));

#ifdef REDISTRIBUTE_DBG
    LOG_CLI((BSL_META_U(unit,
                        "%s: Redistribution ERROR unit %d Taps handle 0x%x old SBH 0x%x new SBH 0x%x\n"),
             FUNCTION_NAME(), unit, (uint32)(arg->taps), (uint32)(old_cookie->obj.tph->sbucket),
             (uint32)(obj->tph->sbucket)));
    LOG_CLI((BSL_META_U(unit,
                        "%s: Redistribution Result unit %d old sram bucket\n"), FUNCTION_NAME(), unit));
    taps_sbucket_dump(unit, arg->taps, old_cookie->obj.tph->sbucket, TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL);
    LOG_CLI((BSL_META_U(unit,
                        "\n\n%s: Redistribution Result unit %d new sram bucket\n"), FUNCTION_NAME(), unit));
    taps_sbucket_dump(unit, arg->taps, obj->tph->sbucket, TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL);
#endif
    assert(0);
    } else if (!(obj->tph && obj->sph && !obj->dph && (rv == SOC_E_NOT_FOUND))) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to find an unused slot %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }

    /* Similiar to state INS_ST_DBKT_PFX
     * Insert & commit dram prefix on to dram bucket
     */
    rv = taps_dbucket_insert_prefix(unit, arg->taps, obj->wgroup,
				    obj->sph->dbh, arg->key, arg->length,
				    arg->payload, arg->cookie, obj->sph->length, &obj->dph);
 
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to insert prefix to dram bucket %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;  
    }
    /* Update redisted_dph for error handling */
    out_cookie->redisted_dph[out_cookie->redisted_dph_count] = obj->dph;
    out_cookie->redisted_dph_count++;
    obj->dph->bpm = FALSE;
        
    /* Similiar to state INS_ST_DBKT_PROPAGATE
     * propagate the inserted prefix in sbucket, update dph->bpm to be TRUE
     * if the prefix is bpm route of some pivot in the sbucket.
     * since the tcam pivot is not inserted yet, we have to calculate
     * the local pointer and pass in to taps_sbucket_propagate_prefix
     */
    rv = taps_calculate_prefix_local_pointer(unit, arg->taps, obj->sph,
                         obj->dph, &local_pointer);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to calculate local point for inserted prefix %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;  
    }    

    rv = taps_sbucket_propagate_prefix(unit, arg->taps, 
                       obj->tph->sbucket, obj->wgroup,
                       arg->key, arg->length, local_pointer,
                       TRUE, &obj->dph->bpm);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to propagate inserted prefix in SRAM %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;  
    }

    /* propagate the inserted prefix in tcam, we have to calculate the new global_index
     * of the moved prefix instead of find it using existing tcam trie. Since the new tcam pivot 
     * is not inserted in the tcam trie yet, we will end up find the old global_index
     * of the moved prefix.
     */
    rv = taps_calculate_prefix_global_pointer(unit, arg->taps, obj->tph, obj->sph,
                          obj->dph, &global_pointer);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to calculate global point for inserted prefix %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;  
    }

    /*
     * since the bpm maskes of the tcam trie should already be updated, the new tcam pivot 
     * is not in the tcam trie yet, and we are only moving the prefix to a different location.
     * we still do it here to prevent unnecessary TMU TCAM hardware command though.
     * NOTE that we need to use the old_cookie->obj.tph as the propagation starting point.
     */    
    obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)
         && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
        }
    }

    rv = taps_tcam_propagate_prefix(unit, arg->taps->tcam_hdl, obj->wgroup,
                    old_cookie->obj.tph, TRUE, global_pointer, arg->key, arg->length,
                    &isbpm);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to propagate inserted prefix in TCAM %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    } else {
        obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
        if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit,  arg->taps->param.host_share_table)
             && !_taps_get_caching(unit)) {
            for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
                slave_unit = arg->taps->slave_units[slave_idx];
                obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
            }
        }
    }

    /* update dph->bpm incase the prefix is bpm of some pivot in TCAM */
    obj->dph->bpm |= isbpm;

    /* check if we need to split the dbucket */
    rv = taps_dbucket_stat_get(unit, arg->taps, obj->sph->dbh, &bkt_stat);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to compute dbucket vacancy %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }
            
    if (_TAPS_BKT_FULL == bkt_stat) {
    /* Similiar to state INS_ST_DBKT_SPLIT
     * dram bucket split when full
     */

#ifdef REDISTRIBUTE_DBG
    LOG_CLI((BSL_META_U(unit,
                        "\n\n%s: Redistribution unit %d new sram bucket\n"), FUNCTION_NAME(), unit));
#endif
    
    /* invalidation if any will be queued by dbucket creation */
    rv = taps_sbucket_pivot_id_alloc(unit, arg->taps,
                     obj->tph->sbucket, &obj->sph_id);
    if (SOC_FAILURE(rv)) {
        obj->sph_id = _TAPS_INV_ID_;
        /* TBD: error handling */       
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to allocate spivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    sal_memset(&bpm[0], 0, _TAPS_MAX_KEY_WORDS_ * sizeof(unsigned int));
    
    rv = taps_dbucket_split(unit, arg->taps, obj->wgroup, obj->sph_id,
                obj->sph->dbh, &pivot[0], &pivot_len,
                &bpm[0], &obj->ndbh);
    if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to split dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    /* 
     * sram bucket update due to dram bucket split
     */
    rv = taps_sbucket_insert_pivot(unit, arg->taps, obj->wgroup, 
                       obj->sph_id, obj->tph->sbucket,
                       &pivot[0], pivot_len,
		       ispfxpivot, NULL, NULL,
                       obj->ndbh, &bpm[0], &obj->nsph);
    if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to Insert sram pivot %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    out_cookie->added_sph[out_cookie->added_sph_count] = obj->nsph;
    out_cookie->added_sph_count++;
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution add sph count %d %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, out_cookie->added_sph_count, rv, soc_errmsg(rv)));
    rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
    if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to compute sbucket vacancy %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    if (_TAPS_BKT_FULL == bkt_stat) {
        /* very rare case. Sbucket split during redistribution
         * not handled for now. Just assert.
         */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution nesting sbucket split !!!\n"), 
                   FUNCTION_NAME(), unit));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution FATAL ERROR CODE 2 !!!\n"), 
                   FUNCTION_NAME(), unit));
        assert(0);
    } 
        
    /* Similiar to state INS_ST_DBKT_PROPAGATE_SPLIT
     * sram/tcam propagation due to dram bucket split
     */
    obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)
         && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE2_WORK;
        }
    }
    
    rv = taps_dbucket_traverse(unit, arg->taps, obj->wgroup,
                   obj->ndbh, cookie, 
                   _dbucket_split_propagate);
    if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to propagate split dram bucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)
         && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE1_WORK;
        }
    }
    }

    /*=====================================================================
     * We need to make sure the tcam entry for new sbucket is inserted in hardware
     * NOTE: during tcam entry insertion, it will move tcam entries around and
     *    might issue look ups to find the global index for tcam entrie's bpm. Given that
     *    we insert the new tph into the tcam trie software database first, those 
     *    prefixes remained in the old sbucket will not be visible during those
     *    lookups so the lookup should find the new sbucket and get the correct
     *    global_index incase the moved prefix is bpm of some tcam pivots.
     *====================================================================*/
    

    /*=====================================================================
     * state machine for deleting from old sbucket 
     * NOTE: (1) the deletion will be "local" to be old sbucket. For tcam
     *         both software state and hardware state should be updated at this point
     *       (2) As described above, it's not possible for the moved prefix
     *         to be the local bpm of the sbucket pivots remained in the old
     *         sbucket. So we don't need to do any propagations.
     *       (3) we can simply free any of dph, and dbh/sph once the dbucket
     *         becomes empty. For now, we are not handling the case of 
     *         the old sbucket become empty due to the redistribution since
     *         it should be a very rare case.
     *====================================================================*/

    /* Similar to state INS_ST_INIT
     * initialize the arg with the prefix being moved (dph)
     * initialize obj with the new sbucket (destination), skip tcam lookup
     */
    cookie->state = INS_ST_INIT;
    sal_memset(obj, 0, sizeof(taps_obj_t));
    obj->tph = old_cookie->obj.tph; /* point to old bucket */
    sal_memcpy(obj->wgroup, old_cookie->obj.wgroup, sizeof(obj->wgroup));
    arg->key = dph->pfx;
    arg->length = dph->length;
    arg->payload = dph->payload;
    arg->cookie = dph->cookie;
    obj->wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table) 
        && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
        }
    }

    /* find the sph in the old sbucket, we should always find it here
     * unless there is a bug
     */
    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_SUCCESS(rv)) { 
    if (!obj->dph) { /* prefix not found */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to find prefix !!!\n"), 
                   FUNCTION_NAME(), unit));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution FATAL ERROR CODE 3 !!!\n"), 
                   FUNCTION_NAME(), unit));
        assert(0);
    }
    }

    /* delete the prefix from the dram bucket */
    rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                    obj->sph->dbh, obj->dph);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */       
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistribution failed to delete dram prefix %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }

    /* There is no need to propagate in old sbucket for this case. Since the prefix would never be
     * bpm of the remaining sbucket pivots. Tcam Pivots should have been propagated during 
     * insertion time to point to the new location of the prefix
     */

    /* check whether the dbucket goes empty */
    rv = taps_dbucket_stat_get(unit, arg->taps, obj->sph->dbh, &bkt_stat);
    if (SOC_FAILURE(rv)) {
    /* TBD: error handling */       
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d redistrubiton failed to compute dbucket vacancy %d:%s !!!\n"), 
               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }

    /* Never free wild bucket */
    if (bkt_stat == _TAPS_BKT_EMPTY) {
    /* dbucket goes empty, might need to destroy the dbucket */
    if (obj->sph != obj->tph->sbucket->wsph) {
        out_cookie->remove_sph[out_cookie->remove_sph_count] = obj->sph;
        out_cookie->remove_sph_count++;
#if 0 
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution remove sph count %d %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, out_cookie->remove_sph_count,rv, soc_errmsg(rv)));
       
        /* destroy the non-wild dbucket object */
        rv = taps_dbucket_destroy(unit, arg->taps, obj->wgroup[unit], obj->sph->dbh);
        if (SOC_FAILURE(rv)) {
        /* TBD: error handling */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution failed to free dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
        }
        obj->sph->dbh = NULL; /* unlink dbh & sram pivot */
        provit_id = obj->sph->index;
            /* destroy sram pivot object */
            rv = taps_sbucket_delete_pivot(unit, arg->taps, obj->wgroup,
                                           obj->tph->sbucket, 
                                           obj->sph);
            if (SOC_FAILURE(rv)) {
        /* TBD: error handling */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to free sram pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
            } 

        /* double check that the sram pivot is not wild (fixed position at index==0) */
            assert(provit_id != 0);
            
            /* free back pivot id */
            rv = taps_sbucket_pivot_id_free(unit, arg->taps, 
                                            obj->tph->sbucket, 
                                            provit_id);
            if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to free sram pivot id:%u %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, provit_id, rv, soc_errmsg(rv)));
        return rv;
            }

        /* check sbucket depth */
            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
        /* TBD: error handling */       
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
            } else if (bkt_stat == _TAPS_BKT_EMPTY) {
        /* this should be a very rare case and we are not handling it for now
         * basically we are spliting a sbucket but due to the redistribution
         * process we moved all prefixes into the new sbucket. Not sure if 
         * this is possible, simply put assert here to capture this in case
         * it ever happens.....
         */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution empty out the old sbucket !!!\n"), 
                   FUNCTION_NAME(), unit));
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution FATAL ERROR CODE 4 !!!\n"), 
                           FUNCTION_NAME(), unit));
        assert(0);
            }
#endif
    } else {
        /* if wild pivot, free it only if there are no more spivots on the  sbucket */
            rv = taps_sbucket_stat_get(unit, obj->tph->sbucket, &bkt_stat);
            if (SOC_FAILURE(rv)) {
        /* TBD: error handling */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution failed to compute sbucket vacancy %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
            } 

            /* if only wild pivot exists then destroy this sbucket & wild dbucket */
            if (_TAPS_BKT_WILD == bkt_stat) {
        /* this should be a very rare case and we are not handling it for now
         * basically we are spliting a sbucket but due to the redistribution
         * process we moved all prefixes into the new sbucket. Not sure if 
         * this is possible, simply put assert here to capture this in case
         * it ever happens.....
         */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d redistribution empty out the old sbucket !!!\n"), 
                   FUNCTION_NAME(), unit));
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribution FATAL ERROR CODE 4 !!!\n"), 
                           FUNCTION_NAME(), unit));
        assert(0);
            }
    }
    }

    /* disable the force_work_type */
    obj->wgroup[unit]->force_work_type_enable = FALSE;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit,  arg->taps->param.host_share_table)
         && !_taps_get_caching(unit)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            obj->wgroup[slave_unit]->force_work_type_enable = FALSE;
        }
    }

    return rv;
}
/* Move back the prefix from split out dbucket */
static int _taps_splitted_prefix_move(taps_dprefix_handle_t dph, taps_dbucket_handle_t dbh,
                                _taps_redistribute_info_t* info)
{
    int rv = SOC_E_NONE;
    int unit;
    taps_cookie_t *cookie;
    taps_cookie_t *old_cookie;
    taps_obj_t *obj;
    taps_arg_t *arg;
    taps_tcam_pivot_t tmp_tp;

    /* construct pointers and validate */
    cookie = &(info->cookie);
    old_cookie = &(info->old_cookie);
    obj = &(cookie->obj);
    arg = &(cookie->arg);
    unit = cookie->unit;

    cookie->state = INS_ST_INIT;
    arg->key = dph->pfx;                  /* we lookup the moved prefix */
    arg->length = dph->length;
    arg->payload = dph->payload;
    arg->cookie = dph->cookie;
    obj->wgroup[unit] = old_cookie->obj.wgroup[unit]; /* use the original work group */

    /* new tph object doesn't exist yet. fake one here and make
     * it point to the new sbucket instead. and obj->tph->sbucket should be 
     * the only thing used below. Everything else is clear to 0 on purpose.
     * (NOTE: _taps_find_prefix_objects and taps_calculate_prefix_global_pointer
     * only use obj->tph->sbucket for now).
     */
    sal_memset(&tmp_tp, 0, sizeof(taps_tcam_pivot_t));
    obj->tph = &tmp_tp;
    obj->tph->sbucket = old_cookie->obj.nsbh;
    obj->sph = NULL;
    obj->dph = NULL;

    /* find where to insert in the new sbucket, we should not have 
     * a duplicate here and we should always be able to find a slot
     * for insertion.
     */
    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_SUCCESS(rv) && (obj->dph != NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Exist in the old domain %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } else { 
        assert(obj->sph != NULL);
        /* coverity[var_deref_op : FALSE] */
        rv = taps_dbucket_insert_prefix(unit, arg->taps, obj->wgroup,
				    obj->sph->dbh, arg->key, arg->length,
				    arg->payload, arg->cookie, obj->sph->length, &obj->dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to re-insert dph to original dbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                                   dbh, dph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to delete the dph from old dbh %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

/* Move back the prefix which should be redistributed */
static int _taps_redistributed_prefix_move(taps_dprefix_handle_t dph, _taps_redistribute_info_t* info)
{
    int rv = SOC_E_NONE;
    int unit;
    taps_cookie_t *cookie;
    taps_cookie_t *old_cookie;
    taps_obj_t *obj;
    taps_arg_t *arg;
    taps_tcam_pivot_t tmp_tp;

    /* construct pointers and validate */
    cookie = &(info->cookie);
    old_cookie = &(info->old_cookie);
    obj = &(cookie->obj);
    arg = &(cookie->arg);
    unit = cookie->unit;

#ifdef REDISTRIBUTE_DBG
    LOG_CLI((BSL_META_U(unit,
                        "%s: Redistributing unit %d Taps handle 0x%x DBH 0x%x DPH 0x%x \n"),
             FUNCTION_NAME(), unit, (uint32)arg->taps, (uint32)old_cookie->obj.sph->dbh, (uint32)dph));
    taps_dbucket_prefix_dump(unit, arg->taps, old_cookie->obj.sph->dbh, dph, TAPS_DUMP_DRAM_SW_PFX);
#endif

    cookie->state = INS_ST_INIT;
    arg->key = dph->pfx;                  /* we lookup the moved prefix */
    arg->length = dph->length;
    arg->payload = dph->payload;
    arg->cookie = dph->cookie;
    obj->wgroup[unit] = old_cookie->obj.wgroup[unit]; /* use the original work group */

    /* new tph object doesn't exist yet. fake one here and make
     * it point to the new sbucket instead. and obj->tph->sbucket should be 
     * the only thing used below. Everything else is clear to 0 on purpose.
     * (NOTE: _taps_find_prefix_objects and taps_calculate_prefix_global_pointer
     * only use obj->tph->sbucket for now).
     */
    sal_memset(&tmp_tp, 0, sizeof(taps_tcam_pivot_t));
    obj->tph = &tmp_tp;
    obj->tph->sbucket = old_cookie->obj.nsbh;
    obj->sph = NULL;
    obj->dph = NULL;
    
    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_FAILURE(rv)|| (obj->dph == NULL)) {
        /* This dph is still in the old sbucket, so just return ok */
        return SOC_E_NONE;
    } else { 
        /* Exist in the nsbh, so delete it */
        rv = taps_dbucket_delete_prefix(unit, arg->taps, obj->wgroup,
                                        obj->sph->dbh, obj->dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to clean up when taps 0x%x is full %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, (uint32)arg->taps, rv, soc_errmsg(rv)));
        }
    }

    obj->tph = NULL;
    obj->sph = NULL;
    obj->dph = NULL;

    rv = _taps_find_prefix_objects(unit, arg, obj);
    if (SOC_SUCCESS(rv) && (obj->dph != NULL)) {
        /* Haven't deleted in the old sbucket, so don't need to re-insert it*/
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s: unit %d Exist in the old domain %d:%s !!!\n"), 
                     FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return SOC_E_NONE;
    } else {
        rv = taps_dbucket_insert_prefix(unit, arg->taps, obj->wgroup,
				    obj->sph->dbh, arg->key, arg->length,
				    arg->payload, arg->cookie, obj->sph->length, &obj->dph);
    }

    return rv;
}
/*
* Fuction : 
*        taps_onchip_prefix_search_and_dump
* Purpose:
*        Given a prefix, search in TAPS and dump the associated 
*        data both in software and hardware.
*/
static int 
_taps_onchip_prefix_search_and_dump(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_obj_t obj;
    uint32 bpm_length;
    uint32 bpm_global_index = _BRR_INVALID_CPE_;
    int tph_found = TRUE;
    int sph_found = TRUE;
    int hw_instance, hw_tcam_entry, hw_bucket_id, hw_sph_idx;
    
    sal_memset(&obj, 0, sizeof(taps_obj_t));

    rv = _taps_find_prefix_objects(unit, arg, &obj);
    
    if (SOC_FAILURE(rv) || (obj.sph == NULL)) {
        sph_found = FALSE;
        
        /* try to find bpm within sbucket */
        rv = _taps_sbucket_find_bpm(unit, arg->taps,
                                   obj.tph->sbucket,
                                   arg->key, arg->length,
                                   &bpm_length, &bpm_global_index);
        
        if (bpm_length == 0) {
            /* try to find bpm on tcam if not found within sbucket domain 
                        * Find global pointer to get the payload data */
            rv = _taps_tcam_find_bpm(unit, arg->taps->tcam_hdl, 
                        arg->key, arg->length, 
                        &bpm_length, &bpm_global_index);
            if (SOC_FAILURE(rv)) {
                /* we should always be able to find a bpm here 
                               * unless something internal is wrong */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed "
                                      "to find BPM info for key\n"),
                           FUNCTION_NAME(), unit));
                return rv;      
            }
        }
    }
     
    if (SOC_SUCCESS(rv)) {
        assert(tph_found == TRUE);
        /* Get hardware instance and entry id */
        rv = taps_instance_and_entry_id_get(unit, arg->taps, 
                                       obj.tph->entry, obj.tph->sbucket->domain, 
                                       &hw_instance, &hw_tcam_entry, &hw_bucket_id);
        if (SOC_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d Failed to get hardware instance and entry id\n"),
                      FUNCTION_NAME(), unit));
           return rv;
        }

        /* Dump TCAM database */
        LOG_CLI((BSL_META_U(unit,
                            "Software Database For TCAM:\n")));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Length:      %d \n"), obj.tph->length));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Entry:       %d \n"), obj.tph->entry));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM's Sbucket:   %d \n"), obj.tph->sbucket->domain));
        LOG_CLI((BSL_META_U(unit,
                            "Hardware Database For TCAM:\n")));
        taps_tcam_entry_dump(unit, 
                     hw_instance, 
                     arg->taps->tcam_hdl->segment,
                     hw_tcam_entry,
                     TAPS_DUMP_TCAM_ALL);

        /* Dump SRAM database */
        if (sph_found == TRUE) {
            LOG_CLI((BSL_META_U(unit,
                                "Software Database For SRAM:\n")));
            LOG_CLI((BSL_META_U(unit,
                                "SRAM Length:      %d \n"), obj.sph->length));
            LOG_CLI((BSL_META_U(unit,
                                "SRAM Entry:       %d\n"), obj.sph->index));
            LOG_CLI((BSL_META_U(unit,
                                "Isperfix?         %d\n"), obj.sph->is_prefix));
            hw_sph_idx = obj.sph->index;
        } else {
            hw_bucket_id = bpm_global_index / 
                        ((1 << (arg->taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_);
            hw_sph_idx = bpm_global_index % 
                        ((1 << (arg->taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_);
            
        }
        LOG_CLI((BSL_META_U(unit,
                            "Hardware Database For SRAM:\n")));
        taps_sbucket_entry_dump(unit,
                         hw_instance, 
                         arg->taps->tcam_hdl->segment,
                         hw_bucket_id, 
                         hw_sph_idx, 
                         arg->taps->param.sbucket_attr.format,
                         TAPS_DUMP_SRAM_ALL);

        if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
            LOG_CLI((BSL_META_U(unit,
                                "Hardware Database For DRAM:\n")));
            taps_dbucket_entry_dump(unit, 
                             arg->taps,
                             0, 
                             0, 
                             bpm_global_index, 
                             TAPS_DUMP_DRAM_ALL,
                             TRUE);
        }
    }
    return rv;
}

/*
* Fuction : 
*        taps_offchip_prefix_search_and_dump
* Purpose:
*        Given a prefix, search in TAPS and dump the associated 
*        data both in software and hardware.
*/
static int 
_taps_offchip_prefix_search_and_dump(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    taps_obj_t obj;
    uint32 bpm_length;
    uint32 bpm_local_index = _BRR_INVALID_CPE_;
    uint32 bpm_global_index = _BRR_INVALID_CPE_;
    int tph_found = TRUE;
    int sph_found = TRUE;
    int dph_found = TRUE;
    int hw_instance, hw_tcam_entry, hw_bucket_id;
    
    sal_memset(&obj, 0, sizeof(taps_obj_t));

    rv = _taps_find_prefix_objects(unit, arg, &obj);

    if (SOC_FAILURE(rv)) {
        if (obj.dph == NULL) {
            dph_found = FALSE;
        }
        if (obj.sph == NULL) {
            sph_found = FALSE;
        } else {
            /* Find local pointer to get the payload data*/
            rv = _taps_sbucket_find_bpm(unit, arg->taps, obj.tph->sbucket, 
                                        arg->key, arg->length,
                                        &bpm_length, &bpm_local_index);
        }
        if (obj.tph == NULL) {
        /* This should not happen */
            tph_found = FALSE;
        } else {
            if (bpm_local_index == _BRR_INVALID_CPE_) {
                /* Find global pointer to get the payload data */
                rv = _taps_tcam_find_bpm(unit, arg->taps->tcam_hdl, 
                            arg->key, arg->length,
                            &bpm_length, &bpm_global_index);
                if (SOC_FAILURE(rv)) {
                    /* we should always be able to find a bpm here unless something internal is wrong */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed "
                                          "to find BPM info for key\n"),
                               FUNCTION_NAME(), unit));
                    return rv;  
                }
           }
        }
    }
     
    if (SOC_SUCCESS(rv)) {
        assert(tph_found == TRUE);
        /* Get hardware instance and entry id */
        rv = taps_instance_and_entry_id_get(unit, arg->taps, 
                                       obj.tph->entry, obj.tph->sbucket->domain, &hw_instance, &hw_tcam_entry, &hw_bucket_id);
        if (SOC_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d Failed to get hardware instance and entry id\n"),
                      FUNCTION_NAME(), unit));
           return rv;
        }

        /* Dump TCAM database */
        LOG_CLI((BSL_META_U(unit,
                            "Software Database For TCAM:\n")));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Length:     %d \n"), obj.tph->length));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Entry:      %d\n"), obj.tph->entry));
        LOG_CLI((BSL_META_U(unit,
                            "TCAM's Sbucket:  %d\n"), obj.tph->sbucket->domain));
        LOG_CLI((BSL_META_U(unit,
                            "Hardware Database For TCAM:\n")));
        taps_tcam_entry_dump(unit, 
                     hw_instance, 
                     arg->taps->tcam_hdl->segment,
                     hw_tcam_entry,
                     TAPS_DUMP_TCAM_ALL);

        /* Dump SRAM database */
        if (sph_found == TRUE) {
            LOG_CLI((BSL_META_U(unit,
                                "Software Database For SRAM:\n")));
            LOG_CLI((BSL_META_U(unit,
                                "SRAM Length:      %d \n"), obj.sph->length));
            LOG_CLI((BSL_META_U(unit,
                                "SRAM Entry:       %d\n"), obj.sph->index));
            LOG_CLI((BSL_META_U(unit,
                                "SRAM's Dbucket:   %d\n"), obj.sph->dbh->bucket));
            LOG_CLI((BSL_META_U(unit,
                                "Isperfix?         %d\n"), obj.sph->is_prefix));
            LOG_CLI((BSL_META_U(unit,
                                "Hardware Database For SRAM:\n")));
            taps_sbucket_entry_dump(unit,
                             hw_instance, 
                             arg->taps->tcam_hdl->segment,
                             hw_bucket_id, 
                             obj.sph->index, 
                             arg->taps->param.sbucket_attr.format,
                             TAPS_DUMP_SRAM_ALL);
        } 

        /* Dump DRAM database */
        if (dph_found == TRUE) {
            assert(sph_found == TRUE);
            LOG_CLI((BSL_META_U(unit,
                                "Software Database For DRAM:\n")));
            LOG_CLI((BSL_META_U(unit,
                                "DRAM Length:      %d \n"), obj.dph->length));
            LOG_CLI((BSL_META_U(unit,
                                "DRAM Entry:  %d\n"), obj.dph->index));
            LOG_CLI((BSL_META_U(unit,
                                "Payload:         0x%x,0x%x,0x%x,0x%x\n"), 
                     obj.dph->payload[0],
                     obj.dph->payload[1],
                     obj.dph->payload[2],
                     obj.dph->payload[3]));
            LOG_CLI((BSL_META_U(unit,
                                "Cookie:          0x%p"), obj.dph->cookie));
            LOG_CLI((BSL_META_U(unit,
                                "Isbpm?           %d\n"), obj.dph->bpm));
            LOG_CLI((BSL_META_U(unit,
                                "Hardware Database For DRAM:\n")));
            taps_dbucket_entry_dump(unit, 
                             arg->taps,
                             obj.sph->dbh->domain, 
                             obj.sph->dbh->bucket, 
                             obj.dph->index, 
                             TAPS_DUMP_DRAM_ALL,
                             FALSE);
        } else if (bpm_local_index != _BRR_INVALID_CPE_) {
            assert(sph_found == TRUE);
            bpm_global_index = bpm_local_index + 
                     obj.tph->sbucket->domain * 
                     arg->taps->param.sbucket_attr.max_pivot_number *
                     arg->taps->param.dbucket_attr.num_dbucket_pfx * 2;

            LOG_CLI((BSL_META_U(unit,
                                "Hardware Database For DRAM:\n")));
            taps_dbucket_entry_dump(unit, 
                             arg->taps,
                             0, 
                             0, 
                             bpm_global_index, 
                             TAPS_DUMP_DRAM_ALL,
                             TRUE);
        } else if (bpm_global_index != _BRR_INVALID_CPE_) {
            LOG_CLI((BSL_META_U(unit,
                                "Hardware Database For DRAM:\n")));
            taps_dbucket_entry_dump(unit, 
                             arg->taps,
                             0, 
                             0, 
                             bpm_global_index, 
                             TAPS_DUMP_DRAM_ALL,
                             TRUE);
        } else {
            /* This should not happen for IPv6 */
        }
    }
    return rv;
}

/*
* Fuction : 
*        taps_prefix_search_and_dump
* Purpose:
*        Given a prefix, search in TAPS and dump the associated 
*        data both in software and hardware.
*/
int 
taps_prefix_search_and_dump(int unit, taps_arg_t *arg)
{
    int rv;
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = _taps_offchip_prefix_search_and_dump(unit, arg);
    } else {
        rv = _taps_onchip_prefix_search_and_dump(unit, arg);
    }
    return rv;
}

static int _taps_onchip_get_lpm_route(int unit, taps_arg_t *arg, 
                                    uint32 *out_bpm_key, uint32 *out_bpm_length)
{
    int rv = SOC_E_NONE;
    taps_arg_t bpm_arg;
    taps_obj_t obj;   
    uint32 bpm_key[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 payload[_TAPS_PAYLOAD_WORDS_];
    uint32 bpm_length = 0;
    
    sal_memset(&obj, 0, sizeof(taps_obj_t));
    sal_memset(&bpm_arg, 0, sizeof(taps_arg_t));
    bpm_arg.key = &bpm_key[0];
    bpm_arg.length = arg->length;
    bpm_arg.payload = &payload[0];
    bpm_arg.taps = arg->taps;
    sal_memcpy(bpm_arg.key , arg->key, 
            sizeof(uint32) * BITS2WORDS(arg->taps->param.key_attr.lookup_length));

    /* Try to get the key with exact match */
    rv = _taps_find_prefix_objects(unit, arg, &obj);
    if (SOC_FAILURE(rv)) {
        /* Can't find the route with exact match
               * Then get the  bpm_length using the bpm mask of sbucket trie*/
        rv = taps_sbucket_find_bpm(unit, arg->taps,
                                   obj.tph->sbucket,
                                   arg->key, arg->length,
                                   &bpm_length);
        if (bpm_length == 0) {
           /* BPM  is not in this bucket, need to get the bpm_length using bpm mask of tcam trie */
           rv = taps_tcam_find_bpm(unit, arg->taps->tcam_hdl,
                                arg->key, arg->length,
                                &bpm_length);
        }
        
        if (SOC_SUCCESS(rv)) {
            /* Construct bpm key */
            bpm_arg.length = bpm_length;
            rv = taps_key_shift(arg->taps->param.key_attr.lookup_length, bpm_arg.key, 
                arg->length, arg->length - bpm_length);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to construct BPM key\n"),
                           FUNCTION_NAME(), unit));
            } else {
                /* Get bpm key with exact match */
                sal_memset(&obj, 0, sizeof(taps_obj_t));
                rv = _taps_find_prefix_objects(unit, &bpm_arg, &obj);
                if (SOC_FAILURE(rv)) {
                    /* This should not happen */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to find bpm route %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
        }
    }

    if (SOC_SUCCESS(rv)) {
        if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
            sal_memcpy(arg->payload, obj.sph->payload,
               _TAPS_PAYLOAD_WORDS_ * sizeof(uint32));
        } else {
            arg->payload[0] = obj.sph->payload[0] 
                            & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        }
	arg->cookie = obj.sph->cookie;
        *out_bpm_length = bpm_arg.length;
        sal_memcpy(out_bpm_key, bpm_arg.key, 
            sizeof(uint32) * BITS2WORDS(arg->taps->param.key_attr.lookup_length));
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to get lpm route %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

static int _taps_offchip_get_lpm_route(int unit, taps_arg_t *arg, 
                                    uint32 *out_bpm_key, uint32 *out_bpm_length)
{
    
    int rv = SOC_E_NONE;
    taps_arg_t bpm_arg;
    taps_obj_t obj;   
    uint32 bpm_key[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 payload[_TAPS_PAYLOAD_WORDS_];
    uint32 bpm_length = 0;
    
    sal_memset(&obj, 0, sizeof(taps_obj_t));
    sal_memset(&bpm_arg, 0, sizeof(taps_arg_t));
    bpm_arg.key = &bpm_key[0];
    bpm_arg.length = arg->length;
    bpm_arg.payload = &payload[0];
    bpm_arg.taps = arg->taps;
    sal_memcpy(bpm_arg.key , arg->key, 
            sizeof(uint32) * BITS2WORDS(arg->taps->param.key_attr.lookup_length));
    

    /* Try to get the key with exact match */
    rv = _taps_find_prefix_objects(unit, arg, &obj);
    if (SOC_FAILURE(rv)) {
        /* Can't find the route with exact match */
        if (obj.sph != NULL) {
            /* get the  bpm_length using the bpm mask of dbucket trie */
            rv = taps_dbucket_find_bpm(unit, arg->taps,
                                   obj.sph->dbh,
                                   arg->key, arg->length,
                                   obj.sph->length, &bpm_length);
            if (SOC_FAILURE(rv) || bpm_length == 0) {
               /* BPM  is not in this dbucket, need to get the bpm key from 
                            bpm mask of sbucket trie */
               rv = taps_sbucket_find_bpm(unit, arg->taps,
                                   obj.tph->sbucket,
                                   arg->key, arg->length,
                                   &bpm_length);
            }
        }
        if (bpm_length == 0) {
            if (obj.tph != NULL) {
                /* BPM  is not in this dbucket, need to get the bpm key from 
                            bpm mask of tcam trie */
               rv = taps_tcam_find_bpm(unit, arg->taps->tcam_hdl,
                                arg->key, arg->length,
                                &bpm_length);
            }
        }
        if (SOC_SUCCESS(rv)) {
            /* Construct bpm key */
            bpm_arg.length = bpm_length;
            rv = taps_key_shift(arg->taps->param.key_attr.lookup_length, bpm_key, 
                arg->length,  arg->length - bpm_length);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to construct BPM key\n"),
                           FUNCTION_NAME(), unit));
            } else {
                /* Get bpm key with exact match */
                sal_memset(&obj, 0, sizeof(taps_obj_t));
                rv = _taps_find_prefix_objects(unit, &bpm_arg, &obj);
                if (SOC_FAILURE(rv)) {
                    /* This should not happen */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to find bpm route %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
        }
    }
    
    if (SOC_SUCCESS(rv)) {
        sal_memcpy(arg->payload, obj.dph->payload,
                       _TAPS_PAYLOAD_WORDS_ * sizeof(uint32));
	arg->cookie = obj.dph->cookie;
        *out_bpm_length = bpm_arg.length;
        sal_memcpy(out_bpm_key, bpm_arg.key, 
            sizeof(uint32) * BITS2WORDS(arg->taps->param.key_attr.lookup_length));
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to get lpm route %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}


/*
* Fuction : 
*        taps_get_lpm_route
* Purpose:
*        Get the lpm of the input route, return the lpm route's key & length & payload
*/
int taps_get_lpm_route(int unit, taps_arg_t *arg, uint32 *out_bpm_key, uint32 *out_bpm_length)
{
    int rv;
    if (!arg || !out_bpm_key || !out_bpm_length) {
        return SOC_E_PARAM;
    }
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = _taps_offchip_get_lpm_route(unit, arg, out_bpm_key, out_bpm_length);
    } else {
        rv = _taps_onchip_get_lpm_route(unit, arg, out_bpm_key, out_bpm_length);
    }
    if(SOC_SUCCESS(rv)) {
        if (arg->taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, out_bpm_key,
              *out_bpm_length, *out_bpm_length - TAPS_IPV4_KEY_SIZE);
        } else {
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, out_bpm_key,
              *out_bpm_length, *out_bpm_length - TAPS_IPV6_KEY_SIZE);
        }
    }
    return rv;
}

/*
 *   Function
 *      _taps_bucket_next_node_get_cb
 *   Purpose
 *      A call back function to get the next node with specified index
 *   Parameters
 *      (IN) payload             : Current node
 *      (OUT) trav_datum      : User data
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
static int _taps_bucket_next_node_get_cb(trie_node_t *payload, void *trav_datum)
{
    int rv = SOC_E_NONE;
    int current_node_index = 0;
    trie_traverse_data_s *user_data = NULL;
    taps_tcam_pivot_handle_t tph = NULL;
    taps_spivot_handle_t sph = NULL;
    taps_dprefix_handle_t dph = NULL;

    if (!payload || !trav_datum) {
        return SOC_E_PARAM;
    }
    if (payload->type != PAYLOAD) {
        return SOC_E_NOT_FOUND;
    }
    
    user_data = (trie_traverse_data_s *)trav_datum;
    if (user_data->trie_type == TCAM_PIVOT_TRIE) {
        tph = TRIE_ELEMENT_GET(taps_tcam_pivot_handle_t, payload, node);
        current_node_index = tph->sbucket->domain;
    } else if (user_data->trie_type == SRAM_PIVOT_TRIE) {
        sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
        current_node_index = sph->index;
    } else if (user_data->trie_type == DRAM_PREFIX_TRIE){
        dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
        current_node_index = dph->index % user_data->max_node_num;
    } else {
        return SOC_E_PARAM; 
    }
    if (current_node_index == user_data->expected_node_idx) {
        user_data->next_node = payload;
    } else {
        rv = SOC_E_NOT_FOUND;
    }
    
    return rv;
}

/*
 *   Function
 *      taps_bucket_next_node_get
 *   Purpose
 *      Get the next node in a trie 
 *   Parameters
 *      (IN) trie                    : Specified trie
 *      (IN) trie_type             : TCAM/SRAM/DRAM trie
 *      (IN) bit_map              : The bit map for used node
 *      (IN) start_index          : The beginning index to search
 *      (IN) max_node_num    : The maxmum index to search
 *      (OUT) next_node         : Next node in this trie
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_bucket_next_node_get(trie_t *trie,
                            trie_type_t trie_type,
                            uint32 *bit_map,
                            int start_index,
                            int max_node_num,
                            trie_node_t **next_node) 
{
    int rv = SOC_E_NONE;
    int found = FALSE;
    int index = 0;
    trie_traverse_data_s user_data;

    for (index = start_index; index < max_node_num; index++) {
        if (SHR_BITGET(&(bit_map[index>>5]), index&0x1F)) {
            user_data.expected_node_idx = index;
            user_data.max_node_num = max_node_num;
            user_data.trie_type = trie_type;
            found = TRUE;
            break;
        }
    }
    if (!found) {
        rv = SOC_E_NOT_FOUND;
    } else {
        rv = trie_traverse_find(trie->trie, _taps_bucket_next_node_get_cb,
                       &user_data);
        if (SOC_SUCCESS(rv)) {
            *next_node = user_data.next_node;
        }
    }
    
    return rv;
}

/*
 *   Function
 *      taps_iterator_first
 *   Purpose
 *      Get the first prefix of taps
 *   Parameters
 *      (IN) unit              : unit number of the device
 *      (IN) arg               : taps arg pramater,include a valid taps specified by arg->taps
 *      (OUT) key            : First key in this taps
 *      (OUT) key_length  : Length of first key
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - there is no prefix in this taps
 *       SOC_E_* - Error return 
 */
int taps_iterator_first(int unit, taps_arg_t *arg, uint32 *key, uint32 *key_length)
{
    int rv = SOC_E_NONE;
    taps_sbucket_handle_t sbh;
    taps_spivot_handle_t cur_sph, next_sph;

    if (!arg || !arg->taps || !key || !key_length) {
        return SOC_E_PARAM;
    }

    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, 
                                arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }
    
    rv = taps_sbucket_first_bucket_handle_get(unit, arg->taps, &sbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to get the first sbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = taps_sbucket_first_pivot_handle_get(unit, arg->taps, sbh, &next_sph);
        if (SOC_SUCCESS(rv)) {
            while (SOC_SUCCESS(rv)) {
                cur_sph = next_sph;
                rv = taps_dbucket_first_prefix_get(unit, arg->taps,
                                        cur_sph->dbh, key, key_length);
                if(SOC_SUCCESS(rv)) {
                    /* Found the first prefix in this dbucket */
                    break;
                } else if (rv == SOC_E_NOT_FOUND){
                    /* Not found in this dbucket, get the next dbucket */
                    rv = taps_sbucket_next_pivot_handle_get(arg->taps, 
                                                        cur_sph->dbh->bucket + 1,
                                                        cur_sph->sbh,
                                                        &next_sph);
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Fail to get the next dbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Fail to get the first dbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    } else {
        rv = taps_sbucket_first_pivot_get(unit, arg->taps, sbh, key, key_length);
    }

    if(SOC_SUCCESS(rv)) {
        if (arg->taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, key,
              *key_length, *key_length - TAPS_IPV4_KEY_SIZE);
        } else {
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, key,
              *key_length, *key_length - TAPS_IPV6_KEY_SIZE);
        }
    } else if (rv != SOC_E_NOT_FOUND){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to get the first key %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *   Function
 *      taps_iterator_next
 *   Purpose
 *      Get the next prefix of taps 
 *   Parameters
 *      (IN) unit              : unit number of the device
 *      (IN) arg               : taps arg pramater,include a valid taps specified by arg->taps,
 *                                  and indicate the last prefix using arg->key,arg->length
 *      (OUT) key            : First key in this taps
 *      (OUT) key_length  : Length of first key
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_iterator_next(int unit, taps_arg_t *arg, uint32 *key, uint32 *key_length)
{
    int rv = SOC_E_NONE;
    int start_index;
    taps_obj_t obj;
    taps_spivot_handle_t cur_sph, next_sph;

    if (!arg || !arg->taps || !arg->key || !key || !key_length) {
        return SOC_E_PARAM;
    }
    
    if (_IS_SLAVE_SHARE_LPM_TABLE(unit, arg->taps->master_unit, 
                                arg->taps->param.host_share_table)) {
        unit = arg->taps->master_unit;
    }

    sal_memset(&obj, 0, sizeof(taps_obj_t));

    /* find the sph/dph of input key*/
    rv = _taps_find_prefix_objects(unit, arg, &obj); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Can't find the input key %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return SOC_E_PARAM;
    }

    /* Get and return the next key and length */
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* Offchip mode*/
        start_index = obj.dph->index + 1;
        next_sph = obj.sph;
        while (SOC_SUCCESS(rv)) {
            cur_sph = next_sph;
            rv = taps_dbucket_next_prefix_get(arg->taps, start_index,
                                    cur_sph->dbh, key, key_length);
            if(SOC_SUCCESS(rv)) {
                /* Found the next prefix in this dbucket */
                break;
            } else if (rv == SOC_E_NOT_FOUND){
                /* Not found in this dbucket, get the next dbucket */
                start_index = 0;
                rv = taps_sbucket_next_pivot_handle_get(arg->taps, 
                                                    cur_sph->dbh->bucket + 1,
                                                    cur_sph->sbh,
                                                    &next_sph);
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Fail to get the next dbucket %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }
    } else {
        /* Onchip mode */
        start_index = obj.sph->index + 1;
        rv = taps_sbucket_next_pivot_get(arg->taps, start_index, 
                                        obj.tph->sbucket, key, key_length);
    }

    if(SOC_SUCCESS(rv)) {
        if (arg->taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, key,
              *key_length, *key_length - TAPS_IPV4_KEY_SIZE);
        } else {
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, key,
              *key_length, *key_length - TAPS_IPV6_KEY_SIZE);
        }
    } else if (rv != SOC_E_NOT_FOUND){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to get the next key %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}


/*
 *
 * Function:
 *     taps_onchip_domain_move
 * Purpose:
 *     Move whole sbucket to new_sbh
 */
int taps_onchip_domain_move(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       uint32 domain,
                       taps_sbucket_handle_t sbh)
{
    int rv = SOC_E_NONE; 
    taps_cookie_t cookie;
    
    sal_memset(&cookie, 0, sizeof(cookie));

    /* This step is similiar with the domain propagation when insert pfx*/
    cookie.state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
    cookie.unit = unit;
    cookie.arg.taps = taps;
    sal_memcpy(cookie.obj.wgroup, wgroup, sizeof(cookie.obj.wgroup));
    cookie.obj.domain_id = domain;
    cookie.obj.nsbh = sbh;

    /* Move all of the sbucket's entry to new domain*/
    rv = taps_sbucket_domain_move(unit, taps, wgroup, sbh, domain);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to move sbucket entry %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    /* traverse through split domain, relocate dram buckets, propagate */
    rv = taps_sbucket_traverse(unit, taps, wgroup,
                               sbh, &cookie, _sbucket_split_propagate_for_onchip_mode);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 
    
    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to commit onchip mode commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    } 
    return rv;
}



/*
 *
 * Function:
 *     taps_offchip_domain_move
 * Purpose:
 *     Move whole sbucket to new_sbh
 */
int taps_offchip_domain_move(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       uint32 domain,
                       taps_sbucket_handle_t sbh)
{
    int rv = SOC_E_NONE; 
    taps_cookie_t cookie;
    
    sal_memset(&cookie, 0, sizeof(cookie));

    /* This step is similiar with the domain propagation when insert pfx*/
    cookie.state = INS_ST_DOMAIN_PROPAGATE_SPLIT;
    cookie.unit = unit;
    cookie.arg.taps = taps;
    sal_memcpy(cookie.obj.wgroup, wgroup, sizeof(cookie.obj.wgroup));
    cookie.obj.domain_id = domain;
    cookie.obj.nsbh = sbh;

    /* Move all of the sbucket's entry to new domain*/
    rv = taps_sbucket_domain_move(unit, taps, wgroup, sbh, domain);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to move sbucket entry %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    /* traverse through split domain, relocate dram buckets, propagate */
    rv = taps_sbucket_traverse(unit, taps, wgroup,
                               sbh, &cookie, _sbucket_split_propagate);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to propagate split sram bucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 

    /* dont issue propagate work item if dram relocate fails */
    rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to commit & propagate"
                              " relocated dram bucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    } else {
        rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_SRAM|_TAPS_COMMIT_FLAG_TCAM);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to commit & propagate"
                                  " relocated dram bucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        } else {
            wgroup[unit]->force_work_type_enable = TRUE;
            wgroup[unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
            rv = taps_work_commit(unit, wgroup[unit], &wgroup[unit]->forced_work_type, 1, _TAPS_BULK_COMMIT);
            wgroup[unit]->force_work_type_enable = FALSE;
            wgroup[unit]->forced_work_type = 0;
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d redistribute failed to commit STAGE 3 works %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                return rv;
            }
        }
    }
    return rv;
}

/*
 *   Function
 *      taps_domain_move
 *   Purpose
 *      Move the whole domain, this is used when the two tcam entry are in different instances
 */
int taps_domain_move(int unit, taps_tcam_handle_t handle, taps_wgroup_handle_t *wgroup, 
                        int old_tcam_entry, int candidate_tcam_entry)
{
    int rv = SOC_E_NONE;
    uint32 candidate_domain_id, old_domain_id;
    taps_tcam_pivot_handle_t tph = NULL;
    
    if (_TAPS_IS_PARALLEL_MODE_(handle->taps->param.instance)
     || _ENTRY_IN_SAME_INSTANCE(old_tcam_entry, candidate_tcam_entry, 
          handle->taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry)) {
        /* For unified mode, if free_entry and candidate_entry are in different instance,
            * then move the whole domain from old instance to new instance. 
            */
        return SOC_E_NONE;
    }
    
    if((old_tcam_entry == candidate_tcam_entry) ||
        (SHR_BITGET(handle->in_use, old_tcam_entry) == 0)) {
    	/* candidate_entry and free_entry are same, or candidate_entry itself is free, do nothing */
    	return SOC_E_NONE;
    }
    tph = handle->map[old_tcam_entry];
    old_domain_id = tph->sbucket->domain;

    /* Allocate new sbucket id for free_entry*/                
    rv = taps_bucket_alloc(unit, handle->taps, candidate_tcam_entry, &candidate_domain_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate domain id  %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 

    if (handle->taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = taps_offchip_domain_move(unit, handle->taps, wgroup, 
                                candidate_domain_id, tph->sbucket); 
        if (SOC_FAILURE(rv)) {
            tph->sbucket->domain = old_domain_id;
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to move domain entry %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        }
    } else {
        rv = taps_onchip_domain_move(unit, handle->taps, wgroup, 
                                candidate_domain_id, tph->sbucket); 
        if (SOC_FAILURE(rv)) {
            tph->sbucket->domain = old_domain_id;
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to move domain entry %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        }
    }
    
    /* Destory domain ID */ 
    rv = taps_bucket_free(unit, handle->taps, old_domain_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to free tcam domain id %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}
/*
 *
 * Function:
 *   taps_work_group_create_for_all_devices
 * Purpose:
 *   creates a serial taps work group for all devices which share this taps
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) work_queue   : handle returned by taps_work_queue_init
 *   (IN) work_group   : work group id
 *   (IN) host_share_table   : host share table or not
 *   (OUT) work_group_handle   : Array of work group handle.
 * Returns
 *   SOC_E_NONE - successfully enqueued work payload object
 *   SOC_E_* as appropriate otherwise
 */
int taps_work_group_create_for_all_devices(int unit, 
                           taps_handle_t taps,
                           taps_wq_handle_t *work_queue, 
                           unsigned int wgroup,
                           int host_share_table,
                           taps_wgroup_handle_t *work_group_handle)
{
    int rv = SOC_E_NONE;
    int slave_idx = 0, slave_unit;

    if ((work_queue[unit] == NULL) || (wgroup > _TAPS_MAX_WGROUP_) 
        || work_group_handle == NULL) {
	    return SOC_E_PARAM;
    }

    rv = taps_work_group_create(unit, work_queue[unit], wgroup, &work_group_handle[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to create work group "
                              "of master unit %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, host_share_table)) {
        work_group_handle[unit]->host_share = TRUE;
        if (!_taps_get_caching(unit)) {
            while (slave_idx < taps->num_slaves) {
                slave_unit = taps->slave_units[slave_idx];
                if (work_queue[slave_unit] == NULL) {
            	    return SOC_E_PARAM;
                }
                rv = taps_work_group_create(slave_unit, work_queue[slave_unit], 
                                    wgroup, &work_group_handle[slave_unit]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate work group "
                                          "for all sharing table units %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    break;
                }
                ++slave_idx;
            }
        }
    }

    return rv;
}

/*
 *
 * Function:
 *   taps_work_group_destroy_for_all_devices
 * Purpose:
 *   destroy an array of taps work group
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) wghdl      : An array of work group handle
 *   (IN) host_share_table    : host share table or not
 * Returns
 *   SOC_E_NONE - successfully enqueued work payload object
 *   SOC_E_* as appropriate otherwise
 */
int taps_work_group_destroy_for_all_devices(int unit, taps_handle_t taps,
                           taps_wgroup_handle_t *wghdl, int host_share_table)
{
    int rv = SOC_E_NONE;
    int slave_idx= 0, slave_unit;

    if (!wghdl[unit] || (wghdl[unit] && wghdl[unit]->wgroup > _TAPS_MAX_WGROUP_)) {
	    return SOC_E_PARAM;
    }

    rv = taps_work_group_destroy(unit, wghdl[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to destroy work group "
                              "of master unit %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, host_share_table)
       && !_taps_get_caching(unit)) {
        while(slave_idx < taps->num_slaves) {
            slave_unit = taps->slave_units[slave_idx];
            if (!wghdl[slave_unit] 
                || (wghdl[slave_unit] && wghdl[slave_unit]->wgroup > _TAPS_MAX_WGROUP_)) {
        	    rv = SOC_E_PARAM;
                break;
            }
            
            rv = taps_work_group_destroy(slave_unit, wghdl[slave_unit]);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group "
                                      "for all sharing table units %d:%s !!!\n"), 
                           FUNCTION_NAME(), slave_unit, rv, soc_errmsg(rv)));
                break;
            }
            ++slave_idx;
        }
    }

    return rv;
}

/*
 *   Function
 *      taps_command_enqueue_for_slave_unit
 *   Purpose
 *      Command enqueue for slave unit. 
 */
int taps_command_enqueue_for_slave_unit(int unit, taps_handle_t taps,
                            const taps_wgroup_handle_t *wgroup, 
                            taps_work_type_e_t type,
                            soc_sbx_caladan3_tmu_cmd_t *master_cmd) 
{
    int rv = SOC_E_NONE;
    int slave_unit, slave_idx = 0;
    soc_sbx_caladan3_tmu_cmd_t *slave_cmd=NULL;

	/* Unit must be the master unit */
    if (!taps || !wgroup[unit] || !master_cmd) {
        return SOC_E_PARAM;
    }
    
    while(slave_idx < taps->num_slaves) {
        slave_unit = taps->slave_units[slave_idx];
        rv = tmu_cmd_alloc(slave_unit, &slave_cmd); 
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate TMU dbucket "
                                  "payload update commands %d:%s !!!\n"), 
                       FUNCTION_NAME(), slave_unit, rv, soc_errmsg(rv)));
            break;
        }
		/* Slave cmd is the same with master's*/
        slave_cmd->opcode = master_cmd->opcode;
        sal_memcpy(&slave_cmd->cmd, &master_cmd->cmd, sizeof(slave_cmd->cmd));
        rv = taps_work_enqueue(slave_unit, wgroup[slave_unit], 
                        type, &(slave_cmd->wq_list_elem));
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), slave_unit, rv, soc_errmsg(rv)));
            tmu_cmd_free(slave_unit, slave_cmd);
            break;
        } 
        ++slave_idx;
    }
    
    return rv;
}

/*
 *   Function
 *      taps_command_enqueue_for_all_devices
 *   Purpose
 *      Command enqueue for master and slave unit. 
 */
int taps_command_enqueue_for_all_devices(int unit, taps_handle_t taps,
                            const taps_wgroup_handle_t *wgroup, 
                            taps_work_type_e_t type,
                            soc_sbx_caladan3_tmu_cmd_t *cmd) 
{
    int rv;
  
    rv = taps_work_enqueue(unit, wgroup[unit],
                           type, &cmd->wq_list_elem);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue cmds work item %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }        

    if (!_taps_get_caching(unit) && 
        _IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table)
        && SOC_SUCCESS(rv)) {
        rv = taps_command_enqueue_for_slave_unit(unit, taps, wgroup, type, cmd);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to alloc cmds for slave unit %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }
    return rv;
}

int taps_command_destory(int unit, taps_handle_t taps, const taps_wgroup_handle_t *wgroup, 
                                taps_work_type_e_t *work_type, uint32 work_type_count)
{
    int index = 0, rv = SOC_E_NONE;
    dq_p_t work_item;
    int slave_unit, slave_idx = 0;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;

    if (!wgroup || !work_type || (work_type_count == 0)) return SOC_E_PARAM;
   
    for (index=0; index < work_type_count; index++) {
        do {
            rv = taps_work_dequeue(unit, wgroup[unit], work_type[index], &work_item, _WQ_DEQUEUE_DEFAULT_); 
            if (SOC_SUCCESS(rv)) {
                _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);
                tmu_cmd_free(unit, cmd);
            }
        } while(SOC_SUCCESS(rv));
    }
    if (!_taps_get_caching(unit) && 
        _IS_MASTER_SHARE_LPM_TABLE(unit, taps->master_unit, taps->param.host_share_table)
        && SOC_SUCCESS(rv)) {
        /* coverity[dead_error_line : FALSE] */
        while(slave_idx < taps->num_slaves) {
            slave_unit = taps->slave_units[slave_idx];
            for (index=0; index < work_type_count; index++) {
                do {
                    rv = taps_work_dequeue(slave_unit, wgroup[slave_unit], work_type[index], &work_item, _WQ_DEQUEUE_DEFAULT_); 
                    if (SOC_SUCCESS(rv)) {
                        _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(work_item, cmd);
                        tmu_cmd_free(slave_unit, cmd);
                    }
                } while(SOC_SUCCESS(rv));
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * dbucket traverse callback function
 */
static int _taps_dbucket_collision_find_cb(trie_node_t *payload, void *user_data)
{
    int rv;
    int unit;
    taps_cookie_t cookie;
    taps_handle_t taps;
    taps_dprefix_handle_t dph = NULL;
    uint8 collision = FALSE;
    
    dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
    /* only callback on the payload nodes */
    if (payload && (payload->type == PAYLOAD)) {
        
        taps_collision_dbucket_find_t *cbdata = (taps_collision_dbucket_find_t*)user_data;
        if (dph->index == cbdata->dph_idx) {
            unit = cbdata->unit;
            taps = cbdata->taps;

            sal_memset(&cookie, 0, sizeof(cookie));
            cookie.unit = unit;
            cookie.arg.taps = taps;
            
            rv = taps_work_group_create_for_all_devices(unit, taps, taps->wqueue, 
                                        _TAPS_DEFAULT_WGROUP_,
                                        taps->param.host_share_table,
                                        cookie.obj.wgroup);
            if (SOC_FAILURE(rv)) {
                rv = SOC_E_FAIL;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate work group %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            
            rv = taps_dbucket_collision_rehash(unit, taps, cookie.obj.wgroup, 
                                    cbdata->key, TAPS_IPV6_KEY_SIZE,
                                    cbdata->dbh, dph,
                                    &collision);
            if (collision) {
                rv = _taps_commit(unit, &cookie, _TAPS_COMMIT_FLAG_DRAM);
                if (SOC_FAILURE(rv)) {
        		    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to commit dram cmds during rehash %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        		} 
            }

            rv = taps_work_group_destroy_for_all_devices(unit, taps, 
                                cookie.obj.wgroup, taps->param.host_share_table);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to destroy groups %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            return SOC_E_LIMIT;
        }
    }
    return SOC_E_NONE;
}

int _taps_dbucket_collision_find(taps_spivot_handle_t sph, void *user_data)
{
    int rv=SOC_E_NONE;
    trie_t *trie = NULL;
    taps_collision_dbucket_find_t *cbdata = (taps_collision_dbucket_find_t*)user_data;
    
    if (!sph || !user_data) return SOC_E_PARAM;
    
    if (cbdata->dbucket_id == sph->index) {
        if (cbdata->dph_idx < cbdata->taps->param.dbucket_attr.num_dbucket_pfx) {
            trie = sph->dbh->trie0;
        } else {
            trie = sph->dbh->trie1;
        }
        cbdata->dbh = sph->dbh;
        rv = trie_traverse(trie, _taps_dbucket_collision_find_cb,
                       cbdata, _TRIE_PREORDER_TRAVERSE);
    }

    return rv;
}

int _taps_sbucket_collision_find(taps_tcam_pivot_handle_t tph, void *user_data)
{
    int rv=SOC_E_NONE;
    taps_collision_dbucket_find_t *cbdata = (taps_collision_dbucket_find_t*)user_data;

    if (!tph || !user_data) return SOC_E_PARAM;

    if (cbdata->domain_id == tph->sbucket->domain) {
        rv = taps_sbucket_traverse(cbdata->unit, cbdata->taps, NULL,
                                   tph->sbucket, cbdata,
                                   _taps_dbucket_collision_find);
    }

    return rv;
}


int taps_collision_rehash(int unit, taps_handle_t taps,  
                                        taps_collision_dbucket_find_t *user_data)
{
    if (!taps || !user_data) return SOC_E_PARAM;
    
    taps_tcam_traverse(unit, taps, taps->tcam_hdl,
                     user_data, _taps_sbucket_collision_find);
    return SOC_E_NONE;
}

int taps_v6_collision_isr(int unit, int table_num, int entry_num, uint32 *key) 
{
    int rv;
    int taps_found = FALSE;
    int master_unit = SOC_SBX_MASTER(unit);
    taps_handle_t taps;
    dq_p_t elem;
    taps_collision_dbucket_find_t *taps_collison_cb_data = NULL;
    int entry_num_each_domain;
    int entry_num_each_dbucket;

    if(!key) {
        return SOC_E_PARAM;
    }
    
    taps_collison_cb_data = sal_alloc(sizeof(taps_collision_dbucket_find_t), "collison cb data");
    if (taps_collison_cb_data == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to alloc memory\n"), 
                   FUNCTION_NAME(), unit));
    	return SOC_E_MEMORY;
    }
    sal_memset(taps_collison_cb_data, 0, sizeof(taps_collision_dbucket_find_t));
    /* traverse to find the corresponding taps using table_num */
    if (!DQ_EMPTY(&taps_state[unit]->taps_object_list)) {
        DQ_TRAVERSE(&taps_state[unit]->taps_object_list, elem) {
            taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
            if (taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE] == table_num) {
                taps_found = TRUE;
#ifdef TAPS_V6_COLLISION_COUNT        
                taps->v6_collision_count++;
#endif
                taps_collison_cb_data->unit = unit;
                taps_collison_cb_data->taps = taps;
                taps_collison_cb_data->key = key;
                entry_num_each_dbucket = taps->param.dbucket_attr.num_dbucket_pfx * 2;
                entry_num_each_domain = taps->param.sbucket_attr.max_pivot_number * entry_num_each_dbucket;
                taps_collison_cb_data->domain_id = entry_num / entry_num_each_domain;
                taps_collison_cb_data->dbucket_id = (entry_num % entry_num_each_domain) / entry_num_each_dbucket;
                taps_collison_cb_data->dph_idx = (entry_num % entry_num_each_domain) % entry_num_each_dbucket;

                sal_mutex_take(taps->taps_mutex, sal_mutex_FOREVER);   
                rv = taps_collision_rehash(unit, taps, taps_collison_cb_data);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to enqueue command to work queue, %d:%s\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                    sal_free(taps_collison_cb_data);
                    sal_mutex_give(taps->taps_mutex);
                	return rv;
                }
                sal_mutex_give(taps->taps_mutex);
                break;
            }
        } DQ_TRAVERSE_END(&taps_state[unit]->taps_object_list, elem);
    }

    if (!taps_found && _IS_SLAVE_SHARE_LPM_TABLE(unit, master_unit, TRUE)) {
        if (!DQ_EMPTY(&taps_state[master_unit]->taps_object_list)) {
            DQ_TRAVERSE(&taps_state[master_unit]->taps_object_list, elem) {
                taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
                if (taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE] == table_num) {
                    taps_found = TRUE;
#ifdef TAPS_V6_COLLISION_COUNT        
                    taps->v6_collision_count++;
#endif
                    taps_collison_cb_data->unit = master_unit;
                    taps_collison_cb_data->taps = taps;
                    taps_collison_cb_data->key = key;
                    entry_num_each_dbucket = taps->param.dbucket_attr.num_dbucket_pfx * 2;
                    entry_num_each_domain = taps->param.sbucket_attr.max_pivot_number * entry_num_each_dbucket;
                    taps_collison_cb_data->domain_id = entry_num / entry_num_each_domain;
                    taps_collison_cb_data->dbucket_id = (entry_num % entry_num_each_domain) / entry_num_each_dbucket;
                    taps_collison_cb_data->dph_idx = (entry_num % entry_num_each_domain) % entry_num_each_dbucket;

                    sal_mutex_take(taps->taps_mutex, sal_mutex_FOREVER);
                    rv = taps_collision_rehash(master_unit, taps, taps_collison_cb_data);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(master_unit,
                                              "%s: unit %d Failed to enqueue command to work queue, %d:%s\n"), 
                                   FUNCTION_NAME(), master_unit, rv, soc_errmsg(rv)));
                        sal_free(taps_collison_cb_data);
                        sal_mutex_give(taps->taps_mutex);
                    	return rv;
                    }
                    sal_mutex_give(taps->taps_mutex);
                    break;
                }
            } DQ_TRAVERSE_END(&taps_state[master_unit]->taps_object_list, elem);
        }
    }
    sal_free(taps_collison_cb_data);
    return SOC_E_NONE;
}

#endif /* BCM_CALADAN3_SUPPORT */
