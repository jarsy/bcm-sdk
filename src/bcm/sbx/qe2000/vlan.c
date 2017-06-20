/*
 * $Id: vlan.c,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Vlan API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/qe2000.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx/trunk.h>

#include <bcm/error.h>
#include <bcm/vlan.h>


/*
 *  This structure contains the local state information for this module.
 *
 *  It also contains a locking device, on a per unit basis, to prevent more
 *  than one concurrent access to the local data per unit.
 *
 *  Note that the 'standard' tag and strip ETEs do not have reference counters
 *  or other data (see _bcm_sbx_vlan_egr_trans_t) since they are allocated
 *  dynamically but held for the duration, and are used in all 'standard' cases
 *  (no translation applied).
 */
typedef struct _bcm_sbx_vlan_state_s {
    sal_mutex_t     lock;                   /* Used for locking this data */
    bcm_vlan_t      default_vid;            /* Default VID */
    bcm_pbmp_t      implicitly_default;     /* Implicitly in default VID */
    sbBool_t        allow_default_vid_deletion;
    unsigned int    vid_count;              /* Number of VIDs */
    bcm_vlan_data_t vid_list[BCM_VLAN_COUNT];
} _bcm_sbx_vlan_state_t;

static _bcm_sbx_vlan_state_t *sbx_vlan_state[BCM_MAX_NUM_UNITS];

static int _sbx_qe2000_vlan_support = TRUE;


/* ensures inter-module functions (e.g. trunk) are not invoked during module */
/* initialization ("_vlan_init()".                                           */
static int initialized[BCM_MAX_NUM_UNITS];


extern int running_simulator;

int
bcm_qe2000_vlan_control_vlan_set(int unit,
                                 bcm_vlan_t vlan,
                                 /* coverity[pass_by_value : FALSE] */
                                 bcm_vlan_control_vlan_t control)
{
    int rv = BCM_E_UNAVAIL;


    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_init
 */
int
bcm_qe2000_vlan_init(int unit)
{
    int         rv = BCM_E_NONE;
    int         vid;
    sbx_qe2000_mvt_entry_t mvtEntry;
    bcm_pbmp_t  pbmp;
    int         create_default_vlan = FALSE;
    bcm_vlan_t  default_vid = BCM_VLAN_DEFAULT;
    int         size;

    if (SAL_BOOT_BCMSIM) {
        running_simulator = 1;
    }

    initialized[unit] = FALSE;

    /* Allocate and initialize memory for soft state info */
    size = sizeof(_bcm_sbx_vlan_state_t);


    /* This code is added for EASY_RELOAD test purposes    */
    /* under normal circumstances during EASY_RELOAD, this */
    /* variable will be null                               */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        sbx_vlan_state[unit] = NULL;
#endif
    }

#ifdef BCM_WARM_BOOT_SUPPORT

	if (SOC_WARM_BOOT(unit))
	  goto warm_boot_skip_default_vlan;
#endif

    if (NULL == sbx_vlan_state[unit]) {
        sbx_vlan_state[unit] = sal_alloc(size, "sbx_qe_vlan_state");
        if (NULL == sbx_vlan_state[unit]) {
            rv = BCM_E_MEMORY;
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Cannot allocate memory, unit(%d), Err: %s (%d)\n"),
                       FUNCTION_NAME(), unit, bcm_errmsg(rv), rv));
            goto err;
        }
        sal_memset(sbx_vlan_state[unit], 0, size);
    }
    else {
        /* cleanup previous state */
        sbx_vlan_state[unit]->allow_default_vid_deletion = TRUE;

        for (vid = 0; vid <= SBX_MVT_ID_VSI_END; vid++) {
            if (sbx_vlan_state[unit]->vid_list[vid].vlan_tag == BCM_VLAN_NONE) {
                continue;
            }

            bcm_qe2000_vlan_destroy_internal(unit, sbx_vlan_state[unit]->vid_list[vid].vlan_tag, FALSE);
        }

        sal_memset(sbx_vlan_state[unit], 0, size);
    }



    for (vid = 0; vid <= SBX_MVT_ID_VSI_END; vid++) {
        rv = soc_qe2000_mvt_entry_get_frm_id(unit, vid, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: %d, Vid: %d\n"),
                       FUNCTION_NAME(), unit, rv, vid));
            goto err;
        }

        mvtEntry.source_knockout = 0x0;
        mvtEntry.egress_data_a = vid;
        mvtEntry.egress_data_b = 0x00;
        SBX_MVT_SET_TB(&mvtEntry, vid);
        mvtEntry.next = SBX_MVT_ID_NULL;

        rv = soc_qe2000_mvt_entry_set_frm_id(unit, vid, &mvtEntry);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: %d\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    SOC_PBMP_CLEAR(pbmp);

#ifndef BCM_VLAN_NO_DEFAULT_SPI_SUBPORT
    /* Initilize VLAN 1 to contain all SPI subports. */
    pbmp = PBMP_SPI_SUBPORT_ALL(unit);
#endif

#ifndef BCM_VLAN_NO_DEFAULT_CPU
    /* Initialize VLAN 1 to contain CPU port */
    SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
#endif

    rv = bcm_qe2000_vlan_create(unit, default_vid);
    if (rv != BCM_E_NONE) {
        goto err;
    }
    create_default_vlan = TRUE;

    rv = bcm_qe2000_vlan_port_add(unit, default_vid, pbmp, pbmp);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    if (BCM_E_NONE == rv) {
        sbx_vlan_state[unit]->default_vid = default_vid;
        sbx_vlan_state[unit]->vid_count = 1;
        sbx_vlan_state[unit]->vid_list[default_vid].vlan_tag = default_vid;
        BCM_PBMP_ASSIGN(sbx_vlan_state[unit]->vid_list[default_vid].port_bitmap, pbmp);
        BCM_PBMP_ASSIGN(sbx_vlan_state[unit]->vid_list[default_vid].ut_port_bitmap, pbmp);

        BCM_PBMP_CLEAR(sbx_vlan_state[unit]->implicitly_default);
    }
#ifdef BCM_WARM_BOOT_SUPPORT
 warm_boot_skip_default_vlan:
#endif
    initialized[unit] = TRUE;
    return(rv);

err:
    if ((sbx_vlan_state[unit] != NULL) && (create_default_vlan == TRUE)) {
        sbx_vlan_state[unit]->allow_default_vid_deletion = TRUE;
        bcm_qe2000_vlan_destroy_internal(unit, default_vid, FALSE);
    }
    if (sbx_vlan_state[unit] != NULL) {
        sal_free(sbx_vlan_state[unit]);
        sbx_vlan_state[unit] = NULL;
    }

    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_create
 */
int
bcm_qe2000_vlan_create(int unit,
                       bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;
    sbx_mvt_id_t mvt_id;
    sbx_qe2000_mvt_entry_t mvtEntry;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (vid > SBX_MVT_ID_VSI_END) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Vlan Id, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return(BCM_E_PARAM);
    }

    mvt_id = vid;
    rv = soc_qe2000_mvt_entry_reserve_id(unit, 1, &mvt_id);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_reserve_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        return(rv);
    }

    /* clear port membership */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    SOC_PBMP_CLEAR(mvtEntry.ports);

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    if (BCM_E_NONE == rv) {
        sbx_vlan_state[unit]->vid_count += 1;
        sbx_vlan_state[unit]->vid_list[vid].vlan_tag = vid;
    }

    return(rv);

err:
    soc_qe2000_mvt_entry_free_frm_id(unit, vid);
    return(rv);
}

int
bcm_qe2000_vlan_destroy_internal(int unit,
                                 bcm_vlan_t vid,
                                 int diags)
{
    int rv = BCM_E_NONE;
    sbx_mvt_id_t mvt_id;
    sbx_qe2000_mvt_entry_t mvtEntry;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (vid > SBX_MVT_ID_VSI_END) {
        if (diags) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Invalid Vlan Id, Unit(%d), VlanId: %d\n"),
                       FUNCTION_NAME(), unit, vid));
        }
        return(BCM_E_PARAM);
    }

    /* cannot delete default vlan */
    if ((vid == sbx_vlan_state[unit]->default_vid) &&
                               (sbx_vlan_state[unit]->allow_default_vid_deletion == FALSE)) {
        return(BCM_E_BADID);
    }

    mvt_id = vid;
    COMPILER_REFERENCE(mvt_id);

    rv = soc_qe2000_mvt_entry_free_frm_id(unit, vid);
    if (rv != BCM_E_NONE) {
        if (diags) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_free_frm_id, Unit(%d), Err: %d\n"),
                       FUNCTION_NAME(), unit, rv));
        }
        if (vid != sbx_vlan_state[unit]->default_vid) {
            /* don't abort unless not default VID (happens during reinit) */
            return(rv);
        }
    }

    /* clear port membership */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        if (diags) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: %d\n"),
                       FUNCTION_NAME(), unit, rv));
        }
        goto err;
    }

    SOC_PBMP_CLEAR(mvtEntry.ports);

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        if (diags) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: %d\n"),
                       FUNCTION_NAME(), unit, rv));
        }
        goto err;
    }

    if (BCM_E_NONE == rv) {
        sbx_vlan_state[unit]->vid_count -= 1;
        BCM_PBMP_CLEAR(sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
        BCM_PBMP_CLEAR(sbx_vlan_state[unit]->vid_list[vid].ut_port_bitmap);
        sbx_vlan_state[unit]->vid_list[vid].vlan_tag = BCM_VLAN_NONE;
    }

    return(rv);

err:
    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_destroy
 */
int
bcm_qe2000_vlan_destroy(int unit,
                        bcm_vlan_t vid)
{
    return bcm_qe2000_vlan_destroy_internal(unit, vid, TRUE);
}

/*
 *   Function
 *      bcm_qe2000_vlan_destroy_all
 */
int
bcm_qe2000_vlan_destroy_all(int unit)
{
    int rv = BCM_E_NONE;
    int vid;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    for (vid = 0; vid <= SBX_MVT_ID_VSI_END; vid++) {

        /* check if vlan is created/present */
        if (sbx_vlan_state[unit]->vid_list[vid].vlan_tag == BCM_VLAN_NONE) {
            continue;
        }

        /* cannot delete default vlan */
        if (sbx_vlan_state[unit]->vid_list[vid].vlan_tag == sbx_vlan_state[unit]->default_vid) {
            continue;
        }

        rv = bcm_qe2000_vlan_destroy_internal(unit, vid, TRUE);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_qe2000_vlan_destroy, Unit(%d), Err: %d\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    return(rv);

err:
    return(rv);
}

/*
 *   Function
 *      _bcm_qe2000_vlan_port_remove
 */
int
_bcm_qe2000_vlan_port_add(int unit,
                          bcm_vlan_t vid,
                          bcm_pbmp_t pbmp,
                          bcm_pbmp_t ubmp)
{
    int rv = BCM_E_NONE;
    sbx_mvt_id_t mvt_id;
    sbx_qe2000_mvt_entry_t mvtEntry;
    bcm_pbmp_t tbmpa, tbmpb;
    bcm_port_t port, mvt_port;
    bcm_pbmp_t  wPbmp;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (vid != sbx_vlan_state[unit]->vid_list[vid].vlan_tag) {
        return BCM_E_NOT_FOUND;
    }

    if (vid > SBX_MVT_ID_VSI_END) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Vlan Id, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return BCM_E_PARAM;
    }

    BCM_PBMP_CLEAR(wPbmp);
#if 0
    /*  This is removed due to Trunk Module dependencies. */
    BCM_PBMP_ASSIGN(wPbmp, sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
#endif /* 0 */
    BCM_PBMP_OR(wPbmp, pbmp);

    /* make sure only spi-subports and cpu ports are specified */
    BCM_PBMP_CLEAR(tbmpa);
    BCM_PBMP_OR(tbmpa, PBMP_SPI_SUBPORT_ALL(unit));
    BCM_PBMP_OR(tbmpa, PBMP_CMIC(unit));
    BCM_PBMP_NEGATE(tbmpb, tbmpa);
    BCM_PBMP_ASSIGN(tbmpa, wPbmp);
    BCM_PBMP_AND(tbmpa, tbmpb);

    if (BCM_PBMP_NOT_NULL(tbmpa)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Ports, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return(BCM_E_PARAM);
    }

    mvt_id = vid;
    COMPILER_REFERENCE(mvt_id);

    /* update port membership */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    BCM_PBMP_ITER(wPbmp, port) {
        mvt_port = IS_SPI_SUBPORT_PORT(unit, port) ? (port - SOC_PORT_MIN(unit, spi_subport)) :
                                                            SB_FAB_DEVICE_QE2000_CPU_PORT;
        SOC_PBMP_PORT_ADD(mvtEntry.ports, mvt_port);
    }

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    return(rv);

err:
    return(rv);
}


/*
 *   Function
 *      bcm_qe2000_vlan_port_add
 */
int
bcm_qe2000_vlan_port_add(int unit,
             bcm_vlan_t vid,
             bcm_pbmp_t pbmp,
             bcm_pbmp_t ubmp)
{
    int rv = BCM_E_NONE;

    rv = _bcm_qe2000_vlan_port_add(unit, vid, pbmp, ubmp);

    if (rv == BCM_E_NONE) {
        BCM_PBMP_OR(sbx_vlan_state[unit]->vid_list[vid].port_bitmap, pbmp);
        BCM_PBMP_OR(sbx_vlan_state[unit]->vid_list[vid].ut_port_bitmap, ubmp);

        if (vid == sbx_vlan_state[unit]->default_vid) {
            BCM_PBMP_OR(sbx_vlan_state[unit]->implicitly_default, pbmp);
        }

        /*
         *  This function will adjust the ports in this VLAN, according to
         *  whether they are members of aggregates.
         */
        rv = bcm_qe2000_trunk_vlan_port_adjust(unit,
                                              vid,
                                              sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
        if (BCM_E_INIT == rv) {
            /* aggregate support not initialised yet; ignore it */
            rv = BCM_E_NONE;
        }
    } /* if (rv == BCM_E_NONE) */

    return(rv);
}

/*
 *   Function
 *      _bcm_qe2000_vlan_port_remove
 */
int
_bcm_qe2000_vlan_port_remove(int unit,
                             bcm_vlan_t vid,
                             bcm_pbmp_t pbmp)
{
    int rv = BCM_E_NONE;
    sbx_mvt_id_t mvt_id;
    sbx_qe2000_mvt_entry_t mvtEntry;
    bcm_pbmp_t tbmpa, tbmpb;
    bcm_port_t port, mvt_port;
#if 0
    bcm_pbmp_t  wPbmp;
#endif

    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (BCM_VLAN_NONE == sbx_vlan_state[unit]->vid_list[vid].vlan_tag) {
        return(BCM_E_INIT);
    }

    if (vid > SBX_MVT_ID_VSI_END) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Vlan Id, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return(BCM_E_PARAM);
    }
#if 0
    BCM_PBMP_ASSIGN(wPbmp, sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
#endif
    /* make sure only spi-subports and cpu ports are specified */
    BCM_PBMP_CLEAR(tbmpa);
    BCM_PBMP_OR(tbmpa, PBMP_SPI_SUBPORT_ALL(unit));
    BCM_PBMP_OR(tbmpa, PBMP_CMIC(unit));
    BCM_PBMP_NEGATE(tbmpb, tbmpa);
    BCM_PBMP_ASSIGN(tbmpa, pbmp);
    BCM_PBMP_AND(tbmpa, tbmpb);

    if (BCM_PBMP_NOT_NULL(tbmpa)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Ports, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return(BCM_E_PARAM);
    }

    mvt_id = vid;
    COMPILER_REFERENCE(mvt_id);

    /* update port membership */
    rv = soc_qe2000_mvt_entry_get_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_get_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

#if 0
    /*  This is removed due to Trunk Module dependencies. */
    BCM_PBMP_ITER(wPbmp, port) {
        mvt_port = IS_SPI_SUBPORT_PORT(unit, port) ? (port - SOC_PORT_MIN(unit, spi_subport)) :
                                                            SB_FAB_DEVICE_QE2000_CPU_PORT;
        SOC_PBMP_PORT_ADD(mvtEntry.ports, mvt_port);
    }
#endif /* 0 */

    BCM_PBMP_ITER(pbmp, port) {
        mvt_port = IS_SPI_SUBPORT_PORT(unit, port) ? (port - SOC_PORT_MIN(unit, spi_subport)) :
                                                      SB_FAB_DEVICE_QE2000_CPU_PORT;
        SOC_PBMP_PORT_REMOVE(mvtEntry.ports, mvt_port);
    }

    rv = soc_qe2000_mvt_entry_set_frm_id(unit, vid, &mvtEntry);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_mvt_entry_set_frm_id, Unit(%d), Err: %d\n"),
                   FUNCTION_NAME(), unit, rv));
        goto err;
    }

    return(rv);

err:
    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_port_remove
 */
int
bcm_qe2000_vlan_port_remove(int unit,
                            bcm_vlan_t vid,
                            bcm_pbmp_t pbmp)
{
    int rv = BCM_E_NONE;

    rv = _bcm_qe2000_vlan_port_remove(unit, vid, pbmp);

    if (BCM_E_NONE == rv) {
        BCM_PBMP_REMOVE(sbx_vlan_state[unit]->vid_list[vid].port_bitmap, pbmp);
        BCM_PBMP_REMOVE(sbx_vlan_state[unit]->vid_list[vid].ut_port_bitmap, pbmp);

        if (vid == sbx_vlan_state[unit]->default_vid) {
            BCM_PBMP_REMOVE(sbx_vlan_state[unit]->implicitly_default, pbmp);
        }

        /*
         *  This function will adjust the ports in this VLAN, according to
         *  whether they are members of aggregates.
         */
        rv = bcm_qe2000_trunk_vlan_port_adjust(unit,
                                              vid,
                                              sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
        if (BCM_E_INIT == rv) {
            /* aggregate support not initialised yet; ignore it */
            rv = BCM_E_NONE;
        }
    } /* if (BCM_E_NONE == rv) */

    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_port_get
 *   Purpose
 *      Get the list of member ports for the specified VID.  The pbmp is the
 *      union of tagged and untagged ports in the VID; the ubmp is the
 *      untagged ports only.
 *   Parameters
 *      unit = unit number whose VID is to be examined for ports
 *      vid  = VID to which the ports are to ba added
 *      pbmp = (out) ptr to bitmap of ports for result
 *      ubmp = (out) ptr to bitmap of untagged ports for result
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_qe2000_vlan_port_get(int unit,
                         bcm_vlan_t vid,
                         pbmp_t *pbmp,
                         pbmp_t *ubmp)
{
    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (BCM_VLAN_NONE == sbx_vlan_state[unit]->vid_list[vid].vlan_tag) {
        return BCM_E_NOT_FOUND;
    }

    if (vid > SBX_MVT_ID_VSI_END) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Vlan Id, Unit(%d), VlanId: %d\n"),
                   FUNCTION_NAME(), unit, vid));
        return BCM_E_PARAM;
    }

    BCM_PBMP_ASSIGN(*pbmp, sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
    BCM_PBMP_ASSIGN(*ubmp, sbx_vlan_state[unit]->vid_list[vid].ut_port_bitmap);

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_qe2000_vlan_list
 *   Purpose
 *      Get a list of the VIDs that exist on this unit.
 *   Parameters
 *      unit   = unit number whose VLAN list is to be built
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_qe2000_vlan_list(int unit,
                     bcm_vlan_data_t **listp,
                     int *countp)
{
    int                 rv = BCM_E_NONE;
    int                 index;
    bcm_vlan_t          vid;
    bcm_vlan_data_t    *wList;
    int                 size;

    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if ((NULL == listp) || (NULL == countp)) {
        return BCM_E_PARAM;
    }

    size = sizeof(bcm_vlan_data_t) * sbx_vlan_state[unit]->vid_count;

    wList = *listp = NULL;
    index = 0;
    if (0 != size) {
        wList = sal_alloc(size, NULL);

        if (NULL == wList) {
            rv = BCM_E_MEMORY;
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Cannot allocate memory, unit(%d), Err: %s (%d)\n"),
                       FUNCTION_NAME(), unit, bcm_errmsg(rv), rv));
            goto err;
        }
        sal_memset(wList, 0, size);

        for (vid = 1; vid < BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_NONE != sbx_vlan_state[unit]->vid_list[vid].vlan_tag) {
                sal_memcpy(&wList[index++], &sbx_vlan_state[unit]->vid_list[vid], sizeof(bcm_vlan_data_t));
            }
        }
    }
    if (index != sbx_vlan_state[unit]->vid_count) {
        rv = BCM_E_INTERNAL;
    }

    if (BCM_E_NONE != rv) {
        if (NULL != wList) {
            sal_free(wList);
        }
        *countp = 0;
        wList  = NULL;
    } else {
        *listp  = wList;
        *countp = sbx_vlan_state[unit]->vid_count;
    }

err:
    return rv;
}

/*
 *   Function
 *      bcm_qe2000_vlan_list_by_pbmp
 *   Purpose
 *      Get a list of the VIDs that exist on this unit, that include any of the
 *      specified ports.
 *   Parameters
 *      unit   = unit number whose VLAN list is to be built
 *      pbmp   = ports to consider
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_qe2000_vlan_list_by_pbmp(int unit,
                             pbmp_t pbmp,
                             bcm_vlan_data_t **listp,
                             int *countp)
{
    int                 rv;
    int                 index, count, match_count = 0;
    pbmp_t              tPbmp;
    bcm_vlan_data_t    *wEntrySrc;
    bcm_vlan_data_t    *wEntryDst;


    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    rv = bcm_qe2000_vlan_list(unit, listp, &count);

    wEntrySrc = wEntryDst = *listp;
    if (BCM_E_NONE == rv) {
        for (index = 0; index < count; index++) {
            BCM_PBMP_ASSIGN(tPbmp, pbmp);
            BCM_PBMP_AND(tPbmp, wEntrySrc->port_bitmap);
            if (BCM_PBMP_IS_NULL(tPbmp)) {
            }
            else {
                if (wEntryDst != wEntrySrc) {
                    sal_memcpy(wEntryDst, wEntrySrc, sizeof(bcm_vlan_data_t));
                }
                wEntryDst++;
                match_count++;
            }
            wEntrySrc++;
        }

        (*countp) = match_count;
    }

    return rv;
}

/*
 *   Function
 *      bcm_qe2000_vlan_list_destroy
 *   Purpose
 *      Dispose of a vlan list obtained from _vlan_list or _vlan_list_by_pbmp.
 *   Parameters
 *      unit   = unit number from which this list was taken
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_qe2000_vlan_list_destroy(int unit,
                             bcm_vlan_data_t *list,
                             int count)
{
    int         rv = BCM_E_NONE;

    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    if (NULL == list) {
        return BCM_E_PARAM;
    }

    sal_free(list);
    list = NULL;

    return rv;
}

/*
 *   Function
 *      bcm_qe2000_vlan_default_set
 *
 *      NOTE:
 *      All ports are implicitly members of the default vlan. The user
 *      application may explicitly add some ports as members of the
 *      default vlan. Ports that are explicitly added are tracked by software.
 *
 *      For the current default vlan all implicit port members will be removed.
 *      This vlan will continue to exist.
 *
 *      The new default vlan should have already been created. Any port
 *      membership that are already configured will be tracked as explicit
 *      port members. All other ports will be implicitly added.
 *
 *      default VID can be disabled by setting it to BCM_VLAN_VID_DISABLE.
 */
int
bcm_qe2000_vlan_default_set(int unit,
                            bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t  default_pbmp, additional_pbmp, temp_pbmp, implicit_pbmp;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    BCM_PBMP_CLEAR(implicit_pbmp);
    BCM_PBMP_CLEAR(default_pbmp);
    BCM_PBMP_CLEAR(additional_pbmp);
    BCM_PBMP_CLEAR(temp_pbmp);

    if (vid != BCM_VLAN_VID_DISABLE) {
        /* check that vlan exists */
        if (sbx_vlan_state[unit]->vid_list[vid].vlan_tag == BCM_VLAN_NONE) {
            return(BCM_E_INIT);
        }

        /* Update all ports to be members of default VLAN */

#ifndef BCM_VLAN_NO_DEFAULT_SPI_SUBPORT
        /* Initilize default VLAN to contain all SPI subports. */
        default_pbmp = PBMP_SPI_SUBPORT_ALL(unit);
#endif

#ifndef BCM_VLAN_NO_DEFAULT_CPU
        /* Initialize default VLAN to contain CPU port */
        BCM_PBMP_OR(default_pbmp, PBMP_CMIC(unit));
#endif

        BCM_PBMP_ASSIGN(implicit_pbmp, sbx_vlan_state[unit]->vid_list[vid].port_bitmap);

        BCM_PBMP_ASSIGN(additional_pbmp, default_pbmp);
        BCM_PBMP_NEGATE(temp_pbmp, sbx_vlan_state[unit]->vid_list[vid].port_bitmap);
        BCM_PBMP_AND(additional_pbmp, temp_pbmp);
        rv = bcm_qe2000_vlan_port_add(unit, vid, additional_pbmp, additional_pbmp);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_qe2000_vlan_port_add, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }
    }

    /* Delete implicit members of current default vlan */
    if (sbx_vlan_state[unit]->default_vid != BCM_VLAN_VID_DISABLE) {
        rv = bcm_qe2000_vlan_port_remove(unit, sbx_vlan_state[unit]->default_vid,
                                        sbx_vlan_state[unit]->implicitly_default);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_qe2000_vlan_port_remove, Unit(%d), Err: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
        }
    }

    /* update default vlan and its implicit members */
    sbx_vlan_state[unit]->default_vid = vid;
    BCM_PBMP_ASSIGN(sbx_vlan_state[unit]->implicitly_default, implicit_pbmp);

    return(rv);

err:
    return(rv);
}

/*
 *   Function
 *      bcm_qe2000_vlan_default_get
 */
int
bcm_qe2000_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr)
{
    int rv = BCM_E_NONE;


    if (_sbx_qe2000_vlan_support == FALSE) {
        return BCM_E_UNAVAIL;
    }

    if (sbx_vlan_state[unit] == NULL) {
        return(BCM_E_INIT);
    }

    (*vid_ptr) = sbx_vlan_state[unit]->default_vid;

    return(rv);
}

