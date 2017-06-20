/*
 * $Id: field.c,v 1.418 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Field Processor APIs
 *
 * Purpose:
 *     API for Field Processor (FP) for ROBO family and later.
 *
 *
 */


#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/field.h>
#include <sal/types.h>
#include <bcm/port.h>
#include <bcm/mirror.h>
#include <bcm_int/robo/field.h>
#include <bcm_int/robo/mirror.h>
#include <bcm_int/robo/port.h>
#include <soc/drv.h>
#include <bcm_int/robo_dispatch.h>

#ifdef BCM_FIELD_SUPPORT 
STATIC int robo_prio_set_with_no_free_entries[BCM_MAX_NUM_UNITS]; 

STATIC bcm_field_entry_t last_allocated_eid[BCM_MAX_NUM_UNITS];


/*
 * Macro:
 *     ASSERT_BAD_UNIT (internal)
 * Purpose:
 *     Assert if invalid unit number used.
 * Parameters:
 *     unit - BCM device number
 * Notes:
 *     Results in assert if invalid unit number.
 */
#define ASSERT_BAD_UNIT(unit) \
    assert(0 <= unit && unit < BCM_MAX_NUM_UNITS)

/*
 * Macro:
 *     FIELD_IS_INIT (internal)
 * Purpose:
 *     Check that the unit is valid and confirm that the field functions
 *     are initialized.
 * Parameters:
 *     unit - BCM device number
 * Notes:
 *     Results in return(BCM_E_UNIT), return(BCM_E_UNAVAIL), or
 *     return(BCM_E_INIT) if fails.
 */
#define FIELD_IS_INIT(unit)                                      \
    if (!SOC_UNIT_VALID(unit)) {                                 \
        return BCM_E_UNIT;                                       \
    }                                                            \
    if (!soc_feature(unit, soc_feature_field)) {                 \
        return BCM_E_UNAVAIL;                                    \
    }                                                            \
    if (_field_control[unit] == NULL) {                          \
        LOG_ERROR(BSL_LS_BCM_FP,                                    \
                  (BSL_META("FP Error: unit=%d not initialized\n"), \
                   unit));                                          \
        return BCM_E_INIT;                                       \
    }

/*
 *STATIC  Macro:
 *     FP_LOCK
 * Purpose:
 *     Lock take the Field control mutex
 */
#define FP_LOCK(control) \
    sal_mutex_take((control)->fc_lock, sal_mutex_FOREVER)

/*
 * Macro:
 *     FP_UNLOCK
 * Purpose:
 *     Lock take the Field control mutex
 */
#define FP_UNLOCK(control) \
    sal_mutex_give((control)->fc_lock)


#ifdef BROADCOM_DEBUG
STATIC char *_robo_field_qual_name(bcm_field_qualify_t qid);
STATIC char *_robo_field_action_name(bcm_field_action_t action);
STATIC char *_robo_field_qual_IpType_name(bcm_field_IpType_t type);

STATIC void _robo_field_slice_dump(int unit, char *prefix, _field_slice_t *fs,
                              char *suffix);

STATIC void _robo_field_range_dump(const char *pfx, _field_range_t *fr);
STATIC void _robo_field_entry_phys_dump(int unit, _field_entry_t *f_ent);
 
void _robo_field_qual_list_dump(char *prefix, _qual_info_t *qual_list,
                                  char *suffix);

STATIC int _robo_field_stat_dump(int unit, const _field_entry_t *f_ent);
STATIC void _robo_field_policers_dump(int unit, _field_entry_t *f_ent);
STATIC void _robo_field_action_dump(const _field_action_t *fa);
STATIC void _robo_field_group_status_dump(const bcm_field_group_status_t *gstat);
#endif /* BROADCOM_DEBUG */
STATIC int _robo_field_group_status_calc(int unit, _field_group_t *fg);
int _robo_field_stat_get(int unit, int sid, _field_stat_t **stat_p);
STATIC int _robo_field_entry_stat_detach(int unit, _field_entry_t *f_ent, int stat_id);
STATIC int _robo_field_stage_data_ctrl_deinit(int unit, _field_stage_t *stage_fc);
STATIC int _robo_field_stage_data_ctrl_init(int unit, _field_stage_t *stage_fc);
STATIC int _robo_field_entry_backup(int unit, bcm_field_entry_t entry_id);
STATIC int _robo_field_entry_restore(int unit, bcm_field_entry_t entry_id);
STATIC int _robo_field_entry_cleanup(int unit, bcm_field_entry_t entry_id);


/*
 * Field control data, one per device.
 */
static _field_control_t         *_field_control[BCM_MAX_NUM_UNITS];


/*
 * Function:
 *     _robo_field_action_alloc
 * Purpose:
 *     Allocate and initialize an action structure.
 * Parameters:
 *     action   - Action to perform (bcmFieldActionXXX)
 *     param0   - Action parameter (use 0 if not required)
 *     param1   - Action parameter (use 0 if not required)
 *     fa (OUT) - pointer to field action structure
 * Returns:
 *     BCM_E_MEMORY - allocation failure
 *     BCM_E_NONE   - Success
 */
STATIC int
_robo_field_action_alloc(bcm_field_action_t action, uint32 param0, uint32 param1,
                    _field_action_t **fa)
{
    *fa = sal_alloc(sizeof (_field_action_t), "field_action");
    if (*fa == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("FP Error: allocation failure for field_action\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(*fa, 0, sizeof (_field_action_t));

    (*fa)->action = action;
    (*fa)->param0 = param0;
    (*fa)->param1 = param1;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_actions_conflict
 * Purpose:
 *     Determine if two actions are compatible. Incompatible actions are ones
 *     that require the same hardware fields written in different manners to 
 *     coexist.
 * Parameters:
 *     action1 - First action
 *     action2 - Second action
 * Returns:
 *     0  if no conflict exists
 *     !0 if conflict exists
 */
STATIC int
_field_actions_conflict(int unit, _field_stage_id_t stage_id, bcm_field_action_t act1, bcm_field_action_t act2)
{
    return DRV_FP_ACTION_CONFLICT(unit, stage_id, act1, act2);
}

/*
 * Function:
 *     _field_actions_conflict_allow
 * Purpose:
 *     For some special cases that even the new and old actions are
 *     determined conflicted in _field_actions_conflict(). 
 * Parameters:
 *     new_act - First action
 *     param0  - param0 of new_act
 *     param1  - param1 of new_act
 *     old_act - Second action structure
 * Returns:
 *     TRUE  if the conflict is acceptable
 *     FALSE if the conflict is not acceptable
 */
STATIC int
_field_actions_conflict_allow(int unit, bcm_field_action_t new_act, 
                        uint32 param0, uint32 param1, _field_action_t *old_act)
{
    bcm_field_action_t act1, act2;

    act1 = new_act;
    act2 = old_act->action;

    /* 
      * Actions that are for out-band and in-band respective should be 
      * acceptable .
      * For example, RpDrop and GpCopyToCpu
      */
    switch(act2) {
        case bcmFieldActionRpCopyToCpu:
        case bcmFieldActionRpRedirectPort:
        case bcmFieldActionRpDrop:
        case bcmFieldActionRpMirrorIngress:
            _FP_ACTIONS_ALLOW(bcmFieldActionGpCopyToCpu);
            _FP_ACTIONS_ALLOW(bcmFieldActionGpRedirectPort);
            _FP_ACTIONS_ALLOW(bcmFieldActionGpDrop);
            _FP_ACTIONS_ALLOW(bcmFieldActionGpMirrorIngress);
            break;
        case bcmFieldActionGpCopyToCpu:
        case bcmFieldActionGpRedirectPort:
        case bcmFieldActionGpDrop:
        case bcmFieldActionGpMirrorIngress:
            _FP_ACTIONS_ALLOW(bcmFieldActionRpCopyToCpu);
            _FP_ACTIONS_ALLOW(bcmFieldActionRpRedirectPort);
            _FP_ACTIONS_ALLOW(bcmFieldActionRpDrop);
            _FP_ACTIONS_ALLOW(bcmFieldActionRpMirrorIngress);
            break;
        default:
            break;
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * If the actions are redirect to cpu and redirect to mirror-to port,
         * they are acceptable.
         */
        if (((act1 == bcmFieldActionRedirect) &&
            (act2 == bcmFieldActionRedirect))  ||
            ((act1 == bcmFieldActionRpRedirectPort) &&
            (act2 == bcmFieldActionRpRedirectPort))){
            if ((param1 == CMIC_PORT(unit)) &&
                (old_act->param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT)) {
                return TRUE;
            } else if ((param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) &&
                (old_act->param1 == CMIC_PORT(unit))) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return FALSE;
}


/*
 * Function:
 *     _field_action_isnot_supported
 *
 * Purpose:
 *     Add action performed when entry rule is matched for a packet
 *
 * Parameters:
 *     unit - BCM device number
 *     action - Action to perform (bcmFieldActionXXX)
 *
 * Returns:
 *     0  - action is supported by device
 *     1  - action is NOT supported by device
 */
STATIC int
_field_action_not_supported(int unit, _field_stage_id_t stage_id, bcm_field_action_t action)
{
    return DRV_FP_ACTION_SUPPORT_CHECK(unit, stage_id, action);
}

STATIC int
_robo_field_group_default_aset_set(int unit, _field_group_t *fg)
{
    /* Set group's aset to all actions supported by stage */

    unsigned action;

    sal_memset(fg->aset.w, 0, sizeof(fg->aset.w));
    
    for (action = 0; action < bcmFieldActionCount; ++action) {
        if (!_field_action_not_supported(unit, fg->stage_id, action)) {
            SHR_BITSET(fg->aset.w, action);
        }
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *     _robo_field_entry_slice_id_set
 *
 * Purpose:
 *     Set the slice id value to the CFP TCAM to determine the slice format
 *
 * Parameters:
 *     unit - BCM device number
 *     f_ent - entry to be set
 *     sliceId - slice id value
 *
 * Returns:
 */
STATIC int
_robo_field_entry_slice_id_set(int unit, _field_stage_id_t stage_id,
                            _field_entry_t *f_ent, uint32 sliceId, uint32 slice_map)
{
    int retval = BCM_E_NONE;
    int mode;

    f_ent->flags = 0;
    mode = 0;
    if (_BCM_FIELD_STAGE_INGRESS == stage_id ) {
        retval =  DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id,
            f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_CHAIN_MODE_GET, sliceId, &mode);
        BCM_IF_ERROR_RETURN(retval);

        retval = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, 
            f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET, sliceId, &slice_map);
        BCM_IF_ERROR_RETURN(retval);
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "%s mode %x\n"),
                   FUNCTION_NAME(),mode));
        if (mode == _DRV_CFP_SLICE_CHAIN) {
            f_ent->flags |= _FP_ENTRY_WITH_CHAIN;
        } else if ( mode == _DRV_CFP_SLICE_CHAIN_SINGLE) {
            f_ent->flags |= _FP_ENTRY_CHAIN_SLICE;
        }         
    } else {
        retval =  BCM_E_NONE;
    }
    return retval;    
}

/*
 * Function:
 *     _robo_field_control_get
 * Purpose:
 *     Lookup a FP control config from a bcm device id.
 * Parameters:
 *     unit -  (IN)BCM unit number.
 *     fc   -  (OUT) Field control structure.   
 * Retruns: 
 *     BCM_E_XXX   
 */
int
_robo_field_control_get(int unit, _field_control_t **fc)
{
    /* Input parameters check. */
    if (NULL == fc) {
        return (BCM_E_PARAM);
    }

    /* Make sure system was initialized. */
    FIELD_IS_INIT(unit);

    /* Fill field control structure. */
    *fc = _field_control[unit];

    return (BCM_E_NONE);
}
/*
 * Function:
 *     _robo_field_stage_control_get
 * Purpose:
 *     Lookup stage FP control config from bcm device id and 
 *     pipeline stage number.
 * Parameters:
 *     unit      -  (IN)BCM unit number.
 *     stage_id  -  (IN)Pipeline stage id number. 
 *     stage_fc  -  (OUT) Stage Field control structure.   
 * Retruns: 
 *     BCM_E_XXX   
 */
int
_robo_field_stage_control_get(int unit, _field_stage_id_t stage_id,
                          _field_stage_t **stage_fc)
{
    _field_stage_t *stage_p;  /* Stages iteration pointer. */

    FIELD_IS_INIT(unit);                                      \

    /* Input parameters check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    /* Check that module was initialized. */
    if (NULL == (_field_control[unit])->stages) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Stage (%d) is not initialized.\n"), 
                   unit, stage_id));
        return (BCM_E_INIT);
    }

    /* Find a stage with stage_id equals to stage. */
    stage_p = (_field_control[unit])->stages;
    while (stage_p) {
        if (stage_p->stage_id == stage_id) {
            break;
        }
        stage_p = stage_p->next;
    }

    if (NULL == stage_p) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unknown pipeline stage (%d).\n"),
                   unit, stage_id));
        return (BCM_E_NOT_FOUND);
    }

    /* Fill stage field control structure. */
    *stage_fc = stage_p;

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_entry_get
 * Purpose:
 *     Lookup a _field_entry_t from a unit ID and slice choice.
 * Parmeters:
 *     unit   - BCM Unit
 *     eid    - Entry ID
 *     slice  - _FP_SLICE_xxx
 *     entry_p - (OUT) Entry lookup result.  
 * Returns:
 *    BCM_E_XXX
 */
int
_robo_field_entry_get(int unit, bcm_field_entry_t eid, _field_entry_t **entry_p, int slice)
{
    _field_control_t    *fc;
    int idx;
    _field_stage_t  *stage_fc;


    /* Input parameters check. */
    if (NULL == entry_p) {
        return (BCM_E_PARAM);
    }
    assert(_FP_SLICE_PRIMARY <= slice && slice <= _FP_SLICE_TERTIARY);

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Iterate over the linked-list of stages */
    stage_fc = fc->stages;
    while (stage_fc) {
        for (idx = 0; idx < stage_fc->tcam_sz; idx++){
            if (stage_fc->field_shared_entries[idx] != NULL) {
                if(stage_fc->field_shared_entries[idx]->eid == eid) {
                    *entry_p =  stage_fc->field_shared_entries[idx];
                    return (BCM_E_NONE);
                }
            }
       }
        stage_fc = stage_fc->next;
    }

    return (BCM_E_NOT_FOUND);
}


/*
 * Function:
 *     _robo_field_entry_prio_cmp
 * Purpose:
 *     Compare two entry priorities
 * Parameters:
 * Returns:
 *     -1 if prio_first <  prio_second
 *      0 if prio_first == prio_second
 *      1 if prio_first >  prio_second
 */
STATIC int
_robo_field_entry_prio_cmp(int prio_first, int prio_second) {
   int retval;

   if (prio_first == prio_second) {
       retval = 0;
   } else if (prio_first < prio_second) {
       retval = -1;
   } else {
       retval = 1;
   }
   LOG_DEBUG(BSL_LS_BCM_FP,
             (BSL_META("_robo_field_entry_prio_cmp (first=0x%x ? second=0x%x) retval=%d\n"),
              prio_first, prio_second, retval));
   return retval;
}

/*
 * Function:
 *     _robo_field_entry_move
 * Purpose:
 *     Move an entry within a slice by "amount" indexes
 *     
 * Parameters:
 *     unit     - BCM device number
 *     f_ent    - entry to be moved
 *     amount   - number of indexes to move + or -
 *     mode  - move the main entry(_BCM_FP_MAIN_ENTRY) or 
 *                     the chained entry (_BCM_FP_CHAIN_ENTRY)
 * Returns:
 *     BCM_E_NONE   - Success
 */
STATIC int
_robo_field_entry_move(int unit, _field_entry_t *f_ent, int amount, int mode) {
    _field_slice_t      *fs;
    uint32      tcam_idx_old, tcam_idx_new;
    _field_entry_stat_t    *f_ent_st; /* Field entry statistics structure.*/
    _field_stat_t       *f_st;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;    
    int rv;

    assert(f_ent != NULL);
    fs = f_ent->fs;
    fg = fs->group;
    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }    
    assert(fs != NULL);

    if (amount == 0) {
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "FP Warning: moving entry=%d, same slice_idx=%d(0x%x)\n"),
                  f_ent->eid, f_ent->slice_idx, f_ent->slice_idx));
        return BCM_E_NONE;
    }

    assert(f_ent->slice_idx < stage_fc->tcam_sz);
    if (mode  == _BCM_FP_MAIN_ENTRY) {
            tcam_idx_old = f_ent->slice_idx;
    } else {
        if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
            tcam_idx_old = f_ent->chain_idx;
        } else {
            tcam_idx_old = f_ent->slice_idx;            
        }
    }
    
    tcam_idx_new = tcam_idx_old + amount;
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: move entry=%d, slice_idx old: %d  new:%d amount %d\n"),
               f_ent->eid, tcam_idx_old, tcam_idx_new, amount));

    if (robo_prio_set_with_no_free_entries[unit] == FALSE) {
        /* assign the tcam_idx_old to drv_entry->id */
        rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, fg->stage_id, f_ent->drv_entry, 
            DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET, tcam_idx_old, NULL);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        f_ent_st = &f_ent->statistic;

        rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, fg->stage_id, f_ent->drv_entry, 
            DRV_FIELD_ENTRY_TCAM_MOVE, amount, ((f_ent_st->flags & _FP_ENTRY_STAT_VALID) ?\
            f_ent_st : NULL));
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        if (0 != (f_ent_st->flags & _FP_ENTRY_STAT_VALID)) {
            /* Get statistics entity descriptor. */
            rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
            if (BCM_FAILURE(rv)) {
                return (rv);
            }            
            if (_BCM_FP_MAIN_ENTRY == mode) {
                if (0 == (f_ent->flags & _FP_ENTRY_WITH_CHAIN)){                
                    /* update stat except the main slice(slice0) in chain mode*/
                    f_st->hw_index = tcam_idx_new;
                }
            }
            if (_BCM_FP_CHAIN_ENTRY == mode) {  
                /* in chain mode, update the chain slice (slice3) only*/
                f_st->hw_index = tcam_idx_new;
            }
        }
    }

    /* Move the software entry to the new index */           
    if (mode  == _BCM_FP_MAIN_ENTRY) {
        if (robo_prio_set_with_no_free_entries[unit] == FALSE) {
            stage_fc->field_shared_entries[(f_ent)->slice_idx]          = NULL;        
        }

        stage_fc->field_shared_entries[(f_ent)->slice_idx + (amount)] = (f_ent);   

        FIELD_ENTRY_SLICE_IDX_CHANGE(f_ent, amount);
    }
    if (mode  == _BCM_FP_CHAIN_ENTRY) {
        if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
            stage_fc->field_shared_entries[(f_ent)->chain_idx] = NULL;        

            stage_fc->field_shared_entries[(f_ent)->chain_idx + (amount)] = (f_ent);   

            FIELD_ENTRY_CHAIN_IDX_CHANGE(f_ent, amount);
        } else {
            if (robo_prio_set_with_no_free_entries[unit] == FALSE) {
                stage_fc->field_shared_entries[(f_ent)->slice_idx]          = NULL;        
            }
            stage_fc->field_shared_entries[(f_ent)->slice_idx + (amount)] = (f_ent);   
            FIELD_ENTRY_SLICE_IDX_CHANGE(f_ent, amount);
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_entry_shift_down
 * Purpose:
 *     
 * Parameters:
 *     unit     - BCM device number
 * Returns:
 *     BCM_E_NONE   - Success
 */
STATIC int
_robo_field_entry_shift_down(int unit, _field_slice_t *fs, 
    uint16 slice_idx_start, uint16 next_null_idx, int mode) {

    uint16              slice_idx_empty;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;    
    int rv;

    assert(fs != NULL);
    fg = fs->group;
    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }    

    slice_idx_empty = next_null_idx;

    while(slice_idx_empty > slice_idx_start) {
       /* Move the entry at the previous index to the empty index. */
       BCM_IF_ERROR_RETURN(
           _robo_field_entry_move(unit, 
            stage_fc->field_shared_entries[slice_idx_empty - 1], 
            1, mode)); 
       slice_idx_empty--;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_entry_shift_up
 * Purpose:
 *     
 * Parameters:
 *     unit     - BCM device number
 * Returns:
 *     BCM_E_NONE   - Success
 */
STATIC int
_robo_field_entry_shift_up(int unit, _field_slice_t *fs, 
    int slice_idx_start, int prev_null_idx, int mode) {

    uint16              slice_idx_empty;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;
    int rv;

    assert(fs != NULL);
    fg = fs->group;
    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    assert(stage_fc != NULL);

    slice_idx_empty = prev_null_idx;

    while(slice_idx_empty < slice_idx_start) {
       /* Move the entry at the previous index to the empty index. */

       BCM_IF_ERROR_RETURN(
           _robo_field_entry_move(unit, 
            stage_fc->field_shared_entries[slice_idx_empty + 1], 
            -1, mode)); 
       slice_idx_empty++;
    }

    return BCM_E_NONE;
}

int
_robo_field_entry_block_move(int unit, _field_group_t *fg, 
        uint32 src_idx, uint32 dst_idx, uint32 entry_amount, int mode)   
{
    _field_stage_t      *stage_fc;
    int rv;
    int i, amount, stat;
    _field_entry_t  *temp_ent;
    _field_entry_stat_t    *f_ent_st; /* Field entry statistics structure.*/
    _field_stat_t       *f_st = NULL;
    
    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s src_idx %d dst_idx %d entry_amount %d \n"),
               FUNCTION_NAME(), src_idx, dst_idx,entry_amount));

    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, fg->stage_id, &src_idx, 
            DRV_FIELD_ENTRY_TCAM_BLOCK_MOVE, dst_idx, &entry_amount);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    amount = dst_idx - src_idx;    

    if (src_idx  < dst_idx) {
        for (i = (entry_amount - 1); i >= 0; i--) {
            temp_ent = stage_fc->field_shared_entries[src_idx + i];
            stage_fc->field_shared_entries[dst_idx + i] = temp_ent;

            f_ent_st = &temp_ent->statistic;
            stat = 0;

            if (0 != (f_ent_st->flags & _FP_ENTRY_STAT_VALID)) {
                /* Get statistics entity descriptor. */
                rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }            
                stat = 1;
            }
            if (mode == _BCM_FP_MAIN_ENTRY) {
                FIELD_ENTRY_SLICE_IDX_CHANGE(temp_ent, amount);
                if ((stat == 1) && (0 == (temp_ent->flags & _FP_ENTRY_WITH_CHAIN))) {
                    f_st->hw_index = dst_idx+i;
                }
            }
            if (mode == _BCM_FP_CHAIN_ENTRY) {
                if (temp_ent->flags & _FP_ENTRY_WITH_CHAIN){
                    FIELD_ENTRY_CHAIN_IDX_CHANGE(temp_ent, amount);
                } else {
                    FIELD_ENTRY_SLICE_IDX_CHANGE(temp_ent, amount);
                }
                if (stat == 1){
                    f_st->hw_index = dst_idx+i;
                }
            }            
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "stage_fc->field_shared_entries[%d] %p\n"),
                       dst_idx+i, temp_ent));
            stage_fc->field_shared_entries[src_idx+i] = NULL;
        }
    }

    if (src_idx > dst_idx) {
        for (i=0; i < entry_amount; i++) {
            temp_ent = stage_fc->field_shared_entries[src_idx + i];
            stage_fc->field_shared_entries[dst_idx + i] = temp_ent;
            f_ent_st = &temp_ent->statistic;
            stat = 0;

            if (0 != (f_ent_st->flags & _FP_ENTRY_STAT_VALID)) {
                /* Get statistics entity descriptor. */
                rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }            
                stat = 1;
            }

            if (mode == _BCM_FP_MAIN_ENTRY) {
                FIELD_ENTRY_SLICE_IDX_CHANGE(temp_ent, amount);
                if ((stat == 1) && (0 == (temp_ent->flags & _FP_ENTRY_WITH_CHAIN))) {
                    f_st->hw_index = dst_idx+i;
                }

           }
            if (mode == _BCM_FP_CHAIN_ENTRY) {
                if (temp_ent->flags & _FP_ENTRY_WITH_CHAIN){
                    FIELD_ENTRY_CHAIN_IDX_CHANGE(temp_ent, amount);
                } else {
                    FIELD_ENTRY_SLICE_IDX_CHANGE(temp_ent, amount);
                }
                if (stat == 1){
                    f_st->hw_index = dst_idx+i;
                }

           }            
           LOG_DEBUG(BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "stage_fc->field_shared_entries[%d] %p\n"),
                      dst_idx+i, temp_ent));
           stage_fc->field_shared_entries[src_idx+i] = NULL;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_entry_phys_create
 *
 * Purpose:
 *     Initialize a physical entry structure.
 *
 * Parameters:
 *     unit      - BCM unit
 *     group     - Group ID
 *     entry     - Entry ID
 *     slice_idx - Entry index within slice
 *     fs        - slice that entry resides in
 *
 * Returns
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - _field_entry_t allocation failure
 */   
STATIC _field_entry_t *
_robo_field_entry_phys_create(int unit, bcm_field_entry_t entry, int prio,
                         _field_slice_t *fs)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent = NULL;
    int rv, retval;
    void *drv_entry=NULL;
    uint32          temp, slice_map;


    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: _robo_field_entry_phys_create(entry=%d, prio=%d)\n"),
               entry, prio));

    assert(fs != NULL);   

    /* Get unit FP control structure. */
    rv = _robo_field_control_get(unit, &fc);
    if (BCM_FAILURE(rv)) {
        return NULL;
    }
    
    /* allocate and zero memory for field entry */
    f_ent = sal_alloc(sizeof (_field_entry_t), "field_entry");
    if (f_ent == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: allocation failure for field_entry\n")));
        return NULL;
    }

    sal_memset(f_ent, 0, sizeof (_field_entry_t));

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, fs->stage_id, 
            DRV_FIELD_ENTRY_MEM_ALLOC, NULL, NULL,&drv_entry);

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: DRV_FIELD_ENTRY_MEM_ALLOC %p\n"),
               drv_entry));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: allocation failure for field drv entry\n")));
        sal_free(f_ent);
        return NULL;
    }

    f_ent->drv_entry =  drv_entry;

    /* Fill in the basic fields. */
    f_ent->unit         = unit;
    f_ent->eid          = entry;
    f_ent->prio         = prio;
    f_ent->color_indep  = fc->color_indep;
    f_ent->fs           = fs;
    f_ent->group    = fs->group;
    f_ent->flags = _FP_ENTRY_DIRTY;

    /* Check for external TCAM slice. */
    f_ent->ext = 0;
    f_ent->slice_id =  fs->sel_codes.fpf;
    /* Set the slice id to tcam */
    temp = fs->sel_codes.fpf;
    slice_map = fs->sel_codes.slice_map;

    rv = _robo_field_entry_slice_id_set(unit, fs->stage_id, f_ent, temp, slice_map);   

    if (BCM_FAILURE(rv)){        
        if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
            retval = DRV_FP_ENTRY_TCAM_CONTROL(unit, fs->stage_id,
                f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY, 0, NULL);
            if (BCM_FAILURE(retval) && (retval != BCM_E_UNAVAIL)){
                if (f_ent->drv_entry != NULL) {
                    sal_free(f_ent->drv_entry);
                }        
                sal_free(f_ent);
                return NULL;
            }        
        }
        if (f_ent->drv_entry != NULL) {
            sal_free(f_ent->drv_entry);
        }        
        sal_free(f_ent);
        return NULL;
    }
    return f_ent;
}


/*
 * Function:
 *     _field_entry_qual_find
 *
 * Purpose:
 *     Lookup a _field_entry_t from a unit ID taking into account mode. Since
 *     double and triple-wide modes can have multiple physical entries with the
 *     same Entry ID, 
 *
 * Parmeters:
 *     unit   - BCM Unit
 *     eid    - Entry ID
 *     qual   - Qualifier used to select the correct slice
 *
 * Returns:
 *     Pointer to entry struct with matching Entry ID, or NULL if no match
 */
STATIC _field_entry_t *
_field_entry_qual_find(int unit, bcm_field_entry_t entry,
                       bcm_field_qualify_t qual) {
    _field_entry_t      *f_ent;
    _field_group_t      *fg;
    int     rv;

    rv = _robo_field_entry_get(unit, entry, &f_ent,_FP_SLICE_PRIMARY);

    if (BCM_FAILURE(rv)) {
        return NULL;
    }


    fg = f_ent->fs->group;

    if (BCM_FIELD_QSET_TEST(fg->qset, qual)) {
        /* Use Primary Slice */
        return f_ent;
    }

    return NULL;
}

int
_robo_field_entry_qual_get(int unit, bcm_field_entry_t entry,
                       bcm_field_qualify_t qual, _field_entry_t **entry_p)
{
    int     rv;
    _field_group_t      *fg;
    uint8           found;
        
    if (NULL == entry_p) {
        return (BCM_E_PARAM);
    }
    
    found = FALSE;

    rv = _robo_field_entry_get(unit, entry, entry_p,_FP_SLICE_PRIMARY);
    BCM_IF_ERROR_RETURN(rv);

    fg = (*entry_p)->group;
    if (NULL == fg) {
        return (BCM_E_INTERNAL);
    }

    if (BCM_FIELD_QSET_TEST(fg->qset, qual)) {
        /* Use Primary Slice */
        found = TRUE;
    }
    if (FALSE == found) {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_entry_tcam_idx_get (INTERNAL)
 *
 * Purpose:
 *     Get the TCAM index of an entry ID.
 *
 * Parameters:
 *     fc     - pointer to field control structure
 *     f_ent  - pointer to field entry structure
 *
 * Returns:
 *     slice number for entry
 *
 * Note:
 *    Assumes that the entry exists. Will assert if entry is not valid.
 */
int
_robo_field_entry_tcam_idx_get(_field_stage_t *stage_fc, _field_entry_t *f_ent,
    int *tcam_idx, int *tcam_chain_idx)
{

    assert(stage_fc != NULL);
    assert(f_ent != NULL);
    assert(f_ent->slice_idx < stage_fc->tcam_sz);

    *tcam_idx = f_ent->slice_idx;
    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        *tcam_chain_idx = f_ent->chain_idx;
    } else {
        *tcam_chain_idx = _FP_INVALID_INDEX;
    }

    return BCM_E_NONE; 
}

/*
 * Function:
 *     _field_tcam_policy_clear
 *
 * Purpose:
 *     Clear a combined TCAM and POLICY_TABLE entry. This clears the actions
 *     and qualifiers from the hardware.
 *
 * Parameters:
 *     unit      - BCM device number
 *     tcam_idx  - TCAM/POLICY_TABLE entry index
 *
 * Returns:
 *     BCM_E_INTERNAL  - Memory Read failure
 *     BCM_E_UNAVAIL   - Device not supported
 *     BCM_E_NONE      - Success
 *
 * Notes:
 */
STATIC int
_field_tcam_policy_clear(int unit, _field_stage_id_t stage_id, int tcam_idx, 
    int tcam_chain_idx) {

    int                 rv = BCM_E_UNAVAIL;

    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, NULL, 
            DRV_FIELD_ENTRY_TCAM_CLEAR, tcam_idx, &tcam_chain_idx);

    return rv;
}


/*
 * Function:
 *     _field_meter_install
 *
 * Purpose:
 *     Install an FP meter into hardware.
 *
 */
STATIC int
_field_meter_install(int unit, _field_stage_id_t stage_id, 
        _field_entry_t *f_ent, int tcam_idx, int tcam_chain_idx) {

    int                 rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s eid %d tcam_idx %d chain_idx %d\n"),
               FUNCTION_NAME(),
               f_ent->eid, tcam_idx, tcam_chain_idx));

    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, f_ent->drv_entry, 
        DRV_FIELD_ENTRY_TCAM_METER_INSTALL, tcam_idx, &tcam_chain_idx);

    return rv;
}

/*
 * Function:
 *     _robo_field_tcam_policy_install
 *
 * Purpose:
 *     Install a combined TCAM and POLICY_TABLE entry. This writes the
 *     qualifiers and actions to the hardware.
 *
 */
STATIC int
_robo_field_tcam_policy_install(int unit, _field_stage_id_t stage_id, 
                _field_entry_t *f_ent, int tcam_idx, int tcam_chain_idx) 
{
    int                 rv = BCM_E_UNAVAIL;
    uint32                 port, slice_map;

    assert(f_ent != NULL);

    /* If it's never been done for the slice, write the field select codes. */
    if ((soc_feature(unit, soc_feature_field_slice_enable)) && 
        (f_ent->fs->inst_flg == 0)) {
        PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            BCM_IF_ERROR_RETURN(DRV_CFP_CONTROL_GET
                (unit, DRV_CFP_SLICE_SELECT, port, &slice_map));
            slice_map |= (0x1 << f_ent->fs->sel_codes.fpf);
            BCM_IF_ERROR_RETURN(DRV_CFP_CONTROL_SET
                (unit, DRV_CFP_SLICE_SELECT, port, slice_map));
        }
    }
    f_ent->fs->inst_flg = 1;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s eid %d tcam_idx %d chain_idx %d\n"),
               FUNCTION_NAME(),f_ent->eid, 
               tcam_idx, tcam_chain_idx));
    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, f_ent->drv_entry, 
        DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL, tcam_idx, &tcam_chain_idx);

    return rv;

}

/*
 * Function:
 *     _robo_field_group_get
 * Purpose:
 *     Lookup a group information on specified bcm device.
 * Parameters:
 *     unit - (IN)BCM device number.
 *     gid  - (IN)Group ID.
 * Returns: 
 *     BCM_E_XXX 
 */
int
_robo_field_group_get(int unit, bcm_field_group_t gid, _field_group_t **group_p)
{
    _field_control_t    *fc;          /* Unit FP control structure. */
    _field_group_t      *fg;          /* Group information.         */

    if (NULL == group_p) {
        return (BCM_E_PARAM);
    }

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Iterate over the groups linked-list looking for a matching Group ID */
    fg = fc->groups;
    while (fg != NULL) {
        if (fg->gid == gid) {
            *group_p = fg;
            return (BCM_E_NONE);
        }
        fg = fg->next;
    }
    /* Group with id == gid not found. */
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *     _field_qualify_support
 *
 * Purpose:
 *     Check if these qualifier set is supported for this chip 
 *
 * Parameters:
 *     unit           - BCM unit
 *     qset           - client qualifier set
 *
 * Returns:
 *     BCM_E_UNAVAIL  - No support
 *     BCM_E_NONE     - Success
 */
int
_field_qualify_support(int unit, _field_stage_id_t stage_id, bcm_field_qset_t qset)
{
    int rv = BCM_E_NONE;
    int i, udf_num = 0;
    
    /* Get the max number of UDFS */
    rv= DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_UDFS_NUM, (uint32 *) &udf_num);
    if (rv < 0) {
        udf_num = 0;
    }

    /* UDF checking */
    for (i=0;i < BCM_FIELD_USER_NUM_UDFS; i++) {
        if (SHR_BITGET(qset.udf_map, i)) {
            if (i >= udf_num) {
                return BCM_E_UNAVAIL;
            }
        }
    }

   rv =  DRV_FP_QUALIFY_SUPPORT(unit, stage_id, &qset);

   return rv;
}

/*
 * Function:
 *     _field_selcode_single_get
 *
 * Purpose:
 *     Calculate the FPFx select codes from a qualifier set for a single slice.
 *
 * Parameters:
 *     unit           - BCM unit
 *     stage_id    - INGRESS/LOOKUP/EGRESS
 *     mode        - single or double
 *     qset           - client qualifier set
 *     sel            - (OUT)
 *
 * Returns:
 *     BCM_E_UNAVAIL  - No select code will satisfy qualifier set
 *     BCM_E_NONE     - Success
 */
STATIC int
_field_selcode_mode_get(int unit, _field_stage_id_t stage_id,
        bcm_field_qset_t qset,  int mode, int8 *slice_id, uint32 *slice_map)
{
    int                 retval = BCM_E_UNAVAIL;
    uint32 flags = 0;
    void *drv_entry = NULL;

    retval = _field_qualify_support(unit, stage_id, qset);
    if (retval != BCM_E_NONE) {
        return retval;
    }
    if (mode == bcmFieldGroupModeDouble){
        if (stage_id  != _BCM_FIELD_STAGE_INGRESS) {
            return retval;
        }
    }

    if (mode == bcmFieldGroupModeSingle) {
        flags =  DRV_FIELD_GROUP_MODE_SINGLE;
    } else if (mode == bcmFieldGroupModeDouble){
        flags =  DRV_FIELD_GROUP_MODE_DOUBLE;
    } else if (mode == bcmFieldGroupModeAuto){
        flags =  DRV_FIELD_GROUP_MODE_AUTO;
    } else {
        return BCM_E_UNAVAIL;
    }
    
    retval = DRV_FP_SELCODE_MODE_GET(unit, stage_id, &qset, flags, slice_id, slice_map, &drv_entry);
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP:_field_selcode_mode_get slice_id %d rv %d %p\n"),
               *slice_id,retval,drv_entry));
    if (BCM_FAILURE(retval)) {
        /* for 53115 furtherly check if the qual can be composed by udf*/
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            drv_cfp_entry_t temp_drv_entry;
            drv_cfp_qual_udf_info_t qual_udf_info;
            int i, j, k;
            uint32 qual_diff, temp;
            _field_control_t    *fc;
            /* Get unit FP control structure. */
            BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
            assert(drv_entry!=NULL);
            flags = DRV_FIELD_QUAL_REPLACE_BY_UDF;
            if (mode == bcmFieldGroupModeDouble) {
                flags |= DRV_FIELD_QUAL_CHAIN;
            }
            /* If the qualify is not support in a slice,*/
            /* using UDF to achieve it */
            retval = DRV_CFP_SLICE_ID_SELECT(unit, 
                drv_entry, &temp, flags);
            if (retval == BCM_E_NONE) {
                /* find the qualify which will be replaced by UDF */
                /* Get the old qualify set of this slice */
                retval = DRV_CFP_SLICE_TO_QSET(unit, 
                                temp, &temp_drv_entry);
                if (BCM_SUCCESS(retval)) {
                     for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
                        /* Qualifys that need support by UDF */
                         /* Assert validates the input for NULL */
                         /* coverity[var_deref_op : FALSE] */
                        qual_diff  = ((drv_cfp_entry_t *)drv_entry)->w[i] & ~(temp_drv_entry.w[i]);
                        if (qual_diff) {
                            for (j=0; j < 32; j++) {
                                if (qual_diff & (0x1 << j)) {
                                    sal_memset(&qual_udf_info, 0, 
                                        sizeof(drv_cfp_qual_udf_info_t));
                                    retval = DRV_CFP_SUB_QUAL_BY_UDF(unit,
                                        1, temp, (i * 32 + j), &qual_udf_info);
                                    if (retval < 0) {
                                        break;
                                    }                                        
                                    for (k=0; k < qual_udf_info.udf_num; k++) {
                                        fc->udf[qual_udf_info.udf_index[k]].\
                                            valid = 1;
                                        fc->udf[qual_udf_info.udf_index[k]].\
                                            use_count = 0;
                                        fc->udf[qual_udf_info.udf_index[k]].\
                                            slice_id = temp;
                                    }                                        
                                }
                            }
                        }
                    }                        
                }
                *slice_id = temp;
                LOG_DEBUG(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP:53115 field_selcode_mode_get slice_id %d rv %d %p\n"),
                           *slice_id,retval,drv_entry));
                /* Need handle failed case */
            }
        }
    }
    if (drv_entry) {
        sal_free(drv_entry);
    }
    return retval;
}

/*
 * Function:
 *     _field_selcode_get
 *
 * Purpose:
 *     Calculate the FPFx select codes from a qualifier set and group mode.
 *
 * Parameters:
 *     unit           - BCM unit
 *     qset           - client qualifier set
 *     fg             - Group structure
 *
 * Returns:
 *     BCM_E_PARAM    - mode unknown
 *     BCM_E_RESOURCE - No select code will satisfy qualifier set
 *     BCM_E_NONE     - Success
 *
 * Notes:
 *     Calling function is responsible for ensuring appropriate slices
 *     are available.
 */
STATIC int
_field_selcode_get(int unit, bcm_field_qset_t qset, _field_group_t *fg) {
    int                 retval = BCM_E_UNAVAIL;

    assert(fg != NULL);

    switch(fg->mode) {
        case bcmFieldGroupModeSingle:
        case bcmFieldGroupModeAuto:
            BCM_FIELD_QSET_INIT(fg->slices[0].qset);
            retval = _field_selcode_mode_get(unit, fg->stage_id, qset, fg->mode,
                                               &fg->slices[0].sel_codes.fpf, &fg->slices[0].sel_codes.slice_map);
            if (BCM_FAILURE(retval)) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: Failure in _field_selcode_get()\n")));
            }
            sal_memcpy(&fg->slices[0].qset, &qset, sizeof(bcm_field_qset_t));
            break;
        case bcmFieldGroupModeDouble:
            if (SOC_IS_TBX(unit) || SOC_IS_VULCAN(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                BCM_FIELD_QSET_INIT(fg->slices[0].qset);
                retval = _field_selcode_mode_get(unit, fg->stage_id, qset, bcmFieldGroupModeDouble,
                                               &fg->slices[0].sel_codes.fpf, &fg->slices[0].sel_codes.slice_map);
                if (BCM_FAILURE(retval)) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "FP Error: Failure in _field_selcode_get() bcmFieldGroupModeDouble\n")));
                }
                sal_memcpy(&fg->slices[0].qset, &qset, sizeof(bcm_field_qset_t));
            } else {
                retval = BCM_E_UNAVAIL;
            }
            break;        
        case bcmFieldGroupModeTriple:
        case bcmFieldGroupModeQuad:
            return retval;
        default: 
            return BCM_E_PARAM;
    }
    return retval;
}

/*
 * Function:
 *     _field_selcode_to_qset
 * 
 * Purpose:
 *     Get the qset that represents all the qualifiers supported by
 *     the given field selection codes.
 *
 * Parameters:
 *     unit     - BCM device number
 *     selcodes - Field Selection codes
 *     qset     - (OUT) Field qualifier set
 *
 * Returns:
 *     BCM_E_NONE - Success
 */
STATIC int
_field_selcode_to_qset(int unit, _field_stage_id_t stage_id,
    _field_sel_t selcodes, bcm_field_qset_t *qset)
{
    int rv;
    uint32 slice_id;
    
    slice_id = selcodes.fpf;
    BCM_FIELD_QSET_INIT(*qset);
    
    rv = DRV_FP_SELCODE_TO_QSET(unit, stage_id, slice_id, qset);
    
    return rv;
}

/*
 * Function:
 *     _robo_field_group_stage_get
 *
 * Purpose:
 *     Extract group pipeline stage from qualifiers set.
 *
 * Parameters:
 *     unit    - (IN)BCM device number.
 *     qset_p  - (IN)Group qualifiers set. 
 *     stage   - (OUT)Pipeline stage id.  
 *
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_group_stage_get(int unit, bcm_field_qset_t *qset_p, 
                       _field_stage_id_t *stage)
{
    int stage_count = 0;   /* Check that only 1 stage specified in qset */

    /* Input parameters check. */
    if ((NULL == qset_p) || (NULL == stage)) {
        return (BCM_E_PARAM);
    }

    if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageExternal)) {
        return BCM_E_UNAVAIL;
    }

    if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageIngress)) {
        *stage  = _BCM_FIELD_STAGE_INGRESS;
        stage_count++;
    }

    /* Set stage based on qualifiers set. */
    if (soc_feature(unit, soc_feature_field_multi_stage)) {
        if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageEgress)) {
            *stage  = _BCM_FIELD_STAGE_EGRESS;
            stage_count++;
        }
        if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageLookup)) {
            *stage  = _BCM_FIELD_STAGE_LOOKUP;     
            stage_count++;
        }
    } else {
        if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageEgress)) {
            return BCM_E_UNAVAIL;
        }
        if(BCM_FIELD_QSET_TEST(*qset_p, bcmFieldQualifyStageLookup)) {
            return BCM_E_UNAVAIL;
        }
    }

    if (stage_count == 0) {
        /* Default to ingress. */
        *stage = _BCM_FIELD_STAGE_INGRESS;
        stage_count++;
    }

    if (stage_count > 1) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: More than one pipeline stage was specified.\n"),
                   unit));
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function: 
 *     _robo_field_group_qset_update
 * Purpose:
 *     Update application requested qset with internal qualifiers.
 * Parameters:
 *     unit  - (IN)     BCM device number.
 *     fg    - (IN/OUT) Field group structure. 
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_group_qset_update(int unit, _field_group_t *fg)
{
    /* Input parameters check. */

    if (NULL == fg) {
        return (BCM_E_PARAM);
    }

    /* All ingress devices implicitly have bcmFieldQualifyStage in Qsets.*/
    if (_BCM_FIELD_STAGE_INGRESS == fg->stage_id) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyStage);
    }

    /*
     * PacketFormat qualifiers was deprecated & replaced 
     * with VlanFormat , IpType.
     */
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyPacketFormat)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyL2Format);
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyVlanFormat);
    }
    /*
     * Automatically include IpType 
     * if the qset contains PacketFormat/Ip4/Ip6.
     */
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyIp4)||
        BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyIp6)||
        BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyPacketFormat)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyIpType);
    }

    /*
     * Vlan qualifiers ared extended to Id/Cfi/Pri
     */
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyOuterVlan)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyOuterVlanId);
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyOuterVlanPri);
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyOuterVlanCfi);
    }
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyInnerVlan)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyInnerVlanId);
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyInnerVlanPri);
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyInnerVlanCfi);
    }

    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifySrcIp6High)||
        BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifySrcIp6Low)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifySrcIp6);
    }
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyDstIp6High)||
        BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyDstIp6Low)) {
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyDstIp6);
    }

    return (BCM_E_NONE);

}

/* 
 * Function:
 *     _robo_field_group_id_generate
 *
 * Purpose:
 *     Find an unused Group ID.
 *
 * Parameters:
 *     unit  - BCM device number
 *     qset - qualify set
 *     group - (OUT) new Group ID
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - null pointer to group
 */
STATIC int
_robo_field_group_id_generate(int unit, bcm_field_qset_t qset, 
        bcm_field_group_t *group, int pri) {

#ifdef SLICE_SUPPORT_MULTI_GROUPS
    _field_control_t    *fc;
    _field_group_t      *fg = NULL;
    int8        slice_id;
    uint32 slice_map;
    int         rv = BCM_E_NONE;
    bcm_field_qset_t    qset_union;
#endif
    _field_group_t *group_p;  /* Group info. */

     if (group == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: group == NULL\n")));
        return BCM_E_PARAM;
    }

#ifdef SLICE_SUPPORT_MULTI_GROUPS
    /* 
     * If pri == BCM_FIELD_GROUP_PRIO_ANY,
     * search the existed groupes.
     * If the group can support the input qset, return this exieted group id.
     */
    if (pri ==BCM_FIELD_GROUP_PRIO_ANY) {     
        /* Check if group(slice id) already created */
    /* Get unit FP control structure. */
        BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

        rv = _field_selcode_mode_get(unit, fg->stage_id, qset, bcmFieldGroupModeSingle,
                                               &slice_id, &slice_map);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: new Qset won't work.\n")));
            return rv;
        }

        fg = fc->groups;
        while(fg != NULL) {
            if (fg->slices[0].sel_codes.fpf == slice_id) {
                *group = fg->gid;
                sal_memset(&qset_union, 0, sizeof(bcm_field_qset_t));
                /* merge qset to this group */
                _robo_field_qset_union(&fg->slices[0].qset,
                      &qset, &qset_union);
                sal_memcpy(&fg->qset, &qset_union, sizeof(bcm_field_qset_t));
                sal_memcpy(&fg->slices[0].qset, &qset_union, 
                    sizeof(bcm_field_qset_t));
                rv = BCM_E_EXISTS;
                return rv;
            }
            fg = fg->next;
        }
    }

#endif


    *group = _FP_GROUP_ID_BASE;
    while (!BCM_FAILURE(_robo_field_group_get(unit, *group, &group_p))) {
        *group += 1;
    }

    return BCM_E_NONE;
}

/* 
 * Function:
 *     _robo_field_group_status_init
 * Purpose:
 *     Fill in the initial fields of a group status struct.
 * Parameters:
 *     unit  -  (IN)BCM unit number.
 *     fg_stat -  (OUT)Initialized field group status structure.   
 * Retruns: 
 *     BCM_E_XXX   
 */
STATIC int
_robo_field_group_status_init(int unit, bcm_field_group_status_t *fg_stat) {

    if (NULL == fg_stat) {
        return (BCM_E_PARAM);
    }
    sal_memset(fg_stat, 0,  sizeof(bcm_field_group_status_t));
    fg_stat->prio_min       = 0;
    fg_stat->prio_max       = BCM_FIELD_ENTRY_PRIO_HIGHEST;

    fg_stat->entries_total  = _FP_INVALID_INDEX;
    fg_stat->entries_free   = _FP_INVALID_INDEX;
    fg_stat->counters_total  = _FP_INVALID_INDEX;
    fg_stat->counters_free   = _FP_INVALID_INDEX;
    fg_stat->meters_total  = _FP_INVALID_INDEX;
    fg_stat->meters_free   = _FP_INVALID_INDEX;
    return (BCM_E_NONE);
}


/*
 * Function:
 *     _field_qset_union
 *
 * Purpose:
 *     make a union of two qsets
 *
 * Parameters:
 *     qset1      - source 1
 *     qset2      - source 2
 *     qset_union - (OUT) result of union operation
 *
 * Returns:
 *     BCM_E_NONE - Success
 */
int
_robo_field_qset_union(const bcm_field_qset_t *qset1,
                  const bcm_field_qset_t *qset2,
                  bcm_field_qset_t *qset_union)
{
    int                    idx;

    assert(qset_union != NULL);

    /* Perform the union of the qualifier bitmap. */
    for (idx = 0; idx < _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX); idx++) {
        qset_union->w[idx] = qset1->w[idx] | qset2->w[idx];
    }

    /* Perform the union of the qualifier udfs. */
    for (idx = 0; idx < _SHR_BITDCLSIZE(BCM_FIELD_USER_NUM_UDFS); idx++) {
        qset_union->udf_map[idx] = qset1->udf_map[idx] | qset2->udf_map[idx];
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *     _field_qset_is_subset
 * Purpose:
 *     Determine if qset one is a subset of qset two.
 * Parameters:
 *     qset_1
 *     qset_2
 * Returns:
 *     1 - if 'qset_1' is a subset of 'qset_2'
 *     0 - if 'qset_1' is NOT a subset of 'qset_2'
 */
int
_robo_field_qset_is_subset(const bcm_field_qset_t *qset_1,
                      const bcm_field_qset_t *qset_2)
{
    int                 idx;

    for (idx = 0; idx < _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX); idx++) {
        if ((qset_1->w[idx] | qset_2->w[idx]) & ~qset_2->w[idx]) {
            return 0;
        }
    }
    return 1;
}

/*
 * Function:
 *     _field_qual_list_destroy (INTERNAL)
 *
 * Purpose:
 *     Frees the memory for a qualifier linked list.
 *
 * Returns:
 *     None
 */
void
_robo_field_qual_list_destroy(_qual_info_t **f_qual_p)
{
    _qual_info_t  *f_qual, *f_qual_old;

    f_qual = *f_qual_p;

    while (f_qual != NULL) {
        f_qual_old = f_qual;
        f_qual     = f_qual->next;
        sal_memset(f_qual_old, 0, sizeof(_qual_info_t));
        sal_free(f_qual_old);
    }
    *f_qual_p = NULL; /* Null out old list pointer. */
}


/*
 * Function:
 *     _robo_field_qual_value32_set
 * Purpose:
 *     Set a qualifier field with the designated data and mask
 *     (<=32-bit) values.
 */
int
_robo_field_qual_value32_set(int unit, bcm_field_entry_t entry,
    bcm_field_qualify_t qual, uint32 data, uint32 mask)
{
    uint32  fld_data, fld_mask;
    
#ifdef BROADCOM_DEBUG
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP: setting qual (%s) values data=0x%x, mask=0x%x\n"), 
                 _robo_field_qual_name(qual),data, mask));
#endif
    fld_data = data;
    fld_mask = mask;

    return _robo_field_qual_value_set(unit, entry, qual, &fld_data, &fld_mask);
}
/*
 * Function:
 *     _field_qual_value_set
 *
 * Purpose:
 *     Set a qualifier field from the designated data and mask arrays.
 *
 * Parameters:
 *     
 *     f_ent - tcam.f0, f1, f2, f3 or ext are outputs.
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - data or mask too big for field
 */
int
_robo_field_qual_value_set(int unit, bcm_field_entry_t entry,  bcm_field_qualify_t qual,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = BCM_E_NONE;
    _field_group_t  *fg;
    _field_entry_t *f_ent;
    int slice_id;

    rv = _robo_field_entry_qual_get(unit, entry, qual, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
        
    assert(p_data != NULL);
    assert(p_mask != NULL);

    fg = f_ent->group;

    rv = DRV_FP_QUAL_VALUE_SET(unit, fg->stage_id, qual, f_ent->drv_entry, p_data,p_mask);

    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    if (rv > 0) {
        slice_id = rv - 1;
        f_ent->slice_id = slice_id;
        rv = BCM_E_NONE;
    }

    f_ent->flags |= _FP_ENTRY_DIRTY;
    
    return rv;
}

/*
 * Function:
 *     _field_udf_value_set
 *
 * Purpose:
 *     Set a UDF qualifier field from the designated data and mask arrays.
 *
 * Parameters:
 *     
 *     f_ent - tcam.f0, f1, f2, f3 or ext are outputs.
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - data or mask too big for field
 */
int
_robo_field_udf_value_set(int unit,  bcm_field_entry_t entry, uint32 udf_idx,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = BCM_E_NONE;
    _field_group_t  *fg;
    _field_entry_t *f_ent;
    int slice_id;
    
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    assert(p_data != NULL);
    assert(p_mask != NULL);

    fg = f_ent->group;

    rv = DRV_FP_UDF_VALUE_SET(unit, fg->stage_id, udf_idx, f_ent->drv_entry, p_data,p_mask);

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (rv > 0) {
        slice_id = rv - 1;
        f_ent->slice_id = slice_id;
        rv = BCM_E_NONE;
    }
    f_ent->flags |= _FP_ENTRY_DIRTY;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_udf_value_get
 *
 * Purpose:
 *     Get a UDF qualifier field from the designated data and mask arrays.
 *
 * Parameters:
 *     
 *     f_ent - tcam.f0, f1, f2, f3 or ext are outputs.
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - data or mask too big for field
 */
int
_robo_field_udf_value_get(int unit,  bcm_field_entry_t entry, uint32 udf_idx,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = BCM_E_NONE;
    _field_group_t  *fg;
    _field_entry_t *f_ent;

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    assert(p_data != NULL);
    assert(p_mask != NULL);

    fg = f_ent->group;

    rv = DRV_FP_UDF_VALUE_GET(unit, fg->stage_id, udf_idx, f_ent->drv_entry, p_data,p_mask);

    if (BCM_FAILURE(rv)) {
        return rv;
    }
    f_ent->flags |= _FP_ENTRY_DIRTY;
    
    return BCM_E_NONE;
}


/*
 * Function:
 *     _robo_field_qual_value32_get
 * Purpose:
 *     Set a qualifier field with the designated data and mask
 *     (<=32-bit) values.
 */
int
_robo_field_qual_value32_get(int unit, bcm_field_entry_t entry,
    bcm_field_qualify_t qual, uint32 *data, uint32 *mask)
{
    uint32 q_data[4];
    uint32 q_mask[4];
    int     rv = BCM_E_NONE;

    rv = _robo_field_qual_value_get(unit, entry, qual, &q_data[0], &q_mask[0]);
    if (BCM_SUCCESS(rv)) {
        *data = q_data[0];
        *mask = q_mask[0];
    }
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_field_qual_value32_get : data= 0x%x, mask =0x%x\n"),
                 *data, *mask));

    return rv;
}
/*
 * Function:
 *     _robo_field_qual_value16_get
 * Purpose:
 *     Set a qualifier field with the designated data and mask
 *     (<=32-bit) values.
 */
int
_robo_field_qual_value16_get(int unit, bcm_field_entry_t entry,
    bcm_field_qualify_t qual, uint16 *data, uint16 *mask)
{
    uint32 q_data[4];
    uint32 q_mask[4];
    int     rv = BCM_E_NONE;

    rv = _robo_field_qual_value_get(unit, entry, qual, &q_data[0], &q_mask[0]);

    if (BCM_SUCCESS(rv)) {
        *data = (uint16)(q_data[0] & 0xffff);
        *mask = (uint16)(q_mask[0] & 0xffff);
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_field_qual_value16_get : data= 0x%x, mask =0x%x\n"),
                 *data, *mask));

    return rv;
}
/*
 * Function:
 *     _robo_field_qual_value8_get
 * Purpose:
 *     Set a qualifier field with the designated data and mask
 *     (<=32-bit) values.
 */
int
_robo_field_qual_value8_get(int unit, bcm_field_entry_t entry,
    bcm_field_qualify_t qual, uint8 *data, uint8 *mask)
{
    uint32 q_data[4];
    uint32 q_mask[4];
    int     rv = BCM_E_NONE;

    rv = _robo_field_qual_value_get(unit, entry, qual, &q_data[0], &q_mask[0]);

    if (BCM_SUCCESS(rv)) {
        *data = (uint8)(q_data[0] & 0xff);
        *mask = (uint8)(q_mask[0] & 0xff);
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_field_qual_value8_get : data= 0x%x, mask =0x%x\n"),
                 *data, *mask));

    return rv;
}

int
_robo_field_qual_value_get(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual, 
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = BCM_E_NONE;
    _field_group_t  *fg;
    _field_entry_t *f_ent;
    
    rv = _robo_field_entry_qual_get(unit, entry, qual, &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    assert(p_data != NULL);
    assert(p_mask != NULL);

    fg = f_ent->group;

    rv = DRV_FP_QUAL_VALUE_GET(unit, fg->stage_id, qual, f_ent->drv_entry, p_data, p_mask);

    return rv;
}

/*
 * Function:
 *     _field_group_prio_make
 *
 * Purpose:
 *     Generate an arbitrary priority choice if none is given.
 *
 * Returns:
 *     BCM_E_RESOURCE - no group available
 *     BCM_E_NONE     - Success
 */
STATIC int
_field_group_prio_make(_field_stage_t *stage_fc, int *pri)
{
    int                 prior;

    assert(stage_fc!= NULL);

    for (prior = (stage_fc->tcam_slices - 1); prior > 0; prior--) {
    /* get the free priority from the max*/ 
        if (stage_fc->slices[prior].group == NULL) {
            *pri = prior;
            return BCM_E_NONE;
        }
    }

    LOG_ERROR(BSL_LS_BCM_FP,
              (BSL_META("FP Error: No Group available.\n")));
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *     _field_slice_clear
 *
 * Purpose:
 *     Reset the fields in a slice. Note that the entries list must be
 *     empty before calling this. Also, this does NOT deallocate the memory for
 *     the slice itself. Normally, this is used when a group no longer needs
 *     ownership of a slice so the slice gets returned to the pool of available
 *     slices.
 *
 * Paramters:
 *     unit - BCM device number
 *     fs   - Link to physical slice structure to be cleared
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_BUSY      - Entries still in slice, can't clear slice
 */
STATIC int
_robo_field_slice_clear(int unit, _field_control_t *fc, _field_slice_t *fs)
{
    _field_stage_t  *stage_fc;
    int rv;
    
    assert(fc != NULL);
    assert(fs != NULL);

    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fs->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }    

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int i;
        int retval;
        _field_group_t *fg;
        int another_slice_use = 0;

        fg = fc->groups;
        while (fg != NULL) {
            if ((fg->slices->sel_codes.fpf == fs->sel_codes.fpf) &&
                (fg->gid != fs->group->gid)) {
                another_slice_use =1;
                break;
            }
            fg = fg->next;
        }

        if (another_slice_use == 0) {
        /* Check if this qualify can be substitute by UDFs*/
        retval = (DRV_SERVICES(unit)->cfp_sub_qual_by_udf)(
            unit, 0, fs->sel_codes.fpf, 0, NULL);
        if (retval < 0) {
            return retval;
        }
        /* Recover the UDF translate by qual */
        for (i = 0; i < BCM_FIELD_USER_NUM_UDFS_MAX; i++) {
            if ((fc->udf[i].valid) && (!fc->udf[i].data_qualifier) && 
                (fc->udf[i].slice_id == fs->sel_codes.fpf)) {
                fc->udf[i].valid = 0;

            }
        }
        }
    }

    fs->group = NULL; /* slice no longer belongs to any group */
    BCM_FIELD_QSET_INIT(fs->qset);
    _FIELD_SELCODE_CLEAR(unit, fs->sel_codes);

    /* No need to clear select codes */

    /* Free slice's qualifier info list */
    /* _field_qual_list_destroy(&fs->qual_list);*/
    fs->qual_list = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_stage_delete
 *
 * Purpose:
 *     Deinitialize field stage.
 *
 * Parameters:
 *     unit      - (IN) BCM device number.
 *     fc        - (IN/OUT)Field control info for device.
 *     stage_fc  - (IN) Stage control structure.  
 *
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_stage_delete(int unit, _field_control_t *fc, _field_stage_t *stage_fc)
{
    _field_stage_t        *stage_iter;    /* Device stages iterator. */
    int rv;
    /* Input parameters check. */
    if (NULL == fc) {
        return (BCM_E_PARAM);
    }

    if (NULL == stage_fc) {
        return (BCM_E_NONE);
    }

    /* Destroy data qualifiers control structure */
    _robo_field_stage_data_ctrl_deinit(unit, stage_fc);

    
    if (NULL != stage_fc->field_shared_entries){
        sal_free(stage_fc->field_shared_entries);
        stage_fc->field_shared_entries = NULL;
    }

    /* Free stage slices info. */
    if (NULL != stage_fc->slices) {
        sal_free(stage_fc->slices);
        stage_fc->slices = NULL;
    }

    /* destroy all ranges */
    while (stage_fc->ranges != NULL) {
        rv = bcm_robo_field_range_destroy(unit, stage_fc->ranges->rid);
        if (BCM_FAILURE(rv)) {
            if (rv != BCM_E_NOT_FOUND) {
                return (rv);
            }
        }
    }

    /* Remove stage from stages linked list. */
    stage_iter = fc->stages;
    while (NULL != stage_iter) {
        if (stage_iter == stage_fc) {
            fc->stages = stage_fc->next;
            break;
        } else if (stage_iter->next == stage_fc) {
            stage_iter->next = stage_fc->next;
            break;
        }
        stage_iter = stage_iter->next; 
    }

    /* Free stage info. */
    sal_free(stage_fc);

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_stages_destroy
 *
 * Purpose:
 *     Deinitialize field stages within the field control struct.
 *
 * Parameters:
 *     unit - (IN) BCM device number.
 *     fc -   (IN/OUT)Field control info for device.
 *
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_stages_destroy(int unit, _field_control_t *fc)
{
    _field_stage_t *stage_fc; /* Stage field control structure. */

    /* Input parameters check. */
    if (fc == NULL) {
        return (BCM_E_PARAM);
    }

    if (NULL != fc->fc_lock) { 
        FP_LOCK(fc);
    }

    /* Free stages & slices structures. */ 
    while (NULL != fc->stages) {
        stage_fc = fc->stages;

        /* Free stage resources & stage itself. */
        _robo_field_stage_delete(unit, fc, stage_fc);
    }

    if (NULL != fc->fc_lock) { 
        FP_UNLOCK(fc);
    }
    return (BCM_E_NONE);
}
/*
 * Function:
 *     _robo_field_tcam_info_init
 * Purpose:
 *     Initialize TCAM related information in Field Control
 *
 * Parameters:
 *     unit     - (IN) BCM device number.
 *     stage_fc - (IN) Stage field control structure pointer. 
 *     fc       - (IN/OUT) Field control infoformation for device.
 *
 * Returns:
 *    BCM_E_XXX 
 */
STATIC int
_robo_field_tcam_info_init(int unit, _field_stage_t *stage_fc,  _field_control_t *fc)
{
    uint32  entry_size;
    uint32  tcam_size;
    int              mem_size;     /* Allocation size. */
    uint32 tcam_slices;


    /* Input parameters check. */
    if ((NULL == fc) || (NULL == stage_fc)) { 
        return (BCM_E_PARAM); 
    }
    entry_size = 0;
    if (_BCM_FIELD_STAGE_INGRESS == stage_fc->stage_id) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_CFP_TCAM_SIZE, &tcam_size));
        entry_size = soc_property_get(unit, spn_BCM_FIELD_ENTRY_SZ, tcam_size);
        stage_fc->tcam_sz = entry_size;
        tcam_slices = soc_property_get(unit, spn_ROBO_INGRESS_TCAM_SLICE, 16);
        stage_fc->tcam_slices = tcam_slices;
        stage_fc->tcam_slice_sz = stage_fc->tcam_sz;
        stage_fc->tcam_bottom = stage_fc->tcam_sz;
    } 
#ifdef BCM_TB_SUPPORT
    else if (soc_feature(unit, soc_feature_field_multi_stage)) {
        if (_BCM_FIELD_STAGE_LOOKUP == stage_fc->stage_id) {            
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_IVM_TCAM_SIZE, &tcam_size));
            stage_fc->tcam_sz = tcam_size;
            tcam_slices = soc_property_get(unit, spn_ROBO_LOOKUP_TCAM_SLICE, 8);
            stage_fc->tcam_slices = tcam_slices;
            stage_fc->tcam_slice_sz = stage_fc->tcam_sz;
            stage_fc->tcam_bottom = stage_fc->tcam_sz;
        } else if (_BCM_FIELD_STAGE_EGRESS == stage_fc->stage_id) {
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_EVM_TCAM_SIZE, &tcam_size));
            stage_fc->tcam_sz = tcam_size;
            tcam_slices = soc_property_get(unit, spn_ROBO_EGRESS_TCAM_SLICE, 8);
            stage_fc->tcam_slices = tcam_slices;
            stage_fc->tcam_slice_sz = stage_fc->tcam_sz;     
            stage_fc->tcam_bottom = stage_fc->tcam_sz;
        } else {
            return (BCM_E_PARAM);
        }
    } 
#endif /* BCM_TB_SUPPORT */    
    else {
        return (BCM_E_PARAM);
    }
    stage_fc->field_shared_entries_free =  stage_fc->tcam_sz;
    mem_size = stage_fc->tcam_sz * sizeof (void *);
    stage_fc->field_shared_entries = sal_alloc(mem_size, "array of entry pointers");
     if (NULL == stage_fc->field_shared_entries ) {
        return (BCM_E_MEMORY);
    }
    sal_memset(stage_fc->field_shared_entries, 0, mem_size);
    fc->tcam_ext_numb = FP_EXT_TCAM_NONE;

    return (BCM_E_NONE);
}


/*
 * Function:
 *     _robo_field_slices_init
 * Purpose:
 *     Allocate the memory for slices. 
 *     Initialize slice specific flags, parameters.
 *
 * Parameters:
 *     unit         - (IN) BCM device number.
 *     stage_fc     - (IN/OUT) Stage field control structure.
 *     fc           - (IN/OUT) Field control structure. 
 *
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_slices_init(int unit, _field_stage_t *stage_fc, _field_control_t *fc)
{
    struct _field_slice_s *fs; /* Slice info.                    */
    int             slice_idx; /* Slice iteration index.         */
    int              mem_size; /* Memory allocation buffer size. */

    /* Input parameters check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    /* Allocate slices info. */
    mem_size = stage_fc->tcam_slices * sizeof(struct _field_slice_s);
    fs = sal_alloc(mem_size, "stage slices info");
    if (NULL == fs) {
        return (BCM_E_MEMORY);
    }

    sal_memset(fs, 0, mem_size);
    stage_fc->slices = fs;

    /* Initialize stage slices info. */
    for (slice_idx = 0; slice_idx < stage_fc->tcam_slices; slice_idx++) {
        fs[slice_idx].slice_numb = slice_idx;
        fs[slice_idx].stage_id = stage_fc->stage_id;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_stage_add
 *
 * Purpose:
 *     Add stage to field processor pipeline.
 *
 * Parameters:
 *     unit - (IN) BCM device number.
 *     fc -   (IN/OUT)Field control info for device.
 *
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Allocation failure
 */
STATIC int
_robo_field_stage_add(int unit, _field_control_t *fc, _field_stage_id_t stage_id)
{
    _field_stage_t *stage_fc; /* Stage info.                 */
    int    ret_val;           /* Operation return value.     */

    /* Input parameters check. */
    if (NULL == fc) {
        return (BCM_E_PARAM);
    }

    /* Allocate stage structure. */
    stage_fc = sal_alloc(sizeof(_field_stage_t), "FP stage info");
    if (NULL == stage_fc) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Allocation failure for stage info\n"),
                   unit));
        return (BCM_E_MEMORY);
    }
    /* Reset allocated buffer. */
    sal_memset(stage_fc, 0, sizeof(_field_stage_t));

    /* Set stage id. */
    stage_fc->stage_id = stage_id;

    if (soc_feature(unit, soc_feature_field_slice_enable)){
        stage_fc->flags |= _FP_STAGE_SLICE_ENABLE;
    } 
    
    /* Init tcam info for stage. */
    ret_val = _robo_field_tcam_info_init(unit, stage_fc, fc);
    if (BCM_FAILURE(ret_val)) {
        _robo_field_stage_delete(unit, fc, stage_fc);
        return (ret_val);
    }

    /* Initialize slices information. */
    ret_val = _robo_field_slices_init (unit, stage_fc, fc);
    if (BCM_FAILURE(ret_val)) {
        _robo_field_stage_delete(unit, fc, stage_fc);
        return (ret_val);  
    }

    
     /* Ingress specific initialization. */
    if (_BCM_FIELD_STAGE_INGRESS == stage_fc->stage_id) {
        
        /* Initialize stage select codes table. */
        ret_val = _robo_field_stage_data_ctrl_init(unit, stage_fc);
        if (BCM_FAILURE(ret_val)) {
            _robo_field_stage_delete(unit, fc, stage_fc);
            return (ret_val);  
        }
    }

    FP_LOCK(fc);
    /* Add stage to field control structure. */
    stage_fc->next = fc->stages;
    fc->stages = stage_fc;
    FP_UNLOCK(fc);    

    return (BCM_E_NONE);
}


/*
 * Function:
 *    _robo_field_stages_init
 *
 * Purpose:
 *     Initialize field stages array within the field control struct.
 *
 * Parameters:
 *     unit - (IN) BCM device number.
 *     fc -   (IN/OUT)Field control info for device.
 *
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Allocation failure
 *
 * Notes:
 *     It is important that _BCM_FIELD_STAGE_INGRESS be inited before
 *         _BCM_FIELD_STAGE_EXTERNAL
 */
STATIC int
_robo_field_stages_init(int unit, _field_control_t *fc)
{
    int            ret_val;    /* Operation return value.     */
   

    /* Input parameters check. */
    if (fc == NULL) {
        return (BCM_E_PARAM);
    }

    ret_val = _robo_field_stage_add(unit, fc, _BCM_FIELD_STAGE_INGRESS);
    if (BCM_FAILURE(ret_val)) {
        _robo_field_stages_destroy(unit, fc);
        return (ret_val);
    }
#ifdef BCM_TB_SUPPORT
    if (soc_feature(unit, soc_feature_field_multi_stage)){
        ret_val = _robo_field_stage_add(unit, fc, _BCM_FIELD_STAGE_LOOKUP);
        if (BCM_FAILURE(ret_val)) {
            _robo_field_stages_destroy(unit, fc);
            return (ret_val);
        }

        ret_val = _robo_field_stage_add(unit, fc, _BCM_FIELD_STAGE_EGRESS);
        if (BCM_FAILURE(ret_val)) {
            _robo_field_stages_destroy(unit, fc);
            return (ret_val);
        }
    }
#endif /* BCM_TB_SUPPORT */
    return (ret_val);
}

/*
 * Function: 
 *     _robo_field_udf_usecount_increment
 *
 * Purpose:
 *     Increment the use counts for any UDFs used
 *
 * Parameters:
 *     fc - unit's field control struct
 *     fg - group's metadata struct
 *
 * Returns:
 *     BCM_E_RESOURCE - Invalid UDF
 *     BCM_E_NONE     - Success
 *
 */
STATIC int
_robo_field_udf_usecount_increment(_field_control_t *fc, _field_group_t *fg) {
    int                 idx;

    for (idx = 0; idx < BCM_FIELD_USER_NUM_UDFS; idx++) {
        if (SHR_BITGET(fg->qset.udf_map, idx)) {
            fc->udf[idx].use_count++;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_entry_phys_destroy
 *
 * Purpose:
 *     Destroy a physical entry from a slice. Note that this does not remove
 *     the entry from the hardware.
 *
 * Parameters:
 *     unit      - BCM device number
 *     fs        - slice that entry resides in
 *     entry     - Entry ID to remove
 *
 * Returns:
 *     BCM_E_NONE - Success
 */   
STATIC int
_robo_field_entry_phys_destroy(int unit, _field_entry_t *f_ent)
{
    _field_slice_t      *fs;
    _field_action_t     *fa;
    _field_stage_t      *stage_fc;
    uint32  next_null_idx, slice_idx_target;
    int rv;

    fs = f_ent->fs;
    assert(fs    != NULL);
    assert(f_ent != NULL);

    BCM_IF_ERROR_RETURN(_robo_field_stage_control_get
        (unit, fs->stage_id, &stage_fc));

    /* Destroy entry policers */
    if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
        rv = _robo_field_entry_stat_detach(unit, f_ent, f_ent->statistic.sid);
        if (BCM_FAILURE(rv)) {
            return(rv);
        }
    }
    /* Remove all actions from entry. */
    fa = f_ent->actions;
    while (fa != NULL) {
        BCM_IF_ERROR_RETURN(bcm_robo_field_action_remove(unit, f_ent->eid, fa->action));
        fa = f_ent->actions;
    }

    /* Destroy entry policers */
    rv = bcm_robo_field_entry_policer_detach_all(unit, f_ent->eid);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    /* Remove entry from slice's entry array. */
    stage_fc->field_shared_entries[f_ent->slice_idx] = NULL;

    if (f_ent->flags & _FP_ENTRY_CHAIN_SLICE) {
        if ( f_ent->slice_idx > stage_fc->tcam_bottom) {
            next_null_idx = f_ent->slice_idx;
            slice_idx_target = stage_fc->tcam_bottom;
            if (soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit,  f_ent->group,
                        slice_idx_target, slice_idx_target+1, 
                        (next_null_idx - slice_idx_target), _BCM_FP_CHAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }
            } else {
                rv = _robo_field_entry_shift_down(unit, fs, 
                    slice_idx_target, next_null_idx, _BCM_FP_CHAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }   
            }
        }
    }

    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        stage_fc->field_shared_entries[f_ent->chain_idx] = NULL;
        if (f_ent->chain_idx > stage_fc->tcam_bottom) {
            next_null_idx = f_ent->chain_idx;
            slice_idx_target = stage_fc->tcam_bottom;

            if (soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit,  f_ent->group,
                        slice_idx_target, slice_idx_target+1, 
                        (next_null_idx - slice_idx_target), _BCM_FP_CHAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }   
            } else {
                rv = _robo_field_entry_shift_down(unit, fs, 
                    slice_idx_target, next_null_idx, _BCM_FP_CHAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    return (rv);
                }   
            }
        }
    }

    if (f_ent->drv_entry != NULL) {
        rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_fc->stage_id,
            f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY, 0, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_UNAVAIL)){
            return rv;
        }        
        sal_free(f_ent->drv_entry);
    }
    /* okay to free entry */
    sal_free(f_ent);

    return BCM_E_NONE;
}

/* Function: _field_robo_reqd_prio_set_move
 *
 * Purpose: Checks if the entry needs to be moved due to prio set
 *
 * Parameters: 
 *   unit - 
 *   f_ent - Entry whose priority is being set
 *   prio - The new priority for f_ent
 *
 * Returns:
 *   TRUE/FALSE
 */
int
_field_robo_reqd_prio_set_move(int unit, _field_entry_t *f_ent, int prio)
{
    _field_slice_t         *fs; 
    _field_stage_t *stage_fc;

    int i, flag; /* flag denotes is we are before OR after f_ent */

    fs = f_ent->fs;

    /* Get stage control structure. */
    BCM_IF_ERROR_RETURN
        (_robo_field_stage_control_get(unit, fs->stage_id, &stage_fc));

    flag = -1; /* We are before f_ent */
    for (i = 0; i < stage_fc->tcam_bottom; i++) {
        if (stage_fc->field_shared_entries[i] == f_ent) {
            flag = 1; /* Now, we are after f_ent */
            continue;
        }
        if (stage_fc->field_shared_entries[i] == NULL) {
            continue;
        }
        if (flag == -1) {
            if (_robo_field_entry_prio_cmp(stage_fc->field_shared_entries[i]->prio, prio) < 0) {
                /* 
                 * An entry before f_ent has lower priority than prio
                 *     Movement is required 
                 */
                return TRUE; 
            }
        } else {
            if (_robo_field_entry_prio_cmp(stage_fc->field_shared_entries[i]->prio, prio) > 0) {
                /* 
                 * An entry after f_ent has higher priority than prio
                 *     Movement is required 
                 */
                return TRUE;
            }
        }
    }
    return FALSE; /* f_ent with new prio is in proper location */
}

    
/*
 * Function: _robo_field_range_flags_check
 *    
 * Purpose:
 *     Sanity check on range flags. 
 *
 * Parameters:
 *     unit   - (IN) BCM device number.
 *     flags  - (IN) One or more of BCM_FIELD_RANGE_* flags.
 *     stage_id - (OUT) Pipeline stage id number.
 *
 * Returns:
 *     BCM_E_XXX  - Flags are valid & supported.
 */
STATIC int
_robo_field_range_flags_check(int unit, uint32 flags, _field_stage_id_t *stage_id)
{
    int   cntr;                 /* Range types counter.  */
    
    if (flags & (BCM_FIELD_RANGE_INVERT |BCM_FIELD_RANGE_EXTERNAL|
        BCM_FIELD_RANGE_TCP |BCM_FIELD_RANGE_UDP|
        BCM_FIELD_RANGE_PACKET_LENGTH)) {
        return (BCM_E_UNAVAIL);
    }
    /* Make sure only one range was selected. */
    cntr = 0;
    if (flags & BCM_FIELD_RANGE_SRCPORT) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_DSTPORT) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_OUTER_VLAN) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_INNER_VLAN) {
        cntr++;
    }
    if (cntr > 1) {
        return (BCM_E_PARAM);
    }
    if (cntr == 0) {
        return (BCM_E_UNAVAIL);
    }

     /* Flags checking */
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* BCM5395 didn't have field ranger */
        return BCM_E_UNAVAIL;
    }  else if (SOC_IS_TBX(unit)) {
        if (SOC_IS_TB(unit)) {
            if (flags & ~(BCM_FIELD_RANGE_INNER_VLAN|
                    BCM_FIELD_RANGE_LOOKUP)) {
                return BCM_E_UNAVAIL;
            }
            flags |= BCM_FIELD_RANGE_LOOKUP;
        }
        if (SOC_IS_VO(unit)) {
            if (flags & BCM_FIELD_RANGE_LOOKUP) {
                if (flags & ~(BCM_FIELD_RANGE_INNER_VLAN |
                       BCM_FIELD_RANGE_LOOKUP)) {
                    return BCM_E_UNAVAIL;
                }
            }
        }
    } 

    /* stage_id */
    if (flags & BCM_FIELD_RANGE_LOOKUP) {
        *stage_id = _BCM_FIELD_STAGE_LOOKUP;
    } else {
        *stage_id = _BCM_FIELD_STAGE_INGRESS;
    }
    return (BCM_E_NONE);
}

/*
 * Function:_robo_field_range_create 
 *    
 * Purpose:
 *     Create field range object.
 *
 * Parameters:
 *     unit   - (IN) BCM device number.
 *
 * Returns:
 *     BCM_E_XXX 
 */
STATIC int
_robo_field_range_create(int unit, bcm_field_range_t *range,
                    uint32 flags, bcm_l4_port_t min,
                    bcm_l4_port_t max, _field_stage_id_t   stage_id)
{
    _field_stage_t      *stage_fc;
    int rv;

    BCM_IF_ERROR_RETURN
            (_robo_field_stage_control_get(unit, stage_id, &stage_fc));

    for (;;) {
        rv = bcm_robo_field_range_create_id(unit, stage_fc->range_id, flags,
                                               min, max);

        if (rv != BCM_E_EXISTS) {
            break;
        }

        if (++stage_fc->range_id == 0) {
            stage_fc->range_id = 1;
        }
    }

    if (BCM_SUCCESS(rv)) {
        *range = stage_fc->range_id;

        if (++stage_fc->range_id == 0) {
            stage_fc->range_id = 1;
        }
    }
    
    return rv;    
}
/*
 * Function: _robo_field_control_free
 *
 * Purpose:
 *    Free field_control structure. 
 *
 * Parameters:
 *     unit - (IN) BCM device number
 *     fc   - (IN) Field control structure.
 *
 * Returns:
 *     BCM_E_XXX  
 */
STATIC int
_robo_field_control_free(int unit, _field_control_t *fc)
{
    _field_control[unit] = NULL;

    if (NULL == fc) {
        return (BCM_E_NONE);
    }

    /* Free protection semaphore. */
    if (NULL != fc->fc_lock) sal_mutex_destroy(fc->fc_lock);

    /* Free udf configuration. */
    if (NULL != fc->udf) sal_free(fc->udf);

    /* Free policers lookup hash. */
    if (NULL != fc->policer_hash) sal_free(fc->policer_hash);

    /* Free counters lookup hash. */
    if (NULL != fc->stat_hash) sal_free(fc->stat_hash);
    
    /* Free module control structure. */
    sal_free(fc);

    return (BCM_E_NONE);
}


STATIC int
_field_tcam_sync_entry(int unit, _field_stage_t *stage_fc, uint32 index)
{
    int retval = BCM_E_NONE;
    drv_cfp_entry_t temp_entry;
    drv_cfp_tcam_t  temp_chain;
    drv_cfp_entry_t *drv_entry;
    _field_entry_t      *f_ent;
    uint32              temp;
    _field_control_t    *fc;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    f_ent = stage_fc->field_shared_entries[index];
    
    sal_memset(&temp_entry, 0, sizeof(drv_cfp_entry_t));
    sal_memset(&temp_chain, 0, sizeof(drv_cfp_tcam_t));
    /* Check if this entry is valid or invalid */
    if (f_ent) {
        /* Valid */
        drv_entry = (drv_cfp_entry_t *)f_ent->drv_entry;
        /* Ignore the dirty entries */
        if (f_ent->flags & _FP_ENTRY_INSTALL) {
            if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
                if (index < stage_fc->tcam_bottom) {
                    temp_entry.flags = _DRV_CFP_SLICE_CHAIN
                        | _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                } else {
                    temp_entry.flags = _DRV_CFP_SLICE_CHAIN
                        | _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                }
                temp_entry.cfp_chain = &temp_chain;
                
            }
            retval = DRV_CFP_ENTRY_READ
                (unit, index, DRV_CFP_RAM_TCAM, &temp_entry);
            if (BCM_FAILURE(retval)) {
                FP_UNLOCK(fc);
                return retval;
            }
            /* 
             Since the Valid field of TCAM Mask always present as zero,
             We need set this field before comparison.
             */
            temp = 0x3;
            retval = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_VALID, 
                &temp_entry, &temp);
            if (BCM_FAILURE(retval)) {
                FP_UNLOCK(fc);
                return retval;
            }

            if ((f_ent->flags & _FP_ENTRY_WITH_CHAIN) &&
                (index < stage_fc->tcam_bottom)){
                /* Compare the chain entry (slice 0) */
                if ((sal_memcmp(&temp_chain.tcam_data, 
                    &drv_entry->cfp_chain->tcam_data, 
                    sizeof(temp_chain.tcam_data)) !=0 ) ||
                    (sal_memcmp(&temp_chain.tcam_mask, 
                    &drv_entry->cfp_chain->tcam_mask, 
                    sizeof(temp_chain.tcam_mask)))) {
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "%s, DIFF : index = %d\n"),
                                 FUNCTION_NAME(), index));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "data : old = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 drv_entry->cfp_chain->tcam_data[0], 
                                 drv_entry->cfp_chain->tcam_data[1], 
                                 drv_entry->cfp_chain->tcam_data[2], 
                                 drv_entry->cfp_chain->tcam_data[3],
                                 drv_entry->cfp_chain->tcam_data[4], 
                                 drv_entry->cfp_chain->tcam_data[5],
                                 drv_entry->cfp_chain->tcam_data[6], 
                                 drv_entry->cfp_chain->tcam_data[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "data : new = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 temp_chain.tcam_data[0], temp_chain.tcam_data[1], 
                                 temp_chain.tcam_data[2], temp_chain.tcam_data[3],
                                 temp_chain.tcam_data[4], temp_chain.tcam_data[5],
                                 temp_chain.tcam_data[6], temp_chain.tcam_data[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "mask : old = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 drv_entry->cfp_chain->tcam_mask[0], 
                                 drv_entry->cfp_chain->tcam_mask[1], 
                                 drv_entry->cfp_chain->tcam_mask[2], 
                                 drv_entry->cfp_chain->tcam_mask[3],
                                 drv_entry->cfp_chain->tcam_mask[4], 
                                 drv_entry->cfp_chain->tcam_mask[5],
                                 drv_entry->cfp_chain->tcam_mask[6], 
                                 drv_entry->cfp_chain->tcam_mask[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "mask : new = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 temp_chain.tcam_mask[0], temp_chain.tcam_mask[1], 
                                 temp_chain.tcam_mask[2], temp_chain.tcam_mask[3],
                                 temp_chain.tcam_mask[4], temp_chain.tcam_mask[5],
                                 temp_chain.tcam_mask[6], temp_chain.tcam_mask[7]));
                    /* write the software copy to chip */
                    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN
                        | _DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                    drv_entry->flags |= _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                    retval = DRV_CFP_ENTRY_WRITE
                        (unit, index, DRV_CFP_RAM_TCAM, f_ent->drv_entry);
                    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                    if (BCM_FAILURE(retval)) {
                        FP_UNLOCK(fc);
                        return retval;
                    }
                }
                
            } else {
                /* Compare the entry or the slice3 entry of chain */
                if ((sal_memcmp(&temp_entry.tcam_data, 
                    &drv_entry->tcam_data, 
                    sizeof(temp_entry.tcam_data)) !=0 ) ||
                    (sal_memcmp(&temp_entry.tcam_mask, 
                    &drv_entry->tcam_mask, 
                    sizeof(temp_entry.tcam_mask)))) {
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "%s, DIFF : index = %d\n"),
                                 FUNCTION_NAME(), index));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "data : old = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 drv_entry->tcam_data[0], 
                                 drv_entry->tcam_data[1], 
                                 drv_entry->tcam_data[2], 
                                 drv_entry->tcam_data[3],
                                 drv_entry->tcam_data[4], 
                                 drv_entry->tcam_data[5],
                                 drv_entry->tcam_data[6], 
                                 drv_entry->tcam_data[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "data : new = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 temp_entry.tcam_data[0], temp_entry.tcam_data[1], 
                                 temp_entry.tcam_data[2], temp_entry.tcam_data[3],
                                 temp_entry.tcam_data[4], temp_entry.tcam_data[5],
                                 temp_entry.tcam_data[6], temp_entry.tcam_data[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "mask : old = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 drv_entry->tcam_mask[0], 
                                 drv_entry->tcam_mask[1], 
                                 drv_entry->tcam_mask[2], 
                                 drv_entry->tcam_mask[3],
                                 drv_entry->tcam_mask[4], 
                                 drv_entry->tcam_mask[5],
                                 drv_entry->tcam_mask[6], 
                                 drv_entry->tcam_mask[7]));
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "mask : new = 0x%x, 0x%x, 0x%x, 0x%x, "
                                            "0x%x, 0x%x, 0x%x, 0x%x\n"),
                                 temp_entry.tcam_mask[0], temp_entry.tcam_mask[1], 
                                 temp_entry.tcam_mask[2], temp_entry.tcam_mask[3],
                                 temp_entry.tcam_mask[4], temp_entry.tcam_mask[5],
                                 temp_entry.tcam_mask[6], temp_entry.tcam_mask[7]));
                    /* write the software copy to chip */
                    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
                        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN
                            | _DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                        drv_entry->flags |= _DRV_CFP_SLICE_CHAIN |
                            _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    }
                    retval = DRV_CFP_ENTRY_WRITE
                        (unit, index, DRV_CFP_RAM_TCAM, f_ent->drv_entry);
                    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
                        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
                    }
                    if (BCM_FAILURE(retval)) {
                        FP_UNLOCK(fc);
                        return retval;
                    }
                }
            }
           
        }        
    } else {
        /* Invalid entry only need to check the valid bits */
        retval = DRV_CFP_ENTRY_WRITE
            (unit, index, DRV_CFP_RAM_TCAM_INVALID, &temp_entry);
        if (BCM_FAILURE(retval)) {
            FP_UNLOCK(fc);
            return retval;
        }
    }
        
    FP_UNLOCK(fc);
    return retval;
}


/*
 * Function:
 *      soc_robo_counter_thread
 * Purpose:
 *      Master counter collection and accumulation thread.
 * Parameters:
 *      unit_vp - StrataSwitch unit # (as a void *).
 * Returns:
 *      Nothing, does not return.
 */
STATIC void
_field_tcam_thread(void *unit_vp)
{
    int             unit = PTR_TO_INT(unit_vp);
    int             rv = BCM_E_NONE;
    sal_sem_t           sem;
    int             interval, div;
    _field_control_t        *fc = NULL;
    sal_usecs_t     stime, etime;
    int         chunk_index, chunk_size;
    uint32         i;
    _field_stage_t      *stage_fc;
#if defined(BCM_TB_SUPPORT) || defined(BCM_53125) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    drv_fp_tcam_checksum_t drv_fp_tcam_chksum;
    uint32 index, mode;
    _field_entry_t *f_ent;    
#endif

    /* Get unit FP control structure. */
    rv = _robo_field_control_get(unit, &fc);
    if (BCM_FAILURE(unit)){
        goto done;
    }

    /*
     * There's a race condition since the PID is used as a
     * signal of whether the counter thread is running.  We sleep
     * here to make sure it gets initialized properly in SOC_CONTROL
     * by the thread_create return.
     */

    sal_sleep(1);

    /*
     * Create a semaphore used to time the trigger scans.
     */
    if ((sem = fc->tcam_notify) != NULL) {
        fc->tcam_notify = NULL;    /* Stop others from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }

    fc->tcam_notify = sal_sem_create("tcam notify", sal_sem_BINARY, 0);

    if (fc->tcam_notify == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "_field_tcam_thread: semaphore init failed\n")));
        rv = BCM_E_INTERNAL;
        goto done;
    }

    div = 1;
    chunk_index = 0;
    chunk_size = 0;
    if(SOC_IS_VULCAN(unit)) {
        rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc);
        chunk_index = 0;
        chunk_size =  BCM_ROBO_FIELD_TCAM_THREAD_CHUNK_SIZE;
        div = stage_fc->tcam_sz / BCM_ROBO_FIELD_TCAM_THREAD_CHUNK_SIZE;
    }
    while ((interval = fc->tcam_interval) != 0) {
        if (SOC_IS_VULCAN(unit)) {

            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "_field_tcam_thread: Process %d-%d\n"),
                         chunk_index, chunk_index + chunk_size - 1));
            rv = _robo_field_stage_control_get(unit, 
                _BCM_FIELD_STAGE_INGRESS, &stage_fc);
            if(BCM_FAILURE(rv)) {
                goto done;
            }
            stime = sal_time_usecs();
            for (i = chunk_index; i < (chunk_index + chunk_size); i++) {
                /* sync entry */
                _field_tcam_sync_entry(unit, stage_fc, i);
            }
            /*
             * Implement the sleep using a semaphore timeout so if the task
             * is requested to exit, it can do so immediately.
             */
            etime = sal_time_usecs();
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "_field_tcam_thread: unit=%d: done in %d usec\n"),
                         unit,
                         SAL_USECS_SUB(etime, stime)));
            if ((chunk_index += chunk_size) >= stage_fc->tcam_sz) {
                chunk_index = 0;
            }
        } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT            
            rv = DRV_FP_TCAM_PARITY_CHECK(unit, &drv_fp_tcam_chksum);
            if(BCM_FAILURE(rv)) {
                goto done;
            }

            mode = 0;
            if (drv_fp_tcam_chksum.tcam_error & 
                    (1 << _BCM_FIELD_STAGE_INGRESS)) {
                rv = _robo_field_stage_control_get(unit, 
                    _BCM_FIELD_STAGE_INGRESS, &stage_fc);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
                index = drv_fp_tcam_chksum.stage_ingress_addr;
                f_ent = stage_fc->field_shared_entries[index];
                if (!f_ent) {
                    _field_tcam_policy_clear(unit, _BCM_FIELD_STAGE_INGRESS,
                        index, -1);
                    continue;
                }

                if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
                    if (index < stage_fc->tcam_bottom) {
                        mode |= _DRV_FP_ENTRY_CHAIN;
                   }
                }

                rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, _BCM_FIELD_STAGE_INGRESS,
                    f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL, 
                    index, &mode);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
            }
            mode = 0;
            if (drv_fp_tcam_chksum.tcam_error & 
                    (1 << _BCM_FIELD_STAGE_LOOKUP)) {
                rv = _robo_field_stage_control_get(unit, 
                    _BCM_FIELD_STAGE_LOOKUP, &stage_fc);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
                index = drv_fp_tcam_chksum.stage_lookup_addr;
                f_ent = stage_fc->field_shared_entries[index];
                if (!f_ent) {
                    _field_tcam_policy_clear(unit, _BCM_FIELD_STAGE_LOOKUP,
                        index, -1);
                    continue;
                }

                rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, _BCM_FIELD_STAGE_LOOKUP,
                    f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL, 
                    index, &mode);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }                
            }

            if (drv_fp_tcam_chksum.tcam_error & 
                    (1 << _BCM_FIELD_STAGE_EGRESS)) {
                rv = _robo_field_stage_control_get(unit, 
                    _BCM_FIELD_STAGE_EGRESS, &stage_fc);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
                index = drv_fp_tcam_chksum.stage_egress_addr;
                f_ent = stage_fc->field_shared_entries[index];
                if (!f_ent) {
                    _field_tcam_policy_clear(unit, _BCM_FIELD_STAGE_EGRESS,
                        index, -1);
                    continue;
                }                
                rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, _BCM_FIELD_STAGE_EGRESS,
                    f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL, 
                    index, &mode);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
            }
#endif  /* BCM_TB_SUPPORT */
        } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
            SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
            SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
            rv = DRV_FP_TCAM_PARITY_CHECK(unit, &drv_fp_tcam_chksum);
            if(BCM_FAILURE(rv)) {
                goto done;
            }

            mode = 0;
            if (drv_fp_tcam_chksum.tcam_error & 
                    (1 << _BCM_FIELD_STAGE_INGRESS)) {
                rv = _robo_field_stage_control_get(unit, 
                    _BCM_FIELD_STAGE_INGRESS, &stage_fc);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
                index = drv_fp_tcam_chksum.stage_ingress_addr;
                f_ent = stage_fc->field_shared_entries[index];
                if (!f_ent) {
                    _field_tcam_policy_clear(unit, _BCM_FIELD_STAGE_INGRESS,
                        index, -1);
                    continue;
                }

                if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
                    if (index < stage_fc->tcam_bottom) {
                        mode |= _DRV_FP_ENTRY_CHAIN;
                   }
                }

                rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, _BCM_FIELD_STAGE_INGRESS,
                    f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL, 
                    index, &mode);
                if(BCM_FAILURE(rv)) {
                    goto done;
                }
            }
#endif /* BCM_53125 || BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ ||SF3 */
        }
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "interval %d\n"),
                   interval/div));
        sal_sem_take(fc->tcam_notify, interval/div);
    }

 done:
    if (rv < 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "_field_tcam_thread: Operation failed; exiting\n")));
    }


    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_field_tcam_thread: exiting\n")));

    if ((sem = fc->tcam_notify) != NULL) {
        fc->tcam_notify = NULL;    /* Stop others from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }
    
    fc->tcam_pid = SAL_THREAD_ERROR;
    fc->tcam_interval = 0;

    sal_thread_exit(0);
}


int 
_robo_field_thread_stop(int unit)
{
    _field_control_t    *fc;
    soc_timeout_t   to;

    int             rv = BCM_E_NONE;

    FIELD_IS_INIT(unit);
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: _robo_field_thread_stop(%d)\n"),
               unit));

    fc = _field_control[unit];

    /* Stop TCAM protection thread */ 
    if (soc_feature(unit, soc_feature_field_tcam_parity_check)) {
        FP_LOCK(fc);
        fc->tcam_interval = 0;
        FP_UNLOCK(fc);
        if (fc->tcam_pid != SAL_THREAD_ERROR) {
            if (fc->tcam_notify) {
                sal_sem_give(fc->tcam_notify);
            }
            /* Give thread a few seconds to wake up and exit */
            soc_timeout_init(&to, BCM_ROBO_FIELD_TCAM_THREAD_INTERVAL, 0);

            while (fc->tcam_pid != SAL_THREAD_ERROR) {
                if (soc_timeout_check(&to)) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "_robo_field_thread_stop: thread did not exit\n")));    
                    rv = BCM_E_INTERNAL;
                    break;
                }
                sal_usleep(10000);
            }
        }
    }

    return (rv);
}

/*
 * Function:
 *     _robo_field_policer_get
 * Purpose:
 *     Lookup a Policer description structure by policer id.
 * Parmeters:
 *     unit      - (IN)BCM device number. 
 *     pid       - (IN)Policer id. 
 *     policer_p - (OUT) Policer lookup result.  
 * Returns:
 *     BCM_E_XXX
 */
int
_robo_field_policer_get(int unit, bcm_policer_t pid, 
                       _field_policer_t **policer_p)
{
    _field_policer_t *f_pl; /* Policer lookup pointer.  */
    _field_control_t *fc;   /* Field control structure. */
    uint32 hash_index;      /* Entry hash.              */

    /* Input parameters check. */
    if (NULL == policer_p) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    hash_index = pid & _FP_HASH_INDEX_MASK(fc);
    f_pl  =  fc->policer_hash[hash_index];
    while (NULL != f_pl) {
        /* Match entry id. */
        if (f_pl->pid == pid) {
            *policer_p = f_pl;
            return (BCM_E_NONE);
        }
        f_pl = f_pl->next;
    }

    /* Policer with pid == pid was not found. */
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _robo_field_policer_destroy2
 * Purpose:
 *      Deinitialize a policer entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      fc      - (IN) Field control structure.
 *      f_pl    - (IN) Internal policer descriptor.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
STATIC int 
_robo_field_policer_destroy2(int unit, _field_control_t *fc,
                        _field_policer_t *f_pl)
{
    /* Input parameters check. */ 
    if ((NULL == fc) || (NULL == f_pl)) {
        return (BCM_E_PARAM);
    }

    /* Reject destroy if policer is in use. */ 
    if (f_pl->sw_ref_count > 1) {
        return (BCM_E_BUSY);
    }

    /* Remove policer for lookup hash. */
    _FP_HASH_REMOVE(fc->policer_hash, _field_policer_t, f_pl, 
                    (f_pl->pid & _FP_HASH_INDEX_MASK(fc)));


    /* De-allocate policer descriptor. */
    sal_free(f_pl);

    /* Decrement number of active policers. */
    if (fc->policer_count > 0) {
        fc->policer_count--;
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _robo_field_policer_destroy
 * Purpose:
 *      Deinitialize a policer entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      pid     - (IN) Policer id.  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_policer_destroy(int unit, bcm_policer_t pid)
{
    _field_control_t    *fc;               /* Field control structure.     */
    _field_policer_t    *f_pl;             /* Internal policer descriptor. */
    int                 rv;                /* Operation return status.     */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Find policer info. */
    rv = _robo_field_policer_get(unit, pid, &f_pl);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    return _robo_field_policer_destroy2(unit, fc, f_pl);
}
/*
 * Function:
 *      _robo_field_policer_hw_flags_set
 * Purpose:
 *      Update policer installation is required flag.    
 * Parameters:
 *      unit    - (IN) Unit number.
 *      f_pl    - (IN/OUT) Internal policer descriptor.
 *      flags   - (IN) Internal flags. 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
STATIC int 
_robo_field_policer_hw_flags_set(int unit, _field_policer_t *f_pl, uint32 flags)
{
    /* Input parameters check. */
    if (NULL == f_pl) {
        return (BCM_E_PARAM);
    }

    f_pl->hw_flags |= flags;

    switch (f_pl->cfg.mode) {
      case bcmPolicerModeCommitted:
            f_pl->hw_flags |= _FP_POLICER_DIRTY;   
            break;
      case bcmPolicerModeSrTcm:
      case bcmPolicerModeTrTcmDs:
      case bcmPolicerModeCoupledTrTcmDs:    
            if ((SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) ||
                (SOC_IS_NORTHSTARPLUS(unit)) || (SOC_IS_STARFIGHTER3(unit))) {
                f_pl->hw_flags |= _FP_POLICER_DIRTY;
            } else {
              return (BCM_E_PARAM);
            }
            break;
      case bcmPolicerModeTrTcm:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                f_pl->hw_flags |= _FP_POLICER_DIRTY;
            } else {
                return (BCM_E_PARAM);
            }
            break;
      default:
          return (BCM_E_PARAM);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_policer_hw_free
 *
 * Purpose:
 *     Deallocate hw policer from an entry.
 * 
 * Parameters:
 *     unit      - (IN) BCM device number.  
 *     level     - (IN) Policer level.
 *     f_ent     - (IN) Entry policer belongs to.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_policer_hw_free (int unit, uint8 level, _field_entry_t *f_ent)
{
    _field_entry_policer_t *f_ent_pl;  /* Field entry policer structure. */
    _field_policer_t       *f_pl;      /* Policer descriptor.            */
    int                 tcam_idx, tcam_chain_idx, rv;
    _field_stage_t  *stage_fc;


    /* Input parameters check. */
    if ((NULL == f_ent) || (level >= _FP_POLICER_LEVEL_COUNT)) {
        return (BCM_E_PARAM);
    }

    f_ent_pl = f_ent->policer + level;

    /* Skip uninstalled policers. */
    if (0 == (f_ent_pl->flags & _FP_POLICER_INSTALLED)) {
        return (BCM_E_NONE);
    }

    /* Read policer configuration.*/
    BCM_IF_ERROR_RETURN(_robo_field_policer_get(unit, f_ent_pl->pid, &f_pl));

    /* Decrement hw reference count. */
    if (f_pl->hw_ref_count > 0) {
        f_pl->hw_ref_count--; 
    }

    /* Policer not used by any other entry. */
    if (f_pl->hw_ref_count == 0) {        
        rv = _robo_field_stage_control_get(unit, f_ent->fs->stage_id, &stage_fc);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        rv  =  _robo_field_entry_tcam_idx_get(stage_fc, f_ent, &tcam_idx, 
                    &tcam_chain_idx);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "tcam_idx %d  tcam_chain_idx %d\n"),
                   tcam_idx, tcam_chain_idx));

        rv = DRV_FP_POLICER_CONTROL(unit, f_ent->fs->stage_id, 
            DRV_FIELD_POLICER_FREE, f_ent->drv_entry, NULL);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: FB failed to install meter.\n")));
            return rv;
        }        

        rv =  _field_meter_install(unit, f_ent->fs->stage_id, f_ent,
            tcam_idx, tcam_chain_idx);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: FB failed to install meter.\n")));
            return rv;
        }        

        /* Mark policer for reinstallation. */
        BCM_IF_ERROR_RETURN (_robo_field_policer_hw_flags_set(unit, f_pl, 0));
    }
    f_ent_pl->flags &= ~_FP_POLICER_INSTALLED;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _robo_field_entry_policer_detach
 * Purpose:
 *      Detach a policer from a field entry.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      entry_id - (IN) Field entry ID.
 *      level    - (IN) Policer level.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
STATIC int 
_robo_field_entry_policer_detach(int unit, _field_entry_t *f_ent, int level)
{
    _field_policer_t       *f_pl;    /* Internal policer descriptor.  */
    _field_entry_policer_t *f_ent_pl;/* Field entry policer structure.*/

    /* Input parameters check. */
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        return (BCM_E_PARAM);
    }

    /* Make sure policer attached to the entry. */
    f_ent_pl = f_ent->policer + level;
    if (0 == (f_ent_pl->flags & _FP_POLICER_VALID)) {
        return (BCM_E_EMPTY);
    }

    /* Get policer description structure. */
    BCM_IF_ERROR_RETURN(_robo_field_policer_get(unit, f_ent_pl->pid, &f_pl));

    /* If entry was installed decrement hw reference counter. */
    BCM_IF_ERROR_RETURN(_robo_field_policer_hw_free (unit, level, f_ent));

    /* Decrement policer reference counter. */
    f_pl->sw_ref_count--;
   
    /* If no one else is using the policer and  
     * policer is internal destroy it. 
     */
    if ((1 == f_pl->sw_ref_count) && (f_pl->hw_flags & _FP_POLICER_INTERNAL)) {
        BCM_IF_ERROR_RETURN(_robo_field_policer_destroy(unit, f_ent_pl->pid));
    }

    /* Detach policer from an entry. */
    f_ent_pl->pid   = _FP_INVALID_INDEX;
    f_ent_pl->flags = 0;

    /* Mark entry for reinstall. */
    f_ent->flags |= _FP_ENTRY_DIRTY;

    f_ent->group->group_status.meter_count--;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _robo_field_policer_id_alloc
 * Purpose:
 *      Allocate a policer id.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      pid     - (OUT) Policer id.  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_policer_id_alloc(int unit, bcm_policer_t *pid)
{
    int               max_count;           /* Maximum number of pid to try.*/
    _field_policer_t  *f_pl;               /* Field policer descriptor.    */
    int               rv;                  /* Operation return status.     */ 
    static uint32  last_allocated_pid = 0; /* Policer id alloc tracker.    */ 
    
    /* Input parameters check. */
    if (NULL == pid) {
        return (BCM_E_PARAM);
    }

    max_count = _FP_ID_MAX;
    while (max_count--) {
        last_allocated_pid++;
        if (_FP_ID_MAX == last_allocated_pid) {
            last_allocated_pid = _FP_ID_BASE;
        }
        rv = _robo_field_policer_get(unit, last_allocated_pid, &f_pl);
        if (BCM_E_NOT_FOUND == rv) {
            *pid = last_allocated_pid;
            return (BCM_E_NONE);
        }
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }
    return (BCM_E_RESOURCE);
}



/*
 * Function:
 *      _robo_field_policer_mode_support
 * Purpose:
 *      Validate policer mode support.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      f_ent   - (IN) Field entry policer attached. 
 *      level   - (IN) Level policer attached.
 *      f_pl    - (IN) Policer descriptor.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_robo_field_policer_mode_support(int unit, _field_entry_t *f_ent,
                            int level, _field_policer_t *f_pl)
{
    int rv = BCM_E_PARAM;   /* Operation return status.     */
    int stage_id;

    /* Level 1 policers mode must be bcmPolicerModePeak */
    if (level) {
        return (rv);
    }

    stage_id = f_ent->fs->stage_id;

    rv = DRV_FP_POLICER_CONTROL(unit, stage_id, 
        DRV_FIELD_POLICER_MODE_SUPPORT,
        f_ent->drv_entry, (drv_policer_config_t *)&(f_pl->cfg));

    return rv;
}

/*
 * Function:
 *      _bcm_robo_field_policer_create
 * Purpose:
 *      Initialize a policer entry.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      pol_cfg - (IN) Policer configuration.
 *      flags   - (IN) HW/API specific flags. 
 *      pid     - (OUT) Policer id.  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_robo_field_policer_create(int unit, bcm_policer_config_t *pol_cfg, 
                      uint32 flags, bcm_policer_t *pid)
{
    _field_control_t    *fc;          /* Field control structure.     */
    int                 rv;           /* Operation return status.     */
    _field_policer_t    *f_pl = NULL; /* Internal policer descriptor. */


    FIELD_IS_INIT(unit);

    /* Input parameters check. */
    if (NULL == pol_cfg) {
        return (BCM_E_PARAM);
    }

    /* Reject unsupported flags. */
    if (pol_cfg->flags & BCM_POLICER_COLOR_MERGE_OR) {
        return (BCM_E_PARAM);
    }
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "%s flag %x policer_mode %d\n"),
                 FUNCTION_NAME(), flags, pol_cfg->mode));
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Generate policer id. */
    if (0 == (pol_cfg->flags & BCM_POLICER_WITH_ID)) {
        rv = _robo_field_policer_id_alloc(unit, pid);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return rv;
        }
    } else {
        /* Check if policer id is already in use. */
        rv = _robo_field_policer_get(unit, *pid, &f_pl);
        if (BCM_SUCCESS(rv)) {
            /* Verify that replace flag is set. */
            if(0 == (pol_cfg->flags & BCM_POLICER_REPLACE)) {
                FP_UNLOCK(fc);
                return (BCM_E_EXISTS);
            }

            /* Make sure police is not attached to any entry. */
            if (1 != f_pl->sw_ref_count) {
                FP_UNLOCK(fc);
                return (BCM_E_BUSY);
            }
            /* Copy new configuration. */
            sal_memcpy(&f_pl->cfg, pol_cfg, sizeof(bcm_policer_config_t)); 

            /* Set policer "dirty" flag and return. */
            rv = _robo_field_policer_hw_flags_set(unit, f_pl, flags);
            FP_UNLOCK(fc);
            return (rv);
        }
    }
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "%s pid %d\n"),
                 FUNCTION_NAME(),*pid));
    /* Allocate policer descriptor. */
    _FP_MEM_ALLOC(f_pl, sizeof (_field_policer_t), "Field policer");
    if (NULL == f_pl) {
        FP_UNLOCK(fc);
        return (BCM_E_MEMORY);
    }

    /* Copy policer configuration. */
    sal_memcpy(&f_pl->cfg, pol_cfg, sizeof(bcm_policer_config_t)); 

    /* Set policer "dirty" flags. */
    rv = _robo_field_policer_hw_flags_set(unit, f_pl, flags);
    if (BCM_FAILURE(rv)) {
        sal_free(f_pl);
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Initialize reference count to 1. */
    f_pl->sw_ref_count = 1;

    /* Set hw index to - no hw resources allocated. */
    f_pl->pool_index = _FP_PARAM_INVALID;
    f_pl->hw_index = _FP_PARAM_INVALID;

    /* Initialize policer id. */
    f_pl->pid = *pid;

    /* Insert policer into policers hash. */
    _FP_HASH_INSERT(fc->policer_hash, f_pl, (*pid & _FP_HASH_INDEX_MASK(fc)));

    /* Increment number of active policers. */
    fc->policer_count++;

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

int
_bcm_robo_field_policer_destroy(int unit, bcm_policer_t policer_id)
{
        _field_control_t *fc;       /* Field control structure. */
        int               rv;       /* Operation return status. */

        FIELD_IS_INIT(unit);
        BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

        FP_LOCK(fc);
        rv = _robo_field_policer_destroy(unit, policer_id);
        FP_UNLOCK(fc);

        return (rv);
}
int
_bcm_robo_field_policer_destroy_all(int unit)
{
       _field_control_t *fc;              /* Field control structure.     */
        int              idx;              /* Policer hash iteration index.*/
        int              rv = BCM_E_NONE;  /* Operation return status.     */

        FIELD_IS_INIT(unit);
        BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

        FP_LOCK(fc);

        /* Iterate over all hash buckets. */
        for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
            /* Destroy entries in each bucket. */
            while (NULL != fc->policer_hash[idx]) {
                rv = _robo_field_policer_destroy2(unit, fc, fc->policer_hash[idx]);
                if (BCM_FAILURE(rv)) {
                    break;
                }
            }
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
        FP_UNLOCK(fc);
        return (rv);
}
int 
_bcm_robo_field_policer_get(int unit, bcm_policer_t policer_id, 
                    bcm_policer_config_t *pol_cfg)
{
    _field_control_t    *fc;     /* Field control structure.     */
    _field_policer_t    *f_pl;   /* Internal policer descriptor. */
    int                 rv;      /* Operation return status.     */

    /* Input parameters check. */
    if (NULL == pol_cfg) {
        return (BCM_E_PARAM);
    }

    FIELD_IS_INIT(unit);
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    rv = _robo_field_policer_get(unit, policer_id, &f_pl);
    if (BCM_SUCCESS(rv)) {
        sal_memcpy(pol_cfg, &f_pl->cfg, sizeof(bcm_policer_config_t));
    }

    

    FP_UNLOCK(fc);
    return (rv);
}
int 
_bcm_robo_field_policer_set(int unit, bcm_policer_t policer_id, 
                    bcm_policer_config_t *pol_cfg)
{
    _field_control_t    *fc;     /* Field control structure.     */
    _field_policer_t    *f_pl = NULL;   /* Internal policer descriptor. */
    int                 rv;      /* Operation return status.     */

    /* Input parameters check. */
    if (NULL == pol_cfg) {
        return (BCM_E_PARAM);
    }

    FIELD_IS_INIT(unit);
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    rv = _robo_field_policer_get(unit, policer_id, &f_pl);
    if (BCM_SUCCESS(rv)) {
        sal_memcpy(&f_pl->cfg, pol_cfg, sizeof(bcm_policer_config_t));
    }

    /* Set policer "dirty" flags. */
    rv = _robo_field_policer_hw_flags_set(unit, f_pl, 0);

    

    FP_UNLOCK(fc);
    return (rv);
}

int
_bcm_robo_field_policer_traverse(int unit, bcm_policer_traverse_cb cb, 
                         void *user_data)
{
    _field_control_t        *fc;      /* Field control structure.       */
    _field_policer_t        *f_pl;    /* Internal policer descriptor.   */
    bcm_policer_config_t    cfg;      /* Policer configuration.         */
    int                     idx;      /* Policer hash iteration index.  */
    int              rv = BCM_E_NONE; /* Operation return status.       */

    /* Input parameter check. */
    if (NULL == cb) {
        return (BCM_E_PARAM);
    } 

    FIELD_IS_INIT(unit);
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_pl = fc->policer_hash[idx]; 
        while (NULL != f_pl) {
            sal_memcpy(&cfg, &f_pl->cfg, sizeof(bcm_policer_config_t));
            rv = (*cb)(unit, f_pl->pid, &cfg, user_data);  
            if (BCM_FAILURE(rv)) {
                break;
            }
            f_pl = f_pl->next;
        }
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function:
 *     _robo_field_stat_get
 * Purpose:
 *     Lookup a statistics entity description structure by stat id.
 * Parmeters:
 *     unit      - (IN)  BCM device number. 
 *     sid       - (IN)  Statistics entity id. 
 *     stat_p    - (OUT) Lookup result.  
 * Returns:
 *     BCM_E_XXX
 */
int
_robo_field_stat_get(int unit, int sid, _field_stat_t **stat_p)
{
    _field_stat_t    *f_st; /* Policer lookup pointer.  */
    _field_control_t *fc;   /* Field control structure. */
    uint32 hash_index;      /* Entry hash.              */

    /* Input parameters check. */
    if (NULL == stat_p) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    hash_index = sid & _FP_HASH_INDEX_MASK(fc);
    f_st  =  fc->stat_hash[hash_index];
    while (NULL != f_st) {
        /* Match entry id. */
        if (f_st->sid == sid) {
            *stat_p = f_st;
            return (BCM_E_NONE);
        }
        f_st = f_st->next;
    }

    /* Statistics entity with sid == sid was not found. */
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _robo_field_stat_id_alloc
 * Purpose:
 *      Allocate a statistics entity id.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      sid     - (OUT) Stat id.  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_stat_id_alloc(int unit, int *sid)
{
    int               max_count;           /* Maximum number of sids to try.*/
    _field_stat_t     *f_st;               /* Field stat descriptor.        */
    int               rv;                  /* Operation return status.      */ 
    static uint32  last_allocated_sid = 0; /* Policer id alloc tracker.     */ 
    
    /* Input parameters check. */
    if (NULL == sid) {
        return (BCM_E_PARAM);
    }

    max_count = _FP_ID_MAX;
    while (max_count--) {
        last_allocated_sid++;
        if (_FP_ID_MAX == last_allocated_sid) {
            last_allocated_sid = _FP_ID_BASE;
        }
        rv = _robo_field_stat_get(unit, last_allocated_sid, &f_st);
        if (BCM_E_NOT_FOUND == rv) {
            *sid = last_allocated_sid;
            return (BCM_E_NONE);
        }
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }
    return (BCM_E_RESOURCE);
}

/*
 * Function:
 *      _robo_field_stat_destroy2
 * Purpose:
 *      Deinitialize a statistics collection entity
 * Parameters:
 *      unit    - (IN) Unit number.
 *      fc      - (IN) Internal stat entity descriptor.
 *      f_st    - (IN) Field stat s
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_stat_destroy2(int unit, _field_control_t *fc, _field_stat_t *f_st)
{
    /* Input parameters check. */
    if ((NULL == fc) || (NULL == f_st)) {
        return (BCM_E_PARAM);
    }

    /* Reject destroy if policer is in use. */ 
    if (f_st->sw_ref_count > 1) {
        return (BCM_E_BUSY);
    }

    /* Remove policer for lookup hash. */
    _FP_HASH_REMOVE(fc->stat_hash, _field_stat_t, f_st,
                    (f_st->sid & _FP_HASH_INDEX_MASK(fc)));

    /* De-allocate statistics entity descriptor. */
    if (NULL != f_st->stat_arr) {
        sal_free(f_st->stat_arr);
    }
    if (NULL != f_st->stat_values) {
        sal_free(f_st->stat_values);
    }
    sal_free(f_st);

    /* Decrement number of active policers. */
    if (fc->stat_count > 0) {
        fc->stat_count--;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _robo_field_stat_destroy
 * Purpose:
 *      Deinitialize a statistics collection entity
 * Parameters:
 *      unit    - (IN) Unit number.
 *      sid     - (IN) Statistics entity id.  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_stat_destroy(int unit, int sid)
{
    _field_control_t  *fc;         /* Field control structure.     */
    _field_stat_t     *f_st;       /* Internal policer descriptor. */
    int               rv;          /* Operation return status.     */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Find stat info. */
    rv = _robo_field_stat_get(unit, sid, &f_st);
    BCM_IF_ERROR_RETURN(rv);

    return _robo_field_stat_destroy2(unit, fc, f_st);
}

/*
 * Function:
 *      _robo_field_stat_destroy_all
 * Purpose:
 *      Destroy all statistics collection entities
 * Parameters:
 *      unit    - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_robo_field_stat_destroy_all(int unit)
{
    _field_control_t *fc;   /* Field control structure. */
    int idx;                /* Entry hash iterator.     */
    int idx_max;            /* Index max.               */          
    int rv;                 /* Operation return status. */
    
    rv = _robo_field_control_get(unit, &fc);
    BCM_IF_ERROR_RETURN(rv);

    idx_max = _FP_HASH_SZ(fc);
    for (idx= 0; idx < idx_max; idx ++) {
        while (NULL != fc->stat_hash[idx]) {
            rv = _robo_field_stat_destroy2(unit, fc, fc->stat_hash[idx]);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    }
    return (rv);
}

/*
 * Function: 
 *    _robo_field_stat_array_init
 *
 * Description:
 *      Preserve API requested statistics array.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      f_st     - (IN) Field statistics entity. 
 *      nstat    - (IN) Number of elements in stat array.
 *      stat_arr - (IN) Collected statistics descriptors array.
 * Returns:
 *      BCM_E_XXX
 */
int 
_robo_field_stat_array_init(int unit, _field_stat_t *f_st, 
                       int nstat, bcm_field_stat_t *stat_arr) 
{

    /* Input parameters check. */
    if ((NULL == f_st) || (0 == nstat) || (NULL == stat_arr)) {
        return (BCM_E_PARAM);
    }

    /* Free currently allocated  stat array if any. */ 
    if (NULL != f_st->stat_arr) {
        sal_free(f_st->stat_arr);
        f_st->stat_arr = NULL;
        f_st->nstat = 0;
    }

    /* Free currently allocated  values array if any. */ 
    if (NULL != f_st->stat_values) {
        sal_free(f_st->stat_values);
        f_st->stat_values = NULL;
    }

    _FP_MEM_ALLOC(f_st->stat_arr, (nstat * sizeof (bcm_field_stat_t)),
                   "Field stat array");
    if (NULL == f_st->stat_arr) {
        return (BCM_E_MEMORY);
    }

    _FP_MEM_ALLOC(f_st->stat_values, 
                (COUNTOF(f_st->_field_x32_counters) * sizeof (uint64)),
                   "Field stat values array");
    if (NULL == f_st->stat_values) {
        sal_free(f_st->stat_arr);
        f_st->stat_arr = NULL;
        return (BCM_E_MEMORY);
    }

    /* Copy stats array. */
    sal_memcpy(f_st->stat_arr, stat_arr, (nstat * sizeof(bcm_field_stat_t)));
    f_st->nstat = nstat;

    return (BCM_E_NONE);
}

/*
 * Function: 
 *    _robo_field_stat_create
 *
 * Description:
 *       Create statistics collection entity.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      group    - (IN) Field group id. 
 *      nstat    - (IN) Number of elements in stat array.
 *      stat_arr - (IN) Collected statistics descriptors array.
 *      flags    - (IN) HW/API specific flags. 
 *      stat_id  - (OUT) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_robo_field_stat_create(int unit, bcm_field_group_t group, int nstat, 
                   bcm_field_stat_t *stat_arr, uint32 flags, int *stat_id) 
{
    _field_control_t    *fc;          /* Field control structure.     */
    _field_group_t      *fg;          /* Field group pointer.         */
    int                 rv;           /* Operation return status.     */
    _field_stat_t       *f_st = NULL; /* Internal stat descriptor.    */

    /* Input parameters check. */
    if ((nstat < 0) || (NULL == stat_id) || (nstat > bcmFieldStatCount)) {
        return (BCM_E_PARAM);
    }
    if ((0 != nstat) && (NULL == stat_arr)) {
        return (BCM_E_PARAM);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN (_robo_field_control_get(unit, &fc));

    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    BCM_IF_ERROR_RETURN(rv);

    if (_BCM_FIELD_STAGE_INGRESS != fg->stage_id) {
        return (BCM_E_UNAVAIL);
    }

    if (flags & _FP_STAT_CREATE_ID) {
        /* Check if statistics entity id already exists. */
        rv = _robo_field_stat_get(unit, *stat_id, &f_st);
        if (BCM_SUCCESS(rv)) {
            /* Stat ID already exists return */
            return (BCM_E_EXISTS);
        }
        flags = 0;
    } else {
        /* Generate statistics entity id. */
        BCM_IF_ERROR_RETURN (_robo_field_stat_id_alloc(unit, stat_id));
    }

    /* Allocate statistics descriptor. */
    _FP_MEM_ALLOC(f_st, sizeof (_field_stat_t), "Field stat entity");
    if (NULL == f_st) {
        return (BCM_E_MEMORY);
    }

    /* Initialize reference count to 1. */
    f_st->sw_ref_count = 1;

    /* Set hw index to - no hw resources allocated. */
    f_st->hw_index = _FP_INVALID_INDEX;

    /* Initialize stat id. */
    f_st->sid = *stat_id;

    /* Store caller internal flags. */
    f_st->hw_flags = flags;
    f_st->stage_id = fg->stage_id;
    f_st->gid      = fg->gid;

    /* Allocate counters array. */
    if (0 != nstat) {
        rv = _robo_field_stat_array_init(unit, f_st, nstat, stat_arr);
        if (BCM_FAILURE(rv)) {
            sal_free(f_st);
            return (rv);
        }
    }

    /* Insert policer into statistics hash. */
    _FP_HASH_INSERT(fc->stat_hash, f_st, (*stat_id & _FP_HASH_INDEX_MASK(fc)));

    /* Increment number of active policers. */
    fc->stat_count++;
    return (BCM_E_NONE);
}

/*
 * Function: 
 *    _robo_field_entry_stat_attach
 *
 * Description:
 *       Attach statistics entity to Field Processor entry.
2 * Parameters:
 *      unit      - (IN) BCM device number.
 *      entry     - (IN) Field entry id. 
 *      stat_id   - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_robo_field_entry_stat_attach(int unit, _field_entry_t *f_ent, int stat_id)
{
    _field_entry_stat_t    *f_ent_st; /* Field entry statistics structure.*/
    _field_stat_t          *f_st;     /* Internal statisics descriptor.   */
    _field_stage_id_t      stage_id;  /* Pipeline stage id.               */
    int                    rv;        /* Operation return status.         */

    /* Input parameters check. */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    /* Get entry pipeline stage id. */
    stage_id = f_ent->group->stage_id;

    if (_BCM_FIELD_STAGE_INGRESS!= stage_id) {
        return (BCM_E_UNAVAIL);
    }


    /* Check if another  statistics entity is already attached. */
    f_ent_st = &f_ent->statistic;
    if (f_ent_st->flags & _FP_ENTRY_STAT_VALID) {
        return (BCM_E_EXISTS);
    }

    /* Get statistics entity  description structure. */
    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    BCM_IF_ERROR_RETURN(rv);

    if ((f_st->stage_id != stage_id) || 
        (f_st->gid !=  f_ent->group->gid)) {
        return (BCM_E_PARAM);
    }

    /* Check if statistics entity can be shared. */
    if (f_st->sw_ref_count > 1) {
        /* statistics can't be shared in ROBO arch*/
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "Statistics can't be shared in ROBO arch")));
        return (BCM_E_PARAM);
    }

    /* Increment statistics entity reference counter. */
    f_st->sw_ref_count++;

    /* in Robo arch, entry and stat are 1-to-1 mapping */
    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        f_st->hw_index = f_ent->chain_idx;
    } else {
        f_st->hw_index = f_ent->slice_idx;
    }
    f_st->eid = f_ent->eid;        

    /* Attach statistics entity to an entry. */
    f_ent_st->flags |= _FP_ENTRY_STAT_VALID;
    f_ent_st->sid    = stat_id;

    /* Entry must be reinstalled for statistics to take effect. */
    f_ent->flags  |= _FP_ENTRY_DIRTY;

    /* Increment group counters count. */ 
    f_ent->group->group_status.counter_count++;
    return (BCM_E_NONE);
}

int
_robo_field_32bit_counter_set_update(int unit, _field_stat_t *f_st, int stat_type, uint64 value)
{
    _field_counter32_collect_t *cntrs32_buf;

    cntrs32_buf = &f_st->_field_x32_counters[stat_type];

    COMPILER_64_ZERO(cntrs32_buf->accumulated_counter);
    COMPILER_64_OR(cntrs32_buf->accumulated_counter, value);
    COMPILER_64_TO_32_LO(cntrs32_buf->last_hw_value, value);

    return BCM_E_NONE;
}
int
_robo_field_32bit_counter_set_update_hw(int unit, _field_stat_t *f_st, int stat_type, uint64 value)
{
    _field_counter32_collect_t *cntrs32_buf;

    cntrs32_buf = &f_st->_field_x32_counters[stat_type];

    COMPILER_64_TO_32_LO(cntrs32_buf->last_hw_value, value);

    return BCM_E_NONE;
}
/*
 * Function:
 *      _robo_field_stat_value_set
 *
 * Description:
 *      Set 64 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      f_st      - (IN) Statistics entity descriptor.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (IN) Collected counters value.
 *      update_hw  - (IN) if TRUE, set the value to hw only. 
 *                           Otherwise update both sw and hw.
 * Returns:
 *      BCM_E_XXX
 */
int 
_robo_field_stat_value_set(int unit, _field_stat_t *f_st, bcm_field_stat_t stat, 
                       uint64 value, int update_hw)
{
    _field_stage_t      *stage_fc;      /* Stage field control.         */
    _field_control_t    *fc;            /* Field control structure.     */
    int                 idx1;           /* First counter index to read. */
    int                 idx2;           /* Second counter index to read.*/
    int                 idx3;           /* Second counter index to read.*/
    int                 rv;             /* Operation return status.     */
    uint32          temp;
    _field_entry_t      *f_ent;   
    _field_entry_policer_t *f_ent_pl;           /* Field entry policer .     */
    _field_policer_t       *f_pl = NULL;        /* Policer descriptor.       */
    bcm_policer_mode_t  policer_mode;

    /* Input parameters check. */
    if (NULL == f_st) {
        return (BCM_E_PARAM);
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "%s stat %d value %x %x\n"),
                 FUNCTION_NAME(), stat, 
                 COMPILER_64_HI(value),COMPILER_64_LO(value)));
    /* Get field control. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Stage field control structure. */
    rv = _robo_field_stage_control_get(unit, f_st->stage_id, &stage_fc);
    BCM_IF_ERROR_RETURN(rv);

    /* Check that stat was requested during statistics entity creation. */
    for (idx1 = 0; idx1 < f_st->nstat; idx1++) {
        if (stat == f_st->stat_arr[idx1]) {
            break;
        }
    }
    if (idx1 == f_st->nstat) {
        return (BCM_E_PARAM);
    }
    rv = _robo_field_entry_get(unit, f_st->eid, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)){
        return rv;        
    }

    f_ent_pl = f_ent->policer;

    if (f_ent_pl->flags & _FP_POLICER_VALID) {
        /* Read policer configuration. */
        rv = _robo_field_policer_get(unit, f_ent_pl->pid, &f_pl);
        policer_mode = f_pl->cfg.mode;
    } else {
        policer_mode = bcmPolicerModeCount;
    }

        
    rv = DRV_FP_STAT_TYPE_GET(unit,  f_st->stage_id, policer_mode, stat, &idx1, &idx2, &idx3);
    BCM_IF_ERROR_RETURN(rv);
    temp = COMPILER_64_LO(value);

    /* Get accumulated counter value at primary index. */
    if (_FP_INVALID_INDEX != idx1) {
        if (TRUE != update_hw) {
            _robo_field_32bit_counter_set_update(unit, f_st, idx1, value);
        } else {
            _robo_field_32bit_counter_set_update_hw(unit, f_st, idx1, value);
        }
        rv = DRV_CFP_STAT_SET(unit, idx1, f_st->hw_index, temp);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Get accumulated counter value at secondary index. */
    if (_FP_INVALID_INDEX != idx2) {
        if (TRUE != update_hw) {
            _robo_field_32bit_counter_set_update(unit, f_st, idx2, value);
        } else {
            _robo_field_32bit_counter_set_update_hw(unit, f_st, idx2, value);
        }
        rv = DRV_CFP_STAT_SET(unit, idx2, f_st->hw_index, temp);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (_FP_INVALID_INDEX != idx3) {
        if (TRUE != update_hw) {
            _robo_field_32bit_counter_set_update(unit, f_st, idx3, value);
        } else {
            _robo_field_32bit_counter_set_update_hw(unit, f_st, idx3, value);
        }
        rv = DRV_CFP_STAT_SET(unit, idx3, f_st->hw_index, temp);
        BCM_IF_ERROR_RETURN(rv);
    }
    return (rv);
}

int
_robo_field_32bit_counter_get_update(int unit, _field_stat_t *f_st, int stat_type, uint32 value)
{
    uint32      diff;
    uint64      diff64;    
    _field_counter32_collect_t *cntrs32_buf;

    cntrs32_buf = &f_st->_field_x32_counters[stat_type];

    if (cntrs32_buf->last_hw_value <= value) {
        diff = value - cntrs32_buf->last_hw_value;
    } else {
        diff = (0xffffffff) - 
            (cntrs32_buf->last_hw_value - value) + 1;
    }

    COMPILER_64_SET(diff64, 0, diff);
    COMPILER_64_ADD_64(
        cntrs32_buf->accumulated_counter, diff64);

    cntrs32_buf->last_hw_value = value;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _robo_field_stat_value_get
 *
 * Description:
 *      Get 64 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      f_st      - (IN) Statistics entity descriptor.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (OUT) Collected counters value.
 * Returns:
 *      BCM_E_XXX
 */
int 
_robo_field_stat_value_get(int unit, _field_stat_t *f_st, bcm_field_stat_t stat, 
                       uint64 *value)
{
    uint32              packet_count_1; /* Primary index packet count.  */
    uint32              packet_count_2; /* Secondary index packet count.*/
    uint32              packet_count_3; /* index packet count (yellow)*/
    _field_stage_t      *stage_fc;      /* Stage field control.         */
    _field_control_t    *fc;            /* Field control structure.     */
    int                 idx1;           /* First counter index to read. */
    int                 idx2;           /* Second counter index to read.*/
    int                 idx3;           /* Second counter index to read.*/
    int                 rv;             /* Operation return status.     */
    _field_entry_t      *f_ent;   
    _field_entry_policer_t *f_ent_pl;           /* Field entry policer .     */
    _field_policer_t       *f_pl = NULL;        /* Policer descriptor.       */
    bcm_policer_mode_t  policer_mode;

    /* Input parameters check. */
    if ((NULL == value) || (NULL == f_st)) {
        return (BCM_E_PARAM);
    }

    /* Initialization. */
    packet_count_1 = 0;
    packet_count_2 = 0;
    packet_count_3 = 0;
    idx1 = idx2 = idx3 = _FP_INVALID_INDEX;

    /* Get field control. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Stage field control structure. */
    rv = _robo_field_stage_control_get(unit, f_st->stage_id, &stage_fc);
    BCM_IF_ERROR_RETURN(rv);

    /* Check that stat was requested during statistics entity creation. */
    for (idx1 = 0; idx1 < f_st->nstat; idx1++) {
        if (stat == f_st->stat_arr[idx1]) {
            break;
        }
    }
    if (idx1 == f_st->nstat) {
        return (BCM_E_PARAM);
    }

    rv = _robo_field_entry_get(unit, f_st->eid, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)){
        return rv;        
    }

    f_ent_pl = f_ent->policer;

    if (f_ent_pl->flags & _FP_POLICER_VALID) {
        /* Read policer configuration. */
        rv = _robo_field_policer_get(unit, f_ent_pl->pid, &f_pl);
        policer_mode = f_pl->cfg.mode;
    } else {
        policer_mode = bcmPolicerModeCount;
    }
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s policer_mode %d\n"),
               FUNCTION_NAME(), policer_mode));
    rv = DRV_FP_STAT_TYPE_GET(unit,  f_st->stage_id, policer_mode, stat, &idx1, &idx2, &idx3);
    BCM_IF_ERROR_RETURN(rv);

    if (f_ent->statistic.flags & _FP_ENTRY_STAT_INSTALLED) {
        /* Get accumulated counter value at primary index. */
        if (_FP_INVALID_INDEX != idx1) {
            rv = DRV_CFP_STAT_GET(unit, idx1, f_st->hw_index, &packet_count_1);
            BCM_IF_ERROR_RETURN(rv);
            _robo_field_32bit_counter_get_update(unit, f_st, idx1, packet_count_1);        
            COMPILER_64_OR(*value, f_st->_field_x32_counters[idx1].accumulated_counter);
        }
      
        /* Get accumulated counter value at secondary index. */
        if (_FP_INVALID_INDEX != idx2) {
            rv = DRV_CFP_STAT_GET(unit, idx2, f_st->hw_index, &packet_count_2);
            BCM_IF_ERROR_RETURN(rv);
            _robo_field_32bit_counter_get_update(unit, f_st, idx2, packet_count_2);        
            COMPILER_64_ADD_64(*value, f_st->_field_x32_counters[idx2].accumulated_counter);
        }

        if (_FP_INVALID_INDEX != idx3) {
            rv = DRV_CFP_STAT_GET(unit, idx3, f_st->hw_index, &packet_count_3);
            BCM_IF_ERROR_RETURN(rv);
            _robo_field_32bit_counter_get_update(unit, f_st, idx3, packet_count_3);        
            COMPILER_64_ADD_64(*value, f_st->_field_x32_counters[idx3].accumulated_counter);
        }
    }else {
        COMPILER_64_ZERO(*value);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_stat_hw_free
 *
 * Purpose:
 *     Deallocate hw counter from an entry.
 * 
 * Parameters:
 *     unit      - (IN) BCM device number.  
 *     f_ent     - (IN) Entry statistics belongs to.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_stat_hw_free (int unit, _field_entry_t *f_ent)
{
    _field_entry_stat_t  *f_ent_st;  /* Field entry statistics structure. */
    _field_stat_t        *f_st;      /* Statistics entity descriptor.     */
    uint64               value;      /* 64 bit zero to reset hw value.    */
    int                  idx;        /* Statistics iteration index.       */
    int                  rv;         /* Operation return status.          */

    /* Input parameters check. */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }
    f_ent_st = &f_ent->statistic;

    /* Skip uninstalled statistics entity. */
    if (0 == (f_ent_st->flags & _FP_ENTRY_STAT_INSTALLED)) {
        return (BCM_E_NONE);
    }

    /* Read stat entity configuration. */
    BCM_IF_ERROR_RETURN(_robo_field_stat_get(unit, f_ent_st->sid, &f_st));

    /* Decrement hw reference count. */
    if (f_st->hw_ref_count > 0) {
        f_st->hw_ref_count--;
    }

    /* Statistics is not used by any other entry. */
    if ((f_st->hw_ref_count == 0) &&  (f_st->nstat > 0)) {
        COMPILER_64_ZERO(value);

        /* Read & Reset  individual statistics. */
        for (idx = 0; idx <COUNTOF(f_st->_field_x32_counters); idx++) {
            memcpy(f_st->stat_values + idx, 
                &f_st->_field_x32_counters[idx].accumulated_counter,
                sizeof(f_st->stat_values[idx]));
        }
        for (idx = 0; idx < f_st->nstat; idx++) {
                rv = _robo_field_stat_value_set(unit, f_st, f_st->stat_arr[idx], value, FALSE);
                BCM_IF_ERROR_RETURN(rv);
        }
    }

    f_ent_st->flags &= ~_FP_ENTRY_STAT_INSTALLED;
    /* Mark entry for reinstall. */
    f_ent->flags |= _FP_ENTRY_DIRTY;
    return (BCM_E_NONE);
}

/*
 * Function: 
 *    _robo_field_entry_stat_detach
 *
 * Description:
 *       Detach statistics entity from Field processor entry.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      f_ent     - (IN) Field entry structure. 
 *      stat_id   - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_robo_field_entry_stat_detach(int unit, _field_entry_t *f_ent, int stat_id)
{
    _field_stat_t       *f_st;     /* Internal statistics entity.  */
    _field_entry_stat_t *f_ent_st; /* Field entry stat structure.  */
    int                 rv;        /* Operation return status.     */

    /* Input parameters check. */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    /* Make sure statistics entity attached to the entry. */
    f_ent_st = &f_ent->statistic;

    if (0 == (f_ent_st->flags & _FP_ENTRY_STAT_VALID)) {
        return (BCM_E_EMPTY);
    }

    /* Compare statistics entity id with attached entity id. */
    if (stat_id  != f_ent_st->sid) {
        return (BCM_E_PARAM);
    }

    /* Get statics entity descriptor structure. */
    rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
    BCM_IF_ERROR_RETURN(rv);

    /* If entry was installed decrement hw reference counter. */
    rv = _robo_field_stat_hw_free (unit, f_ent);
    BCM_IF_ERROR_RETURN(rv);

    /* Decrement statistics entity reference counter. */
    f_st->sw_ref_count--;

    /* If no one else is using the entity and it is internal destroy it. */
    if ((1 == f_st->sw_ref_count) && (f_st->hw_flags & _FP_STAT_INTERNAL)) {
        rv = _robo_field_stat_destroy(unit, f_ent_st->sid);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Detach policer from an entry. */
    f_ent_st->sid   = _FP_INVALID_INDEX;
    f_ent_st->flags = 0;

    /* Mark entry for reinstall. */
    f_ent->flags |= _FP_ENTRY_DIRTY;

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_data_qualifier_free
 *
 * Purpose:
 *     Free data qualifier descriptor structure. 
 *
 * Parameters:
 *     unit       - (IN) BCM device number. 
 *     qual       - (OUT)Qualifier strusture. 
 * 
 * Returns:
 *     BCM_E_XXX 
 */
int
_robo_field_data_qualifier_free(int unit, _field_data_qualifier_t *qual)
{
    /* Input parameters check. */
    if (NULL == qual) {
        return (BCM_E_PARAM);
    } 

    /* Free data qualifier structure. */
    sal_free(qual);

    return (BCM_E_NONE);
}


/*
 * Function:
 *     _robo_field_data_qualifier_alloc
 *
 * Purpose:
 *     Allocate data qualifier descriptor structure. 
 *
 * Parameters:
 *     unit       - (IN) BCM device number. 
 *     qual       - (OUT)Qualifier strusture. 
 * 
 * Returns:
 *     BCM_E_XXX 
 */

int
_robo_field_data_qualifier_alloc(int unit, _field_data_qualifier_t **qual)
{
    int mem_sz;                  /* Allocated memory size.  */
    _field_data_qualifier_p f_dq;/* Data qualifier pointer. */

    /* Input parameters check. */
    if (NULL == qual) {
        return (BCM_E_PARAM);
    }

    /* Allocate field data qualifier structure. */
    mem_sz = sizeof(_field_data_qualifier_t);
    f_dq = sal_alloc(mem_sz, "Data qualifier");
    if (NULL == f_dq) {
        return (BCM_E_MEMORY);
    }
    sal_memset(f_dq, 0, mem_sz);
    *qual = f_dq;

    return (BCM_E_NONE);
}


/*
 * Function:
 *     _robo_field_stage_data_ctrl_init
 *
 * Purpose:
 *     Allocate and initialize data qualifiers control structure. 
 *
 * Parameters:
 *     unit     - (IN)     BCM unit
 *     stage_fc - (IN/OUT) Stage to be initialize.
 * Returns:
 *     BCM_E_XXX 
 */
STATIC int
_robo_field_stage_data_ctrl_init(int unit, _field_stage_t *stage_fc)
{
    _field_data_control_t *data_ctrl = NULL;

    /* Input parameters check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    /* Allocate data control structure. */
    data_ctrl = sal_alloc(sizeof(_field_data_control_t),
        "Data qualification control");
    if (NULL == data_ctrl) {
        return (BCM_E_MEMORY);
    }

    sal_memset(data_ctrl, 0, sizeof(_field_data_control_t));

    stage_fc->data_ctrl = data_ctrl;
    
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _robo_field_data_qualifier_destroy
 * Purpose:
 *      Destroy a data/offset based qualifier.
 * Parameters:
 *      unit  - (IN) Bcm device number.
 *      qid   - (IN) Data qualifier id.
 *                        
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_data_qualifier_destroy(int unit, int qid) 
{
    _field_data_qualifier_t *f_dq;/* Internal data qualifier descriptor. */
    _field_data_qualifier_t *f_dq_prev;/* Prev data qualifier descriptor.*/
    _field_stage_t  *stage_fc;    /* Stage field control.                */
    _field_control_t  *fc;    /* Field control. */     
    int rv;                                     

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_control_get(unit, &fc); 
    BCM_IF_ERROR_RETURN(rv);

    f_dq_prev = f_dq = stage_fc->data_ctrl->data_qual;
    while (NULL != f_dq) {
        if (f_dq->qid == qid) {
            /* if there is still some field groups use it */
            if (!fc->udf[qid].valid) {
                /* UDF already destroyed */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: udf=%d not configured in unit=%d.\n"),
                           qid, unit));
                return BCM_E_PARAM;
            }
            if (fc->udf[qid].use_count) {
                /* UDF still in use; do not destroy */
                return BCM_E_EXISTS;
            }

            fc->udf[qid].valid = 0;
            fc->udf[f_dq->qid].data_qualifier = 0;

            /* Remove qualifier from qualifiers linked list. */
            if (f_dq == f_dq_prev) {
                stage_fc->data_ctrl->data_qual = f_dq->next;
            } else {
                f_dq_prev->next = f_dq->next;
            }
            /* Free qualifieer allocated memory. */
            return _robo_field_data_qualifier_free(unit, f_dq);
        }
        f_dq_prev = f_dq;
        f_dq = f_dq->next;
    }
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _field_data_qualifier_destroy_all
 * Purpose:
 *      Destroy all data/offset  based qualifiers.
 * Parameters:
 *      unit           - (IN) bcm device.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_data_qualifier_destroy_all(int unit) 
{
    _field_data_qualifier_t *f_dq;/* Internal data qualifier descriptor. */
    _field_stage_t  *stage_fc;    /* Stage field control.                */
    int rv;                       /* Operation return status.            */

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc); 
    BCM_IF_ERROR_RETURN(rv);
    
    while (NULL != stage_fc->data_ctrl->data_qual) {
        f_dq = stage_fc->data_ctrl->data_qual; 
        BCM_IF_ERROR_RETURN (_robo_field_data_qualifier_destroy(unit, f_dq->qid));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_stage_data_ctrl_deinit
 *
 * Purpose:
 *     De-allocate data qualifiers control structure. 
 *
 * Parameters:
 *     unit     - (IN)     BCM unit
 *     stage_fc - (IN/OUT) Stage to be initialize.
 * Returns:
 *     BCM_E_XXX 
 */
STATIC int
_robo_field_stage_data_ctrl_deinit(int unit, _field_stage_t *stage_fc)
{
    _field_data_control_t *data_ctrl;

    /* Input parameters check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    if (stage_fc->stage_id != _BCM_FIELD_STAGE_INGRESS) {
        return (BCM_E_NONE);
    }                   

    data_ctrl = stage_fc->data_ctrl;
    if (NULL == data_ctrl) {
        return (BCM_E_NONE);
    }

    /* Delete all data qualifiers. */
    _robo_field_data_qualifier_destroy_all(unit) ;

    /* Free control structures. */
    if (NULL != data_ctrl->tcam_entry_arr) {
        sal_free(data_ctrl->tcam_entry_arr);
    }

    if (NULL != data_ctrl) {
        sal_free(data_ctrl);
    }

    stage_fc->data_ctrl  = NULL;
    return (BCM_E_NONE);
}



/*
 * Function:
 *      _robo_field_data_qualifier_get
 * Purpose:
 *      Get data qualifier by id.
 * Parameters:
 *      unit           - (IN) bcm device.
 *      stage_fc       - (IN) Stage field control.
 *      qid            - (IN) Qualifier id. 
 *      data_qual      - (OUT) Internal data qualifier descriptor:
 *                         
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_data_qualifier_get(int unit, _field_stage_t *stage_fc,  
                              int qid, _field_data_qualifier_t **data_qual)
{
    _field_data_qualifier_p f_dq;  /* Field data qualifier iterator. */

    /* Input parameters check. */
    if((NULL == data_qual) ||  (NULL == stage_fc)){
        return (BCM_E_PARAM);
    } 

    f_dq = stage_fc->data_ctrl->data_qual; 
    while (NULL != f_dq) {
        if (f_dq->qid == qid) {
            *data_qual = f_dq;
            return (BCM_E_NONE);
        }
        f_dq = f_dq->next;
    }
    return (BCM_E_NOT_FOUND);
}


/*
 * Function:
 *      _robo_field_data_qualifier_id_alloc
 * Purpose:
 *      Allocate unused date qualifier id.
 * Parameters:
 *      unit           - (IN) bcm device.
 *      stage_fc       - (IN) Stage field control.
 *      data_qualifier - (IN) API level qualifier descriptor:
 *                         
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_data_qualifier_id_alloc(int unit, _field_control_t *fc,
                            _field_stage_t *stage_fc,  
                             bcm_field_data_qualifier_t *data_qualifier)
{
    _field_data_qualifier_t *f_dq;    /* Data qualifier descriptor.    */
    int qid;                          /* Qualifier id.                 */
    int rv;                           /* Operation return status.      */
    uint32      udf_num = 0;
    int free_udf_id, tuid;
    int s_udf_id, check_id;


    /* Input parameters check. */
    if ((NULL == data_qualifier) || (NULL == stage_fc)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num));

    if (udf_num == 0) {
        /* No UDF supported */
        return (BCM_E_UNAVAIL);
    }

    /* If qualifier id was passed by the caller, 
       verify the range and make sure id is not used. */
    if (data_qualifier->flags & BCM_FIELD_DATA_QUALIFIER_WITH_ID) {
        qid = data_qualifier->qual_id;
        
        if (qid >= udf_num) {
            return BCM_E_PARAM;
        }

        /* Update the hw configuration */
        if (fc->udf[qid].valid) {
            if (!(data_qualifier->flags & BCM_FIELD_DATA_QUALIFIER_REPLACE)) {
                /* "Replace" flag was not specified => Return error */

                return (BCM_E_EXISTS);
            }
        }
        if (SOC_IS_VO(unit)){
            s_udf_id = qid % 14;
            check_id = -1;
            if ((s_udf_id == 0) ||(s_udf_id == 2)){
                check_id = qid + 1;
            } else if ((s_udf_id == 1) ||(s_udf_id == 3)){
                check_id = qid - 1;
            }
            if (check_id  != -1){
                if (fc->udf[check_id].valid) {
                    if (!(data_qualifier->flags & BCM_FIELD_DATA_QUALIFIER_REPLACE)) {
                        /* "Replace" flag was not specified => Return error */

                        return (BCM_E_EXISTS);
                    }
                }
            }     
        }
        
        rv = _robo_field_data_qualifier_get(unit, stage_fc, qid, &f_dq);
        if (BCM_SUCCESS(rv)) {
            /*  Data qual with requested id already exists */

            if (!(data_qualifier->flags & BCM_FIELD_DATA_QUALIFIER_REPLACE)) {
                /* "Replace" flag was not specified => Return error */

                return (BCM_E_EXISTS);
            }

            /* Destroy existing one */

            _robo_field_data_qualifier_destroy(unit, qid);
        }
        return (BCM_E_NONE);
    } 

    free_udf_id = udf_num;
    for (tuid = 0; tuid < udf_num; tuid++) {
        if (!fc->udf[tuid].valid) {

            if (SOC_IS_VO(unit)){
                s_udf_id = tuid % 14;
                check_id = -1;
                if ((s_udf_id == 0) ||(s_udf_id == 2)){
                    check_id = tuid + 1;
                } else if ((s_udf_id == 1) ||(s_udf_id == 3)){
                    check_id = tuid - 1;
                }
                if (check_id  != -1){
                    if (fc->udf[check_id].valid) {
                        continue;
                    }
                }     
            }
            
            if (udf_num == free_udf_id) {
                free_udf_id = tuid;
                break;
            }
        }
        
    }

    /* No UDFs to share; available UDF? */
    if (udf_num == free_udf_id) {
        return (BCM_E_RESOURCE);
    }

    data_qualifier->qual_id = free_udf_id;

    return (BCM_E_NONE);
}

int
_robo_field_udf_offset_base_supported(int unit, bcm_field_data_offset_base_t offset_base)
{
    uint16 dev_id= 0;
    uint8 rev_id = 0;
    
    if (SOC_IS_VULCAN(unit)||SOC_IS_TBX(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        switch (offset_base) {
            case bcmFieldDataOffsetBasePacketStart:
            case bcmFieldDataOffsetBaseOuterL4Header:
            case bcmFieldDataOffsetBaseOuterL3Header:
                break;
            case bcmFieldDataOffsetBaseEndTag:
                if (!SOC_IS_VO(unit)) {
                    return (BCM_E_PARAM);
                }
                break;
            default:
                return (BCM_E_PARAM);
        }
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        soc_cm_get_id(unit, &dev_id, &rev_id);
        switch (offset_base) {
            case bcmFieldDataOffsetBasePacketStart:
                if ((rev_id != BCM53242_B0_REV_ID) && 
                    (rev_id != BCM53242_B1_REV_ID)) {
                    return (BCM_E_PARAM);
                    
                }
                break;
            case bcmFieldDataOffsetBaseOuterL4Header:
            case bcmFieldDataOffsetBaseOuterL3Header:
            case bcmFieldDataOffsetBaseEndTag:
                break;
            default:
                return (BCM_E_PARAM);
        }
    } else {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

int
_robo_field_data_qualifier_init(int                        unit,
                           _field_stage_t             *stage_fc,
                           _field_data_qualifier_t    *f_dq,
                           bcm_field_data_qualifier_t *data_qualifier
                           )
{
    uint32 offset_max = 0;
    bcm_pbmp_t  pbmp;
    bcm_port_t  port;
    uint32  offset = 0, offset_base = 0;

    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET   
        (unit, DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX, &offset_max));

    if (f_dq->offset > offset_max) {
        return (BCM_E_PARAM); 
    }

    if (f_dq->offset_base > offset_max) {
        return (BCM_E_PARAM); 
    }

    /* Check if the UDF offset base is supported */
    BCM_IF_ERROR_RETURN(
        _robo_field_udf_offset_base_supported(unit, f_dq->offset_base));
    
    f_dq->qid         = data_qualifier->qual_id;
    f_dq->offset_base = data_qualifier->offset_base;
    f_dq->offset      = data_qualifier->offset;
    f_dq->length      = data_qualifier->length;
    f_dq->flags       = data_qualifier->flags;

    f_dq->next        = stage_fc->data_ctrl->data_qual;            
    stage_fc->data_ctrl->data_qual = f_dq; 

    if (SOC_IS_TBX(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit) ||
        SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* UDFs are system base for TBX*/
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_SET(pbmp, 0);
    } else {
        pbmp = PBMP_PORT_ALL(unit);
    }
    PBMP_ITER(pbmp, port) {
        offset = data_qualifier->offset;
        switch (data_qualifier->offset_base) {
            case bcmFieldDataOffsetBasePacketStart:
                offset_base = DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME;
                break;
            case bcmFieldDataOffsetBaseOuterL3Header:
                offset_base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR;
                break;
            case bcmFieldDataOffsetBaseOuterL4Header:
                offset_base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR;
                break;
            case bcmFieldDataOffsetBaseEndTag:
                offset_base = DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG;
                break;
            default:
                return (BCM_E_PARAM);
        }

        BCM_IF_ERROR_RETURN(
            DRV_CFP_UDF_SET
            (unit, port, (uint32)f_dq->qid, offset, offset_base));
    }
    
    return (BCM_E_NONE);
}


STATIC int
_robo_field_data_qualifier_create(int unit,    _field_control_t *fc,
                             bcm_field_data_qualifier_t *data_qualifier)
{
    _field_data_qualifier_t *f_dq;/* Internal data qualifier descriptor. */
    _field_stage_t  *stage_fc;    /* Stage field control.                */
    int rv;                       /* Operation return status.            */

    /* Input parameters check. */
    if (NULL == data_qualifier) {
        return (BCM_E_PARAM);
    }

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc);
    BCM_IF_ERROR_RETURN(rv);

    /* Allocate data qualifier id. */
    rv = _robo_field_data_qualifier_id_alloc(unit, fc, stage_fc, data_qualifier);
    BCM_IF_ERROR_RETURN(rv);

    /* Allocated internal data qualifier descriptor. */
    rv = _robo_field_data_qualifier_alloc(unit, &f_dq);
    BCM_IF_ERROR_RETURN(rv);

    /* Initialize internal data qualifier record from given one */
    rv = _robo_field_data_qualifier_init(unit, stage_fc, f_dq, data_qualifier);
    if (BCM_FAILURE(rv)) {
        goto error_cleanup;
    }

    fc->udf[f_dq->qid].valid = 1;
    fc->udf[f_dq->qid].use_count = 0;
    fc->udf[f_dq->qid].data_qualifier = 1;

    return (BCM_E_NONE);

 error_cleanup:
    _robo_field_data_qualifier_destroy(unit, f_dq->qid);
    return (rv);
}

/*
 * Function: _robo_field_qualify_data
 *
 * Purpose:
 *
 * Parameters:
 *     unit     - (IN) BCM device number.
 *     eid      - (IN) Entry ID.
 *     qual_id  - (IN) Data qualifier id.
 *     data     - (IN) Data bytes for the indicated data qualifier.
 *                     Network byte ordered. 
 *     mask     - (IN) Mask bytes for the indicated data qualifier.
 *                     Network byte ordered.
 *     length   - (IN) data/mask length.
 * Returns:
 *     BCM_E_XXX   
 */
int
_robo_field_qualify_data (int unit, bcm_field_entry_t eid, int qual_id,
                     uint8 *data, uint8 *mask, uint16 length)
{
    _field_stage_t          *stage_fc;  /* Stage field control.       */
    _field_data_qualifier_t *f_dq;      /* Data qualifier descriptor. */
    _field_control_t        *fc;        /* Field control structure.   */
    int                     rv, i;         /* Operation return status.   */
    uint32      udf_data, udf_mask;
    uint32      udf_length;
    uint32      udf_id;

    /* Input parameters check. */
    for (i = 0; i < length; i++) {
        if ((NULL == data) || (NULL == mask)) {
            return (BCM_E_PARAM);
        }
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc); 
    BCM_IF_ERROR_RETURN(rv);

    /* Get data qualifier info. */
    rv = _robo_field_data_qualifier_get(unit, stage_fc, qual_id,  &f_dq);
    BCM_IF_ERROR_RETURN(rv);

    udf_id = f_dq->qid;
    udf_data = 0;
    udf_mask = 0;

    if (SOC_IS_TBX(unit) || SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = DRV_CFP_UDF_GET(unit, DRV_CFP_UDF_LENGTH_GET, udf_id,
            &udf_length, NULL);
        if (BCM_FAILURE(rv)) {
            return rv;
        }        
    } else {
        udf_length = 4;
    }

    if (length > udf_length) {
        return (BCM_E_PARAM);
    }
    
    for ( i = 0; i< udf_length; i ++) {

        if (i%4) {
            udf_data = udf_data << 8;
            udf_mask = udf_mask << 8;
        }
        udf_data |= *(data +( i%4));
        udf_mask |= *(mask +( i%4));
    }

    rv = _robo_field_udf_value_set(unit, eid,
        udf_id, &udf_data, &udf_mask);
    
    return (rv);
}



/*
 * Function:
 *      bcm_robo_field_data_qualifier_create
 * Purpose:
 *      Create an data/offset based qualifier.
 * Parameters:
 *      unit           - (IN) bcm device.
 *      data_qualifier - (IN) Qualifier descriptor:
 *                           such as packet type, byte offset, etc.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_create(int unit,  
                                 bcm_field_data_qualifier_t *data_qualifier)
{
    _field_control_t    *fc;     /* Field control structure.     */
    int                 rv;      /* Operation return status.     */

    /* Input parameters check. */
    if (NULL == data_qualifier) {
        return (BCM_E_PARAM);
    }

    if (data_qualifier->flags & 
        (BCM_FIELD_DATA_QUALIFIER_OFFSET_IP4_OPTIONS_ADJUST |
        BCM_FIELD_DATA_QUALIFIER_OFFSET_IP6_EXTENSIONS_ADJUST |
        BCM_FIELD_DATA_QUALIFIER_OFFSET_GRE_OPTIONS_ADJUST)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);
    
    rv = _robo_field_data_qualifier_create(unit, fc, data_qualifier); 

    FP_UNLOCK(fc);
    return (rv);
}


/*
 * Function:
 *      bcm_robo_field_data_qualifier_multi_get
 *
 * Purpose:
 *      Return list of ids of defined data qualifiers, per standard API idiom.
 *
 * Parameters:
 *      unit       - (IN)  bcm device.
 *      qual_size  - (IN)  Size of given qualifier id array; if 0, indicates
 *                         return count of data qualifiers only.
 *      qual_array - (OUT) Base of array where to store returned data qualifier
 *                         ids.
 *      qual_count - (OUT) Number of qualifier ids stored in above array; if
 *                         qual_size was given as 0, then number of defined
 *                         qualifiers. 
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_field_data_qualifier_multi_get(int unit, int qual_size, int *qual_array, int *qual_count)
{
    _field_control_t        *fc;
    _field_stage_t          *stage_fc;
    _field_data_control_t   *data_ctrl;
    _field_data_qualifier_t *f_dq;
    unsigned                dqcnt;

    /* Parameter checking */

    if (qual_count == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: qual_count == NULL.\n"),
                   unit));
        return (BCM_E_PARAM);
    }

    if (qual_size != 0 && qual_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: qual_array == NULL.\n"),
                   unit));
        return (BCM_E_PARAM);
    }

    /* Point to needed things */

    BCM_IF_ERROR_RETURN (_robo_field_control_get(unit, &fc));
    BCM_IF_ERROR_RETURN(_robo_field_stage_control_get(unit,
                                                 _BCM_FIELD_STAGE_INGRESS,
                                                 &stage_fc
                                                 )
                        );
    data_ctrl = stage_fc->data_ctrl;

    FP_LOCK(fc);

    /* Count defined data qualifiers */

    for (dqcnt = 0, f_dq = data_ctrl->data_qual;
         f_dq;
         f_dq = f_dq->next, ++dqcnt
         );

    if (qual_size == 0) {
        /* Return count of data qualifiers only */

        *qual_count = dqcnt;
    } else {
        /* Return array of data qualifier ids */ 

        if (qual_size > dqcnt) {
            qual_size = dqcnt;
        }
        
        *qual_count = qual_size;

        for (f_dq = data_ctrl->data_qual;
             qual_size != 0 && f_dq != NULL;
             --qual_size, f_dq = f_dq->next, ++qual_array
             ) {
            *qual_array = f_dq->qid;
        }
    }

    FP_UNLOCK(fc);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_field_data_qualifier_get
 *
 * Purpose:
 *      Return configuration of given data qualifier.
 *
 * Parameters:
 *      unit       - (IN)  bcm device.
 *      qual_id    - (IN)  Id of data qualifier.
 *      qual       - (OUT) Attributes of given data qualifier.
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_field_data_qualifier_get(int unit, int qual_id, bcm_field_data_qualifier_t *qual)
{
    int                     rv;
    _field_control_t        *fc;
    _field_stage_t          *stage_fc;
    _field_data_qualifier_t *f_dq;

    /* Parameter checking */

    if (qual == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: qual == NULL.\n"),
                   unit));
        return (BCM_E_PARAM);
    }

    /* Point to needed things */

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    BCM_IF_ERROR_RETURN(_robo_field_stage_control_get(unit,
                                                 _BCM_FIELD_STAGE_INGRESS,
                                                 &stage_fc
                                                 )
                        );

    FP_LOCK(fc);
                        
    if ((rv = _robo_field_data_qualifier_get(unit,
                                            stage_fc,
                                            qual_id,
                                            &f_dq
                                            )
         )
        == BCM_E_NONE
        ) {
        qual->qual_id     = f_dq->qid;
        qual->flags       = f_dq->flags;
        qual->offset_base = f_dq->offset_base;
        qual->offset      = f_dq->offset;
        qual->length      = f_dq->length;
    }

    FP_UNLOCK(fc);

    return (rv);
}


/*
 * Function:
 *      bcm_robo_field_data_qualifier_destroy
 * Purpose:
 *      Destroy an data/offset based qualifier.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      qual_id  - (IN)  Data qualifier id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_destroy(int unit, int qual_id)
{
    _field_control_t    *fc;     /* Field control structure.     */
    int                 rv;      /* Operation return status.     */

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_data_qualifier_destroy(unit, qual_id); 

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function:
 *      bcm_robo_field_data_qualifier_destroy_all
 * Purpose:
 *      Delete all data/offset based qualifiers.
 * Parameters:
 *      unit          - (IN)  bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_destroy_all(int unit)
{
    _field_control_t    *fc;     /* Field control structure.     */
    int                 rv;      /* Operation return status.     */

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_data_qualifier_destroy_all(unit); 

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function:
 *      bcm_robo_field_qset_data_qualifier_get
 * Purpose:
 *      Get field data qualifiers included in group qset. 
 * Parameters:
 *      unit          - (IN) bcm device.
 *      qset          - (IN) Group qualifier set.
 *      qual_max      - (IN) Maximum qualifiers to fill.
 *      qual_arr      - (OUT) Data qualifiers array.
 *      qual_count    - (OUT) Number of data qualifiers filled in qual_arr.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_qset_data_qualifier_get(int unit, bcm_field_qset_t qset, int qual_max,
                                  int *qual_arr, int *qual_count) 
{
    _field_data_qualifier_p f_dq;  /* Field data qualifier iterator. */
    _field_control_t *fc;          /* Field control structure.       */
    _field_stage_t *stage_fc;      /* Stage field control structure. */
    bcm_field_qset_t  qset_temp;   /* Temporary qset copy.           */
    int count;                     /* Filled entries count.          */
    int rv;                        /* Operation return status.       */

    /* Input parameters check. */
    if ((qual_max <= 0) || (NULL == qual_arr) ||
        (NULL == qual_count)) {
        return BCM_E_PARAM;
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc); 
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Reset caller provided array. */
    sal_memset(qual_arr, 0, qual_max * sizeof(int));

    /* Copy qset to a temporary variable. */
    sal_memcpy(&qset_temp, &qset, sizeof(bcm_field_qset_t));

    /* Fill caller array. */
    count = 0;
    f_dq = stage_fc->data_ctrl->data_qual; 


    /* Stage data qualifiers iteration array. */
    while (NULL != f_dq) {
        if (SHR_BITGET(qset_temp.udf_map, f_dq->qid)) {
            /* Remove data qualifier udfs from the temporary qset. */
            SHR_BITCLR(qset_temp.udf_map, f_dq->qid);
            /* Add data qualifier id to caller array. */
            qual_arr[count] = f_dq->qid;
            count++;
            if (count >= qual_max) {
                break;
            }
        }
        f_dq = f_dq->next;
    }
    *qual_count = count;

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_field_qset_data_qualifier_add
 * Purpose:
 *      Add field data qualifier to group qset.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      qset          - (IN/OUT) Group qualifier set.
 *      qualifier_id  - (IN) Data qualifier id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qset_data_qualifier_add(int unit, bcm_field_qset_t *qset,  
                                      int qual_id)
{
    _field_stage_t          *stage_fc;  /* Stage field control.       */
    _field_data_qualifier_t *f_dq;      /* Data qualifier descriptor. */
    _field_control_t        *fc;        /* Field control structure.   */
    int                     rv;         /* Operation return status.   */
    
    /* Input parameters check. */
    if (NULL == qset) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc); 
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /*Get data qualifier info. */
    rv = _robo_field_data_qualifier_get(unit, stage_fc, qual_id,  &f_dq);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    SHR_BITSET(qset->udf_map, qual_id); /* in use */


    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_field_qualify_data
 * Purpose:
 *      Set data/mask in the search engine for entry field data qualifier.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      eid           - (IN) Entry id. 
 *      qual_id       - (IN) Data qualifier id.
 *      data          - (IN) Match data.
 *      mask          - (IN) Match mask.
 *      length        - (IN) Length of data & mask arrays.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qualify_data(int unit, bcm_field_entry_t eid, int qual_id,
                           uint8 *data, uint8 *mask, uint16 length)
{
    _field_control_t        *fc;        /* Field control structure.   */
    int                     rv;         /* Operation return status.   */

    /* Input parameters check. */
    if ((NULL == data) || (NULL == mask)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_qualify_data(unit, eid, qual_id, data, mask, length);

    FP_UNLOCK(fc);
    return (rv);
}


/*
 * Function:
 *      bcm_robo_field_data_qualifier_ip_protocol_add
 * Purpose:
 *      Add ip protocol based offset to data qualifier object.
 * Parameters:
 *      unit        - (IN) bcm device.
 *      qual_id     - (IN) Data qualifier id.
 *      ip_protocol - (IN) Ethertype based offset specification.                 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_ip_protocol_add(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_data_qualifier_ip_protocol_delete
 * Purpose:
 *      Remove ip protocol based offset from data qualifier object. 
 * Parameters:
 *      unit        - (IN) bcm device.
 *      qual_id     - (IN) Data qualifier id.
 *      ip_protocol - (IN) Ethertype based offset specification.                 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_ip_protocol_delete(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_data_qualifier_ethertype_add
 * Purpose:
 *      Add ethertype based offset to data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_ethertype_add(int unit,  int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_data_qualifier_ethertype_delete
 * Purpose:
 *      Remove ethertype based offset from data qualifier object. 
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.                 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_ethertype_delete(int unit, int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      bcm_robo_field_data_qualifier_packet_format_add
 * Purpose:
 *      Add packet format based offset to data qualifier object.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      qual_id       - (IN) Data qualifier id.
 *      packet_format - (IN) Ethertype based offset specification.                 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_packet_format_add(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_esw_field_data_qualifier_packet_format_delete
 * Purpose:
 *      Remove packet format based offset from data qualifier object. 
 * Parameters:
 *      unit           - (IN) bcm device.
 *      qual_id        - (IN) Data qualifier id.
 *      packet_format  - (IN) Ethertype based offset specification.                 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_data_qualifier_packet_format_delete(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_field_qualify_SrcGport
 * Purpose:
 *     Add Source ModPort or MPLS/MiM/WLAN port qualification to a
 *     field entry.
 * Parameters:
 *     unit     - (IN) BCM device number
 *     entry    - (IN) Field Entry id.
 *     port_id  - (IN) Source Generic Logical port or virtual port id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qualify_SrcGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_field_qualify_SrcGport_get
 * Purpose:
 *     Get Source ModPort or MPLS/MiM/WLAN gport value from a
 *     field entry.
 * Parameters:
 *     unit     - (IN) BCM device number
 *     entry    - (IN) Field Entry id.
 *     port_id  - (OUT) ModPort Gport or MPLS/MiM/WLAN Gport ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qualify_SrcGport_get(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_gport_t *port_id)
{
    return BCM_E_UNAVAIL;
}
#ifdef BROADCOM_DEBUG
/*
 * Function:
 *     _robo_field_qual_name
 * Purpose:
 *     Translate a Qualifier enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_qualify_e. (ex.bcmFieldQualifyInPorts)
 * Returns:
 *     Text name of indicated qualifier enum value.
 */
STATIC char *
_robo_field_qual_name(bcm_field_qualify_t qid)
{
    /* Text names of the enumerated qualifier IDs. */
    static char *qual_text[bcmFieldQualifyCount] = BCM_FIELD_QUALIFY_STRINGS;

    return (qid >= bcmFieldQualifyCount ? "??" : qual_text[qid]);
}

/*
 * Function:
 *     _robo_field_action_name
 * Purpose:
 *     Return text name of indicated action enum value.
 */
STATIC char *
_robo_field_action_name(bcm_field_action_t action)
{
    /* Text names of Actions. These are used for debugging output and CLIs.
     * Note that the order needs to match the bcm_field_action_t enum order.
     */
    static char *action_text[] = BCM_FIELD_ACTION_STRINGS;
    assert(COUNTOF(action_text)     == bcmFieldActionCount);

    return (action >= bcmFieldActionCount ? "??" : action_text[action]);
}

/*
 * Function:
 *     _robo_field_group_mode_name
 * Purpose:
 *     Return text name of indicated group mode enum value.
 */
STATIC char *
_robo_field_group_mode_name(bcm_field_group_mode_t mode)
{
    static char *mode_text[bcmFieldGroupModeCount] =
                 BCM_FIELD_GROUP_MODE_STRINGS;

    return (mode >= bcmFieldGroupModeCount ? "??" : mode_text[mode]);
}
STATIC char *
_robo_field_qual_IpType_name(bcm_field_IpType_t type)
{
    /* Text names of the enumerated qualifier IpType values. */
    /* All these are prefixed with "bcmFieldIpType" */
    static char *iptype_text[bcmFieldIpTypeCount] = BCM_FIELD_IPTYPE_STRINGS;

    assert(COUNTOF(iptype_text) == bcmFieldIpTypeCount);

    return (type >= bcmFieldIpTypeCount ? "??" : iptype_text[type]);
}

/*
 * Function:
 *     _field_stage_name
 * Purpose:
 *     Translate group pipeline  stage to a text string.
 * Parameters:
 *     stage_fc stage field control structure. 
 * Returns:
 *     Text name of indicated stage qualifier enum value.
 */
STATIC char *
_robo_field_stage_name(_field_stage_t *stage_fc)
{
    static char *stage_text[] = _BCM_FIELD_STAGE_STRINGS;

    if (stage_fc->stage_id >= COUNTOF(stage_text)) {
        return "??";
    }
    return stage_text[stage_fc->stage_id];
}
#endif /* BROADCOM_DEBUG */

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *     bcm_robo_field_group_flush
 * Purpose:
 *     Flush the entries belonging to a given group; Removes
 *     hw entries from the slice and destroys sw instances;
 *     Destroys group sw object as well
 * Parameters:
 *     unit   - BCM device number
 *     group  - Field Group ID
 * Returns:
 *     BCM_E_PARAM
 *     BCM_E_NONE
 */
int
bcm_robo_field_group_flush(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


#else

int
bcm_robo_field_group_flush(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *     _robo_field_action_meter_config
 * Purpose:
 *     Set meter mode.
 * Parameters:
 *     unit     - (IN) BCM device number. 
 *     f_ent    - (IN) Entry meter belongs to.
 *     param0   - (IN) Meter mode.
 *     param1   - (IN) Committed/Peak selector.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_robo_field_action_meter_config(int unit, _field_entry_t *f_ent,  
                           uint32 param0, uint32 param1)
{
    _field_entry_policer_t *f_ent_pl; /* Field entry policer structure.*/
    _field_policer_t       *f_pl;     /* Policer descriptor.           */

    /* Input parameter check. */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    f_ent_pl = f_ent->policer;
    if (0 == (f_ent_pl->flags & _FP_POLICER_VALID)) {
        return (BCM_E_PARAM);
    }

    /* Get policer config. */
    BCM_IF_ERROR_RETURN(_robo_field_policer_get(unit, f_ent_pl->pid, &f_pl));

    /* mode = 4 & 5 are undefined */
    if (param0 == 4 || param0 == 5 || param0 > 7) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: invalid meter mode=%d\n"),
                   unit, param0));
        return (BCM_E_PARAM);
    }

    /* Set meter mode. */
    switch (param0) {
      case BCM_FIELD_METER_MODE_DEFAULT:
      case BCM_FIELD_METER_MODE_FLOW:
          if ((0 == param1) || (BCM_FIELD_METER_PEAK == param1)) {
              f_pl->cfg.mode = bcmPolicerModePeak;
              f_pl->hw_flags |= _FP_POLICER_PEAK_DIRTY;
          } else {
              f_pl->cfg.mode = bcmPolicerModeCommitted;
              f_pl->hw_flags |= _FP_POLICER_COMMITTED_DIRTY;
          }
          break;
      case BCM_FIELD_METER_MODE_trTCM_COLOR_BLIND:
          f_pl->cfg.mode = bcmPolicerModeTrTcm; 
          f_pl->cfg.flags |= BCM_POLICER_COLOR_BLIND; 
          f_pl->hw_flags |= _FP_POLICER_DIRTY;
          break;
      case BCM_FIELD_METER_MODE_trTCM_COLOR_AWARE:
          f_pl->cfg.mode = bcmPolicerModeTrTcm; 
          f_pl->hw_flags |= _FP_POLICER_DIRTY;
          break;
      case BCM_FIELD_METER_MODE_srTCM_COLOR_BLIND:
          f_pl->cfg.mode = bcmPolicerModeSrTcm; 
          f_pl->cfg.flags |= BCM_POLICER_COLOR_BLIND; 
          f_pl->hw_flags |= _FP_POLICER_DIRTY;
          break;
      case BCM_FIELD_METER_MODE_srTCM_COLOR_AWARE:
          f_pl->cfg.mode = bcmPolicerModeSrTcm; 
          f_pl->hw_flags |= _FP_POLICER_DIRTY;
          break;
      default:
          return (BCM_E_PARAM);
    }

	f_ent->flags          |= _FP_ENTRY_DIRTY;

    return (BCM_E_NONE);
}

STATIC int
_robo_field_action_update_counter(int unit, _field_entry_t *f_ent,
                             uint32 param0, uint32 param1)
{
    _field_entry_stat_t    *f_ent_st;  /* Field entry stat structure.  */
    _field_stat_t          *f_st;      /* Field statistics descriptor. */
    bcm_field_stat_t       stat_arr[2];/* Requested statistic array.   */
    uint8                  nstat;      /* Statistics array size.       */
    int                    rv;         /* Operation return status.     */
    int mode;

    /* Input parameters check. */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }
    f_ent_st = &f_ent->statistic;

    /* Reject unused counters. */
    if (0 == (f_ent_st->flags & _FP_ENTRY_STAT_VALID)) {
        return (BCM_E_PARAM);
    }
    
    /* check support */
    if (param0 & BCM_FIELD_COUNTER_MODE_BYTES) {
        return (BCM_E_PARAM);
    }
    if (f_ent->policer[0].flags & _FP_POLICER_VALID){
        mode = 1;
    } else {
        mode = 0;
    }        
    rv = DRV_FP_STAT_SUPPORT_CHECK(unit, f_ent->fs->stage_id, 
        _DRV_FP_STAT_OP_COUNTER_MODE, param0, (void *)&mode);
    BCM_IF_ERROR_RETURN(rv);


    /* Free counters if entry was previously installed. */
    if (f_ent_st->flags & _FP_ENTRY_STAT_INSTALLED) {
        rv = _robo_field_stat_hw_free (unit, f_ent);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Read policer configuration. */
    BCM_IF_ERROR_RETURN(_robo_field_stat_get(unit, f_ent_st->sid, &f_st));


    switch (param0) {
      case BCM_FIELD_COUNTER_MODE_NO_NO:
          f_ent_st->flags |= _FP_ENTRY_STAT_EMPTY; 
          return (BCM_E_NONE);
      case BCM_FIELD_COUNTER_MODE_DEFAULT: 
            /* BCM_FIELD_COUNTER_MODE_NO_YES*/
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          nstat = 2;
          stat_arr[1] = bcmFieldStatNotGreenPackets; 
          stat_arr[0] = bcmFieldStatGreenPackets; 
          break;
      case BCM_FIELD_COUNTER_MODE_YES_NO:
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          nstat = 2;
          stat_arr[1] = bcmFieldStatGreenPackets; 
          stat_arr[0] = bcmFieldStatNotGreenPackets; 
          break;
      case BCM_FIELD_COUNTER_MODE_RED_NOTRED:
          nstat = 2;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[1] = bcmFieldStatRedPackets; 
          stat_arr[0] = bcmFieldStatNotRedPackets; 
          break;
      case BCM_FIELD_COUNTER_MODE_GREEN_NOTGREEN:
          nstat = 2;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[1] = bcmFieldStatGreenPackets;
          stat_arr[0] = bcmFieldStatNotGreenPackets;
          break;
      case BCM_FIELD_COUNTER_MODE_GREEN_RED:
          nstat = 2;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[1] = bcmFieldStatGreenPackets;
          stat_arr[0] = bcmFieldStatRedPackets;
          break;
      case BCM_FIELD_COUNTER_MODE_GREEN_YELLOW:
          nstat = 2;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[1] = bcmFieldStatGreenPackets;
          stat_arr[0] = bcmFieldStatYellowPackets;
          break;
      case BCM_FIELD_COUNTER_MODE_RED_YELLOW:
          nstat = 2;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[1] = bcmFieldStatRedPackets;
          stat_arr[0] = bcmFieldStatYellowPackets;
          break;
      case BCM_FIELD_COUNTER_MODE_GREEN:
          nstat = 1;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[0] = bcmFieldStatGreenPackets;
          break;
      case BCM_FIELD_COUNTER_MODE_YELLOW:
          nstat = 1;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[0] = bcmFieldStatYellowPackets; 
          break;
      case BCM_FIELD_COUNTER_MODE_RED:
          nstat = 1;
          f_ent_st->flags &= ~_FP_ENTRY_STAT_EMPTY;
          stat_arr[0] = bcmFieldStatRedPackets; 
          break;
      default:
          return (BCM_E_PARAM);
    }
    rv = _robo_field_stat_array_init(unit, f_st, nstat, stat_arr);
    return (rv);

}

/*
 * Function: bcm_robo_field_action_add
 *
 * Purpose:
 *     Add action performed when entry rule is matched for a packet
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_MEMORY    - Allocation failure
 *     BCM_E_RESOURCE  - Conflicting actions exist in entry.
 *     BCM_E_RESOURCE  - Counter not previously created for entry.
 *     BCM_E_PARAM     - param0 or param1 out of range for action
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_action_add(int unit,
             bcm_field_entry_t entry,
             bcm_field_action_t action,
             uint32 param0,
             uint32 param1)
{
    _field_control_t    *fc;           /* Field control structure. */
    _field_entry_t      *f_ent;
    _field_action_t     *fa = NULL;
    _field_stage_id_t stage_id = 0;
    int                 retval = BCM_E_NONE;

#ifdef BROADCOM_DEBUG
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_action_add(unit=%d, entry=%d, "
                          "action=%s, p0=%d, p1=%d)\n"),
               unit, entry, _robo_field_action_name(action), param0, param1));
#endif

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    retval = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        return (retval);
    }

    /* Get current stage */
    stage_id = f_ent->fs->stage_id;

    /* Check for existing actions that conflict with the new action. */    
    fa = f_ent->actions;
    while(fa != NULL) {
        if (_field_actions_conflict(unit, stage_id, action, f_ent->actions->action)) {
            if (!_field_actions_conflict_allow(unit, action, param0, param1, fa)) {
                FP_UNLOCK(fc);
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      
                                       "FP Error: action=%d conflicts with existing action in entry=%d\n"),
                           action, entry ));
                return BCM_E_CONFIG;
            }
        }
        fa = fa->next;
    }

    /* Confirm that action is in aset. */
    if (SHR_BITGET(f_ent->group->aset.w, action) == 0) {
#ifdef BROADCOM_DEBUG
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: action=%s not supported\n"),
                   unit, 
                   _robo_field_action_name(action)));
#endif
        FP_UNLOCK(fc);
        return (BCM_E_UNAVAIL);
    }

    /* 
     * For meter configuration actions, confirm meter exists and set
     * its mode value to param0. Param1 is not used. Don't create an action
     * struct.
     */
    /* Check for this support by bcm5348 or not */
    if (action == bcmFieldActionMeterConfig) {
         retval = _robo_field_action_meter_config(unit, f_ent, param0, param1);        
        if (BCM_FAILURE(retval)) {
            FP_UNLOCK(fc);
            return retval;
        }
        FP_UNLOCK(fc);
        return BCM_E_NONE;
    }

    if (action == bcmFieldActionUpdateCounter) {
        retval = _robo_field_action_update_counter(unit, f_ent, param0, param1);
        if (BCM_FAILURE(retval)) {
            FP_UNLOCK(fc);
            return retval;
        }
    }

    /* NewClassID action can't be add in Slice 0 for BCM53115 */
    if ((action == bcmFieldActionNewClassId) &&
        (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
         SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
         SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        _field_group_t      *fg;
           
        /* Get group descriptor. */
        retval = _robo_field_group_get (unit, f_ent->gid, &fg);
        if (BCM_FAILURE(retval)){
            FP_UNLOCK(fc);
            return retval;
        }
        /* 
         * The value of fpf is the combination of L3 framing and
         * slice id value for BCM53115 */
        if ((fg->slices->sel_codes.fpf & 0x3) == 0) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: bcmFieldActionNewCalssId can't add "
                                  "to this entry\n")));
            return BCM_E_UNAVAIL;
        }
    }

    retval = DRV_FP_ACTION_ADD(unit, stage_id, f_ent->drv_entry, action, param0, param1);

    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        if (action == bcmFieldActionMirrorIngress) {
            retval = _bcm_robo_mirror_fp_dest_add(unit, 
                BCM_MIRROR_PORT_INGRESS, param1);
            if (BCM_FAILURE(retval)) {
                FP_UNLOCK(fc);
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: failure in bcmFieldActionMirrorIngress \n")));
                return retval;            
            }
        }
    }
#endif

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if ((action == bcmFieldActionMirrorIngress)||
            (action == bcmFieldActionRpMirrorIngress)) {
            if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                retval = _bcm_robo_mirror_fp_dest_add(unit, 
                    BCM_MIRROR_PORT_INGRESS, -1);
                if (BCM_FAILURE(retval)) {
                    FP_UNLOCK(fc);
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "FP Error: failure in bcmFieldActionMirrorIngress \n")));
                    return retval;            
                }
            }
        }
    }

    /*
     * It's okay to allocate the action and add it to entry's linked-list.
     */
    retval = _robo_field_action_alloc(action, param0, param1, &fa);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: failure in _robo_field_action_alloc()\n")));
        return retval;
    }

    assert(fa != NULL);
    fa->inst_flg = 0; /* mark new action as not yet installed */

    /* Add action to front of entry's linked-list. */
    fa->next = f_ent->actions;
    f_ent->actions  = fa;
    f_ent->flags |= _FP_ENTRY_DIRTY;

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_action_get
 *
 * Purpose:
 *     Get parameters associated with an entry action
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - (OUT) Action parameter
 *     param1 - (OUT) Action parameter
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - paramX is NULL
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_action_get(int unit,
             bcm_field_entry_t entry,
             bcm_field_action_t action,
             uint32 *param0,
             uint32 *param1)
{
    _field_entry_t      *f_ent;
    _field_action_t     *fa;
    _field_control_t     *fc;          /* Field control structure.   */
    int                  rv;           /* Operation return value.    */    

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    /* Find matching action in the entry */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (fa->action == action) {
            break;
        }
    }

    if (fa == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: action not in entry=%d\n"),
                   entry));
        return BCM_E_NOT_FOUND;
    }

    if (param0 == NULL || param1 == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: param0 == NULL || param1 == NULL\n")));
        return BCM_E_PARAM;
    }

    *param0 = fa->param0;
    *param1 = fa->param1;

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

int
_robo_field_action_remove(int unit, _field_stage_id_t stage_id,
        _field_entry_t *f_ent,  bcm_field_action_t action, 
            uint32 param0, uint32 param1)
{
    _field_action_t     *fa;
    _field_action_t     *fa_prev = NULL;
    int                         rv = BCM_E_NONE;

    /* Find the action in the entry */
    fa = f_ent->actions; /* start at head */
    while (fa != NULL) {
        if (fa->action == action) { /* found match, destroy action */
#ifdef BROADCOM_DEBUG
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP: %s remove entry %d action %s\n"),
                   FUNCTION_NAME(),
                   f_ent->eid, _robo_field_action_name(action)));
#endif
            rv = DRV_FP_ACTION_REMOVE(unit, stage_id, 
                f_ent->drv_entry, action, param0, param1);

            if (BCM_FAILURE(rv)) {
                return rv;
            }

            if (fa_prev != NULL) {
                fa_prev->next = fa->next;
            } else { /* matched head of list */
                f_ent->actions = fa->next;
            }
            /* okay to free entry */
            sal_free(fa);
            f_ent->flags |= _FP_ENTRY_DIRTY;
            return BCM_E_NONE;
        }
        fa_prev = fa;
        fa      = fa->next;
    }
    return BCM_E_NOT_FOUND;
}
/*
 * Function: bcm_robo_field_action_remove
 *
 * Purpose:
 *     Remove an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_action_remove(int unit,
                bcm_field_entry_t entry,
                bcm_field_action_t action)
{
    return bcm_robo_field_action_delete(unit, entry, action, 
                    (uint32)_FP_PARAM_INVALID, (uint32)_FP_PARAM_INVALID);
}

/*
 * Function: bcm_robo_field_action_delete
 *
 * Purpose:
 *     Delete an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_action_delete(int unit,
                bcm_field_entry_t entry,
                bcm_field_action_t action, 
                uint32 param0, uint32 param1)
{
    _field_control_t *fc;  /* Field control structure. */
    _field_entry_t      *f_ent;
    _field_stage_id_t stage_id = 0;
    int rv = BCM_E_NONE;
#ifdef BCM_TB_SUPPORT
    uint32 p0, p1;
#endif

#ifdef BROADCOM_DEBUG
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_action_remove(unit=%d, entry=%d, action=%s)\n"),
               unit, entry, _robo_field_action_name(action)));
#endif

    if (action < 0 || bcmFieldActionCount <= action) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: unknown action=%d\n"),
                   action));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        if (action == bcmFieldActionMirrorIngress) {
            rv = bcm_robo_field_action_get(unit, entry,bcmFieldActionMirrorIngress, 
                &p0, &p1);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }            
        }
    }
#endif
    
    /* Get current stage */
    stage_id = f_ent->fs->stage_id;
    rv = _robo_field_action_remove(unit, stage_id, f_ent, action, param0, param1);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        if (action == bcmFieldActionMirrorIngress) {
            rv = _bcm_robo_mirror_fp_dest_delete(unit, 
                BCM_MIRROR_PORT_INGRESS, p1);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return rv;            
            }
        }
    }
#endif
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if ((action == bcmFieldActionMirrorIngress)||
            (action == bcmFieldActionRpMirrorIngress)) {
            rv = _bcm_robo_mirror_fp_dest_delete(unit, 
                BCM_MIRROR_PORT_INGRESS, -1);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return rv;            
            }
        }
    }
    FP_UNLOCK(fc);
    return rv;

}

/*
 * Function: bcm_field_action_remove_all
 *
 * Purpose:
 *     Remove all actions from an entry rule.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_action_remove_all(int unit,
                                bcm_field_entry_t entry)
{
    _field_entry_t      *f_ent;
    _field_action_t     *fa;
    int rv;
    _field_control_t    *fc;      /* Field control structure.  */    
    
    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return rv;
    }

    /* start at the head of the actions list and burn them up */
    fa = f_ent->actions;
    while (fa != NULL) {
        rv = bcm_robo_field_action_remove(unit, entry, fa->action);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return rv;
        }
        fa =  f_ent->actions;
    }
    f_ent->flags |= _FP_ENTRY_DIRTY;

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_field_action_mac_add
 * Purpose:
 *      Add an action to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action parameter.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_action_mac_add(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_action_mac_get
 * Purpose:
 *      Retrieve the parameters for an action previously added to a
 *      field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action argument.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_action_mac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t *mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_group_traverse
 * Purpose:
 *      Traverse all the fp groups in the system, calling a specified
 *      callback for each one
 * Parameters:
 *      unit - (IN) Unit number.
 *      callback - (IN) A pointer to the callback function to call for each fp group
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_field_group_traverse(int unit, bcm_field_group_traverse_cb callback,
                              void *user_data)
{
    bcm_field_group_t *grp_arr;/* Field group pointers.    */ 
    _field_control_t *fc;      /* Field control structure. */
    _field_group_t *fg;        /* Field group structure.   */
    int group_count;           /* Number of fp groups.     */
    int mem_sz;                /* Allocated memory size.   */
    int idx;                   /* Group array iterator.    */
    int rv = BCM_E_NONE;       /* Operation return status. */

    /* Input parameters check. */
    if (NULL == callback) {
        return (BCM_E_PARAM);
    }

    /* Field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc)); 
    FP_LOCK(fc);

    /* Count fp groups. */
    fg = fc->groups;
    group_count = 0;
    while (fg != NULL) {
        group_count++;
        fg = fg->next;
    }

    if (0 == group_count) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* 
     * API can not use field contol groups linked list, 
     * since group might be destroyed in callback.  
     */
    mem_sz = group_count * sizeof(bcm_field_group_t);
    grp_arr = NULL;
    grp_arr = sal_alloc(mem_sz, "FP groups array");
    if (NULL == grp_arr) {
        FP_UNLOCK(fc);
        return (BCM_E_MEMORY);
    }

    /* Programm fp group ids into allocated array. */
    fg = fc->groups;
    idx = 0;
    while (fg != NULL) {
        grp_arr[idx] = fg->gid;
        idx++;
        fg = fg->next;
    }

    /* Call user callback. */
    for (idx = 0; idx < group_count; idx++) {
        rv = (*callback)(unit, grp_arr[idx], user_data);
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    FP_UNLOCK(fc);
    sal_free(grp_arr);
    return (rv);
}


int
bcm_robo_field_action_ports_add(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t pbmp)
{
    _field_entry_t *f_ent;
    _field_action_t *fa;
    _field_stage_id_t  stage_id;
    int rv;
    _field_control_t     *fc;          /* Field control structure.   */    
    
    if (action != bcmFieldActionRedirectPbmp) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "Incorrect action parameter\n")));
        return (BCM_E_UNAVAIL);
    }

    if (!(SOC_IS_VULCAN(unit))||
            SOC_IS_TBX(unit) ||
            SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_BLACKBIRD2(unit) ||
            SOC_IS_POLAR(unit) ||
            SOC_IS_NORTHSTAR(unit)||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
            SOC_IS_STARFIGHTER3(unit)) {
        return BCM_E_UNAVAIL;
    }


    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 
    /* Action is always in PRIMARY_SLICE  */
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Get current stage */
    stage_id = f_ent->fs->stage_id;
    
    /* Check for existing actions that conflict with the new action. */ 
    fa = f_ent->actions;
    while (fa != NULL) {
        if (_field_actions_conflict(unit, stage_id, action, fa->action)) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: action=%d conflicts with existing"
                                   "action in entry=%d\n"), action, entry));
            return BCM_E_CONFIG;
        }
        fa = fa->next;
    }

    /* Create a new action entry */
    fa = sal_alloc(sizeof (_field_action_t), "field_action");
    if (fa == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: allocation failure for field_action\n")));
        return (BCM_E_MEMORY);
    }
    sal_memset(fa, 0, sizeof (_field_action_t));

    fa->action = action;
    fa->param0 = SOC_PBMP_WORD_GET(pbmp, 0);
    fa->param1 = 0;

    if (0 == (f_ent->policer[0].flags & _FP_POLICER_VALID)) {
        rv = DRV_CFP_ACTION_SET
            (unit, DRV_CFP_ACT_OB_REDIRECT, 
            f_ent->drv_entry, fa->param0, 0);
        if (BCM_FAILURE(rv)) {
            sal_free(fa);
            FP_UNLOCK(fc);
            return (rv);
        }
    }
    rv = DRV_CFP_ACTION_SET
        (unit, DRV_CFP_ACT_IB_REDIRECT, 
        f_ent->drv_entry, fa->param0, 0);
    if (BCM_FAILURE(rv)) {
        sal_free(fa);
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Add action to front of entry's linked-list. */
    fa->next = f_ent->actions;
    f_ent->actions  = fa;
    f_ent->flags |= _FP_ENTRY_DIRTY;

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

int
bcm_robo_field_action_ports_get(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t *pbmp)
{
    _field_entry_t *f_ent;
    _field_action_t *fa;
    _field_control_t     *fc;          /* Field control structure.   */
    int     rv;
    
    if (action != bcmFieldActionRedirectPbmp) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "Incorrect action parameter\n")));
        return (BCM_E_UNAVAIL);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }


    /* Find matching action in the entry */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (fa->action == action) {
            break;
        }
    }

    if (fa == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: action not in entry=%d\n"),
                   entry));
        return (BCM_E_NOT_FOUND);
    }

    if (pbmp == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: pbmp == NULL\n")));
        return (BCM_E_PARAM);
    }
    
    BCM_PBMP_CLEAR(*pbmp);

    SOC_PBMP_WORD_SET(*pbmp, 0, fa->param0);

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/* Section: Field Statistics */

/*
 * Function: 
 *    bcm_robo_field_stat_create
 *
 * Description:
 *       Create statistics collection entity.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      group    - (IN) Field group id. 
 *      nstat    - (IN) Number of elements in stat array.
 *      stat_arr - (IN) Collected statistics descriptors array.
 *      stat_id  - (OUT) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_create(int unit, bcm_field_group_t group, int nstat, 
                          bcm_field_stat_t *stat_arr, int *stat_id) 
{
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s\n"),
               FUNCTION_NAME()));
    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_create(unit, group, nstat, stat_arr, 0, stat_id);

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *    bcm_robo_field_stat_create_id
 *
 * Description:
 *       Create statistics collection entity with specified ID.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      group    - (IN) Field group id. 
 *      nstat    - (IN) Number of elements in stat array.
 *      stat_arr - (IN) Collected statistics descriptors array.
 *      stat_id  - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_create_id(int unit, bcm_field_group_t group, int nstat, 
                          bcm_field_stat_t *stat_arr, int stat_id) 
{
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */
    int                  stat_id_local = stat_id; /* Stats ID value */

    /* Check if Stat ID is in software supported range */
    if ((stat_id < _FP_ID_BASE) || (stat_id > _FP_ID_MAX)) {
        return (BCM_E_PARAM);
    }
    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_create(unit, group, nstat, stat_arr,
            _FP_STAT_CREATE_ID, &stat_id_local);

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *    bcm_robo_field_stat_destroy
 *
 * Description:
 *       Destroy statistics collection entity.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      stat_id  - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_destroy(int unit, int stat_id)
{
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_destroy(unit, stat_id);

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *     bcm_robo_field_stat_size
 *
 * Description:
 *      Get number of different statistics associated with statistics
 *      collection entity.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      stat_size - (OUT) Number of collercted statistics
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_size(int unit, int stat_id, int *stat_size)
{
    _field_stat_t       *f_st;    /* Field statistics entity.  */
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */

    /* Input parameters check */
    if (NULL == stat_size) {
        return (BCM_E_PARAM);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    *stat_size = f_st->nstat; 

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *     bcm_robo_field_stat_config_get
 *
 * Description:
 *      Get enabled statistics for specific collection entity.
 * Parameters:
 *      unit     - (IN) BCM device number.
 *      stat_id  - (IN) Statistics entity id.
 *      nstat    - (IN) Number of elements in stat array.
 *      stat_arr - (OUT) Collected statistics descriptors array.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_config_get(int unit, int stat_id, int nstat, 
                              bcm_field_stat_t *stat_arr)
{
    _field_stat_t       *f_st;     /* Field statistics entity.   */
    _field_control_t    *fc;       /* Field control structure.   */
    int                 stat_count;/* Number of elements to fill.*/
    int                 rv;        /* Operation return status.   */

    /* Input parameters check. */
    if ((NULL == stat_arr) || (nstat <= 0) || (nstat > bcmFieldStatCount)) {
        return (BCM_E_PARAM);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }


    /* Initialize application provided array. */
    sal_memset(stat_arr, 0, (nstat * sizeof(bcm_field_stat_t)));

    /* Number of items to copy. */
    stat_count = (nstat > f_st->nstat) ? f_st->nstat : nstat;

    sal_memcpy(stat_arr, f_st->stat_arr, 
               (stat_count * sizeof(bcm_field_stat_t)));

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_stat_set
 *
 * Description:
 *      Set 64 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (OUT) Collected counters value.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_set(int unit, int stat_id, bcm_field_stat_t stat, 
                       uint64 value)
{
    _field_stat_t       *f_st;     /* Field statistics entity.   */
    _field_control_t    *fc;       /* Field control structure.   */
    int                 rv;        /* Operation return status.   */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get statistics entity descriptor. */
    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Write specific counter only if statistics entity was installed. */
    if (_FP_INVALID_INDEX == f_st->hw_index)  {
        if (COMPILER_64_IS_ZERO(value)) {
            rv = BCM_E_NONE;
        } else {
            rv = BCM_E_UNAVAIL;
        }
    } else {
        rv = _robo_field_stat_value_set(unit, f_st, stat, value, FALSE);
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_stat_set32
 *
 * Description:
 *      Set lower 32 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (OUT) Collected counters value.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_set32(int unit, int stat_id, 
                         bcm_field_stat_t stat, uint32 value)
{
    uint64 val64;

    COMPILER_64_SET(val64, 0, value);
    return bcm_robo_field_stat_set (unit, stat_id, stat, val64);
}

/*
 * Function: 
 *      bcm_robo_field_stat_all_set
 *
 * Description:
 *      Set 64 bit counter values for all statistic types.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      nstat     - (IN) Number of elements in stat array.
 *      stat_arr  - (IN) Collected statistics descriptors array.
 *      value_arr - (OUT) Collected counters values.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_all_set(int unit, int stat_id, uint64 value)
{
    _field_control_t    *fc;       /* Field control structure.    */
    _field_stat_t       *f_st;     /* Field statistics entity.    */
    int                 idx;       /* Statistics iteration index. */
    int                 rv;        /* Operation return status.    */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Read individual statistics. */
    for (idx = 0; idx < f_st->nstat; idx++) {
        rv = bcm_robo_field_stat_set(unit, stat_id, f_st->stat_arr[idx], value);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_stat_all_set32
 *
 * Description:
 *      Set 64 bit counter values for all statistic types.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      nstat     - (IN) Number of elements in stat array.
 *      stat_arr  - (IN) Collected statistics descriptors array.
 *      value_arr - (OUT) Collected counters values.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_all_set32(int unit, int stat_id, uint32 value)
{
    uint64              val64;     /* 64 bit value. */

    COMPILER_64_SET(val64, 0, value);
    return bcm_robo_field_stat_all_set(unit, stat_id, val64);
}

/*
 * Function: 
 *      bcm_robo_field_stat_get
 *
 * Description:
 *      Get 64 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (OUT) Collected counters value.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_get(int unit, int stat_id, bcm_field_stat_t stat, 
                       uint64 *value)
{
    _field_stat_t       *f_st;     /* Field statistics entity.   */
    _field_control_t    *fc;       /* Field control structure.   */
    int                 rv;        /* Operation return status.   */

    /* Input parameters check. */
    if (NULL == value) {
        return (BCM_E_PARAM);
    }

    COMPILER_64_ZERO(*value);

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get statistics entity descriptor. */
    rv = _robo_field_stat_get(unit, stat_id, &f_st);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "%s stat_id %d stat %d\n"),
                 FUNCTION_NAME(), stat_id, stat));
    /* Read specific counter only if statistics entity was installed. */
    if (_FP_INVALID_INDEX != f_st->hw_index) {
        rv = _robo_field_stat_value_get(unit, f_st, stat, value);
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_stat_get32
 *
 * Description:
 *      Get lower 32 bit counter value for specific statistic type.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      stat      - (IN) Collected statistics descriptor.
 *      value     - (OUT) Collected counters value.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_get32(int unit, int stat_id, 
                         bcm_field_stat_t stat, uint32 *value)
{
    uint64 val64;            /* 64 bit counter value.    */
    int rv;                  /* Operation return status. */

    /* Input parameters check. */
    if (NULL == value) {
        return (BCM_E_PARAM);
    }

    /* Read 64 bit counter value. */
    rv = bcm_robo_field_stat_get (unit, stat_id, stat, &val64);
    if (BCM_SUCCESS(rv)) {
        *value = COMPILER_64_LO(val64);
    } 
    return rv;
}

/*
 * Function: 
 *      bcm_robo_field_stat_multi_get
 *
 * Description:
 *      Get 64 bit counter values for multiple statistic types.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      nstat     - (IN) Number of elements in stat array.
 *      stat_arr  - (IN) Collected statistics descriptors array.
 *      value_arr - (OUT) Collected counters values.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_multi_get(int unit, int stat_id, int nstat, 
                             bcm_field_stat_t *stat_arr, uint64 *value_arr)
{
    _field_control_t    *fc;       /* Field control structure.    */
    int                 rv;        /* Operation return status.    */
    int                 idx;       /* Statistics iteration index. */

    /* Input parameters check. */
    if ((stat_arr == NULL) || (value_arr == NULL)) {
        return (BCM_E_PARAM);
    }
    
    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Read individual statistics. */
    for (idx = 0; idx < nstat; idx++) {
        rv = bcm_robo_field_stat_get(unit, stat_id, stat_arr[idx], 
                                    value_arr + idx);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
    }

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/*
 * Function: 
 *      bcm_robo_field_stat_multi_get32
 *
 * Description:
 *      Get lower 32 bit counter values for multiple statistic types.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      stat_id   - (IN) Statistics entity id.
 *      nstat     - (IN) Number of elements in stat array.
 *      stat_arr  - (IN) Collected statistics descriptors array.
 *      value_arr - (OUT) Collected counters values.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_stat_multi_get32(int unit, int stat_id, int nstat, 
                               bcm_field_stat_t *stat_arr, 
                               uint32 *value_arr)
{
    _field_control_t    *fc;       /* Field control structure.    */
    uint64              value;     /* 64 bit counter value.       */
    int                 rv;        /* Operation return status.    */
    int                 idx;       /* Statistics iteration index. */

    /* Input parameters check. */
    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return (BCM_E_PARAM);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Read individual statistics. */
    for (idx = 0; idx < nstat; idx++) {
        rv = bcm_robo_field_stat_get(unit, stat_id, stat_arr[idx], &value);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
        value_arr[idx] = COMPILER_64_LO(value);
    }

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/*
 * Function: 
 *      bcm_robo_field_entry_stat_attach
 *
 * Description:
 *       Attach statistics entity to Field Processor entry.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      entry     - (IN) Field entry id. 
 *      stat_id   - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_field_entry_stat_attach(int unit, bcm_field_entry_t entry, 
                                    int stat_id)
{
    _field_entry_t      *f_ent;   /* Internal entry descriptor.       */
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get entry description structure. */
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Attach statistics entity to the entry. */
    rv = _robo_field_entry_stat_attach(unit, f_ent, stat_id);

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_entry_stat_detach
 *
 * Description:
 *       Detach statistics entity to Field Processor entry.
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      entry     - (IN) Field entry id. 
 *      stat_id   - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_field_entry_stat_detach(int unit, bcm_field_entry_t entry,
                                    int stat_id)
{
    _field_entry_t      *f_ent;   /* Field entry structure.    */
    _field_control_t    *fc;      /* Field control structure.  */
    int                  rv;      /* Operation return status.  */

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN (_robo_field_control_get(unit, &fc));
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "%s eid %d statid %d\n"),
                 FUNCTION_NAME(),entry, stat_id));
    FP_LOCK(fc);
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);

    if (BCM_FAILURE(rv)) {
       FP_UNLOCK(fc);
       return (rv);
    }

    rv = _robo_field_entry_stat_detach(unit, f_ent, stat_id);

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: 
 *      bcm_robo_field_entry_stat_get
 *
 * Description:
 *      Get statistics entity attached to Field Processor entry.  
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      entry     - (IN) Field entry id. 
 *      stat_id   - (IN) Statistics entity id.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_field_entry_stat_get(int unit, bcm_field_entry_t entry, int *stat_id)
{
    _field_control_t    *fc;       /* Field control structure.   */
    _field_entry_t      *f_ent;    /* Field entry structure.     */
    int                 rv;        /* Operation return status.   */

    /* Input parameters check. */
    if (NULL == stat_id) {
        return (BCM_E_PARAM);
    }

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get field processor entry structure. */
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
        *stat_id = f_ent->statistic.sid; 
    } else {
        rv = BCM_E_NOT_FOUND;
    }

    FP_UNLOCK(fc);
    return (rv);
}



/*
 * Function: bcm_robo_field_detach
 *
 * Purpose:
 *     Free resources associated with field module
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_INIT - BCM Unit not initialized.
 *     BCM_E_XXX  - Error code from bcm_field_group_destroy() or
 *                  bcm_field_entry_destroy_all().
 *     BCM_E_NONE - Success
 */
int 
bcm_robo_field_detach(int unit)
{
    _field_control_t    *fc;
    _field_group_t      *fg;
    int rv;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_detach(%d)\n"),
               unit));

    /* Make sure the Unit can support this module. */
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_field)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No Field Processor available Unit=%d\n"),
                   unit));
        return BCM_E_UNAVAIL;
    }

    fc = _field_control[unit];

    if (NULL == fc) {
        return (BCM_E_NONE);
    }

    if (soc_feature(unit, soc_feature_field_tcam_parity_check)) {
        rv = _robo_field_thread_stop(unit);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }
    /* Unregister counter collection callback. */
    if (fc->fc_lock != NULL) {
        FP_LOCK(fc);
    }
    /* Destroy all entries in unit. */
    rv = bcm_robo_field_entry_destroy_all(unit);
    if (BCM_FAILURE(rv)) {
        if (fc->fc_lock != NULL) {
            FP_UNLOCK(fc);
        }
        return (rv);
    }
    /* Destroy all groups in unit. */
    fg = fc->groups;
    while (fg != NULL) {
        rv = bcm_robo_field_group_destroy(unit, fg->gid);
        if (BCM_FAILURE(rv)) {
            if (fc->fc_lock != NULL) {
                FP_UNLOCK(fc);
            }
            return (rv);
        }
        fg = fc->groups;
    }
    rv = DRV_FP_DEINIT(unit, -1);
    if (BCM_FAILURE(rv)) {
        if (fc->fc_lock != NULL) {
            FP_UNLOCK(fc);  
        }
        return (rv);
    }
    /* Destroy a Unit's stages*/
    rv = _robo_field_stages_destroy(unit, fc);
    if (BCM_FAILURE(rv)) {
        if (fc->fc_lock != NULL) {
            FP_UNLOCK(fc);  
        }
        return (rv);
    }
    if (fc->fc_lock != NULL) {
        FP_UNLOCK(fc);  
    }
    _robo_field_control_free(unit, fc);
    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_entry_copy
 *
 * Purpose:
 *     Create a copy of an existing entry. The new entry will be a member of
 *     the same group as the source entry.
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Entry to copy
 *     dst_entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM Unit not initialized
 *     BCM_E_NOT_FOUND   Source entry not found
 *     BCM_E_INTERNAL    No group exists for source entry ID.
 *     BCM_E_PARAM       dst_entry pointing to NULL
 *     BCM_E_RESOURCE    No destination entry available
 *     BCM_E_XXX         Error from bcm_field_entry_copy_id()
 */
int 
bcm_robo_field_entry_copy(int unit,
             bcm_field_entry_t src_entry,
             bcm_field_entry_t *dst_entry)
{
    _field_entry_t      *f_ent_src;
    _field_control_t     *fc;          /* Field control structure.   */
    int rv;

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 

    rv = _robo_field_entry_get(unit, src_entry, &f_ent_src, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (dst_entry == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: dst_entry == NULL\n")));
        FP_UNLOCK(fc);
        return BCM_E_PARAM;
    }

    /* Generate a destination Entry ID.  */
    *dst_entry = src_entry + 1;

    while (BCM_SUCCESS
        (_robo_field_entry_get(unit, *dst_entry, &f_ent_src, _FP_SLICE_PRIMARY))) {
        *dst_entry += 1;
        assert(*dst_entry < _FP_ENTRY_ID_BASE + 0x1000000);
    }

    rv = bcm_robo_field_entry_copy_id(unit, src_entry, *dst_entry);

    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function: bcm_robo_field_entry_copy_id
 *
 * Purpose:
 *     Create a copy of an existing entry with a requested ID
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Source entry to copy
 *     dst_entry - Destination entry for copy
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Source Entry ID not found
 *     BCM_E_XXX       - Error code from bcm_field_entry_create_id()
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_entry_copy_id(int unit,
             bcm_field_entry_t src_entry,
             bcm_field_entry_t dst_entry)
{
    _field_entry_t      *f_ent_src, *f_ent_dst;
    _field_action_t     *fa_src;
    int rv, i;
    _field_control_t     *fc;          /* Field control structure.   */    
    bcm_policer_config_t pol_cfg;        /* Policer configuration.     */
    bcm_policer_t        policer_id = _FP_INVALID_INDEX;/* Policer id.                */
    _field_policer_t    *f_pl;
    _field_stat_t       *f_st;
    int         stat_id = _FP_INVALID_INDEX;


    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 
    rv = _robo_field_entry_get(unit, src_entry, &f_ent_src, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: %s src:%d dst:%d\n"),
               FUNCTION_NAME(),src_entry, dst_entry));

    rv = bcm_robo_field_entry_create_id(unit, f_ent_src->fs->group->gid, dst_entry);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    rv = _robo_field_entry_get(unit, dst_entry, &f_ent_dst, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_fp_entry_copy : src data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[0],
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[1], 
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[2], 
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[3], 
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[4], 
                 ((drv_cfp_entry_t *)(f_ent_src->drv_entry))->tcam_data[5]));


    /* sal_memcpy(&f_ent_dst->tcam, &f_ent_src->tcam, sizeof(_field_tcam_t)); */

    if (f_ent_src->flags & _FP_ENTRY_WITH_CHAIN) {
        f_ent_dst->flags |= _FP_ENTRY_WITH_CHAIN;
    }

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, f_ent_src->fs->stage_id, 
        DRV_FIELD_ENTRY_MEM_COPY, f_ent_src->drv_entry, f_ent_dst->drv_entry,NULL);

    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return rv;
    }

    if (f_ent_src->fs->stage_id == _BCM_FIELD_STAGE_INGRESS) {
        /* Copy counter, if it exists. */
        if (f_ent_src->statistic.flags & _FP_ENTRY_STAT_VALID) {
            rv = _robo_field_stat_get(unit,  f_ent_src->statistic.sid, 
                &f_st);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }
            rv = _robo_field_stat_create(unit, f_ent_src->group->gid, 
                    f_st->nstat, f_st->stat_arr, _FP_STAT_INTERNAL, &stat_id);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }

            rv = bcm_robo_field_entry_stat_attach(unit, dst_entry,
                                                 stat_id);
            if (BCM_FAILURE(rv)) {
                (void)_robo_field_stat_destroy(unit, stat_id);
                FP_UNLOCK(fc);
                return (rv);
            }
            /* Preseve original entry flags. */
            f_ent_dst->statistic.flags = (f_ent_src->statistic.flags &  \
                (_FP_ENTRY_STAT_VALID | _FP_ENTRY_STAT_EMPTY));
        }
        /*  Copy meter, if it exists. */
        for (i = 0; i < _FP_POLICER_LEVEL_COUNT; i++)  {
            if (f_ent_src->policer[i].flags & _FP_POLICER_VALID) {
                rv = _robo_field_policer_get(unit, f_ent_src->policer[i].pid,
                            &f_pl);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }
                pol_cfg = f_pl->cfg; 
                pol_cfg.flags &= ~(BCM_POLICER_WITH_ID |BCM_POLICER_REPLACE);
                rv = _bcm_robo_field_policer_create(unit, &pol_cfg, 
                    _FP_POLICER_INTERNAL,
                               &policer_id);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }
                rv = bcm_robo_field_entry_policer_attach(unit, dst_entry, i,
                                                        policer_id);
                if (BCM_FAILURE(rv)) {
                    (void)_robo_field_policer_destroy(unit, policer_id);
                    FP_UNLOCK(fc);
                    return (rv);
                }
            }
        }
    }
    /* Copy source entry's action linked list.  */
    for (fa_src = f_ent_src->actions; fa_src != NULL; fa_src = fa_src->next) {
        rv = bcm_robo_field_action_add(unit, dst_entry, fa_src->action, 
                                 fa_src->param0, fa_src->param1);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return rv;
        }
    }

    f_ent_dst->flags |= _FP_ENTRY_DIRTY;

    /* Set the destination entry's priority to the same as the source's. */

    rv = bcm_robo_field_entry_prio_set(unit, dst_entry, f_ent_src->prio);
    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function: bcm_robo_field_entry_create
 *
 * Purpose:
 *     Create a blank entry based on a group. Automatically generate an entry
 *     ID.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group ID
 *     entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM unit not initialized
 *     BCM_E_NOT_FOUND   group not found in unit
 *     BCM_E_PARAM       *entry was NULL
 *     BCM_E_RESOURCE    No unused entries available
 *     BCM_E_XXX         Error from bcm_field_entry_create_id
 *
 * See Also:
 * bcm_field_entry_create_id
 */
int 
bcm_robo_field_entry_create(int unit,
               bcm_field_group_t group,
               bcm_field_entry_t *entry)
{
    _field_stage_t      *stage_fc;    /* Stage field control info.    */
    _field_group_t      *fg;
    int rv;
    _field_control_t     *fc;          /* Field control structure.   */
    _field_entry_t      *f_ent;        /* Field entry pointer.        */     

    FIELD_IS_INIT(unit);
    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 

    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    /* Get group stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc); 
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Stage (%d) control get failure.\n"), 
                   unit, fg->stage_id));
        return (rv);
    }

    if (stage_fc->field_shared_entries_free == 0) {
        FP_UNLOCK(fc); 
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No entries free in field.\n")));
        return BCM_E_RESOURCE;
    }


    if (entry == NULL) {
        FP_UNLOCK(fc); 
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: entry == NULL.\n")));
        return BCM_E_PARAM;
    }

    /* Generate an entry ID.  */
    last_allocated_eid[unit]++;
    while (BCM_SUCCESS(_robo_field_entry_get(unit, \
                last_allocated_eid[unit], &f_ent,  _FP_SLICE_PRIMARY))) {
        last_allocated_eid[unit]++;
        if (_FP_ENTRY_ID_MAX == last_allocated_eid[unit]) {
            last_allocated_eid[unit] = _FP_ENTRY_ID_BASE;
        }
    }
    *entry = last_allocated_eid[unit];

    rv = bcm_robo_field_entry_create_id(unit, group, *entry);
    FP_UNLOCK(fc); 
    return rv;
}

/*
 * Function: bcm_robo_field_entry_create_id
 *
 * Purpose:
 *     Create a blank entry group based on a group;
 *     allows selection of a specific slot in a slice
 *
 * Parameters:
 *     unit - BCM device number
 *     group - Field group ID
 *     entry - Requested entry ID; must be in the range prio_min through
 *             prio_max as returned by bcm_field_group_status_get().
 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_EXISTS    - Entry ID already in use
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_MEMORY    - allocation failure
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_entry_create_id(int unit,
               bcm_field_group_t group,
               bcm_field_entry_t entry)
{
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;    /* Stage field control info.    */    
    _field_entry_t      *f_ent;
    int     idx, i, null_idx;
    int rv, retval, entry_free, chain_entry_free;
    _field_control_t     *fc;          /* Field control structure.   */    

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_robo_field_entry_create_id(group=%d, entry=%d)\n"),
               group, entry));

    /* Get field control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc); 

    /* Confirm that 'entry' is not already used on unit */
    if(BCM_SUCCESS(_robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY))){
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: entry=%d already exists.\n"),
                   entry));
        return BCM_E_EXISTS;
    }

    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    /* Get group stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Stage (%d) control get failure.\n"), 
                   unit, fg->stage_id));
        return (rv);
    }


    if (stage_fc->field_shared_entries_free == 0) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No entries free in field.\n")));
        return BCM_E_RESOURCE;
    }

    /* Create entry in primary slice. */
    /* 
     * COVERITY
     *  f_ent is created when creating the entry, 
     *  will be free when destroying it (bcm_field_entry_destroy)
     */
    /* coverity[-alloc] */
    f_ent = _robo_field_entry_phys_create(unit, entry,
                                     BCM_FIELD_ENTRY_PRIO_LOWEST,
                                     &fg->slices[0]);
    if (f_ent == NULL) {
        FP_UNLOCK(fc);
        return BCM_E_MEMORY;
    }

    f_ent->gid = group;

    entry_free = 0;
    chain_entry_free = 0;

    /* Find unused entry in slice. */
    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        if (stage_fc->field_shared_entries_free < 2) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: No enough entries free in field.\n")));
            rv = BCM_E_RESOURCE;
            goto error_return;            
        }        
        idx = stage_fc->tcam_bottom - 1;
        if (stage_fc->field_shared_entries[idx] !=NULL) {
            null_idx = _FP_INVALID_INDEX;
            for (i = idx; i >= 0; i --) {
                if (stage_fc->field_shared_entries[i] == NULL) {
                    null_idx = i;
                    break;
                }        
            }
            if (null_idx == _FP_INVALID_INDEX) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: No enough entries free in field.\n")));
                rv = BCM_E_RESOURCE;
                goto error_return;
            }
            if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit, fg,
                    null_idx+1, null_idx, 
                    (idx - null_idx), _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    goto error_return;
                }   
            } else {
                rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                idx, null_idx, _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    goto error_return;
                }   
            }
        }
        f_ent->chain_idx = idx;
        stage_fc->field_shared_entries[idx] = f_ent;
        stage_fc->tcam_bottom --;
        chain_entry_free = 1;
    }
    
    if (f_ent->flags & _FP_ENTRY_CHAIN_SLICE) {
        if (stage_fc->field_shared_entries_free < 1) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: No enough entries free in field.\n")));
            rv = BCM_E_RESOURCE;
            goto error_return;            
        }        
        idx = stage_fc->tcam_bottom - 1;
        if (stage_fc->field_shared_entries[idx] !=NULL) {
            null_idx = _FP_INVALID_INDEX;
            for (i = idx; i >= 0; i --) {
                if (stage_fc->field_shared_entries[i] == NULL) {
                    null_idx = i;
                    break;
                }        
            }
            if (null_idx == _FP_INVALID_INDEX) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: No enough entries free in field.\n")));
                rv = BCM_E_RESOURCE;
                goto error_return;
            }
            if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit, fg,
                    null_idx+1, null_idx, 
                    (idx - null_idx), _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    goto error_return;
                }   
            } else {
                rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                idx, null_idx, _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    goto error_return;
                }   
            }
        }
        f_ent->slice_idx = idx;
        stage_fc->field_shared_entries[idx] = f_ent;
        stage_fc->tcam_bottom --;    
    } else 
    {
        f_ent->slice_idx = stage_fc->tcam_bottom;

        for (idx = 0; idx < stage_fc->tcam_bottom; idx++) {
            if (NULL == stage_fc->field_shared_entries[idx]) {
                stage_fc->field_shared_entries[idx] = f_ent;
                f_ent->slice_idx = idx;
                break;
            }
        }

        if (f_ent->slice_idx == stage_fc->tcam_bottom){
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: Can not get the slice_idx.\n")));
            rv = BCM_E_RESOURCE;
            goto error_return;        
        }
    }
    entry_free = 1;

    /* Move entry to the default position. */

    rv = bcm_robo_field_entry_prio_set(unit, entry, BCM_FIELD_ENTRY_PRIO_DEFAULT);
    if (BCM_FAILURE(rv)){
        goto error_return;
    }        
 
    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyIp4)) {
        rv = bcm_robo_field_qualify_IpType(unit, entry, bcmFieldIpTypeIpv4Any);
        if (BCM_FAILURE(rv)){
            goto error_return;
        }
    }

    if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyIp6)) {
        rv = bcm_robo_field_qualify_IpType(unit, entry, bcmFieldIpTypeIpv6);
        if (BCM_FAILURE(rv)){
            goto error_return;
        }
   }

    stage_fc->field_shared_entries_free--;

    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        stage_fc->field_shared_entries_free--;
    }

    fg->group_status.entry_count ++;

    FP_UNLOCK(fc);

    /* 
      * COVERITY
      *  f_ent is created when creating the entry, 
      *  will be free when destroying it (bcm_field_entry_destroy)
      */
    /* coverity[leaked_storage]  */
    return BCM_E_NONE;

error_return:    
    if (entry_free == 1) {
        stage_fc->field_shared_entries[f_ent->slice_idx] = NULL;
        if (f_ent->flags & _FP_ENTRY_CHAIN_SLICE) {
            stage_fc->tcam_bottom ++;
        }
    }
    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        if (chain_entry_free == 1) {
            stage_fc->field_shared_entries[f_ent->chain_idx] = NULL;
            stage_fc->tcam_bottom ++;
        }
        retval = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_fc->stage_id,
            f_ent->drv_entry, DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY, 0, NULL);
        if (BCM_FAILURE(retval) && (retval != BCM_E_UNAVAIL)){
            if (f_ent->drv_entry != NULL) {
                sal_free(f_ent->drv_entry);
            }        
            sal_free(f_ent);
            FP_UNLOCK(fc);
            return retval;
        }        
    }
    if (f_ent->drv_entry != NULL) {
        sal_free(f_ent->drv_entry);
    }
    sal_free(f_ent);
    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function: bcm_robo_field_entry_multi_get
 *
 * Purpose:
 *     Gets an array of a group's entry IDs
 *
 * Parameters:
 *     unit -  (IN) BCM device number.
 *     group - (IN) Field group ID.
 *     entry_size - (IN) Maximum number of entries to return.  Set to 0
 *                       to get the number of entries available
 *     entry_array - (OUT) Pointer to a buffer to fill with the array of
 *                         entry IDs.  Ignored if entry_size is 0
 *     entry_count - (OUT) Returns the number of entries returned in the
 *                         array, or if entry_size was 0, the number of
 *                         entries available

 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_entry_multi_get(int unit, bcm_field_group_t group,
    int entry_size, bcm_field_entry_t *entry_array, int *entry_count)
{
    _field_control_t *fc;
    int entry_index;
    _field_group_t *fg_p;
    int rv;
    int idx;
    _field_stage_t      *stage_fc; 

    BCM_IF_ERROR_RETURN (_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_group_get(unit, group, &fg_p);

    if (BCM_SUCCESS(rv))
    {
        if (entry_count == NULL) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: entry_count == NULL.\n"),
                       unit));
            return (BCM_E_PARAM);
        }

        if (entry_size == 0)
        {
            *entry_count = fg_p->group_status.entry_count;
        }
        else
        {
            if (entry_array == NULL) {
                FP_UNLOCK(fc);
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: entry_array == NULL.\n"),
                           unit));
                return (BCM_E_PARAM);
            }
            if (entry_size > fg_p->group_status.entry_count)
            {
                entry_size = fg_p->group_status.entry_count;
            }

            /* Get group stage control structure. */
            rv = _robo_field_stage_control_get(unit, fg_p->stage_id, &stage_fc);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: Stage (%d) control get failure.\n"), 
                           unit, fg_p->stage_id));
                return (rv);
            }

            entry_index = 0;
            for (idx = 0; idx < stage_fc->tcam_sz; idx++) {
                if (idx >= stage_fc->tcam_bottom){
                    if (NULL != stage_fc->field_shared_entries[idx]) {                    
                        if (stage_fc->field_shared_entries[idx]->flags & _FP_ENTRY_WITH_CHAIN){
                            continue;
                        }
                    }
                }
                if (NULL != stage_fc->field_shared_entries[idx]) {
                    if (stage_fc->field_shared_entries[idx]->gid == group){
                        *entry_array = stage_fc->field_shared_entries[idx]->eid;
                        entry_index++;
                        if (entry_index == entry_size) {
                            break;
                        }
                        entry_array++;
                    }
                }
            }
            *entry_count = entry_size;
        }
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: bcm_robo_field_group_create_mode
 *     
 * Purpose:
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) New field group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - No select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_group_create_mode(int unit, 
                    bcm_field_qset_t qset,
                    int pri,
                    bcm_field_group_mode_t mode,
                    bcm_field_group_t *group)
{
    int rv;
    FIELD_IS_INIT(unit);

    rv = _robo_field_group_id_generate(unit, qset, group, pri);
    if ( rv == BCM_E_EXISTS) {
        return BCM_E_NONE;
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: new group won't create.\n")));
        return rv;
    }

    return bcm_robo_field_group_create_mode_id(unit, qset, pri, mode, *group);
}

/*
 * Function: bcm_robo_field_group_create_mode_id
 *     
 * Purpose:
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - Requested field group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_group_create_mode_id(int unit,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_mode_t mode,
                                   bcm_field_group_t group)
{
    _field_control_t    *fc = NULL;
    _field_group_t      *fg = NULL;
    int                 retval;
    _field_stage_id_t   stage;
    _field_stage_t          *stage_fc;    
#ifdef DEBUG
    int                 empty_qset;
#endif /* DEBUG */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: %s(unit=%d, pri %d mode %d gid=%d)\n"),
               FUNCTION_NAME(),unit, pri, mode,group));

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get pipeline stage from qualifiers set. */
    retval =_robo_field_group_stage_get(unit, &qset, &stage);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;
    }

    /* Get field stage control pointer. */
    retval = _robo_field_stage_control_get(unit, stage, &stage_fc);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;
    }

    /* Group IDs must be unique within a unit. */
    retval = _robo_field_group_get (unit, group, &fg);
    if (BCM_SUCCESS(retval)){
        FP_UNLOCK(fc);
        return BCM_E_EXISTS;
    }
    
    /* If not specified, generate a priority. */
    if (pri == BCM_FIELD_GROUP_PRIO_ANY) {
        retval = _field_group_prio_make(stage_fc, &pri);
        if (BCM_FAILURE(retval)){
            FP_UNLOCK(fc);
            return retval;
        }
    } else if (pri < 0 || stage_fc->tcam_slices <= pri ) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: pri=%d out-of-range.\n"),
                   pri));
        return BCM_E_PARAM;
    }

    /* Allocate & initialize memory for field group. */
    fg = sal_alloc(sizeof (_field_group_t), "field_group");
    if (fg == NULL) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Allocation failure for field_group\n")));
        return BCM_E_MEMORY;
    }

    /* Check if this slice is used */
    if (stage_fc->slices[pri].group != NULL) {
        sal_free(fg);
        FP_UNLOCK(fc);
        return BCM_E_CONFIG;
    }

    if (stage_fc->field_shared_entries_free == 0) {
        sal_free(fg);
        FP_UNLOCK(fc);
        return BCM_E_RESOURCE;
    }

    sal_memset(fg, 0, sizeof (_field_group_t));

    fg->unit       = unit;
    fg->gid        = group;
    fg->mode       = mode;
    fg->stage_id = stage;
    fg->slices     = &stage_fc->slices[pri];

    sal_memcpy(&(fg->qset), &qset, sizeof (bcm_field_qset_t));
    _FIELD_SELCODE_CLEAR(unit, fg->slices[0].sel_codes);
    BCM_FIELD_QSET_INIT(fg->slices[0].qset);

    retval = _field_selcode_get(unit, fg->qset, fg);

    if (BCM_FAILURE(retval)) {
        sal_free(fg);
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Failure in _field_selcode_get()\n")));
        return BCM_E_UNAVAIL;
    }
    fg->slices[0].inst_flg = 0;
    fg->slices[0].group = fg;

    retval = _robo_field_group_status_init(unit, &fg->group_status);
    if (BCM_FAILURE(retval)) {
        sal_free(fg);
        FP_UNLOCK(fc);
        return retval;
    }

    retval = _robo_field_group_qset_update(unit, fg);
    if (BCM_FAILURE(retval)) {
        sal_free(fg);
        FP_UNLOCK(fc);
        return retval;
    }


#ifdef DEBUG
    if (BCM_DEBUG_CHECK(BCM_DBG_FP | BCM_DBG_VERBOSE)) {
        LOG_CLI((BSL_META_U(unit,
                            "bcm_field_group_create_mode_id(gid=%d, "), group));
        _field_qset_dump("qset=", qset, ")\n");
    }
    /* Warn if using empty Qset. */
    empty_qset = 1;
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if(BCM_FIELD_QSET_TEST(qset, qual)) {
            empty_qset = 0;
            break;
        }
    }

    if(empty_qset != 0) {
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "FP Warning: Creating group with empty Qset\n")));
    }

#endif /* DEBUG */

    /* Increment the use counts for any UDFs used */
    retval = _robo_field_udf_usecount_increment(fc, fg);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        sal_free(fg);
        return retval;
    }

    /* Add default action set */
    retval = _robo_field_group_default_aset_set(unit, fg);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        sal_free(fg);
        return retval;
    }

    /* Insert new field group into head of Unit's groups list */
    fg->next = fc->groups;
    fc->groups = fg;

    if(soc_feature(unit, soc_feature_field_slice_enable)) {
        retval = bcm_robo_field_group_enable_set(unit, group, TRUE);
    }
    FP_UNLOCK(fc);
    return retval;
}

/*
 * Function: bcm_robo_field_entry_destroy
 *
 * Purpose:
 *     Destroy an entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_XXX       - From bcm_field_counter_destroy() or
 *                       bcm_field_meter_destroy()
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_entry_destroy(int unit,
                bcm_field_entry_t entry)
{
    _field_group_t      *fg;
    _field_entry_t      *f_ent;
    _field_stage_t      *stage_fc;
    int rv;
    _field_control_t    *fc;         /* Field control structure. */
    int chain_entry = _FP_INVALID_INDEX;
    int chain_slice = _FP_INVALID_INDEX;
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_entry_destroy(unit=%d, entry=%d)\n"),
               unit, entry));

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    
    fg =  f_ent->group;
    assert(fg != NULL);

    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    rv = bcm_robo_field_entry_remove(unit, entry);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        chain_entry = 1;
    }
    if (f_ent->flags & _FP_ENTRY_CHAIN_SLICE) {
        chain_slice = 1;
    }
    /* Destroy physical entry in primary slice */
    rv = _robo_field_entry_phys_destroy(unit, f_ent);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    

    /* Increment group's count of free entries. */
    stage_fc->field_shared_entries_free++;
    
    if (chain_slice == 1) {
        stage_fc->tcam_bottom++;
    }
    if (chain_entry == 1) {
        stage_fc->field_shared_entries_free++;
        stage_fc->tcam_bottom++;
    }

    assert(stage_fc->field_shared_entries_free <= stage_fc->tcam_sz);

    fg->group_status.entry_count --;

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}


/*
 * Function: bcm_robo_field_entry_destroy_all
 *
 * Purpose:
 *     Destroy all entries on a unit. It iterates over all slices in a unit.
 *     For each slice, If entries exist, it calls bcm-field_entry_destroy()
 *     using the Entry ID.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_XXX       - Error from bcm_field_entry_destroy()
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_entry_destroy_all(int unit)
{
    _field_control_t    *fc;
    _field_stage_t      *stage_fc;
    int         rv;
    uint16              slice_idx;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_entry_destroy_all(unit=%d)\n"),
               unit));

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    stage_fc = fc->stages;

    while (NULL != stage_fc)
    {
        for (slice_idx = 0; slice_idx < stage_fc->tcam_sz; slice_idx++) {
            if (NULL != stage_fc->field_shared_entries[slice_idx]) {
                rv = bcm_robo_field_entry_destroy(unit, stage_fc->field_shared_entries[slice_idx]->eid);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }
            }
        }
        stage_fc = stage_fc->next;
        if (stage_fc == fc->stages){
            break;
        }
    }
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}


/* bcm_field_entry_dump not dispatchable */

/*
 * Function: bcm_robo_field_entry_install
 *
 * Purpose:
 *     Install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit.
 *     BCM_E_XXX       - Error from _field_XX_tcam_policy_install()
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Qualifications should be made and actions should be added
 *     prior to installing the entry.
 */
int 
bcm_robo_field_entry_install(int unit,
                bcm_field_entry_t entry)
{
    int                 tcam_idx, tcam_chain_idx;
    _field_control_t    *fc;
    _field_entry_t      *f_ent;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;
    int rv;
    _field_entry_policer_t *f_ent_pl;           /* Field entry policer .     */
    _field_policer_t       *f_pl;               /* Policer descriptor.       */
    _field_entry_stat_t    *f_ent_st;  /* Field entry stat structure.  */
    _field_stat_t          *f_st;      /* Field statistics descriptor. */
    int idx;
    uint64 value;
    
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "%s entry %d\n"),
               FUNCTION_NAME(),entry));
    FP_LOCK(fc);
    
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    fg = f_ent->group;

    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return rv;
    }
    rv = _robo_field_entry_tcam_idx_get(stage_fc, f_ent, &tcam_idx,
                        &tcam_chain_idx);

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "tcam_idx %d  tcam_chain_idx %d\n"),
               tcam_idx, tcam_chain_idx));

    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return rv;
    }
    /* Clear the TCAM and Policy entry */
    rv = _field_tcam_policy_clear(unit, fg->stage_id,tcam_idx, tcam_chain_idx);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Failed to clear tcam and policy tables.\n")));
        FP_UNLOCK(fc);
        return rv;
    }

    f_ent_pl = f_ent->policer;
    if (f_ent_pl->flags & _FP_POLICER_VALID) {
        /* Read policer configuration. */
        rv = _robo_field_policer_get(unit, f_ent_pl->pid, &f_pl);
   
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: Failed to get policer config\n")));
            FP_UNLOCK(fc);
            return rv;
        }
        if ((f_pl->hw_flags & _FP_POLICER_DIRTY) || 
            (f_ent->flags &  _FP_ENTRY_DIRTY)){
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "install meter policer id %d mode %d \n"),
                         f_ent_pl->pid, f_pl->cfg.mode));
            rv = DRV_FP_POLICER_CONTROL(unit, fg->stage_id, 
                DRV_FIELD_POLICER_CONFIG, f_ent->drv_entry, 
                (drv_policer_config_t *)&(f_pl->cfg));
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: Failed to set policer config \n")));
                FP_UNLOCK(fc);
                return rv;
            }
            rv = _field_meter_install(unit, fg->stage_id, f_ent,
                tcam_idx, tcam_chain_idx);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: FB failed to install meter.\n")));
                FP_UNLOCK(fc);
                return rv;
            }
            f_pl->hw_flags &= ~_FP_POLICER_DIRTY;
        }
        f_ent_pl->flags |= _FP_POLICER_INSTALLED;   
    }

    f_ent_st = &f_ent->statistic;
    if (f_ent_st->flags & _FP_ENTRY_STAT_VALID) {    
        rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return rv;
        }

        COMPILER_64_ZERO(value);
        for (idx = 0; idx < f_st->nstat; idx++) {
                rv = _robo_field_stat_value_set(unit, f_st, f_st->stat_arr[idx], value, TRUE);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return rv;
                }
        }

        if (0 == (f_ent_st->flags & _FP_ENTRY_STAT_INSTALLED)) {
            f_st->hw_ref_count++;
            if ((1 == f_st->hw_ref_count) && (f_st->nstat > 0)) {
                /* Write individual statistics privious value. */
                for (idx = 0; idx <COUNTOF(f_st->_field_x32_counters); idx++) {
                    memcpy(
                        f_st->_field_x32_counters+idx,
                        f_st->stat_values + idx, 
                        sizeof(f_st->stat_values[idx]));
                }
            }
            f_ent_st->flags |=  _FP_ENTRY_STAT_INSTALLED;
        }

    }
    /* Install the TCAM entry along with any needed policies (actions).
     * Note that this includes installing any counters. */
    rv = _robo_field_tcam_policy_install(unit, fg->stage_id, f_ent, 
            tcam_idx, tcam_chain_idx);

    if (BCM_SUCCESS(rv)) {
        f_ent->flags &= ~_FP_ENTRY_DIRTY;
        f_ent->flags |= _FP_ENTRY_INSTALL;
        f_ent->flags |= _FP_ENTRY_ENABLED;
    } else {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: failed to install entry.\n")));
    }
    FP_UNLOCK(fc);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "bcm_robo_field_entry_install(0-7) : data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,0x%x, 0x%x\n"),
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[0], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[1], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[2], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[3], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[4], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[5],
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[6], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[7]));
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "bcm_robo_field_entry_install(8-14) : data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,0x%x\n"),
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[8], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[9], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[10], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[11], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[12], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[13],
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[14]));
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "bcm_robo_field_entry_install : mask= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,0x%x, 0x%x\n"),
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[0], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[1], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[2], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[3], 
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[4], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[5],
                 ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[6], ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[7]));

    return rv;
}

/*     
 * Function:
 *      bcm_field_entry_policer_attach
 * Purpose:
 *      Attach a policer to a field entry.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      entry_id   - (IN) Field entry ID.
 *      level      - (IN) Policer level.
 *      policer_id - (IN) Policer ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */ 
int
bcm_robo_field_entry_policer_attach(int unit, bcm_field_entry_t entry_id,
                                    int level, bcm_policer_t policer_id)
{
    _field_entry_policer_t *f_ent_pl; /* Field entry policer structure.*/
    _field_control_t       *fc;       /* Field control structure.      */
    _field_policer_t       *f_pl;     /* Internal policer descriptor.  */
    _field_entry_t         *f_ent;    /* Internal entry descriptor.    */
    _field_stage_id_t      stage_id;  /* Pipeline stage id.            */
    int                    idx;       /* Entry policers iterator.      */
    int                    rv;        /* Operation return status.      */


    /* Input parameters check. */
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        return (BCM_E_PARAM);
    }

    FIELD_IS_INIT(unit);
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Get entry description structure. */
    rv = _robo_field_entry_get(unit, entry_id, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }


    /* Get entry pipeline stage id. */
    stage_id = f_ent->group->stage_id;


    /* Make sure stage has meters. */ 
    if (_BCM_FIELD_STAGE_INGRESS != stage_id) {
        FP_UNLOCK(fc);
        return (BCM_E_UNAVAIL);
    }

    /* Check if another  policer already attached at this level. */
    f_ent_pl = f_ent->policer + level;
    if (f_ent_pl->flags & _FP_POLICER_VALID) {
        FP_UNLOCK(fc);
        return (BCM_E_EXISTS);
    }


    /* Check if policer already attached to the entry. */
    for (idx = 0; idx < _FP_POLICER_LEVEL_COUNT; idx++) {
        f_ent_pl = f_ent->policer + idx;
        if (f_ent_pl->pid == policer_id) {
            FP_UNLOCK(fc);
            return (idx == level) ? (BCM_E_NONE) : (BCM_E_PARAM);
        }
    }

    /* Get policer description structure. */
    rv = _robo_field_policer_get(unit, policer_id, &f_pl);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }


    /* Check that policer can be shared. */
    if (f_pl->sw_ref_count > 1) {
        /* Policer can't be shared in ROBO arch*/
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "Policer can't be shared in ROBO arch")));
        FP_UNLOCK(fc);
        return (BCM_E_PARAM);
    }

    /* Check policer mode support. */    
    rv = _robo_field_policer_mode_support(unit, f_ent, level, f_pl);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    /* Increment policer reference counter. */
    f_pl->sw_ref_count++;

    /* Set policer stage. */
    f_pl->stage_id = stage_id;

    /* Set policer attachment level. */
    f_pl->level = level;

    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        f_pl->hw_index = f_ent->chain_idx;
    } else {
        f_pl->hw_index = f_ent->slice_idx;
    }

    /* Attach policer to an entry. */
    f_ent_pl = f_ent->policer + level;
    f_ent_pl->flags |= _FP_POLICER_VALID;
    f_ent_pl->pid    = policer_id;

    /* Entry must be reinstalled for policer to take effect. */
    f_ent->flags  |= _FP_ENTRY_DIRTY;

    /* Increment group meter count. */
    f_ent->group->group_status.meter_count++;

    FP_UNLOCK(fc);

    return BCM_E_NONE;
}   

/*
 * Function:
 *      bcm_field_entry_policer_detach
 * Purpose:
 *      Detach a policer from a field entry.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      entry_id - (IN) Field entry ID.
 *      level    - (IN) Policer level.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_entry_policer_detach(int unit, bcm_field_entry_t entry_id,
                                    int level)
{

    _field_control_t       *fc;       /* Field control structure.      */
    _field_entry_t         *f_ent;    /* Internal entry descriptor.    */
    int                     rv;       /* Operation return status.      */

    /* Input parameters check. */
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get entry description structure. */
    rv = _robo_field_entry_get(unit, entry_id, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    rv = _robo_field_entry_policer_detach(unit, f_ent, level);

    FP_UNLOCK(fc);
    return (rv);

}

/*
 * Function:
 *      bcm_field_entry_policer_detach_all
 * Purpose:
 *      Detach all policers from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry_id - (IN) Field entry ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_entry_policer_detach_all(int unit, bcm_field_entry_t entry_id)
{
    int       idx;                /* Entry policers iterator. */
    int       rv = BCM_E_NONE;    /* Operation return status. */

    /* Detach all the policers attached to an entry. */
    for (idx = 0; idx < _FP_POLICER_LEVEL_COUNT; idx++) {
        rv =  bcm_robo_field_entry_policer_detach(unit, entry_id, idx);
        if (BCM_E_EMPTY == rv) {
            /* No policer at this level. */
            rv = BCM_E_NONE;
        } else if (BCM_FAILURE(rv)) {
            break;
        }
    }
    return (rv);
}

/*
 * Function:
 *      bcm_field_entry_policer_get
 * Purpose:
 *      Get the policer(s) attached to a field entry.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      entry_id   - (IN) Field entry ID.
 *      level      - (IN) Policer level.
 *      policer_id - (OUT) Policer ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_entry_policer_get(int unit, bcm_field_entry_t entry_id,
                                int level, bcm_policer_t *policer_id)
{
    _field_entry_policer_t *f_ent_pl;/* Field entry policer structure.*/
    _field_entry_t         *f_ent;   /* Internal entry descriptor.    */
    _field_control_t       *fc;      /* Field control structure.      */
    int                    rv;       /* Operation return status.      */

    /* Input parameters check. */
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        return (BCM_E_PARAM);
    }

    if (NULL == policer_id) {
        return (BCM_E_PARAM);
    }

    FIELD_IS_INIT(unit);
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Get entry description structure. */
    rv = _robo_field_entry_get(unit, entry_id, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    f_ent_pl = f_ent->policer + level;
    /* Make sure policer attached to the entry. */
    if (0 == (f_ent_pl->flags & _FP_POLICER_VALID)) {
        rv = (BCM_E_NOT_FOUND);
    } else {
        *policer_id = f_ent_pl->pid;
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: bcm_robo_field_entry_prio_get
 *
 * Purpose:
 *     Gets the priority within the group of the entry.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Field entry to operate on
 *     prio   - (OUT) priority of entry
 *
 * Returns:
 *     BCM_E_NONE       - Success
 *     BCM_E_PARAM      - prio pointing to NULL
 *     BCM_E_NOT_FOUND  - Entry ID not found on unit
 */
int
bcm_robo_field_entry_prio_get(int unit, bcm_field_entry_t entry, int *prio)
{
    _field_entry_t      *f_ent;
    _field_control_t    *fc;         /* Field control structure. */
    int                 rv;          /* Operation return status. */

    if (prio == NULL ) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: prio==NULL\n")));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);
    
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    *prio = f_ent->prio;
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_entry_prio_set
 *
 * Purpose:
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_NONE       Success
 */
int
bcm_robo_field_entry_prio_set(int unit, bcm_field_entry_t entry, int prio)
{
    int              slice_idx_target = 0, temp;
    _field_group_t      *fg = NULL;
    _field_stage_t      *stage_fc;    
    _field_entry_t      *f_ent_pri = NULL;/* Entry in primary slice   */
    int    prev_null_idx = 0, next_null_idx = 0;
    int    prev_null_found = 0, next_null_found = 0;
    int    shift_up_amount = 0, shift_down_amount = 0;
    int intr_level = 0, rv;
    int flag_no_free_entries = FALSE;
    int hit = 0;
    _field_control_t    *fc;         /* Field control structure. */
    int src_idx, target_idx;
    int i, entry_reinstall;
    _field_stat_t *f_st;
    _field_entry_stat_t *f_ent_st;
        
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry, &f_ent_pri, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }   
    fg = f_ent_pri->group;

    /* Get stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    robo_prio_set_with_no_free_entries[unit] = FALSE; /* Global variable */    

    /* We divide CFP tcam to two regions main and slice3 region.
     * For those chips don't have slice 3, there would be one main region only.
     * Prioirity compare and entry move work independently in two regions.
     */

    /* 
      * Slice3 region contains 1)slice3 only and 
      *                           2)the slice 3 being as a chain with slice 0.
      * And it sets between  stage_fc->tcam_bottom ~ stage_fc->tcam_sz.
      * In this region, priority need to be sorted. 
      * Higher priority entry sets with lower tcam index.
      */
    if ((f_ent_pri->flags & _FP_ENTRY_CHAIN_SLICE) ||
        (f_ent_pri->flags & _FP_ENTRY_WITH_CHAIN)) {
        target_idx = _FP_INVALID_INDEX;

        for (i = stage_fc->tcam_bottom; i < stage_fc->tcam_sz; i++) {
            if (_robo_field_entry_prio_cmp(stage_fc->field_shared_entries[i]->prio, prio) < 0) {
                target_idx = i;
                break;
            }
        }
        if (target_idx == _FP_INVALID_INDEX) {
            target_idx =  stage_fc->tcam_sz - 1;
        }
        if (f_ent_pri->flags & _FP_ENTRY_CHAIN_SLICE) {
            src_idx = f_ent_pri->slice_idx;
        } else {
            src_idx = f_ent_pri->chain_idx;
        }
        if (src_idx != target_idx) {
            prev_null_idx = _FP_INVALID_INDEX;
            entry_reinstall = 0;

            for(i = stage_fc->tcam_bottom-1; i >= 0; i--) {
                if (NULL == stage_fc->field_shared_entries[i]) {
                    prev_null_idx = i;
                    break;
                }
            }
            if (_FP_INVALID_INDEX != prev_null_idx) {
                if (soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                     rv = _robo_field_entry_block_move(unit, f_ent_pri->group,
                            src_idx, prev_null_idx, 
                            1, _BCM_FP_CHAIN_ENTRY);
                } else {
                   rv = _robo_field_entry_move(unit, 
                        f_ent_pri, 
                        (prev_null_idx - src_idx), _BCM_FP_CHAIN_ENTRY); 
                }
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
            } else {
                entry_reinstall = 1;       
                if (f_ent_pri->statistic.flags & _FP_ENTRY_STAT_VALID) {
                    rv = _robo_field_stat_hw_free(unit, f_ent_pri);
                    if (BCM_FAILURE(rv)) {
                        FP_UNLOCK(fc);  
                        return (rv);
                    }               
                }
            }

            if (src_idx > target_idx) {
                if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                     rv = _robo_field_entry_block_move(unit, f_ent_pri->group,
                            target_idx, target_idx+1, 
                            (src_idx - target_idx), 
                            _BCM_FP_CHAIN_ENTRY);
                } else {
                    rv = _robo_field_entry_shift_down(unit, &fg->slices[0], 
                        target_idx, src_idx, _BCM_FP_CHAIN_ENTRY);
                }
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
                
            } else {
                if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                     rv = _robo_field_entry_block_move(unit, f_ent_pri->group,
                            src_idx+1, src_idx, 
                            (target_idx - src_idx), 
                            _BCM_FP_CHAIN_ENTRY);
                } else {
                    rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                        target_idx, src_idx, _BCM_FP_CHAIN_ENTRY);
                }
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
            }    
              
            if (1 != entry_reinstall) {
                if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                     rv = _robo_field_entry_block_move(unit, f_ent_pri->group,
                            prev_null_idx, target_idx,
                            1, _BCM_FP_CHAIN_ENTRY);
                } else {
                   rv = _robo_field_entry_move(unit, 
                        f_ent_pri, 
                        (target_idx - prev_null_idx), _BCM_FP_CHAIN_ENTRY);
                }
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);  
                    return (rv);
                }               
            } else {
                stage_fc->field_shared_entries[target_idx] = f_ent_pri;
                if (f_ent_pri->statistic.flags & _FP_ENTRY_STAT_VALID) {
                    /* Get statistics entity descriptor. */
                    f_ent_st = &f_ent_pri->statistic;
                    rv = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
                    if (BCM_FAILURE(rv)) {
                        FP_UNLOCK(fc);  
                        return (rv);
                    }            
                    f_st->hw_index = target_idx;
                }
                if (f_ent_pri->flags & _FP_ENTRY_CHAIN_SLICE) {
                    f_ent_pri->slice_idx = target_idx;
                } else {
                    f_ent_pri->chain_idx = target_idx;
                }
                rv = bcm_robo_field_entry_reinstall(unit, f_ent_pri->eid);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);  
                    return (rv);
                }               
            }
        }
        if (f_ent_pri->flags & _FP_ENTRY_CHAIN_SLICE) {            
            f_ent_pri->prio = prio;
        }
    }


    /* If the priority isn't changing, just return.*/
    if (f_ent_pri->prio == prio) {
        FP_UNLOCK(fc);
        return BCM_E_NONE;
    }
    /* 
     * Main region contains slice 0~2. 
     * And it sets between 0 ~ stage_fc->tcam_bottom.
     * In this region, priority need to be sorted. 
     * Higher priority entry sets with lower tcam index.
     */

    if (_field_robo_reqd_prio_set_move(unit, f_ent_pri, prio) == FALSE) {
        goto end;
    }

    if (stage_fc->field_shared_entries_free <= 0) {
        if ((f_ent_pri->flags & _FP_ENTRY_DIRTY)) {
            /*
             * As there are no free entries in any of the slices belonging to
             * this group, and this entry is NOT installed,
             *     fake that it does not exist.
             */
            intr_level = sal_splhi();
            stage_fc->field_shared_entries[f_ent_pri->slice_idx] = NULL;
            flag_no_free_entries = TRUE;
        } else {
            FP_UNLOCK(fc);
            return BCM_E_CONFIG;
        }
    }

    assert(f_ent_pri->slice_idx < stage_fc->tcam_sz);

    /* Find the target slice index. That is the one with the highest index
     * for this priority class. */
    for (slice_idx_target = 0;
         slice_idx_target < stage_fc->tcam_bottom;
         slice_idx_target++){

        if (stage_fc->field_shared_entries[slice_idx_target] == NULL) {
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "Found an empty slice_idx=%d\n"),
                       slice_idx_target));
            /* Find the previous free entry */
            prev_null_found = 1;
            prev_null_idx = slice_idx_target;
        } else {
            if (stage_fc->field_shared_entries[slice_idx_target] == f_ent_pri) {
                /* Skip the entry itself */
                continue;
            }           
            if (_robo_field_entry_prio_cmp(prio, 
                stage_fc->field_shared_entries[slice_idx_target]->prio)
                > 0) {
                hit = 1;
                LOG_DEBUG(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "Found target slice_idx=%d %p \n"), 
                           slice_idx_target,stage_fc->field_shared_entries[slice_idx_target]));
                break;
            }
        }
        if (slice_idx_target == (stage_fc->tcam_bottom - 1)) {
            /* last entry in the slice */
            break;
        }
    }

    /* Find next free entry */
    temp = slice_idx_target;
    for (; temp < stage_fc->tcam_bottom ; temp++) {

        if (stage_fc->field_shared_entries[temp] == NULL) {
            next_null_found = 1;
            next_null_idx = temp;
            break;
        }
    }

    /* 
     * Put the entry back, 
     *     in case there is a context switch, AND
     *     another thread calls entry_create
     */
    if (flag_no_free_entries == TRUE) {
        stage_fc->field_shared_entries[f_ent_pri->slice_idx] = f_ent_pri;
        sal_spl(intr_level);
    }

    if ((!prev_null_found) && (!next_null_found)) {
        FP_UNLOCK(fc);
        return BCM_E_CONFIG;
    }

    if (prev_null_found) {
        shift_up_amount = slice_idx_target - prev_null_idx;
    } else {
        shift_up_amount = stage_fc->tcam_sz;
    }
    
    if (next_null_found) {
        shift_down_amount = next_null_idx - slice_idx_target;
    } else {
        shift_down_amount = stage_fc->tcam_sz;
    }

    /* Move the entry at the target index to target_index+1. This may
     * mean shifting more entries down to make room. In other words,
     * shift the target index and any that follow it down 1 as far as the
     * next empty index.
     */

    if (stage_fc->field_shared_entries[slice_idx_target] != NULL) {
        if (shift_down_amount == stage_fc->tcam_sz) {
            /* 
             * from slice_idx_target to max tcam size index are all full.
             * Can't move any existed entries down.
             * Can only set to the prev_null_idx(shift up). 
             */
            if (slice_idx_target == (stage_fc->tcam_bottom -1)) {
                if (hit) {
                    /*
                     * If ("hit" && "target idx is the lastest entry id").
                     * The original latest entry can't move down any further,
                     * the only way is to move the previous one up and insert the
                     * new rule to the previous(latest - 1) entry.
                     */
                    slice_idx_target -= 1;
                }
                if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                    rv = _robo_field_entry_block_move(unit, fg,
                        prev_null_idx+1, prev_null_idx, 
                        (slice_idx_target - prev_null_idx), _BCM_FP_MAIN_ENTRY);
                    if (BCM_FAILURE(rv)) {
                        FP_UNLOCK(fc);
                        return (rv);
                    }   
                } else {
                    rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                    slice_idx_target, prev_null_idx, _BCM_FP_MAIN_ENTRY);
                    if (BCM_FAILURE(rv)) {
                        FP_UNLOCK(fc);
                        return (rv);
                    }   
                }
            } else {
                slice_idx_target -= 1;

                if (stage_fc->field_shared_entries[slice_idx_target] != NULL) {
                    /* If the previous entries are not null, need to move them up. */
                    if(soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                        rv = _robo_field_entry_block_move(unit, fg,
                            prev_null_idx+1, prev_null_idx, 
                            (slice_idx_target - prev_null_idx), _BCM_FP_MAIN_ENTRY);
                        if (BCM_FAILURE(rv)) {
                            FP_UNLOCK(fc);
                            return (rv);
                        }   
                    } else {
                        rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                        slice_idx_target, prev_null_idx, _BCM_FP_MAIN_ENTRY);
                        if (BCM_FAILURE(rv)) {
                            FP_UNLOCK(fc);
                            return (rv);
                        }   
                   }
                }
            }
        } else if (shift_down_amount < shift_up_amount) {
            if (soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit, fg,
                    slice_idx_target, slice_idx_target+1, 
                    (next_null_idx - slice_idx_target), _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
            } else {
                rv = _robo_field_entry_shift_down(unit, &fg->slices[0], 
                    slice_idx_target, next_null_idx, _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
            }
        } else {
            /* 
             * If the direction is up, the target entry 
             * will be the previous entry of original one. 
             */
            slice_idx_target = slice_idx_target - 1;
            if (soc_feature(unit, soc_feature_field_tcam_hw_move)) {
                rv = _robo_field_entry_block_move(unit, fg,
                    prev_null_idx+1, prev_null_idx, 
                    (slice_idx_target - prev_null_idx), _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
            } else {
                rv = _robo_field_entry_shift_up(unit, &fg->slices[0], 
                slice_idx_target, prev_null_idx, _BCM_FP_MAIN_ENTRY);
                if (BCM_FAILURE(rv)) {
                    FP_UNLOCK(fc);
                    return (rv);
                }   
           }
        }
    }

    /* Move the entry from its old slice index to the target slice index. */
    if (slice_idx_target != f_ent_pri->slice_idx) {
        if (flag_no_free_entries) {
            robo_prio_set_with_no_free_entries[unit] = TRUE;
        }
        rv = _robo_field_entry_move(unit, f_ent_pri,
                              slice_idx_target - f_ent_pri->slice_idx, 
                              _BCM_FP_MAIN_ENTRY);
        if (BCM_FAILURE(rv)) {
            robo_prio_set_with_no_free_entries[unit] = FALSE; 
            FP_UNLOCK(fc);
            return rv;
        }

        /* in case _robo_field_entry_move is called from some other function */
        robo_prio_set_with_no_free_entries[unit] = FALSE; 
    }


end:
    /* Assign the requested priority to the entry. */
    f_ent_pri->prio = prio;

    if (fg->group_status.prio_max < prio) {
        fg->group_status.prio_max = prio;
    }
    if (0 <= prio &&
        (prio < fg->group_status.prio_min || fg->group_status.prio_min < 0)) {
        fg->group_status.prio_min = prio;
    }
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}


/*
 * Function: bcm_robo_field_entry_reinstall
 *
 * Purpose:
 *     Re-install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_UNAVAIL   - Feature not implemented.
 *     BCM_E_XXX       - From _bcm_XXX_install() calls
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Reinstallation may only be used to change the actions for
 *     an installed entry without having to remove and re-add the
 *     entry.
 */
int 
bcm_robo_field_entry_reinstall(int unit,
                bcm_field_entry_t entry)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent;   
    int                 retval = BCM_E_UNAVAIL;
    int                 tcam_idx, tcam_chain_idx;
    _field_stage_t  *stage_fc;
    _field_group_t  *fg;
    _field_entry_policer_t *f_ent_pl;           /* Field entry policer .     */
    _field_policer_t       *f_pl;               /* Policer descriptor.       */
    _field_entry_stat_t    *f_ent_st;  /* Field entry stat structure.  */
    _field_stat_t          *f_st;      /* Field statistics descriptor. */
    int idx;    
    uint64 value;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);
    retval = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;        
    }

    fg = f_ent->group;
    
    retval = _robo_field_stage_control_get(unit,  fg->stage_id, &stage_fc);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;        
    }
        
   retval =  _robo_field_entry_tcam_idx_get(stage_fc, f_ent, &tcam_idx, 
                    &tcam_chain_idx);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        return retval;
    }

    /* If it exists and is changed, install the entry's meter. */
    f_ent_pl = f_ent->policer;
    if (f_ent_pl->flags & _FP_POLICER_VALID) {
        /* Read policer configuration. */
        retval = _robo_field_policer_get(unit, f_ent_pl->pid, &f_pl);
   
        if (BCM_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: Failed to get policer config\n")));
            FP_UNLOCK(fc);
            return retval;
        }
        if ((f_pl->hw_flags & _FP_POLICER_DIRTY) || 
            (f_ent->flags &  _FP_ENTRY_DIRTY)){
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "re-install meter policer id %d mode %d \n"),
                         f_ent_pl->pid, f_pl->cfg.mode));
            retval = DRV_FP_POLICER_CONTROL(unit, fg->stage_id, 
                DRV_FIELD_POLICER_CONFIG, f_ent->drv_entry, 
                (drv_policer_config_t *)&(f_pl->cfg));
            if (BCM_FAILURE(retval)) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: Failed to set policer config \n")));
                FP_UNLOCK(fc);
                return retval;
            }

            retval =  _field_meter_install(unit, fg->stage_id, f_ent,
            tcam_idx, tcam_chain_idx);

            if (BCM_FAILURE(retval)) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: failed to install meter\n")));
                FP_UNLOCK(fc);
                return retval;
            }
            f_pl->hw_flags &= ~ _FP_POLICER_DIRTY;
        }
        f_ent_pl->flags |=  _FP_POLICER_INSTALLED;
    }

    if (f_ent->flags &  _FP_ENTRY_DIRTY) {
        f_ent_st = &f_ent->statistic;
        if (f_ent_st->flags & _FP_ENTRY_STAT_VALID) {    
            /* Read stat entity configuration. */
            retval = _robo_field_stat_get(unit, f_ent_st->sid, &f_st);
            if (BCM_FAILURE(retval)) {
                FP_UNLOCK(fc);
                return retval;
            }

            if (0 == (f_ent_st->flags & _FP_ENTRY_STAT_INSTALLED)) {
                f_st->hw_ref_count++;
                if ((1 == f_st->hw_ref_count) && (f_st->nstat > 0)) {
                    /* Write individual statistics privious value. */
                    COMPILER_64_ZERO(value);
                    for (idx = 0; idx < f_st->nstat; idx++) {
                            retval = _robo_field_stat_value_set(unit, 
                                f_st, f_st->stat_arr[idx], value, TRUE);
                            if (BCM_FAILURE(retval)) {
                                FP_UNLOCK(fc);
                                return retval;
                            }
                    }   
                    for (idx = 0; idx <COUNTOF(f_st->_field_x32_counters); idx++) {
                        memcpy(
                            f_st->_field_x32_counters+idx,
                            f_st->stat_values + idx, 
                            sizeof(f_st->stat_values[idx]));
                    }   
                }
                f_ent_st->flags |=  _FP_ENTRY_STAT_INSTALLED;
            }
        
        }
        /* Write the TCAM and Policy entry */
        retval = _robo_field_tcam_policy_install(unit, fg->stage_id, f_ent, 
                        tcam_idx, tcam_chain_idx);
        if (BCM_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: FB failed to install entry\n")));
        } else {
            f_ent->flags &= ~_FP_ENTRY_DIRTY;
            f_ent->flags |= _FP_ENTRY_INSTALL;
        }
    } else {
        retval = BCM_E_NONE;
    }

    FP_UNLOCK(fc);

    return retval;
}

/*
 * Function: bcm_robo_field_entry_remove
 *
 * Purpose:
 *     Remove an entry from the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to remove
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_XXX
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     This does not destroy the entry; it uninstalls it from
 *     any hardware tables.
 *     Destroy a entry using bcm_field_entry_destroy.
 */

int
bcm_robo_field_entry_remove(int unit,
                bcm_field_entry_t entry)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent;
    _field_stage_t      *stage_fc;
    int                 retval = BCM_E_NONE;
    _field_stage_id_t   stage_id;
    int slice_idx = _FP_INVALID_INDEX, chain_idx = _FP_INVALID_INDEX;
    _field_entry_policer_t *f_ent_pl;  /* Field entry policer structure. */    
    _field_policer_t       *f_pl;      /* Policer descriptor.            */    
    

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_entry_remove(%d, %d)\n"),
               unit, entry));
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    retval = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);

    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;        
    }

    assert(f_ent);    
    stage_id = f_ent->fs->stage_id;

    retval = _robo_field_stage_control_get(unit,  stage_id, &stage_fc);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;        
    }
    if (f_ent->flags & _FP_ENTRY_INSTALL) {
        f_ent_pl = f_ent->policer;
        if (0 != (f_ent_pl->flags & _FP_POLICER_INSTALLED)) {
            retval = _robo_field_policer_get(unit, f_ent_pl->pid, &f_pl);
            if (BCM_FAILURE(retval)){
                FP_UNLOCK(fc);
                return retval;        
            }
            /* Decrement hw reference count. */
            if (f_pl->hw_ref_count > 0) {
                f_pl->hw_ref_count--; 
            }
            /* Mark policer for reinstallation. */
            retval = _robo_field_policer_hw_flags_set(unit, f_pl, 0);
            f_ent_pl->flags &= ~_FP_POLICER_INSTALLED;
        }
        _robo_field_entry_tcam_idx_get(stage_fc, f_ent, &slice_idx, &chain_idx);
        retval = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, NULL,
            DRV_FIELD_ENTRY_TCAM_REMOVE, slice_idx, &chain_idx);   
        if (BCM_FAILURE(retval)){
            FP_UNLOCK(fc);
            return retval;        
        }
        f_ent->flags &= ~_FP_ENTRY_INSTALL;
        f_ent->flags |= _FP_ENTRY_DIRTY;
        f_ent->flags &= ~_FP_ENTRY_ENABLED;
    }
    FP_UNLOCK(fc);

    return retval;
}

/*
 * Function: bcm_field_entry_enable_set
 *
 * Purpose:
 *     Enable/Disable an entry from the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to be enabled/disabled
 *     enable_flag - Flag to enable or disable
 *
 * Returns:
 *     BCM_E_XXX
 *
 * Notes:
 *     This does not destroy the entry, nor deallocate any related resources;
 *     it only enables/disables a rule from hardware table using VALIDf of the
 *     corresponding hardware entry. To deallocate the memory used by the entry
 *     call bcm_field_entry_destroy.
 */
int
bcm_robo_field_entry_enable_set(int unit, bcm_field_entry_t entry, int enable_flag)
{
    _field_control_t    *fc;         /* Field control structure. */
    _field_entry_t      *f_ent;      /* Field entry pointer.     */
    _field_stage_t      *stage_fc;   /* Stage field control.            */
    _field_stage_id_t   stage_id;
    int slice_idx = _FP_INVALID_INDEX, chain_idx = _FP_INVALID_INDEX;
    int                 rv;          /* Operation return status. */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP(unit %d) vverb: bcm_field_entry_enable_set "
                          "(entry=%d, enable=%d)\n"),
               unit, entry, enable_flag));

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (!(f_ent->flags & _FP_ENTRY_INSTALL)) {
        FP_UNLOCK(fc);
        return (BCM_E_PARAM);
    }

    stage_id = f_ent->fs->stage_id;
    rv = _robo_field_stage_control_get(unit,  stage_id, &stage_fc);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;        
    }

     /* Get tcam indexes for installed entry. */
    _robo_field_entry_tcam_idx_get(stage_fc, f_ent, &slice_idx, &chain_idx);

    /* Set the VALIDf bits appropriately */
    if (enable_flag) {
        rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, NULL,
            DRV_FIELD_ENTRY_TCAM_ENABLE, slice_idx, &chain_idx);
    } else {
        rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, stage_id, NULL,
            DRV_FIELD_ENTRY_TCAM_REMOVE, slice_idx, &chain_idx);
    }
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;        
    }

    if (enable_flag) {
       f_ent->flags |= _FP_ENTRY_ENABLED; 
    }
    else {
        f_ent->flags &= ~_FP_ENTRY_ENABLED;
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function: bcm_field_entry_enable_get
 *
 * Purpose:
 *     Get the Enable or Disable status of a field Entry.
 *
 * Parameters:
 *     unit - (IN)BCM device number
 *     entry - (IN)Entry to be checked for Enabled status
 *     enable_flag - (OUT)Status(Enable/Disable) of the given entry
 *
 * Returns:
 *     BCM_E_XXX
 *
 */
int
bcm_robo_field_entry_enable_get(int unit, bcm_field_entry_t entry, int *enable_flag)
{
    _field_entry_t      *f_ent;      /* Field entry pointer.     */
    int                 rv;          /* Operation return status. */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP(unit %d) vverb: bcm_field_entry_enable_get "
                          "(entry=%d)\n"),
               unit, entry));

    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    if (!(f_ent->flags & _FP_ENTRY_INSTALL)) {
        return (BCM_E_PARAM);
    }

    *enable_flag = (f_ent->flags & _FP_ENTRY_ENABLED) ? 1 : 0;
    return (BCM_E_NONE);
}


/*
 * Function: bcm_robo_field_group_create
 *
 * Purpose:
 *     Create a field group based on the field group selector flags.
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *     group - (OUT) New field group ID
 *
 * Returns:
 *     BCM_E_INIT     - BCM unit not initialized
 *     BCM_E_XXX      - Error code from _field_group_prio_make()
 *     BCM_E_PARAM    - pri out of range (0-15 for FB & ER) or group == NULL
 *     BCM_E_RESOURCE - no select codes will satisfy qualifier set
 *     BCM_E_NONE     - Success
 *
 * Notes:
 *      Allocates a hardware slice at the requested priority or better.
 *      Higher numerical value for priority has better priority for
 *      conflict resolution when there is a search hit on multiple slices.
 */

int 
bcm_robo_field_group_create(int unit,
               bcm_field_qset_t qset,
               int pri,
               bcm_field_group_t *group)
{
    int     rv;
    
    FIELD_IS_INIT(unit);

    rv = _robo_field_group_id_generate(unit, qset, group, pri);
    if ( rv == BCM_E_EXISTS) {
        return BCM_E_NONE;
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: new group won't create.\n")));
        return rv;
    }

    return bcm_robo_field_group_create_mode_id(unit, qset, pri,
                                          bcmFieldGroupModeDefault, *group);
}


/*
 * Function: bcm_robo_field_group_create_id
 *     
 * Purpose:
 *     Create a field group based on the field group selector flags
 *     with a requested ID
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    group - Requested field group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - No unused group/slices left
 *     BCM_E_PARAM     - priority out of range (0-15 for FB & ER)
 *     BCM_E_EXISTS    - group with that id already exists on this unit.
 *     BCM_E_MEMORY    - Group memory allocation failure
 *     BCM_E_XXX       - Error code from _field_group_prio_make().
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_group_create_id(int unit,
                  bcm_field_qset_t qset,
                  int pri,
                  bcm_field_group_t group)
{
    FIELD_IS_INIT(unit);

    return bcm_robo_field_group_create_mode_id(unit, qset, pri,
                                          bcmFieldGroupModeDefault, group);
}

/*
 * Function: bcm_robo_field_group_destroy
 *    
 * Purpose:
 *     Delete a field group
 *     
 * Parameters:
 *     unit - BCM device number
 *     port - Port number
 *     group - Field group
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_BUSY      - Entries not destroyed yet
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *      All entries that uses this group should have been destroyed
 *      before calling this routine.
 *      Operation will fail if entries exist that uses this temptlate
 */
int 
bcm_robo_field_group_destroy(int unit,
                bcm_field_group_t group)
{
    _field_control_t    *fc;
    _field_group_t      *fg,                /* Group structure to free up   */
                        *fg_prev = NULL;    /* Previous node in groups list */
    int                 idx;
    uint32              udf_num;
    int                 rv;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_field_group_destroy(unit=%d, group=%d)\n"),
               unit, group));

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Search the field control for the Group ID. */
    fg = fc->groups;
    while(fg != NULL) {
        if (fg->gid == group) {
            break;
        }
        fg_prev = fg;
        fg      = fg->next;
    }
    if (fg == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Group=%d not found in unit=%d.\n"),
                   group, unit));
        FP_UNLOCK(fc);
        return BCM_E_NOT_FOUND;
    }
    
    /* Entries must be freed first (see note above). */
    if (fg->group_status.entry_count != 0) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: %d entries still in group=%d.\n"), 
                   fg->group_status.entry_count,
                   group));
        return BCM_E_BUSY;
    }

    /* Clear primary slice used by this group. */
    rv = _robo_field_slice_clear(unit, fc, &fg->slices[0]);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    /* okay, go ahead and free the group */

    /* Decrement the use counts for any UDFs used by the group */
    rv = DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    for (idx = 0; idx < udf_num; idx++) {
        if (SHR_BITGET(fg->qset.udf_map, idx))  {
            if (fc->udf[idx].use_count > 0) {
                fc->udf[idx].use_count--;
            }
        }
    }

    if(soc_feature(unit, soc_feature_field_slice_enable)) {
        rv = bcm_robo_field_group_enable_set(unit, group, FALSE);
        if (BCM_FAILURE(rv)){
            FP_UNLOCK(fc);
            return rv;
        }
    }
    /* Remove this group from device's linked-list of groups. */
    if (fg_prev == NULL) { /* Group is at head of list. */
        fc->groups = fg->next;
    } else {
        fg_prev->next = fg->next;
    }

    sal_free(fg);

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}



/*
 * Function: bcm_robo_field_group_get
 *    
 * Purpose:
 *     Get the qualifier set for a previously created field group
 *     
 * Parameters:
 *     unit  - BCM device number
 *     port  - Port number
 *     group - Field group ID
 *     qset  - (OUT) Field qualifier set
 *     
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized.
 *     BCM_E_NOT_FOUND - Group ID not found in this unit
 *     BCM_E_PARAM     - qset is NULL
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_group_get(int unit,
            bcm_field_group_t group,
            bcm_field_qset_t *qset)
{
    _field_control_t       *fc;     /* Field control structure.    */
    _field_group_t      *fg;
    int rv;

    if (qset == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: qset == NULL\n")));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    *qset = fg->qset;
     FP_UNLOCK(fc);
    return BCM_E_NONE;
}

int
bcm_robo_field_group_lookup_disable(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_lookup_enable(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function: bcm_robo_field_group_get
 *
 * Purpose:
 *     Return the mode of a Group ID. This is its single, double or triple-wide
 *     state. Mode specified the number of slices allocated to the group.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group ID
 *     mode  - (OUT) single, double, triple or auto mode
 *     
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found for in this unit
 *     BCM_E_PARAM     - mode pointing to NULL
 *     BCM_E_NONE      - Success
 *     
 */
int
bcm_robo_field_group_mode_get(int unit,
                             bcm_field_group_t group,
                             bcm_field_group_mode_t *mode)
{
    _field_group_t      *fg;
    _field_control_t       *fc;     /* Field control structure.    */
    int rv;

    if (mode == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: mode=>NULL\n")));
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    *mode = fg->mode;

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

int 
bcm_robo_field_group_range_get(int unit,
                  bcm_field_group_t group,
                  uint32 flags,
                  bcm_l4_port_t *min,
                  bcm_l4_port_t *max)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_group_range_set(int unit,
                  bcm_field_group_t group,
                  uint32 flags,
                  bcm_l4_port_t min,
                  bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function: bcm_robo_field_group_set
 *    
 * Purpose:
 *     Update a previously created field group based on the field
 *     group selector flags
 *     
 * Parameters:
 *     unit  - BCM device number
 *     port  - Port number
 *     group - Field group ID
 *     qset  - Field qualifier set
 *     
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found for in this unit
 *     BCM_E_RESOURCE  - No select code can satisfy qualifier set
 *     BCM_E_NONE      - Success
 *     
 * Notes:
 *     If no entry exist that use this group then updates are always
 *     permitted.
 *     If entries exist that use this group then updates are permitted
 *     only if it can be satisfied with the current selection of
 *     (fpf0, fpf1, fpf2, fpf3) field selector encodings.
 */
int 
bcm_robo_field_group_set(int unit,
            bcm_field_group_t group,
            bcm_field_qset_t qset)
{
    bcm_field_qset_t        qset_pri;
    bcm_field_qset_t        qset_old;
    _field_control_t        *fc = NULL;
    _field_group_t          *fg = NULL;
    int                     retval;
    int                     re_create;
    _field_slice_t          *fs_pri = NULL;
    bcm_field_group_mode_t  mode;
    uint8                   slice;
    int     empty_qset, qual;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_field_group_set(unit=%d, group=%d)\n"),
               unit, group));

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);

    /* Get group descriptor. */
    retval = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(retval)){
        FP_UNLOCK(fc);
        return retval;
    }
    
    fs_pri           = &fg->slices[0];
    fs_pri->inst_flg = 0 ;

    BCM_FIELD_QSET_INIT(qset_pri);

    /* Check NULL qualifier set */
    empty_qset = 1;
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if(BCM_FIELD_QSET_TEST(qset, qual)) {
            empty_qset = 0;
            break;
        }
    }
    if (empty_qset) {
        FP_UNLOCK(fc);
        return BCM_E_UNAVAIL;
    }

    /* If no entries have been added to group try requested qset.  */
    if (fg->group_status.entry_count == 0) {
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP: bcm_field_group_set() with no entries\n")));

        /* Remember the group's current Qset, mode, and base slice. */
        sal_memset(&qset_old, 0, sizeof(bcm_field_qset_t));
        sal_memcpy(&qset_old, &fg->qset, sizeof(bcm_field_qset_t));
        slice = fg->slices[0].slice_numb;
        mode  = fg->mode;

        /* Destroy the old group. */
        retval = bcm_robo_field_group_destroy(unit, group);
        if (BCM_FAILURE(retval)) {
            FP_UNLOCK(fc);
            return retval;
        }

        /* Try to re-create the group with the requested Qset. */
        retval = bcm_robo_field_group_create_mode_id(unit, qset, slice, mode, group);

        /* On failure, re-create the old group. */
        if (BCM_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: new Qset won't work on group=%d.\n"),
                       group));
            re_create = bcm_robo_field_group_create_mode_id(unit, qset_old, slice,
                                                       mode, group);
            if (BCM_FAILURE(re_create)) { /* Should never fail. */
                FP_UNLOCK(fc);
                return BCM_E_INTERNAL;
            }
        }

        /* Release the unit's lock */
        FP_UNLOCK(fc);
        return retval;
    }

    /*
     * Handle cases where entries have been previously set in group.
     */

    /* If the requested Qset is supported by the current select codes, 
     * change the group's qset.
     */
    retval = _field_selcode_to_qset(unit, fs_pri->stage_id, fs_pri->sel_codes, &qset_pri);
    if (BCM_FAILURE(retval)) {
        FP_UNLOCK(fc);
        return retval;
    }

    /* qset_full represents total Qset that current select codes support. */
    if (_robo_field_qset_is_subset(&qset, &qset_pri)) {
        retval = _robo_field_qset_union(&fg->qset, &qset, &fg->qset);
        if (BCM_FAILURE(retval)) {
            FP_UNLOCK(fc);
            return retval;
        }
        FP_UNLOCK(fc);
        return BCM_E_NONE;
    } 

    FP_UNLOCK(fc);
    /* otherwise, no select codes can support */
    return BCM_E_RESOURCE;
}

int
bcm_robo_field_group_compress(int unit, bcm_field_group_t group)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_group_priority_set(int unit, bcm_field_group_t group,
                                 int priority)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_group_priority_get(int unit, bcm_field_group_t group,
                                 int *priority)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function: bcm_robo_field_group_status_get
 *
 * Purpose:
 *     Get the number of used and available entries, counters, and
 *     meters for a field group.
 *
 * Paramters:
 *     unit - BCM device number
 *     group - Field group ID
 *     status - (OUT) Status structure
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit has not been intialized
 *     BCM_E_NOT_FOUND - Group ID not found on unit.
 *     BCM_E_PARAM     - *status is NULL
 *     BCM_E_NONE      - Success
 */
int 
bcm_robo_field_group_status_get(int unit,
                   bcm_field_group_t group,
                   bcm_field_group_status_t *status)
{
    /* Since Robo chip didn't abled to configure the slice format
     * Show we keep this feature ???
     */
    _field_group_t      *fg;
    _field_control_t       *fc;     /* Field control structure.    */
    int rv;

    if (status == NULL) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);
    
    /* Get group descriptor. */
    rv = _robo_field_group_get (unit, group, &fg);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;
    }

    /* Update the numbers of free entries, meters and counters. */
    rv = _robo_field_group_status_calc(unit, fg);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    *status = fg->group_status;
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

int
bcm_robo_field_group_port_create_mode(int unit, bcm_port_t port,
                                         bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_port_create_mode_id(int unit, bcm_port_t port,
                                        bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_group_ports_create_mode(int unit, bcm_pbmp_t pbmp,
                                      bcm_field_qset_t qset, int pri,
                                      bcm_field_group_mode_t mode,
                                      bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_ports_create_mode_id(int unit, bcm_pbmp_t pbmp,
                                         bcm_field_qset_t qset, int pri,
                                         bcm_field_group_mode_t mode,
                                         bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_group_action_set(int               unit, 
                               bcm_field_group_t group, 
                               bcm_field_aset_t  aset
                               )
{
    int              errcode = BCM_E_NONE;
    _field_control_t *fc;
    _field_group_t   *fg;
    unsigned         action;

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    if (BCM_FAILURE(errcode = _robo_field_group_get(unit, group, &fg))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: group=%d not found \n"),
                   unit, group));
        goto cleanup;
    }
    if (fg->group_status.entry_count != 0) {
        /* Some entries defined in group => error */
        
        errcode = BCM_E_CONFIG;
        goto cleanup;
    }
    
    /* Check all actions are supported */
    for (action = 0; action < bcmFieldActionCount; ++action) {
        if (!SHR_BITGET(aset.w, action)) {
            continue;
        }
        
        if (_field_action_not_supported(unit, fg->stage_id, action)) {
            errcode = BCM_E_UNAVAIL;
            goto cleanup;
        }
    }

    fg->aset = aset;        /* Assign aset to group */


 cleanup:
    FP_UNLOCK(fc);
    return (errcode);
}


int 
bcm_robo_field_group_action_get(int               unit, 
                               bcm_field_group_t group, 
                               bcm_field_aset_t  *aset
                               )
{
    int              errcode;
    _field_control_t *fc;
    _field_group_t   *fg;

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    errcode = _robo_field_group_get(unit, group, &fg);
    if (BCM_FAILURE(errcode)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: group=%d not found \n"),
                   unit, group));
        FP_UNLOCK(fc);
        return (errcode);
    }

    *aset = fg->aset;
    FP_UNLOCK(fc);

    return (BCM_E_NONE);
}



/*
 * Function: bcm_robo_field_control_get
 *
 * Purpose:
 *     Get control status info.
 *
 * Parameters:
 *     unit     - BCM device number
 *     control  - Control element to get
 *     status   - (OUT) Status of field element
 *
 * Returns:
 *     BCM_E_INIT    - BCM unit not initialized
 *     BCM_E_PARAM   - *state pointing to NULL
 *     BCM_E_NONE    - Success
 */
int
bcm_robo_field_control_get(int unit, bcm_field_control_t control, uint32 *state)
{
    _field_control_t    *fc;
	int	rv = BCM_E_NONE;

    if (state == NULL) {
        return BCM_E_PARAM;
    }

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);
    switch (control) {
        case bcmFieldControlStage:
            *state = fc->stage;
            break;
		case bcmFieldControlRedirectIngressVlanCheck:
			rv = (DRV_CFP_CONTROL_GET
            	(unit, DRV_CFP_BYPASS_VLAN_CHECK, 0,state));
			break;
        default:
            FP_UNLOCK(fc);
            return BCM_E_UNAVAIL;
    }
    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function: bcm_robo_field_control_set
 *
 * Purpose:
 *     Set control status.
 *
 * Parameters:
 *     unit     - BCM device number
 *     control  - Control element to set
 *     status   - Status for field module
 *
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_INIT    - BCM unit not initialized
 *     BCM_E_PARAM   - Flag state not valid on device
 */
int
bcm_robo_field_control_set(int unit, bcm_field_control_t control, uint32 state)
{
    _field_control_t    *fc;
	int rv = BCM_E_NONE;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));    

    FP_LOCK(fc);
    switch (control) {
        case bcmFieldControlStage:
                fc->stage = state;
            break;
		case bcmFieldControlRedirectIngressVlanCheck:
			rv = (DRV_CFP_CONTROL_SET
            	(unit, DRV_CFP_BYPASS_VLAN_CHECK, 0,state));
			break;
        default:
            FP_UNLOCK(fc);            
            return BCM_E_UNAVAIL;
    }
    FP_UNLOCK(fc);            
    return rv;
}


/*
 * Function: bcm_robo_field_init
 *
 * Purpose:
 *     Initialize field module.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_UNIT    - Invalid BCM unit number.
 *     BCM_E_UNAVAIL - Field Processor not on device.
 *     BCM_E_MEMORY  - Allocation failure
 *     BCM_E_XXX     - Error code from bcm_XX_field_init()
 *     BCM_E_NONE    - Success
 */
int 
bcm_robo_field_init(int unit)
{
    int                 udf_idx, retval = BCM_E_UNAVAIL;
    _field_control_t    *fc = NULL;
    uint32              udf_num;


    /* Make sure the Unit can support this module. */
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_field)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No Field Processor available Unit=%d\n"),
                   unit));
        return BCM_E_UNAVAIL;
    }

    assert(BCM_FIELD_QUALIFY_MAX        >= bcmFieldQualifyCount);

    /* Detatch first if it's been previously initialized. */
    if (_field_control[unit] != NULL) {
        retval = bcm_robo_field_detach(unit);
        if (BCM_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Module deinit failed.\n"),
                       unit));
            return (retval);
        }
    }

    /* Allocate a bcm_field_control */
    fc = sal_alloc(sizeof (_field_control_t), "field_control");
    if (fc == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Allocation failure for Field Control\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(fc, 0, sizeof (_field_control_t));

    /* Allocate policer lookup hash. */
    _FP_MEM_ALLOC(fc->policer_hash, _FP_HASH_SZ(fc) * \
                   sizeof(_field_policer_t *), "Policer hash");
    if (NULL == fc->policer_hash) {
        _robo_field_control_free(unit, fc);
        return (BCM_E_MEMORY);
    }

    /* Allocate statistics collection lookup hash. */
    _FP_MEM_ALLOC(fc->stat_hash, _FP_HASH_SZ(fc) * \
                   sizeof(_field_stat_t *), "Stat hash");
    if (NULL == fc->stat_hash) {
        _robo_field_control_free(unit, fc);
        return (BCM_E_MEMORY);
    }

    /* Allocate user defined qualifiers. */
    fc->udf = sal_alloc(BCM_FIELD_USER_NUM_UDFS * \
                   sizeof(_field_udf_t), "Udf table");
    if (NULL == fc->udf) {
        _robo_field_control_free(unit, fc);
        return (BCM_E_MEMORY);
    }
    sal_memset(fc->udf, 0, BCM_FIELD_USER_NUM_UDFS *\
        sizeof (_field_udf_t));

    /* Create protection mutex. */
    fc->fc_lock = sal_mutex_create("field_control.lock");
    if (fc->fc_lock == NULL) {
        _robo_field_control_free(unit, fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Allocation failure for Field Control Lock\n")));
        return BCM_E_MEMORY;
    }
    /* Initialize pipeline stages field control structures. */
    retval = _robo_field_stages_init(unit, fc);
    if (BCM_FAILURE(retval)) {
        _robo_field_control_free(unit, fc);
        return retval;
    }

    /* Initialize fp status structure. */
    fc->stage       = bcmFieldStageDefault;

    /* Assign the virtual UDFs to the underlying H/W */
    retval = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num);
    if (BCM_FAILURE(retval)) {
         _robo_field_stages_destroy(unit, fc);
        _robo_field_control_free(unit, fc);
        return retval;
    }

    for (udf_idx = 0; udf_idx < udf_num; udf_idx++) {
        fc->udf[udf_idx].udf_num = udf_idx;
    }

    if ((retval = DRV_FP_INIT(unit, -1)) < 0) {
         _robo_field_stages_destroy(unit, fc);
        _robo_field_control_free(unit, fc);
        return retval;
    } 

    last_allocated_eid[unit] = 0;
    _field_control[unit] = fc;
    assert (_field_control[unit] != NULL);


    fc->tcam_pid = SAL_THREAD_ERROR;
    fc->tcam_notify = NULL;
    fc->tcam_interval = BCM_ROBO_FIELD_TCAM_THREAD_INTERVAL; /* 60 sec */
    /* Create thread for 65nm TCAM protection thread */
    if (soc_feature(unit, soc_feature_field_tcam_parity_check)) {
        fc->tcam_pid = sal_thread_create("TCAM",
                     SAL_THREAD_STKSZ,
                     BCM_ROBO_FIELD_TCAM_THREAD_PRI,
                     _field_tcam_thread,
                     INT_TO_PTR(unit));
        if (fc->tcam_pid == SAL_THREAD_ERROR) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: Thread create failed\n")));
             DRV_FP_DEINIT(unit, -1);
             _robo_field_stages_destroy(unit, fc);
            _robo_field_control_free(unit, fc);
            return (BCM_E_MEMORY);
        }
    }
    robo_prio_set_with_no_free_entries[unit]= FALSE;
    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_qualify_clear
 *
 * Purpose:
 *     Remove all field qualifications from a filter entry
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_INIT       BCM Unit not initialized.
 *     BCM_E_NOT_FOUND  Entry ID not found in unit.
 *     BCM_E_NONE       Success
 */
int 
bcm_robo_field_qualify_clear(int unit,
                bcm_field_entry_t entry)
{
    _field_entry_t      *f_ent;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;
    uint32 temp, slice_map;
    int rv = BCM_E_NONE;
    _field_control_t    *fc;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);
    
    rv = _robo_field_entry_get(unit, entry, &f_ent, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)){
        FP_UNLOCK(fc);
        return rv;        
    }

    /* 
     * The slice ID  should not be reset.
     * Without this value, the slice format is unknown.
     */
    rv = _robo_field_group_get (unit,  f_ent->gid, &fg);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }                       

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, fg->stage_id, 
        DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK, f_ent->drv_entry, NULL, NULL);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }                
    
    /* Get group stage control structure. */
    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Stage (%d) control get failure.\n"), 
                   unit, fg->stage_id));
        return (rv);
    }

    temp =fg->slices[0].sel_codes.fpf;
    slice_map = fg->slices[0].sel_codes.slice_map;
    rv = _robo_field_entry_slice_id_set(unit, stage_fc->stage_id, f_ent, temp, slice_map);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }                    

    f_ent->flags |= _FP_ENTRY_DIRTY;
    FP_UNLOCK(fc);
    return BCM_E_NONE;

}

int
_robo_field_qualify_frame_type_set(int unit, _field_entry_t *f_ent, int frame_type) 
{
    int rv;
    int type;
    if (f_ent->fs->stage_id != _BCM_FIELD_STAGE_INGRESS) {
        return BCM_E_NONE;
    }
        
    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, f_ent->fs->stage_id, f_ent->drv_entry, 
        DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET, -1, &type);
    BCM_IF_ERROR_RETURN(rv);

    type &= _DRV_CFP_FRAME_ALL;

    if (type) {
        if ((type & frame_type) == 0) {
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "frame type %x conflict with the previous setting %x!\n"),
                         frame_type, type));
            return BCM_E_UNAVAIL;
        } else {
            frame_type &= type;
            if (type & _DRV_CFP_FRAME_IPANY){
                frame_type |= _DRV_CFP_FRAME_IPANY;
            }
        }
    } 
    rv = DRV_FP_ENTRY_TCAM_CONTROL(unit, f_ent->fs->stage_id, f_ent->drv_entry, 
        DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET, frame_type, &type);
    BCM_IF_ERROR_RETURN(rv);

    return rv;
}

int 
bcm_robo_field_qualify_DSCP(int unit, bcm_field_entry_t entry,
               uint8 data, uint8 mask)
{
    int                 rv;
    _field_entry_t      *f_ent;
    
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyDSCP, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyDSCP, data, mask);

    return rv;
}

int
bcm_robo_field_qualify_Decap(int unit, bcm_field_entry_t entry,
                            bcm_field_decap_t decap)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_Drop(int unit, bcm_field_entry_t entry,
                                     uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_DstIp(int unit, bcm_field_entry_t entry,
                bcm_ip_t data, bcm_ip_t mask)
{
    int                 rv;
     _field_entry_t      *f_ent;
    
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyDstIp, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyDstIp, data, mask);

    return rv;
}

int 
bcm_robo_field_qualify_DstIp6(int unit, bcm_field_entry_t entry,
                 bcm_ip6_t data, bcm_ip6_t mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyDstIp6, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);
    return rv;
}

int 
bcm_robo_field_qualify_DstIp6High(int unit, bcm_field_entry_t entry,
                 bcm_ip6_t data, bcm_ip6_t mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyDstIp6, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);    
  
    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    sal_memset(data_field, 0, 2 * sizeof(uint32));
    sal_memset(mask_field, 0, 2 * sizeof(uint32));
    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);
    return rv;
}

int 
bcm_robo_field_qualify_DstMac(int unit, bcm_field_entry_t entry,
                 bcm_mac_t data, bcm_mac_t mask)
{
    int                 rv;
    uint32      mac_field[2], mask_field[2];

    sal_memset(mac_field, 0, 2 * sizeof(uint32));
    sal_memset(mask_field, 0, 2 * sizeof(uint32));
    SAL_MAC_ADDR_TO_UINT32(data, mac_field);
    SAL_MAC_ADDR_TO_UINT32(mask, mask_field);

    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifyDstMac,
        mac_field, mask_field);

    return rv;
}

int
bcm_robo_field_qualify_DstPort(int unit, bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstTrunk(int unit, bcm_field_entry_t entry,
                               bcm_trunk_t data, bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_EtherType(int unit, bcm_field_entry_t entry,
                uint16 data, uint16 mask)
{
    int                 rv;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyEtherType, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_NONIP); 
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyEtherType, 
                     data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualifier_delete
 * Purpose:
 *      Remove match criteria from a field processor entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      qual_id - (IN) BCM field qualifier id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualifier_delete(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_qualify_t qual_id)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_InPort(int unit, bcm_field_entry_t entry,
                             bcm_port_t data, bcm_port_t mask)
{
    _field_entry_t      *f_ent;
    int                 rv = SOC_E_UNAVAIL;
    bcm_port_t tmp_port;
    uint32 mask_val, pbmp_vld;
    bcm_pbmp_t pbmp;
    bcm_trunk_t tmp_trunk;
    bcm_module_t tmp_mod;
    int         tmp_id;
    

    if (BCM_GPORT_IS_SET(data)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, data, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
    } else {
        tmp_port = data;
    }

    if (0 == SOC_PORT_VALID(unit, tmp_port)) {
        return (BCM_E_PARAM);
    }

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyInPort, &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    pbmp_vld = 0;
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit)|| SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        pbmp_vld = 1;
    } 
        
    if (SOC_IS_VO(unit)){
        if ( _BCM_FIELD_STAGE_INGRESS ==  f_ent->fs->stage_id) {
            /* VO CFP (_BCM_FIELD_STAGE_INGRESS) support pbmp */
            pbmp_vld = 1;
        }
    }
    if (1 == pbmp_vld) {
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_PORT_ADD(pbmp, tmp_port);
        mask_val = ~(SOC_PBMP_WORD_GET(pbmp, 0));
        tmp_port = 0;
        rv = DRV_FP_QUAL_VALUE_SET(unit, f_ent->fs->stage_id, 
            bcmFieldQualifyInPorts, f_ent->drv_entry, (uint32 *)&tmp_port,&mask_val);
    } else {
        mask_val = mask;
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInPort,
            tmp_port, mask_val);
    }
    return rv;
}

int
bcm_robo_field_qualify_OutPort(int unit, bcm_field_entry_t entry,
                               bcm_port_t data, bcm_port_t mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_UNAVAIL;

    if (0 == SOC_PORT_VALID(unit, data)) {
        return (BCM_E_PARAM);
    }

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyOutPort, &f_ent);
    BCM_IF_ERROR_RETURN(rv);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        rv = _robo_field_qual_value32_set(unit, entry,bcmFieldQualifyOutPort, 
                     data, mask);
    }
#endif

    return rv;
}

int 
bcm_robo_field_qualify_InPorts(int unit, bcm_field_entry_t entry,
                  bcm_pbmp_t data, bcm_pbmp_t mask)
{
    int                 rv;
    uint32 mask_val = 0;
    int port;
    
    FIELD_IS_INIT(unit);

    /* Check if all data ports are valid on this device */
    SOC_PBMP_ITER(data, port) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PARAM;
        }
    }

    /*
     * Check and clear invalid ports in the mask. 
     */ 
    SOC_PBMP_ITER(mask, port) {
        if (!SOC_PORT_VALID(unit, port)) {
            BCM_PBMP_PORT_REMOVE(mask, port);
        }
    }


  
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit)||
        SOC_IS_VO(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        mask_val = ~(SOC_PBMP_WORD_GET(data, 0));
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInPorts, 
                                 0,
                                 mask_val);
    } else {
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInPorts, 
                             SOC_PBMP_WORD_GET(data, 0),
                            SOC_PBMP_WORD_GET(mask, 0));
    }
    return rv;
}

int 
bcm_robo_field_qualify_Ip6FlowLabel(int unit, bcm_field_entry_t entry,
                   uint32 data, uint32 mask)
{
    int                 rv;
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIp6FlowLabel,data, mask);

    return rv;
}

int 
bcm_robo_field_qualify_Ip6HopLimit(int unit, bcm_field_entry_t entry,
                  uint8 data, uint8 mask)
{
    int                 rv;
    uint8    new_data, new_mask;
    uint32      fld_index = 0;
    uint32      temp;
    _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6HopLimit, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    /* Since BCM53115 and BCM53242 only have some values to set. */
    /* The selections are 0, 1, 255 and others */
    if (SOC_IS_VULCAN(unit) || SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        new_mask = 3;
        switch (data) {
            case 0:
                new_data = 0;
                break;
            case 1:
                new_data = 1;
                break;
            case 255:
                new_data = 3;
                break;
            default:
                new_data = 2;
        }
        rv = _robo_field_qual_value32_set
                (unit, entry,bcmFieldQualifyIp6HopLimit, 
                 new_data, new_mask);
    } else {
        rv = _robo_field_qual_value32_set
                (unit, entry,bcmFieldQualifyIp6HopLimit, 
                 data, mask);
    }
    if (rv < 0) {
        return rv;
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * Since the Ip6HopLimit qualifier id is equal to 
         * Ttl, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6HopLimit.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        temp = FP_BCM53242_L3_FRM_FMT_IP6;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp));
        temp = FP_BCM53242_L3_FRM_FMT_MASK;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp));
    }


    return rv;
}

int 
bcm_robo_field_qualify_Ip6NextHeader(int unit, bcm_field_entry_t entry,
                    uint8 data, uint8 mask)
{
    int                 rv;
    uint32      fld_index = 0;
    uint32      temp;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6NextHeader, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_qual_value32_set
            (unit, entry, bcmFieldQualifyIp6NextHeader, 
             data, mask);
    if (rv < 0) {
        return rv;
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * Since the Ip6NextHeader qualifier id is equal to 
         * IpProtocol, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6NextHeader.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        temp = FP_BCM53242_L3_FRM_FMT_IP6;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp));
        temp = FP_BCM53242_L3_FRM_FMT_MASK;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp));
    }


    return rv;
}

int 
bcm_robo_field_qualify_Ip6TrafficClass(int unit, bcm_field_entry_t entry,
                      uint8 data, uint8 mask)
{
    int                 rv;
    uint32      fld_index = 0;
    uint32      temp;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6TrafficClass, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_qual_value32_set
            (unit, entry, bcmFieldQualifyIp6TrafficClass, 
             data, mask);
    if (rv < 0) {
        return rv;
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * Since the Ip6TrafficClass qualifier id is equal to 
         * DSCP, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6TrafficClass.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        temp = FP_BCM53242_L3_FRM_FMT_IP6;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp));
    
        temp = FP_BCM53242_L3_FRM_FMT_MASK;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp));
    }

    return rv;
}

int 
bcm_robo_field_qualify_IpFlags(int unit, bcm_field_entry_t entry,
                  uint8 data, uint8 mask)
{
    int                 rv;
    const uint8      data_max = BCM_FIELD_IPFLAGS_MASK & ~BCM_FIELD_IPFLAGS_RF;    
    uint8 new_data=0, new_mask=0;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpFlags, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);
 
    /* Reserved flag bit (RFC791) is unused. */
    data &= ~BCM_FIELD_IPFLAGS_RF;
    mask &= ~BCM_FIELD_IPFLAGS_RF;

    /* Range check data and mask values. */
    if (data > data_max) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: IpFlags data=%#x out of range (0-%d)."),
                   unit, data, data_max));
        return (BCM_E_PARAM);
    }

    if (data  == BCM_FIELD_IPFLAGS_MF) {
        new_data =  1;
        new_mask = 1;        
    } else if (data == BCM_FIELD_IPFLAGS_DF) {
        new_data = 0;
        new_mask = 1;        
    } else {
        new_mask = 0;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpFlags, 
                               new_data, new_mask);

    return rv;
}


int 
bcm_robo_field_qualify_IpProtocol(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    int                 rv;
    _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpProtocol, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpProtocol, 
                               data, mask);
    return rv;
}

int
bcm_robo_field_qualify_ForwardingType(int unit, bcm_field_entry_t entry,
                             bcm_field_ForwardingType_t data)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_IpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t data)
{    
    int                 rv;
    uint32              new_data = 0, new_mask= 0;
    _field_entry_t      *f_ent;
     

    if (data >= bcmFieldIpTypeCount) {
        return (BCM_E_PARAM);
    }
#ifdef BROADCOM_DEBUG
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: %s IpType %s"),
               FUNCTION_NAME(), 
               _robo_field_qual_IpType_name(data)));
#endif
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpType, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        new_mask = FP_BCM53242_L3_FRM_FMT_MASK;
        switch(data) {
            case bcmFieldIpTypeIpv4Any:
                new_data = FP_BCM53242_L3_FRM_FMT_IP4;
                break;
            case bcmFieldIpTypeIpv6:
                new_data = FP_BCM53242_L3_FRM_FMT_IP6;
                break;
            case bcmFieldIpTypeNonIp:
                new_data = FP_BCM53242_L3_FRM_FMT_OTHERS;
                break;
            case bcmFieldIpTypeAny:
                /* don't care*/
                new_mask = 0;
                break;
            default:
                return BCM_E_UNAVAIL;
        }

    } else if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        new_mask = FP_BCM53115_L3_FRM_FMT_MASK;
        switch (data) {
            case bcmFieldIpTypeIpv4Any:
                new_data = FP_BCM53115_L3_FRM_FMT_IP4;
                break;
            case bcmFieldIpTypeIpv6:
                new_data = FP_BCM53115_L3_FRM_FMT_IP6;
                break;
            case bcmFieldIpTypeNonIp:
                new_data = FP_BCM53115_L3_FRM_FMT_NON_IP;
                break;
            case bcmFieldIpTypeAny:
                /* don't care*/
                new_mask = 0;
                break;
            default:
                return BCM_E_UNAVAIL;
        }

    } else if (SOC_IS_TBX(unit)){
        new_mask = FP_TB_L3_FRM_FMT_MASK;
        switch (data) {
            case bcmFieldIpTypeIpv4Any:
                new_data = FP_TB_L3_FRM_FMT_IP4;
                rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
                BCM_IF_ERROR_RETURN(rv);
                break;
            case bcmFieldIpTypeIpv6:
                new_data = FP_TB_L3_FRM_FMT_IP6;
                rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
                BCM_IF_ERROR_RETURN(rv);
                break;
            case bcmFieldIpTypeNonIp:
                new_data = FP_TB_L3_FRM_FMT_NON_IP;
                rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_NONIP); 
                BCM_IF_ERROR_RETURN(rv);
                break;
            case bcmFieldIpTypeAny:
                new_data = 0;
                new_mask = 0;
                rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_ANY); 
                BCM_IF_ERROR_RETURN(rv);
                break;
            case bcmFieldIpTypeIp:
                new_data = 0;
                new_mask = 0x2;
                rv = _robo_field_qualify_frame_type_set(unit, f_ent, 
                    _DRV_CFP_FRAME_IPANY|_DRV_CFP_FRAME_IP); 
                BCM_IF_ERROR_RETURN(rv);
                break;
            default:
                return BCM_E_UNAVAIL;
        }

    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpType, 
                               new_data, new_mask);

    return rv;
}


int 
bcm_robo_field_qualify_L4DstPort(int unit, bcm_field_entry_t entry,
                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    _field_entry_t      *f_ent;
    int                 rv;

    FIELD_IS_INIT(unit);

    f_ent = _field_entry_qual_find(unit, entry, bcmFieldQualifyL4DstPort);
    if (f_ent == NULL) {
        return BCM_E_NOT_FOUND;
    }

    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyL4DstPort, 
                                 data, mask);

    return rv;
}

int 
bcm_robo_field_qualify_L4SrcPort(int unit, bcm_field_entry_t entry,
                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    _field_entry_t      *f_ent;
    int                 rv;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyL4SrcPort, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);


    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyL4SrcPort, 
                                  data, mask);

    return rv;
}

int 
bcm_robo_field_qualify_MHOpcode(int unit, bcm_field_entry_t entry,
                   uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanId
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    int                 rv;

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyOuterVlanId, 
                             data, mask);

    return rv;
}


/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanPri
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    int                 rv;

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyOuterVlanPri, 
                             data, mask);

    return rv;
}


/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    int                 rv;
 
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyOuterVlanCfi, 
                             data, mask);
    return rv;
}



int 
bcm_robo_field_qualify_OuterVlan(int unit, bcm_field_entry_t entry,
                  bcm_vlan_t data, bcm_vlan_t mask)
{
    int rv; /* Operation return status. */

    rv = bcm_robo_field_qualify_OuterVlanId(unit, entry, 
                                           (data & 0xfff), 
                                           (mask & 0xfff));
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_robo_field_qualify_OuterVlanCfi(unit, entry, 
                                            ((data >> 12) & 0x1),
                                            ((mask >> 12) & 0x1));
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_robo_field_qualify_OuterVlanPri(unit, entry,
                                            (data >> 13),
                                            (mask >> 13));
    return (rv);
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanId
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    int                 rv;
  
    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInnerVlanId, 
                             data, mask);

    return rv;
}


/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanPri
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    int                 rv;

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInnerVlanPri, 
                             data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    int                 rv;

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInnerVlanCfi, 
                             data, mask);

    return rv;
}


int 
bcm_robo_field_qualify_InnerVlan(int unit, bcm_field_entry_t entry,
                 bcm_vlan_t data, bcm_vlan_t mask)
{
    int rv; /* Operation return status. */

    rv = bcm_robo_field_qualify_InnerVlanId(unit, entry, 
                                           (data & 0xfff), 
                                           (mask & 0xfff));
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_robo_field_qualify_InnerVlanCfi(unit, entry, 
                                            ((data >> 12) & 0x1),
                                            ((mask >> 12) & 0x1));
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_robo_field_qualify_InnerVlanPri(unit, entry,
                                            (data >> 13),
                                            (mask >> 13));
    return (rv);
}


/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return _robo_field_qual_value16_get(unit, entry, bcmFieldQualifyOuterVlanId, 
                             data, mask);
}


/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyOuterVlanPri, 
                             data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyOuterVlanCfi, 
                             data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
  return _robo_field_qual_value16_get(unit, entry, bcmFieldQualifyInnerVlanId, 
                             data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyInnerVlanPri, 
                             data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyInnerVlanCfi, 
                             data, mask);
}

int 
bcm_robo_field_qualify_L2Format(int unit, bcm_field_entry_t entry,
                                bcm_field_L2Format_t type)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32              data = 0, mask = 0;
#ifdef BCM_TB_SUPPORT
    _field_stage_id_t stage_id = 0;
#endif /* BCM_TB_SUPPORT */
    
    if (type >= bcmFieldL2FormatCount) {
        return (BCM_E_PARAM);
    }
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyL2Format, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);   

#ifdef BCM_TB_SUPPORT
    /* Get current stage */
    stage_id = f_ent->fs->stage_id;
#endif /* BCM_TB_SUPPORT */

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        switch (type) {
          case bcmFieldL2FormatAny:
              data = 0x0;
              mask = 0x0;
              break; 
          case bcmFieldL2FormatEthII:
              data = 0x0;
              mask = 0x3;
              break;
          case bcmFieldL2FormatSnap:
              data = 0x1;
              mask = 0x1; 
              break;
          case bcmFieldL2FormatLlc:
          case bcmFieldL2Format802dot3:
              data = 0x2;
              mask = 0x3;
              break;
          case bcmFieldL2FormatPPPoE:
              if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                  /* For Northstarplus, use bit 2 to indicate the PPPoE session packet */
                  data = 0x4;
                  mask = 0x4;
              } else {
                  return (BCM_E_UNAVAIL);
              }
              break;
          default:
              LOG_ERROR(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "FP Error: %d not supported on unit=%d\n"),
                         type, unit));
              return (BCM_E_UNAVAIL);
              break;
        }
        rv = _robo_field_qual_value32_set
            (unit, entry, bcmFieldQualifyL2Format, data, mask);
    } else if (SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit)) {
        switch (type) {
          case bcmFieldL2FormatAny:
              data = 0x0;
              mask = 0x0;
              break; 
          case bcmFieldL2FormatEthII:
              data = 0x1;
              mask = 0x3;
              break;
          case bcmFieldL2FormatSnap:
              data = 0x2;
              mask = 0x3; 
              break;
          default:
              data = 0x0;
              mask = 0x3; 
              break;
        }
        rv = _robo_field_qual_value32_set
            (unit, entry, bcmFieldQualifyL2Format, data, mask);
#ifdef BCM_TB_SUPPORT
    } else if (SOC_IS_TBX(unit)) {
        switch(stage_id) {
            case _BCM_FIELD_STAGE_INGRESS:
            case _BCM_FIELD_STAGE_LOOKUP:
                /* IVM */
                mask = 0x3;
                switch (type) {
                    case bcmFieldL2FormatEthII:
                        data = 0x0;
                        break;
                    case bcmFieldL2FormatSnap:
                        data = 0x1;
                        break;
                    case bcmFieldL2Format802dot3:
                    case bcmFieldL2FormatLlc:
                        data = 0x2;
                        mask = 0x2;
                        rv = _robo_field_qualify_frame_type_set(unit, f_ent, 
                            _DRV_CFP_FRAME_NONIP); 
                        BCM_IF_ERROR_RETURN(rv);
                        break;
                    case bcmFieldL2FormatSnapPrivate:
                        data = 0x3;
                        rv = _robo_field_qualify_frame_type_set(unit, f_ent, 
                            _DRV_CFP_FRAME_NONIP); 
                        BCM_IF_ERROR_RETURN(rv);
                        break;
                    case bcmFieldL2FormatAny:
                        mask = 0x0;
                        break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_FP,
                                  (BSL_META_U(unit,
                                              "FP Error: %d not supported on unit=%d\n"),
                                   type, unit));
                        return (BCM_E_UNAVAIL);
                      break;
                }
                rv = _robo_field_qual_value32_set
                    (unit, entry, bcmFieldQualifyL2Format, data, mask);
                break;
            case _BCM_FIELD_STAGE_EGRESS:
            default:
                return BCM_E_PARAM;
                break;
        }
#endif
    }

    return rv;
}

int 
bcm_robo_field_qualify_VlanFormat(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32 new_data, new_mask;
    int    dtag_mode = BCM_PORT_DTAG_MODE_NONE;
#ifdef BCM_TB_SUPPORT
    _field_stage_id_t stage_id = 0;
    uint32 temp = 0;
#endif

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyVlanFormat, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);   
#ifdef BCM_TB_SUPPORT
    /* Get current stage */
    stage_id = f_ent->fs->stage_id;

    if (SOC_IS_TBX(unit)) {
        if (stage_id == _BCM_FIELD_STAGE_EGRESS) {
            return BCM_E_PARAM;
        }
        /* Configure STag status. */
        temp = data & (BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED | 
                                BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO);
        switch(temp) {
            case 0:
                /* STag is untagged. */
                new_data = 0x0;
                new_mask = 0x3;
                break;
            case BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED:
                /* STag is tagged. */
                new_data = 0x3;
                new_mask = 0x3;
                break;
            case BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO:
                /* STag is pri-tagged. */
                new_data = 0x1;
                new_mask = 0x3;
                break;
            default:
                /* 
                  * STag can be tagged or pri-tagged. 
                  */
                new_data = 0x3;
                new_mask = 0x1;
                break;
        }

        if (stage_id == _BCM_FIELD_STAGE_INGRESS) {
            rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
                                             DRV_CFP_FIELD_SPTAGGED, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
                                             DRV_CFP_FIELD_SPTAGGED,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        } else if (stage_id == _BCM_FIELD_STAGE_LOOKUP) {
            rv = DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_DATA, 
                                                 DRV_VM_FIELD_IVM_INGRESS_STAG_STAUS, 
                                                 f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            rv = DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_MASK, 
                                                 DRV_VM_FIELD_IVM_INGRESS_STAG_STAUS, 
                                                 f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }

        /* Configure CTag status. */
        temp = data & (BCM_FIELD_VLAN_FORMAT_INNER_TAGGED | 
                                BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO);
        switch(temp) {
            case 0:
                /* CTag is untagged. */
                new_data = 0x0;
                new_mask = 0x3;
                break;
            case BCM_FIELD_VLAN_FORMAT_INNER_TAGGED:
                /* CTag is tagged. */
                new_data = 0x3;
                new_mask = 0x3;
                break;
            case BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO:
                /* CTag is pri-tagged. */
                new_data = 0x1;
                new_mask = 0x3;
                break;
            default:
                /* 
                  * CTag can be tagged or pri-tagged. 
                  */
                new_data = 0x3;
                new_mask = 0x1;
                break;
        }

        if (stage_id == _BCM_FIELD_STAGE_INGRESS) {
            rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
                                             DRV_CFP_FIELD_1QTAGGED, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
                                             DRV_CFP_FIELD_1QTAGGED,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        } else if (stage_id == _BCM_FIELD_STAGE_LOOKUP) {

            rv = DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_DATA, 
                                             DRV_VM_FIELD_IVM_INGRESS_CTAG_STATUS, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_MASK, 
                                             DRV_VM_FIELD_IVM_INGRESS_CTAG_STATUS,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }

        return rv;
    }
#endif
    
    rv = bcm_port_dtag_mode_get(unit, 0, &dtag_mode);
    if (rv < 0) {
        return rv;
    }

    if (dtag_mode ==  BCM_PORT_DTAG_MODE_NONE) {
        /* Check 1q VLAN tag */
        if (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED) {
            new_data = (data & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED);
            new_mask = (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED);
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                if (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO) {
                     new_mask |= 0x2;
                     if (!(data & 
                        BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO)) {
                        new_data |= 0x2;
                     }
                }
            }
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_1QTAGGED, 
                    f_ent->drv_entry, &new_data));
            
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_1QTAGGED, 
                    f_ent->drv_entry, &new_mask));
        }
    } else {
        if (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED) {
            new_data = (data & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED);
            new_mask = (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED);
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                if (mask & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO) {
                     new_mask |= 0x2;
                     if (!(data & 
                        BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO)) {
                        new_data |= 0x2;
                     }
                }
            }
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SPTAGGED, 
                    f_ent->drv_entry, &new_data));
            
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SPTAGGED, 
                    f_ent->drv_entry, &new_mask));
        }
        if (mask & BCM_FIELD_VLAN_FORMAT_INNER_TAGGED) {
            new_data = (data & BCM_FIELD_VLAN_FORMAT_INNER_TAGGED) >> 1;
            new_mask = (mask & BCM_FIELD_VLAN_FORMAT_INNER_TAGGED) >> 1;
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                if (mask & BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO) {
                     new_mask |= 0x2;
                     if (!(data & 
                        BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO)) {
                        new_data |= 0x2;
                     }
                }
            }
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_1QTAGGED, 
                    f_ent->drv_entry, &new_data));
            
            BCM_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_1QTAGGED, 
                    f_ent->drv_entry, &new_mask));
        }
    }

    return rv;
}

int 
bcm_robo_field_qualify_RangeCheck(int unit, bcm_field_entry_t entry,
                 bcm_field_range_t range, int invert)
{
    _field_entry_t      *f_ent;
    int                 rv;
    _field_control_t *fc;
    _field_range_t      *fr;
    _field_stage_t         *stage_fc;
    int                 hw_index;
    uint32  fld_data = 0, fld_mask = 0;
    uint32 flags = 0, range_type = 0;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyRangeCheck, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    FP_LOCK(fc);

    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, f_ent->group->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    hw_index = _FP_INVALID_INDEX;
     /* Find range hw entry index. */
    for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
        if (fr->rid == range) {
            hw_index = fr->hw_index;
            flags = fr->flags;
            break;
        }
    }
    if (hw_index < 0) {
        FP_UNLOCK(fc);
        return (BCM_E_NOT_FOUND);
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * Check the hw_index of this range is 4 or not.
         * If the hw_index is 4 then set the L4SRC_LESS1024 qualifier,
         * otherwise configure the range checker field.
         */

        if (hw_index > 9) {
            FP_UNLOCK(fc);
            return BCM_E_INTERNAL;
        }

        if (hw_index == 9) {
            if (invert < 0) {
                fld_mask = 0x0;
            } else {
                fld_mask = 0x1;
                if (invert) {
                    fld_data = 0x0;
                } else {
                    fld_data = 0x1;
                }
            }
            
            rv = _robo_field_qual_value32_set(unit, entry,
                bcmFieldQualifyRangeCheck, fld_data, fld_mask);
        } else {
            rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM, range_type, 
                f_ent->drv_entry, &fld_data);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }    
            rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM_MASK, range_type, 
                f_ent->drv_entry, &fld_mask);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }

            if (flags & BCM_FIELD_RANGE_OUTER_VLAN) {
                if (hw_index < 4) {
                    FP_UNLOCK(fc);
                    return (BCM_E_NOT_FOUND);
                }
                hw_index -= 4;
                range_type = DRV_CFP_FIELD_VLAN_RANGE;
            } else {
                range_type = DRV_CFP_FIELD_L4_PORT_RANGE;
            }

            if (invert < 0) {
                fld_mask &= ~(1 << hw_index);

            } else {
                fld_mask |= (1 << hw_index);
        
                if (invert) {
                    fld_data &= ~(1 << hw_index);
                } else {
                    fld_data |= (1 << hw_index);
                }
            }

            rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, range_type, 
                f_ent->drv_entry, &fld_data);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }    
            rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, range_type, 
                f_ent->drv_entry, &fld_mask);
        }
                
    } else if (SOC_IS_TBX(unit)){
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyRangeCheck, 
            &fld_data, &fld_mask);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
        if (invert < 0) {
            fld_mask &= ~(1 << hw_index);
        } else {
            fld_mask |= (1 << hw_index);
            if (invert) {
                fld_data &= ~(1 << hw_index);
            } else {
                fld_data |= (1 << hw_index);
            }
        }
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyRangeCheck, 
                                 fld_data, fld_mask);
    } else {
        if (invert < 0) {
            fld_mask = 0x0;
        } else {
            fld_mask = 0x1;
    
            if (invert) {
                fld_data = 0x0;
            } else {
                fld_data = 0x1;
            }
        }

        /* BCM5348/BCM5347 */
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyRangeCheck, 
                                 fld_data, fld_mask);
    }

    FP_UNLOCK(fc);
    return rv;
}

int 
bcm_robo_field_qualify_SrcIp(int unit, bcm_field_entry_t entry,
                bcm_ip_t data, bcm_ip_t mask)
{
    int rv;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifySrcIp, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);
    return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifySrcIp, 
                        data, mask);
}

int 
bcm_robo_field_qualify_SrcIp6(int unit, bcm_field_entry_t entry,
                 bcm_ip6_t data, bcm_ip6_t mask)
{  
    int                 rv;
    uint32      data_field[4], mask_field[4];
    int i;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifySrcIp6, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        if (data[11] & 0xf0){
            LOG_WARN(BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "FP WARN: 53242/53262 only support lower 44bit qualifiy_SrcIp6 . "
                                 "Only lower 44 bit will take effect !\n")));
        } else {
            for  (i = 0; i < 10; i++) {
                if (data[i] != 0) {
                    LOG_WARN(BSL_LS_BCM_FP,
                             (BSL_META_U(unit,
                                         "FP WARN: 53242/53262 only support lower 44bit qualifiy_SrcIp6. "
                                         "Only lower 44 bit will take effect !\n")));
                    break;
                }
            }
       }
    }

    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifySrcIp6,
        data_field, mask_field);

    return rv;
}

int
bcm_robo_field_qualify_SrcIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
    uint32      old_data_field[4], old_mask_field[4];
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifySrcIp6, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);
    
  
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "FP WARN: 53242/53262 only support lower-44 bit qualifiy_SrcIp6")));
        return BCM_E_UNAVAIL;
    }
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp6, 
        (uint32 *)&old_data_field, (uint32 *)&old_mask_field);

    BCM_IF_ERROR_RETURN(rv);
    
    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    sal_memcpy(&data_field, &old_data_field, 2 * sizeof(uint32));
    sal_memcpy(&mask_field, &old_mask_field, 2 * sizeof(uint32));

    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifySrcIp6, 
        data_field, mask_field);
    return rv;
}


int 
bcm_robo_field_qualify_SrcMac(int unit, bcm_field_entry_t entry,
                 bcm_mac_t data, bcm_mac_t mask)
{
    int                 rv;
    uint32  mac_field[2], mask_field[2];

    sal_memset(mac_field, 0, 2 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 2 * sizeof(uint32)); 
    SAL_MAC_ADDR_TO_UINT32(data, mac_field);
    SAL_MAC_ADDR_TO_UINT32(mask, mask_field);
    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifySrcMac,
        mac_field, mask_field);

    return rv;
}

int
bcm_robo_field_qualify_InterfaceClassL2(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InterfaceClassL3(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InterfaceClassPort(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
#ifdef BCM_TB_SUPPORT

    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInterfaceClassPort, 
                                     data, mask);
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_ForwardingVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyForwardingVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ForwardingVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Vpn_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyVpn
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Vpn_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t *data, 
    bcm_vpn_t *mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_robo_field_qualify_ForwardingVlanId
 * Purpose:
 *      Set match criteria for bcmFieildQualifyForwardingVlanId
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ForwardingVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_robo_field_qualify_Vpn
 * Purpose:
 *      Set match criteria for bcmFieildQualifyVpn
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Vpn(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t data, 
    bcm_vpn_t mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_SrcIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];  
    uint32      old_data_field[4], old_mask_field[4];  
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifySrcIp6, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP6); 
    BCM_IF_ERROR_RETURN(rv);

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        if (data[11] & 0xf0){
            LOG_WARN(BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "FP WARN: 53242/53262 only support lower 44bit qualifiy_SrcIp6. "
                                 "Only lower 44 bit will take effect !\n")));
        }
    }
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp6, 
        (uint32 *)&old_data_field, (uint32 *)&old_mask_field);

    BCM_IF_ERROR_RETURN(rv);

    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    sal_memcpy(&data_field[2], &old_data_field[2], 2 * sizeof(uint32));
    sal_memcpy(&mask_field[2], &old_mask_field[2], 2 * sizeof(uint32));
    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifySrcIp6, 
        data_field, mask_field);
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_DstIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
    uint32      old_data_field[4], old_mask_field[4];
  
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifyDstIp6, 
        (uint32 *)&old_data_field, (uint32 *)&old_mask_field);

    BCM_IF_ERROR_RETURN(rv);

    SAL_IP6_ADDR_TO_UINT32(data, data_field);
    SAL_IP6_ADDR_TO_UINT32(mask, mask_field);

    sal_memcpy(&data_field[2], &old_data_field[2], 2 * sizeof(uint32));
    sal_memcpy(&mask_field[2], &old_mask_field[2], 2 * sizeof(uint32));
    rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
  
    sal_memset(data_field, 0, 4 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 4 * sizeof(uint32)); 
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp6, 
        data_field, mask_field);

    sal_memset(&data_field[2], 0, 2 * sizeof(uint32)); 
    sal_memset(&mask_field[2], 0, 2 * sizeof(uint32)); 

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
  
    sal_memset(data_field, 0, 4 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 4 * sizeof(uint32)); 
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);

    sal_memset(&data_field[2], 0, 2 * sizeof(uint32)); 
    sal_memset(&mask_field[2], 0, 2 * sizeof(uint32)); 

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);
    return rv;
}
int
bcm_robo_field_qualify_SrcClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcPort(int unit, bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifySrcPort,
        data_port, mask_port);
}

int
bcm_robo_field_qualify_SrcTrunk(int unit, bcm_field_entry_t entry,
                               bcm_trunk_t data, bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_TcpControl(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyTcpControl, 
                                 data, mask);
}

int 
bcm_robo_field_qualify_Ttl(int unit, bcm_field_entry_t entry,
              uint8 data, uint8 mask)
{
    int                 rv = BCM_E_NONE;
    uint8 new_data, new_mask;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyTtl, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);
    

    /* Since BCM53115 and BCM53242 only have some values to set. */
    /* The selections are 0, 1, 255 and others */
    if (SOC_IS_VULCAN(unit) || SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        new_mask = 3;
        switch (data) {
            case 0:
                new_data = 0;
                break;
            case 1:
                new_data = 1;
                break;
            case 255:
                new_data = 3;
                break;
            default:
                new_data = 2;
        }
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyTtl, 
            new_data, new_mask);
    } else {
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyTtl, 
            data, mask);
    }

    return rv;
}

int
bcm_robo_field_qualify_IpInfo(int unit, bcm_field_entry_t entry,
                 uint32 data, uint32 mask)
{
    _field_entry_t      *f_ent;
    _field_group_t      *fg = NULL;	
    int                 rv;
    const uint32        data_max = BCM_FIELD_IP_HDR_OFFSET_ZERO |
                                   BCM_FIELD_IP_HDR_FLAGS_MF;
    uint32 temp;
    uint32 frag_data, frag_mask;
    uint32 nonfirst_data, nonfirst_mask;
    
    /* Range check data and mask values. */
    if (data > data_max || mask > data_max) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: IpInfo data=%#x or mask=%#x out of range (0-%d).\n"
                              "BCM_FIELD_IP_CHECKSUM_OK is not supported in robo\n"),
                   unit, data, mask, data_max));
        return (BCM_E_PARAM);
    }
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpInfo, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    frag_data = 0;
    frag_mask = 0;
    temp = ((data << 2) & 0xc) | (mask & 0x3);


    /* frag_mask==0 or nonfirst_mask==0 means we don't care the data value
      * but for the easy implementation in the bcm_robo_field_qualify_IpInfo_get
      * we desing the 1-to-1 mapping table as following
      */
    /*  data    mask
     * MF,OZ   MF,OZ       Frag    Non_First 
     *  0,0     0,0   0     0,0     0,0     0
     *  0,0     0,1   1     0,0     1,1     3
     *  0,0     1,0   2     0,1     0,0     4
     *  0,0     1,1   3     0,1     1,1     7
     *  0,1     0,0   4     0,0     1,0     2
     *  0,1     0,1   5     0,0     0,1     1
     *  0,1     1,0   6     0,1     1,0     6
     *  0,1     1,1   7     0,1     0,1     5
     *  1,0     0,0   8     1,0     0,0     8
     *  1,0     0,1   9     1,0     1,1     11
     *  1,0     1,0  10     1,1     0,0     12
     *  1,0     1,1  11     1,1     1,1     15
     *  1,1     0,0  12     1,0     1,0     10
     *  1,1     0,1  13     1,0     0,1     9
     *  1,1     1,0  14     1,1     1,0     14
     *  1,1     1,1  15     1,1     0,1     13
     */
    switch (temp) {
        case (0):
            frag_data = 0;
            frag_mask = 0;                
            nonfirst_data = 0;
            nonfirst_mask = 0;
            break;
        case (1):
            frag_data = 0;
            frag_mask = 0;                
            nonfirst_data = 1;
            nonfirst_mask = 1;
            break;
        case (2):
            frag_data = 0;
            frag_mask = 1;                
            nonfirst_data = 0;
            nonfirst_mask = 0;
            break;
        case (3):
            frag_data = 0;
            frag_mask = 1;                
            nonfirst_data = 1;
            nonfirst_mask = 1;
            break;
        case (4):
            frag_data = 0;
            frag_mask = 0;                
            nonfirst_data = 1;
            nonfirst_mask = 0;
            break;
        case (5):
            frag_data = 0;
            frag_mask = 0;                
            nonfirst_data = 0;
            nonfirst_mask = 1;
            break;
        case (6):
            frag_data = 0;
            frag_mask = 1;                
            nonfirst_data = 1;
            nonfirst_mask = 0;
            break;
        case (7):
            frag_data = 0;
            frag_mask = 1;                
            nonfirst_data = 0;
            nonfirst_mask = 1;
            break;
        case (8):
            frag_data = 1;
            frag_mask = 0;                
            nonfirst_data = 0;
            nonfirst_mask = 0;
            break;
        case (9):
            frag_data = 1;
            frag_mask = 0;                
            nonfirst_data = 1;
            nonfirst_mask = 1;
            break;
        case (10):
            frag_data = 1;
            frag_mask = 1;                
            nonfirst_data = 0;
            nonfirst_mask = 0;
            break;
        case (11):
            frag_data = 1;
            frag_mask = 1;                
            nonfirst_data = 1;
            nonfirst_mask = 1;
            break;
        case (12):
            frag_data = 1;
            frag_mask = 0;                
            nonfirst_data = 1;
            nonfirst_mask = 0;
            break;
        case (13):
            frag_data = 1;
            frag_mask = 0;                
            nonfirst_data = 0;
            nonfirst_mask = 1;
            break;
        case (14):
            frag_data = 1;
            frag_mask = 1;                
            nonfirst_data = 1;
            nonfirst_mask = 0;
            break;
        case (15):
            frag_data = 1;
            frag_mask = 1;                
            nonfirst_data = 0;
            nonfirst_mask = 1;
            break;
        default:
            break;            
    }

    fg = f_ent->group;
    BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyIpFlags);	

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpFlags, 
                               frag_data, frag_mask);    
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = DRV_CFP_FIELD_SET        
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &nonfirst_data);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = DRV_CFP_FIELD_SET        
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &nonfirst_mask);    

    return (rv);

}
int
bcm_robo_field_qualify_PacketRes(int unit, bcm_field_entry_t entry,
                uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_IpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    uint32 data, mask;
    int    rv;
    _field_entry_t      *f_ent;
#ifdef BCM_TB_SUPPORT
    uint32 frame_data, frame_mask;    
#endif


    if (protocol >= bcmFieldIpProtocolCommonCount) {
        return (BCM_E_PARAM);
    }

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpProtocolCommon, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    data = 0;
    mask = 0;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        mask = 0xff;
        switch (protocol) {
          case bcmFieldIpProtocolCommonTcp:
              data = 6;
              break;
          case bcmFieldIpProtocolCommonUdp:
              data = 17;
              break;
          case bcmFieldIpProtocolCommonIgmp:
              data = 2;
              break;
          case bcmFieldIpProtocolCommonIcmp:
              data = 1;
              break;
          case bcmFieldIpProtocolCommonIp6Icmp:
              data = 58;
              break;
          case bcmFieldIpProtocolCommonIp6HopByHop:
              data = 0;
              break;
          case bcmFieldIpProtocolCommonIpInIp:
              data = 4;
              break;
          case bcmFieldIpProtocolCommonTcpUdp:
          default:
              return (BCM_E_UNAVAIL); 
        }
        /* L3 framing make sure this won't be non-ip format, don't care ipv4 or ipv6*/
        frame_data = 0;
        frame_mask =  0x2;
        rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_L3_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_data);
    
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_L3_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_mask);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
#endif

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        
        switch (protocol) {
            case bcmFieldIpProtocolCommonTcp:
                data = FP_BCM53242_L4_FRM_FMT_TCP;
                break;
            case bcmFieldIpProtocolCommonUdp:
                data = FP_BCM53242_L4_FRM_FMT_UDP;
                break;
            case bcmFieldIpProtocolCommonIgmp:
            case bcmFieldIpProtocolCommonIcmp:
                data = FP_BCM53242_L4_FRM_FMT_ICMPIGMP;
                break;
            case bcmFieldIpProtocolCommonIp6Icmp:
            case bcmFieldIpProtocolCommonTcpUdp:
                return BCM_E_UNAVAIL;
                break;
            default:
                data = FP_BCM53242_L4_FRM_FMT_OTHERS;
                break;
        }
        mask = FP_BCM53242_L4_FRM_FMT_MASK;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpProtocolCommon, 
                               data, mask);
    
    return rv;    
}

int
bcm_robo_field_qualify_Snap(int unit, bcm_field_entry_t entry,
                            bcm_field_snap_header_t data, 
                            bcm_field_snap_header_t mask)
{
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int     rv;
        uint32              ref_data[2], ref_mask[2];

        ref_data[1] = (data.org_code & 0x00ff0000) >> 16;
        ref_data[0] = (data.org_code & 0x0000ffff) << 16 | data.type;
 
        ref_mask[1] = (mask.org_code & 0x00ff0000) >> 16;
        ref_mask[0] = (mask.org_code & 0x0000ffff) << 16 | mask.type;

        rv = _robo_field_qual_value_set(unit, entry, bcmFieldQualifySnap, 
            ref_data, ref_mask);

        return rv;
    }  
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_Llc(int unit, bcm_field_entry_t entry,
                           bcm_field_llc_header_t data, 
                           bcm_field_llc_header_t mask)
{
    _field_entry_t      *f_ent;
    int     rv = BCM_E_NONE;
    uint16 ref_data, ref_mask;
    uint32 frame_data, frame_mask;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyLlc, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, 
        _DRV_CFP_FRAME_NONIP); 
    BCM_IF_ERROR_RETURN(rv);    

    if (SOC_IS_TBX(unit)) {
        _field_group_t	*fg = NULL;

        if (mask.control != 0){
            /* TBx could only support dsap and ssap */
            return BCM_E_PARAM;
        }

        /* L2 framing should be llc, L3 framing should be non-ip */
        rv = _robo_field_qualify_frame_type_set(unit, f_ent, 
            _DRV_CFP_FRAME_NONIP); 
        BCM_IF_ERROR_RETURN(rv);

        frame_data = 0x2;
        frame_mask =  0x3;
        rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_L2_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_data);   
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        rv = DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_L2_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_mask);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        
        ref_data = (data.dsap << 8) || (data.ssap);
        ref_mask = (mask.dsap << 8) || (mask.ssap);

        fg = f_ent->group;
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyEtherType);
		
        rv = bcm_robo_field_qualify_EtherType(unit, entry, ref_data, ref_mask);
    }

    return rv;
}

int
bcm_robo_field_qualify_InnerTpid(int unit, bcm_field_entry_t entry,
                                uint16 tpid)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_OuterTpid(int unit, bcm_field_entry_t entry,
                                uint16 tpid)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IpAuth(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    int rv;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpAuth, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyIpAuth, 
                                 data, mask);
}

/*
 * Function: bcm_robo_field_range_create
 *    
 * Purpose:
 *     Allocate a range checker and set its parameters.
 *
 * Parameters:
 *     unit   - BCM device number
 *     range  - (OUT) Range check ID, will not be zero
 *     flags  - One or more of BCM_FIELD_RANGE_* flags
 *     min    - Lower bounds of range to be checked, inclusive
 *     max    - Upper bounds of range to be checked, inclusive
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */

int
bcm_robo_field_range_create(int unit, bcm_field_range_t *range,
               uint32 flags, bcm_l4_port_t min, bcm_l4_port_t max)
{
    _field_control_t    *fc;
    int                 rv;
    _field_stage_id_t   stage_id;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_robo_field_range_create(unit=%d, range->%p, "),
               unit,
               range));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "flags=0x%08x, min=0x%x, max=0x%x)\n"),
               flags, min, max));

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    

    /* Range type sanity check. */
    BCM_IF_ERROR_RETURN(_robo_field_range_flags_check(unit, flags, &stage_id));

    FP_LOCK(fc);
    
    rv = _robo_field_range_create(unit, range, flags, min, max, stage_id);

    FP_UNLOCK(fc);

    return rv;
}

/*
 * Function: bcm_robo_field_range_group_create
 *    
 * Purpose:
 *     Allocate a range checker and set its parameters.
 *
 * Parameters:
 *     unit   - BCM device number
 *     range  - (OUT) Range check ID, will not be zero
 *     flags  - One or more of BCM_FIELD_RANGE_* flags
 *     min    - Lower bounds of range to be checked, inclusive
 *     max    - Upper bounds of range to be checked, inclusive
 *     group  - L3 interface group number
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_PARAM     - Invalid L3 interface group number
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */

int
bcm_robo_field_range_group_create(int unit,
                                 bcm_field_range_t *range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max,
                                 bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_robo_field_range_create_id
 *    
 * Purpose:
 *     Allocate a range checker and set its parameters.
 *
 * Parameters:
 *     unit   - BCM device number
 *     range  - Range check ID to use
 *     flags  - One or more of BCM_FIELD_RANGE_* flags
 *     min    - Lower bounds of range to be checked, inclusive
 *     max    - Upper bounds of range to be checked, inclusive
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_EXISTS    - Range ID already in use
 *     BCM_E_RESOURCE  - Hardware range checkers all in use
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */
int
bcm_robo_field_range_create_id(int unit, bcm_field_range_t range,
              uint32 flags, bcm_l4_port_t min, bcm_l4_port_t max)
{
    _field_control_t    *fc;
    _field_range_t      *fr;
    int                 hw_index  = _FP_INVALID_INDEX; /* Free/matching rang index.*/
    uint32            ranger_num = 0; 
    uint32            hw_used = 0;     /* Used indexes map.        */
    uint32            hw_used_vlan = 0, hw_used_l4port = 0;
    int    rv = BCM_E_NONE;
    uint32 type = 0;
    _field_stage_id_t   stage_id;       /* Pipeline stage id.       */
    _field_stage_t      *stage_fc;      /* Stage field control info.*/

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_robo_field_range_create_id(unit=%d, range=%d, "),
               unit,
               range));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "flags=0x%08x, min=0x%x, max=0x%x)\n"),
               flags, min, max));

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    /* Range type sanity check. */
    BCM_IF_ERROR_RETURN(_robo_field_range_flags_check(unit, flags, &stage_id));


    if (SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        return BCM_E_UNAVAIL;
    }  else if (SOC_IS_ROBO53242(unit) || 
        SOC_IS_ROBO53262(unit)) {
        if (flags & BCM_FIELD_RANGE_SRCPORT && 
            flags & BCM_FIELD_RANGE_DSTPORT) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: Can't select both source and destination.\n")));
            return (BCM_E_PARAM);
        }

        /*
         * Although the BCM53242/BCM53262 has 4 L4 port range checkers,
         * it also has the qualifier, L4SRCLESS1024(L4 src port less than 1024).
         * So we define hw_index of this qualifier is 4(5th) as its index.
         */
         if (min == 0 && max == 1023) {
            hw_index = 9;
        }
    }


    FP_LOCK(fc);

    rv = _robo_field_stage_control_get(unit, stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);   
        return (rv);
    }
    /* Search existing ranges */
    for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
        if (fr->rid == range) {
            FP_UNLOCK(fc);  
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: range_id=%d already exists.\n"),
                       range));
            return (BCM_E_EXISTS);
        }

        /* Build maps of hardware in-use indexes. */
        hw_used |= (1 << fr->hw_index);

        /* Found an exisiting match so use it. */
        if (flags == fr->flags && min == fr->min && max == fr->max) {
            hw_index = fr->hw_index;
        }

        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            if (fr->flags & BCM_FIELD_RANGE_OUTER_VLAN) {
                hw_used_vlan ++;
            }

            if ((fr->flags & BCM_FIELD_RANGE_SRCPORT) || 
               (fr->flags & BCM_FIELD_RANGE_DSTPORT)) {
                hw_used_l4port ++;
            }
        }
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if (flags & BCM_FIELD_RANGE_OUTER_VLAN) { 
            if (hw_used_vlan >= 4) {
                FP_UNLOCK(fc);  
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: No hardware range checkers left.\n")));
                return (BCM_E_RESOURCE);
            }
        }

        if ((flags & BCM_FIELD_RANGE_SRCPORT) || 
           (flags & BCM_FIELD_RANGE_DSTPORT)) {
            if (hw_used_l4port >= 4) {
                FP_UNLOCK(fc);  
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP Error: No hardware range checkers left.\n")));
                return (BCM_E_RESOURCE);
            }
        }
    }

    /* get ranger num */
    if (SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit)) {
        rv= DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_CFP_RNG_NUM, &ranger_num);
        if (rv < 0) {
            FP_UNLOCK(fc);  
            return rv;
        }
    }

    if (SOC_IS_TBX(unit)) {
        if (flags & BCM_FIELD_RANGE_LOOKUP) {
            rv= DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_IVM_RNG_NUM, &ranger_num);
        } else {
            rv= DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_CFP_RNG_NUM, &ranger_num);
        }
        if (rv < 0) {
            FP_UNLOCK(fc);  
            return rv;
        }        
    }

    /* If no match found, allocate a new hardware index. */
    if (hw_index < 0) {
        int start_idx = 0;
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            if (flags & BCM_FIELD_RANGE_OUTER_VLAN) { 
                start_idx = 4;
            }
        }
        for (hw_index = start_idx; hw_index < ranger_num+start_idx; hw_index++) {
            /* Found an unused FB style range checker */
            if ((hw_used & (1 << hw_index)) == 0) {
                break;
            }
        }

        /* No hardware indexes left. */
        if (hw_index == (ranger_num+start_idx)) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: No hardware range checkers left.\n")));
            return (BCM_E_RESOURCE);
        }
    }
    
    /* Create a new range entry for the list */
    if ((fr = sal_alloc(sizeof (*fr), "fp_range")) == NULL) {
        FP_UNLOCK(fc);
        return BCM_E_MEMORY;
    }

    fr->flags    = flags;
    fr->rid      = range;
    fr->min      = min;
    fr->max      = max;
    fr->hw_index = hw_index;

    if ((SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit)) && (hw_index != 9)) {
        uint32 idx;

        /* parameter checking */
        rv = DRV_CFP_RANGER(unit, flags, min, max);
        if (rv < 0) {
            FP_UNLOCK(fc);
            sal_free(fr);
            return rv;
        }
        if (flags & BCM_FIELD_RANGE_SRCPORT) {
            type = DRV_CFP_RANGE_SRCPORT;
            idx= hw_index;
        } else if (flags & BCM_FIELD_RANGE_DSTPORT){
            type = DRV_CFP_RANGE_DSTPORT;
            idx= hw_index;
        } else {
            type = DRV_CFP_RANGE_VLAN;
            idx = hw_index -4;
        }

        /* ranger _set */
        rv = DRV_CFP_RANGE_SET(unit, type, idx, min, max);
        if (BCM_FAILURE(rv)){
            FP_UNLOCK(fc); 
            sal_free(fr);
            return rv;            
        }
    }
    if (SOC_IS_TBX(unit)) {
        if (flags & BCM_FIELD_RANGE_LOOKUP) {
            rv = DRV_VM_RANGE_SET(unit, hw_index, min, max);
        } else {
            if (flags & BCM_FIELD_RANGE_SRCPORT) {
                type = DRV_CFP_RANGE_SRCPORT;
            } else if (flags & BCM_FIELD_RANGE_DSTPORT){
                type = DRV_CFP_RANGE_DSTPORT;
            }  else if (flags & BCM_FIELD_RANGE_INNER_VLAN){
                type = DRV_CFP_RANGE_VLAN;
            }else if (flags & BCM_FIELD_RANGE_OUTER_VLAN){
                type = DRV_CFP_RANGE_OUTER_VLAN;
            }

            rv = DRV_CFP_RANGE_SET(unit, type, hw_index, min, max);        
        }
        if (BCM_FAILURE(rv)){
            FP_UNLOCK(fc); 
            sal_free(fr);
            return rv;            
        }
    }
    /* Add to list of range checkers. */
    fr->next = stage_fc->ranges;
    stage_fc->ranges = fr;

    FP_UNLOCK(fc);

    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_range_group_create_id
 *    
 * Purpose:
 *     Allocate an interface group range checker and set its parameters.
 *
 * Parameters:
 *     unit   - BCM device number
 *     range  - Range check ID to use
 *     flags  - One or more of BCM_FIELD_RANGE_* flags
 *     min    - Lower bounds of range to be checked, inclusive
 *     max    - Upper bounds of range to be checked, inclusive
 *     group  - L3 interface group number
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_EXISTS    - Range ID already in use
 *     BCM_E_RESOURCE  - Hardware range checkers all in use
 *     BCM_E_PARAM     - Invalid L3 interface group number
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */

int
bcm_robo_field_range_group_create_id(int unit,
                                    bcm_field_range_t range,
                                    uint32 flags,
                                    bcm_l4_port_t min,
                                    bcm_l4_port_t max,
                                    bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function: bcm_robo_field_range_destroy
 *
 * Purpose:
 *     Deallocate a range
 *
 * Parameters:
 *     unit  - BCM device number
 *     range  - Range check ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Invalid range ID
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */
int
bcm_robo_field_range_destroy(int unit, bcm_field_range_t range)
{
    _field_control_t    *fc;
    _field_range_t      *fr, *fr_prev; 
    _field_stage_t      *stage_fc;
    int                       rv;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);

    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS,  
        &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }


    /* Find matching entry */
    fr_prev = NULL;
    for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
        if (fr->rid == range) {
            break;
        }
        fr_prev = fr;
    }

    if (fr == NULL) {
        rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_LOOKUP,  
            &stage_fc);
        if (rv != BCM_E_NONE) {
            FP_UNLOCK(fc);
            return (BCM_E_NOT_FOUND);
        }
        fr_prev = NULL;
        for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
            if (fr->rid == range) {
                break;
            }
            fr_prev = fr;
        }
        if (fr == NULL) {        
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: range=%d not found in unit=%d.\n"),
                       range, unit));
            return BCM_E_NOT_FOUND;
        }
    }


    /* Remove from list and free */
    if (fr_prev == NULL) {
        stage_fc->ranges = fr->next;
    } else {
        fr_prev->next = fr->next;
    }

    FP_UNLOCK(fc);

    sal_free(fr);

    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_range_get
 *    
 * Purpose:
 *     Get the TCP/UDP port for a range
 *
 * Parameters:
 *     unit  - BCM device number
 *     range  - Range check ID
 *     flags - (OUT) Current range checker flags
 *     min   - (OUT) Lower bounds of range to be checked
 *     max   - (OUT) Upper bounds of range to be checked
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Invalid range ID
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX
 */
int
bcm_robo_field_range_get(int unit, bcm_field_range_t range,
            uint32 *flags, bcm_l4_port_t *min, bcm_l4_port_t *max)
{
    _field_control_t    *fc;
    _field_range_t      *fr;
    _field_stage_t      *stage_fc;
    int                         rv;
    
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);

    rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS,  &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
        if (fr->rid == range) {
            break;
        }
    }

    if (fr == NULL) {
                /* Check if it is present in the Lookup */
        rv = _robo_field_stage_control_get(unit, _BCM_FIELD_STAGE_LOOKUP,  
            &stage_fc);
        if (rv != BCM_E_NONE) {
            FP_UNLOCK(fc);
            return (BCM_E_NOT_FOUND);
        }
        for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
            if (fr->rid == range) {
                break;
            }
        }
        if (fr == NULL) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: range=%d not found in unit=%d.\n"),
                       range, unit));
            return BCM_E_NOT_FOUND;
        }
    }

    if (flags != NULL) {
        *flags = fr->flags;
    }

    if (min != NULL) {
        *min = fr->min;
    }

    if (max != NULL) {
        *max = fr->max;
    }

    FP_UNLOCK(fc);

    return BCM_E_NONE;
}


/* bcm_field_show not dispatchable */

/*
 * Function: bcm_robo_field_group_enable_set
 *    
 * Purpose:
 *     Enable/disable packet lookup on a group.
 *
 * Parameters:
 *     unit   - BCM device number
 *     enable - lookup enable!=0/disable==0 state of group
 *     group  - Field group ID
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_UNAVAIL   - BCM device does not have enable/disable feature
 */
int
bcm_robo_field_group_enable_set(int unit, bcm_field_group_t group, int enable)
{
    _field_group_t      *fg;
    bcm_port_t  port;
    uint32  temp;
    _field_control_t    *fc;
    int     rv;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));    

    FP_LOCK(fc);

    rv = _robo_field_group_get (unit,  group, &fg);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }          

    if(soc_feature(unit, soc_feature_field_slice_enable)) {

        /* Enable/disable slice. */
        port = 0;
        rv = DRV_CFP_CONTROL_GET
                (unit, DRV_CFP_SLICE_SELECT, port, &temp);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }          

        if (enable) {
            temp |= (0x1 << fg->slices[0].sel_codes.fpf);
        } else {
            temp &= ~(0x1 << fg->slices[0].sel_codes.fpf);
        }
        PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            rv = DRV_CFP_CONTROL_SET
                (unit, DRV_CFP_SLICE_SELECT, port, temp);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }          
        }
    } else {
        if (!enable) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP Error: group is always enabled !\n")));
            FP_UNLOCK(fc);
            return BCM_E_UNAVAIL;
        }
    }
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

/*
 * Function: bcm_robo_field_group_enable_get
 *    
 * Purpose:
 *     Get the lookup enable/disable state of a group
 *
 * Parameters:
 *     unit   - BCM device number
 *     group  - Field group ID
 *     enable - (OUT) lookup enable/disable state of group
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found on unit
 *     BCM_E_UNAVAIL   - BCM device does not have enable/disable feature
 */
int
bcm_robo_field_group_enable_get(int unit, bcm_field_group_t group, int *enable)
{
    _field_group_t      *fg;
    uint32  temp;
    _field_control_t    *fc;
    int     rv;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));    

    FP_LOCK(fc);
    
    rv = _robo_field_group_get (unit,  group, &fg);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }          
        
    if(soc_feature(unit, soc_feature_field_slice_enable)) {
        
        temp = 0;
        rv = DRV_CFP_CONTROL_GET
                (unit, DRV_CFP_SLICE_SELECT, 0, &temp);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }          
        if (temp & (0x1 << fg->slices[0].sel_codes.fpf)) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    } else {
        *enable = TRUE;
    }
    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

int
bcm_robo_field_qualify_HiGig(int unit, bcm_field_entry_t entry,
                            uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstHiGig
 * Purpose:
 *      Qualify on HiGig destination packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstHiGig(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstHiGig_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstHiGig
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstHiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIpEqualDstIp(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    int                 rv;
    uint32      data;
 
    if (flag) {
        data = 1;;
    } else {
        data = 0;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifySrcIpEqualDstIp, 
                                 data, 1);

    return rv;
}

int 
bcm_robo_field_qualify_EqualL4Port(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    int                 rv;
    uint32      data;
    
    if (flag) {
        data = 1;;
    } else {
        data = 0;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyEqualL4Port, 
                                 data, 1);

    return rv;
}

int 
bcm_robo_field_qualify_TcpSequenceZero(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    int                 rv;
    uint32      data;

    FIELD_IS_INIT(unit);

    if (flag) {
        data = 1;;
    } else {
        data = 0;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyTcpSequenceZero, 
                                 data, 1);

    return rv;
}

/*
 * Function:
 *     bcm_robo_field_qualify_TcpHeaderSize
 * Purpose:
 *     Set a TCP Header Size qualify
 * Parameters:
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *     data - The number of 32 bit words.
 *            The real TcpHeaderSize would be data*4
 *     mask - Mask to qualify with (type is same as for data)
 *
 * Returns:
 *     BCM_E_INIT       BCM Unit not initialized.
 *     BCM_E_NOT_FOUND  Entry ID not found in unit.
 *     BCM_E_XXX        Other errors
 */
int
bcm_robo_field_qualify_TcpHeaderSize(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
    int                 rv;
    uint8 data_val, mask_val, tmp;
 
    if ((data & 0xF0) ||(mask& 0xF0)){
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "FP ERROR: data/mask is in the number of 32bit words. It should not be large than 0xF\n")));
        return BCM_E_PARAM;
    }
        
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if (mask) {
            data_val = 1;
            mask_val = 1;
            /* Set desired TCP header size */
            tmp = data & mask;
            rv = DRV_CFP_RANGE_SET
                    (unit, DRV_CFP_RANGE_TCP_HEADER_LEN, 0, tmp, 0);
            if (rv < 0) {
                return rv;
            }
        } else {
            data_val = 0;
            mask_val = 0;
        }
    } else {
        data_val = data;
        mask_val = mask;
    }

    rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyTcpHeaderSize, 
                                 data_val, mask_val);

    return rv;
}

int 
bcm_robo_field_qualify_IpFrag(int unit, bcm_field_entry_t entry,
                                  bcm_field_IpFrag_t frag_info)
{
     _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32 frag_data, frag_mask, non_first_data, non_first_mask;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpFrag, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

    switch (frag_info) {
        case bcmFieldIpFragNon:
            frag_data = 0;
            frag_mask = 1;
            non_first_mask = 0;
            break;
        case bcmFieldIpFragFirst:
            frag_data = 1;
            frag_mask = 1;
            non_first_data = 0;
            non_first_mask = 1;
            break;
        case bcmFieldIpFragNonOrFirst:
            frag_mask = 0;
            non_first_data = 0;
            non_first_mask = 1;
            break;
        case bcmFieldIpFragNotFirst:
            frag_data = 1;
            frag_mask = 1;
            non_first_data = 1;
            non_first_mask = 1;
            break;
        default:
            return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_FRAG, 
            f_ent->drv_entry, &frag_data));
    
    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_FRAG, 
            f_ent->drv_entry, &frag_mask));

    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &non_first_data));
    
    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &non_first_mask));

    return rv;
    
}

int 
bcm_robo_field_qualify_L3Routable(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{    
    return BCM_E_UNAVAIL;
}
int
bcm_robo_field_qualify_Tos(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
    int rv;
     _field_entry_t      *f_ent;
     
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyTos, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP4); 
    BCM_IF_ERROR_RETURN(rv);

    return _robo_field_qual_value32_set(unit, entry, 
                    bcmFieldQualifyTos, data, mask);

}

int bcm_robo_field_qualify_Vrf(int unit, bcm_field_entry_t entry,
                              uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_L3Ingress(int unit, bcm_field_entry_t entry,
                              uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeaderType(int unit, 
                                               bcm_field_entry_t entry,
                                               uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeader2Type(int unit, 
                                               bcm_field_entry_t entry,
                                               uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

/* Function    : bcm_robo_field_qualify_L4Ports
 * Description : qualify on the 4 bytes after L3 header.
 * Parameters  : (IN) unit   BCM device number
 *               (IN) entry  Field entry to qualify
 *               (IN) data   0/1 4 bytes after L3 header are present. 
 *               (IN) mask   data mask.
 * Returns     : BCM_E_XXX
 */
int bcm_robo_field_qualify_L4Ports(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
#ifdef BCM_VO_SUPPORT
    int rv;
     _field_entry_t      *f_ent;     

    if (SOC_IS_VO(unit)) {
        rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyL4Ports, &f_ent);
        BCM_IF_ERROR_RETURN(rv);
        
        rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
        BCM_IF_ERROR_RETURN(rv);
        return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyL4Ports, 
                             data, mask);
    }
#endif

    return (BCM_E_UNAVAIL);
}

/* Function    : bcm_robo_field_qualify_MirrorCopy
 * Description : qualify on the mirrored packets only.
 * Parameters  : (IN) unit   BCM device number
 *               (IN) entry  Field entry to qualify
 *               (IN) data   0/1 Not Mirrored/Mirrored packets. 
 *               (IN) mask   data mask.
 * Returns     : BCM_E_XXX
 */
int bcm_robo_field_qualify_MirrorCopy(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}


/* Function    : bcm_robo_field_qualify_TunnelTerminated
 * Description : qualify on the tunnel terminated packets only.
 * Parameters  : (IN) unit   BCM device number
 *               (IN) entry  Field entry to qualify
 *               (IN) data   0/1 Not Tunneled/Tunnel Terminated packets. 
 *               (IN) mask   data mask.
 * Returns     : BCM_E_XXX
 */
int bcm_robo_field_qualify_TunnelTerminated(int unit, bcm_field_entry_t entry,
                                           uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeaderSubCode(int unit, 
                                                  bcm_field_entry_t entry,
                                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_OutPorts(int unit, bcm_field_entry_t entry,
                               bcm_pbmp_t data, bcm_pbmp_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerSrcIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerSrcIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerSrcIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerTos(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDSCP(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerIpProtocol(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}


int bcm_robo_field_qualify_InnerIpFrag(int unit, bcm_field_entry_t entry,
                                  bcm_field_IpFrag_t frag_info)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerTtl(int unit, bcm_field_entry_t entry,
                          uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}


int 
bcm_robo_field_qualify_DosAttack(int unit, bcm_field_entry_t entry, 
                                uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IpmcStarGroupHit(int unit, bcm_field_entry_t entry, 
                                       uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_qualify_MyStationHit
 * Purpose:
 *      Qualify for bcmFieildQualifyMyStationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_MyStationHit(int unit, bcm_field_entry_t entry, 
                                       uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_L2PayloadFirstEightBytes(int unit, bcm_field_entry_t entry,
                                            uint32 data1, uint32 data2,
                                            uint32 mask1, uint32 mask2)
{
    return (BCM_E_UNAVAIL);
}


int 
bcm_robo_field_qualify_L3DestRouteHit(int unit, bcm_field_entry_t entry, 
                                     uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L3DestHostHit(int unit, bcm_field_entry_t entry, 
                                     uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L3SrcHostHit(int unit, bcm_field_entry_t entry, 
                                    uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2CacheHit(int unit, bcm_field_entry_t entry, 
                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2StationMove(int unit, bcm_field_entry_t entry, 
                                    uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2DestHit(int unit, bcm_field_entry_t entry, 
                                uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2SrcStatic(int unit, bcm_field_entry_t entry, 
                                  uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2SrcHit(int unit, bcm_field_entry_t entry, 
                               uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IngressStpState(int unit,bcm_field_entry_t entry, 
                                      uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_ForwardingVlanValid(int unit, bcm_field_entry_t entry, 
                                          uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_SrcVirtualPortValid(int unit, bcm_field_entry_t entry,
                                           uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstL3EgressNextHops(int unit, bcm_field_entry_t entry,
                                          uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_VlanTranslationHit(int unit, bcm_field_entry_t entry, 
                                         uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_BigIcmpCheck(int unit, bcm_field_entry_t entry,
                                         uint32 flag, uint32 size)
{
    int                 rv;
    uint32      data;
  
    if (SOC_IS_ROBO53242(unit)|| SOC_IS_ROBO53262(unit)) {
        if (flag) {
            data = 1;
            /* icmp size set */
            rv = DRV_CFP_RANGE_SET
                    (unit, DRV_CFP_RANGE_BIG_ICMP, 0, size, 0);
            if (rv < 0) {
                return rv;
            }
        } else {
            data = 0;
        }    
        rv = _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyBigIcmpCheck, 
                                     data, 1);

    } else {
        rv = BCM_E_UNAVAIL;
    }
    return rv;
}

int 
bcm_robo_field_qualify_IcmpTypeCode(int unit, bcm_field_entry_t entry,
                                         uint16 data,uint16 mask)
{
    int rv;
     _field_entry_t      *f_ent;     


    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIcmpTypeCode, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);

   return _robo_field_qual_value32_set(unit, entry,
                    bcmFieldQualifyIcmpTypeCode, data, mask);
}

int 
bcm_robo_field_qualify_IgmpTypeMaxRespTime(int unit, bcm_field_entry_t entry,
                                         uint16 data,uint16 mask)
{
   
    int rv;
     _field_entry_t      *f_ent;     

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIgmpTypeMaxRespTime, &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    rv = _robo_field_qualify_frame_type_set(unit, f_ent, _DRV_CFP_FRAME_IP); 
    BCM_IF_ERROR_RETURN(rv);
    return _robo_field_qual_value32_set(unit, entry, 
                    bcmFieldQualifyIgmpTypeMaxRespTime, data, mask);
}

int
bcm_robo_field_qualify_InnerL4DstPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerL4SrcPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIp6FlowLabel(int unit, bcm_field_entry_t entry,
                                   uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_DstL3Egress(int unit, 
                                   bcm_field_entry_t entry, 
                                   bcm_if_t if_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMulticastGroup(int unit, 
                                 bcm_field_entry_t entry, 
                                 bcm_gport_t group)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_SrcMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_SrcMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_SrcWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_Loopback(int unit, 
                               bcm_field_entry_t entry, 
                               uint8 data, 
                               uint8 mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_LoopbackType(int unit, 
                                   bcm_field_entry_t entry, 
                                   bcm_field_LoopbackType_t loopback_type)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_TunnelType(int unit, 
                                 bcm_field_entry_t entry, 
                                 bcm_field_TunnelType_t tunnel_type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_FlowId(int unit, bcm_field_entry_t entry,
                 uint16 data, uint16 mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyFlowId, 
                                     data, mask);
    }
#endif

    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_InVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyInVPort, 
                                     data, mask);
    } 
#endif

    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_OutVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value32_set(unit, entry, bcmFieldQualifyOutVPort, 
                                     data, mask);
    } 
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_LoopbackType_get
 * Purpose:
 *      Get loopback type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      loopback_type - (OUT) Loopback type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_LoopbackType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_LoopbackType_t *loopback_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_TunnelType_get
 * Purpose:
 *      Get tunnel type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      tunnel_type - (OUT) Tunnel type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TunnelType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_TunnelType_t *tunnel_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstL3Egress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstL3Egress
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      if_id - (OUT) L3 forwarding object.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstL3Egress_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_if_t *if_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstMulticastGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMulticastGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      group - (OUT) Multicast group id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstMulticastGroup_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *group)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Loopback_get
 * Purpose:
 *      Get loopback field qualification from  a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data - (OUT) Data to qualify with.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Loopback_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    _field_entry_t      *f_ent;
    int                 rv = SOC_E_UNAVAIL;
    int pbmp_vld;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyInPort, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);



    pbmp_vld = 0;
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit)|| SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        pbmp_vld = 1;
    } 
        
    if (SOC_IS_VO(unit)){
        if ( _BCM_FIELD_STAGE_INGRESS ==  f_ent->fs->stage_id) {
            /* VO CFP (_BCM_FIELD_STAGE_INGRESS) support pbmp */
            pbmp_vld = 1;
        }
    }

    if (1 == pbmp_vld){
        uint32 mask_val, temp;
        bcm_pbmp_t pbmp;
        int port;

        rv = DRV_FP_QUAL_VALUE_GET(unit, f_ent->fs->stage_id, 
            bcmFieldQualifyInPorts, f_ent->drv_entry, &temp,&mask_val);

        if (rv != BCM_E_NONE) {
            return rv;
        }
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, ~mask_val);

        SOC_PBMP_ITER(pbmp, port) {
            *data = port;
            break;
        }        
        *mask = SOC_PBMP_WORD_GET(PBMP_E_ALL(unit), 0);        
    } else {
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyInPort,
            (uint32 *)data, (uint32 *)mask);
    }
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_OutPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OutPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    int                 rv = BCM_E_UNAVAIL;

    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        rv = _robo_field_qual_value32_get(unit, entry,bcmFieldQualifyOutPort, 
                     (uint32 *)data, (uint32 *)mask);
    }
#endif

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    int                 rv;
    uint32 data_val = 0, mask_val = 0, temp = 0;
    bcm_pbmp_t pbmp;
    int port;
    
    FIELD_IS_INIT(unit);

  
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_VULCAN(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit)||
        SOC_IS_VO(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyInPorts, 
                                 &data_val, &mask_val);
        if (rv != BCM_E_NONE) {
            return rv;
        }
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, ~mask_val);
        SOC_PBMP_AND(pbmp, PBMP_E_ALL(unit));

        SOC_PBMP_ITER(pbmp, port) {
            if (port < SOC_ROBO_MAX_NUM_PORTS) {
                temp |= (0x1 << port);
            }    
        }
        SOC_PBMP_WORD_SET(*data, 0, temp);

        *mask = PBMP_E_ALL(unit);
        
    } else {
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyInPorts, 
                             &data_val, &mask_val);
        if (rv == BCM_E_NONE) {
            SOC_PBMP_WORD_SET(*data, 0, data_val);
            SOC_PBMP_WORD_SET(*mask, 0, mask_val);
        }
    }
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_OutPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OutPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Drop_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDrop
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Drop_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    int rv;
    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifySrcPort,
        (uint32 *)data_port, (uint32 *)mask_port);
    BCM_IF_ERROR_RETURN(rv);

    *data_modid = 0;
    *mask_modid = 0;
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerL4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerL4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerL4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerL4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    _field_entry_t      *f_ent;
    int                 rv;

    FIELD_IS_INIT(unit);

    f_ent = _field_entry_qual_find(unit, entry, bcmFieldQualifyL4SrcPort);
    if (f_ent == NULL) {
        return BCM_E_NOT_FOUND;
    }

    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyL4SrcPort, 
                                  (uint32 *)data, (uint32 *)mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    _field_entry_t      *f_ent;
    int                 rv;

    FIELD_IS_INIT(unit);

    f_ent = _field_entry_qual_find(unit, entry, bcmFieldQualifyL4DstPort);
    if (f_ent == NULL) {
        return BCM_E_NOT_FOUND;
    }

    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyL4DstPort, 
                                  (uint32 *)data, (uint32 *)mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_OuterVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    int     rv;       /* Operation return status. */
    uint8  hw_data;  /* Installed entry value.   */
    uint8  hw_mask;  /* Installed entry mask.    */


    if ((NULL == data) || (NULL == mask)) {
        return (BCM_E_PARAM);
    }

    rv = bcm_field_qualify_OuterVlanId_get(unit, entry, 
                                               data, mask);
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_field_qualify_OuterVlanCfi_get(unit, entry,
                                                &hw_data, &hw_mask); 
    BCM_IF_ERROR_RETURN(rv);
    *data |= (hw_data << 12);
    *mask |= (hw_mask << 12);

    rv = bcm_field_qualify_OuterVlanPri_get(unit, entry,
                                                &hw_data, &hw_mask);
    BCM_IF_ERROR_RETURN(rv);
    *data |= (hw_data << 13);
    *mask |= (hw_mask << 13);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    uint8  hw_data;  /* Installed entry value.   */
    uint8  hw_mask;  /* Installed entry mask.    */
    int    rv;       /* Operation return status. */


    if ((NULL == data) || (NULL == mask)) {
        return (BCM_E_PARAM);
    }

    rv = bcm_field_qualify_InnerVlanId_get(unit, entry, 
                                               data, mask);
    BCM_IF_ERROR_RETURN(rv);

    rv = bcm_field_qualify_InnerVlanCfi_get(unit, entry,
                                                &hw_data, &hw_mask); 
    BCM_IF_ERROR_RETURN(rv);
    *data |= (hw_data << 12);
    *mask |= (hw_mask << 12);

    rv = bcm_field_qualify_InnerVlanPri_get(unit, entry,
                                                 &hw_data, &hw_mask);
    BCM_IF_ERROR_RETURN(rv);
    *data |= (hw_data << 13);
    *mask |= (hw_mask << 13);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_EtherType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    int                 rv;

    rv = _robo_field_qual_value16_get(unit, entry, bcmFieldQualifyEtherType, 
                     data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_EqualL4Port_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEqualL4Port
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_EqualL4Port_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    int                 rv;
    uint32      data = 0, mask= 0;
    

    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyEqualL4Port, 
                                 &data, &mask);

    if (data & mask) {
        *flag = 1;
    } else {
        *flag = 0;
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    int                 rv;
 
    rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyIpProtocol, 
                               data, mask);
    return rv;
}


/*
 * Function:
 *      bcm_robo_field_qualify_IpInfo_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpInfo
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpInfo_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    _field_entry_t      *f_ent;
    _field_group_t      *fg = NULL;		
    int                 rv;
    uint32 temp, temp_frag, temp_nonfirst;
    uint8 frag_data = 0, frag_mask = 0;
    uint32 nonfirst_data, nonfirst_mask;
        
    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpInfo, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    fg = f_ent->group;
    BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyIpFlags);	

    rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyIpFlags, 
                               &frag_data, &frag_mask);

    BCM_IF_ERROR_RETURN(rv);

    rv = DRV_CFP_FIELD_GET        
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &nonfirst_data);
    BCM_IF_ERROR_RETURN(rv);

    rv = DRV_CFP_FIELD_GET        
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &nonfirst_mask);  
    BCM_IF_ERROR_RETURN(rv);


    /* frag_mask==0 or nonfirst_mask==0 means we don't care the data value
      * but for the easy implementation in the bcm_robo_field_qualify_IpInfo_get
      * we desing the 1-to-1 mapping table as following
      */
    /*  data    mask
     * MF,OZ   MF,OZ       Frag    Non_First 
     *  0,0     0,0   0     0,0     0,0     0
     *  0,0     0,1   1     0,0     1,1     3
     *  0,0     1,0   2     0,1     0,0     4
     *  0,0     1,1   3     0,1     1,1     7
     *  0,1     0,0   4     0,0     1,0     2
     *  0,1     0,1   5     0,0     0,1     1
     *  0,1     1,0   6     0,1     1,0     6
     *  0,1     1,1   7     0,1     0,1     5
     *  1,0     0,0   8     1,0     0,0     8
     *  1,0     0,1   9     1,0     1,1     11
     *  1,0     1,0  10     1,1     0,0     12
     *  1,0     1,1  11     1,1     1,1     15
     *  1,1     0,0  12     1,0     1,0     10
     *  1,1     0,1  13     1,0     0,1     9
     *  1,1     1,0  14     1,1     1,0     14
     *  1,1     1,1  15     1,1     0,1     13
     */

    temp_frag = ((frag_data << 1)&0x2) |(frag_mask & 0x1);
    temp_nonfirst = ((nonfirst_data << 1)&0x2) |(nonfirst_mask & 0x1);

    temp = ((temp_frag << 2) & 0xc) | (temp_nonfirst & 0x3);

    switch (temp) {
        case (0):
            *data = 0;
            *mask = 0;
            break;
        case (1):
            *data = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        case (2):
            *data = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = 0;
            break;
        case (3):
            *data = 0;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        case (4):
            *data = 0;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF;
            break;
        case (5):
            *data = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO|BCM_FIELD_IP_HDR_FLAGS_MF;
            break;            
        case (6):
            *data = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF;
            break;
        case (7):
            *data = 0;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO|BCM_FIELD_IP_HDR_FLAGS_MF;
            break;
        case (8):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF;
            *mask = 0;
            break;
        case (9):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        case (10):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = 0;
            break;
        case (11):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF;
            *mask = BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        case (12):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF;
            break;
        case (13):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        case (14):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF;
            break;
        case (15):
            *data = BCM_FIELD_IP_HDR_FLAGS_MF;
            *mask = BCM_FIELD_IP_HDR_FLAGS_MF|BCM_FIELD_IP_HDR_OFFSET_ZERO;
            break;
        default:
            break;
    }

    return (rv);
}

/*
 * Function:
 *      bcm_robo_field_qualify_PacketRes_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPacketRes
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_PacketRes_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp, 
                        data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    
    return _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyDstIp, data, mask); 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Tos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTos
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Tos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, 
        bcmFieldQualifyTos, data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{

    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyDSCP, data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpFlags_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFlags
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpFlags_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    int rv;
    uint8 new_data = 0, new_mask = 0;

    rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyIpFlags, 
                               &new_data, &new_mask);
    SOC_IF_ERROR_RETURN(rv);

    if (new_mask == 1){
        if (new_data == 1) {
            *data = BCM_FIELD_IPFLAGS_MF;
            *mask = BCM_FIELD_IPFLAGS_MF;
        } else {
            *data = BCM_FIELD_IPFLAGS_DF;
            *mask = BCM_FIELD_IPFLAGS_DF;
        }
    } else {
        *data = BCM_FIELD_IPFLAGS_MF|BCM_FIELD_IPFLAGS_DF;
        *mask = BCM_FIELD_IPFLAGS_MF|BCM_FIELD_IPFLAGS_DF;
    }
    return rv;

}

/*
 * Function:
 *      bcm_robo_field_qualify_TcpControl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpControl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TcpControl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyTcpControl, 
                                 data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_TcpSequenceZero_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpSequenceZero
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TcpSequenceZero_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    int                 rv;
    uint32      data = 0, mask = 0;

    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyTcpSequenceZero, 
                                 &data, &mask);

    BCM_IF_ERROR_RETURN(rv);

    if (data) {
        *flag = 1;
    } else {
        *flag = 0;
    }
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_TcpHeaderSize_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpHeaderSize
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TcpHeaderSize_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    int                 rv;
    uint8 data_val = 0, mask_val = 0;
    uint32 tmp = 0;
 
    rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyTcpHeaderSize, 
                                 &data_val, &mask_val);
    BCM_IF_ERROR_RETURN(rv);
    
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if (data_val & mask_val) {
            rv = DRV_CFP_RANGE_GET
                    (unit, DRV_CFP_RANGE_TCP_HEADER_LEN, 0, &tmp, 0);
            BCM_IF_ERROR_RETURN(rv);
            *data = tmp;
            *mask = tmp;
        } else {
            *data = 0;
            *mask = 0;
        }
    } else {
        *data = data_val;
        *mask = mask_val;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_field_qualify_Ttl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Ttl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    int                 rv = BCM_E_NONE;
    uint8 new_data = 0, new_mask = 0;

    /* The selections are 0, 1, 255 and others */
    if (SOC_IS_VULCAN(unit) || SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyTtl, 
            &new_data, &new_mask);
        
        switch (new_data) {
            case 0:
                *data = 0;
                *mask = 255;
                break;
            case 1:
                *data = 1;
                *mask = 255;
                break;
            case 3:
                *data = 255;
                *mask = 255;
                break;
            default: /* 2 ~ 254 */
                *data = 2;
                *mask = 255;
                
        }
    } else {
        rv = _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyTtl, 
            data, mask);
    }
    return rv; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_RangeCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    int max_count, 
    bcm_field_range_t *range, 
    int *invert, 
    int *count)
{
    _field_entry_t      *f_ent;
    int                 rv;
    _field_control_t *fc;
    _field_range_t      *fr;
    _field_stage_t         *stage_fc;
    uint32  fld_data = 0, fld_mask = 0;
    int idx = 0;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyRangeCheck, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);
    
    /* Get stage field control structure. */
    rv = _robo_field_stage_control_get(unit, f_ent->group->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* Fill range indexes. */
        /* Ranger 1~4 */
        rv = DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_L4_PORT_RANGE, 
            f_ent->drv_entry, &fld_data);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }

        rv = DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_L4_PORT_RANGE, 
            f_ent->drv_entry, &fld_mask);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }

        idx = 0;
        for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
            if (fld_mask & (1 <<  fr->hw_index)) {
                if (idx >=  max_count) {
                    idx++;
                    continue;
                }
                if (NULL != range) {
                    range[idx] = fr->rid;
                }
                if (NULL != invert) {
                    if (fld_data & (1 << fr->hw_index)) {
                        invert[idx] = FALSE;
                    } else {
                        invert[idx] = TRUE;
                    }
                }
                idx++;
            }
        }

        /* Ranger 5~8 */
        rv = DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VLAN_RANGE, 
            f_ent->drv_entry, &fld_data);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }

        rv = DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_VLAN_RANGE, 
            f_ent->drv_entry, &fld_mask);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }

        fld_data <<= 4;
        fld_mask <<= 4;
        
        for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
            if (fld_mask & (1 <<  fr->hw_index)) {
                if (idx >=  max_count) {
                    idx++;
                    continue;
                }
                if (NULL != range) {
                    range[idx] = fr->rid;
                }
                if (NULL != invert) {
                    if (fld_data & (1 << fr->hw_index)) {
                        invert[idx] = FALSE;
                    } else {
                        invert[idx] = TRUE;
                    }
                }
                idx++;
            }
        }

        /* Ranger 9 */
        rv = _robo_field_qual_value32_get(unit, entry,
            bcmFieldQualifyRangeCheck, &fld_data, &fld_mask);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
        if (fld_mask) {
            if (NULL != range) {
                range[idx] = 9;
            }
            if (NULL != invert) {
                if (fld_data) {
                    invert[idx] = FALSE;
                } else {
                    invert[idx] = TRUE;
                }
            }
            idx ++;
        }
         *count = idx;
    } else if (SOC_IS_TBX(unit)){
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyRangeCheck, 
                                 &fld_data, &fld_mask);
        /* Fill range indexes. */
        idx = 0;
        for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
            if (fld_mask & (1 <<  fr->hw_index)) {
                if (idx >=  max_count) {
                    idx++;
                    continue;
                }
                if (NULL != range) {
                    range[idx] = fr->rid;
                }
                if (NULL != invert) {
                    if (fld_data & (1 << fr->hw_index)) {
                        invert[idx] = FALSE;
                    } else {
                        invert[idx] = TRUE;
                    }
                }
                idx++;
            }
        }
        *count = idx;
    } else {
        /* BCM5348/BCM5347 */
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyRangeCheck, 
                                 &fld_data, &fld_mask);
        idx = 0;
        if (max_count) {
            if (fld_mask) {
                if (NULL != range) {
                    range[idx] = 1;
                }
                if (NULL != invert) {
                    if (fld_data) {
                        invert[idx] = FALSE;
                    } else {
                        invert[idx] = TRUE;
                    }
                }

            }
        }
        *count = idx;
  }

    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];

    sal_memset(data_field, 0, 4 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 4 * sizeof(uint32)); 
    
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp6, 
        data_field, mask_field);

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];

    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
  
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        LOG_WARN(BSL_LS_BCM_FP,
                 (BSL_META_U(unit,
                             "FP WARN: 53242/53262 only support lower-44 bit SrcIp6 qualifier")));
        return BCM_E_UNAVAIL;
    }
    sal_memset(data_field, 0, 4 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 4 * sizeof(uint32)); 
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcIp6, 
        data_field, mask_field);

    sal_memset(&data_field, 0, 2 * sizeof(uint32)); 
    sal_memset(&mask_field, 0, 2 * sizeof(uint32)); 

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcIpEqualDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIpEqualDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcIpEqualDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32      fld_index = 0;
    uint32      temp_data = 0, temp_mask = 0;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifySrcIpEqualDstIp, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* 
         * When get the configured value, check if L3_FRM_FORMAT
         * is IP4 mode.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp_data));
    
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp_mask));

        temp_data &= temp_mask;

        if (temp_data != FP_BCM53242_L3_FRM_FMT_IP4) {
            *flag = 0;
            return BCM_E_NONE;
        }
    }

    rv = _robo_field_qual_value32_get(unit, entry, 
                   bcmFieldQualifySrcIpEqualDstIp, &temp_data, &temp_mask);
    if (temp_data & temp_mask) {
        *flag = 1;
    } else {
        *flag = 0;
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    int                 rv;
    uint32      data_field[4], mask_field[4];
  
    sal_memset(data_field, 0, 4 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 4 * sizeof(uint32)); 
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifyDstIp6, 
        data_field, mask_field);

    sal_memset(data_field, 0, 2 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 2 * sizeof(uint32)); 

    SAL_IP6_ADDR_FROM_UINT32((*data), data_field);
    SAL_IP6_ADDR_FROM_UINT32((*mask), mask_field);
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_Ip6NextHeader_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6NextHeader
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Ip6NextHeader_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32      fld_index = 0;
    uint32      temp_data = 0, temp_mask = 0;

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6NextHeader, 
                &f_ent);
        BCM_IF_ERROR_RETURN(rv);
    
        /* 
         * Since the Ip6NextHeader qualifier id is equal to 
         * IpProtocol, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6NextHeader.
         * 
         * When get the configured value, check if L3_FRM_FORMAT
         * is IP6 mode.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp_data));
    
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp_mask));

        temp_data &= temp_mask;

        if (temp_data != FP_BCM53242_L3_FRM_FMT_IP6) {
            *data = 0;
            *mask = 0;
            return BCM_E_NONE;
        }
    }

    rv = _robo_field_qual_value8_get
            (unit, entry, bcmFieldQualifyIp6NextHeader, 
             data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_Ip6TrafficClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6TrafficClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Ip6TrafficClass_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32      fld_index = 0;
    uint32      temp_data = 0, temp_mask = 0;

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6TrafficClass, 
                &f_ent);
        BCM_IF_ERROR_RETURN(rv);

        /* 
         * Since the Ip6TrafficClass qualifier id is equal to 
         * DSCP, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6TrafficClass.
         * 
         * When get the configured value, check if L3_FRM_FORMAT
         * is IP6 mode.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp_data));
    
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp_mask));

        temp_data &= temp_mask;

        if (temp_data != FP_BCM53242_L3_FRM_FMT_IP6) {
            *data = 0;
            *mask = 0;
            return BCM_E_NONE;
        }
    }

    rv = _robo_field_qual_value8_get
            (unit, entry, bcmFieldQualifyIp6TrafficClass, 
             data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerIp6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerIp6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Ip6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Ip6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32      fld_index = 0;
    uint32      temp_data = 0, temp_mask = 0;

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIp6FlowLabel, 
                &f_ent);
        BCM_IF_ERROR_RETURN(rv);

        /* 
         * Since the Ip6FlowLabel qualifier id is equal to 
         * DSCP, the L3_FRM_FORMAT field will be programmed 
         * as IP4 at _robo_field_qual_value32_set().
         *
         * The L3_FRM_FORMAT has to be configured to IP6
         * after set qualifier value of Ip6TrafficClass.
         * 
         * When get the configured value, check if L3_FRM_FORMAT
         * is IP6 mode.
         */
        fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, fld_index,
                f_ent->drv_entry, &temp_data));
    
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                f_ent->drv_entry, &temp_mask));

        temp_data &= temp_mask;

        if (temp_data != FP_BCM53242_L3_FRM_FMT_IP6) {
            *data = 0;
            *mask = 0;
            return BCM_E_NONE;
        }
    }

    rv = _robo_field_qual_value32_get
            (unit, entry, bcmFieldQualifyIp6FlowLabel, 
             data, mask);

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_Ip6HopLimit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6HopLimit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Ip6HopLimit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    int rv; 
    uint8 new_data = 0;

    rv = _robo_field_qual_value8_get
            (unit, entry,bcmFieldQualifyIp6HopLimit, 
             &new_data, mask);

    BCM_IF_ERROR_RETURN(rv);

    if (SOC_IS_VULCAN(unit) || SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        switch (new_data) {
            case 0:
                *data = 0;
                break;
            case 1:
                *data = 1;
                break;
            case 3:
                *data = 255;
                break;
            default:
                *data = 2;
        }
    }else {
        *data = new_data;
    }

    return rv;
    
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    int                 rv;
    uint32  mac_field[2], mask_field[2];

    sal_memset(mac_field, 0, 2 * sizeof(uint32)); 
    sal_memset(mask_field, 0, 2 * sizeof(uint32)); 

    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySrcMac,
        mac_field, mask_field);

    SAL_MAC_ADDR_FROM_UINT32((*data), mac_field);
    SAL_MAC_ADDR_FROM_UINT32((*mask), mask_field);
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    int                 rv;
    uint32      mac_field[2], mask_field[2];

    sal_memset(mac_field, 0, 2 * sizeof(uint32));
    sal_memset(mask_field, 0, 2 * sizeof(uint32));
    rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifyDstMac,
        mac_field, mask_field);

    SAL_MAC_ADDR_FROM_UINT32((*data), mac_field);
    SAL_MAC_ADDR_FROM_UINT32((*mask), mask_field);


    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerIpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Inner ip header ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerIpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_ForwardingType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match forwarding type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ForwardingType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_ForwardingType_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    int                 rv;
    uint32            data = 0, mask= 0;
    
    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyIpType, 
                               &data, &mask);
    SOC_IF_ERROR_RETURN(rv);

    rv = BCM_E_UNAVAIL;
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        rv = BCM_E_NONE;
        switch(data) {
            case FP_BCM53242_L3_FRM_FMT_IP4:
                *type = bcmFieldIpTypeIpv4Any;
                break;
            case FP_BCM53242_L3_FRM_FMT_IP6:
                *type = bcmFieldIpTypeIpv6;
                break;
            case FP_BCM53242_L3_FRM_FMT_OTHERS:
                *type = bcmFieldIpTypeNonIp;
                break;
            default:
                if (mask == 0) {
                    *type = bcmFieldIpTypeAny;
                } else {
                    rv = BCM_E_UNAVAIL;
                }
                break;
        }
    }        

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = BCM_E_NONE;
        switch (data) {
            case FP_BCM53115_L3_FRM_FMT_IP4:
                *type = bcmFieldIpTypeIpv4Any;
                break;
            case FP_BCM53115_L3_FRM_FMT_IP6:
                *type = bcmFieldIpTypeIpv6;
                break;
            case FP_BCM53115_L3_FRM_FMT_NON_IP:
                *type = bcmFieldIpTypeNonIp;
                break;
            default:
                if (mask == 0) {
                    *type = bcmFieldIpTypeAny;
                } else {
                    rv = BCM_E_UNAVAIL;
                }
                break;
        }
    }
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){        
        rv = BCM_E_NONE;
        switch (data) {
            case FP_TB_L3_FRM_FMT_IP4:
                *type = bcmFieldIpTypeIpv4Any;
                break;
            case FP_TB_L3_FRM_FMT_IP6:
                *type = bcmFieldIpTypeIpv6;
                break;
            case FP_TB_L3_FRM_FMT_NON_IP:
                *type = bcmFieldIpTypeNonIp;
                break;
            default:
                if (mask == 0) {
                    *type = bcmFieldIpTypeAny;
                } else {
                    rv = BCM_E_UNAVAIL;
                }
                break;
        }
    }
#endif

    if (BCM_SUCCESS(rv)) {
#ifdef BROADCOM_DEBUG
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP: %s IpType %s"),
                   FUNCTION_NAME(), 
                   _robo_field_qual_IpType_name(data)));
#endif
    }

    return rv;

}

/*
 * Function:
 *      bcm_robo_field_qualify_L2Format_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2Format
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match l2 format.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2Format_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_L2Format_t *type)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32              data = 0, mask = 0;
#ifdef BCM_TB_SUPPORT
    _field_stage_id_t stage_id = 0;
#endif /* BCM_TB_SUPPORT */

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyL2Format, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = _robo_field_qual_value32_get
            (unit, entry, bcmFieldQualifyL2Format, &data, &mask);

        if ((data == 0x0) && (mask == 0x0)) {
            *type = bcmFieldL2FormatAny;
        } else if ((data == 0x0) && (mask == 0x3)) {
            *type = bcmFieldL2FormatEthII;
        } else if ((data == 0x1) && (mask == 0x1)) {
            *type = bcmFieldL2FormatSnap;
        } else if ((data == 0x2) && (mask == 0x3)) {
            *type = bcmFieldL2FormatLlc;
        } else if ((data & 0x4) && (mask & 0x4)) {
            *type = bcmFieldL2FormatPPPoE;
        }
    } else if (SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit)) {
        rv = _robo_field_qual_value32_get
            (unit, entry, bcmFieldQualifyL2Format, &data, &mask);
        if ((data == 0x0) && (mask == 0x0)) {
            *type = bcmFieldL2FormatAny;
        } else if ((data == 0x1) && (mask == 0x3)) {
            *type = bcmFieldL2FormatEthII;
        } else if ((data == 0x2) && (mask == 0x3)) {
            *type = bcmFieldL2FormatSnap;
        }
#if defined(BCM_TB_SUPPORT)
    } else if (SOC_IS_TBX(unit)) {
        /* Get current stage */
        stage_id = f_ent->fs->stage_id;

        switch(stage_id) {
            case _BCM_FIELD_STAGE_INGRESS:
            case _BCM_FIELD_STAGE_LOOKUP:
                /* IVM */
                rv = _robo_field_qual_value32_get
                    (unit, entry, bcmFieldQualifyL2Format, &data, &mask);
                if ((mask == 0x3) && (data == 0x0)) {
                    *type = bcmFieldL2FormatEthII;
                } else if ((mask == 0x3) && (data == 0x1)) {
                    *type = bcmFieldL2FormatSnap;
                } else if ((mask == 0x3) && (data == 0x3)) {
                    *type = bcmFieldL2FormatSnapPrivate;
                } else if ((mask == 0x2) && (data == 0x2)) {
                    /* LLC or 802.3 */
                    *type = bcmFieldL2FormatLlc;
                } else if (mask == 0x0) {
                    *type = bcmFieldL2FormatAny;
                }
                break;
            case _BCM_FIELD_STAGE_EGRESS:
            default:
                return BCM_E_PARAM;
                break;
        }
#endif
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_VlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_VlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32 new_data = 0, new_mask = 0;
    int    dtag_mode = BCM_PORT_DTAG_MODE_NONE;
#ifdef BCM_TB_SUPPORT
    _field_stage_id_t stage_id = 0;
#endif

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyVlanFormat, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
#ifdef BCM_TB_SUPPORT
    /* Get current stage */
    stage_id = f_ent->fs->stage_id;

    if (SOC_IS_TBX(unit)) {
        if (stage_id == _BCM_FIELD_STAGE_EGRESS) {
            return BCM_E_PARAM;
        }

        /* Get STag status. */
        if (stage_id == _BCM_FIELD_STAGE_INGRESS) {
            rv = DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_TCAM, 
                                             DRV_CFP_FIELD_SPTAGGED, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_VM_FIELD_GET(unit, DRV_CFP_RAM_TCAM_MASK, 
                                             DRV_CFP_FIELD_SPTAGGED,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        } else if (stage_id == _BCM_FIELD_STAGE_LOOKUP) {
            rv = DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_DATA, 
                                                 DRV_VM_FIELD_IVM_INGRESS_STAG_STAUS, 
                                                 f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            rv = DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_MASK, 
                                                 DRV_VM_FIELD_IVM_INGRESS_STAG_STAUS, 
                                                 f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }

        if ((new_data == 0x0) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
        } else if ((new_data == 0x3) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        } else if ((new_data == 0x1) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
        } else if ((new_data == 0x3) && (new_mask == 0x1)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
        } else {
            /* unexpected value */
            rv = BCM_E_INTERNAL;
            return rv;
        }

        /* Get CTag status. */
        if (stage_id == _BCM_FIELD_STAGE_INGRESS) {
            rv = DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_TCAM, 
                                             DRV_CFP_FIELD_1QTAGGED, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_VM_FIELD_GET(unit, DRV_CFP_RAM_TCAM_MASK, 
                                             DRV_CFP_FIELD_1QTAGGED,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        } else if (stage_id == _BCM_FIELD_STAGE_LOOKUP) {

            rv = DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_DATA, 
                                             DRV_VM_FIELD_IVM_INGRESS_CTAG_STATUS, 
                                             f_ent->drv_entry, &new_data);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
            
            rv = DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_MASK, 
                                             DRV_VM_FIELD_IVM_INGRESS_CTAG_STATUS,
                                             f_ent->drv_entry, &new_mask);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }

        if ((new_data == 0x0) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
        } else if ((new_data == 0x3) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
        } else if ((new_data == 0x1) && (new_mask == 0x3)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
        } else if ((new_data == 0x3) && (new_mask == 0x1)) {
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
        } else {
            /* unexpected value */
            rv = BCM_E_INTERNAL;
            return rv;
        }

        return rv;
    }
#endif
    
    rv = bcm_port_dtag_mode_get(unit, 0, &dtag_mode);
    if (rv < 0) {
        return rv;
    }

    if (dtag_mode ==  BCM_PORT_DTAG_MODE_NONE) {
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_1QTAGGED, 
                f_ent->drv_entry, &new_data));
        
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_1QTAGGED, 
                f_ent->drv_entry, &new_mask));
        if (new_data & 0x1) {
            *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        }
        if (new_mask & 0x1) {
            *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        }
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            if (new_data & 0x2) {
                *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            }
            if (new_mask & 0x2) {
                *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            }  
        }
    } else {
        /* Check SP tag */
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SPTAGGED, 
                f_ent->drv_entry, &new_data));
        
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SPTAGGED, 
                f_ent->drv_entry, &new_mask));
        if (new_data & 0x1) {
            *data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        }
        if (new_mask & 0x1) {
            *mask = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        }
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            if (new_data & 0x2) {
                *data |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            }
            if (new_mask & 0x2) {
                *mask |= BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            }  
        }
        
        /* Check 1Q tag */
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_1QTAGGED, 
                f_ent->drv_entry, &new_data));
        
        BCM_IF_ERROR_RETURN(
            DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_1QTAGGED, 
                f_ent->drv_entry, &new_mask));

        if (new_data & 0x1) {
            *data = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
        }
        if (new_mask & 0x1) {
            *mask = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
        }
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            if (new_data & 0x2) {
                *data |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            }
            if (new_mask & 0x2) {
                *mask |= BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            }  
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_MHOpcode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMHOpcode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_MHOpcode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_HiGig_get
 * Purpose:
 *      Qualify on HiGig packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_HiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InterfaceClassPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InterfaceClassPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
#ifdef BCM_TB_SUPPORT

    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyInterfaceClassPort, 
                                     data, mask);
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InterfaceClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InterfaceClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InterfaceClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InterfaceClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DstClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_robo_field_qualify_IpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier protocol encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    uint32 data = 0, mask = 0;
    int    rv;
    
    rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyIpProtocolCommon, 
                               &data, &mask);
    SOC_IF_ERROR_RETURN(rv);

    rv = BCM_E_UNAVAIL;

#ifdef BCM_TB_SUPPORT
    if( SOC_IS_TBX(unit)) {
        rv = BCM_E_NONE;
        switch(data) {
            case 6:
                *protocol = bcmFieldIpProtocolCommonTcp;
                break;                
            case 17:
                *protocol = bcmFieldIpProtocolCommonUdp;
                break;
            case 2:
                *protocol = bcmFieldIpProtocolCommonIgmp;
                break;
            case 1:
                *protocol = bcmFieldIpProtocolCommonIcmp;
                break;
            case 58:
                *protocol = bcmFieldIpProtocolCommonIp6Icmp;
                break;
            case 0:
                *protocol = bcmFieldIpProtocolCommonIp6HopByHop;
                break;
            case 4:
                *protocol = bcmFieldIpProtocolCommonIpInIp;
                break;
            default:
                rv = BCM_E_UNAVAIL;
                break;
        }
    }
#endif

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {        
        rv = BCM_E_NONE;        
        switch (data) {
            case FP_BCM53242_L4_FRM_FMT_TCP:
                *protocol = bcmFieldIpProtocolCommonTcp;
                break;
            case FP_BCM53242_L4_FRM_FMT_UDP:
                *protocol = bcmFieldIpProtocolCommonUdp;
                break;
            default:
                rv = BCM_E_UNAVAIL;                
                break;
        }
    }

    return rv;
    
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerIpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier inner ip protocol encodnig.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerIpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Snap_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySnap
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Snap_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_snap_header_t *data, 
    bcm_field_snap_header_t *mask)
{
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int     rv;
        uint32              ref_data[2], ref_mask[2];

        rv = _robo_field_qual_value_get(unit, entry, bcmFieldQualifySnap, 
            &ref_data[0], &ref_mask[0]);

        data->type = ref_data[0] & 0x0000ffff;
        data->org_code = ((ref_data[0] & 0xffff0000) >> 16) |
            ((ref_data[1] & 0xff) << 16);

        mask->type = ref_mask[0] & 0x0000ffff;
        mask->org_code = ((ref_mask[0] & 0xffff0000) >> 16) |
            ((ref_mask[1] & 0xff) << 16);

        return rv;
    }  
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_qualify_Llc_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLlc
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Llc_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_llc_header_t *data, 
    bcm_field_llc_header_t *mask)
{
    _field_entry_t      *f_ent;
    int     rv = BCM_E_NONE;
    uint16 ref_data, ref_mask;
    uint32 frame_data, frame_mask;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyLlc, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    if (SOC_IS_TBX(unit)) {
        _field_group_t	*fg = NULL;
		
        /* L2 framing should be llc, L3 framing should be non-ip */
        rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_L3_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_data);   
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_L3_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_mask);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        if (!((frame_data == 0x3) && (frame_mask == 0x3))) {
            return rv;
        }

        rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_L2_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_data);   
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        rv = DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_L2_FRM_FORMAT, 
                    f_ent->drv_entry, &frame_mask);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        if (!((frame_data == 0x2) && (frame_mask == 0x3))) {
            return rv;
        }


        fg = f_ent->group;
        BCM_FIELD_QSET_ADD(fg->qset, bcmFieldQualifyEtherType);
        ref_data = 0;
        ref_mask = 0;
        /* the endianness had been handled in DRV_FP_QUAL_VALUE_GET */
        rv = bcm_robo_field_qualify_EtherType_get(unit, entry, 
            &ref_data, &ref_mask);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        data->dsap = (ref_data & 0xff00) >> 8; 
        data->ssap = (ref_data & 0xff); 
        data->control = 0;
        mask->dsap = (ref_mask & 0xff00) >> 8; 
        mask->ssap = (ref_mask & 0xff); 
        mask->control = 0; 
        
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_OuterTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OuterTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L3Routable_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3Routable
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L3Routable_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Qualifier ip framentation encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{

    _field_entry_t      *f_ent;
    int                 rv = BCM_E_NONE;
    uint32 frag_data, frag_mask, non_first_data, non_first_mask;

    rv = _robo_field_entry_qual_get(unit, entry, bcmFieldQualifyIpFrag, 
            &f_ent);
    BCM_IF_ERROR_RETURN(rv);
    
    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_FRAG, 
            f_ent->drv_entry, &frag_data));
    
    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_FRAG, 
            f_ent->drv_entry, &frag_mask));

    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &non_first_data));
    
    BCM_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_IP_NON_FIRST_FRAG, 
            f_ent->drv_entry, &non_first_mask));

    if (non_first_mask == 0) {
        *frag_info = bcmFieldIpFragNon;
    } else {
        if (non_first_data == 1) {
            *frag_info = bcmFieldIpFragNotFirst;
        } else {
            if(frag_mask == 0) {
                *frag_info =bcmFieldIpFragNonOrFirst;
            } else {
                *frag_info = bcmFieldIpFragFirst;
            }
        }
    }

    return rv;
 
}


/*
 * Function:
 *      bcm_robo_field_qualify_Vrf_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVrf
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Vrf_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L3Ingress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVrf
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L3Ingress_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_ExtensionHeaderType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ExtensionHeaderType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_ExtensionHeaderSubCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderSubCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ExtensionHeaderSubCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_ExtensionHeader2Type_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeader2Type
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ExtensionHeader2Type_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L4Ports_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4Ports
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L4Ports_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    if (SOC_IS_VO(unit)) {
        return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyL4Ports, 
                             data, mask);
    }

    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_MirrorCopy_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMirrorCopy
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_MirrorCopy_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_TunnelTerminated_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTunnelTerminated
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TunnelTerminated_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerSrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerSrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerSrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerSrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerDstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerDstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerSrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerSrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerDstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerDstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerTtl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerTtl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerTos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerTos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerDSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerDSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerIpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerIpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_InnerIpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Inner ip header fragmentation info.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InnerIpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DosAttack_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDosAttack
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_DosAttack_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpmcStarGroupHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpmcStarGroupHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpmcStarGroupHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_MyStationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMyStationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_MyStationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2PayloadFirstEightBytes_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyL2PayloadFirstEightBytes_get
 *          qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data1 - (OUT) Qualifier first four bytes of match data.
 *      data2 - (OUT) Qualifier last four bytes of match data.
 *      mask1 - (OUT) Qualifier first four bytes of match mask.
 *      mask2 - (OUT) Qualifier last four bytes of match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_L2PayloadFirstEightBytes_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data1,
    uint32 *data2,
    uint32 *mask1,
    uint32 *mask2)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_L3DestRouteHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestRouteHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L3DestRouteHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L3DestHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L3DestHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L3SrcHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3SrcHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L3SrcHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2CacheHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2CacheHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2CacheHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2StationMove_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2StationMove
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2StationMove_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2DestHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2DestHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2DestHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2SrcStatic_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcStatic
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2SrcStatic_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_L2SrcHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_L2SrcHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IngressStpState_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIngressStpState
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IngressStpState_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_ForwardingVlanValid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingVlanValid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_ForwardingVlanValid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/* Function: bcm_robo_field_group_config_create
 *
 * Purpose:
 *     Create a group with a mode (single, double, etc.), a port bitmap,
 *     group size and a Group ID. 
 * Parameters:
 *     unit - BCM device number.
 *     group_config - Group create attributes namely:
 *          flags       - (IN) Bits indicate which parameters have been
 *                             passed to API and should be used during group
 *                             creation.
 *          qset        - (IN) Field qualifier set
 *          priority    - (IN) Priority within allowable range,
 *                             or BCM_FIELD_GROUP_PRIO_ANY to automatically
 *                             assign a priority; each priority value may be
 *                             used only once
 *          mode        - (IN) Group mode (single, double, triple or Auto-wide)
 *          ports       - (IN) Ports where group is defined
 *          group       - (IN/OUT) Requested Group ID. If Group ID is not set,
 *                              then API allocates and returns the created
 *                              Group ID.
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_group_config_create(int unit,
    bcm_field_group_config_t *group_config)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcVirtualPortValid_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifySrcVirtualPortValid
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcVirtualPortValid_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstL3EgressNextHops_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyDstL3EgressNextHops
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_DstL3EgressNextHops_get(int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_robo_field_qualify_VlanTranslationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanTranslationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_VlanTranslationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IpAuth_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpAuth
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IpAuth_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyIpAuth, 
                                 data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_BigIcmpCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyBigIcmpCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Flag.
 *      size - (OUT) Size.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_BigIcmpCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag, 
    uint32 *size)
{
    int                 rv;
    uint32      data = 0, mask = 0;
  
    if (SOC_IS_ROBO53242(unit)|| SOC_IS_ROBO53262(unit)) {
        
        rv = _robo_field_qual_value32_get(unit, entry, bcmFieldQualifyBigIcmpCheck, 
                                     &data, &mask);
        BCM_IF_ERROR_RETURN(rv);

       if (data & mask) {
            rv = DRV_CFP_RANGE_GET(unit, DRV_CFP_RANGE_BIG_ICMP, 0, size, 0);
            BCM_IF_ERROR_RETURN(rv);
            *flag = 1;
       } else {
            *flag = 0;
       }
    } else {
        rv = BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *      bcm_robo_field_qualify_IcmpTypeCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIcmpTypeCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IcmpTypeCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return _robo_field_qual_value16_get(unit, entry,
                    bcmFieldQualifyIcmpTypeCode, data, mask);
}

/*
 * Function:
 *      bcm_robo_field_qualify_IgmpTypeMaxRespTime_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIgmpTypeMaxRespTime
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IgmpTypeMaxRespTime_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return _robo_field_qual_value16_get(unit, entry, 
                    bcmFieldQualifyIgmpTypeMaxRespTime, data, mask);
}


/*
 * Function:
 *      bcm_robo_field_qualify_TranslatedVlanFormat
 * Purpose:
 *      Set match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TranslatedVlanFormat(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IntPriority
 * Purpose:
 *      Set match criteria for bcmFieildQualifyIntPriority
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IntPriority(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_robo_field_qualify_Color
 * Purpose:
 *      Set match criteria for bcmFieildQualifyColor
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (IN) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Color(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 color)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_VnTag
 * Purpose:
 *      Add NIV VN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_VnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_CnTag
 * Purpose:
 *      Add QCN CN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_CnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_FabricQueueTag
 * Purpose:
 *      Add Fabric Queue tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_FabricQueueTag(int unit, bcm_field_entry_t entry,
                                     uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcModPortGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcModPortGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcModuleGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcModuleGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}
/*
 * Function:
 *      bcm_robo_field_qualify_TranslatedVlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_TranslatedVlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_IntPriority_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIntPriority
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_IntPriority_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_Color_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyColor
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (OUT) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_Color_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *color)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_VnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyVnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_VnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_CnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyCnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_CnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_FabricQueueTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyFabricQueueTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_FabricQueueTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcModuleGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcModuleGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcModPortGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcModPortGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_FlowId_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyFlowId
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_FlowId_get(int unit, bcm_field_entry_t entry,
                 uint16 *data, uint16 *mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value16_get(unit, entry, bcmFieldQualifyFlowId, 
                                     data, mask);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_InVPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInVPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_InVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyInVPort, 
                                     data, mask);
    } 
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_OutVPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutVPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_OutVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    FIELD_IS_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        return _robo_field_qual_value8_get(unit, entry, bcmFieldQualifyOutVPort, 
                                     data, mask);
    } 
#endif
    return BCM_E_UNAVAIL;
}

/* Function: bcm_robo_field_group_wlan_create_mode
 *     
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_group_wlan_create_mode(int unit, bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL; 
}

/* Function: bcm_robo_field_group_wlan_create_mode_id
 *     
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_group_wlan_create_mode_id(int unit, bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_CpuQueue
 * Purpose:
 *      Set match criteria for bcmFieldQualifyCpuQueue 
 *      qualifier for this field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (IN) CPU COS queue value.
 *      mask    - (IN) CPU COS match mask value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_CpuQueue(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_CpuQueue_get
 * Purpose:                                                                  
 *      Get match criteria for bcmFieldQualifyCpuQueue
 *      qualifier from the field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data  - (OUT) CPU COS Queue matched value.
 *      mask  - (OUT) CPU COS Queue matched mask value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_CpuQueue_get(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 *data,
                                   uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

STATIC int
_robo_field_group_status_calc(int unit, _field_group_t *fg)
{
    _field_stage_t              *stage_fc; /* Stage field control structure. */
    bcm_field_group_status_t    *status;
    
    /* Input parameters check. */
    if (NULL == fg) {
        return (BCM_E_PARAM);
    }    

    BCM_IF_ERROR_RETURN
        (_robo_field_stage_control_get(unit, fg->stage_id, &stage_fc));

    status = &fg->group_status;

    status->entries_total = stage_fc->tcam_sz;
    status->entries_free = stage_fc->field_shared_entries_free;
    status->counters_total = stage_fc->tcam_sz;
    status->counters_free = stage_fc->field_shared_entries_free + 
        (status->entry_count - status->counter_count);
    status->meters_total = stage_fc->tcam_sz;
    status->meters_free = stage_fc->field_shared_entries_free + 
        (status->entry_count - status->meter_count);

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _robo_field_entry_backup
 * Purpose:
 *     Backup a field entry configuration.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     entry_id   - (IN) Entry identifier
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_entry_backup(int unit, bcm_field_entry_t entry_id)
{
    _field_control_t    *fc;                /* Pointer to field control       */
    _field_entry_t      *f_ent_orig;        /* Field entry to be backed up.   */
    _field_entry_t      *f_ent_copy = NULL; /* Backup copy of field entry.    */
    _field_action_t     *fa_orig;           /* Pointer to entry action list.  */
    _field_action_t     *fa = NULL;         /* Field action descriptor.       */
    _field_action_t     *fa_free = NULL;    /* Field action descriptor.       */
    _field_stat_t       *f_st;              /* Internal Statisics descriptor. */
    _field_policer_t    *f_pl;              /* Internal policer descriptor.   */
    int                 rv;                 /* Operation return status.       */
    void                *drv_entry = NULL;
    int                 i;

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry_id, &f_ent_orig, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (NULL != f_ent_orig->ent_copy) {
        rv = _robo_field_entry_cleanup(unit, entry_id);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }
    }

    /* allocate and zero memory for field entry */
    f_ent_copy = sal_alloc(sizeof (_field_entry_t), "field_entry");
    if (f_ent_copy == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: allocation failure for field_entry\n")));
        return (BCM_E_MEMORY);
    }

    sal_memset(f_ent_copy, 0, sizeof (_field_entry_t));

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, f_ent_orig->fs->stage_id, 
            DRV_FIELD_ENTRY_MEM_ALLOC, NULL, NULL,&drv_entry);

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: DRV_FIELD_ENTRY_MEM_ALLOC %p\n"),
               drv_entry));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: allocation failure for field drv entry\n")));
        sal_free(f_ent_copy);
        FP_UNLOCK(fc);
        return rv;
    }

    f_ent_copy->drv_entry =  drv_entry;

    f_ent_copy->eid = f_ent_orig->eid;
    f_ent_copy->gid = f_ent_orig->gid;
    f_ent_copy->prio = f_ent_orig->prio;
    f_ent_copy->slice_idx = f_ent_orig->slice_idx;
    f_ent_copy->chain_idx = f_ent_orig->chain_idx;
    f_ent_copy->fs = f_ent_orig->fs;
    f_ent_copy->group = f_ent_orig->group;
    f_ent_copy->flags = f_ent_orig->flags;

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, f_ent_orig->fs->stage_id, 
        DRV_FIELD_ENTRY_MEM_COPY, f_ent_orig->drv_entry, 
        f_ent_copy->drv_entry,NULL);

    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }

    if (f_ent_orig->statistic.flags & _FP_ENTRY_STAT_VALID) {

        /* Get statistics entity  description structure. */
        rv = _robo_field_stat_get(unit, f_ent_orig->statistic.sid, &f_st);
        if (BCM_FAILURE(rv)) {
            goto cleanup;
        }

        f_ent_copy->statistic.sid = f_ent_orig->statistic.sid;

        /* Increment statistics entity reference counter. */
        f_st->sw_ref_count++;

        f_ent_copy->statistic.flags = f_ent_orig->statistic.flags;

    }

    /*  Copy meter, if it exists. */
    for (i = 0; i < _FP_POLICER_LEVEL_COUNT; i++)  {
        if (f_ent_orig->policer->flags & _FP_POLICER_VALID) {

            /* Get policer description structure. */
            rv = _robo_field_policer_get(unit,
                                        f_ent_orig->policer->pid,
                                        &f_pl);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }

            f_ent_copy->policer->pid = f_ent_orig->policer->pid;

            /* Increment policer reference counter. */
            f_pl->sw_ref_count++;

            f_ent_copy->policer->flags = f_ent_orig->policer->flags;
        }
    }

    for (fa_orig = f_ent_orig->actions;
        fa_orig != NULL;
        fa_orig = fa_orig->next) {
        rv = _robo_field_action_alloc(fa_orig->action,
                                fa_orig->param0,
                                fa_orig->param1,
                                &fa);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: failure in _robo_field_action_alloc()\n"),
                       unit));
            goto cleanup;
        }
        
        fa->next = f_ent_copy->actions;
        fa->inst_flg |= fa_orig->inst_flg;
        f_ent_copy->actions = fa;
    }

    f_ent_orig->ent_copy = f_ent_copy;

    FP_UNLOCK(fc);
    return (BCM_E_NONE);

cleanup:

    fa = f_ent_copy->actions;
    while (fa != NULL) {
        fa_free = fa;
        fa = fa->next;
        sal_free(fa_free);
    }

    if (f_ent_copy->drv_entry != NULL) {
        sal_free(f_ent_copy->drv_entry);
    }

    if (NULL != f_ent_copy) {
        sal_free(f_ent_copy);
    }

    FP_UNLOCK(fc);
    return (rv);
}

/*
 * Function:
 *     _robo_field_entry_restore
 * Purpose:
 *     Restore the configuration of a field entry from backup. Resources
 *     allocated for the backup entry are released by calling the API
 *     with BCM_FIELD_ENTRY_OPER_CLEANUP flag bit set.
 * Parameters:
 *     unit         - (IN) BCM device number
 *     entry_id   - (IN) Entry identifier
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_robo_field_entry_restore(int unit, bcm_field_entry_t entry_id)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent_orig;
    _field_entry_t      *f_ent_copy;
    _field_action_t     *fa_copy;
    _field_action_t     *fa = NULL;         /* Field action descriptor.       */
    _field_stat_t       *f_st;          /* Internal statisics descriptor. */
    _field_policer_t    *f_pl;          /* Internal policer descriptor.   */
    int                 i;
    int                 rv;


    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry_id, &f_ent_orig, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: failure in "
                               "_robo_field_entry_restore() - rv:%s\n"),
                   unit, bcm_errmsg(rv)));
        return (rv);
    }

    if (NULL == f_ent_orig->ent_copy) {
        FP_UNLOCK(fc);
        return (BCM_E_NOT_FOUND);
    }

    /* Detach any attached policers. */
    for (i = 0; i < _FP_POLICER_LEVEL_COUNT; i++)  {
        if (f_ent_orig->policer->flags & _FP_POLICER_VALID) {
            rv = bcm_robo_field_entry_policer_detach(unit,
                                                    f_ent_orig->eid,
                                                    i);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: failure in "
                                       "bcm_robo_field_entry_policer_detach() - rv:%s\n"),
                           unit, bcm_errmsg(rv)));
                return (rv);
            }
        }
    }

    /* Detach attached STATs. */
    if (f_ent_orig->statistic.flags & _FP_ENTRY_STAT_VALID) {
        rv = bcm_robo_field_entry_stat_detach(unit,
                                             f_ent_orig->eid,
                                             f_ent_orig->statistic.sid);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: failure in "
                                   "bcm_robo_field_stat_detach() - rv:%s\n"),
                       unit, bcm_errmsg(rv)));
            return (rv);
        }
    }

    /* Remove all actions set for this entry. */
    rv = bcm_robo_field_action_remove_all(unit, f_ent_orig->eid);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: failure in "
                               "bcm_robo_field_action_remove_all() - rv:%s\n"),
                   unit, bcm_errmsg(rv)));
        return (rv);
    }

    /* Clear qualifiers set for this entry. */
    rv = bcm_robo_field_qualify_clear(unit, f_ent_orig->eid);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: failure in "
                               "bcm_robo_field_qualify_clear() - rv:%s\n"),
                   unit, bcm_errmsg(rv)));
        return (rv);
    }

    /* Get the backup entry pointer. */
    f_ent_copy = f_ent_orig->ent_copy;

    f_ent_orig->eid = f_ent_copy->eid;
    f_ent_orig->gid = f_ent_copy->gid;
    f_ent_orig->prio = f_ent_copy->prio;
    f_ent_orig->slice_idx = f_ent_copy->slice_idx;
    f_ent_orig->chain_idx = f_ent_copy->chain_idx;
    f_ent_orig->fs = f_ent_copy->fs;
    f_ent_orig->group = f_ent_copy->group;
    f_ent_orig->flags = f_ent_copy->flags;

    rv = DRV_FP_ENTRY_MEM_CONTROL(unit, f_ent_copy->fs->stage_id, 
        DRV_FIELD_ENTRY_MEM_COPY, f_ent_copy->drv_entry, 
        f_ent_orig->drv_entry,NULL);

    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    if (f_ent_copy->statistic.flags & _FP_ENTRY_STAT_VALID) {

        /* Get statistics entity  description structure. */
        rv = _robo_field_stat_get(unit, f_ent_copy->statistic.sid, &f_st);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        }

        rv = bcm_robo_field_entry_stat_attach(unit, f_ent_orig->eid,
                                             f_ent_copy->statistic.sid);
        if (BCM_FAILURE(rv)) {
            FP_UNLOCK(fc);
            return (rv);
        } 

        f_ent_copy->statistic.flags
            = f_ent_orig->statistic.flags;

    }

    /*  Copy meter, if it exists. */
    for (i = 0; i < _FP_POLICER_LEVEL_COUNT; i++)  {

        if (f_ent_copy->policer[i].flags & _FP_POLICER_VALID) {

            /* Get policer description structure. */
            rv = _robo_field_policer_get(unit,
                                        f_ent_copy->policer[i].pid,
                                        &f_pl);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }

            /* Decrement policer entity reference counter. */
            f_pl->sw_ref_count--;

            rv = bcm_robo_field_entry_policer_attach(unit,
                                                    f_ent_orig->eid,
                                                    i,
                                                    f_ent_copy->policer[i].pid);
            if (BCM_FAILURE(rv)) {
                FP_UNLOCK(fc);
                return (rv);
            }
        }
    }

    for (fa_copy = f_ent_copy->actions;
        fa_copy != NULL;
        fa_copy = fa_copy->next) {
        rv = _robo_field_action_alloc(fa_copy->action,
                                fa_copy->param0,
                                fa_copy->param1,
                                &fa);
        if (BCM_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "FP(unit %d) Error: failure in _robo_field_action_alloc()\n"),
                      unit));
            FP_UNLOCK(fc);
            return (rv);
        }
        
        fa->next = f_ent_orig->actions;
        fa->inst_flg |= fa_copy->inst_flg;
        f_ent_orig->actions = fa;
    }

    /* Restore the entry priority. */
    rv = bcm_robo_field_entry_prio_set(unit, entry_id, f_ent_copy->prio);

    FP_UNLOCK(fc);
    return (rv);

}

/*
 * Function:
 *     _robo_field_entry_cleanup
 * Purpose:
 *     Release the resources allocated for the back entry.
 * Parameters:
 *     unit         - (IN) BCM device number
 *     entry_id   - (IN) Entry identifier
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_robo_field_entry_cleanup(int unit, bcm_field_entry_t entry_id)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent_orig;
    _field_entry_t      *f_ent_copy;
    _field_action_t     *fa = NULL;         /* Field action descriptor.       */
    _field_action_t     *fa_free = NULL;    /* Field action descriptor.       */
    int                 rv;

    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    FP_LOCK(fc);

    rv = _robo_field_entry_get(unit, entry_id, &f_ent_orig, _FP_SLICE_PRIMARY);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }
    
    if (NULL == f_ent_orig->ent_copy) {
        FP_UNLOCK(fc);
        /* No resources to free. */
        return (BCM_E_NONE);
    }

    f_ent_copy = f_ent_orig->ent_copy;

    if (f_ent_copy->drv_entry) {
        sal_free(f_ent_copy->drv_entry);
    }

    fa = f_ent_copy->actions;
    while (fa != NULL) {
        fa_free = fa;
        fa = fa->next;
        sal_free(fa_free);
    }

    sal_free(f_ent_orig->ent_copy);
    f_ent_orig->ent_copy = NULL;

    FP_UNLOCK(fc);
    return (BCM_E_NONE);
}

/*
 * Function:
 *     bcm_robo_field_entry_operation
 * Purpose:
 *     Perform entry backup, restore and backup copy free operations
 * Parameters:
 *     unit       - (IN) BCM device number.
 *     entry_oper - (IN) Pointer to field entry operation structure
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_robo_field_entry_operation(int unit, bcm_field_entry_oper_t *entry_oper)
{
    /* Input parameter check. */
    if (entry_oper == NULL) {
        return (BCM_E_PARAM);
    }

    /* Invalid operation check. */
    if (entry_oper->flags & (~BCM_FIELD_ENTRY_OPER_MASK)) {
        return (BCM_E_PARAM);
    }

    if (entry_oper->flags & BCM_FIELD_ENTRY_OPER_BACKUP) {
        BCM_IF_ERROR_RETURN(_robo_field_entry_backup(unit,
            entry_oper->entry_id));
    }

    if (entry_oper->flags & BCM_FIELD_ENTRY_OPER_RESTORE) {
        BCM_IF_ERROR_RETURN(_robo_field_entry_restore(unit,
            entry_oper->entry_id));
    }

    if (entry_oper->flags & BCM_FIELD_ENTRY_OPER_CLEANUP) {
        BCM_IF_ERROR_RETURN(_robo_field_entry_cleanup(unit,
            entry_oper->entry_id));
    }

    return (BCM_E_NONE);
}

#ifdef BROADCOM_DEBUG

/* Section: Field Debug */

/*
 * Function: bcm_field_show
 *
 * Purpose:
 *     Show current S/W state if compiled in debug mode.
 *
 * Parameters:
 *     unit - BCM device number
 *     pfx - Character string to prefix output lines
 *
 * Returns:
 *     Nothing.
 */


int
bcm_robo_field_show(int unit, const char *pfx)
{
    int                 idx;
    _field_control_t    *fc;
    _field_group_t      *fg;
    _field_range_t      *fr;
    _field_stage_t      *stage_fc;
    uint32              udf_num;
    int rv = BCM_E_NONE;

    if (_field_control[unit] == NULL) {
        LOG_CLI(("%s: BCM.%d: not initialized!\n", pfx, unit));
        return BCM_E_INIT;
    }
    fc = _field_control[unit];

    FP_LOCK(fc);    
    stage_fc = fc->stages;
    
    LOG_CLI(("%s:\tunit %d:", pfx, unit));

    while(stage_fc) {
        switch (stage_fc->stage_id) {
          case _BCM_FIELD_STAGE_INGRESS: 
              LOG_CLI(("PIPELINE STAGE INGRESS\n"));
              break;
          case _BCM_FIELD_STAGE_LOOKUP: 
              LOG_CLI(("PIPELINE STAGE LOOKUP\n"));
              break;
          case _BCM_FIELD_STAGE_EGRESS: 
              LOG_CLI(("PIPELINE STAGE EGRESS\n"));
              break; 
          default: 
              break;
        }

        LOG_CLI(("%s:\t      :tcam_sz=%d(%#x),", pfx, stage_fc->tcam_sz, stage_fc->tcam_sz));
        LOG_CLI((" tcam_slices=%d,", stage_fc->tcam_slices));
        LOG_CLI((" tcam_slice_sz=%d(%#x),", stage_fc->tcam_slice_sz, stage_fc->tcam_slice_sz));
        LOG_CLI((" tcam_ext_numb=%d,", fc->tcam_ext_numb));
        LOG_CLI(("\n"));

        if (_BCM_FIELD_STAGE_INGRESS  == stage_fc->stage_id) {

            /* Display any range checkers defined. */
            for (fr = stage_fc->ranges; fr != NULL; fr = fr->next) {
                _robo_field_range_dump(pfx, fr);
            }
        }
        stage_fc = stage_fc->next;
    }
    /* Print the any defined UDFs. */
    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return rv;
    }
    for (idx = 0; idx < udf_num; idx++) {
        if (fc->udf[idx].valid) {
        LOG_CLI(("%s:\tudf %d: use_count=%d, udf_num=UserDefined%d\n",
                 pfx, idx, fc->udf[idx].use_count,idx));
        }
    }

    /* Print the groups, along with their entries. */
    for (fg = fc->groups; fg != NULL ; fg = fg->next) {
        bcm_field_group_dump(unit, fg->gid);
    }

    FP_UNLOCK(fc);
    return BCM_E_NONE;
}

/*
 * Function: bcm_field_entry_dump
 *
 * Purpose:
 *     Show contents of a field entry.
 *
 * Parameters:
 *     unit  - BCM device number
 *     entry - Field entry to dump
 *
 * Returns:
 *     Nothing.
 */
int
bcm_robo_field_entry_dump(int unit, bcm_field_entry_t entry)
{
    _field_control_t    *fc;
    _field_entry_t      *f_ent;
    _field_action_t     *fa;


    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));

    if(BCM_FAILURE(_robo_field_entry_get(unit, entry, &f_ent,  _FP_SLICE_PRIMARY))) {
        LOG_CLI(("unit %d entry %d not initialized\n", unit, entry));
        return BCM_E_NOT_FOUND;
    }

    /* Dump the primary entry. */
    _robo_field_entry_phys_dump(unit, f_ent);

    /* Display action data */
    fa = f_ent->actions;
    while (fa != NULL) {
        LOG_CLI(("         action="));
        _robo_field_action_dump(fa);
        LOG_CLI(("\n"));
        fa = fa->next;
    }
    /* Display entry's meter. */
    LOG_CLI(("         policer="));
    _robo_field_policers_dump(unit, f_ent);
    LOG_CLI((",\n"));

    /* Display entry's counter. */
    LOG_CLI(("         statistics="));
    BCM_IF_ERROR_RETURN(_robo_field_stat_dump(unit, f_ent));
    LOG_CLI(("\n"));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_field_range_dump
 *
 * Purpose:
 *     Show contents of a range checker
 *
 * Parameters:
 *     fr - range checker structure pointer
 *
 * Returns:
 *     Nothing.
 */
STATIC void
_robo_field_range_dump(const char *pfx, _field_range_t *fr)
{
    LOG_CLI(("%s: Range ID=%d, flag=%#x, min=%d, max=%d, hw_index=%d(%#x), ",
                 pfx, fr->rid, fr->flags, fr->min, fr->max, fr->hw_index,
                 fr->hw_index));
}

STATIC void
_robo_field_entry_cfp_phys_dump(int unit, _field_entry_t *f_ent)
{
    int         idx;

    LOG_CLI(("          "));
    LOG_CLI((" KEY=0x"));
    if (SOC_IS_VO(unit)) {
        for (idx = 15; idx >=8; idx--) {
            LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[idx]));
            if (idx == 12) {
                LOG_CLI(("\n               "));
            }
        }
        LOG_CLI(("\n               "));
    }
    for (idx = 7; idx >=0; idx--) {
        LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_data[idx]));
        if (idx == 4) {
            LOG_CLI(("\n               "));
        }
    }

    LOG_CLI(("\n           "));
    LOG_CLI(("MASK=0x"));
    if (SOC_IS_VO(unit)) {
        for (idx = 15; idx >=8; idx--) {
            LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[idx]));
            if (idx == 12) {
                LOG_CLI(("\n               "));
            }
        }
        LOG_CLI(("\n               "));
    }
    for (idx = 7; idx >= 0; idx--) {
        LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->tcam_mask[idx]));
        if (idx == 4) {
            LOG_CLI(("\n               "));
        }  
    }
  
    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        LOG_CLI(("\n           "));
        LOG_CLI(("CHAIN KEY=0x"));
        if (SOC_IS_VO(unit)) {
            for (idx = 15; idx >=8; idx--) {
                LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->cfp_chain->tcam_data[idx]));
                if (idx == 12) {
                    LOG_CLI(("\n               "));
                }
            }
            LOG_CLI(("\n               "));
        }
        for (idx = 7; idx >=0; idx--) {
            LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->cfp_chain->tcam_data[idx]));
            if (idx == 4) {
                LOG_CLI(("\n               "));
            }
        }
    
        LOG_CLI(("\n           "));
        LOG_CLI(("CHAIN MASK=0x"));
        if (SOC_IS_VO(unit)) {
            for (idx = 15; idx >=8; idx--) {
                LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->cfp_chain->tcam_mask[idx]));
                if (idx == 12) {
                    LOG_CLI(("\n               "));
                }
            }
            LOG_CLI(("\n               "));
        }
        for (idx = 7; idx >= 0; idx--) {
            LOG_CLI(("%08x ", ((drv_cfp_entry_t *)(f_ent->drv_entry))->cfp_chain->tcam_mask[idx]));
            if (idx == 4) {
                LOG_CLI(("\n               "));
            }      
        }
    }
    LOG_CLI(("\n"));

}

/*
 * Function:
 *     _robo_field_entry_phys_dump
 *
 * Purpose:
 *     Show contents of a physical entry structure
 *
 * Parameters:
 *     unit  - BCM device number
 *     f_ent - Physical entry to dump
 *
 * Returns:
 *     Nothing.
 */
STATIC void
_robo_field_entry_phys_dump(int unit, _field_entry_t *f_ent)
{
    _field_group_t      *fg;

    fg = f_ent->fs->group;

    LOG_CLI(("EID %3d: gid=%d\n", f_ent->eid, f_ent->fs->group->gid));
    LOG_CLI(("         slice_idx=%#x, slice_id= %d, prio=%#x, ", f_ent->slice_idx,        
                 f_ent->slice_id, f_ent->prio));

    if (f_ent->flags & _FP_ENTRY_WITH_CHAIN) {
        LOG_CLI(("chain_idx=%#x, ", f_ent->chain_idx));
    }

    if (f_ent->flags &  _FP_ENTRY_INSTALL) {
        LOG_CLI(("Installed, "));
        if (!(f_ent->flags & _FP_ENTRY_ENABLED)) {
            LOG_CLI(("Disabled\n"));
        }
        else {
            LOG_CLI(("Enabled\n"));
        }
    } else {
        LOG_CLI(("Not installed\n"));
    }

    switch (fg->stage_id) {
        case _BCM_FIELD_STAGE_INGRESS:
            _robo_field_entry_cfp_phys_dump(unit, f_ent);
            break;
        case _BCM_FIELD_STAGE_LOOKUP:
            break;
        case _BCM_FIELD_STAGE_EGRESS:
            break;
        default:
            break;
    }

}

/*
 * Function: bcm_field_group_dump
 *
 * Purpose:
 *     Show contents of a field group.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group to dump
 *
 * Returns:
 *     Nothing.
 */

int
bcm_robo_field_group_dump(int unit, bcm_field_group_t group)
{
    _field_control_t    *fc;
    _field_group_t      *fg;
    _field_stage_t      *stage_fc;
    int                 enable;
    int                 rv = BCM_E_NONE;
    int entry_count;
    int entry_index;    
    bcm_field_entry_t *entry_ids;

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_robo_field_control_get(unit, &fc));
    
    FP_LOCK(fc);
    
    rv = _robo_field_group_get (unit,  group, &fg);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }          

    rv = _robo_field_stage_control_get(unit, fg->stage_id, &stage_fc);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }

    LOG_CLI(("GID %3d: unit=%d, gid=%d, mode=%s stage=%s, ", group,
                 fg->unit, fg->gid, _robo_field_group_mode_name(fg->mode),
                 _robo_field_stage_name(stage_fc)));
    rv = bcm_field_group_enable_get(unit, group, &enable);
    if (BCM_FAILURE(rv)) {
        FP_UNLOCK(fc);
        return (rv);
    }          

    if (enable) {
        LOG_CLI(("lookup=Enabled,\n"));
    } else {
        LOG_CLI(("lookup=Disabled,\n"));
    }
    _robo_field_qset_dump("         qset=", fg->qset, ",\n");

    /* Print the primary slice data. */
    LOG_CLI(("         slice_pri="));
    _robo_field_slice_dump(unit, "                    ", &fg->slices[0], "\n");

    /* Print group used resources status */
    if (BCM_SUCCESS(_robo_field_group_status_calc(unit, fg))) {
        LOG_CLI(("         group_status="));
        _robo_field_group_status_dump(&fg->group_status);
        LOG_CLI(("\n"));
    }


    /* Print group's entries */
    rv = bcm_robo_field_entry_multi_get(unit, group, 0, NULL, &entry_count);

    if (BCM_SUCCESS(rv))
    {
        entry_ids = sal_alloc(entry_count * sizeof (bcm_field_entry_t),
            "Entry ID array");

        if (entry_ids == NULL)
        {
            rv = BCM_E_MEMORY;
        }
        else
        {
            rv = bcm_robo_field_entry_multi_get(unit, group, entry_count,
                entry_ids, &entry_count);

            if (BCM_SUCCESS(rv))
            {
                for (entry_index = 0; entry_index < entry_count;
                    ++entry_index)
                {
                    bcm_robo_field_entry_dump(unit, entry_ids[entry_index]);
                }
            }

            sal_free(entry_ids);
        }
    }

    FP_UNLOCK(fc);
    return rv;
}

/*
 * Function:
 *     _robo_field_selcode_dump
 * Purpose:
 *     Output a set of field selects code.
 */
void
_robo_field_selcode_dump(int unit, char *prefix, _field_sel_t sel_codes,
                    char *suffix)
{
    LOG_CLI(("%s{", (prefix == NULL) ? "" : prefix));
    LOG_CLI(("FPF=%2d", sel_codes.fpf));
    if (sel_codes.slice_map) {
        LOG_CLI((" SLICE_MAP = 0x%x", sel_codes.slice_map));
    }
    LOG_CLI(("}%s", (suffix == NULL) ? "" : suffix));
}

/*
 * Function:
 *     _robo_field_slice_dump
 * Purpose:
 *     Output a slice worth of data, including any entries in the slice.
 */
STATIC void 
_robo_field_slice_dump(int unit, char *prefix, _field_slice_t *fs, char *suffix)
{
    int                 first_print;
    _qual_info_t        *qi = NULL;

    LOG_CLI(("{slice_numb=%d, ", fs->slice_numb));
    _robo_field_selcode_dump(unit, "tcam_config=", fs->sel_codes, ", ");

    if (fs->group != NULL) {
        LOG_CLI(("gid=%d,\n", fs->group->gid));
    } else {
        LOG_CLI(("gid=NONE,\n"));
    }

    LOG_CLI(("%sqset=", prefix));
    _robo_field_qset_dump("", fs->qset, "");
    LOG_CLI((",\n"));

    qi = fs->qual_list;
    LOG_CLI(("%squal_list={", prefix));
    first_print = 1;

    /* Output the qualifier info list*/
    while (qi != NULL) {
        LOG_CLI(("%s%s", 
                     (first_print ? "" : "->"), 
                     _robo_field_qual_name(qi->qid)));
        first_print = 0;
        qi = qi->next;
    }
    LOG_CLI(("}},%s", (suffix == NULL) ? "" : suffix));
}

/*
 * Function:
 *     _robo_field_qual_list_dump
 * Purpose:
 *     Output qualiers set in 'qset'.
 */
 void
_robo_field_qual_list_dump(char *prefix, _qual_info_t *qual_list, char *suffix)
{
    _qual_info_t        *qi;
    int                 first_print = 0;

    LOG_CLI(("%s{", (prefix == NULL) ? "" : prefix));
    qi = qual_list;
    first_print = 1;

    /* Output the qualifier info list*/
    while (qi != NULL) {
        LOG_CLI(("%s%s", 
                     (first_print ? "" : "->"), 
                     _robo_field_qual_name(qi->qid)));
        first_print = 0;
        qi = qi->next;
    }

    LOG_CLI(("}%s", (suffix == NULL) ? "" : suffix));
}

/*
 * Function:
 *     _field_qset_dump
 * Purpose:
 *     Output qualiers set in 'qset'.
 */
void
_robo_field_qset_dump(char *prefix, bcm_field_qset_t qset, char *suffix)
{
    bcm_field_qualify_t qual;
    int                 idx;
    int first_qual = 1, first_udf_id = 1;
    uint32              udf_num;
    int                 unit = 0;

    if (prefix == NULL) {
        prefix = "";
    }
    if (suffix == NULL) {
        suffix = "";
    }

    LOG_CLI(("%s{", prefix));
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if (BCM_FIELD_QSET_TEST(qset, qual)) {
            LOG_CLI(("%s%s", (first_qual ? "" : ", "), 
                         _robo_field_qual_name(qual)));
            first_qual = 0;
        }
    }

    if (DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num) < 0) {
        udf_num = 0;
    }
    for (idx = 0; idx < udf_num; idx++) {
        if (!SHR_BITGET(qset.udf_map, idx)) {
            continue;
        }
        LOG_CLI(("%s%d", (first_udf_id ? " : udf_id={" : ", "), idx));
        first_udf_id = 0;
    }
    if (first_udf_id == 0) {
        LOG_CLI(("}"));
    }

    LOG_CLI(("}%s", suffix));
}

/*
 * Function:
 *     _field_qset_debug
 * Purpose:
 *     Output qualier set in 'qset' for debug mode only.
 */
void
_robo_field_qset_debug(bcm_field_qset_t qset)
{
    bcm_field_qualify_t qual;
    int first_qual = 1;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META("{")));
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if (BCM_FIELD_QSET_TEST(qset, qual)) {
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META("%s%s"),
                         (first_qual ? "" : ", "), 
                         _robo_field_qual_name(qual)));
            first_qual = 0;
        }
    }
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META("}")));
}
/*
 * Function:
 *     _robo_field_stat_dump
 * Purpose:
 *     Output stat data for given entry.
 */
STATIC int
_robo_field_stat_dump(int unit, const _field_entry_t *f_ent)
{
    _field_stat_t        *f_st;
    int                  idx;
    int                  rv = BCM_E_NONE;
    char                 *sname[] = BCM_FIELD_STAT;


    /* Input parameters check. */
    if (f_ent == NULL) {
        return (BCM_E_PARAM); 
    }

    if (f_ent->statistic.flags  & _FP_ENTRY_STAT_VALID) {
        rv = _robo_field_stat_get (unit, f_ent->statistic.sid, &f_st);  
        if (BCM_FAILURE(rv)) {
            return (rv);
        }

        LOG_CLI(("{stat id %d  idx=%d }", f_st->sid, f_st->hw_index));
        for (idx = 0; idx < f_st->nstat; idx++) { 
            if (f_st->stat_arr[idx] < bcmFieldStatCount) {
                LOG_CLI(("{%s}", sname[f_st->stat_arr[idx]]));
            } 
        }
    } else {
        LOG_CLI(("NULL"));
    }
    return (rv);
}

/*
 * Function:
 *     _field_policers_dump
 * Purpose:

 *     Output meter data for given entry.
 */
STATIC void
_robo_field_policers_dump(int unit, _field_entry_t *f_ent)
{
    bcm_policer_t     policer_id;   /* Policer id.                  */
    _field_policer_t  *f_pl;        /* Internal policer descriptor. */
    int               idx;          /* Policers levels iterator.    */
    int               rv;           /* Operation return status.     */

    for (idx = 0; idx < _FP_POLICER_LEVEL_COUNT; idx++) {
        /* Read policer id from entry. */
        rv = bcm_robo_field_entry_policer_get(unit, f_ent->eid, 
                                             idx, &policer_id);
        if (BCM_E_NOT_FOUND == rv) {
            continue;
        }
        if (BCM_FAILURE(rv)) {
            return;
        }

        /* Get policer reference count. */
        rv = _robo_field_policer_get(unit, policer_id, &f_pl);
        if (BCM_FAILURE(rv)) {
            return;
        }
        LOG_CLI(("{"));

        if (bcmPolicerModeCommitted != f_pl->cfg.mode) {
            LOG_CLI(("peak_kbits_sec=%#x, peak_kbits_burst=%#x,",
                         f_pl->cfg.pkbits_sec, f_pl->cfg.pkbits_burst));
        } 
        if (bcmPolicerModePeak != f_pl->cfg.mode) {
            LOG_CLI((" commit_kbits_sec=%#x, commit_kbits_burst=%#x, ",
                         f_pl->cfg.ckbits_sec, f_pl->cfg.ckbits_burst));
        }

        LOG_CLI(("id %d mode=%#x, %s, %s}",
                 policer_id, f_pl->cfg.mode, 
                 (f_pl->cfg.flags & BCM_POLICER_COLOR_BLIND) ? "ColorBlind":"",
                 (f_pl->hw_flags & _FP_POLICER_DIRTY) ? "Dirty" : "Clean"));
    }
}

/*
 * Function:
 *     _robo_field_action_dump
 * Purpose:
 *     Output fields in a _field_action_s struct.
 */
STATIC void
_robo_field_action_dump(const _field_action_t *fa)
{
    if (fa == NULL) {
        LOG_CLI(("NULL"));
    } else {
        LOG_CLI(("{act=%s, param0=%d(%#x), param1=%d(%#x)},",
                     _robo_field_action_name(fa->action), fa->param0, fa->param0,
                     fa->param1, fa->param1));
    }
}
/*
 * Function:
 *     _robo_field_group_status_dump
 * Purpose:
 *     Output the fields in a bcm_field_group_status_s struct.
 */
STATIC void
_robo_field_group_status_dump(const bcm_field_group_status_t *gstat)
{
    LOG_CLI((" {entries_total=%d,",  gstat->entries_total));
    LOG_CLI((" entries_free=%d,",   gstat->entries_free));
    LOG_CLI((" entry_count=%d,",   gstat->entry_count));
    LOG_CLI(("\n                      "));
    LOG_CLI((" counters_total=%d,", gstat->counters_total));
    LOG_CLI((" counters_free=%d,",  gstat->counters_free));
    LOG_CLI((" counter_count=%d,",   gstat->counter_count));
    LOG_CLI(("\n                      "));
    LOG_CLI((" meters_total=%d,",   gstat->meters_total));
    LOG_CLI((" meters_free=%d,",   gstat->meters_free));
    LOG_CLI((" meter_count=%d}",   gstat->meter_count));    
}

#endif /* BROADCOM_DEBUG */
int bcm_robo_field_resync(int unit)
{
    return BCM_E_UNAVAIL;
}
#else  /* !BCM_FIELD_SUPPORT */



int
bcm_robo_field_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_detach(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_flush(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_control_get(int unit, bcm_field_control_t control, uint32 *state)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_control_set(int unit, bcm_field_control_t control, uint32 state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_create(int unit,
                           bcm_field_qset_t qset,
                           int pri,
                           bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_create_id(int unit,
                              bcm_field_qset_t qset,
                              int pri,
                              bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_create_mode(int unit,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_mode_t mode,
                                   bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_create_mode_id(int unit,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_mode_t mode,
                                   bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_mode_get(int unit,
                         bcm_field_group_t group,
                         bcm_field_group_mode_t *mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_set(int unit,
                        bcm_field_group_t group,
                        bcm_field_qset_t qset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_get(int unit,
                        bcm_field_group_t group,
                        bcm_field_qset_t *qset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_destroy(int unit,
                            bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_compress(int unit, bcm_field_group_t group)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_group_priority_set(int unit, bcm_field_group_t group,
                                 int priority)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_group_priority_get(int unit, bcm_field_group_t group,
                                 int *priority)
{
    return (BCM_E_UNAVAIL);
}


int
bcm_robo_field_group_status_get(int unit,
                               bcm_field_group_t group,
                               bcm_field_group_status_t *status)
{
    return BCM_E_UNAVAIL;
}

    int
bcm_robo_field_group_enable_set(int unit, bcm_field_group_t group, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_enable_get(int unit, bcm_field_group_t group, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_range_create_id(int unit,
                              bcm_field_range_t range,
                              uint32 flags,
                              bcm_l4_port_t min,
                              bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_range_group_create_id(int unit,
                                    bcm_field_range_t range,
                                    uint32 flags,
                                    bcm_l4_port_t min,
                                    bcm_l4_port_t max,
                                    bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_range_create(int unit,
                           bcm_field_range_t *range,
                           uint32 flags,
                           bcm_l4_port_t min,
                           bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_range_group_create(int unit,
                                 bcm_field_range_t *range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max,
                                 bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_range_get(int unit,
                        bcm_field_range_t range,
                        uint32 *flags,
                        bcm_l4_port_t *min,
                        bcm_l4_port_t *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_range_destroy(int unit,
                            bcm_field_range_t range)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_range_set(int unit,
                              bcm_field_group_t group,
                              uint32 flags,
                              bcm_l4_port_t min,
                              bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_range_get(int unit,
                              bcm_field_group_t group,
                              uint32 *flags,
                              bcm_l4_port_t *min,
                              bcm_l4_port_t *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_create(int unit,
                           bcm_field_group_t group,
                           bcm_field_entry_t *entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_create_id(int unit,
                              bcm_field_group_t group,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_multi_get(int unit, bcm_field_group_t group,
    int entry_size, bcm_field_entry_t *entry_array, int *entry_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_destroy(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_copy(int unit,
                         bcm_field_entry_t src_entry,
                         bcm_field_entry_t *dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_copy_id(int unit,
                            bcm_field_entry_t src_entry,
                            bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_install(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_reinstall(int unit,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_remove(int unit,
                           bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_entry_policer_attach(int unit, bcm_field_entry_t entry_id, 
                                   int level, bcm_policer_t policer_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_entry_policer_detach(int unit, bcm_field_entry_t entry_id, 
                                   int level)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_entry_policer_detach_all(int unit, bcm_field_entry_t entry_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_entry_policer_get(int unit, bcm_field_entry_t entry_id, 
                                int level, bcm_policer_t *policer_id)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_entry_prio_get(int unit, bcm_field_entry_t entry, int *prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_prio_set(int unit, bcm_field_entry_t entry, int prio)
{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_field_stat_create(int unit, bcm_field_group_t group, int nstat, 
                          bcm_field_stat_t *stat_arr, int *stat_id) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_create_id(int unit, bcm_field_group_t group, int nstat, 
                          bcm_field_stat_t *stat_arr, int stat_id) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_destroy(int unit, int stat_id)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_size(int unit, int stat_id, int *stat_size)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_config_get(int unit, int stat_id, int nstat, 
                              bcm_field_stat_t *stat_arr)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_get(int unit, int stat_id, bcm_field_stat_t stat, 
                       uint64 *value)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_stat_get32(int unit, int stat_id, 
                             bcm_field_stat_t stat, uint32 *value)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_stat_multi_get(int unit, int stat_id, int nstat, 
                                 bcm_field_stat_t *stat_arr, uint64 *value_arr)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_stat_multi_get32(int unit, int stat_id, int nstat, 
                                   bcm_field_stat_t *stat_arr, 
                                   uint32 *value_arr)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_entry_stat_attach(int unit, bcm_field_entry_t entry, 
                                    int stat_id)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_entry_stat_detach(int unit, bcm_field_entry_t entry,
                                    int stat_id)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_entry_stat_get(int unit, bcm_field_entry_t entry, int *stat_id)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_set(int unit, int stat_id, bcm_field_stat_t stat, 
                       uint64 value)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_stat_set32(int unit, int stat_id, bcm_field_stat_t stat, 
                         uint32 value)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_all_set(int unit, int stat_id, uint64 value)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_stat_all_set32(int unit, int stat_id, uint32 value)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_clear(int unit, bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InPort(int unit, bcm_field_entry_t entry,
                             bcm_port_t data, bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_OutPort(int unit, bcm_field_entry_t entry,
                             bcm_port_t data, bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_qualify_InPorts(int unit, bcm_field_entry_t entry,
                              bcm_pbmp_t data, bcm_pbmp_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Drop(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcPort(int unit, bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcTrunk(int unit, bcm_field_entry_t entry,
                               bcm_trunk_t data, bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstPort(int unit, bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstTrunk(int unit, bcm_field_entry_t entry,
                               bcm_trunk_t data, bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_L4SrcPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_L4DstPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_OuterVlan(int unit, bcm_field_entry_t entry,
                                bcm_vlan_t data, bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerVlan(int unit, bcm_field_entry_t entry,
                                bcm_vlan_t data, bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_EtherType(int unit, bcm_field_entry_t entry,
                                uint16 data, uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_IpProtocol(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_IpInfo(int unit, bcm_field_entry_t entry,
                             uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_PacketRes(int unit, bcm_field_entry_t entry,
                                uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DSCP(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_IpFlags(int unit, bcm_field_entry_t entry,
                              uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_TcpControl(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Ttl(int unit, bcm_field_entry_t entry,
                          uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_RangeCheck(int unit, bcm_field_entry_t entry,
                 bcm_field_range_t range, int invert)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_L2Format(int unit, bcm_field_entry_t entry,
                                bcm_field_L2Format_t type)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_VlanFormat(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_DstIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Ip6NextHeader(int unit, bcm_field_entry_t entry,
                                    uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Ip6TrafficClass(int unit, bcm_field_entry_t entry,
                                      uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Ip6FlowLabel(int unit, bcm_field_entry_t entry,
                                   uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Ip6HopLimit(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_SrcMac(int unit, bcm_field_entry_t entry,
                             bcm_mac_t data, bcm_mac_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InterfaceClassL2(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InterfaceClassL3(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InterfaceClassPort(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_SrcClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstMac(int unit, bcm_field_entry_t entry,
                             bcm_mac_t data, bcm_mac_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_ForwardingType(int unit, bcm_field_entry_t entry,
                             bcm_field_ForwardingType_t data)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_IpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_MHOpcode(int unit, bcm_field_entry_t entry,
                               uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_Decap(int unit, bcm_field_entry_t entry,
                            bcm_field_decap_t decap)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_HiGig(int unit, bcm_field_entry_t entry,
                            uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_DstHiGig(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstHiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIpEqualDstIp(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_EqualL4Port(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_TcpSequenceZero(int unit, bcm_field_entry_t entry,
                            uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_TcpHeaderSize(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_qualify_IpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_Snap(int unit, bcm_field_entry_t entry,
                            bcm_field_snap_header_t data, 
                            bcm_field_snap_header_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_Llc(int unit, bcm_field_entry_t entry,
                           bcm_field_llc_header_t data, 
                           bcm_field_llc_header_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerTpid(int unit, bcm_field_entry_t entry,
                                uint16 tpid)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_OuterTpid(int unit, bcm_field_entry_t entry,
                                uint16 tpid)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_action_add(int unit,
                         bcm_field_entry_t entry,
                         bcm_field_action_t action,
                         uint32 param0,
                         uint32 param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_action_get(int unit,
                         bcm_field_entry_t entry,
                         bcm_field_action_t action,
                         uint32 *param0,
                         uint32 *param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_action_remove(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_action_remove_all(int unit,
                                bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_field_action_ports_add(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_action_ports_get(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_action_delete(int unit, bcm_field_entry_t entry,
                       bcm_field_action_t action,
                       uint32 param0, uint32 param1)
{
   return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_OutPorts(int unit, bcm_field_entry_t entry,
                               bcm_pbmp_t data, bcm_pbmp_t mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_Vrf(int unit, bcm_field_entry_t entry,
                              uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeaderType(int unit, 
                                               bcm_field_entry_t entry,
                                               uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeader2Type(int unit, 
                                               bcm_field_entry_t entry,
                                               uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_L3Ingress(int unit, bcm_field_entry_t entry,
                              uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_L4Ports(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_MirrorCopy(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_TunnelTerminated(int unit, bcm_field_entry_t entry,
                                           uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int bcm_robo_field_qualify_ExtensionHeaderSubCode(int unit, 
                                                  bcm_field_entry_t entry,
                                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IpFrag(int unit, bcm_field_entry_t entry,
                                  bcm_field_IpFrag_t frag_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_L3Routable(int unit, bcm_field_entry_t entry,
                                  uint8 data, uint8 mask)
{    
    return BCM_E_UNAVAIL;
}
int
bcm_robo_field_qualify_Tos(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_field_group_port_create_mode(int unit, bcm_port_t port,
                                         bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_port_create_mode_id(int unit, bcm_port_t port,
                                        bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_ports_create_mode(int unit, bcm_pbmp_t pbmp,
                                      bcm_field_qset_t qset, int pri,
                                      bcm_field_group_mode_t mode,
                                      bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_ports_create_mode_id(int unit, bcm_pbmp_t pbmp,
                                         bcm_field_qset_t qset, int pri,
                                         bcm_field_group_mode_t mode,
                                         bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_group_action_set(int               unit, 
                               bcm_field_group_t group, 
                               bcm_field_aset_t  aset
                               )
{
    return BCM_E_UNAVAIL;
}


int 
bcm_robo_field_group_action_get(int               unit, 
                               bcm_field_group_t group, 
                               bcm_field_aset_t  *aset
                               )
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_counter_set32(int unit, bcm_field_entry_t entry, int counter_num,
                            uint32 count)
{
    return BCM_E_UNAVAIL;
}
int bcm_robo_field_resync(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_show(int unit, const char *pfx)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_entry_dump(int unit, bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_group_dump(int unit, bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerSrcIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp(int unit, bcm_field_entry_t entry,
                            bcm_ip_t data, bcm_ip_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerSrcIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp6(int unit, bcm_field_entry_t entry,
                             bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerSrcIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDstIp6High(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerTos(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerDSCP(int unit, bcm_field_entry_t entry,
                           uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerIpProtocol(int unit, bcm_field_entry_t entry,
                                 uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}


int bcm_robo_field_qualify_InnerIpFrag(int unit, bcm_field_entry_t entry,
                                  bcm_field_IpFrag_t frag_info)
{
     return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_InnerTtl(int unit, bcm_field_entry_t entry,
                          uint8 data, uint8 mask)
{
     return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_DosAttack(int unit, bcm_field_entry_t entry, 
                                uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IpmcStarGroupHit(int unit, bcm_field_entry_t entry, 
                                       uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_MyStationHit(int unit, bcm_field_entry_t entry, 
                                       uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_L2PayloadFirstEightBytes(int unit, bcm_field_entry_t entry,
                                                uint32 data1, uint32 data2,
                                                uint32 mask1, uint32 mask2)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L3DestRouteHit(int unit, bcm_field_entry_t entry, 
                                      uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L3DestHostHit(int unit, bcm_field_entry_t entry, 
                                     uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L3SrcHostHit(int unit, bcm_field_entry_t entry, 
                                    uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2CacheHit(int unit, bcm_field_entry_t entry, 
                                  uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2StationMove(int unit, bcm_field_entry_t entry, 
                                    uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2DestHit(int unit, bcm_field_entry_t entry, 
                                uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2SrcStatic(int unit, bcm_field_entry_t entry, 
                                  uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_L2SrcHit(int unit, bcm_field_entry_t entry, 
                               uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IngressStpState(int unit,bcm_field_entry_t entry, 
                                      uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_ForwardingVlanValid(int unit, bcm_field_entry_t entry, 
                                          uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_SrcVirtualPortValid(int unit, bcm_field_entry_t entry,
                                           uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstL3EgressNextHops(int unit, bcm_field_entry_t entry,
                                          uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_VlanTranslationHit(int unit, bcm_field_entry_t entry, 
                                         uint8 data, uint8 mask) 
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IpAuth(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_BigIcmpCheck(int unit, bcm_field_entry_t entry,
                                         uint32 flag, uint32 size)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IcmpTypeCode(int unit, bcm_field_entry_t entry,
                                         uint16 data,uint16 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_IgmpTypeMaxRespTime(int unit, bcm_field_entry_t entry,
                                         uint16 data,uint16 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_InnerL4DstPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerL4SrcPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_InnerIp6FlowLabel(int unit, bcm_field_entry_t entry,
                                   uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_DstL3Egress(int unit, 
                                   bcm_field_entry_t entry, 
                                   bcm_if_t if_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMulticastGroup(int unit, 
                                         bcm_field_entry_t entry, 
                                         bcm_gport_t group)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_SrcMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}



int 
bcm_robo_field_qualify_SrcWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_DstWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_Loopback(int unit, 
                               bcm_field_entry_t entry, 
                               uint8 data, 
                               uint8 mask)
{
    return BCM_E_UNAVAIL; 
}



int 
bcm_robo_field_qualify_LoopbackType(int unit, 
                                   bcm_field_entry_t entry, 
                                   bcm_field_LoopbackType_t loopback_type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TunnelType(int unit, 
                                 bcm_field_entry_t entry, 
                                 bcm_field_TunnelType_t tunnel_type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_FlowId(int unit, bcm_field_entry_t entry,
                 uint16 data, uint16 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OutVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_LoopbackType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_LoopbackType_t *loopback_type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TunnelType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_TunnelType_t *tunnel_type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstL3Egress_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_if_t *if_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstMulticastGroup_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *group)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Loopback_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OutPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OutPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Drop_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerL4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerL4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_EtherType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_EqualL4Port_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpInfo_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_PacketRes_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Tos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpFlags_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TcpControl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TcpSequenceZero_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TcpHeaderSize_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Ttl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_RangeCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    int max_count, 
    bcm_field_range_t *range, 
    int *invert, 
    int *count)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcIpEqualDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Ip6NextHeader_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Ip6TrafficClass_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerIp6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Ip6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Ip6HopLimit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_InnerIpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_ForwardingType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_ForwardingType_t *type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2Format_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_L2Format_t *type)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_VlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_MHOpcode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_HiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InterfaceClassPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InterfaceClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InterfaceClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerIpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Snap_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_snap_header_t *data, 
    bcm_field_snap_header_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Llc_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_llc_header_t *data, 
    bcm_field_llc_header_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L3Routable_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Vrf_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_L3Ingress_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_ExtensionHeaderType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_ExtensionHeaderSubCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_ExtensionHeader2Type_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L4Ports_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_MirrorCopy_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TunnelTerminated_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerSrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerSrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerDstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerSrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerDstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerTtl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerTos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerDSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerIpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerIpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DosAttack_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpmcStarGroupHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_MyStationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_L2PayloadFirstEightBytes_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data1,
    uint32 *data2,
    uint32 *mask1,
    uint32 *mask2)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_L3DestRouteHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L3DestHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L3SrcHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2CacheHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2StationMove_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2DestHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2SrcStatic_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_L2SrcHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IngressStpState_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_ForwardingVlanValid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/* Function: bcm_robo_field_group_config_create
 *
 * Purpose:
 *     Create a group with a mode (single, double, etc.), a port bitmap,
 *     group size and a Group ID. 
 * Parameters:
 *     unit - BCM device number.
 *     group_config - Group create attributes namely:
 *          flags       - (IN) Bits indicate which parameters have been
 *                             passed to API and should be used during group
 *                             creation.
 *          qset        - (IN) Field qualifier set
 *          priority    - (IN) Priority within allowable range,
 *                             or BCM_FIELD_GROUP_PRIO_ANY to automatically
 *                             assign a priority; each priority value may be
 *                             used only once
 *          mode        - (IN) Group mode (single, double, triple or Auto-wide)
 *          ports       - (IN) Ports where group is defined
 *          group       - (IN/OUT) Requested Group ID. If Group ID is not set,
 *                              then API allocates and returns the created
 *                              Group ID.
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_robo_field_group_config_create(int unit,
    bcm_field_group_config_t *group_config)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_field_qualify_SrcVirtualPortValid_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifySrcVirtualPortValid
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_field_qualify_SrcVirtualPortValid_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_robo_field_qualify_DstL3EgressNextHops_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyDstL3EgressNextHops
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_field_qualify_DstL3EgressNextHops_get(int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_VlanTranslationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IpAuth_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_BigIcmpCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag, 
    uint32 *size)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IcmpTypeCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IgmpTypeMaxRespTime_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualifier_delete(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_qualify_t qual_id)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TranslatedVlanFormat(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IntPriority(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_Color(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 color)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_VnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_CnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_field_qualify_FabricQueueTag(int unit, bcm_field_entry_t entry,
                                     uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_SrcModPortGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcModuleGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_TranslatedVlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_IntPriority_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Color_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *color)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_VnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_CnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_FabricQueueTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcModPortGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_SrcModuleGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_FlowId_get(int unit, bcm_field_entry_t entry,
                 uint16 *data, uint16 *mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_InVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_field_qualify_OutVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_field_group_wlan_create_mode(int unit, bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_group_wlan_create_mode_id(int unit, bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_OuterVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_OuterVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_InnerVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_OuterVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_InnerVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_InnerVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_action_mac_add(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t mac)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_action_mac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t *mac)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_group_traverse(int unit, bcm_field_group_traverse_cb callback,
                              void *user_data)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_ForwardingVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Vpn_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t *data, 
    bcm_vpn_t *mask)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_robo_field_qualify_ForwardingVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_Vpn(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t data, 
    bcm_vpn_t mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_SrcIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_DstIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qualify_SrcIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_robo_field_qualify_DstIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_CpuQueue(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_qualify_CpuQueue_get(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 *data,
                                   uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_robo_field_data_qualifier_create(int unit,  
                                 bcm_field_data_qualifier_t *data_qualifier)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_multi_get(int unit, int qual_size, int *qual_array, int *qual_count)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_get(int unit, int qual_id, bcm_field_data_qualifier_t *qual)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_destroy(int unit, int qual_id)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_destroy_all(int unit)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_robo_field_qset_data_qualifier_get(int unit, bcm_field_qset_t qset, int qual_max,
                                  int *qual_arr, int *qual_count) 
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qset_data_qualifier_add(int unit, bcm_field_qset_t *qset,  
                                      int qual_id)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_qualify_data(int unit, bcm_field_entry_t eid, int qual_id,
                           uint8 *data, uint8 *mask, uint16 length)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_ip_protocol_add(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_ip_protocol_delete(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_ethertype_add(int unit,  int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_ethertype_delete(int unit, int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_packet_format_add(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_robo_field_data_qualifier_packet_format_delete(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_field_qualify_SrcGport
 * Purpose:
 *     Add Source ModPort or MPLS/MiM/WLAN port qualification to a
 *     field entry.
 * Parameters:
 *     unit     - (IN) BCM device number
 *     entry    - (IN) Field Entry id.
 *     port_id  - (IN) Source Generic Logical port or virtual port id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qualify_SrcGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_field_qualify_SrcGport_get
 * Purpose:
 *     Get Source ModPort or MPLS/MiM/WLAN gport value from a
 *     field entry.
 * Parameters:
 *     unit     - (IN) BCM device number
 *     entry    - (IN) Field Entry id.
 *     port_id  - (OUT) ModPort Gport or MPLS/MiM/WLAN Gport ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_field_qualify_SrcGport_get(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_gport_t *port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_field_entry_operation
 * Purpose:
 *     Perform entry backup, restore and backup copy free operations
 * Parameters:
 *     unit       - (IN) BCM device number.
 *     entry_oper - (IN) Pointer to field entry operation structure
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_robo_field_entry_operation(int unit, bcm_field_entry_oper_t *entry_oper)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function: bcm_field_entry_enable_set
 *
 * Purpose:
 *     Enable/Disable an entry from the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to be enabled/disabled
 *     enable_flag - Flag to enable or disable
 *
 * Returns:
 *     BCM_E_XXX
 *
 * Notes:
 *     This does not destroy the entry, nor deallocate any related resources;
 *     it only enables/disables a rule from hardware table using VALIDf of the
 *     corresponding hardware entry. To deallocate the memory used by the entry
 *     call bcm_field_entry_destroy.
 */
int
bcm_robo_field_entry_enable_set(int unit, bcm_field_entry_t entry, int enable_flag)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function: bcm_field_entry_enable_get
 *
 * Purpose:
 *     Get the Enable or Disable status of a field Entry.
 *
 * Parameters:
 *     unit - (IN)BCM device number
 *     entry - (IN)Entry to be checked for Enabled status
 *     enable_flag - (OUT)Status(Enable/Disable) of the given entry
 *
 * Returns:
 *     BCM_E_XXX
 *
 */
int
bcm_robo_field_entry_enable_get(int unit, bcm_field_entry_t entry, int *enable_flag)
{
    return (BCM_E_UNAVAIL);
}

#endif  /* !BCM_FIELD_SUPPORT */
