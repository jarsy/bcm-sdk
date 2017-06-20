/*
 * $Id: trunk.c,v 1.94 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    trunk.c
 * Purpose: BCM level APIs for Link Aggregation (a.k.a Trunking)
 */

#define SBX_HASH_DEFINED 0
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <soc/mcm/memacc.h>
#include <soc/sbx/sirius.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/sirius/multicast.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>

/*****************************************************************************
 *
 *  Implementation
 */

/*
 *  WARNING: This module assumes that higig oversubscription will only occur
 *  when higig bandwidth allocation is greater than the maximum supported speed
 *  for the higig ports in an oversubscribed aggregate.  If the customer tries
 *  to run the higig ports at a speed that is lower, and requires the system
 *  support oversubscription, the speed test in _bcm_sirius_trunk_set must be
 *  changed to use the appropriate speed.
 */

#define _SBX_SIRIUS_LAG_EXCESS_VERBOSITY TRUE
#if _SBX_SIRIUS_LAG_EXCESS_VERBOSITY
#define LAG_EVERB(stuff)      LOG_DEBUG(BSL_LS_BCM_TRUNK, stuff) 
#else /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */
#define LAG_EVERB(stuff)
#endif /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */

/*
 *  We need a bitmap of targets, for internal use when filling in the squelch
 *  table and tracking membership (to refuse to include a single target in
 *  multiple aggregates, for example).
 */
#define _LAG_TARGET_SIZE ((_SIRIUS_MC_MAX_TARGETS + 31) / 32)
typedef uint32 _lag_target_bitmap[_LAG_TARGET_SIZE];

/*
 *  Need module plus port...  Also want to keep the target selected by the
 *  module plus port cached for better performance and simpler code path in
 *  certain cases.
 */
typedef struct _modPort_s {
    bcm_gport_t gport;
    bcm_module_t module;
    unsigned int target;
    unsigned int count;
} _modPort_t;

/*
 *  This describes a single aggregate.  These are dynamically allocated instead
 *  of statically because they're a bit big -- over 8.5Kbits worst case each,
 *  and we rather expect sparse population.
 *
 *  origGport is a list of the original GPORTs in the aggregate, for 'get' type
 *  APIs that require the original values back somehow.  We don't actually use
 *  this internally, though...  It's formatted as an array, and attached in its
 *  own alloc cell because its size will vary between aggregates and can even
 *  vary between previous and new values when setting an aggregate's members.
 *
 *  targetCount is the number of targets in this aggregate (thence both the
 *  number of 'set' bits in the targets bitmap and the number of elements in
 *  the origGport array).
 *
 *  targets is a bitmap containing the targets that are participating in this
 *  specific aggregate.  This is how aggregate members are tracked here.
 */
typedef struct _sirius_aggregate_s {
    _modPort_t *origGport;                      /* ptr to original GPORT IDs */
    unsigned int targetCount;                   /* number of targets */
    _lag_target_bitmap targets;                 /* all member targets */
} _sirius_aggregate_t;

/*
 *  This describes all of the aggregates on a unit.
 *
 *  lock is the unit lock for aggregates.
 *
 *  targets is a bitmap indicating which targets are participating in
 *  aggregates (*all* aggregates).  It is used as a fast way to prevent the
 *  inclusion of a single target in multiple aggregates.
 *
 *  aggr is an array of pointers to the unit's aggregates' information.
 *
 *  fab is an array of pointers to the unit's fabric aggregates' information.
 *
 *  hgSubport is the subport ID for the higig ports.  The associated target is
 *  used by multicast when the higig is in XGS mode, and used here if the higig
 *  is in XGS mode and in a higig fabric aggregate.  This will be an impossible
 *  value if there has been no such subport allocated.
 *
 *  ohgSubport is the subport ID matrix for the oversubscribed condition,
 *  indexed by original subport and then the higig on which the 'spare' subport
 *  was created.  As for hgSubport, the value is impossible indicates that
 *  there is no 'spare' for this subport on the higig in question.  Yes, this
 *  is expected to be a *very* sparse array.
 *
 *  downTargets is a bitmap of the targets that are down (administratively or
 *  due to port failure or whatever reason).  When a target is set in this
 *  bitmap, it indicates the target is down, and aggregates have been
 *  recalculated to avoid these targets.  Note that intentionally updating an
 *  aggregate that includes such a target will remove this indication, and that
 *  target will participate in aggregates normally afterward, until it is
 *  declared down again.
 *
 *  The lagRedirectOnSirius switch controls whether Sirius will replicate and
 *  squelch unicast sent to an aggregate.  Certain front-panel devices do not
 *  use LBID in a linear fashion as we expect with our code here.  This means
 *  that the Sirius device will drop unicast sent to an aggregate by such a
 *  device (the front-panel device will send unicast to a target that does not
 *  line up with the LBID in the same way that Sirius expects it to line up).
 *  Effectively, this means that Sirius overrides aggregate frame distribution
 *  decisions made by the front-panel device for unicast.  It does not affect
 *  how Sirius handles multicast on aggregates or that all members of an
 *  aggregate must also be members of any multicast group that is to include
 *  such an aggregate.
 *
 *  Since there are so few aggregates, it seems that a linear search through
 *  the available ones is a sufficiently reasonable way to allocate them.
 *  Because of this, there is no more interesting alloc tracking -- a NULL aggr
 *  of fab element is an aggregate that is not currently in use.
 */
typedef struct _sirius_lag_unit_s {
    _lag_target_bitmap targets;                 /* all tgts in all aggrs */
    _lag_target_bitmap downTargets; /* targets declared down for some reason */
    _sirius_aggregate_t *aggr[_SIRIUS_LAG_COUNT]; /* panel aggregates' info */
    _sirius_aggregate_t *fab[_SIRIUS_FAB_LAG_COUNT]; /* fabric aggrs' info */
    int redirectUnicast; /* whether to redirect unicast over aggregates */
    int hgSubport[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];  /* higig MC XGS mode targets */
    int ohgSubport[SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS][SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
} _sirius_lag_unit_t;

/*
 *  Globals used by this module
 */
static volatile sal_mutex_t _lag_lock = NULL;
static _sirius_lag_unit_t *_lag_unit[SOC_MAX_NUM_DEVICES];

/*
 *  Some basic macros...
 */
#define LAG_UNIT_CHECK(__unit) \
    if (SOC_MAX_NUM_DEVICES <= (__unit)) { \
        LOG_ERROR(BSL_LS_BCM_TRUNK, \
                  (BSL_META_U(__unit, \
                              "invalid unit ID %d\n"), __unit));        \
        return BCM_E_UNIT; \
    }
#define LAG_INIT_CHECK(__unit, __dataPtr) \
    (__dataPtr) = _lag_unit[unit]; \
    if ((!_lag_lock) || \
        (!(__dataPtr)) || \
        (!SOC_CONTROL(unit)) || \
        (!SOC_SBX_CONTROL(unit)) || \
        (!SOC_SBX_CFG(unit)) || \
        (!SOC_SBX_CFG_SIRIUS(unit)) || \
        (!(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock))) { \
        LOG_ERROR(BSL_LS_BCM_TRUNK, \
                  (BSL_META_U(__unit, \
                             "unit %d is not initialised\n"),   \
                   __unit)); \
        return BCM_E_INIT; \
    }
#define LAG_LOCK_TAKE(__unit, __dataPtr) \
    if (sal_mutex_take(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock, \
                       sal_mutex_FOREVER)) { \
        LOG_ERROR(BSL_LS_BCM_TRUNK, \
                  (BSL_META_U(__unit, \
                              "unable to take unit %d lock\n"), \
                   (__unit))); \
        return BCM_E_INTERNAL; \
    }
#define LAG_LOCK_GIVE(__unit, __dataPtr) \
    if (sal_mutex_give(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock)) { \
        LOG_ERROR(BSL_LS_BCM_TRUNK, \
                  (BSL_META_U(__unit, \
                              "unable to release unit %d lock\n"),      \
                   (__unit))); \
        return BCM_E_INTERNAL; \
    }

/*
 *  Function
 *    _bcm_sirius_aggregate_higig_uc_prepare
 *  Purpose
 *    Sets up target and supporting infrastructure for sending unicast traffic
 *    via a set of oversubscribed higig ports in XGS mode.
 *  Arguments
 *    (in) int unit = the unit on which to operate
 *    (in) bcm_trunk_add_info_t *aggrData = pointer to aggregate data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Aggregate members *must* be MODPORT form destined to local module for
 *    there to be any effect here, any members that do not meet this
 *    requirement will not have additional subports prepared.
 *
 *    Everywhere this refers to 'target' it now means 'destination'.  Targets
 *    no longer map directly to subports and this interfaces with other
 *    modules, so subports must be used.  The translation from subport to
 *    target will be done inline.
 */
static int
_bcm_sirius_aggregate_higig_uc_prepare(const int unit,
                                       const int member_count,
                                       const bcm_trunk_member_t *member_array)
{
    int result = BCM_E_NONE;
    int subport = -1;
    unsigned int index0;
    unsigned int index1;
    unsigned int spTarget;
    bcm_port_t hgp0;
    bcm_module_t hgm0;
    bcm_port_t hgp1;
    bcm_module_t hgm1;
    bcm_gport_t sp;
    bcm_module_t myModId;

    if (!(_lag_unit[unit])) {
        /* the unit has not been initialised */
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to prepare unit %d higig aggregate for XGS"
                               " oversubscribed mode: trunk APIs not initialised\n"),
                   unit));
        return BCM_E_INIT;
    } /* if (!(_lag_unit[unit])) */
    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        /* can't get modid; give up */
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /*
     *  If an aggregate member is local higig, create a subport on each of the
     *  other aggregate members that are local higigs for each of this
     *  aggregate member's subports.
     */
    for (index0 = 0;
         (index0 < member_count) && (BCM_E_NONE == result);
         index0++) {
        /* check this aggregate member */
        if (BCM_GPORT_IS_MODPORT(member_array[index0].gport)) {
            hgp0 = BCM_GPORT_MODPORT_PORT_GET(member_array[index0].gport);
            hgm0 = BCM_GPORT_MODPORT_MODID_GET(member_array[index0].gport);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(member_array[index0].gport)) {
            hgp0 = BCM_GPORT_EGRESS_MODPORT_PORT_GET(member_array[index0].gport);
            hgm0 = BCM_GPORT_MODPORT_MODID_GET(member_array[index0].gport);
        } else {
            hgp0 = -1;
            hgm0 = -1;
        }
        if ((0 <= hgp0) &&
            (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS > hgp0) &&
            (hgm0 == myModId) &&
            (_lag_unit[unit]->hgSubport[hgp0] < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
            /* this aggregate member is local higig */
            sp = BCM_GPORT_INVALID;
            do { /* while (no errors and not yet run out of subports) */
                result = bcm_sirius_port_subport_getnext(unit, hgp0, SBX_SUBPORT_FLAG_INTERNAL, &sp);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to get unit %d module %d higig"
                                           " %d (%08X) next subport after %08X:"
                                           " %d (%s)\n"),
                               unit,
                               myModId,
                               hgp0,
                               member_array[index0].gport,
                               sp,
                               result,
                               _SHR_ERRMSG(result)));
                    break;
                }
                if (BCM_GPORT_INVALID == sp) {
                    /* we're done scanning & replicating this port */
                    break;
                }
                if (BCM_GPORT_IS_CHILD(sp)) {
                    spTarget = BCM_GPORT_CHILD_PORT_GET(sp);
                } else if (BCM_GPORT_IS_EGRESS_CHILD(sp)) {
                    spTarget = BCM_GPORT_EGRESS_CHILD_PORT_GET(sp);
                } else if (BCM_GPORT_IS_EGRESS_GROUP(sp)) {
                    result = bcm_sbx_cosq_egress_group_info_get(unit,
                                                                sp,
                                                                &subport,
                                                                NULL,
                                                                NULL);
                    if (BCM_E_NONE != result) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to get unit %d egress group"
                                               " 0x%x subport %d (%s)\n"),
                                   unit,
                                   sp,
                                   result,
                                   _SHR_ERRMSG(result)));
                        break;
                    }
                    spTarget = subport;
                } else {
                    /* should never get here, but forget this one if we do */
                    continue;
                }
                /* just copy the target ID to its home higig */
                _lag_unit[unit]->ohgSubport[spTarget][hgp0] = spTarget;
                for (index1 = 0;
                     (index1 < member_count) && (BCM_E_NONE == result);
                     index1++) {
                    if (index0 != index1) {
                        /* different aggregate member; check it */
                        if (BCM_GPORT_IS_MODPORT(member_array[index1].gport)) {
                            hgp1 = BCM_GPORT_MODPORT_PORT_GET(member_array[index1].gport);
                            hgm1 = BCM_GPORT_MODPORT_MODID_GET(member_array[index1].gport);
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(member_array[index1].gport)) {
                            hgp1 = BCM_GPORT_EGRESS_MODPORT_PORT_GET(member_array[index1].gport);
                            hgm1 = BCM_GPORT_MODPORT_MODID_GET(member_array[index1].gport);
                        } else {
                            hgp1 = -1;
                            hgm1 = -1;
                        }
                        if ((0 > hgp1) ||
                            (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS <= hgp1) ||
                            (hgm1 != myModId) ||
                            (_lag_unit[unit]->hgSubport[hgp1] >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
                            /* not local, not modport, &c; don't bother */
                            continue;
                        }
                        if (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS <= _lag_unit[unit]->ohgSubport[spTarget][index1]) {
                            /* create an internal path to this aggr member */
                            result = bcm_sirius_cosq_create_egress_internal_port(unit,
                                                                                 member_array[index1].gport,
                                                                                 BCM_SIRIUS_COSQ_INTERNAL_PORT_UNICAST_OVERSUB,
                                                                                 0, /* num_fifos */
                                                                                 sp,
                                                                                 &(_lag_unit[unit]->ohgSubport[spTarget][hgp1]));
                            if (BCM_E_NONE == result) {
                                LAG_EVERB((BSL_META("create unit %d target %d"
                                                    " as higig %d target for"
                                                    " oversubscribed higig %d"
                                                    " target %d (%08X)\n"),
                                           unit,
                                           _lag_unit[unit]->ohgSubport[spTarget][hgp1],
                                           hgp1,
                                           hgp0,
                                           spTarget,
                                           sp));
                            } else {
                                LOG_ERROR(BSL_LS_BCM_TRUNK,
                                          (BSL_META_U(unit,
                                                      "unable to create target on"
                                                       " unit %d higig %d for"
                                                       " oversubscribed higig %d"
                                                       " target %d (%08X):"
                                                       " %d (%s)\n"),
                                           unit,
                                           hgp1,
                                           hgp0,
                                           spTarget,
                                           sp,
                                           result,
                                           _SHR_ERRMSG(result)));
                            }
                        }
                    } /* if (index0 != index1) */
                } /* for (all aggregate members as long as no errors) */
            } while ((BCM_E_NONE == result) && (BCM_GPORT_INVALID != sp));
        } /* if (aggregate member is local higig) */
    } /* for (all aggregate members as long as no errors) */
    return result;
}

/*
 *  Function
 *    bcm_sirius_aggregate_gport_translate
 *  Purpose
 *    Given a module,gport pair, translate it into a target ID if it is local,
 *    or a bogus value (>= _SIRIUS_MC_MAX_TARGETS) otherwise.
 *  Arguments
 *    (in) int unit = the unit on which the operation occurs
 *    (in) uint32 flags = operational flags
 *    (in) bcm_module_t myModId = the intended local module
 *    (in) bcm_module_t module = the module for the port
 *    (in) bcm_port_t port = the gport on the given module
 *    (out) unsigned int *target = where to put the target
 *    (out) unsigned int *count = where to put target count
 *    (out) int *isHigig = indicates target is higig MC target (not child)
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 *
 *    Does not validate ports on remote modules (actually, doesn't do anything
 *    with them -- it sets the target to a bogus value to indicate that it is a
 *    remote target if it is not local).
 *
 *    It is possible that, in certain models, a single destination will spam
 *    multiple targets.  In this case, count will be >1, indicating that the
 *    code may need to handle additional targets.  Right now, all targets for a
 *    destination will be contiguous (and the 'target' parameter is filled by
 *    the *lowest* numbered target).
 */
int
bcm_sirius_aggregate_gport_translate(const int unit,
                                     const uint32 flags,
                                     const bcm_module_t myModId,
                                     const bcm_module_t module,
                                     bcm_port_t port,
                                     unsigned int *target,
                                     unsigned int *count,
                                     int *isHigig)
{
    int result = BCM_E_NONE;
    unsigned int spid = ~0;     /* subport ID */
    bcm_port_t tgtPort;         /* target port from translation/extracion */
    bcm_module_t tgtModule;     /* target module from translation/extracion */
    bcm_gport_t lgport;
    int ihg = FALSE;            /* TRUE if directly involves a higig port */

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,%08X,%d,%d,%08X,*,*) enter\n"),
               unit,
               flags,
               myModId,
               module,
               port));

    /*
     *  If we were given a 'local' port (non-GPORT) plus module, or a
     *  LOCAL_GPORT, make it into a MODPORT so we don't need cases for local
     *  ports (or even remote 'local' ports) later on.
     */
    if (0 == ((port >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK)) {
        BCM_GPORT_MODPORT_SET(lgport, module, port);
        LAG_EVERB((BSL_META("converted module=%d port=%d into GPORT %08X\n"),
                   module,
                   port,
                   lgport));
        port = lgport;
    } else if (BCM_GPORT_IS_LOCAL(port)) {
        BCM_GPORT_MODPORT_SET(lgport, myModId, BCM_GPORT_LOCAL_GET(port));
        LAG_EVERB((BSL_META("converted GPORT %08X to GPORT %08X\n"),
                   port,
                   lgport));
        port = lgport;
    }

    /*
     *  If we have a 'modport', then it could be a number of things; if it is
     *  not local, we need to try to map it to something local.  If we can't
     *  map it, then just take it as a placeholder locally.
     */
    if (BCM_GPORT_IS_MODPORT(port) || BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* get the target module for this modport */
        if (BCM_GPORT_IS_MODPORT(port)) {
            tgtModule = BCM_GPORT_MODPORT_MODID_GET(port);
        } else {
            tgtModule = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
        }
        if (tgtModule != myModId) {
            if (flags & BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP) {
                /*
                 *  It's not local, but caller does not want modportmap used in
                 *  an effort to translate it to local.  Possible that this has
                 *  to do with state maintenance or similar, but in any case,
                 *  since it looks remote at this point, just return the values
                 *  that indicate it is remote.
                 */
                *target = _SIRIUS_MC_MAX_TARGETS;
                if (count) {
                    *count = 0;
                }
                if (isHigig) {
                    *isHigig = FALSE;
                }
                return BCM_E_NONE;
            } else { /* if (BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP) */
                /* since it's not 'local', we shall assume it is 'front panel' */
                /* need to translate it to our local CHILD GPORT */
                result = bcm_stk_fabric_map_get(unit, port, &lgport);
                if (BCM_E_NONE == result) {
                    LAG_EVERB((BSL_META("translated unit %d port %08X -> %08X\n"),
                               unit,
                               port,
                               lgport));
                    port = lgport;
                } else { /* if (BCM_E_NONE == result) */
                    /* it did not translate; assume it's somewhere else */
                    LAG_EVERB((BSL_META("unit %d port %08X translation error;"
                                        " taken as remote: %d (%s)\n"),
                               unit,
                               port,
                               result,
                               _SHR_ERRMSG(result)));
                    *target = _SIRIUS_MC_MAX_TARGETS;
                    if (count) {
                        *count = 0;
                    }
                    if (isHigig) {
                        *isHigig = FALSE;
                    }
                    return BCM_E_NONE;
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP) */
        } /* if (tgtModule != myModId) */
    } /* if (port is MODPORT or EGRESS_MODPORT) */

    /*
     *  If we made it to here and it's still a MODPORT, check it.  If it's
     *  non-local, then map it as such.  If it's local, that means it must be a
     *  higig port, and we'll need to look it up.
     */
    if (BCM_GPORT_IS_MODPORT(port) || BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* get the target module and port for this modport */
        if (BCM_GPORT_IS_MODPORT(port)) {
            tgtModule = BCM_GPORT_MODPORT_MODID_GET(port);
            lgport = BCM_GPORT_MODPORT_PORT_GET(port);
        } else {
            tgtModule = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
            lgport = BCM_GPORT_EGRESS_MODPORT_PORT_GET(port);
        }
        if (tgtModule != myModId) {
            /* not on local module, so only placeholder here */
            *target = _SIRIUS_MC_MAX_TARGETS;
            if (count) {
                *count = 0;
            }
            if (isHigig) {
                *isHigig = FALSE;
            }
            return BCM_E_NONE;
        }
        /* It's a local MODPORT, so must be higig */
        if ((0 <= lgport) &&
            (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS > lgport)) {
            /* it's a valid higig port number */
            ihg = TRUE;
            if (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS >
                _lag_unit[unit]->hgSubport[lgport]) {
                spid = _lag_unit[unit]->hgSubport[lgport];
            } else {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d higig %08X(%d) has no multicast"
                                       " subport (was it changed to non-SBX after"
                                       " groups/aggregates were created?)\n"),
                           unit,
                           port,
                           lgport));
                result = BCM_E_NOT_FOUND;
            }
        } else if ((SOC_SBX_QE_MODE_TME_BYPASS == SOC_SBX_CFG(unit)->bTmeMode) &&
                   (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS <= lgport) &&
                   ((SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS +
                     SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS) > lgport)) {
            /*
             *  Inline mode, and it refers to a fabric-side higig port, which
             *  should not be included in multicast or aggregation.
             */
            result = BCM_E_NOT_FOUND;
        } else {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "invalid higig %08X(%d) on unit %d\n"),
                       port,
                       lgport,
                       unit));
            return BCM_E_PARAM;
        }
    } else { /* if (BCM_GPORT_IS_MODPORT(port)) */
        /* it's not a MODPORT; try to map to a target */
        result = bcm_sbx_gport_fifo_get(unit,
                                        port,
                                        &tgtPort,
                                        &tgtModule);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to determine subport from"
                                   " module %d port %08X: %d (%s)\n"),
                       module,
                       port,
                       result,
                       _SHR_ERRMSG(result)));
        } else {
            if (myModId == tgtModule) {
                /* on local module */
                if ((0 <= tgtPort) &&
                    (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS > tgtPort)) {
                    /* target is valid */
                    spid = tgtPort;
                } else {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "invalid subport %d derived from"
                                           " module %d port %08X\n"),
                               tgtPort,
                               module,
                               port));
                    result = BCM_E_PARAM;
                }
            } else { /* if (myModId == tgtModule) */
                /* not on local module, so only placeholder here */
                spid = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
            } /* if (myModId == tgtModule) */
        } /* if (BCM_E_NONE != result) */
    } /* if (BCM_GPORT_IS_MODPORT(port)) */
    if (BCM_E_NONE == result) {
        if ((spid < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) &&
            SOC_SBX_STATE(unit)->port_state->subport_info[spid].valid) {
            /*
             *  Compute target ID from subport ID.
             *
             *  This fetches the FIFO information for the subport and translates
             *  that to the target ID.  This is done using a fixed map (shr 2)
             *  because the hardware uses a fixed map for this translation.
             */
            *target = (SOC_SBX_STATE(unit)->port_state->subport_info[spid].egroup[0].es_scheduler_level0_node) >> 2;
            /*
             *  Also maybe report whether this is an internal port via the
             *  higig reporting mechanism.  Note this overrides the normal
             *  behaviour of the higig reporting mechanism.
             */
            if (flags & BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_CALL_INTERNAL_HIGIG) {
                if (SOC_SBX_STATE(unit)->port_state->subport_info[spid].flags &
                    (SBX_SUBPORT_FLAG_TRUNK_UCAST | SBX_SUBPORT_FLAG_TRUNK_MCAST)) {
                    ihg = TRUE;
                } else {
                    ihg = FALSE;
                }
            }
        } else {
            /* not local, or invalid; no associated targets */
            *target = _SIRIUS_MC_MAX_TARGETS;
        }
    }
    if ((BCM_E_NONE == result) && (NULL != count)) {
        /* All is well and caller also wanted target count */
        if ((spid < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)  &&
            SOC_SBX_STATE(unit)->port_state->subport_info[spid].valid) {
            /*
             *  Compute number of 'touched' targets for this destination.
             *
             *  This gets the number of FIFOs, adds the offset into the FIFO
             *  group, then divides that by four (rounding up).
             */
            *count = (SOC_SBX_STATE(unit)->port_state->subport_info[spid].egroup[0].num_fifos +
                      (SOC_SBX_STATE(unit)->port_state->subport_info[spid].egroup[0].es_scheduler_level0_node & 3) +
                      3) >> 2;
        } else { /* if ((*target) < _SIRIUS_MC_MAX_TARGETS) */
            /* not local, or invalid; no associated targets */
            *count = 0;
        } /* if ((*target) < _SIRIUS_MC_MAX_TARGETS) */
    } /* if ((BCM_E_NONE == result) && (NULL != count)) */
    if ((BCM_E_NONE == result) && (NULL != isHigig)) {
        /* caller wanted higig state, so provide it */
        *isHigig = ihg;
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,%08X,%d,%d,%08X,&(%d),&(%d),%s) return %d (%s)\n"),
               unit,
               flags,
               myModId,
               module,
               port,
               *target,
               (count)?(*count):-1,
               (isHigig)?((*isHigig)?"&(TRUE)":"&(FALSE)"):"NULL",
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_squelch_read
 *  Purpose
 *    Read the squelch table and parse it into something we can use quickly.
 *  Arguments
 *    (in) int unit = the unit whose squelch table is to be read
 *    (out) _lag_target_bitmap *svt = where to put the parsed table
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 */
static int
_bcm_sirius_squelch_read(const int unit,
                         _lag_target_bitmap *svt)
{
    int result;
    unsigned int index;
    unsigned int indexCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    eg_fd_svt_entry_t *buffer;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,*) enter\n"),
               unit));

    /* get a buffer for the squelch vector table */
    buffer = soc_cm_salloc(unit,
                           sizeof(*buffer) * indexCount,
                           "SVT buffer for reading");
    if (buffer) {
        /* invalidate cache for the buffer space */
        result = soc_cm_sinval(unit,
                               buffer,
                               sizeof(*buffer) * indexCount);

        if (BCM_E_NONE == result) {
            /* fetch the whole squelch vector table */
            result = soc_mem_read_range(unit,
                                        EG_FD_SVTm,
                                        MEM_BLOCK_ANY,
                                        SOC_MEM_INFO(unit,
                                                     EG_FD_SVTm).index_min,
                                        SOC_MEM_INFO(unit,
                                                     EG_FD_SVTm).index_max,
                                        buffer);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to read %d:SVT[%08X..%08X]:"
                                       " %d (%s)\n"),
                           unit,
                           SOC_MEM_INFO(unit, EG_FD_SVTm).index_min,
                           SOC_MEM_INFO(unit, EG_FD_SVTm).index_max,
                           result,
                           _SHR_ERRMSG(result)));
            }
        } else { /* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to invalidate %d:SVT buffer: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */

        if (BCM_E_NONE == result) {
            /* parse the table into our local representation */
            for (index = 0; index < indexCount; index++) {
                soc_EG_FD_SVTm_field_get(unit,
                                         &(buffer[index]),
                                         SVT_ENTRYf,
                                         &(svt[index][0]));
            }
        }

        /* get rid of the buffer */
        soc_cm_sfree(unit, buffer);
    } else { /* if (buffer) */
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to allocate %d bytes for %d:SVT buffer\n"),
                   sizeof(*buffer) * indexCount,
                   unit));
        result = BCM_E_MEMORY;
    } /* if (buffer) */

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_squelch_write
 *  Purpose
 *    Write the squelch table after building it from our internal view.
 *  Arguments
 *    (in) int unit = the unit whose squelch table is to be written
 *    (in) _lag_target_bitmap *svt = the internal view of the table
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 *
 *    svt should point to const _lag_target_bitmap, but called function does
 *    not honour this.
 */
static int
_bcm_sirius_squelch_write(const int unit,
                          _lag_target_bitmap *svt)
{
    int result;
    unsigned int index;
    unsigned int indexCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    eg_fd_svt_entry_t *buffer;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,*) enter\n"),
               unit));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (!SOC_IS_RELOADING(unit)) {
        /* not reloading, so we can write to the squelch table */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

        /* get a buffer for the squelch vector table */
        buffer = soc_cm_salloc(unit,
                               sizeof(*buffer) * indexCount,
                               "SVT buffer for reading");
        if (buffer) {
            

            /* fill in the table from our data */
            for (index = 0; index < indexCount; index++) {
                soc_EG_FD_SVTm_field_set(unit,
                                         &(buffer[index]),
                                         SVT_ENTRYf,
                                         &(svt[index][0]));
            }

            /* commit anything left in the cache */
            result = soc_cm_sflush(unit,
                                   buffer,
                                   sizeof(*buffer) * indexCount);

            if (BCM_E_NONE == result) {
                /* write the whole squelch vector table */
                /*    coverity[negative_returns : FALSE]    */
                result = soc_mem_write_range(unit,
                                             EG_FD_SVTm,
                                             MEM_BLOCK_ALL,
                                             SOC_MEM_INFO(unit, EG_FD_SVTm).index_min,
                                             SOC_MEM_INFO(unit, EG_FD_SVTm).index_max,
                                             buffer);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to write %d:SVT[%08X..%08X]:"
                                           " %d (%s)\n"),
                               unit,
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_min,
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_max,
                               result,
                               _SHR_ERRMSG(result)));
                }
            } else { /* if (BCM_E_NONE == result) */
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to flush %d:SVT buffer: %d (%s)\n"),
                           unit,
                           result,
                           _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */

            /* get rid of the buffer */
            soc_cm_sfree(unit, buffer);
        } else { /* if (buffer) */
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes for %d:SVT buffer\n"),
                       sizeof(*buffer) * indexCount,
                       unit));
            result = BCM_E_MEMORY;
        } /* if (buffer) */

#if defined(BCM_EASY_RELOAD_SUPPORT)
    } else { /* if (!SOC_IS_RELOADING(unit)) */
        /* hopefully, we can always successfully do nothing... */
        result = BCM_E_NONE;
    } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_squelch_targets_always
 *  Purpose
 *    Given a set of targets, adjust the provided squelch table so as to
 *    squelch the targets on all LBIDs.
 *  Arguments
 *    (in) int unit = the unit to which the operation will apply
 *    (in) _lag_target_bitmap *svt = pointer to squelch vector table data
 *    (in) _lag_target_bitmap targets = which targets to always squelch
 *  Return
 *    none
 *  Notes
 *    Assumes arguments are correct and valid.
 */
static void
_bcm_sirius_squelch_targets_always(const int unit,
                                   _lag_target_bitmap *svt,
                                   const _lag_target_bitmap targets)
{
    _lag_target_bitmap mask;
    unsigned int index;
    unsigned int offset;
    unsigned int indexCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);

    /* set up an AND mask for the targets in question */
    for (offset = 0; offset < _LAG_TARGET_SIZE; offset++) {
        mask[offset] = ~(targets[offset]);
    }

    /* AND the targets out of the squelch vectors */
    for (index = 0; index < indexCount; index++) {
        for (offset = 0; offset < _LAG_TARGET_SIZE; offset++) {
            svt[index][offset] &= mask[offset];
        }
    }
}

/*
 *  Function
 *    _bcm_sirius_squelch_targets_never
 *  Purpose
 *    Given a set of targets, adjust the provided squelch table so as to remove
 *    the targets from any aggregates (no longer squelch them by any LBID).
 *  Arguments
 *    (in) int unit = the unit to which the operation will apply
 *    (in) _lag_target_bitmap *svt = pointer to squelch vector table data
 *    (in) _lag_target_bitmap targets = which targets to never squelch
 *  Return
 *    none
 *  Notes
 *    Assumes arguments are correct and valid.
 */
static void
_bcm_sirius_squelch_targets_never(const int unit,
                                  _lag_target_bitmap *svt,
                                  const _lag_target_bitmap targets)
{
    unsigned int index;
    unsigned int offset;
    unsigned int indexCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                               SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);

    /* OR the targets into the squelch vectors */
    for (index = 0; index < indexCount; index++) {
        for (offset = 0; offset < _LAG_TARGET_SIZE; offset++) {
            svt[index][offset] |= targets[offset];
        }
    }
}

/*
 *  Function
 *    _bcm_sirius_squelch_targets_round_robin
 *  Purpose
 *    Given a set of targets, adjust the provided squelch table so as to
 *    distribute the targets as evenly as possible based upon LBID.
 *  Arguments
 *    (in) int unit = the unit to which the operation will apply
 *    (in) _aggr_target_bitmap *svt = pointer to squelch vector table data
 *    (in) unsigned int targetCount = number of members
 *    (in) _modPort_t *members = pointer to aggregate membership list
 *  Return
 *    none
 *  Notes
 *    Assumes arguments are correct and valid.
 *    Will not include targets that are in the 'down' state.
 */
static void
_bcm_sirius_squelch_targets_round_robin(const int unit,
                                        _lag_target_bitmap *svt,
                                        const unsigned int targetCount,
                                        const _modPort_t *members)
{
    _lag_target_bitmap mask;
    unsigned int target[BCM_TRUNK_MAX_PORTCNT];
    unsigned int count[BCM_TRUNK_MAX_PORTCNT];
    unsigned int tgtOFfset;
    unsigned int index;
    unsigned int offset;
    unsigned int tgtIndex;
    unsigned int upMembers;
    unsigned int indexCount;

    /*
     *  Build an AND mask for the specified GPORTs.  Also build a working set
     *  of targets that are to be used when populating this aggregate in the
     *  squelch table.
     *
     *  Note that any targets that are marked 'down' are skipped in the working
     *  set, but are included in the mask, so they will be squelched but not
     *  added back to the distribution.  This prevents frames being directed to
     *  a targets that is marked 'down'.  However, this means that it is quite
     *  possible for an entire aggregate to be squelched locally if all of the
     *  included targets are 'down'.  This local complete squelch is NOT
     *  considered an error here, whether the aggregate contains remote GPORTs
     *  or not.  The application software must somehow become aware of the
     *  'down' condition and configure the affected aggrogates appropriately.
     */
    sal_memset(&mask, 0xFF, sizeof(mask));
    for (index = 0, upMembers=0; index < targetCount; index++) {
        target[upMembers] = members[index].target;
        count[upMembers] = members[index].count;
        if (_SIRIUS_MC_MAX_TARGETS > target[upMembers]) {
            /* provided member is local; mask it out */
            indexCount = 0;
            for (offset = 0; offset < count[upMembers]; offset++) {
                tgtOFfset = target[upMembers] + offset;
                mask[tgtOFfset >> 5] &= ~(1 << (tgtOFfset & 0x1F));
                if (0 == (_lag_unit[unit]->downTargets[tgtOFfset >> 5] &
                          (1 << (tgtOFfset & 0x1F)))) {
                /* this target is not 'down'; consider it 'up' */
                    indexCount++;
                }
            }
            if (0 < indexCount) {
                /* there are targets not down for this member */
                upMembers++;
            }
        } else { /* if (_SIRIUS_MC_MAX_TARGETS > target[upTargets]) */
            /* provided target is remote; assume remote targets are up */
            upMembers++;
        } /* if (_SIRIUS_MC_MAX_TARGETS > target[upTargets]) */
    } /* for (all provided modules+ports/targets) */

    /* adjust the squelch vector table */
    indexCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                  SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    for (index = 0, tgtIndex = 0; index < indexCount; index++) {
        /* remove all of the local targets */
        for (offset = 0; offset < _LAG_TARGET_SIZE; offset++) {
            svt[index][offset] &= mask[offset];
        }
        if (upMembers) {
            /*
             *  We have targets, so put this one back, if it's local.  If this
             *  one is not local, just skip this one.
             *
             *  If there are no remote targets, and all local targets are
             *  'down', there is no point in doing this.
             */
            if (_SIRIUS_MC_MAX_TARGETS > target[tgtIndex]) {
                for (offset = 0; offset < count[tgtIndex]; offset++) {
                    tgtOFfset = target[tgtIndex] + offset;
                    if (0 == (_lag_unit[unit]->downTargets[tgtOFfset >> 5] &
                              (1 << (tgtOFfset & 0x1F)))) {
                        svt[index][tgtOFfset >> 5] |= (1 << (tgtOFfset & 0x1F));
                    }
                }
            }
            tgtIndex++;
            if (tgtIndex >= upMembers) {
                tgtIndex = 0;
            }
        } /* if (upTargets) */
    } /* for (all squelch vectors) */
}

/*
 *  Function
 *    _bcm_sirius_aggregate_initial_squelch_state
 *  Purpose
 *    Set initial state of squelch table for init or detach.
 *  Arguments
 *    (in) int unit = the unit to configure for initial state
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 */
static int
_bcm_sirius_aggregate_initial_squelch_state(const int unit)
{
    _lag_target_bitmap targets;
    _lag_target_bitmap *squelchTable = NULL;
    unsigned int index;
    unsigned int count;
    int result = BCM_E_NONE;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d) enter\n"),
               unit));

    /* Compute active targets bitmap */
    
    index = 0;
    
    /* coverity[dead_error_condition : FALSE] */
    for (count = _SIRIUS_MC_MAX_TARGETS;
         count >= 32;
         count -= 32) {
        targets[index] = 0xFFFFFFFF;
        index++;
    }
    if (count > 0) {
        /* coverity[overrun-local : FALSE] */
        targets[index] = 0;
        while (count > 0) {
            targets[index] = (targets[index] << 1) | 1;
            count--;
        }
    }

    /* allocate squelch table workspace */
    count = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
             SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    squelchTable = sal_alloc(sizeof(*squelchTable) * count,
                             "squelch table workspace");
    if (!squelchTable) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to allocate %d bytes"
                               " squelch table workspace\n"),
                   sizeof(*squelchTable) * count));
        result = BCM_E_MEMORY;
    }

    /* read squelch table */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_squelch_read(unit, squelchTable);
        /* called function displayed error */
    }

    /* update and write squelch table */
    if (BCM_E_NONE == result) {
        /* don't squelch any targets that should be enabled */
        _bcm_sirius_squelch_targets_never(unit, squelchTable, targets);
        /* squelch all targets that should not be enabled */
        for (index = 0; index < _LAG_TARGET_SIZE; index++) {
            targets[index] = ~(targets[index]);
        }
        _bcm_sirius_squelch_targets_always(unit, squelchTable, targets);
        /* write squelch table */
        result = _bcm_sirius_squelch_write(unit, squelchTable);
    } /* if (BCM_E_NONE == result) */

    /* dispose of workspace */
    if (squelchTable) {
        sal_free(squelchTable);
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_aggregate_data_create
 *  Purpose
 *    Create the aggregate management data for the specified unit, and prepare
 *    the hardware appropriately.
 *  Arguments
 *    (in) int unit = the unit to prepare
 *    (in) _sirius_aggregates_t **unitPtr = where to put unit data pointer
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 *
 *    This could be optimised (it zeroes the whole struct then writes nonzero
 *    data to a lot of it).
 */
static int
_bcm_sirius_aggregate_data_create(const int unit,
                                  _sirius_lag_unit_t **unitPtr)
{
    volatile sal_mutex_t *lockPtr;
    sal_mutex_t lock = NULL;
    _sirius_lag_unit_t *unitData;
    unsigned int index0;
    unsigned int index1;
    bcm_module_t myModule = -1;
    int result = BCM_E_NONE;
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,*) enter\n"),
               unit));

    if ((!SOC_CONTROL(unit)) ||
        (!SOC_SBX_CONTROL(unit)) ||
        (!SOC_SBX_CFG(unit)) ||
        (!SOC_SBX_CFG_SIRIUS(unit))) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to access unit %d config struct\n"),
                   unit));
        return BCM_E_INIT;
    }
    result = bcm_stk_my_modid_get(unit, &myModule);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d modid: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    lockPtr = &(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock);

    if (!(*lockPtr)) {
        /* need to create shared lock */
        
        *lockPtr = lock = sal_mutex_create("unit aggr/mc lock");
        if (!lock) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to create unit %d lock\n"),
                       unit));
            return BCM_E_RESOURCE;
        }
    }

    /* allocate and initialise unit descriptor */
    unitData = sal_alloc(sizeof(*unitData), "unit aggregate information");
    if (!unitData) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to allocate %d byte for unit data\n"),
                   sizeof(*unitData)));
        result = BCM_E_MEMORY;
    } else { /* if (!unitData) */
        
        sal_memset(unitData, 0x00, sizeof(*unitData));
        /* mark higigs as not yet mapped */
        for (index0 = 0;
             index0 < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
             index0++) {
            unitData->hgSubport[index0] = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
        }
        /* mark 'spare' oversub paths as not yet mapped */
        for (index1 = 0; index1 < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; index1++) {
            for (index0 = 0;
                 index0 < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
                 index0++) {
                unitData->ohgSubport[index1][index0] = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
            }
        }
        /* cache locally the unicast redirection mode */
        if (SOC_SBX_CFG_SIRIUS(unit)->redirectUcLag) {
            unitData->redirectUnicast = TRUE;
        } else {
            unitData->redirectUnicast = FALSE;
        }
    } /* if (!unitData) */

    /* pass out new unit information or clean up */
    if (BCM_E_NONE == result) {
        *unitPtr = unitData;
    } /* if (BCM_E_NONE == result) */
    /* try to avoid 'obvious' race condition */
    if (lock && (*lockPtr != lock)) {
        LOG_WARN(BSL_LS_BCM_TRUNK,
                 (BSL_META_U(unit,
                             "detected race condition setting up aggregate"
                              " data for unit %d; trying to compensate\n"),
                  unit));
        sal_mutex_destroy(lock);
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,&(%08X)) return %d (%s)\n"),
               unit,
               (uint32)(*unitPtr),
               result,
               _SHR_ERRMSG(result)));
    return result;
}

int
_bcm_sirius_aggregate_data_map_internal_port(const int unit, int port, int si_index)
{
    int result = BCM_E_NONE;
    _sirius_lag_unit_t *unitData;
    bcm_module_t myModule = -1;
    bcm_gport_t xgport = -1;
    bcm_gport_t xegport = -1;
    bcm_gport_t fgport = -1;
    bcm_gport_t efgport = -1;
    unsigned int index0;
    unsigned int index1;
    unsigned int hgSubport[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_gport_t sched[3][SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    int i=0;
    int num_fifos = 0;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,%d) enter\n"),
                 unit, port, si_index));

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    result = bcm_stk_my_modid_get(unit, &myModule);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d modid: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    unitData->hgSubport[port] = si_index;
    num_fifos = SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port].egroup[0].num_fifos;

    /* we are running XGS mode, so create higig MC targets */
    for (i = 0;
         ((i < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) &&
          (BCM_E_NONE == result));
         i++) {

        BCM_GPORT_MODPORT_SET(xgport, myModule, i);
        BCM_GPORT_EGRESS_MODPORT_SET(xegport, myModule, i);

        if (i != port) {
            LAG_EVERB((BSL_META("create egress scheduler hierarchy for"
                                " unit %d higig %d (%08X)\n"),
                       unit,
                       i,
                       xegport));

            result = bcm_cosq_gport_add(unit,
                                        xegport,
                                        0,
                                        BCM_COSQ_GPORT_SCHEDULER,
                                        &(sched[0][i]));
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to create egress scheduler node 0 for"
                                       " unit %d higig %d (%08X): %d (%s)\n"),
                           unit,
                           i,
                           xegport,
                           result,
                           _SHR_ERRMSG(result)));
            } else {
                LAG_EVERB((BSL_META("created unit %d higig %d (%08X) egress"
                                    " scheduler 0 as %08X\n"),
                           unit,
                           i,
                           xegport,
                           sched[0][i]));
            }
            if (BCM_E_NONE == result) {
                result = bcm_cosq_gport_attach(unit, xegport, sched[0][i], -1);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to attach egress scheduler node 0"
                                           " %08X to unit %d higig %d (%08X):"
                                           " %d (%s)\n"),
                               sched[0][i],
                               unit,
                               i,
                               xegport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    LAG_EVERB((BSL_META("attached scheduler %08X to unit %d"
                                        " higig %d (%08X)\n"),
                               sched[0][i],
                               unit,
                               i,
                               xegport));
                }
            }
            if (BCM_E_NONE == result) {
                result = bcm_cosq_gport_add(unit,
                                            xegport,
                                            1,
                                            BCM_COSQ_GPORT_SCHEDULER,
                                            &(sched[1][i]));
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to create egress scheduler node 1 for"
                                           " unit %d higig %d (%08X): %d (%s)\n"),
                               unit,
                               i,
                               xegport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    LAG_EVERB((BSL_META("created unit %d higig %d (%08X) egress"
                                        " scheduler 1 as %08X\n"),
                               unit,
                               i,
                               xegport,
                               sched[1][i]));
                }
            }
            if (BCM_E_NONE == result) {
                result = bcm_cosq_gport_attach(unit, sched[0][i], sched[1][i], -1);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to attach egress scheduler node 1"
                                           " %08X to scheduler node 0 %08X to"
                                           " unit %d higig %d (%08X): %d (%s)\n"),
                               sched[1][i],
                               sched[0][i],
                               unit,
                               i,
                               xegport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    LAG_EVERB((BSL_META("attached scheduler node 1 %08X to"
                                        " scheduler node 0 %08X"
                                        " to unit %d higig %d (%08X)\n"),
                               sched[1][i],
                               sched[0][i],
                               unit,
                               i,
                               xegport));
                }
            }
            if (BCM_E_NONE == result) {
                result = bcm_fabric_port_create(unit,
                                                xgport,
                                                -1,
                                                BCM_FABRIC_PORT_EGRESS_MULTICAST,
                                                &fgport);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to create fabric port for unit %d"
                                           " higig %d (%08X): %d (%s)\n"),
                               unit,
                               i,
                               xgport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    BCM_GPORT_EGRESS_CHILD_SET(efgport,
                                               BCM_GPORT_CHILD_MODID_GET(fgport),
                                               BCM_GPORT_CHILD_PORT_GET(fgport));
                    LAG_EVERB((BSL_META("created fabric port %08X (%08X) for"
                                        " unit %d higig %d (%08X)\n"),
                               fgport,
                               efgport,
                               unit,
                               i,
                               xgport));
                }
            }
            if (BCM_E_NONE == result) {
                result = bcm_cosq_gport_add(unit,
                                            efgport,
                                            num_fifos,
                                            BCM_COSQ_GPORT_EGRESS_GROUP,
                                            &(sched[2][i]));
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to create egress group for unit %d"
                                           " higig %d (%08X) egress fabric port %08X:"
                                           " %d (%s)\n"),
                               unit,
                               i,
                               xgport,
                               efgport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    LAG_EVERB((BSL_META("created egress group %08X for unit %d"
                                        " higig %d (%08X) egress fabric port"
                                        " %08X\n"),
                               sched[2][i],
                               unit,
                               i,
                               xgport,
                               efgport));
                }
            }
            if (BCM_E_NONE == result) {
                result = bcm_cosq_gport_attach(unit, sched[1][i], sched[2][i], -1);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to attach egress group %08X"
                                           " to egress scheduler node 1 %08X"
                                           " to scheduler node 0 %08X to"
                                           " unit %d higig %d (%08X): %d (%s)\n"),
                               sched[2][i],
                               sched[1][i],
                               sched[0][i],
                               unit,
                               i,
                               xegport,
                               result,
                               _SHR_ERRMSG(result)));
                } else {
                    LAG_EVERB((BSL_META("attached egress group %08X to"
                                        " scheduler node 1 %08X to"
                                        " scheduler node 0 %08X"
                                        " to unit %d higig %d (%08X)\n"),
                               sched[2][i],
                               sched[1][i],
                               sched[0][i],
                               unit,
                               i,
                               xegport));
                }
            }
            if (BCM_E_NONE == result) {
                unitData->hgSubport[i] = BCM_GPORT_CHILD_PORT_GET(fgport);
                LAG_EVERB((BSL_META("associate internal fabric port %08X (%d)"
                                    " with unit %d higig %d (%08X,%08X)\n"),
                           fgport,
                           unitData->hgSubport[i],
                           unit,
                           i,
                           xgport,
                           xegport));
            }
        }
    } /* for (all front-facing-higig ports as long as no errors) */

    /* this is required; it adjusts the scheduler configuration */
    
    if (BCM_E_NONE == result) {
        LAG_EVERB((BSL_META("update configuration for unit %d ES root"
                            " scheduler\n"),
                   unit));
        result = soc_sirius_es_root_scheduler_config(unit);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to set up root egress scheduler config"
                                   " for unit %d: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }

    for (i=0;
         (BCM_E_NONE == result) && (i < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS);
         i++) {
        /*
         *  Until all hgTargets configured, initialization process incomplete.
         *  Should not reach this point in that state, but just in case...
         */
        if (unitData->hgSubport[i] == SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) {
            result = BCM_E_INTERNAL;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unexpected/invalid condition: not all four of"
                                   " the higig 'raw' paths exist on unit %d.\n"),
                       unit));
        }
        /* fetch actual hardware subport ID for use later */
        hgSubport[i] = SOC_SBX_STATE(unit)->port_state->subport_info[unitData->hgSubport[i]].es_scheduler_level1_node[0];
    }

    /*
     *  Now, the predicate that is used for determining that frames are
     *  going down the higigs on this newly-created 'raw' path requires
     *  that the created targets be contiguous -- not necessarily
     *  monotonically increasing, but absolutely necessarily contiguous.
     *  Frankly, monotonically increasing is far easier!
     *
     *  We're going to take a shortcut, and assume that we would not get
     *  more than one of the same ID back, so all we have to do is acertain
     *  the range and make sure it's four across.  If we stumble across
     *  duplicates, that will be an error, but we don't really check for
     *  duplicates very thoroughly.
     */
    if (BCM_E_NONE == result) {
        for (i = 1, index0 = index1 = hgSubport[0];
             i < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
             i++) {
            if (index0 > hgSubport[i]) {
                index0 = hgSubport[i];
            } else if (index0 == hgSubport[i]) {
                result = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d higig subport %d (%d) overlaps an"
                                       " existing higig subport\n"),
                           unit,
                           i,
                           hgSubport[i]));
            }
            if (index1 < hgSubport[i]) {
                index1 = hgSubport[i];
            } else if (index1 == hgSubport[i]) {
                result = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d higig subport %d (%d) overlaps an"
                                       " existing higig subport\n"),
                           unit,
                           i,
                           hgSubport[i]));
            }
        }
        if ((index0 + SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) !=
            (index1 + 1)) {
            result = BCM_E_CONFIG;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unit %d egress internal multicast targets are"
                                   " not contiguous.\n"),
                       unit));
        }
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to prepare unit %d contiguous range of"
                                   " egress internal multicast targets: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        } else { /* if (BCM_E_NONE != result) */
            /* the higig multicast targets are contiguous */
            LAG_EVERB((BSL_META("set unit %d remote replication predicate to"
                                " subport range %d..%d\n"),
                       unit,
                       index0,
                       index1));
            result = soc_sirius_hw_xgs_remote_replication_range_set(unit,
                                                                    index0,
                                                                    index1);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to configure remote replication"
                                       " subport range %d..%d on unit %d:"
                                       " %d (%s)\n"),
                           index0,
                           index1,
                           unit,
                           result,
                           _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE != result) */
    } /* if (BCM_E_NONE == result) */

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,%d) return %d (%s)\n"),
                 unit,
                 port,
                 si_index,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_trunk_get
 *  Purpose
 *    Get the description of an aggregate.
 *  Arguments
 *    (in) _sirius_lag_unit_t *unitData = local unit data
 *    (in) unsigned int lagIndex = the local aggregate 'index'
 *    (in) int fabric = TRUE if fabric, FALSE if front panel
 *    (out) bcm_trunk_info_t *t_data = where to put aggregate information
 *    (in) int member_max = max number of members to retrieve
 *    (out) bcm_trunk_member_t *member_array = pointer to member buffer
 *    (out) int *member_count = where to put retrieved member count
 *  Return
 *    nothing
 *  Notes
 *    Some fields are not supported on 'set'; they're always zeroed on 'get'.
 *
 *    Some data filled in are literally the data used in the 'set', cached
 *    specifically for the purpose of implementing 'get' (they're not used at
 *    all internally).
 *
 *    If member_max is zero, will fill in only struct at t_data and the value
 *    at member_count.  The member_array will not be filled (and can be NULL).
 *
 *    If member_max is nonzero, will fill in up to member_max members.  If
 *    there are more than member_max members, will still only fill in up to
 *    member_max.  In this case, the value at member_count will reflect the
 *    number of elements filled at member_array (so could be less than the
 *    number of members in the aggregate if member_max was nonzero and less
 *    than the number of members in the aggregate).
 */
static void
_bcm_sirius_trunk_get(_sirius_lag_unit_t *unitData,
                      const unsigned int lagIndex,
                      const int fabric,
                      bcm_trunk_info_t *t_data,
                      int member_max,
                      bcm_trunk_member_t *member_array,
                      int *member_count)
{
    unsigned int index;
    _sirius_aggregate_t *thisLag;

    LAG_EVERB((BSL_META("(%08X,%d,%s,*,%d,*,*) enter\n"),
               (unsigned int)unitData,
               lagIndex,
               fabric?"FABRIC":"FRONT-PANEL",
               member_max));

    if (fabric) {
        thisLag = unitData->fab[lagIndex];
    } else { /* if (fabric) */
        thisLag = unitData->aggr[lagIndex];
    } /* if (fabric) */
    /* fill in the outbound descriptor */
    if (t_data) {
        bcm_trunk_info_t_init(t_data);
        /*
         *  At this time, we do not support any of the new features in the new
         *  aggregate descriptor, so it's just left initialised and no more.
         */
    }
    /* fill in the member information */
    if (0 == member_max) {
        /* special case -- only check size needed */
        if (member_count) {
            *member_count = thisLag->targetCount;
        }
    } else {
        /* normal case -- copy member information to caller's buffer */
        for (index = 0;
             (index < thisLag->targetCount) &&
             (index < member_max);
             index++) {
            if (member_array) {
                sal_memset(&(member_array[index]),
                           0x00,
                           sizeof(*member_array));
                member_array[index].gport = thisLag->origGport[index].gport;
                /*
                 *  At this time, we do not support any of the new features in
                 *  the new aggregate member descriptor, so each member
                 *  descriptor is cleared and then only the GPORT provided.
                 */
            }
        }
        /* set number of members copied to caller's buffer */
        if (member_count) {
            *member_count = index;
        }
    }

    LAG_EVERB((BSL_META("(%08X,%d,%s,*,%d,*,*) return\n"),
               (unsigned int)unitData,
               lagIndex,
               fabric?"FABRIC":"FRONT-PANEL",
               member_max));
}

/*
 *  Function
 *    _bcm_sirius_trunk_find
 *  Purpose
 *    Find the aggregate, given a target that is a member.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) _sirius_lag_unit_t *unitData = local unit data
 *    (in) bcm_module_t modid = the module ID to check (with port ID)
 *    (in) bcm_port_t port = the port ID to check (with module ID)
 *    (out) unsigned int *lagIndex = where to put the local aggregate index
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_NOT_FOUND if the module,port's target is not in any aggregate
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of module or port is not permitted.
 *
 *    An aggregate will still be found for a *local* module,port tuple that
 *    points to a target for that aggregate, even if that module,port is not
 *    specifically the tuple used to add that target to the aggregate.  Remote
 *    targets must be specified exactly as they were added to the aggregate.
 */

static int
_bcm_sirius_trunk_find(const int unit,
                       _sirius_lag_unit_t *unitData,
                       const bcm_module_t modid,
                       const bcm_port_t port,
                       unsigned int *lagIndex)
{
    int result;
    int isFabric = FALSE;
    unsigned int index;
    unsigned int limit;
    unsigned int target;
    bcm_module_t myModule;
    _modPort_t *modPort;

    /* need local module ID */
    result = bcm_stk_my_modid_get(unit, &myModule);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /* see if we can find the port by checking local usage */
    result = bcm_sirius_aggregate_gport_translate(unit,
                                                  0 /* flags */,
                                                  myModule,
                                                  modid,
                                                  port,
                                                  &target,
                                                  NULL,
                                                  NULL);
    if ((BCM_E_NONE == result) && (_SIRIUS_MC_MAX_TARGETS > target)) {
        /* is local unit; we can check whether target is in an aggregate */
        if (!(unitData->targets[target >> 5] & (1 << (target & 0x1F)))) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "target %d (%d:%08X) is not in"
                                   " an aggregate\n"),
                       target,
                       modid,
                       port));
            return BCM_E_NOT_FOUND;
        }
    } else if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get target from %d:%08X: %d (%s)\n"),
                   modid,
                   port,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /* find the target */
    if (_SIRIUS_MC_MAX_TARGETS <= target) {
        LAG_EVERB((BSL_META("%d:%08X is not local; scan cache\n"),
                   modid,
                   port));
        /* can't scan for it in the local bitmaps; look for mod,gport */
        /* first check fabric aggregates since multicast will do this a lot */
        for (index = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) {
            /* check all existing fabric aggregates */
            if (unitData->fab[index]) {
                /* this one exists */
                limit = unitData->fab[index]->targetCount;
                modPort = unitData->fab[index]->origGport;
                for (target = 0; target < limit; target++) {
                    if ((modid == modPort[target].module) &&
                        (port == modPort[target].gport)) {
                        /* this is the target */
                        LAG_EVERB((BSL_META("found %d:%08X at %08X.%d\n"),
                                   modid,
                                   port,
                                   index,
                                   target));
                        break;
                    }
                }
                if (target < limit) {
                    isFabric = TRUE;
                    break;
                }
            } /* if (unitData->fab[index]) */
        } /* for (index = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) */
        /* then check other aggregates (hopefully requested less often) */
        if (index >= _SIRIUS_FAB_LAG_COUNT) {
            /* not in fabric aggregates */
            for (index = 0; index < _SIRIUS_LAG_COUNT; index++) {
                /* check all existing front panel aggregates */
                if (unitData->aggr[index]) {
                    /* this one exists */
                    limit = unitData->aggr[index]->targetCount;
                    modPort = unitData->aggr[index]->origGport;
                    for (target = 0; target < limit; target++) {
                        /* check each module+port */
                        if ((modid == modPort[target].module) &&
                            (port == modPort[target].gport)) {
                            /* this is the target */
                            LAG_EVERB((BSL_META("found %d:%08X at %08X.%d\n"),
                                       modid,
                                       port,
                                       index,
                                       target));
                            break;
                        }
                    }
                    if (target < limit) {
                        break;
                    }
                } /* if (unitData->aggr[index]) */
            } /* for (index = 0; index < _LAG_MAX_COUNT; index++) */
        } /* if (index >= _SIRIUS_FAB_LAG_COUNT) */
    } else { /* if (_SIRIUS_MC_MAX_TARGETS > target) */
        /* just look for it in the bitmaps */
        /* first check fabric aggregates since multicast will do this a lot */
        for (index = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) {
            /* check all existing fabric aggregates */
            if (unitData->fab[index] &&
                (unitData->fab[index]->targets[target >> 5] &
                 (1 << (target & 0x1F)))) {
                /* this aggregate exists and contains the target */
                isFabric = TRUE;
                break;
            }
        }
        /* then check other aggregates (hopefully requested less often) */
        if (index >= _SIRIUS_FAB_LAG_COUNT) {
            for (index = 0; index < _SIRIUS_LAG_COUNT; index++) {
                /* check all existing front panel aggregates */
                if (unitData->aggr[index] &&
                    (unitData->aggr[index]->targets[target >> 5] &
                     (1 << (target & 0x1F)))) {
                    /* this aggregate exists and contains the target */
                    break;
                }
            } /* for (index = 0; index < _LAG_MAX_COUNT; index++) */
        } /* if (index >= _SIRIUS_FAB_LAG_COUNT) */
    } /* if (_SIRIUS_MC_MAX_TARGETS > target) */

    /* report results */
    if (isFabric) {
        if (_SIRIUS_FAB_LAG_COUNT > index) {
            /* found a fabric aggregate with the target */
            *lagIndex = index + _SIRIUS_FAB_LAG_MIN;
            return BCM_E_NONE;
        }
    } else {
        if (_SIRIUS_LAG_COUNT > index) {
            /* found a front panel aggregate with the target */
            *lagIndex = index;
            return BCM_E_NONE;
        }
    }
    LOG_ERROR(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "did not find %d:%08X in any aggregates\n"),
               modid,
               port));
    return BCM_E_NOT_FOUND;
}

/*
 *  Function
 *    _bcm_sirius_trunk_set
 *  Purpose
 *    Set the membership for the provided aggregate
 *  Arguments
 *    (in) int unit = the unit number on which to operate
 *    (in) _sirius_lag_unit_t *unitData = pointer to the unit data
 *    (in) bcm_trunk_id tid = the aggregate ID
 *    (in) bcm_trunk_add_info_t *addInfo = ptr to the add_info struct
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_BADID if the ID is invalid
 *      BCM_E_NOT_FOUND if the ID is not in use
 *      BCM_E_PARAM if any other argument is bogus
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    A single subport may only participate in a single aggregate at a time; it
 *    must be removed from the first one before adding it to another.
 *
 *    The BCM_TRUNK_MAX_PORTCNT limit of targets participating in an aggregate
 *    is completely bogus for Sirius, but it is still imposed due entirely to
 *    limitations in the API.
 *
 *    Remote ports (on other modules) are tracked but NOT verified.
 *
 *    Assumes most arguments are valid, and that the lock is owned.
 *
 *    Removes the 'down' state from any subports that were members of the
 *    aggregate but will be removed, as well as any that will be members of the
 *    aggregate when done.
 */
/*
 *  int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
        int member_count, bcm_trunk_member_t *member_array
 */
static int
_bcm_sirius_trunk_set(const int unit,
                      _sirius_lag_unit_t *unitData,
                      const bcm_trunk_t tid,
                      const bcm_trunk_info_t *trunk_info,
                      const int member_count,
                      const bcm_trunk_member_t *member_array)
{
    bcm_trunk_info_t ucLagInfo;             /* unicast workspace */
    bcm_trunk_member_t *ucLagMembers = NULL;/* unicast workspace */
    _lag_target_bitmap exTargets;           /* targets to remove from lag */
    _lag_target_bitmap newTargets;          /* targets in lag afterward */
    _lag_target_bitmap newGlobTargets;      /* new global tgts in lags */
    _lag_target_bitmap otherTargets;        /* targets in other aggregates */
    int result;                             /* working result */
    unsigned int lagIndex;                  /* aggregate index */
    _lag_target_bitmap *svt = NULL;         /* vector table buffer pointer */
    unsigned int svtCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                             SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    _modPort_t *newTargetList;              /* ptr to aggregate cache space */
    bcm_module_t myModule;                  /* local unit module ID */
    bcm_module_t module;                    /* working module ID */
    bcm_gport_t gport;                      /* working GPORT ID */
    _sirius_aggregate_t *thisLag = NULL;    /* ptr to working aggregate */
    unsigned int target = 0;                /* target index */
    unsigned int count = 0;                 /* target count */
    int os;                                 /* whether an HG is oversub */
    unsigned int osCount = 0;               /* number of oversubscribed */
    bcm_port_t osPort;                      /* port to check for oversub */
    int higigSpeed;                         /* higig physical speed */
    int subportSpeed;                       /* higig total subport speed */
    unsigned int *hg = NULL;                /* working higigs in uc groups */
    unsigned int currTarget;                /* working target for uc groups */
    int sysport[2];                         /* sysport IDs for unicast path */
    bcm_gport_t *gp = NULL;                 /* workspace for member port */
    bcm_if_t *ei = NULL;                    /* workspace for mamber encapId */
    int distCount = 0;                      /* distrib group member count */
    int xresult;                            /* discard result for backout */

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,%08X,*) enter\n"),
               unit, tid));

    /* get the data for this aggregate */
    if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
        (_SIRIUS_FAB_LAG_MAX >= tid)) {
        /* it is a 'fabric' aggregate */
        lagIndex = tid - _SIRIUS_FAB_LAG_MIN;
        thisLag = unitData->fab[lagIndex];
#if (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS)
        if (_SIRIUS_MC_MAX_TARGETS <= member_count) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "too many members (%d) in fabric aggregate on"
                                   " unit %d -- limit is %d\n"),
                       member_count,
                       unit,
                       _SIRIUS_MC_MAX_TARGETS));
            return BCM_E_PARAM;
        }
#else /* (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
        if (BCM_TRUNK_FABRIC_MAX_PORTCNT <= member_count) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "too many members (%d) in fabric aggregate on"
                                   " unit %d -- limit is %d\n"),
                       member_count,
                       unit,
                       BCM_TRUNK_FABRIC_MAX_PORTCNT));
            return BCM_E_PARAM;
        }
#endif /* (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
    } else if (_SIRIUS_LAG_COUNT > tid) {
        /* it is a 'non-fabric' aggregate */
        lagIndex = tid;
        thisLag = unitData->aggr[lagIndex];
    } else {
        /* it is not valid */
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "invalid aggregate ID %08X on unit %d\n"),
                   tid,
                   unit));
        return BCM_E_BADID;
    }
    if (!thisLag) {
        /* looked okay so far, but there's no aggregate here */
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unused aggregate ID %08X on unit %d\n"),
                   tid,
                   unit));
        return BCM_E_NOT_FOUND;
    }

    result = bcm_stk_my_modid_get(unit, &myModule);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (member_count) {
        newTargetList = sal_alloc(sizeof(*newTargetList) * member_count,
                                  "aggregate target GPORT cache");
        if (!newTargetList) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes GPORT cache\n"),
                       sizeof(*newTargetList) * member_count));
            return BCM_E_MEMORY;
        }
        sal_memset(newTargetList,
                   0x00,
                   sizeof(*newTargetList) * member_count);
    } else {
        newTargetList = NULL;
    }
    /* build new and ex target information */
    sal_memset(&newTargets, 0x00, sizeof(newTargets));
    for (lagIndex = 0; lagIndex < _LAG_TARGET_SIZE; lagIndex++) {
        exTargets[lagIndex] = thisLag->targets[lagIndex];
        newGlobTargets[lagIndex] = (unitData->targets[lagIndex] &
                                 ~(exTargets[lagIndex]));
        otherTargets[lagIndex] = newGlobTargets[lagIndex];
    }
    for (lagIndex = 0;
         (lagIndex < member_count) && (BCM_E_NONE == result);
         lagIndex++) {
        /* translate this module,GPORT to localPort */
        gport = member_array[lagIndex].gport;
        if (BCM_GPORT_IS_MODPORT(gport)) {
            module = BCM_GPORT_MODPORT_MODID_GET(gport);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
            module = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
        } else {
            /* don't need it here except for non-GPORT or above */
            module = myModule;
        }
        result = bcm_sirius_aggregate_gport_translate(unit,
                                                      0 /* flags */,
                                                      myModule,
                                                      module,
                                                      gport,
                                                      &target,
                                                      &count,
                                                      NULL);
        if (BCM_E_NONE != result) {
            /* called function displayed diagnostic */
            break;
        }
        if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
            (_SIRIUS_FAB_LAG_MAX >= tid) &&
            (myModule == module) &&
            (BCM_GPORT_IS_MODPORT(gport) ||
             BCM_GPORT_IS_EGRESS_MODPORT(gport) ||
             (!BCM_GPORT_IS_SET(gport)))) {
            /*
             *  The aggregate is a 'fabric' aggregate, this port is a local
             *  port or MODPORT/EGRESS_MODPORT class GPORT.  We need to check
             *  to see if it's oversubscribed.
             */
            if (BCM_GPORT_IS_MODPORT(gport)) {
                osPort = BCM_GPORT_MODPORT_PORT_GET(gport);
            } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
                osPort = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
            } else {
                osPort = gport;
            }
            if (_SIRIUS_FAB_LAG_MAX_PORTS <= osPort) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "invalid higig port %d on unit %d\n"),
                           osPort,
                           unit));
                result = BCM_E_PARAM;
                break;
            }
            /*
             *  WARNING: This bcm_port_speed_max line assumes that anybody who
             *  runs higigs in oversubscribed state will run them at their
             *  rated capacity, rather than running them at slower rates.  If
             *  this is not true for a particular customer, the actual
             *  configured higig speed needs to be written to the higigSpeed
             *  variable, rather than making this call to bcm_port_speed_max.
             */
            
            result = bcm_port_speed_max(unit, osPort, &higigSpeed);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to get unit %d higig port %d (%08X)"
                                       " speed: %d (%s)\n"),
                           unit,
                           osPort,
                           gport,
                           result,
                           _SHR_ERRMSG(result)));
                break;
            }
            
            /* get cumulative total of subport speeds */
            result = bcm_sirius_port_all_subports_speed_get(unit,
                                                            osPort,
                                                            &subportSpeed);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to get unit %d higig port %d (%08X)"
                                       " combined subport speed: %d (%s)\n"),
                           unit,
                           osPort,
                           gport,
                           result,
                           _SHR_ERRMSG(result)));
                break;
            }
            os = (subportSpeed > higigSpeed);
            LAG_EVERB((BSL_META("unit %d higig %d (%08X) speed=%d,"
                                " subports=%d%s\n"),
                       unit,
                       osPort,
                       gport,
                       higigSpeed,
                       subportSpeed,
                       os?" (oversubscribed)":""));
            /* fabric aggrigate and the current port is oversub higig */
            if (os) {
                osCount++;
            }
        }
        /* copy new target to the cached list */
        newTargetList[lagIndex].module = module;
        newTargetList[lagIndex].gport = gport;
        newTargetList[lagIndex].target = target;
        newTargetList[lagIndex].count = count;
        /* make sure local target is not in an aggregate already */
        if (_SIRIUS_MC_MAX_TARGETS > (target + count)) {
            while (count > 0) {
                count--;
                if (otherTargets[(target + count)>> 5] &
                    (1 << ((target + count) & 0x1F))) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unit %d gport %08X target %d"
                                           " (%08X) is in another aggregate\n"),
                               unit,
                               gport,
                               target + count,
                               gport));
                    result = BCM_E_CONFIG;
                    break;
                } /* if (this target already is in an aggregate) */
            } /* while (count > 0) */
        } /* if (no error and target is local) */
        /* update target maps */
        count = newTargetList[lagIndex].count;
        while (count > 0) {
            count--;
            if (_SIRIUS_MC_MAX_TARGETS > (target + count)) {
                /* add new local target to this aggregate */
                newTargets[(target + count) >> 5] |= (1 << ((target + count) & 0x1F));
                /* mark new local target as already in an aggregate */
                newGlobTargets[(target + count) >> 5] |= (1 << ((target + count) & 0x1F));
                /* don't remove new target from this aggregate later */
                exTargets[(target + count) >> 5] &= ~(1 << ((target + count) & 0x1F));
            } /* if (_SIRIUS_MC_MAX_TARGETS > target) */
        } /* while (count > 0) */
    } /* for (all ports in the new description) */

    /* remove replication groups for all of the old targets */
    /*
     *  This is done for all of the old targets because:
     *
     *  1) it is better to drop a frame that should not be forwarded than to
     *     forward it to the wrong place (for security reasons)
     *  2) code architecture requires the old distribution groups be destroyed
     *     before new ones can be configured
     *  3) code architecture requires the old distribution groups be destroyed
     *     before the sysport can be put beck into unicast mode
     */
    
    if ((_SIRIUS_LAG_COUNT > tid) && (unitData->redirectUnicast)) {
        /* only do the unicast distributions this way for non-fabric */
        sal_memset(&(otherTargets), 0x00, sizeof(otherTargets));
        for (lagIndex = 0;
             (lagIndex < thisLag->targetCount) && (BCM_E_NONE == result);
             lagIndex++) {
            if (1 == thisLag->origGport[lagIndex].count) {
                if (otherTargets[thisLag->origGport[lagIndex].target >> 5] &
                    (1 << (thisLag->origGport[lagIndex].target & 0x1F))) {
                    /* already hit this one */
                    continue;
                }
                /* indicate this one has been hit */
                otherTargets[thisLag->origGport[lagIndex].target >> 5] |=
                    (1 << (thisLag->origGport[lagIndex].target & 0x1F));
                /* this member has a local target, so find its local sysport */
                result = bcm_sbx_cosq_gport_sysport_from_egress_object(unit,
                                                                       thisLag->origGport[lagIndex].gport,
                                                                       &(sysport[0]),
                                                                       &(sysport[1]));
                if (BCM_E_NONE == result) {
                    LAG_EVERB((BSL_META("unit %d aggregate %d member %d:%08X"
                                        "(%d) -> sysport %03X,%03X -> GMT"
                                        " %05X,%05X\n"),
                               unit,
                               tid,
                               thisLag->origGport[lagIndex].module,
                               thisLag->origGport[lagIndex].gport,
                               thisLag->origGport[lagIndex].target,
                               sysport[0],
                               sysport[1],
                               sysport[0] + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET,
                               sysport[1] + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET));
                    /* now have group ID, get rid of its distribution group */
                    for (count = 0;
                         (count < 2) && (BCM_E_NONE == result);
                         count++) {
                        result = bcm_sirius_multicast_destroy(unit,
                                                              sysport[count] +
                                                              SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET);
                        if (BCM_E_NOT_FOUND == result) {
                            /* it does not exist, and we don't want it; okay. */
                            result = BCM_E_NONE;
                        } else if (BCM_E_NONE != result) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "unable to destroy unit %d unicast"
                                                   " distribution %d: %d (%s)\n"),
                                       unit,
                                       sysport[count],
                                       result,
                                       _SHR_ERRMSG(result)));
                        }
                        if (BCM_E_NONE == result) {
                            result = soc_sirius_fd_unicast_gmt_set(unit,
                                                                   sysport[count],
                                                                   thisLag->origGport[lagIndex].target);
                            if (BCM_E_NONE != result) {
                                LOG_ERROR(BSL_LS_BCM_TRUNK,
                                          (BSL_META_U(unit,
                                                      "unable to set unit %d sysport %d"
                                                       " unicast path to target %d: %d (%s)\n"),
                                           unit,
                                           sysport[count],
                                           thisLag->origGport[lagIndex].target,
                                           result,
                                           _SHR_ERRMSG(result)));
                            }
                        } /* if (BCM_E_NONE == result) */
                    } /* for (count = 0; count < 2; count++) */
                } /* if (BCM_E_NONE == result) */
            } else if (0 != thisLag->origGport[lagIndex].count) {
                /*
                 *  If zero, it's not local and can be skipped.  But if we didn't
                 *  cover it otherwise, something is inconsistent or unexpected.
                 */
                result = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d aggregate %d has inconsistent or"
                                       " unexpected state (%d,%08X,%d,%d)\n"),
                           unit,
                           tid,
                           thisLag->origGport[lagIndex].module,
                           thisLag->origGport[lagIndex].gport,
                           thisLag->origGport[lagIndex].count,
                           thisLag->origGport[lagIndex].target));
            }
        } /* for (all members of the old aggregate) */
    } /* if ((_SIRIUS_LAG_COUNT > tid)  && (unitData->redirectUnicast)) */

    /* compute the local distribution group */
    if ((_SIRIUS_LAG_COUNT > tid) && (unitData->redirectUnicast)) {
        /* only do the unicast distributions this way for non-fabric */
        gp = sal_alloc(sizeof(*gp) * BCM_TRUNK_MAX_PORTCNT, "gport workspace");
        if (!gp) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes gport workspace\n"),
                       sizeof(*gp) * BCM_TRUNK_MAX_PORTCNT));
            result = BCM_E_MEMORY;
        }
        ei = sal_alloc(sizeof(*ei) * BCM_TRUNK_MAX_PORTCNT, "encapid workspace");
        if (!ei) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes encapid workspace\n"),
                       sizeof(*ei) * BCM_TRUNK_MAX_PORTCNT));
            result = BCM_E_MEMORY;
        }
        sal_memset(&(otherTargets), 0x00, sizeof(otherTargets));
        for (lagIndex = 0;
             (lagIndex < member_count) && (BCM_E_NONE == result);
             lagIndex++) {
            if (1 == newTargetList[lagIndex].count) {
                if (otherTargets[newTargetList[lagIndex].target >> 5] &
                    (1 << (newTargetList[lagIndex].target & 0x1F))) {
                    /* already hit this one */
                    continue;
                }
                /* indicate this one has been hit */
                otherTargets[newTargetList[lagIndex].target >> 5] |=
                    (1 << (newTargetList[lagIndex].target & 0x1F));
                /* add this one to the distribution group */
                LAG_EVERB((BSL_META("adding %08X to distribution group for"
                                    " unit %d aggregate %d\n"),
                           newTargetList[lagIndex].gport,
                           unit,
                           tid));
                gp[distCount] = newTargetList[lagIndex].gport;
                ei[distCount] = 0;
                distCount++;
            } else if (0 != newTargetList[lagIndex].count) {
                result = BCM_E_CONFIG;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d aggregate %d configuration is not"
                                       " valid: Sirius is redistributing unciast"
                                       " over aggregates and index %d has > 1"
                                       " target: (%d,%08X,%d,%d)\n"),
                           unit,
                           tid,
                           lagIndex,
                           newTargetList[lagIndex].module,
                           newTargetList[lagIndex].gport,
                           newTargetList[lagIndex].count,
                           newTargetList[lagIndex].target));
            }
        } /* for (all members of the new aggregate) */
    } /* if ((_SIRIUS_LAG_COUNT > tid)  && (unitData->redirectUnicast)) */

    /* fetch the squelch table */
    if (BCM_E_NONE == result) {
        svt = sal_alloc(sizeof(*svt) * svtCount,
                        "squelch vector workspace");
        if (!svt) {
            result = BCM_E_MEMORY;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes SVT workspace\n"),
                       sizeof(*svt) * svtCount));
        } else {
            result = _bcm_sirius_squelch_read(unit, svt);
        }
    }

    /* adjust and commit the squelch table */
    if (BCM_E_NONE == result) {
        /* ensure none of the members or ex-members are 'down' */
        for (lagIndex = 0; lagIndex < _LAG_TARGET_SIZE; lagIndex++) {
            unitData->downTargets[lagIndex] &= (~(newTargets[lagIndex] |
                                                  exTargets[lagIndex]));
        }
        /* unsquelch ex-members of the aggregate */
        _bcm_sirius_squelch_targets_never(unit, svt, exTargets);
        /* distribute aggregate members across LBIDs */
        _bcm_sirius_squelch_targets_round_robin(unit,
                                                svt,
                                                member_count,
                                                newTargetList);
        /* commit the squelch table changes */
        result = _bcm_sirius_squelch_write(unit, svt);
    }

    /* commit changes to the aggregate */
    if (BCM_E_NONE == result) {
        /* dispose of old member cache */
        if (thisLag->origGport) {
            sal_free(thisLag->origGport);
        }
        /* set new aggregate properties */
        thisLag->origGport = newTargetList;
        thisLag->targetCount = member_count;
        for (lagIndex = 0; lagIndex < _LAG_TARGET_SIZE; lagIndex++) {
            thisLag->targets[lagIndex] = newTargets[lagIndex];
            unitData->targets[lagIndex] = newGlobTargets[lagIndex];
        }
    } else {
        /* something went wrong; get rid of new data space */
        if (newTargetList) {
            sal_free(newTargetList);
        }
    }

    /* create replication groups for all targets in new aggregate */
    /*
     *  This is done *after* the commit of the aggregate state because we don't
     *  want to have odd squelch vector settings in case this fails.  It is
     *  possible for any of the remaining code to fail but the basic aggregate
     *  still work.  Even so, we want to inform the caller of the failure while
     *  keeping proper track of current state.  It therefore also keeps the
     *  backout complexity lower on the error path.
     */
    if ((_SIRIUS_LAG_COUNT > tid) &&
        (unitData->redirectUnicast) &&
        (BCM_E_NONE == result)) {
        /* only do the unicast distributions this way for non-fabric */
        sal_memset(&(otherTargets), 0x00, sizeof(otherTargets));
        for (lagIndex = 0;
             (lagIndex < member_count) && (BCM_E_NONE == result);
             lagIndex++) {
            if (1 == newTargetList[lagIndex].count) {
                if (otherTargets[newTargetList[lagIndex].target >> 5] &
                    (1 << (newTargetList[lagIndex].target & 0x1F))) {
                    /* already hit this one */
                    continue;
                }
                /* indicate this one has been hit */
                otherTargets[newTargetList[lagIndex].target >> 5] |=
                    (1 << (newTargetList[lagIndex].target & 0x1F));
                /* this member has a local target, so find its local sysport */
                result = bcm_sbx_cosq_gport_sysport_from_egress_object(unit,
                                                                       thisLag->origGport[lagIndex].gport,
                                                                       &(sysport[0]),
                                                                       &(sysport[1]));
                if (BCM_E_NONE == result) {
                    LAG_EVERB((BSL_META("unit %d aggregate %d member %d:%08X"
                                        "(%d) -> sysport %03X,%03X -> GMT"
                                        " %05X,%05X\n"),
                               unit,
                               tid,
                               thisLag->origGport[lagIndex].module,
                               thisLag->origGport[lagIndex].gport,
                               thisLag->origGport[lagIndex].target,
                               sysport[0],
                               sysport[1],
                               sysport[0] + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET,
                               sysport[1] + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET));
                    /* now have group ID, get rid of its distribution group */
                    for (count = 0;
                         (count < 2) && (BCM_E_NONE == result);
                         count++) {
                        /* now have group ID, get rid of its distribution group */
                        result = bcm_sirius_multicast_aggregate_create_id(unit,
                                                                          BCM_MULTICAST_TYPE_L2 |
                                                                          BCM_MULTICAST_DISABLE_SRC_KNOCKOUT,
                                                                          sysport[count] +
                                                                          SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET);
                        if (BCM_E_EXISTS == result) {
                            /* possible inconsistent but recoverable state */
                            /* exists when try to create; we want to keep it */
                            result = BCM_E_NONE;
                        } else if (BCM_E_NONE != result) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "unable to create unit %d unicast"
                                                   " distribution %d: %d (%s)\n"),
                                       unit,
                                       sysport[count],
                                       result,
                                       _SHR_ERRMSG(result)));
                        }
                        if (BCM_E_NONE == result) {
                            result = bcm_sirius_multicast_egress_set_override_oi(unit,
                                                                                 sysport[count] +
                                                                                 SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET,
                                                                                 distCount,
                                                                                 &(gp[0]),
                                                                                 &(ei[0]),
                                                                                 FALSE);
                            if (BCM_E_NONE != result) {
                                LOG_ERROR(BSL_LS_BCM_TRUNK,
                                          (BSL_META_U(unit,
                                                      "unable to set unit %d sysport %d"
                                                       " distribution: %d (%s)\n"),
                                           unit,
                                           sysport[count],
                                           result,
                                           _SHR_ERRMSG(result)));
                            }
                        } /* if (BCM_E_NONE == result) */
                    } /* for (count = 0; count < 2; count++) */
                } /* if (BCM_E_NONE == result) */
            } else if (0 != thisLag->origGport[lagIndex].count) {
                /*
                 *  If zero, it's not local and can be skipped.  But if we didn't
                 *  cover it otherwise, something is inconsistent or unexpected.
                 */
                result = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unit %d aggregate %d has inconsistent or"
                                       " unexpected state (%d,%08X,%d,%d)\n"),
                           unit,
                           tid,
                           thisLag->origGport[lagIndex].module,
                           thisLag->origGport[lagIndex].gport,
                           thisLag->origGport[lagIndex].count,
                           thisLag->origGport[lagIndex].target));
            }
            if (BCM_E_NONE != result) {
                /* don't want loop increment if failure */
                break;
            }
        } /* for (all members of the old aggregate) */
        if (BCM_E_NONE != result) {
            /* something went wrong; revert changed sysports to unicast */
            lagIndex++;
            while (0 < lagIndex) {
                lagIndex--;
                if (1 == newTargetList[lagIndex].count) {
                    /* this member has a local target, so find its local sysport */
                    xresult = bcm_sbx_cosq_gport_sysport_from_egress_object(unit,
                                                                            thisLag->origGport[lagIndex].gport,
                                                                            &(sysport[0]),
                                                                            &(sysport[1]));
                    if (BCM_E_NONE == xresult) {
                        /* now have group ID, get rid of its distribution group */
                        for (count = 0;
                             count < 2;
                             count++) {
                            xresult = bcm_sirius_multicast_destroy(unit,
                                                                   sysport[count] +
                                                                   SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET);
                            if (BCM_E_NOT_FOUND == xresult) {
                                /* it does not exist, and we don't want it; okay. */
                                xresult = BCM_E_NONE;
                            } else if (BCM_E_NONE != xresult) {
                                LOG_ERROR(BSL_LS_BCM_TRUNK,
                                          (BSL_META_U(unit,
                                                      "unable to destroy unit %d unicast"
                                                       " distribution %d: %d (%s)\n"),
                                           unit,
                                           sysport[count],
                                           xresult,
                                           _SHR_ERRMSG(xresult)));
                            }
                            if (BCM_E_NONE == xresult) {
                                xresult = soc_sirius_fd_unicast_gmt_set(unit,
                                                                        sysport[count],
                                                                        thisLag->origGport[lagIndex].target);
                                if (BCM_E_NONE != xresult) {
                                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                                              (BSL_META_U(unit,
                                                          "unable to set unit %d sysport %d"
                                                           " unicast path to target %d: %d (%s)\n"),
                                               unit,
                                               sysport[count],
                                               thisLag->origGport[lagIndex].target,
                                               xresult,
                                               _SHR_ERRMSG(xresult)));
                                }
                            } /* if (BCM_E_NONE == xresult) */
                        } /* for (count = 0; count < 2; count++) */
                    } /* if (BCM_E_NONE == xresult) */
                } /* if (1 == newTargetList[lagIndex].count) */
            } /* while (0 < lagIndex) */
        } /* if (BCM_E_NONE != result) */
    } /* if (tid in range and redistrib unicast and no error so far) */

    /* for fabric aggregates of oversubscribed higig, set up uc targets */
    if (osCount && (BCM_E_NONE == result)) {
        LAG_EVERB((BSL_META("unit %d fabric aggregate with %d oversubscribed"
                            " higig port%s detected\n"),
                   unit,
                   osCount,
                   (osCount > 1)?"s":""));
        hg = sal_alloc(sizeof(*hg) * BCM_TRUNK_MAX_PORTCNT, "higig workspace");
        if (!hg) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes higig workspace\n"),
                       sizeof(*hg) * BCM_TRUNK_MAX_PORTCNT));
            result = BCM_E_MEMORY;
        }
        /* collect the higigs that are particpating */
        for (target = 0, osCount = 0;
             (target < member_count) && (BCM_E_NONE == result);
             target++) {
            gport = member_array[target].gport;
            if (BCM_GPORT_IS_MODPORT(gport)) {
                module = BCM_GPORT_MODPORT_MODID_GET(gport);
            } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
                module = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
            } else {
                /* don't need it here except for non-GPORT or above */
                module = myModule;
            }
            LAG_EVERB((BSL_META("tp[%d] = %08X\n"), target, gport));
            if (BCM_GPORT_IS_MODPORT(gport) &&
                (module == myModule)) {
                /* is local modport */
                hg[osCount] = BCM_GPORT_MODPORT_PORT_GET(gport);
                LAG_EVERB((BSL_META("local modport %08X -> higig %d\n"),
                           gport,
                           hg[osCount]));
            } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport) &&
                       (module == myModule)) {
                /* is local egress modport */
                hg[osCount] = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
                LAG_EVERB((BSL_META("local egress modport %08X -> higig %d\n"),
                           gport,
                           hg[osCount]));
            } else {
                /* it's not a local higig port */
                hg[osCount] = _SIRIUS_MC_MAX_TARGETS;
            }
            if ((SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS > hg[osCount]) &&
                (unitData->hgSubport[hg[osCount]] < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
                /* port is local higig */
                osCount++;
                if (osCount > _SIRIUS_FAB_LAG_MAX_PORTS) {
                    /* too many local ports got included somehow??? */
                    
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "there are too many local higig ports"
                                           " included in unit %d aggregate %08X\n"),
                               unit,
                               tid));
                    result = BCM_E_CONFIG;
                }
            }
        } /* for (all ports in original aggregate) */
        if (BCM_E_NONE == result) {
            LAG_EVERB((BSL_META("using %d local higig%s for unicast replication\n"),
                       osCount,
                       (osCount > 1)?"s":""));
#if _SBX_SIRIUS_LAG_EXCESS_VERBOSITY
            for (lagIndex = 0; lagIndex < osCount; lagIndex++) {
                LAG_EVERB((BSL_META("local higig %d is participating\n"),
                           hg[lagIndex]));
            }
#endif /* _SBX_SIRIUS_LAG_EXCESS_VERBOSITY */
        } /* if (BCM_E_NONE == result) */
    } /* if (osCount && (BCM_E_NONE == result)) */
    if (osCount && (BCM_E_NONE == result)) {
        ucLagMembers = sal_alloc(sizeof(*ucLagMembers) * osCount,
                                 "unicast aggregate member space");
        if (ucLagMembers) {
            bcm_trunk_info_t_init(&ucLagInfo);
            result = _bcm_sirius_aggregate_higig_uc_prepare(unit,
                                                            member_count,
                                                            member_array);
        } else {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes for unit %d"
                                   " unicast aggregate workspace\n"),
                       sizeof(*ucLagMembers) * osCount,
                       unit));
            result = BCM_E_MEMORY;
        }
        for (target = 0;
             (target < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) &&
             (BCM_E_NONE == result);
             target++) {
            os = TRUE;
            for (lagIndex = 0; lagIndex < osCount; lagIndex++) {
                currTarget = unitData->ohgSubport[target][hg[lagIndex]];
                if (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS > currTarget) {
                    /* this target has a replicant on this higig */
                    BCM_GPORT_CHILD_SET(ucLagMembers[lagIndex].gport,
                                        myModule,
                                        currTarget);
                } else {
                    /* this target has no replicant on this higig */
                    os = FALSE;
                }
            } /* for (all higigs in the original aggregate) */
            if (os) {
                /* this target has replicants on all higigs this aggregate */
                
                /*
                 *  We are creating an aggregate that covers the distribution
                 *  group for the current subport, but note that the
                 *  distribution group itself is not created here.  Instead,
                 *  the actual distribution group is created *later* when the
                 *  user calls bcm_cosq_gport_add, since at that time the local
                 *  target queues are also built (and we don't want to build
                 *  them here).
                 */
                for (lagIndex = _SIRIUS_FAB_LAG_MAX;
                     lagIndex >= _SIRIUS_FAB_LAG_MIN;
                     lagIndex--) {
                    if (!(unitData->fab[lagIndex - _SIRIUS_FAB_LAG_MIN])) {
                        /* found a free fabric aggregate */
                        break;
                    }
                }
                if (lagIndex >= _SIRIUS_FAB_LAG_MIN) {
                    unitData->fab[lagIndex -
                                  _SIRIUS_FAB_LAG_MIN] = sal_alloc(sizeof(*(unitData->fab[lagIndex])),
                                                                   "fabric aggregate descriptor");
                    if (!(unitData->fab[lagIndex - _SIRIUS_FAB_LAG_MIN])) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to allocate %d byte for"
                                               " fabric aggregate descriptor\n"),
                                   sizeof(*(unitData->fab[lagIndex]))));
                        result = BCM_E_MEMORY;
                        break;
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "no available fabric aggregates when"
                                           " trying to create internal aggregate"
                                           " for unit %d target %d\n"),
                               unit,
                               target));
                    result = BCM_E_RESOURCE;
                    break;
                }
                LAG_EVERB((BSL_META("create unit %d aggregate %08X for unicast"
                                    " oversubscribed distribution\n"),
                           unit,
                           lagIndex));
                sal_memset(unitData->fab[lagIndex - _SIRIUS_FAB_LAG_MIN],
                           0x00,
                           sizeof(*(unitData->fab[lagIndex - _SIRIUS_FAB_LAG_MIN])));
                result = _bcm_sirius_trunk_set(unit,
                                               unitData,
                                               lagIndex,
                                               &ucLagInfo,
                                               osCount,
                                               ucLagMembers);
                /*
                 *  The cosq module (specifically bcm_sirius_cosq_target_set)
                 *  will set up the unicast distribution aggregate when
                 *  performing the cosq_add_queue operation.
                 */
            } /* if (target has replicants on any higig in this aggregate) */
        } /* for (all possible targets) */
    } /* if (osCount && (BCM_E_NONE == result)) */

    if (hg) {
        sal_free(hg);
    }
    if (ucLagMembers) {
        sal_free(ucLagMembers);
    }
    if (ei) {
        sal_free(ei);
    }
    if (gp) {
        sal_free(gp);
    }
    if (svt) {
        sal_free(svt);
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "(%d,%08X,*) return %d (%s)\n"),
               unit,
               tid,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_aggregate_data_destroy
 *  Purpose
 *    Destroy any aggregate state for the unit and ensure hardware is
 *    configured appropriately.
 *  Arguments
 *    (in) _sirius_aggregates_t *unitData = pointer to unit data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 */
static int
_bcm_sirius_aggregate_data_destroy(int unit,
                                   _sirius_lag_unit_t *unitData)
{
    unsigned int lagIndex;
    unsigned int tgtIndex;
    bcm_module_t myModId;
    bcm_gport_t gport;
    int tmpRes;
    int result;
    bcm_trunk_info_t trunkInfo;

    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    if (unitData) {
        /* tear down aggregates to free any owned resources */
        bcm_trunk_info_t_init(&trunkInfo);
        for (lagIndex = 0; lagIndex < _SIRIUS_LAG_COUNT; lagIndex++) {
            if (unitData->aggr[lagIndex]) {
                /* this non-fabric aggregate exists */
                tmpRes = _bcm_sirius_trunk_set(unit,
                                               unitData,
                                               lagIndex,
                                               &trunkInfo,
                                               0,
                                               NULL);
            } /* if (this non-fabric aggregate exists) */
        } /* for (all possible non-fabric aggregates) */
        for (lagIndex = _SIRIUS_FAB_LAG_MIN;
             lagIndex <= _SIRIUS_FAB_LAG_MAX;
             lagIndex++) {
            if (unitData->fab[lagIndex - _SIRIUS_FAB_LAG_MIN]) {
                /* this fabric aggregate exists */
                tmpRes = _bcm_sirius_trunk_set(unit,
                                               unitData,
                                               lagIndex,
                                               &trunkInfo,
                                               0,
                                               NULL);
            } /* if (this fabric aggregate exists) */
        } /* for (all possible fabric aggregates) */
        /* tear down multicast targets for XGS mode higig ports */
        for (lagIndex = 0;
             lagIndex < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
             lagIndex++) {
            if (unitData->hgSubport[lagIndex] <
                SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) {
                BCM_GPORT_MODPORT_SET(gport, myModId, lagIndex);
                /* this higig is in XGS mode and has special target */
                /*
                 *  We don't abort (but do complain) if unable to destroy the
                 *  cosq internal egress ports.  It is possible that they have
                 *  already been destroyed by some other means.  However, we
                 *  will report any unexpected result to the caller.  For this
                 *  context, any error other than BCM_E_INIT is unexpected
                 *  (BCM_E_INIT is not unexpected because cosq may have already
                 *  been detached).
                 */
                tmpRes = bcm_sirius_cosq_destroy_egress_internal_port(unit,
                                                                      gport,
                                                                      unitData->hgSubport[lagIndex]);
                if ((BCM_E_NONE != tmpRes) && (BCM_E_INIT != tmpRes)) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unable to destroy unit %d egress"
                                           " internal port %d for higig %d:"
                                           " %d (%s)\n"),
                               unit,
                               unitData->hgSubport[lagIndex],
                               lagIndex,
                               result,
                               _SHR_ERRMSG(tmpRes)));
                    result = tmpRes;
                }
                unitData->hgSubport[lagIndex] = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
            } /* if (hgTarget[lagIndex] in use) */
        } /* for (all higig ports) */
        for (tgtIndex = 0;
             tgtIndex < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
             tgtIndex++) {
            for (lagIndex = 0;
                 lagIndex < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
                 lagIndex++) {
                if ((unitData->ohgSubport[tgtIndex][lagIndex] <
                     SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) &&
                    (unitData->ohgSubport[tgtIndex][lagIndex] != tgtIndex)) {
                    BCM_GPORT_MODPORT_SET(gport, myModId, lagIndex);
                    /* this target has another target for XGS oversub UC dst */
                    /*
                     *  We don't abort (but do complain) if unable to destroy
                     *  the cosq internal egress ports.  It is possible that
                     *  they have already been destroyed by some other means.
                     *  However, we will report any unexpected result to the
                     *  caller.  For this context, any error other than
                     *  BCM_E_INIT is unexpected (BCM_E_INIT is not unexpected
                     *  because cosq may have already been detached).
                     */
                    tmpRes = bcm_sirius_cosq_destroy_egress_internal_port(unit,
                                                                          gport,
                                                                          unitData->ohgSubport[tgtIndex][lagIndex]);
                    if ((BCM_E_NONE != tmpRes) && (BCM_E_INIT != tmpRes)) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to destroy unit %d egress"
                                               " internal port %d for target %d"
                                               " higig %d: %d (%s)\n"),
                                   unit,
                                   unitData->ohgSubport[tgtIndex][lagIndex],
                                   tgtIndex,
                                   lagIndex,
                                   result,
                                   _SHR_ERRMSG(result)));
                        result = tmpRes;
                    }
                    unitData->ohgSubport[tgtIndex][lagIndex] = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;
                } /* if (ohgTarget[tgtIndex][lagIndex] in use) */
            } /* for (all higig ports) */
        } /* for (all targets) */
        /* tear down unicast targets for XGS oversub unicast aggr subports */
        for (lagIndex = 0; lagIndex < _SIRIUS_LAG_COUNT; lagIndex++) {
            if (unitData->aggr[lagIndex]) {
                if (unitData->aggr[lagIndex]->origGport) {
                    sal_free(unitData->aggr[lagIndex]->origGport);
                }
                sal_free(unitData->aggr[lagIndex]);
                unitData->aggr[lagIndex] = NULL;
            }
        }
        for (lagIndex = 0; lagIndex < _SIRIUS_FAB_LAG_COUNT; lagIndex++) {
            if (unitData->fab[lagIndex]) {
                if (unitData->fab[lagIndex]->origGport) {
                    sal_free(unitData->fab[lagIndex]->origGport);
                }
                sal_free(unitData->fab[lagIndex]);
                unitData->fab[lagIndex] = NULL;
            }
        }
        sal_free(unitData);
    }
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_sirius_trunk_debug_aggregate
 *  Purpose
 *    Print debugging information for a single aggregate
 *  Arguments
 *    (in) int fabric = TRUE if fabric aggregate, FALSE otherwise
 *    (in) int id = the aggregate's numerical ID
 *    (in) _sirius_aggregate_t *aggrData = the aggregate's local data
 *  Return
 *    (void)
 *  Notes
 */
static void
_bcm_sirius_trunk_debug_aggregate(const int fabric,
                                  const int index,
                                  const _sirius_aggregate_t *aggrData)
{
    unsigned int offset;

    LOG_CLI((BSL_META("\n    aggregate ID %d (%s index %d)"),
             fabric?index + _SIRIUS_FAB_LAG_MIN:index,
             fabric?"fabric":"standard",
             index));
    LOG_CLI((BSL_META("\n      member count   = %d"), aggrData->targetCount));
    LOG_CLI((BSL_META("\n      member targets = ")));
    offset = _LAG_TARGET_SIZE;
    while (offset > 0) {
        offset--;
        LOG_CLI((BSL_META("%08X"), aggrData->targets[offset]));
    }
    for (offset = 0; offset < aggrData->targetCount; offset++) {
        LOG_CLI((BSL_META("\n      member %2d info = %08X %02X %02X"),
                 offset,
                 aggrData->origGport[offset].gport,
                 aggrData->origGport[offset].target,
                 aggrData->origGport[offset].count));
    }
}

/*
 *  Function
 *    _bcm_sirius_trunk_port_collect
 *  Purpose
 *    Collect all of the targets that go to a particular higig port.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_module_t myModId = local module ID (of the unit above)
 *    (in) bcm_port_t port = higig port to find
 *    (out) _lag_target_bitmap *targets = where to put target bitmap
 *  Return
 *    bcm_error_t cast as int
 *  Notes
 *    This assumes the lock is taken, and that the input data are valid.
 */
static int
_bcm_sirius_trunk_port_collect(int unit,
                               bcm_module_t myModId,
                               bcm_port_t port,
                               _lag_target_bitmap *targets)
{
    unsigned int target;
    unsigned int count;
    int result;
    bcm_gport_t subport = BCM_GPORT_INVALID;

    sal_memset(targets, 0x00, sizeof(*targets));
    do { /* while (no error and not at end of subport list) */
        result = bcm_sirius_port_subport_getnext(unit, port, 0, &subport);
        if (BCM_E_NONE == result) {
            if (BCM_GPORT_INVALID != subport) {
                /* this subport is valid, so not at end yet */
                LAG_EVERB((BSL_META("consider GPORT %08X down\n"),
                           subport));
                result = bcm_sirius_aggregate_gport_translate(unit,
                                                              0 /* flags */,
                                                              myModId,
                                                              myModId,
                                                              subport,
                                                              &target,
                                                              &count,
                                                              NULL);
                if ((BCM_E_NONE == result) &&
                    (_SIRIUS_MC_MAX_TARGETS > target)) {
                    LAG_EVERB((BSL_META("consider target %3d down\n"),
                               target));
                    /* got target okay and it's local; mark it as down */
                    while (count > 0) {
                        count--;
                        (*targets)[(target + count) >> 5] |= (1 << ((target + count) & 0x1F));
                    }
                }
            } else { /* if (BCM_GPORT_INVALID != subport) */
                LAG_EVERB((BSL_META("done collecting GPORTs\n")));
            } /* if (BCM_GPORT_INVALID != subport) */
        } else { /* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "Unable to get next GPORT from unit %d"
                                   "port %d GPORT %08X: %d (%s)\n"),
                       unit,
                       port,
                       subport,
                       result,
                       _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } while ((BCM_E_NONE == result) && (BCM_GPORT_INVALID != subport));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_trunk_port_recalculate
 *  Purpose
 *    Recalculate the aggregates on a unit after the 'down' ports change.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in/out) _sirius_lag_unit_t *unitData = the data for the unit
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    This assumes the lock is taken, and that the input data are valid.
 */
static int
_bcm_sirius_trunk_port_recalculate(int unit,
                                   _sirius_lag_unit_t *unitData)
{
    _lag_target_bitmap *svt = NULL;
    unsigned int svtCount = (1 + SOC_MEM_INFO(unit, EG_FD_SVTm).index_max -
                             SOC_MEM_INFO(unit, EG_FD_SVTm).index_min);
    _sirius_aggregate_t *currAggr;
    unsigned int index;
    int result;

    /* set up squelch vector table workspace and fetch it */
    LAG_EVERB((BSL_META("read current squelch table\n")));
    svt = sal_alloc(sizeof(*svt) * svtCount,
                    "squelch vector table workspace");
    if (!svt) {
        result = BCM_E_MEMORY;
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to allocate %d bytes SVT workspace\n"),
                   sizeof(*svt) * svtCount));
    } else {
        result = _bcm_sirius_squelch_read(unit, svt);
        /* called function emitted diagnostic */
    }

    if (BCM_E_NONE == result) {
        LAG_EVERB((BSL_META("recompute non-fabric aggregates\n")));
        for (index = 0; index < _SIRIUS_LAG_COUNT; index++) {
            if ((currAggr = unitData->aggr[index])
                /* assignment intentional */) {
                /* this aggregate exists; refresh it */
                _bcm_sirius_squelch_targets_round_robin(unit,
                                                        svt,
                                                        currAggr->targetCount,
                                                        currAggr->origGport);
            }
        } /* for (index = 0; index < _SIRIUS_LAG_COUNT; index++) */
        LAG_EVERB((BSL_META("recompute fabric aggregates\n")));
        for (index = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) {
            if ((currAggr = unitData->fab[index])
                /* assignment intentional */) {
                /* this fabric aggregate exists; refresh it */
                _bcm_sirius_squelch_targets_round_robin(unit,
                                                        svt,
                                                        currAggr->targetCount,
                                                        currAggr->origGport);
            }
        } /* for (index = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) */
        LAG_EVERB((BSL_META("commit revised squelch table\n")));
        result = _bcm_sirius_squelch_write(unit, svt);
    } /* if (BCM_E_NONE == result) */

    if (svt) {
        /* get rid of SVT working buffer */
        sal_free(svt);
    }
    return result;
}

/*****************************************************************************
 *
 *  Interface -- standard API
 */

/*
 *  Function
 *    bcm_sirius_trunk_init
 *  Purpose
 *    Initialise the 'trunk' module (which manages aggregates, not trunking!).
 *    Sets up basic defaults where nothing is in an aggregate.
 *  Arguments
 *    (in) int unit = the unit number to initialise
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    none
 */
int
bcm_sirius_trunk_init(int unit)
{
    int result = BCM_E_NONE;
    int xres;
    sal_mutex_t tempLock = NULL;
    _sirius_lag_unit_t *tempUnit = NULL;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    LAG_UNIT_CHECK(unit);

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        /* Certain blocks are not fully inited in this case, can not write to memory in
	 * those blocks, skip trunk BCM init. This is here for emulator and bringup
	 * could be cleaned up after sirius pass bringup stage.
	 */
        return BCM_E_NONE;
    }

    /* see if this module has been initialised anywhere locally */
    if (!_lag_lock) {
        /* not initialised yet; set up global information */
        tempLock = sal_mutex_create("aggregation lock");
        if (!tempLock) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to create global lock\n")));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to claim new global lock\n")));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        _lag_lock = tempLock;
        memset(&_lag_unit, 0, sizeof(_lag_unit));
        if (sal_mutex_give(tempLock)) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to release new global lock\n")));
            return BCM_E_INTERNAL;
        }
    }
    /* claim global lock */
    if (sal_mutex_take(_lag_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to claim global lock\n")));
        result = BCM_E_INTERNAL;
    }
    /* try to recover from first create race condition */
    if (tempLock && (tempLock != _lag_lock)) {
        /* looks like somebody clobbered our lock; toss it */
        sal_mutex_destroy(tempLock);
    }
    if (BCM_E_NONE != result) {
        /* at this point, we can safely exit if error */
        return result;
    }
    /* detach first if already initialised */
    if (_lag_unit[unit]) {
        tempUnit = _lag_unit[unit];
        _lag_unit[unit] = NULL;
        /* this unit is already initialised; detach and then reinit */
        result = _bcm_sirius_aggregate_data_destroy(unit, tempUnit);
        if (BCM_E_NONE != result) {
            _lag_unit[unit] = tempUnit;
        }
    } /* if (_lag_unit[unit]) */
    /* create management structures */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_aggregate_data_create(unit,&tempUnit);
        /* called function displayed error */
    } /* if (BCM_E_NONE == result) */
    /* initialise the hardware */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_aggregate_initial_squelch_state(unit);
        /* called function displayed error */
        if (BCM_E_NONE != result) {
            /* something went wrong; dispose of unit information */
            xres = _bcm_sirius_aggregate_data_destroy(unit, tempUnit);
            COMPILER_REFERENCE(xres);
            tempUnit = NULL;
        }
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* success; commit unit init and unlock it */
        _lag_unit[unit] = tempUnit;
    } /* if (BCM_E_NONE == result) */
    /* release global lock */
    if (sal_mutex_give(_lag_lock)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to release global lock\n")));
        result = BCM_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_detach
 *  Purpose
 *    Shut down the 'trunk' module (which manages aggregates, not trunking!),
 *    ensuring nothing is left in aggregated state.
 *  Arguments
 *    (in) int unit = the unit number to shut down
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    none
 */
int
bcm_sirius_trunk_detach(int unit)
{
    int result = BCM_E_INTERNAL;
    _sirius_lag_unit_t *tempUnit;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    /* make sure something is initialised */
    if (!_lag_lock) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "no local units are initialised\n")));
        return BCM_E_INIT;
    }
    /* take the global lock (prevent concurrent init/detach) */
    if (sal_mutex_take(_lag_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to take global lock\n")));
        return BCM_E_INTERNAL;
    }
    /* get rid of this unit */
    tempUnit = _lag_unit[unit];
    _lag_unit[unit] = NULL;
    if (tempUnit) {
        /* this unit is initialised */

        if (sal_mutex_take(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock,
                           sal_mutex_FOREVER)) {
            /* but we can't take the lock */
            result = BCM_E_INTERNAL;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to take unit %d lock\n"),
                       unit));
            _lag_unit[unit] = tempUnit;
        } else {
            /* we took the lock; destroy unit data */
            result = _bcm_sirius_aggregate_data_destroy(unit, tempUnit);
            if (BCM_E_NONE == result) {
                /* set initial squelch vector table state */
                result = _bcm_sirius_aggregate_initial_squelch_state(unit);
            } else {
                /* could not destroy unit; put it back */
                _lag_unit[unit] = tempUnit;
            }
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unit %d was was not initialised\n"),
                   unit));
        result = BCM_E_INIT;
    }
    if (sal_mutex_give(_lag_lock)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unable to release global lock\n")));
        result = BCM_E_INTERNAL;
    }
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_create
 *  Purpose
 *    Create a new aggregate on the specified unit.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (out) bcm_trunk_id *tid = where to put the aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_FULL if no more IDs available
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Never creates 'fabric' aggregates.
 */
int
bcm_sirius_trunk_create(int unit, bcm_trunk_t *tid)
{
    int result = BCM_E_NONE;
    unsigned int index;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,*) enter\n"),
                 unit));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unit %d can only create with ID during reload\n"),
                   unit));
        return BCM_E_CONFIG;
    }
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    if (!tid) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "NULL out argument is unacceptable\n")));
        return BCM_E_PARAM;
    }
    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    for (index = 0; index < _SIRIUS_LAG_COUNT; index++) {
        if (!unitData->aggr[index]) {
            /* this one is free! */
            break;
        }
    }
    if (index >= _SIRIUS_LAG_COUNT) {
        /* did not find one */
        result = BCM_E_FULL;
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "unit %d has no available aggregates\n"),
                   unit));
    } else {
        /* found one that was free */
        unitData->aggr[index] = sal_alloc(sizeof(*unitData->aggr[index]),
                                          "aggregate descriptor");
        if (!unitData->aggr[index]) {
            /* did not get the alloc cell */
            result = BCM_E_MEMORY;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "not enough memory to create aggregate\n")));
        } else {
            /* got the needed alloc cell */
            sal_memset(unitData->aggr[index],
                       0x00,
                       sizeof(*unitData->aggr[index]));
            *tid = index;
            result = BCM_E_NONE;
        }
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,&(%08X)) return %d (%s)\n"),
                 unit,
                 *tid,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_create_id
 *  Purpose
 *    Create a new aggregate on the specified unit, with a specific ID.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_trunk_id tid = the requested aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_BADID if the requested ID is not valid
 *      BCM_E_EXISTS if the requested ID is already in use
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The return code for invalid ID DOES NOT CONFORM to typical BCM API
 *    returns (most other BCM API sections seem to insist upon BCM_E_NOT_FOUND
 *    when an invalid ID is used for any reason).
 */
int
bcm_sirius_trunk_create_id(int unit, bcm_trunk_t tid)
{
    int result = BCM_E_NONE;
    unsigned int index;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X) enter\n"),
                 unit, tid));

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
        (_SIRIUS_FAB_LAG_MAX >= tid)) {
        /* it's a 'fabric' aggregate */
        index = tid - _SIRIUS_FAB_LAG_MIN;
        if (unitData->fab[index]) {
            /* ID already in use */
            result = BCM_E_EXISTS;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "aggregate ID %08X already exists on unit"
                                   " %d\n"),
                       tid,
                       unit));
        } else {
            unitData->fab[index] = sal_alloc(sizeof(*(unitData->fab[index])),
                                             "fabric aggregate descriptor");
            if (unitData->fab[index]) {
                /* got the new cell; initialise */
                sal_memset(unitData->fab[index],
                           0x00,
                           sizeof(*(unitData->fab[index])));
                result = BCM_E_NONE;
            } else {
                /* did not get the alloc cell */
                result = BCM_E_MEMORY;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "not enough memory to create fabric"
                                       " aggregate\n")));
            }
        }
    } else { /* if (tid implies a fabric aggregate) */
        /* ID is valid */
        index = tid;
        if (index >= _SIRIUS_LAG_COUNT) {
            /* invalid ID */
            result = BCM_E_BADID;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "invalid aggregate ID %08X on unit %d\n"),
                       tid,
                       unit));
        } else if (unitData->aggr[index]) {
            /* ID already in use */
            result = BCM_E_EXISTS;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "aggregate ID %08X already exists on unit"
                                   " %d\n"),
                       tid,
                       unit));
        } else { /* if (unitData->aggr[index]) */
            /* ID is not in use */
            unitData->aggr[index] = sal_alloc(sizeof(*(unitData->aggr[index])),
                                              "aggregate descriptor");
            if (!unitData->aggr[index]) {
                /* did not get the alloc cell */
                result = BCM_E_MEMORY;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "not enough memory to create"
                                       " aggregate\n")));
            } else { /* if (!unitData->aggr[index]) */
                /* got the needed alloc cell */
                sal_memset(unitData->aggr[index],
                           0x00,
                           sizeof(*(unitData->aggr[index])));
                result = BCM_E_NONE;
            } /* if (!unitData->aggr[index]) */
        } /* if (unitData->aggr[index]) */
    } /* if (tid implies a fabric aggregate) */

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X) return %d (%s)\n"),
                 unit,
                 tid,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_set
 *  Purpose
 *    Set the target membership for an aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_trunk_id tid = the aggregate ID
 *    (in) bcm_trunk_info_t *trunk_info = ptr to the trunk_info struct
 *    (in) int member_count = number of members
 *    (in) bcm_trunk_member_t *member_array = where to find member information
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_BADID if the ID is invalid
 *      BCM_E_NOT_FOUND if the ID is not in use
 *      BCM_E_PARAM if any other argument is bogus
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The canonical purpose description for this function, 'Add ports to a
 *    trunk group', is bogus; it neglects to consider even the simple fact that
 *    targets can actually be removed by merely not including them in the list
 *    (in fact, this is the *only* way short of destroying and recreating).
 *    The notes in the code for the function finally describe the actual
 *    behaviour, but that's probably not in the documentation...
 *
 *    A single target may only participate in a single aggregate at a time; it
 *    must be removed from the first one before adding it to another.
 *
 *    Remote targets (on other modules) are tracked but NOT verified.
 */
int
bcm_sirius_trunk_set(int unit,
                     bcm_trunk_t tid,
                     bcm_trunk_info_t *trunk_info,
                     int member_count,
                     bcm_trunk_member_t *member_array)
{
    int result;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*) enter\n"),
                 unit, tid));

    /* get this checking out of the way before locking */
    if (!trunk_info) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "NULL pointer unacceptable for"
                               " mandatory argument\n")));
        return BCM_E_PARAM;
    }
    if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
        (_SIRIUS_FAB_LAG_MAX >= tid)) {
        /* sanity checks for fabric aggregates */
        /* none other than below and making sure ID valid (got to here) */
    } else {
        /* sanity checks for front-panel aggregates */
        if (tid >= _SIRIUS_LAG_COUNT) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "invalid aggregate ID %08X on unit %d\n"),
                       tid,
                       unit));
            return BCM_E_BADID;
        }
    }
    if ((0 > member_count) || (BCM_TRUNK_MAX_PORTCNT < member_count)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "invalid number (%d) of members in unit %d"
                               " aggregate %d\n"),
                   member_count,
                   unit,
                   tid));
        return BCM_E_PARAM;
    }
    if (member_count && (!member_array)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "member array can not be NULL if count nonzero\n")));
        return BCM_E_PARAM;
    }

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    /* set up the aggregate */
    result = _bcm_sirius_trunk_set(unit,
                                   unitData,
                                   tid,
                                   trunk_info,
                                   member_count,
                                   member_array);

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*) return %d (%s)\n"),
                 unit,
                 tid,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_get
 *  Purpose
 *    Get the target membership for an aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_trunk_id tid = the aggregate ID
 *    (out) bcm_trunk_info_t *t_data = where to put aggregate information
 *    (in) int member_max = max number of members to retrieve
 *    (out) bcm_trunk_member_t *member_array = pointer to member buffer
 *    (out) int *member_count = where to put retrieved member count
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_BADID if the ID is invalid
 *      BCM_E_NOT_FOUND if the ID is not in use
 *      BCM_E_PARAM if any other argument is bogus
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Some fields are not supported on 'set'; they're always zeroed on 'get'.
 *
 *    The data filled in are literally the data used in the 'set', cached
 *    specifically for the purpose of implementing 'get' (they're not used at
 *    all internally).
 */
int
bcm_sirius_trunk_get(int unit,
                     bcm_trunk_t tid,
                     bcm_trunk_info_t *t_data,
                     int member_max,
                     bcm_trunk_member_t *member_array,
                     int *member_count)
{
    int result = BCM_E_INTERNAL;
    unsigned int index;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*,%d,*,*) enter\n"),
                 unit, tid, member_max));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        LOG_WARN(BSL_LS_BCM_TRUNK,
                 (BSL_META_U(unit,
                             "unit %d state may be out of sync during reload\n"),
                  unit));
    }
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    /* get this checking out of the way before locking */
    if ((!t_data) || (!member_count)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "NULL pointer unacceptable for"
                               " obligatory outbound argument\n")));
        return BCM_E_PARAM;
    }
    if (0 > member_max) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "member_max must be non-negative\n")));
        return BCM_E_PARAM;
    } else if ((0 < member_max) && !member_array) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "member_array must not be NULL if member_max is"
                               " non-negative\n")));
        return BCM_E_PARAM;
    }

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    result = BCM_E_NONE;
    if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
        (_SIRIUS_FAB_LAG_MAX >= tid)) {
        /* it's a 'fabric' aggregate */
        index = tid - _SIRIUS_FAB_LAG_MIN;
        if (!unitData->fab[index]) {
            /* not in use */
            result = BCM_E_NOT_FOUND;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unit %d aggregate ID %08X does not exist\n"),
                       unit,
                       tid));
        }
    } else { /* if (fabric aggregate) */
        /* it's not a 'fabric' aggregate */
        index = tid;
        if (index >= _SIRIUS_LAG_COUNT) {
            /* invalid ID */
            result = BCM_E_BADID;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "invalid aggregate ID %08X on unit %d\n"),
                       tid,
                       unit));
        } else if (!unitData->aggr[index]) {
            /* ID not in use */
            result = BCM_E_NOT_FOUND;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unit %d aggregate ID %08X does not exist\n"),
                       unit,
                       tid));
        }
    } /* if (fabric aggregate) */
    /* build the outbound descriptor */
    if (BCM_E_NONE == result) {
        _bcm_sirius_trunk_get(unitData,
                              index,
                              tid > index,
                              t_data,
                              member_max,
                              member_array,
                              member_count);
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*,%d,*,&(%d)) return %d (%s)\n"),
                 unit,
                 tid,
                 member_max,
                 *member_count,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_find
 *  Purpose
 *    Find the aggregate, given a target that is a member.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_module_t modid = the module ID to check (with port ID)
 *    (in) bcm_port_t port = the port ID to check (with module ID)
 *    (out) bcm_trunk_id *tid = where to put the aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_NOT_FOUND if the module,port's target is not in any aggregate
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of module or port is not permitted.
 *
 *    For simplicity of explanation, only the original module,port should be
 *    used when searching.  There are conditions under which this is not
 *    obligatory, but it should generally be considered necessary.
 */
int
bcm_sirius_trunk_find(int unit,
                      bcm_module_t modid,
                      bcm_port_t port,
                      bcm_trunk_t *tid)
{
    int result;
    unsigned int index;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d,%08X,*) enter\n"),
                 unit, modid, port));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        LOG_WARN(BSL_LS_BCM_TRUNK,
                 (BSL_META_U(unit,
                             "unit %d state may be out of sync during reload\n"),
                  unit));
    }
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    /* get this checking out of the way before locking */
    if (!tid) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "NULL pointer unacceptable for"
                               " outbound argument\n")));
        return BCM_E_PARAM;
    }

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    /* search for an aggregate with the module,port included */
    result = _bcm_sirius_trunk_find(unit, unitData, modid, port, &index);
    /* called function already displayed error */
    if (BCM_E_NONE == result) {
        *tid = index;
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d,%08X,&(%08X)) return %d (%s)\n"),
                 unit,
                 modid,
                 port,
                 *tid,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_destroy
 *  Purpose
 *    Destroy an aggregate, returning its (remaining) members to normal use.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_trunk_id tid = the aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_BADID if the ID is invalid
 *      BCM_E_NOT_FOUND if the ID is not in use
 *      BCM_E_PARAM if any other argument is bogus
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_sirius_trunk_destroy(int unit, bcm_trunk_t tid)
{
    int result;
    unsigned int index;
    _sirius_lag_unit_t *unitData;
    _sirius_aggregate_t *thisLag = NULL;
    bcm_trunk_info_t trunkInfo;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*) enter\n"),
                 unit, tid));

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    /* set aggregate to have no membership */
    bcm_trunk_info_t_init(&trunkInfo);
    result = _bcm_sirius_trunk_set(unit, unitData, tid, &trunkInfo, 0, NULL);

    if (BCM_E_NONE == result) {
        /* aggregate now has no members */
        if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
            (_SIRIUS_FAB_LAG_MAX >= tid)) {
            /* it's a 'fabric' aggregate */
            index = tid - _SIRIUS_FAB_LAG_MIN;
            thisLag = unitData->fab[index];
        } else { /* if (fabric aggregate) */
            /* it's not a 'fabric' aggregate */
            index = tid;
            if (index >= _SIRIUS_LAG_COUNT) {
                /* invalid ID */
                result = BCM_E_BADID;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "invalid aggregate ID %08X on unit %d\n"),
                           tid,
                           unit));
            } else { /* if (index >= _SIRIUS_LAG_COUNT) */
                thisLag = unitData->aggr[index];
            } /* if (index >= _SIRIUS_LAG_COUNT) */
        } /* if (fabric aggregate) */
        if ((BCM_E_NONE == result) && !thisLag) {
            /* not in use */
            result = BCM_E_NOT_FOUND;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unit %d aggregate ID %08X does not exist\n"),
                       unit,
                       tid));
        }
    }

    /* dispose of the aggregate */
    if (BCM_E_NONE == result) {
        /* dispose of old member cache */
        if (thisLag->origGport) {
            sal_free(thisLag->origGport);
        }
        /* dispose of the aggregate */
        if ((_SIRIUS_FAB_LAG_MIN <= tid) &&
            (_SIRIUS_FAB_LAG_MAX >= tid)) {
            /* fabric aggregate */
            unitData->fab[index] = NULL;
            sal_free(thisLag);
        } else {
            /* non-fabric aggregate */
            unitData->aggr[index] = NULL;
            sal_free(thisLag);
        }
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%08X,*) return %d (%s)\n"),
                 unit,
                 tid,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_destroy
 *  Purpose
 *    Destroy an aggregate, returning its (remaining) members to normal use.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_trunk_id tid = the aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    We don't really need the init check at this time for Sirius (everything
 *    returned is a constant), but the API doc directly implies that it is
 *    necessary.  Maybe later if there are multiple versions...
 *
 *    This does not take the lock because it is a read-only operation of
 *    metadata that will not change.
 */
int
bcm_sirius_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    _sirius_lag_unit_t *unitData;

    if (!ta_info) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "NULL pointer to obligatory out argument\n")));
        return BCM_E_PARAM;
    }

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);

    bcm_trunk_chip_info_t_init(ta_info);
#if (BCM_TRUNK_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS)
    ta_info->trunk_ports_max = _SIRIUS_MC_MAX_TARGETS;
#else /* (BCM_TRUNK_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
    ta_info->trunk_ports_max = BCM_TRUNK_MAX_PORTCNT;
#endif /* (BCM_TRUNK_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = _SIRIUS_LAG_COUNT - 1;
    ta_info->trunk_group_count = _SIRIUS_LAG_COUNT;
    ta_info->trunk_fabric_id_min = _SIRIUS_FAB_LAG_MIN;
    ta_info->trunk_fabric_id_max = _SIRIUS_FAB_LAG_MAX;
#if (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS)
    ta_info->trunk_fabric_ports_max = _SIRIUS_FAB_LAG_MAX_PORTS;
#else /* (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
    ta_info->trunk_fabric_ports_max = BCM_TRUNK_FABRIC_MAX_PORTCNT;
#endif /* (BCM_TRUNK_FABRIC_MAX_PORTCNT > _SIRIUS_MC_MAX_TARGETS) */
    return BCM_E_NONE;
}


/*****************************************************************************
 *
 *  Interface -- special API
 */

/*
 *  Function
 *    bcm_sirius_trunk_find_and_get
 *  Purpose
 *    Find the aggregate, given a target that is a member, and also get the
 *    'set' information for the aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_module_t modid = the module ID to check (with port ID)
 *    (in) bcm_port_t port = the port ID to check (with module ID)
 *    (out) bcm_trunk_id *tid = where to put the aggregate ID
 *    (out) bcm_trunk_info_t *t_data = where to put aggregate information
 *    (in) int member_max = max number of members to retrieve
 *    (out) bcm_trunk_member_t *member_array = pointer to member buffer
 *    (out) int *member_count = where to put retrieved member count
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_NOT_FOUND if the module,port is not in any aggregate
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of module or port is not permitted.
 *
 *    For simplicity of explanation, only the original module,port should be
 *    used when searching.  There are conditions under which this is not
 *    obligatory, but it should generally be considered necessary.
 *
 *    NULL arguments are allowed for tid and info; if these arguments are NULL,
 *    the appropriate data will not be returned.  It is valid for both to be
 *    NULL; this function will serve as an 'is this port in an aggregate' query
 *    in that case, still returning the expected result code without specifics,
 *    but then bcm_trunk_find would be similar.
 */
int
bcm_sirius_trunk_find_and_get(int unit,
                              bcm_module_t modid,
                              bcm_port_t port,
                              bcm_trunk_t *tid,
                              bcm_trunk_info_t *t_data,
                              int member_max,
                              bcm_trunk_member_t *member_array,
                              int *member_count)
{
    int result;
    unsigned int index;
    _sirius_lag_unit_t *unitData;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d,%08X,*,*) enter\n"),
                 unit, modid, port));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        LOG_WARN(BSL_LS_BCM_TRUNK,
                 (BSL_META_U(unit,
                             "unit %d state may be out of sync during reload\n"),
                  unit));
    }
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    /* search for an aggregate with the module,port included */
    result = _bcm_sirius_trunk_find(unit, unitData, modid, port, &index);
    /* called function already displayed error */
    if (BCM_E_NONE == result) {
        if (t_data) {
            if ((_SIRIUS_FAB_LAG_MIN <= index) &&
                (_SIRIUS_FAB_LAG_MAX >= index)) {
                /* valid fabric lag */
                _bcm_sirius_trunk_get(unitData,
                                      index - _SIRIUS_FAB_LAG_MIN,
                                      TRUE,
                                      t_data,
                                      member_max,
                                      member_array,
                                      member_count);
            } else if (_SIRIUS_LAG_COUNT > index) {
                /* valid front-panel lag */
                _bcm_sirius_trunk_get(unitData,
                                      index,
                                      FALSE,
                                      t_data,
                                      member_max,
                                      member_array,
                                      member_count);
            } else {
                
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unexpected condition searching unit %d for"
                                       " an aggregate with modid=%d,port=%08X:"
                                       " index=%08X (%d)\n"),
                           unit,
                           modid,
                           port,
                           index,
                           index));
                result = BCM_E_INTERNAL;
            }
        }
        if (tid) {
            *tid = index;
        }
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d,%08X,&(%08X),*) return %d (%s)\n"),
                 unit,
                 modid,
                 port,
                 tid?*tid:0xFFFFFFFF,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_port_down
 *  Purpose
 *    Mark all of the targets going to a particular port as down, and remove
 *    them from all active aggregates, recalculating the aggregates so that the
 *    traffic will be put on any remaining subports.  It is possible that an
 *    aggregate will lose all local ports if all of the associated higigs go
 *    down; hopefully the control software can react quickly enough to prevent
 *    multiple-module aggregates from losing all traffic that would have gone
 *    to a particular module in such a case.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_port_t = the higig port number being declared 'down'
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of port is not permitted.  Declaring multiple ports as down
 *    will require multiple calls.
 *
 *    All targets associated with the specified higig will be declared 'down'
 *    by this function.  To get them back up, the caller needs only to update
 *    the aggregates that contain them (yes, the same configuration is
 *    acceptable, but doing so will cause problems such as frames getting stuck
 *    in buffers or queues if the higig link has not been fixed before the call
 *    is made).
 */
int
bcm_sirius_trunk_port_down(int unit, bcm_port_t port)
{
    int result;
    _sirius_lag_unit_t *unitData;
    bcm_module_t myModId;
    _lag_target_bitmap downTargets;
    unsigned int index;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d) enter\n"),
                 unit, port));

    LAG_UNIT_CHECK(unit);
    if ((0 > port) || (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS <= port)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "higig port %d is invalid on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PARAM;
    }
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    LAG_EVERB((BSL_META("Unit %d port %d has gone down; recomputing"
                        " aggregates that include it\n"),
               unit,
               port));

    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
    }

    /* get all of the subports on this higig; make them into targets */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_trunk_port_collect(unit,
                                                myModId,
                                                port,
                                                &downTargets);
        /* called function displayed diagnostic */
    } /* if (BCM_E_NONE == result) */

    if (BCM_E_NONE == result) {
        LAG_EVERB((BSL_META("mark downed GPORTs as being down\n")));
        for (index = 0; index < _LAG_TARGET_SIZE; index++) {
            unitData->downTargets[index] |= downTargets[index];
        }
        /* recompute the aggregates based upon the new target states */
        result = _bcm_sirius_trunk_port_recalculate(unit, unitData);
        /* called function displayed diagnostic */
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d) return %d (%s)\n"),
                 unit,
                 port,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_port_up
 *  Purpose
 *    Mark all of the targets going to a particular port as up, and return them
 *    to active aggregates, recalculating the aggregates so that the traffic
 *    will be put on all available subports.  This has the opposite effect of
 *    the complementary bcm_sirius_trunk_port_down call.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_port_t = the higig port number being declared 'down'
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of port is not permitted.  Declaring multiple ports as down
 *    will require multiple calls.
 *
 *    All targets associated with the specified higig will be declared not
 *    'down' by this function.
 */
int
bcm_sirius_trunk_port_up(int unit, bcm_port_t port)
{
    int result;
    _sirius_lag_unit_t *unitData;
    bcm_module_t myModId;
    _lag_target_bitmap downTargets;
    unsigned int index;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d) enter\n"),
                 unit, port));

    LAG_UNIT_CHECK(unit);
    if ((0 > port) || (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS <= port)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "higig port %d is invalid on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PARAM;
    }
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    LAG_EVERB((BSL_META("Unit %d port %d has gone down; recomputing"
                        " aggregates that include it\n"),
               unit,
               port));

    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
    }

    /* get all of the subports on this higig; make them into targets */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_trunk_port_collect(unit,
                                                myModId,
                                                port,
                                                &downTargets);
        /* called function displayed diagnostic */
    } /* if (BCM_E_NONE == result) */

    if (BCM_E_NONE == result) {
        LAG_EVERB((BSL_META("mark downed GPORTs as being down\n")));
        for (index = 0; index < _LAG_TARGET_SIZE; index++) {
            unitData->downTargets[index] &= (~(downTargets[index]));
        }
        /* recompute the aggregates based upon the new target states */
        result = _bcm_sirius_trunk_port_recalculate(unit, unitData);
        /* called function displayed diagnostic */
    }

    LAG_LOCK_GIVE(unit, unitData);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "(%d,%d) return %d (%s)\n"),
                 unit,
                 port,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_trunk_debug
 *  Purpose
 *    Print a diagnostic dump of this module's status.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    uses bsl_printf without conditions (so should always show up)
 */
int
bcm_sirius_trunk_debug(int unit) {
    _sirius_lag_unit_t *unitData;
    _sirius_aggregate_t *aggrData;
    unsigned int index;
    unsigned int offset;
    int result = BCM_E_NONE;

    LAG_UNIT_CHECK(unit);
    LAG_INIT_CHECK(unit, unitData);
    LAG_LOCK_TAKE(unit, unitData);

    LOG_CLI((BSL_META_U(unit,
                        "unit %d aggregate data:"), unit));
    LOG_CLI((BSL_META_U(unit,
                        "\n  member  = ")));
    offset = _LAG_TARGET_SIZE;
    while (offset > 0) {
        offset--;
        LOG_CLI((BSL_META_U(unit,
                            "%08X"), unitData->targets[offset]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n  down    = ")));
    offset = _LAG_TARGET_SIZE;
    while (offset > 0) {
        offset--;
        LOG_CLI((BSL_META_U(unit,
                            "%08X"), unitData->downTargets[offset]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n  higig   =")));
    for (index = 0; index < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; index++) {
        LOG_CLI((BSL_META_U(unit,
                            " %03X"), unitData->hgSubport[index]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n  oversub =")));
    for (index = 0; index < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; index++) {
        if (index) {
            if (index & 0x03) {
                LOG_CLI((BSL_META_U(unit,
                                    ";")));
            } else {
                LOG_CLI((BSL_META_U(unit,
                                    ";\n           ")));
            }
        }
        for (offset = 0;
             offset < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
             offset++) {
            LOG_CLI((BSL_META_U(unit,
                                " %03X"), unitData->ohgSubport[index][offset]));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n  standard aggregates:")));
    for (index = 0, offset = 0; index < _SIRIUS_LAG_COUNT; index++) {
        if ((aggrData = unitData->aggr[index])
            /* assignment intentional */) {
            _bcm_sirius_trunk_debug_aggregate(FALSE, index, aggrData);
            offset++;
        }
    }
    if (!offset) {
        LOG_CLI((BSL_META_U(unit,
                            "\n    (none)")));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n  fabric aggregates:")));
    for (index = 0, offset = 0; index < _SIRIUS_FAB_LAG_COUNT; index++) {
        if ((aggrData = unitData->fab[index])
            /* assignment intentional */) {
            _bcm_sirius_trunk_debug_aggregate(TRUE, index, aggrData);
            offset++;
        }
    }
    if (!offset) {
        LOG_CLI((BSL_META_U(unit,
                            "\n    (none)")));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    LAG_LOCK_GIVE(unit, unitData);
    return result;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/* Include this check to avoid potential resource leak */
#define __WB_CHECK_OVERRUN_NO_RETURN(z,str)                             \
    if ((operation != _WB_OP_SIZE) && (ptr+sizeof(z)>end_ptr)) {        \
        LOG_ERROR(BSL_LS_BCM_COMMON, \
                  (BSL_META("%s: Incoherent state of scache"       \
                            " (%s is not stored correctly).\n"),            \
                   FUNCTION_NAME(), #str));                       \
        goto error;                                                     \
    }

int
bcm_sirius_wb_trunk_state_sync(int unit, uint16 default_ver, uint16 recovered_ver,
			       uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int   rv = BCM_E_NONE;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    int   num_modports = 0, num_targets = 0;
    int   lag, modport, tmp_modport = -1, index, offset;
    _modPort_t *mp;
    _modPort_t *tmp_mp;
    _modPort_t *modports = NULL;
    uint8 *templates = NULL;
    uint8 *fab_templates =  NULL;
    int   found;
    unsigned int tgtOffset;
    int alloc_size;

    /* assuming that there will be less than 256 _modPort_t in a single unit */
#define _LAG_UNIT_MAX_PORTCNT (256)
    
    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Cannot calculate size during WarmBoot, unit %d\n"),
                           unit));
                rv = SOC_E_INTERNAL;
                goto error;
            }

            if (tmp_len == NULL) {
                rv = BCM_E_PARAM;
                goto error;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Cannot write scache during WarmBoot, unit %d\n"),
                           unit));
                rv = SOC_E_INTERNAL;
                goto error;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                rv = BCM_E_PARAM;
                goto error;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            
            alloc_size = sizeof(uint8) * _SIRIUS_LAG_COUNT * BCM_TRUNK_MAX_PORTCNT;
            templates = sal_alloc(alloc_size, "templates");
            if (templates == NULL) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to allocate %d bytes for unit %d templates\n"),
                           alloc_size, unit));
                rv = BCM_E_MEMORY;
                goto error;
            } else {
                sal_memset(templates, 0x00, alloc_size);
            }
            
            alloc_size = sizeof(uint8) * _SIRIUS_FAB_LAG_COUNT * BCM_TRUNK_MAX_PORTCNT;
            fab_templates = sal_alloc(alloc_size, "fab_templates");
            if (fab_templates == NULL) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to allocate %d bytes for unit %d fab_templates\n"),
                           alloc_size, unit));
                rv = BCM_E_MEMORY;
                goto error;
            } else {
                sal_memset(fab_templates, 0x00, alloc_size);
            }
            
            alloc_size = sizeof(_modPort_t) * _LAG_UNIT_MAX_PORTCNT;
            modports = sal_alloc(alloc_size, "_modPort_t for all lags");
            if (modports == NULL) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "unable to allocate %d bytes for unit %d total lag port list\n"),
                           alloc_size, unit));
                rv = BCM_E_MEMORY;
                goto error;
            } else {
                sal_memset(modports, 0x00, alloc_size);
            }
            break;
	case _WB_OP_DUMP:
	default:
            break;
    }
    
    /* _sirius_lag_unit_s */

    /* typedef struct _sirius_lag_unit_s {
     *     _lag_target_bitmap targets;                       (20 bytes)      
     *     _lag_target_bitmap downTargets;                   (20 bytes)
     *     _sirius_aggregate_t *aggr[_SIRIUS_LAG_COUNT];     (66 * sizeof(_sirius_aggregate_t), compressed)
     *     _sirius_aggregate_t *fab[_SIRIUS_FAB_LAG_COUNT];  (70 * sizeof(_sirius_aggregate_t), compressed)
     *     int redirectUnicast;                              (1 byte)
     *     int hgSubport[];                                  (4*2 bytes)
     *     int ohgSubport[][];                               (264*4*2 bytes)
     *  } _sirius_lag_unit_t;                                (total 7033 bytes)
     *
     * typedef struct _sirius_aggregate_s {
     *     _modPort_t *origGport;                            (16 bytes))
     *     unsigned int targetCount;                         (1 byte, this value <= 16)
     *     _lag_target_bitmap targets;                       (0 byte, recover from all of its members)
     * } _sirius_aggregate_t;                                (total 17 bytes)
     * 
     * typedef struct _modPort_s {
     *     bcm_port_t port;                                  (4 bytes)
     *     bcm_module_t module;                              (4 bytes)
     *     unsigned int target;                              (1 byte)
     *     unsigned int count;                               (1 byte)
     * } _modPort_t;                                         (total 10 bytes)
     * 
     * 
     * Given that a sirius only support 132 trunkable fabric ports and we might have 2 siriuses
     * connected to same PP so trunk could happen on 2 modules, we assume that we could have upto
     * 256 uniq _modPort_t structures. (SDK issue error when there is more than 256 structures)
     * We save these structures seperately and only keep an ID (1 byte) of the _modPort_t structure array.
     * 
     */
    switch (operation) {
	case _WB_OP_SIZE:	    /* size was calculated manually above for worst case */
        case _WB_OP_DECOMPRESS:
	    if (SOC_WARM_BOOT(unit) && (_lag_unit[unit] == NULL)) {
		/* coverity[ stack_use_overflow ] */
		rv = bcm_sirius_trunk_init(unit);
		if (rv != BCM_E_NONE) {
		    goto error;
		}
	    }

	    /* load all _modPort_t templates */
	    /* load all the _modPort_t templates */
            __WB_CHECK_OVERRUN_NO_RETURN(uint16, num_modports);
	    __WB_DECOMPRESS_SCALAR(uint16, num_modports);
            if (operation == _WB_OP_SIZE) {
                num_modports = _LAG_UNIT_MAX_PORTCNT;
            }
	    for (modport = 0; modport < num_modports; modport++) {
		__WB_CHECK_OVERRUN_NO_RETURN(int, modports[modport].module);
		__WB_DECOMPRESS_SCALAR(int, modports[modport].module);
		__WB_CHECK_OVERRUN_NO_RETURN(int, modports[modport].gport);
		__WB_DECOMPRESS_SCALAR(int, modports[modport].gport);
		__WB_CHECK_OVERRUN_NO_RETURN(uint8, modports[modport].target);
		__WB_DECOMPRESS_SCALAR(uint8, modports[modport].target);
		__WB_CHECK_OVERRUN_NO_RETURN(uint8, modports[modport].count);
		__WB_DECOMPRESS_SCALAR(uint8, modports[modport].count);
	    }
	    
	    /* load the lag unit */
	    /* targets */
	    for (index = 0; index < _LAG_TARGET_SIZE; index++) {
		__WB_CHECK_OVERRUN_NO_RETURN(uint32, _lag_unit[unit]->targets[index]);
		__WB_DECOMPRESS_SCALAR(uint32, _lag_unit[unit]->targets[index]);
	    }
	    /* downTargets */
	    for (index = 0; index < _LAG_TARGET_SIZE; index++) {
		__WB_CHECK_OVERRUN_NO_RETURN(uint32, _lag_unit[unit]->downTargets[index]);
		__WB_DECOMPRESS_SCALAR(uint32, _lag_unit[unit]->downTargets[index]);
	    }
	    /* redirectUnicast */
	    __WB_CHECK_OVERRUN_NO_RETURN(uint8, _lag_unit[unit]->redirectUnicast);
	    __WB_DECOMPRESS_SCALAR(uint8, _lag_unit[unit]->redirectUnicast);
	    /* hgSubport */
	    for (index = 0; index < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; index++) {
		__WB_CHECK_OVERRUN_NO_RETURN(uint16, _lag_unit[unit]->hgSubport[index]);
		__WB_DECOMPRESS_SCALAR(uint16, _lag_unit[unit]->hgSubport[index]);
	    }
	    /* ohgSubport */
	    for (index = 0; index < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; index++) {
		for (offset = 0; offset < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; offset++) {
		    __WB_CHECK_OVERRUN_NO_RETURN(uint16, _lag_unit[unit]->ohgSubport[index][offset]);
		    __WB_DECOMPRESS_SCALAR(uint16, _lag_unit[unit]->ohgSubport[index][offset]);
		}
	    }
	    /* aggr */
	    for (lag = 0; lag < _SIRIUS_LAG_COUNT; lag++) {
                if (SOC_WARM_BOOT(unit)) {
                    /* make sure the pointer is NULL */
                    if (_lag_unit[unit]->aggr[lag] != NULL) {
                        sal_free(_lag_unit[unit]->aggr[lag]);
                        _lag_unit[unit]->aggr[lag] = NULL;
                    }
                }
		__WB_CHECK_OVERRUN_NO_RETURN(uint8, num_targets);
		__WB_DECOMPRESS_SCALAR(uint8, num_targets);
		
		if (SOC_WARM_BOOT(unit) && (num_targets == 0xFF)) {
		    continue;
		}
		
                if (SOC_WARM_BOOT(unit)) {
                    /* allocate memory */
                    _lag_unit[unit]->aggr[lag] = sal_alloc(sizeof(_sirius_aggregate_t),
                                                           "aggregate target");
                    if (_lag_unit[unit]->aggr[lag] == NULL) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to allocate %d bytes for aggregate\n"),
                                   sizeof(_sirius_aggregate_t)));
                        rv = BCM_E_MEMORY;
                        goto error;
                    }

                    if ((num_targets > 0) && (num_targets < _LAG_TARGET_SIZE)) {
                        _lag_unit[unit]->aggr[lag]->origGport = 
                            sal_alloc(sizeof(_modPort_t)*num_targets, "aggregate target GPORT cache");
                        if (_lag_unit[unit]->aggr[lag]->origGport == NULL) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "unable to allocate %d bytes for aggregate\n"),
                                       sizeof(_modPort_t)*num_targets));
                            if (_lag_unit[unit]->aggr[lag] != NULL) {
                                sal_free(_lag_unit[unit]->aggr[lag]);
                                _lag_unit[unit]->aggr[lag] = NULL;
                            }
                            rv = BCM_E_MEMORY;
                            goto error;
                        }
                    } else {
                        _lag_unit[unit]->aggr[lag]->origGport = NULL;
                    }
                    for (index = 0; index < _LAG_TARGET_SIZE; index++) {
                        _lag_unit[unit]->aggr[lag]->targets[index] = 0;
                    }
                    _lag_unit[unit]->aggr[lag]->targetCount = num_targets;
                } else {
                    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                        num_targets = BCM_TRUNK_MAX_PORTCNT;
                    } else { /* SBX mode */
                        num_targets = (BCM_TRUNK_MAX_PORTCNT >> 5);
                    }
                }
                
                for (modport = 0; modport < num_targets; modport++) {
                    __WB_CHECK_OVERRUN_NO_RETURN(uint8, tmp_modport);
                    __WB_DECOMPRESS_SCALAR(uint8, tmp_modport);
                    if (SOC_WARM_BOOT(unit)) {
                        if (tmp_modport >= num_modports) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "Warmboot TRUNK database inconsistence detected on unit %d\n"),
                                       unit));
                            rv = BCM_E_INTERNAL;
                            goto error;
                        }
                        /* recover the GPORT */
                        _lag_unit[unit]->aggr[lag]->origGport[modport].module = modports[tmp_modport].module;
                        _lag_unit[unit]->aggr[lag]->origGport[modport].gport = modports[tmp_modport].gport;
                        _lag_unit[unit]->aggr[lag]->origGport[modport].target = modports[tmp_modport].target;
                        _lag_unit[unit]->aggr[lag]->origGport[modport].count = modports[tmp_modport].count;
                        
                        /* restore part of the target bitmap */
                        for (offset = 0; offset < modports[tmp_modport].count; offset++) {
                            tgtOffset = modports[tmp_modport].target + offset;
                            _lag_unit[unit]->aggr[lag]->targets[tgtOffset >> 5] |= (1 << (tgtOffset & 0x1F));
                        }
                    }
                }
            }
    
	    /* fab */
	    for (lag = 0; lag < _SIRIUS_FAB_LAG_COUNT; lag++) {
                if (SOC_WARM_BOOT(unit)) {
                    /* make sure the pointer is NULL */
                    if (_lag_unit[unit]->fab[lag] != NULL) {
                        sal_free(_lag_unit[unit]->fab[lag]);
                        _lag_unit[unit]->fab[lag] = NULL;
                    }
                }
		__WB_CHECK_OVERRUN_NO_RETURN(uint8, num_targets);
		__WB_DECOMPRESS_SCALAR(uint8, num_targets);
		
		if (SOC_WARM_BOOT(unit) && (num_targets == 0xFF)) {
		    continue;
		}

                if (SOC_WARM_BOOT(unit)) {
                    /* allocate memory */
                    _lag_unit[unit]->fab[lag] = sal_alloc(sizeof(_sirius_aggregate_t),
                                                          "aggregate target");
                    if (_lag_unit[unit]->fab[lag] == NULL) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to allocate %d bytes for aggregate\n"),
                                   sizeof(_sirius_aggregate_t)));
                        rv = BCM_E_MEMORY;
                        goto error;
                    }
                    if ((num_targets > 0) && (num_targets < _LAG_TARGET_SIZE)) {
                        _lag_unit[unit]->fab[lag]->origGport = 
                            sal_alloc(sizeof(_modPort_t)*num_targets, "aggregate target GPORT cache");
                        if (_lag_unit[unit]->fab[lag]->origGport == NULL) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "unable to allocate %d bytes for aggregate\n"),
                                       sizeof(_modPort_t)*num_targets));
                            if (_lag_unit[unit]->fab[lag] != NULL) {
                                sal_free(_lag_unit[unit]->fab[lag]);
                                _lag_unit[unit]->fab[lag] = NULL;
                            }
                            rv = BCM_E_MEMORY;
                            goto error;
                        }
                    } else {
                        _lag_unit[unit]->fab[lag]->origGport = NULL;
                    }
                    for (index = 0; index < _LAG_TARGET_SIZE; index++) {
                        _lag_unit[unit]->fab[lag]->targets[index] = 0;
                    }
                    _lag_unit[unit]->fab[lag]->targetCount = num_targets;

                } else {
                    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                        num_targets = BCM_TRUNK_MAX_PORTCNT;
                    } else { /* SBX mode */
                        num_targets = (BCM_TRUNK_MAX_PORTCNT >> 5);
                    }
                }

                for (modport = 0; modport < num_targets; modport++) {
                    __WB_CHECK_OVERRUN_NO_RETURN(uint8, tmp_modport);
                    __WB_DECOMPRESS_SCALAR(uint8, tmp_modport);
                    if (SOC_WARM_BOOT(unit)) {
                        if (tmp_modport >= num_modports) {
                            LOG_ERROR(BSL_LS_BCM_TRUNK,
                                      (BSL_META_U(unit,
                                                  "Warmboot TRUNK database inconsistence detected on unit %d\n"),
                                       unit));
                            rv = BCM_E_INTERNAL;
                            goto error;
                        }
                        /* recover the GPORT */
                        _lag_unit[unit]->fab[lag]->origGport[modport].module = modports[tmp_modport].module;
                        _lag_unit[unit]->fab[lag]->origGport[modport].gport = modports[tmp_modport].gport;
                        _lag_unit[unit]->fab[lag]->origGport[modport].target = modports[tmp_modport].target;
                        _lag_unit[unit]->fab[lag]->origGport[modport].count = modports[tmp_modport].count;
			
                        /* restore part of the target bitmap */
                        for (offset = 0; offset < modports[tmp_modport].count; offset++) {
                            tgtOffset = modports[tmp_modport].target + offset;
                            _lag_unit[unit]->fab[lag]->targets[tgtOffset >> 5] |= (1 << (tgtOffset & 0x1F));
                        }
                    }
                }
            }
	    break;
	case _WB_OP_COMPRESS:
	    if (_lag_unit[unit] != NULL) {
		/* scan and save all _modPort_t templates */
		num_modports = 0;

		/* scan the aggr database */
		for (lag = 0; lag < _SIRIUS_LAG_COUNT; lag++) {
		    if (_lag_unit[unit]->aggr[lag] == NULL) {
			/* skip the unused lag */
			continue;
		    }

		    /* scan the modport list for this lag */
		    for (modport = 0; modport < _lag_unit[unit]->aggr[lag]->targetCount; modport++) {
			/* the duplication should only happens within the lag */
			mp = &(_lag_unit[unit]->aggr[lag]->origGport[modport]);

			/* scan all the previous modports in the lag */
			found = FALSE;
			for (tmp_modport = 0; tmp_modport < modport; tmp_modport++) {
			    tmp_mp = &(_lag_unit[unit]->aggr[lag]->origGport[tmp_modport]);
			    if ((mp->module == tmp_mp->module) &&
				(mp->gport == tmp_mp->gport)) {
				/* found a matched one, no need to compare targets, it should be same */
				found = TRUE;
				break;
			    }
			}

			if (found) {
			    /* use the matched template id */
			    templates[lag*BCM_TRUNK_MAX_PORTCNT+modport] = 
				templates[lag*BCM_TRUNK_MAX_PORTCNT+tmp_modport];
			} else {
			    /* found a new one, copy to global templates and increment the global template id*/
			    modports[num_modports].module = mp->module;
			    modports[num_modports].gport  = mp->gport;
			    modports[num_modports].target = mp->target;
			    modports[num_modports].count  = mp->count;
			    num_modports++;
			    
			    if (num_modports >= _LAG_UNIT_MAX_PORTCNT) {
				LOG_ERROR(BSL_LS_BCM_TRUNK,
				          (BSL_META_U(unit,
				                      "more than %d _modport_t structures on unit %d\n"),
				           _LAG_UNIT_MAX_PORTCNT, unit));
				rv = BCM_E_INTERNAL;
				goto error;
			    }
			}
		    }
		}

		/* scan the fab database */
		for (lag = 0; lag < _SIRIUS_FAB_LAG_COUNT; lag++) {
		    if (_lag_unit[unit]->fab[lag] == NULL) {
			/* skip the unused lag */
			continue;
		    }
		    
		    for (modport = 0; modport < _lag_unit[unit]->fab[lag]->targetCount; modport++) {
			/* the duplication should only happens within the lag */
			mp = &(_lag_unit[unit]->fab[lag]->origGport[modport]);
			found = FALSE;
			for (tmp_modport = 0; tmp_modport < modport; tmp_modport++) {
			    tmp_mp = &(_lag_unit[unit]->fab[lag]->origGport[tmp_modport]);
			    if ((mp->module == tmp_mp->module) &&
				(mp->gport == tmp_mp->gport)) {
				/* found a matched one, no need to compare targets, it should be same */
				found = TRUE;
				break;
			    }
			}
			
			if (found) {
			    /* use the matched template id */
			    fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport] = 
				fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+tmp_modport];
			} else {
			    /* found a new one, copy to global templates and increment the global template id*/
			    modports[num_modports].module = mp->module;
                            modports[num_modports].gport  = mp->gport;
			    modports[num_modports].target = mp->target;
			    modports[num_modports].count  = mp->count;
			    fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport] = num_modports;
			    num_modports++;
			    if (num_modports >= _LAG_UNIT_MAX_PORTCNT) {
				LOG_ERROR(BSL_LS_BCM_TRUNK,
				          (BSL_META_U(unit,
				                      "more than %d _modport_t structures on unit %d\n"),
				           _LAG_UNIT_MAX_PORTCNT, unit));
				rv = BCM_E_INTERNAL;
				goto error;
			    }
			}
		    }
		}
		
		/* save all the _modPort_t templates */
		__WB_CHECK_OVERRUN_NO_RETURN(uint16, num_modports);
		__WB_COMPRESS_SCALAR(uint16, num_modports);
		for (modport = 0; modport < num_modports; modport++) {
		    __WB_CHECK_OVERRUN_NO_RETURN(int, modports[modport].module);
		    __WB_COMPRESS_SCALAR(int, modports[modport].module);
		    __WB_CHECK_OVERRUN_NO_RETURN(int, modports[modport].gport);
		    __WB_COMPRESS_SCALAR(int, modports[modport].gport);
		    __WB_CHECK_OVERRUN_NO_RETURN(uint8, modports[modport].target);
		    __WB_COMPRESS_SCALAR(uint8, modports[modport].target);
		    __WB_CHECK_OVERRUN_NO_RETURN(uint8, modports[modport].count);
		    __WB_COMPRESS_SCALAR(uint8, modports[modport].count);
		}

		/* save the lag unit */
		/* targets */
		for (index = 0; index < _LAG_TARGET_SIZE; index++) {
		    __WB_CHECK_OVERRUN_NO_RETURN(uint32, _lag_unit[unit]->targets[index]);
		    __WB_COMPRESS_SCALAR(uint32, _lag_unit[unit]->targets[index]);
		}
		/* downTargets */
		for (index = 0; index < _LAG_TARGET_SIZE; index++) {
		    __WB_CHECK_OVERRUN_NO_RETURN(uint32, _lag_unit[unit]->downTargets[index]);
		    __WB_COMPRESS_SCALAR(uint32, _lag_unit[unit]->downTargets[index]);
		}
		/* redirectUnicast */
		__WB_CHECK_OVERRUN_NO_RETURN(uint8, _lag_unit[unit]->redirectUnicast);
		__WB_COMPRESS_SCALAR(uint8, _lag_unit[unit]->redirectUnicast);
		/* hgSubport */
		for (index = 0; index < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; index++) {
		    __WB_CHECK_OVERRUN_NO_RETURN(uint16, _lag_unit[unit]->hgSubport[index]);
		    __WB_COMPRESS_SCALAR(uint16, _lag_unit[unit]->hgSubport[index]);
		}
		/* ohgSubport */
		for (index = 0; index < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; index++) {
		    for (offset = 0; offset < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; offset++) {
			__WB_CHECK_OVERRUN_NO_RETURN(uint16, _lag_unit[unit]->ohgSubport[index][offset]);
			__WB_COMPRESS_SCALAR(uint16, _lag_unit[unit]->ohgSubport[index][offset]);
		    }
		}
		/* aggr */
		for (lag = 0; lag < _SIRIUS_LAG_COUNT; lag++) {
		    if (_lag_unit[unit]->aggr[lag] == NULL) {
                      __WB_CHECK_OVERRUN_NO_RETURN(uint8, 0xFF);
                      __WB_COMPRESS_SCALAR(uint8, 0xFF);
		    } else {
			__WB_CHECK_OVERRUN_NO_RETURN(uint8, _lag_unit[unit]->aggr[lag]->targetCount);
			__WB_COMPRESS_SCALAR(uint8, _lag_unit[unit]->aggr[lag]->targetCount);
			for (modport = 0; modport < _lag_unit[unit]->aggr[lag]->targetCount; modport++) {
			    __WB_CHECK_OVERRUN_NO_RETURN(uint8, fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport]);
			    __WB_COMPRESS_SCALAR(uint8, fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport]);
			}
		    }
		}
		/* fab */
		for (lag = 0; lag < _SIRIUS_FAB_LAG_COUNT; lag++) {
		    if (_lag_unit[unit]->fab[lag] == NULL) {
                        __WB_CHECK_OVERRUN_NO_RETURN(uint8, 0xFF);
                        __WB_COMPRESS_SCALAR(uint8, 0xFF);
		    } else {
			__WB_CHECK_OVERRUN_NO_RETURN(uint8, _lag_unit[unit]->fab[lag]->targetCount);
			__WB_COMPRESS_SCALAR(uint8, _lag_unit[unit]->fab[lag]->targetCount);
			for (modport = 0; modport < _lag_unit[unit]->fab[lag]->targetCount; modport++) {
			    __WB_CHECK_OVERRUN_NO_RETURN(uint8, fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport]);
			    __WB_COMPRESS_SCALAR(uint8, fab_templates[lag*BCM_TRUNK_MAX_PORTCNT+modport]);
			}
		    }
		}
	    }
	    break;
	case _WB_OP_DUMP:
	default:
	    if (_lag_unit[unit] != NULL) {
		rv = bcm_sirius_trunk_debug(unit);
	    }
	    break;
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
	                (BSL_META_U(unit,
	                            "%s _lag_unit total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
	                (BSL_META_U(unit,
	                            "%s _lag_unit total %d bytes decompressed on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
	                (BSL_META_U(unit,
	                            "%s _lag_unit total %d bytes compressed on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

error:
    if (templates != NULL) {
	sal_free(templates);
    }

    if (fab_templates != NULL) {
	sal_free(fab_templates);
    }

    if (modports != NULL) {
	sal_free(modports);
    }
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
