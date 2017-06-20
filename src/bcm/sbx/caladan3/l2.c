/* 
 * $Id: l2.c,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l2.c
 * Purpose:     BCM Layer-2 switch API
 */

#include <shared/bsl.h>

#include <sal/core/sync.h>

#include <soc/macipadr.h>
#include <soc/sbx/sbx_drv.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#endif /* BCM_CALADAN3_G3P1_SUPPORT */

#include <shared/gport.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/stack.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/field.h>

#include <shared/idxres_fl.h>
#include <shared/hash_tbl.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/state.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <bcm_int/sbx/caladan3/l2.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#include <bcm_int/sbx/caladan3/mcast.h>
#include <bcm_int/sbx/caladan3/wb_db_l2.h>
#endif

/*
 * Mutex Lock
 */
sal_mutex_t      _l2_mlock[BCM_LOCAL_UNITS_MAX];

#ifdef DEBUG_MACLOG
sal_mutex_t      _l2_log_lock[BCM_LOCAL_UNITS_MAX];

#define L2_LOG_LOCK(unit) sal_mutex_take(_l2_log_lock[unit], sal_mutex_FOREVER)
#define L2_LOG_UNLOCK(unit)  sal_mutex_give(_l2_log_lock[unit])

 
typedef struct maclog_s {
  
  bcm_mac_t       mac;
  uint32          fn;
  int             flag;
  int             unit;
  int             port;
  int             modid;
  int             vid;
  int             fte;
} maclog_t;


int log_index;
maclog_t maclog[0x4000];
            

#endif /* DEBUG_MACLOG */
/*
 * L2 Age
 *
 * The Age Timer thread is started when aging is enabled.
 *
 * Age-Interval / Age-Cycles
 *   The age time provided by the user indicates the duration of
 *   an age-interval.
 *
 *   Each age-interval can have one or more age-cycles.  This allows
 *   for the microcode aging process to traverse in each age-cycle
 *   blocks of entries, rather than the complete table.  The entire table
 *   should be processed at the end of each age-interval.
 *
 *   The number of entries processed in each age-cycle run is calculated
 *   from the user provided number of cycles for each age-interval as
 *   follows:
 *       entries_per_cycle = MacTableSize / cycles_per_interval
 *
 *   The number of cycles for an age-interval can be specified with
 *   the SOC property "l2_age_cycles".  The default value is '1',
 *   which results in processing ALL the MAC entries in one aging run
 *   (one age-cycle).
 *   
 *   The ager-age (or age timestamp) is incremented in each age-interval.
 *
 *   An entry is aged and removed after one age-interval, up to
 *   just beginning of the second interval, has ocurred since the
 *   last update to the entry.
 */
typedef struct _l2_age_s {
    char                thread_name[16];  /* L2 age timer thread name */
    VOL sal_thread_t    thread_id;        /* L2 age timer thread id */
    sal_sem_t           sem;              /* Semaphore to signal thread */
    uint32              ager;             /* Current ager age */
    int                 num_old_mac;      /* Number of old mac entries */
    int                 table_max;        /* L2 max table size */
    int                 cycles_interval;  /* Cycles per age interval */
    int                 entries_cycle;    /* Entries to process in one cycle */
    VOL int             age_sec;          /* User input age time */
    VOL uint64          cycle_usec;       /* Cycle time */
    int                 sleep_count;      /* For larger timer sleep */
    uint32              age_range;        /* Range of values age can take [0,age_range-1] */
} _l2_age_t;


static _l2_age_t      _l2_age[BCM_LOCAL_UNITS_MAX];
static uint           _l2_sem_usec_max;   /* Max value for sal_sem_take() */

#define L2_AGE_TIMER_THREAD_PRIO        50
#define L2_AGE_TIMER_STOP_WAIT_USEC     1000      /* 1 msec */
#define L2_AGE_TIMER_STOP_RETRIES_MAX   100

/*  Maximum entries in the L2 MAC Table */
#define L2_MAX_TABLE_ENTRIES   4096

#define L2_AGE_TIMER_WAKE(unit)                                        \
    do {                                                               \
        _l2_age[unit].sleep_count = 0;                                 \
        sal_sem_give(_l2_age[unit].sem);                               \
    } while(0)

#define L2_AGE_TIMER_SLEEP(unit, usec)                                 \
    do {                                                               \
        uint32  sleep_time;                                            \
        if (soc_sbx_div64(usec, _l2_sem_usec_max, &sleep_time)         \
            == SOC_E_PARAM) break;                                     \
        _l2_age[unit].sleep_count = sleep_time + 1;                    \
        if (soc_sbx_div64(usec, _l2_age[unit].sleep_count, &sleep_time)\
            == SOC_E_PARAM) break;                                     \
        while ((_l2_age[unit].sleep_count-- > 0) &&                    \
               (_l2_age[unit].age_sec != 0)) {                         \
            sal_sem_take(_l2_age[unit].sem, sleep_time);               \
        }                                                             \
    } while(0)



                                                 
#define L2_AGE_CYCLE_MIN_USEC           100       /* Minimum between cycles */

#define L2_AGE_INCR(unit,age,range)    \
    ((age) = ((age+1) == range) ? 0 : ((age)+1))

#define L2_AGE_TIMER_MAX_SECS           1000000   /* Max age timer value */

#define L2_INVALID_EGRESS_ID         ~0

#define L2_ID_RESERVED(u, id) \
   ( (_l2_egress[u].idList.reservedLow != L2_INVALID_EGRESS_ID) &&   \
     (_l2_egress[u].idList.reservedHigh != L2_INVALID_EGRESS_ID) &&  \
     ((id) >= _l2_egress[u].idList.reservedLow ) &&                  \
     ((id) <= _l2_egress[u].idList.reservedHigh) )


typedef struct _l2_egress_id_list_s {
    shr_idxres_list_handle_t    idMgr;
    uint32                    reservedHigh;
    uint32                    reservedLow;
} _l2_egress_id_list_t;

/*
 * L2 Egress
 */
typedef struct _l2_egress_s {
    uint8                       init_done;
    _l2_egress_id_list_t        idList;
    int                         encap_id_max;
    uint32                      *dest_ports;
} _l2_egress_t;

static _l2_egress_t   _l2_egress[BCM_LOCAL_UNITS_MAX];


#define L2_EGR_DEST_PORT_GET(unit, idx, gport) \
    do { \
        if ((_l2_egress[(unit)].dest_ports) && \
            ((idx) <= _l2_egress[(unit)].encap_id_max)) { \
            (gport) = *(((uint32 *)(_l2_egress[(unit)].dest_ports)) + idx); \
        } \
    } while (0)
    
#define L2_EGR_DEST_PORT_SET(unit, idx, gport) \
    do { \
        if ((_l2_egress[(unit)].dest_ports) && \
            ((idx) <= _l2_egress[(unit)].encap_id_max)) { \
            *(((uint32 *)(_l2_egress[(unit)].dest_ports)) + idx) = (gport); \
        } \
    } while (0)

#define L2_EGR_DEST_PORT_INVALID 0xffffffff

/*
 * L2 Callback Message Registration
 */
#define L2_CB_MAX             3    /* Max registered callbacks per unit */

typedef struct _l2_cb_entry_s {
    bcm_l2_addr_callback_t  fn;         /* Callback routine */
    void                    *fn_data;   /* User defined data */
} _l2_cb_entry_t;

typedef struct _l2_cb_s {
    _l2_cb_entry_t    entry[L2_CB_MAX];
    int               count;
} _l2_cb_t;

static _l2_cb_t                  _l2_cb[BCM_LOCAL_UNITS_MAX];

#define L2_CB_ENTRY(unit, i)    (_l2_cb[unit].entry[i])
#define L2_CB_COUNT(unit)       (_l2_cb[unit].count)

/* Execute registered callback routines for given unit */
#define L2_CB_RUN(unit, l2addr, insert)    \
    do {                                                              \
        int _i;                                                       \
        for (_i = 0; _i < L2_CB_MAX; _i++) {                          \
            if (L2_CB_ENTRY(unit, _i).fn) {                           \
                L2_CB_ENTRY(unit, _i).fn((unit), (l2addr), (insert),  \
                                   L2_CB_ENTRY((unit), _i).fn_data);  \
            }                                                         \
        }                                                             \
    } while (0)


/*
 * General Utility Macros
 */
#define UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) { return BCM_E_UNIT; }

#define UNIT_INIT_DONE(unit)    (_l2_mlock[unit] != NULL)

#define UNIT_INIT_CHECK(unit) \
    do { \
        UNIT_VALID_CHECK(unit); \
        if (_l2_mlock[unit] == NULL) { return BCM_E_INIT; } \
    } while (0)


#define TRUNK_VALID_CHECK(tid) \
    if (!SBX_TRUNK_VALID(tid)) { return BCM_E_BADID; }

/*
 * BCM L2 flags NOT supported in device
 */
#define L2_FLAGS_UNSUPPORTED \
    ( BCM_L2_COPY_TO_CPU | BCM_L2_HIT       | BCM_L2_REPLACE_DYNAMIC | \
      BCM_L2_SRC_HIT     | BCM_L2_DES_HIT   | BCM_L2_REMOTE_LOOKUP   | \
      BCM_L2_NATIVE      | BCM_L2_MOVE      | BCM_L2_FROM_NATIVE     | \
      BCM_L2_TO_NATIVE   | BCM_L2_MOVE_PORT )


#define L2_EGRESS_VALID_ENCAP_ID(encap_id) \
    ((SOC_SBX_IS_VALID_L2_ENCAP_ID(encap_id)) && \
      (SOC_SBX_OFFSET_FROM_L2_ENCAP_ID((uint32)encap_id) < SBX_MAX_L2_EGRESS_OHI))

#define L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id) \
    if (!L2_EGRESS_VALID_ENCAP_ID(encap_id)) { return BCM_E_PARAM; }



/* L2 Stat state */
typedef struct _l2_stat_state_s {
    shr_htb_hash_table_t  fieldTable; /* hash(port,mac) -> fieldEntry */

    int                   numStatsInUse;
    int                   groupsValid;
    bcm_field_group_t     smacGroup;  /* count both smac & dmac */
    bcm_field_group_t     dmacGroup;
    
} _l2_stat_state_t;

typedef struct _l2_stat_fields_s {
    bcm_field_entry_t   smac;
    bcm_field_entry_t   dmac;
} _l2_stat_fields_t;

typedef uint8 _l2_stat_key_t[10];

#define L2_STAT_NUM_QUALIFIERS    3


#define L2_MAX_STAT_ENTRIES       (4*1024)

static _l2_stat_state_t      *_l2_stat_state[BCM_LOCAL_UNITS_MAX];

#define L2_STAT_STATE(u) (_l2_stat_state[(u)])


/* Forward local function declarations */
STATIC int _bcm_caladan3_l2_age_timer_init(int unit);
STATIC int _bcm_caladan3_l2_age_timer_enable(int unit, int age_seconds);
STATIC int _bcm_caladan3_l2_age_timer_disable(int unit);
STATIC int _bcm_caladan3_l2_age_ager_set(int unit, uint32 ager);
STATIC int _bcm_caladan3_l2_egress_init(int unit);
int _bcm_caladan3_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion);
extern int bcm_caladan3_l2_cache_init(int unit);
extern int bcm_caladan3_l2_cache_detach(int unit);

/* Get an id to link to the microcode's aging table */

extern int _bcm_caladan3_l2_age_id_get(int unit);

extern _ageid_to_mac_info_t *p_age_id_mac_array[BCM_LOCAL_UNITS_MAX];
extern _age_id_stack_t      age_indexes_stack[BCM_LOCAL_UNITS_MAX];

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Restore L2 Egress 
 */
int
_bcm_caladan3_l2_egress_restore(int unit)
{
    int             rv = BCM_E_NONE;
    int             i;
    uint32        localId, encapId=L2_EGR_DEST_PORT_INVALID;

    /* Reallocated OHIs as needed */
    for (i=0; i < SBX_MAX_L2_EGRESS_OHI; i++) {
        L2_EGR_DEST_PORT_GET(unit, i, encapId);
        if (encapId != L2_EGR_DEST_PORT_INVALID) {
            localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encapId);
            /* reserve the id only if it's not in the application reserved range */
            if (!L2_ID_RESERVED(unit, localId)) {
                rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                             localId, localId);
                if (rv != BCM_E_NONE) {
                    /* non-fatal error during warmboot */
                    LOG_WARN(BSL_LS_BCM_L2,
                             (BSL_META_U(unit,
                                         "L2 egress failed to re-allocate encap id: 0x%x, localId: 0x%x\n"),
                              encapId, localId));
                }
            }else{
                LOG_VERBOSE(BSL_LS_BCM_L2,
                            (BSL_META_U(unit,
                                        "encap 0x%08x (locaId 0x%x) found in reserved range,"
                                         " not re-allocated - OK\n"),
                             encapId, localId));
            }
        }
    }

    return rv;
}

/*
 * Get the dest 
 */
uint32 *bcm_sbx_l2_egress_dest_port_ptr_get(int unit)
{
    return _l2_egress[unit].dest_ports;
}

/*
 * Get the age_sec from the _l2_age 
 */
INLINE int bcm_sbx_caladan3_l2_age_sec_get (int unit)
{
    return _l2_age[unit].age_sec; 
}
#endif /* BCM_WARM_BOOT_SUPPORT */
/*
 * Function:
 *     _bcm_caladan3_l2_hw_init
 * Purpose:
 *     Initialize hardware and ucode L2 layer management tables
 *     for the specified device.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_hw_init(int unit)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2_hw_init(unit);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_UNAVAIL;
    }
}

int
_bcm_caladan3_l2_egress_detach(int unit)
{
    int rv = BCM_E_NONE;

    /* free resources */
    rv = shr_idxres_list_destroy(_l2_egress[unit].idList.idMgr);
    _l2_egress[unit].idList.idMgr = NULL;
    if (_l2_egress[unit].dest_ports) {
        sal_free(_l2_egress[unit].dest_ports);
        _l2_egress[unit].dest_ports = NULL;
    }

    return rv;
}


/*
 * Function:
 *     bcm_l2_detach
 * Purpose:
 *     Detach BCM L2 layer from unit.
 *     Deallocate software local resources such as semaphores.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_caladan3_l2_detach(int unit)
{
    int  rv = BCM_E_NONE;
    int  i;

    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

    /*
     * Call chip-dependent finalization ?
     */

    /* Stop L2 age timer and destroy semaphore */
    rv = _bcm_caladan3_l2_age_timer_disable(unit);
    if (BCM_SUCCESS(rv)) {
        if (_l2_age[unit].sem != NULL) {
            sal_sem_destroy(_l2_age[unit].sem);
            _l2_age[unit].sem = NULL;
        }
    }

    /* Clear L2 callback routines */
    for (i = 0; i < L2_CB_MAX; i++) {
        L2_CB_ENTRY(unit, i).fn = NULL;
        L2_CB_ENTRY(unit, i).fn_data = NULL;
    }
    L2_CB_COUNT(unit) = 0;

    if (BCM_FAILURE(rv = _bcm_caladan3_l2_egress_detach(unit))) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_l2_egress_detach failed rv=%d(%s)"),
                   rv,
                   bcm_errmsg(rv)));
    }

#if 0
    /* Note: this is commented out in the fe2000 code */
    if (BCM_FAILURE(rv = bcm_caladan3_l2_cache_detach(unit))) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "bcm_caladan3_l2_cache_detach failed rv=%d(%s)"),
                   rv,
                   bcm_errmsg(rv)));
    }
#endif

    L2_UNLOCK(unit);

    if (p_age_id_mac_array[unit] != NULL) {
        sal_free(p_age_id_mac_array[unit]);
        p_age_id_mac_array[unit] = NULL;
    }
    if (age_indexes_stack[unit].age_indexes != NULL) {
        sal_free(age_indexes_stack[unit].age_indexes);
        age_indexes_stack[unit].age_indexes = NULL;
    }

    /* Destroy mutex lock */
    sal_mutex_destroy(_l2_mlock[unit]);
    _l2_mlock[unit] = NULL;

#ifdef DEBUG_MACLOG
    if (_l2_log_lock[unit] != NULL) {
      sal_mutex_destroy(_l2_log_lock[unit]);
      _l2_log_lock[unit] = NULL;
    } 
#endif /* DEBUG_MACLOG */

    return rv;
}

/*
 * Function:
 *     bcm_l2_init
 * Purpose:
 *     Initialize the L2 interface layer for the specified device.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE    - Success (or already initialized)
 *     BCM_E_MEMORY  - Failed to allocate required memory or lock
 *     BCM_E_XXX     - Failure, other errors
 */
int
bcm_caladan3_l2_init(int unit)
{
    int  rv = BCM_E_NONE;
    int  i;

    /* Check unit */
    UNIT_VALID_CHECK(unit);


#ifdef DEBUG_MACLOG
    if (_l2_log_lock[unit] != NULL) {
      sal_mutex_destroy(_l2_log_lock[unit]);
      _l2_log_lock[unit] = NULL;
    } 
#endif /* DEBUG_MACLOG */

    if (_l2_mlock[unit] != NULL) {
        rv = bcm_caladan3_l2_detach(unit);
        BCM_IF_ERROR_RETURN(rv);
        _l2_mlock[unit] = NULL;
    }

    /* Mutex lock */
    if (_l2_mlock[unit] == NULL) {
        if ((_l2_mlock[unit] = sal_mutex_create("bcm_l2_lock")) == NULL) {
            rv = BCM_E_MEMORY;
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "failed to create L2 mutex rv=%d(%s)\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }
    }

#ifdef DEBUG_MACLOG
    if ((_l2_log_lock[unit] = sal_mutex_create("mac_log_lock")) == NULL) {
      rv = BCM_E_MEMORY;
      LOG_ERROR(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "failed to create MAC LOG  mutex rv=%d(%s)\n"),
                 rv, bcm_errmsg(rv)));
      return rv;
    }
#endif /* DEBUG_MACLOG */


    L2_LOCK(unit);

    /* Initialize L2 age timer */
    if (BCM_FAILURE(rv = _bcm_caladan3_l2_age_timer_init(unit))) {
        L2_UNLOCK(unit);
        sal_mutex_destroy(_l2_mlock[unit]);
        _l2_mlock[unit] = NULL;
        return rv;
    }

    /* Initialize L2 callback routines */
    for (i = 0; i < L2_CB_MAX; i++) {
        L2_CB_ENTRY(unit, i).fn = NULL;
        L2_CB_ENTRY(unit, i).fn_data = NULL;
    }
    L2_CB_COUNT(unit) = 0;

    /* Initialize device hardware, ucode */
    if (BCM_FAILURE(rv = _bcm_caladan3_l2_hw_init(unit))) {
        L2_UNLOCK(unit);
        sal_mutex_destroy(_l2_mlock[unit]);
        _l2_mlock[unit] = NULL;
        return rv;        
    }

    L2_UNLOCK(unit);


    /* Initialize the l2 egress */
    if (BCM_FAILURE(rv = _bcm_caladan3_l2_egress_init(unit))) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_l2_egress_init failed rv=%d(%s)"),
                   rv, bcm_errmsg(rv)));
    }

    /* Initialize the l2_cache */
    if (BCM_FAILURE(rv = bcm_caladan3_l2_cache_init(unit))) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "bcm_caladan3_l2_cache_init failed rv=%d(%s)"),
                   rv, bcm_errmsg(rv)));
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    rv = bcm_caladan3_wb_l2_state_init(unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "returning rv=%d\n"),
                 rv));

    /* Disable the L2 API cache  */
    SOC_SBX_STATE(unit)->cache_l2 = FALSE;
    /* Enable l2_age deletion */
    SOC_SBX_STATE(unit)->l2_age_delete = TRUE;


    return rv;
}


/*
 * Function:
 *     bcm_l2_clear
 * Purpose:
 *     Clear the BCM L2 layer for given unit.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_caladan3_l2_clear(int unit)
{
    UNIT_INIT_CHECK(unit);

    return bcm_caladan3_l2_init(unit);
}


/*
 * Function:
 *     _bcm_caladan3_l2addr_to_mcaddr
 * Purpose:
 *     Translate information from L2 'bcm_l2_addr_t' structure
 *     to MCAST 'bcm_mcast_addr_t' structure.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - L2 address structure to translate
 *     mcaddr    - (OUT) MCAST address structure
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_caladan3_l2addr_to_mcaddr(int unit, bcm_l2_addr_t  *l2addr,
                             bcm_mcast_addr_t *mcaddr)
{
    bcm_mcast_addr_t_init(mcaddr, l2addr->mac, l2addr->vid);
    mcaddr->cos_dst    = l2addr->cos_dst;
    mcaddr->l2mc_index = l2addr->l2mc_group;

    /* Adding the L2 MAC address starts without any port members */
    BCM_PBMP_CLEAR(mcaddr->pbmp);
    BCM_PBMP_CLEAR(mcaddr->ubmp);
    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_mcast_add
 * Purpose:
 *     Add a MCAST entry to Dmac table with given L2 address information.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - L2 address to add
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_caladan3_l2_mcast_add(int unit, bcm_l2_addr_t *l2addr)
{
    uint32            fte;
    uint32            mcgroup;
    bcm_mcast_addr_t  mcaddr;
    
    BCM_IF_ERROR_RETURN(_bcm_caladan3_l2addr_to_mcaddr(unit, l2addr, &mcaddr));

    return _bcm_caladan3_mcast_addr_add(unit, &mcaddr,
                                        l2addr->flags,
                                        BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX,
                                        &fte, &mcgroup);
}


/*
 * Function:
 *     _bcm_caladan3_l2_mcast_get
 * Purpose:
 *     Update MCAST information in L2 address structure
 *     for given MAC address and VLAN ID.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - (IN/OUT) L2 address structure to update
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_caladan3_l2_mcast_get(int unit, bcm_l2_addr_t *l2addr)
{
    bcm_mcast_addr_t  mcaddr;

    BCM_IF_ERROR_RETURN(bcm_mcast_port_get(unit, l2addr->mac, l2addr->vid,
                                           &mcaddr));

    l2addr->l2mc_group = mcaddr.l2mc_index;
    l2addr->cos_dst    = mcaddr.cos_dst;
    l2addr->flags     |= BCM_L2_MCAST;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_multicast_l2_encap_get
 * Purpose:
 *     Given the multicast group, port parameters get
 *     corresponding encap_id
 * Parameters:
 *     unit   - Device number
 *     group  - multicast group
 *     gport  - gport for exiting port
 *     vlan   - vlan for the l2 address
 *     encap_id - encap_id 
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     !!! Called from outside of L2 module (from multicast)
 *     Hence take and release necessary locks
 */

int
_bcm_caladan3_multicast_l2_encap_get(int              unit, 
                                   bcm_multicast_t  group,
                                   bcm_gport_t      gport,
                                   bcm_vlan_t       vlan,
                                   bcm_if_t        *encap_id)
{
    bcm_port_t port = gport;
    
    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(encap_id);

    if (BCM_GPORT_IS_LOCAL(gport)) {
        port = BCM_GPORT_LOCAL_GET(gport);
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        port  = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            return (_bcm_caladan3_g3p1_multicast_l2_encap_get(unit,
                                                            group,
                                                            gport,
                                                            vlan,
                                                            encap_id));
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            return BCM_E_CONFIG;
        }
    } else {
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    *encap_id = vlan;

    return BCM_E_NONE;
}


int
_bcm_caladan3_l2addr_hw_add(int unit, bcm_l2_addr_t *l2addr)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2addr_hw_add(unit, l2addr);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     bcm_l2_addr_add
 * Purpose:
 *     Add a MAC address to the Switch Address Resolution Logic (ARL)
 *     port with the given VLAN ID and parameters.
 * Parameters:
 *     unit   - Device number
 *     l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Use flag of BCM_L2_LOCAL_CPU to associate the entry with the CPU.
 *     Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *     Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *     passed as well with non-zero trunk group ID)
 */
int
bcm_caladan3_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    int    rv = BCM_E_NONE;

    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);

#ifdef DEBUG_MACLOG
    int    i;
    if (l2addr->mac[0]==0xff && l2addr->mac[1]==0xff && l2addr->mac[2]==0xff && 
	l2addr->mac[3]==0xff && l2addr->mac[4]==0xff && l2addr->mac[5]==0xff) {

      L2_LOG_LOCK(unit);
      for (i=0; i<0x4000; i++) {
	if (maclog[i].flag == 1) {
	  LOG_CLI((BSL_META_U(unit,
                              "maclog[%4x]: mac=%02x:%02x:%02x:%02x:%02x:%02x fn = 0x%04x, port = 0x%04x,modid=0x%04x, vid=%d, fte=%x\n"),
                   i,  
                   maclog[i].mac[0],
                   maclog[i].mac[1],
                   maclog[i].mac[2],
                   maclog[i].mac[3],
                   maclog[i].mac[4],
                   maclog[i].mac[5],
                   maclog[i].fn, 
                   maclog[i].port,
                   maclog[i].modid,
                   maclog[i].vid,
                   maclog[i].fte));

	  
	  maclog[i].mac[0]=0;
	  maclog[i].mac[1]=0;
	  maclog[i].mac[2]=0;
	  maclog[i].mac[3]=0;
	  maclog[i].mac[4]=0;
	  maclog[i].mac[5]=0;
	  maclog[i].fn=0;
	  maclog[i].port=0;
	  maclog[i].modid=0;
	  maclog[i].vid=0;
	  maclog[i].flag=0;
	  maclog[i].fte=0;
	}
      }
      log_index=0;
      L2_LOG_UNLOCK(unit);
    } 


# if 0
    int my_log_id;    
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=1;
    maclog[my_log_id].mac[0]=l2addr->mac[0];
    maclog[my_log_id].mac[1]=l2addr->mac[1];
    maclog[my_log_id].mac[2]=l2addr->mac[2];
    maclog[my_log_id].mac[3]=l2addr->mac[3];
    maclog[my_log_id].mac[4]=l2addr->mac[4];
    maclog[my_log_id].mac[5]=l2addr->mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=l2addr->port;
    maclog[my_log_id].modid=l2addr->modid;
    maclog[my_log_id].vid=l2addr->vid;
    maclog[my_log_id].fte=-1;
#endif    
    

#endif /* DEBUG_MACLOG */
    /* Check for valid flags */
    if (l2addr->flags & L2_FLAGS_UNSUPPORTED) {
        return BCM_E_PARAM;
    }

    /* Special handling on a mcast entry */
    if (l2addr->flags & BCM_L2_MCAST) {
        L2_LOCK(unit);
        rv = _bcm_caladan3_l2_mcast_add(unit, l2addr);
        L2_UNLOCK(unit);
        return rv;        
    }

    /* If LOCAL_CPU, get local CPU port and modid */
    if (l2addr->flags & BCM_L2_LOCAL_CPU) {
        l2addr->port = CMIC_PORT(unit);
        BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &l2addr->modid));
    }

    rv = _bcm_caladan3_l2addr_hw_add(unit, l2addr);

    return rv;
}

/*
 * Function:
 *     bcm_l2_addr_get
 * Purpose:
 *     Given a MAC address and VLAN ID, return all associated information
 *     if entry is present in the L2 tables.
 * Parameters:
 *     unit   - Device number
 *     mac    - MAC address to search
 *     vid    - VLAN id to search
 *     l2addr - (OUT) Pointer to bcm_l2_addr_t structure to return L2 entry
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_PARAM     - Illegal parameter (NULL pointer)
 *     BCM_E_XXX       - Failure, other
 */
int
bcm_caladan3_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                       bcm_l2_addr_t *l2addr)
{
    int  rv = BCM_E_UNAVAIL;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);


    L2_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_addr_get(unit, mac, vid, l2addr);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    L2_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete
 * Purpose:
 *     Delete an L2 address (MAC+VLAN) from the device.
 * Parameters:
 *     unit - Device number
 *     mac  - MAC address to delete
 *     vid  - VLAN id 
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_XXX       - Failure
 */
int
bcm_caladan3_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    int rv = BCM_E_UNAVAIL;


#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=10;
    maclog[my_log_id].mac[0]=mac[0];
    maclog[my_log_id].mac[1]=mac[1];
    maclog[my_log_id].mac[2]=mac[2];
    maclog[my_log_id].mac[3]=mac[3];
    maclog[my_log_id].mac[4]=mac[4];
    maclog[my_log_id].mac[5]=mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=-1;     
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;


#endif /* DEBUG_MACLOG */
    /* Check params */
    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

      switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_addr_delete(unit, mac, vid);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
      default:
          SBX_UNKNOWN_UCODE_WARN(unit);
          rv = BCM_E_CONFIG;
      }

    L2_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_l2_addr_delete_by
 * Purpose:
 *     Delete MAC entries associated with specified information
 *     from Smac and Dmac tables.
 * Parameters:
 *     unit  - bcm unit number
 *     match - L2 addr information to compare
 *     flags - Indicates what information to compare
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_addr_delete_by(int unit, bcm_l2_addr_t *match, uint32 cmp_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2_addr_delete_by(unit, match, cmp_flags);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_mac
 * Purpose:
 *     Delete L2 entries associated with a MAC address.
 * Parameters:
 *     unit  - Device number
 *     mac   - MAC address
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_mac(int unit, bcm_mac_t mac, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    UNIT_INIT_CHECK(unit);



#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=9;
    maclog[my_log_id].mac[0]=mac[0];
    maclog[my_log_id].mac[1]=mac[1];
    maclog[my_log_id].mac[2]=mac[2];
    maclog[my_log_id].mac[3]=mac[3];
    maclog[my_log_id].mac[4]=mac[4];
    maclog[my_log_id].mac[5]=mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=-1;     
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=-1;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    bcm_l2_addr_t_init(&match, mac, 0);
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match, L2_CMP_MAC);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan
 * Purpose:
 *     Delete L2 entries associated with a VLAN.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_vlan(int unit, bcm_vlan_t vid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;


    /* Check params */
    UNIT_INIT_CHECK(unit);

#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=8;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=-1;     
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    sal_memset(&match, 0, sizeof(match));
    match.vid   = vid;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match, L2_CMP_VLAN);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_port
 * Purpose:
 *     Delete L2 entries associated with a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_port(int unit, bcm_module_t mod, bcm_port_t port,
                                  uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    /* Need to check for mod/port */
    UNIT_INIT_CHECK(unit);

#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=2;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=port;
    maclog[my_log_id].modid=mod;
    maclog[my_log_id].vid=0;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match, L2_CMP_PORT);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_trunk
 * Purpose:
 *     Delete L2 entries associated with a trunk.
 * Parameters:
 *     unit  - Device number
 *     tid   - Trunk id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_trunk(int unit, bcm_trunk_t tid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;
    /* Check params */
    UNIT_INIT_CHECK(unit);
    TRUNK_VALID_CHECK(tid);

#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;

    L2_LOG_UNLOCK(unit);
    
    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=3;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=tid;     /* tid share with port */
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=0;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    match.tgid  = tid;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match, L2_CMP_TRUNK);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_mac_port
 * Purpose:
 *     Delete L2 entries associated with a MAC address and
 *     a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     mac   - MAC address
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_mac_port(int unit, bcm_mac_t mac,
                                      bcm_module_t mod, bcm_port_t port,
                                      uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;


    /* Check params */
    /* Need to check mod/port */
    UNIT_INIT_CHECK(unit);

#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=4;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=port;     /* tid share with port */
    maclog[my_log_id].modid=mod;
    maclog[my_log_id].vid=0;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */
    
    L2_LOCK(unit);

    bcm_l2_addr_t_init(&match, mac, 0);
    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match, L2_CMP_MAC | L2_CMP_PORT);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan_port
 * Purpose:
 *     Delete L2 entries associated with a VLAN and
 *     a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_vlan_port(int unit, bcm_vlan_t vid,
                                       bcm_module_t mod, bcm_port_t port,
                                       uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;


    /* Check params */
    /* Need to check mod/port */
    UNIT_INIT_CHECK(unit);

#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=5;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=port;     /* tid share with port */
    maclog[my_log_id].modid=mod;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    match.vid   = vid;
    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match,
                                       L2_CMP_VLAN | L2_CMP_PORT);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan_trunk
 * Purpose:
 *     Delete L2 entries associated with a VLAN and a destination trunk.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     tid   - Trunk id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_caladan3_l2_addr_delete_by_vlan_trunk(int unit, bcm_vlan_t vid,
                                        bcm_trunk_t tid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    TRUNK_VALID_CHECK(tid);


#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=6;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=tid;     /* tid share with port */
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    L2_LOCK(unit);

    match.vid   = vid;
    match.tgid  = tid;
    match.flags = flags;
    rv = _bcm_caladan3_l2_addr_delete_by(unit, &match,
                                       L2_CMP_VLAN | L2_CMP_TRUNK);

    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Returning %d\n"),
                 rv));

    return rv;
}

      
/*
 * Function:
 *     bcm_l2_addr_register
 * Purpose:
 *     Register a callback routine that will be called whenever
 *     an entry is inserted into or deleted from the L2 address table
 *     by the BCM layer.
 *     For SBX devices, the BCM layer only perform deletes of
 *     old mac entries from L2 address table when aging is enabled.
 * Parameters:
 *     unit    - Device number
 *     fn      - Callback function of type bcm_l2_addr_callback_t
 *     fn_data - Arbitrary value passed to callback along with messages
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Caladan3 does not support L2 learning and aging in hardware.
 *     Software aging provided by the BCM layer when aging is enabled.
 *     The BCM layer does not perform L2 learning.
 *     User must register callback to RX to get notification on new mac.
 */
int
bcm_caladan3_l2_addr_register(int unit, bcm_l2_addr_callback_t fn,
                            void *fn_data)
{
    int  i;
    unsigned char *p = (unsigned char *)&fn;
    char addr[16];

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(fn);

    L2_LOCK(unit);

    /* Check if given function is already registered (with same data) */
    for (i = 0; i < L2_CB_MAX; i++) {
        if ((L2_CB_ENTRY(unit, i).fn == fn) &&
            (L2_CB_ENTRY(unit, i).fn_data == fn_data)) {
            L2_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }

    /* Add to list */
    if (L2_CB_COUNT(unit) >= L2_CB_MAX) {
        L2_UNLOCK(unit);
        return BCM_E_RESOURCE;
    }

    for (i = 0; i < L2_CB_MAX; i++) {
        if (L2_CB_ENTRY(unit, i).fn == NULL) {
            L2_CB_ENTRY(unit, i).fn = fn;
            L2_CB_ENTRY(unit, i).fn_data = fn_data;
            L2_CB_COUNT(unit)++;
            break;
        }
    }

    L2_UNLOCK(unit);

    sal_sprintf(addr, "0x%02x%02x%02x%02x", p[0], p[1], p[2], p[3]);
    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "fn=%s data=%p count=%d\n"),
                 addr, fn_data, L2_CB_COUNT(unit)));

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_l2_addr_unregister
 * Purpose:
 *     Unregister a previously registered callback routine.
 * Parameters:
 *     unit    - Device number
 *     fn      - Same callback function used to register callback
 *     fn_data - Same arbitrary value used to register callback
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_caladan3_l2_addr_unregister(int unit, bcm_l2_addr_callback_t fn,
                              void *fn_data)
{
    int  i;
    unsigned char *p = (unsigned char *)&fn;
    char addr[16];

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(fn);

    L2_LOCK(unit);

    if (L2_CB_COUNT(unit) == 0) {
        L2_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }        

    for (i = 0; i < L2_CB_MAX; i++) {
        if ((L2_CB_ENTRY(unit, i).fn == fn) &&
            (L2_CB_ENTRY(unit, i).fn_data == fn_data)) {
            L2_CB_ENTRY(unit, i).fn = NULL;
            L2_CB_ENTRY(unit, i).fn_data = NULL;
            L2_CB_COUNT(unit)--;
            break;
        }
    }

    L2_UNLOCK(unit);

    if (i >= L2_CB_MAX) {
        return BCM_E_NOT_FOUND;
    }

    sal_sprintf(addr, "0x%02x%02x%02x%02x", p[0], p[1], p[2], p[3]);
    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "fn=%s data=%p count=%d\n"),
                 addr, fn_data, L2_CB_COUNT(unit)));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_mac_size_get
 * Purpose:
 *     Get the L2 MAC table size.
 * Parameters:
 *     unit       - Device number
 *     table_size - Returns the L2 MAC table size.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid params.
 */
 int
_bcm_caladan3_l2_mac_size_get(int unit, int *table_size)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2_mac_size_get(unit, table_size);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }
    
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_caladan3_l2_age_timer_init
 * Purpose:
 *     Initialize the L2 age timer submodule.
 *     Create semaphore and initialize L2 age timer internal information.
 *     It does NOT start the L2 age timer thread (if thread is running,
 *     this will be stopped).
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure, could not create signaling semaphore, or
 *                  could not stop L2 age timer thread
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_age_timer_init(int unit)
{
    int   rv = BCM_E_NONE;
    int   cycles;
    char  sem_name[20];


    /* Init ager */
    BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_ager_set(unit, 0));
    
    /* Get number of cycles per age interval */
    cycles =  SOC_SBX_CFG_CALADAN3(unit)->l2_age_cycles;

    /*
     * Get max positive integer value, needed to determine max
     * value allowed for sal_sem_take().
     */
    _l2_sem_usec_max = (1 << ((sizeof(int) * 8) - 1));
    _l2_sem_usec_max--;

    /* Initialize age interval and cycle information */
    _l2_age[unit].num_old_mac = 0;
    _l2_age[unit].table_max = soc_sbx_g3p1_macage_table_size_get(unit);
    _l2_age[unit].cycles_interval = cycles;
    _l2_age[unit].entries_cycle = (_l2_age[unit].table_max + (cycles - 1)) / cycles;

    /* Initialize times */
    _l2_age[unit].age_sec = 0;
    COMPILER_64_ZERO(_l2_age[unit].cycle_usec);

    /* Initialize the range for L2 aging */
     _bcm_caladan3_g3p1_l2_age_range_get(unit, &_l2_age[unit].age_range);

    /* Initialize L2 age timer thread information, stop thread if running */
    _l2_age[unit].thread_name[0] ='\0';
    _l2_age[unit].sleep_count    = 0;
    if (_l2_age[unit].thread_id != NULL) {
        rv = _bcm_caladan3_l2_age_timer_disable(unit);
    }

    /* Create L2 age timer semaphore */
    if (_l2_age[unit].sem == NULL) {
        sal_snprintf(sem_name, sizeof(sem_name), "bcm_l2_age_SLEEP%d", unit);
        if ((_l2_age[unit].sem = sal_sem_create(sem_name,
                                                sal_sem_BINARY, 0)) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "L2 age timer init table_max=%d "
                             "cycles=%d entries_cycles=%d rv=%d\n"),
                 _l2_age[unit].table_max, cycles, _l2_age[unit].entries_cycle, rv));

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_l2_age_timer_thread_stop
 * Purpose:
 *     Stop L2 age timer thread.  Clears thread information.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure, could not stop L2 age timer thread
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_age_timer_thread_stop(int unit)
{
    int  retries;

    _l2_age[unit].age_sec = 0;
    COMPILER_64_ZERO(_l2_age[unit].cycle_usec);
    _l2_age[unit].thread_name[0] ='\0';

    if (_l2_age[unit].thread_id == NULL) {
        return BCM_E_NONE;
    }

    /* Stop L2 age timer thread */
    L2_AGE_TIMER_WAKE(unit);

    /* Wait for thread to exit */
    for (retries = 0; retries < L2_AGE_TIMER_STOP_RETRIES_MAX; retries++) {
        if (_l2_age[unit].thread_id == NULL) {
            break;
        }
        sal_usleep(L2_AGE_TIMER_STOP_WAIT_USEC);
    }
    if (retries >= L2_AGE_TIMER_STOP_RETRIES_MAX) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "L2 age timer thread did not stop as requested\n")));
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l2_age_ager_get
 * Purpose:
 *     Get the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     ager - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */
STATIC int
_bcm_caladan3_l2_age_ager_get(int unit, uint32 *ager)
{
	
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2_age_ager_get(unit, ager);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }
    
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *     _bcm_caladan3_l2_age_ager_set
 * Purpose:
 *     Set the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     ager - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */
STATIC int
_bcm_caladan3_l2_age_ager_set(int unit, uint32 ager)
{
    _l2_age[unit].ager = ager;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_l2_age_ager_set(unit, ager);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     _bcm_caladan3_l2_async_cb
 * Purpose:
 *     L2  async callback routine to report and possibly delete given MAC entry from
 *     Smac and Dmac tables.
 *     This routine is called from the aging function
 *     each time there is an aged mac entry.
 * Parameters:
 *     unit       - Device number
 *     mac        - mac address
 *     vid        - vlan id
 *     delete     - whether to delete the entry or not
 * Returns:
 *     None
 * Notes:
 */
STATIC void
_bcm_caladan3_l2_async_cb(int unit, sal_mac_addr_t mac, int vid, int delete)
{
    int rv = BCM_E_NONE;
    bcm_l2_addr_t l2addr;

    L2_LOCK(unit);
   
    /* get the l2addr to send to callback function(s) */
    rv = bcm_caladan3_l2_addr_get(unit, mac, vid, &l2addr);

    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "WARM_BOOT_TODO: Bailing out L2 adress get on unit=%d for the delete until FDB is warmbooted in TMU:%s "),
                   unit, bcm_errmsg(rv)));
        rv = BCM_E_NONE;
    }

    /* Delete and call back if needed */
    if (BCM_SUCCESS(rv)) {
        if (delete == 1) {
            rv=bcm_caladan3_l2_addr_delete(unit, mac, vid);
            if (BCM_FAILURE(rv)) {
                LOG_WARN(BSL_LS_BCM_L2,
                         (BSL_META_U(unit,
                                     "L2 age unit=%d, error attempting to delete rv=%d\n"),
                          unit, rv));
            }
        }
        /* Callback to registered user routines */
        if (L2_CB_COUNT(unit)) {
            L2_CB_RUN(unit, &l2addr, BCM_L2_CALLBACK_REPORT);
        }
    }
    L2_UNLOCK(unit);
    return;
}


/*
 * Function:
 *     _bcm_caladan3_l2_age_timer_thread
 * Purpose:
 *     L2 age timer thread to run the microcode aging process
 *     at given intervals of time.
 *
 *     The BCM L2 age async callback routine _bcm_caladan3_l2_age_async_cb()
 *     is hooked to the main ILIB async callback  to
 *     get notification for aged mac entries.  This is later uninstall
 *     when thread exits.
 *
 *     This routine will perform the 'commit' of mac entries
 *     'removed' by the l2 age async callback routine
 *     _bcm_caladan3_l2_async_cb().
 * Parameters:
 *     unit - Device number
 * Returns:
 *     None
 */
STATIC void
_bcm_caladan3_l2_age_timer_thread(int unit)
{
    int          rv = BCM_E_UNAVAIL;
    int          cycles_done;
    uint64       cycle_age;
    uint64       cycle_left;
    uint64       cycle_total;
    sal_usecs_t  cycle_start;
    sal_usecs_t  cycle_end;
    uint64       interval_left;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "L2 age timer thread starting\n")));

    _l2_age[unit].thread_id = sal_thread_self();



    /* Main thread loop */
    while (_l2_age[unit].age_sec != 0) {
        LOG_VERBOSE(BSL_LS_BCM_L2,
                    (BSL_META_U(unit,
                                "L2 age timer interval start age=%d(seconds)\n"),
                     _l2_age[unit].age_sec));

        cycles_done = 0;
        COMPILER_64_ZERO(cycle_total);
        COMPILER_64_SET(cycle_age, 
                        COMPILER_64_HI(_l2_age[unit].cycle_usec), 
                        COMPILER_64_LO(_l2_age[unit].cycle_usec));

        while (!cycles_done) {

            /* Mark starting cycle time */
            cycle_start = sal_time_usecs();

            /* Run aging */
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
            case SOC_SBX_UCODE_TYPE_G3P1:
                if(!SOC_WARM_BOOT(unit))
                {
                    rv = _bcm_caladan3_g3p1_run_ager(unit,_bcm_caladan3_l2_async_cb,
                		SOC_SBX_STATE(unit)->l2_age_delete,_l2_age[unit].ager,
                		_l2_age[unit].entries_cycle, _l2_age[unit].age_range);

                    if (rv == BCM_E_EMPTY) {
/*                      All table has been processed */
                	cycles_done = 1;
                	rv = BCM_E_NONE;
                    }
                }
                break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
            default:
                rv = BCM_E_CONFIG;
            }

            if (BCM_FAILURE(rv)) {
                LOG_WARN(BSL_LS_BCM_L2,
                         (BSL_META_U(unit,
                                     "L2 age unit=%d, error during age run rv=%d\n"),
                          unit, rv));
                break;
            }

            /* Mark ending cycle time */
            cycle_end = sal_time_usecs();


            /* Calculate time left in a cycle */
            /* Set minimum delay if cycle took longer or bad time value */
            /* NOTE:
             * This works fine if we assume that the Ilib aging does not
             * take more than 71 minutes per run.  Otherwise, the calculation
             * of how much time is left would be off.
             *
             * The calculated run aging time is limited to the
             * max value supported by sal_time_usecs(), which
             * is 4294.967296 seconds (~71.5 minutes) before it
             * wraps around (see comments in 'time.h')
             */

            COMPILER_64_SET(cycle_left, 0,(cycle_end - cycle_start)); 
            if ((cycle_end < cycle_start) ||
                COMPILER_64_GT(cycle_left, cycle_age)) {
                COMPILER_64_SET(cycle_left,0, L2_AGE_CYCLE_MIN_USEC);
            } else {
                COMPILER_64_SET(cycle_left,
                                COMPILER_64_HI(cycle_age),
                                COMPILER_64_LO(cycle_age));
                COMPILER_64_SUB_32(cycle_left, (cycle_end - cycle_start));
            }
            
            /* Complete (or apply minimum delay) age cycle time */
            L2_AGE_TIMER_SLEEP(unit, cycle_left);

            COMPILER_64_ADD_64(cycle_total, cycle_age);
        }

        /* Check for exit */
        if (_l2_age[unit].age_sec == 0) {
            break;
        }


        /* Complete age interval time, if needed */
        COMPILER_64_SET(interval_left,0,_l2_age[unit].age_sec);
        COMPILER_64_UMUL_32(interval_left, SECOND_USEC);
        if (COMPILER_64_GT(interval_left, cycle_total)) {
            COMPILER_64_SUB_64(interval_left, cycle_total);
            L2_AGE_TIMER_SLEEP(unit, interval_left);
        }

        /* Increment ager step */
        L2_AGE_INCR(unit,_l2_age[unit].ager,_l2_age[unit].age_range);
  /*      LOG_CLI((BSL_META_U(unit,
                              "Setting time with  %d\n"),_l2_age[unit].ager)); */
        rv = _bcm_caladan3_l2_age_ager_set(unit, _l2_age[unit].ager);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "L2 age timer thread, unable to update ager: "
                                   "%d %s\n"), rv, bcm_errmsg(rv)));
            LOG_CLI((BSL_META_U(unit,
                                "Got Error %d\n"),rv));
            break;
        }
    }

    /* Uninstall L2 age async callback routine */
    _l2_age[unit].thread_id = NULL;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "L2 age timer thread exiting\n")));

    sal_thread_exit(0);
}


/*
 * Function:
 *     _bcm_caladan3_l2_age_timer_enable
 * Purpose:
 *     Enable L2 age timer:
 *     (1) Enable and set aging in microcode.
 *         The microcode will update the MAC 'age' field
 *         when a HIT occurs.
 *     (2) Start the L2 age timer thread.
 *         The thread will wake up at each intervals of time
 *         and remove old MAC entries.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_age_timer_enable(int unit, int age_seconds)
{
    int       cycles;
    uint32    res;
	uint32    ager;
    uint64    uuSec;
    uint64    res_u64=COMPILER_64_INIT(0,0);

    /*
     * Age information
     */
    cycles = _l2_age[unit].cycles_interval;
    _l2_age[unit].age_sec = age_seconds;


    /*
     * _l2_age[unit].cycle_usec = ((((uint64) age_seconds) * SECOND_USEC)
     *                            + (cycles - 1)) / cycles;
     */
    COMPILER_64_SET(uuSec, 0, age_seconds);
    COMPILER_64_UMUL_32(uuSec, SECOND_USEC);
    COMPILER_64_ADD_32(uuSec, (cycles - 1));
    if (COMPILER_64_HI(uuSec) == 0) {
        res = COMPILER_64_LO(uuSec) / cycles;
        COMPILER_64_SET(res_u64, 0, res);
    } else {
        COMPILER_64_ZERO(uuSec);
        COMPILER_64_SET(uuSec, 0, age_seconds);
        COMPILER_64_UMUL_32(uuSec, MILLISECOND_USEC);
        if (soc_sbx_div64(uuSec, cycles, &res) == SOC_E_PARAM) {
            return SOC_E_INTERNAL;
        }
        
        COMPILER_64_SET(res_u64, 0, res);
        COMPILER_64_UMUL_32(res_u64, MILLISECOND_USEC);
        
        COMPILER_64_ADD_32(res_u64, ((cycles - 1)/cycles));
    }
    
    _l2_age[unit].cycle_usec = res_u64;
    

    /* If thread running, just notify with updated interval and return */
    if (_l2_age[unit].thread_id != NULL) {
        L2_AGE_TIMER_WAKE(unit);
        return BCM_E_NONE;
    }

    /*
     * Enable age updates by microcode
     */
	BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_ager_get(unit, &ager));
	
    BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_ager_set(unit, ager));

    /* Create L2 age timer thread */
    sal_snprintf(_l2_age[unit].thread_name,
                 sizeof(_l2_age[unit].thread_name),
                 "bcmL2Ager.%d", unit);
    if (sal_thread_create(_l2_age[unit].thread_name,
                          SAL_THREAD_STKSZ,
                          L2_AGE_TIMER_THREAD_PRIO,
                          (void (*)(void*))_bcm_caladan3_l2_age_timer_thread,
                          INT_TO_PTR(unit)) == SAL_THREAD_ERROR) {
        _l2_age[unit].thread_name[0] ='\0';
        _l2_age[unit].age_sec        = 0;

        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_age_timer_disable
 * Purpose:
 *     Disable L2 age timer:
 *     (1) Stop the L2 age timer thread.
 *     (2) Disable aging in microcode.
 * Parameters:
 *     unit        - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_caladan3_l2_age_timer_disable(int unit)
{
	uint32    ager;
	
    /* Stop L2 age timer thread */
    BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_timer_thread_stop(unit));

	BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_ager_get(unit, &ager));

    /* Reset age stamp */
    BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_age_ager_set(unit, ager));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_l2_age_timer_get
 * Purpose:
 *     Returns the current age timer value.
 *     The value is 0 if aging is not enabled.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Place to store returned age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_caladan3_l2_age_timer_get(int unit, int *age_seconds)
{
    int       rv = BCM_E_NONE;

    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(age_seconds);

    *age_seconds = _l2_age[unit].age_sec;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "age=%d(seconds), age-cycles=%d, rv=%d\n"),
                 *age_seconds, _l2_age[unit].cycles_interval, rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_age_timer_set
 * Purpose:
 *     Set the age timer for all blocks.
 *     Setting the value to 0 disables the age timer.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_caladan3_l2_age_timer_set(int unit, int age_seconds)
{
    int       rv = BCM_E_NONE;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    if ((age_seconds < 0) || (age_seconds > L2_AGE_TIMER_MAX_SECS)) {
        return BCM_E_PARAM;
    }

    L2_LOCK(unit);
    if (age_seconds == 0) {
        rv = _bcm_caladan3_l2_age_timer_disable(unit);
    } else {
        rv = _bcm_caladan3_l2_age_timer_disable(unit);
        if (rv != BCM_E_NONE)
        {
            L2_UNLOCK(unit);

            LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "bcm_l2_age_timer_set: unit=%d, age=%d(seconds), age-cycles=%d, "
                            "cycle_usec=0x%08x%08x rv=%d\n"),
                 unit, age_seconds, _l2_age[unit].cycles_interval,
                 COMPILER_64_HI(_l2_age[unit].cycle_usec),
                 COMPILER_64_LO(_l2_age[unit].cycle_usec),
                 rv));

            return rv;
        }

        rv = _bcm_caladan3_l2_age_timer_enable(unit, age_seconds);
    }
    L2_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "age=%d(seconds), age-cycles=%d, "
                             "cycle_usec=0x%08x%08x rv=%d\n"),
                 age_seconds, _l2_age[unit].cycles_interval,
                 COMPILER_64_HI(_l2_age[unit].cycle_usec),
                 COMPILER_64_LO(_l2_age[unit].cycle_usec),
                 rv));

    return rv;
}



/*
 * Displays Smac entries.
 *
 * Note: To start from first entry, set mac and vlan to zeros.
 *       To display to the end of list, set count to <= 0.
 */
STATIC int
_bcm_caladan3_smac_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid, int max_count)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_smac_dump(unit, mac, vid, max_count);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


STATIC int
_bcm_caladan3_dmac_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid, int max_count)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_dmac_dump(unit, mac, vid, max_count);
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_caladan3_l2_addr_dump
 * Purpose:
 *     Displays L2 Address Management Smac and Dmac table entries.
 *     If MAC and VLAN id are zeros, routine displays entries starting
 *     at the first entry in table.  Otherwise, routine displays
 *     entries starting at entry matching given MAC and VLAN.
 * Parameters:
 *     unit      - Device number
 *     mac       - Starting MAC address key to display
 *     vid       - Starting VLAN id key to display
 *     max_count - Max number of entries to display per table
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     To start from first entry, set mac and vlan to zeros.
 *     To display to the end of list, set count to <= 0.
 */
int
_bcm_caladan3_l2_addr_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid,
                         int max_count)
{
    UNIT_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(_bcm_caladan3_smac_dump(unit, mac, vid, max_count));
    L2_DUMP("\n");
    BCM_IF_ERROR_RETURN(_bcm_caladan3_dmac_dump(unit, mac, vid, max_count));

    return BCM_E_NONE;
}

/*
 * Dump L2 State 
 */
void bcm_sbx_caladan3_l2_state_dump(int unit)
{
    LOG_INFO(BSL_LS_BCM_L2,
             (BSL_META_U(unit,
                         "Age second %d \n"),
              _l2_age[unit].age_sec));
}
/*
 * Function:
 *     _bcm_caladan3_init_key
 * Purpose:
 *     Packs the relevant fields from bcm_l2_addr_t for hashing
 * Parameters:
 *      addr  - src address to pack
 *      key   - storage location for packed key
 * Returns:
 *    n/a
 * Notes:
 */
STATIC void
_bcm_caladan3_init_key(bcm_l2_addr_t *addr, _l2_stat_key_t key)
{
#define L2_KEY_PACK(dest, src, size) \
   sal_memcpy(dest, src, size);       \
   dest += size;

    uint8 *loc = key;

    /* pack the relevant l2 fields into the key */ 
    assert (sizeof(_l2_stat_key_t) >= 
            (sizeof(addr->mac) + sizeof(addr->port)));

    L2_KEY_PACK(loc, addr->mac, sizeof(addr->mac));
    L2_KEY_PACK(loc, &addr->port, sizeof(addr->port));
}


/*
 * Function:
 *     _bcm_caladan3_l2_stat_state_init
 * Purpose:
 *     initialize the statitistics state information
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_caladan3_l2_stat_state_init(int unit)
{
    int rv;
    shr_htb_hash_table_t  fieldTable;

    rv = shr_htb_create(&fieldTable, L2_MAX_STAT_ENTRIES, 
                        sizeof(_l2_stat_key_t), "l2 field entry hash");
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to create hash table: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    L2_STAT_STATE(unit) = sal_alloc(sizeof(_l2_stat_state_t), "l2_stat_state");
    if (L2_STAT_STATE(unit) == NULL) {
        rv = shr_htb_destroy(&fieldTable, NULL);
        return BCM_E_MEMORY;
    }
    
    sal_memset(L2_STAT_STATE(unit), 0, sizeof(_l2_stat_state_t));

    L2_STAT_STATE(unit)->fieldTable = fieldTable;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_stat_state_destroy
 * Purpose:
 *     free resources associated with statitistics state information
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_caladan3_l2_stat_state_destroy(int unit) 
{
    int rv;
    
    rv = shr_htb_destroy(&L2_STAT_STATE(unit)->fieldTable, NULL);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to destroy hash table: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
    }
    
    sal_free(L2_STAT_STATE(unit));
    
    L2_STAT_STATE(unit) = NULL;
    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_l2_stat_group_init
 * Purpose:
 *     Create the two groups necessary for src and destination mac
 *     statitisics
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_caladan3_l2_stat_group_init(int unit)
{
    int rv;
    bcm_field_qset_t qset;
    int i;
    int qualifiers[L2_STAT_NUM_QUALIFIERS] = {
        bcmFieldQualifyStageIngressQoS,
        bcmFieldQualifyInPorts,
        bcmFieldQualifySrcMac
    };

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Enter\n")));

    BCM_FIELD_QSET_INIT(qset);
    for (i=0; i < L2_STAT_NUM_QUALIFIERS; i++) {
        BCM_FIELD_QSET_ADD(qset, qualifiers[i]);
    }

    rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, 
                                &(L2_STAT_STATE(unit)->smacGroup));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to create smac group: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    BCM_FIELD_QSET_INIT(qset);
    qualifiers[L2_STAT_NUM_QUALIFIERS - 1] = bcmFieldQualifyDstMac;
    for (i=0; i < L2_STAT_NUM_QUALIFIERS; i++) {
        BCM_FIELD_QSET_ADD(qset, qualifiers[i]);
    }

    rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, 
                                &(L2_STAT_STATE(unit)->dmacGroup));

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to create dmac group: %d %s\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    L2_STAT_STATE(unit)->groupsValid = TRUE;
    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Created smac group=%d,  dmac group=%d\n"),
                 L2_STAT_STATE(unit)->smacGroup,
                 L2_STAT_STATE(unit)->dmacGroup));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_stat_group_destroy
 * Purpose:
 *     free resources associated with the field groups for statistics
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC void 
_bcm_caladan3_l2_stat_group_destroy(int unit)
{
    int rv;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Enter - destroying field groups\n")));

    rv = bcm_field_group_destroy(unit, L2_STAT_STATE(unit)->smacGroup);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to destroy smac group, keep trying: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_group_destroy(unit, L2_STAT_STATE(unit)->dmacGroup);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to destroy dmac group, keep trying: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
    }

    L2_STAT_STATE(unit)->smacGroup = L2_STAT_STATE(unit)->dmacGroup = ~0;

    L2_STAT_STATE(unit)->groupsValid = FALSE;
}


/*
 * Function:
 *     _bcm_caladan3_l2_stat_field_entry_create
 * Purpose:
 *     Create the field entries used for statistics gathering
 * Parameters:
 *     unit    - bcm device number
 *     l2_addr - l2 address to track
 *     fields  - fields created for statistics information
 * Returns:
 *    BCM_E_*
 * Notes:
 */
int 
_bcm_caladan3_l2_stat_field_entry_create(int unit, bcm_l2_addr_t *l2_addr,
                                       _l2_stat_fields_t *fields)
{
    int rv;
    int idx;
    bcm_field_group_t *pGroup;
    bcm_field_entry_t *pEntry;
    bcm_pbmp_t pbmp;
    bcm_mac_t macMask;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Enter port=%d mac=" L2_6B_MAC_FMT "\n"),
                 l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac)));

    macMask[0] = macMask[1] = macMask[2] = 
        macMask[3] = macMask[4] = macMask[5] = 0xFF;

    pGroup = &L2_STAT_STATE(unit)->smacGroup;
    pEntry = &fields->smac;

    for (idx = 0; idx<2; idx++) {
        rv = bcm_field_entry_create(unit, *pGroup, pEntry);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to create entry idx=%d group=%d: %d %s"),
                       idx, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_ADD(pbmp, l2_addr->port);
    
        rv = bcm_field_qualify_InPorts(unit, *pEntry, pbmp, PBMP_ALL(unit));
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to add qualifiers to entry=%d, group=%d idx=%d"
                                   ": %d %s\n"),
                       *pEntry, *pGroup, idx, rv, bcm_errmsg(rv)));
            return rv;
        }
        
        if (idx == 0) {
            rv = bcm_field_qualify_SrcMac(unit, *pEntry, l2_addr->mac, 
                                          macMask);
        } else {
            rv = bcm_field_qualify_DstMac(unit, *pEntry, l2_addr->mac, 
                                          macMask);
        }
        
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to add %s qualifier: %d %s\n"),
                       (idx==0) ? "SrcMac" : "DstMac", rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_entry_install(unit, *pEntry);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to install entry %d into group %d: %d %s\n"),
                       *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_entry_stat_attach(unit, *pEntry, 0);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to create counter on Entry %d, group %d"
                                   ": %d %s\n"),
                       *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_action_add(unit, *pEntry, bcmFieldActionUpdateCounter,
                                  BCM_FIELD_COUNTER_MODE_BYTES_PACKETS, 0);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to add update counter action to entry %d, "
                                   "group %d: %d %s\n"),
                       *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        pGroup = &L2_STAT_STATE(unit)->dmacGroup;
        pEntry = &fields->dmac;
    }

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Created smac entry %d for group %d\n"),
                 fields->smac, L2_STAT_STATE(unit)->smacGroup));
    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Created dmac entry %d for group %d\n"),
                 fields->dmac, L2_STAT_STATE(unit)->dmacGroup));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l2_stat_field_entry_destroy
 * Purpose:
 *     cleanup state associated with a single l2 statistic
 * Parameters:
 *      unit - bcm device number
 *      fields - fields to destroy
 * Returns:
 *    BCM_E_*
 * Notes:
 */
int
_bcm_caladan3_l2_stat_field_entry_destroy(int unit, _l2_stat_fields_t *fields)
{
    int rv;

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Enter\n")));
    
    rv = bcm_field_entry_remove(unit, fields->smac);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to remove SMAC field match, keep trying:  %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }
       
    rv = bcm_field_entry_destroy(unit, fields->smac);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to destroy SMAC field match, keep trying:  %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_entry_remove(unit, fields->dmac);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to remove DMAC field match, keep trying:  %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_entry_destroy(unit, fields->dmac);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to destroy DMAC field match, keep trying:  %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_caladan3_l2_stat_get
 * Description:
 *      Get the specified L2 statistic from the chip. MAC entry must have 
 *      previously been enabled for stats collection.
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - (OUT) 64-bit counter value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 */
int bcm_caladan3_l2_stat_get(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat, 
                           uint64 *val)
{
    int rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    if (L2_STAT_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Must enable a stat before retrieving entry\n")));
        return BCM_E_INIT;
    }

    _bcm_caladan3_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to find l2 addr: %d %s\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    switch (stat) {
    case bcmL2StatSourcePackets:
        rv =  bcm_field_stat_get(unit, fields->smac, 0, val);
        break;
    case bcmL2StatSourceBytes:
        rv =  bcm_field_stat_get(unit, fields->smac, 1, val);
        break;
    case bcmL2StatDestPackets:
        rv =  bcm_field_stat_get(unit, fields->dmac, 0, val);
        break;
    case bcmL2StatDestBytes:
        rv =  bcm_field_stat_get(unit, fields->dmac, 1, val);
        break;
    case bcmL2StatDropPackets: /* fall thru intentional */
    case bcmL2StatDropBytes: /* fall thru intentional */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "stat %d not supported\n"),
                   stat));
        rv = BCM_E_PARAM;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "unrecognized stat:%d\n"),
                   stat));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_stat_get32
 * Description:
 *      Get the specified L2 statistic from the chip
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - (OUT) 32-bit counter value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      Same as bcm_caladan3_l2_stat_get, except converts result to 32-bit.
 */
int bcm_caladan3_l2_stat_get32(int unit, bcm_l2_addr_t *l2_addr, 
                             bcm_l2_stat_t stat, uint32 *val)
{
    int rv;
    uint64 tmp;

    rv = bcm_caladan3_l2_stat_get(unit, l2_addr, stat, &tmp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to get stat %d: %d %s\n"),
                   stat, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    if (COMPILER_64_HI(tmp) > 0) {
        LOG_WARN(BSL_LS_BCM_L2,
                 (BSL_META_U(unit,
                             "Data loss when 64bit int cast down to 32 bit for "
                              "stat %d\n"), stat));
    }
    *val = COMPILER_64_LO(tmp);
    
    return rv;
}    

/*
 * Function:
 *      bcm_caladan3_l2_stat_set
 * Description:
 *      Set the specified L2 statistic to the indicated value
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - 64-bit value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      'val' must be zero (0).
 */
int bcm_caladan3_l2_stat_set(int unit, bcm_l2_addr_t *l2_addr, 
                           bcm_l2_stat_t stat, uint64 val)
{
    int rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    if (L2_STAT_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Must enable a stat before retrieving entry\n")));
        return BCM_E_INIT;
    }

    _bcm_caladan3_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed to find l2 addr: %d %s\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    switch (stat) {
    case bcmL2StatSourcePackets:
        rv =  bcm_field_stat_set(unit, fields->smac, 0, val);
        break;
    case bcmL2StatSourceBytes:
        rv =  bcm_field_stat_set(unit, fields->smac, 1, val);
        break;
    case bcmL2StatDestPackets:
        rv =  bcm_field_stat_set(unit, fields->dmac, 0, val);
        break;
    case bcmL2StatDestBytes:
        rv =  bcm_field_stat_set(unit, fields->dmac, 1, val);
        break;
    case bcmL2StatDropPackets: /* fall thru intentional */
    case bcmL2StatDropBytes: /* fall thru intentional */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "stat %d not supported\n"),
                   stat));
        rv = BCM_E_PARAM;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "unrecognized stat:%d\n"),
                   stat));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_stat_set32
 * Description:
 *      Set the specified L2 statistic to the indicated value
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - 32-bit value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      Same as bcm__l2_stat_set, except accepts 32-bit value. 
 *      'val' must be zero (0).
 */
int bcm_caladan3_l2_stat_set32(int unit, bcm_l2_addr_t *l2_addr, 
                             bcm_l2_stat_t stat, uint32 val)
{   uint64 val64;
    COMPILER_64_SET(val64, 0, val);
    return bcm_caladan3_l2_stat_set(unit, l2_addr, stat, val64);
}

/*
 * Function:
 *      bcm_caladan3_l2_stat_enable_set
 * Description:
 *      Enable/Disable statistics on the indicated L2 entry.
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      enable  - TRUE/FALSE indicator indicating action to enable/disable.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Either the L2 or the stats module is not yet initialized
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_XXX       - Failure, other
 */
int bcm_caladan3_l2_stat_enable_set(int unit, bcm_l2_addr_t *l2_addr, int enable)
{
    int                rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Enter enable=%d port=%d mac=" L2_6B_MAC_FMT "\n"),
                 enable, l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac)));

    if (L2_STAT_STATE(unit) == NULL) {
        rv = _bcm_caladan3_l2_stat_state_init(unit);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to initialize l2 statistics: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    _bcm_caladan3_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "_find (%d," L2_6B_MAC_FMT ") rv=%d\n"),
               l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac), rv));

    if (enable) {
        _l2_stat_fields_t tmpFields;

        if (BCM_SUCCESS(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Attempted to enable existing entry\n")));
            return BCM_E_PARAM;
        }
        
        if (rv != BCM_E_NOT_FOUND) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Attempted to enable non-existent entry\n")));
            return rv;
        }

        if (!L2_STAT_STATE(unit)->groupsValid) {
            rv = _bcm_caladan3_l2_stat_group_init(unit);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "Faield to initialize group: %d %s\n"), 
                           rv, bcm_errmsg(rv)));
                return rv;
            }
        }
        
        rv = _bcm_caladan3_l2_stat_field_entry_create(unit, l2_addr, &tmpFields);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to create field match entries: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }
        
        fields = sal_alloc(sizeof(_l2_stat_fields_t), "l2_stats fields");
        if (fields == NULL) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to alloc stat field memory\n")));
            return BCM_E_MEMORY;
        }
        
        sal_memcpy(fields, &tmpFields, sizeof(_l2_stat_fields_t));

        rv = shr_htb_insert(L2_STAT_STATE(unit)->fieldTable, key,
                            (shr_htb_data_t)fields);
        if (BCM_FAILURE(rv)) {
            sal_free(fields);
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to insert fields into hash: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }
        
        L2_STAT_STATE(unit)->numStatsInUse++;
    } else {   /* if (enable) */

        if (rv == BCM_E_NOT_FOUND) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Attempted to disable non-existent entry\n")));
            return rv;
        }
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Unexpected error: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }

        if (!L2_STAT_STATE(unit)->groupsValid) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Invalid state - found fields, but groups not "
                                   "valid\n")));
            return BCM_E_INTERNAL;  /* how did this happen? */
        }
        
        rv = _bcm_caladan3_l2_stat_field_entry_destroy(unit, fields);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to destroy field entries, keep trying: "
                                   "%d %s\n"), rv, bcm_errmsg(rv)));
        }
        
        rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                          (shr_htb_data_t *)&fields,
                          1 /* == remove */);

        sal_free(fields);
        L2_STAT_STATE(unit)->numStatsInUse--;
    }

    if (L2_STAT_STATE(unit)->numStatsInUse == 0 &&
        L2_STAT_STATE(unit)->groupsValid) 
    {
        _bcm_caladan3_l2_stat_group_destroy(unit);
        _bcm_caladan3_l2_stat_state_destroy(unit);
    }

    return rv;
}


/*
 * Function:
 *      _bcm_caladan3_l2_egress_entry_add
 * Description:
 *      add an l2 egress entry with the given config at specified index
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - index at which to add the l2 egress entry 
 *      egr         - ptr to l2 egress config
 * Returns:
 *      BCM_E_NONE  - if successfully added entry
 *      BCM_E_XXX   - otherwise
 */
int
_bcm_caladan3_l2_egress_entry_add(int unit, bcm_if_t encap_id, 
                                bcm_l2_egress_t *egr)
{
    int rv = BCM_E_INTERNAL;
    
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_egress_hw_entry_add(unit, encap_id, egr);
        if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_DEST_PORT)) {
            L2_EGR_DEST_PORT_SET(unit, 
                                 SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id),
                                 egr->dest_port);
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG; 
    }

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_egress_entry_delete
 * Description:
 *      delete the entry pointed to by the given encap_id
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to delete
 * Returns:
 *      BCM_E_NONE  - if successfully deleted the specified entry
 *      BCM_E_XXX   - otherwise
 */
int
_bcm_caladan3_l2_egress_entry_delete(int unit, bcm_if_t encap_id)
{
    int elem_state;
    int rv = BCM_E_INTERNAL;

    /* perform common list management, and param check */
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, 
                                            SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
    if (elem_state != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }
    
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_egress_hw_entry_delete(unit, encap_id);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    /* finalize with common list managment */
    if (BCM_SUCCESS(rv)) {
        uint32 localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id);

        /* Free the element only if it is out of the reserved range */
        if (!L2_ID_RESERVED(unit, localId)) {
            rv = shr_idxres_list_free(_l2_egress[unit].idList.idMgr, localId);
        } else {
            LOG_VERBOSE(BSL_LS_BCM_L2,
                        (BSL_META_U(unit,
                                    "encap 0x%08x found in reserved range, not freeing\n"),
                         encap_id));
        }

        L2_EGR_DEST_PORT_SET(unit, localId, L2_EGR_DEST_PORT_INVALID);
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_egress_entry_delete_all
 * Description:
 *      delete all l2 egress entries on a give unit
 * Parameters:
 *      unit        - device unit number.
 * Returns:
 *      BCM_E_NONE  - if successfully deleted all existing entries
 *      BCM_E_XXX   - otherwise
 */
int 
_bcm_caladan3_l2_egress_entry_delete_all(int unit)
{
    int rv = BCM_E_NONE;
    int idx;
    int elem_state;

    for (idx=0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_caladan3_l2_egress_entry_delete(unit, 
                                        SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx));
        }
        if (rv != BCM_E_NONE) {
            break;
        }
    }

    return rv;
}


int
_bcm_caladan3_l2_egress_entry_get(int unit, bcm_if_t encap_id, 
                                bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_INTERNAL;
    int                         elem_state;
    uint32                      dest_port;

    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);
    PARAM_NULL_CHECK(egr);
    
    elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, 
                                   SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
    dest_port = L2_EGR_DEST_PORT_INVALID;

    if (elem_state != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_egress_hw_entry_get(unit, encap_id, egr);
        if (rv == BCM_E_NONE) {
            L2_EGR_DEST_PORT_GET(unit, 
                                 SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id),
                                 dest_port);
            if (dest_port != L2_EGR_DEST_PORT_INVALID) {
                egr->dest_port = dest_port;
                egr->flags |= BCM_L2_EGRESS_DEST_PORT;
            }
        }
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_egress_entries_compare
 * Description:
 *      Compare if two l2 egress entries are same
 * Parameters:
 *      egr1 - Ptr of first entry config
 *      egr2 - Ptr of second entry config
 * Returns:
 *      0           - if both entries are same
 *      non-zero    - otherwise
 * Notes:
 *      Ignores BCM_L2_EGRESS_WITH_ID & BCM_L2_EGRESS_REPLACE config flags
 *      and encap_ids
 */
int
_bcm_caladan3_l2_egress_entries_compare(bcm_l2_egress_t *pegr1, 
                                      bcm_l2_egress_t *pegr2)
{
    bcm_l2_egress_t egr1, egr2;
    uint32 config_mask = ~(BCM_L2_EGRESS_WITH_ID | BCM_L2_EGRESS_REPLACE);

    if ((!pegr1) || (!pegr2)) {
        return BCM_E_PARAM;
    }

    sal_memcpy(&egr1, pegr1, sizeof(bcm_l2_egress_t));
    sal_memcpy(&egr2, pegr2, sizeof(bcm_l2_egress_t));

    /* ignore WITH_ID & REPLACE flags */
    egr1.flags &= config_mask;
    egr2.flags &= config_mask;

    /* ignore the encap_ids */
    egr1.encap_id = egr2.encap_id = 0;   

    return sal_memcmp(&egr1, &egr2, sizeof(bcm_l2_egress_t));
}


/*
 * Function:
 *      bcm_caladan3_l2_egress_create
 * Description:
 *      Create an encapsulation index with the give l2 egress config
 * Parameters:
 *      unit        - device unit number.
 *      egr         - Ptr of entry config
 * Returns:
 *      BCM_E_NONE      - if entry successfully created
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      If call is successful, created encapsulation index is stored in 
 *      egr->encap_id
 */
int
bcm_caladan3_l2_egress_create(int unit, bcm_l2_egress_t *egr)
{
    int         rv = BCM_E_NONE;
    int         elem_state;
    bcm_if_t    encap_id = 0;
    uint32    localId;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    
    /* if BCM_L2_EGRESS_REPLACE is set, then BCM_L2_EGRESS_WITH_ID 
    should also be set */
    if ((egr->flags & BCM_L2_EGRESS_REPLACE) && 
        !(egr->flags & BCM_L2_EGRESS_WITH_ID)) {
        return BCM_E_PARAM; 
    }

    /* setting both is invalid */
    if ((egr->flags & BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE) &&
        (egr->flags & BCM_L2_EGRESS_DEST_MAC_REPLACE)) {
        return BCM_E_PARAM;
    }
    
    localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(egr->encap_id);

    L2_LOCK(unit);
    if (egr->flags & BCM_L2_EGRESS_WITH_ID) {
        if (L2_EGRESS_VALID_ENCAP_ID(egr->encap_id)) {
            elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr,
                                SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(egr->encap_id));
            if (egr->flags & BCM_L2_EGRESS_REPLACE) {
                /* trying to replace, check if it exists */
                if (elem_state != BCM_E_EXISTS) {
                    rv = BCM_E_PARAM;
                } else {
                    /* delete the existing entry */
                    rv = _bcm_caladan3_l2_egress_entry_delete(unit, 
                                                            egr->encap_id);
                    if (rv == BCM_E_NONE) {
                        rv = _bcm_caladan3_l2_egress_entry_add(unit, 
                                                             egr->encap_id, 
                                                             egr);
                    }

                    /* reserve the id only if it's not in the application
                     * reserved range 
                     */
                    if (rv == BCM_E_NONE && !L2_ID_RESERVED(unit, localId)) {
                        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                                     localId, localId);
                    } else {
                        LOG_VERBOSE(BSL_LS_BCM_L2,
                                    (BSL_META_U(unit,
                                                "encap 0x%08x found in reserved range,"
                                                 " not re-allocated, flags=0x%x - OK\n"),
                                     egr->encap_id, egr->flags));
                    }
                    
                }
            } else {

                if (L2_ID_RESERVED(unit, localId)) {
                    LOG_VERBOSE(BSL_LS_BCM_L2,
                                (BSL_META_U(unit,
                                            "found encap 0x%x in reserved range, state=%d"
                                             " %s; OK\n"),  
                                 egr->encap_id, elem_state, 
                                 bcm_errmsg(elem_state)));
                    elem_state = BCM_E_NONE;
                } 

                /* User specified encap id & no replace. error if it exists */
                if (elem_state == BCM_E_EXISTS) {
                    rv = BCM_E_EXISTS;
                } else {
                    rv = _bcm_caladan3_l2_egress_entry_add(unit, egr->encap_id, 
                                                         egr);

                    /* don't reserve it if it's in the application 
                     * reserved space 
                     */
                    if (rv == BCM_E_NONE && !(L2_ID_RESERVED(unit, localId))) {
                        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                                     localId, localId);
                    } else {
                        LOG_VERBOSE(BSL_LS_BCM_L2,
                                    (BSL_META_U(unit,
                                                "encap 0x%08x found in reserved range,"
                                                 " not reallocated - flags=0x%x\n"), 
                                     egr->encap_id, egr->flags));

                    }
                }
            }
        } else {
            rv = BCM_E_PARAM;
        }
    } else {
        /* allocate an encap id and create the entry */
        rv = shr_idxres_list_alloc(_l2_egress[unit].idList.idMgr, 
                                   (uint32 *)&encap_id);
        if (rv == BCM_E_NONE) {
            encap_id = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(encap_id);
            if (L2_EGRESS_VALID_ENCAP_ID(encap_id)) {
                rv = _bcm_caladan3_l2_egress_entry_add(unit, encap_id, egr);
                if (rv != BCM_E_NONE) {
                    shr_idxres_list_free(_l2_egress[unit].idList.idMgr, 
                                  SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
                }
            } else {
                rv = BCM_E_PARAM;
            }
        }
    }
    L2_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_egress_destroy
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to destroy
 * Returns:
 *      BCM_E_NONE      - if entry successfully deleted
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_caladan3_l2_egress_destroy(int unit, bcm_if_t encap_id)
{
    int rv = BCM_E_NONE;
    
    UNIT_INIT_CHECK(unit);
    
    L2_LOCK(unit);
    rv = _bcm_caladan3_l2_egress_entry_delete(unit, encap_id);
    L2_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_egress_get
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to retrieve config for
 *      egr         - Ptr of where to store the l2 egress config
 * Returns:
 *      BCM_E_NONE      - if entry found & able to read the entry successfully
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_caladan3_l2_egress_get(int unit, bcm_if_t encap_id, bcm_l2_egress_t *egr)
{
    int         rv = BCM_E_NONE;
    int         elem_state;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    L2_LOCK(unit);
    if (rv == BCM_E_NONE) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr,
                                  SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_caladan3_l2_egress_entry_get(unit, encap_id, egr);
        } else {
            rv = BCM_E_NOT_FOUND;
        }
    }
    L2_UNLOCK(unit);
    
    return rv;
}

/*
 * Function:
 *      bcm_l2_egress_find
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      egr         - configuration of l2 egress to look for
 *      pencap_id   - Ptr of where to store the encap_id
 * Returns:
 *      BCM_E_NONE      - if entry found.
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_caladan3_l2_egress_find(int unit, bcm_l2_egress_t *egr, bcm_if_t *pencap_id)
{
    int rv = BCM_E_NONE;
    int elem_state;
    int idx;
    bcm_l2_egress_t temp_egr;
    
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    PARAM_NULL_CHECK(pencap_id);

    L2_LOCK(unit);
    for (idx = 0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_caladan3_l2_egress_entry_get(unit, 
                                                 SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx),
                                                 &temp_egr);
            if (rv != BCM_E_NONE) {
                break;
            }
            if (!_bcm_caladan3_l2_egress_entries_compare(egr, &temp_egr)) {
                /* found the entry */
                *pencap_id = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx);
                break;
            }
        }
    }
    if (idx > _l2_egress[unit].encap_id_max) {
        rv = BCM_E_NOT_FOUND;
    }
    L2_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_l2_egress_traverse
 * Description:
 *      Traverse the list of existing l2 egress entries and call user provided
 *      callback with user_data
 * Parameters:
 *      unit        - device unit number.
 *      trav_fn     - user provided callback fn
 *      user_data   - user cookie
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      Stops if the callback fn returns anything other than BCM_E_NONE
 */
int bcm_caladan3_l2_egress_traverse(int unit, bcm_l2_egress_traverse_cb trav_fn, 
                           void *user_data)
{
    int                 rv = BCM_E_NONE;
    int                 idx;
    int                 elem_state;
    bcm_l2_egress_t     egr;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(trav_fn);

    L2_LOCK(unit);
    for (idx=0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_caladan3_l2_egress_entry_get(unit, 
                                                 SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx),
                                                 &egr);
            if (rv != BCM_E_NONE) {
                break;
            }
            rv = (*trav_fn)(unit, &egr, user_data);
            if (rv != BCM_E_NONE) {
                break;
            }
        }
    }
    L2_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     _bcm_l2_addr_update_dest
 * Purpose:
 *     Changes FTE of existing DMAC entry by specifying a new qidunion (basequeue).
 *     Code below will allocate new FTE  and point this l2_addr to it.
 * Parameters:
 *     unit   - Device number
 *     l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     This is an internal function and should not be used. Only works on Unicast.
 *     Assumes that caller will de-allocate the FTE which is allocated here
 */
int
_bcm_caladan3_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion)
{
    int rv = BCM_E_UNAVAIL;
    
    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);

    /* Check for valid flags */
    if (l2addr->flags & L2_FLAGS_UNSUPPORTED) {
        return BCM_E_PARAM;
    }

    L2_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_addr_update_dest(unit, l2addr, qidunion);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    L2_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_egress_init
 * Description:
 *      Initialize the L2 egress for the specified unit
 * Parameters:
 *      unit    - device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      Re-init clears all existing l2 egress entries.
 */

int
_bcm_caladan3_l2_egress_init(unit)
{
    int             rv = BCM_E_NONE;
    int             l2_egr_num;

    /* number of supported l2 egr objs */
    l2_egr_num = SBX_MAX_L2_EGRESS_OHI;

    UNIT_INIT_CHECK(unit);
    L2_LOCK(unit);

    /* if re-init clear all existing entries */
    if ( (_l2_egress[unit].init_done) && (!SOC_WARM_BOOT(unit)) ){
        if ((rv = _bcm_caladan3_l2_egress_entry_delete_all(unit)) != BCM_E_NONE){
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META("Re-init on failed (%s) \n"),
                       bcm_errmsg(rv)));
        }
    } else if (!SOC_WARM_BOOT(unit)){
        /* first time init */
        _l2_egress[unit].idList.reservedHigh = L2_INVALID_EGRESS_ID;
        _l2_egress[unit].idList.reservedLow  = L2_INVALID_EGRESS_ID;
        
    }
        _l2_egress[unit].dest_ports = sal_alloc((sizeof(uint32) * 
                                                 l2_egr_num),
                                                "L2 egr Dest Port");
        if (_l2_egress[unit].dest_ports == NULL) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META("Allocating L2 Egress dest port storage "
                       "failed\n")));
            rv = BCM_E_MEMORY;
            return rv;
        }

        sal_memset(_l2_egress[unit].dest_ports, 0xff, 
                   sizeof(uint32) * l2_egr_num);


    rv = shr_idxres_list_create(& _l2_egress[unit].idList.idMgr, 0, 
                                (l2_egr_num - 1), 0, (l2_egr_num - 1), 
                                "l2 egress entries");
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META("Failed to create id manager:%s\n"),
                   bcm_errmsg(rv)));
    }

    
    if (rv == BCM_E_NONE) {
        _l2_egress[unit].init_done = 1;
        _l2_egress[unit].encap_id_max = (l2_egr_num -1);
    } else {

        /* free resources */
        rv = shr_idxres_list_destroy(_l2_egress[unit].idList.idMgr);
        if (_l2_egress[unit].dest_ports) {
            sal_free(_l2_egress[unit].dest_ports);
        }
    }

    L2_UNLOCK(unit);
    return rv;
}

/*
 *   Function
 *      bcm_caladan3_l2_egress_range_reserve
 *   Purpose
 *     Reserve a range of L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int
bcm_caladan3_l2_egress_range_reserve(int unit, int highOrLow, uint32 val)
{

    int rv = BCM_E_NONE;
    int clearIt = 0, reserveIt = 0;
    uint32 first, last;

    UNIT_INIT_CHECK(unit);
    L2_EGRESS_VALID_ENCAP_ID_CHECK(val);


    L2_LOCK(unit);
    first = _l2_egress[unit].idList.reservedLow;
    last = _l2_egress[unit].idList.reservedHigh;

    /* Zero for any value, high or low, will clear the known range */
    if (val == 0) {
        clearIt = 1;
        _l2_egress[unit].idList.reservedLow = L2_INVALID_EGRESS_ID;
        _l2_egress[unit].idList.reservedHigh = L2_INVALID_EGRESS_ID;
    } else {
        val = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(val);

        if (highOrLow) {
            _l2_egress[unit].idList.reservedHigh = val; 
        } else {
            _l2_egress[unit].idList.reservedLow = val;
        }

        if ((_l2_egress[unit].idList.reservedHigh != L2_INVALID_EGRESS_ID) && 
            (_l2_egress[unit].idList.reservedLow != L2_INVALID_EGRESS_ID)) 
        {
            if (_l2_egress[unit].idList.reservedHigh < 
                _l2_egress[unit].idList.reservedLow) 
            {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "Upper bounds is set less than lower"
                                       " bounds: 0x%x < 0x%x\n"),
                           _l2_egress[unit].idList.reservedHigh, 
                           _l2_egress[unit].idList.reservedLow));

                rv = BCM_E_PARAM;
            } else {
                reserveIt = 1;
            }
        }
    }

    if (reserveIt) {
        
        first = _l2_egress[unit].idList.reservedLow;
        last = _l2_egress[unit].idList.reservedHigh;

        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr, 
                                     first, last);
        
        LOG_VERBOSE(BSL_LS_BCM_L2,
                    (BSL_META_U(unit,
                                "Reserved egress ids: 0x%08x-0x%08x rv=%d %s\n"),
                     first, last, rv, bcm_errmsg(rv)));

    } else if (clearIt) {

        if (first && last) {
            int elt, ignoreRv;

            for (elt = first; elt <= last; elt++) {
                ignoreRv = shr_idxres_list_free(_l2_egress[unit].idList.idMgr,
                                                elt);

                if (BCM_FAILURE(ignoreRv)) {
                    LOG_VERBOSE(BSL_LS_BCM_L2,
                                (BSL_META_U(unit,
                                            "failed to free element 0x%08x  rv=%d %s "
                                             "(ignored)\n"),
                                 elt, ignoreRv, bcm_errmsg(ignoreRv)));
                } 
            }
            LOG_VERBOSE(BSL_LS_BCM_L2,
                        (BSL_META_U(unit,
                                    "Freed reserved l2 egress ids: 0x%08x-0x%08x\n"),
                         first, last));
        }
    }

    L2_UNLOCK(unit);

    return rv;

}

/*
 *   Function
 *      bcm_caladan3_l2_egress_range_get
 *   Purpose
 *     Retrieve the range of valid L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_caladan3_l2_egress_range_get(int unit, uint32 *low, uint32 *high)
{
    int rv = BCM_E_NONE;
    uint32 validLow, validHigh, freeCount, allocCount, first, last;

    UNIT_INIT_CHECK(unit);
    L2_LOCK(unit);

    rv = shr_idxres_list_state(_l2_egress[unit].idList.idMgr, &first, &last, 
                               &validLow, &validHigh, &freeCount, &allocCount);

    L2_UNLOCK(unit);

    *low = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(first);
    *high = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(last);
    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "first=0x%x last=0x%x low=0x%08x high=0x%08x allocated=%d rv=%d\n"),
                 first, last, *low, *high, allocCount, rv));

    return rv;
}




