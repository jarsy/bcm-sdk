/*
 * $Id: stg.c,v 1.15.24.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stg.c
 * Purpose:     Spanning tree group support
 *
 * Multiple spanning trees (MST) is supported on this chipset
 */

#include <shared/bsl.h>

#include <shared/bitop.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>

#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/stg.h>
#include <bcm_int/sbx/caladan3/wb_db_stg.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#endif

bcm_stg_info_t  caladan3_stg_info[BCM_MAX_NUM_UNITS];

int bcm_caladan3_stg_destroy(int unit, bcm_stg_t stg);
int bcm_caladan3_stg_init(int unit);

#define STG_LEVEL_ONE  1

#define STG_CHECK_INIT(unit)                                           \
    do {                                                                \
        if (!BCM_UNIT_VALID(unit)) return BCM_E_UNIT;                  \
        if (unit >= BCM_MAX_NUM_UNITS) return BCM_E_UNIT;              \
        if (STG_CNTL(unit).init == FALSE) return BCM_E_INIT;           \
        if (STG_CNTL(unit).init != TRUE) return STG_CNTL(unit).init;  \
    } while (0);

#define STG_CHECK_STG(si, stg)                            \
    if ((stg) < (si)->stg_min || (stg) > (si)->stg_max) return BCM_E_BADID;

#define STG_CHECK_PORT(unit, port)                 \
    if (((port < 0) || (port >= SBX_MAX_PORTS))    \
        || (!IS_E_PORT((unit), (port))             \
            && !IS_HG_PORT((unit), (port))         \
            && !IS_IL_PORT((unit), (port))         \
            && !IS_SPI_SUBPORT_PORT((unit), (port))\
            && !IS_CPU_PORT((unit), (port)))) {    \
        return BCM_E_PORT;                         \
    }

#if 0
#define G2_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }
#endif

/*
 * Allocation bitmap macros
 */
#define STG_BITMAP_TST(si, stg)     SHR_BITGET(si->stg_bitmap, stg)
#define STG_BITMAP_SET(si, stg)     SHR_BITSET(si->stg_bitmap, stg)
#define STG_BITMAP_CLR(si, stg)     SHR_BITCLR(si->stg_bitmap, stg)

#define NUM_EGR_PVID2ETC_WORDS BCM_VLAN_COUNT/16

static bcm_vlan_t _stg_vlan_min     = BCM_VLAN_MIN + 1;
static bcm_vlan_t _stg_vlan_max     = BCM_VLAN_MAX - 1;
static bcm_stg_t  _stg_default      = BCM_STG_DEFAULT;
static bcm_stg_t  _stg_min          = 1;
static bcm_stg_t  _stg_max          = BCM_STG_MAX;

#ifdef BCM_WARM_BOOT_SUPPORT
extern bcm_caladan3_wb_stg_state_scache_info_t
    *_bcm_caladan3_wb_stg_state_scache_info_p[BCM_MAX_NUM_UNITS];
#endif

void
_bcm_caladan3_stg_sw_dump(int unit)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    bcm_vlan_t      vid;
    int             cnt, num_display_vids;

    num_display_vids = 8;

    si = &STG_CNTL(unit);
    if (FALSE == si->init) {
        LOG_CLI((BSL_META_U(unit,
                            "Unit %d STG not initialized\n"), unit));
        return;
    }

    LOG_CLI((BSL_META_U(unit,
                        "stg_min=%d stg_max=%d stg_default=%d allocated STGs=%d\n"),
             si->stg_min, si->stg_max, si->stg_defl, si->stg_count));
    LOG_CLI((BSL_META_U(unit,
                        "STG list:\nSTG :  VID list\n")));

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {

        if (STG_BITMAP_TST(si, stg)) {
            LOG_CLI((BSL_META_U(unit,
                                "%4d: "), stg));

            cnt = 0;
            for (vid = si->vlan_first[stg];
                 vid != BCM_VLAN_NONE;
                 vid = si->vlan_next[vid]) {
                
                if (cnt < num_display_vids) {
                    if (cnt==0) {
                        LOG_CLI((BSL_META_U(unit,
                                            "%d"), vid));
                    } else {
                        LOG_CLI((BSL_META_U(unit,
                                            ", %d"), vid));
                    }
                }

                cnt++;
            }

            if (cnt > num_display_vids) {
                LOG_CLI((BSL_META_U(unit,
                                    ", ... %d more"), cnt - num_display_vids));
            }

            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    
}

/*
 * Function:
 *      _bcm_caladan3_stg_map_add
 * Purpose:
 *      Add VLAN to STG linked list
 */
static void
_bcm_caladan3_stg_map_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_vlan_t  *wb_vlan_first;
    bcm_vlan_t  *wb_vlan_next;

    wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                  unit));
        goto exit;
    }
    wb_vlan_first = (bcm_vlan_t*)(wb_info_ptr->scache_ptr + wb_info_ptr->vlan_first_offset);
    wb_vlan_next = (bcm_vlan_t*)(wb_info_ptr->scache_ptr + wb_info_ptr->vlan_next_offset);
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg, vid));

    assert(BCM_VLAN_NONE != vid);

    si = &STG_CNTL(unit);

    si->vlan_next[vid] = si->vlan_first[stg];
#ifdef BCM_WARM_BOOT_SUPPORT
    wb_vlan_next[vid] = si->vlan_first[stg];
#endif
    si->vlan_first[stg] = vid;
#ifdef BCM_WARM_BOOT_SUPPORT
    wb_vlan_first[stg] = vid;

exit:
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit\n"),
               FUNCTION_NAME(), unit, stg, vid));
}

/*
 * Function:
 *      _bcm_caladan3_stg_map_delete
 * Purpose:
 *      Remove VLAN from STG linked list. No action if VLAN is not in list.
 */
static void
_bcm_caladan3_stg_map_delete(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    bcm_vlan_t     *vp;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_vlan_t  *wb_vlan_first;
    bcm_vlan_t  *wb_vp;
    bcm_vlan_t  *wb_vlan_next;

    wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                  unit));
        goto exit;
    }
    wb_vlan_first = (bcm_vlan_t*)(wb_info_ptr->scache_ptr + wb_info_ptr->vlan_first_offset);
    wb_vlan_next = (bcm_vlan_t*)(wb_info_ptr->scache_ptr + wb_info_ptr->vlan_next_offset);
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg, vid));

    assert(BCM_VLAN_NONE != vid);

    si = &STG_CNTL(unit);

    vp = &si->vlan_first[stg];
#ifdef BCM_WARM_BOOT_SUPPORT
    wb_vp = &wb_vlan_first[stg];
#endif

    while (BCM_VLAN_NONE != *vp) {
        if (*vp == vid) {
#ifdef BCM_WARM_BOOT_SUPPORT
            *wb_vp = wb_vlan_next[*vp];
#endif
            *vp = si->vlan_next[*vp];
        } else {
#ifdef BCM_WARM_BOOT_SUPPORT
            wb_vp = &wb_vlan_next[*vp];
#endif
            vp = &si->vlan_next[*vp];
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
exit:
#endif
    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit\n"),
               FUNCTION_NAME(), unit, stg, vid));
}

/*
 * Function:
 *      _bcm_caladan3_stg_map_get
 * Purpose:
 *      Get STG that a VLAN is mapped to.
 * Parameters:
 *      unit - device unit number.
 *      vid  - VLAN id to search for
 *      *stg - Spanning tree group id if found
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_caladan3_stg_map_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;
    int             index;
    bcm_vlan_t      vlan;

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, *) - Enter\n"),
               FUNCTION_NAME(), unit, vid));

    /* this "internal" function is public so check parms */
    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    assert(BCM_VLAN_NONE != vid);
    *stg = 0;

    for (index = si->stg_min; index < si->stg_max; index++) {
        vlan = si->vlan_first[index];

        while (BCM_VLAN_NONE != vlan) {
            if (vlan  == vid) {
                /* since a vlan may exist in only one STG, safe to exit */
                *stg = index;
                result = BCM_E_NONE;
                break;
            }

            vlan = si->vlan_next[vlan];
        }

        if (BCM_E_NONE == result) {
            break;
        }
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, vid, *stg, bcm_errmsg(result)));

    return result;
}



/*
 * Function:
 *      _bcm_caladan3_stg_vid_compare
 * Purpose:
 *      Compare routine for sorting on VLAN ID.
 */
static int
_bcm_caladan3_stg_vid_compare(void *a, void *b)
{
    uint16 a16, b16;

    a16 = *(uint16 *)a;
    b16 = *(uint16 *)b;

    return a16 - b16;
}


/*
 * Function:
 *      _bcm_caladan3_stg_vid_stp_set
 * Purpose:
 *      Set the spanning tree state for a port in specified VLAN.
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 *      stp_state - Port STP state.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_caladan3_stg_vid_stp_set(int unit, bcm_vlan_t vid, bcm_port_t port, int stp_state)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        return _bcm_caladan3_g3p1_stg_vid_stp_set(unit, vid, port, stp_state);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

/*
 * Function:
 *      _bcm_caladan3_stg_stp_get
 * Purpose:
 *      Retrieve the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - Spanning tree group id.
 *      port      - device port number.
 *      stp_state - (OUT)Port STP state int the group.
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_caladan3_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state)
{
    bcm_stg_info_t *si;
    int             state = 0;
    int             result = BCM_E_NONE; /* local result code */

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, *) - Enter\n"),
               FUNCTION_NAME(), unit, stg, port));

    /* Input parameters check. */
    STG_CHECK_PORT(unit, port);
    si = &STG_CNTL(unit);

    if (BCM_PBMP_MEMBER(si->stg_enable[stg], port)) {
        if (BCM_PBMP_MEMBER(si->stg_state_h[stg], port)) {
            state |= 0x2;
        }
        if (BCM_PBMP_MEMBER(si->stg_state_l[stg], port)) {
            state |= 0x1;
        }
        switch (state) {
        case 0:  *stp_state = BCM_STG_STP_BLOCK;
            break;
        case 1:  *stp_state = BCM_STG_STP_LISTEN;
            break;
        case 2:  *stp_state = BCM_STG_STP_LEARN;
            break;
        case 3:  *stp_state = BCM_STG_STP_FORWARD;
            break;
	/* coverity[dead_error_begin] */
        default: *stp_state = BCM_STG_STP_DISABLE;
        }
    } else {
        *stp_state = BCM_STG_STP_DISABLE;
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, port, *stp_state, bcm_errmsg(result)));

    return result;
}

#ifdef BCM_CALADAN3_G3P1_SUPPORT
int
_bcm_caladan3_g3p1_stg_fast_stp_set(int unit, bcm_stg_t stg,
                                      bcm_port_t port, int stp_state)
{
    int rv;
    bcm_vlan_t vid;
    bcm_stg_info_t *si; 
    int *fastSets;

    si = &STG_CNTL(unit);
    
    fastSets = sal_alloc(BCM_VLAN_COUNT * sizeof(int), "STG-fast sets");
    if (fastSets == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(fastSets, 0, BCM_VLAN_COUNT * sizeof(int));

    vid = si->vlan_first[stg];
    for (; BCM_VLAN_NONE != vid; vid = si->vlan_next[vid]) {
        fastSets[vid] = 1;
    }

    rv = _bcm_caladan3_g3p1_stp_fast_set(unit, port, stp_state, fastSets);

    sal_free(fastSets);

    return rv;
}
#endif

/*
 * Function:
 *      _bcm_caladan3_stg_stp_set
 * Purpose:
 *      Set the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - Spanning tree group id.
 *      port      - device port number.
 *      stp_state - (OUT)Port STP state int the group.
 * Returns:
 *      BCM_E_XXX
 */


#define _STG_VLAN_COUNT BCM_VLAN_COUNT

static int
_bcm_caladan3_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state)
{
    bcm_stg_info_t *si;
    bcm_vlan_t      vid;
    int             result = BCM_E_NOT_FOUND; /* local result code */
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_pbmp_t      *wb_pbmp_enable_ptr;
    bcm_pbmp_t      *wb_pbmp_state_h_ptr;
    bcm_pbmp_t      *wb_pbmp_state_l_ptr;

    wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                  unit));
        return BCM_E_INTERNAL;
    }
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg, port, stp_state));

    /* Input parameters check. */
    STG_CHECK_PORT(unit, port);

    si = &STG_CNTL(unit);

#ifdef BCM_CALADAN3_G3P1_SUPPORT
    if (SOC_IS_SBX_G3P1(unit) && !SAL_BOOT_BCMSIM) {
        result = _bcm_caladan3_g3p1_stg_fast_stp_set(unit, stg, port, stp_state);
    } else
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
    {
        /* get first vlan */
        vid = si->vlan_first[stg];

        while (BCM_VLAN_NONE != vid) {
            result = _bcm_caladan3_stg_vid_stp_set(unit, vid, port, stp_state);
            if (BCM_E_NONE != result) {
                break;
            }
            vid = si->vlan_next[vid];
        }
    }

    if (BCM_FAILURE(result)) {
        return result;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
        wb_pbmp_enable_ptr = (bcm_pbmp_t *)
            (wb_info_ptr->scache_ptr + wb_info_ptr->stg_enable_offset);
        wb_pbmp_state_h_ptr = (bcm_pbmp_t *)
            (wb_info_ptr->scache_ptr + wb_info_ptr->stg_state_h_offset);
        wb_pbmp_state_l_ptr = (bcm_pbmp_t *)
            (wb_info_ptr->scache_ptr + wb_info_ptr->stg_state_l_offset);
#endif

    if (stp_state == BCM_STG_STP_DISABLE) {
        BCM_PBMP_PORT_REMOVE(si->stg_enable[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_REMOVE(wb_pbmp_enable_ptr[stg], port);
#endif

    } else {
        BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_ADD(wb_pbmp_enable_ptr[stg], port);
#endif

        if ((stp_state == BCM_STG_STP_LEARN) ||
            (stp_state == BCM_STG_STP_FORWARD)) {
            BCM_PBMP_PORT_ADD(si->stg_state_h[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_ADD(wb_pbmp_state_h_ptr[stg], port);
#endif
        } else {
            BCM_PBMP_PORT_REMOVE(si->stg_state_h[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_REMOVE(wb_pbmp_state_h_ptr[stg], port);
#endif
        }
        if ((stp_state == BCM_STG_STP_LISTEN) ||
            (stp_state == BCM_STG_STP_FORWARD)) {
            BCM_PBMP_PORT_ADD(si->stg_state_l[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_ADD(wb_pbmp_state_l_ptr[stg], port);
#endif
        } else {
            BCM_PBMP_PORT_REMOVE(si->stg_state_l[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_REMOVE(wb_pbmp_state_l_ptr[stg], port);
#endif
        }
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, port, stp_state, bcm_errmsg(result)));


    return result;
}


/*
 * Function:
 *      _bcm_caladan3_stg_vlan_add
 * Purpose:
 *      Main part of bcm_stg_vlan_add; assumes locks already acquired.
 * Parameters:
 *      unit    - device unit number.
 *      stg     - spanning tree group id.
 *      vid     - vlan to add
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The vlan is removed from it's current STG if necessary.
 */
static int
_bcm_caladan3_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg_cur;
    int             result = BCM_E_NOT_FOUND; /* local result code */

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg, vid));

    si = &STG_CNTL(unit);

    if ((_stg_vlan_min > vid) || (_stg_vlan_max < vid)) {
        result = BCM_E_PARAM;

    } else if (STG_BITMAP_TST(si, stg)) { /* STG must already exist */

        /* Get the STG the VLAN is currently associated to */
        result = _bcm_caladan3_stg_map_get(unit, vid, &stg_cur);
        if ((BCM_E_NONE == result) || (BCM_E_NOT_FOUND == result)) {

            /* iff found, delete it */
            if (BCM_E_NONE == result) {
                _bcm_caladan3_stg_map_delete(unit, stg_cur, vid);
            }
        }

        /* Set the new STG */
        _bcm_caladan3_stg_map_add(unit, stg, vid);

        result = BCM_E_NONE;
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_caladan3_stg_vlan_remove
 * Purpose:
 *      Main part of bcm_stg_vlan_remove; assumes lock already acquired.
 * Parameters:
 *      unit    - device unit number.
 *      stg     - spanning tree group id.
 *      vid     - vlan id to remove
 *      destroy - boolean flag indicating the VLAN is being destroyed and is
 *                not to be added to the default STG. Also used internally
 *                to supress default STG assignment during transition.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
_bcm_caladan3_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid, int destroy)
{
    bcm_stg_info_t *si;
    int             stg_cur;
    int             result = BCM_E_NOT_FOUND; /* local result code */

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg, vid, destroy));

    si = &STG_CNTL(unit);

    /* STG must already exist */
    if (STG_BITMAP_TST(si, stg)) {

        /* Get the STG the VLAN is currently associated to */
        result = _bcm_caladan3_stg_map_get(unit, vid, &stg_cur);
        if ((BCM_E_NONE == result) && ((stg == stg_cur))) {
            _bcm_caladan3_stg_map_delete(unit, stg, vid);

            /* If the VLAN is not being destroyed, set the VLAN to the default STG */
            if (FALSE == destroy) {
                _bcm_caladan3_stg_map_add(unit, si->stg_defl, vid);
            }
        }
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, vid, destroy, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_caladan3_stg_vlan_port_add
 * Purpose:
 *      Callout for vlan code to get a port that has been added to a vlan into
 *      the proper STP state
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_caladan3_stg_vlan_port_add(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    int         result = BCM_E_FAIL; /* local result code */
    bcm_stg_t   stg;
    int         state;

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, vid, port));

    result = _bcm_caladan3_stg_map_get(unit, vid, &stg);

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_stg_stp_get(unit, stg, port, &state);
    }

    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_stg_vid_stp_set(unit, vid, port, state);
    }

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, vid, port, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_caladan3_stg_vlan_port_remove
 * Purpose:
 *      Callout for vlan code to get a port that has been removed from a vlan
 *      into the proper STP state
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 * Returns:
 *      BCM_E_NONE - This function is best effort
 */
int
_bcm_caladan3_stg_vlan_port_remove(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, vid, port));

    (void)_bcm_caladan3_stg_vid_stp_set(unit, vid, port, BCM_STG_STP_BLOCK);

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, vid, port, bcm_errmsg(BCM_E_NONE)));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_stg_stp_init
 * Purpose:
 *      Initialize spanning tree group on a single device.
 * Parameters:
 *      unit - SOC unit number.
 *      stg  - Spanning tree group id.
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_caladan3_stg_stp_init(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    bcm_port_t      port;
    int             result = BCM_E_NONE;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_pbmp_t      *wb_pbmp_enable_ptr;
    bcm_pbmp_t      *wb_pbmp_state_h_ptr;
    bcm_pbmp_t      *wb_pbmp_state_l_ptr;
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg));

    si = &STG_CNTL(unit);

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"), unit));
        return BCM_E_INTERNAL;
    }
    wb_pbmp_enable_ptr = (bcm_pbmp_t *)
        (wb_info_ptr->scache_ptr + wb_info_ptr->stg_enable_offset);
    wb_pbmp_state_h_ptr = (bcm_pbmp_t *)
        (wb_info_ptr->scache_ptr + wb_info_ptr->stg_state_h_offset);
    wb_pbmp_state_l_ptr = (bcm_pbmp_t *)
        (wb_info_ptr->scache_ptr + wb_info_ptr->stg_state_l_offset);
#endif

    PBMP_ALL_ITER(unit, port) {

        if (SOC_RECONFIG_TDM &&
            SOC_PBMP_MEMBER(SOC_CONTROL(unit)->all_skip_pbm, port)) {
                continue;
        }

        /* place port in blocked */
        BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);


        BCM_PBMP_PORT_REMOVE(si->stg_state_h[stg], port);
        BCM_PBMP_PORT_REMOVE(si->stg_state_l[stg], port);
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_PBMP_PORT_ADD(wb_pbmp_enable_ptr[stg], port);
        BCM_PBMP_PORT_REMOVE(wb_pbmp_state_h_ptr[stg], port);
        BCM_PBMP_PORT_REMOVE(wb_pbmp_state_l_ptr[stg], port);
#endif
    }
    
    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, bcm_errmsg(BCM_E_NONE)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_vlan_list
 * Purpose:
 *      Return a list of VLANs in a specified Spanning Tree Group (STG).
 * Parameters:
 *      unit  - device unit number
 *      stg   - STG ID to list
 *      list  - Place where pointer to return array will be stored.
 *              Will be NULL if there are zero VLANs returned.
 *      count - Place where number of entries in array will be stored.
 *              Will be 0 if there are zero VLANs associated to the STG.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the Spanning Tree Group (STG) requested is not defined in the
 *      system, call returns NULL list and count of zero (0). The caller is
 *      responsible for freeing the memory that is returned, using
 *      bcm_stg_vlan_list_destroy().
 */
int
bcm_caladan3_stg_vlan_list(int unit, bcm_stg_t stg, bcm_vlan_t **list,
                         int *count)
{
    bcm_stg_info_t *si;
    bcm_vlan_t      vlan;
    int             index;
    int             result = BCM_E_NOT_FOUND;

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d, *, *) - Enter\n"),
               FUNCTION_NAME(), unit, stg));
    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    *list = NULL;
    *count = 0;

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        /* Traverse list once just to get an allocation count */
        vlan = si->vlan_first[stg];

        while (BCM_VLAN_NONE != vlan) {
            (*count)++;
            vlan = si->vlan_next[vlan];
        }

        if (0 == *count) {
            result = BCM_E_NONE;
        }
        else {
            *list = sal_alloc(*count * sizeof (bcm_vlan_t), "bcm_caladan3_stg_vlan_list");

            if (NULL == *list) {
                result = BCM_E_MEMORY;
            }
            else {
                /* Traverse list a second time to record the VLANs */
                vlan = si->vlan_first[stg];
                index = 0;

                while (BCM_VLAN_NONE != vlan) {
                    (*list)[index++] = vlan;
                    vlan = si->vlan_next[vlan];
                }

                /* Sort the vlan list */
                _shr_sort(*list, *count, sizeof (bcm_vlan_t), _bcm_caladan3_stg_vid_compare);

                result = BCM_E_NONE;
            }
        }

    }
    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, *, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, *count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_vlan_list_destroy
 * Purpose:
 *      Destroy a list returned by bcm_caladan3_stg_vlan_list.
 * Parameters:
 *      unit  - device unit number
 *      list  - Pointer to VLAN array to be destroyed.
 *      count - Number of entries in array.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_vlan_list_destroy(int unit, bcm_vlan_t *list, int count)
{
    int     result = BCM_E_NONE;
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, count));

    if (NULL != list) {
        sal_free(list);
    }

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, count, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_default_get
 * Purpose:
 *      Returns the default STG for the device.
 * Parameters:
 *      unit    - device unit number.
 *      stg_ptr - STG ID for default.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_default_get(int unit, bcm_stg_t *stg_ptr)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    *stg_ptr = si->stg_defl;

    STG_CHECK_STG(si, *stg_ptr);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, *stg_ptr, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_default_set
 * Purpose:
 *      Changes the default STG for the device.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      stg  - STG ID to become default.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The specified STG must already exist.
 */
int
bcm_caladan3_stg_default_set(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif


    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        si->stg_defl = stg;
#if defined(BCM_WARM_BOOT_SUPPORT)
    SBX_WB_DB_SYNC_VARIABLE_OFFSET(bcm_stg_t, 1,
        wb_info_ptr->stg_defl_offset, stg);
#endif

    } else {

        /* The stg does not exist */
        result = BCM_E_NOT_FOUND;

    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_vlan_add
 * Purpose:
 *      Add a VLAN to a spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be added to STG
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Spanning tree group ID must have already been created. The
 *      VLAN is removed from the STG it is currently in.
 */
int
bcm_caladan3_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_BADID;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg, vid));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (BCM_VLAN_VALID(vid)) {
        result = _bcm_caladan3_stg_vlan_add(unit, stg, vid);
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_vlan_remove
 * Purpose:
 *      Remove a VLAN from a spanning tree group. The VLAN is placed in the
 *      default spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be removed from STG
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_BADID;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg, vid));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (BCM_VLAN_VALID(vid)) {
        result = _bcm_caladan3_stg_vlan_remove(unit, stg, vid, FALSE);
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_vlan_remove_all
 * Purpose:
 *      Remove all VLANs from a spanning tree group. The VLANs are placed in
 *      the default spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to clear
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_vlan_remove_all(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;
    bcm_vlan_t      vid;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (stg != si->stg_defl) {

        if (!STG_BITMAP_TST(si, stg)) {   /* STG must already exist */
            result = BCM_E_NOT_FOUND;
        } else {
            vid = si->vlan_first[stg];
            while (BCM_VLAN_NONE != vid) {
                result = _bcm_caladan3_stg_vlan_remove(unit, stg, vid, FALSE);
                if (BCM_E_NONE != result) {
                    break;
                }

                result = _bcm_caladan3_stg_vlan_add(unit, si->stg_defl, vid);
                if (BCM_E_NONE != result) {
                    break;
                }

                /*
                 * Iterate through list. The vid just removed was popped from
                 * the list. Use the new first.
                 */
                vid = si->vlan_first[stg];
            }
        }
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_clear
 * Description:
 *      Destroy all STGs
 * Parameters:
 *      unit - device unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_clear(int unit)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (STG_BITMAP_TST(si, stg)) {
            STG_DB_UNLOCK(unit);
            /* ignore error code as unit will be 'init'ed later. */
            bcm_caladan3_stg_destroy(unit, stg);
            STG_DB_LOCK(unit);
        }
    }

    STG_DB_UNLOCK(unit);

    result = bcm_caladan3_stg_init(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_create_id
 * Description:
 *      Create a STG, using a specified ID.
 * Parameters:
 *      unit - Device unit number
 *      stg -  STG to create
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      In the new STG, all ports are in the DISABLED state.
 */
int
bcm_caladan3_stg_create_id(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_EXISTS;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    uint32              *wb_bitop_ptr;
#endif

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (!STG_BITMAP_TST(si, stg)) {
        /* No device action needed */
        result = _bcm_caladan3_stg_stp_init(unit, stg);
        if (BCM_E_NONE == result) {
            STG_BITMAP_SET(si, stg);
            si->stg_count++;
#ifdef BCM_WARM_BOOT_SUPPORT
            wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
            if(wb_info_ptr == NULL) {
                LOG_ERROR(BSL_LS_BCM_STG,
                          (BSL_META_U(unit,
                                      "Warm boot not initialized for unit %d \n"),
                           unit));
                return BCM_E_INTERNAL;
            }
            wb_bitop_ptr = (uint32 *)(wb_info_ptr->scache_ptr + wb_info_ptr->stg_bitmap_offset);
            SHR_BITSET(wb_bitop_ptr, stg);
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
                wb_info_ptr->stg_count_offset, si->stg_count);
#endif
        }
    }

    STG_DB_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_create
 * Description:
 *      Create a STG, picking an unused ID and returning it.
 * Parameters:
 *      unit - device unit number
 *      stg_ptr - (OUT) the STG ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_create(int unit, bcm_stg_t *stg_ptr)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg = BCM_STG_INVALID;
    int             result = BCM_E_FULL;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (!STG_BITMAP_TST(si, stg)) {
            break;          /* free id found */
        }
    }

    if (si->stg_max >= stg) {
        result = bcm_caladan3_stg_create_id(unit, stg);
    }

    STG_DB_UNLOCK(unit);

    *stg_ptr = stg;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, *stg_ptr, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_destroy
 * Description:
 *      Destroy an STG.
 * Parameters:
 *      unit - device unit number.
 *      stg  - The STG ID to be destroyed.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The default STG may not be destroyed.
 */
int
bcm_caladan3_stg_destroy(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_PARAM;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    uint32              *wb_bitop_ptr;
#endif

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (si->stg_defl != stg) {
        /* The next call checks if STG exists as well as removing all VLANs */
        result = bcm_caladan3_stg_vlan_remove_all(unit, stg);

        if (BCM_E_NONE == result) {
            STG_BITMAP_CLR(si, stg);
            si->stg_count--;
#ifdef BCM_WARM_BOOT_SUPPORT
            wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
            if(wb_info_ptr == NULL) {
                LOG_ERROR(BSL_LS_BCM_STG,
                          (BSL_META_U(unit,
                                      "Warm boot not initialized for unit %d \n"),
                           unit));
                return BCM_E_INTERNAL;
            }
            wb_bitop_ptr = (uint32 *)(wb_info_ptr->scache_ptr + wb_info_ptr->stg_bitmap_offset);
            SHR_BITCLR(wb_bitop_ptr, stg);
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
                wb_info_ptr->stg_count_offset, si->stg_count);
#endif
        }
    }

    STG_DB_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_list
 * Purpose:
 *      Return a list of defined Spanning Tree Groups
 * Parameters:
 *      unit  - device unit number
 *      list  - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The caller is responsible for freeing the memory that is returned,
 *      using bcm_stg_list_destroy().
 */
int
bcm_caladan3_stg_list(int unit, bcm_stg_t **list, int *count)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    int             index = 0;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    if (0 == si->stg_count) {
        *count = 0;
        *list = NULL;
    } else {
        *count = si->stg_count;
        *list = sal_alloc(si->stg_count * sizeof (bcm_stg_t), "bcm_stg_list");

        if (NULL == *list) {
            result = BCM_E_MEMORY;
        } else {
            for (stg = si->stg_min; stg <= si->stg_max; stg++) {
                if (STG_BITMAP_TST(si, stg)) {
                    assert(index < *count);
                    (*list)[index++] = stg;
                }
            }
        }
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, *count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_list_destroy
 * Purpose:
 *      Destroy a list returned by bcm_stg_list.
 * Parameters:
 *      unit  - device unit number
 *      list  - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_NONE
 */
int
bcm_caladan3_stg_list_destroy(int unit, bcm_stg_t *list, int count)
{
    int     result = BCM_E_NONE;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, count));

    if (NULL != list) {
        sal_free(list);
    }

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_stp_set
 * Purpose:
 *      Set the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - Spanning Tree State of port.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, stg, port, stp_state));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);
    STG_CHECK_PORT(unit, port);
    if ((stp_state < BCM_STG_STP_DISABLE) || (stp_state > BCM_STG_STP_FORWARD))
        return BCM_E_PARAM;

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        result = _bcm_caladan3_stg_stp_set(unit, stg, port, stp_state);
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, port, stp_state, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_stg_stp_get
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int
bcm_caladan3_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, stg, port));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);
    STG_CHECK_PORT(unit, port);

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        result = _bcm_caladan3_stg_stp_get(unit, stg, port, stp_state);
    }

    STG_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, stg, port, *stp_state, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *     bcm_stg_count_get
 * Purpose:
 *     Get the maximum number of STG groups the device supports
 * Parameters:
 *     unit    - device unit number.
 *     max_stg - max number of STG groups supported by this unit
 * Returns:
 *     BCM_E_xxx
 */
int
bcm_caladan3_stg_count_get(int unit, int *max_stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    *max_stg = si->stg_max - si->stg_min + 1;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, *max_stg, bcm_errmsg(result)));

    return result;
}

int
bcm_caladan3_stg_detach(int unit)
{
    bcm_stg_info_t     *si;

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        /* stg_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX
         */
        return BCM_E_UNIT;
    }

    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit); 
    if (si->stg_bitmap) {
        sal_free(si->stg_bitmap);
        si->stg_bitmap = NULL;
    }

    if (si->stg_enable) {
        sal_free(si->stg_enable);
        si->stg_enable = NULL;
    } 

    if (si->stg_state_h) {
        sal_free(si->stg_state_h);
        si->stg_state_h = NULL;
    }

    if (si->stg_state_l) {
        sal_free(si->stg_state_l);
        si->stg_state_l = NULL;
    }

    if (si->vlan_first) {
        sal_free(si->vlan_first); 
        si->vlan_first = NULL;
    }

    if (si->vlan_next) {
        sal_free(si->vlan_next);
        si->vlan_next = NULL;
    }
    STG_DB_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_caladan3_stg_init
 * Description:
 *      Initialize the STG module according to Initial Configuration.
 * Parameters:
 *      unit - Device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_stg_init(int unit)
{
    bcm_stg_info_t     *si;
    int                 sz_bmp, sz_e, sz_sh, sz_sl, sz_vfirst, sz_vnext;
    int                 index;
    bcm_port_t          port;
    int                 result = BCM_E_NONE;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    uint32              *wb_bitop_ptr;

#endif

    LOG_VERBOSE(BSL_LS_BCM_STG,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        /* stg_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return BCM_E_UNIT;
    }

    si = &STG_CNTL(unit);

    if (!SOC_RECONFIG_TDM) {

        if (NULL == si->lock) {
            if (NULL == (si->lock = sal_mutex_create("caladan3_stg_lock"))) {
                return BCM_E_MEMORY;
            }
        }

        /* Set the device properties */
        si->stg_count = 0;                  /* no STGs currently defined */
        si->stg_min   = _stg_min;
        si->stg_max   = _stg_max;
        si->stg_defl  = _stg_default;       /* The default STG is always 1 */

        assert(si->stg_defl >= si->stg_min && si->stg_defl <= si->stg_max &&
               si->stg_min <= si->stg_max);

        /* alloc memory and/or clear */
        sz_bmp = SHR_BITALLOCSIZE(si->stg_max+1);
        if (NULL == si->stg_bitmap) {
            si->stg_bitmap = sal_alloc(sz_bmp, "STG-bitmap");
        }

        /* array of port bitmaps indicating whether the port+stg has STP enabled,
         * 0 = BCM_STG_STP_DISABLE, 1= Enabled.
         */
        sz_sh = sz_sl = sz_e = sizeof(bcm_pbmp_t) * (si->stg_max + 1);
        if (NULL == si->stg_enable) {
            si->stg_enable = sal_alloc(sz_e, "STG-enable");
        }

        /* array of port bitmaps indicating STP state for the port+stg combo.
         * Only valid if stg_enable = TRUE. Use high (_h) and low (_l) for
         * each port to represent one of four states in bcm_stg_stp_t, i.e
         * BLOCK(_h,_l = 0,0), LISTEN(0,1), LEARN(1,0), FORWARD(1,1).
         */
        if (NULL == si->stg_state_h) {
            si->stg_state_h = sal_alloc(sz_sh, "STG-state_h");
        }

        if (NULL == si->stg_state_l) {
            si->stg_state_l = sal_alloc(sz_sl, "STG-state_l");
        }

        sz_vfirst = (si->stg_max + 1) * sizeof (bcm_vlan_t);
        if (NULL == si->vlan_first) {
            si->vlan_first = sal_alloc(sz_vfirst, "STG-vfirst");
        }

        sz_vnext = BCM_VLAN_COUNT * sizeof (bcm_vlan_t);
        if (NULL == si->vlan_next) {
            si->vlan_next = sal_alloc(sz_vnext, "STG-vnext");
        }

        if (si->stg_bitmap == NULL || si->stg_enable == NULL || 
            si->stg_state_h == NULL || si->stg_state_l == NULL ||
            si->vlan_first == NULL || si->vlan_next == NULL) {

            result = bcm_caladan3_stg_detach(unit);
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_STG,
                          (BSL_META_U(unit,
                                      "Failed to clean up data\n")));
            }
            return BCM_E_MEMORY;
        }

        sal_memset(si->stg_bitmap, 0, sz_bmp);
        sal_memset(si->stg_enable, 0, sz_e);
        sal_memset(si->stg_state_h, 0, sz_sh);
        sal_memset(si->stg_state_l, 0, sz_sl);
        sal_memset(si->vlan_first, 0, sz_vfirst);
        sal_memset(si->vlan_next, 0, sz_vnext);

    }

#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_stg_state_init(unit);
#endif
    /*
     * Create default STG and add all VLANs to it.  Use private calls.
     * This creates a slight maintenance issue but allows delayed setting
     * of the init flag. This will prevent any public API functions
     * from executing.
     */
    if (!SOC_WARM_BOOT(unit)) {
        result = _bcm_caladan3_stg_stp_init(unit, si->stg_defl);
        BCM_IF_ERROR_RETURN(result);
    }

    if (!SOC_RECONFIG_TDM && !SOC_WARM_BOOT(unit)) {

        STG_BITMAP_SET(si, si->stg_defl);
        si->stg_count++;

        BCM_PBMP_ITER(PBMP_CMIC(unit), port) {
            for (index = _stg_vlan_min; index <= _stg_vlan_max; index++) {
                (void)_bcm_caladan3_stg_vid_stp_set(unit, (bcm_vlan_t)index,
                                                    port, BCM_STG_STP_DISABLE);
            }
        }

#ifdef BCM_CALADAN3_G3P1_SUPPORT
        if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G3P1) {
            result = _bcm_caladan3_g3p1_stg_init(unit);
            BCM_IF_ERROR_RETURN(result);
        }
#endif

    }

    si->init = TRUE;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
        wb_info_ptr = SBX_SCACHE_INFO_PTR(unit);
        if(wb_info_ptr == NULL) {
            LOG_ERROR(BSL_LS_BCM_STG,
                      (BSL_META_U(unit,
                                  "Warm boot not initialized for unit %d \n"),
                       unit));
            return BCM_E_INTERNAL;
        }
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(bcm_stg_t, 1,
            wb_info_ptr->stg_defl_offset, si->stg_defl);
        wb_bitop_ptr = (uint32 *)(wb_info_ptr->scache_ptr + wb_info_ptr->stg_bitmap_offset);
        SHR_BITSET(wb_bitop_ptr, si->stg_defl);
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->stg_count_offset, si->stg_count);
    } else {
        _bcm_caladan3_wb_stg_state_restore(unit);
    }
#endif

    LOG_DEBUG(BSL_LS_BCM_STG,
              (BSL_META_U(unit,
                          "bcm_stg_init: unit=%d rv=%d(%s)\n"),
               unit, BCM_E_NONE, bcm_errmsg(BCM_E_NONE)));

    return BCM_E_NONE;
}
