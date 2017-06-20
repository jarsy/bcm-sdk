/*
 * $Id: mirror.c,v 1.12 Broadcom SDK $
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
#include <bcm/vlan.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/mirror.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/caladan3/wb_db_mirror.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>


/*
 *  This module implements mirror APIs on the SBX CALADAN3 (not sure what BCM
 *  part number it will eventualy bear).
 *
 *  the C3 has an extremely limited pool of mirrors, split into ingress and
 *  egress mirrors.  The operational behaviour of each appears to be similar,
 *  but the configuration (as the SBX library sees it) differs.  Even more
 *  interestingly, the mirror pool is asymmetric between ingress and egress,
 *  plus there are reserved mirrors.
 *
 *  This module is also used internally by at least the field module to manage
 *  mirroring on its entries.
 */


/*
 * Notes about the relationship between bcm_mirror_destination_t, 
 * _caladan3_ingress_mirror, _caladan3_ingress_mirror, and mirrorIds -
 *  
 * Mirror_ids directly map to the ingress mirror table, irrespective mirror 
 * type (ingress or egress).  When a mirror desination is created, a mirror_id
 * must be returned and since at creation time, the mirror type is not known,
 * an ingress mirror is used.  This approach also simplifies warm boot for
 * mirror id recovery since all ids are defined by the table, and every egress
 * mirror must have a corresponding ingress mirror entry even if the ingress
 * mirror is not used.
 * 
 */

/*
 *  Basic common stuff as macros
 */
#define MIRROR_UNIT_INIT_CHECK \
    /* make sure init has been run (try to do it if not) */ \
    if ((!_caladan3_mirror_glob_lock) || \
        (!_mirror_glob[unit]) || \
        (!_mirror_glob[unit]->tableLock)) { \
        LOG_DEBUG(BSL_LS_BCM_MIRROR, \
                  (BSL_META_U(unit, \
                              "not init yet; try to init\n"))); \
        result = bcm_caladan3_mirror_init(unit); \
        if (BCM_E_NONE != result) { \
            /* something went wrong during init */ \
            LOG_ERROR(BSL_LS_BCM_MIRROR, \
                      (BSL_META_U(unit, \
                                  "bcm_caladan3_mirror_init(%d)" \
                                   " returned %d (%s)\n"), \
                       unit, \
                       result, \
                       _SHR_ERRMSG(result))); \
            return result; \
        } \
    }
#define MIRROR_TABLE_LOCK_TAKE \
    /* claim the lock */ \
    LOG_DEBUG(BSL_LS_BCM_MIRROR, \
              (BSL_META_U(unit, \
                          "claim mirror table lock\n"))); \
    if (0 != sal_mutex_take(_mirror_glob[unit]->tableLock, \
                            sal_mutex_FOREVER)) { \
        /* something went wrong claiming the lock */ \
        LOG_ERROR(BSL_LS_BCM_MIRROR, \
                  (BSL_META_U(unit, \
                              "unable to claim mirror table lock\n"))); \
        return BCM_E_INTERNAL; \
    }
#define MIRROR_TABLE_LOCK_GIVE \
    /* release the lock */ \
    LOG_DEBUG(BSL_LS_BCM_MIRROR, \
              (BSL_META_U(unit, \
                          "release mirror table lock\n"))); \
    if (0 != sal_mutex_give(_mirror_glob[unit]->tableLock)) { \
        /* something went wrong releasing the lock */ \
        LOG_ERROR(BSL_LS_BCM_MIRROR, \
                  (BSL_META_U(unit, \
                              "unable to release" \
                               " mirror table lock\n"))); \
        return BCM_E_INTERNAL; \
    }

/*
 *  Global variables for this module
 */
_caladan3_mirror_glob_t *_mirror_glob[BCM_MAX_NUM_UNITS];
volatile sal_mutex_t _caladan3_mirror_glob_lock = NULL;

/*
 *  Forward declarations
 */
int bcm_caladan3_mirror_init(int unit);

/*
 *   Function
 *      _bcm_caladan3_modPort_to_ftEntry
 *   Purpose
 *      Look up the correct FT entry from a destModule,destPort pair.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const bcm_module_t destModId = the destination module ID
 *      (in) const bcm_port_t destPort = the destination port ID
 *      (out) uint32 *destFte = where to put the destination FT entry ID
 *      (out) uint32 *destQueue = where to put the destination queue ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 */
int
_bcm_caladan3_modPort_to_ftEntry(const int unit,
                               const bcm_module_t destModId,
                               const bcm_port_t destPort,
                               uint32 *destFte,
                               uint32 *destQueue)
{
    int destNode;                                   /* remote node ID */
    int fabUnit;                                    /* fabric unit */
    int fabPort;                                    /* fabric port */
    int result;                                     /* result to caller */

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%d,*)\n"),
               unit, destModId, destPort));

    /* make sure we parameters are valid */
    if (!SOC_SBX_PORT_ADDRESSABLE(unit, destPort)) {
        /* destination port is invalid */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalid target port %d\n"),
                   destPort));
        return BCM_E_PARAM;
    }

    if (!SOC_SBX_MODID_ADDRESSABLE(unit, destModId)) {
        /* destination module invalid */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "inaccessible target module %d\n"),
                   destModId));
        return BCM_E_PARAM;
    }

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "get destination information\n")));
    result = soc_sbx_node_port_get(unit,
                                   destModId,
                                   destPort,
                                   &fabUnit,
                                   &destNode,
                                   &fabPort);
    if (BCM_E_NONE != result) {
        /* failed to get target information */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "soc_sbx_node_port_get(%d,%d,%d,&(%d),"
                               "&(%d),&(%d)) returned %d (%s)\n"),
                   unit, destModId, destPort, fabUnit, destNode, fabPort, 
                   result, _SHR_ERRMSG(result)));

    } else { /* if (BCM_E_NONE != result) */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "fabUnit %d, fabNode %d, fabPort %d\n"),
                   fabUnit, destNode, fabPort));

        /* check destination node */
        if (!SOC_SBX_NODE_ADDRESSABLE(unit, destNode)) {
            /* inaccessible destination node */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "target node %d inaccessible\n"),
                       destNode));
            result = BCM_E_BADID;
        }
    } /* if (BCM_E_NONE != result) */

    if (BCM_E_NONE == result) {
        if (destFte) {
            /* provide the port's FT entry ID */
            *destFte = SOC_SBX_PORT_FTE(unit, destNode, fabPort);
        }
        if (destQueue) {
            /* provide the port's queue number */
            *destQueue = SOC_SBX_NODE_PORT_TO_QID(unit, destNode, fabPort,
                                                  NUM_COS(unit));
        }
    }

    return result;
}


static int
_bcm_caladan3_egr_mirror_set(const int unit,
                           const unsigned int mirrorId,
                           const bcm_gport_t destGport)
{
    uint32 ftIndex;                                 /* working FT entry */
    uint32 portQueue;                               /* working port queue */
    int myModId;                                    /* working local ModId */
    int result;                                     /* result to caller */
    soc_sbx_g3p1_ft_t ft;                           /* working ft entry */
    soc_sbx_g3p1_emirror_t emirror;                 /* egress mirror structure */
    bcm_module_t destModId;
    bcm_port_t destPort;
    int dqueue = 0;

    destModId = BCM_GPORT_MODPORT_MODID_GET(destGport);
    destPort = BCM_GPORT_MODPORT_PORT_GET(destGport);

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%d,%d)\n"),
               unit, mirrorId, destModId, destPort));

    /* start with a clean entry */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "initialise the mirror data\n")));

    /* handle appropriate mirror target type */
    /* mirror type is port */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "mirror is destined to mod %d port %d\n"),
               destModId, destPort));

    /* get the target FT information (also checks some parameters */
    result = _bcm_caladan3_modPort_to_ftEntry(unit,
                                            destModId,
                                            destPort,
                                            &ftIndex,
                                            &portQueue);
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_CALADAN3(unit)){
            /* only local mirroring on CALADAN3 */
            /* get my module ID */
            result = bcm_stk_my_modid_get(unit, &myModId);
            if ((BCM_E_NONE == result) && (destModId != myModId)) {
                /* not local; can't do it */
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "can't egress mirror from unit %d"
                                       " to mod %d port %d: not local\n"),
                           unit,
                           destModId,
                           destPort));
                result = BCM_E_PARAM;
            } else if (BCM_E_NONE != result) {
                /* error getting my module ID */
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "unable to get unit %d modID\n"),
                           unit));
            }else{
                soc_sbx_g3p1_emirror_t_init(&emirror);
                emirror.local = 1; /* only local for CALADAN3 */

		result = soc_sbx_caladan3_get_dqueue_from_port(unit, destPort, 0 /* direction */, 
							       0 /* cos */, &dqueue);
		if (BCM_FAILURE(result)) {
		  /* something went wrong */
		  LOG_ERROR(BSL_LS_BCM_MIRROR,
		            (BSL_META_U(unit,
		                        "failed to get SWS dqueue\n")));
		}
                emirror.dqueue = dqueue;

                result = soc_sbx_g3p1_emirror_set(unit, mirrorId, &emirror);
                if (BCM_FAILURE(result)) {
                    /* something went wrong */
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "soc_sbx_g3p1_emirror_set(%08X,"
                                           "%d,{%08X,%08X,%08X,%08X}) returned"
                                           "%d (%s)\n"),
                               unit, mirrorId, emirror.local, emirror.dqueue,
                               emirror.qid, emirror.oi,
                               result, _SHR_ERRMSG(result)));
                }
            }

        } else { 
            result = soc_sbx_g3p1_ft_get(unit, ftIndex, &ft);

            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "soc_sbx_g3p1_ft_get(%08X,"
                                       "%d,) returned"
                                       " -> %d (%s)\n"),
                           unit, ftIndex,
                           result, _SHR_ERRMSG(result)));

            } else{
                soc_sbx_g3p1_emirror_t_init(&emirror);
                emirror.local = 0; /* otherwise non-local */

                emirror.dqueue = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;

                emirror.qid = portQueue;
                emirror.oi = ft.oi;

                result = soc_sbx_g3p1_emirror_set(unit, mirrorId, &emirror);
                if (BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "soc_sbx_g3p1_emirror_set(%08X,"
                                           "%d,{%08X,%08X,%08X,%08X}) returned"
                                           " %d (%s)\n"),
                               unit, mirrorId, emirror.local,
                               emirror.dqueue, emirror.qid, emirror.oi,
                               result, _SHR_ERRMSG(result)));
                }
            }
        }
    }

    return result;
}

/*
 *   Function
 *      _bcm_caladan3_ingr_mirror_set
 *   Purpose
 *      Fill in the data for an ingress mirror.
 *   Parameters 
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *      (in) const bcm_gport_t destGport = the destination 
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 */
static int
_bcm_caladan3_ingr_mirror_set(const int unit,
                            const unsigned int mirrorId,
                            const bcm_gport_t destGport)
{
    soc_sbx_g3p1_mirror_t ingrMirror;               /* working ingress mirror */
    soc_sbx_g3p1_ft_t fteMirror;                    /* working FTE for mirror */
    uint32 portFte;                                 /* standard port FTE */
    uint32 portQueue;                               /* standard port queue */
    int result;                                     /* result to caller */
    bcm_module_t destModId;
    bcm_port_t destPort;

    destModId = BCM_GPORT_MODPORT_MODID_GET(destGport);
    destPort = BCM_GPORT_MODPORT_PORT_GET(destGport);

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%d,%d)\n"),
               unit, mirrorId, destModId, destPort));

    /* start with a clean entry */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "initialise the mirror data\n")));
    soc_sbx_g3p1_mirror_t_init(&ingrMirror);

    /* mirror target is a port, need to find the FTE for it */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "mirror is destined to mod %d port %d\n"),
               destModId,
               destPort));

    /* fill in the port FT information */
    result = _bcm_caladan3_modPort_to_ftEntry(unit,
                                            destModId,
                                            destPort,
                                            &portFte,
                                            &portQueue);
    if (BCM_E_NONE == result) {
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "using modPort FT entry %08X\n"),
                   portFte));
        /* read the port FT entry to get the OI */
        result = soc_sbx_g3p1_ft_get(unit, portFte, &fteMirror);
        if (BCM_FAILURE(result)) {
            /* something went wrong */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unit %d unable to read mod %d"
                                   " port %d FT entry (0x%08X):"
                                   "  -> %d (%s)\n"),
                       unit, destModId, destPort, portFte,
                       result, _SHR_ERRMSG(result)));
        }
        ingrMirror.oi = fteMirror.oi;
        ingrMirror.qid = fteMirror.qid;
    } /* if (BCM_E_NONE == result) */

    if (SOC_E_NONE == result) {
        /* set up the mirror information */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "configure the mirror -> OI[0x%08X], QID[0x%08X]\n"),
                   ingrMirror.oi, ingrMirror.qid));

        /* commit the mirror information */
        result = soc_sbx_g3p1_mirror_set(unit, mirrorId, &ingrMirror);
        if (BCM_FAILURE(result)) {
            /* something went wrong committing the mirror data */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_mirror_set(%08X,%d,&{0x%08X})"
                                   " returned -> %d (%s)\n"),
                       unit, mirrorId, ingrMirror.oi,
                       result, _SHR_ERRMSG(result)));
        }
    } /* if (BCM_E_NONE == result) */

    return result;
}

/*
 *   Function
 *      _bcm_caladan3_mirror_resource_alloc
 *   Purpose
 *      Get a mirror resource matching the specification 
 *      Or get an index to a free entry.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const caladan3_mirror_direction_t direction = ingress or egress mirror
 *      (out) unsigned int *mirrorId = where to put the mirror number
 *      (in) const bcm_gport_t destGport = the destination gport ID
 *   Returns
 *      BCM_E_NONE if No Entry Found
 *      BCM_E_BUSY if Entry found
 *      BCM_E_RESOURCE if no resource available
 *   Notes
 *      May return the same mirror to multiple callers if they specify the same
 *      <gport> .  Will handle this internally.
 *      Will claim & release the unit lock.
 *      Never allocates a reserved mirror.
 *      This is called directly by other modules that need to allocate mirrors.
 */

int
_bcm_caladan3_mirror_resource_alloc(const int unit,
                                  _caladan3_mirror_direction_t direction,
                                  unsigned int *mirrorId,
                                  const bcm_gport_t destGport)
{
    int result;                                     /* result to caller */
    int free = -1;                                  /* working free entry ID */
    int index;                                      /* working match ID */
    _caladan3_mirror_t *mirror;
    int mirror_min, mirror_max;

    if ((direction != _caladan3_ingress_mirror) &&
       (direction != _caladan3_egress_mirror)) {
       return BCM_E_PARAM;
    }

    MIRROR_TABLE_LOCK_TAKE;

    mirror_max = _mirror_glob[unit]->state[direction].mirror_max;
    mirror_min = _mirror_glob[unit]->state[direction].mirror_min;
    /* find a free entry or a match (if there is a match) */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "search for match or free entry\n")));

    free = mirror_max;
    for (index = mirror_min; index < mirror_max; index++) {

        mirror = &(_mirror_glob[unit]->state[direction].mirrors[index]);
        if (mirror->flags & C3_MIRROR_VALID) {
            if (destGport == mirror->dest_gport) {
                /* this entry is a match */
                break;
            }
        } else {
            free = index;
        }
    } 

    if (mirror_max > index) {
        result = BCM_E_EXISTS;
    } else if (mirror_max > free) {
        /* we didn't find a match, but we did find a free entry */
        index = free;
        result = BCM_E_NONE;
    } else {
        /* no match and no free entries */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "no match and no free entries\n")));
        result = BCM_E_RESOURCE;
    }

    *mirrorId = index;
    MIRROR_TABLE_LOCK_GIVE;
    return result;
}

int 
_bcm_caladan3_ingr_mirror_tunnel_set (int unit, unsigned int mirrorId, 
                                       bcm_gport_t destGport)
{

    soc_sbx_g3p1_mirror_t ing_mirror;             /* working ingress mirror */
    soc_sbx_g3p1_ft_t fte;                        /* working FTE for mirror */
    uint32        fti; 
    int result;

    fti = BCM_GPORT_MPLS_PORT_ID_GET(destGport);

    /* read the port FT entry to get the OI */
    result = soc_sbx_g3p1_ft_get(unit, fti, &fte);

    if (BCM_FAILURE(result)) {
        /* something went wrong */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit %d unable to read FT entry (%08X):"
                               "  -> %d (%s)\n"),
                   unit, fti,
                   result, _SHR_ERRMSG(result)));
    } else {

        /* start with a clean entry */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "initialise the mirror data\n")));
        soc_sbx_g3p1_mirror_t_init(&ing_mirror);

        /* mirror target is a port, need to find the FTE for it */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "mirror is destined to port %x\n"),
                   destGport));

        ing_mirror.oi = fte.oi;
        ing_mirror.qid = fte.qid;

        /* set up the mirror information */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "configure the mirror -> OI[0x%08X], QID[0x%08X]\n"),
                   ing_mirror.oi, ing_mirror.qid));

        /* commit the mirror information */
        result = soc_sbx_g3p1_mirror_set(unit, mirrorId, &ing_mirror);
        if (BCM_FAILURE(result)) {
            /* something went wrong committing the mirror data */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_mirror_set(%08X,%d,&{%08X})"
                                  " returned -> %d (%s)\n"),
                       unit, mirrorId, ing_mirror.oi,
                       result, _SHR_ERRMSG(result)));
        }
    }       
    return result;
}

int 
_bcm_caladan3_egr_mirror_tunnel_set (int unit, unsigned int mirrorId, 
                                          bcm_gport_t destGport) 
{
    soc_sbx_g3p1_ft_t fte;                        /* working FTE for mirror */
    soc_sbx_g3p1_emirror_t egr_mirror;            /* egress mirror structure */
    uint32        fti; 
    bcm_gport_t    dest_port;
    int           result;

    fti = BCM_GPORT_MPLS_PORT_ID_GET(destGport);

    /* read the port FT entry to get the OI */
    result = soc_sbx_g3p1_ft_get(unit, fti, &fte);

    if (BCM_FAILURE(result)) {
            /* something went wrong */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unit %d unable to read FT entry (%08X):"
                                   "  -> %d (%s)\n"),
                       unit, fti,
                       result, _SHR_ERRMSG(result)));
    } else {

        result = _bcm_caladan3_mirror_qid_translate(unit, fte.qid, &dest_port);

        if (BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "Failed converting Qid to Port: %s\n"),
                       bcm_errmsg(result)));
        } else {
            soc_sbx_g3p1_emirror_t_init(&egr_mirror);
            egr_mirror.local = 0; 
            egr_mirror.dqueue = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
            egr_mirror.qid = fte.qid;
            egr_mirror.oi = fte.oi;

            result = soc_sbx_g3p1_emirror_set(unit, mirrorId, &egr_mirror);
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "soc_sbx_g3p1_emirror_set(%08X,"
                                       "%d,{%08X,%08X,%08X,%08X}) returned"
                                       " %d (%s)\n"),
                           unit, mirrorId, egr_mirror.local,
                           egr_mirror.dqueue, egr_mirror.qid, egr_mirror.oi,
                           result, _SHR_ERRMSG(result)));
            }
        }
    }
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_mirror_alloc
 *   Purpose
 *      Get a mirror that fits the <destModId,destPort> or a <tunnelPort> spec.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const caladan3_mirror_direction_t direction = ingress or egress mirror
 *      (out) unsigned int *mirrorId = where to put the mirror number
 *      (in) const bcm_gport_t destGport = the destination gport
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      May return the same mirror to multiple callers if they specify the same
 *      <destModId,destPort> or <TunnelPort> .  Will handle this internally.
 *      Will claim & release the unit lock.
 *      Never allocates a reserved mirror.
 *      This is called directly by other modules that need to allocate mirrors.
 */
int
_bcm_caladan3_mirror_alloc(const int unit,
                         _caladan3_mirror_direction_t direction,
                         unsigned int *mirrorId,
                         const bcm_gport_t destGport)
{
    
    int result;                                     /* result to caller */
    _caladan3_mirror_t *mirror = NULL;
    int tunnel_mode = 0;
    bcm_module_t destModId = -1;
    bcm_port_t   destPort = -1;
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};

    MIRROR_UNIT_INIT_CHECK;

    result = _bcm_caladan3_mirror_resource_alloc(unit, direction, mirrorId, destGport);

    if (BCM_GPORT_IS_MPLS_PORT(destGport)) {
        tunnel_mode = 1;
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "(%d,*,%x)\n"),
                   unit, destGport));
    } else {
        destModId = BCM_GPORT_MODPORT_MODID_GET(destGport);
        destPort = BCM_GPORT_MODPORT_PORT_GET(destGport);
        tunnel_mode = 0;
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "(%d,*,%d,%d)\n"),
                   unit, destModId, destPort));
    }

    MIRROR_TABLE_LOCK_TAKE;

    if (BCM_E_NONE == result || BCM_E_EXISTS == result) {
        /* set up the selected free entry */
#ifdef BROADCOM_DEBUG
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "claim and fill %s mirror %d\n"),
                   dir_str[direction], *mirrorId));
#endif /* BROADCOM_DEBUG */

        mirror = &(_mirror_glob[unit]->state[direction].mirrors[*mirrorId]);
        if (tunnel_mode) {
            if (direction == _caladan3_ingress_mirror) {
                result = _bcm_caladan3_ingr_mirror_tunnel_set(unit, *mirrorId, destGport);
            } else {
                result = _bcm_caladan3_egr_mirror_tunnel_set(unit, *mirrorId, destGport);
            }
        } else {
            if (direction == _caladan3_ingress_mirror) {
                result = _bcm_caladan3_ingr_mirror_set(unit, *mirrorId, destGport);
            } else {
                result = _bcm_caladan3_egr_mirror_set(unit, *mirrorId, destGport);
            }
        }

        if (BCM_E_NONE == result) {
            /* success! */
            mirror->refCount++;
            mirror->dest_gport = destGport;
            if (tunnel_mode) {
                mirror->flags |= C3_MIRROR_VALID | C3_MIRROR_TUNNEL;
            } else {
                mirror->flags |= C3_MIRROR_VALID;
            }
        } else {
            /* something went wrong */
            if (tunnel_mode) {
#ifdef BROADCOM_DEBUG
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "mirror_%s_tunnel_set(%d,%d,%x)"
                                       " returned %d (%s)\n"),
                           dir_str[direction], unit, *mirrorId, destGport,
                           result, _SHR_ERRMSG(result)));
#endif /* BROADCOM_DEBUG */
            } else {
#ifdef BROADCOM_DEBUG
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "mirror_%s_set(%d,%d,%d,%d)"
                                       " returned %d (%s)\n"),
                           dir_str[direction], unit, *mirrorId, destModId, destPort,
                           result, _SHR_ERRMSG(result)));
#endif /* BROADCOM_DEBUG */
            }
        }
    }
    
    MIRROR_TABLE_LOCK_GIVE;

    return result;
}


/*
 *   Function
 *      _bcm_caladan3_mirror_free
 *   Purpose
 *      Release a mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const caladan3_mirror_direction_t direction = ingress or egress mirror
 *      (in) const unsigned int mirrorId = the mirror number
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      This will correctly handle the case where multiple users have a mirror
 *      that is being freed.
 *      Will claim & release the unit lock.
 *      Will refuse to free reserved mirrors.
 *      This is called directly by other modules that need to allocate mirrors.
 */
int
_bcm_caladan3_mirror_free(const int unit,
                        _caladan3_mirror_direction_t direction,
                        const unsigned int mirrorId)
{
    int result = BCM_E_NONE;                        /* result to caller */
    _caladan3_mirror_t *mirror;
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};

#ifdef BROADCOM_DEBUG
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d)\n"),
               direction, mirrorId));
#endif /* BROADCOM_DEBUG */

    if (direction >= _caladan3_mirror_direction_max) {
        return BCM_E_INTERNAL;
    }

    MIRROR_UNIT_INIT_CHECK;

    if ((mirrorId <  _mirror_glob[unit]->state[direction].mirror_min) || 
        (mirrorId >= _mirror_glob[unit]->state[direction].mirror_max)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "mirror ID %d is not valid\n"),
                   mirrorId));
        return BCM_E_PARAM;
    }

    MIRROR_TABLE_LOCK_TAKE;

    mirror = &(_mirror_glob[unit]->state[direction].mirrors[mirrorId]);

    /* make sure the entry in question is in use */
    if (0 == (mirror->flags & C3_MIRROR_VALID)) {
        /* this entry isn't allocated */
#ifdef BROADCOM_DEBUG
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "%s mirror %d is not in use\n"),
                   dir_str[direction], mirrorId));
#endif /* BROADCOM_DEBUG */
        result = BCM_E_NOT_FOUND;

    } else if (0 == mirror->refCount) {
        /* something is corrupt here */
#ifdef BROADCOM_DEBUG
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "%s mirror %d in use but not referenced\n"),
                   dir_str[direction], mirrorId));
#endif /* BROADCOM_DEBUG */
        mirror->flags = 0;
        mirror->dest_gport = -1;
        result = BCM_E_INTERNAL;
    } else {
        /* the entry is in use; decrement use count */
#ifdef BROADCOM_DEBUG
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "decrement use count for %s "
                               "mirror ID %d\n"),
                   dir_str[direction], mirrorId));
#endif /* BROADCOM_DEBUG */
        mirror->refCount--;
        if (0 == mirror->refCount) {
            /* the entry is no longer in use; free it */
#ifdef BROADCOM_DEBUG
            LOG_DEBUG(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "%s mirror ID %d no longer"
                                   " referenced; freeing it\n"),
                       dir_str[direction], mirrorId));
#endif /* BROADCOM_DEBUG */
            result = BCM_E_NONE;
            mirror->dest_gport = -1;
            mirror->flags = 0;
        } else {
            /* okay, still has users, so we're done after decr use count */
            result = BCM_E_NONE;
        }
    }

    MIRROR_TABLE_LOCK_GIVE;

    return result;
}

/*
 *   Function
 *      _bcm_caladan3_mirror_get
 *   Purpose
 *      Get the data for a mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *      (in) const caladan3_mirror_direction_t direction = ingress or egress mirror
 *      (out) bcm_module_t *destModId = where to put destination module ID
 *      (out) bcm_port_t *destPort = where to put destination port ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 *      This is called directly by other modules that need to allocate mirrors.
 *      It is possible to have invalid destModId and destPort (both -1) when
 *      reading FTE type, but it is also possible to have valid in this case.
 *      In either PORT or FTE type, the FTE will be valid.
 *      We don't use the type here yet, but may in the future.
 */
int
_bcm_caladan3_mirror_get(const int unit,
                       const unsigned int mirrorId,
                       const _caladan3_mirror_direction_t direction,
                       bcm_gport_t *destPort)
{
    int result;                                     /* result to caller */
    _caladan3_mirror_t *mirror;                         /* pointer to mirror */
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%d,*,*,*)\n"),
               unit, direction, mirrorId));

    if (direction >= _caladan3_mirror_direction_max) {
        return BCM_E_INTERNAL;
    }

    MIRROR_UNIT_INIT_CHECK;

    /* make sure the mirror ID is valid */
    if ((mirrorId <  _mirror_glob[unit]->state[direction].mirror_min) || 
        (mirrorId >= _mirror_glob[unit]->state[direction].mirror_max)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "mirror ID %d is not valid\n"),
                   mirrorId));
        return BCM_E_PARAM;
    }

    MIRROR_TABLE_LOCK_TAKE;

    mirror = &(_mirror_glob[unit]->state[direction].mirrors[mirrorId]);

    /* get the data */
    if (mirror->flags & C3_MIRROR_VALID) {
        /* the mirror is valid; fill in the data */
#ifdef BROADCOM_DEBUG
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "get %s mirror information\n"), 
                   dir_str[direction]));
#endif /* BROADCOM_DEBUG */
	if (destPort) {
            *destPort = mirror->dest_gport;
        }
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "dest gport = %d\n"),
                   mirror->dest_gport));
        result = BCM_E_NONE;
    } else {
        /* the mirror is not valid */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "mirror %d is not in use\n"),
                   mirrorId));
        result = BCM_E_NOT_FOUND;
    }

    MIRROR_TABLE_LOCK_GIVE;

    return result;
}

/*
 *  Exported functions for other caladan3 module (field) usage.
 */

int
_bcm_caladan3_egr_mirror_get(const int unit,
                           const unsigned int mirrorId,
                           bcm_gport_t *destGport)
{
    return _bcm_caladan3_mirror_get(unit, mirrorId, _caladan3_egress_mirror, destGport);
}

int
_bcm_caladan3_ingr_mirror_get(const int unit,
                            const unsigned int mirrorId,
                            bcm_gport_t *destGport)
{
    return _bcm_caladan3_mirror_get(unit, mirrorId, _caladan3_ingress_mirror, destGport);
}

int
_bcm_caladan3_egr_mirror_alloc(const int unit,
                             unsigned int *mirrorId,
                             const bcm_gport_t destGport)
{
    return _bcm_caladan3_mirror_alloc(unit, _caladan3_egress_mirror, mirrorId, destGport);
}

int
_bcm_caladan3_ingr_mirror_alloc(const int unit,
                              unsigned int *mirrorId,
                              const bcm_gport_t destGport)
{
    return _bcm_caladan3_mirror_alloc(unit, _caladan3_ingress_mirror, mirrorId, destGport);
}


int
_bcm_caladan3_egr_mirror_free(const int unit,
                            const unsigned int mirrorId)
{
    return _bcm_caladan3_mirror_free(unit, _caladan3_egress_mirror, mirrorId);
}

int
_bcm_caladan3_ingr_mirror_free(const int unit,
                             const unsigned int mirrorId)
{
    return _bcm_caladan3_mirror_free(unit, _caladan3_ingress_mirror, mirrorId);
}


static int
_bcm_caladan3_port_ingr_mirror_get(const int unit,
                                 const bcm_port_t port,
                                 unsigned int *mirrorId)
{
    soc_sbx_g3p1_lp_t lp;                           /* working lp ent */
    int result;                                     /* result to caller */

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%08X)\n"),
               unit,
               port,
               (unsigned int)mirrorId));

    result = soc_sbx_g3p1_lp_get(unit, port, &lp);
    if (BCM_SUCCESS(result)) {
        *mirrorId = lp.mirror;
        result = BCM_E_NONE;
    } else {
        /* failed to get the entry */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "soc_sbx_g3p1_lp_get(*,%d,*) returned"
                               " -> %d (%s)\n"),
                   port, result, _SHR_ERRMSG(result)));
    }

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "ingress mirror %d\n"),
               *mirrorId));

    return result;
}

/*
 *  _bcm_caladan3_port_ingr_mirror_set
 *
 *  Routine sets up the port for ingress mirroring.
 */
static int
_bcm_caladan3_port_ingr_mirror_set(const int unit,
                                 const bcm_port_t port,
                                 unsigned int mirrorId)
{
    soc_sbx_g3p1_lp_t lp;                           /* working lp ent */
    soc_sbx_g3p1_pvv2edata_t pvv2e;
    int iterPort, iterIvid, iterOvid;
    int rc;
    int result;                                     /* result to caller */
    uint32 lpi = 0;

    /* for untagged case - vid is set to 0 in g3p1 */
    soc_sbx_g3p1_pv2e_t pv2e;
    soc_sbx_g3p1_lp_t   tempLp;
    soc_sbx_g3p1_lp_t   localOldValues;

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%08X)\n"),
               unit,
               port,
               mirrorId));

    /* expect success */
    result = BCM_E_NONE;

    soc_sbx_g3p1_lp_t_init(&lp);
    lp.mirror = mirrorId;

    /* set the mirror index in all logical ports associated with this physical port */
    result = bcm_caladan3_vlan_port_lp_replicate(unit,
                                               port,
                                               BCM_CALADAN3_LP_COPY_MIRROR,
                                               NULL, /* force update */
                                               &lp);

    /* for untagged case - vid is set to 0 in g3p1 */
    /* get the pv2e's logcal port index for vid=0 */
    result = SOC_SBX_G3P1_PV2E_GET(unit, port, 0, &pv2e);
    if (BCM_SUCCESS(result)) {
        lpi = pv2e.lpi;
        if (pv2e.vpws) {
            lpi += _BCM_CALADAN3_VPWS_UNI_OFFSET;
        }
        if (lpi) {
            /* got pv2e and lp's not 'default'; read logical port */
            result = soc_sbx_g3p1_lp_get(unit, lpi, &tempLp);
            if (BCM_SUCCESS(result)) {
                /* force update to new values in ALL cases if oldValues is
                 * NULL */
                memcpy(&localOldValues, &tempLp, sizeof(soc_sbx_g3p1_lp_t));
                if (localOldValues.mirror == tempLp.mirror) {
                    tempLp.mirror = lp.mirror;
                    result = soc_sbx_g3p1_lp_set(unit, lpi, &tempLp);
                }
            }
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "unable to update %d:lp[%08X]:"
                                       " %d (%s)\n"),
                           unit, lpi,
                           result, _SHR_ERRMSG(result)));
            }
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to read %d:pv2e[%d,%03X]:"
                               " %d (%s)\n"),
                   unit, port, 0,
                   result, _SHR_ERRMSG(result)));
    }

    /* set lp == port */
    result = soc_sbx_g3p1_lp_get(unit, port, &lp);
    if (BCM_FAILURE(result)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "soc_sbx_g3p1_lp_get(*,%d,*) returned"
                               " -> %d (%s)\n"),
                   port, result, _SHR_ERRMSG(result)));
        return result;
    }else{
        lp.mirror = mirrorId;
        result = soc_sbx_g3p1_lp_set(unit, port, &lp);
        if (BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_lp_set(*,%d,*) returned"
                                   " -> %d (%s)\n"),
                       port, result, _SHR_ERRMSG(result)));
            return result;
        }
    }

    /* iterate over all the <port,vid,vid> and update the lp.mirror for this port
     */
    rc = soc_sbx_g3p1_util_pvv2e_first(unit, &iterPort, &iterOvid, &iterIvid);

    while (BCM_SUCCESS(rc)) {
        if (iterPort == port) {

            result  = soc_sbx_g3p1_util_pvv2e_get(unit, port, iterOvid, iterIvid, &pvv2e);
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "soc_sbx_g3p1_pvv2e_get"
                                       "(0x%x,0x%x,0x%x>) returned"
                                       " -> %d (%s)\n"),
                           iterPort, iterIvid, iterOvid, 
                           result, _SHR_ERRMSG(result)));
                return result;
            }

            lpi = pvv2e.lpi;
            if (pvv2e.vpws) {
                lpi += _BCM_CALADAN3_VPWS_UNI_OFFSET;
            }

            /* If the logical port is non-default (non-zero), then update
             * the sid for trunk membership
             */
            if (lpi != 0) {
                result = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
                if (BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "soc_sbx_g3p1_lp_get(0x%x) returned"
                                           " -> %d (%s)\n"),
                               lpi, result, _SHR_ERRMSG(result)));
                    return result;
                }
                lp.mirror = mirrorId;

                result = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
                if (BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "soc_sbx_g3p1_lp_set(0x%x) returned"
                                           " -> %d (%s)\n"),
                               lpi, result, _SHR_ERRMSG(result)));
                    return result;
                }
            }
        }

        rc = soc_sbx_g3p1_util_pvv2e_next(unit, iterPort, iterOvid, iterIvid,
                                       &iterPort, &iterOvid, &iterIvid);
    }

    return result;

}

/*
 *  _bcm_caladan3_port_egr_mirror_get
 *
 *  Returns the mirror configured on the given port
 */
static int
_bcm_caladan3_port_egr_mirror_get(int unit,
                                bcm_port_t port,
                                unsigned int *mirrorId)
{
    soc_sbx_g3p1_ep2e_t ep2e;     /* working evp2e ent */
    int result;                   /* result to caller */

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%08X)\n"),
               unit,
               port,
               (unsigned int)mirrorId));

    result = BCM_E_NONE;

    /* get the mirror ID form the evp2e table */
    result = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);

    if (BCM_FAILURE(result)) {
        /* failed to get the entry */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "soc_sbx_g3p1_ep2e_get"
                               "(*,%d,*,1)"
                               " returned -> %d (%s)\n"),
                   port, result, _SHR_ERRMSG(result)));
    }

    *mirrorId = ep2e.mirroridx;

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "egress mirror %08X\n"),
               *mirrorId));

    if (*mirrorId == 0) {
        return BCM_E_NOT_FOUND;
    }

    return result;

}


/*
 *  _bcm_caladan3_port_egr_mirror_set
 *
 *  Configures a port for Egress mirroring
 */
static int
_bcm_caladan3_port_egr_mirror_set(const int unit,
                                const bcm_port_t port,
                                unsigned int mirrorId)
{
    soc_sbx_g3p1_ep2e_t ep2e;                       /* working evp2e ent */
    int result;                                     /* result to caller */

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d,%d,%08X)\n"),
               unit, port, mirrorId));

    /* set ep2e mirror */
    result = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);

    if (BCM_SUCCESS(result)) {
        /* got the entry; update it */
        ep2e.mirroridx = mirrorId;

        result = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);
        if (BCM_FAILURE(result)) {
            /* failed to set the entry */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_ep2e_set"
                                   "(*,%d,*,1)"
                                   " returned  -> %d (%s)\n"),
                       port, result, _SHR_ERRMSG(result)));
        }
    } else {
        /* failed to get the entry */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "soc_sbx_g3p1_ep2e_get"
                               "(*,%d,*,1)"
                               " returned  -> %d (%s)\n"),
                   port, result, _SHR_ERRMSG(result)));
    }

    return result;


}


int
bcm_caladan3_mirror_detach(int unit)
{
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d)\n"),
               unit));

    if (_mirror_glob[unit]) {
        sal_mutex_destroy(_mirror_glob[unit]->tableLock);
        _mirror_glob[unit]->tableLock = NULL;
        sal_free(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors);
        sal_free(_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors);
        sal_free(_mirror_glob[unit]);
        _mirror_glob[unit] = NULL;
    } 
    return BCM_E_NONE;
}


/*
 *   Function
 *      _bcm_caladan3_mirror_qid_translate
 *   Purpose
 *      Translate a queue id to a module and port
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) int qid = queue id to translate
 *      (out) int dest_mod = destination mod of queue
 *      (out) int dest_port = destination port of queue
 *   Returns
 *      BCM_E_*
 *   Notes
 *      Must be called with a valid mod-map.
 */
int
_bcm_caladan3_mirror_qid_translate(int unit, uint32 qid,
                                 int *switch_gport)
{
    bcm_gport_t fab_gport;
    int fab_node, fab_port;
    int rv;

    rv = map_qid_to_np(unit, qid, &fab_node, &fab_port, NUM_COS(unit));
    fab_node += SBX_QE_BASE_MODID;

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    BCM_GPORT_MODPORT_SET(fab_gport, fab_node, fab_port);
    rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, fab_gport,
                                                switch_gport);
    
    return rv;
}



#if BCM_CALADAN3_MIRROR_TABLE_RECOVER
/*
 *   Function
 *      _bcm_caladan3_mirror_recover_ingress_port
 *   Purpose
 *      Recover internal ingress mirror state directly from hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) int port = ingress mirror port 
 *      (out) int mirror_id = mirror id for port, -1 if none
 *      (out) int dest_mod = destination mod of mirror
 *      (out) int dest_port=  destination port of mirror
 *   Returns
 *      BCM_E_*
 *   Notes
 */
int
_bcm_caladan3_mirror_recover_ingress_port(int unit, int port, int *mirror_id,
                                        int *switch_gport)
{
    int rv;
    soc_sbx_g3p1_lp_t lp;
    soc_sbx_g3p1_mirror_t imir;
    
    rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "failed to read lp[%d]: %s\n"),
                   port, bcm_errmsg(rv)));
        return rv;
    }

    *mirror_id = -1;
    if (lp.mirror) {
        rv = soc_sbx_g3p1_mirror_get(unit, lp.mirror, &imir);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "failed to read mirror[%d]: %s\n"),
                       lp.mirror, bcm_errmsg(rv)));
            return rv;
        }

        rv = _bcm_caladan3_mirror_qid_translate(unit, imir.qid, 
                                              switch_gport);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "failed to convert qid 0x%08x : %s\n"),
                       imir.qid, bcm_errmsg(rv)));
            return rv;
        }
        *mirror_id = lp.mirror;

    } else {
        *switch_gport = *mirror_id = -1;
    }

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_mirror_recover_egress_port
 *   Purpose
 *      Recover egress mirror information directly from hw
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) int port = mirror port
 *      (out) int mirror_id = recovered mirror id, -1 if none
 *      (out) int dest_mod = mirror destination module
 *      (out) int dest_port = mirror destination port
 *   Returns
 *      BCM_E_*
 *   Notes
 */
int
_bcm_caladan3_mirror_recover_egress_port(int unit, int port, int *mirror_id,
                                       int *dest_gport)
{
    int rv;
    soc_sbx_g3p1_ep2e_t ep;
    soc_sbx_g3p1_emirror_t emir;
    bcm_module_t local_mod;

    rv = bcm_stk_my_modid_get(unit, &local_mod);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    rv = soc_sbx_g3p1_ep2e_get(unit, port, &ep);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "failed to read ep[%d]: %s\n"),
                   port, bcm_errmsg(rv)));                
        return rv;
    } 

    if (ep.mirroridx) {
        rv = soc_sbx_g3p1_emirror_get(unit, ep.mirroridx, &emir);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "failed to read emirror[%d]: %s\n"),
                       ep.mirroridx, bcm_errmsg(rv)));
            return rv;
        }

        if (emir.local == 1) {
            BCM_GPORT_MODPORT_SET(*dest_gport, local_mod, emir.dqueue - SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE);
        } else {
            rv = _bcm_caladan3_mirror_qid_translate(unit, emir.qid,
                                                  dest_gport);
        }

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "failed to convert qid 0x%08x : %s\n"),
                       emir.qid, bcm_errmsg(rv)));
            return rv;
        } 

        *mirror_id = ep.mirroridx;
    } else {
        *mirror_id = *dest_gport = -1;
    }

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_mirror_recover
 *   Purpose
 *      Recover internal mirror state
 *   Parameters
 *      (in) const int unit = the unit number
 *   Returns
 *      BCM_E_*
 *   Notes
 *      Must be called with a valid mod-map.  Either after the application
 *    reconfigures it, or during init with level2 warm boot available.
 */
int
_bcm_caladan3_mirror_recover(int unit)
{

#ifdef BCM_WARM_BOOT_SUPPORT
    int rv = BCM_E_NONE;
    int port, mirror_id;
    int dest_gport;
    int e_min, e_max, e_index;
    int i_min, i_max, index;
    _caladan3_mirror_direction_t direction;
    _caladan3_mirror_state_t *state;
    _caladan3_mirror_t *mirror, *emirror;
    int dest_port = -1;
    int dest_mod = -1;
    soc_sbx_g3p1_emirror_t soc_emirror;
    soc_sbx_g3p1_mirror_t  soc_mirror;
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "Recovering...\n")));

    /* Iterate over all port, examine both ingress and egress physical ports
     * for configured mirrors.  If found, reconstruct the internal state.
     * Logical ports ARE NOT scanned - since mirrors are only applied to
     * physical ports, it is sufficient to limit the scan to physical ports.
     */
    BCM_PBMP_ITER(PBMP_PORT_ALL(unit), port) {

        for (direction = 0; 
             direction < _caladan3_mirror_direction_max;
             direction++) {

            state = &_mirror_glob[unit]->state[direction];

            if (direction == _caladan3_ingress_mirror) {
                rv = _bcm_caladan3_mirror_recover_ingress_port(unit, port,
                                                             &mirror_id,
                                                             &dest_gport);

            } else if (direction == _caladan3_egress_mirror) {
                rv = _bcm_caladan3_mirror_recover_egress_port(unit, port,
                                                            &mirror_id,
                                                            &dest_gport);               
            }

            if (BCM_FAILURE(rv)) {
#ifdef BROADCOM_DEBUG
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "failed to recover %s mirror from "
                                       "port %d: %s\n"),
                           dir_str[direction], port, bcm_errmsg(rv)));
#endif /* BROADCOM_DEBUG */
                return rv;
            }
            
            if (mirror_id >= state->mirror_min) {
                mirror = &state->mirrors[mirror_id];
                mirror->dest_gport  = dest_gport;
                if (BCM_GPORT_IS_MPLS_PORT(dest_gport)) {
                    mirror->flags     = C3_MIRROR_VALID | C3_MIRROR_TUNNEL;
                } else {
                    mirror->flags     = C3_MIRROR_VALID;
                }

                if ((direction == _caladan3_egress_mirror) || 
                    ((direction == _caladan3_ingress_mirror) && (mirror->refCount))) {
                    mirror->refCount++;
                } else {
                    mirror->refCount = 2;
                }

                if (mirror->flags & C3_MIRROR_TUNNEL) {
#ifdef BROADCOM_DEBUG
                    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                                (BSL_META_U(unit,
                                            "%s MirrorId=%d refCount=%d "
                                             "srcPort=%d Destination=%x\n"),
                                 dir_str[direction], mirror_id, mirror->refCount, 
                                 port, dest_gport));
#endif /* BROADCOM_DEBUG */
                } else {
                    dest_mod = BCM_GPORT_MODPORT_MODID_GET(dest_gport);
                    dest_port = BCM_GPORT_MODPORT_PORT_GET(dest_gport);
#ifdef BROADCOM_DEBUG
                    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                                (BSL_META_U(unit,
                                            "%s MirrorId=%d refCount=%d "
                                             "srcPort=%d Destination=%d:%d\n"),
                                 dir_str[direction], mirror_id, mirror->refCount, 
                                 port, dest_mod, dest_port));
#endif /* BROADCOM_DEBUG */
                }

            }
        }
    }

    /* Mirror_Id == ingress mirror table entry, always
     * Egress mirrors may or may not exist; however, if one exists there MUST 
     * be an associated ingress mirror entry that is matched via the SAME 
     * destination, else it is an error.
     */
    e_max = _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_max;
    e_min = _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_min;

    i_max = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max;
    i_min = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_min;

    for (e_index = e_min; e_index < e_max; e_index++) {

        mirror = NULL;
        emirror = &(_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors[e_index]);
        if ((emirror->flags & C3_MIRROR_VALID) == 0 ) {
            continue;
        }
        
        /* check the cached state first */
        for (index = i_min; index < i_max; index++) {
            mirror = &(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[index]);

            if (mirror->flags & C3_MIRROR_VALID) {
                if ((dest_gport  == mirror->dest_gport) &&
                    (mirror->flags == emirror->flags)) {
                    /* this entry is a match */
                    break;
                }
            }
        }
        
        /* if not found in cache, scan the hard tables */
        if (index >= i_max) {            
            rv = soc_sbx_g3p1_emirror_get(unit, e_index, &soc_emirror);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MIRROR,
                          (BSL_META_U(unit,
                                      "Failed to read emirror %d:%s\n"),
                           e_index, bcm_errmsg(rv)));
                return rv;
            }

            for (index = i_min; index < i_max; index++) {
                rv = soc_sbx_g3p1_mirror_get(unit, index, &soc_mirror);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_MIRROR,
                              (BSL_META_U(unit,
                                          "Failed to read mirror %d:%s\n"),
                               index, bcm_errmsg(rv)));
                    return rv;
                }
                
                if ((soc_mirror.qid == soc_emirror.qid) &&
                    (soc_mirror.oi == soc_emirror.oi))
                {
                    break;
                }
            }
        }

        if (index >= i_max) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "Failed to recover mirrors state."
                                   "  Valid egress with no "
                                   "matching ingress\n")));
            return BCM_E_FAIL;
        }


        mirror = &(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[index]);

        mirror->flags      = C3_MIRROR_VALID;
        mirror->dest_gport = emirror->dest_gport;
        mirror->refCount  += emirror->refCount + 1;
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_WARM_BOOT_SUPPORT */
}
#endif

/*
 *   Function
 *      bcm_caladan3_mirror_init
 *   Purpose
 *      Initialise mirror support.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      May be called implicitly by some functions.
 */
int
bcm_caladan3_mirror_init(int unit)
{
    sal_mutex_t tempLock;                           /* working mutex handle */
    _caladan3_mirror_glob_t *tempUnit;                  /* working unit data */
    _caladan3_mirror_t *temp_ingrMirror, *temp_egrMirror;
    int index;                                      /* working index */
    uint32 imax, emax;
    _caladan3_mirror_direction_t direction;
    int result = BCM_E_NONE;
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "(%d)\n"),
               unit));

    /* see if we already have the primary globals */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "check primary globals\n")));
    tempLock = NULL;

    if (_mirror_glob[unit] != NULL) {

        result = bcm_caladan3_mirror_detach(unit);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "mirror detach failed\n")));
            return result;
        }
    }

    if (!_caladan3_mirror_glob_lock) {
        /* don't have the primary globals */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "create temp primary lock\n")));
        tempLock = sal_mutex_create("CALADAN3 mirror primary lock");
        if (!tempLock) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unable to allocate"
                                   " temp primary lock\n")));
            return BCM_E_RESOURCE;
        }
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "take primary lock\n")));
        if (0 != sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unable to take temp"
                                   " primary lock\n")));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "set primary lock to temp lock\n")));
        _caladan3_mirror_glob_lock = tempLock;
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "clear unit data pointers\n")));
        sal_memset(&_mirror_glob, 0x00, sizeof(_mirror_glob));
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "release temp primary lock\n")));
        if (0 != sal_mutex_give(tempLock)) {
            LOG_DEBUG(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unable to release"
                                   " temp primary lock\n")));
            return BCM_E_INTERNAL;
        }
    } /* if (!_caladan3_mirror_glob_lock) */

    /* reclaim the primary lock */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "take primary lock\n")));
    if (0 != sal_mutex_take(_caladan3_mirror_glob_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }
    if (tempLock) {
        /* may need to dispose of temporary lock */
        if (_caladan3_mirror_glob_lock != tempLock) {
            LOG_DEBUG(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "init collision; dispose of"
                                   " temporary lock\n")));
            sal_mutex_destroy(tempLock);
        }
    }

    /* determine mirror counts */
    imax = soc_sbx_g3p1_mirror_table_size_get(unit);
    emax = soc_sbx_g3p1_emirror_table_size_get(unit);

    /* see if we already have the unit globals */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "check unit globals\n")));
    if (_mirror_glob[unit]) {
  
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unit globals exist; clear them\n")));

        if (_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors) {
            sal_free(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors);
        }
        if (_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors) {
            sal_free(_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors);
        }
        sal_mutex_destroy(_mirror_glob[unit]->tableLock);
        sal_free(_mirror_glob[unit]);
        _mirror_glob[unit] = NULL;
    }

    /* unit globals do not exist; create new ones */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "create new unit globals\n")));    
    tempUnit = sal_alloc(sizeof(*tempUnit), "CALADAN3 mirror unit globals");
    if (tempUnit == NULL) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "error allocating unit globals\n")));
        return BCM_E_MEMORY;
    }

    temp_ingrMirror = sal_alloc(sizeof(*temp_ingrMirror) * imax,
                                "CALADAN3 ingress mirrors");
    temp_egrMirror = sal_alloc(sizeof(*temp_egrMirror) * emax,
                               "CALADAN3 egress mirrors");


    if (temp_ingrMirror == NULL || temp_egrMirror == NULL) {
        if (temp_egrMirror) {
            sal_free(temp_egrMirror);
        }
        if (temp_ingrMirror) {
            sal_free(temp_ingrMirror);
        }
        sal_free(tempUnit);

        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "error allocating unit globals\n")));
        return BCM_E_MEMORY;
    }

    /* now fill in the unit globals */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "create new unit lock\n")));
    /* we have unit global space; build globals */
    tempLock = sal_mutex_create("CALADAN3 mirror table lock");
    if (tempLock) {
        /* we have the lock; fill in the rest */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "clear unit information\n")));
        sal_memset(tempUnit, 0x00, sizeof(*tempUnit));

        tempUnit->state[_caladan3_ingress_mirror].mirrors    = temp_ingrMirror;
        tempUnit->state[_caladan3_ingress_mirror].mirror_max = imax;
        tempUnit->state[_caladan3_ingress_mirror].mirror_min = G3P1_INGR_MIRROR_MIN;

        tempUnit->state[_caladan3_egress_mirror].mirrors     = temp_egrMirror;
        tempUnit->state[_caladan3_egress_mirror].mirror_max  = emax;
        tempUnit->state[_caladan3_egress_mirror].mirror_min  = G3P1_EGR_MIRROR_MIN;
            
        /* make sure module/port defaults are invalid */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalidate mirror targets\n")));
        for (direction=0; 
             direction < _caladan3_mirror_direction_max;
             direction++) {
            for (index = 0; 
                 index < tempUnit->state[direction].mirror_max; 
                 index++) {
                tempUnit->state[direction].mirrors[index].dest_gport = -1;
                tempUnit->state[direction].mirrors[index].flags = 0;
                tempUnit->state[direction].mirrors[index].refCount = 0;
            }
        }

        /* reserved low numbered entries */
        for (direction=0; 
             direction < _caladan3_mirror_direction_max;
             direction++) {
#ifdef BROADCOM_DEBUG
            LOG_DEBUG(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "reserve low %d %s mirrors\n"),
                       tempUnit->state[direction].mirror_min, 
                       dir_str[direction]));
#endif /* BROADCOM_DEBUG */
            index = 0;
            while (index < tempUnit->state[direction].mirror_min) {
                tempUnit->state[direction].mirrors[index].flags = 
                    C3_MIRROR_VALID;
                tempUnit->state[direction].mirrors[index].refCount = 1;
                index++;
            }
        }

        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "reserve entry zero of both ingress"
                               " and egress mirror tables\n")));
        /* commit lock to working unit descriptor */
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "create port mirror table lock\n")));
        tempUnit->tableLock = tempLock;
        _mirror_glob[unit] = tempUnit;

    } else { /* if (tempLock) */
            /* problem creating the lock; bail out */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "unable to create table lock\n")));
            result = BCM_E_RESOURCE;
    } /* if if (tempLock) */
        
    if (BCM_E_NONE != result) {

        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "error initialising unit; cleaning up\n")));

        if (tempUnit->tableLock) {
            LOG_DEBUG(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "destroying table lock\n")));
            sal_mutex_destroy(tempUnit->tableLock);
        }

        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "destroying unit data space\n")));

        sal_free(temp_ingrMirror);
        sal_free(temp_egrMirror);

        sal_memset(tempUnit, 0x00, sizeof(*tempUnit));
        sal_free(tempUnit);
    } /* if (BCM_E_NONE != result) */

    if (SOC_WARM_BOOT(unit)) {
#if BCM_CALADAN3_MIRROR_TABLE_RECOVER
        result = _bcm_caladan3_mirror_recover(unit);
#endif
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    result = bcm_caladan3_wb_mirror_state_init(unit);
#endif

    /* release the primary lock */
    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "release primary lock\n")));
    if (0 != sal_mutex_give(_caladan3_mirror_glob_lock)) {
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to release primary lock\n")));
        return BCM_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                (BSL_META_U(unit,
                            "bcm_mirror_init: unit=%d rv=%d(%s)\n"),
                 unit, result, bcm_errmsg(result)));

    return result;

}


/* Free a mirror (destination, encapsulation) pair. */
static int
_bcm_caladan3_mirror_destination_free(int unit, bcm_gport_t mirror_dest_id)
{
    unsigned int mirror_idx;
    int ref_count;
    int flags;
    _caladan3_mirror_t *mirror;
    int result = BCM_E_NONE;

    /* Get mirror information */
    mirror_idx = BCM_GPORT_MIRROR_GET(mirror_dest_id);
    mirror= &_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[mirror_idx];
    flags  = mirror->flags;
    ref_count = mirror->refCount;

    if (!(flags & C3_MIRROR_VALID)) {
        return BCM_E_NOT_FOUND;
    } 

    /* If ingress mirror count > 1, return busy */
    if (ref_count < 1) {
        return (BCM_E_NOT_FOUND);
    } else {
        result = _bcm_caladan3_ingr_mirror_free(unit, mirror_idx);
    }

    if (BCM_E_NONE != result) {
        return result;
    }

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "free mirror destination (ingress table) %08X\n"),
               mirror_idx));

    return (BCM_E_NONE);
}

static int
_bcm_caladan3_mirror_destination_create(int unit, bcm_mirror_destination_t *mirror_dest)
{
    unsigned int ing_mirror_id;
    int          result;

    if (mirror_dest->flags & BCM_MIRROR_DEST_WITH_ID) {
        /* when supported, handle BCM_MIRROR_DEST_REPLACE flag, too */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "BCM_MIRROR_DEST_WITH_ID - not supported\n")));
        return (BCM_E_UNAVAIL);
    }

    result = _bcm_caladan3_mirror_alloc(unit, _caladan3_ingress_mirror,
                                      &ing_mirror_id, mirror_dest->gport);

    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "unable to allocate new"
                               " ingress mirror entry\n")));
    } else {
        LOG_DEBUG(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "set up new mirror destination (ingress table) 0x%08X\n"),
                   ing_mirror_id));
        BCM_GPORT_MIRROR_SET(mirror_dest->mirror_dest_id, ing_mirror_id);
    }
    return (result);
}

static int
_bcm_caladan3_mirror_port_ingress_dest_add(int unit, bcm_port_t port, bcm_gport_t mirror_dest_id)
{
    unsigned int ing_mirror_id;
    int result;
    _caladan3_mirror_t *mirror;

    ing_mirror_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "add ingress mirror\n")));

    mirror = &(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[ing_mirror_id]);

    if (!(mirror->flags & C3_MIRROR_VALID)) {
        return (BCM_E_NOT_FOUND);
    }

    result = _bcm_caladan3_port_ingr_mirror_set(unit, port, ing_mirror_id);

    if (BCM_E_NONE == result) {
        mirror->refCount++;
    } else {
        /* something went wrong */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_port_ingr_mirror_set"
                               "(%d,%d,%08X) returned %d (%s)\n"),
                   unit, port, ing_mirror_id,
                   result, _SHR_ERRMSG(result)));
    }

    return (BCM_E_NONE);
}

static int
_bcm_caladan3_mirror_port_egress_dest_add(int unit, bcm_port_t port, bcm_gport_t mirror_dest_id)
{
    unsigned int midx;
    unsigned int egr_mirror_id;
    int result;
    _caladan3_mirror_t *mirror, *emirror;
    bcm_gport_t  dest_gport;

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "ENTER\n")));

    /* mirror ids always point to ingress mirror table entries, egress mirrors
     * are shadows of ingress mirrors 
     */
    midx = BCM_GPORT_MIRROR_GET(mirror_dest_id);
    mirror = &(_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[midx]);

    if (!(mirror->flags & C3_MIRROR_VALID)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Mirror is invalid \n")));
        return (BCM_E_NOT_FOUND);
    }

    dest_gport = mirror->dest_gport;

    result = _bcm_caladan3_mirror_alloc(unit,
                                      _caladan3_egress_mirror,
                                      &egr_mirror_id,
                                      dest_gport);
    if (result == BCM_E_EXISTS) {
        emirror = &_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors[egr_mirror_id];
        emirror->refCount++;
        result = BCM_E_NONE;
        LOG_VERBOSE(BSL_LS_BCM_MIRROR,
                    (BSL_META_U(unit,
                                "Updated egress mirror[%d] refCount=%d\n"),
                     egr_mirror_id, emirror->refCount));
    }

    if (BCM_FAILURE(result)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Mirror alloc failed \n")));
        return result;
    }

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "add egress mirror\n")));

    result = _bcm_caladan3_port_egr_mirror_set(unit, port, egr_mirror_id);

    if (BCM_E_NONE == result) {
        mirror->refCount++;
    } else {
        /* something went wrong */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_port_egr_mirror_set"
                               "(%d,%d,%08X) returned %d (%s)\n"),
                   unit, port, egr_mirror_id,
                   result, _SHR_ERRMSG(result)));
    }

    return (BCM_E_NONE);
}

static int
_bcm_caladan3_mirror_port_ingress_dest_delete(int unit, bcm_port_t port, bcm_gport_t mirror_dest_id)
{
    unsigned int ing_mirror_id;
    int ref_count;
    int result;

    ing_mirror_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "delete ingress mirror\n")));

    ref_count = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[ing_mirror_id].refCount;

    if (ref_count < 2) {
        return (BCM_E_NOT_FOUND);
    }

    result = _bcm_caladan3_port_ingr_mirror_set(unit, port, 0);

    if (BCM_E_NONE == result) {
        _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[ing_mirror_id].refCount--;
    } else {
        /* something went wrong */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_port_ingr_mirror_set"
                               "(%d,%d,%08X) returned %d (%s)\n"),
                   unit,
                   port,
                   ing_mirror_id,
                   result,
                   _SHR_ERRMSG(result)));
    }

    return (BCM_E_NONE);
}

static int
_bcm_caladan3_mirror_port_egress_dest_delete(int unit, bcm_port_t port, 
		                           bcm_gport_t mirror_dest_id)
{
    unsigned int egr_mirror_id = 0;
    unsigned int e_min, e_max;
    unsigned int mirror_idx, idx;
    int ref_count;
    int result;
    _caladan3_mirror_t *mirror, *emirror;
    soc_sbx_g3p1_emirror_t soc_emirror;

    mirror_idx = BCM_GPORT_MIRROR_GET(mirror_dest_id);
    mirror = &_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[mirror_idx];
    ref_count = mirror->refCount;

    if (ref_count < 2) {
        return (BCM_E_NOT_FOUND);
    }

    e_max = _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_max;
    e_min = _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_min;
    emirror = NULL;

    for (idx = e_min; idx < e_max; idx++) {
        emirror = &(_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors[idx]);
        if ((emirror->flags & C3_MIRROR_VALID) == 0 ) {
            continue;
        }
        
        if (mirror->dest_gport == emirror->dest_gport){
            egr_mirror_id = idx;
            emirror->refCount--;
            break;
        }
        emirror = NULL;
    }

    if (emirror && emirror->refCount <= 0) {
        emirror->refCount = 0;
        emirror->flags = 0;
        emirror->dest_gport = -1;
        soc_sbx_g3p1_emirror_t_init(&soc_emirror);
        result = soc_sbx_g3p1_emirror_set(unit, egr_mirror_id, &soc_emirror);
        if (BCM_FAILURE(result)) {
            /* something went wrong */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_emirror_set(%08X,"
                                   "%d,{%08X,%08X,%08X,%08X}) returned"
                                   "%d (%s)\n"),
                       unit, egr_mirror_id, 
                       soc_emirror.local, soc_emirror.dqueue,
                       soc_emirror.qid, soc_emirror.oi,
                       result, _SHR_ERRMSG(result)));
        }
    }

    result = _bcm_caladan3_port_egr_mirror_set(unit, port, 0);

    if (BCM_E_NONE == result) {
        mirror->refCount--;
    } else {
        /* something went wrong */
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_port_egr_mirror_set"
                               "(%d,%d,%08X) returned %d (%s)\n"),
                   unit,
                   port,
                   mirror_idx,
                   result,
                   _SHR_ERRMSG(result)));
    }


    return (BCM_E_NONE);
}

static int
_bcm_caladan3_mirror_port_ingress_dest_get(int unit, bcm_port_t port, 
		                         bcm_gport_t *mirror_dest_id)
{
    unsigned int ing_mirror_id;
    int result;

    result = _bcm_caladan3_port_ingr_mirror_get(unit, port, &ing_mirror_id);

    if (BCM_E_NONE != result) {
        return result;
    }

    BCM_GPORT_MIRROR_SET(*mirror_dest_id, ing_mirror_id);

    return (BCM_E_NONE);

}

static int
_bcm_caladan3_mirror_port_egress_dest_get(int unit, bcm_port_t port, bcm_gport_t *mirror_dest_id)
{
    unsigned int egr_mirror_id;
    unsigned int ing_mirror_id = ~0;
    _caladan3_mirror_t *e_mirror, *i_mirror;
    int idx, mmax, mmin;
    int result;

    result = _bcm_caladan3_port_egr_mirror_get(unit, port, &egr_mirror_id);

    if (BCM_E_NONE != result) {
        return result;
    }

    /* find matching ingress mirror id; this is the true software mirror id. */
    e_mirror = 
        &_mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors[egr_mirror_id];

    mmin = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_min;
    mmax = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max;
    
    for (idx=mmin; idx<mmax; idx++) {
        i_mirror = 
            &_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[idx];
        if (!(i_mirror->flags & C3_MIRROR_VALID)) {
            continue;
        }

        if (e_mirror->dest_gport == i_mirror->dest_gport) {
            ing_mirror_id = idx;
            break;
        }
    }

    if (ing_mirror_id == ~0) {
        return BCM_E_NOT_FOUND;
    }

    BCM_GPORT_MIRROR_SET(*mirror_dest_id, ing_mirror_id);

    return (BCM_E_NONE);
}

/*
 * Function:
 *     bcm_caladan3_mirror_destination_create
 * Purpose:
 *     Create mirror destination description.
 * Parameters:
 *      unit         - (IN) BCM device number.
 *      mirror_dest  - (IN) Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_destination_create(int unit, bcm_mirror_destination_t *mirror_dest)
{
    int result;

    MIRROR_UNIT_INIT_CHECK;

    /* Input parameters check. */
    if (NULL == mirror_dest) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "mirror destination is null \n")));
        return (BCM_E_PARAM);
    }
   
    /* Check for supported mirroring modes. */
    if (mirror_dest->flags & (BCM_MIRROR_DEST_TUNNEL_IP_GRE |
                              BCM_MIRROR_DEST_PAYLOAD_UNTAGGED)) {

        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Unsupported mirroring modes \n")));
        return (BCM_E_UNAVAIL);
    }

    /* Verify mirror destination port/trunk. */
    if ((0 == BCM_GPORT_IS_MODPORT(mirror_dest->gport)) &&
        (0 == BCM_GPORT_IS_TRUNK(mirror_dest->gport)) &&
        (0 == BCM_GPORT_IS_MPLS_PORT(mirror_dest->gport))) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Invalid destination gport \n")));
        return (BCM_E_PARAM);
    }

    /* No mirroring to a trunk. */
    if (BCM_GPORT_IS_TRUNK(mirror_dest->gport)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Mirroring to trunk is not supported \n")));
        return (BCM_E_UNAVAIL);
    }

    MIRROR_TABLE_LOCK_TAKE;

    result = _bcm_caladan3_mirror_destination_create(unit, mirror_dest);

    MIRROR_TABLE_LOCK_GIVE;
    return (result);
}

/*
 * Function:
 *     bcm_caladan3_mirror_destination_destroy
 * Purpose:
 *     Destroy mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number.
 *      mirror_dest_id  - (IN) Mirror destination id.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_destination_destroy(int unit, bcm_gport_t mirror_dest_id)
{
    int result;

    MIRROR_UNIT_INIT_CHECK;

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    MIRROR_TABLE_LOCK_TAKE;

    result = _bcm_caladan3_mirror_destination_free(unit, mirror_dest_id);

    MIRROR_TABLE_LOCK_GIVE;
    return (result);
}

/*
 * Function:
 *     bcm_caladan3_mirror_destination_get
 * Purpose:
 *     Get mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number.
 *      mirror_dest_id  - (IN) Mirror destination id.
 *      mirror_dest     - (OUT)Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_destination_get(int unit, bcm_gport_t mirror_dest_id,
                                  bcm_mirror_destination_t *mirror_dest)
{
    int ref_count;
    unsigned int mirror_idx;
    int result;
    _caladan3_mirror_t   *mirror;

    MIRROR_UNIT_INIT_CHECK;

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    if (NULL == mirror_dest) {
        return (BCM_E_PARAM);
    }

    bcm_mirror_destination_t_init(mirror_dest);
    mirror_dest->mirror_dest_id = mirror_dest_id;
    mirror_dest->flags = 0;

    mirror_idx = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    /* ALL mirror ids occupy an ingress mirror resource, even if the ingress
     * side is not used.  See notes at top of file for more details
     */
    MIRROR_TABLE_LOCK_TAKE;
    mirror = &_mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[mirror_idx];
    ref_count = mirror->refCount;
    
    if (ref_count == 0) {
        MIRROR_TABLE_LOCK_GIVE;
        return (BCM_E_NOT_FOUND);
    }
    mirror_dest->gport = mirror->dest_gport;
 

    MIRROR_TABLE_LOCK_GIVE;
    return (BCM_E_NONE);
}

/*
 * Function:
 *     bcm_caladan3_mirror_destination_traverse
 * Purpose:
 *     Traverse installed mirror destinations
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      cb        - (IN) Mirror destination traverse callback.
 *      user_data - (IN) User cookie
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_destination_traverse(int unit, bcm_mirror_destination_traverse_cb cb,
                                       void *user_data)
{
    int index;
    bcm_gport_t mirror_dest_id;
    bcm_mirror_destination_t   mirror_dest; 
    int rv = SOC_E_NONE;
    int mirror_min, mirror_max;
    int result;

    MIRROR_UNIT_INIT_CHECK;

    /* Input parameters check. */
    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    MIRROR_TABLE_LOCK_TAKE;

    mirror_max = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max;
    mirror_min = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_min;

    MIRROR_TABLE_LOCK_GIVE;

    /* Iterate mirror destinations & call user callback for valid ones. */
    for (index = mirror_min; index < mirror_max; index++) {
        BCM_GPORT_MIRROR_SET(mirror_dest_id, index);
        result = bcm_caladan3_mirror_destination_get(unit, 
                                                   mirror_dest_id,
                                                   &mirror_dest);
        if (result == BCM_E_NONE) {
            rv = (*cb)(unit, &mirror_dest, user_data);
            COMPILER_REFERENCE(rv);
#ifdef BCM_CB_ABORT_ON_ERR
            if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                return rv;
            }
#endif
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     bcm_caladan3_mirror_port_dest_add
 * Purpose:
 *      Add mirroring destination to a port.
 * Parameters:
 *      unit         -  (IN) BCM device number.
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_port_dest_add(int unit, bcm_port_t port,
                                uint32 flags, bcm_gport_t mirror_dest)
{
    bcm_module_t mymodid;           /* Local module id.              */
    bcm_module_t dest_mod;          /* Destination module id.        */
    unsigned int mirror_id;
    int result;
    bcm_gport_t  dest_gport;

    MIRROR_UNIT_INIT_CHECK;

    LOG_DEBUG(BSL_LS_BCM_MIRROR,
              (BSL_META_U(unit,
                          "ENTER\n")));

    if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalid port number %d on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PORT;
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS |
                   BCM_MIRROR_PORT_EGRESS))) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Mirror type (ingress or egress)"
                               " not specified\n")));
        return (BCM_E_PARAM);
    }

    /* Get local modid. */
    result = bcm_stk_my_modid_get(unit, &mymodid);
    if (BCM_FAILURE(result)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "Failed to get local modid \n")));
        return result;
    }

    MIRROR_TABLE_LOCK_TAKE;

    /* Check mirror destination */
    mirror_id = BCM_GPORT_MIRROR_GET(mirror_dest);

    if (!SOC_IS_SBX_CALADAN3(unit) && (flags & BCM_MIRROR_PORT_EGRESS)) {

        /* dont worry about tunnels on c1, they are not supported  */
        dest_gport = _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors[mirror_id].dest_gport;
        dest_mod = BCM_GPORT_MODPORT_MODID_GET(dest_gport);
        if (mymodid != dest_mod) {
            /* only local mirroring on CALADAN3 */
            LOG_ERROR(BSL_LS_BCM_MIRROR,
                      (BSL_META_U(unit,
                                  "can't egress mirror from unit %d"
                                   " modid %d to %d: not local\n"),
                       unit,
                       mymodid,
                       dest_mod));
            result = BCM_E_PARAM;
        }
    }

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        result = _bcm_caladan3_mirror_port_ingress_dest_add(unit, port, mirror_dest);
    }

    if (BCM_SUCCESS(result) && (flags & BCM_MIRROR_PORT_EGRESS)) {
        result = _bcm_caladan3_mirror_port_egress_dest_add(unit, port, mirror_dest);

        /* Check for operation failure. */
        if (BCM_FAILURE(result)) {
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                _bcm_caladan3_mirror_port_ingress_dest_delete(unit, port, mirror_dest);
            }
        }
    }

    MIRROR_TABLE_LOCK_GIVE;
    return (result);
}

/*
 * Function:
 *     bcm_caladan3_mirror_port_dest_delete
 * Purpose:
 *      Remove mirroring destination from a port.
 * Parameters:
 *      unit         -  (IN) BCM device number.
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_port_dest_delete(int unit, bcm_port_t port,
                                   uint32 flags, bcm_gport_t mirror_dest)
{
    int result;
    int rv2 = BCM_E_NONE;

    MIRROR_UNIT_INIT_CHECK;

    if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalid port number %d on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PORT;
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS |
                   BCM_MIRROR_PORT_EGRESS))) {
        return (BCM_E_PARAM);
    }

    MIRROR_TABLE_LOCK_TAKE;

    if ((flags & BCM_MIRROR_PORT_INGRESS) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        rv2 = _bcm_caladan3_mirror_port_ingress_dest_delete(unit, port,
                                                          mirror_dest);
    }

    if ((flags & BCM_MIRROR_PORT_EGRESS) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        result = _bcm_caladan3_mirror_port_egress_dest_delete(unit, port,
                                                            mirror_dest);
        if (!BCM_FAILURE(rv2)) {
            rv2 = result;
        }
    }

    MIRROR_TABLE_LOCK_GIVE;
    return (rv2);
}

/*
 * Function:
 *     bcm_caladan3_mirror_port_dest_get
 * Purpose:
 *     Get port mirroring destinations.
 * Parameters:
 *     unit             - (IN) BCM device number.
 *     port             - (IN) Port mirrored port.
 *     flags            - (IN) BCM_MIRROR_PORT_XXX flags.
 *     mirror_dest_size - (IN) Preallocated mirror_dest array size.
 *     mirror_dest      - (OUT)Filled array of port mirroring destinations
 *     mirror_dest_count - (OUT)Actual number of mirroring destinations filled.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_port_dest_get(int unit, bcm_port_t port, uint32 flags,
                         int mirror_dest_size, bcm_gport_t *mirror_dest,
                         int *mirror_dest_count)
{
    int         index=0;                /* Mirror to port iteration index.  */
    bcm_gport_t m_dest=0;               /* Mirror destinations.             */
    int         result = BCM_E_NONE;    /* Operation return status.         */

    MIRROR_UNIT_INIT_CHECK;

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalid port number %d on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PORT;
    }

    if ((0 != mirror_dest_size) && (NULL == mirror_dest)) {
        return (BCM_E_PARAM);
    }

    if (NULL == mirror_dest_count) {
        return (BCM_E_PARAM);
    }

    MIRROR_TABLE_LOCK_TAKE;

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        result = _bcm_caladan3_mirror_port_ingress_dest_get(unit, port, &m_dest);
        mirror_dest[index] = m_dest;
        index++;
    }

    if (flags & BCM_MIRROR_PORT_EGRESS) {
        result = _bcm_caladan3_mirror_port_egress_dest_get(unit, port, &m_dest);
        if (mirror_dest_size <= index) { 
            MIRROR_TABLE_LOCK_GIVE;
            return (BCM_E_PARAM);
        }
        mirror_dest[index] = m_dest;
        index++;
    }

    *mirror_dest_count = index;

    MIRROR_TABLE_LOCK_GIVE;
    return (result);
}

/*
 * Function:
 *     bcm_caladan3_mirror_port_dest_delete_all
 * Purpose:
 *      Remove all mirroring destinations from a port.
 * Parameters:
 *      unit         -  (IN) BCM device number.
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_caladan3_mirror_port_dest_delete_all(int unit, bcm_port_t port, uint32 flags)
{
    bcm_gport_t mirror_dest[G3P1_MIRROR_PORT_MAX];
    int         mirror_dest_count;
    int         index;
    int         result = BCM_E_NONE;

    MIRROR_UNIT_INIT_CHECK;

    if (!(flags & (BCM_MIRROR_PORT_INGRESS |
                   BCM_MIRROR_PORT_EGRESS))) {
        return (BCM_E_PARAM);
    }

    MIRROR_TABLE_LOCK_TAKE;

    if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
        LOG_ERROR(BSL_LS_BCM_MIRROR,
                  (BSL_META_U(unit,
                              "invalid port number %d on unit %d\n"),
                   port,
                   unit));
        return BCM_E_PORT;
    }

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        /* Read port ingress mirroring destination ports. */
        result = bcm_caladan3_mirror_port_dest_get(unit, port, BCM_MIRROR_PORT_INGRESS,
                                             G3P1_MIRROR_PORT_MAX, mirror_dest,
                                             &mirror_dest_count);
        if (BCM_FAILURE(result)) {
            MIRROR_TABLE_LOCK_GIVE;
            return (result);
        }

        /* Remove all ingress mirroring destination ports. */
        for (index = 0; index < mirror_dest_count; index++) {
            result = bcm_caladan3_mirror_port_dest_delete(unit, port,
                                                    BCM_MIRROR_PORT_INGRESS,
                                                    mirror_dest[index]);
            if (BCM_FAILURE(result)) {
                MIRROR_TABLE_LOCK_GIVE;
                return (result);
            }
        }
    }

    if (flags & BCM_MIRROR_PORT_EGRESS) {
        result = bcm_caladan3_mirror_port_dest_get(unit, port, BCM_MIRROR_PORT_EGRESS,
                                             G3P1_MIRROR_PORT_MAX, mirror_dest,
                                             &mirror_dest_count);
        if (BCM_FAILURE(result)) {
            MIRROR_TABLE_LOCK_GIVE;
            return (result);
        }

        /* Remove all egress mirroring destination ports. */
        for (index = 0; index < mirror_dest_count; index++) {
            result = bcm_caladan3_mirror_port_dest_delete(unit, port,
                                                    BCM_MIRROR_PORT_EGRESS,
                                                    mirror_dest[index]);
            if (BCM_FAILURE(result)) {
                MIRROR_TABLE_LOCK_GIVE;
                return (result);
            }
        }
    }

    MIRROR_TABLE_LOCK_GIVE;
    return (result);
}


void
_bcm_caladan3_mirror_sw_dump(int unit)
{
    int midx;
    char tmp[32];
    _caladan3_mirror_direction_t direction;
    _caladan3_mirror_state_t *state;
    char *dir_str[_caladan3_mirror_direction_max] = {"ingress", "egress"};
    _caladan3_mirror_t  *mirror;

#define PRINT_HEADER(dir_str_)  \
    LOG_CLI((BSL_META_U(unit, \
                        "%s Mirrors:\n"), dir_str_));  \
    LOG_CLI((BSL_META_U(unit, \
                        "%3s %11s %6s %5s\n"),   \
             "ID", "Destination", "RefCnt", "flags")); \
    LOG_CLI((BSL_META_U(unit, \
                        "--- ----------- ------ -----\n"))); 

    for (direction = 0; direction < _caladan3_mirror_direction_max; direction++) {
        state = &_mirror_glob[unit]->state[direction];
        PRINT_HEADER(dir_str[direction]);
        for (midx=0; midx < state->mirror_max; midx++) {
            mirror = &(state->mirrors[midx]);
	    if (mirror->flags & C3_MIRROR_VALID) {
                if (mirror->flags & C3_MIRROR_TUNNEL) { 
                    sal_snprintf(tmp, 32, "%x", mirror->dest_gport); 
                    LOG_CLI((BSL_META_U(unit,
                                        "%3d %11s %6d 0x%03x\n"),   
                             midx, tmp, mirror->refCount, mirror->flags)); 
                } else if (mirror->dest_gport != -1) { 
                    sal_snprintf(tmp, 32, "%d:%d",  
                        BCM_GPORT_MODPORT_MODID_GET(mirror->dest_gport), 
                        BCM_GPORT_MODPORT_PORT_GET(mirror->dest_gport)); 
                    LOG_CLI((BSL_META_U(unit,
                                        "%3d %11s %6d 0x%03x\n"),   
                             midx, tmp, mirror->refCount, mirror->flags)); 
                }
            }
        }
    }

#undef PRINT_HEADER
#undef PRINT_ENTRY
}


