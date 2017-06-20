/*
 * $Id: qos.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * QoS implementation for caladan3
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <bcm/qos.h>
#include <bcm/error.h>
#include <bcm/stack.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/sbx/caladan3/qos.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/caladan3/mim.h>
#include <bcm_int/sbx/caladan3/wb_db_qos.h>
#endif /* def BCM_CALADAN3_SUPPORT */


#define QOS_EXCESS_VERBOSITY 0
#if QOS_EXCESS_VERBOSITY
#define QOS_EVERB(stuff)        LOG_DEBUG(BSL_LSBCM_QOS, stuff)
#else /* QOS_EXCESS_VERBOSITY */
#define QOS_EVERB(stuff)
#endif /* QOS_EXCESS_VERBOSITY */

#define QOS_MAP_NO_DIRECTIONS         0

#define QOS_MAP_ALL_DIRECTIONS                  \
    (BCM_QOS_MAP_INGRESS |                      \
     BCM_QOS_MAP_EGRESS)

#define QOS_MAP_ALL_LAYERS                      \
    (BCM_QOS_MAP_L2 |                           \
     BCM_QOS_MAP_L3 |                           \
     BCM_QOS_MAP_MPLS)

#define QOS_MAP_FLAGS                           \
    (BCM_QOS_MAP_WITH_ID |                      \
     BCM_QOS_MAP_REPLACE |                      \
     QOS_MAP_ALL_LAYERS  |                      \
     QOS_MAP_ALL_DIRECTIONS)


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static sal_mutex_t _qosLock = NULL;
_caladan3_qos_info_t *_qosInfo[BCM_MAX_NUM_UNITS];

/*
 * There is a set of ingress and a set of egress profiles.  The ingress qos
 * profile contains two conceptual maps:
 *    1.  a pri,cfi mapping OR MPLS mapping  (qos_t)
 *    2.  a dscp table                       (dscp_qos_t)
 * The g3p1 profile maps directly to a qos map id.
 *
 * The Egress profiles contain three maps as well, but they live in the same
 * mapping:
 *   1. pri,cfi
 *   2. MPLS
 *   3. dscp
 *
 * A profile entry is a mapping.  The maximum number of mappings is defined by
 * ucoded and queried at init time.  Currently, all 16 pri,cfi mappings are
 * supported, and a subset of the DSCPs are supported.
 *
 */
static uint32 max_qos_profile=0;
static uint32 max_qos_mapping=0;

/* QOS_DSCP_MAPPING_SIZE should be equal to max_qos_mapping.  Assuming there
 * is little or no change in the number of mapping entries, it was made a
 * compile time size.  If needed, can be converted to runtime...
 */
#define QOS_DSCP_MAPPING_SIZE  64

#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

#define QOS_UNIT_INIT_CHECK(_unit)                                      \
    do {                                                                \
        if ((0 > (_unit)) || (BCM_MAX_NUM_UNITS <= (_unit))) {          \
            LOG_ERROR(BSL_LS_BCM_QOS, \
                      (BSL_META("unit %d is not valid\n"), \
                       _unit));       \
            return BCM_E_UNIT;                                          \
        }                                                               \
        if (!(_qosLock && _qosInfo[_unit] && _qosInfo[_unit]->lock)) {  \
            LOG_ERROR(BSL_LS_BCM_QOS, \
                      (BSL_META("unit %d is not initialised\n"), \
                       _unit)); \
            return BCM_E_INIT;                                          \
        }                                                               \
    } while (0)
#define QOS_LOCK_TAKE(_unit)                                            \
    do {                                                                \
        if (sal_mutex_take(_qosInfo[_unit]->lock, sal_mutex_FOREVER)) { \
            LOG_ERROR(BSL_LS_BCM_QOS, \
                      (BSL_META("unit %d unable to take lock\n"), \
                       _unit)); \
            return BCM_E_INTERNAL;                                      \
        }                                                               \
    } while (0)
#define QOS_LOCK_GIVE(_unit)                                            \
    do {                                                                \
        if (sal_mutex_give(_qosInfo[_unit]->lock)) {                    \
            LOG_ERROR(BSL_LS_BCM_QOS, \
                      (BSL_META("unit %d unable to release lock\n"), \
                       _unit)); \
            return BCM_E_INTERNAL;                                      \
        }                                                               \
    } while (0)

#ifdef BCM_CALADAN3_G3P1_SUPPORT

#ifdef BCM_WARM_BOOT_SUPPORT
extern bcm_caladan3_wb_qos_state_scache_info_t
    *_bcm_caladan3_wb_qos_state_scache_info_p[BCM_MAX_NUM_UNITS];
#endif

/* translation api while port qos api is still supported;
 * remove when fully deprecated */
int
_bcm_caladan3_qos_map_id_to_hw_id(int flags, int map_id)
{
    if (flags & BCM_QOS_MAP_INGRESS) {
        return map_id;
    } else if (flags & BCM_QOS_MAP_EGRESS) {
        return map_id - QOS_MAP_ID_EGRESS_OFFSET;
    }
    return -1;
}

/* translation api while port qos api is still supported;
 * remove when fully deprecated */
int
_bcm_caladan3_qos_hw_id_to_map_id(int flags, int hw_id)
{

    if (flags & BCM_QOS_MAP_INGRESS) {
        return hw_id;
    } else if (flags & BCM_QOS_MAP_EGRESS) {
        return hw_id + QOS_MAP_ID_EGRESS_OFFSET;
    }
    return -1;
}

/*
 *  Function
 *    _bcm_caladan3_qos_1p_to_fcos
 *  Purpose
 *    Convert the given pri to an fcos mapping to the 802.1p
 *    recommended priority mapping
 *  Arguments
 *    unit   bcm device number
 *    pri    packet priority to convert
 *  Returns
 *      SBX FCOS
 *
 *  Notes
 *      configure all QOS profiles to default to 1:1 map for L2 on ingress
 *      Set the fcos according to the 802.1 spec's recommended table:
 *      "Recommended priority to traffic class mappings"
 *      where QE's highest priority is queue 0
 *      For 8 classes of service:
 *      pri  0  1  2  3  4  5  6  7
 *      fcos 6  7  5  4  3  2  1  0
 *
 *     Table 8-2 Recommended priority to traffic class mappings
 *
 *        Number of Available Traffic Classes
 *                  1 2 3 4 5 6 7 8
 *    Priority
 *     0 (Default)  0 0 0 0 0 1 1 1
 *     1            0 0 0 0 0 0 0 0
 *     2            0 0 0 1 1 2 2 2
 *     3            0 0 0 1 1 2 3 3
 *     4            0 1 1 2 2 3 4 4
 *     5            0 1 1 2 2 3 4 5
 *     6            0 1 2 3 3 4 5 6
 *     7            0 1 2 3 4 5 6 7
 */
STATIC int
_bcm_caladan3_qos_1p_to_fcos(int unit, int pri)
{
    uint8 num_cos_idx;
    int fcos_map[8][8] =
        {
            {0, 0, 0, 0, 0, 0, 0, 0},
            {1, 1, 1, 1, 0, 0, 0, 0},
            {2, 2, 2, 2, 1, 1, 0, 0},
            {3, 3, 2, 2, 1, 1, 0, 0},
            {4, 4, 3, 3, 2, 2, 1, 0},
            {4, 5, 3, 3, 2, 2, 1, 0},
            {5, 6, 4, 3, 2, 2, 1, 0},
            {6, 7, 5, 4, 3, 2, 1, 0}
        };
    num_cos_idx = NUM_COS(unit) - 1;
    if (num_cos_idx > 7) {
        num_cos_idx = 7;
    }
    return fcos_map[num_cos_idx][pri];
}

/*
 *  Function
 *    _bcm_caladan3_qos_profile_default_get
 *  Purpose
 *    Common routine to fill a soft copy of a complete qos mapping
 *  Arguments
 *    unit   bcm device number
 *    qos_a  storage array for l2 qos mapping
 *    size   number of entries in dscp_qos_a array
 *    dscp_qos_a storage array for l3 qos mapping
 *  Returns
 *      BCM_E_*
 */
int
_bcm_caladan3_qos_profile_default_get(int unit, soc_sbx_g3p1_qos_t qos_a[16],
                                     int size,
                                     soc_sbx_g3p1_dscpqos_t *dscp_qos_a)
{
    int                       cfi, pri, dscp;
    soc_sbx_g3p1_qos_t       *qos = qos_a;
    soc_sbx_g3p1_dscpqos_t  *dscp_qos = dscp_qos_a;

    if (size < max_qos_mapping) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Insufficient storage for l3.  "
                              "%d supplied, %d required\n"),
                   size, max_qos_mapping));
        return BCM_E_PARAM;
    }

    for (cfi = 0; cfi < 2; cfi++) {
        for (pri = 0; pri < 8; pri++) {

            /* Establish a default ingress qos map for l2 pri based on 802.1p */
            soc_sbx_g3p1_qos_t_init(qos);
            qos->cos    = pri;
            qos->fcos   = _bcm_caladan3_qos_1p_to_fcos(unit, pri);
            qos->mefcos = pri;
            qos++;
        }
    }

    for (dscp = 0; dscp < max_qos_mapping; dscp++) {

        /* Set the DSCP QOS Mapping table to Default Values */
        /* ------------------------------------------------------------------
         * DSCP | 0-7 | 8-15 | 16-23 | 24-31 | 32-39 | 40-47 | 48-55 | 56-63 |
         * COS  |  0  |  1   |  2    |   3   |   4   |   5   |   6   |   7   |
         * FCOS |  7  |  6   |  5    |   4   |   3   |   2   |   1   |   0   |
         * ------------------------------------------------------------------ */
        soc_sbx_g3p1_dscpqos_t_init(dscp_qos);
        dscp_qos->cos    = dscp / 8;
        dscp_qos->fcos   = 7 - dscp_qos->cos;
        dscp_qos->mefcos = dscp_qos->cos;
        dscp_qos++;
    }

    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_qos_map_write
 *  Purpose
 *    Write an entire qos profile to hw
 *  Arguments
 *    unit           - BCM unit
 *    idx            - hardware profile index
 *    qos_a          - l2 qos mapping
 *    size           - number of elements in l3 qos mapping
 *    dscp_qos_a     - l3 qos mapping
 *  Returns
 *      BCM_E_*
 */
int
_bcm_caladan3_qos_map_write(int unit, int idx, soc_sbx_g3p1_qos_t qos_a[16],
                           int size, soc_sbx_g3p1_dscpqos_t *dscp_qos_a)
{
    int                      cfi, pri, dscp, rv;
    soc_sbx_g3p1_qos_t      *qos = qos_a;
    soc_sbx_g3p1_dscpqos_t *dscp_qos = dscp_qos_a;

    for (cfi = 0; cfi < 2; cfi++) {
        for (pri = 0; pri < 8; pri++) {
            rv = soc_sbx_g3p1_qos_set(unit, cfi, pri, idx, qos);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "Failed to set qos[%d, %d, %d]: %s\n"),
                           cfi, pri, idx, bcm_errmsg(rv)));
                return rv;
            }
            qos++;
        }
    }

    for (dscp = 0; dscp < max_qos_mapping; dscp++) {

        rv = soc_sbx_g3p1_dscpqos_set(unit, dscp, idx, dscp_qos);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to set dscp_qos[%d, %d]: %s\n"),
                       dscp, idx, bcm_errmsg(rv)));
            return rv;
        }
        dscp_qos++;
    }

    return BCM_E_NONE;
}


/*
 *  Function
 *    _bcm_caladan3_qos_remark_default_get
 *  Purpose
 *    Initialize a soft copy of the egress remarking table
 *  Arguments
 *    unit           - BCM unit
 *    remark_a       - storage location for remarking table
 *  Returns
 *      BCM_E_*
 */
int
_bcm_caladan3_qos_remark_default_get(int unit, soc_sbx_g3p1_remark_t remark_a[64])
{
    soc_sbx_g3p1_remark_t *remark = remark_a;
    int ecn, dp, cos;

    for (ecn = 0; ecn < 2; ecn++) {
        for (dp = 0; dp < 4; dp++) {
            for (cos = 0; cos < 8; cos++) {

                soc_sbx_g3p1_remark_t_init(remark);
                /* one to one mapping */
                remark->dscp = cos << 3;
                remark->exp  = cos;
                remark->pri  = cos;

                remark++;
            }
        }
    }
    return BCM_E_NONE;
}


/*
 *  Function
 *    _bcm_caladan3_qos_remark_write
 *  Purpose
 *    Commit a remarking table to hardware
 *  Arguments
 *    unit           - BCM unit
 *    idx            - hardware profile index
 *    remark_a       - remarking table to commit
 *  Returns
 *      BCM_E_*
 */
int
_bcm_caladan3_qos_remark_write(int unit, int idx,
                              soc_sbx_g3p1_remark_t remark_a[64])
{
    soc_sbx_g3p1_remark_t *remark = remark_a;
    int                   ecn, dp, cos, rv;

    for (ecn = 0; ecn < 2; ecn++) {
        for (dp = 0; dp < 4; dp++) {
            for (cos = 0; cos < 8; cos++) {
                rv = soc_sbx_g3p1_remark_set(unit, ecn, dp, cos, idx, remark);

                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "Failed to set remark[%d, %d, %d, %d]: "
                                          "%s\n"),
                               ecn, dp, cos, idx, bcm_errmsg(rv)));
                    return rv;
                }
                remark++;
            }
        }
    }
    return BCM_E_NONE;
}


/*
 *  Function
 *    _bcm_caladan3_qos_profile_default_init
 *  Purpose
 *    Commit a default ingress or egress qos profile to hardware
 *  Arguments
 *    unit           - BCM unit
 *    flags          - BCM_QOS_MAP_* attribute flags
 *    idx            - hardware profile index
 *    map_id         - soft map id stored
 *  Returns
 *      BCM_E_*
 */
STATIC int
_bcm_caladan3_qos_profile_default_init(int unit, int flags, int idx, int *map_id)
{
    int rv;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_qos_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = SBX_QOS_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    if (flags & BCM_QOS_MAP_INGRESS) {
        soc_sbx_g3p1_qos_t qos_a[16];
        soc_sbx_g3p1_dscpqos_t dscp_qos_a[QOS_DSCP_MAPPING_SIZE];

        rv = _bcm_caladan3_qos_profile_default_get(unit, qos_a,
                                                  QOS_DSCP_MAPPING_SIZE,
                                                  dscp_qos_a);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to configure qos profile: %s\n"),
                       bcm_errmsg(rv)));
            return rv;
        }

        /* commit to hw */
        rv = _bcm_caladan3_qos_map_write(unit, idx, qos_a,
                                        QOS_DSCP_MAPPING_SIZE,
                                        dscp_qos_a);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to write qos profile 0x%x: %s\n"),
                       idx, bcm_errmsg(rv)));
            return rv;
        }

        _qosInfo[unit]->ingrFlags[idx] = flags;
        *map_id = idx;
#if defined(BCM_WARM_BOOT_SUPPORT)
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->ingrFlags_offset + (idx * sizeof(uint32)),
            _qosInfo[unit]->ingrFlags[idx]);
#endif

    } else if (flags & BCM_QOS_MAP_EGRESS) {
        soc_sbx_g3p1_remark_t remark[64];

        rv = _bcm_caladan3_qos_remark_default_get(unit, remark);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to configure remark table: %s\n"),
                       bcm_errmsg(rv)));
            return rv;
        }

        /* commit to hw */
        rv = _bcm_caladan3_qos_remark_write(unit, idx, remark);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to write remark table 0x%x: %s\n"),
                       idx, bcm_errmsg(rv)));
            return rv;
        }

        _qosInfo[unit]->egrFlags[idx] = flags;
        *map_id = idx + QOS_MAP_ID_EGRESS_OFFSET;
#if defined(BCM_WARM_BOOT_SUPPORT)
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->egrFlags_offset + (idx * sizeof(uint32)),
            _qosInfo[unit]->egrFlags[idx]);
#endif
    }

    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_caladan3_qos_detach
 *  Purpose
 *    Deinitialise the QoS module
 *  Arguments
 *    int unit = the unit to deinitialise
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes global lock is taken and that unit number is valid, and that the
 *    unit in question has been initialised.
 *
 *    Unlinks the unit immediately, then waits on the lock, to prevent other
 *    threads from trying to use the unit.  At the point when this gets the
 *    lock, there should be no other threads waiting (since we already marked
 *    the unit as not initialised), but just in case, the OS should return an
 *    error for their lock take since the lock is destroyed.
 */
static int
_bcm_caladan3_qos_detach(int unit)
{
    _caladan3_qos_info_t *tempUnit = _qosInfo[unit];
    uint32 map;
    int xrv;

    LOG_DEBUG(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d) enter\n"),
               unit));

    _qosInfo[unit] = NULL;
    if (sal_mutex_take(tempUnit->lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unable to take unit %d lock\n"),
                   unit));
        _qosInfo[unit] = tempUnit;
        return BCM_E_INTERNAL;
    }

    for (map = SBX_QOS_PROFILE_BASE;
         map < max_qos_profile;
         map++) {
        if (~0 != tempUnit->ingrFlags[map]) {
            /* allocated this ingress map here; need to free it here */
            xrv = _sbx_caladan3_resource_free(unit,
                                              SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                              1,
                                              &map,
                                              0);
            COMPILER_REFERENCE(xrv);
        }
    }
    for (map = SBX_QOS_EGR_REMARK_BASE;
         map < max_qos_profile;
         map++) {
        if (~0 != tempUnit->egrFlags[map]) {
            /* allocated this egress map here; need to free it here */
            xrv = _sbx_caladan3_resource_free(unit,
                                              SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                              1,
                                              &map,
                                              0);
            COMPILER_REFERENCE(xrv);
        }
    }
    sal_mutex_give(tempUnit->lock);
    sal_mutex_destroy(tempUnit->lock);
    sal_free(tempUnit);
    return BCM_E_NONE;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 *  Function
 *    bcm_caladan3_qos_init
 *  Purpose
 *    Initialise the QoS module
 *  Arguments
 *    int unit = the unit to initialise
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_caladan3_qos_init(int unit)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int id, flags, rv = BCM_E_INTERNAL;
    uint32 max_remark;
    sal_mutex_t tempLock = NULL;
    _caladan3_qos_info_t *tempUnit = NULL;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_qos_state_scache_info_t *wb_info_ptr = NULL;
    int i;
#endif

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    if ((0 > unit) || (BCM_MAX_NUM_UNITS <= unit)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d is not valid\n"),
                   unit));
        return BCM_E_UNIT;
    }

    if (!SOC_IS_SBX_G3P1(unit)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        rv = BCM_E_UNAVAIL;
        return rv;
    }

    /* max number of profiles; a profile contains mappings */
    rv = soc_sbx_g3p1_max_qos_profile_index_get(unit, &max_qos_profile);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* maximum number of mappings in a profile */
    rv = soc_sbx_g3p1_max_qos_map_table_get(unit, &max_qos_mapping);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* verify egress remark table max is the same, current code
     * assumes max ingress mapping == max egress mapping
     */
    rv = soc_sbx_g3p1_max_qos_remark_table_get(unit, &max_remark);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    if (max_qos_mapping != max_remark) {
        return BCM_E_INTERNAL;
    }

    if (!_qosLock) {
        /* no units exist yet */
        tempLock = sal_mutex_create("caladan3 global QoS lock");
        if (!tempLock) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unable to create global lock\n")));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unable to initially take global lock\n")));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        _qosLock = tempLock;
        sal_memset(&(_qosInfo[0]),
                   0x00,
                   sizeof(_qosInfo[0]) * BCM_MAX_NUM_UNITS);
        sal_mutex_give(tempLock);

        sal_thread_yield();

    } else if (sal_mutex_take(_qosLock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unable to take global lock\n")));
        return BCM_E_RESOURCE;
    }

    if (tempLock && (tempLock != _qosLock)) {
        LOG_WARN(BSL_LS_BCM_QOS,
                 (BSL_META_U(unit,
                             "detected init race; compensating\n")));
        sal_mutex_destroy(tempLock);
        tempLock = NULL;
    }

    if (_qosInfo[unit]) {
        /* this unit is initialised */
        LOG_VERBOSE(BSL_LS_BCM_QOS,
                    (BSL_META_U(unit,
                                "unit %d already initialised; detach first\n"),
                     unit));
        rv = _bcm_caladan3_qos_detach(unit);
    } else {
        /* successfully don't detach */
        rv = BCM_E_NONE;
    } /* if (_qosInfo[unit]) */

    if (BCM_E_NONE == rv) {
        tempUnit = sal_alloc(sizeof(*tempUnit) +
                             (sizeof(tempUnit->ingrFlags[0]) *
                              (max_qos_profile)) +
                             (sizeof(tempUnit->egrFlags[0]) *
                              (max_qos_profile)),
                             "caladan3 qos unit information");
        if (tempUnit) {
            sal_memset(tempUnit,
                       0xFF,
                       sizeof(*tempUnit) +
                       (sizeof(tempUnit->ingrFlags[0]) *
                        (max_qos_profile)) +
                       (sizeof(tempUnit->egrFlags[0]) *
                        (max_qos_profile)));
            tempUnit->ingrFlags = (uint32*)(&(tempUnit[1]));
            tempUnit->egrFlags = (uint32*)&(tempUnit->ingrFlags[max_qos_profile]);
            tempUnit->lock = sal_mutex_create("caladan3 unit QoS lock");
            if (!tempUnit->lock) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unable to create unit %d lock\n"),
                           unit));
                sal_free(tempUnit);
                tempUnit = NULL;
                rv = BCM_E_RESOURCE;
                return rv;
            }
            _qosInfo[unit] = tempUnit;
        } else { /* if (tempUnit) */
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes for unit %d"
                                   " information\n"),
                       sizeof(*tempUnit) +
                       (sizeof(tempUnit->ingrFlags[0]) *
                       (max_qos_profile)) +
                       (sizeof(tempUnit->egrFlags[0]) *
                       (max_qos_profile)),
                       unit));
            rv = BCM_E_MEMORY;
            return rv;
        } /* if (tempUnit) */
    } /* if (BCM_E_NONE == rv) */

#if defined(BCM_WARM_BOOT_SUPPORT)
    rv = bcm_caladan3_wb_qos_state_init(unit);
#endif

    /* initialize the default ingress and egress profile for l2 only.  L3 maps
     * need to be allocated and assiged seperately using the api.
     */
    if (!SOC_WARM_BOOT(unit)) {
        if (BCM_SUCCESS(rv)) {
            flags = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2;

            rv = _bcm_caladan3_qos_profile_default_init(unit, flags, 0, &id);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "Failed to init default ingress qos map: %s\n"),
                           bcm_errmsg(rv)));
            }
        }

        if (BCM_SUCCESS(rv)) {
            flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;

            rv = _bcm_caladan3_qos_profile_default_init(unit, flags, 0, &id);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "Failed to init default egress qos map: %s\n"),
                           bcm_errmsg(rv)));
            }
        }

#if defined(BCM_WARM_BOOT_SUPPORT)
        /* Write the flags to cache */
        wb_info_ptr = SBX_QOS_SCACHE_INFO_PTR(unit);
        if(wb_info_ptr == NULL) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Warm boot not initialized for unit %d \n"),
                       unit));
            rv = BCM_E_INTERNAL;
            BCM_EXIT;
        }

        for (i = 0; i < max_qos_profile; i++) {
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                wb_info_ptr->ingrFlags_offset + (i * sizeof(uint32)),
                _qosInfo[unit]->ingrFlags[i]);
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                wb_info_ptr->egrFlags_offset + (i * sizeof(uint32)),
                _qosInfo[unit]->egrFlags[i]);
        }
#endif
    }


#if defined(BCM_WARM_BOOT_SUPPORT)
exit:
#endif

        LOG_VERBOSE(BSL_LS_BCM_QOS,
                    (BSL_META_U(unit,
                                "(%d) return %d (%s)\n"),
                     unit, rv, _SHR_ERRMSG(rv)));


    if (!tempLock) {
        sal_mutex_give(_qosLock);
    }

    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d) not supported\n"),
               unit));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}

/*
 *  Function
 *    bcm_caladan3_qos_detach
 *  Purpose
 *    Deinitialise the QoS module
 *  Arguments
 *    int unit = the unit to deinitialise
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_caladan3_qos_detach(int unit)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int rv = BCM_E_INTERNAL;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    if (SOC_IS_SBX_G3P1(unit)) {
        QOS_UNIT_INIT_CHECK(unit);

        if (sal_mutex_take(_qosLock, sal_mutex_FOREVER)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unable to take global lock\n")));
            return BCM_E_INTERNAL;
        }

        rv = _bcm_caladan3_qos_detach(unit);

        if (sal_mutex_give(_qosLock)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unable to release global lock\n")));
            return BCM_E_INTERNAL;
        }
    } else { /* if (SOC_IS_SBX_G3P1(unit)) */
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        rv = BCM_E_UNAVAIL;
    } /* if (SOC_IS_SBX_G3P1(unit)) */

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit,
                 rv,
                 _SHR_ERRMSG(rv)));
    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d) not supported\n"),
               unit));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}


/*
 *  Function
 *    _bcm_caladan3_qos_map_create
 *  Purpose
 *    Allocates or reserves resources, creates a qos map, initializes to
 *    a default mapping
 *  Arguments
 *    unit           - BCM unit
 *    flags          - BCM_QOS_MAP_* attribute flags
 *    map_id         - soft map id stored
 *  Returns
 *      BCM_E_*
 *  Notes:
 *   Assumes lock is taken, an primative arg checking
 */
STATIC int
_bcm_caladan3_qos_map_create(int unit, uint32 flags, int *map_id)
{
    int rv;
    uint32 newMapId = ~0;
    int   resource = SBX_CALADAN3_USR_RES_MAX;
    char *direction_str = "<none>";

    /* Convert to HW id */
    if (flags & BCM_QOS_MAP_WITH_ID) {
        /* caller wants to specify, so get and check range */
        if (flags & BCM_QOS_MAP_INGRESS) {
            newMapId = *map_id;
            if ((max_qos_profile <= newMapId) ||
                (SBX_QOS_PROFILE_BASE > newMapId)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "Ingress QoS map ID %d is not valid on"
                                       " unit %d\n"),
                           *map_id,
                           unit));
                return BCM_E_PARAM;
            }
        } else if (flags & BCM_QOS_MAP_EGRESS) {
            newMapId = (*map_id) - QOS_MAP_ID_EGRESS_OFFSET;
            if ((max_qos_profile <= newMapId) ||
                (SBX_QOS_EGR_REMARK_BASE > newMapId)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "Egress QoS map ID %d is not valid on"
                                       " unit %d\n"),
                           *map_id,
                           unit));
                return BCM_E_PARAM;
            }
        }
    } /* if (flags & BCM_QOS_MAP_WITH_ID) */

    /* Determine resource allocation */
    if (flags & BCM_QOS_MAP_INGRESS) {
        resource  = SBX_CALADAN3_USR_RES_QOS_PROFILE;
        direction_str = "ingress";
    } else if (flags & BCM_QOS_MAP_EGRESS) {
        resource  = SBX_CALADAN3_USR_RES_QOS_EGR_REMARK;
        direction_str = "egress";
    }

    /* Allocate or reserve the hw resource type & id */
    if (flags & BCM_QOS_MAP_WITH_ID) {

        if (flags & BCM_QOS_MAP_REPLACE) {
            if ((uint32)(~((uint32)(0))) ==
                _qosInfo[unit]->ingrFlags[newMapId]) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "tried to replace %s map %d on"
                                       " unit %d, but that map was not"
                                       " allocated using this mechanism\n"),
                           direction_str, *map_id, unit));
                return BCM_E_NOT_FOUND;
            }

        } else { /* if (flags & BCM_QOS_MAP_REPLACE) */
            rv = _sbx_caladan3_resource_alloc(unit, resource, 1, &newMapId,
                                              _SBX_CALADAN3_RES_FLAGS_RESERVE);
            if (BCM_E_NONE != rv) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unable to reserve unit %d %s"
                                       " QoS map %d: %d (%s)\n"),
                           unit, direction_str, *map_id, rv,
                           _SHR_ERRMSG(rv)));
                return rv;
            }
        } /* if (flags & BCM_QOS_MAP_REPLACE) */

    } else { /* if (flags & BCM_QOS_MAP_WITH_ID) */
        rv = _sbx_caladan3_resource_alloc(unit, resource, 1, &newMapId, 0);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Failed to alloc %s qos profile: %s\n"),
                       direction_str, bcm_errmsg(rv)));
            return rv;
        }
    } /* if (flags & BCM_QOS_MAP_WITH_ID) */

    /* Initialize the profile to default settings */
    rv = _bcm_caladan3_qos_profile_default_init(unit, flags, newMapId, map_id);

    if (BCM_FAILURE(rv)) {
        if ((flags & BCM_QOS_MAP_REPLACE) == 0) {
            rv = _sbx_caladan3_resource_free(unit, resource, 1, &newMapId, 0);
        }
    }

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_qos_map_create
 *  Purpose
 *    Allocate a QoS map
 *  Arguments
 *    int unit = the unit on which to operate
 *    uint32 flags = flags for the create
 *    int *map_id = where to put the new map's ID
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Initialises the 'created' map to a set of default values that are
 *    basically direct maps, except egress DSCP, which is (remark_pri << 3).
 */
int
bcm_caladan3_qos_map_create(int unit, uint32 flags, int *map_id)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int rv = BCM_E_INTERNAL;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,*) enter\n"),
                 unit, flags));

    if (!SOC_IS_SBX_G3P1(unit)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        return BCM_E_UNAVAIL;
    }

    if (!map_id) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "NULL pointer argument is unacceptable\n")));
        return BCM_E_PARAM;
    }

    /* L2 & MPLS are not mutually exclusive, only one can take effect for Ingress QOS mapping */
    /* L3 is mutually exclusive from l2,mpls as it uses independent ingress dscp mapping table */
    if (flags & (~QOS_MAP_FLAGS)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unsupported flags %08X included\n"),
                   (flags & (~QOS_MAP_FLAGS))));
        return BCM_E_PARAM;
    }

    if ((0 == (flags & (BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_EGRESS))) ||
        ((BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_EGRESS) ==
         (flags & (BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_EGRESS)))) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "must specify exactly one of ingress or egress\n")));
        return BCM_E_PARAM;
    }

    if (flags & BCM_QOS_MAP_INGRESS) {
        if ((BCM_QOS_MAP_L2 | BCM_QOS_MAP_MPLS) ==
            (flags & (BCM_QOS_MAP_L2 | BCM_QOS_MAP_MPLS))) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "must specify exactly one of L2 or MPLS\n")));
            return BCM_E_PARAM;
        }
    }

    QOS_UNIT_INIT_CHECK(unit);

    QOS_LOCK_TAKE(unit);
    rv = _bcm_caladan3_qos_map_create(unit, flags, map_id);
    QOS_LOCK_GIVE(unit);

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%d)) return %d (%s)\n"),
                 unit,
                 flags,
                 *map_id,
                 rv,
                 _SHR_ERRMSG(rv)));
    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d,%08X,*) not supported\n"),
               unit, flags));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}

/*
 *  Function
 *    bcm_caladan3_qos_map_destroy
 *  Purpose
 *    Free a QoS map
 *  Arguments
 *    int unit = the unit on which to operate
 *    int map_id = the map to be freed
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_caladan3_qos_map_destroy(int unit, int map_id)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int rv = BCM_E_INTERNAL;
    uint32 mapId;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_qos_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = SBX_QOS_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d) enter\n"),
                 unit, map_id));

    if (SOC_IS_SBX_G3P1(unit)) {
        QOS_UNIT_INIT_CHECK(unit);
        rv = BCM_E_NONE;
        if (QOS_MAP_ID_EGRESS_OFFSET > map_id) {
            /* ingress map */
            mapId = map_id;
            if ((max_qos_profile <= mapId) || (SBX_QOS_PROFILE_BASE > mapId)) {
                rv = BCM_E_NOT_FOUND;
            }
        } else { /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
            /* egress map */
            mapId = map_id - QOS_MAP_ID_EGRESS_OFFSET;
            if ((max_qos_profile <= mapId) ||
                (SBX_QOS_EGR_REMARK_BASE > mapId)) {
                rv = BCM_E_NOT_FOUND;
            }
        } /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unit %d QoS map %d is not valid\n"),
                       unit,
                       map_id));
            return rv;
        } /* if (BCM_E_NONE != rv) */
        QOS_LOCK_TAKE(unit);
        if (QOS_MAP_ID_EGRESS_OFFSET > map_id) {
            /* ingress map */
            if ((uint32)(~((uint32)(0))) ==
                _qosInfo[unit]->ingrFlags[mapId]) {
                /* not allocated here */
                rv = BCM_E_NOT_FOUND;
            } else {
                /* allocated here, so free it */
                rv = _sbx_caladan3_resource_free(unit,
                                                 SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                                 1,
                                                 &mapId,
                                                 0);
                if (BCM_E_NONE == rv) {
                    _qosInfo[unit]->ingrFlags[mapId] = ~0;
#if defined(BCM_WARM_BOOT_SUPPORT)
                    SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                        wb_info_ptr->ingrFlags_offset + (mapId * sizeof(uint32)),
                        _qosInfo[unit]->ingrFlags[mapId]);
#endif
                } else {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "unable to free unit %d ingress map %d:"
                                          " %d (%s)\n"),
                               unit,
                               map_id,
                               rv,
                               _SHR_ERRMSG(rv)));
                }
            }
        } else { /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
            /* egress map */
            if ((uint32)(~((uint32)(0))) ==
                _qosInfo[unit]->egrFlags[mapId]) {
                /* not allocated here */
                rv = BCM_E_NOT_FOUND;
            } else {
                rv = _sbx_caladan3_resource_free(unit,
                                                 SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                                 1,
                                                 &mapId,
                                                 0);
                if (BCM_E_NONE == rv) {
                    _qosInfo[unit]->egrFlags[mapId] = ~0;
#if defined(BCM_WARM_BOOT_SUPPORT)
                    SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                        wb_info_ptr->egrFlags_offset + (mapId * sizeof(uint32)),
                        _qosInfo[unit]->egrFlags[mapId]);
#endif
                } else {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "unable to free unit %d egress map %d:"
                                          " %d (%s)\n"),
                               unit,
                               map_id,
                               rv,
                               _SHR_ERRMSG(rv)));
                }
            }
        } /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
        QOS_LOCK_GIVE(unit);
    } else { /* if (SOC_IS_SBX_G3P1(unit)) */
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        rv = BCM_E_UNAVAIL;
    } /* if (SOC_IS_SBX_G3P1(unit)) */

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d) return %d (%s)\n"),
                 unit,
                 map_id,
                 rv,
                 _SHR_ERRMSG(rv)));
    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d,%d) not supported\n"),
               unit, map_id));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}

/*
 *  Function
 *    _bcm_caladan3_qos_map_data_set
 *  Purpose
 *    Verify arguments and set values in a QoS map
 *  Arguments
 *    int unit = the unit to affect
 *    uint32 flags = must be subset of flag value used in qos_create
 *    bcm_qos_map_t *map = pointer to map data to set
 *    int map_id = which map needs its data set
 *    int revert = zero to set data; nonzero to set default values
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The fields of the bcm_qos_map_t which are 'index' or 'data' will vary
 *    according to whether manipulating an ingress map or egress map.  Some
 *    'index' fields are ignored for L2, some for MPLS.
 *
 *    For ingress: pkt_pri, pkt_cfi, exp are 'index'; others are data.
 *
 *    For egress: remark_int_pri, remark_color are 'index'; others are data.
 *
 *    For L2: exp is ignored as an index.
 *
 *    For MPLS: pkt_pri, pkt_cfi are ignored as indices.
 *
 *    For all: color is ignored (other than range checking).
 */
static int
_bcm_caladan3_qos_map_data_set(int unit,
                               uint32 flags,
                               bcm_qos_map_t *map,
                               int map_id,
                               int revert)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    soc_sbx_g3p1_qos_t iQosData;
    soc_sbx_g3p1_remark_t eQosData;
    soc_sbx_g3p1_dscpqos_t dscpQosData;
    uint32 mapId;
    uint32 mapFlags;
    uint8  isEgressMap = 0;
    int rv = BCM_E_INTERNAL;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,*,%d,%s) enter\n"),
                 unit,
                 flags,
                 map_id,
                 revert?"DELETE":"ADD"));

    if (!map) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "NULL pointer argument is unacceptable\n")));
        return BCM_E_PARAM;
    }

    if (/* (0 > map->pkt_pri) || */
        (7 < map->pkt_pri) ||
        /* (0 > map->pkt_cfi) || */
        (1 < map->pkt_cfi) ||
        (0 > map->exp) ||
        (7 < map->exp) ||
        (0 > map->color) ||
        (3 < map->color) ||
        (0 > map->dscp) ||
        (63 < map->dscp) ||
        (0 > map->int_pri) ||
        (7 < map->int_pri) ||
        (0 > map->policer_offset) ||
        (7 < map->policer_offset) ||
        (0 > map->remark_color) ||
        (3 < map->remark_color) ||
        (0 > map->remark_int_pri) ||
        (7 < map->remark_int_pri)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "invalid value(s) in provided map data\n")));
        return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_G3P1(unit)) {
        QOS_UNIT_INIT_CHECK(unit);

        if (QOS_MAP_ID_EGRESS_OFFSET > map_id) {
            /* ingress map */
            mapId = map_id;
            if ((max_qos_profile <= mapId) || (SBX_QOS_PROFILE_BASE > mapId)) {
                rv = BCM_E_NOT_FOUND;
            } else {
                rv = BCM_E_NONE;
            }
        } else { /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
            /* egress map */
            mapId = map_id - QOS_MAP_ID_EGRESS_OFFSET;
            isEgressMap = 1;
            if ((max_qos_profile <= mapId) ||
                (SBX_QOS_EGR_REMARK_BASE > mapId)) {
                rv = BCM_E_NOT_FOUND;
            } else {
                rv = BCM_E_NONE;
            }
        } /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unit %d QoS map %d is not valid\n"),
                       unit,
                       map_id));
            return rv;
        } /* if (BCM_E_NONE != rv) */

        QOS_LOCK_TAKE(unit);

        /* L2 & MPLS are not mutually exclusive, only one can take effect for Ingress QOS mapping */
        /* L3 is mutually exclusive from l2,mpls as it uses independent ingress dscp mapping table */
        /* L2/MPLS/L3 egress remarking flags are mutually exclusive */

        /* verify if its a ingress or egress qos map */
        if (isEgressMap) {
            /* egress map */
            mapFlags = _qosInfo[unit]->egrFlags[mapId];
        } else {
            mapFlags = _qosInfo[unit]->ingrFlags[mapId];
        }

        if (~0 == mapFlags) {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "unit %d %s map %d not allocated here\n"),
                       unit, (isEgressMap)?"Egress":"Ingress",
                       map_id));
            rv = BCM_E_NOT_FOUND;
        }

        if(BCM_SUCCESS(rv)) {
            if(flags){
                /* verify if flag is not super set of created flags */
                if((flags & mapFlags) == 0) {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "Bad input flags %08X not related to creation time Flag %08x\n"),
                               flags, mapFlags));
                    rv = BCM_E_PARAM;

                } else if (flags & (~(BCM_QOS_MAP_L2 | BCM_QOS_MAP_L3 | BCM_QOS_MAP_MPLS))) {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "unsupported flags %08X\n"),
                               flags));
                    rv = BCM_E_PARAM;
                } else if (!isEgressMap) {
                    /* ingress qos map supported flags */
                    if((flags & (BCM_QOS_MAP_L2 | BCM_QOS_MAP_MPLS)) ==
                       (BCM_QOS_MAP_L2 | BCM_QOS_MAP_MPLS)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "Bad input Flag %08x Ingress L2/MPLS are mutually exclusive\n"),
                                   flags));
                        rv = BCM_E_PARAM;
                    }
                }
            } else {
                flags = mapFlags;
            }
        }

        if(BCM_SUCCESS(rv)) {
            if (!isEgressMap) {
                /* ingress map */
                soc_sbx_g3p1_qos_t_init(&iQosData);
                if (flags & BCM_QOS_MAP_L2) {
                    /* L2 map */
                    if (revert) {
                        iQosData.cos = map->pkt_pri;
                        iQosData.fcos =
                            _bcm_caladan3_qos_1p_to_fcos(unit, map->pkt_pri);
                        iQosData.mefcos = map->pkt_pri;
                        iQosData.dp = 0;
                        iQosData.e = 0;
                    } else {
                        iQosData.cos = map->remark_int_pri;
                        iQosData.fcos = map->int_pri;
                        iQosData.mefcos = map->policer_offset;
                        iQosData.dp = map->remark_color;
                        iQosData.e = 0;
                    }
                    QOS_EVERB((BSL_META_U(unit,
                                          "map %d: cfi=%d, pri=%d ->"
                                          " cos=%d, fcos=%d, mefcos=%d,"
                                          " dp=%d, e=%d\n"),
                               mapId,
                               map->pkt_cfi,
                               map->pkt_pri,
                               iQosData.cos,
                               iQosData.fcos,
                               iQosData.mefcos,
                               iQosData.dp,
                               iQosData.e));
                    rv = soc_sbx_g3p1_qos_set(unit,
                                              map->pkt_cfi,
                                              map->pkt_pri,
                                              mapId,
                                              &iQosData);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "unit %d ingress PCP map %d set"
                                               " failed: %d (%s)\n"),
                                   unit,
                                   map_id,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                    }
                } else if (flags & BCM_QOS_MAP_MPLS) {
                    /* MPLS map */
                    if (revert) {
                        iQosData.cos = map->exp;
                        iQosData.fcos = map->exp;
                        iQosData.mefcos = map->exp;
                        iQosData.dp = 0;
                        iQosData.e = 0;
                    } else {
                        iQosData.cos = map->remark_int_pri;
                        iQosData.fcos = map->int_pri;
                        iQosData.mefcos = map->policer_offset;
                        iQosData.dp = map->remark_color;
                        iQosData.e = 0;
                    }
                    QOS_EVERB((BSL_META_U(unit,
                                          "map %d: exp=%d ->"
                                          " cos=%d, fcos=%d, mefcos=%d,"
                                          " dp=%d, e=%d\n"),
                               mapId,
                               map->exp,
                               iQosData.cos,
                               iQosData.fcos,
                               iQosData.mefcos,
                               iQosData.dp,
                               iQosData.e));
                    rv = soc_sbx_g3p1_qos_set(unit,
                                              0,
                                              map->exp,
                                              mapId,
                                              &iQosData);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "unit %d ingress EXP map %d set"
                                               " failed: %d (%s)\n"),
                                   unit,
                                   map_id,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                    }
                }

                /* if l3 flag is enabled set DSCP qos mapping */
                if(flags & BCM_QOS_MAP_L3) {
                    soc_sbx_g3p1_dscpqos_t_init(&dscpQosData);

                    /* DSCP map */
                    if (revert) {
                        dscpQosData.cos    = map->dscp % 7;
                        dscpQosData.fcos   = map->dscp % 7;
                        dscpQosData.mefcos = map->dscp % 7;
                        dscpQosData.dp     = 0;
                        dscpQosData.e      = 0;
                    } else {
                        dscpQosData.cos    = map->remark_int_pri;
                        dscpQosData.fcos   = map->int_pri;
                        dscpQosData.mefcos = map->policer_offset;
                        dscpQosData.dp     = map->remark_color;
                        dscpQosData.e      = 0;
                    }

                    QOS_EVERB((BSL_META_U(unit,
                                          "DSCP map %d: dscp=%d ->"
                                          " cos=%d, fcos=%d, mefcos=%d,"
                                          " dp=%d, e=%d\n"),
                               mapId,
                               map->dscp,
                               dscpQosData.cos,
                               dscpQosData.fcos,
                               dscpQosData.mefcos,
                               dscpQosData.dp,
                               dscpQosData.e));

                    rv = soc_sbx_g3p1_dscpqos_set(unit,
                                                  map->dscp,
                                                  mapId,
                                                  &dscpQosData);

                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "unit %d dscp ingress map %d set"
                                               " failed: %d (%s)\n"),
                                   unit,
                                   map_id,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                    }
                }
            } else { /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
                /* egress map */
                soc_sbx_g3p1_remark_t_init(&eQosData);
                rv = soc_sbx_g3p1_remark_get(unit,
                                             0,
                                             map->remark_color,
                                             map->remark_int_pri,
                                             mapId,
                                             &eQosData);

                if(BCM_SUCCESS(rv)) {

                    if (flags & BCM_QOS_MAP_L2) {
                        /* L2 map */
                        if (revert) {
                            eQosData.pri = map->remark_int_pri;
                            eQosData.cfi = 0;
                        } else {
                            eQosData.pri = map->pkt_pri;
                            eQosData.cfi = map->pkt_cfi;
                        }
                    }

                    if (flags & BCM_QOS_MAP_MPLS) {
                        /* MPLS map */
                        if (revert) {
                            eQosData.exp = map->remark_int_pri;
                        } else {
                            eQosData.exp = map->exp;
                        }
                    }

                    if (flags & BCM_QOS_MAP_L3) {
                        /* L2 map */
                        if (revert) {
                            eQosData.dscp = map->remark_int_pri << 3;
                        } else {
                            eQosData.dscp = map->dscp;
                        }
                    }

                    QOS_EVERB((BSL_META_U(unit,
                                          "map %d: rem_col=%d, rem_pri=%d ->"
                                          " cfi=%d, pri=%d, exp=%d, dscp=%d\n"),
                               mapId,
                               map->remark_color,
                               map->remark_int_pri,
                               eQosData.cfi,
                               eQosData.pri,
                               eQosData.exp,
                               eQosData.dscp));

                    rv = soc_sbx_g3p1_remark_set(unit,
                                                 0,
                                                 map->remark_color,
                                                 map->remark_int_pri,
                                                 mapId,
                                                 &eQosData);
                    if (BCM_E_NONE != rv) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "unit %d egress map %d set"
                                               " failed: %d (%s)\n"),
                                   unit,
                                   map_id,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                    }
                } else {
                    /* log error */
                }

            } /* if (QOS_MAP_ID_EGRESS_OFFSET > map_id) */
        } /*BCM_SUCCESS(rv) */
        QOS_LOCK_GIVE(unit);
    } else { /* if (SOC_IS_SBX_G3P1(unit)) */
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        rv = BCM_E_UNAVAIL;
    } /* if (SOC_IS_SBX_G3P1(unit)) */

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,*,%d,%s) return %d (%s)\n"),
                 unit,
                 flags,
                 map_id,
                 revert?"DELETE":"ADD",
                 rv,
                 _SHR_ERRMSG(rv)));
    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d,%08X,*,%d,%s) not supported\n"),
               unit,
               flags,
               map_id,
               revert?"DELETE":"ADD"));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}

/*
 *  Function
 *    _bcm_caladan3_qos_map_translate
 *  Purpose
 *    Translate a BCM software qos map ids into hardware indexes
 *  Arguments
 *    (in)  unit       - bcm unit number
 *    (in)  ing_map    - bcm qos map id
 *    (in)  egr_map    - bcm qos map id
 *    (out) ing_idx    - qos profile
 *    (out) egr_idx    - remark index
 *  Returns
 *      BCM_E_*
 */
static int
_bcm_caladan3_qos_map_translate(int unit, int32 ing_map, int32 egr_map,
                                int32 *ing_idx, int32 *egr_idx)
{
    int rv = BCM_E_NONE;

    if (ing_map == -1 || ing_map == 0) {
        *ing_idx = ing_map;
    } else if (QOS_MAP_ID_EGRESS_OFFSET > ing_map) {
        /* ingress map */
        *ing_idx = ing_map;
        if (max_qos_profile <= ing_map) {
            rv = BCM_E_NOT_FOUND;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "invalid ingress map %d specified\n"),
                   ing_map));
        return BCM_E_NOT_FOUND;
    }

    if (egr_map == -1 || egr_map == 0) {
        *egr_idx = egr_map;
    } else if (QOS_MAP_ID_EGRESS_OFFSET < egr_map) {
        /* egress map */
        *egr_idx = egr_map - QOS_MAP_ID_EGRESS_OFFSET;
        if (max_qos_profile <= *egr_idx) {
            rv = BCM_E_NOT_FOUND;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "invalid egress map %d specified\n"),
                   egr_map));
        return BCM_E_NOT_FOUND;
    }

    return rv;
}


/*
 *  Function
 *    _bcm_caladan3_qos_map_flags_get
 *  Purpose
 *    Get soft-state flag for given hardware qos indexex
 *  Arguments
 *    (in)  unit       - bcm unit number
 *    (in)  ing_idx    - qos profile
 *    (in)  egr_idx    - remark index
 *    (out) ing_flags  - BCM_QOS_MAP_* flags for ingress mapping
 *    (out) egr_flags  - BCM_QOS_MAP_* flags for egress mapping
 *  Returns
 *      BCM_E_*
 *  Notes:
 *     Assumes lock is taken
 */
static int
_bcm_caladan3_qos_map_flags_get(int unit, int32 ing_idx, int32 egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags)
{
    if ((ing_idx >= SBX_QOS_PROFILE_BASE) &&
        (~0 == _qosInfo[unit]->ingrFlags[ing_idx]))
      {
          LOG_ERROR(BSL_LS_BCM_QOS,
                    (BSL_META_U(unit,
                                "unit %d ingress map not created here\n"),
                     unit));
          return BCM_E_NOT_FOUND;
      }

    if ((egr_idx >= SBX_QOS_EGR_REMARK_BASE) &&
        (~0 == _qosInfo[unit]->egrFlags[egr_idx]))
      {
          LOG_ERROR(BSL_LS_BCM_QOS,
                    (BSL_META_U(unit,
                                "unit %d egress map not created here\n"),
                     unit));
          return BCM_E_NOT_FOUND;
      }

    *ing_flags = _qosInfo[unit]->ingrFlags[ing_idx];
    *egr_flags = _qosInfo[unit]->egrFlags[egr_idx];

    return BCM_E_NONE;
}

/*
 *  Function
 *    bcm_caladan3_qos_map_add
 *  Purpose
 *    Set values in a QoS map
 *  Arguments
 *    int unit = the unit to affect
 *    uint32 flags = must be zero
 *    bcm_qos_map_t *map = pointer to map data to set
 *    int map_id = which map needs its data set
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    See notes for _bcm_caladan3_qos_map_data_set, above.
 */
int
bcm_caladan3_qos_map_add(int unit, uint32 flags, bcm_qos_map_t *map, int map_id)
{
    return _bcm_caladan3_qos_map_data_set(unit, flags, map, map_id, FALSE);
}


/*
 *  Function
 *    _bcm_caladan3_qos_map_multi_get
 *  Purpose
 *    Get values in a QoS map
 *  Arguments
 *    int unit = the unit to affect
 *    uint32 flags = configuration flags
 *    int map_id = map to retrieve
 *    int array_size = number of elements in array; or 0
 *    bcm_qos_map_t *array = array of maps to be filled
 *    int array_count = number of elements filled by API
 *  Returns
 *      BCM_E_*
 *  Notes
 *    No parameter checking - assumes exactly one map is specfied
 *    lock has been taken
 *
 */
static int
_bcm_caladan3_qos_map_multi_get(int unit, uint32 flags, int map_id,
                                int array_size, bcm_qos_map_t *array,
                                int* array_count)
{
    uint32                   profile;
    int                      rv, de, pri, dscp;
    int                      cos, dp, ecn;
    bcm_qos_map_t           *qmap = array;
    soc_sbx_g3p1_qos_t       qos;
    soc_sbx_g3p1_dscpqos_t  dqos;
    soc_sbx_g3p1_remark_t    remark;

    /* convert map_id to an SBX profile */
    if (map_id != 0 && flags & BCM_QOS_MAP_EGRESS) {
        profile = map_id - QOS_MAP_ID_EGRESS_OFFSET;
    } else {
        profile = map_id;
    }

    rv = BCM_E_NONE;
    if (flags & BCM_QOS_MAP_INGRESS) {
        if (flags & (BCM_QOS_MAP_L2 | BCM_QOS_MAP_MPLS)) {

            for (de=0; de<2; de++) {
                for (pri=0; pri<8; pri++) {

                    rv = soc_sbx_g3p1_qos_get(unit, de, pri, profile, &qos);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "Failed to read qos[%d, %d, %d]"
                                               ": %s\n"),
                                   de, pri, profile, bcm_errmsg(rv)));
                        return rv;
                    }

                    bcm_qos_map_t_init(qmap);
                    qmap->pkt_pri        = pri;
                    qmap->pkt_cfi        = de;
                    qmap->int_pri        = qos.fcos;
                    qmap->remark_int_pri = qos.cos;
                    qmap->remark_color   = qos.dp;
                    qmap->policer_offset = qos.mefcos;
                    qmap++;
                }
            }
        } else if (flags & BCM_QOS_MAP_L3) {
            for (dscp=0; dscp<*array_count; dscp++) {
                rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, profile, &dqos);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_QOS,
                              (BSL_META_U(unit,
                                          "Failed to read dscp_qos[%d, %d]"
                                           ": %s\n"),
                               dscp, profile, bcm_errmsg(rv)));
                    return rv;
                }

                bcm_qos_map_t_init(qmap);
                qmap->dscp           = dscp;
                qmap->int_pri        = dqos.fcos;
                qmap->remark_int_pri = dqos.cos;
                qmap->remark_color   = dqos.dp;
                qmap->policer_offset = dqos.mefcos;
                qmap++;
            }
        }
    }

    if (flags & BCM_QOS_MAP_EGRESS) {
        for (ecn=0; ecn<2; ecn++) {
            for (dp=0; dp<4; dp++) {
                for (cos=0; cos<8; cos++) {

                    rv = soc_sbx_g3p1_remark_get(unit, ecn, dp, cos,
                                                 profile, &remark);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_QOS,
                                  (BSL_META_U(unit,
                                              "Failed to read "
                                               "remark[%d, %d, %d, %d]"
                                               ": %s\n"),
                                   ecn, dp, cos, profile, bcm_errmsg(rv)));
                        return rv;
                    }

                    bcm_qos_map_t_init(qmap);

                    qmap->remark_color   = dp;
                    qmap->remark_int_pri = cos;

                    if (flags & BCM_QOS_MAP_L2) {

                        qmap->pkt_pri = remark.pri;
                        qmap->pkt_cfi = remark.cfi;
                    } else if (flags & BCM_QOS_MAP_MPLS) {

                        qmap->exp = remark.exp;
                    } else if (flags & BCM_QOS_MAP_L3) {
                        qmap->dscp = remark.dscp;
                    }

                    qmap++;
                }
            }
        }
    }

    return rv;
}


/*
 *  Function
 *    bcm_caladan3_qos_map_multi_get
 *  Purpose
 *    Get values in a QoS map
 *  Arguments
 *    int unit = the unit to affect
 *    uint32 flags = configuration flags
 *    int map_id = map to retrieve
 *    int array_size = number of elements in array; or 0
 *    bcm_qos_map_t *array = array of maps to be filled
 *    int array_count = number of elements filled by API
 *  Returns
 *      BCM_E_*
 *  Notes
 *    if array_size is passed as 0, array will be ignored and array_count will
 *   return with the number of elements required to satisfy the request.
 *
 */
int
bcm_caladan3_qos_map_multi_get(int unit, uint32 flags,
                               int map_id, int array_size,
                               bcm_qos_map_t *array, int *array_count)
{
    int rv = BCM_E_UNAVAIL;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,0x%08X,%d,%d,(0x%08x),(0x%08x)) enter\n"),
                 unit, flags, map_id, array_size, (int)array, (int)array_count));

    QOS_UNIT_INIT_CHECK(unit);

    /* Verify flags - only one map table can be retrieved at one time */
    if ( ((flags & QOS_MAP_ALL_DIRECTIONS) == QOS_MAP_ALL_DIRECTIONS)  ||
         ((flags & QOS_MAP_ALL_DIRECTIONS) == QOS_MAP_NO_DIRECTIONS) ||
         (((flags & QOS_MAP_ALL_LAYERS) != BCM_QOS_MAP_L2) &&
          ((flags & QOS_MAP_ALL_LAYERS) != BCM_QOS_MAP_L3) &&
          ((flags & QOS_MAP_ALL_LAYERS) != BCM_QOS_MAP_MPLS)))
      {
          LOG_ERROR(BSL_LS_BCM_QOS,
                    (BSL_META_U(unit,
                                "invalid flags, exaclty one mapping"
                                 " may be specfied: ingress or egress and"
                                 " l2 or l3 or mpls.\n")));
          return BCM_E_PARAM;
      }

    /* ingress or egress L2 is 16 mappings and
     * ingress mpls only is 16 mappings */
    if ((flags & BCM_QOS_MAP_L2) ||
        ((flags & (BCM_QOS_MAP_MPLS | BCM_QOS_MAP_INGRESS)) ==
         (BCM_QOS_MAP_MPLS | BCM_QOS_MAP_INGRESS)))
      {
          *array_count = 16;

      } else if (flags & BCM_QOS_MAP_MPLS) {
        *array_count = 8; /* implied egress */
    }

    if (flags & BCM_QOS_MAP_L3) {
        *array_count = max_qos_mapping;
    }

    if (array_size == 0) {
        return BCM_E_NONE;
    } else if ((array_size < *array_count) || (array == NULL) || map_id < 0) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Invalid Parameter aSize=%d aCount=%d"
                               " a=0x%x map_id=%d\n"),
                   array_size, *array_count, (int)array, map_id));
        *array_count = 0;
        return BCM_E_PARAM;
    }

    QOS_LOCK_TAKE(unit);
    rv = _bcm_caladan3_qos_map_multi_get(unit, flags, map_id,
                                         array_size, array, array_count);
    QOS_LOCK_GIVE(unit);

    return rv;
}


/*
 *  Function
 *    bcm_caladan3_qos_map_delete
 *  Purpose
 *    Revert values in a QoS map to their default values
 *  Arguments
 *    int unit = the unit to affect
 *    uint32 flags = must be zero
 *    bcm_qos_map_t *map = pointer to map data to set
 *    int map_id = which map needs its data set
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    See notes for _bcm_caladan3_qos_map_data_set, above.
 */
int
bcm_caladan3_qos_map_delete(int unit, uint32 flags, bcm_qos_map_t *map, int map_id)
{
    return _bcm_caladan3_qos_map_data_set(unit, flags, map, map_id, TRUE);
}

/*
 *  Function
 *    _bcm_caladan3_qos_port_map_set
 *  Purpose
 *    Attach a QoS map to a GPORT of some flavour
 *  Arguments
 *    int unit = which unit to affect
 *    bcm_gport_t port = the GPORT to which the QoS maps are to be attached
 *    int ing_map = the ingress map to attach
 *    int egr_map = the egress map to attach
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Locks taken, params checked by caller
 */
STATIC int
_bcm_caladan3_qos_port_map_set(int unit, bcm_gport_t port, int ing_map, int egr_map)
{
    int                   rv, gport_type;
    int32                 ing_idx, egr_idx;
    uint32                ing_flags, egr_flags;
    bcm_module_t          mymod = 0;
    int                   physical_port;

    rv = _bcm_caladan3_qos_map_translate(unit, ing_map, egr_map,
                                         &ing_idx, &egr_idx);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "invalid map specified\n")));
        return rv;
    }

    rv = _bcm_caladan3_qos_map_flags_get(unit, ing_idx, egr_idx,
                                         &ing_flags, &egr_flags);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Invalid MapId found: inr=%d egr=%d\n"),
                   ing_map, egr_map));
        return rv;
    }

    if (ing_idx == 0) {
        ing_flags = BCM_QOS_MAP_L2;
    }

    if (egr_idx == 0) {
        egr_flags = BCM_QOS_MAP_L2;
    }

    gport_type = port >> _SHR_GPORT_TYPE_SHIFT;
    if (gport_type == _SHR_GPORT_NONE) {
        gport_type = _SHR_GPORT_TYPE_LOCAL;
        BCM_GPORT_LOCAL_SET(port, port);
    }

    /* assume gport is a local gport */
    physical_port = BCM_GPORT_LOCAL_GET(port);

    switch (gport_type)
      {
       case _SHR_GPORT_TYPE_VLAN_PORT:
           rv = bcm_caladan3_vlan_port_qosmap_set(unit, port,
                                                  ing_idx, egr_idx,
                                                  ing_flags, egr_flags);
           break;
       case _SHR_GPORT_TYPE_MPLS_PORT:
           rv = bcm_caladan3_mpls_port_qosmap_set(unit, port,
                                                  ing_idx, egr_idx,
                                                  ing_flags, egr_flags);
           break;


       case _SHR_GPORT_TYPE_MODPORT:
           physical_port = BCM_GPORT_MODPORT_PORT_GET(port);

           rv = bcm_stk_my_modid_get(unit, &mymod);
           if (BCM_FAILURE(rv)) {
               LOG_ERROR(BSL_LS_BCM_QOS,
                         (BSL_META_U(unit,
                                     "failed to get modId: %d %s\n"),
                          rv, bcm_errmsg(rv)));
               return rv;
           }

           if (BCM_GPORT_MODPORT_MODID_GET(port) != mymod) {
               return rv;
           }

           /* fall thru intentional */
       case _SHR_GPORT_TYPE_LOCAL:
           if (!(SOC_PORT_VALID(unit, physical_port) &&
                 SOC_SBX_PORT_ADDRESSABLE(unit, physical_port))) {
               LOG_ERROR(BSL_LS_BCM_QOS,
                         (BSL_META_U(unit,
                                     "invalid port: %d (0x%x)\n"),
                          port, port));
               return BCM_E_PARAM;
           }

           rv = bcm_caladan3_port_qosmap_set(unit, physical_port,
                                             ing_idx, egr_idx,
                                             ing_flags, egr_flags);
           break;

       case _SHR_GPORT_TYPE_MIM_PORT:
#ifdef  BCM_CALADAN3_MIM_SUPPORT 
           /* Need mim port */
           rv = _bcm_caladan3_mim_qosmap_set(unit, port,
                                           ing_idx, egr_idx,
                                           ing_flags, egr_flags);
#else
           rv = BCM_E_INTERNAL;
#endif
           break;

       default:
           LOG_ERROR(BSL_LS_BCM_QOS,
                     (BSL_META_U(unit,
                                 "don't know how to add QoS maps to unit"
                                  " %d gport %08X\n"),
                      unit, port));
           rv = BCM_E_PARAM;
      }

    return rv;
}


/*
 *  Function
 *    bcm_caladan3_qos_port_map_set
 *  Purpose
 *    Attach a QoS map to a GPORT of some flavour
 *  Arguments
 *    int unit = which unit to affect
 *    bcm_gport_t port = the GPORT to which the QoS maps are to be attached
 *    int ing_map = the ingress map to attach
 *    int egr_map = the egress map to attach
 *  Returns
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    This API associates a GPORT with an ingress and an egress QoS map. A map
 *    ID of zero will clear the existing QoS map and a map ID of -1 will leave
 *    the existing map unchanged.
 *
 *    We don't requre that the map type be appropriate for the port type.
 */
int
bcm_caladan3_qos_port_map_set(int unit, bcm_gport_t port, int ing_map, int egr_map)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int rv = BCM_E_INTERNAL;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%d) enter\n"),
                 unit, port, ing_map, egr_map));

    QOS_UNIT_INIT_CHECK(unit);

    QOS_LOCK_TAKE(unit);
    rv = _bcm_caladan3_qos_port_map_set(unit, port, ing_map, egr_map);
    QOS_LOCK_GIVE(unit);

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%d) return %d (%s)\n"),
                 unit, port, ing_map, egr_map, rv, _SHR_ERRMSG(rv)));
    return rv;
#else /* ifdef BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d,%08X,%d,%d) not supported\n"),
               unit, port, ing_map, egr_map));
    return BCM_E_UNAVAIL;
#endif /* ifdef BCM_CALADAN3_G3P1_SUPPORT */
}


/* bcm_qos_port_map_get */
int
bcm_caladan3_qos_port_map_get(int unit, bcm_gport_t port,
                              int *ing_map, int *egr_map)
{
    int          rv = BCM_E_NONE;
    int32        gport_type;
    uint32       ing_flags, egr_flags;
    bcm_module_t mymod = 0;
    int          physical_port;

    gport_type = port >> _SHR_GPORT_TYPE_SHIFT;
    if (gport_type == _SHR_GPORT_NONE) {
        gport_type = _SHR_GPORT_TYPE_LOCAL;
        BCM_GPORT_LOCAL_SET(port, port);
    }

    /* assume gport is a local gport */
    physical_port = BCM_GPORT_LOCAL_GET(port);

    QOS_LOCK_TAKE(unit);

    switch (gport_type)
      {
       case _SHR_GPORT_TYPE_VLAN_PORT:
           rv = bcm_caladan3_vlan_port_qosmap_get(unit, port, ing_map, egr_map,
                                                  &ing_flags, &egr_flags);
           break;
       case _SHR_GPORT_TYPE_MPLS_PORT:
           rv = bcm_caladan3_mpls_port_qosmap_get(unit, port, ing_map, egr_map,
                                                  &ing_flags, &egr_flags);
           break;
       case _SHR_GPORT_TYPE_MODPORT:
           physical_port = BCM_GPORT_MODPORT_PORT_GET(port);

           rv = bcm_stk_my_modid_get(unit, &mymod);
           if (BCM_FAILURE(rv)) {
               LOG_ERROR(BSL_LS_BCM_QOS,
                         (BSL_META_U(unit,
                                     "Failed to get local modid: %s\n"),
                          bcm_errmsg(rv)));
               break;
           }

           if (BCM_GPORT_MODPORT_MODID_GET(port) != mymod) {
               break;
           }

           /* fall thru intentional */
       case _SHR_GPORT_TYPE_LOCAL:
           if (!(SOC_PORT_VALID(unit, physical_port) &&
                 SOC_SBX_PORT_ADDRESSABLE(unit, physical_port))) {
               LOG_ERROR(BSL_LS_BCM_QOS,
                         (BSL_META_U(unit,
                                     "invalid port: %d (0x%x)\n"),
                          port, port));
               return BCM_E_PARAM;
           }

           rv = bcm_caladan3_port_qosmap_get(unit, physical_port,
                                             ing_map, egr_map,
                                             &ing_flags, &egr_flags);
           break;

       case _SHR_GPORT_TYPE_MIM_PORT:
#ifdef  BCM_CALADAN3_MIM_SUPPORT
           /* Need mim port */
           rv = _bcm_caladan3_mim_qosmap_get(unit, port, ing_map, egr_map,
                                             &ing_flags, &egr_flags);
#else
           rv = BCM_E_INTERNAL;
#endif
           break;
       default:
           LOG_ERROR(BSL_LS_BCM_QOS,
                     (BSL_META_U(unit,
                                 "Invalid GPORT: 0x%08X\n"),
                      port));
           rv = BCM_E_PARAM;
      }

    if (BCM_SUCCESS(rv)) {
        if (*egr_map) {
            *egr_map += QOS_MAP_ID_EGRESS_OFFSET;
        }
    }

    QOS_LOCK_GIVE(unit);

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_qos_port_vlan_map_set
 *  Purpose
 *    Attach a QoS map to port,vid
 *  Arguments
 *    int unit = which unit to affect
 *    bcm_port_t port = the port to which the QoS maps are to be attached
 *    bcm_vlan_t vid = vid to which the QoS maps are to be attached
 *    int ing_map = the ingress map to attach
 *    int egr_map = the egress map to attach
 *  Returns
 *      BCM_E_*
 *  Notes
 *    This API associates a port,vid with an ingress and an egress QoS map.
 *    A map ID of zero will clear the existing QoS map and a map ID of -1 will
 *    leave the existing map unchanged.
 *
 *    Values for vid have the following meanings:
 *      BCM_VLAN_NONE       : mapping is applied to untagged packets
 *      BCM_VLAN_ALL        : mapping applied to all vids, including untagged
 *      BCM_VLAN_VALID(vid) : mapping applied to the particular <port,vid>.
 *
 */
int
bcm_caladan3_qos_port_vlan_map_set(int unit, bcm_port_t port, bcm_vlan_t vid,
                                   int ing_map, int egr_map)
{
    int rv;
    int32  ing_idx, egr_idx;
    uint32 ing_flags, egr_flags;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,0x%08x,%d,%d) enter\n"),
                 unit, port, vid, ing_map, egr_map));

    QOS_UNIT_INIT_CHECK(unit);

    /* translate from bcm map ids to internal indexes */
    rv = _bcm_caladan3_qos_map_translate(unit, ing_map, egr_map,
                                         &ing_idx, &egr_idx);
    if (BCM_FAILURE(rv)) {
        /* translate already output a more detailed error message */
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "invalid map specified\n")));
        return rv;
    }

    QOS_LOCK_TAKE(unit);

    rv = _bcm_caladan3_qos_map_flags_get(unit, ing_idx, egr_idx,
                                         &ing_flags, &egr_flags);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Invalid MapId: ing=%d egr=%d\n"),
                   ing_idx, egr_idx));
    }

    if (ing_idx == 0) {
        ing_flags = BCM_QOS_MAP_L2;
    }
    if (egr_idx == 0) {
        egr_flags = BCM_QOS_MAP_L2;
    }

    if (BCM_SUCCESS(rv)) {
        if (vid == BCM_VLAN_NONE || BCM_VLAN_VALID(vid)) {
            /* untagged mapping, or specific port,vid */
            rv = bcm_caladan3_port_vlan_qosmap_set(unit, port, vid,
                                                   ing_idx, egr_idx,
                                                   ing_flags, egr_flags);

        } else if (vid == BCM_VLAN_ALL) {
            /* apply to all port,vids; including untagged; which is the same as
             * setting it for the entire port
             */
            rv = bcm_caladan3_port_qosmap_set(unit, port, ing_idx, egr_idx,
                                              ing_flags, egr_flags);
        } else {
            LOG_ERROR(BSL_LS_BCM_QOS,
                      (BSL_META_U(unit,
                                  "Invalid vid: 0x%x\n"),
                       vid));
            rv = BCM_E_PARAM;
        }
    } /* BCM_SUCCESS */


    QOS_LOCK_GIVE(unit);

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,0x%03x,%d,%d) return %d (%s)\n"),
                 unit, port, vid, ing_map, egr_map, rv, _SHR_ERRMSG(rv)));

    return rv;
}



/*
 *  Function
 *    bcm_caladan3_qos_port_vlan_map_get
 *  Purpose
 *    Get a QoS mapping from a port,vid
 *  Arguments
 *    int unit = which unit to affect
 *    bcm_port_t port = the port to which the QoS maps are to be retrieved
 *    bcm_vlan_t vid = vid to which the QoS maps are to be retrived
 *    int *ing_map = configured ingress map
 *    int *egr_map = configured egress map
 *  Returns
 *      BCM_E_*
 *  Notes
 *
 *    Values for vid have the following meanings:
 *      BCM_VLAN_NONE       : mapping is applied to untagged packets
 *      BCM_VLAN_ALL        : mapping applied to all vids, including untagged
 *      BCM_VLAN_VALID(vid) : mapping applied to the particular <port,vid>.
 *
 */
int
bcm_caladan3_qos_port_vlan_map_get(int unit, bcm_port_t port, bcm_vlan_t vid,
                                   int *ing_map, int *egr_map)
{
    int rv;
    uint32 ing_flags, egr_flags;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,0x%08x) enter\n"),
                 unit, port, vid));

    QOS_UNIT_INIT_CHECK(unit);

    QOS_LOCK_TAKE(unit);

    if (vid == BCM_VLAN_NONE || BCM_VLAN_VALID(vid)) {
        /* untagged mapping, or specific port,vid */
        rv = bcm_caladan3_port_vlan_qosmap_get(unit, port, vid,
                                               ing_map, egr_map,
                                               &ing_flags, &egr_flags);

    } else if (vid == BCM_VLAN_ALL) {
        /* apply to all port,vids; including untagged; which is the same as
         * setting it for the entire port
         */
        rv = bcm_caladan3_port_qosmap_get(unit, port, ing_map, egr_map,
                                          &ing_flags, &egr_flags);
    } else {
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "Invalid vid: 0x%x\n"),
                   vid));
        rv = BCM_E_PARAM;
    }

    QOS_LOCK_GIVE(unit);

    if (BCM_SUCCESS(rv)) {
        if (*egr_map) {
            *egr_map += QOS_MAP_ID_EGRESS_OFFSET;
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,0x%03x,%d,%d) return %d (%s)\n"),
                 unit, port, vid, *ing_map, *egr_map, rv, _SHR_ERRMSG(rv)));

    return rv;
}


/*
 *  Function
 *    _bcm_caladan3_qos_map_validate
 *  Purpose
 *    Ensure a QoS map ID is valid from another module.
 *  Arguments
 *    int unit = the unit on which the verification is to be made
 *    int qosMapId = the QoS map ID to check
 *    uint32 *qosMapRaw = where to put raw QoS map index
 *  Returns
 *      BCM_E_NONE for success and valid map ID
 *      BCM_E_NOT_FOUND for success but invalid or unowned map ID
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    This does not take the unit's QoS lock.  This is because the verification
 *    is a read-only test and if the user deletes or creates a QoS map at the
 *    same time as using it in another module, he deserves whatever happens.
 *
 *    Note that ingress and egress raw map indices overlap, so they must be
 *    tracked separately if you are manipulating both.
 *
 *    NULL is acceptable for qosMapRaw if you don't care about it.
 */
int
_bcm_caladan3_qos_map_validate(int unit, int qosMapId, uint32 *qosMapRaw)
{
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    int rv = BCM_E_INTERNAL;
    int map = -1;

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,*) enter\n"),
                 unit,
                 qosMapId));

    if (SOC_IS_SBX_G3P1(unit)) {
        QOS_UNIT_INIT_CHECK(unit);
        if (QOS_MAP_ID_EGRESS_OFFSET > qosMapId) {
            /* ingress map */
            map = qosMapId;

            if ((max_qos_profile <= map) || (SBX_QOS_PROFILE_BASE > map)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unit %d ingress map %d not valid\n"),
                           unit,
                           qosMapId));
                rv = BCM_E_NOT_FOUND;
            } else if (~0 == _qosInfo[unit]->ingrFlags[map]) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unit %d ingress map %d not created here\n"),
                           unit,
                           qosMapId));
                rv = BCM_E_NOT_FOUND;
            } else {
                rv = BCM_E_NONE;
            }
        } else {
            /* egress map */
            map = qosMapId - QOS_MAP_ID_EGRESS_OFFSET;
            if ((max_qos_profile <= map) ||
                (SBX_QOS_EGR_REMARK_BASE > map)) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unit %d egress map %d not valid\n"),
                           unit,
                           qosMapId));
                rv = BCM_E_NOT_FOUND;
            } else if (~0 == _qosInfo[unit]->egrFlags[map]) {
                LOG_ERROR(BSL_LS_BCM_QOS,
                          (BSL_META_U(unit,
                                      "unit %d egress map %d not created here\n"),
                           unit,
                           qosMapId));
                rv = BCM_E_NOT_FOUND;
            } else {
                rv = BCM_E_NONE;
            }
        }
    } else { /* if (SOC_IS_SBX_G3P1(unit)) */
        LOG_ERROR(BSL_LS_BCM_QOS,
                  (BSL_META_U(unit,
                              "unit %d microcode is not supported\n"),
                   unit));
        rv = BCM_E_UNAVAIL;
    } /* if (SOC_IS_SBX_G3P1(unit)) */

    /* fill in the raw map index if requested */
    if ((BCM_E_NONE == rv) && qosMapRaw) {
        *qosMapRaw = map;
    }

    LOG_VERBOSE(BSL_LS_BCM_QOS,
                (BSL_META_U(unit,
                            "(%d,%d,&(%d))  return %d (%s)\n"),
                 unit,
                 qosMapId,
                 qosMapRaw?(*qosMapRaw):-1,
                 rv,
                 _SHR_ERRMSG(rv)));
    return rv;
#else /* def BCM_CALADAN3_G3P1_SUPPORT */
    LOG_ERROR(BSL_LS_BCM_QOS,
              (BSL_META_U(unit,
                          "(%d,%08X,%d,%d) not supported\n"),
               unit,
               port,
               ing_map,
               egr_map));
    return BCM_E_UNAVAIL;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
}

/* bcm_qos_multi_get */
int
bcm_caladan3_qos_multi_get(int unit, int array_size, int *map_ids_array,
                           int *flags_array, int *array_count)
{
    return BCM_E_UNAVAIL;
}


