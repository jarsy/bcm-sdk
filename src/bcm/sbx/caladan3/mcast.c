/*
 * $Id: mcast.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Multicast Table Management
 */


/* This is used to mark off (ifdef out)
 * places that need to be revisited and fixed later. */
#define C2_SPECIFIC_UNPORTED (0)

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/feature.h>
#include <soc/mem.h>
#include <soc/macipadr.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/error.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/l2.h>
#include <bcm/pkt.h>
#include <bcm_int/sbx/state.h>

/*
 *  More includes, but some of these are based upon configuration.
 */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/l2.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/mcast.h>
#if !C2_SPECIFIC_UNPORTED
static
int soc_sbx_g3p1_mac_commit(int unit, int runlength)
{
    return BCM_E_NONE;
}
#endif /* !C2_SPECIFIC_UNPORTED */
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>


#define MCAST_RETURN_IF_NOT_MCAST_ADDR(_m_)                     \
    if (BCM_MAC_IS_MCAST(_m_) == 0) {                           \
        LOG_ERROR(BSL_LS_BCM_MCAST, \
                  (BSL_META("%s: " L2_6B_MAC_FMT " is not multicast\n"), \
                   FUNCTION_NAME(), L2_6B_MAC_PFMT(_m_)));      \
        return BCM_E_PARAM;                                     \
    }

/*
 *  This function is defined later but is needed by add to get the ports bitmap
 *  for the appropriate VID's all-routers entry (if present).
 */
int
bcm_caladan3_mcast_port_get(int unit,
                            sal_mac_addr_t mac,
                            bcm_vlan_t vid,
                            bcm_mcast_addr_t *mcAddr);


/*
 *   Function
 *      _bcm_caladan3_mcast_addr_add
 *   Purpose
 *      Adds a multicast address to the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address to add
 *      (in) uint32 spec_flags = special flags from early in bcm_int/mcast.h
 *      (in) uint32 flags = BCM_L2_* flags from early in l2.h
 *      (out) uint32 *new_ftIndex = where to put the resulting ftIndex
 *      (out) uint32 *new_mcGroup = where to put the resulting mcGroup
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Flags currently honoured:
 *          BCM_L2_DISCARD_DST
 *          BCM_L2_COPY_TO_CPU
 *          BCM_L2_STATIC
 *      Must always set bit 0 in spec_flags, and must always provide the
 *      mcGroup as l2mc_index (it is not even taken from an existing entry).
 *      Does not change l2mc_index value in the provided bcm_mcast_addr_t.
 *      Set new_ftIndex to NULL if you don't want it.
 *      Set new_mcGroup to NULL if you don't want it.
 *      Replaces existing entry if it is there and enough matches.
 *      Quietly ignores lack of all_routers address if none for the VID.
 *      Enforces that tagging and ports agree with the VID configuration.
 */
int
_bcm_caladan3_mcast_addr_add(int unit,
                             bcm_mcast_addr_t *mcAddr,
                             uint32 flags,
                             uint32 spec_flags,
                             uint32 *new_ftIndex,
                             uint32 *new_mcGroup)
{
    int                       tempRes;      /* result code from called func */
    soc_sbx_g3p1_mac_t        p3macPyld;    /* MAC payload */
    soc_sbx_g3p1_ft_t         p3ft;         /* FT (multicast) entry */
    bcm_pbmp_t                pbmp;        /* working port bitmap */
    bcm_pbmp_t                vbmp;        /* ports in this VLAN */
    bcm_pbmp_t                ubmp;        /* untagged ports in this VLAN */
    uint32                    ftIndex = ~0;/* FT (multicast) entry index */
    uint16                    groupIndex;  /* mc group index */
    bcm_mcast_addr_t          arData;      /* all-routers group information */
    int                       result;      /* result code to calling func */
    sal_mac_addr_t            arMac;       /* all-routers MAC */

    /*
     *  This tracks what has been allocated during this call that might need to
     *  be freed/destroyed/removed if an error occurs so that we don't leak on
     *  error, but also don't destroy existing state on errors, all while
     *  keeping the code simpler (only one copy of free/destroy/remove code).
     */
    enum _mcaa_resflags { _alloc_dm = 0x01,
                          _alloc_ft = 0x02,
                          _set_dm   = 0x10,
                          _set_ft   = 0x20
                        } resflags;

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* Check other parameter validity */
    if (!mcAddr) {
        return BCM_E_PARAM;
    }

    /* be chatty about the requested operation, if desired */
    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "_bcm_caladan3_mcast_addr_add(%d,"
                             "(%03X/" L2_6B_MAC_FMT ",%08X,%08X),"
                            "%08X,%08X,*,*)\n"),
                 unit,
                 mcAddr->vid,
                 L2_6B_MAC_PFMT(mcAddr->mac),
                 SOC_PBMP_WORD_GET(mcAddr->pbmp,0),
                 SOC_PBMP_WORD_GET(mcAddr->ubmp,0),
                 flags,
                 spec_flags));

    /* make sure the port bitmaps are valid */
    BCM_PBMP_NEGATE(pbmp,PBMP_ALL(unit));
    BCM_PBMP_AND(pbmp,mcAddr->pbmp);
    if (BCM_PBMP_NOT_NULL(pbmp)) {
        /* the requested pbmp contains invalid ports (not in PBMP_ALL) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_add: pbmp contains ports not"
                               " in pbmp_all\n")));
        return BCM_E_PARAM;
    }
    BCM_PBMP_NEGATE(pbmp, mcAddr->pbmp);
    BCM_PBMP_AND(pbmp, mcAddr->ubmp);
    if (BCM_PBMP_NOT_NULL(pbmp)) {
        /* the requested ubmp contains invalid ports (not in req pbmp) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_add: ubmp contains ports not"
                               " in the pbmp\n")));
        return BCM_E_PARAM;
    }

    /* We'll depend upon the MVT calls to reject invalid MVTE index. */
    /* get the VLAN port information */
    result = _bcm_caladan3_vlan_port_fetch(unit,
                                           mcAddr->vid,
                                           &vbmp,
                                           &ubmp);
    if (BCM_E_NONE != result) {
        /* something went wrong getting ports; assume no ports */
        BCM_PBMP_CLEAR(ubmp);
        BCM_PBMP_CLEAR(vbmp);
    }
    /* make sure port information is valid */
    BCM_PBMP_NEGATE(pbmp, vbmp);
    BCM_PBMP_AND(pbmp, mcAddr->pbmp);
    if (BCM_PBMP_NOT_NULL(pbmp)) {
        /* ports not in the VLAN are in the PBMP */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_add: pbmp contains ports not"
                               " in the VLAN\n")));
        return BCM_E_PARAM;
    }
    BCM_PBMP_ASSIGN(pbmp, ubmp);
    BCM_PBMP_AND(pbmp, mcAddr->pbmp);
    if (!BCM_PBMP_EQ(pbmp, mcAddr->ubmp)) {
        /* ports in the map don't agree with VLAN tag mode */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_add: ports are not tagged in the"
                               " same way as the VLAN\n")));
        return BCM_E_PARAM;
    }

    /* may need the all-routers address later */
    if (0 == (spec_flags & BCM_CALADAN3_MCAST_ADD_INHIBIT_ARG)) {
        sal_memcpy(&arMac, &_soc_mac_all_routers, sizeof(sal_mac_addr_t));
    }

    /* some further initialisation */
    resflags = 0;

    result = soc_sbx_g3p1_mac_get(unit,
                                  mcAddr->mac,
                                  mcAddr->vid,
                                  &p3macPyld);
    if (BCM_E_NONE == result) {
        /* it's in the table; get it and associated things */
        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                    (BSL_META_U(unit,
                                "_bcm_caladan3_mcast_addr_add: updating existing"
                                 " VID+DMACa\n")));
        ftIndex = p3macPyld.ftidx;
        result = soc_sbx_g3p1_ft_get(unit, ftIndex, &p3ft);
        if (BCM_E_NONE == result) {
            /* FT entry was read; get the mcGroup/MVT index */
            groupIndex = p3ft.oi;
            if (spec_flags & BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX) {
                /* using l2mc_index value; validate that */
                if ((groupIndex) != mcAddr->l2mc_index) {
                    /* but it's wrong value; no match */
                    LOG_ERROR(BSL_LS_BCM_MCAST,
                              (BSL_META_U(unit,
                                          "_bcm_caladan3_mcast_addr_add: "
                                           "l2mc_index disagreed with "
                                           "existing dest VID+MACa\n")));
                    result = BCM_E_NOT_FOUND;
                }
            } /* if (spec_flags & BCM_CALADAN3_MCAST_ADD_SPEC_MCGROUP) */
            groupIndex = 0; 
        } /* if (BCM_E_NONE == result) */
    } else if (BCM_E_NOT_FOUND == result) {
        /* it's not in the table */
        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                    (BSL_META_U(unit,
                                "_bcm_caladan3_mcast_addr_add: adding new VID+DMACa\n")));
        resflags |= _alloc_dm;
        /* clear out a new destination MAC payload */
        soc_sbx_g3p1_mac_t_init(&p3macPyld);
        /* it's not in the table; try to allocate needed resources */

        result = _sbx_caladan3_resource_alloc(unit,
                                              SBX_CALADAN3_USR_RES_FTE_L2MC,
                                              1,
                                              &ftIndex,
                                              0);
        if (BCM_E_NONE == result) {
            /* the FT entry is marked ours now; initialise it */
            resflags |= _alloc_ft;
            soc_sbx_g3p1_ft_t_init(&p3ft);
            /* we also need an mcGroup / MVT entry; try to get it */
            if (spec_flags & BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX) {
                /* using the l2mc_index value; get that */
                groupIndex = mcAddr->l2mc_index;
            } else {

                /* should never get here, but it's because invalid param */
                LOG_ERROR(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "_bcm_caladan3_mcast_addr_add: "
                                       "MVT entry not provided\n")));
                groupIndex = 0; 
                result = BCM_E_PARAM;
            }

        } else { /* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "_bcm_caladan3_mcast_addr_add: "
                                   "unable to allocate or reserve l2mc entry\n")));
            groupIndex = 0; 
        } /* if (BCM_E_NONE == result) */
    } else {/* else if (SB_MAC_NOT_FOUND == rc) */
            /* neither found nor not found; something else went wrong */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_add:"
                               " unexpeced result %d (%s) searching VID/DMACa"
                               " table\n"),
                   result,
                   _SHR_ERRMSG(result)));
        groupIndex = 0; 
    }
    if (BCM_E_NONE == result) {
        /* everything verified and resources allocated */
        /* get a working copy of the port enable bitmap */
        BCM_PBMP_ASSIGN(pbmp, mcAddr->pbmp);
        /* see if we need the all-routers entry data */
        if (0 == (spec_flags & BCM_CALADAN3_MCAST_ADD_INHIBIT_ARG)) {
            /* need all-routers data; try to fetch */
            bcm_mcast_addr_t_init(&arData, arMac, mcAddr->vid);
            if (BCM_E_NONE == bcm_caladan3_mcast_port_get(unit,
                                                          arMac,
                                                          mcAddr->vid,
                                                          &arData)) {
                /* we have all-routers entry on this VID; add the ports */
                BCM_PBMP_OR(pbmp, arData.pbmp);
            }
        }
        /* set proper values in the FT entry */
        p3ft.excidx = 0;
        p3ft.hc = FALSE;
        p3ft.oi = groupIndex;
        p3ft.mc = 1;
        p3ft.qid = MCAST_QID_BASE;
        if ((mcAddr->distribution_class > 0) &&
            (mcAddr->distribution_class < SOC_SBX_CFG(unit)->num_ds_ids)) {
            p3ft.qid += (mcAddr->distribution_class * SBX_MAX_COS);
        } else if ((mcAddr->distribution_class < 0) ||
                   (mcAddr->distribution_class >=
                    SOC_SBX_CFG(unit)->num_ds_ids)) {
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "unit %d: invalid fabric distribution class %d\n"),
                       unit, mcAddr->distribution_class));
            result = BCM_E_PARAM;
        }

        /* set proper values in the destination MAC payload */
        p3macPyld.ageid = 0;
        p3macPyld.sdrop = TRUE; /* bogus to send from MC address */
        p3macPyld.ddrop = FALSE;
        p3macPyld.ftidx = ftIndex;

        if (BCM_E_NONE == result) {

            /* now the FT entry */
            result = soc_sbx_g3p1_ft_set (unit, ftIndex, &p3ft);
            if (BCM_E_NONE == result) {
                /* good; FT entry set */
                resflags |= _set_ft;
                /* now add the destination MAC address */
                if (resflags & _alloc_dm) {
                    /* needed a new dMACa; add it */
                    result = soc_sbx_g3p1_mac_add(unit,
                                                  mcAddr->mac,
                                                  mcAddr->vid,
                                                  &p3macPyld);
                } else { /* if (alloc_dm) */
                    /* already had a dMACa; update it */
                    result = soc_sbx_g3p1_mac_update(unit,
                                                     mcAddr->mac,
                                                     mcAddr->vid,
                                                     &p3macPyld);
                } /* if (alloc_dm) */
                if (BCM_E_NONE == result) {
                    if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
                        /* commit the table update */
                        result = soc_sbx_g3p1_mac_commit(unit,
                                                         0xFFFFFFFF);
                    }
                    if (BCM_E_NONE == result) {
                        /* good; dMACa now set */
                        resflags |= _set_dm;
                        if (new_ftIndex) {
                            *new_ftIndex = ftIndex;
                        }
                        if (new_mcGroup) {
                            *new_mcGroup = groupIndex;
                        }
                        mcAddr->l2mc_index = groupIndex;

                        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                                    (BSL_META_U(unit,
                                                "_bcm_caladan3_mcast_addr_add: "
                                                 "success; ftIdx = %08X, "
                                                 "mcGrp = %08X "
                                                 "l2mcI = %08X\n"),
                                     ftIndex,
                                     groupIndex,
                                     mcAddr->l2mc_index));
                    } /* if (SB_OK == rc) */
                } /* if (SB_OK == rc) */
                if (BCM_E_NONE != result) {
                    /* something went wrong */
                    LOG_ERROR(BSL_LS_BCM_MCAST,
                              (BSL_META_U(unit,
                                          "unable to add/update/commit dMACa\n")));
                }
            } else { /* if (BCM_E_NONE == result) */
                /* could not set the FT entry */
                LOG_ERROR(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "can't set %d:ft[%08X]: %d (%s)\n"),
                           unit,
                           ftIndex,
                           result,
                           _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        }
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE != result) {
        /* we're here because something went wrong. */
        /*
         *  We try to undo partial adds, but not updates (partial updates do
         *  not leak resources nor do they cause table inconsistencies the way
         *  we're doing things).
         */
        if ((_set_dm | _alloc_dm) == (resflags & (_set_dm | _alloc_dm))) {
            /* commit failed on dMACa add; remove the dMACa and recommit */
            soc_sbx_g3p1_mac_remove(unit, mcAddr->mac, mcAddr->vid);
            if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
                soc_sbx_g3p1_mac_commit(unit, 0xFFFFFFFF);
            }
        } /* ((_set_dm | _alloc_dm) == (resflags & (_set_dm | _alloc_dm))) */
        if (resflags & _alloc_ft) {
            if (resflags & _set_ft) {
                /* something failed after we added an FT entry; back it out */
                p3ft.excidx = MCAST_FT_INVALID_EXC;
                tempRes = soc_sbx_g3p1_ft_set(unit, ftIndex, &p3ft);
                COMPILER_REFERENCE(tempRes);
                
            }
            /* something failed after we allocated an FT entry; free it */
            _sbx_caladan3_resource_free(unit,
                                        SBX_CALADAN3_USR_RES_FTE_L2MC,
                                        1,
                                        &ftIndex,
                                        0);
        } /* if (resflags & _alloc_ft) */
    } /* if (BCM_E_NONE != result) */

    return result;
}

/*
 *   Function
 *      _bcm_caladan3_mcast_addr_remove
 *   Purpose
 *      Removes a multicast address from the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) sal_mac_addr_t mac = MAC address
 *      (in) bcm_vlan_t vid = VID
 *      (in) uint32 l2mc_index = l2mc_index value (if used, else zero)
 *      (in) uint32 spec_flags = special flags from early in bcm_int/mcast.h
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Removes the address only if the provided l2mc_index matches the actual
 *      one in the existing entry.
 */
static
int
_bcm_caladan3_mcast_addr_remove(int unit,
                                sal_mac_addr_t mac,
                                bcm_vlan_t vid,
                                uint32 l2mc_index,
                                uint32 spec_flags)
{
    soc_sbx_g3p1_mac_t       p3macPyld;    /* MAC payload */
    soc_sbx_g3p1_ft_t         p3ft;         /* FT (multicast) entry */
    int                       result;       /* result code */
    int                       vsi_max;

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "_bcm_caladan3_mcast_addr_remove(%d,"
                             "(0x%03X/" L2_6B_MAC_FMT ",%08X),%08X) enter\n"),
                 unit, vid, L2_6B_MAC_PFMT(mac),
                 l2mc_index, spec_flags));

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= vid) || (vsi_max <= vid)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_remove: %03X is not a valid VID\n"),
                   vid));
        return BCM_E_PARAM;
    }

    /* need to fetch the entry before deletion (need data from it) */
    result = soc_sbx_g3p1_mac_get(unit, mac, vid, &p3macPyld);
    if (BCM_E_NONE == result) {
        /* okay, we have the entry from the table */
        /* get the FT entry */
        result = soc_sbx_g3p1_ft_get(unit, p3macPyld.ftidx, &p3ft);
        if ((BCM_E_NONE == result) && (spec_flags &
                                       BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX)) {
            /* check the l2mc_index provided against the actual entry */
            if ((l2mc_index) != p3ft.oi) {
                /* this is not the correct entry; remark it as not found */
                /* rc = SB_MAC_NOT_FOUND; */
            }
        } /* if ((SB_OK == rc) && (checking l2mc_index value)) */
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "_bcm_caladan3_mcast_addr_remove: get %d:ft[%08X] for"
                                   " " L2_6B_MAC_FMT " resulted in"
                                   " %d (%s)\n"),
                       unit, p3macPyld.ftidx, L2_6B_MAC_PFMT(mac),
                       result, _SHR_ERRMSG(result)));
        }

    } else { /* if (BCM_E_NONE == result) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_mcast_addr_remove: get "
                               " " L2_6B_MAC_FMT " resulted in %d (%s)\n"),
                   L2_6B_MAC_PFMT(mac), result,
                   _SHR_ERRMSG(result)));

    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* remove the entry from the table */
        result = soc_sbx_g3p1_mac_delete(unit, mac, vid);
        if (BCM_E_NONE == result) {
            /* okay, deleted the entry; commit the change */
            if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
                
                soc_sbx_g3p1_mac_commit(unit, 0xFFFFFFFF);
            }
        } else { /* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "_bcm_caladan3_mcast_addr_remove: delete "
                                   " " L2_6B_MAC_FMT " resulted in"
                                   " %d (%s)\n"),
                       L2_6B_MAC_PFMT(mac), result,
                       _SHR_ERRMSG(result)));

        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            /* deleted entry and committed change */
            /* now disable FT entry */
            p3ft.excidx = MCAST_FT_INVALID_EXC;
            result = soc_sbx_g3p1_ft_set(unit, p3macPyld.ftidx, &p3ft);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "_bcm_caladan3_mcast_addr_remove: set %d:ft[0x%08X]"
                                       " for " L2_6B_MAC_FMT " resulted"
                                       " in %d (%s)\n"),
                           unit, p3macPyld.ftidx, L2_6B_MAC_PFMT(mac), result,
                           _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* now dispose of resources associated with the entry */
        result = _sbx_caladan3_resource_free(unit,
                                             SBX_CALADAN3_USR_RES_FTE_L2MC,
                                             1,
                                             &(p3macPyld.ftidx),
                                             0);
    } /* if (BCM_E_NONE == result) */

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "_bcm_caladan3_mcast_addr_remove(%d,"
                            "(Ox%03X/" L2_6B_MAC_FMT ",%08X),%08X) return"
                            " %d (%s)\n"),
                 unit, vid, L2_6B_MAC_PFMT(mac), l2mc_index,
                 spec_flags, result, _SHR_ERRMSG(result)));

    /* done; return the result */
    return result;
}

/*
 *   Function
 *      bcm_caladan3_mcast_init
 *   Purpose
 *      Initialise L2 multicast handling
 *   Parameters
 *      (in) int unit = unit number
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_mcast_init(int unit)
{
    soc_sbx_g3p1_6_byte_t     p3prevMac;     /* previous search key */
    soc_sbx_g3p1_mac_t        p3prevMacPyld; /* previous mac payload */
    int                       p3prevVid = 0; /* previous VID */
    soc_sbx_g3p1_6_byte_t     p3thisMac;     /*  current search key */
    soc_sbx_g3p1_mac_t        p3thisMacPyld; /* current mac payload */
    int                       p3thisVid;     /* current VID */
    soc_sbx_g3p1_ft_t         p3ft;          /* FT (multicast) entry */
    int                       doneScan;     /* done scanning */
    unsigned int              delPrev;       /* delete previous entry flag */
    int                       tempRes;       /* result code from called */
    int                       result;        /* result code to caller */
    int                       entriesDeleted; /* count of deleted entries */

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "enter\n")));

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    entriesDeleted = 0;
    sal_memset(&p3prevMacPyld, 0, sizeof(p3prevMacPyld));
    soc_sbx_g3p1_mac_t_init(&p3thisMacPyld);

    /* be optimisitic about return values */
    result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "mcast_init %s tables (SLOW).\n"),
                 SOC_WARM_BOOT(unit) ? "recovering" : "purging"));

    /* walk the L2 destination MAC table, clearing out our entries */
    memset(&p3prevMac, 0, sizeof(p3prevMac));
    memset(&p3thisMac, 0, sizeof(p3thisMac));
    delPrev = FALSE;
    doneScan = FALSE;
    /* get the first entry */
    tempRes = soc_sbx_g3p1_mac_first(unit, p3thisMac, &p3thisVid);
    /* *and* its contents */
    if (BCM_E_NONE == tempRes) {
        soc_sbx_g3p1_mac_t_init(&p3thisMacPyld);
        tempRes = soc_sbx_g3p1_mac_get(unit,
                                       p3thisMac,
                                       p3thisVid,
                                       &p3thisMacPyld);
        if (BCM_FAILURE(tempRes)) {
           LOG_VERBOSE(BSL_LS_BCM_MCAST,
                       (BSL_META_U(unit,
                            "mcast_init get mac entry failed(%d).\n"),
                       tempRes));            
        }
    } else {
        /* or nothing, and skip this if there isn't a first entry */
        doneScan = TRUE;
    }

    /* Scan the entire l2 table searching for multicast addresses.
     * If warm boot, recover the PID and reserve it
     * else, purge the entry
     */
    while (!doneScan || delPrev) {

        if (delPrev) {
            assert(SOC_WARM_BOOT(unit) == FALSE);
            /* need to delete previous entry */
            LOG_VERBOSE(BSL_LS_BCM_MCAST,
                        (BSL_META_U(unit,
                                    "deleting old dMAC entry"
                                     " 0x%03X/" L2_6B_MAC_FMT "\n"),
                         p3prevVid, L2_6B_MAC_PFMT(p3prevMac)));

            tempRes = soc_sbx_g3p1_ft_get(unit,
                                          p3prevMacPyld.ftidx,
                                          &p3ft);

            if (BCM_E_NONE == tempRes) {
                /* got the forwarding entry */
                /* dispose of the MAC entry */
                soc_sbx_g3p1_mac_remove(unit,
                                        p3prevMac,
                                        p3prevVid);

                /* clear the FT entry & write it back */
                LOG_VERBOSE(BSL_LS_BCM_MCAST,
                            (BSL_META_U(unit,
                                        "clearing and freeing FT entry "
                                         "0x%08X\n"),
                             p3prevMacPyld.ftidx));
                p3ft.excidx = MCAST_FT_INVALID_EXC;
                p3ft.oi = 0;
                p3ft.mc = 0;
                /* coverity [returned_value] */
                tempRes = soc_sbx_g3p1_ft_set(unit, p3prevMacPyld.ftidx, &p3ft);

                /* now free the FT entry */
                LOG_VERBOSE(BSL_LS_BCM_MCAST,
                            (BSL_META_U(unit,
                                        "freeing FT entry %08X\n"),
                             p3prevMacPyld.ftidx));

                _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_L2MC,
                                            1, &(p3prevMacPyld.ftidx), 0);
                entriesDeleted++;
            } /* if (BCM_E_NONE == tempRes) */

            delPrev = FALSE;
        } /* if (delPrev) */

        /* examine the next one */
        if (!doneScan) {
            sal_memcpy(&p3prevMac, &p3thisMac, sizeof(p3prevMac));
            p3prevVid = p3thisVid;
            p3prevMacPyld = p3thisMacPyld;
            memset(&p3thisMac, 0, sizeof(p3thisMac));
            LOG_DEBUG(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "examining dMACa"
                                   " 0x%03X/" L2_6B_MAC_FMT
                                   " (FTIdx 0x%08X)\n"),
                       p3prevVid, L2_6B_MAC_PFMT(p3prevMac),
                       p3prevMacPyld.ftidx));

            /* we have to guess if it's multicast or not */
            if (BCM_MAC_IS_MCAST(p3prevMac)) {
                /* this looks like it's ours */
                LOG_DEBUG(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "found dMACa to %s "
                                       " 0x%03X/" L2_6B_MAC_FMT "\n"),
                           SOC_WARM_BOOT(unit) ? "recover" : "purge",
                           p3prevVid, L2_6B_MAC_PFMT(p3prevMac)));

                delPrev = TRUE;
            } /* if (this MAC address should be purged) */

            /* warm boot will reserve entries, not purge them. */
            if (delPrev && (SOC_WARM_BOOT(unit) == TRUE)) {
                delPrev = FALSE;

                LOG_DEBUG(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "recovering PID 0x%04x\n"),
                           p3prevMacPyld.ftidx));
                tempRes = _sbx_caladan3_resource_alloc(unit,
                                                       SBX_CALADAN3_USR_RES_FTE_L2MC,
                                                       1, &p3prevMacPyld.ftidx,
                                                       _SBX_CALADAN3_RES_FLAGS_RESERVE);

                /* other modules may reserve these before mcast, l2cache
                 * for example.  Since the l2 table is fully recovered, it is
                 * safe to assume that any address in the table should be
                 * reserved, and this warm boot will catch the remainders */
                if (tempRes == BCM_E_RESOURCE) {
                    tempRes = BCM_E_NONE;
                }
                if (BCM_FAILURE(tempRes)) {
                    LOG_ERROR(BSL_LS_BCM_MCAST,
                              (BSL_META_U(unit,
                                          "Failed to reserve PID "
                                           "0x%04x: %s\n"),
                               p3prevMacPyld.ftidx, bcm_errmsg(tempRes)));
                }
            }

            /* get the next address */
            tempRes = soc_sbx_g3p1_mac_next(unit,
                                            p3prevMac, p3prevVid,
                                            p3thisMac, &p3thisVid);

            if (BCM_E_NONE == tempRes) {
                tempRes = soc_sbx_g3p1_mac_get(unit,
                                               p3thisMac, p3thisVid,
                                               &p3thisMacPyld);
                if (BCM_E_NONE != tempRes) {
                    doneScan = TRUE;
                }
            } else {
                /* didn't get a new one */
                doneScan = TRUE;
            }
        } /* if (!doneScan) */
    } /* while (!doneScan || delPrev) */


    if (entriesDeleted) {
        result = soc_sbx_g3p1_mac_commit(unit, 0xFFFFFFFF);
        if (BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "failed to commit deleted entries: %s\n"),
                       bcm_errmsg(result)));
        }
    }

    /* and destroy the old list just in case */
    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "rv=%d(%s)\n"),
                 result, bcm_errmsg(result)));

    /* and return the result to the caller */
    return result;
}

int
bcm_caladan3_mcast_detach(int unit)
{

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_caladan3_mcast_port_add
 *   Purpose
 *      Adds ports to those in the multicast group
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address & ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

int
bcm_caladan3_mcast_port_add(int unit,
                            bcm_mcast_addr_t *mcAddr)
{
    soc_sbx_g3p1_mac_t      p3macPyld;     /* MAC payload */
    soc_sbx_g3p1_ft_t       p3ft;          /* FT (multicast) entry */
    bcm_pbmp_t              pbmp;        /* working port bitmap */
    unsigned int            ftIndex = ~0;/* FT (multicast) entry index */
    int                     result;      /* result code to calling function */
    int                     vsi_max;

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* Check other parameter validity */
    if (!mcAddr) {
        return BCM_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "MCast %d: add ports"
                             " Ox%03X/" L2_6B_MAC_FMT ", %08X\n"),
                 unit, mcAddr->vid, L2_6B_MAC_PFMT(mcAddr->mac),
                 SOC_PBMP_WORD_GET(mcAddr->pbmp,0)));

    /* make sure the port bitmap is valid */
    BCM_PBMP_ASSIGN(pbmp,mcAddr->pbmp);
    BCM_PBMP_AND(pbmp,PBMP_ALL(unit));
    if (!BCM_PBMP_EQ(pbmp,mcAddr->pbmp)) {
        /* but the requested bitmap contains invalid ports */
        return BCM_E_PARAM;
    }

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mcAddr->mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= mcAddr->vid) || (vsi_max <= mcAddr->vid)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_port_add: %03X is not a valid VID\n"),
                   mcAddr->vid));
        return BCM_E_PARAM;
    }

    /* search for the entry to see if it already exists */
    result = soc_sbx_g3p1_mac_get(unit, mcAddr->mac, mcAddr->vid,
                                  &p3macPyld);

    if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) {
        /* it's in the table; we can continue */
        /* read the forwarding table entry */
        ftIndex = p3macPyld.ftidx;
        result = soc_sbx_g3p1_ft_get(unit, ftIndex, &p3ft);

        if (0 != p3ft.excidx)
          {
              /* FT entry was not valid */
              LOG_ERROR(BSL_LS_BCM_MCAST,
                        (BSL_META_U(unit,
                                    "p3ft was marked with exception %02X\n"),
                         p3ft.excidx));
              result = BCM_E_NOT_FOUND;

          } else if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "unable to read %d:ft[%08X]: %d (%s)\n"),
                       unit,
                       ftIndex,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else { /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "unable to find mcast group: %d (%s)\n"),
                   result,
                   _SHR_ERRMSG(result)));
    } /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */

    return result;
}

/*
 *   Function
 *      bcm_caladan3_mcast_port_remove
 *   Purpose
 *      Removes ports from those in the multicast group
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address & ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Ports that are *SET* in the provided port bitmap will be removed.
 */
int
bcm_caladan3_mcast_port_remove(int unit,
                               bcm_mcast_addr_t *mcAddr)
{
    soc_sbx_g3p1_mac_t      p3macPyld;     /* MAC payload */
    soc_sbx_g3p1_ft_t       p3ft;          /* FT (multicast) entry */
    bcm_pbmp_t              pbmp;        /* working port bitmap */
    int                     result;      /* result code to calling function */
    unsigned int            ftIndex = ~0;/* FT (multicast) entry index */
    int                     vsi_max;

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* Check other parameter validity */
    if (!mcAddr) {
        return BCM_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "MCast %d: remove ports"
                             " 0x%03X/" L2_6B_MAC_FMT ", %08X\n"),
                 unit, mcAddr->vid, L2_6B_MAC_PFMT(mcAddr->mac),
                 SOC_PBMP_WORD_GET(mcAddr->pbmp,0)));

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mcAddr->mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= mcAddr->vid) || (vsi_max <= mcAddr->vid)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_port_remove: %03X is not a valid VID\n"),
                   mcAddr->vid));
        return BCM_E_PARAM;
    }

    /* make sure the port bitmap is valid */
    BCM_PBMP_ASSIGN(pbmp,mcAddr->pbmp);
    BCM_PBMP_AND(pbmp,PBMP_ALL(unit));
    if (!BCM_PBMP_EQ(pbmp,mcAddr->pbmp)) {
        /* but the requested bitmap contains invalid ports */
        return BCM_E_PARAM;
    }
    BCM_PBMP_NEGATE(pbmp,mcAddr->pbmp);
    BCM_PBMP_AND(pbmp,PBMP_ALL(unit));

    /* search for the entry to see if it already exists */
    result = soc_sbx_g3p1_mac_get(unit, mcAddr->mac, mcAddr->vid,
                                  &p3macPyld);
    if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) {
        /* it's in the table; we can continue */
        /* read the forwarding table entry */
        ftIndex = p3macPyld.ftidx;
        result = soc_sbx_g3p1_ft_get(unit, ftIndex, &p3ft);

        if (0 != p3ft.excidx) {
            /* FT entry was not valid */
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "p3ft was marked with exception %02X\n"),
                       p3ft.excidx));
            result = BCM_E_NOT_FOUND;
        } else if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_MCAST,
                      (BSL_META_U(unit,
                                  "unable to read %d:ft[%08X]: %d (%s)\n"),
                       unit,
                       ftIndex,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else { /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "unable to find mcast group: %d (%s)\n"),
                   result,
                   _SHR_ERRMSG(result)));
    } /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */

    return result;
}

/*
 *   Function
 *      bcm_caladan3_mcast_addr_add_w_l2mcindex
 *   Purpose
 *      Adds a multicast address to the hardware tables, using l2mcindex
 *      provided by the caller.
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address to add
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Makes a call to _bcm_caladan3_mcast_addr_add, setting the flags to STATIC
 *      so the entry is not automatically aged.
 */
int
bcm_caladan3_mcast_addr_add_w_l2mcindex(int unit,
                                        bcm_mcast_addr_t *mcAddr)
{
    return _bcm_caladan3_mcast_addr_add(unit,
                                        mcAddr,
                                        BCM_L2_STATIC,
                                        (BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX |
                                         BCM_CALADAN3_MCAST_ADD_INCLUDE_ARG),
                                        NULL,
                                        NULL);
}

/*
 *   Function
 *      bcm_caladan3_mcast_addr_add
 *   Purpose
 *      Adds a multicast address to the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address to add
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Makes a call to _bcm_caladan3_mcast_addr_add, setting the flags to STATIC
 *      so the entry is not automatically aged.
 */
int
bcm_caladan3_mcast_addr_add(int unit,
                            bcm_mcast_addr_t *mcAddr)
{
    return _bcm_caladan3_mcast_addr_add(unit,
                                        mcAddr,
                                        BCM_L2_STATIC,
                                        (BCM_CALADAN3_MCAST_ADD_AUTO_L2MCIDX |
                                         BCM_CALADAN3_MCAST_ADD_INCLUDE_ARG),
                                        NULL,
                                        NULL);
}

/*
 *   Function
 *      bcm_caladan3_mcast_addr_remove_w_l2mcindex
 *   Purpose
 *      Adds a multicast address to the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address to add
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Removes the address only if the provided l2mc_index matches the actual
 *      one in the existing entry.
 */
int
bcm_caladan3_mcast_addr_remove_w_l2mcindex(int unit,
                                           bcm_mcast_addr_t *mcAddr)
{
    if (mcAddr) {
        return _bcm_caladan3_mcast_addr_remove(unit,
                                               mcAddr->mac,
                                               mcAddr->vid,
                                               mcAddr->l2mc_index,
                                               BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX);
    } else {
        return BCM_E_PARAM;
    }
}

/*
 *   Function
 *      bcm_caladan3_mcast_addr_remove
 *   Purpose
 *      Removes a multicast address from the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) sal_mac_addr_t mac = MAC address
 *      (in) bcm_vlan_t vid = VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_mcast_addr_remove(int unit,
                               sal_mac_addr_t mac,
                               bcm_vlan_t vid)
{
    return _bcm_caladan3_mcast_addr_remove(unit,
                                           mac,
                                           vid,
                                           0,
                                           BCM_CALADAN3_MCAST_ADD_AUTO_L2MCIDX);
}

/*
 *   Function
 *      bcm_caladan3_mcast_port_get
 *   Purpose
 *      Gets ports in the multicast group
 *   Parameters
 *      (in) int unit = unit number
 *      (in) sal_mac_addr_t mac = MAC address
 *      (in) bcm_vlan_t vid = VID
 *      (out) bcm_mcast_addr_t *mcAddr = information about the address & ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Derives untagged bitmap from VLAN untagged bitmap ANDed with the mcast
 *      port bitmap.
 */
int
bcm_caladan3_mcast_port_get(int unit,
                            sal_mac_addr_t mac,
                            bcm_vlan_t vid,
                            bcm_mcast_addr_t *mcAddr)
{
    soc_sbx_g3p1_mac_t      p3macPyld;  /* MAC payload */
    soc_sbx_g3p1_ft_t       p3ft;        /* FT (multicast) entry */
    unsigned int            ftIndex = ~0;/* FT (multicast) entry index */
    unsigned int            groupIndex;  /* mc group index */
    int                     result;      /* result code to calling function */
    int                     vsi_max;
    bcm_pbmp_t              vbmp;        /* VLAN port bitmap */
    bcm_pbmp_t              ubmp;        /* VLAN untagged port bitmap */

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* Check other parameter validity */
    if (!mcAddr) {
        return BCM_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "MCast %d: get ports 0x%03X/" L2_6B_MAC_FMT"\n"),
                 unit, vid, L2_6B_MAC_PFMT(mac)));

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= vid) || (vsi_max <= vid)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_port_get: %03X is not a valid VID\n"),
                   vid));
        return BCM_E_PARAM;
    }

    /* get the VLAN port information */
    result = _bcm_caladan3_vlan_port_fetch(unit,
                                           vid,
                                           &vbmp,
                                           &ubmp);
    if (BCM_E_NONE != result) {
        /* something went wrong getting ports; assume no ports */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_vlan_port_fetch(*,%d,%03X,*,*)"
                               " returned %d (%s)\n"),
                   unit,
                   vid,
                   result,
                   _SHR_ERRMSG(result)));
        BCM_PBMP_CLEAR(ubmp);
        BCM_PBMP_CLEAR(vbmp);
    }
    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "_bcm_caladan3_vlan_port_fetch(*,%d,%03X,*,*)"
                             " returned %d (%s)\n"
                             "  pbmp = %08X\n"
                             "  ubmp = %08X\n"),
                 unit,
                 vid,
                 result,
                 _SHR_ERRMSG(result),
                 _SHR_PBMP_WORD_GET(vbmp, 0),
                 _SHR_PBMP_WORD_GET(ubmp, 0)));

    /* hope for success */
    result = BCM_E_NONE;

    /* set up the result space */
    bcm_mcast_addr_init(mcAddr, mac, vid);

    /* search for the entry to see if it already exists */
    result = soc_sbx_g3p1_mac_get(unit, mac, vid, &p3macPyld);
    if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) {
        /* it's in the table; we can continue */
        /* read the forwarding table entry */
        ftIndex = p3macPyld.ftidx;
        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                    (BSL_META_U(unit,
                                "macPayload indicates FT index is %08X\n"),
                     ftIndex));
        result = soc_sbx_g3p1_ft_get(unit, ftIndex, &p3ft);
        if ((BCM_E_NONE == result) && (0 == p3ft.excidx)) {
            /* got the forwarding table entry; get the mcGroup */
            groupIndex = p3ft.oi;
            LOG_VERBOSE(BSL_LS_BCM_MCAST,
                        (BSL_META_U(unit,
                                    "ftEntry indicates mcGroup index is %08X\n"),
                         groupIndex));
            mcAddr->l2mc_index = groupIndex;
            mcAddr->distribution_class = (p3ft.qid - MCAST_QID_BASE) /
                SBX_MAX_COS;
        } else { /* if ((BCM_E_NONE == result) && (0 == p3ft.excidx)) */
            if (BCM_E_NONE == result) {
                /* FT entry was not valid */
                LOG_ERROR(BSL_LS_BCM_MCAST,
                          (BSL_META_U(unit,
                                      "p3ft was marked with exception %02X\n"),
                           p3ft.excidx));
                result = BCM_E_NOT_FOUND;
            }
        } /* if ((SB_OK == rc) && (ftEntry.ulValid)) */
    } else { /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "unable to find mcast group: %d (%s)\n"),
                   result,
                   _SHR_ERRMSG(result)));
    } /* if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) */

    return result;
}

/*
 *   Function
 *      bcm_caladan3_mcast_leave
 *   Purpose
 *      Remove a single port from the mcast group
 *   Parameters
 *      (in) int unit = unit number
 *      (in) sal_mac_addr_t mac = MAC address
 *      (in) bcm_vlan_t vid = VID
 *      (in) bcm_port_t port = port to leave the group
 *   Returns
 *      int = BCM_MCAST_LEAVE_UPDATED if removed from existing group
 *            BCM_MCAST_LEAVE_NOTFOUND if no existing group
 *            BCM_MCAST_LEAVE_DELETED if existing group removed
 *            BCM_E_* appropriately if failure
 *   Notes
 *      Prepares an bcm_mcast_addr_t and calls bcm_caladan3_mcast_port_remove.
 *      This function does not automatically delete the group if everybody has
 *      left because that would cause consistency issues on this platform at
 *      this time (some time in the future, an architecture where that is not
 *      the case is planned, but it's not just this file that causes the
 *      consistency problem).
 *      Never actually returns BCM_E_NONE!
 *      This implementation does not remove groups; the application must do
 *      this because it requires coordination of a global resource.
 */
int
bcm_caladan3_mcast_leave(int unit,
                         bcm_mac_t mac,
                         bcm_vlan_t vlan,
                         bcm_port_t port)
{
    bcm_mcast_addr_t          mcAddr;      /* working mc address */
    int                       result;      /* result to caller (maybe?) */
    int                       vsi_max;

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    if ((!BCM_PBMP_MEMBER(PBMP_ALL(unit),port))) {
        /* something is wrong here */
        return BCM_E_PARAM;
    }

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= vlan) || (vsi_max <= vlan)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_leave: %03X is not a valid VID\n"),
                   vlan));
        return BCM_E_PARAM;
    }

    /* okay, looks good; build request for port remove */
    bcm_mcast_addr_t_init(&mcAddr, mac, vlan);
    BCM_PBMP_PORT_ADD(mcAddr.pbmp,port);

    /* now get the port remove result */
    result = bcm_caladan3_mcast_port_remove(unit, &mcAddr);

    /* this has a special return code...! */
    if (BCM_E_NONE == result) {
        return BCM_MCAST_LEAVE_UPDATED;
    } else if (BCM_E_NOT_FOUND == result) {
        return BCM_MCAST_LEAVE_NOTFOUND;
    } else {
        return result;
    }
}

/*
 *   Function
 *      bcm_caladan3_mcast_join
 *   Purpose
 *      Add a single port from the mcast group
 *   Parameters
 *      (in) int unit = unit number
 *      (in) sal_mac_addr_t mac = MAC address
 *      (in) bcm_vlan_t vid = VID
 *      (in) bcm_port_t port = port to join the group
 *      (out) bcm_mcast_addr_t *mcAddr = ptr to where to put addr data
 *      (out) bcm_pbmp_t *all_router_pbmp = all_routers pbmp for the VLAN
 *   Returns
 *      int = BCM_MCAST_JOIN_UPDATED if updated existing group
 *            BCM_MCAST_JOIN_ADDED if added a new group
 *            BCM_E_* appropriately if failure
 *
 *   Notes
 *      May add the group if it's not already present.  Will add the port
 *      according to the proper VLAN tagging mode for the port.
 *      Never actually returns BCM_E_NONE!
 */
int
bcm_caladan3_mcast_join(int unit,
                        bcm_mac_t mac,
                        bcm_vlan_t vlan,
                        bcm_port_t port,
                        bcm_mcast_addr_t *mcAddr,
                        bcm_pbmp_t *all_router_pbmp)
{
    soc_sbx_g3p1_mac_t        p3macPyld;   /* MAC payload */
    int                       result;      /* result to caller (maybe?) */
    int                       vsi_max;
    int                       joinStat = BCM_E_UNAVAIL;
    /* result to caller (maybe?) */
    bcm_mac_t                 arMac;       /* working all-routers MAC address */
    bcm_mcast_addr_t          arData;      /* working all-routers MAC data */
    bcm_pbmp_t                pbmp;        /* working port bitmap */
    bcm_pbmp_t                vbmp;        /* ports in this VLAN */
    bcm_pbmp_t                ubmp;        /* untagged ports in this VLAN */

    /* Check unit valid; return unit error if not */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    if ((!BCM_PBMP_MEMBER(PBMP_ALL(unit),port)) ||
        (!mcAddr) ||
        (!all_router_pbmp)) {
        /* something is wrong here */
        return BCM_E_PARAM;
    }

    /* check address */
    MCAST_RETURN_IF_NOT_MCAST_ADDR(mac);

    /* check vid */
    vsi_max = soc_sbx_g3p1_v2e_table_size_get(unit);
    if ((BCM_VLAN_MIN >= vlan) || (vsi_max <= vlan)) {
        /* the vid is not valid */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_join: %03X is not a valid VID\n"),
                   vlan));
        return BCM_E_PARAM;
    }

    /* get the VLAN port information */
    result = _bcm_caladan3_vlan_port_fetch(unit,
                                           vlan,
                                           &vbmp,
                                           &ubmp);
    LOG_VERBOSE(BSL_LS_BCM_MCAST,
                (BSL_META_U(unit,
                            "_bcm_caladan3_vlan_port_fetch(*,%d,%03X,*,*)"
                             " returned %d (%s)\n"
                             "  pbmp = %08X\n"
                             "  ubmp = %08X\n"),
                 unit,
                 vlan,
                 result,
                 _SHR_ERRMSG(result),
                 _SHR_PBMP_WORD_GET(vbmp, 0),
                 _SHR_PBMP_WORD_GET(ubmp, 0)));

    /* build request for mcast port add */
    bcm_mcast_addr_t_init(mcAddr, mac, vlan);
    BCM_PBMP_PORT_ADD(mcAddr->pbmp, port);
    BCM_PBMP_ASSIGN(mcAddr->ubmp, ubmp);
    BCM_PBMP_AND(mcAddr->ubmp,  mcAddr->pbmp);

    /* verify that the port is in the VLAN */
    BCM_PBMP_ASSIGN(pbmp, mcAddr->pbmp);
    BCM_PBMP_AND(pbmp, vbmp);
    if (!BCM_PBMP_EQ(pbmp, mcAddr->pbmp)) {
        /* the port isn't in the VLAN */
        LOG_ERROR(BSL_LS_BCM_MCAST,
                  (BSL_META_U(unit,
                              "bcm_caladan3_mcast_join: port is not in the vlan\n")));
        return BCM_E_PARAM;
    }


    /* build the search/index key for the dest MAC table */
    result = soc_sbx_g3p1_mac_get(unit, mac, vlan, &p3macPyld);
    if ((BCM_E_NONE == result) || (BCM_E_EXISTS == result)) {
        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                    (BSL_META_U(unit,
                                "bcm_caladan3_mcast_join: group exists; adding port\n")));
        /* the group already exists */
        joinStat = BCM_MCAST_JOIN_UPDATED;

        /* try the port add operation */
        result = bcm_caladan3_mcast_port_add(unit, mcAddr);
    } else {
        LOG_VERBOSE(BSL_LS_BCM_MCAST,
                    (BSL_META_U(unit,
                                "bcm_caladan3_mcast_join: group does not exist; adding group\n")));

        /* we're going to add the group */
        joinStat = BCM_MCAST_JOIN_ADDED;

        /* attempt to add the new MC group */
        result = _bcm_caladan3_mcast_addr_add(unit,
                                              mcAddr,
                                              BCM_L2_STATIC,
                                              BCM_CALADAN3_MCAST_ADD_AUTO_L2MCIDX |
                                              BCM_CALADAN3_MCAST_ADD_INCLUDE_ARG,
                                              NULL,
                                              NULL);
    }

    /* if that worked, get the mc group information */
    if (BCM_E_NONE == result) {
        /* get the current info for the given group */
        result = bcm_caladan3_mcast_port_get(unit, mac, vlan, mcAddr);
    }

    /* if all is well now, try to fill in the all-routers group pbmp */
    if (BCM_E_NONE == result) {
        /* get the all-routers group pbmp */
        sal_memcpy(&arMac, &_soc_mac_all_routers, sizeof(sal_mac_addr_t));
        bcm_mcast_addr_t_init(&arData, arMac, vlan);
        if (BCM_E_NONE == bcm_caladan3_mcast_port_get(unit,
                                                      arMac,
                                                      vlan,
                                                      &arData)) {
            /* got it; copy it to output data */
            BCM_PBMP_ASSIGN(*all_router_pbmp, arData.pbmp);
        } else {
            /* didn't get it; just assume it's no ports */
            BCM_PBMP_CLEAR(*all_router_pbmp);
        }
    }

    /* this has a special return code...! */
    if (BCM_E_NONE == result) {
        return joinStat;
    } else {
        return result;
    }
}
