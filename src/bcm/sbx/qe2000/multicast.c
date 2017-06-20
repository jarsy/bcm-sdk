/*
 * $Id: multicast.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Multicast API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/qe2000.h>

#include <bcm/error.h>
#include <bcm/mcast.h>
#include <bcm/multicast.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>



static int
bcm_qe2000_get_internal_type(int unit, int type, int *internal_type)
{
    int rv = BCM_E_NONE;


    type = (type & BCM_MULTICAST_TYPE_MASK) & ~BCM_MULTICAST_TYPE_SUBPORT;

    switch (type) {
        case BCM_MULTICAST_TYPE_L2:
            (*internal_type) = SBX_MVT_TYPE_VID;
            break;

        case BCM_MULTICAST_TYPE_L3:
        case BCM_MULTICAST_TYPE_VPLS:
        case BCM_MULTICAST_TYPE_MIM:
            (*internal_type) = SBX_MVBT_TYPE_OHI;
            break;

        default:
            return(BCM_E_PARAM);
            break;
    }

    return(rv);
}

static void
_bcm_qe2000_get_encap_id(int unit, sbx_qe2000_mvt_entry_t *mvtEntry, bcm_if_t *encap_id)
{

    if (SOC_SBX_CFG_QE2000(unit)->bEpDisable == FALSE) {

        switch (SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat) {
            case SBX_MVT_FORMAT0:
            default:
                if (SBX_MVT_IS_LI(mvtEntry))
                    SBX_MVT_GET_LI_OHI(mvtEntry, (*encap_id));
                else
                    SBX_MVT_GET_VID(mvtEntry, (*encap_id));

                break;

            case SBX_MVT_FORMAT1:
                SBX_MVT_GET_EP_DISABLED(mvtEntry, (*encap_id));
                break;
        }
    }
    else {
        SBX_MVT_GET_EP_DISABLED(mvtEntry, (*encap_id));
    }

    return;
}


static void
_bcm_qe2000_set_encap_id(int unit, sbx_qe2000_mvt_entry_t *mvtEntry, bcm_if_t encap_id)
{

    if (SOC_SBX_CFG_QE2000(unit)->bEpDisable == FALSE) {
        
        switch (SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat) {
            case SBX_MVT_FORMAT0:
            default:
                if (BCM_VLAN_VALID(encap_id)) {
                    SBX_MVT_SET_TB(mvtEntry, encap_id);
                } else {
                    SOC_SBX_OHI_FROM_ANY_ENCAP_ID(encap_id, encap_id);
                    SBX_MVT_SET_LI_OHI(mvtEntry, encap_id);
                }

                break;

            case SBX_MVT_FORMAT1:
                SBX_MVT_SET_EP_DISABLED(mvtEntry, encap_id);
        }
    }
    else {
        SBX_MVT_SET_EP_DISABLED(mvtEntry, encap_id);
    }

    return;
}


int
bcm_qe2000_multicast_create(int unit,
                            uint32 flags,
                            bcm_multicast_t *group)
{
    int rv = BCM_E_NONE;
    sbx_mvt_id_t entry_id;
    int mvt_id_block;
    sbx_qe2000_mvt_entry_t mvtEntry;
    int type, internal_type = 0;


    /* consistency checks */
    if (group == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, NULL group parameter, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_PARAM);
    }
    type = (flags & BCM_MULTICAST_TYPE_MASK) & ~BCM_MULTICAST_TYPE_SUBPORT;
    if ( ((flags & ~(BCM_MULTICAST_TYPE_MASK | BCM_MULTICAST_WITH_ID | BCM_MULTICAST_DISABLE_SRC_KNOCKOUT)) != 0) ||
            (type == 0) || ( (type != BCM_MULTICAST_TYPE_L2) &&
                (type != BCM_MULTICAST_TYPE_L3) && (type != BCM_MULTICAST_TYPE_VPLS) &&
		 (type != BCM_MULTICAST_TYPE_MIM)) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid flag parameter, Unit(%d), flags(0x%x)\n"),
                   FUNCTION_NAME(), unit, flags));
        return(BCM_E_PARAM);
    }

    /* If we are in EASY_RELOAD and we are reloading, we should never create a multicast group */
    /* without ID                                                                              */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	if ((flags & BCM_MULTICAST_WITH_ID)==0) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Easy Reload - when reloading, all mcgroups must be added with flag BCM_MULTICAST_WITH_ID\n")));
	    return (BCM_E_PARAM);
	}
#endif
    }

    if (flags & BCM_MULTICAST_WITH_ID) {
        if (!soc_qe2000_mvt_entry_id_valid(unit, (*group))) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                       FUNCTION_NAME(), unit, *group));
            return(BCM_E_PARAM);
        }

        entry_id = (*group);
        mvt_id_block = ((*group) <= SBX_MVT_ID_VSI_END) ? SBX_MVT_ID_VLAN : SBX_MVT_ID_GLOBAL;
        rv = soc_qe2000_mvt_entry_reserve_id(unit, 1, &entry_id);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_reserve_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            return(rv);
        }
    }
    else {
        mvt_id_block = SBX_MVT_ID_GLOBAL;
        rv = soc_qe2000_mvt_entry_allocate(unit, 1, &entry_id, mvt_id_block);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_allocate, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            return(rv);
        }
        (*group) = entry_id;
    }

    /* setup the McGroup entry */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_id, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    mvtEntry.source_knockout = (flags & BCM_MULTICAST_DISABLE_SRC_KNOCKOUT) ? 0 : 1;
    if (mvt_id_block == SBX_MVT_ID_VLAN) {
        SBX_MVT_SET_TB(&mvtEntry, entry_id);
    }
    else {
        rv = bcm_qe2000_get_internal_type(unit, type, &internal_type);
        if (rv) {
            return rv;
        }
        SBX_MVT_SET_NOT_USED(&mvtEntry);
    }
    mvtEntry.next = SBX_MVT_ID_NULL;
    SOC_PBMP_CLEAR(mvtEntry.ports);

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_id, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    return(rv);

err:
    soc_qe2000_mvt_entry_free_frm_id(unit, entry_id);
    return(rv);
}

int
bcm_qe2000_multicast_group_get(int unit,
                               bcm_multicast_t group,
                               uint32 *flags)
{
    int result;
    uint32 working = BCM_MULTICAST_WITH_ID;

    /* consistency checks */
    if (!flags) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s obligatory out argument is NULL\n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return BCM_E_PARAM;
    }
    result = soc_qe2000_mvt_entry_chk_frm_id(unit, group);
    if (SOC_E_NONE == result) {
        
        if (group <= SBX_MVT_ID_VSI_END) {
            /* VLAN space, claim it's L2 */
            working |= BCM_MULTICAST_TYPE_L2;
        } else {
            /* logical, claim it's L3 */
            working |= BCM_MULTICAST_TYPE_L3;
        }
        *flags = working;
    }
    return result;
}

int
bcm_qe2000_multicast_group_traverse(int unit,
                                    bcm_multicast_group_traverse_cb_t cb,
                                    uint32 flags,
                                    void *user_data)
{
    bcm_multicast_t gid;
    uint32 tempFlags;
    int result = BCM_E_NONE;

    /* consistency checks */
    if (!flags) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s callback function may not be NULL\n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    for (gid = SBX_MVT_ID_VSI_BASE;
         (BCM_E_NONE == result) &&
         (gid < SOC_SBX_CFG(unit)->mcgroup_local_start_index);
         gid++) {
        result = bcm_qe2000_multicast_group_get(unit, gid, &tempFlags);
        if (BCM_E_NONE == result) {
            if ((flags & BCM_MULTICAST_TYPE_MASK) & tempFlags) {
                /* at least one of the requested 'type' bits is set */
                result = cb(unit, gid, tempFlags, user_data);
            } /* if ((flags & BCM_MULTICAST_TYPE_MASK) & tempFlags) */
        } else if (BCM_E_NOT_FOUND == result) {
            /* no group here; just go to next one */
            result = BCM_E_NONE;
        }
    } /* for (all possible groups as long as everything goes well) */
    return result;
}

int
bcm_qe2000_multicast_destroy(int unit,
                             bcm_multicast_t group)
{
    int rv = BCM_E_NONE;
    sbx_qe2000_mvt_entry_t mvtEntry;
    int i, entry_index;


    /* consistency checks */
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }

    for (i = 0, entry_index = group;
                     (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {

        /* retreive entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        if (entry_index == group) {
            /* check if the entry is for VLAN */
            if (group <= SBX_MVT_ID_VSI_END) {
                SOC_PBMP_CLEAR(mvtEntry.ports);
            }

            /* otherwise it is a global entry */
            else {
                SOC_PBMP_CLEAR(mvtEntry.ports);
                SBX_MVT_SET_NOT_USED(&mvtEntry);
            }
        }
        else {
            SOC_PBMP_CLEAR(mvtEntry.ports);
            SBX_MVT_SET_NOT_USED(&mvtEntry);
        }
        mvtEntry.next = SBX_MVT_ID_NULL;

        rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        soc_qe2000_mvt_entry_free_frm_id(unit, entry_index);

        entry_index = mvtEntry.next;
    }

    if (i >= SBX_MAX_MVT_CHAINED) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Inconsistency in MVT Chain, Unit(%d), Chain  length greater then: 0x%x\n"),
                   FUNCTION_NAME(), unit, SBX_MAX_MVT_CHAINED));
    }

    return(rv);

err:
    return(rv);
}

int
bcm_qe2000_multicast_egress_add(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    int rv = BCM_E_NONE;
    sbx_qe2000_mvt_entry_t mvtEntry, LocalMvtEntry;
    int module_id, num_port, mvt_port;
    int i;
    sbx_mvt_id_t  entry_index, last_entry_index = 0;
    int cfg_encap_id, hw_encap_id;
    int found = FALSE, mvt_entry_alloc = FALSE;
    int base_source_knockout = 0;

    /* consistency checks */
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }
    if (!BCM_GPORT_IS_MODPORT(port)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid gport type, Unit(%d), gport(0x%x)\n"),
                   FUNCTION_NAME(), unit, port));
        return(BCM_E_PARAM);
    }

    /* check if the request is for the current unit */
    bcm_qe2000_stk_modid_get(unit, &module_id);
    if (BCM_GPORT_MODPORT_MODID_GET(port) != module_id) {
        return(rv);
    }

    num_port = BCM_GPORT_MODPORT_PORT_GET(port);
    if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                   (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
        mvt_port = num_port - SOC_PORT_MIN(unit, spi_subport);
    }
    else if (num_port == CMIC_PORT(unit)) {
        mvt_port = SB_FAB_DEVICE_QE2000_CPU_PORT;
    }
    else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid port, Unit(%d), port(0x%x)\n"),
                   FUNCTION_NAME(), unit, num_port));
        return(BCM_E_PARAM);
    }

   /* Search chain for matching encap_id or for free entry */
    for (i = 0, entry_index = group; (found == FALSE) &&
                     (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {
        /* retrieve entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        /* convert the encap ID to something meaningful for comparison
         * against an MVT entry
         */
        SOC_SBX_OHI_FROM_ANY_ENCAP_ID(hw_encap_id, encap_id);

	/* If we are reloading, read the entry from the hardware, if it is not invalid, reserve it  */
	/* If the entry is a valid encap id (non-zero), then the entry was allocated before we were */
	/* reloading and we should update the software state to match the hardware state.           */
#ifdef BCM_EASY_RELOAD_SUPPORT
	if (SOC_IS_RELOADING(unit)) {

	    if (SBX_MVT_INVALID(&mvtEntry) == 0) {

		rv = soc_qe2000_mvt_entry_reserve_id(unit, 1, &entry_index);

		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, reserving entry for soc_qe2000_mvt_entry_reserve_id, Unit(%d), Err: 0x%x\n"),
		               FUNCTION_NAME(), unit, rv));
		    return(rv);
		}
	    }
	}
#endif /* BCM_EASY_RELOAD_SUPPORT */

        if (i == 0) {
            base_source_knockout = mvtEntry.source_knockout;
        }

        if (SBX_MVT_INVALID(&mvtEntry)) {
            _bcm_qe2000_set_encap_id(unit, &mvtEntry, encap_id);
            found = TRUE;
        }
        else {
            _bcm_qe2000_get_encap_id(unit, &mvtEntry, &cfg_encap_id);
            if (cfg_encap_id == hw_encap_id)
                found = TRUE;
        }

        if (found != TRUE) {
            last_entry_index = entry_index;
            entry_index = mvtEntry.next;
        }

    }

    /* check for error condition */
    if ( (found != TRUE) && (entry_index != SBX_MVT_ID_NULL) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Inconsistency in MVT Chain, Unit(%d), Chain  length greater then: 0x%x\n"),
                   FUNCTION_NAME(), unit, SBX_MAX_MVT_CHAINED));
        rv = BCM_E_INTERNAL;
        goto err;
    }

    if (i >= SBX_MAX_MVT_CHAINED) {
        rv = BCM_E_RESOURCE;
        goto err;
    }

    /* If we are reloading and we do not find the entry, return an error, there was a problem during playback of the commands */
	if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	    if (found != TRUE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Inconsistency in MVT Chain, Unit(%d), EASY_RELOAD, reloading but chain MVT entry not found: 0x%x\n"),
		           FUNCTION_NAME(), unit, SBX_MAX_MVT_CHAINED));
		rv = BCM_E_INTERNAL;
		goto err;
	    }
#endif /* BCM_EASY_RELOAD_SUPPORT */
	}

    /* check if MVT entry needs to be allocated */
    if (entry_index == SBX_MVT_ID_NULL) {
        /* allocate Local MVT entry */
        rv = soc_qe2000_mvt_entry_allocate(unit, 1, &entry_index, SBX_MVT_ID_LOCAL);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_allocate, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            return(rv);
        }
        mvt_entry_alloc = TRUE;

        /* initialize the entry */
        LocalMvtEntry.source_knockout = base_source_knockout;
        _bcm_qe2000_set_encap_id(unit, &LocalMvtEntry, encap_id);

        LocalMvtEntry.next = SBX_MVT_ID_NULL;
        SOC_PBMP_CLEAR(LocalMvtEntry.ports);
        SOC_PBMP_PORT_ADD(LocalMvtEntry.ports, mvt_port);

        rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &LocalMvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        /* chain entry */
        mvtEntry.next = entry_index;
        rv = soc_qe2000_mvt_entry_set_frm_id(unit, last_entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    else {
        /* update MVT entry */
        SOC_PBMP_PORT_ADD(mvtEntry.ports, mvt_port);
        rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    return(rv);

err:
    if (mvt_entry_alloc == TRUE)
        soc_qe2000_mvt_entry_free_frm_id(unit, entry_index);
    return(rv);
}

int
bcm_qe2000_multicast_egress_delete(int unit,
                                   bcm_multicast_t group,
                                   bcm_gport_t port,
                                   bcm_if_t encap_id)
{
    int rv = BCM_E_NONE;
    int module_id, num_port, mvt_port;
    sbx_mvt_id_t  entry_index, previous_entry_index = 0, next_entry_index = 0;
    sbx_mvt_id_t  remove_index = SBX_MVT_ID_NULL;
    sbx_qe2000_mvt_entry_t mvtEntry, previousMvtEntry;
    int i;
    int cfg_encap_id;
    int ohi;
    int found = FALSE;


    /* consistency checks */
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }
    if (!BCM_GPORT_IS_MODPORT(port)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid gport type, Unit(%d), gport(0x%x)\n"),
                   FUNCTION_NAME(), unit, port));
        return(BCM_E_PARAM);
    }

    /* check if the request is for the current unit */
    bcm_qe2000_stk_modid_get(unit, &module_id);
    if (BCM_GPORT_MODPORT_MODID_GET(port) != module_id) {
        return(rv);
    }

    num_port = BCM_GPORT_MODPORT_PORT_GET(port);
    if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                   (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
        mvt_port = num_port - SOC_PORT_MIN(unit, spi_subport);
    }
    else if (num_port == CMIC_PORT(unit)) {
        mvt_port = SB_FAB_DEVICE_QE2000_CPU_PORT;
    }
    else {
        return(BCM_E_PARAM);
    }

    SOC_SBX_OHI_FROM_ANY_ENCAP_ID(ohi, encap_id);

    for (i = 0, entry_index = group; (found == FALSE) &&
                     (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {
        /* retreive entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        if (!SBX_MVT_INVALID(&mvtEntry)) {
            _bcm_qe2000_get_encap_id(unit, &mvtEntry, &cfg_encap_id);
            if (cfg_encap_id == ohi)
                found = TRUE;
        }

        if (found != TRUE) {
            previous_entry_index = entry_index;
            entry_index = mvtEntry.next;
        }

    }

    /* check for error condition */
    if (found != TRUE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, MVT entry not found, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rv = BCM_E_NOT_FOUND;
        goto err;
    }

    /* update MVT entry */
    next_entry_index = mvtEntry.next;
    SOC_PBMP_PORT_REMOVE(mvtEntry.ports, mvt_port);

    if (SOC_PBMP_IS_NULL(mvtEntry.ports)) {

        if (entry_index == group && mvtEntry.next != SBX_MVT_ID_NULL) { 
            sbx_qe2000_mvt_entry_t mvtNextEntry;

            /* this is the head (global) entry, and there are no ports left
             * in this mvt.  Copy the next entry here, and remove 'the next'
             * entry; keeping the head in tact
             */
            rv = soc_qe2000_mvt_entry_get_frm_id(unit, mvtEntry.next, 
                                                 &mvtNextEntry);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, "
                                       "Unit(%d), Err: 0x%x\n"), FUNCTION_NAME(), unit, rv));
                goto err;
            }

            next_entry_index = mvtNextEntry.next;
            remove_index = mvtEntry.next; 

            sal_memcpy(&mvtEntry, &mvtNextEntry, sizeof(mvtEntry));

            /* mvt_entry written below */
            
        } else if (entry_index > SBX_MVT_ID_VSI_END) {
            SBX_MVT_SET_NOT_USED(&mvtEntry);
            mvtEntry.next = SBX_MVT_ID_NULL;

            /* caller must use multicast_destroy to free the group */
            if (entry_index != group) {
                remove_index = entry_index;
            }
        }
    }

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    /* remove and update links, if necessary */    
    if (remove_index != SBX_MVT_ID_NULL) {
        int tmp;
  
        if (previous_entry_index != 0) {
            /* retreive the previous entry */
            rv = soc_qe2000_mvt_entry_get_frm_id(unit, previous_entry_index,
                                                 &previousMvtEntry);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d),"
                                       " Err: 0x%x\n"), FUNCTION_NAME(), unit, rv));
                goto err;
            }
             
            /* update mvt chain */
            previousMvtEntry.next = next_entry_index;
            rv = soc_qe2000_mvt_entry_set_frm_id(unit, previous_entry_index,
                                                 &previousMvtEntry);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), "
                                       "Err: 0x%x\n"), FUNCTION_NAME(), unit, rv));
                goto err;
            }
        }
 
        sal_memset(&mvtEntry, 0, sizeof(mvtEntry));
        mvtEntry.next = SBX_MVT_ID_NULL;
         
        tmp = soc_qe2000_mvt_entry_set_frm_id(unit, remove_index, &mvtEntry);
        if (tmp != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), "
                                  "Err: 0x%x\n"), FUNCTION_NAME(), unit,tmp));
        }
         
        tmp = soc_qe2000_mvt_entry_free_frm_id(unit, remove_index);
        if (tmp != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: %s, soc_qe2000_mvt_entry_free_frm_id, Unit(%d), "
                                  "Err: 0x%x\n"), FUNCTION_NAME(), unit, tmp));
        }
    }

err:
    return(rv);
}

int
bcm_qe2000_multicast_egress_delete_all(int unit,
                                bcm_multicast_t group)
{
    int rv = BCM_E_NONE;
    int i, entry_index, next_entry_index;
    sbx_qe2000_mvt_entry_t mvtEntry;


    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }

    /* Delete Current Egress onfiguration */
    for (i = 0, entry_index = group; (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {
        /* retreive entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
             LOG_ERROR(BSL_LS_BCM_COMMON,
                       (BSL_META_U(unit,
                                   "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                        FUNCTION_NAME(), unit, rv));
            goto err;
        }

        /* configuration created via "*_multicast_create" needs to be retained */
        if (entry_index == group) {
            if (group <= SBX_MVT_ID_VSI_END) {
            }
            else {
                SBX_MVT_SET_NOT_USED(&mvtEntry);
            }
        }
        else {
            SBX_MVT_SET_NOT_USED(&mvtEntry);
        }

        next_entry_index = mvtEntry.next;

        mvtEntry.next = SBX_MVT_ID_NULL;
        SOC_PBMP_CLEAR(mvtEntry.ports);

        rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        if (entry_index == group) {
        }
        else {
            /* free entry */
            soc_qe2000_mvt_entry_free_frm_id(unit, entry_index);
        }

        entry_index = next_entry_index;
    }

    return(rv);

err:
    return(rv);
}


int
bcm_qe2000_multicast_egress_set(int unit,
                                bcm_multicast_t group,
                                int port_count,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array)
{
    int rv = BCM_E_NONE;
    int i;


    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }

    /* Delete Current Egress onfiguration */
    rv = bcm_qe2000_multicast_egress_delete_all(unit, group);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, bcm_qe2000_multicast_egress_delete_all, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    /* configure egress configuration */
    for (i = 0; i < port_count; i++) {
        rv = bcm_qe2000_multicast_egress_add(unit, group, *(port_array + i), *(encap_id_array + i));
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_qe2000_multicast_egress_add, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    return(rv);

err:
    return(rv);
}

int
bcm_qe2000_multicast_egress_get(int unit,
                                bcm_multicast_t group,
                                int port_max,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array,
                                int *port_count)
{
    int rv = BCM_E_NONE;
    int i, entry_index, mvt_port, port;
    sbx_qe2000_mvt_entry_t mvtEntry;
    int module_id;


    if (port_max < 0) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, invalid port_max parameter, Unit(%d), port_max: 0x%x\n"),
                   FUNCTION_NAME(), unit, port_max));
        return(BCM_E_PARAM);
    }
    if (port_max != 0 && port_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, port_array parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_PARAM);
    }
    if (port_max != 0 && encap_id_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, encap_id_array parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_PARAM);
    }
    if (port_count == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, port_count parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_PARAM);
    }
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        return(BCM_E_NOT_FOUND);
    }

    (*port_count) = 0;
    for (i = 0, entry_index = group; ((*port_count) < port_max) &&
                     (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {
        /* retreive entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
             LOG_ERROR(BSL_LS_BCM_COMMON,
                       (BSL_META_U(unit,
                                   "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                        FUNCTION_NAME(), unit, rv));
            goto err;
        }

        SOC_PBMP_ITER(mvtEntry.ports, mvt_port) {
            if (mvt_port == SB_FAB_DEVICE_QE2000_CPU_PORT) {
                port = CMIC_PORT(unit);
            }
            else {
                port = mvt_port + SOC_PORT_MIN(unit, spi_subport);
            }

            bcm_qe2000_stk_modid_get(unit, &module_id);
            BCM_GPORT_MODPORT_SET(*(port_array + (*port_count)), module_id, port);
            _bcm_qe2000_get_encap_id(unit, &mvtEntry, (encap_id_array + (*port_count)));

            (*port_count)++;

            if ((*port_count) >= port_max) {
                break;
            }
        }

        entry_index = mvtEntry.next;
    }

    if (i >= SBX_MAX_MVT_CHAINED) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Inconsistency in MVT Chain, Unit(%d), Chain  length greater then: 0x%x\n"),
                   FUNCTION_NAME(), unit, SBX_MAX_MVT_CHAINED));
    }

    return(rv);

err:
    return(rv);
}


int
bcm_qe2000_multicast_source_knockout_get(int unit,
                                         bcm_multicast_t group,
                                         int *source_knockout)
{
    int rv = BCM_E_NONE;
    sbx_qe2000_mvt_entry_t mvtEntry;


    /* consistency check */
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }

    /* retreive entry */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, group, &mvtEntry);
    if (rv != BCM_E_NONE) {
         LOG_ERROR(BSL_LS_BCM_COMMON,
                   (BSL_META_U(unit,
                               "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                    FUNCTION_NAME(), unit, rv));
        goto err;
    }

    /* update source knockout field */
    (*source_knockout) = (mvtEntry.source_knockout == TRUE) ? TRUE: FALSE;

    return(rv);

err:
    return(rv);

}

int
bcm_qe2000_multicast_source_knockout_set(int unit,
                                         bcm_multicast_t group,
                                         int source_knockout)
{
    int rv = BCM_E_NONE;
    int i, entry_index;
    sbx_qe2000_mvt_entry_t mvtEntry;


    /* consistency check */
    if ( (source_knockout != FALSE) && (source_knockout != TRUE) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid source knockout parameter, Unit(%d), SrcKnockout(0x%x)\n"),
                   FUNCTION_NAME(), unit, source_knockout));
        return(BCM_E_PARAM);
    }
    if (!soc_qe2000_mvt_entry_id_valid(unit, group)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid group parameter, Unit(%d), group(0x%x)\n"),
                   FUNCTION_NAME(), unit, group));
        return(BCM_E_PARAM);
    }

    for (i = 0, entry_index = group;
                (i < SBX_MAX_MVT_CHAINED) && (entry_index != SBX_MVT_ID_NULL); i++) {

        /* retrieve entry */
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
             LOG_ERROR(BSL_LS_BCM_COMMON,
                       (BSL_META_U(unit,
                                   "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: 0x%x\n"),
                        FUNCTION_NAME(), unit, rv));
            goto err;
        }

        /* update source knockout field */
        mvtEntry.source_knockout = (source_knockout == FALSE) ? FALSE : TRUE;

        /* write back entry */
        rv = soc_qe2000_mvt_entry_set_frm_id(unit, entry_index, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        entry_index = mvtEntry.next;
    }

    if (i >= SBX_MAX_MVT_CHAINED) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Inconsistency in MVT Chain, Unit(%d), Chain  length greater then: 0x%x\n"),
                   FUNCTION_NAME(), unit, SBX_MAX_MVT_CHAINED));
    }

    return(rv);

err:
    return(rv);
}


int
bcm_qe2000_multicast_state_get(int unit, char *pbuf)
{
    int rv = BCM_E_NONE;

    rv = soc_qe2000_mvt_state_get(unit, pbuf);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, soc_qe2000_mvt_state_get, Unit(%d), Err: %s\n"),
	           FUNCTION_NAME(), unit, bcm_errmsg(rv)));
    }


    return rv;
}
