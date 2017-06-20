/*
 * $Id: oam.c,v 1.35 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * CALADAN3 OAM API
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_OAM

#if defined(INCLUDE_L3)

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm_int/common/debug.h>
#include <bcm/oam.h>
#include <shared/idxres_fl.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/caladan3/oam/oam.h>
#include <soc/sbx/sbDq.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_cop.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <bcm_int/sbx/caladan3/oam/enet.h>
#include <bcm_int/sbx/caladan3/allocator.h>


/***************************************************************************************
 * CONVENTIONS                                                                         *
 * This is the common OAM module.  The intention is that protocol specific information *
 * not be accessed here.  Only common OAM data structures should be referred to in     *
 * this file.  For example, a VID may be referenced by the Ethernet specific OAM       *
 * standards and an MPLS label may be reference by the MPLS OAM standards.  These      *
 * should be referenced in enet.c or perhaps in the future mpls.c.                     *
 *                                                                                     *
 * Functions which begin with bcm_caladan3 are API dispatch functions.                 *
 *                                                                                     *
 * Functions which begin with bcm_c3 are non static local common support functions.    *
 *                                                                                     *
 * Functions which begin with _bcm_c3 are static local common support functions.       *
 *                                                                                     *
 * Non static local support functions may be called by protocol specific OAM functions *
 * to access common data structures.                                                   *
 ***************************************************************************************/
#define OAM_STATE(unit) (bcm_c3_oam_state[unit])

/* Is oam module initialized? (bcm_caladan3_oam_init) successful */
#define BCM_C3_OAM_IS_INIT(unit)   (bcm_c3_oam_state[unit] != NULL && bcm_c3_oam_state[unit]->mutex)

bcm_c3_oam_state_t* bcm_c3_oam_state[SOC_MAX_NUM_DEVICES];

STATIC int _bcm_c3_oam_is_initialized(int unit);
STATIC int _bcm_c3_oam_cleanup(int unit);
STATIC int _bcm_c3_oam_is_group_id_in_reserved_range(int unit, int group_id);
STATIC int _bcm_c3_oam_num_timers_get(int unit, uint32 *oam_timer_count);
STATIC int _bcm_c3_oam_validate_group_info(int unit, bcm_oam_group_info_t *group_info);
STATIC int _bcm_c3_oam_validate_endpoint(int unit, bcm_oam_endpoint_info_t *endpoint_info);
STATIC int _bcm_c3_oam_group_replace(int unit, bcm_oam_group_info_t *group_info);
STATIC void _bcm_c3_oam_timer_event_callback(int unit, soc_sbx_g3p1_util_timer_event_t *event, void *user_cookie);
STATIC int _bcm_c3_oam_exceptions_set(int unit, int enable);
STATIC int _bcm_c3_oam_endpoint_info_state_get(int unit, bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info);

/*
 *   Function
 *      bcm_caladan3_oam_init
 *   Purpose
 *      Initialize the oam software state.
 *   Parameters
 *       unit        = BCM device number
 *       group_info  = description of group to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
#define BCM_C3_OAM_TIMER_COOKIE 0xf6f60000
int
bcm_caladan3_oam_init(int unit)
{
    int rv = BCM_E_NONE;
    int size;
    uint32 timer_cb_cookie = BCM_C3_OAM_TIMER_COOKIE;
    
    BCM_INIT_FUNC_DEFS;
    
    if (BCM_C3_OAM_IS_INIT(unit)) {
        BCM_IF_ERR_EXIT(bcm_caladan3_oam_detach(unit));
    }

    /*
     * Create OAM State - global OAM state structure
     */
    OAM_STATE(unit) = sal_alloc(sizeof(bcm_c3_oam_state_t), "oam_state");
    
    if (OAM_STATE(unit) == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate oam_state"), unit));
    }
    
    sal_memset(OAM_STATE(unit), 0, sizeof(bcm_c3_oam_state_t));
    

    /*
     * Create per endpoint state array OAM_STATE(unit)->endpoint_state
     */
    rv = soc_sbx_g3p1_oam_num_endpoints_get(unit, 
                                            &OAM_STATE(unit)->max_endpoints);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, num endpoints get failure(%d)"), unit, rv));
    }
    size = sizeof(bcm_c3_oam_endpoint_state_t) * OAM_STATE(unit)->max_endpoints;
    OAM_STATE(unit)->endpoint_state = sal_alloc(size, "bcm_c3_oam_endpoint_state");
    
    if (OAM_STATE(unit)->endpoint_state == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, bcm_c3_oam_endpoint_state allocation failure"), unit));
    }
    sal_memset(OAM_STATE(unit)->endpoint_state, 0, size);


    rv = shr_htb_create(&OAM_STATE(unit)->mamep_htbl,
                        OAM_STATE(unit)->max_endpoints,
                        sizeof(bcm_c3_oam_endpoint_hash_key_t),
                        "mamep_hash");
    
    if( BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, mamep_htbl hash table creation failure"), unit));
    }

    /*
     * Create per group state array OAM_STATE(unit)->group_info
     */
    size = sizeof(bcm_c3_oam_group_desc_t) * OAM_STATE(unit)->max_endpoints;
    OAM_STATE(unit)->group_info = sal_alloc(size, "oam_group_info");
    
    if (OAM_STATE(unit)->group_info == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, oam_group_info allocation failure"), unit));
    }
    sal_memset(OAM_STATE(unit)->group_info, 0, size);
    
    /* Avoid using entry 0 */
    rv = shr_idxres_list_create(&OAM_STATE(unit)->endpoint_pool,
                                1, OAM_STATE(unit)->max_endpoints - 1,
                                0, OAM_STATE(unit)->max_endpoints - 1,
                                "oam endpoint_id pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, endpoint_pool list creation failure"), unit));
    }
    
    /* Avoid using entry 0 */
    rv = shr_idxres_list_create(&OAM_STATE(unit)->group_pool,
                                1, OAM_STATE(unit)->max_endpoints - 1,
                                0, OAM_STATE(unit)->max_endpoints - 1,
                                "oam group_id pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, group_pool list creation failure"), unit));
    }

    /*
     * Create timer state and array of timer handles
     */
    rv = _bcm_c3_oam_num_timers_get(unit, &OAM_STATE(unit)->num_oam_timers);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, oam num timers get failure"), unit));
    }
 
    /* Avoid using entry 0 */   
    rv = shr_idxres_list_create(&OAM_STATE(unit)->timer_pool,
                                1, OAM_STATE(unit)->num_oam_timers - 1,
                                0, OAM_STATE(unit)->num_oam_timers - 1,
                                "oam timer pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, timer_pool list creation failure"), unit));
    }

    size = sizeof(uint32) * OAM_STATE(unit)->num_oam_timers;
    OAM_STATE(unit)->timer_handle = sal_alloc(size, "oam timer handles");
    
    if (OAM_STATE(unit)->timer_handle == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, oam timer handle allocation failure"), unit));
    }
    sal_memset(OAM_STATE(unit)->timer_handle, 0, size);

    rv = shr_htb_create(&OAM_STATE(unit)->timer_htbl,
                        OAM_STATE(unit)->num_oam_timers,
                        sizeof(uint32),
                        "timer_hash");
    
    if( BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, (_BCM_MSG("unit %d, timer_htbl hash table creation failure"), unit));
    }

    OAM_STATE(unit)->mutex = sal_mutex_create("OAM_MUTEX");
    if (OAM_STATE(unit)->mutex == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, (_BCM_MSG("unit %d, oam mutex creation failed"), unit));
    }

    /* OAM Watchdog timer event callback setup */
    soc_sbx_g3p1_util_register_timer_callback(unit, _bcm_c3_oam_timer_event_callback, &timer_cb_cookie);

    /* Set the exceptions for OAM */
    BCM_IF_ERR_EXIT(_bcm_c3_oam_exceptions_set(unit, TRUE /* enable=true */));

    /* Currently we only support ethernet CCM - we will need a mechanism to indicate whether enet is enabled
     * in the future
     */
    BCM_IF_ERR_EXIT(bcm_c3_oam_enet_init(unit, OAM_STATE(unit)->max_endpoints));

    rv = soc_sbx_caladan3_lr_bubble_enable(unit, TRUE /* enable */, OAM_STATE(unit)->max_endpoints - 1 /* size */);
    if (rv != BCM_E_NONE) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, (_BCM_MSG("unit %d, OAM bubble init failure"), unit));
    }

    BCM_EXIT;
exit:
    if (rv != BCM_E_NONE) {
        _bcm_c3_oam_cleanup(unit);
    }
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_oam_detach(int unit)
{
    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_oam_cleanup(unit));
 exit:
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_group_range_reserve
 *   Purpose
 *     Reserve a range of oam group ID's
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_UNAVAIL - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int
bcm_caladan3_oam_group_range_reserve(int unit, int reserve_hi, uint32 val)
{
    int rv = BCM_E_NONE;
    uint32 first, last, group;

    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    if (val > OAM_STATE(unit)->max_endpoints) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, Group id out of range max(%d) val(%d)\n"), unit,
                                       OAM_STATE(unit)->max_endpoints, val));
    }
    OAM_LOCK (unit);

    /* Zero for any value, high or low, will clear the known range */
    if (val == 0) {

        first = OAM_STATE(unit)->group_reserved_lo;
        last  = OAM_STATE(unit)->group_reserved_hi;
        
        for (group = first; group <= last; group++) {

            rv = shr_idxres_list_free(OAM_STATE(unit)->group_pool, group);

            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, Group id out of range max(%d) val(%d)\n"), unit,
                                               OAM_STATE(unit)->max_endpoints, val));
            } 
        }
        LOG_VERBOSE(BSL_LS_BCM_OAM,
                    (BSL_META_U(unit,
                                "Freed reserved oam group ids: 0x%08x-0x%08x\n"),
                     first, last));
 
        OAM_STATE(unit)->group_reserved_lo = 0;
        OAM_STATE(unit)->group_reserved_hi = 0;
    } else {
        (reserve_hi) ? (OAM_STATE(unit)->group_reserved_hi = val): (OAM_STATE(unit)->group_reserved_lo = val);
        
        if (OAM_STATE(unit)->group_reserved_hi < OAM_STATE(unit)->group_reserved_lo) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit(%d) Upper bound < lower bound: 0x%x < 0x%x\n"),
                                           unit, OAM_STATE(unit)->group_reserved_hi, 
                                           OAM_STATE(unit)->group_reserved_lo));
        } else {
            rv = shr_idxres_list_reserve(OAM_STATE(unit)->group_pool, 
                                         OAM_STATE(unit)->group_reserved_lo, 
                                         OAM_STATE(unit)->group_reserved_hi);
            
            LOG_VERBOSE(BSL_LS_BCM_OAM,
                        (BSL_META_U(unit,
                                    "Reserved group ids: 0x%08x-0x%08x rv=%d %s\n"),
                         OAM_STATE(unit)->group_reserved_lo, OAM_STATE(unit)->group_reserved_hi, rv, bcm_errmsg(rv)));
            BCM_IF_ERR_EXIT(rv);
        }
    }
    BCM_EXIT;
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_group_range_get
 *   Purpose
 *     Retrieve the range of OAM group IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) low    : first valid ID
 *      (OUT) high   : last valid ID
 *   Returns
 *       BCM_E_UNAVAIL - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_caladan3_oam_group_range_get(int unit, uint32 *low, uint32 *high)
{
    uint32 valid_lo, valid_hi, free_count, alloc_count;

    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    OAM_LOCK(unit);

    BCM_IF_ERR_EXIT(shr_idxres_list_state(OAM_STATE(unit)->group_pool, low, high, 
                                          &valid_lo, &valid_hi, &free_count, &alloc_count));

    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "low=0x%08x high=0x%08x allocated=%d rv=%d\n"),
                 *low, *high, alloc_count, _rv));

    BCM_EXIT;
 exit:
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_group_create
 *   Purpose
 *      Create an oam group, stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *       group_info  = description of group to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_group_create(int unit, bcm_oam_group_info_t *group_info)
{   
    int rv = BCM_E_NONE; 
    int is_replace = FALSE;
    int is_with_id = FALSE;

    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);

    rv = _bcm_c3_oam_validate_group_info(unit, group_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("group info validation failed\n")));
    }

    if ((group_info->flags & BCM_OAM_GROUP_WITH_ID))     is_with_id = TRUE;
    if ((group_info->flags & BCM_OAM_GROUP_REPLACE))     is_replace = TRUE;


    if (is_replace) {
        rv = _bcm_c3_oam_group_replace(unit, group_info);
        BCM_RETURN_VAL_EXIT(rv);
    }

    if (!is_with_id) {

        rv = shr_idxres_list_alloc(OAM_STATE(unit)->group_pool,
                                   (shr_idxres_element_t *)&group_info->id);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, (_BCM_MSG("Failed to allocate oam group id: %d %s\n"), rv, bcm_errmsg(rv)));
        }

    } else /* is_with_id */ {
        
        rv = shr_idxres_list_elem_state(OAM_STATE(unit)->group_pool, group_info->id);

        if (rv == BCM_E_NOT_FOUND) { 
            rv = shr_idxres_list_reserve(OAM_STATE(unit)->group_pool,
                                         group_info->id, group_info->id);
            if (rv != BCM_E_NONE) {
                BCM_ERR_EXIT_MSG(BCM_E_RESOURCE,(_BCM_MSG("Failed to reserve oam group id %d: %d %s\n"),
                                                 group_info->id, rv, bcm_errmsg(rv)));
            }
        } else if (rv != BCM_E_EXISTS) {
            rv = BCM_E_PARAM;
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Group %d not reserved:"
                            "%d %s\n"), group_info->id, rv, bcm_errmsg(rv)));
        } else {
            /* BCM_E_EXIST case, Group either reserved by app or incorrectly being re-created */
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Group %d already reserved:%d %s\n"),
                                           group_info->id, rv, bcm_errmsg(rv)));
        }
    }

    OAM_STATE(unit)->group_info[group_info->id].state = 
        sal_alloc(sizeof(bcm_oam_group_info_t), "oam group info");
    
    if (OAM_STATE(unit)->group_info[group_info->id].state == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("Failed to allocate group_info state\n")));
    }

    /* New group - no associated endpoints */
    DQ_INIT(&OAM_STATE(unit)->group_info[group_info->id].endpoint_list);

    sal_memcpy(OAM_STATE(unit)->group_info[group_info->id].state, group_info, 
               sizeof(bcm_oam_group_info_t));
        
    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "Created oam_group id=%d\n"),
                 group_info->id));

    BCM_EXIT;
exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_group_get
 *   Purpose
 *      Retrieve the group information for the given group id
 *   Parameters
 *       unit        = BCM device number
 *       group       = group to retrieve
 *       group_info  = storage location for found information
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_group_get(int unit, bcm_oam_group_t group, 
                           bcm_oam_group_info_t *group_info)
{
    int rv  COMPILER_ATTRIBUTE((unused));
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    if (group_info == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Unit(%d) group_info pointer passed invalid"), unit));
    }

    rv = bcm_c3_oam_group_get(unit, group, group_info);
    BCM_RETURN_VAL_EXIT(rv);

 exit:              
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_group_destroy
 *   Purpose
 *      Destroy an oam group, stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *       group       = group id to destroy
 *   Returns
 *       BCM_E_*
 *  Notes:
 *      Any associated endpoint will also be destroyed
 */
int
bcm_caladan3_oam_group_destroy(int unit, bcm_oam_group_t group_id)
{
    int rv = BCM_E_NONE;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);

    rv = shr_idxres_list_elem_state(OAM_STATE(unit)->group_pool, group_id);

    if (rv != BCM_E_EXISTS) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Attempt to destroy non-existent oam group id %d\n"), group_id));
    }

    rv = BCM_E_NONE;
    if (!_bcm_c3_oam_is_group_id_in_reserved_range(unit, group_id)) {
        rv = shr_idxres_list_free(OAM_STATE(unit)->group_pool, group_id);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_OAM,
                     (BSL_META_U(unit,
                                 "Failed to free oam group %d: %d %s\n"),
                      group_id,
                      rv, bcm_errmsg(rv)));
            /* non-fatal, keep trying to destroy this group */
        }
    }

    if (OAM_STATE(unit)->group_info[group_id].state) {
        sal_free(OAM_STATE(unit)->group_info[group_id].state);
        OAM_STATE(unit)->group_info[group_id].state = NULL;
    }

    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "Destroyed oam_group id=%d\n"),
                 group_id));

    BCM_EXIT;
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_group_destroy_all
 *   Purpose
 *      Destroy all empty and active oam groups stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *   Returns
 *       BCM_E_*
 *  Notes:
 *      If a non-empty group is found, the group is not destroyed, but 
 *      continues to destroy the remaining groups.
 */
int
bcm_caladan3_oam_group_destroy_all(int unit)
{
    int rv = BCM_E_NONE;
    int group_id;
    int rv_tmp = BCM_E_NONE;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);
    for (group_id = 0; group_id < OAM_STATE(unit)->max_endpoints; group_id++) {
        if (OAM_STATE(unit)->group_info[group_id].state) {    

            rv_tmp = bcm_caladan3_oam_group_destroy(unit, group_id);
            if (BCM_FAILURE(rv_tmp)) {
                LOG_WARN(BSL_LS_BCM_OAM,
                         (BSL_META_U(unit,
                                     "Failed to destroy group %d: %d %s\n"),
                          group_id,
                          rv_tmp, bcm_errmsg(rv)));
                /* Non-fatal error, keep trying, but keep last error */
                rv = rv_tmp;
            }
        }
    }
    BCM_RETURN_VAL_EXIT(rv);
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_group_traverse
 *   Purpose
 *      Traverse the set of active groups
 *   Parameters
 *       unit        = BCM device number
 *       cb          = callback invoked for each active group
 *       user_data   = user data passed to each invocation of the callback
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_group_traverse(int unit, bcm_oam_group_traverse_cb cb, 
                                void *user_data)
{
    int group;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);
    for (group=0; group < OAM_STATE(unit)->max_endpoints; group++) {
        if (OAM_STATE(unit)->group_info[group].state) {
            (cb)(unit, OAM_STATE(unit)->group_info[group].state, user_data);
        }
    }

    BCM_EXIT;
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_endpoint_range_reserve
 *   Purpose
 *     Reserve a range of oam endpoint ID's
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_UNAVAIL - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int
bcm_caladan3_oam_endpoint_range_reserve(int unit, int reserve_hi, uint32 val)
{
    int rv = BCM_E_NONE;
    uint32 first, last, endpoint;

    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    if (val > OAM_STATE(unit)->max_endpoints) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, endpoint id out of range max(%d) val(%d)\n"), unit,
                                       OAM_STATE(unit)->max_endpoints, val));
    }
    OAM_LOCK (unit);

    /* Zero for any value, high or low, will clear the known range */
    if (val == 0) {

        first = OAM_STATE(unit)->endpoint_reserved_lo;
        last  = OAM_STATE(unit)->endpoint_reserved_hi;
        
        for (endpoint = first; endpoint <= last; endpoint++) {
            rv = shr_idxres_list_free(OAM_STATE(unit)->endpoint_pool, endpoint);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_OAM,
                            (BSL_META_U(unit,
                                        "failed to free endpoint %d from endpoint_pool rv=%d %s (ignored)\n"),
                             endpoint, rv, bcm_errmsg(rv)));
            } 
        }

        LOG_VERBOSE(BSL_LS_BCM_OAM,
                    (BSL_META_U(unit,
                                "Freed reserved oam endpoint ids: 0x%08x-0x%08x\n"),
                     first, last));

        OAM_STATE(unit)->endpoint_reserved_lo = 0;
        OAM_STATE(unit)->endpoint_reserved_hi = 0;

    } else {
        (reserve_hi) ? (OAM_STATE(unit)->endpoint_reserved_hi = val): (OAM_STATE(unit)->endpoint_reserved_lo = val);
        
        if (OAM_STATE(unit)->endpoint_reserved_hi < OAM_STATE(unit)->endpoint_reserved_lo) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit(%d) Upper bound < lower bound: 0x%x < 0x%x\n"),
                                           unit, OAM_STATE(unit)->endpoint_reserved_hi, 
                                           OAM_STATE(unit)->endpoint_reserved_lo));
        } else {
            rv = shr_idxres_list_reserve(OAM_STATE(unit)->endpoint_pool, 
                                         OAM_STATE(unit)->endpoint_reserved_lo, 
                                         OAM_STATE(unit)->endpoint_reserved_hi);
            
            LOG_VERBOSE(BSL_LS_BCM_OAM,
                        (BSL_META_U(unit,
                                    "Reserved group ids: 0x%08x-0x%08x rv=%d %s\n"),
                         OAM_STATE(unit)->endpoint_reserved_lo, OAM_STATE(unit)->endpoint_reserved_hi, rv, bcm_errmsg(rv)));
        }
    }
    BCM_RETURN_VAL_EXIT(rv);
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_endpoint_range_get
 *   Purpose
 *     Retrieve the range of OAM endpoint IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_UNAVAIL - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_caladan3_oam_endpoint_range_get(int unit, uint32 *low, uint32 *high)
{
    int rv = BCM_E_UNAVAIL;
    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 function unavailable"), unit));
    /* BCM_EXIT; */
 exit:              
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_endpoint_action_set
 *   Purpose
 *
 *   Parameters
 *      (IN)  unit    : unit number of the device
 *      (IN)  id      : endpoint ID
 *      (IN)  action  : Action
 *   Returns
 *       BCM_E_UNAVAIL - Not used for Caladan3
 *   Notes
 */
int bcm_caladan3_oam_endpoint_action_set(int unit, bcm_oam_endpoint_t id, bcm_oam_endpoint_action_t *action)
{
    int rv = BCM_E_UNAVAIL;
    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 function unavailable"), unit));
    /* BCM_EXIT; */
 exit:              
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_endpoint_create
 *   Purpose
 *      Create an oam endpoint and commit to hardware
 *   Parameters
 *       unit           = BCM device number
 *       endpoint_info  = description of endpoint to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 * 
 */
int
bcm_caladan3_oam_endpoint_create(int unit, bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK (unit);

    /* Generic endpoint check */
    rv = _bcm_c3_oam_validate_endpoint(unit, endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Failed to validate endpoint\n")));
    }

    switch (endpoint_info->type) 
    {
        case (bcmOAMEndpointTypeEthernet):
            rv = bcm_c3_oam_enet_endpoint_create(unit, endpoint_info);
            BCM_IF_ERR_EXIT(rv);
        break;
        default:
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, oam type unsupported"), unit));
        break;

    }
exit:
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_endpoint_get
 *   Purpose
 *      Retrieve an oam endpoint with the given endpoint id
 *   Parameters
 *       unit           = BCM device number
 *       endpoint       = endpoint ID to retrieve
 *       endpoint_info  = storage location for found endpoint
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_endpoint_get(int unit, bcm_oam_endpoint_t endpoint_id, 
                              bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);

    if (endpoint_id == 0 || endpoint_id > OAM_STATE(unit)->max_endpoints) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid endpoint.\n")));
    }


    bcm_oam_endpoint_info_t_init(endpoint_info);
    endpoint_info->id = endpoint_id;
    
    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "Entered oam_endpoint_get with id=0x%x.\n"),
                 endpoint_id));    
    
    /* verify found record index is allocated */
    if (bcm_c3_oam_endpoint_is_allocated(unit, endpoint_id) == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_NOT_FOUND, (_BCM_MSG("endpoint_id(%d) not found\n"), endpoint_id));
    }
    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "Found endpoint entry 0x%8x,  %d. \n"),
                 endpoint_id, endpoint_id));

    rv = _bcm_c3_oam_endpoint_info_state_get(unit, endpoint_id, endpoint_info);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("endpoint_id(%d) endpoint_state get error\n"), endpoint_id));
    }

    switch (endpoint_info->type) {
        case (bcmOAMEndpointTypeEthernet):
            rv = bcm_c3_oam_enet_endpoint_get(unit, endpoint_id, endpoint_info);
            BCM_IF_ERR_EXIT(rv);
            break;
        default:
            BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, oam type unsupported"), unit));
        break;
    }

    endpoint_info->flags &= ~BCM_OAM_ENDPOINT_WITH_ID;    

    BCM_EXIT;
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_endpoint_destroy
 *   Purpose
 *      Destroy an oam endpoint and all allocated resources  with the given 
 *      endpoint id
 *   Parameters
 *       unit           = BCM device number
 *       endpoint       = endpoint ID to destroy
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_endpoint_destroy(int unit, bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    bcm_oam_endpoint_info_t endpoint_info;

    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    OAM_LOCK(unit);

    if (bcm_c3_oam_endpoint_is_allocated(unit, endpoint_id) == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_NOT_FOUND, (_BCM_MSG("endpoint_id(%d) not found\n"), endpoint_id));
    }

    rv = _bcm_c3_oam_endpoint_info_state_get(unit, endpoint_id, &endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("endpoint_state for id(%d) retrieval error\n"), endpoint_id));
    }

    switch (endpoint_info.type) 
    {
        case (bcmOAMEndpointTypeEthernet):
            rv = bcm_c3_oam_enet_endpoint_destroy(unit, &endpoint_info);
            BCM_IF_ERR_EXIT(rv);
        break;
        default:
            BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, oam type unsupported"), unit));
        break;
    }

exit:
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_caladan3_oam_endpoint_destroy_all
 *   Purpose
 *      Destroy all endpoints associated with the given group
 *   Parameters
 *       unit         = BCM device number
 *       group        = endpoints belonging to this group id will be destroyed
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_endpoint_destroy_all(int unit, bcm_oam_group_t group)
{
    int rv = BCM_E_UNAVAIL;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 function unavailable"), unit));
    /* BCM_EXIT; */
 exit:              
    BCM_FUNC_RETURN;
}


/*
 *   Function
 *      bcm_caladan3_oam_endpoint_traverse
 *   Purpose
 *      Traverse the set of active endpoints in a given group
 *   Parameters
 *       unit        = BCM device number
 *       group       = group id to traverse
 *       cb          = callback invoked for each active endpoint
 *       user_data   = user data passed to each invocation of the callback
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_endpoint_traverse(int unit,  bcm_oam_group_t group, 
                                 bcm_oam_endpoint_traverse_cb cb,  
                                 void *user_data)
{
    int rv = BCM_E_UNAVAIL;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));

    BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 function unavailable"), unit));
    /* BCM_EXIT; */
 exit:              
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_dump_reservation
 *   Purpose
 *     Dumps reservation of OAM resources
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *   Notes
 */
void bcm_caladan3_oam_dump_reservation(int unit)
{
    int fw = 10;
    
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    OAM_LOCK(unit);

    LOG_CLI((BSL_META_U(unit,
                        "%-8s%-*s%-8s   %-*s%-8s\n"), "OAM-GROUP", 19, "", "", fw, "", ""));

    if (OAM_STATE(unit)->group_reserved_lo || OAM_STATE(unit)->group_reserved_hi) {
        LOG_CLI((BSL_META_U(unit,
                            "  %-20s %-*s0x%08x %-*s0x%08x\n\n"), "RESERVATION:", fw, "",
                 OAM_STATE(unit)->group_reserved_lo, fw, "",
                 OAM_STATE(unit)->group_reserved_hi));
    } else {           
        LOG_CLI((BSL_META_U(unit,
                            "***** No Reservations ******\n")));
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "%-8s%-*s%-8s   %-*s%-8s\n"), "OAM-ENDPOINT", 19, "", "", fw, "", ""));

    if (OAM_STATE(unit)->endpoint_reserved_lo || OAM_STATE(unit)->endpoint_reserved_hi) {
        LOG_CLI((BSL_META_U(unit,
                            "  %-20s %-*s0x%08x %-*s0x%08x\n\n"), "RESERVATION:", fw, "",
                 OAM_STATE(unit)->endpoint_reserved_lo, fw, "",
                 OAM_STATE(unit)->endpoint_reserved_hi));
    } else {           
        LOG_CLI((BSL_META_U(unit,
                            "***** No Reservations ******\n")));
    }

    BCM_EXIT;
 exit:
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN_VOID;
}

/*
 *   Function
 *      bcm_caladan3_oam_event_register
 *   Purpose
 *      Register a callback for various oam events
 *   Parameters
 *       unit           = BCM device number
 *       event_types    = events to cause an invocation of the callback
 *       cb             = user callback
 *       user_data      = user supplied data, passed to user callback
 *                        when invoked
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_event_register(int unit, bcm_oam_event_types_t event_types, 
                                bcm_oam_event_cb cb, void *user_data)
{
    bcm_oam_event_type_t event_id;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    OAM_LOCK(unit);

    for (event_id = 0; event_id < bcmOAMEventCount; event_id++) {
         if (BCM_OAM_EVENT_TYPE_GET(event_types, event_id)) {
             if ((OAM_STATE(unit)->event_cb_info[event_id].event_cb)  &&
                 (OAM_STATE(unit)->event_cb_info[event_id].event_cb != cb)) {
                 /* A different calblack is already registered for this event. Return error */
                 BCM_ERR_EXIT_MSG(BCM_E_EXISTS, (_BCM_MSG("EVENT %d already has a registered callback"), event_id));
             }

             switch (event_id) {
			    case bcmOAMEventEndpointCCMTimeout: 
                    break;
				default:
					BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,
					 (_BCM_MSG("Event ID is not supported: %d"), event_id));
			}
             OAM_STATE(unit)->event_cb_info[event_id].event_cb = cb;
             OAM_STATE(unit)->event_cb_info[event_id].user_data = user_data;
         }
    }
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_oam_event_unregister
 *   Purpose
 *      Unregister a callback from an event
 *   Parameters
 *       unit        = BCM device number
 *       event_types = event to unregister
 *       cb          = callback to unregister
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_caladan3_oam_event_unregister(int unit, bcm_oam_event_types_t event_types, 
                                bcm_oam_event_cb cb)
{
    bcm_oam_event_type_t event_id;
    BCM_INIT_FUNC_DEFS;

    BCM_IF_ERR_EXIT(_bcm_c3_oam_is_initialized(unit));
    OAM_LOCK(unit);

    for (event_id = 0; event_id < bcmOAMEventCount; event_id++) {
         if (BCM_OAM_EVENT_TYPE_GET(event_types, event_id)) {
             if ((OAM_STATE(unit)->event_cb_info[event_id].event_cb)  &&
                 (OAM_STATE(unit)->event_cb_info[event_id].event_cb != cb)) {
                 /* A different calblack is already registered for this event. Return error */
                 BCM_ERR_EXIT_MSG(BCM_E_EXISTS, (_BCM_MSG("EVENT %d already has a registered callback"), event_id));
             }

             switch (event_id) {
			    case bcmOAMEventEndpointCCMTimeout: 
                    break;
				default:
					BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,
					 (_BCM_MSG("Event ID is not supported: %d"), event_id));
			}
             OAM_STATE(unit)->event_cb_info[event_id].event_cb = NULL;
             OAM_STATE(unit)->event_cb_info[event_id].user_data = NULL;
         }
    }
 exit:              
    OAM_UNLOCK(unit);
    BCM_FUNC_RETURN;
}

/******************** 
 * Internal Routines 
 ********************/

/*  Verify that bcm_oam_init() has successfully been run prior to the current function call */
STATIC int _bcm_c3_oam_is_initialized(int unit)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        if (!BCM_C3_OAM_IS_INIT(unit)) {
            rv = BCM_E_INIT;
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 OAM not initialized\n"), unit));
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
        BCM_RETURN_VAL_EXIT(rv);
    }
 exit:
    BCM_FUNC_RETURN;
}
int
bcm_c3_oam_group_get(int unit, bcm_oam_group_t group, 
                     bcm_oam_group_info_t *group_info) {

    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    rv = shr_idxres_list_elem_state(OAM_STATE(unit)->group_pool, group);

    if (rv != BCM_E_EXISTS) {
        BCM_ERR_EXIT_MSG(BCM_E_NOT_FOUND, (_BCM_MSG("oam group id %d does not exist!\n"), group));
    } else {
        sal_memcpy(group_info, (uint8*)(OAM_STATE(unit)->group_info[group].state), sizeof(bcm_oam_group_info_t));
        rv = BCM_E_NONE;
    }
    BCM_EXIT;
 exit:              
    BCM_FUNC_RETURN;
}

/* Return true if group id is part of reserved range */
STATIC int _bcm_c3_oam_is_group_id_in_reserved_range(int unit, int group_id) {
    if ((OAM_STATE(unit)->group_reserved_hi < group_id) ||
        (OAM_STATE(unit)->group_reserved_lo > group_id)) {
        /* Outside range */
        return FALSE;
    } else {
        return TRUE;
    }
}

/* return true if endpoint id is part of reserved range */
int bcm_c3_oam_is_endpoint_id_in_reserved_range(int unit, bcm_oam_endpoint_t endpoint_id) {
    if ((OAM_STATE(unit)->endpoint_reserved_lo < endpoint_id) ||
        (OAM_STATE(unit)->endpoint_reserved_hi > endpoint_id)) {
        /* Outside range */
        return FALSE;
    } else {
        return TRUE;
    }
}

/* endpoint state table access */
void
bcm_c3_oam_endpoint_state_from_id_get(int unit, bcm_c3_oam_endpoint_state_t *endpoint_state, bcm_oam_endpoint_t endpoint_id)
{
    *endpoint_state = OAM_STATE(unit)->endpoint_state[endpoint_id];
}
int
bcm_c3_oam_endpoint_state_from_key_get(int unit, bcm_c3_oam_endpoint_hash_key_t endpoint_key,
                                       bcm_c3_oam_endpoint_state_t *endpoint_state, int remove)
{
    int rv = BCM_E_NONE;
    BCM_INIT_FUNC_DEFS;

    rv = shr_htb_find(OAM_STATE(unit)->mamep_htbl, endpoint_key, (shr_htb_data_t*)&endpoint_state, remove);
    BCM_RETURN_VAL_EXIT(rv); 
 exit:
    BCM_FUNC_RETURN;
}
int
bcm_c3_oam_endpoint_key_set(int unit, bcm_c3_oam_endpoint_hash_key_t endpoint_key,
                            bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    bcm_c3_oam_endpoint_state_t *endpoint_state = NULL;
    BCM_INIT_FUNC_DEFS;

    endpoint_state = &OAM_STATE(unit)->endpoint_state[endpoint_id];
    rv = shr_htb_insert(OAM_STATE(unit)->mamep_htbl, endpoint_key, endpoint_state);
    BCM_RETURN_VAL_EXIT(rv); 

 exit:
    BCM_FUNC_RETURN;
}

/* Returns true if allocated, false if not */
int
bcm_c3_oam_endpoint_is_allocated(int unit, bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    
    /* check if the requested ID has already been allocated. */
    rv = shr_idxres_list_elem_state(OAM_STATE(unit)->endpoint_pool, endpoint_id);
    if (rv == BCM_E_NOT_FOUND) {
        return FALSE;
    } else {
        return TRUE;
    }
}

int
bcm_c3_oam_endpoint_allocate(int unit, bcm_oam_endpoint_t *endpoint_id, int is_with_id)
{
    int rv = BCM_E_NONE;
    int endpoint_is_allocated = TRUE;

    BCM_INIT_FUNC_DEFS;

    if (is_with_id == TRUE) {
        
        /* check if the requested ID has already been allocated. */
        endpoint_is_allocated = bcm_c3_oam_endpoint_is_allocated(unit, *endpoint_id);
        
        if (endpoint_is_allocated == FALSE) {
            /* reserve the requested endpoint-id */
            rv = shr_idxres_list_reserve(OAM_STATE(unit)->endpoint_pool, *endpoint_id, *endpoint_id);
            
            if (rv != BCM_E_NONE) {
                BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("Endpoint(%d) could not be reserved\n"), *endpoint_id));
            }     
        } else {
            rv = BCM_E_RESOURCE;
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("Endpoint(%d) already allocated\n"), *endpoint_id));
        }
        
    } else { /* is_with_id == FALSE */
        rv = shr_idxres_list_alloc(OAM_STATE(unit)->endpoint_pool, (uint32*)endpoint_id);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("Endpoint(%d) could not be allocated\n"), *endpoint_id));
        }
    }
 exit:
    BCM_FUNC_RETURN;
}

int
bcm_c3_oam_endpoint_free(int unit, bcm_oam_endpoint_t endpoint_id)
{
    int rv  COMPILER_ATTRIBUTE((unused));
    BCM_INIT_FUNC_DEFS;
    rv = shr_idxres_list_free(OAM_STATE(unit)->endpoint_pool, endpoint_id);
    BCM_IF_ERR_EXIT(rv);
exit:
    BCM_FUNC_RETURN;
}
int
bcm_c3_oam_endpoint_state_set(int unit, bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info)
{
    bcm_c3_oam_endpoint_state_t *endpoint_state = NULL;
    int is_upmep = FALSE;

    BCM_INIT_FUNC_DEFS;

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))   is_upmep = TRUE;

    endpoint_state = &OAM_STATE(unit)->endpoint_state[endpoint_id];
    endpoint_state->endpoint_id = endpoint_id;
    endpoint_state->name        = endpoint_info->name;
    endpoint_state->flags       = endpoint_info->flags;
    endpoint_state->type        = endpoint_info->type;
    endpoint_state->group       = endpoint_info->group;
    endpoint_state->gport       = endpoint_info->gport;
    endpoint_state->ing_map     = endpoint_info->ing_map;
    endpoint_state->egr_map     = endpoint_info->egr_map;
    endpoint_state->intf_id     = endpoint_info->intf_id;
    endpoint_state->local_id    = endpoint_info->local_id; /* local EP's ID */
    endpoint_state->mdlevel     = endpoint_info->level;
    endpoint_state->ccm_period  = endpoint_info->ccm_period;
    endpoint_state->direction   = is_upmep;

    BCM_FUNC_RETURN;
}

int
bcm_c3_oam_endpoint_state_clear(int unit, bcm_oam_endpoint_t endpoint_id)
{
    bcm_c3_oam_endpoint_state_t *endpoint_state = NULL;

    BCM_INIT_FUNC_DEFS;

    endpoint_state = &OAM_STATE(unit)->endpoint_state[endpoint_id];
    sal_memset(endpoint_state, 0, sizeof(bcm_c3_oam_endpoint_state_t));

    BCM_FUNC_RETURN;
}

/**** Endpoint in group info routines ****/
int
bcm_c3_oam_group_info_endpoint_list_count(int unit, int group)
{
    int count=0;
    DQ_LENGTH(&OAM_STATE(unit)->group_info[group].endpoint_list, count);
    return count;
}

int
bcm_c3_oam_group_info_endpoint_list_add(int unit, int group, int endpoint_id) 
{

    int rv  COMPILER_ATTRIBUTE((unused));
    dq_p_t endpoint_list;

    BCM_INIT_FUNC_DEFS;
    rv = BCM_E_NONE;

    if (OAM_STATE(unit)->group_info[group].state == NULL) {

        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Invalid Group\n")));
        rv = BCM_E_PARAM;
        BCM_RETURN_VAL_EXIT(rv);
    }

    endpoint_list = &OAM_STATE(unit)->endpoint_state[endpoint_id].endpoint_list;
    DQ_INSERT_HEAD(&OAM_STATE(unit)->group_info[group].endpoint_list, endpoint_list); 

 exit:
    BCM_FUNC_RETURN;
}

int
bcm_c3_oam_group_info_endpoint_list_remove(int unit, int group, int endpoint_id)
{
    int rv  COMPILER_ATTRIBUTE((unused));
    dq_p_t endpoint_list;

    BCM_INIT_FUNC_DEFS;
    rv = BCM_E_NONE;

    if (OAM_STATE(unit)->group_info[group].state == NULL) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Invalid Group\n")));
        rv = BCM_E_PARAM;
        BCM_RETURN_VAL_EXIT(rv);
    }
    endpoint_list = &OAM_STATE(unit)->endpoint_state[endpoint_id].endpoint_list;
    DQ_REMOVE(endpoint_list);
exit:
    BCM_FUNC_RETURN;
}

/***** End endpoint in group management routines *****/

/* Given the endpoint id, return the endpoint info from the stored endpoint state */
STATIC int
_bcm_c3_oam_endpoint_info_state_get(int unit, bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv  COMPILER_ATTRIBUTE((unused));
    bcm_c3_oam_endpoint_state_t *endpoint_state = NULL;

    BCM_INIT_FUNC_DEFS;
    rv = BCM_E_NONE;

    if (endpoint_id > OAM_STATE(unit)->max_endpoints) {
        rv = BCM_E_PARAM;
        BCM_RETURN_VAL_EXIT(rv);
    }

    endpoint_state = &OAM_STATE(unit)->endpoint_state[endpoint_id];

    sal_memset(endpoint_info, 0, sizeof(bcm_oam_endpoint_info_t));

    endpoint_info->flags      = endpoint_state->flags;
    endpoint_info->type       = endpoint_state->type;
    endpoint_info->group      = endpoint_state->group;
    endpoint_info->gport      = endpoint_state->gport;
    endpoint_info->name       = endpoint_state->name;
    endpoint_info->local_id   = endpoint_state->local_id;
    endpoint_info->level      = endpoint_state->mdlevel;
    endpoint_info->ccm_period = OAM_STATE(unit)->endpoint_state[endpoint_id].ccm_period;
    endpoint_info->id         = endpoint_id;

    switch (endpoint_info->type) 
    {
        case (bcmOAMEndpointTypeEthernet):
            BCM_IF_ERR_EXIT(bcm_c3_oam_enet_endpoint_info_state_get(unit, endpoint_info, endpoint_id));
        break;
        default:
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, oam type unsupported"), unit));
        break;
    }

exit:
    BCM_FUNC_RETURN;
}


STATIC int
_bcm_c3_oam_get_endpoint_by_timer(int unit, uint32 watchdog_id, bcm_oam_endpoint_t *endpoint_id)
{
    int rv;
    rv = shr_htb_find(OAM_STATE(unit)->timer_htbl, &watchdog_id,
                      (shr_htb_data_t *)endpoint_id,
                      0 /* == don't remove */);

    return rv;
}

STATIC
uint32 _bcm_c3_oam_timeout_in_ms[8] = {0,          /* invalid      */
                                       11660,      /* 3.5 * 3.33ms */
                                       35000,      /* 3.5 *   10ms */
                                       350000,     /* 3.5 *  100ms */
                                       3500000,    /* 3.5 *     1s */
                                       35000000,   /* 3.5 *    10s */
                                       210000000,  /* 3.5 *     1m */
                                       2100000000};/* 3.5 *    10m */

int bcm_c3_oam_timer_allocate(int unit, bcm_oam_endpoint_t endpoint_id, 
                              int ccm_period,
                              int started, uint32 *watchdog_id)
{
    int rv = BCM_E_NONE;
    uint32 timeout_in_msec;
    int packet_ccm_period;
    uint32 timer_handle;

    rv = bcm_c3_oam_enet_sdk_ccm_period_to_packet_ccm_period(unit, ccm_period, &packet_ccm_period);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "ccm period calculation failure\n")));
        return rv;
    }

    timeout_in_msec = _bcm_c3_oam_timeout_in_ms[packet_ccm_period];

    rv = shr_idxres_list_alloc(OAM_STATE(unit)->timer_pool, watchdog_id);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Out of timer resources\n")));
        return rv;
    }

    /* Timeout tick is 1000us (1ms) */
    rv = soc_sbx_g3p1_timer_create(unit, COP_TIMER_SEGMENT_OAMTIMER, timeout_in_msec, &timer_handle, *watchdog_id, 
                                   1 /* interrupt */, started);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to create OAM timer: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    OAM_STATE(unit)->timer_handle[*watchdog_id] = timer_handle;

    rv = shr_htb_insert(OAM_STATE(unit)->timer_htbl, watchdog_id,
                        (shr_htb_data_t)endpoint_id);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to insert watchdog_id=%d and endpoint_id=%d: %d %s\n"),
                   *watchdog_id, endpoint_id, rv, bcm_errmsg(rv)));
    }

    LOG_VERBOSE(BSL_LS_BCM_OAM,
                (BSL_META_U(unit,
                            "allocated timer: id=%d (0x%x) rate=%d(msec) started=%d ep=%d\n"),
                 *watchdog_id, *watchdog_id, (int)timeout_in_msec, started, endpoint_id));

    return rv;
}

int bcm_c3_oam_timer_free(int unit, uint32 watchdog_id)
{
    int rv = BCM_E_NONE;
    int endpoint_id;
    uint32 timer_handle;

    shr_idxres_list_free(OAM_STATE(unit)->timer_pool, watchdog_id);

    timer_handle = OAM_STATE(unit)->timer_handle[watchdog_id];

    rv = soc_sbx_g3p1_timer_delete(unit, timer_handle);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to delete OAM timer id=%d: %d %s\n"),
                   watchdog_id, rv, bcm_errmsg(rv)));
    }

    OAM_STATE(unit)->timer_handle[watchdog_id] = 0;

    rv = shr_htb_find(OAM_STATE(unit)->timer_htbl, &watchdog_id,
                      (shr_htb_data_t *)&endpoint_id,
                      1 /* == remove */);

    return rv;
}

void  _bcm_c3_oam_timer_event_callback(int unit, 
                                       soc_sbx_g3p1_util_timer_event_t *event, 
                                       void *user_cookie)
{
    bcm_oam_endpoint_t endpoint_id;
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    LOG_CLI((BSL_META_U(unit,
                        "unit(%d) oam timer id(0x%08x) forced_timeout(%d) timer_active_when_forced(%d)\n"),
             unit, event->id, event->forced_timeout, event->timer_active_when_forced));


    rv = _bcm_c3_oam_get_endpoint_by_timer(unit, event->id, &endpoint_id);

    if (BCM_FAILURE(rv) ) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("failed to find endpoint with timer handle=%d\n"), 
                                         event->id));
    }
    
    
    if (OAM_STATE(unit)->event_cb_info[bcmOAMEventEndpointCCMTimeout].event_cb != NULL) {
        
        (OAM_STATE(unit)->event_cb_info[bcmOAMEventEndpointCCMTimeout].event_cb)(unit, 
                                                                                 0 /* flags */, bcmOAMEventEndpointCCMTimeout, 
                                                                                 0 /* group id */, endpoint_id,
                                                                                 OAM_STATE(unit)->event_cb_info[bcmOAMEventEndpointCCMTimeout].user_data);
    }

exit:
    BCM_FUNC_RETURN_VOID;
}

/*
 * Function
 *      _bcm_3_oam_num_timers_get
 * Purpose
 *      Returns the max oam timers possible
 * Parameters
 *      (in) unit   = unit number
 * Returns
 *      int - max number of oam timers
 */
STATIC int
_bcm_c3_oam_num_timers_get(int unit, uint32 *oam_timer_count) 
{
    int rv;
    BCM_INIT_FUNC_DEFS;

    rv = soc_sbx_g3p1_constant_get(unit, "num_oam_timers", oam_timer_count);
    if (rv != BCM_E_NONE) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, unable to allocate oam_state"), unit));
    }
    BCM_EXIT;
exit:
    BCM_FUNC_RETURN;
    
}


STATIC 
int _bcm_c3_oam_validate_group_info(int unit, 
                                    bcm_oam_group_info_t *group_info)
{
    int status = BCM_E_PARAM;

    if(!group_info) {

        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Null group info\n")));

    } else if ((group_info->flags & BCM_OAM_GROUP_WITH_ID) &&
               ((group_info->id == 0) || 
                (group_info->id > OAM_STATE(unit)->max_endpoints)))  {

        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Invalid group id.  Must supply group id \n")));

    }  else if ((group_info->flags & BCM_OAM_GROUP_WITH_ID) &&
                (_bcm_c3_oam_is_group_id_in_reserved_range(unit, group_info->id))) {

        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Invalid Group id in reserved range\n")));

    } else if ((group_info->flags & BCM_OAM_GROUP_REPLACE) &&
               ((group_info->flags & BCM_OAM_GROUP_WITH_ID) == 0)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Cannot replace group without id\n")));
    } else {
        status = BCM_E_NONE;
    }

    return status;
}
STATIC int
_bcm_c3_oam_group_replace(int unit, bcm_oam_group_info_t *group_info)
{
    int rv = BCM_E_NONE; 
    int allocate_group_info;
    int ep_count = 0;
    int maid_change;

    BCM_INIT_FUNC_DEFS;

    /* On replace with id, verify the group exists
     * On with id, reserve the group id & allocate group_info
     * else, allocate a new group id, and allocate group_info
     */       
    allocate_group_info = 0;

    rv = shr_idxres_list_elem_state(OAM_STATE(unit)->group_pool, group_info->id);

    if (rv != BCM_E_EXISTS) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM,(_BCM_MSG("unit %d, Attempted to replace non-existent group(%d)\n"),
                                      unit, group_info->id));
    }
    
    ep_count = bcm_c3_oam_group_info_endpoint_list_count(unit, group_info->id);
    
    maid_change = sal_memcmp(group_info->name,
                             OAM_STATE(unit)->group_info[group_info->id].state->name,
                             sizeof(group_info->name));
    
    /* if the MAID changed and there are active EP's in this group,
     * then fail the request.
     */
    if ((maid_change != 0) && (ep_count != 0)) {
        LOG_VERBOSE(BSL_LS_BCM_OAM,
                    (BSL_META_U(unit,
                                "Replacing group's name %d while group contains %d active endpoints.\n"),
                     group_info->id, ep_count));
        rv = BCM_E_PARAM;
        BCM_IF_ERR_EXIT(rv);
    }
    
    if (group_info->id > OAM_STATE(unit)->max_endpoints) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Invalid group_id:%d\n"),
                   group_info->id));
        rv = BCM_E_PARAM;
        BCM_RETURN_VAL_EXIT(rv);
    }
    
    if (allocate_group_info) {
        OAM_STATE(unit)->group_info[group_info->id].state = 
            sal_alloc(sizeof(bcm_oam_group_info_t), "oam group info");
        
        if (OAM_STATE(unit)->group_info[group_info->id].state == NULL) {
            LOG_ERROR(BSL_LS_BCM_OAM,
                      (BSL_META_U(unit,
                                  "Failed to allocate group_info state\n")));
            rv = BCM_E_MEMORY;
            BCM_RETURN_VAL_EXIT(rv);
        }
        
        /* New group - no associated endpoints */
        DQ_INIT(&OAM_STATE(unit)->group_info[group_info->id].endpoint_list);
    }

    if (BCM_SUCCESS(rv)) {
         /* coverity [dead_error_begin] */
        sal_memcpy(OAM_STATE(unit)->group_info[group_info->id].state, group_info, 
                   sizeof(bcm_oam_group_info_t));
        
        LOG_VERBOSE(BSL_LS_BCM_OAM,
                    (BSL_META_U(unit,
                                "Created oam_group id=%d\n"),
                     group_info->id));
    } else {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to create oam group: %d %s\n"), 
                   rv, bcm_errmsg(rv)));
        BCM_RETURN_VAL_EXIT(rv);
    }
    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}

/***** Validate endpoint information *******/
STATIC int
_bcm_c3_oam_endpoint_type_supported(bcm_oam_endpoint_info_t *endpoint_info)
{
    switch (endpoint_info->type)
    {
    case bcmOAMEndpointTypeEthernet:
        return BCM_E_NONE;
    case bcmOAMEndpointTypeMPLSPerformance:  
    case bcmOAMEndpointTypePSC:
    case bcmOAMEndpointTypePwStatus:
    default:
        return BCM_E_FAIL;
    }
    return BCM_E_FAIL;
}

STATIC 
int _bcm_c3_oam_validate_endpoint(int unit, 
                                  bcm_oam_endpoint_info_t *endpoint_info)
{
  int rv  COMPILER_ATTRIBUTE((unused));

    BCM_INIT_FUNC_DEFS;

    rv = BCM_E_PARAM;
    if(!endpoint_info) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Null endpoint info\n")));

    } else if (BCM_FAILURE(_bcm_c3_oam_endpoint_type_supported(endpoint_info))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Endpoint type %d not supported\n"), 
                 endpoint_info->type));

    } else if ((endpoint_info->group > OAM_STATE(unit)->max_endpoints) ||
               (endpoint_info->group == 0) ||
               (OAM_STATE(unit)->group_info[endpoint_info->group].state == NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid oam group: %d\n"), endpoint_info->group));

    } else if ((endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) &&
               ((endpoint_info->id == 0) || 
                (endpoint_info->id > OAM_STATE(unit)->max_endpoints)))  {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid endpoint id.  Must supply endpoint ID \n")));

    } else if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE) &&
               ((endpoint_info->local_id == 0) ||
                (endpoint_info->local_id > OAM_STATE(unit)->max_endpoints))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid local_id.  Remote EPs must supply "
                                                "the associated endpoint_id between (0-%d) value (%d)\n"), 
                                       OAM_STATE(unit)->max_endpoints, endpoint_info->local_id));

    } else if ((endpoint_info->gport == BCM_GPORT_TYPE_NONE || endpoint_info->gport == BCM_GPORT_INVALID)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid Gport id 0x%x\n"),endpoint_info->gport));

    } else {
        rv = BCM_E_NONE;
    }
    BCM_RETURN_VAL_EXIT(rv);
exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_cleanup(int unit)
{
    int rv = BCM_E_NONE;
    BCM_INIT_FUNC_DEFS;

    if (OAM_STATE(unit)->endpoint_pool) {
        shr_idxres_list_destroy(OAM_STATE(unit)->endpoint_pool);
        OAM_STATE(unit)->endpoint_pool = 0;
    }

    if (OAM_STATE(unit)->group_pool) {
        shr_idxres_list_destroy(OAM_STATE(unit)->group_pool);
        OAM_STATE(unit)->group_pool = 0;
    }

    if (OAM_STATE(unit)->timer_pool) {
        shr_idxres_list_destroy(OAM_STATE(unit)->timer_pool);
        OAM_STATE(unit)->timer_pool = 0;
    }

    if (OAM_STATE(unit)->mamep_htbl) {
        rv = shr_htb_destroy(&OAM_STATE(unit)->mamep_htbl, NULL);
        if( BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, mamep hash table destroy failure"), unit));
        }
    }
    if (OAM_STATE(unit)->timer_htbl) {
        rv = shr_htb_destroy(&OAM_STATE(unit)->timer_htbl, NULL);
        if( BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, timer_htbl hash table destroy failure"), unit));
        }
    }

    if (OAM_STATE(unit)->endpoint_state) {
        sal_free(OAM_STATE(unit)->endpoint_state);
    }

    if (OAM_STATE(unit)->group_info) {
        uint32 group;

        for (group=0; group < OAM_STATE(unit)->max_endpoints; group++) {
            if  (OAM_STATE(unit)->group_info[group].state) {
                sal_free(OAM_STATE(unit)->group_info[group].state);
                OAM_STATE(unit)->group_info[group].state = NULL;
            }
        }

        sal_free(OAM_STATE(unit)->group_info);
    }

    if (OAM_STATE(unit)->timer_handle) {
        sal_free(OAM_STATE(unit)->timer_handle);
    }

    /* ethernet OAM specific cleanup */
    rv = bcm_c3_oam_enet_cleanup(unit, OAM_STATE(unit)->max_endpoints);


    _bcm_c3_oam_exceptions_set(unit, FALSE /* enable=false */);

    if(OAM_STATE(unit)->mutex) {
        sal_mutex_destroy(OAM_STATE(unit)->mutex);
    }

    sal_free(OAM_STATE(unit));
    OAM_STATE(unit) = NULL;
    BCM_RETURN_VAL_EXIT(rv);
 exit:
    BCM_FUNC_RETURN;
}
STATIC int
_bcm_c3_oam_exceptions_set(int unit, int enable)
{
    int i, rv;
    soc_sbx_g3p1_xt_t exception_table;
    uint32 exc_mismatch_id;
    uint32 exc_peer_not_found_id;
    uint32 exc_no_endpoint_id;
    uint32 exc_unk_type_id;
    uint32 exc_rdi_id;
    uint32 exc_threshold_id;
    uint32 exc_copy_id;
    uint32 exc_param_id;
    uint32 exc_internal_error_id;
    uint32 oam_exception[9];

    rv = soc_sbx_g3p1_exc_oam_mismatch_idx_get(unit, &exc_mismatch_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g3p1_exc_oam_peer_not_found_idx_get(unit, &exc_peer_not_found_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g3p1_exc_oam_no_endpoint_idx_get(unit, &exc_no_endpoint_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g3p1_exc_oam_unk_type_idx_get(unit, &exc_unk_type_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g3p1_exc_oam_rdi_idx_get(unit, &exc_rdi_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_exc_oam_threshold_exceeded_idx_get(unit, &exc_threshold_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_exc_oam_copy_to_host_idx_get(unit, &exc_copy_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_exc_oam_param_change_idx_get(unit, &exc_param_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_exc_oam_internal_error_idx_get(unit, &exc_internal_error_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    oam_exception[0] = exc_mismatch_id;
    oam_exception[1] = exc_peer_not_found_id;
    oam_exception[2] = exc_no_endpoint_id;
    oam_exception[3] = exc_unk_type_id;
    oam_exception[4] = exc_rdi_id;
    oam_exception[5] = exc_threshold_id;
    oam_exception[6] = exc_copy_id;
    oam_exception[7] = exc_param_id;
    oam_exception[8] = exc_internal_error_id;

    for (i=0; i < sizeof(oam_exception)/sizeof(oam_exception[0]); i++) {
        rv = soc_sbx_g3p1_xt_get(unit, oam_exception[i], &exception_table);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* enable them all for now, but "no_endpoint"
           should eventually be just a drop. */
        exception_table.forward = enable;

        rv = soc_sbx_g3p1_xt_set(unit, oam_exception[i], &exception_table);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
    return rv;
}

/* returns a CRC value calculated over the 48 byte MAID */
uint32
bcm_c3_oam_maid_crc32_get(uint8 *maid)
{
  uint32  maid1, maid2, hash1, hash2;
  int byte;

  maid2 = BCM_C3_OAM_MAID_PACK_TO_WORD(&maid[0]);      
  maid1 = BCM_C3_OAM_MAID_PACK_TO_WORD(&maid[4]);     
  hash1 = soc_sbx_g3p1_util_crc32_word(maid1 ^ maid2);     
  for (byte=8; byte<48; byte=byte+8) {
    maid2 = BCM_C3_OAM_MAID_PACK_TO_WORD(&maid[byte]); 
    maid1 = BCM_C3_OAM_MAID_PACK_TO_WORD(&maid[byte+4]);
    hash2 = soc_sbx_g3p1_util_crc32_word(maid1 ^ hash1); 
    hash1 = soc_sbx_g3p1_util_crc32_word(hash2 ^ maid2);
  }  
  return hash1;
}

#else   /* INCLUDE_L3 */
int bcm_caladan3_oam_not_empty;
void
bcm_caladan3_oam_dump_reservation(unit)
{
    return;
}
#endif  /* INCLUDE_L3 */

