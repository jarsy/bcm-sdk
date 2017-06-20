/*
* $Id: field.c,v 1.42 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Field Processor APIs
 *
 * Purpose:
 *     Implement 'Field Processor' (FP) API for BCM88030 (SBX Caladan3 +
 *     Guadalupe-2000) G3P1 forwarder microcode and classifier picocode (plus
 *     other undocumented arcana).
 */

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#define _SBX_CALADAN3_FIELD_H_NEEDED_ TRUE

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/field.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/mirror.h>

#include <shared/bitop.h>
#include <shared/idxres_fl.h>

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g3p1/g3p1_cmu.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1.h>

#include <soc/sbx/caladan3.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/mirror.h>
#include <bcm_int/sbx/caladan3/field.h>
#include <shared/idxres_afl.h>
#include <bcm_int/sbx/caladan3/policer.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/mcast.h>

_bcm_c3_rce_action_arg_t ingressActionMap[socC3RceActionCount];
_bcm_c3_rce_action_arg_t egressActionMap[socC3RceActionCount];

/*
 *  These values are used when manipulating policers...
 */
#define _SBX_CALADAN3_FIELD_POLICER_READ 1
#define _SBX_CALADAN3_FIELD_POLICER_CLEAR 0
#define _SBX_CALADAN3_FIELD_POLICER_WRITE -1

/*
 *  These macros deal with the SBX port bitmap of a rule.
 */
#define FIELD_PBMP_SBX_LEN 7
#define FIELD_PBMP_SBX_MAX (FIELD_PBMP_SBX_LEN - 1)
#define FIELD_PBMP_TO_SBX(_sbxPBmp, _bcmPBmp) \
    do { \
        unsigned int _i1; \
        unsigned int _i2; \
        sal_memset(&(_sbxPBmp), 0xFF, sizeof(_sbxPBmp)); \
        for (_i1 = 0, _i2 = 0; \
             (_i1 < _SHR_PBMP_WORD_MAX) && (_i2 < FIELD_PBMP_SBX_LEN); \
             _i2++) { \
            (_sbxPBmp)[FIELD_PBMP_SBX_MAX - _i2] = ((~(_SHR_PBMP_WORD_GET(_bcmPBmp, _i1))) >> ((_i2 & 0x03) * 8)) & 0xFF; \
            if (3 == (_i2 & 3)) { \
                _i1++; \
            } \
        } \
    } while (0)
#define FIELD_PBMP_FROM_SBX(_bcmPBmp, _sbxPBmp, _unit) \
    do { \
        unsigned int _i1; \
        unsigned int _i2; \
        uint32 _pbTmp; \
        sal_memset(&(_bcmPBmp), 0x00, sizeof(_bcmPBmp)); \
        for (_i1 = 0, _i2 = 0, _pbTmp = 0; \
             (_i1 < _SHR_PBMP_WORD_MAX) && (_i2 < FIELD_PBMP_SBX_LEN); \
             _i2++) { \
            _pbTmp |= (((~(_sbxPBmp[FIELD_PBMP_SBX_MAX - _i2])) & 0xFF) << ((_i2 & 3) * 8)); \
            if (3 == (_i2 & 3)) { \
                _SHR_PBMP_WORD_SET(_bcmPBmp, _i1, _pbTmp); \
                _pbTmp = 0; \
                _i1++; \
            } \
        } \
        if ( (3 != ((_i2 - 1) & 3)) && (_i1 < _SHR_PBMP_WORD_MAX) ){ \
            _SHR_PBMP_WORD_SET(_bcmPBmp, _i1, _pbTmp); \
        } \
        BCM_PBMP_AND(_bcmPBmp, PBMP_ALL(_unit)); \
    } while (0)
#define FIELD_PBMP_SBX_FORMAT "%02X %02X %02X %02X %02X %02X %02X"
#define FIELD_PBMP_SBX_SHOW(_pbmp) \
    (_pbmp)[0], \
    (_pbmp)[1], \
    (_pbmp)[2], \
    (_pbmp)[3], \
    (_pbmp)[4], \
    (_pbmp)[5], \
    (_pbmp)[6]

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Entry management
 *
 */

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_alloc_id
 *  Purpose
 *    Get a specific entry description off the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_entry_index entry = which entry to get
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_entry_alloc_id(_bcm_caladan3_g3p1_field_glob_t *glob,
                                    const _bcm_caladan3_field_entry_index entry)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;

    /* check free count */
    if (!(glob->entryFreeCount)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("no available entries on unit %d\n"),
                   glob->unit));
        return BCM_E_RESOURCE;
    }
    /* check whether this entry is available */
    thisEntry = &(glob->entry[entry]);
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry ID %08X is already in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_EXISTS;
    }
    /* remove the entry from the list */
    if (thisEntry->nextEntry < glob->entryTotal) {
        /* not end of list */
        glob->entry[thisEntry->nextEntry].prevEntry = thisEntry->prevEntry;
    }
    if (thisEntry->prevEntry < glob->entryTotal) {
        /* not head of list */
        glob->entry[thisEntry->prevEntry].nextEntry = thisEntry->nextEntry;
    } else {
        /* head of list */
        glob->entryFreeHead = thisEntry->nextEntry;
    }
    glob->entryFreeCount--;
    /* initial setup of entry */
    thisEntry->prevEntry = glob->entryTotal;
    thisEntry->nextEntry = glob->entryTotal;
    thisEntry->entryFlags = _CALADAN3_G3P1_ENTRY_VALID;
    /* done */
    FIELD_EVERB((BSL_META("unit %d entry %08X allocated from free list\n"),
                 glob->unit,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_alloc
 *  Purpose
 *    Get an entry description off the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_caladan3_field_entry_index *entry = where to put entry ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 *    Destroys current value at entry, even if it fails.
 */
static int
_bcm_caladan3_g3p1_field_entry_alloc(_bcm_caladan3_g3p1_field_glob_t *glob,
                                 _bcm_caladan3_field_entry_index *entry)
{
    /* get head of list */
    *entry = glob->entryFreeHead;
    /* allocate that */
    return _bcm_caladan3_g3p1_field_entry_alloc_id(glob, *entry);
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_free
 *  Purpose
 *    Return an entry description to the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_entry_index entry = entry ID to free
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; clears most.
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_entry_free(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_entry_index entry)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;

    /* check valid entry ID */
    if (glob->entryTotal <= entry) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry ID %08X invalid\n"),
                   glob->unit,
                   entry));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    /* make sure entry is not already free */
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* trying to free already free entry */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is already free\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* clear the entry */
    sal_memset(thisEntry, 0, sizeof(*thisEntry));
    /* set up some fields specifically */
    thisEntry->prevEntry = glob->entryTotal;
    thisEntry->prevEntryRb = glob->entryTotal;
    thisEntry->nextEntryRb = glob->entryTotal;
    thisEntry->group = glob->groupTotal;
    /* insert the entry as head of free list */
    thisEntry->nextEntry = glob->entryFreeHead;
    glob->entryFreeHead = entry;
    if (thisEntry->nextEntry < glob->entryTotal) {
        /* not only one in list */
        glob->entry[thisEntry->nextEntry].prevEntry = entry;
    }
    glob->entryFreeCount++;
    /* done */
    FIELD_EVERB((BSL_META("unit %d entry %08X returned to free list\n"),
                 glob->unit,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_rules_clear_actions
 *  Purpose
 *    Clear all actions from the rule associated with an entry.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_entry_index entry = which entry to prepare
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others zeroed.
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_rules_clear_actions(_bcm_caladan3_g3p1_field_glob_t *glob,
                                         const _bcm_caladan3_field_entry_index entry)
{
    _bcm_caladan3_g3p1_field_entry_t        *thisEntry;
    int                                     result;

    thisEntry = &(glob->entry[entry]);

    /* Clear all actions */
    result = soc_c3_rce_entry_action_clear(glob->unit, entry);

    FIELD_EVERB((BSL_META("set no actions for unit %d group %08X"
                            " entry %08X\n"),
                 glob->unit,
                 thisEntry->group,
                 entry));
    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_add
 *  Purpose
 *    Add specified entry to speicifed group, priority order (as last entry of
 *    same priority when there are others of this entry's priority.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_group_index group = which group for add
 *    (in) _bcm_caladan3_field_entry_index entry = which entry to add
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */

static int
_bcm_caladan3_g3p1_field_entry_add(_bcm_caladan3_g3p1_field_glob_t *glob,
                               const _bcm_caladan3_field_group_index group,
                               const _bcm_caladan3_field_entry_index entry)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;
    _bcm_caladan3_field_entry_index currEntry;
    _bcm_caladan3_field_entry_index prevEntry;
    _bcm_caladan3_field_group_index currGroup;
    soc_c3_rce_entry_desc_t         entryDesc;
    int                             result;

    /* check valid group */
    if (glob->groupTotal <= group) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(glob->group[group]);
    if (glob->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* check valid entry */
    if (glob->entryTotal <= entry) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (0 > thisEntry->priority) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X has invalid priority %d\n"),
                   glob->unit,
                   entry,
                   thisEntry->priority));
        return BCM_E_CONFIG;
    }
    /* find where the entry goes in the group entry list */
    for (prevEntry = glob->entryTotal,
         currEntry = thisGroup->entryHead;
         currEntry < glob->entryTotal;
         prevEntry = currEntry,
         currEntry = glob->entry[currEntry].nextEntry) {
        if (0 < _bcm_caladan3_compare_entry_priority(thisEntry->priority,
                                                   glob->entry[currEntry].priority)) {
            /* this is the first entry whose priority is lower than new one */
            break;
        }
    }
    /* insert the entry within the group after prevEntry */
    if (glob->entryTotal > prevEntry) {
        /* insert within the list */
        thisEntry->nextEntry = glob->entry[prevEntry].nextEntry;
        thisEntry->prevEntry = prevEntry;
        glob->entry[prevEntry].nextEntry = entry;
    } else {
        /* insert at head of list */
        thisEntry->nextEntry = thisGroup->entryHead;
        thisEntry->prevEntry = glob->entryTotal;
        thisGroup->entryHead = entry;
    }
    if (glob->entryTotal > thisEntry->nextEntry) {
        /* not at end of list */
        glob->entry[thisEntry->nextEntry].prevEntry = entry;
    } else {
        /* at end of list */
        thisGroup->entryTail = entry;
    }
    /* find where the entry goes in the rulebase entry list */
    for (currGroup = thisGroup->prevGroup,
         prevEntry = thisEntry->prevEntry;
         (glob->groupTotal > currGroup) &&
         (glob->entryTotal <= prevEntry);
         prevEntry = glob->group[currGroup].entryTail,
         currGroup = glob->group[currGroup].prevGroup) {
        /*
         *  This entry is the first in its group; need to search toward (maybe
         *  all the way to) the beginning of this rulebase and pick the last
         *  entry from a prior group.
         */
    }
    for (currGroup = thisGroup->nextGroup,
         currEntry = thisEntry->nextEntry;
         (glob->groupTotal > currGroup) &&
         (glob->entryTotal <= currEntry);
         currEntry = glob->group[currGroup].entryHead,
         currGroup = glob->group[currGroup].nextGroup) {
        /*
         *  This entry is the last in its group; need to search toward (maybe
         *  all the way to) the end of this rulebase and pick the first entry
         *  from a following group.
         */
    }
    /* we know predecessor and successor in rulebase list, insert it */
    thisEntry->prevEntryRb = prevEntry;
    if (glob->entryTotal > prevEntry) {
        /* not at beginning of rulebase; update prev entry's next pointer */
        glob->entry[prevEntry].nextEntryRb = entry;
    } else {
        /* at beginning of rulebase; update head of list */
        glob->rulebase[thisGroup->rulebase].entryHead = entry;
    }
    thisEntry->nextEntryRb = currEntry;
    if (glob->entryTotal > currEntry) {
        /* not at end of rulebase; update next entry's back pointer */
        glob->entry[currEntry].prevEntryRb = entry;
    }
    /* account for the new entry */
    thisGroup->entries++;
    thisGroup->counters++;
    glob->rulebase[thisGroup->rulebase].entries++;
    /* diagnostics */
    FIELD_EVERB((BSL_META("added entry %08X to group %08X on unit %d\n"),
                 entry,
                 group,
                 glob->unit));
    sal_memset(&entryDesc,0,sizeof(entryDesc));
    entryDesc.groupId = group;
    entryDesc.entryPriority = thisEntry->priority;
    result = soc_c3_rce_entry_create(glob->unit, entry, &entryDesc);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("oc_c3_rce_entry_create failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_del
 *  Purpose
 *    Remove specified entry from its group.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_group_index group = which group for add
 *    (in) _bcm_caladan3_field_entry_index entry = which entry to add
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Does not remove entry from counter sharing, but does adjust group counter
 *    information if the entry is not sharing a counter.
 */
static int
_bcm_caladan3_g3p1_field_entry_del(_bcm_caladan3_g3p1_field_glob_t *glob,
                               const _bcm_caladan3_field_entry_index entry)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;
    int result;

    /* check valid entry */
    if (glob->entryTotal <= entry) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    thisGroup = &(glob->group[thisEntry->group]);
    /* pull the entry from the hardware */
    result = soc_c3_rce_entry_destroy(glob->unit, entry);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to remove unit %d entry %08X rules"
                   " from hardware\n"),
                   glob->unit,
                   entry));
        return result;
    }
    /* remove the entry from the rulebase */
    if (glob->entryTotal > thisEntry->prevEntryRb) {
        /* not the first entry in the rulebase */
        glob->entry[thisEntry->prevEntryRb].nextEntryRb = thisEntry->nextEntryRb;
    } else {
        /* the first entry in the rulebase */
        glob->rulebase[thisGroup->rulebase].entryHead = thisEntry->nextEntryRb;
    }
    if (glob->entryTotal > thisEntry->nextEntryRb) {
        /* not the last entry in the rulebase */
        glob->entry[thisEntry->nextEntryRb].prevEntryRb = thisEntry->prevEntryRb;
    }
    /* remove the entry from the group */
    if (glob->entryTotal > thisEntry->prevEntry) {
        /* not the first entry in the group */
        glob->entry[thisEntry->prevEntry].nextEntry = thisEntry->nextEntry;
    } else {
        /* the first entry in the group */
        thisGroup->entryHead = thisEntry->nextEntry;
    }
    if (glob->entryTotal > thisEntry->nextEntry) {
        /* not the last entry in the group */
        glob->entry[thisEntry->nextEntry].prevEntry = thisEntry->prevEntry;
    } else {
        /* the last entry in the group */
        thisGroup->entryTail = thisEntry->prevEntry;
    }
    glob->rulebase[thisGroup->rulebase].entries--;
    thisGroup->entries--;
    thisGroup->counters--;

    return BCM_E_NONE;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in/out) bcm_field_entry_t *entry = desired ID if valid
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 *
 *      Will use the first available entry if an invalid entry ID is given.
 */
static int
_bcm_caladan3_g3p1_field_entry_create(_bcm_caladan3_g3p1_field_glob_t *glob,
                                  const _bcm_caladan3_field_group_index group,
                                  _bcm_caladan3_field_entry_index *entry)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */
    int result;                                 /* working result */

    /* check whether group is in use */
    if (glob->uMaxSupportedStages <= glob->group[group].rulebase) {
        /* group is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (glob->rulebase[glob->group[group].rulebase].entries >=
        glob->rulebase[glob->group[group].rulebase].entriesMax) {
        /* there would be too many entries in this rulebase */
        return BCM_E_RESOURCE;
    }
    if (glob->rulebase[glob->group[group].rulebase].rules >=
        glob->rulebase[glob->group[group].rulebase].rulesMax) {
        /* there would be too many rules in this rulebase */
        return BCM_E_RESOURCE;
    }

    /* allocate the new entry */
    if ((glob->entryTotal <= *entry) || (0 > *entry)) {
        /* invalid entry ID; get first available */
        result = _bcm_caladan3_g3p1_field_entry_alloc(glob, entry);
    } else {
        /* valid entry ID; try to allocate it */
        result = _bcm_caladan3_g3p1_field_entry_alloc_id(glob, *entry);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    thisEntry = &(glob->entry[*entry]);

    /* associate the entry with the group (thence the rulebase) */
    thisEntry->group = group;
    thisEntry->priority = BCM_FIELD_ENTRY_PRIO_DEFAULT;

    /* add the entry to its group */
    result = _bcm_caladan3_g3p1_field_entry_add(glob, group, *entry);
    if (BCM_E_NONE != result) {
        /* failed to add the entry to the group; free the entry */
        _bcm_caladan3_g3p1_field_entry_free(glob, *entry);
    }
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_entry_copy
 *   Purpose
 *      Create an copy of a field entry, within the same group
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t orgEntry = original entry ID
 *      (in/out) bcm_field_entry_t *newEntry = desired new ID if valid
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will use the first available entry if an invalid entry ID is given.
 */
static int
_bcm_caladan3_g3p1_field_entry_copy(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_entry_index orgEntry,
                                _bcm_caladan3_field_entry_index *newEntry)
{
    _bcm_caladan3_g3p1_field_entry_t *thatEntry;    /* working entry data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */
    int result;                                 /* working result */

    /* check arguments */
    thatEntry = &(glob->entry[orgEntry]);
    if (!(thatEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d original entry %08X is not in use\n"),
                   glob->unit,
                   orgEntry));
        return BCM_E_NOT_FOUND;
    }
    /* allocate the new entry */
    if ((glob->entryTotal <= *newEntry) || (0 > *newEntry)) {
        /* invalid entry ID; get first available */
        result = _bcm_caladan3_g3p1_field_entry_alloc(glob, newEntry);
    } else {
        /* valid entry ID; try to allocate it */
        result = _bcm_caladan3_g3p1_field_entry_alloc_id(glob, *newEntry);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    thisEntry = &(glob->entry[*newEntry]);

    /* read the qualifier and action data from the old entry */

    /* copy other appropriate data from the old entry */
    thisEntry->entryFlags |= (thatEntry->entryFlags &
                              (~_CALADAN3_G3P1_ENTRY_VALID));
    thisEntry->priority = thatEntry->priority;
    thisEntry->group = thatEntry->group;
    /* add the new entry to its group */
    return _bcm_caladan3_g3p1_field_entry_add(glob,
                                          thisEntry->group,
                                          *newEntry);
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_entry_destroy
 *   Purpose
 *      Destroy an existing field entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = entry ID to be destroyed
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
static int
_bcm_caladan3_g3p1_field_entry_destroy(_bcm_caladan3_g3p1_field_glob_t *glob,
                                   const _bcm_caladan3_field_entry_index entry)
{
    int result;                                 /* working result */

    /* remove the entry from its group */
    result = _bcm_caladan3_g3p1_field_entry_del(glob, entry);
    if (BCM_E_NONE == result) {
        /* now dispose of the entry */
        result = _bcm_caladan3_g3p1_field_entry_free(glob, entry);
    }

    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_check_qset
 *  Purpose
 *    Make sure entry's group has the specified qualifier
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to validate
 *    (in) bcm_field_qualify_t qual = qualifier to check
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_entry_check_qset(const _bcm_caladan3_g3p1_field_glob_t *glob,
                                      const bcm_field_entry_t entry,
                                      const bcm_field_qualify_t qual)
{
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > qual) || (bcmFieldQualifyCount <= qual)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d does not support invalid"
                   " qualifier %d\n"),
                   glob->unit,
                   qual));
        return BCM_E_PARAM;
    }
    if (!BCM_FIELD_QSET_TEST(glob->group[glob->entry[entry].group].qset,
                             qual)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X qset does not include %s\n"),
                   glob->unit,
                   glob->entry[entry].group,
                   _sbx_caladan3_field_qual_name[qual]));
        /* BCM API claims this should be PARAM, though CONFIG is more sense */
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_check_action
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to validate
 *    (in) bcm_field_qualify_t qual = qualifier to check
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_entry_check_action(const _bcm_caladan3_g3p1_field_glob_t *glob,
                                        const bcm_field_entry_t entry,
                                        const bcm_field_action_t action)
{
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > action) || (bcmFieldActionCount <= action)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d does not support invalid action %d\n"),
                   glob->unit,
                   action));
        return BCM_E_PARAM;
    }
    if (!SHR_BITGET(glob->rulebase[glob->group[glob->entry[entry].group].rulebase].action,
                    action)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d rulebase %d (group %08X entry %08X)"
                   " actionset does not include %s\n"),
                   glob->unit,
                   glob->group[glob->entry[entry].group].rulebase,
                   glob->entry[entry].group,
                   entry,
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_CONFIG;
    }
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_counter_access
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to access
 *    (in) int clear = TRUE to clear counts during read, FALSE to only read
 *    (out) soc_sbx_g3p1_turbo64_count_t *counts = where to put counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Practically no error checking.
 *    Performs saturation instead of rollover.
 */
static int
_bcm_caladan3_g3p1_field_entry_counter_access(const _bcm_caladan3_g3p1_field_glob_t *glob,
                                          const bcm_field_entry_t entry,
                                          const int clear,
                                          soc_sbx_g3p1_turbo64_count_t *counts)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */
    int result = BCM_E_NONE;                    /* working result */
    soc_sbx_g3p1_turbo64_count_t tempCounts;    /* working counters */

    thisEntry = &(glob->entry[entry]);
    if (0 == (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_IN_HW)) {
        /* don't bother reading counters for entries not in hardware */
        return BCM_E_NONE;
    }

    /* read each available rule's counter pair */
    FIELD_EVERB((BSL_META("%s counter for unit %d entry %08X"
                            " (rulebase %02X)\n"),
                 clear?"clear":"read",
                 glob->unit,
                 entry,
                 glob->group[thisEntry->group].rulebase));
    result = soc_c3_rce_entry_counter_get(glob->unit,
                                          entry,
                                          clear,
                                          (uint64*)&tempCounts);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to read unit %d entry %08X"
                   " counters: %d (%s)\n"),
                   glob->unit,
                   entry,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    FIELD_EVERB((BSL_META("byte counter for unit %d entry %08X"
                            " (rulebase %02X)"
                            " = %08X%08X\n"),
                 glob->unit,
                 entry,
                 glob->group[thisEntry->group].rulebase,
                 COMPILER_64_HI(tempCounts.bytes),
                 COMPILER_64_LO(tempCounts.bytes)));
    FIELD_EVERB((BSL_META("frame counter for unit %d entry %08X"
                            " (rulebase %02X)"
                            " = %08X%08X\n"),
                 glob->unit,
                 entry,
                 glob->group[thisEntry->group].rulebase,
                 COMPILER_64_HI(tempCounts.packets),
                 COMPILER_64_LO(tempCounts.packets)));
    /* accumulate the counts according to the entry's counter mode */
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_COUNTER) {
        COMPILER_64_ADD_64(counts->bytes, tempCounts.bytes);
        COMPILER_64_ADD_64(counts->packets, tempCounts.packets);
        if (COMPILER_64_LT(counts->bytes, tempCounts.bytes)) {
            COMPILER_64_SET(counts->bytes, 0xFFFFFFFF, 0xFFFFFFFF);
        }
        if (COMPILER_64_LT(counts->packets, tempCounts.packets)) {
            COMPILER_64_SET(counts->packets, 0xFFFFFFFF, 0xFFFFFFFF);
        }
    } /* if (entry has counter and counter is enabled) */
    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entries_counter_access
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to access
 *    (in) int clear = TRUE to clear counts during read, FALSE to only read
 *    (out) soc_sbx_g3p1_turbo64_count_t *counts = where to put counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Performs saturation instead of rollover.
 */
static int
_bcm_caladan3_g3p1_field_entries_counter_access(const _bcm_caladan3_g3p1_field_glob_t *glob,
                                            const bcm_field_entry_t entry,
                                            const int clear,
                                            soc_sbx_g3p1_turbo64_count_t *counts)
{
    int result = BCM_E_NONE;

    if ((glob->entryTotal <= entry) || (0 > entry)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (!(glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_COUNTER)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X has no counter\n"),
                   glob->unit,
                   entry));
        return BCM_E_EMPTY;
    }
    COMPILER_64_ZERO(counts->bytes);
    COMPILER_64_ZERO(counts->packets);
    result = _bcm_caladan3_g3p1_field_entry_counter_access(glob,
                                                       entry,
                                                       clear,
                                                       counts);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to read unit %d entry %08X counts:"
                   " %d (%s)\n"),
                   glob->unit,
                   entry,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_qual_index_find
 *  Purpose
 *    Find the qualIndex for the qualify type in the qset.
 *  Parameters
 *    (in)  bcm_field_qset_t *qset = qset to search
 *    (in)  bcm_field_qualify_t qualType = qualify type to find
 *    (out) unsigned int *qaulIndex = index for qualify type if found
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_NOT_FOUND if qualType not in qset
 *  Notes
 *    The qualIndex is the number of the qualType in the qset
 *    counting only bits set in the qset.
 */
static int _bcm_caladan3_g3p1_qual_index_find(bcm_field_qset_t qset,
                                              bcm_field_qualify_t qualType,
                                              unsigned int *qualIndex)
{
    int     i;
    int     qualCount = 0;

    /* Search the qset */
    for (i = 0; i < bcmFieldQualifyCount; i++) {
        /* The range is not part of the qset in the soc layer so skip it here */
        if (i == bcmFieldQualifyRangeCheck) {
            continue;
        }
        if (BCM_FIELD_QSET_TEST(qset, i)) {
            if (i == qualType) {
                *qualIndex = qualCount;
                return BCM_E_NONE;
            }
            qualCount++;
        }
    }

    /* The qualType is not in the qset */
    *qualIndex = 0;

    return BCM_E_NOT_FOUND;

}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Group management
 *
 */

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_alloc_id
 *  Purpose
 *    Get a specific group description off the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_caladan3_field_group_index *group = where to put group ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_group_alloc_id(_bcm_caladan3_g3p1_field_glob_t *glob,
                                    const _bcm_caladan3_field_group_index group)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;

    /* check free count */
    if (!(glob->groupFreeCount)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("no available group on unit %d\n"),
                   glob->unit));
        return BCM_E_RESOURCE;
    }
    /* make sure group is free */
    thisGroup = &(glob->group[group]);
    if (_FIELD_CALADAN3_INVALID_RULEBASE != thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group ID %08X is already in use\n"),
                   glob->unit,
                   group));
        return BCM_E_EXISTS;
    }
    /* remove the entry from the list */
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not end of list */
        glob->group[thisGroup->nextGroup].prevGroup = thisGroup->prevGroup;
    }
    if (thisGroup->prevGroup < glob->groupTotal) {
        /* not head of list */
        glob->group[thisGroup->prevGroup].nextGroup = thisGroup->nextGroup;
    } else {
        /* head of list */
        glob->groupFreeHead = thisGroup->nextGroup;
    }
    glob->groupFreeCount--;
    /* don't leave stale links lying about */
    thisGroup->prevGroup = glob->groupTotal;
    thisGroup->nextGroup = glob->groupTotal;
    /* mark group as neither being in a rulebase nor as being free */
    thisGroup->rulebase = _FIELD_CALADAN3_TEMP_RULEBASE;
    /* done */
    FIELD_EVERB((BSL_META("unit %d group %08X allocated from free list\n"),
                 glob->unit,
                 group));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_alloc
 *  Purpose
 *    Get a group description off the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_caladan3_field_group_index *group = where to put group ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 *    Destroys current value at group, even if it fails.
 */
static int
_bcm_caladan3_g3p1_field_group_alloc(_bcm_caladan3_g3p1_field_glob_t *glob,
                                 _bcm_caladan3_field_group_index *group)
{
    /* get head of list */
    *group = glob->groupFreeHead;
    /* allocate that */
    return _bcm_caladan3_g3p1_field_group_alloc_id(glob, *group);
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_free
 *  Purpose
 *    Return an group description to the free list
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_group_index *group = which group ID to free
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; clears most.
 *    Very limited error checking.
 */
static int
_bcm_caladan3_g3p1_field_group_free(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_group_index group)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;

    /* check valid group ID */
    if (glob->groupTotal <= group) {
        /* invalid group ID */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group ID %08X invalid\n"),
                   glob->unit,
                   group));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    /* make sure group is not already free and is not in use */
    thisGroup = &(glob->group[group]);
    if (thisGroup->rulebase == _FIELD_CALADAN3_INVALID_RULEBASE) {
        /* trying to free already free group */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is already free\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (thisGroup->entries) {
        /* trying to free a group with entries */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X still has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_BUSY;
    }
    if (thisGroup->rulebase != _FIELD_CALADAN3_TEMP_RULEBASE) {
        /* trying to free an in-use group */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is still in rulebase %d\n"),
                   glob->unit,
                   group,
                   thisGroup->rulebase));
        return BCM_E_BUSY;
    }
    /* clear the group */
    sal_memset(thisGroup, 0, sizeof(*thisGroup));
    /* set up some fields specifically */
    thisGroup->prevGroup = glob->groupTotal;
    thisGroup->entryHead = glob->entryTotal;
    thisGroup->entryTail = glob->entryTotal;
    thisGroup->rulebase = _FIELD_CALADAN3_INVALID_RULEBASE;
    /* insert the group as head of free list */
    thisGroup->nextGroup = glob->groupFreeHead;
    glob->groupFreeHead = group;
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not only one in list */
        glob->group[thisGroup->nextGroup].prevGroup = group;
    }
    glob->groupFreeCount++;
    /* done */
    FIELD_EVERB((BSL_META("unit %d group %08X returned to free list\n"),
                 glob->unit,
                 group));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_add
 *  Purpose
 *    Insert a group into the specified rulebase, based upon the priority of
 *    the group and any other groups in that rulebase.  Should be strict
 *    priority, though will follow the 'most recent last' rule if there is a
 *    group with the same priority.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) unsigned int rulebase = which rulebase (by index)
 *    (in) _bcm_caladan3_field_group_index group = ID of group to be inserted
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Will not insert a group that has entries.
 *    BCM_FIELD_GROUP_PRIO_ANY will always be added last.
 *    Prohibits negative priorities that are not 'special'.
 */
static int
_bcm_caladan3_g3p1_field_group_add(_bcm_caladan3_g3p1_field_glob_t *glob,
                               const unsigned int rulebase,
                               const _bcm_caladan3_field_group_index group)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;
    _bcm_caladan3_field_group_index currGroup;
    _bcm_caladan3_field_group_index prevGroup;

    /* make sure everything looks okay */
    if (glob->uMaxSupportedStages <= rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid rulebase %d on unit %d\n"),
                   rulebase,
                   glob->unit));
        return BCM_E_PARAM;
    }
    if (glob->groupTotal <= group) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return BCM_E_PARAM;
    }
    thisGroup = &(glob->group[group]);
    if (0 > thisGroup->priority) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X has invalid priority %d\n"),
                   glob->unit,
                   group,
                   thisGroup->priority));
        return BCM_E_CONFIG;
    }
    if (thisGroup->entries) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X already has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_CONFIG;
    }
    if (thisGroup->rulebase != _FIELD_CALADAN3_TEMP_RULEBASE) {
        if (_FIELD_CALADAN3_INVALID_RULEBASE == thisGroup->rulebase) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d group %08X is not allocated\n"),
                       glob->unit,
                       group));
            return BCM_E_NOT_FOUND;
        }
        if (glob->uMaxSupportedStages > thisGroup->rulebase) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d group %08X is already in"
                       " rulebase %d\n"),
                       glob->unit,
                       group,
                       thisGroup->rulebase));
            return BCM_E_BUSY;
        }
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is corrupt\n"),
                   glob->unit,
                   group));
        return BCM_E_INTERNAL;
    } /* if (thisGroup->rulebase != _FIELD_CALADAN3_TEMP_RULEBASE) */
    /* figure out where to put this group */
    currGroup = glob->rulebase[rulebase].groupHead;
    prevGroup = glob->groupTotal;
    while (currGroup < glob->groupTotal) {
        if (glob->group[currGroup].priority < thisGroup->priority) {
            /* this is the first group that is lower priority than inserted */
            break;
        }
        /* go to the next group */
        prevGroup = currGroup;
        currGroup = glob->group[currGroup].nextGroup;
    } /* while (currGroup < glob->groupTotal) */
    /* always insert *after* prevGroup */
    if (prevGroup < glob->groupTotal) {
        /* inserting somewhere after beginning of list */
        thisGroup->nextGroup = glob->group[prevGroup].nextGroup;
        thisGroup->prevGroup = prevGroup;
        glob->group[prevGroup].nextGroup = group;
    } else { /* if (currGroup < glob->groupTotal) */
        /* inserting at the beginning of the list */
        thisGroup->nextGroup = glob->rulebase[rulebase].groupHead;
        thisGroup->prevGroup = glob->groupTotal;
        glob->rulebase[rulebase].groupHead = group;
    } /* if (currGroup < glob->groupTotal) */
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* make sure next element points back correctly */
        glob->group[thisGroup->nextGroup].prevGroup = group;
    }
    /* mark group as being in the new rulebase */
    thisGroup->rulebase = rulebase & 0xFF;
    FIELD_EVERB((BSL_META("unit %d group %08X added to rulebase %d"
                            " at priority %d\n"),
                 glob->unit,
                 group,
                 rulebase & 0xFF,
                 thisGroup->priority));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_del
 *  Purpose
 *    Remove a group from its rulebase.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_caladan3_field_group_index group = ID of group to be removed
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Will not remove a group that has entries.
 */
static int
_bcm_caladan3_g3p1_field_group_del(_bcm_caladan3_g3p1_field_glob_t *glob,
                               const _bcm_caladan3_field_group_index group)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup;
    int result;

    /* make sure everything looks okay */
    if (glob->groupTotal <= group) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return BCM_E_PARAM;
    }
    thisGroup = &(glob->group[group]);
    if (thisGroup->entries) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X still has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_CONFIG;
    }
    if (thisGroup->rulebase >= glob->uMaxSupportedStages) {
        if ((_FIELD_CALADAN3_TEMP_RULEBASE == thisGroup->rulebase) ||
            (_FIELD_CALADAN3_INVALID_RULEBASE == thisGroup->rulebase)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d group %d is not %s\n"),
                       glob->unit,
                       group,
                       ((_FIELD_CALADAN3_TEMP_RULEBASE == thisGroup->rulebase)?
                       "in a rulebase":
                       "allocated")));
            return ((_FIELD_CALADAN3_TEMP_RULEBASE == thisGroup->rulebase)?
                    BCM_E_CONFIG:
                    BCM_E_NOT_FOUND);
        }
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %d is corrupt\n"),
                   glob->unit,
                   group));
        return BCM_E_INTERNAL;
    } /* if (thisGroup->rulebase >= glob->uMaxSupportedStages) */
    /* remove this group from the rulebase */
    if (thisGroup->prevGroup < glob->groupTotal) {
        /* not beginning of list */
        glob->group[thisGroup->prevGroup].nextGroup = thisGroup->nextGroup;
    } else {
        /* beginning of list */
        glob->rulebase[thisGroup->rulebase].groupHead = thisGroup->nextGroup;
    }
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not ending of list */
        glob->group[thisGroup->nextGroup].prevGroup = thisGroup->prevGroup;
    }
    /* don't leave stale links lying about */
    thisGroup->nextGroup = glob->groupTotal;
    thisGroup->prevGroup = glob->groupTotal;
    /* mark group as neither being in a rulebase nor as being free */
    FIELD_EVERB((BSL_META("unit %d group %08X removed from rulebase %d\n"),
                 glob->unit,
                 group,
                 thisGroup->rulebase));
    result = soc_c3_rce_group_destroy(glob->unit, group);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_group_destroy failed: %s\n"),
                   bcm_errmsg(result)));
    }

    thisGroup->rulebase = _FIELD_CALADAN3_TEMP_RULEBASE;
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_create
 *  Purpose
 *    Create a new group with the specified ID that has the specified
 *    qualifier set and priority.
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in/out) _bcm_caladan3_field_group_index group = desired ID if valid
 *    (in) _bcm_caladan3_g3p1_field_group_t *groupData = group data to use
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Allocates first available group ID and fills it into group argument if
 *    provided ID is not valid.
 *    Can not specify a priority already taken by an existing group.
 *    Can not specify a qualifier that another group in the same stage has.
 *    If no stage qualifier, bcmFieldQualifyStageIngressQoS is assumed.
 *    Picks an available priority value if none is given (mods groupData).
 */
static int
_bcm_caladan3_g3p1_field_group_create(_bcm_caladan3_g3p1_field_glob_t *glob,
                                  _bcm_caladan3_field_group_index *group,
                                  _bcm_caladan3_g3p1_field_group_t *groupData)
{
    bcm_field_qset_t qset = groupData->qset;
    int result;
    int priority;
    _bcm_caladan3_field_group_index     currGroup;
    _bcm_caladan3_g3p1_field_group_t    *thisGroup;
    uint8                               tempPri[_FIELD_CALADAN3_G3P1_MAX_GROUPS];
    uint8                               rulebase;
    soc_c3_rce_group_desc_t             groupDesc;
    soc_c3_rce_header_field_info_t      header;
    _sbx_caladan3_field_header_info_t   *pTable;
    _sbx_caladan3_field_header_info_t   *pHeaderInfo;
    int                                 qualIndex;
    int                                 i;

    /* allocate a group */
    if ((glob->groupTotal <= *group) || (0 > *group)) {
        /* invalid group ID; get first available */
        result = _bcm_caladan3_g3p1_field_group_alloc(glob, group);
    } else {
        /* valid group ID; try to allocate it */
        result = _bcm_caladan3_g3p1_field_group_alloc_id(glob, *group);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* figure out which stage is appropriate; try ingress first */
    for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) {
        result = _bcm_caladan3_qset_subset(glob->rulebase[rulebase].qset,
                                         groupData->qset);
        if ((BCM_E_NONE == result) ||
            ((BCM_E_NONE != result) && (BCM_E_FAIL != result))) {
            /* it fits in this rulebase or something went wrong */
            break;
        }
    } /* for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) */
    if (BCM_E_FAIL == result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("specified qset does not match any stage"
                   " on unit %d\n"),
                   glob->unit));
        result = _SBX_CALADAN3_FIELD_INVALID_QSET_ERR;
    } else if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to compare qsets on unit %d: %d (%s)\n"),
                   glob->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    /* check and maybe assign priority */
    if (_SBX_CALADAN3_FIELD_GROUP_PRIO_ANY == groupData->priority) {
        /* no priority given by caller */
        sal_memset(&(tempPri[0]),
                   0x00,
                   sizeof(tempPri[0]) * glob->groupTotal);
        /* find out which of the first (group count) priorities are in use */
        for (currGroup = glob->rulebase[rulebase].groupHead;
             glob->groupTotal > currGroup;
             currGroup = glob->group[currGroup].nextGroup) {
            if (glob->group[currGroup].priority < glob->groupTotal) {
                tempPri[glob->group[currGroup].priority] = TRUE;
            }
        }
        /* pick the highest available of the first (group count) priorities */
        for (priority = (glob->groupTotal - 1);
             priority > 0;
             priority --) {
            if (!(tempPri[priority])) {
                break;
            }
        }
        groupData->priority = priority;
        groupData->rulebase = rulebase;
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("chose priority %d for this group\n"),
                   priority));
    }
    /* mark stage in qset and scan for same priority in the rulebase */
    if (BCM_E_NONE == result) {
#if 0
        /* mark the qset for the stage, just in case it's not set */
        BCM_FIELD_QSET_ADD(qset, (rulebase?
                                  bcmFieldQualifyStageEgressSecurity:
                                  bcmFieldQualifyStageIngressQoS));
#endif
        /* make sure there are no groups with the same priority or qset */
        for (currGroup = glob->rulebase[rulebase].groupHead;
             glob->groupTotal > currGroup;
             currGroup = glob->group[currGroup].nextGroup) {
            if (glob->group[currGroup].priority == groupData->priority) {
                /* found one with same priority; bail */
                break;
            }
        }
        if (glob->groupTotal > currGroup) {
            /* we found one that matches priority or qset */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("can not add new group to unit %d that has"
                       " same priority %d as existing group\n"),
                       glob->unit,
                       groupData->priority));
            /* BCM API regression says this should result in BCM_E_EXISTS */
            result = BCM_E_EXISTS;
        }
    } /* if (BCM_E_NONE == result) */

    /* fill in the allocated group with the provided data & add to rulebase */
    thisGroup = &(glob->group[*group]);
    if (BCM_E_NONE == result) {
        /* fill in the allocated group with the provided data */
        thisGroup->priority = groupData->priority;
        thisGroup->qset = qset;
        /* add the group to the rulebase */
        result = _bcm_caladan3_g3p1_field_group_add(glob, rulebase, *group);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FP,
                     (BSL_META("soc_c3_rce_program_qualifier_build failed when add group to rule base: %s\n"),
                     bcm_errmsg(result)));
           return result;
        }
    }

    /* Initialize data */
    qualIndex = 0;
    sal_memset(&groupDesc,0,sizeof(groupDesc));
    groupDesc.resultLrp = 0x8;
    groupData->rulebase = rulebase;
    groupDesc.rceProgram = groupData->rulebase;
    groupDesc.groupPriority = groupData->priority;
    if (BCM_FIELD_QSET_TEST(qset,bcmFieldQualifyRangeCheck)) {
        groupDesc.rangesPerFilterSet = 2;
    }

    /* Select table to use */
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6) ||
        BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6)) {
        pTable = caladan3_ip6_rce_field_info;
    } else {
        pTable = caladan3_rce_field_info;
    }

    /* Check and fill in the qualifiers */
    for (i = 0; i < bcmFieldQualifyCount; i++) {
        if (BCM_FIELD_QSET_TEST(qset, i)) {

            /* Soc information for range is specified when range is created */
            if (i == bcmFieldQualifyRangeCheck) {
                continue;
            }

            /* Find soc info for this qualify type */
            pHeaderInfo = pTable;
            while (pHeaderInfo->bcmQualType != i && pHeaderInfo->bcmQualType != bcmFieldQualifyCount) {
                pHeaderInfo++;
            }

            /* Check if this qualifier is supported */
            if (pHeaderInfo->bcmQualType == bcmFieldQualifyCount) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("qualifier %s not supported\n"),
                           _sbx_caladan3_field_qual_name[i]));
                return BCM_E_PARAM;
            }

            /* Data for soc call */
            header.header = pHeaderInfo->header;
            header.startBit = pHeaderInfo->startBit;
            header.numBits = pHeaderInfo->numBits;
#if defined(BROADCOM_DEBUG)
            header.fieldName = _sbx_caladan3_field_qual_name[i];
#else /* defined(BROADCOM_DEBUG) */
            header.fieldName = NULL;
#endif /* defined(BROADCOM_DEBUG) */

            /* Create the qualifier set */
            result = soc_c3_rce_program_qualifier_build(glob->unit,
                groupDesc.rceProgram, &header, pHeaderInfo->socQualType,
                &(groupDesc.qualData[qualIndex]));
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_program_qualifier_build failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            } else {
                qualIndex++;
            }
        }
    }

    /* Set qualifier count */
    groupDesc.qualCount = qualIndex;

    /* Create the group */
    result = soc_c3_rce_group_create(glob->unit, *group, &groupDesc);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_group_create failed: %s\n"),
                   bcm_errmsg(result)));

        /* something went wrong; free the group */
        /* anything that goes wrong above already provided a diagnostic */
        _bcm_caladan3_g3p1_field_group_free(glob, *group);
    }

    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Range management
 *
 */

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_range_create_id
 *   Purpose
 *      Fill in a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *      (in) uint32 range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range with the specified ID must already be allocated.
 *      No effect on the hardware.
 *      We support Src and Dst port flags together, but not TCP and UDP.  We
 *      also don't support both Src and Dst or both TCP and UDP clear.
 */
static int
_bcm_caladan3_g3p1_field_range_create_id(_bcm_caladan3_g3p1_field_glob_t *glob,
                                       const uint32 range,
                                       const uint32 flags,
                                       const bcm_l4_port_t min,
                                       const bcm_l4_port_t max)
{
    soc_c3_rce_range_desc_t     rangeDesc;
    int                         result;

    /* verify range parameters */
    if (min > max) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("range min %08X must be <= max %08X\n"),
                   min,
                   max));
        return BCM_E_PARAM;
    }
    if (flags & (~(BCM_FIELD_RANGE_TCP |
                   BCM_FIELD_RANGE_UDP |
                   BCM_FIELD_RANGE_SRCPORT |
                   BCM_FIELD_RANGE_DSTPORT))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid range flags specified: %08X\n"),
                   flags & (~(BCM_FIELD_RANGE_TCP |
                   BCM_FIELD_RANGE_UDP |
                   BCM_FIELD_RANGE_SRCPORT |
                   BCM_FIELD_RANGE_DSTPORT))));
        return BCM_E_PARAM;
    }
    if ((BCM_FIELD_RANGE_TCP | BCM_FIELD_RANGE_UDP) ==
         (flags & (BCM_FIELD_RANGE_TCP | BCM_FIELD_RANGE_UDP))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("at most one of TCP or UDP"
                   " may be specified\n")));
        return BCM_E_PARAM;
    }
    if (!(flags & (BCM_FIELD_RANGE_SRCPORT | BCM_FIELD_RANGE_DSTPORT))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("must specify at least one of"
                   " SRCPORT or DSTPORT\n")));
        return BCM_E_PARAM;
    }
    if ((0 > min) ||
        (0 > max) ||
        (0xFFFF < min) ||
        (0xFFFF < max)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("range min %08X or max %08X is not valid TCP"
                   " or UDP port number\n"),
                   min,
                   max));
        return BCM_E_PARAM;
    }
    /* set up the range */
    glob->range[range].flags = flags;
    glob->range[range].min = min;
    glob->range[range].max = max;

    /* Create the range in the soc layer */
    sal_memset(&rangeDesc, 0, sizeof(soc_c3_rce_range_desc_t));
    rangeDesc.headerField.header = socC3RceDataHeaderTcpUdp;
    rangeDesc.headerField.numBits = 16;
    if (flags & BCM_FIELD_RANGE_DSTPORT) {
        /* DST */
#if defined(BROADCOM_DEBUG)
        rangeDesc.headerField.fieldName = _sbx_caladan3_field_qual_name[bcmFieldQualifyL4DstPort];
#else /* defined(BROADCOM_DEBUG) */
        rangeDesc.headerField.fieldName = NULL;
#endif /* defined(BROADCOM_DEBUG) */
        rangeDesc.headerField.startBit = SOC_C3_RCE_FRAME_BIT(2, 7);
    } else {
        /* SRC */
#if defined(BROADCOM_DEBUG)
        rangeDesc.headerField.fieldName = _sbx_caladan3_field_qual_name[bcmFieldQualifyL4SrcPort];
#else /* defined(BROADCOM_DEBUG) */
        rangeDesc.headerField.fieldName = NULL;
#endif /* defined(BROADCOM_DEBUG) */
        rangeDesc.headerField.startBit = SOC_C3_RCE_FRAME_BIT(0, 7);
    }
    rangeDesc.lowerBound = min;
    rangeDesc.upperBound = max;
    result = soc_c3_rce_range_create(glob->unit, range, &rangeDesc);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_range_create failed on unit %d:%s\n"),
                   glob->unit, _SHR_ERRMSG(result)));
    }

    /* done */
    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Rulebase management
 *
 */

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_rulebase_commit
 *  Purpose
 *    Commit a rulebase and update entry in hardware status
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *glob = pointer to unit globals
 *    (in) unsigned int rulebase = rulebase to commit
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    Little error checking and no locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_rulebase_commit(_bcm_caladan3_g3p1_field_glob_t *glob,
                                     unsigned int rulebase)
{
    _bcm_caladan3_field_entry_index entry;          /* working entry ID */
    int result = BCM_E_NONE;                    /* working result */

    /* update entry in hardware state for affected entries */
    if (BCM_E_NONE == result) {
        FIELD_EVERB((BSL_META("update 'in hardware' state for entries\n")));
        for (entry = glob->rulebase[rulebase].entryHead;
             entry < glob->entryTotal;
             entry = glob->entry[entry].nextEntryRb) {
        }
    } else {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to commit rules to hardware for"
                   " rulebase %d: %d (%s)\n"),
                   rulebase,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

#ifdef BROADCOM_DEBUG
/*****************************************************************************
 *
 *  Debugging support
 *
 */

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_rule_dump
 *  Purpose
 *    Dump information about a single rule to debug output
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_caladan3_field_entry_index entry = the entry ID
 *    (in) char *prefix = a string to display in front of each line
 *    (in) int full = TRUE if displaying full rule, FALSE for ID & counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_rule_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                               const _bcm_caladan3_field_entry_index entry,
                               const char *prefix,
                               const int full)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry = &(glob->entry[entry]);
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       data[6];
    unsigned char                       mask[6];
    bcm_policer_t                       policer;
    _bcm_caladan3_field_range_t         *thisRange;
    int displayed;
    int result=0;
    bcm_module_t modId;
    bcm_port_t portId;
    bcm_gport_t gportId;
    int enabled = (0 != (glob->entry[entry].entryFlags &
                         _CALADAN3_G3P1_ENTRY_IN_HW));

    /* display rule information */
    FIELD_PRINT(("%sUnit %d rule %02X:%08X information:\n",
                 prefix,
                 glob->unit,
                 glob->group[thisEntry->group].rulebase,
                 entry));
    /* display rule contents */
    if (full) {
        FIELD_PRINT(("%s  Rule enabled = %s\n",
                         prefix,
                         enabled ? "TRUE":"FALSE"));

        displayed = FALSE;
        FIELD_PRINT(("%s  Qualifiers:\n", prefix));
        /* Get the group */
        group = glob->entry[entry].group;

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyInPort)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyInPort,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyInPort],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyOutPort)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyOutPort,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyOutPort],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifySrcMac)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifySrcMac,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/" FIELD_MACA_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifySrcMac],
                         FIELD_MACA_SHOW_REVERSE(data),
                         FIELD_MACA_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyDstMac)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyDstMac,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/" FIELD_MACA_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyDstMac],
                         FIELD_MACA_SHOW_REVERSE(data),
                         FIELD_MACA_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyEtherType)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyEtherType,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_SHORT_FORMAT "/" FIELD_SHORT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyEtherType],
                         FIELD_SHORT_SHOW_REVERSE(data),
                         FIELD_SHORT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyOuterVlan)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyOuterVlan,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_SHORT_FORMAT "/" FIELD_SHORT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlan],
                         FIELD_SHORT_SHOW_REVERSE(data),
                         FIELD_SHORT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyOuterVlanId)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyOuterVlanId,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_SHORT_FORMAT "/" FIELD_SHORT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlanId],
                         FIELD_SHORT_SHOW_REVERSE(data),
                         FIELD_SHORT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyOuterVlanPri)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyOuterVlanPri,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s =  %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlanPri],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyOuterVlanCfi)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyOuterVlanCfi,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s =  %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlanCfi],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifySrcIp)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifySrcIp,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_INT_FORMAT "/" FIELD_INT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifySrcIp],
                         FIELD_INT_SHOW_REVERSE(data),
                         FIELD_INT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyDstIp)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyDstIp,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_INT_FORMAT "/" FIELD_INT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyDstIp],
                         FIELD_INT_SHOW_REVERSE(data),
                         FIELD_INT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyIpProtocol)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyIpProtocol,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyIpProtocol],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyL4SrcPort)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyL4SrcPort,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_SHORT_FORMAT "/" FIELD_SHORT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyL4SrcPort],
                         FIELD_SHORT_SHOW_REVERSE(data),
                         FIELD_SHORT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyL4DstPort)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyL4DstPort,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = " FIELD_SHORT_FORMAT "/" FIELD_SHORT_FORMAT "\n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyL4DstPort],
                         FIELD_SHORT_SHOW_REVERSE(data),
                         FIELD_SHORT_SHOW_REVERSE(mask)));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyRangeCheck)) {
            if (thisEntry->range[0] != 0) {
                thisRange = &glob->range[thisEntry->range[0]];
                FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d) Invert: %s\n",
                             prefix,
                             "(source port range)",
                             thisRange->min,
                             thisRange->max,
                             thisRange->min,
                             thisRange->max,
                             thisRange->invert ? "True" : "False"));
                displayed = TRUE;
            }
            if (thisEntry->range[1] != 0) {
                thisRange = &glob->range[thisEntry->range[1]];
                FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d) Invert: %s\n",
                             prefix,
                             "(destination port range)",
                             thisRange->min,
                             thisRange->max,
                             thisRange->min,
                             thisRange->max,
                             thisRange->invert ? "True" : "False"));
                displayed = TRUE;
            }
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyTcpControl)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyTcpControl,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyTcpControl],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (BCM_FIELD_QSET_TEST(glob->group[group].qset, bcmFieldQualifyTos)) {
            result = _bcm_caladan3_g3p1_qual_index_find(glob->group[group].qset,
                                                        bcmFieldQualifyTos,
                                                        &qualIndex);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = soc_c3_rce_entry_qualify_get(glob->unit,
                                                  entry,
                                                  qualIndex,
                                                  data,
                                                  mask);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            FIELD_PRINT(("%s    %s = %02X/%02X \n",
                         prefix,
                         _sbx_caladan3_field_qual_name[bcmFieldQualifyTos],
                         data[0],
                         mask[0]));
            displayed = TRUE;
        }

        if (!displayed) {
            FIELD_PRINT(("%s    (no qualifiers; rule matches all frames)\n",
                         prefix));
        }
        displayed = FALSE;
        FIELD_PRINT(("%s  Actions:\n", prefix));
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            /* Check VLAN new */
            if (glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VLANNEW) {
                FIELD_PRINT(("%s    %s = %04X (%d) (VSI, *not* OuterVID)\n",
                            prefix,
                            "(VSI)",
                            glob->entry[entry].ftvlData.VSI,
                            glob->entry[entry].ftvlData.VSI));
                displayed = TRUE;
            }
            /* Check ft index */
            if (glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_L3SWITCH) {
                FIELD_PRINT(("%s    %s = %08X\n",
                            prefix,
                            "(FTI)",
                            glob->entry[entry].ftvlData.ftHandle));
                displayed = TRUE;
            }
            /* Check ingress mirror */
            result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                ingressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                result = _bcm_caladan3_ingr_mirror_get(glob->unit,
                                                     data[0],
                                                     &gportId);
                if (BCM_E_NONE == result) {
                    if (BCM_GPORT_IS_MODPORT(gportId)) {
                        modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                        portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                        FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                 prefix,
                                 _sbx_caladan3_field_action_name[bcmFieldActionMirrorIngress],
                                 data[0],
                                 data[0],
                                 modId,
                                 portId));
                    }
                } else {
                    FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                 prefix,
                                 _sbx_caladan3_field_action_name[bcmFieldActionMirrorIngress],
                                 data[0],
                                 data[0],
                                 result,
                                 _SHR_ERRMSG(result)));
                }
                displayed = TRUE;
            }
            /* Check copy to CPU */
            if (glob->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_CPYTOCPU) {
                FIELD_PRINT(("%s    %s\n",
                            prefix,
                            _sbx_caladan3_field_action_name[bcmFieldActionCopyToCpu]));
                displayed = TRUE;
            }
            /* Check drop precedence */
            result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                ingressActionMap[socC3RceActionNewDp].modIndex, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                    ingressActionMap[socC3RceActionNewDp].index, data);
                if (result == BCM_E_NONE) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                prefix,
                                _sbx_caladan3_field_action_name[bcmFieldActionDropPrecedence],
                                data[0]));
                }
                displayed = TRUE;
            }
            /* Check cos */
            result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                ingressActionMap[socC3RceActionNewPrio].modIndex, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                    ingressActionMap[socC3RceActionNewPrio].index, data);
                if (result == BCM_E_NONE) {
                FIELD_PRINT(("%s    %s = %01X\n",
                             prefix,
                             _sbx_caladan3_field_action_name[bcmFieldActionCosQNew],
                             data[0] & 0x7));
                }
                displayed = TRUE;
            }
            /* Check policer */
            result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                ingressActionMap[socC3RceActionPolicer].index, data);
            if (result == BCM_E_NONE) {
                policer = data[0] + (data[1] << 8);
                if (policer != 0) {
                    /* Get flags */
                    result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                        ingressActionMap[socC3RceActionPolicer].modIndex, data);
                    if (result == BCM_E_NONE) {
                        FIELD_PRINT(("%s    %s = %08X"
                                    " (typed = %s, mef = %s (%d))\n",
                                    prefix,
                                    "(police)",
                                    policer,
                                    (data[0] & 1) ? "TRUE":"FALSE",
                                    (data[0] & 2) ? "TRUE":"FALSE",
                                    (data[0] & 4) ? 1 : 0));
                    }
                    displayed = TRUE;
                }
            }
            break;
        case 1:
            if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_DROP) {
                FIELD_PRINT(("%s    %s\n",
                             prefix,
                             _sbx_caladan3_field_action_name[bcmFieldActionDrop]));
                displayed = TRUE;
            }
            /* Check egress mirror */
            result = soc_c3_rce_entry_action_get(glob->unit, entry, 
                egressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                result = _bcm_caladan3_egr_mirror_get(glob->unit,
                                                    data[0],
                                                    &gportId);
                if (BCM_E_NONE == result) {
                    if (BCM_GPORT_IS_MODPORT(gportId)) {
                        modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                        portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                        FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                 prefix,
                                 _sbx_caladan3_field_action_name[bcmFieldActionMirrorEgress],
                                 data[0],
                                 data[0],
                                 modId,
                                 portId));
                    }
                } else {
                    FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                 prefix,
                                 _sbx_caladan3_field_action_name[bcmFieldActionMirrorEgress],
                                 data[0],
                                 data[0],
                                 result,
                                 _SHR_ERRMSG(result)));
                }
                displayed = TRUE;
            }
            break;
        default:
            /* already bailed out on this condition, but compiler will gripe */
            break;
        } /* switch (glob->group[thisEntry->group].rulebase) */
        if (!displayed) {
            FIELD_PRINT(("%s    (no actions; rule does not affect frames)\n",
                         prefix));
        }
    } /* if (full) */

    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_entry_dump
 *  Purpose
 *    Dump information about the specified entry to debug output
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_caladan3_field_entry_index entry = the entry ID
 *    (in) char *prefix = a string to display in front of each line
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_entry_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_entry_index entry,
                                const char *prefix)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry = &(glob->entry[entry]);
    soc_sbx_g3p1_turbo64_count_t counters;
    unsigned int column;
    char *rulePrefix = NULL;
    int result = BCM_E_NONE;
    int auxRes;
    int full;

    FIELD_PRINT(("%sUnit %d entry " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " (%d) information:\n",
                 prefix,
                 glob->unit,
                 entry,
                 entry));
    FIELD_PRINT(("%s  Priority = %d\n", prefix, thisEntry->priority));
    FIELD_PRINT(("%s  Group = " _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d)\n",
                 prefix,
                 thisEntry->group,
                 thisEntry->group));
    FIELD_PRINT(("%s  Entry flags = %08X\n", prefix, thisEntry->entryFlags));
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_COUNTER) {
        FIELD_PRINT(("%s  Counters:\n", prefix));
        COMPILER_64_ZERO(counters.bytes);
        COMPILER_64_ZERO(counters.packets);
        result = _bcm_caladan3_g3p1_field_entries_counter_access(glob,
                                                             entry,
                                                             FALSE,
                                                             &counters);
        if (BCM_E_NONE == result) {
            FIELD_PRINT(("%s    Packets hit = %08X%08X\n",
                         prefix,
                         COMPILER_64_HI(counters.packets),
                         COMPILER_64_LO(counters.packets)));
            FIELD_PRINT(("%s    Bytes hit =   %08X%08X\n",
                         prefix,
                         COMPILER_64_HI(counters.bytes),
                         COMPILER_64_LO(counters.bytes)));
        } else {
            FIELD_PRINT(("%s    [Unable to read entry counters: %d (%s)]\n",
                         prefix,
                         result,
                         _SHR_ERRMSG(result)));
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to read unit %d entry %08X"
                       " counters: %d (%s)\n"),
                       glob->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else {
        FIELD_PRINT(("%s  Counters disabled\n", prefix));
    }
    column = 0;
    FIELD_PRINT(("%s  Emulated actions: (rules show direct mapped)\n", prefix));
    /* Ingress drop is emulated by using the drop all policer */
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_DROP &&
        glob->group[thisEntry->group].rulebase == 0) {
        FIELD_PRINT(("%s    %s\n",
                     prefix,
                     _sbx_caladan3_field_action_name[bcmFieldActionDrop]));
        column = 1;
    }
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VLANNEW) {
        FIELD_PRINT(("%s    %s %04X (%d)\n",
                     prefix,
                     _sbx_caladan3_field_action_name[bcmFieldActionVlanNew],
                     thisEntry->ftvlData.VSI,
                     thisEntry->ftvlData.VSI));
        column = 1;
    }
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_L3SWITCH) {
        FIELD_PRINT(("%s    %s to %08X\n",
                     prefix,
                     _sbx_caladan3_field_action_name[bcmFieldActionL3Switch],
                     thisEntry->ftvlData.ftHandle));
        column = 1;
    }
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_REDIRECT) {
        FIELD_PRINT(("%s    %s module %d port %d\n",
                     prefix,
                     _sbx_caladan3_field_action_name[bcmFieldActionRedirect],
                     thisEntry->ftvlData.target.module,
                     thisEntry->ftvlData.target.port));
        column = 1;
    }

    if (!column) {
        FIELD_PRINT(("%s    (none; see rules for all actions)\n", prefix));
    }
    FIELD_PRINT(("%s  Prev/Next entry in group = " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " / " _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisEntry->prevEntry,
                 thisEntry->nextEntry,
                 thisEntry->prevEntry,
                 thisEntry->nextEntry));
    FIELD_PRINT(("%s  Prev/Next entry in rulebase = "
                 _SBX_CALADAN3_FIELD_INDEX_FORMAT " / "
                 _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisEntry->prevEntryRb,
                 thisEntry->nextEntryRb,
                 thisEntry->prevEntryRb,
                 thisEntry->nextEntryRb));
    rulePrefix = sal_alloc(sal_strlen(prefix) + 6,
                            "entry dump working string");
    if (rulePrefix) {
        sal_snprintf(rulePrefix, sal_strlen(prefix) + 5, "%s    ", prefix);
    }
    FIELD_PRINT(("%s  Rules for this entry:\n", prefix));
    full = TRUE;
    column = 0;
    if (rulePrefix) {
        /* can dump the rule properly */
        auxRes = _bcm_caladan3_g3p1_field_rule_dump(glob,
                                                entry,
                                                rulePrefix,
                                                full);
        if (BCM_E_NONE != auxRes) {
            result = auxRes;
        } else {
            full = _FIELD_CALADAN3_G3P1_DUMP_ALL_RULES;
        }
        column = 1;
    } else {
        /* no buffer so just dump rule IDs */
        if (0 == column) {
            FIELD_PRINT(("%s    %02X",
                         prefix,
                         glob->group[thisEntry->group].rulebase));
            column = sal_strlen(prefix) + 18;
        }
    }
    if (column) {
        if (!rulePrefix) {
            FIELD_PRINT(("\n"));
        }
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    if (rulePrefix) {
        sal_free(rulePrefix);
        rulePrefix = NULL;
    }
    return result;
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_field_group_dump
 *  Purpose
 *    Dump information about the specified group to debug output
 *  Parameters
 *    (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_caladan3_field_group_index group = the group ID
 *    (in) char *prefix = a string to display in front of each line
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_group_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_group_index group,
                                const char *prefix)
{
    _bcm_caladan3_g3p1_field_group_t *thisGroup = &(glob->group[group]);
    _bcm_caladan3_field_entry_index entry;
    char *entryPrefix = NULL;
    int auxRes;
    int result = BCM_E_NONE;

    FIELD_PRINT(("%sUnit %d group " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " (%d) information:\n",
                 prefix,
                 glob->unit,
                 group,
                 group));
    FIELD_PRINT(("%s  Priority = %d\n", prefix, thisGroup->priority));
    FIELD_PRINT(("%s  Entries = %d (%d counter shares)\n",
                 prefix,
                 thisGroup->entries,
                 thisGroup->entries - thisGroup->counters));
    FIELD_PRINT(("%s  Rulebase = %d\n",
                 prefix,
                 thisGroup->rulebase));
    FIELD_PRINT(("%s  Prev/Next group = " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " / " _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisGroup->prevGroup,
                 thisGroup->nextGroup,
                 thisGroup->prevGroup,
                 thisGroup->nextGroup));
#if _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
    FIELD_PRINT(("%s  First/Last entry = " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " / " _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisGroup->entryHead,
                 thisGroup->entryTail,
                 thisGroup->entryHead,
                 thisGroup->entryTail));
#endif /* _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */
    entryPrefix = sal_alloc(sal_strlen(prefix) + 6,
                            "group dump working string");
    if (entryPrefix) {
        sal_snprintf(entryPrefix, sal_strlen(prefix) + 5, "%s    ", prefix);
        FIELD_PRINT(("%s  Qualifier set:\n", prefix));
        result = _bcm_caladan3_field_qset_dump(thisGroup->qset,
                                             entryPrefix);
        sal_snprintf(entryPrefix, sal_strlen(prefix) + 3, "%s  ", prefix);
        for (entry = thisGroup->entryHead;
             entry < glob->entryTotal;
             entry = glob->entry[entry].nextEntry) {
            auxRes = _bcm_caladan3_g3p1_field_entry_dump(glob, entry, entryPrefix);
            if (BCM_E_NONE != auxRes) {
                /* keep the error but try to proceed despite it */
                result = auxRes;
            }
        }
        sal_free(entryPrefix);
        entryPrefix = NULL;
    } else { /* if (entryPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for qset/entry"
                     " display]\n",
                     prefix));
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to allocate working"
                   " prefix for qset and entry dump\n")));
        result = BCM_E_MEMORY;
    } /* if (entryPrefix) */
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_rulebase_dump
 *   Purpose
 *      Dump information about the specified rulebase to debug output
 *   Parameters
 *      (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *      (in) uint8 rulebase = the rulebase ID
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_rulebase_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                                   const uint8 rulebase,
                                   const char *prefix)
{
    _bcm_caladan3_g3p1_field_rulebase_t *thisRb = &(glob->rulebase[rulebase]);
    _bcm_caladan3_field_group_index group;
    bcm_field_action_t action;
    char *groupPrefix = NULL;
    int result = BCM_E_NONE;
    int tempRes;
    unsigned int column = 0;
#if _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
#if _FIELD_CALADAN3_G3P1_DUMP_LISTS
    _bcm_caladan3_field_entry_index entry;
#endif /* _FIELD_CALADAN3_G3P1_DUMP_LISTS */
#endif /* _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */

    FIELD_PRINT(("%sUnit %d rulebase %d information:\n",
                 prefix,
                 glob->unit,
                 rulebase));

    switch (rulebase) {
    case 0:
        FIELD_PRINT(("%s  Ingress database\n", prefix));
        break;

    case 1:
        FIELD_PRINT(("%s  Egress database\n", prefix));
        break;

    default:
        FIELD_PRINT(("%s  Unknown database\n", prefix));
        break;
    }
    
    FIELD_PRINT(("%s  Rules = %d (%d max)\n",
                 prefix,
                 thisRb->rules,
                 thisRb->rulesMax));
    FIELD_PRINT(("%s  Entries = %d (%d max; first = "
                 _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 thisRb->entries,
                 thisRb->entriesMax,
                 thisRb->entryHead,
                 thisRb->entryHead));
    FIELD_PRINT(("%s  Ports = " FIELD_PBMP_FORMAT "\n",
                 prefix,
                 FIELD_PBMP_SHOW(thisRb->ports)));
    groupPrefix = sal_alloc(strlen(prefix) + 6,
                            "rulebase dump working string");
    if (groupPrefix) {
        sal_snprintf(groupPrefix, strlen(prefix) + 5, "%s    ", prefix);
        FIELD_PRINT(("%s  Supported qualifiers:\n", prefix));
        result = _bcm_caladan3_field_qset_dump(thisRb->qset,
                                             groupPrefix);
    } else { /* if (groupPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for rulebase"
                     " display]\n",
                     prefix));
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to allocate working"
                   " prefix for rulebase dump\n")));
        result = BCM_E_MEMORY;
    } /* if (groupPrefix) */
    FIELD_PRINT(("%s  Supported actions:\n", prefix));
    for (action = 0; action < bcmFieldActionCount; action++) {
        if (SHR_BITGET(thisRb->action, action)) {
            if (0 == column) {
                /* just starting out */
                FIELD_PRINT(("%s    %s",
                             prefix,
                             _sbx_caladan3_field_action_name[action]));
                column = (4 + sal_strlen(prefix) +
                          sal_strlen(_sbx_caladan3_field_action_name[action]));
            } else if ((2 + column +
                       sal_strlen(_sbx_caladan3_field_action_name[action])) >=
                       _SBX_CALADAN3_FIELD_PAGE_WIDTH) {
                /* this qualifier would wrap */
                FIELD_PRINT(("\n%s    %s",
                             prefix,
                             _sbx_caladan3_field_action_name[action]));
                column = (4 + sal_strlen(prefix) +
                          sal_strlen(_sbx_caladan3_field_action_name[action]));
            } else {
                /* this qualifier fits on the line */
                FIELD_PRINT((", %s",
                             _sbx_caladan3_field_action_name[action]));
                column += (2 +
                           sal_strlen(_sbx_caladan3_field_action_name[action]));
            }
        } /* if (SHR_BITGET(thisRb->action, action)) */
    } /* for (action = 0; action < bcmFieldActionCount; action++) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    FIELD_PRINT(("%s  Unit %d rulebase %d information:\n",
                 prefix,
                 glob->unit,
                 rulebase));
    FIELD_PRINT(("%s    Ports = " FIELD_PBMP_FORMAT "\n",
                 prefix,
                 FIELD_PBMP_SHOW(thisRb->ports)));
    FIELD_PRINT(("%s    Rules = %d (%d max)\n",
                 prefix,
                 thisRb->rules,
                 thisRb->rulesMax));
    if (groupPrefix) {
        sal_snprintf(groupPrefix, strlen(prefix) + 3, "%s  ", prefix);
        for (group = thisRb->groupHead;
             group < glob->groupTotal;
             group = glob->group[group].nextGroup) {
            tempRes = _bcm_caladan3_g3p1_field_group_dump(glob,
                                                      group,
                                                      groupPrefix);
            if (BCM_E_NONE != tempRes) {
                /* keep the error but try to proceed despite it */
                result = tempRes;
            }
        } /* for (all groups in this stage) */
        sal_free(groupPrefix);
        groupPrefix = NULL;
    } /* if (groupPrefix) */
#if _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
#if _FIELD_CALADAN3_G3P1_DUMP_LISTS
    FIELD_PRINT(("%s  Unit %d rulebase %d entries list:\n",
                 prefix,
                 glob->unit,
                 rulebase));
    column = 0;
    for (entry = thisRb->entryHead;
         entry < glob->entryTotal;
         entry = glob->entry[entry].nextEntryRb) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_CALADAN3_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_CALADAN3_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_CALADAN3_FIELD_INDEX_FORMAT, entry));
            column += (2 + _SBX_CALADAN3_FIELD_INDEX_WIDTH);
        }
    } /* for (all entries in this stage) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
#endif /* _FIELD_CALADAN3_G3P1_DUMP_LISTS */
#endif /* _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_range_dump
 *   Purpose
 *      Dump information about the specified range to debug output
 *   Parameters
 *      (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *      (in) bcm_fe2k_field_range_index range = the range to dump
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_range_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                                const _bcm_caladan3_field_range_index range,
                                const char *prefix)
{
    _bcm_caladan3_field_range_t *thisRange = &(glob->range[range]);

    FIELD_PRINT(("%sUnit %d range " _SBX_CALADAN3_FIELD_INDEX_FORMAT
                 " (%d) information\n",
                 prefix,
                 glob->unit,
                 range + 1,
                 range + 1));
    FIELD_PRINT(("%s  Flags = %08X:\n", prefix, thisRange->flags));
    if (thisRange->flags & BCM_FIELD_RANGE_TCP) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_TCP\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_UDP) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_UDP\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_SRCPORT\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_DSTPORT\n", prefix));
    }
    FIELD_PRINT(("%s  Port range = %04X..%04X (%d..%d)\n",
                 prefix,
                 thisRange->min,
                 thisRange->max,
                 thisRange->min,
                 thisRange->max));
    return BCM_E_NONE;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_dump
 *   Purpose
 *      Dump information about the field subsystem
 *   Parameters
 *      (in) _bcm_caladan3_g3p1_field_glob_t *thisUnit = pointer to unit info
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_caladan3_g3p1_field_dump(_bcm_caladan3_g3p1_field_glob_t *glob,
                          const char *prefix)
{
    char *rbPrefix = NULL;
    int result = BCM_E_NONE;
    int tempRes;
    uint8 rulebase;
    uint32 rangeStart;
    uint32 rangeEnd;
    uint32 rangeUsed;
    uint32 rangeFree;
    _bcm_caladan3_field_range_index range;
#if _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
#if _FIELD_CALADAN3_G3P1_DUMP_LISTS
    unsigned int column;
    _bcm_caladan3_field_group_index group;
    _bcm_caladan3_field_entry_index entry;
#endif /* _FIELD_CALADAN3_G3P1_DUMP_LISTS */
#endif /* _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */

    FIELD_PRINT(("%sUnit %d field subsystem information\n",
                 prefix,
                 glob->unit));
    result = shr_idxres_list_state(glob->rangeFree,
                                   &rangeStart,
                                   &rangeEnd,
                                   NULL,
                                   NULL,
                                   &rangeFree,
                                   &rangeUsed);
    if (BCM_E_NONE == result) {
        FIELD_PRINT(("%s  Ranges = %d (%d max)\n",
                     prefix,
                     rangeUsed,
                     rangeUsed + rangeFree));
    } else { /* if (BCM_E_NONE == result) */
        FIELD_PRINT(("%s  [Unable to get range information: %d (%s)]\n",
                     prefix,
                     result,
                     _SHR_ERRMSG(result)));
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to read unit %d range information:"
                   " %d (%s)\n"),
                   glob->unit,
                   result,
                   _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */
    FIELD_PRINT(("%s  Groups = %d (%d max, next alloc "
                 _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 glob->groupTotal - glob->groupFreeCount,
                 glob->groupTotal,
                 glob->groupFreeHead,
                 glob->groupFreeHead));
    FIELD_PRINT(("%s  Entries = %d (%d max, next alloc "
                 _SBX_CALADAN3_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 glob->entryTotal - glob->entryFreeCount,
                 glob->entryTotal,
                 glob->entryFreeHead,
                 glob->entryFreeHead));
    rbPrefix = sal_alloc(strlen(prefix) + 4, "rulebase dump working string");
    if (rbPrefix) {
        sal_snprintf(rbPrefix, strlen(prefix) + 3, "%s  ", prefix);
        for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) {
            tempRes = _bcm_caladan3_g3p1_field_rulebase_dump(glob,
                                                         rulebase,
                                                         rbPrefix);
            if (BCM_E_NONE != tempRes) {
                /* keep the error but try to proceed despite it */
                result = tempRes;
            }
        } /* for (all rulebases) */
        for (range = rangeStart; range < rangeEnd; range++) {
            if (BCM_E_EXISTS == shr_idxres_list_elem_state(glob->rangeFree,
                                                           range)) {
                tempRes = _bcm_caladan3_g3p1_field_range_dump(glob,
                                                          range,
                                                          rbPrefix);
                if (BCM_E_NONE != tempRes) {
                    /* keep the error but try to proceed despite it */
                    result = tempRes;
                }
            }
        } /* for (all ranges) */
        sal_free(rbPrefix);
        rbPrefix = NULL;
    } else { /* if (rbPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for entry/range"
                     " display]\n",
                     prefix));
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to allocate working"
                   " prefix for entry/range dump\n")));
        result = BCM_E_MEMORY;
    } /* if (rbPrefix) */
#if _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
#if _FIELD_CALADAN3_G3P1_DUMP_LISTS
    FIELD_PRINT(("%s  Unit %d free groups list:\n", prefix, glob->unit));
    column = 0;
    for (group = glob->groupFreeHead;
         group < glob->groupTotal;
         group = glob->group[group].nextGroup) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         group));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_CALADAN3_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_CALADAN3_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         group));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_CALADAN3_FIELD_INDEX_FORMAT, group));
            column += (2 + _SBX_CALADAN3_FIELD_INDEX_WIDTH);
        }
    } /* for (all free groups) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    FIELD_PRINT(("%s  Unit %d free entries list:\n", prefix, glob->unit));
    column = 0;
    for (entry = glob->entryFreeHead;
         entry < glob->entryTotal;
         entry = glob->entry[entry].nextEntry) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_CALADAN3_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_CALADAN3_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_CALADAN3_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_CALADAN3_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_CALADAN3_FIELD_INDEX_FORMAT, entry));
            column += (2 + _SBX_CALADAN3_FIELD_INDEX_WIDTH);
        }
    } /* for (all free entries) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
#endif /* _FIELD_CALADAN3_G3P1_DUMP_LISTS */
#endif /* _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */
    return result;
}

#endif /* def BROADCOM_DEBUG */

/*****************************************************************************
 *
 *  External interface
 *
 */

/*
 *   Function
 *      bcm_caladan3_g3p1_field_init
 *   Purpose
 *      Initialise the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) void** unitData = where to put the pointer to the unit data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_init(int unit, void **unitData)
{
    int result = BCM_E_NONE;                  /* working result */
    int tmpRes;                               /* working result */
    unsigned int groupsTotal;                 /* max supported groups */
    unsigned int rangesTotal;                 /* ranges total */
    unsigned int entriesTotal;                /* total entries supported */
    unsigned int entriesIngress;              /* ingress entries supported */
    unsigned int entriesEgress;               /* egress entries supported */
    unsigned int rulesTotal;                  /* total rules supported */
    unsigned int rulesIngress = 0;            /* rules ingress */
    unsigned int rulesEgress = 0;             /* rules egress */
    int i;                                    /* working indices */
    uint32 excId;                             /* exception ID for copyToCpu */
    soc_sbx_g3p1_xt_t copyToCpuExc;           /* copyToCpu exception data */
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    soc_c3_rce_unit_desc_t          *pUnitInfo;
    soc_c3_rce_program_desc_t       *pProgInfo;
    soc_c3_rce_actiontable_desc_t   *pActTblInfo;
    soc_c3_rce_action_uc_type_t     action;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "(%d,*) enter\n"),
               unit));

    result = soc_c3_rce_info_get(unit, &pUnitInfo);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "soc_c3_rce_unit_info_get failed unit %d program 0: %s\n"),
                   unit,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /*
     *  Find out how many rules are allowed, for ingress and for
     *  egress.  These are the bases for the sizing calculation.
     */
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "probing unit %d...\n"),
               unit));
    result = soc_c3_rce_program_info_get(unit, 0, &pProgInfo);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "soc_c3_rce_program_info_get failed unit %d program 0: %s\n"),
                   unit,
                   _SHR_ERRMSG(result)));
        soc_c3_rce_info_free(unit, pUnitInfo);
        return result;
    }
    result = soc_c3_rce_actiontable_info_get(unit,
                                             pProgInfo->actionTable[3],
                                             &pActTblInfo);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "soc_c3_rce_actiontable_info_get failed unit %d action table %u %s\n"),
                   unit,
                   pProgInfo->actionTable[3],
                   _SHR_ERRMSG(result)));
        soc_c3_rce_program_info_free(unit, pProgInfo);
        soc_c3_rce_info_free(unit, pUnitInfo);
        return result;
    }

    /* Initialize program data from soc data */
    rulesIngress = pProgInfo->entryMaxCount;

    /* Initialize action table */
    sal_memset(ingressActionMap, 0, sizeof(ingressActionMap));
    for (i = 0; i < pActTblInfo->actionCount; i++) {
        action = pActTblInfo->actFields[i].action;
        if (action == socC3RceActionEnable) {
            continue;
        }
        ingressActionMap[action].index = i;
        ingressActionMap[action].modIndex = pActTblInfo->actFields[i].enableIndex;
        ingressActionMap[action].disableVal = pActTblInfo->actFields[i].disableVal;
    }

    soc_c3_rce_actiontable_info_free(unit, pActTblInfo);
    soc_c3_rce_program_info_free(unit, pProgInfo);

    /* Initialize program one, egress */
    if (pUnitInfo->programCount > 1) {
        result = soc_c3_rce_program_info_get(unit, 1, &pProgInfo);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "soc_c3_rce_program_info_get failed unit %d program 1: %s\n"),
                       unit,
                       _SHR_ERRMSG(result)));
            soc_c3_rce_info_free(unit, pUnitInfo);
            return result;
        }
        result = soc_c3_rce_actiontable_info_get(unit,
                                                 pProgInfo->actionTable[3],
                                                 &pActTblInfo);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "soc_c3_rce_actiontable_info_get failed unit %d action table %u %s\n"),
                       unit,
                       pProgInfo->actionTable[3],
                       _SHR_ERRMSG(result)));
            soc_c3_rce_program_info_free(unit, pProgInfo);
            soc_c3_rce_info_free(unit, pUnitInfo);
            return result;
        }
        rulesEgress = pProgInfo->entryMaxCount;
        sal_memset(egressActionMap, 0, sizeof(egressActionMap));
        for (i = 0; i < pActTblInfo->actionCount; i++) {
            action = pActTblInfo->actFields[i].action;
            if (action == socC3RceActionEnable) {
                continue;
            }
            egressActionMap[action].index = i;
            egressActionMap[action].modIndex = pActTblInfo->actFields[i].enableIndex;
            egressActionMap[action].disableVal = pActTblInfo->actFields[i].disableVal;
        }
        soc_c3_rce_actiontable_info_free(unit, pActTblInfo);
        soc_c3_rce_program_info_free(unit, pProgInfo);
    }

    rulesTotal = rulesEgress + rulesIngress;
    entriesTotal = rulesTotal;
    entriesIngress = rulesIngress;
    entriesEgress = rulesEgress;
    groupsTotal = pUnitInfo->groupMaxCount;
    rangesTotal = pUnitInfo->rangeMaxCount;

    /* dump the probe results as diagnostics */
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d supports %d ingress rules (probed)\n"),
               unit,
               rulesIngress));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d supports %d egress rules (probed)\n"),
               unit,
               rulesEgress));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d supports %d entries (computed)\n"),
               unit,
               entriesTotal));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d supports %d ranges (probed)\n"),
               unit,
               rangesTotal));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d supports %d groups (probed)\n"),
               unit,
               groupsTotal));

    /* begin real initialisation based upon the probe results */
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "unit %d probe complete; initialising...\n"),
               unit));

    /* allocate private description for the unit */
    FIELD_EVERB((BSL_META("allocate unit description (%d bytes)\n"),
                 sizeof(_bcm_caladan3_g3p1_field_glob_t)));
    thisUnit = sal_alloc(sizeof(_bcm_caladan3_g3p1_field_glob_t),
                         "caladan3_g3p1_unit_field_data");
    if (!thisUnit) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unable to allocate unit %d description\n"),
                   unit));
        soc_c3_rce_info_free(unit, pUnitInfo);
        return BCM_E_MEMORY;
    }

    /* clear out the structure */
    sal_memset(thisUnit, 0x00, sizeof(_bcm_caladan3_g3p1_field_glob_t));

    /* keep unit number around; we'll be using it later */
    thisUnit->unit = unit;

    /* Set the maximum supported stages */
    thisUnit->uMaxSupportedStages = pUnitInfo->programCount;

    /* Free the unit info */
    soc_c3_rce_info_free(unit, pUnitInfo);

    /* allocate index space */
    FIELD_EVERB((BSL_META("allocate unit %d index space (%d bytes)\n"),
                 unit,
                 ((sizeof(_bcm_caladan3_g3p1_field_group_t) * groupsTotal) +
                  (sizeof(_bcm_caladan3_g3p1_field_entry_t) * entriesTotal) +
                  (sizeof(_bcm_caladan3_field_range_t) * rangesTotal))));
    thisUnit->group = sal_alloc(((sizeof(_bcm_caladan3_g3p1_field_group_t) *
                                  groupsTotal) +
                                 (sizeof(_bcm_caladan3_g3p1_field_entry_t) *
                                  entriesTotal) +
                                 (sizeof(_bcm_caladan3_field_range_t) *
                                  rangesTotal)),
                                "_bcm_fe2k_field_unit_indices");
    if (thisUnit->group) {
        /* clear the index space */
        sal_memset(thisUnit->group,
                   0,
                   ((sizeof(_bcm_caladan3_g3p1_field_group_t) * groupsTotal) +
                    (sizeof(_bcm_caladan3_g3p1_field_entry_t) * entriesTotal) +
                    (sizeof(_bcm_caladan3_field_range_t) * rangesTotal)));
        /* split the index space */
        FIELD_EVERB((BSL_META("split unit %d index space\n"), unit));
        thisUnit->entry = ((_bcm_caladan3_g3p1_field_entry_t *)
                           (&(thisUnit->group[groupsTotal])));
        thisUnit->range = ((_bcm_caladan3_field_range_t *)
                           (&(thisUnit->entry[entriesTotal])));
        FIELD_EVERB((BSL_META("unit %3d index space: group = %08X\n"),
                     unit,
                     (uint32)(thisUnit->group)));
        FIELD_EVERB((BSL_META("                      entry = %08X\n"),
                     (uint32)(thisUnit->entry)));
        FIELD_EVERB((BSL_META("                      range = %08X\n"),
                     (uint32)(thisUnit->range)));
    } else { /* if (thisUnit->group) */
        /* sal_alloc failed if we're here */
        result = BCM_E_MEMORY;
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "failed to allocate unit %d index space\n"),
                   unit));
    } /* if (thisUnit->group) */

    /* initialise entry and group and range data and create free lists */
    if (BCM_E_NONE == result) {
        /* initialise rules */
        FIELD_EVERB((BSL_META("init unit %d entries into free list\n"),
                     unit));
        for (i = 0; i < entriesTotal; i++) {
            if (0 < i) {
                thisUnit->entry[i].prevEntry = i - 1;
            } else {
                thisUnit->entry[i].prevEntry = entriesTotal;
            }
            if (i < (rulesTotal - 1)) {
                thisUnit->entry[i].nextEntry = i + 1;
            } else {
                thisUnit->entry[i].nextEntry = entriesTotal;
            }
            thisUnit->entry[i].nextEntryRb = entriesTotal;
            thisUnit->entry[i].prevEntryRb = entriesTotal;
            thisUnit->entry[i].group = groupsTotal;
        } /* for (i = 0; i < entriesTotal; i++) */
        thisUnit->entryFreeHead = 0;
        thisUnit->entryFreeCount = entriesTotal;
        thisUnit->entryTotal = entriesTotal;

        FIELD_EVERB((BSL_META("init unit %d groups into free list\n"),
                     unit));
        for (i = 0; i < groupsTotal; i++) {
            if (0 < i) {
                thisUnit->group[i].prevGroup = i - 1;
            } else {
                thisUnit->group[i].prevGroup = groupsTotal;
            }
            thisUnit->group[i].nextGroup = i + 1;
            thisUnit->group[i].entryHead = entriesTotal;
            thisUnit->group[i].entryTail = entriesTotal;
            thisUnit->group[i].rulebase = _FIELD_CALADAN3_INVALID_RULEBASE;
        } /* for (i = 0; i < groupsTotal; i++) */
        thisUnit->groupFreeHead = 0;
        thisUnit->groupFreeCount = groupsTotal;
        thisUnit->groupTotal = groupsTotal;

        FIELD_EVERB((BSL_META("init unit %d ranges\n"),
                     unit));
        for (i = 0; i < rangesTotal; i++) {
            thisUnit->range[i].max = 0xFFFF;
        }
        thisUnit->rangeTotal = rangesTotal;
        FIELD_EVERB((BSL_META("create unit %d range freelist\n"), unit));
        result = shr_idxres_list_create(&(thisUnit->rangeFree),
                                        1,
                                        rangesTotal,
                                        1,
                                        rangesTotal,
                                        "_bcm_fe2k_field_unit_rangeList");
        if ((BCM_E_NONE != result) || (!(thisUnit->rangeFree))) {
            /* failed to create the range freelist */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "failed to create range freelist\n")));
            thisUnit->rangeFree = NULL;
            if (BCM_E_NONE == result) {
                /* but there should have been an error */
                result = BCM_E_INTERNAL;
            }
        } /* if ((BCM_E_NONE != result) || (!(thisUnit->rangeFree))) */
    } /* if (BCM_E_NONE == result) */

    /*
     *  Set up the rulebase information in the unit description.
     *
     */
    if (BCM_E_NONE == result) {
        FIELD_EVERB((BSL_META("set up unit %d rulebases)\n"),
                     unit));

        /* ingress (StageQoS) rulebase */
        thisUnit->rulebase[0].entriesMax = entriesIngress;
        thisUnit->rulebase[0].rulesMax = rulesIngress;
        thisUnit->rulebase[0].groupHead = groupsTotal;
        BCM_PBMP_ASSIGN(thisUnit->rulebase[0].ports, PBMP_ALL(unit));

        if (thisUnit->uMaxSupportedStages > 1) {
            /* egress (StageEgressSecurity) rulebase */
            thisUnit->rulebase[1].entriesMax = entriesEgress;
            thisUnit->rulebase[1].rulesMax = rulesEgress;
            thisUnit->rulebase[1].groupHead = groupsTotal;
            thisUnit->rulebase[1].ports = PBMP_ALL(unit);
        }

        /*
         *  Set up the allowed qualifiers.  These are assumed for now, but in
         *  the future, it's possible that the rule schema will change and
         *  these will then need to be probed somehow.
         */
        thisUnit->rulebase[0].entryHead = thisUnit->entryTotal;
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifySrcMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDstMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDSCP);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlan);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlanId);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlanPri);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlanCfi);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifySrcIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDstIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyRangeCheck);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyL4DstPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyL4SrcPort);
#if 0
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifySrcIp6);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDstIp6);
#endif

        /*
         *  Set up the supported actions.  Hopefully this will not change, but
         *  maybe in the future it will.  Again, we'll probably need a way to
         *  probe that in the case that it does.
         */
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionDrop);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionVlanNew);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionCosQNew);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionDropPrecedence);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionL3Switch);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionCopyToCpu);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionRedirect);
#ifdef C3_REDIRECT_SUPPORTED
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionRedirectMcast);
#endif
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionMirrorIngress);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionPolicerLevel0);

        if (thisUnit->uMaxSupportedStages > 1) {
            /*
             *  Set up the allowed qualifiers.  These are assumed for now, but in
             *  the future, it's possible that the rule schema will change and
             *  these will then need to be probed somehow.
             */
            thisUnit->rulebase[1].entryHead = thisUnit->entryTotal;
#if 0
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyStageEgressSecurity);
#endif
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifySrcMac);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyDstMac);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyDSCP);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyIpProtocol);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifySrcIp);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyDstIp);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyEtherType);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyTcpControl);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOutPort);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOutPorts);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyRangeCheck);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOuterVlan);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOuterVlanId);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOuterVlanPri);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyOuterVlanCfi);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyL4DstPort);
            BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                               bcmFieldQualifyL4SrcPort);
    
            /*
             *  Set up the supported actions.  Hopefully this will not change, but
             *  maybe in the future it will.  Again, we'll probably need a way to
             *  probe that in the case that it does.
             */
            SHR_BITSET(thisUnit->rulebase[1].action,
                       bcmFieldActionDrop);
            SHR_BITSET(thisUnit->rulebase[1].action,
                       bcmFieldActionMirrorEgress);
        }

    } /* if (BCM_E_NONE == result) */

    if (result == BCM_E_NONE) {

        /* use well-known policer for Drop action */
        thisUnit->dropPolicer = BCM_CALADAN3_SPEC_POL_DROP_ALL;

        /* turn on the 'copyToCpu' exception */
        tmpRes = soc_sbx_g3p1_exc_rt_copy_idx_get(unit, &excId);
        if (BCM_E_NONE == tmpRes) {

            soc_sbx_g3p1_xt_t_init(&copyToCpuExc);
            tmpRes = soc_sbx_g3p1_xt_get(unit, excId, &copyToCpuExc);
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "unable to get %d:xt[%08X]: %d (%s)\n"),
                           unit, excId, result, _SHR_ERRMSG(result)));
            }

            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "program unit %d CopyToCpu exception"
                                   " %02X:qid=%08X\n"),
                       unit,
                       excId,
                       copyToCpuExc.qid));

            copyToCpuExc.trunc = FALSE;
            copyToCpuExc.forward = TRUE;
            tmpRes = soc_sbx_g3p1_xt_set(unit, excId, &copyToCpuExc);
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "unable to set %d:xt[%08X]: %d (%s)\n"),
                           unit,
                           excId,
                           result,
                           _SHR_ERRMSG(result)));
            }
        } else {
            result = tmpRes;
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unable to get unit %d copyToCpu exception ID:"
                                   " %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }

    /* wrap it up */
    if (BCM_E_NONE == result) {
        /* return the unit information pointer */
        *unitData = (void*)thisUnit;
    } else {/* if (BCM_E_NONE == result) */
        if (thisUnit) {
            if (thisUnit->rangeFree) {
                /* get rid of range freelist */
                FIELD_EVERB((BSL_META("get rid of unit %d range freelist\n"),
                             unit));
                shr_idxres_list_destroy(thisUnit->rangeFree);
            }
            if (thisUnit->group) {
                /* get rid of unit indices */
                FIELD_EVERB((BSL_META("get rid of unit %d index space\n"),
                             unit));
                sal_free(thisUnit->group);
            }
            /* get rid of unit private data */
            FIELD_EVERB((BSL_META("get rid of unit %d private data\n"),
                         unit));
            sal_free(thisUnit);
            thisUnit = NULL;
        } /* if (thisUnit) */
    }/* if (BCM_E_NONE == result) */

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_detach
 *   Purpose
 *      Initialise the field APIs.
 *   Parameters
 *      (in) void* unitData = the pointer to the unit data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Returns last error if any occur.
 *      Allows limited resource dangling in order to improve performance.
 */
int
bcm_caladan3_g3p1_field_detach(void *unitData)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;
    _bcm_caladan3_g3p1_field_entry_t    *thisEntry;
    int                                 result;
    int                                 i;
    unsigned char                       data[4];

    /* map local unit information */
    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* get rid of the range freelist */
    result = shr_idxres_list_destroy(thisUnit->rangeFree);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to destroy unit %d range freelist:"
                   " %d (%s)\n"),
                   thisUnit->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }

    /* get rid of all rules in all stages */
    for (i = 0; i < thisUnit->entryTotal; i++) {
        /* get rid of rules for this entry */
        thisEntry = &(thisUnit->entry[i]);
        if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
            /* don't free from non-entries */
            continue;
        }

        switch(thisUnit->group[thisEntry->group].rulebase) {
        case 1:
            result = soc_c3_rce_entry_action_get(thisUnit->unit, i, 
                egressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                _bcm_caladan3_egr_mirror_free(thisUnit->unit, data[0]);
            }
            break;
        default:
            break;
        }

    }

    
    /* get rid of unit indices */
    sal_free(thisUnit->group);
    thisUnit->group = NULL;

    /* get rid of unit data */
    sal_free(thisUnit);

    /* okay, done */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_create
 *   Purpose
 *      Create a new group that has the specified qualifier set and priority.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group (does
 *      this mean globally or within just the single stage?)
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be ingress stage.
 */
int
bcm_caladan3_g3p1_field_group_create(void *unitData,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_t *group)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t groupData;   /* working group data */
    _bcm_caladan3_field_group_index groupId;      /* working group ID */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* set up the group */
    sal_memset(&groupData, 0, sizeof(groupData));
    groupData.qset = qset;
    if (BCM_FIELD_GROUP_PRIO_ANY == pri) {
        groupData.priority = _SBX_CALADAN3_FIELD_GROUP_PRIO_ANY;
    }else{
        groupData.priority = pri;
    }
    groupId = thisUnit->groupTotal;
    result = _bcm_caladan3_g3p1_field_group_create(unitData, &groupId, &groupData);
    if (BCM_E_NONE == result) {
        *group = groupId;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_create_id
 *   Purpose
 *      Create a new group with the specified ID that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 */
int
bcm_caladan3_g3p1_field_group_create_id(void *unitData,
                                      bcm_field_qset_t qset,
                                      int pri,
                                      bcm_field_group_t group)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t groupData;   /* working group data */
    _bcm_caladan3_field_group_index groupId;      /* working group ID */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't create invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    /* set up the group */
    sal_memset(&groupData, 0, sizeof(groupData));
    groupData.qset = qset;
    if (BCM_FIELD_GROUP_PRIO_ANY == pri) {
        groupData.priority = _SBX_CALADAN3_FIELD_GROUP_PRIO_ANY;
    }else{
        groupData.priority = pri;
    }
    groupId = group;
    return _bcm_caladan3_g3p1_field_group_create(unitData, &groupId, &groupData);
}


/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_install
 *   Purpose
 *      Insert all of a group's entries to the hardware.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This inserts and updates all of the group's entries to the hardware as
 *      appropriate.  No error is asserted for entries already in hardware,
 *      even if the entire group is already in hardware.
 *      Since this calls the SBX commit for the database, entries that are
 *      changed even in other groups will also be committed to the hardware.
 */
int
bcm_caladan3_g3p1_field_group_install(void *unitData,
                                    bcm_field_group_t group)
{
    int result = BCM_E_NONE;                  /* working result */
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */
    _bcm_caladan3_field_entry_index entry;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't install invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }

    result = soc_c3_rce_group_install(thisUnit->unit, group);

    for (entry = thisGroup->entryHead; entry < thisUnit->entryTotal; entry = thisUnit->entry[entry].nextEntry) {
        thisUnit->entry[entry].entryFlags |= _CALADAN3_G3P1_ENTRY_IN_HW;
    }


    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisGroup->rulebase);
        /* called function will display error */
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes all of the group's entries from the hardware as
 *      appropriate.  No error is asserted for entries not in hardware,
 *      even if the entire group is not in hardware.
 *      Since this calls the SBX commit for the database, entries that are
 *      changed even in other groups will also be committed to the hardware.
 */
int
bcm_caladan3_g3p1_field_group_remove(void *unitData,
                                   bcm_field_group_t group)
{
    int result = BCM_E_NONE;                  /* working result */
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't install invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }

        result = soc_c3_rce_group_remove(thisUnit->unit, group);

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisGroup->rulebase);
        /* called function will display error */
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_flush
 *   Purpose
 *      Remove all of a group's entries from the hardware, remove the group
 *      from the hardware, remove the group's entries from the software, and
 *      remove the group from the software.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This will destroy the field group and all its entries.  No mention is
 *      made that it affects ranges, so they aren't destroyed.  This also
 *      destroys the field group and its entries in hardware, so it also
 *      commits the appropriate stage entries to the SBX driver.
 */
int
bcm_caladan3_g3p1_field_group_flush(void *unitData,
                                  bcm_field_group_t group)
{
    int result;                               /* working result */
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */
    _bcm_caladan3_field_entry_index entry;        /* working entry ID */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't check qset of invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* get rid of all the entries in the group */
    for (entry = thisGroup->entryHead;
         thisUnit->entryTotal > entry;
         entry = thisGroup->entryHead) {
        result = _bcm_caladan3_g3p1_field_entry_destroy(unitData, entry);
        if (BCM_E_NONE != result) {
            /* called function will display appropriate errors */
            return result;
        }
    }
    /* commit the updates to the driver */
    result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                  thisGroup->rulebase);
    if (BCM_E_NONE != result) {
        /* called function displays error message */
        return result;
    }
    /* get rid of the group */
    result = _bcm_caladan3_g3p1_field_group_del(unitData, group);
    if ((BCM_E_NONE == result) || (BCM_E_CONFIG == result)) {
        /*
         *  We want to try to free the group if we got BCM_E_CONFIG because it
         *  could indicate bad state (a group that, for example, tried to move
         *  between rulebases and failed, getting caught in limbo).
         */
        result = _bcm_caladan3_g3p1_field_group_free(unitData, group);
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type (it will also fail in this case).
 *
 *      Updates are always permitted if there are no entries present.
 *
 *      We don't bother to update any of the rulebase/entry linkages when
 *      moving a group between rulebases because we don't allow any group with
 *      entries to engage in such a move.
 */
int
bcm_caladan3_g3p1_field_group_set(void *unitData,
                                bcm_field_group_t group,
                                bcm_field_qset_t qset)
{
    int result = BCM_E_NONE;                  /* working result */
    int auxRes;                               /* spare result */
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */
    uint8 rulebase;                           /* working rulebase */
    uint8 oldRulebase;                        /* original rulebase */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't check qset of invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (thisGroup->entries) {
        /* the group has entries; make sure new qset is superset of current */
        result = _bcm_caladan3_qset_subset(qset, thisGroup->qset);
        if (BCM_E_FAIL == result) {
            /* new qset is not a superset of current */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d group %08X has entries and new"
                       " qset is not superset of current\n"),
                       thisUnit->unit,
                       group));
            return BCM_E_CONFIG;
        } else if (BCM_E_NONE != result) {
            /* some other error comparing qsets */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to compare new qset to unit %d"
                       " group %08X: %d (%s)\n"),
                       thisUnit->unit,
                       group,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        /* now make sure it still fits in the stage */
        result = _bcm_caladan3_qset_subset(thisUnit->rulebase[thisGroup->rulebase].qset,
                                         qset);
        if (BCM_E_FAIL == result) {
            /* new qset does not fit in this stage */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("new qset does not fit in the group's"
                       " current stage %d\n"),
                       thisGroup->rulebase));
            return BCM_E_CONFIG;
        } else if (BCM_E_NONE != result) {
            /* some other error comparing qsets */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to compare new qset to unit %d"
                       " stage %d qset: %d (%s)\n"),
                       thisUnit->unit,
                       thisGroup->rulebase,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        /* now make sure the LLC field hasn't toggled */
        if (BCM_FIELD_QSET_TEST(thisGroup->qset, bcmFieldQualifyLlc) !=
            BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyLlc)) {
            /* the LLC mode changed; can't allow that with entries */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d group %08X qset LLC differs from"
                       " the new qset LLC; this change affects"
                       " entry format and can not be done with"
                       " entries present\n"),
                       thisUnit->unit,
                       group));
            return BCM_E_CONFIG;
        }
        /* finally, all looks well, so update the qset */
        thisGroup->qset = qset;
        FIELD_EVERB((BSL_META("updated unit %d group %08X qset in place\n"),
                     thisUnit->unit,
                     group));
        return BCM_E_NONE;
    } /* if (thisGroup->entries) */
    /* figure out which stage is appropriate; try ingress first */
    for (rulebase = 0; rulebase < thisUnit->uMaxSupportedStages; rulebase++) {
        result = _bcm_caladan3_qset_subset(thisUnit->rulebase[rulebase].qset,
                                         qset);
        if ((BCM_E_NONE == result) ||
            ((BCM_E_NONE != result) && (BCM_E_FAIL != result))) {
            /* it fits in this rulebase or something went wrong */
            break;
        }
    } /* for (rulebase = 0; rulebase < thisUnit->uMaxSupportedStages; rulebase++) */
    if (BCM_E_FAIL == result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("specified qset does not match any stage"
                   " on unit %d\n"),
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_QSET_ERR;
    } else if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to compare qsets on unit %d: %d (%s)\n"),
                   thisUnit->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    if (rulebase != thisGroup->rulebase) {
        /* need to change stages */
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("need to move unit %d group %08X from rulebase"
                   " %d to rulebase %d...\n"),
                   thisUnit->unit,
                   group,
                   thisGroup->rulebase,
                   rulebase));
        oldRulebase = thisGroup->rulebase;
        result = _bcm_caladan3_g3p1_field_group_del(thisUnit, group);
        if (BCM_E_NONE == result) {
            result = _bcm_caladan3_g3p1_field_group_add(thisUnit, rulebase, group);
        }
        if (BCM_E_NONE == result) {
            /* that went well; update the group's qset */
            thisGroup->qset = qset;
        } else { /* if (BCM_E_NONE == result) */
            if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) {
                /* there was an error, but the state is consistent */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("unable to move unit %d group %08X from"
                           " rulebase %d to rulebase %d: %d (%s)\n"),
                           thisUnit->unit,
                           group,
                           oldRulebase,
                           rulebase,
                           result,
                           _SHR_ERRMSG(result)));
            } else { /* if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) */
                /* there was an error that left the state inconsistent */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("it appears that unit %d group %08X is"
                           " caught in limbo because of an error"
                           " moving between rulebases: %d (%s)\n"),
                           thisUnit->unit,
                           group,
                           result,
                           _SHR_ERRMSG(result)));
                auxRes = _bcm_caladan3_g3p1_field_group_add(thisUnit,
                                                        oldRulebase,
                                                        group);
                if (BCM_E_NONE != auxRes) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META("unable to revert unit %d group %08X"
                               " to rulebase %d: %d (%s)\n"),
                               thisUnit->unit,
                               group,
                               oldRulebase,
                               auxRes,
                               _SHR_ERRMSG(auxRes)));
                    auxRes = _bcm_caladan3_g3p1_field_group_free(thisUnit,
                                                             group);
                    if (BCM_E_NONE == auxRes) {
                        LOG_ERROR(BSL_LS_BCM_FP,
                                  (BSL_META("destroyed unit %d group %08X"
                                   " to avoid inconsistent state\n"),
                                   thisUnit->unit,
                                   group));
                    } else { /* if (BCM_E_NONE != auxRes) */
                        LOG_ERROR(BSL_LS_BCM_FP,
                                  (BSL_META("unit %d group %08X is in an"
                                   " inconsistent state and can"
                                   " only be destroyed, but doing"
                                   " so failed: %d (%s)\n"),
                                   thisUnit->unit,
                                   group,
                                   auxRes,
                                   _SHR_ERRMSG(auxRes)));
                    } /* if (BCM_E_NONE != auxRes) */
                } /* if (BCM_E_NONE != auxRes) */
            } /* if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) */
        } /* if (BCM_E_NONE == result) */
    } else { /* if (rulebase != thisGroup->rulebase) */
        /* just update the qset */
        thisGroup->qset = qset;
        FIELD_EVERB((BSL_META("updated unit %d group %08X qset in place\n"),
                     thisUnit->unit,
                     group));
        result = BCM_E_NONE;
    } /* if (rulebase != thisGroup->rulebase) */
    if (BCM_E_NONE == result) {
        /* mark the qset for the stage, just in case it's not set */
        BCM_FIELD_QSET_ADD(thisGroup->qset, (thisGroup->rulebase?
                                             bcmFieldQualifyStageEgressSecurity:
                                             bcmFieldQualifyStageIngressQoS));
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_get
 *   Purpose
 *      Gets the group's qualifier set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_qset_t *qset = where to put the current qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_group_get(void *unitData,
                                bcm_field_group_t group,
                                bcm_field_qset_t *qset)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(thisUnit->unit,
                              "can't get qset of invalid group ID %08X"
                               " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(thisUnit->unit,
                              "unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    *qset = thisGroup->qset;

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_caladan3_g3p1_field_group_destroy(void *unitData,
                                    bcm_field_group_t group)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can not destroy invalid group %08X on"
                   " unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }

    result = _bcm_caladan3_g3p1_field_group_del(unitData, group);
    if ((BCM_E_NONE == result) || (BCM_E_CONFIG == result)) {
        /*
         *  We want to try to free the group if we got BCM_E_CONFIG because it
         *  could indicate bad state (a group that, for example, tried to move
         *  between rulebases and failed, getting caught in limbo).
         */
        result = _bcm_caladan3_g3p1_field_group_free(unitData, group);
        if (BCM_E_NONE  != result) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to free group 0x%x: %d (%s)\n"),
                       group,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    }

    if (BCM_E_NONE  != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to delete group 0x%x: %d (%s)\n"),
                   group,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[group].rulebase);
        /* called function will display error */
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Frankly, the data returned by
 *      this call will look bogus to anybody used to the XGS implementation.
 */
int
bcm_caladan3_g3p1_field_group_status_get(void *unitData,
                                       bcm_field_group_t group,
                                       bcm_field_group_status_t *status)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_rulebase_t *thisRb;  /* working rulebase data */
    _bcm_caladan3_g3p1_field_group_t *thisGroup;  /* working group data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;  /* working entry data */
    _bcm_caladan3_field_entry_index entry;        /* working entry ID */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(thisUnit->unit,
                              "can't get qset of invalid group ID %08X"
                               " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(thisUnit->unit,
                              "unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    thisRb = &(thisUnit->rulebase[thisGroup->rulebase]);

    /* make sure the buffer is empty */
    bcm_field_group_status_t_init(status);

    /* fill in the known capacities */
    status->entries_total = thisRb->entriesMax;
    status->counters_total = thisRb->entriesMax;
    status->meters_total = 0;

    /* walk the group and fill in things from that path */
    status->counter_count = 0;
    status->entry_count = 0;
    status->meter_count = 0;
    status->prio_min = _SBX_CALADAN3_FIELD_ENTRY_PRIO_HIGHEST;
    status->prio_max = BCM_FIELD_ENTRY_PRIO_LOWEST;
    for (entry = thisGroup->entryHead;
         entry < thisUnit->entryTotal;
         entry = thisUnit->entry[entry].nextEntry) {
        thisEntry = &(thisUnit->entry[entry]);
        status->counter_count++;
        if (0 < _bcm_caladan3_compare_entry_priority(status->prio_max,
                                                   thisEntry->priority)) {
            /* entry priority is higher than highest so far; keep it */
            status->prio_max = thisEntry->priority;
        }
        if (0 > _bcm_caladan3_compare_entry_priority(status->prio_min,
                                                   thisEntry->priority)) {
            /* entry priority is lower than lowest so far; keep it */
            status->prio_min = thisEntry->priority;
        }
    } /* for (all entries in this group) */

    /* fill in the free numbers */
    status->entries_free = thisRb->entriesMax - thisRb->entries;
    status->counters_free = status->entries_total - status->counter_count;
    status->meters_free = 0;

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.  This is translated here instead of using
 *      the list manager feature because we have ranges in a zero-based array.
 *      This, range_create_id, and range_destroy are the places where we hold
 *      two locks at the same time: one for this unit's field work, and one
 *      that is managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 */
int
bcm_caladan3_g3p1_field_range_create(void *unitData,
                                   bcm_field_range_t *range,
                                   uint32 flags,
                                   bcm_l4_port_t min,
                                   bcm_l4_port_t max)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    uint32 tempRange;                         /* working range ID */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* try to reserve the range */
    result = shr_idxres_list_alloc(thisUnit->rangeFree, &tempRange);
    if (BCM_E_NONE == result) {
        /* the range was reserved; create it */
        FIELD_EVERB((BSL_META("create unit %d range"
                                " [alloc %08X] (%08X %08X..%08X)\n"),
                     thisUnit->unit,
                     tempRange,
                     flags,
                     min,
                     max));
        result = _bcm_caladan3_g3p1_field_range_create_id(thisUnit,
                                                        tempRange,
                                                        flags,
                                                        min,
                                                        max);
        if (BCM_E_NONE != result) {
            /* something went wrong; destroy the range */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("error creating unit %d range %08X,"
                       " destroying it\n"),
                       thisUnit->unit,
                       *range));
            sal_memset(&(thisUnit->range[tempRange]),
                       0,
                       sizeof(thisUnit->range[tempRange]));
            shr_idxres_list_free(thisUnit->rangeFree, tempRange);
        }
    }
    if (BCM_E_NONE == result) {
        *range = tempRange;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.  This is translated here instead of using
 *      the list manager feature because we have ranges in a zero-based array.
 *      This, range_create, and range_destroy are the places where we hold two
 *      locks at the same time: one for this unit's field work, and one that is
 *      managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 */
int
bcm_caladan3_g3p1_field_range_create_id(void *unitData,
                                      bcm_field_range_t range,
                                      uint32 flags,
                                      bcm_l4_port_t min,
                                      bcm_l4_port_t max)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* try to reserve the range */
    result = shr_idxres_list_reserve(thisUnit->rangeFree,
                                     range,
                                     range);
    if (BCM_E_NONE == result) {
        /* the range was reserved; create it */
        FIELD_EVERB((BSL_META("create unit %d range"
                                " %08X (%08X %08X..%08X)\n"),
                     thisUnit->unit,
                     range,
                     flags,
                     min,
                     max));
        result = _bcm_caladan3_g3p1_field_range_create_id(thisUnit,
                                                        range,
                                                        flags,
                                                        min,
                                                        max);
        if (BCM_E_NONE != result) {
            /* something went wrong; destroy the range */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("error creating unit %d range %08X,"
                       " destroying it\n"),
                       thisUnit->unit,
                       range));
            sal_memset(&(thisUnit->range[range]),
                       0,
                       sizeof(thisUnit->range[range]));
            shr_idxres_list_free(thisUnit->rangeFree, range);
        }
    } else if (BCM_E_RESOURCE == result) {
        /* translate 'resource not available' to 'it already exists' */
        result = BCM_E_EXISTS;
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_caladan3_g3p1_field_range_get(void *unitData,
                                bcm_field_range_t range,
                                uint32 *flags,
                                bcm_l4_port_t *min,
                                bcm_l4_port_t *max)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* try to access the range */
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, range);
    if (BCM_E_EXISTS == result) {
        /* the range is allocated; adjust return code */
        FIELD_EVERB((BSL_META("get range %08X information\n"), range));
        result = BCM_E_NONE;
        /* fill in the values */
        *flags = thisUnit->range[range].flags;
        *min = thisUnit->range[range].min;
        *max = thisUnit->range[range].max;
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This, range_create, and range_create_id are the places where we hold
 *      two locks at the same time: one for this unit's field work, and one
 *      that is managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 *      Destroying a range has absolutely no effect upon the rules that were
 *      set up using that range, at least on this hardware.
 */
int
bcm_caladan3_g3p1_field_range_destroy(void *unitData,
                                    bcm_field_range_t range)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* try to access the range */
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, range);
    if (BCM_E_EXISTS == result) {
        /* the range is allocated; destroy it */

        result = soc_c3_rce_range_destroy(thisUnit->unit, range);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("soc_c3_rce_range_destroy on unit %d failed %s\n"),
                       thisUnit->unit, _SHR_ERRMSG(result)));
        }

        FIELD_EVERB((BSL_META("destroy range %08X\n"), range));
        sal_memset(&(thisUnit->range[range]),
                   0,
                   sizeof(thisUnit->range[range]));
        /* free the range */
        result = shr_idxres_list_free(thisUnit->rangeFree, range);
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (out) bcm_field_entry_t *entry = where to put the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 */
int
bcm_caladan3_g3p1_field_entry_create(void *unitData,
                                   bcm_field_group_t group,
                                   bcm_field_entry_t *entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_field_entry_index entryId;      /* working entry ID */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        /* group ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid group %08X specified on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        /* group is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* don't care about the entry ID */
    entryId = thisUnit->entryTotal;
    /* create the new entry */
    result = _bcm_caladan3_g3p1_field_entry_create(thisUnit, group, &entryId);
    if (BCM_E_NONE == result) {
        /* success */
        *entry = entryId;
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_create_id
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in) bcm_field_entry_t entry = the entry ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Note
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 */
int
bcm_caladan3_g3p1_field_entry_create_id(void *unitData,
                                      bcm_field_group_t group,
                                      bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_field_entry_index entryId;      /* working entry ID */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        /* group ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid group %08X specified on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        /* group is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* use the specified entry ID */
    entryId = entry;
    /* create the new entry */
    return _bcm_caladan3_g3p1_field_entry_create(thisUnit, group, &entryId);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_destroy
 *   Purpose
 *      Destroy a field entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_caladan3_g3p1_field_entry_destroy(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check parameters */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }

    return _bcm_caladan3_g3p1_field_entry_destroy(thisUnit, entry);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_destroy_all
 *   Purpose
 *      Destroy all field entries
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API spec indicates that this function does not remove any
 *      associated entries from the hardware.  Unfortunately, if we're to avoid
 *      resource leakage, we can't honour this.  We don't *commit* the database
 *      after this call, so the entries are still in the actual hardware, but
 *      they're no longer in the SBX driver, so next commit they *will* die.
 */
int
bcm_caladan3_g3p1_field_entry_destroy_all(void *unitData)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_field_entry_index entry;        /* working entry ID */
    int result = BCM_E_NONE;                  /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* destroy all (active) entries */
    for (entry = 0; entry < thisUnit->entryTotal; entry++) {
        if (thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID) {
            result = bcm_caladan3_g3p1_field_entry_destroy(unitData, entry);
        }
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to delete unit %d entry %08X:"
                       " %d (%s); aborting\n"),
                       thisUnit->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
            break;
        }
    }
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_copy
 *   Purpose
 *      Copy an existing field entry to another one
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (out) bcm_field_entry_t *dst_entry = where to put the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter if not sharing).
 */
int
bcm_caladan3_g3p1_field_entry_copy(void *unitData,
                                 bcm_field_entry_t src_entry,
                                 bcm_field_entry_t *dst_entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_field_entry_index entryId;      /* working entry ID */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= src_entry) || (0 > src_entry)) {
        /* source entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid source entry %08X specified"
                   " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    /* don't care about the new entry ID */
    entryId = thisUnit->entryTotal;
    /* create the new entry */
    result = _bcm_caladan3_g3p1_field_entry_copy(thisUnit, src_entry, &entryId);
    if (BCM_E_NONE == result) {
        /* success */
        *dst_entry = entryId;
    }
    return result;
}


/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_copy_id
 *   Purpose
 *      Copy an existing field entry to a specific one
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (in) bcm_field_entry_t dst_entry = the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter if not sharing).
 */
int
bcm_caladan3_g3p1_field_entry_copy_id(void *unitData,
                                    bcm_field_entry_t src_entry,
                                    bcm_field_entry_t dst_entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_field_entry_index entryId;      /* working entry ID */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= src_entry) || (0 > src_entry)) {
        /* source entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid source entry %08X specified"
                   " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if ((thisUnit->entryTotal <= dst_entry) || (0 > dst_entry)) {
        /* destination entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid destination entry %08X specified"
                   " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    /* set up for the desired new entry ID */
    entryId = dst_entry;
    /* create the new entry */
    return _bcm_caladan3_g3p1_field_entry_copy(thisUnit, src_entry, &entryId);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_install
 *   Purpose
 *      Install a field entry to the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This API is one of a set that will cause poor performance on SBX.
 *      This will commit the appropriate database to the hardware.
 */
int
bcm_caladan3_g3p1_field_entry_install(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* specified entry is not used */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    result = soc_c3_rce_entry_install(thisUnit->unit, entry);

    if (result == BCM_E_NONE) {
        thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_IN_HW;
    }

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_reinstall
 *   Purpose
 *      Reinstall a field entry to the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Apprarently, despite the API doc indicating this can only be used to
 *      refresh an entry that is already in hardware, the regression tests
 *      require that this work to install an entry that is not in hardware.
 */
int
bcm_caladan3_g3p1_field_entry_reinstall(void *unitData,
                                      bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* specified entry is not used */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    result = soc_c3_rce_entry_install(thisUnit->unit, entry);

    if (result == BCM_E_NONE) {
        thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_IN_HW;
    }

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_remove
 *   Purpose
 *      Remove a field entry from the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API doc indicates that this can only be used against an entry that
 *      is already in hardware, but the regression tests require that it work
 *      even if the entry isn't in hardware.
 */
int
bcm_caladan3_g3p1_field_entry_remove(void *unitData,
                                   bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* specified entry is not used */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /*
     *  Should check and return error if not in hardware according to API doc,
     *  but regression tests demand that this call succeed in that case.
     */
    result = soc_c3_rce_entry_remove(thisUnit->unit, entry);

    if (result == BCM_E_NONE) {
        thisEntry->entryFlags &= (~_CALADAN3_G3P1_ENTRY_IN_HW);
    }

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_g3p1_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }

    return result;
}


/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_prio_get
 *   Purpose
 *      Get the priority of a specific entry (within its group)
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) int *prio = where to put the entry's priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 */
int
bcm_caladan3_g3p1_field_entry_prio_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     int *prio)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    *prio = thisUnit->entry[entry].priority;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_prio_set
 *   Purpose
 *      Set the priority of a specific entry (within its group)
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int prio = the entry's new priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings.
 *      Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 */
int
bcm_caladan3_g3p1_field_entry_prio_set(void *unitData,
                                     bcm_field_entry_t entry,
                                     int prio)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    int result;                                 /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (0 > prio) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry priority %d specified for"
                   " unit %d entry %08X\n"),
                   prio,
                   thisUnit->unit,
                   entry));
        return BCM_E_PARAM;
    }
    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* remove the entry from its current position in its group */
    result = _bcm_caladan3_g3p1_field_entry_del(thisUnit, entry);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* change the priority */
    if (prio == BCM_FIELD_ENTRY_PRIO_HIGHEST) {
        thisUnit->entry[entry].priority = _SBX_CALADAN3_FIELD_ENTRY_PRIO_HIGHEST;
    }else{
        thisUnit->entry[entry].priority = prio;
    }
    /* reinsert the entry to the group according to its new priority */
    return _bcm_caladan3_g3p1_field_entry_add(thisUnit,
                                          thisUnit->entry[entry].group,
                                          entry);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_clear
 *   Purpose
 *      Clear all qualifiers for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_qualify_clear(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    int result = BCM_E_NONE;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }

    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }


        /* Clear qualifiers for this entry */
        result = soc_c3_rce_entry_qualify_clear(thisUnit->unit,
                                              entry);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("soc_c3_rce_entry_qualify_clear failed: %s\n"),
                       bcm_errmsg(result)));
        }

    return result;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_qualify_Port
 *   Purpose
 *      Set expected port qualifier for an entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_gport_t gport = gport as qualifier
 *      (in) bcm_field_qualify_t qual = which qualifier to consider
 *      (in) int apply = TRUE to apply, FALSE to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 *      Accepts 'gport' for port, and will dissect it to find the hardware port
 *      if applicable.
 *      Clobbers 'ports' qualifiers.
 */
static int
_bcm_caladan3_g3p1_field_qualify_Port(_bcm_caladan3_g3p1_field_glob_t *thisUnit,
                                    const bcm_field_entry_t entry,
                                    const bcm_gport_t gport,
                                    const bcm_port_t mask,
                                    const bcm_field_qualify_t qual)
{
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       portData[1];
    unsigned char                       portMask[1];
    int                                 result;
    bcm_module_t                        locMod = -1;   /* local module */
    bcm_module_t                        tgtMod = -1;   /* target module */
    bcm_port_t                          tgtPort = -1;  /* target port */

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   qual);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (mask != 0) {
        /* adding port qualifier; map the target port */
        result = _bcm_caladan3_map_vlan_gport_target(thisUnit->unit,
                                                   gport,
                                                   &locMod,
                                                   &tgtMod,
                                                   &tgtPort,
                                                   NULL,
                                                   NULL);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to get target data for unit %d"
                       " port %08X: %d (%s)\n"),
                       thisUnit->unit,
                       gport,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        if (tgtMod != locMod) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d port %08X is not local,"
                       " so can't set it as a filter\n"),
                       thisUnit->unit,
                       gport));
            return BCM_E_PARAM;
        }
    } else {
        tgtPort = 0;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset,
                                                qual,
                                                &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write Port(%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 gport,
                 thisUnit->unit,
                 entry));

    portData[0] = tgtPort;
    portMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           portData,
                                           portMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_InPort
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = which ingress port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_InPort(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_port_t data,
                                     bcm_port_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    if (0x7f < data ) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Port %04X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }
    if (0x7f < mask ) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Mask %04X is not valid\n"),
                   mask));
        return BCM_E_PARAM;
    }

    /* update the port */
    return _bcm_caladan3_g3p1_field_qualify_Port(thisUnit,
                                                 entry,
                                                 data,
                                                 mask,
                                                 bcmFieldQualifyInPort);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OutPort
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = which egress port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_OutPort(void *unitData,
                                      bcm_field_entry_t entry,
                                      bcm_port_t data,
                                      bcm_port_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    if (0x7f < data ) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Port %04X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }
    if (0x7f < mask ) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Mask %04X is not valid\n"),
                   mask));
        return BCM_E_PARAM;
    }

    /* update the port */
    return _bcm_caladan3_g3p1_field_qualify_Port(thisUnit,
                                               entry,
                                               data,
                                               mask,
                                               bcmFieldQualifyOutPort);
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_qualify_Ports_get
 *   Purpose
 *      Get expected port(s) qualifier for an entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_qualify_t qual = which qualifier to consider
 *      (out) bcm_gport_t *gport = where to put 'port' qualifier data
 *      (out) bcm_gport_t *gpmask = where to put 'port' qualifier mask
 *      (out) bcm_pbmp_t *pbmp = where to put 'ports' qualifier data
 *      (out) bcm_pbmp_t *pbmask = where to put 'ports' qualifier mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will only fill in gport or pbmp, according to the qualifier given.
 *
 *      Will return BCM_E_CONFIG if can't represent appropriately (such as
 *      multiple ports in 'ports' qualifier but asked for 'port' qualifier).
 *
 *      Returned GPORT is ModPort variety, if 'port' mode.
 *
 *      The unwanted return pointer can be NULL.
 *
 *      Will return BCM_E_NOT_FOUND if the entry applies to all ports 
 */
static int
_bcm_caladan3_g3p1_field_qualify_Ports_get(_bcm_caladan3_g3p1_field_glob_t *thisUnit,
                                         const bcm_field_entry_t entry,
                                         const bcm_field_qualify_t qual,
                                         bcm_port_t *gport,
                                         bcm_port_t *gpmask)
{
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       portData[1];
    unsigned char                       portMask[1];
    int result;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   qual);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset,
                                                qual,
                                                &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           portData,
                                           portMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *gport = portData[0];
    *gpmask = portMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_InPort
 *   Purpose
 *      Get expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_vlan_t *data = where to put ingress port
 *      (out) bcm_vlan_t *mask = where to put ingress port mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will return BCM_E_CONFIG if entry is qualified on multiple ports.
 */
int
bcm_caladan3_g3p1_field_qualify_InPort_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         bcm_port_t *data,
                                         bcm_port_t *mask)
{
    /* get the port qualifier */
    return _bcm_caladan3_g3p1_field_qualify_Ports_get((_bcm_caladan3_g3p1_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyInPort,
                                                    data,
                                                    mask);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OutPort_get
 *   Purpose
 *      Get expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_vlan_t *data = where to put ingress port
 *      (out) bcm_vlan_t *mask = where to put ingress port mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will return BCM_E_CONFIG if entry is qualified on multiple ports.
 */
int
bcm_caladan3_g3p1_field_qualify_OutPort_get(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_port_t *data,
                                          bcm_port_t *mask)
{
    /* get the port qualfier */
    return _bcm_caladan3_g3p1_field_qualify_Ports_get((_bcm_caladan3_g3p1_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyOutPort,
                                                    data,
                                                    mask);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OuterVlan
 *   Purpose
 *      Set expected outer VLAN tag for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = VLAN tag data (16 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all zeroes or ones for supported subfields.  While we
 *      accept a nonzero mask for CFI, it is ignored with a warning.
 *
 *      BCM API docs neglect to metion that this function should apply to the
 *      entire tag, rather than just the VID.
 */
int
bcm_caladan3_g3p1_field_qualify_OuterVlan(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_vlan_t data,
                                        bcm_vlan_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[2];
    unsigned char                       vlanMask[2];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlan);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFFFF != mask) &&
        (0xEFFF != mask) &&
        (0xE000 != mask) &&
        (0x0FFF != mask) &&
        (0x0000 != mask)) {
        /*
         *  We accept FFFF as mask even though we ignore CFI in classifier:
         *  this is legacy behaviour that our customers probably expect.
         *
         *  We do check PRI and VID, so we also accept masks appropriately.
         */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d can only mask %s based upon"
                   " entire PRI or entire VID or both.\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlan]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlan, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write OuterVlan(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    vlanData[0] = data & 0xff;
    vlanData[1] = (data >> 8) & 0xff;
    vlanMask[0] = mask & 0xff;
    vlanMask[1] = (mask >> 8) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;

}
int
bcm_caladan3_g3p1_field_qualify_OuterVlan_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_vlan_t *data,
                                        bcm_vlan_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[2];
    unsigned char                       vlanMask[2];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlan);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlan, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = vlanData[0] + (vlanData[1] << 8);
    *mask = vlanMask[0] + (vlanMask[1] << 8);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VID for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = VLAN tag data (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_OuterVlanId(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_vlan_t data,
                                          bcm_vlan_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[2];
    unsigned char                       vlanMask[2];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanId);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if (0xFFF < data) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("VID %04X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanId, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write OuterVlanId(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    vlanData[0] = data & 0xff;
    vlanData[1] = (data >> 8) & 0xff;
    vlanMask[0] = mask & 0xff;
    vlanMask[1] = (mask >> 8) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}
int
bcm_caladan3_g3p1_field_qualify_OuterVlanId_get(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_vlan_t *data,
                                          bcm_vlan_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[2];
    unsigned char                       vlanMask[2];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanId);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanId, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = vlanData[0] + (vlanData[1] <<8);
    *mask = vlanMask[0] + (vlanMask[1] <<8);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer pri for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = VLAN pri data (3 bits)
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN Pri to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_OuterVlanPri(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 data,
                                           uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[1];
    unsigned char                       vlanMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanPri);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFF != mask) && (0x07 != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing mask"
                   "for %s\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlanPri]));
        return BCM_E_PARAM;
    }

    if (0x07 < data) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Pri %02X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanPri, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write OuterVlanPri(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    vlanData[0] = data & 0xff;
    vlanMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_OuterVlanPri_get(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 *data,
                                           uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[1];
    unsigned char                       vlanMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanPri);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanPri, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = vlanData[0];
    *mask = vlanMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_OuterVlanCfi
 *   Purpose
 *      Set expected outer pri for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = VLAN pri data (3 bits)
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN Pri to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_OuterVlanCfi(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 data,
                                           uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[1];
    unsigned char                       vlanMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanCfi);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0x01 != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing mask"
                   "for %s\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyOuterVlanCfi]));
        return BCM_E_PARAM;
    }

    if (0x01 < data) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("Pri %02X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanCfi, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write OuterVlanCfi(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    vlanData[0] = data & 0xff;
    vlanMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_OuterVlanCfi_get(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 *data,
                                           uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       vlanData[1];
    unsigned char                       vlanMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanCfi);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyOuterVlanCfi, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           vlanData,
                                           vlanMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = vlanData[0];
    *mask = vlanMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_EtherType
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VLAN
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the EtherType to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_EtherType(void *unitData,
                                        bcm_field_entry_t entry,
                                        uint16 data,
                                        uint16 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       etData[2];
    unsigned char                       etMask[2];
    int result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyEtherType);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFFFF != mask) && (0x0000 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyEtherType]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyEtherType, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write EtherType(%04X,%04X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    etData[0] = data & 0xff;
    etData[1] = (data >> 8) & 0xff;
    etMask[0] = mask & 0xff;
    etMask[1] = (mask >> 8) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           etData,
                                           etMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_g3p1_field_qualify_EtherType_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        uint16 *data,
                                        uint16 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       etData[2];
    unsigned char                       etMask[2];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyEtherType);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyEtherType, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           etData,
                                           etMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = etData[0] + (etData[1] << 8);
    *mask = etMask[0] + (etMask[1] << 8);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_IpProtocol(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIpProtocol);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFF != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyIpProtocol]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIpProtocol, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write IpProtocol(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    protoData[0] = data & 0xff;
    protoMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_IpProtocol_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIpProtocol);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIpProtocol, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = protoData[0] & 0xff;
    *mask = protoMask[0] & 0xff;

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_SrcIp(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    bcm_ip_t prefBuff;                          /* working prefix buffer */
    unsigned char                       ipdaData[4];
    unsigned char                       ipdaMask[4];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~mask) + 1) & (~mask)) {
        /* mask is not prefix form with significant bits set */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifySrcIp]));
        return BCM_E_PARAM;
    }

    /* figure out the mask length by shifting left until it's zero */
    for (prefBuff = mask, prefLen = 0;
         prefBuff && (prefLen < 32);
         prefBuff <<= 1, prefLen++) {
        /* just iterate */
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcIp, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write SrcIp(%08X,%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    ipdaData[0] = data & 0xff;
    ipdaData[1] = (data >> 8) & 0xff;
    ipdaData[2] = (data >> 16) & 0xff;
    ipdaData[3] = (data >> 24) & 0xff;
    ipdaMask[0] = mask & 0xff;
    ipdaMask[1] = (mask >> 8) & 0xff;
    ipdaMask[2] = (mask >> 16) & 0xff;
    ipdaMask[3] = (mask >> 24) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipdaData,
                                           ipdaMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;

}
int
bcm_caladan3_g3p1_field_qualify_SrcIp_get(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t *data,
                                    bcm_ip_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       ipsaData[4];
    unsigned char                       ipsaMask[4];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcIp, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipsaData,
                                           ipsaMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    *data = ipsaData[0] + (ipsaData[1] << 8) + (ipsaData[2] << 16) + (ipsaData[3] << 24);
    *mask = ipsaMask[0] + (ipsaMask[1] << 8) + (ipsaMask[2] << 16) + (ipsaMask[3] << 24);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_DstIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_DstIp(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       ipdaData[4];
    unsigned char                       ipdaMask[4];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~mask) + 1) & (~mask)) {
        /* mask is not prefix form with significant bits set */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyDstIp]));
        return BCM_E_PARAM;
    }
    FIELD_EVERB((BSL_META("write DstIp(%08X,%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstIp, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write DstIp(%08X,%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    ipdaData[0] = data & 0xff;
    ipdaData[1] = (data >> 8) & 0xff;
    ipdaData[2] = (data >> 16) & 0xff;
    ipdaData[3] = (data >> 24) & 0xff;
    ipdaMask[0] = mask & 0xff;
    ipdaMask[1] = (mask >> 8) & 0xff;
    ipdaMask[2] = (mask >> 16) & 0xff;
    ipdaMask[3] = (mask >> 24) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipdaData,
                                           ipdaMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_DstIp_get(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t *data,
                                    bcm_ip_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       ipdaData[4];
    unsigned char                       ipdaMask[4];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstIp, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipdaData,
                                           ipdaMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    *data = ipdaData[0] + (ipdaData[1] << 8) + (ipdaData[2] << 16) + (ipdaData[3] << 24);
    *mask = ipdaMask[0] + (ipdaMask[1] << 8) + (ipdaMask[2] << 16) + (ipdaMask[3] << 24);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_L4SrcPort
 *   Purpose
 *      Set expected L4 source port
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_l4_port_t data = destination port
 *      (in) bcm_l4_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_caladan3_g3p1_field_qualify_L4SrcPort(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_l4_port_t data,
                                        bcm_l4_port_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       l4PortData[2];
    unsigned char                       l4PortMask[2];
    int result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyL4SrcPort);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFFFF != mask) && (0x0000 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyL4SrcPort]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyL4SrcPort, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write L4DstPort(%04X,%04X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    l4PortData[0] = data & 0xff;
    l4PortData[1] = (data >> 8) & 0xff;
    l4PortMask[0] = mask & 0xff;
    l4PortMask[1] = (mask >> 8) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           l4PortData,
                                           l4PortMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_g3p1_field_qualify_L4SrcPort_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_l4_port_t *data,
                                        bcm_l4_port_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       l4PortData[2];
    unsigned char                       l4PortMask[2];
    int result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyL4SrcPort);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyL4SrcPort, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           l4PortData,
                                           l4PortMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = l4PortData[0] + (l4PortData[1] << 8);
    *mask = l4PortMask[0] + (l4PortMask[1] << 8);

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_L4DstPort
 *   Purpose
 *      Set expected L4 destination port
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_l4_port_t data = destination port
 *      (in) bcm_l4_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_caladan3_g3p1_field_qualify_L4DstPort(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_l4_port_t data,
                                        bcm_l4_port_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       l4PortData[2];
    unsigned char                       l4PortMask[2];
    int result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyL4DstPort);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFFFF != mask) && (0x0000 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyL4DstPort]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyL4DstPort, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    /* Write out the entry data */
    FIELD_EVERB((BSL_META("write L4DstPort(%04X,%04X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    l4PortData[0] = data & 0xff;
    l4PortData[1] = (data >> 8) & 0xff;
    l4PortMask[0] = mask & 0xff;
    l4PortMask[1] = (mask >> 8) & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           l4PortData,
                                           l4PortMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_g3p1_field_qualify_L4DstPort_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_l4_port_t *data,
                                        bcm_l4_port_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       l4PortData[2];
    unsigned char                       l4PortMask[2];
    int result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyL4DstPort);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyL4DstPort, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }
    
    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           l4PortData,
                                           l4PortMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = l4PortData[0] + (l4PortData[1] << 8);
    *mask = l4PortMask[0] + (l4PortMask[1] << 8);

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_DSCP(void *unitData,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;
    unsigned char                       dscpData[1];
    unsigned char                       dscpMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDSCP);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0x3F != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyDSCP]));
        return BCM_E_PARAM;
    }
    if (0xC0 & data) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid value %02X for unit %d %s\n"),
                   data,
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyDSCP]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDSCP, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write DSCP(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    dscpData[0] = data & 0xff;
    dscpMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           dscpData,
                                           dscpMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;

}
int
bcm_caladan3_g3p1_field_qualify_DSCP_get(void *unitData,
                                   bcm_field_entry_t entry,
                                   uint8 *data,
                                   uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       dscpData[1];
    unsigned char                       dscpMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDSCP);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDSCP, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           dscpData,
                                           dscpMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = dscpData[0];
    *mask = dscpMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_TcpControl
 *   Purpose
 *      Set expected TCP control flags for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which TCP control bits
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Also implies TCP as protocol.
 */
int
bcm_caladan3_g3p1_field_qualify_TcpControl(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       controlData[1];
    unsigned char                       controlMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyTcpControl);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~(BCM_FIELD_TCPCONTROL_FIN |
            BCM_FIELD_TCPCONTROL_SYN |
            BCM_FIELD_TCPCONTROL_RST |
            BCM_FIELD_TCPCONTROL_PSH |
            BCM_FIELD_TCPCONTROL_ACK |
            BCM_FIELD_TCPCONTROL_URG)) & mask) ||
        ((~(BCM_FIELD_TCPCONTROL_FIN |
            BCM_FIELD_TCPCONTROL_SYN |
            BCM_FIELD_TCPCONTROL_RST |
            BCM_FIELD_TCPCONTROL_PSH |
            BCM_FIELD_TCPCONTROL_ACK |
            BCM_FIELD_TCPCONTROL_URG)) & data) /* ||
        ((~mask) & data)*/) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid combination of TCP control mask %02X"
                   " and data %02X\n"),
                   mask,
                   data));
        return BCM_E_PARAM;
    }

    /* normalise the data to under the mask */
    data &= mask;

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyTcpControl, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write TcpControl(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    controlData[0] = data & 0xff;
    controlMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           controlData,
                                           controlMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_TcpControl_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       controlData[1];
    unsigned char                       controlMask[1];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyTcpControl);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyTcpControl, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           controlData,
                                           controlMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    *data = controlData[0];
    *mask = controlMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_caladan3_g3p1_field_qualify_RangeCheck(void *unitData,
                                         bcm_field_entry_t entry,
                                         bcm_field_range_t range,
                                         int invert)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_range_t     *thisRange;     /* working range data */
    int                             result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyRangeCheck);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* make sure the range is valid and exists */
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, range);
    if (BCM_E_EXISTS != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to access unit %d range %08X: %d (%s)\n"),
                   thisUnit->unit,
                   range,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /* get a pointer to the range; it's faster */
    thisRange = &(thisUnit->range[range]);

    if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
        thisUnit->entry[entry].range[0] = range;
    }
    if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
        thisUnit->entry[entry].range[1] = range;
    }
    thisRange->invert = invert;
    FIELD_EVERB((BSL_META("write RangeCheck(%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 range,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    result = soc_c3_rce_entry_qualify_range_set(thisUnit->unit, entry, range, invert ? -1 : 1);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_range_set failed on unit %d: %s\n"),
                   thisUnit->unit, _SHR_ERRMSG(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_RangeCheck_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         int max_count,
                                         bcm_field_range_t *range,
                                         int *invert,
                                         int *count)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    int result, i;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyRangeCheck);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* rangeIds are not reflected in hardware */
    *count = 0;
    for (i=0; i<_FIELD_CALADAN3_G3P1_RANGES_PER_ENTRY; i++) {
        if ( (*count) == max_count ) {
            break;
        }
        range[*count] = thisUnit->entry[entry].range[i];
        invert[*count] = thisUnit->range[thisUnit->entry[entry].range[i]].invert;
        if (range[*count]) {
            (*count)++;
        }
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_SrcMac(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t data,
                                     bcm_mac_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                 /* mask working byte */
    uint8 bit;                                  /* mask working bit */
    unsigned char                       macData[6];
    unsigned char                       macMask[6];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* ensure MAC address is prefix masked */
    byte = 0;
    maskLen = 0;
    while ((byte < 6) && (0xFF == mask[byte])) {
        byte++;
        maskLen += 8;
    }
    if (byte < 6) {
        if (((~(mask[byte])) & 0xFF) & (((~(mask[byte])) + 1) & 0xFF)) {
            /* this byte is not prefix masked */
            result = BCM_E_PARAM;
        }
        for (bit = 0x80; bit & (mask[byte]); bit = bit >> 1) {
            /* current byte has this bit set */
            maskLen++;
        }
        byte++;
        while (byte < 6) {
            if (mask[byte]) {
                /* not prefix masked if bits are set down here */
                result = BCM_E_PARAM;
            }
            byte++;
        }
    }

    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifySrcMac]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcMac, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write SrcMac(" FIELD_MACA_FORMAT ","
                            FIELD_MACA_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    macData[0] = data[5];
    macData[1] = data[4];
    macData[2] = data[3];
    macData[3] = data[2];
    macData[4] = data[1];
    macData[5] = data[0];
    macMask[0] = mask[5];
    macMask[1] = mask[4];
    macMask[2] = mask[3];
    macMask[3] = mask[2];
    macMask[4] = mask[1];
    macMask[5] = mask[0];
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           macData,
                                           macMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_SrcMac_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t *data,
                                     bcm_mac_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    int                             result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       macData[6];
    unsigned char                       macMask[6];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcMac, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           macData,
                                           macMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    (*data)[0] = macData[5];
    (*data)[1] = macData[4];
    (*data)[2] = macData[3];
    (*data)[3] = macData[2];
    (*data)[4] = macData[1];
    (*data)[5] = macData[0];
    (*mask)[0] = macMask[5];
    (*mask)[1] = macMask[4];
    (*mask)[2] = macMask[3];
    (*mask)[3] = macMask[2];
    (*mask)[4] = macMask[1];
    (*mask)[5] = macMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_DstMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_DstMac(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t data,
                                     bcm_mac_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                /* working byte */
    uint8 bit;                                  /* mask working bit */
    unsigned char                       macData[6];
    unsigned char                       macMask[6];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* ensure MAC address is prefix masked */
    byte = 0;
    maskLen = 0;
    while ((byte < 6) && (0xFF == mask[byte])) {
        byte++;
        maskLen += 8;
    }
    if (byte < 6) {
        if (((~(mask[byte])) & 0xFF) & (((~(mask[byte])) + 1) & 0xFF)) {
            /* this byte is not prefix masked */
            result = BCM_E_PARAM;
        }
        for (bit = 0x80; bit & (mask[byte]); bit = bit >> 1) {
            /* current byte has this bit set */
            maskLen++;
        }
        byte++;
        while (byte < 6) {
            if (mask[byte]) {
                /* not prefix masked if bits are set down here */
                result = BCM_E_PARAM;
            }
            byte++;
        }
    }
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyDstMac]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstMac, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write DstMac(" FIELD_MACA_FORMAT ","
                            FIELD_MACA_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    macData[0] = data[5];
    macData[1] = data[4];
    macData[2] = data[3];
    macData[3] = data[2];
    macData[4] = data[1];
    macData[5] = data[0];
    macMask[0] = mask[5];
    macMask[1] = mask[4];
    macMask[2] = mask[3];
    macMask[3] = mask[2];
    macMask[4] = mask[1];
    macMask[5] = mask[0];
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           macData,
                                           macMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

int
bcm_caladan3_g3p1_field_qualify_DstMac_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t *data,
                                     bcm_mac_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    int                                 result;
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       macData[6];
    unsigned char                       macMask[6];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstMac, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           macData,
                                           macMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    (*data)[0] = macData[5];
    (*data)[1] = macData[4];
    (*data)[2] = macData[3];
    (*data)[3] = macData[2];
    (*data)[4] = macData[1];
    (*data)[5] = macData[0];
    (*mask)[0] = macMask[5];
    (*mask)[1] = macMask[4];
    (*mask)[2] = macMask[3];
    (*mask)[3] = macMask[2];
    (*mask)[4] = macMask[1];
    (*mask)[5] = macMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_SrcIp6
 *   Purpose
 *      Set expected source IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_SrcIp6(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    int i,j;
    int isPrefixOver=0;
    unsigned char                       ipData[16];
    unsigned char                       ipMask[16];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* figure out the mask length by shifting left until it's zero */
    prefLen = 0;
    for (i = 0; i < 16 ; i++) {
        for(j = 7; j >= 0; j--) {
            if(!(mask[i] & ( 1 << j))) {
                isPrefixOver = 1;
            } else if(isPrefixOver) {
                /* mask is not prefix form with significant bits set */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("unit %d only supports prefix %s mask\n"),
                           thisUnit->unit,
                           _sbx_caladan3_field_qual_name[bcmFieldQualifySrcIp6]));
                return BCM_E_PARAM;
            }
            
            if(!isPrefixOver) {
                prefLen++;
            }
        }
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcIp6, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write SrcIp6(" FIELD_IPV6A_FORMAT ","
                            FIELD_IPV6A_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 thisUnit->unit,
                 entry));

    for (i = 0; i < 16; i++) {
        ipData[15-i] = data[i];
        ipMask[15-i] = mask[i];
    }

    /* update the entry with the new rules */
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipData,
                                           ipMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_SrcIp6_get
 *   Purpose
 *      Get source IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_SrcIp6_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t *data,
                                     bcm_ip6_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;
    int i;
    unsigned char                       ipData[16];
    unsigned char                       ipMask[16];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifySrcIp6, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipData,
                                           ipMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    for (i = 0; i < 16; i++) {
        (*data)[i] = ipData[15-i];
        (*mask)[i] = ipMask[15-i];
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_DstIp6
 *   Purpose
 *      Set expected destination IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which destination IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_DstIp6(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    int i,j;
    int isPrefixOver=0;
    unsigned char                       ipData[16];
    unsigned char                       ipMask[16];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* figure out the mask length by shifting left until it's zero */
    prefLen = 0;
    for (i = 0; i < 16 ; i++) {
        for(j = 7; j >= 0; j--) {
            if(!(mask[i] & ( 1 << j))) {
                isPrefixOver = 1;
            } else if(isPrefixOver) {
                /* mask is not prefix form with significant bits set */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("unit %d only supports prefix %s mask\n"),
                           thisUnit->unit,
                           _sbx_caladan3_field_qual_name[bcmFieldQualifyDstIp6]));
                return BCM_E_PARAM;
            }
            
            if(!isPrefixOver) {
                prefLen++;
            }
        }
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstIp6, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write DstIp6(" FIELD_IPV6A_FORMAT ","
                            FIELD_IPV6A_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 thisUnit->unit,
                 entry));

    for (i = 0; i < 16; i++) {
        ipData[15-i] = data[i];
        ipMask[15-i] = mask[i];
    }

    /* update the entry with the new rules */
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipData,
                                           ipMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_DstIp6_get
 *   Purpose
 *      Get destination IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which destination IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_g3p1_field_qualify_DstIp6_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t *data,
                                     bcm_ip6_t *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    int result;
    int i;
    unsigned char                       ipData[16];
    unsigned char                       ipMask[16];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyDstIp6, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           ipData,
                                           ipMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }

    for (i = 0; i < 16; i++) {
        (*data)[i] = ipData[15-i];
        (*mask)[i] = ipMask[15-i];
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_Ip6NextHeader
 *   Purpose
 *      Set expected IPv6 Next Header for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_Ip6NextHeader(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIp6NextHeader);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFF != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyIp6NextHeader]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIp6NextHeader, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write Ip6NextHeader(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    protoData[0] = data & 0xff;
    protoMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_Ip6NextHeader_get
 *   Purpose
 *      Get IPv6 Next Header for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_Ip6NextHeader_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIp6NextHeader);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIp6NextHeader, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }
    *data = protoData[0];
    *mask = protoMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass
 *   Purpose
 *      Set expected IPv6 Traffic Class for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIp6TrafficClass);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    if ((0xFF != mask) && (0x00 != mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_caladan3_field_qual_name[bcmFieldQualifyIp6TrafficClass]));
        return BCM_E_PARAM;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIp6TrafficClass, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    FIELD_EVERB((BSL_META("write Ip6TrafficClass(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    protoData[0] = data & 0xff;
    protoMask[0] = mask & 0xff;
    result = soc_c3_rce_entry_qualify_set(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_set failed: %s\n"),
                   bcm_errmsg(result)));
    }

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass_get
 *   Purpose
 *      Get IPv6 Traffic Class for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    _bcm_caladan3_field_group_index     group;
    unsigned int                        qualIndex;
    unsigned char                       protoData[1];
    unsigned char                       protoMask[1];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_caladan3_g3p1_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIp6TrafficClass);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Find the qualify index for this qualify type */
    group = thisUnit->entry[entry].group;
    result = _bcm_caladan3_g3p1_qual_index_find(thisUnit->group[group].qset, bcmFieldQualifyIp6TrafficClass, &qualIndex);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_caladan3_g3p1_qual_index_find failed: %s\n"),
                   bcm_errmsg(result)));
        return result;
    }

    result = soc_c3_rce_entry_qualify_get(thisUnit->unit,
                                           entry,
                                           qualIndex,
                                           protoData,
                                           protoMask);
    if (result != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_qualify_get failed: %s\n"),
                   bcm_errmsg(result)));
    }
    *data = protoData[0];
    *mask = protoMask[0];

    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_action_add
 *   Purpose
 *      Add a specific action to a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to add
 *      (in) uint32 param0 = action parameter 0 (some actions)
 *      (in) uint32 param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_g3p1_field_action_add(void *unitData,
                                 bcm_field_entry_t entry,
                                 bcm_field_action_t action,
                                 uint32 param0,
                                 uint32 param1)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_g3p1_fld_entry_flags_t newFlags = 0;  /* working flags */
    _bcm_caladan3_g3p1_fld_entry_flags_t chkFlags;  /* working flags */
    int result;                                 /* working result */
    unsigned char                       data[4];
    bcm_mirror_destination_t            mirror_dest;
    unsigned int                        mirrorId = 0;
    int destNode;                               /* working target node */
    int fabUnit;                                /* working fabric unit */
    int fabPort;                                /* working fabric port */
    uint32 ftIndex = ~0;                        /* working FT index */
    unsigned char                       qos_q_new;
    int                                 ingress;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    result = _bcm_caladan3_g3p1_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* Ingress or egress action */
    if (thisUnit->group[thisUnit->entry[entry].group].rulebase == 0) {
        ingress = TRUE;
    } else {
        ingress = FALSE;
    }

    /* get the entry qualifiers and actions so we can update them */
    newFlags = thisUnit->entry[entry].entryFlags;
    chkFlags = 0;

    switch (action) {
    case bcmFieldActionVlanNew:
        if (param0 > SBX_DYNAMIC_VSI_END(thisUnit->unit)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d only supports VSI %04X..%04X\n"),
                       thisUnit->unit,
                       0,
                       SBX_DYNAMIC_VSI_END(thisUnit->unit) + 1));
            return BCM_E_PARAM;
        }
        /* Write VLAN */
        data[0] = param0 & 0xff;
        data[1] = (param0 >> 8) & 0xff;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionNewVsi].index, data);
        if (result == BCM_E_NONE) {
            /* Write VLAN enable */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewVsi].modIndex, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_VLANNEW;
        break;
    case bcmFieldActionL3Switch:
        if (!(_CALADAN3_L3_FTE_VALID(thisUnit->unit, param0))) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d only supports FT values"
                       " %d..%d\n"),
                       thisUnit->unit,
                       0,
                       0x0003FFFF));
            return BCM_E_PARAM;
        }
        /* Write FT index */
        data[0] = param0 & 0xff;
        data[1] = (param0 >> 8) & 0xff;
        data[2] = (param0 >> 16) & 0xff;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionNewFtIndex].index, data);
        if (result == BCM_E_NONE) {
            /* Write FT index enable */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewFtIndex].modIndex, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        if ((param0 >= SBX_MAX_MODIDS) || (param1 >= SBX_MAX_PORTS)) {
            /* port or module invalid; bad parameter */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("module %d or port %d is invalid\n"),
                       param0,
                       param1));
            return BCM_E_PARAM;
        }
        result = soc_sbx_node_port_get(thisUnit->unit,
                                       param0, /* modid */
                                       param1, /* port */
                                       &fabUnit,
                                       &destNode,
                                       &fabPort);
        if (BCM_E_NONE == result) {
            /* make sure target node is accessible */
            if (!SOC_SBX_NODE_ADDRESSABLE(thisUnit->unit, destNode)) {
                /* inaccessible destination node */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META("node %d (module %d port %d) is"
                           "inaccessible from unit %d\n"),
                           destNode,
                           param0,
                           param1,
                           thisUnit->unit));
                return BCM_E_BADID;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to locate redirect target for mod"
                       " %d port %d: %d (%s)\n"),
                       param0,
                       param1,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        ftIndex = SOC_SBX_PORT_FTE(thisUnit->unit, destNode, fabPort);
        /* Write FT index */
        data[0] = ftIndex & 0xff;
        data[1] = (ftIndex >> 8) & 0xff;
        data[2] = (ftIndex >> 16) & 0xff;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionNewFtIndex].index, data);
        if (result == BCM_E_NONE) {
            /* Write FT index enable */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewFtIndex].modIndex, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_REDIRECT;
        break;
    case bcmFieldActionCopyToCpu:
        data[0] = 1;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionCopyToCpu].index, data);
        chkFlags = _CALADAN3_G3P1_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionDropPrecedence:
        if (param0 >= 4  ) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d only suports DP values"
                       " %d..%d\n"),
                       thisUnit->unit,
                       0,
                       4 ));
            return BCM_E_PARAM;
        }
        /* Write drop precedence */
        data[0] = param0;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionNewDp].index, data);
        if (result == BCM_E_NONE) {
            /* Write drop precedence enable */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewDp].modIndex, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_DROPPREC;
        break;
    case bcmFieldActionCosQNew:
        if (param0 >= SBX_MAX_COS) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d only supports COS queues %d..%d\n"),
                       thisUnit->unit,
                       0,
                       SBX_MAX_COS));
            return BCM_E_PARAM;
        }
        /* Write cos */
        qos_q_new = param0 & 0x7;
        data[0] = qos_q_new + (qos_q_new << 3);
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionNewPrio].index, data);
        if (result == BCM_E_NONE) {
            /* Write drop precedence enable */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewPrio].modIndex, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_COSQNEW;
        break;
    case bcmFieldActionMirrorIngress:
        /* mirror destination gport */
        if (BCM_GPORT_IS_MIRROR(param1)) {
            result = bcm_mirror_destination_get(thisUnit->unit, param1, &mirror_dest);
            if (BCM_E_NONE != result) {
                return result;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("Invalid Mirror port (%08X)\n"),
                       param1));
            return BCM_E_PARAM;
        }
        /* Write mirror ID */
        data[0] = BCM_GPORT_MIRROR_GET(param1);
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            ingressActionMap[socC3RceActionMirror].index, data);
        chkFlags = _CALADAN3_G3P1_ENTRY_MIRROR;
        break;
    case bcmFieldActionPolicerLevel0:
        /* Write policer ID */
        result = bcm_caladan3_g3p1_field_entry_policer_attach(thisUnit,
                                                              entry,
                                                              0,
                                                              param1);
        chkFlags = _CALADAN3_G3P1_ENTRY_POLICER;
        break;
    case bcmFieldActionDrop:
        if (ingress) {
            /* Ingress drop is via the drop all policer */
            data[0] = thisUnit->dropPolicer & 0xff;
            data[1] = (thisUnit->dropPolicer >> 8) & 0xff;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionPolicer].index, data);
            if (result == BCM_E_NONE) {
                /* Drop policer is a simple policer so clear modifier bits */
                data[0] = 0;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionPolicer].modIndex, data);
            }
        } else {
            /* Egress drop is via explicit drop */
            data[0] = 1;
            result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                egressActionMap[socC3RceActionDrop].index, data);
        }
        chkFlags = _CALADAN3_G3P1_ENTRY_DROP;
        break;
    case bcmFieldActionMirrorEgress:
        /* mirror destination gport */
        if (BCM_GPORT_IS_MIRROR(param1)) {
            result = bcm_mirror_destination_get(thisUnit->unit, param1, &mirror_dest);
            if (BCM_E_NONE != result) {
                return result;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("Invalid Mirror port (%08X)\n"),
                       param1));
            return BCM_E_PARAM;
        }
        result = _bcm_caladan3_egr_mirror_alloc(thisUnit->unit,
                                              &mirrorId,
                                              mirror_dest.gport);
        if (BCM_E_NONE != result) {
            /* called function should have displayed reason */
            return result;
        }
        /* Write mirror ID */
        data[0] = mirrorId;
        result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
            egressActionMap[socC3RceActionMirror].index, data);
        chkFlags = _CALADAN3_G3P1_ENTRY_MIRROR;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unexpected action %s\n"),
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_INTERNAL;
    }

    /* anticipate success here */
    newFlags |= chkFlags;

    if (BCM_E_NONE == result) {
        /* Update entry flags */
        FIELD_EVERB((BSL_META("update unit %d entry %08X flags"
                                " %08X -> %08X\n"),
                     thisUnit->unit,
                     entry,
                     thisUnit->entry[entry].entryFlags,
                     newFlags));
        thisUnit->entry[entry].entryFlags = newFlags;
        switch (action) {
        case bcmFieldActionVlanNew:
            thisUnit->entry[entry].ftvlData.VSI = param0;
            break;
        case bcmFieldActionL3Switch:
            thisUnit->entry[entry].ftvlData.ftHandle = param0;
            break;
        case bcmFieldActionRedirect:
            thisUnit->entry[entry].ftvlData.target.module = param0;
            thisUnit->entry[entry].ftvlData.target.port = param1;
            break;
        default:
            /* to keep compiler from complaining */
            break;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("soc_c3_rce_entry_action_set failed: %s\n"),
                   soc_errmsg(result)));

        /* clean up any resources allocated */
        if ((bcmFieldActionMirrorEgress == action) && mirrorId) {
            _bcm_caladan3_egr_mirror_free(thisUnit->unit, mirrorId);
        }

    }

    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_action_get
 *   Purpose
 *      Get a specific action from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *      (out) uint32 param0 = where to put action parameter 0 (some actions)
 *      (out) uint32 param1 = where to put action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 *      If the action does not use one or both of param0 or param1, the unused
 *      will be zeroed.
 */
int
bcm_caladan3_g3p1_field_action_get(void *unitData,
                                 bcm_field_entry_t entry,
                                 bcm_field_action_t action,
                                 uint32 *param0,
                                 uint32 *param1)
{
    _bcm_caladan3_g3p1_field_glob_t         *thisUnit;      /* working unit data */
    _bcm_caladan3_g3p1_fld_entry_flags_t    oldFlags;  /* working flags */
    int                                     result;
    bcm_gport_t                             gportId;
    bcm_policer_t                           policer;
    unsigned char                           data[6];
    int                                     modid;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    result = _bcm_caladan3_g3p1_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    oldFlags = thisUnit->entry[entry].entryFlags;

    /* make sure the action exists */
    switch (action) {
    case bcmFieldActionDrop:
        oldFlags &= _CALADAN3_G3P1_ENTRY_DROP;
        break;
    case bcmFieldActionCopyToCpu:
        oldFlags &= _CALADAN3_G3P1_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionVlanNew:
        oldFlags &= _CALADAN3_G3P1_ENTRY_VLANNEW;
        break;
    case bcmFieldActionCosQNew:
        oldFlags &= _CALADAN3_G3P1_ENTRY_COSQNEW;
        break;
    case bcmFieldActionDropPrecedence:
        oldFlags &= _CALADAN3_G3P1_ENTRY_DROPPREC;
        break;
    case bcmFieldActionL3Switch:
        oldFlags &= _CALADAN3_G3P1_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        oldFlags &= _CALADAN3_G3P1_ENTRY_REDIRECT;
        break;
    case bcmFieldActionRedirectMcast:
        oldFlags &= _CALADAN3_G3P1_ENTRY_REDIRMC;
        break;
    case bcmFieldActionPolicerLevel0:
        oldFlags &= _CALADAN3_G3P1_ENTRY_POLICER;
        break;
    case bcmFieldActionMirrorIngress:
        /* fallthrough intentional; same flag for ingress & egress */
    case bcmFieldActionMirrorEgress:
        oldFlags &= _CALADAN3_G3P1_ENTRY_MIRROR;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unexpected action %s\n"),
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_INTERNAL;
    } /* switch (action) */
    if (!oldFlags) {
        /* the action does not exist */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X has no %s action\n"),
                   thisUnit->unit,
                   entry,
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_NOT_FOUND;
    }

    /* deal with cached action types (shortcut) */
    switch (action) {
    case bcmFieldActionRedirect:
        *param0 = thisUnit->entry[entry].ftvlData.target.module;
        *param1 = thisUnit->entry[entry].ftvlData.target.port;
        return BCM_E_NONE;
    case bcmFieldActionL3Switch:
        *param0 = thisUnit->entry[entry].ftvlData.ftHandle;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionVlanNew:
        *param0 = thisUnit->entry[entry].ftvlData.VSI;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionRedirectMcast:
        *param0 = thisUnit->entry[entry].ftvlData.mcHandle;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionCopyToCpu:
        /* no arguments, so fall through to Drop (also no args) */
    case bcmFieldActionDrop:
        *param0 = 0;
        *param1 = 0;
        return BCM_E_NONE;
    default:
        /* so the compiler doesn't complain */
        break;
    }

    /* fill in the results based upon the data in the rule */
    FIELD_EVERB((BSL_META("read %s action from unit %d entry %08X\n"),
                 _sbx_caladan3_field_action_name[action],
                 thisUnit->unit,
                 entry));
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
    case 0:
        switch (action) {
        case bcmFieldActionMirrorIngress:
            result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE) {
                result = _bcm_caladan3_ingr_mirror_get(thisUnit->unit,
                                                     data[0],
                                                     &gportId);
            }
            if (result == BCM_E_NONE) {
                if (BCM_GPORT_IS_MODPORT(gportId)) {
                    *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                    *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
                }
            }
            break;
        case bcmFieldActionDropPrecedence:
            result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewDp].index, data);
            if (result == BCM_E_NONE) {
                *param0 = data[0];
            }
            *param1 = 0;
            break;
        case bcmFieldActionCosQNew:
            result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                ingressActionMap[socC3RceActionNewPrio].index, data);
            if (result == BCM_E_NONE) {
                *param0 = (data[0] & 0x7);
            }
            *param1 = 0;
            break;
        case bcmFieldActionPolicerLevel0:
            result = bcm_stk_modid_get(thisUnit->unit, &modid);
            if (result == BCM_E_NONE) {
                result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionPolicer].index, data);
                if (result == BCM_E_NONE) {
                    policer = data[0] + (data[1] << 8);
                    *param0 = modid;
                    *param1 = policer;
                }
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    case 1:
        switch (action) {
        case bcmFieldActionMirrorEgress:
            result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                egressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                result = _bcm_caladan3_egr_mirror_get(thisUnit->unit,
                                                    data[0],
                                                    &gportId);
                /* called function should have displayed any error diagnostic */
                
                if (BCM_E_NONE == result) {
                    if (BCM_GPORT_IS_MODPORT(gportId)) {
                        *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                        *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
                    }
                }
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    default:
        /* should never reach this */
        result = BCM_E_INTERNAL;
    } /* switch (rulebase) */
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unable to read %s action from unit %d"
                   " entry %08X: %d (%s)\n"),
                   _sbx_caladan3_field_action_name[action],
                   thisUnit->unit,
                   entry,
                   result,
                   _SHR_ERRMSG(result)));
    }
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_action_remove
 *   Purpose
 *      Remove a specific action from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_g3p1_field_action_remove(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_field_action_t action)
{
    _bcm_caladan3_g3p1_field_glob_t         *thisUnit;      /* working unit data */
    _bcm_caladan3_g3p1_fld_entry_flags_t    newFlags;  /* working flags */
    _bcm_caladan3_g3p1_fld_entry_flags_t    chkFlags;  /* working flags */
    int                                     result;
    unsigned int                            mirrorId = 0;
    unsigned char                           data[4];

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    result = _bcm_caladan3_g3p1_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    newFlags = thisUnit->entry[entry].entryFlags;
    chkFlags = 0;

    /* make sure the action exists */
    switch (action) {
    case bcmFieldActionVlanNew:
        chkFlags = _CALADAN3_G3P1_ENTRY_VLANNEW;
        break;
    case bcmFieldActionL3Switch:
        chkFlags = _CALADAN3_G3P1_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        chkFlags = _CALADAN3_G3P1_ENTRY_REDIRECT;
        break;
    case bcmFieldActionCopyToCpu:
        chkFlags = _CALADAN3_G3P1_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionDropPrecedence:
        chkFlags = _CALADAN3_G3P1_ENTRY_DROPPREC;
        break;
    case bcmFieldActionCosQNew:
        chkFlags = _CALADAN3_G3P1_ENTRY_COSQNEW;
        break;
    case bcmFieldActionMirrorIngress:
        /* fallthrough intentional; same flag for ingress & egress */
    case bcmFieldActionMirrorEgress:
        chkFlags = _CALADAN3_G3P1_ENTRY_MIRROR;
        break;
    case bcmFieldActionPolicerLevel0:
        chkFlags = _CALADAN3_G3P1_ENTRY_POLICER;
        break;
    case bcmFieldActionDrop:
        chkFlags = _CALADAN3_G3P1_ENTRY_DROP;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unexpected action %s\n"),
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_INTERNAL;
    } /* switch (action) */

    if (!(newFlags & chkFlags)) {
        /* the action does not exist */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X has no %s action\n"),
                   thisUnit->unit,
                   entry,
                   _sbx_caladan3_field_action_name[action]));
        return BCM_E_NOT_FOUND;
    }

    /* anticipate success here */
    newFlags &= (~chkFlags);

    /* remove the action from the entry's rule */
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            switch (action) {
            case bcmFieldActionVlanNew:
                /* Write VLAN */
                data[0] = (ingressActionMap[socC3RceActionNewVsi].disableVal) & 0xff;
                data[1] = (ingressActionMap[socC3RceActionNewVsi].disableVal >> 8) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionNewVsi].index, data);
                if (result == BCM_E_NONE) {
                    /* Write VLAN enable */
                    data[0] = 0;
                    result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                        ingressActionMap[socC3RceActionNewVsi].modIndex, data);
                }
                break;
            case bcmFieldActionL3Switch:
                /* fallthrough intentional; same fields affected */
            case bcmFieldActionRedirect:
                /* Write FT index */
                data[0] = (ingressActionMap[socC3RceActionNewFtIndex].disableVal) & 0xff;
                data[1] = (ingressActionMap[socC3RceActionNewFtIndex].disableVal >> 8) & 0xff;
                data[2] = (ingressActionMap[socC3RceActionNewFtIndex].disableVal >> 16) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionNewFtIndex].index, data);
                if (result == BCM_E_NONE) {
                    /* Write FT index enable */
                    data[0] = 0;
                    result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                        ingressActionMap[socC3RceActionNewFtIndex].modIndex, data);
                }
                break;
            case bcmFieldActionCopyToCpu:
                data[0] = (ingressActionMap[socC3RceActionCopyToCpu].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionCopyToCpu].index, data);
                break;
            case bcmFieldActionDropPrecedence:
                /* Write drop precedence */
                data[0] = (ingressActionMap[socC3RceActionNewDp].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionNewDp].index, data);
                if (result == BCM_E_NONE) {
                    /* Write drop precedence enable */
                    data[0] = 0;
                    result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                        ingressActionMap[socC3RceActionNewDp].modIndex, data);
                }
                break;
            case bcmFieldActionCosQNew:
                /* Write cos */
                data[0] = (ingressActionMap[socC3RceActionNewPrio].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionNewPrio].index, data);
                if (result == BCM_E_NONE) {
                    /* Write cos enable */
                    data[0] = 0;
                    result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                        ingressActionMap[socC3RceActionNewPrio].modIndex, data);
                }
                break;
            case bcmFieldActionMirrorIngress:
                /* Read mirror ID so it can be freed */
                result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionMirror].index, data);
                if (result == BCM_E_NONE) {
                    mirrorId = data[0];
                }
                /* Write mirror ID */
                data[0] = (ingressActionMap[socC3RceActionMirror].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionMirror].index, data);
                break;
            case bcmFieldActionPolicerLevel0:
                result = bcm_caladan3_g3p1_field_entry_policer_detach(thisUnit,
                                                              entry,
                                                              0);
                break;
            case bcmFieldActionDrop:
                /* When remove drop action, set policer to the
                 * value in the entry which may be zero
                 * (no policer) or the ID of a policer that
                 * was attached before the drop action was enabled.
                 */
                data[0] = thisUnit->entry[entry].policer & 0xff;
                data[1] = (thisUnit->entry[entry].policer >> 8) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    ingressActionMap[socC3RceActionPolicer].index, data);
                if (result == BCM_E_NONE) {
                    data[0] = 0;
                    if (thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_TYPEPOL) {
                        data[0] |= 1;
                    }
                    if (thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_MEFCOS) {
                        data[0] |= 2;
                    }
                    if (thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_MEF) {
                        data[0] |= 4;
                    }
                    result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                        ingressActionMap[socC3RceActionPolicer].modIndex, data);
                }
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 1:
            switch (action) {
            case bcmFieldActionDrop:
                data[0] = (egressActionMap[socC3RceActionDrop].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    egressActionMap[socC3RceActionDrop].index, data);
                break;
            case bcmFieldActionMirrorEgress:
                /* Read mirror ID so it can be freed */
                result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                    egressActionMap[socC3RceActionMirror].index, data);
                if (result == BCM_E_NONE) {
                    mirrorId = data[0];
                }
                data[0] = (egressActionMap[socC3RceActionMirror].disableVal) & 0xff;
                result = soc_c3_rce_entry_action_set(thisUnit->unit, entry, 
                    egressActionMap[socC3RceActionMirror].index, data);
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
    } /* switch (rulebase) */

    if (BCM_E_NONE == result) {
        FIELD_EVERB((BSL_META("remove %s action from unit %d entry %08X\n"),
                     _sbx_caladan3_field_action_name[action],
                     thisUnit->unit,
                     entry));
    } else { /* if (BCM_E_NONE == result) */
        switch (result) {
        case BCM_E_NOT_FOUND:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d entry %08X does not have any"
                       " %s action\n"),
                       thisUnit->unit,
                       entry,
                       _sbx_caladan3_field_action_name[action]));
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to remove %s action from unit %d"
                       " entry %08X: %d (%s)\n"),
                       _sbx_caladan3_field_action_name[action],
                       thisUnit->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
        } /* switch (result) */
    } /* if (BCM_E_NONE == result) */

    if (BCM_E_NONE == result) {
        /* commit any changes that are conditional upon success */
        FIELD_EVERB((BSL_META("update unit %d entry %08X flags"
                                " %08X -> %08X\n"),
                     thisUnit->unit,
                     entry,
                     thisUnit->entry[entry].entryFlags,
                     newFlags));
        thisUnit->entry[entry].entryFlags = newFlags;

        /* update any cached data */
        switch (action) {
        case bcmFieldActionRedirect:
            thisUnit->entry[entry].ftvlData.target.module = 0;
            thisUnit->entry[entry].ftvlData.target.port = 0;
            break;
        case bcmFieldActionL3Switch:
            thisUnit->entry[entry].ftvlData.ftHandle = 0;
            break;
        case bcmFieldActionVlanNew:
            thisUnit->entry[entry].ftvlData.VSI = 0;
            break;
#ifdef C3_REDIRECT_SUPPORTED
        case bcmFieldActionRedirectMcast:
            thisUnit->entry[entry].ftvlData.mcHandle = 0;
            break;
#endif
        default:
            /* to keep compiler from complaining */
            break;
        }

        /* clean up any resources we need to free now */
        FIELD_EVERB((BSL_META("release resources used by %s action"
                                " from unit %d entry %08X\n"),
                     _sbx_caladan3_field_action_name[action],
                     thisUnit->unit,
                     entry));
        if ((bcmFieldActionMirrorEgress == action) && mirrorId) {
            _bcm_caladan3_egr_mirror_free(thisUnit->unit, mirrorId);
        }


    } /* if (BCM_E_NONE == result) */

    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_action_remove_all
 *   Purpose
 *      Remove all actions from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_g3p1_field_action_remove_all(void *unitData,
                                        bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t     *thisUnit;      /* working unit data */
    unsigned char                       data[4];
    int                                 result;

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }

    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }

    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 1:
            /* Read mirror ID so it can be freed */
            result = soc_c3_rce_entry_action_get(thisUnit->unit, entry, 
                egressActionMap[socC3RceActionMirror].index, data);
            if (result == BCM_E_NONE && data[0] != 0) {
                _bcm_caladan3_egr_mirror_free(thisUnit->unit, data[0]);
            }
            break;

        default:
            break;
    }

    /* remove the actions from this entry's rules */
    result = _bcm_caladan3_g3p1_field_rules_clear_actions(thisUnit,
                                                      entry);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* update the entry with the new rules */
    FIELD_EVERB((BSL_META("remove all actions from unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));

    /* commit any changes that are conditional upon success */
    FIELD_EVERB((BSL_META("update unit %d entry %08X flags"
                            " %08X -> %08X\n"),
                 thisUnit->unit,
                 entry,
                 thisUnit->entry[entry].entryFlags,
                 thisUnit->entry[entry].entryFlags & (~_CALADAN3_G3P1_ENTRY_ACTIONS)));

    /* make sure no actions are indicated */
    thisUnit->entry[entry].entryFlags &= (~_CALADAN3_G3P1_ENTRY_ACTIONS);
    sal_memset(&(thisUnit->entry[entry].ftvlData),
               0,
               sizeof(thisUnit->entry[entry].ftvlData));

    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_counter_create
 *   Purpose
 *      'Create' a counter for the specified entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This really doesn't do anything on the SBX hardware, which maintains
 *      counters for all rules.  What it does is update the software state to
 *      indicate that this particular entry has a counter.
 */
int
bcm_caladan3_g3p1_field_counter_create(void *unitData,
                                     bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_COUNTER) {
        /* entry already has a counter */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X already has a counter\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_EXISTS;
    }
    FIELD_EVERB((BSL_META("enabling counter on unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));
    thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_COUNTER;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_counter_destroy
 *   Purpose
 *      Remove the entry's counter
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If an entry is not sharing a counter, this merely turns off the counter
 *      for that entry.  If the entry is sharing a counter, it removes that
 *      entry from the sharing list and then disables the counter for that
 *      entry (so the end result is the specified entry has no counter but any
 *      other entries that shared with it are left alone).
 */
int
bcm_caladan3_g3p1_field_counter_destroy(void *unitData,
                                      bcm_field_entry_t entry)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_COUNTER)) {
        /* entry already has a counter */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X has no counter\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_EMPTY;
    }
    FIELD_EVERB((BSL_META("disabling counter on unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));
    /* adjust group potential counter count */
    thisUnit->group[thisEntry->group].counters++;
    thisEntry->entryFlags &= ~_CALADAN3_G3P1_ENTRY_COUNTER;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_counter_set
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only supported value is zero.
 *      We ignore counter_num because we don't have a way to only clear one
 *      side or the other.
 */
int
bcm_caladan3_g3p1_field_counter_set(void *unitData,
                                  bcm_field_entry_t entry,
                                  int counter_num,
                                  uint64 val)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    soc_sbx_g3p1_turbo64_count_t tempCounts;    /* working counters */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > counter_num) || (1 < counter_num)) {
        /* counter number is bogus */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid counter number %d\n"),
                   counter_num));
        return BCM_E_PARAM;
    }
    if (!COMPILER_64_IS_ZERO(val)) {
        /* can't do any writes but zero */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d does not support arbitrary counter"
                   " writes -- only reading or clearing\n"),
                   thisUnit->unit));
        return BCM_E_PARAM;
    }

    return _bcm_caladan3_g3p1_field_entries_counter_access(thisUnit,
                                                       entry,
                                                       TRUE,
                                                       &tempCounts);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_counter_get
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 *val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only supported value is zero.
 *      We ignore counter_num because we don't have a way to only clear one
 *      side or the other.
 */
int
bcm_caladan3_g3p1_field_counter_get(void *unitData,
                                  bcm_field_entry_t entry,
                                  int counter_num,
                                  uint64 *val)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;      /* working unit data */
    soc_sbx_g3p1_turbo64_count_t tempCounts;    /* working counters */
    int result;                                 /* working result */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }

    result = _bcm_caladan3_g3p1_field_entries_counter_access(thisUnit,
                                                         entry,
                                                         FALSE,
                                                         &tempCounts);
    /* return appropriate count if no errors */
    if (BCM_E_NONE == result) {
        if (counter_num) {
            /* caller wants 'upper' counter */
            *val = tempCounts.bytes;
        } else {
            /* caller wants 'lower' counter */
            *val = tempCounts.packets;
        }
    }
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_field_policer_manipulate
 *   Purpose
 *      Manipulate (get,set,clear) the policer on a given entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in/out) bcm_policer_t *policer_id = pointer to the policer ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
static int
_bcm_caladan3_g3p1_field_policer_manipulate(_bcm_caladan3_g3p1_field_glob_t *glob,
                                          const bcm_field_entry_t entry,
                                          bcm_policer_t *policer_id,
                                          int action)
{
    _bcm_caladan3_g3p1_field_entry_t *thisEntry;    /* working entry data */
    bcm_policer_t policer = 0;                  /* working policer ID */
    bcm_policer_group_mode_t polMode;           /* working policer mode */
    int result = BCM_E_NONE;                    /* working result */
    int typedPolice = FALSE;                    /* working policer mode flag */
    int mef = FALSE;                            /* working policer mode flag */
    int mefCos = FALSE;                         /* working policer mode flag */
    unsigned char                       data[4];

    /* check entry validity */
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("invalid entry %08X specified"
                   " on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        /* entry is not in use */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }

    /* check for and execute read if appropriate */
    if (_SBX_CALADAN3_FIELD_POLICER_READ == action) {
        /* reading; just get it from the cache (h/w read has special cases) */
        if (thisEntry->policer) {
            /* entry has a policer */
            *policer_id = thisEntry->policer;
            return BCM_E_NONE;
        } else {
            /* entry has no policer */
            
            return BCM_E_NOT_FOUND;
        }
    }

    /* prep for write if appropriate */
    if (_SBX_CALADAN3_FIELD_POLICER_WRITE == action) {
        if (thisEntry->policer) {
            /* entry already has associated policer; must dissociate first */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d entry %08X already has policer"
                       " %08X; dissociate before replacing\n"),
                       glob->unit,
                       entry,
                       thisEntry->policer));
            return BCM_E_EXISTS;
        }
        /* get information about this policer */
        policer = *policer_id;
        result = _bcm_caladan3_policer_group_mode_get(glob->unit,
                                                    policer,
                                                    &polMode);
        if (BCM_E_NONE != result) {
            /* error accessing the policer; assume it isn't valid */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unable to access unit %d policer %08X:"
                       " %d (%s)\n"),
                       glob->unit,
                       policer,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        switch (polMode) {
        case bcmPolicerGroupModeSingle:
            break;
        case bcmPolicerGroupModeTyped:
            typedPolice = TRUE;
            break;
        case bcmPolicerGroupModeTypedIntPri:
            mefCos = TRUE;
            /* fallthrough intentional */
        case bcmPolicerGroupModeTypedAll:
            mef = TRUE;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unexpected policer mode %d on unit %d"
                       " policer %08X\n"),
                       polMode,
                       glob->unit,
                       policer));
            return BCM_E_CONFIG;
        } /* switch (mode) */
    } /* if (_SBX_CALADAN3_FIELD_POLICER_WRITE == action) */

    /* prep for clear if appropriate */
    if (_SBX_CALADAN3_FIELD_POLICER_CLEAR == action) {
        if (!thisEntry->policer) {
            /* entry has no associated policer; can't dissociate nothing */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("unit %d entry %08X has no policer\n"),
                       glob->unit,
                       entry));
            return BCM_E_NOT_FOUND;
        }
    } /* if (_SBX_CALADAN3_FIELD_POLICER_CLEAR == action) */

    /* clear or set the policer as appropriate */
    if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_DROP)) {
        /* we're not dropping, so can modify hardware */

        /* update entry rule */
        switch (glob->group[glob->entry[entry].group].rulebase) {
        case 0:
            /* ingress */
            /* Write policer ID */
            data[0] = policer & 0xff;
            data[1] = (policer >> 8) & 0xff;
            result = soc_c3_rce_entry_action_set(glob->unit, entry, 
                ingressActionMap[socC3RceActionPolicer].index, data);
            if (result == BCM_E_NONE) {
                /* Write Policer mode flags */
                data[0] = 0;
                if (typedPolice) {
                    data[0] |= 1;
                }
                if (mefCos) {
                    data[0] |= 2;
                }
                if (mef) {
                    data[0] |= 4;
                }
                result = soc_c3_rce_entry_action_set(glob->unit, entry, 
                    ingressActionMap[socC3RceActionPolicer].modIndex, data);
            }
            break;
#if 0
        case 1:
            /* egress */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("policing not supported on egress rules"
                       " for unit %d\n"),
                       glob->unit));
            return BCM_E_UNAVAIL;
            break;
#endif
        default:
            /* should never reach this */
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    } /* if (!(thisEntry->entryFlags & _CALADAN3_G3P1_ENTRY_DROP)) */

    /* update the entry information to reflect the policer */
    thisEntry->policer = policer;
    if (mef) {
        thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_MEF;
    } else {
        thisEntry->entryFlags &= (~_CALADAN3_G3P1_ENTRY_MEF);
    }
    if (mefCos) {
        thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_MEFCOS;
    } else {
        thisEntry->entryFlags &= (~_CALADAN3_G3P1_ENTRY_MEFCOS);
    }
    if (typedPolice) {
        thisEntry->entryFlags |= _CALADAN3_G3P1_ENTRY_TYPEPOL;
    } else {
        thisEntry->entryFlags &= (~_CALADAN3_G3P1_ENTRY_TYPEPOL);
    }
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_policer_attach
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not allow direct replace; must dissociate first.
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_g3p1_field_entry_policer_attach(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level,
                                           bcm_policer_t policer_id)
{

    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* we don't allow policer zero */
    if (!policer_id) {
        /* policer zero isn't valid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("policer %08X is not valid\n"),
                   policer_id));
        return BCM_E_PARAM;
    }
    /* okay, so now set the policer */
    return _bcm_caladan3_g3p1_field_policer_manipulate((_bcm_caladan3_g3p1_field_glob_t*)unitData,
                                                     entry_id,
                                                     &policer_id,
                                                     _SBX_CALADAN3_FIELD_POLICER_WRITE);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_policer_detach
 *   Purpose
 *      Detach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_g3p1_field_entry_policer_detach(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level)
{
    bcm_policer_t policer;

    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* okay, so now clear the policer */
    return _bcm_caladan3_g3p1_field_policer_manipulate((_bcm_caladan3_g3p1_field_glob_t*)unitData,
                                                     entry_id,
                                                     &policer,
                                                     _SBX_CALADAN3_FIELD_POLICER_CLEAR);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_policer_detach_all
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_entry_policer_detach_all(void *unitData,
                                               bcm_field_entry_t entry_id)
{
    /* we don't support heirarchical, so just detach level 0  */
    return bcm_caladan3_g3p1_field_entry_policer_detach(unitData, entry_id, 0);
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_policer_get
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t *policer_id = where to put current policer
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not allow direct replace; must dissociate first.
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_g3p1_field_entry_policer_get(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level,
                                           bcm_policer_t *policer_id)
{
    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* okay, so now set the policer */
    return _bcm_caladan3_g3p1_field_policer_manipulate((_bcm_caladan3_g3p1_field_glob_t*)unitData,
                                                     entry_id,
                                                     policer_id,
                                                     _SBX_CALADAN3_FIELD_POLICER_READ);
}


/*
 *   Function
 *      bcm_caladan3_g3p1_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_show(void *unitData,
                           const char *pfx)
{
#ifdef BROADCOM_DEBUG
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    return _bcm_caladan3_g3p1_field_dump(thisUnit, pfx);
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */
}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_entry_dump
 *   Purpose
 *      Dump information about the specified entry to debug output
 *   Parameters
 *      (in) _bcm_fe2k_g2p2_field_glob_t *thisUnit = pointer to unit info
 *      (in) _field_entry_index entry = the entry ID
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.  Under most cases, this will
 *      return BCM_E_NONE unconditionally.  The only time when it would not is
 *      if it is unable to fetch the rule data from the SBX driver.
 */
int
bcm_caladan3_g3p1_field_entry_dump(void *unitData,
                                 bcm_field_entry_t entry)
{
#ifdef BROADCOM_DEBUG
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't dump invalid entry ID %08X"
                   " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _CALADAN3_G3P1_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    return _bcm_caladan3_g3p1_field_entry_dump(unitData, entry, "");
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */

}

/*
 *   Function
 *      bcm_caladan3_g3p1_field_group_dump
 *   Purpose
 *      Dump information about the specified group to debug output
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_g3p1_field_group_dump(void *unitData,
                                 bcm_field_group_t group)
{
#ifdef BROADCOM_DEBUG

    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("can't dump invalid group ID %08X"
                   " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_CALADAN3_FIELD_INVALID_ID_ERR;
    }
#if !_SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE
    /* only verify in-use if *not* in diagnostic mode */
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
#endif /* !_SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE */
    return _bcm_caladan3_g3p1_field_group_dump(unitData, group, "");
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */

}

/*
 *  Function
 *     bcm_caladan3_g3p1_field_unit_data_size_get
 *  Purpose
 *     To provide the size of the unit data size
 *  Parameters
 *     (in)  unitData
 *     (out) unit_data_size
 *  Returns
 *     None
 */
void
bcm_caladan3_g3p1_field_unit_data_size_get(void         *unitData, 
                                           unsigned int *unit_data_size)
{
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)unitData;

    *unit_data_size = 
            ((sizeof(_bcm_caladan3_g3p1_field_group_t) * thisUnit->groupTotal) +
                  (sizeof(_bcm_caladan3_g3p1_field_entry_t) * thisUnit->entryTotal) +
                  (sizeof(_bcm_caladan3_field_range_t) * thisUnit->rangeTotal));
    return;
}

#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

