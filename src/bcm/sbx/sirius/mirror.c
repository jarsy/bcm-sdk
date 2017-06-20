/*
 * $Id: mirror.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Mirror - Broadcom StrataSwitch Mirror API.
 */

#include <shared/bsl.h>

#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/stack.h>
#include <bcm/multicast.h>

#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/sirius/multicast.h>

#include <soc/sbx/sbFabCommon.h>
#include <soc/sbx/sirius.h>

/*
 *  This module implements mirror APIs on the Sirius.
 *
 *  The Sirius basically can only mirror on egress, since that is the only
 *  place where it has distribution set control, and the mirroring using such
 *  control would only apply to unicast frames (application would still have to
 *  set up multicast groups as needed so they also replicated frames similarly
 *  to the unicast mirroring).
 *
 *  This code depends heavily upon code and state in the Sirius trunk and
 *  multicast modules, in an effort to avoid local state and avoid tightly
 *  coupled code interactions.
 *
 *  Note that due to the way the Sirius works, the mirror APIs only work for
 *  child/egress_child port and modport/egress_modport ports, so the dest_mod
 *  argument is essentially ignored on set and will be based upon the port on
 *  get.  Also, due to hardware limitations, mirroring to an off-device
 *  destination is not permitted.  Mirror source is similarly limited.
 *
 *  Also, mirror source must be something that resolves as a sysport, while
 *  mirror destination is permitted to be anything addressable by the frame
 *  distribution hardware (any of the 132 targets).  Since the resolution is
 *  thus limited, mirroring is not supported in '264 port mode'.
 */

#define _SBX_SIRIUS_MIRROR_EXCESS_VERBOSITY TRUE
#if _SBX_SIRIUS_MIRROR_EXCESS_VERBOSITY
#define MIRROR_EVERB(stuff) LOG_DEBUG(BSL_LS_BCM_MIRROR, stuff)
#else /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */
#define MIRROR_EVERB(stuff)
#endif /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */


int
bcm_sirius_mirror_port_set(int unit,
                           bcm_port_t port,
                           bcm_module_t dest_mod,
                           bcm_port_t dest_port,
                           uint32 flags)
{
    int result;
    int oldResult = BCM_E_NONE;
    bcm_module_t myModId;
    bcm_module_t modId = (~0);
    bcm_port_t xport;
    int sysport[2]; /* ef and nef sysports */
    bcm_gport_t gport[2]; /* replicant destinations */
    bcm_if_t encap[2]; /* not used but required by internal call */
    unsigned int target = 0; /* base target for source */
    unsigned int count; /* target count for source */
    unsigned int index;

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "(%d, %08X, %d, %08X, %08X) enter\n"),
                 unit,
                 port,
                 dest_mod,
                 dest_port,
                 flags));

    if ((flags & (~(BCM_MIRROR_PORT_EGRESS)))) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d does not support flags %08X\n"),
                   unit,
                   flags & (~(BCM_MIRROR_PORT_EGRESS))));
        return BCM_E_PARAM;
    }
    
    if ((!BCM_GPORT_IS_CHILD(port)) &&
        (!BCM_GPORT_IS_EGRESS_CHILD(port)) &&
        (!BCM_GPORT_IS_EGRESS_GROUP(port))) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "source port %08X is not a supported form\n"),
                   port));
        return BCM_E_PARAM;
    }
    /* need local module ID */
    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (BCM_GPORT_IS_CHILD(port)) {
        modId = BCM_GPORT_CHILD_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
        modId = BCM_GPORT_EGRESS_CHILD_MODID_GET(port);
    } else if (BCM_GPORT_IS_MODPORT(port)) {
        modId = BCM_GPORT_MODPORT_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        modId = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(port)) {
        modId = BCM_GPORT_EGRESS_GROUP_MODID_GET(port);
    }
    if (modId != myModId) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "source port %08X module %d is not local"
                               " module %d\n"),
                   port,
                   modId,
                   myModId));
        return BCM_E_PARAM;
    }
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        /* mirroring is to be enabled, check dest module and port */
        if (dest_mod != myModId) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "dest module %d is not local module %d\n"),
                       dest_mod,
                       myModId));
            return BCM_E_PARAM;
        }
        if ((!BCM_GPORT_IS_CHILD(dest_port)) &&
            (!BCM_GPORT_IS_EGRESS_CHILD(dest_port)) &&
            (!BCM_GPORT_IS_EGRESS_GROUP(dest_port)) &&
            (!BCM_GPORT_IS_MODPORT(dest_port)) &&
            (!BCM_GPORT_IS_EGRESS_MODPORT(dest_port))) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "dest port %08X is not a supported form\n"),
                       dest_port));
            return BCM_E_PARAM;
        }
        if (BCM_GPORT_IS_CHILD(dest_port)) {
            modId = BCM_GPORT_CHILD_MODID_GET(dest_port);
        } else if (BCM_GPORT_IS_EGRESS_CHILD(dest_port)) {
            modId = BCM_GPORT_EGRESS_CHILD_MODID_GET(dest_port);
        } else if (BCM_GPORT_IS_MODPORT(dest_port)) {
            modId = BCM_GPORT_MODPORT_MODID_GET(dest_port);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(dest_port)) {
            modId = BCM_GPORT_EGRESS_MODPORT_MODID_GET(dest_port);
        } else if (BCM_GPORT_IS_EGRESS_GROUP(dest_port)) {
            modId = BCM_GPORT_EGRESS_GROUP_MODID_GET(dest_port);
        }
        if (modId != myModId) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "destination port %08X module %d is not"
                                   " local module %d\n"),
                       dest_port,
                       modId,
                       myModId));
            return BCM_E_PARAM;
        }
    } /* if (flags & BCM_MIRROR_PORT_EGRESS) */
    /* get target information for the source port */
    result = bcm_sirius_aggregate_gport_translate(unit,
                                                  BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP,
                                                  myModId,
                                                  myModId,
                                                  port,
                                                  &target,
                                                  &count,
                                                  NULL);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d source port %08X unable to determine"
                               " local target information: %d (%s)\n"),
                   unit,
                   port,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (_SIRIUS_MC_MAX_TARGETS <= target) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d source port %08X target %d not valid,"
                               " so probably not conifgured\n"),
                   unit,
                   port,
                   target));
        return BCM_E_PORT;
    }
    if (BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* the source port is a modport; must use target instead */
        BCM_GPORT_CHILD_SET(xport, myModId, target);
    } else {
        /* the source port is child or egress object */
        xport = port;
    }
    /* get the sysport information for this 'port' */
    result = bcm_sbx_cosq_gport_sysport_from_egress_object(unit,
                                                           xport,
                                                           &(sysport[0]),
                                                           &(sysport[1]));
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d port %08X->%08X unable to determine"
                               " local sysport: %d (%s)\n"),
                   unit,
                   port,
                   xport,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    MIRROR_EVERB((BSL_META("unit %d port %08X: %d ef, %d nef -- %d, %d\n"),
                  unit,
                  port,
                  sysport[0],
                  sysport[1],
                  target,
                  count));
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        /* enable egress mirroring */
        /* build distribution set */
        sal_memset(&(encap[0]), 0x00, sizeof(encap[0]) * 2);
        gport[0] = port;
        gport[1] = dest_port;
        for (index = 0;
             (index < 2) && (BCM_E_NONE == result);
             index++) {
            result = bcm_sirius_multicast_aggregate_create_id(unit,
                                                              BCM_MULTICAST_TYPE_L2 |
                                                              BCM_MULTICAST_DISABLE_SRC_KNOCKOUT,
                                                              sysport[index] +
                                                              SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET);
            if (BCM_E_EXISTS == result) {
                /* possible inconsistent but recoverable state */
                /* exists when try to create; we want to keep it */
                result = BCM_E_NONE;
            } else if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "unable to create unit %d unicast"
                                       " distribution %d: %d (%s)\n"),
                           unit,
                           sysport[index],
                           result,
                           _SHR_ERRMSG(result)));
            }
            if (BCM_E_NONE == result) {
                result = bcm_sirius_multicast_egress_set_override_oi(unit,
                                                                     sysport[index] +
                                                                     SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET,
                                                                     2,
                                                                     &(gport[0]),
                                                                     &(encap[0]),
                                                                     FALSE);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "unable to set unit %d sysport %d"
                                           " distribution: %d (%s)\n"),
                               unit,
                               sysport[index],
                               result,
                               _SHR_ERRMSG(result)));
                }
            } /* if (BCM_E_NONE == result) */
        } /* for (all sysports in the source) */
        oldResult = result; /* preserve this in case backout needed */
    }
    if ((BCM_E_NONE != oldResult) || (0 == (flags & BCM_MIRROR_PORT_EGRESS))) {
        /*
         *  Either something went wrong enabling mirroring, so we should
         *  switch the sysports back to unicast mode, or the caller wanted
         *  to disable mirroring, so we should switch back to unicast mode.
         *
         *  Only difference is we need to preserve the error if we are in
         *  this code due to an error.
         */
        for (index = 0;
             (index < 2) &&
             ((BCM_E_NONE == result) || (BCM_E_NONE != oldResult));
             index++) {
            result = bcm_sirius_multicast_destroy(unit,
                                                  sysport[index] +
                                                  SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET);
            if (BCM_E_NONE == result) {
                /* that went well, restore original sysport target pointer */
                /* coverity[overrun-local: : FALSE] */
                result = soc_sirius_fd_unicast_gmt_set(unit,
                                                       sysport[count],
                                                       (count>1)?target+index:target);
            } else if (BCM_E_NOT_FOUND == result) {
                /* no distribution group is the disired result, take it! */
                result = BCM_E_NONE;
            }
        } /* for (all sysports in the source */
        if (BCM_E_NONE != oldResult) {
            /* we are here because of error enabling mirroring */
            result = oldResult; /* restore original error */
        }
    } /* if (error or request was to disable mirroring) */

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "(%d, %08X, %d, %08X, %08X) return %d (%s)\n"),
                 unit,
                 port,
                 dest_mod,
                 dest_port,
                 flags,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int
bcm_sirius_mirror_port_get(int unit,
                           bcm_port_t port,
                           bcm_module_t *dest_mod,
                           bcm_port_t *dest_port,
                           uint32 *flags)
{
    int result;
    bcm_port_t xport;
    bcm_module_t myModId;
    bcm_module_t modId = (~0);
    int members; /* MC group member count */
    int sysport[2]; /* ef and nef sysports */
    bcm_gport_t gport[3]; /* replicant destinations */
    bcm_if_t encap[3]; /* not used but required by internal call */
    unsigned int target = 0; /* base target for source */
    unsigned int count; /* target count for source */
    unsigned int index;
    unsigned int t, c; /* target and count for dest */

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "(%d, %08X, *, *, *) enter\n"),
                 unit,
                 port));

    if ((!flags) || (!dest_port) || (!dest_mod)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "obligatort out arguments must not be NULL\n")));
        return BCM_E_PARAM;
    }
    
    if ((!BCM_GPORT_IS_CHILD(port)) &&
        (!BCM_GPORT_IS_EGRESS_CHILD(port)) &&
        (!BCM_GPORT_IS_EGRESS_GROUP(port))) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "source port %08X is not a supported form\n"),
                   port));
        return BCM_E_PARAM;
    }
    /* need local module ID */
    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to get unit %d module ID: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (BCM_GPORT_IS_CHILD(port)) {
        modId = BCM_GPORT_CHILD_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
        modId = BCM_GPORT_EGRESS_CHILD_MODID_GET(port);
    } else if (BCM_GPORT_IS_MODPORT(port)) {
        modId = BCM_GPORT_MODPORT_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        modId = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(port)) {
        modId = BCM_GPORT_EGRESS_GROUP_MODID_GET(port);
    }
    if (modId != myModId) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "source port %08X module %d is not local"
                               " module %d\n"),
                   port,
                   modId,
                   myModId));
        return BCM_E_PARAM;
    }
    /* get target information for the source port */
    result = bcm_sirius_aggregate_gport_translate(unit,
                                                  BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP,
                                                  myModId,
                                                  myModId,
                                                  port,
                                                  &target,
                                                  &count,
                                                  NULL);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d source port %08X unable to determine"
                               " local target information: %d (%s)\n"),
                   unit,
                   port,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (_SIRIUS_MC_MAX_TARGETS <= target) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d source port %08X target %d not valid,"
                               " so probably not conifgured\n"),
                   unit,
                   port,
                   target));
        return BCM_E_PORT;
    }
    if (BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* the source port is a modport; must use target instead */
        BCM_GPORT_CHILD_SET(xport, myModId, target);
    } else {
        /* the source port is child or egress object */
        xport = port;
    }
    /* get the sysport information for this 'port' */
    result = bcm_sbx_cosq_gport_sysport_from_egress_object(unit,
                                                           xport,
                                                           &(sysport[0]),
                                                           &(sysport[1]));
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d port %08X->%08X unable to determine"
                               " local sysport: %d (%s)\n"),
                   unit,
                   port,
                   xport,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    MIRROR_EVERB((BSL_META("unit %d port %08X: %d ef, %d nef -- %d, %d\n"),
                  unit,
                  port,
                  sysport[0],
                  sysport[1],
                  target,
                  count));
    /* get distribution group for ef sysport (both should be same) */
    /* coverity[uninit_use_in_call] */
    result = bcm_sirius_multicast_egress_get(unit,
                                             sysport[0] +
                                             SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET,
                                             3,
                                             &(gport[0]),
                                             &(encap[0]),
                                             &members);
    if (BCM_E_NONE == result) {
        /* group exists */
        if (2 != members) {
            /* group membership is wrong; mirroring only uses two */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unit %d port %08X has unexpected size"
                                   " distribution group\n"),
                       unit,
                       port));
            return BCM_E_CONFIG;
        }
        /* need to figure out which one is the mirror-to and return it */
        for (index = 0;
             (index < members) && (BCM_E_NONE == result);
             index++) {
            /* coverity[uninit_use_in_call] */    
            result = bcm_sirius_aggregate_gport_translate(unit,
                                                          BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP,
                                                          myModId,
                                                          myModId,
                                                          gport[index],
                                                          &t,
                                                          &c,
                                                          NULL);
            if (BCM_E_NONE == result) {
                if (((target + count) <= t) ||
                    ((t + c) <= target)) {
                    /* non-overlapping, so must be this one */
                    *dest_mod = myModId;
                    *dest_port = gport[index];
                    *flags = BCM_MIRROR_PORT_EGRESS;
                    break;
                }
            }
        } /* for (all members of the group) */
        if (index >= members) {
            /* did not find a non-identity member */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unit %d port %08X distribution group"
                                   " did not appear to include any targets"
                                   " other than the original port\n"),
                       unit,
                       port));
            result = BCM_E_INTERNAL;
        }
    } else if (BCM_E_NOT_FOUND == result) {
        /* group does not exist; no mirroring */
        result = BCM_E_NONE;
        *flags = 0;
    }

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "(%d, %08X, &(%d), &(%08X), &(%08X))"
                            " return %d (%s)\n"),
                 unit,
                 port,
                 *dest_mod,
                 *dest_port,
                 *flags,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


