/*
 * $Id: port.c,v 1.27.24.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Caladan3 Port API
 */

#include <shared/bsl.h>


#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/error.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1_diags.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/trunk.h>
#include <bcm/qos.h>

#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/port.h>

#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/qos.h>

#include <bcm_int/sbx/caladan3/wb_db_port.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT


#define   PORT_PARAM_CHECK(_u, p, min, max )   \
if (((p) < (min))  || ((p) > (max))) {      \
  LOG_ERROR(BSL_LS_BCM_PORT, \
            (BSL_META_U(_u, "Invalid parameter: %s=%d min=%d max=%d\n"), \
             #p , (p), (min), (max)));                \
  return BCM_E_PARAM; \
}

#define FCOS_2_RCOS(fcos) (7 - (fcos))

/* Temporary */

#define SB_G3P1_CALADAN3_CTPID_INDEX    0
#define SB_G3P1_CALADAN3_STPID0_INDEX   1
#define SB_G3P1_CALADAN3_STPID1_INDEX   2

/* End Temporary */


/*
 * TPID
 *
 * G3P1 supports only 1 customer and 2 provider TPIDs for the entire
 * device (tpids are shared among all ports in a device), as follows:
 *     C-TPID
 *     S-TPID0
 *     S-TPID1
 *
 * DTAG modes
 *     BCM_PORT_DTAG_MODE_EXTERNAL (CUSTOMER)
 *         C-TPID is the tpid
 *
 *     BCM_PORT_DTAG_MODE_INTERNAL (PROVIDER)
 *         S-TPID (0 or 1) is the tpid
 *                         c-tpid is also outer tpid when twin is n use
 *         C-TPID          is the inner_tpid
 */

_g3p1_port_t  _port_state[BCM_LOCAL_UNITS_MAX];

int _bcm_caladan3_g3p1_port_qos_profile_dump(int unit, int profile);

static uint32 max_qos_profile_idx=0;

#ifdef BCM_WARM_BOOT_SUPPORT
extern bcm_caladan3_wb_port_state_scache_info_t
    *_bcm_caladan3_wb_port_state_scache_info_p[BCM_MAX_NUM_UNITS];
#endif

/*
 * Assumes a valid unit
 */
int
_bcm_caladan3_g3p1_port_ilib_common_init(int unit)
{
    int rv;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

    /* Init TPID reference usage count */
    TPID_COUNT(unit).ctpid  = 0;
    TPID_COUNT(unit).stpid0 = 0;
    TPID_COUNT(unit).stpid1 = 0;
    TPID_COUNT(unit).twin   = 0;

    rv = soc_sbx_g3p1_max_qos_profile_index_get (unit, &max_qos_profile_idx);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d rv=%d soc_sbx_g3p1_max_qos_profile_index_get\n"),
                   unit, rv));
        return rv;
    } 

    _port_state[unit].profile_count = sal_alloc(sizeof(int)*max_qos_profile_idx,
                                                "Profile Ref Count");
    if (_port_state[unit].profile_count == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: sal_alloc failed\n")));
        return BCM_E_MEMORY;
    } 

    sal_memset(_port_state[unit].profile_count, 0, sizeof(int)*max_qos_profile_idx);

#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
        wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
        if(wb_info_ptr == NULL) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Warm boot not initialized for unit %d \n"),
                       unit));
            BCM_EXIT;
        }
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->max_profile_count_offset, max_qos_profile_idx);
    }

exit:

#endif


    return rv;
}

int
_bcm_caladan3_g3p1_port_ilib_common_detach(int unit)
{
    if (_port_state[unit].profile_count != NULL) {
        sal_free(_port_state[unit].profile_count);
        _port_state[unit].profile_count = NULL;
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_port_ilib_entry_init(int unit,
                                      bcm_port_t port,
                                      bcm_vlan_data_t *vd)
{
    soc_sbx_g3p1_p2e_t    sbx_port;
    soc_sbx_g3p1_ep2e_t   egrport;
    soc_sbx_g3p1_lp_t     sbx_logical_port;
    int                   rv, temp_rv;
    int                   modid;

    rv = BCM_E_NONE;

    /* Set up Port to Etc table
     */
    soc_sbx_g3p1_p2e_t_init(&sbx_port);

    sbx_port.state_num = 0x0;
    temp_rv = soc_sbx_g3p1_htype_eth_get(unit, &sbx_port.hdrtype);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_g3p1_port_ilib_entry_init: unit=%d "
                              "rv=%d port=%d soc_sbx_g3p1_htype_eth_get\n"),
                  unit, rv, port));
        return rv;
    }

    sbx_port.customer = 1;
    sbx_port.nativevid  = 0 /* NOT vd->vlan_tag */;

    /* by default enable mpls processing on all ports. To filter
     * mpls packets per port, use port switch controls */
    sbx_port.mplstp = 1;

    temp_rv = soc_sbx_g3p1_p2e_set(unit, port, &sbx_port);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d rv=%d port=%d "
                               "soc_sbx_g3p1_p2e_set\n"),
                   unit, rv, port));
        return rv;
    }

 
    /* Set up Logical port table
     */
    soc_sbx_g3p1_lp_t_init(&sbx_logical_port);

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &modid));

    sbx_logical_port.pid = SOC_SBX_PORT_SID(unit, modid, port);

    temp_rv = soc_sbx_g3p1_lp_set(unit, port, &sbx_logical_port);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d rv=%d port=%d soc_sbx_g3p1_lp_set\n"),
                   unit, rv, port));
    }

   soc_sbx_g3p1_ep2e_t_init(&egrport);

    egrport.state_num = 0x80;
    temp_rv = soc_sbx_g3p1_htype_erh_get(unit, &egrport.hdrtype);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_g3p1_port_ilib_entry_init: unit=%d rv=%d port=%d "
                               "soc_sbx_g3p1_htype_erh_get\n"),
                   unit, rv, port));
        return rv;
    }

    egrport.pid = SOC_SBX_PORT_SID(unit, modid, port);
    temp_rv = soc_sbx_g3p1_ep2e_set(unit, port, &egrport);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d rv=%d port=%d "
                               "soc_sbx_g3p1_ep2e_set\n"),
                   unit, rv, port));
        return rv;
    }

    return rv;
}

int
_bcm_caladan3_g3p1_port_ilib_egr_init(int unit, bcm_port_t port)
{
    soc_sbx_g3p1_oi2e_t    sbx_ohi;
    soc_sbx_g3p1_ete_t     sbx_ete;
    int                    temp_rv, rv = BCM_E_NONE;
    uint32                 etei, ohi;

    /*
     * ETE L2 entries are allocated per port
     *
     * NOTE: ETE L2 resources have been allocated already
     * as part of the BCM common initialization, so just need to
     * setup the ETE L2 table.
     */
    etei = SOC_SBX_PORT_ETE(unit, port);
    ohi  = SOC_SBX_PORT_OHI(port);

    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);
    sbx_ohi.eteptr = etei;

    temp_rv = soc_sbx_g3p1_oi2e_set(unit, ohi - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d port=%d soc_sbx_g3p1_oi2e_set\n"),
                   unit, port));
    }

    soc_sbx_g3p1_ete_t_init(&sbx_ete);
    sbx_ete.mtu = SBX_DEFAULT_MTU_SIZE;
    sbx_ete.remark = PORT(unit, port).egr_remark_table_idx; 
    temp_rv = soc_sbx_g3p1_ete_set(unit, etei, &sbx_ete);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "write tag %d:ete[%08X]\n"),
                 unit, etei));
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d port=%d soc_sbx_g3p1_ete_set\n"),
                   unit, port));
    }

    /*
     * In g3p1, untagged is the same as tagged, stripping comes from
     * setting epv2e.strip = 1.
     */
    temp_rv = soc_sbx_g3p1_ete_set(unit,
                                   SOC_SBX_PORT_UT_ETE(unit, port),
                                   &sbx_ete);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "write untag %d:ete[%08X]\n"),
                 unit,
                 SOC_SBX_PORT_UT_ETE(unit, port)));
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d port=%d soc_sbx_g3p1_ete_set\n"),
                   unit, port));
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_ilib_tpid_init(int unit, bcm_port_t port)
{
    soc_sbx_g3p1_tpid_t  g3p1_tpid;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    if (SOC_WARM_BOOT(unit)) {
        soc_sbx_g3p1_p2e_t  p2e;
        int                 rv;

        rv = soc_sbx_g3p1_p2e_get(unit, port, &p2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Failed to recover: p2e[%d] "
                                  "read failed: %s\n"),
                       port, bcm_errmsg(rv)));
            return rv;
        }

        /* Customer ports implicitly reference both ctpid, and an stpid */
        TPID_COUNT(unit).ctpid++;
        if (p2e.twintpid) {
            TPID_COUNT(unit).twin++;
        } else if (p2e.tpid) {
            TPID_COUNT(unit).stpid1++;
        } else {
            TPID_COUNT(unit).stpid0++;
        }

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Port %d p2e.provider=%d twin=%d tpid=%d\n"),
                     port, p2e.provider, p2e.twintpid, p2e.tpid));
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "c/t/0/1=%d/%d/%d/%d\n"),
                     TPID_COUNT(unit).ctpid, TPID_COUNT(unit).twin,
                     TPID_COUNT(unit).stpid0, TPID_COUNT(unit).stpid1));

        return BCM_E_NONE;
    }

    /*
     * Set default TPID values
     *
     * ctpid=0x8100 stpid0=0x88a8 stpid1=0x9100
     * All ports are set to use ctpid and stpid0
     */

    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_tpid));
    if (g3p1_tpid.tpid != TPID_CTPID_DEFAULT) {
        g3p1_tpid.tpid = TPID_CTPID_DEFAULT;
        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_tpid_set(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_tpid));
    }

    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_STPID0_INDEX, &g3p1_tpid));
    if (g3p1_tpid.tpid != TPID_STPID0_DEFAULT) {
        g3p1_tpid.tpid = TPID_STPID0_DEFAULT;
        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_tpid_set(unit, SB_G3P1_CALADAN3_STPID0_INDEX, &g3p1_tpid));
    }

    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_STPID1_INDEX, &g3p1_tpid));
    if (g3p1_tpid.tpid != TPID_STPID1_DEFAULT) {
        g3p1_tpid.tpid = TPID_STPID1_DEFAULT;
        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_tpid_set(unit, SB_G3P1_CALADAN3_STPID1_INDEX, &g3p1_tpid));
    }

    /*
     * Update reference usage count
     * By default, all ports are set to use ctpid and stpid0
     */
    TPID_COUNT(unit).ctpid++;
    TPID_COUNT(unit).stpid0++;

#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->ctpid_offset, TPID_COUNT(unit).ctpid);
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->stpid0_offset, TPID_COUNT(unit).stpid0);
#endif

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_port_ilib_fte_init(int unit, int node, bcm_port_t port)
{
    int                 temp_rv, rv = BCM_E_NONE;
    soc_sbx_g3p1_ft_t   sbx_fte;
    int                 queue;
    int                 fte_idx;

    fte_idx = SOC_SBX_PORT_FTE(unit, node, port);
    queue   = SOC_SBX_NODE_PORT_TO_QID(unit,node, port, NUM_COS(unit));

    /*
     * Do not program OHI in the FTE; this will cause ucode
     * to use TB model where EgrVlanPort2Etc will be looked up
     * to fetch the ETE address
     */
    soc_sbx_g3p1_ft_t_init(&sbx_fte);
    sbx_fte.qid     = queue;

    temp_rv = soc_sbx_g3p1_ft_set(unit, fte_idx, &sbx_fte);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d node %d port%d fte_idx=%d rv=%d "
                               "soc_sbx_g3p1_ft_set\n"), unit, node, port, fte_idx, rv));
    }

    return rv;
}


int
_bcm_caladan3_egr_path_get(int unit, uint32 fti,
                      soc_sbx_g3p1_ft_t *ft,
                      soc_sbx_g3p1_oi2e_t *oh,
                      soc_sbx_g3p1_ete_t *ete,
                      uint32               *etei)
{
    int rv;

    /* Drill down to the ETE for this VLAN PORT */
    rv = soc_sbx_g3p1_ft_get(unit, fti, ft);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to read FT[0x%x]: %d (%s)\n"),
                   fti, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g3p1_oi2e_get(unit, ((ft->oi) - SBX_RAW_OHI_BASE), oh);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to read oi2e[0x%x] FT[0x%x]: %d (%s)\n"),
                   ft->oi, fti, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g3p1_ete_get(unit, oh->eteptr, ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to read ETE[0x%x] FT[0x%x] "
                               "oi2e[0x%x]: %d (%s)\n"),
                   oh->eteptr, fti, ft->oi, rv, bcm_errmsg(rv)));
        return rv;
    }

    *etei = oh->eteptr;

    return rv;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_lp_path_get
 *   Purpose
 *      Get the p3 forwarding information for the logical port
 *   Parameters
 *      (in)  unit    - BCM device number
 *      (in)  port    - GPORT_VLAN_PORT to access discard settings
 *      (out) lpIdx   - logical port index for given gport
 *      (out) ft      - forwarding table information
 *      (out) ftIdx   - forwarding table index
 *      (out) oh      - out header index information
 *      (out) ete     - egress table entry
 *      (out) eteIdx  - egress table entry index
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 */
int
_bcm_caladan3_g3p1_lp_path_get(int unit, bcm_port_t port,
                             soc_sbx_g3p1_ft_t *ft, uint32 *ftIdx,
                             soc_sbx_g3p1_oi2e_t *oh,
                             soc_sbx_g3p1_ete_t *ete, uint32 *eteIdx)
{
    int rv = BCM_E_INTERNAL;

    rv = _bcm_caladan3_vlan_fte_gport_get(unit, port, ftIdx);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    rv = _bcm_caladan3_egr_path_get(unit, *ftIdx, ft, oh, ete, eteIdx);

    return rv;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_port_lp_discard_action
 *   Purpose
 *      Get/Set the discard state of a logical port (vlan_port)
 *   Parameters
 *      (in)  unit    - BCM device number
 *      (in)  port    - GPORT_VLAN_PORT to access discard settings
 *      (in/out) discardTagged   - drop/discard tagged packet setting
 *      (in/out) discardUnagged  - drop/discard untagged packet setting
 *      (in)  set     - set/get action
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 */
int
_bcm_caladan3_g3p1_port_lp_discard_access(int unit,
                                        bcm_port_t port,
                                        int *discardTagged,
                                        int *discardUntagged,
                                        int set)
{
    int                   rv;
    uint32                ftIdx, eteIdx;
    soc_sbx_g3p1_ft_t     ft;
    soc_sbx_g3p1_oi2e_t   outHeader;
    soc_sbx_g3p1_ete_t    ete;

    sal_memset(&ete, 0, sizeof(soc_sbx_g3p1_ete_t));
    sal_memset(&ft, 0, sizeof(soc_sbx_g3p1_ft_t));

    rv = _bcm_caladan3_g3p1_lp_path_get(unit, port, &ft, &ftIdx,
                                      &outHeader, &ete, &eteIdx);

    if (set) {
        ete.dropuntagged = *discardUntagged;
        ete.droptagged   = *discardTagged;
        rv = soc_sbx_g3p1_ete_set(unit, eteIdx, &ete);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Failed to write ETE on "
                                   "vlan_port=0x%x ftIdx=0x%x ohIdx=0x%x eteIdx=0x%x "
                                   "rv=%d (%s)\n"),
                       FUNCTION_NAME(), unit, port, ftIdx, ft.oi,
                       outHeader.eteptr, rv, bcm_errmsg(rv)));
            return rv;
        }

    } else {
        *discardUntagged = ete.dropuntagged;
        *discardTagged   = ete.droptagged;
    }

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_g3p1_port_lp_untagged_vlan_access
 *   Purpose
 *      Get/Set the default vid of a logical port (vlan_port)
 *   Parameters
 *      (in)  unit    - BCM device number
 *      (in)  port    - GPORT_VLAN_PORT to access settings
 *      (in/out) vid  - default vid to set/get
 *      (in)  set     - set/get action
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 */
int
_bcm_caladan3_g3p1_port_lp_untagged_vlan_access(int unit,
                                              bcm_port_t port,
                                              bcm_vlan_t *vid,
                                              int set)
{
    int                   rv;
    uint32                ftIdx, eteIdx;
    soc_sbx_g3p1_ft_t     ft;
    soc_sbx_g3p1_oi2e_t   outHeader;
    soc_sbx_g3p1_ete_t    ete;

    sal_memset(&ete, 0, sizeof(soc_sbx_g3p1_ete_t));
    sal_memset(&ft, 0, sizeof(soc_sbx_g3p1_ft_t));

    rv = _bcm_caladan3_g3p1_lp_path_get(unit, port, &ft, &ftIdx,
                                        &outHeader, &ete, &eteIdx);

    if (set) {
        /* if vid is invalid, interpret as a 'clear' */
        if (*vid > BCM_VLAN_MAX) {
            ete.usevid = 0;
            ete.vid    = 0;
        } else {
            ete.usevid = 1;
            ete.vid = *vid;
        }

        rv = soc_sbx_g3p1_ete_set(unit, eteIdx, &ete);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Failed to write ETE on "
                                   "vlan_port=0x%x ftIdx=0x%x ohIdx=0x%x eteIdx=0x%x "
                                   "rv=%d (%s)\n"),
                       FUNCTION_NAME(), unit, port, ftIdx, ft.oi,
                       outHeader.eteptr, rv, bcm_errmsg(rv)));
            return rv;
        }

    } else {
        if (ete.usevid) {
            *vid = ete.vid;
        } else {
            /* ete is not set to use the vid */
            *vid = BCM_VLAN_MAX;
            rv = BCM_E_NOT_FOUND;
        }
    }

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_g3p1_port_lp_untagged_priority_access
 *   Purpose
 *      Get/Set the default priority of a logical port (vlan_port)
 *   Parameters
 *      (in)  unit    - BCM device number
 *      (in)  port    - GPORT_VLAN_PORT to access settings
 *      (in/out) priority - default priority to set/get
 *      (in)  set     - set/get action
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 */
int
_bcm_caladan3_g3p1_port_lp_untagged_priority_access(int unit,
                                                  bcm_port_t port,
                                                  int *priority,
                                                  int set)
{
    int                   rv;
    uint32                ftIdx, eteIdx;
    soc_sbx_g3p1_ft_t     ft;
    soc_sbx_g3p1_oi2e_t   outHeader;
    soc_sbx_g3p1_ete_t    ete;

    sal_memset(&ete, 0, sizeof(soc_sbx_g3p1_ete_t));
    sal_memset(&ft, 0, sizeof(soc_sbx_g3p1_ft_t));

    rv = _bcm_caladan3_g3p1_lp_path_get(unit, port, &ft, &ftIdx,
                                      &outHeader, &ete, &eteIdx);

    if (set) {
        ete.defpricfi = (*priority) << 1;

        rv = soc_sbx_g3p1_ete_set(unit, eteIdx, &ete);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Failed to write ETE on "
                                   "vlan_port=0x%x ftIdx=0x%x ohIdx=0x%x eteIdx=0x%x "
                                   "rv=%d (%s)\n"),
                       FUNCTION_NAME(), unit, port, ftIdx, ft.oi,
                       outHeader.eteptr, rv, bcm_errmsg(rv)));
            return rv;
        }

    } else {
        *priority = (ete.defpricfi >> 1);
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_lp_set(int unit, bcm_module_t modid,
                                  bcm_port_t port, bcm_vlan_t vlan)
{
    soc_sbx_g3p1_pv2e_t   pv2e;
    soc_sbx_g3p1_lp_t     lp;
    int                   rv = BCM_E_NONE;
    int                   fab_unit, fab_node, fab_port;

    BCM_IF_ERROR_RETURN
       (soc_sbx_node_port_get(unit, modid, port, &fab_unit,
                              &fab_node, &fab_port));

    soc_sbx_g3p1_pv2e_t_init(&pv2e);
    rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vlan, &pv2e);
    if (rv == BCM_E_NONE && pv2e.lpi) {
        soc_sbx_g3p1_lp_t_init(&lp);

        rv = soc_sbx_g3p1_lp_get(unit, pv2e.lpi, &lp);
        if (rv == BCM_E_NONE) {
            lp.pid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
            rv = soc_sbx_g3p1_lp_set(unit, pv2e.lpi, &lp);
        }
    }

    return rv;
}

/*
 * Note that this is called for local (FE) ports only
 */
int
_bcm_caladan3_g3p1_port_ilib_lp_init(int unit, bcm_module_t modid,
                                   bcm_port_t port)
{
    int                 rv = BCM_E_NONE;
    uint32            lp_index;
    soc_sbx_g3p1_lp_t   sbx_logical_port;
    int                 fab_unit, fab_node, fab_port;
    soc_sbx_g3p1_ep2e_t egrport;
    bcm_vlan_data_t     *vlan_list;
    int                 vlan_count;
    bcm_trunk_t         tid = BCM_TRUNK_INVALID;

    /*
     * Cannot change PID for trunk ports
     */
    rv = bcm_trunk_find(unit, modid, port, &tid);
    if (rv == BCM_E_NONE && tid != BCM_TRUNK_INVALID) {
        return rv;
    }

    rv = BCM_E_NONE;

    /* reserve a particular index such that future references may use the
     * logical port macro for easy lookups
     */
    lp_index = port;

    BCM_IF_ERROR_RETURN
       (soc_sbx_node_port_get(unit, modid, port, &fab_unit,
                              &fab_node, &fab_port));

    soc_sbx_g3p1_lp_t_init(&sbx_logical_port);
    rv = soc_sbx_g3p1_lp_get(unit, lp_index, &sbx_logical_port);
    if (BCM_E_NONE == rv) {
    sbx_logical_port.pid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
    rv = soc_sbx_g3p1_lp_set(unit, lp_index, &sbx_logical_port);
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "bcm_port_init: unit=%d rv=%d  port=%d soc_sbx_g3p1_lp_set\n"),
                   unit, rv, port));
    }

    soc_sbx_g3p1_ep2e_t_init(&egrport);
    rv = soc_sbx_g3p1_ep2e_get(unit, port, &egrport);
    if (rv == BCM_E_NONE) {
        egrport.pid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
        rv = soc_sbx_g3p1_ep2e_set(unit, port, &egrport);
    }

    if (rv == BCM_E_NONE) {
        rv = bcm_vlan_list(unit, &vlan_list, &vlan_count);

        if (rv == BCM_E_NONE) {
            int idx;

            for (idx=0; idx < vlan_count; idx++) {
                 if (BCM_PBMP_MEMBER(vlan_list[idx].port_bitmap,
                                     port)) {
                      rv = _bcm_caladan3_g3p1_port_vlan_lp_set(unit,
                                                        modid,
                                                        port,
                                                        vlan_list[idx].vlan_tag);
                      if (rv != BCM_E_NONE) {
                          break;
                      }
                 }
            }
        }
        bcm_vlan_list_destroy(unit, vlan_list, vlan_count);
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_ilib_well_known_egr_init(int unit)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g3p1_ete_t      sbx_ete;
    soc_sbx_g3p1_oi2e_t     sbx_oi2e;
    uint32                  ete_l2;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    /*
     * Only allocate resource if this has not been allocated.
     *
     * NOTE:
     * Allocated resource IDs may be already referenced
     * by other modules, so new ones should not be re-assigned
     * unless the PORT re-initialization is coordinated with
     * the other dependent modules, or ALL modules are initialized too.
     */
    ete_l2 = _sbx_port_handler[unit]->ete_l2;
    if ((!_sbx_port_handler[unit]->ete_l2_valid) ||
        (_sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_ETE,
                                _sbx_port_handler[unit]->ete_l2)
         != BCM_E_EXISTS)) {
        rv = _sbx_caladan3_resource_alloc(unit,
                                     SBX_CALADAN3_USR_RES_ETE,
                                     1,
                                     &ete_l2,
                                     0);
        if (BCM_FAILURE(rv)) {
            _sbx_port_handler[unit]->ete_l2_valid = FALSE;
            _sbx_port_handler[unit]->ete_l2 = 0;
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "bcm_port_init: unit=%d Failed to alloc ete\n"),
                       unit));
            return rv;
        }

        _sbx_port_handler[unit]->ete_l2_valid = TRUE;
        _sbx_port_handler[unit]->ete_l2 = ete_l2;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->ete_l2_offset, _sbx_port_handler[unit]->ete_l2);
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->ete_l2_valid_offset, _sbx_port_handler[unit]->ete_l2_valid);

#endif
    }

#ifdef BROADCOM_DEBUG
    if (SOC_WARM_BOOT(unit)) {
        rv = soc_sbx_g3p1_oi2e_get(unit, SBX_RAW_OHI_BASE - SBX_RAW_OHI_BASE,
                                   &sbx_oi2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Failed to get raw OHI: %s\n"),
                       bcm_errmsg(rv)));
            return rv;
        }

        if (sbx_oi2e.eteptr != ete_l2) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Warmboot Failed.  "
                                   "ete mismatch: 0x%x/0x%x\n"),
                       sbx_oi2e.eteptr, ete_l2));
            return BCM_E_INTERNAL;
        }
    }
#endif

    soc_sbx_g3p1_ete_t_init(&sbx_ete);

    sbx_ete.stpcheck = 0;
    sbx_ete.mtu = SBX_DEFAULT_MTU_SIZE;
    sbx_ete.nostrip = 0;
    sbx_ete.noclass = 1;
    sbx_ete.usevid = 1;
    sbx_ete.vid = 0xfff;
    sbx_ete.nosplitcheck = 1;

    rv = soc_sbx_g3p1_ete_set(unit, ete_l2, &sbx_ete);

    if (BCM_SUCCESS(rv)) {
        soc_sbx_g3p1_oi2e_t_init(&sbx_oi2e);
        sbx_oi2e.eteptr = ete_l2;
        rv = soc_sbx_g3p1_oi2e_set(unit, SBX_RAW_OHI_BASE - SBX_RAW_OHI_BASE, &sbx_oi2e);
    }

    return rv;
}


/* Use the qos api to manage the resource allocation, but the port api will 
 * still set the mappings.  Just a stop-gap until port qos api is completely
 * deprecated
 */
int
_bcm_caladan3_port_qos_map_create(int unit, int flags, bcm_port_t port)
{
    int                    rv, map_id, hw_id;
    uint32                 etei;
    soc_sbx_g3p1_ete_t     ete;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif


    /* grab the stored one for WarmBoot recovery; caller may pass
     * _WITH_ID to reserve the profile
     */
    map_id = PORT(unit, port).egr_remark_table_idx;
    map_id = _bcm_caladan3_qos_hw_id_to_map_id(flags, map_id);

    rv = bcm_qos_map_create(unit, flags, &map_id);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Port %d failed to create qos map: %s\n"),
                   port, bcm_errmsg(rv)));
        return rv;
    }
    
    /* save it, update porte etes */
    hw_id = _bcm_caladan3_qos_map_id_to_hw_id(flags, map_id);

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Port %d changing remark idx from 0x%x to 0x%x\n"),
                 port, PORT(unit, port).egr_remark_table_idx, hw_id));

    PORT(unit, port).egr_remark_table_idx = hw_id;

#ifdef BCM_WARM_BOOT_SUPPORT
    SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
        wb_info_ptr->egr_remark_table_idx_offset + (port * sizeof(uint32)), hw_id);
#endif

    etei = SOC_SBX_PORT_ETE(unit, port);

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Updating port %d ete 0x%x remark\n"),
                 port, etei));

    rv = soc_sbx_g3p1_ete_get(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    ete.remark = hw_id;

    rv = soc_sbx_g3p1_ete_set(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to write ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_qos_init(int unit)
{
    bcm_port_t            port;
    bcm_pbmp_t            pbmp;
#if 0
    int                   rv, etei;
    soc_sbx_g3p1_ete_t    ete;
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }

    SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
        wb_info_ptr->port_count_offset, SBX_MAX_PORTS);
#endif

    /* The port_vlan_pri*_map*_set/get interface is in the process of being
     * deprecated in favor of the QoS api.  However, both must be supported
     * for the time being.  The the port and qos module will both use qos
     * and remark index 0 as the default, however, the qos module owns and
     * initializes it.
     *
     * To further reduce resource usage, the port module will use the
     * default index, and allocate on demand.  
     */

    BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(pbmp, PBMP_CMIC(unit));
    if (SOC_RECONFIG_TDM) {
        BCM_PBMP_REMOVE(pbmp,SOC_CONTROL(unit)->all_skip_pbm);
    }
    BCM_PBMP_ITER(pbmp, port) {
        if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
            continue;
        }
    
        if (SOC_WARM_BOOT(unit)) {
#if 0
            /* Temporarily the remark index will be stored in the
             * scache memory and retrieved from there instead of
             * being retored from the ete table.
             */
            etei = SOC_SBX_PORT_ETE(unit, port);
            rv = soc_sbx_g3p1_ete_get(unit, etei, &ete);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "Failed to get ete[0x%x]: %s\n"),
                           etei, bcm_errmsg(rv)));
                return rv;
            }

            PORT(unit, port).egr_remark_table_idx = ete.remark;
#endif

            
#if 0
            if (ete.remark != SBX_QOS_DEFAULT_REMARK_IDX) {
                int flags = (BCM_QOS_MAP_EGRESS | 
                             BCM_QOS_MAP_L2 | 
                             BCM_QOS_MAP_WITH_ID);
         
                /* This depends on the qos module */
                rv = _bcm_caladan3_port_qos_map_create(unit, flags, port);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "Failed to create qos map: %s\n"),
                               bcm_errmsg (rv)));
                    return rv;
                }
            }
#endif
            
        } else {
            /* remark table indexes are valid after one is allocated, 
             * the default is not valid to be freed or modfied.
             */
            PORT(unit, port).egr_remark_table_idx = SBX_QOS_DEFAULT_REMARK_IDX;
#ifdef BCM_WARM_BOOT_SUPPORT
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                wb_info_ptr->egr_remark_table_idx_offset + (port * sizeof(uint32)),
                SBX_QOS_DEFAULT_REMARK_IDX);
#endif

        }
        
    }


    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_port_qos_detach(int unit)
{
    bcm_port_t            port;
    bcm_pbmp_t            pbmp;
    int                   map_id, rv;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif
    
    BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(pbmp, PBMP_CMIC(unit));
    BCM_PBMP_ITER(pbmp, port) {
        if (!SOC_PORT_VALID(unit, port)) {
            continue;
        }

        if (PORT(unit, port).egr_remark_table_idx != 
            SBX_QOS_DEFAULT_REMARK_IDX) 
        {
            map_id = PORT(unit, port).egr_remark_table_idx;
            /* supposedly deprecated in g3p1 */
            map_id = _bcm_caladan3_qos_hw_id_to_map_id(BCM_QOS_MAP_EGRESS, 
                                                     map_id);

            rv = bcm_qos_map_destroy(unit, map_id);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "Failed to destroy qos map 0x%x (0x%x)"
                                      ": %s\n"),
                           map_id, PORT(unit, port).egr_remark_table_idx,
                           bcm_errmsg(rv)));
                /* keep trying, don't return */
            }

            PORT(unit, port).egr_remark_table_idx = SBX_QOS_DEFAULT_REMARK_IDX;
#ifdef BCM_WARM_BOOT_SUPPORT
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                wb_info_ptr->egr_remark_table_idx_offset + (port * sizeof(uint32)),
                SBX_QOS_DEFAULT_REMARK_IDX);
#endif
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_g3p1_port_ctpid_set
 * Description:
 *     Set the Customer Tag Protocol ID for a port.
 *     G3P1 has only 1 CTPID to be shared for all ports.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held, valid parameter values.
 */
STATIC int
_bcm_caladan3_g3p1_port_ctpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    soc_sbx_g3p1_tpid_t  g3p1_tpid;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    /* Check current TPID value */
    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_tpid));

    if (g3p1_tpid.tpid == tpid) {  /* Nothing to do */
        return BCM_E_NONE;
    }

    /*
     * Set CTPID
     *
     * Assume this port is already referencing this TPID
     * All BCM ports always reference to one customer and one of
     * the provider TPIDs (the active STPID).
     *
     * NOTE
     * The current G3P1 does not allow the CTPID to be changed.
     * But, this could change and BCM driver should not
     * make that assumption.  Instead, any 'error' or limitation from the
     * G3P1 routine should come from that layer.
     *
     * The only thing that the BCM driver should check is the usage
     * reference count to guard against changing TPID value when other
     * ports are using it (which is how the BCM driver defines it).
     */
    if (TPID_COUNT(unit).ctpid > 1) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: unit=%d CTPID in use by %d ports, "
                              "cannot change.\n"),
                   FUNCTION_NAME(), unit, TPID_COUNT(unit).ctpid));
            return BCM_E_RESOURCE;
    }

    if (TPID_COUNT(unit).twin > 1) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: unit=%d CTPID used as STPID by %d ports, cannot change.\n"),
                   FUNCTION_NAME(), unit, TPID_COUNT(unit).twin));
        return BCM_E_RESOURCE;
    }

    g3p1_tpid.tpid = tpid;
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_set(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_tpid));

    if (TPID_COUNT(unit).ctpid == 0) {
        TPID_COUNT(unit).ctpid++;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->ctpid_offset, TPID_COUNT(unit).ctpid);
#endif
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_stpid_set
 * Description:
 *     Set the Provider Tag Protocol ID for a port.
 *     G3P1 has 2 STPID to be shared for all ports, and only 1 can be
 *     active per port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 *     p2e  - (IN/OUT) G3P1 p2e configuration
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held, valid parameter values.
 */
STATIC int
_bcm_caladan3_g3p1_port_stpid_set(int unit, bcm_port_t port, uint16 tpid,
                                soc_sbx_g3p1_p2e_t *p2e,
                                soc_sbx_g3p1_ep2e_t *ep2e, int *out_tpid_idx)
{
    soc_sbx_g3p1_tpid_t  g3p1_tpid, g3p1_tpid2, g3p1_ctpid;
    int                  tpid_index, tpid_index2;
    int                  *p_count;
    int                  *p_count2;
    int                  *p_count_twin;
    int                  select_tpid2;
    int                  select_twin;
    int                  rv;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
    unsigned int    count_offset;
    unsigned int    count2_offset;
    unsigned int    twin_offset;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    /* Get active STPID */
    if (p2e->tpid) {  /* S-TPID1 */
        tpid_index  = SB_G3P1_CALADAN3_STPID1_INDEX;
        tpid_index2 = SB_G3P1_CALADAN3_STPID0_INDEX;
        p_count     =  &(TPID_COUNT(unit).stpid1);
        p_count2    =  &(TPID_COUNT(unit).stpid0);
#ifdef BCM_WARM_BOOT_SUPPORT
        count_offset = wb_info_ptr->stpid1_offset;
        count2_offset = wb_info_ptr->stpid0_offset;
#endif

    } else  {  /* S-TPID0 */
        tpid_index  = SB_G3P1_CALADAN3_STPID0_INDEX;
        tpid_index2 = SB_G3P1_CALADAN3_STPID1_INDEX;
        p_count     =  &(TPID_COUNT(unit).stpid0);
        p_count2    =  &(TPID_COUNT(unit).stpid1);
#ifdef BCM_WARM_BOOT_SUPPORT
        count_offset = wb_info_ptr->stpid0_offset;
        count2_offset = wb_info_ptr->stpid1_offset;
#endif
    }

    p_count_twin = &(TPID_COUNT(unit).twin);
#ifdef BCM_WARM_BOOT_SUPPORT
    twin_offset = wb_info_ptr->twin_offset;
#endif

    rv = soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_ctpid);
    if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Error=%d (%s) failed to get CTPID.\n"),
                       FUNCTION_NAME(), unit, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Check current TPID value */
    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);

    rv = soc_sbx_g3p1_tpid_get(unit, tpid_index, &g3p1_tpid);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: unit=%d Error=%d (%s) failed to get STPID %d.\n"),
                   FUNCTION_NAME(), unit, rv, bcm_errmsg(rv), tpid_index));
        return rv;
    }

    if ( ((p2e->twintpid == 1) && g3p1_ctpid.tpid == tpid) ||
         ((p2e->twintpid == 0) && g3p1_tpid.tpid  == tpid) )
    {
        /* Nothing to do */
        return BCM_E_NONE;
    }

    /*
     * Provider TPID
     *
     * Perform one of these in following order:
     * (a) IF CTPID matches given TPID, set select twintpid
     * (b) IF TPID matches given TPID, reset to tpid1 due to transition
     *     from twintpid
     * (c) IF secondary (non-active) STPID matches given TPID or
     *     secondary STPID count is 0, select/set secondary STPID.
     * (d) ELSE, if active STPID count is <=1 , reset STPID
     * (e) ELSE, return error
     *
     * NOTE: Assume this port is already referencing this TPID
     * All BCM ports always reference to one customer and one of
     * the provider TPIDs (the active STPID).
     */

    /* Get secondary Provider TPID */
    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid2);
    rv = soc_sbx_g3p1_tpid_get(unit, tpid_index2, &g3p1_tpid2);
    if (BCM_FAILURE(rv)){
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: unit=%d Error=%d (%s) failed to get STPID %d.\n"),
                   FUNCTION_NAME(), unit, rv, bcm_errmsg(rv), tpid_index2));
        return rv;
    }

    select_twin = FALSE;
    if (g3p1_ctpid.tpid == tpid) {
        select_tpid2 = FALSE;
        select_twin  = TRUE;
    } else if (g3p1_tpid.tpid == tpid) {
        select_tpid2 = FALSE;
    } else if ((g3p1_tpid2.tpid == tpid) || (*p_count2 == 0))  {
        select_tpid2 = TRUE;
    } else if (*p_count <= 1) {
        select_tpid2 = FALSE;
    } else {
        return BCM_E_RESOURCE;
    }

    /* Set new Provider TPID */
    if (select_twin) {
        /* select twin TPID - CTPID == STPID */
        p2e->twintpid = 1;
        ep2e->stpid1  = 0;
        ep2e->stpid0  = 0;

        rv = soc_sbx_g3p1_p2e_set(unit, port, p2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Error=%d (%s) failed to set p2e.\n"),
                       FUNCTION_NAME(), unit, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = soc_sbx_g3p1_ep2e_set(unit, port, ep2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: unit=%d Error=%d (%s) failed to set ep2e.\n"),
                       FUNCTION_NAME(), unit, rv, bcm_errmsg(rv)));
            return rv;
        }

        (*p_count)--;
        (*p_count_twin)++;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            count_offset, *p_count);
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            twin_offset, *p_count_twin);
#endif

    } else {
        int updateTwinCount = FALSE;

        /* If this port was using the twintpid feature and now moved to its
         * own, must also update reference count
         */
        if (p2e->twintpid) {
            updateTwinCount = TRUE;
            p2e->twintpid = 0;
        }

        if (select_tpid2) {
            /*
             * Set and select secondary STPID
             */

            /* Set new TPID value */
            if (g3p1_tpid2.tpid != tpid) {
                g3p1_tpid2.tpid = tpid;
                BCM_IF_ERROR_RETURN
                    (soc_sbx_g3p1_tpid_set(unit, tpid_index2, &g3p1_tpid2));
            }

            /* Select secondary Provider TPID */
            if (tpid_index2 == SB_G3P1_CALADAN3_STPID1_INDEX) {
                p2e->tpid = 1;
                ep2e->stpid1 = 1;
                ep2e->stpid0 = 0;
            } else {
                p2e->tpid = 0;
                ep2e->stpid1 = 0;
                ep2e->stpid0 = 1;
            }

            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_set(unit, port, p2e));
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ep2e_set(unit, port, ep2e));

            /* Update reference usage count */
            (*p_count2)++;
            if (updateTwinCount == FALSE) {
                (*p_count)--;
            }
#ifdef BCM_WARM_BOOT_SUPPORT
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                count2_offset, *p_count2);
            SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                count_offset, *p_count);
#endif

            if (out_tpid_idx != NULL) {
                *out_tpid_idx = tpid_index2;
            }
        } else {
            /*
             *  Reset active STPID
             */
            g3p1_tpid.tpid = tpid;
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_tpid_set(unit, tpid_index, &g3p1_tpid));

            /* Update reference usage count */
            if (*p_count == 0 || updateTwinCount) {
                (*p_count)++;
#ifdef BCM_WARM_BOOT_SUPPORT
                SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                    count_offset, *p_count);
#endif
            }

            /* clear twintpid, if necessary */
            if (updateTwinCount) {
                if (tpid_index == SB_G3P1_CALADAN3_STPID0_INDEX) {
                    ep2e->stpid0 = 1;
                    ep2e->stpid1 = 0;
                } else {
                    ep2e->stpid0 = 0;
                    ep2e->stpid1 = 1;
                }
                BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_set(unit, port, p2e));
                BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ep2e_set(unit, port, ep2e));
            }

            if (out_tpid_idx != NULL) {
                *out_tpid_idx = tpid_index;
            }
        }

        /* update twin count only when moving from twin tpid to stpid */
        if (updateTwinCount) {
            (*p_count_twin)--;
#ifdef BCM_WARM_BOOT_SUPPORT
                SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                    twin_offset, *p_count_twin);
#endif
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "%s %d: tpidCounts c/1/2/t=%d/%d/%d/%d\n"),
                 FUNCTION_NAME(), unit, TPID_COUNT(unit).ctpid,
                 TPID_COUNT(unit).stpid0, TPID_COUNT(unit).stpid1,
                 TPID_COUNT(unit).twin));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_tpid_set
 * Description:
 *     Set the default Tag Protocol ID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held, valid parameter values.
 *
 *     This API is not specifically double-tagging-related, but
 *     the port TPID becomes the service provider TPID when double-tagging
 *     is enabled on a port.
 *
 *     TPID set only allowed when in following port modes:
 *        BCM_PORT_DTAG_MODE_EXTERNAL -> Customer
 *        BCM_PORT_DTAG_MODE_INTERNAL -> Service Provider
 */
int
_bcm_caladan3_g3p1_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    int                  rv;
    soc_sbx_g3p1_p2e_t   p2e;
    soc_sbx_g3p1_ep2e_t  ep2e;
    int                  tpid_idx;

    /* Check current dtag mode */
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_get(unit, port, &p2e));
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ep2e_get(unit, port, &ep2e));
    if (!p2e.customer && !p2e.provider) {
        return BCM_E_CONFIG;
    }

    if (p2e.customer) {
        rv = _bcm_caladan3_g3p1_port_ctpid_set(unit, port, tpid);
    } else {
        rv = _bcm_caladan3_g3p1_port_stpid_set(unit, port, tpid, &p2e,
                                             &ep2e, &tpid_idx);
    }

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_tpid_get
 * Description:
 *     Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - (OUT) Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_caladan3_g3p1_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    soc_sbx_g3p1_p2e_t  p2e;
    int                 tpid_index;

    soc_sbx_g3p1_p2e_t_init(&p2e);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_get(unit, port, &p2e));

    /* Get TPID based on dtag mode */
    if (p2e.customer || p2e.twintpid) {  /* Customer C-TPID */
        tpid_index = SB_G3P1_CALADAN3_CTPID_INDEX;

    } else if (p2e.provider) {  /* Provider S-TPID 0 or 1 */
        if (p2e.tpid) {
            tpid_index = SB_G3P1_CALADAN3_STPID1_INDEX;
        } else {
            tpid_index = SB_G3P1_CALADAN3_STPID0_INDEX;
        }

    } else {  /* Use default */
        tpid_index = -1;
    }

    if (tpid_index >= 0) {
        soc_sbx_g3p1_tpid_t  g3p1_tpid;

        soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_tpid_get(unit, tpid_index, &g3p1_tpid));
        *tpid = g3p1_tpid.tpid;

    } else {  /* Default */
        *tpid = TPID_DEFAULT;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_inner_tpid_set
 * Purpose:
 *     Set the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 *
 * Notes:
 *     Assumes lock is held, valid parameter values.
 *
 *     This is only valid when port is in PROVIDER mode (dtag_mode = INTERNAL).
 *     Inner tag must be set after outer tag is set with 'bcm_port_tpid_set()'
 */
int
_bcm_caladan3_g3p1_port_inner_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    soc_sbx_g3p1_p2e_t   p2e;

    soc_sbx_g3p1_p2e_t_init(&p2e);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_get(unit, port, &p2e));

    /* Check Provider mode */
    if (!p2e.provider) {
        return BCM_E_CONFIG;
    }

    return _bcm_caladan3_g3p1_port_ctpid_set(unit, port, tpid);
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_inner_tpid_get
 * Purpose:
 *     Get the TPID for the inner tag in double-tagging mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - (OUT) Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held, valid parameter values.
 */
int
_bcm_caladan3_g3p1_port_inner_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    soc_sbx_g3p1_p2e_t   p2e;
    soc_sbx_g3p1_tpid_t  g3p1_tpid;

    soc_sbx_g3p1_p2e_t_init(&p2e);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_get(unit, port, &p2e));

    /* Check Provider mode */
    if (!p2e.provider) {
        return BCM_E_CONFIG;
    }

    /* Get current TPID value */
    soc_sbx_g3p1_tpid_t_init(&g3p1_tpid);
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_tpid_get(unit, SB_G3P1_CALADAN3_CTPID_INDEX, &g3p1_tpid));

    *tpid = g3p1_tpid.tpid;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_tpid_add
 * Purpose:
 *     Add allowed TPID for a port.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     tpid         - Tag Protocol ID
 *     color_select - Color mode for TPID, ignored for this device
 * Returns:
 *     BCM_E_UNAVAIL
 * Notes:
 * In G3P1, each given port can only have 1 'active' Provider STPID, 0 or 1.
 * (set in the soc_sbx_g3p1_p2e_t configuration).
 *
 * G3P1 has 2 provider STPIDs. The second STPID is used as an extra
 * TPID available for ports to use (set in bcm_port_tpid_set,
 * when in provider mode).
 */
int
_bcm_caladan3_g3p1_port_tpid_add(int unit, bcm_port_t port,
                               uint16 tpid, int color_select)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_tpid_delete
 * Purpose:
 *     Delete allowed TPID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_UNAVAIL
 * Notes:
 *     API 'port_tpid_add()' is not available in G3P1, similarly,
 *     'port_tpid_delete()' returns 'unavail'.
 *     If user wants to set a TPID to zero, the 'port_tpid_set()'
 *     API can be used.
 */
int
_bcm_caladan3_g3p1_port_tpid_delete(int unit, bcm_port_t port, uint16 tpid)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_tpid_delete_all
 * Purpose:
 *     Delete all allowed TPIDs for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     BCM_E_UNAVAIL
 * Notes:
 *     API 'port_tpid_add()' is not available in G3P1, similarly,
 *     'port_tpid_delete_all()' returns 'unavail'.
 *     If user wants to set a TPID to zero, the 'port_tpid_set()'
 *     API can be used.
 */
int
_bcm_caladan3_g3p1_port_tpid_delete_all(int unit, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
_bcm_caladan3_g3p1_port_dtag_mode_set(int unit, bcm_port_t port, int mode)
{
    soc_sbx_g3p1_p2e_t    p2e;
    soc_sbx_g3p1_ep2e_t   ep2e;
    int                   rv = BCM_E_NONE;

    rv = soc_sbx_g3p1_p2e_get(unit, port, &p2e);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    rv = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* only one, or none should be set */
    p2e.customer = 0;
    p2e.provider = 0;
    p2e.pbb      = 0;
    p2e.mim      = 0;
    ep2e.pbb     = 0;
    ep2e.mim     = 0;

    switch (mode) {
    case BCM_PORT_DTAG_MODE_TRANSPARENT:
        /* "port-mode" ports, all packets are treated as untagged and
         *  assume the defined port_untagged_vlan
         */
        p2e.customer  = 0;
        p2e.provider  = 0;
        ep2e.customer = 0;
        ep2e.cep      = 0;
        p2e.defstrip  = 1;
        p2e.pstrip    = 0;
        ep2e.stpid0   = 0;
        ep2e.stpid1   = 0;
        break;

    case BCM_PORT_DTAG_MODE_NONE:
        /* 802.1d ports */
        p2e.customer  = 1;
        p2e.provider  = 0;
        ep2e.customer = 0;
        ep2e.cep      = 0;
        p2e.defstrip  = 1;
        p2e.pstrip    = 0;
        ep2e.stpid0   = 0;
        ep2e.stpid1   = 0;
        break;

    case BCM_PORT_DTAG_MODE_INTERNAL:
        /* 802.1ad Provider Ports */
        p2e.customer  = 0;
        p2e.provider  = 1;
        p2e.defstrip  = 0;
        p2e.pstrip    = 1;
        ep2e.customer = 0;
        ep2e.cep      = 0;
        /* only update stpid 0 & 1 if twintpid is disabled */
        ep2e.stpid0   = !p2e.tpid && !p2e.twintpid;
        ep2e.stpid1   = p2e.tpid && !p2e.twintpid;
        break;

    case BCM_PORT_DTAG_MODE_EXTERNAL:
        /* 802.1ad Customer Ports */
        p2e.customer  = 1;
        p2e.provider  = 0;
        ep2e.customer = 1;
        ep2e.cep      = 1;
        p2e.defstrip  = 1;
        p2e.pstrip    = 0;
        ep2e.stpid0   = 0;
        ep2e.stpid1   = 0;
        break;

    default:  /* Shouldn't get here... */
        rv = BCM_E_CONFIG;
        return rv;
    }

    rv = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);
    }

    return rv;
}

int
_bcm_caladan3_g3p1_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    soc_sbx_g3p1_p2e_t    p2e;
    soc_sbx_g3p1_ep2e_t   ep2e;
    int                   rv = BCM_E_NONE;

    soc_sbx_g3p1_p2e_t_init(&p2e);
    rv = soc_sbx_g3p1_p2e_get(unit, port, &p2e);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    soc_sbx_g3p1_ep2e_t_init(&ep2e);
    rv = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    if (p2e.provider) {
        *mode = BCM_PORT_DTAG_MODE_INTERNAL;
    } else if (p2e.customer == 1 ) {
        if (ep2e.customer == 1) {
            *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        } else {
            *mode = BCM_PORT_DTAG_MODE_NONE;
        }
    } else { /* provider == 0 && p2e.customer == 0 */
        *mode = BCM_PORT_DTAG_MODE_TRANSPARENT;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_strip_tag(int unit, bcm_port_t port, int strip)
{
    soc_sbx_g3p1_p2e_t portData;
    int                rv;

    rv = soc_sbx_g3p1_p2e_get(unit, port, &portData);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: unit=%d port=%d rv=%d Failed to read port data. \n"),
                   FUNCTION_NAME(), unit, port, rv));
    }
    portData.defstrip = !!strip;
    rv = soc_sbx_g3p1_p2e_set(unit, port, &portData);

    return rv;
}



int
_bcm_caladan3_g3p1_port_untagged_priority_get(int unit,
                                            bcm_port_t port,
                                            int *priority)
{
    soc_sbx_g3p1_p2e_t sbx_port;
    int                rv;

    rv = soc_sbx_g3p1_p2e_get(unit, port, &sbx_port);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    *priority = sbx_port.defpri;

    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_port_qos_map_get(int unit,
                                  bcm_port_t port,
                                  bcm_vlan_t vid,
                                  int isPortVid,
                                  int de, int pri,
                                  soc_sbx_g3p1_qos_t *qos)
{
    int rv;
    soc_sbx_g3p1_lp_t   lp;

    /* get the logical port */
    if (isPortVid) {
        rv = bcm_caladan3_lp_get_by_pv(unit, port, vid, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to find or access lp for "
                                   "pv2e[%d,%03X]: %d (%s)\n"), 
                       port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to read lp[%08X]: %d (%s)\n"),
                       port, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* get the QoS data */
    rv = soc_sbx_g3p1_qos_get(unit, de, pri, lp.qos, qos);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to read qos[%02X][%d][%d]: %d (%s)\n"),
                   lp.qos, de, pri, rv, bcm_errmsg(rv)));
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_dscp_qos_map_get(int unit,
                                  bcm_port_t port,
                                  bcm_vlan_t vid,
                                  int isPortVid,
                                  int dscp,
                                  soc_sbx_g3p1_dscpqos_t *qos)
{
    int rv;
    soc_sbx_g3p1_lp_t   lp;

    /* get the logical port */
    if (isPortVid) {
        rv = bcm_caladan3_lp_get_by_pv(unit, port, vid, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to find or access lp for "
                                   " pv2e[%d,%03X]: %d (%s)\n"), 
                       port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to read lp[%08X]: %d (%s)\n"),
                       port, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* First check the lp record is DSCP QOS map is set.
       If not return error saying DSCP QOS map is disabled
     */

    if( !lp.usedscp)
        return BCM_E_DISABLED;

    /* get the QoS data */
    rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, lp.qos, qos);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to read dscp_qos[%02X][%d]: %d (%s)\n"),
                   lp.qos, dscp, rv, bcm_errmsg(rv)));
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_map_get(int unit,
                                            bcm_port_t port,
                                            int pkt_pri,
                                            int cfi,
                                            int *internal_pri,
                                            bcm_color_t *color,
                                            int *remark_pri,
                                            bcm_color_t *remark_color,
                                            int *policer_offset)
{
    int                   rv;
    soc_sbx_g3p1_qos_t    qosMap;

    rv = _bcm_caladan3_g3p1_port_qos_map_get(unit, port, 0, FALSE,
                                           cfi, pkt_pri, &qosMap);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get qos map for port %d pri/cfi=%d/%d "
                               ": %d %s\n"), port, pkt_pri, cfi, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (internal_pri != NULL) {
        *internal_pri = qosMap.fcos;
    }

    if (color != NULL) {
        *color        = qosMap.dp;
    }

    if (remark_pri != NULL) {
        *remark_pri   = qosMap.cos;
    }

    if (remark_color != NULL) {
        *remark_color = qosMap.dp;
    }

    if (policer_offset != NULL) {
        *policer_offset = qosMap.mefcos;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_dscp_map_get(int unit,
                                                  bcm_port_t port,
                                                  int pkt_dscp,
                                                  int *internal_pri,
                                                  bcm_color_t *color,
                                                  int *remark_pri,
                                                  bcm_color_t *remark_color,
                                                  int *policer_offset)
{
    int                   rv;
    soc_sbx_g3p1_dscpqos_t    qosMap;

    rv = _bcm_caladan3_g3p1_port_dscp_qos_map_get(unit, port, 0, FALSE,
                                           pkt_dscp, &qosMap);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get qos map for port %d dscp=%d "
                               ": %d %s\n"), port, pkt_dscp, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (internal_pri != NULL) {
        *internal_pri = qosMap.fcos;
    }

    if (color != NULL) {
        *color        = qosMap.dp;
    }

    if (remark_pri != NULL) {
        *remark_pri   = qosMap.cos;
    }

    if (remark_color != NULL) {
        *remark_color = qosMap.dp;
    }

    if (policer_offset != NULL) {
        *policer_offset = qosMap.mefcos;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_pri_map_get(int unit,
                                       bcm_port_t port,
                                       bcm_vlan_t vid,
                                       int pkt_pri,
                                       int cfi,
                                       int *internal_pri,
                                       bcm_color_t *color)
{
    int rv;
    soc_sbx_g3p1_qos_t qos;

    rv = _bcm_caladan3_g3p1_port_qos_map_get(unit, port, vid,
                                           TRUE /* isPortVid */,
                                           cfi, pkt_pri, &qos);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get qos map for port %d pri/cfi=%d/%d "
                               ": %d %s\n"), port, pkt_pri, cfi, rv, bcm_errmsg(rv)));
        return rv;
    }

    *internal_pri = qos.fcos;
    *color        = qos.dp;

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_mapping_get(int unit,
                                                bcm_port_t port,
                                                bcm_vlan_t vid,
                                                int pkt_pri,
                                                int cfi,
                                                bcm_priority_mapping_t *pri_map)
{
    int                   rv;
    soc_sbx_g3p1_qos_t    qosMap;

    rv = _bcm_caladan3_g3p1_port_qos_map_get(unit, port, 0, FALSE,
                                           cfi, pkt_pri, &qosMap);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get qos map for port %d pri/cfi=%d/%d "
                               ": %d %s\n"), port, pkt_pri, cfi, rv, bcm_errmsg(rv)));
        return rv;
    }

    pri_map->internal_pri   = qosMap.fcos;
    pri_map->color          = qosMap.dp;
    pri_map->remark_internal_pri = qosMap.cos;
    pri_map->remark_color        = qosMap.dp;
    pri_map->policer_offset = qosMap.mefcos;

    return rv;
}


STATIC int
_bcm_caladan3_g3p1_port_qos_map_alloc(int unit,
                                    uint32 *pProfileId)
{
    int                rv;
    int                pri;
    int                cfi;
    int                dscp;
    soc_sbx_g3p1_qos_t qos;
    soc_sbx_g3p1_dscpqos_t dscp_qos;

    /* allocate a new QOS profile */
    rv = _sbx_caladan3_resource_alloc(unit,
                                 SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                 1,
                                 pProfileId,
                                 0);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to allocate QOS map on unit %d: %d (%s)\n"),
                   unit,
                   rv,
                   _SHR_ERRMSG(rv)));
        return rv;
    }

    /* start out with it the same as the default */
    for (pri = 0; (pri < 8) && (BCM_E_NONE == rv); pri ++) {
        for (cfi = 0; (cfi < 2) && (BCM_E_NONE == rv); cfi ++) {
            rv = soc_sbx_g3p1_qos_get(unit,
                                      cfi,
                                      pri,
                                      SBX_QOS_DEFAULT_PROFILE_IDX,
                                      &qos);
            if (BCM_E_NONE == rv) {
                rv = soc_sbx_g3p1_qos_set(unit,
                                          cfi,
                                          pri,
                                          *pProfileId,
                                          &qos);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "unable to write %d:qos[%02X][%d,%d]: %d (%s)\n"),
                               unit,
                               *pProfileId,
                               cfi,
                               pri,
                               rv,
                               _SHR_ERRMSG(rv)));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to read %d:qos[%02X][%d,%d]: %d (%s)\n"),
                           unit,
                           SBX_QOS_DEFAULT_PROFILE_IDX,
                           cfi,
                           pri,
                           rv,
                           _SHR_ERRMSG(rv)));
            }
        }
    }

   /* Start out filling with the default value mapping */
   for(dscp = 0; (dscp <64) && (BCM_E_NONE ==rv) ; dscp ++) {
         rv = soc_sbx_g3p1_dscpqos_get(unit,
                                        dscp,
                                        SBX_QOS_DEFAULT_PROFILE_IDX,
                                        &dscp_qos);
            if (BCM_E_NONE == rv) {
                rv = soc_sbx_g3p1_dscpqos_set(unit,
                                               dscp,
                                               *pProfileId,
                                               &dscp_qos);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "unable to write %d:dscp_qos[%02X][%d]: %d (%s)\n"),
                               unit,
                               *pProfileId,
                               dscp,
                               rv,
                               _SHR_ERRMSG(rv)));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to read %d:dscp_qos[%02X][%d]: %d (%s)\n"),
                           unit,
                           SBX_QOS_DEFAULT_PROFILE_IDX,
                           dscp,
                           rv,
                           _SHR_ERRMSG(rv)));

            }

   }

    if (BCM_E_NONE != rv) {
        /* something went wrong copying the profile; forget it */
        _sbx_caladan3_resource_free(unit,
                               SBX_CALADAN3_USR_RES_QOS_PROFILE,
                               1,
                               pProfileId,
                               0);
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "freed QOS map due to errors initialising it\n")));
    }

    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_port_profile_copy(int unit,
                                   uint32 destProfileIdx,
                                   uint32 srcProfileIdx)
{
    int rv;
    int de, pri;
    int dscp;
    soc_sbx_g3p1_qos_t qos;
    soc_sbx_g3p1_dscpqos_t dscp_qos;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Copy profile %d to %d\n"),
                 srcProfileIdx, destProfileIdx));
    for (de = 0; de < 2; de++) {
        for (pri = 0; pri < 8; pri++) {

            rv = soc_sbx_g3p1_qos_get(unit, de, pri, srcProfileIdx, &qos);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to read qos map[%d,%d,%d] : %d (%s)\n"),
                           de, pri, srcProfileIdx, rv, bcm_errmsg(rv)));
                return rv;
            }

            rv = soc_sbx_g3p1_qos_set(unit, de, pri, destProfileIdx, &qos);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to write qos map[%d,%d,%d] : %d (%s)\n"),
                           de, pri, destProfileIdx, rv, bcm_errmsg(rv)));
                return rv;
            }

        }
    }

    for (dscp = 0; dscp < 64; dscp++) {

            rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, srcProfileIdx, &dscp_qos);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to read dscp_qos map[%d,%d] : %d (%s)\n"),
                           dscp, srcProfileIdx, rv, bcm_errmsg(rv)));
                return rv;
            }

            rv = soc_sbx_g3p1_dscpqos_set(unit, dscp, destProfileIdx, &dscp_qos);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to write qos map[%d,%d] : %d (%s)\n"),
                           dscp, destProfileIdx, rv, bcm_errmsg(rv)));
                return rv;
            }

        }
    return BCM_E_NONE;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_port_profile_shareable
 *   Purpose
 *      Find a suitable profile that can be shared with the given changed
 *      qos map line
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  curProfileIdx - current profile index in use
 *      (in)  deChange      - drop eligibity index to change
 *      (in)  priChange     - pri index to change
 *      (in)  qosChange     - new mapping for de,pri
 *      (out) shrProfileIdx - shareable profile index
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 *   Returns:
 *     BCM_E_NOT_FOUND  - no suitable profile available
 *     BCM_E_NONE       - found shareable profile
 */
int
_bcm_caladan3_g3p1_port_profile_shareable(int unit, uint32 curProfileIdx,
                                        int deChange, int priChange,
                                        int dscpChange, int dscp_modify,
                                        soc_sbx_g3p1_qos_t *qosChange,
                                        soc_sbx_g3p1_dscpqos_t *dscpQosChange,
                                        uint32 *shrProfileIdx)
{
    int        rv = BCM_E_NOT_FOUND;
    uint32   searchIdx;
    int        sharePriProfile, shareDscpProfile;
    int        de, pri,dscp;
    soc_sbx_g3p1_qos_t *pQosRequest;
    soc_sbx_g3p1_qos_t  qosRequest, qosInUse;
    soc_sbx_g3p1_dscpqos_t *pDscpQosRequest;
    soc_sbx_g3p1_dscpqos_t  dscpQosRequest, dscpQosInUse;

    if (LOG_CHECK(BSL_LS_BCM_PORT | BSL_VERBOSE)) {

        if(!dscp_modify)
        {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "cprof=%d de/pri=%d/%d qosMap:\n  "),
                      curProfileIdx,
                      deChange, priChange));
#if 0
            soc_sbx_g3p1_qos_print(unit, qosChange);
#endif
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "\n")));
       } else {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "cprof=%d dscp=%d dscpQosMap:\n  "),
                      curProfileIdx,
                      dscpChange));
#if 0
            soc_sbx_g3p1_dscpqos_print(unit, dscpQosChange);
#endif
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "\n")));
       }
    }

    /* all three must be set to see this amount of data - */
    if (LOG_CHECK(BSL_LS_BCM_PORT | BSL_DEBUG)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Finding shareable map for profile=%d\n"), 
                  curProfileIdx));
        _bcm_caladan3_g3p1_port_qos_profile_dump(unit, curProfileIdx);
    }

    /* foreach profile (including default)
     *   foreach profile line (de,pri)
     *       if (profile[i].line != curProfile.line
     *          next profile
     */
    sharePriProfile = FALSE;
    shareDscpProfile = FALSE;
    for (searchIdx = 0;
         searchIdx < max_qos_profile_idx && !(sharePriProfile && shareDscpProfile);
         searchIdx++) {

        /* compare the current profile to the given profile, assume they are
         * the same and can be shared until proven otherwise
         */
        if (PROFILE_REF_COUNT(unit, searchIdx) ||
            searchIdx ==  SBX_QOS_DEFAULT_PROFILE_IDX) {

            /* all three must be set to see this amount of data - */
            if (LOG_CHECK(BSL_LS_BCM_PORT | BSL_DEBUG)) {
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Comparing against profile=%d\n"),
                          searchIdx));
                _bcm_caladan3_g3p1_port_qos_profile_dump(unit, searchIdx);
            }

            /* assume match to start */
            sharePriProfile = TRUE;

            for (de=0; de < 2 && sharePriProfile; de++) {
                for (pri=0; pri < 8 && sharePriProfile; pri++) {

                    /* optimization:
                     * when cur==searchIdx, only need to compare change request
                     *
                     */
                    if (curProfileIdx == searchIdx &&
                        !(deChange == de && priChange == pri) && !dscp_modify) {
                        continue;
                    }

                    rv = soc_sbx_g3p1_qos_get(unit, de, pri, searchIdx,
                                              &qosInUse);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_PORT,
                                  (BSL_META_U(unit,
                                              "Error %d(%s) reading qos table.  "
                                               "profile=%d de/pri=%d/%d\n"),
                                   rv,_SHR_ERRMSG(rv),
                                   searchIdx, de, pri));
                        return rv;
                    }

                    if (deChange == de && priChange == pri && !dscp_modify) {
                        pQosRequest = qosChange;
                    } else {

                        rv = soc_sbx_g3p1_qos_get(unit, de, pri, curProfileIdx,
                                                  &qosRequest);
                        if (BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_PORT,
                                      (BSL_META_U(unit,
                                                  "Error %d(%s) reading given qos table.  "
                                                   "profile=%d de/pri=%d/%d\n"),
                                       rv,_SHR_ERRMSG(rv),
                                       curProfileIdx, de, pri));
                            return rv;
                        }

                        pQosRequest = &qosRequest;
                    }

                    if (sal_memcmp(pQosRequest, &qosInUse, sizeof(qosInUse)) != 0) {
                        LOG_VERBOSE(BSL_LS_BCM_PORT,
                                    (BSL_META_U(unit,
                                                "profiles differ, cannot be shared. \n"
                                                 "  curProfile=%d testProfile=%d"
                                                 " changeDe/Pri=%d/%d\n"),
                                     curProfileIdx, searchIdx,
                                     deChange, priChange));
                        sharePriProfile = FALSE;
                    }
                }
            }
            /* assume match to start */
            shareDscpProfile = TRUE;

            /* check for matching profile in the DACP QOS table */
            for(dscp = 0; dscp <64 && shareDscpProfile; dscp ++) {

                    /* optimization:
                     * when cur==searchIdx, only need to compare change request
                     *
                     */
                    if (curProfileIdx == searchIdx &&
                        !(dscpChange == dscp) && dscp_modify) {
                        continue;
                    }

                    rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, searchIdx,
                                              &dscpQosInUse);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_PORT,
                                  (BSL_META_U(unit,
                                              "Error %d(%s) reading dscp qos table.  "
                                               "profile=%d dscp=%d\n"),
                                   rv,_SHR_ERRMSG(rv),
                                   searchIdx, dscp));
                        return rv;
                    }

                    if (dscpChange == dscp && dscp_modify) {
                        pDscpQosRequest = dscpQosChange;
                    } else {

                        rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, curProfileIdx,
                                                  &dscpQosRequest);
                        if (BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_PORT,
                                      (BSL_META_U(unit,
                                                  "Error %d(%s) reading given dscp qos table.  "
                                                   "profile=%d dscp=%d\n"),
                                       rv,
                                       _SHR_ERRMSG(rv),
                                       curProfileIdx, dscp));
                            return rv;
                        }

                        pDscpQosRequest = &dscpQosRequest;
                    }

                    if (sal_memcmp(pDscpQosRequest, &dscpQosInUse, sizeof(dscpQosInUse)) != 0) {
                        LOG_VERBOSE(BSL_LS_BCM_PORT,
                                    (BSL_META_U(unit,
                                                "profiles differ, cannot be shared. \n"
                                                 "  curProfile=%d testProfile=%d"
                                                 " changeDe/Pri=%d/%d\n"),
                                     curProfileIdx, searchIdx,
                                     deChange, priChange));
                        shareDscpProfile = FALSE;
                    }

            }


            /* found a profile that is the same as requested; share them */
            if (shareDscpProfile && sharePriProfile) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "profiles match, sharing profile=%d\n"),
                             searchIdx));

                *shrProfileIdx = searchIdx;
                rv = BCM_E_NONE;
            } else {
                rv = BCM_E_NOT_FOUND;
            }
        }
    }

    /* if, after scanning for a complete match, this profile is referenced
     * only by this user, and it is not the default, then it is shareable
     */
    if (rv == BCM_E_NOT_FOUND &&
        PROFILE_REF_COUNT(unit, curProfileIdx) == 1 &&
        (curProfileIdx != SBX_QOS_DEFAULT_PROFILE_IDX)) {

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Only user of profile=%d, marked as shareable\n"),
                     curProfileIdx));
        *shrProfileIdx = curProfileIdx;
        return BCM_E_NONE;
    }

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_g3p1_port_qos_profile_manage
 *   Purpose
 *      Manage the shareable qos profile resource.
 *   Parameters
 *      (in)  unit       - BCM device number
 *      (in)  port       - The owning port of the profile to manage
 *      (in)  vid        - The owning vid of the profile to manage, if isPortVid
 *      (in)  isPortVid  - Indicates vid is a vaild param
 *      (in)  de         - Drop Eligibility to update
 *      (in)  pri        - Priority to update
 *      (in)  qosMapChange - qos map line to commit
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 *
 *    Seeks out a shareable qos profile with the given change.  If none
 *    exists, allocate a new qos profile; copy the origial mapping to new
 *    profile.  Frees unreferenced profiles.
 */
 STATIC int
_bcm_caladan3_g3p1_port_qos_profile_manage(int unit,
                                         bcm_port_t port,
                                         bcm_vlan_t vid,
                                         int isPortVid,
                                         int de, int pri,
                                         int dscp, int dscp_change,
                                         soc_sbx_g3p1_qos_t *qosMapChange,
                                         soc_sbx_g3p1_dscpqos_t *dscpQosMapChange)
{
    int                         rv;
    int                         updateMap = FALSE;
    _bcm_caladan3_vlan_divergence_t divergence = _CALADAN3_VLAN_DIVERGE_QOS;
    int                         causeDivergence;
    int                         forceAlloc = 0;
    uint32                    shrProfileIdx;
    uint32                    oldProfileIdx;
    soc_sbx_g3p1_lp_t           tempLp;
    soc_sbx_g3p1_lp_t           lp;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif

    soc_sbx_g3p1_lp_t_init(&lp);

    
    

    /* passing a vid of 0 forces a divergence of the qos profile for untagged
     * traffic from the native vid. Only check for PRI QOS. Neglect for DSCP mapping
     */
    causeDivergence = (isPortVid && (vid == 0) &&(!dscp_change));

    /* get the logical port information; needed for current qos profile value
     */
    if (isPortVid) {

        rv = bcm_caladan3_lp_get_by_pv(unit, port, vid, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to get lp for port/vid=%d/0x%x: %d (%s)\n"),
                       port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {

        rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to read lp for port %d: %d (%s)\n"), 
                       port, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* need a copy of the original state */
    tempLp = lp;
    oldProfileIdx = lp.qos;

    /* if setting for VID 0, check for divergence */
    if (causeDivergence) {
        rv = bcm_caladan3_lp_check_diverged(unit, port, vid, &divergence);
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to check lp divergence for pv2e[%d,0x%03X]"
                                   ": %d (%s)\n"),  port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
        forceAlloc = !(divergence & _CALADAN3_VLAN_DIVERGE_QOS);
    }

    /*
     *  If causeDivergence (ie vid == 0), and qos is not already diverged,
     *  skip checking to share profiles and straight to allocation
     */
    if (forceAlloc) {

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Divergence caused; force new profile allocation\n")));
        rv = BCM_E_NOT_FOUND;  /* allocate a new profile */
    } else {

        /* Find a shareable qos profile, if one exists */
        rv = _bcm_caladan3_g3p1_port_profile_shareable(unit, lp.qos,
                                                     de, pri,
                                                     dscp, dscp_change,
                                                     qosMapChange,
                                                     dscpQosMapChange,
                                                     &shrProfileIdx);
    }

    /* no suitable profile found (or forced), must allocate one,
     * and update the qos map line requested
     *
     * Or, another profile has been found to match; only need to
     * update the logical_port.qos value
     */
    if (rv == BCM_E_NOT_FOUND ||
        (BCM_SUCCESS(rv) && shrProfileIdx != oldProfileIdx)) {

        updateMap = TRUE;

        /* save the new profile to local var so reference counts are
         * updated properly below
         */
        if (rv == BCM_E_NOT_FOUND) {
            rv = _bcm_caladan3_g3p1_port_qos_map_alloc(unit, &shrProfileIdx);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to allocate QOS map for pv2e[%d,%03X]:"
                                       " %d (%s)\n"), port, vid, rv, bcm_errmsg(rv)));
                return rv;
            }

            /* copy the existing to the newly allocated */
            _bcm_caladan3_g3p1_port_profile_copy(unit,
                                               shrProfileIdx, oldProfileIdx);
        }

        lp.qos = shrProfileIdx;

        /* In case of dscp map, set the lp.usedscp to 1 */
        if(dscp_change)
        {
            lp.usedscp = 1;
        }

        /* configure the <port,vlan> to use the profile, if caller specified */
        if (isPortVid) {
            rv = bcm_caladan3_lp_set_by_pv_and_diverge(unit, port, vid,
                                                     _CALADAN3_VLAN_DIVERGE_QOS,
                                                     &lp);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to write lp for pv2e[%d,%03X]: %d (%s)\n"),
                           port, vid, rv, bcm_errmsg(rv)));

                /* could not write newly allocated qos map; free it */
                _sbx_caladan3_resource_free(unit,
                                       SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                       1,
                                       &shrProfileIdx,
                                       0);
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "freed QOS map due to errors using it\n")));
                return rv;
            }

        } else {

            /* configure the physical port to use the profile */
            rv = soc_sbx_g3p1_lp_set(unit, port, &lp);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to write lp for port %d: %d (%s)\n"),
                           port, rv, bcm_errmsg(rv)));

                /* could not write newly allocated qos map; free it */
                _sbx_caladan3_resource_free(unit,
                                       SBX_CALADAN3_USR_RES_QOS_PROFILE, 1,
                                       &shrProfileIdx, 0);
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "freed QOS map due to errors using it\n")));
                return rv;
            }

            /* need to propagate p2e.lp change to pv2e.lp for all VIDs this port */
            rv = bcm_caladan3_vlan_port_lp_replicate(unit, port,
                                                   BCM_CALADAN3_LP_COPY_QOS,
                                                   &tempLp, &lp);
            if (BCM_FAILURE(rv)) { 
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to update pv2e entries for new QOS:"
                                       " %d (%s)\n"), rv, bcm_errmsg(rv)));
                return rv;
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unexpected error while searching for a shareable "
                               "qos profile: %d (%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    /* update reference counts if switching from one to profile to another */
    if ((shrProfileIdx != oldProfileIdx) && !forceAlloc) {
        PROFILE_REF_COUNT(unit, shrProfileIdx)++;
        PROFILE_REF_COUNT(unit, oldProfileIdx)--;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->profile_count_offset + (shrProfileIdx * sizeof(int)),
            PROFILE_REF_COUNT(unit, shrProfileIdx));
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->profile_count_offset + (oldProfileIdx * sizeof(int)),
            PROFILE_REF_COUNT(unit, oldProfileIdx));
#endif

        if (PROFILE_REF_COUNT(unit, oldProfileIdx) == 0 &&
            (oldProfileIdx !=  SBX_QOS_DEFAULT_PROFILE_IDX)) {

            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "Freeing profile=%d\n"),
                         oldProfileIdx));
            _sbx_caladan3_resource_free(unit,
                                   SBX_CALADAN3_USR_RES_QOS_PROFILE, 1,
                                   &oldProfileIdx, 0);
        }
    } else {
        PROFILE_REF_COUNT(unit, shrProfileIdx)++;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->profile_count_offset + (shrProfileIdx * sizeof(int)),
            PROFILE_REF_COUNT(unit, shrProfileIdx));
#endif
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "updated profile reference counts to:  "
                             "shrProfile=%d count=%d; oldProfile=%d count=%d\n"),
                 shrProfileIdx, PROFILE_REF_COUNT(unit, shrProfileIdx),
                 oldProfileIdx, PROFILE_REF_COUNT(unit, oldProfileIdx)));

    /* don't update the default profile, not matter what */
    if ((updateMap || (shrProfileIdx == oldProfileIdx)) &&
        lp.qos != SBX_QOS_DEFAULT_PROFILE_IDX)
    {

       /* Update the corresponding Table - PRI/EXP QOS or DSCP QOS */
       if(!dscp_change)
       {
            rv = soc_sbx_g3p1_qos_set(unit, de, pri, lp.qos, qosMapChange);
            if (BCM_FAILURE(rv)) { 
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to update qos map profile= %d de,pri=%d,%d"
                                       " :%d (%s)\n"), lp.qos, de, pri, 
                           rv, bcm_errmsg(rv)));
                return rv;
            }
       }
       else
       {
            rv = soc_sbx_g3p1_dscpqos_set(unit, dscp, lp.qos, dscpQosMapChange);
            if (BCM_FAILURE(rv)) { 
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "unable to update dscp_qos map profile= %d dscp=%d"
                                       " :%d (%s)\n"), lp.qos, dscp, 
                           rv, bcm_errmsg(rv)));
                return rv;
           }
       }

    }

    return rv;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_port_egr_remark_set
 *   Purpose
 *      Set the remark table for the given port's ete
 *   Parameters
 *      (in)  unit       - BCM device number
 *      (in)  port       - The port's remark index to change
 *      (in)  remark     - The new remark index
 *      (in)  dscp_remark- indicates remark index contains l3 mappings
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 *     Modifies the globally configured Port ETE.
 */
static int
_bcm_caladan3_g3p1_port_egr_remark_set(int unit, bcm_port_t port, 
                                     int remark, uint32 dscp_remark)
{
    uint32                 etei;
    soc_sbx_g3p1_ete_t     ete;
    int                    rv = BCM_E_NONE;

    if (remark < 0) {
        return BCM_E_NONE;
    }

    etei = SOC_SBX_PORT_ETE(unit, port);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Updating port %d ete 0x%04x\n"),
                 port, etei));

    rv = soc_sbx_g3p1_ete_get(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    ete.remark     = remark ? remark : PORT(unit, port).egr_remark_table_idx;
    ete.dscpremark = dscp_remark;
    
    rv = soc_sbx_g3p1_ete_set(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to write ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}


/*
 * Force a port,vid back to the default QoS profile.  The existing qos profile
 * will be managed such that the refcount is decremented, and freed if 0
 *
 *  Assumes port lock has been aquired
 */
int
_bcm_caladan3_g3p1_port_default_qos_profile_set(int unit, bcm_port_t port,
                                              bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_pv2e_t pv2e;
    soc_sbx_g3p1_lp_t lp;
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_caladan3_wb_port_state_scache_info_t *wb_info_ptr = NULL;
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    wb_info_ptr = SBX_PORT_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif
    
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Setting port/vid %d/0x%x back to default\n"),
                 port, vid));

    rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &pv2e);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to get pv2e for port/vid=%d/0x%x: %d (%s)\n"),
                   port, vid, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g3p1_lp_get(unit, pv2e.lpi, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to get lp for port/vid=%d/0x%x: %d (%s)\n"),
                   port, vid, rv, bcm_errmsg(rv)));
        return rv;
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "current qos profile=%d refCount=%d\n"), 
                 lp.qos, PROFILE_REF_COUNT(unit, lp.qos)));
    if (PROFILE_REF_COUNT(unit, lp.qos)) {
        PROFILE_REF_COUNT(unit, lp.qos)--;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->profile_count_offset + (lp.qos * sizeof(int)),
            PROFILE_REF_COUNT(unit, lp.qos));
#endif
    } else if (PROFILE_REF_COUNT(unit, lp.qos) == 0) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "invalid reference count for qos profile %d\n"),
                   lp.qos));
        return BCM_E_INTERNAL;
    }

    if (PROFILE_REF_COUNT(unit, lp.qos) == 0) {
        uint32 tmp = lp.qos;

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Freeing profile=%d\n"),
                     lp.qos));
        _sbx_caladan3_resource_free(unit,
                               SBX_CALADAN3_USR_RES_QOS_PROFILE, 1,
                               &tmp, 0);
    }

    lp.qos = SBX_QOS_DEFAULT_PROFILE_IDX;
    rv = soc_sbx_g3p1_lp_set(unit, pv2e.lpi, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to write lp for port/vid=%d/0x%x: %d (%s)\n"),
                   port, vid, rv, bcm_errmsg(rv)));
        return rv;
    }

    PROFILE_REF_COUNT(unit, lp.qos)++;
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(int, 1,
            wb_info_ptr->profile_count_offset + (lp.qos * sizeof(int)),
            PROFILE_REF_COUNT(unit, lp.qos));
#endif

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_map_set(int unit,
                                            bcm_port_t port,
                                            int pkt_pri,
                                            int cfi,
                                            int internal_pri,
                                            bcm_color_t color,
                                            int remark_internal_pri,
                                            int remark_color,
                                            int policer_offset)
{
    int rv;
    soc_sbx_g3p1_qos_t qosMap;
    soc_sbx_g3p1_dscpqos_t dscpQosMap;

    PORT_PARAM_CHECK(unit, port,           0, SBX_MAX_PORTS-1);
    PORT_PARAM_CHECK(unit, pkt_pri,        0, 7);
    PORT_PARAM_CHECK(unit, cfi,            0, 1);
    PORT_PARAM_CHECK(unit, internal_pri,   0, 7);
    PORT_PARAM_CHECK(unit, color,          0, 3);
    PORT_PARAM_CHECK(unit, remark_internal_pri, 0, 7);
    PORT_PARAM_CHECK(unit, remark_color,   0, 3);
    PORT_PARAM_CHECK(unit, policer_offset, 0, 7);

    soc_sbx_g3p1_qos_t_init(&qosMap);
    soc_sbx_g3p1_dscpqos_t_init(&dscpQosMap);

    qosMap.e      = 0;
    qosMap.dp     = color;
    qosMap.cos    = remark_internal_pri;
    qosMap.fcos   = internal_pri;
    qosMap.mefcos = policer_offset;

    
    

    rv = _bcm_caladan3_g3p1_port_qos_profile_manage(unit, port, 0, FALSE,
                                                  cfi, pkt_pri,0, FALSE,
                                                  &qosMap, &dscpQosMap);
    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_pri_map_set(int unit,
                                       bcm_port_t port,
                                       bcm_vlan_t vid,
                                       int pkt_pri,
                                       int cfi,
                                       int internal_pri,
                                       int color)
{
    int rv;
    soc_sbx_g3p1_qos_t qosMap;
    soc_sbx_g3p1_dscpqos_t dscpQosMap;

    PORT_PARAM_CHECK(unit, port,          0, SBX_MAX_PORTS-1);
    PORT_PARAM_CHECK(unit, pkt_pri,       0, 7);
    PORT_PARAM_CHECK(unit, cfi,           0, 1);
    PORT_PARAM_CHECK(unit, internal_pri,  0, 7);
    PORT_PARAM_CHECK(unit, color,         0, 3);

    soc_sbx_g3p1_qos_t_init(&qosMap);

    qosMap.e      = 0;
    qosMap.dp     = color;
    qosMap.cos    = internal_pri;
    qosMap.fcos   = internal_pri;
    qosMap.mefcos = internal_pri;

    
    

    rv = _bcm_caladan3_g3p1_port_qos_profile_manage(unit, port, vid, TRUE,
                                                  cfi, pkt_pri,0, FALSE,
                                                  &qosMap, &dscpQosMap);
    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_mapping_set(int unit,
                                                bcm_port_t port,
                                                bcm_vlan_t vid,
                                                int pkt_pri,
                                                int cfi,
                                                bcm_priority_mapping_t *pri_map)
{
    int rv;
    soc_sbx_g3p1_qos_t qosMap;
    soc_sbx_g3p1_dscpqos_t dscpQosMap;

    PORT_PARAM_CHECK(unit, port,          0, SBX_MAX_PORTS-1);
    PORT_PARAM_CHECK(unit, pkt_pri,       0, 7);
    PORT_PARAM_CHECK(unit, cfi,           0, 1);
    PORT_PARAM_CHECK(unit, pri_map->internal_pri,        0, 7);
    PORT_PARAM_CHECK(unit, pri_map->color,               0, 3);
    PORT_PARAM_CHECK(unit, pri_map->remark_internal_pri, 0, 7);
    PORT_PARAM_CHECK(unit, pri_map->policer_offset,      0, 7);

    soc_sbx_g3p1_qos_t_init(&qosMap);

    qosMap.e      = 0;
    qosMap.dp     = pri_map->color;
    qosMap.cos    = pri_map->remark_internal_pri;
    qosMap.fcos   = pri_map->internal_pri;
    qosMap.mefcos = pri_map->policer_offset;

    
    

    rv = _bcm_caladan3_g3p1_port_qos_profile_manage(unit, port, vid, TRUE,
                                                  cfi, pkt_pri,0, FALSE,
                                                  &qosMap, &dscpQosMap);
    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_unmap_get(int unit,
                                              bcm_port_t port,
                                              int internal_pri,
                                              bcm_color_t color,
                                              int *pkt_pri,
                                              int *cfi)
{
    int                   rv;
    soc_sbx_g3p1_remark_t zEgrRemark;

    rv = soc_sbx_g3p1_remark_get(unit,
                                 0,
                                 color,
                                 internal_pri,
                                 PORT(unit,port).egr_remark_table_idx,
                                 &zEgrRemark);
    if (BCM_E_NONE == rv) {
        *pkt_pri  = zEgrRemark.pri;
        *cfi      = zEgrRemark.cfi;
    } else {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to read %d:remark[%02X][%d,%d,%d]: %d (%s)\n"),
                   unit,
                   PORT(unit,port).egr_remark_table_idx,
                   0,
                   color,
                   internal_pri,
                   rv,
                   _SHR_ERRMSG(rv)));
    }

    return rv;
}


int
_bcm_caladan3_g3p1_port_vlan_priority_unmap_set(int unit,
                                              bcm_port_t port,
                                              int internal_pri,
                                              bcm_color_t color,
                                              int pkt_pri,
                                              int cfi)
{
    int                   rv;
    soc_sbx_g3p1_remark_t zEgrRemark;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Port %d updating remark=%d\n"),
                 port, PORT(unit,port).egr_remark_table_idx));


#if 0
    if (PORT(unit, port).egr_remark_table_idx == 
        SBX_QOS_DEFAULT_REMARK_IDX)
    {
        int flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
        rv = _bcm_caladan3_port_qos_map_create(unit, flags, port);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Failed to create qos map: %s\n"),
                       bcm_errmsg (rv)));
            return rv;
        }
    }
#endif

    rv = soc_sbx_g3p1_remark_get(unit,
                                 0,
                                 color,
                                 internal_pri,
                                 PORT(unit,port).egr_remark_table_idx,
                                 &zEgrRemark);
    if (BCM_E_NONE == rv) {
        zEgrRemark.pri = pkt_pri;
        zEgrRemark.cfi = cfi;
        rv = soc_sbx_g3p1_remark_set(unit,
                                     0,
                                     color,
                                     internal_pri,
                                     PORT(unit,port).egr_remark_table_idx,
                                     &zEgrRemark);
    }
    if (BCM_E_NONE != rv) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to update %d:remark[%02X][%d,%d,%d]: %d (%s)\n"),
                   unit,
                   PORT(unit,port).egr_remark_table_idx,
                   0,
                   color,
                   internal_pri,
                   rv,
                   _SHR_ERRMSG(rv)));
    }

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_frame_max_access
 * Description:
 *     Set/get the MTU maximum frame size for the port in the iLib ucode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - (IN/OUT) Maximum frame size in bytes
 *     set  - Indicates whether to set or get
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_caladan3_g3p1_port_frame_max_access(int unit, bcm_port_t port,
                                       int *size, int set)
{
    soc_sbx_g3p1_ete_t    ete;
    int                   eteIdx;

    /* Tagged */
    eteIdx = SOC_SBX_PORT_ETE(unit, port);
    soc_sbx_g3p1_ete_t_init(&ete);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ete_get(unit, eteIdx, &ete));

    /* For 'get', just return value for 'tagged' case */
    if (!set) {
        *size = ete.mtu+4;
        return BCM_E_NONE;
    }

    ete.mtu = *size-4;
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ete_set(unit, eteIdx, &ete));

    /* Untagged */
    eteIdx = SOC_SBX_PORT_UT_ETE(unit, port);
    soc_sbx_g3p1_ete_t_init(&ete);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ete_get(unit, eteIdx, &ete));

    ete.mtu = *size-4;
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ete_set(unit, eteIdx, &ete));

    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_port_qos_profile_dump(int unit, int profile)
{
    int de, pri, dscp;
    soc_sbx_g3p1_qos_t qosEnt;
    soc_sbx_g3p1_dscpqos_t dscpQosEnt;
    int rv;

    LOG_CLI((BSL_META_U(unit,
                        "de pri | e dp cos fcos mefcos\n")));
    LOG_CLI((BSL_META_U(unit,
                        "-------|---------------------\n")));
    for (de=0; de < 2; de++) {
        for (pri=0; pri < 8; pri++) {
            rv = soc_sbx_g3p1_qos_get(unit, de, pri, profile, &qosEnt);
            if (BCM_SUCCESS(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    "%2d %3d | %d %2d %3d %4d %6d\n"),
                         de, pri, qosEnt.e, qosEnt.dp, qosEnt.cos,
                         qosEnt.fcos, qosEnt.mefcos));
            }
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "dscp | e dp cos fcos mefcos\n")));
    LOG_CLI((BSL_META_U(unit,
                        "-----|---------------------\n")));

    for (dscp=0; dscp < 64; dscp++) {
        rv = soc_sbx_g3p1_dscpqos_get(unit, dscp, profile, &dscpQosEnt);
            if (BCM_SUCCESS(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    "%2d | %d %2d %3d %4d %6d\n"),
                         dscp, dscpQosEnt.e, dscpQosEnt.dp, dscpQosEnt.cos,
                         dscpQosEnt.fcos, dscpQosEnt.mefcos));
            }

    }
    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_port_vlan_dscp_map_set(int unit,
                                        bcm_port_t port,
                                        bcm_vlan_t vlan,
                                        int dscp,
                                        int internal_pri,
                                        bcm_color_t color)
{
    int rv;
    soc_sbx_g3p1_qos_t qosMap;
    soc_sbx_g3p1_dscpqos_t dscpQosMap;

    PORT_PARAM_CHECK(unit, port,          0, SBX_MAX_PORTS-1);
    PORT_PARAM_CHECK(unit, dscp,          0, 63);
    PORT_PARAM_CHECK(unit, internal_pri,  0, 7);
    PORT_PARAM_CHECK(unit, color,         0, 3);

    soc_sbx_g3p1_qos_t_init(&qosMap);
    soc_sbx_g3p1_dscpqos_t_init(&dscpQosMap);

    dscpQosMap.e      = 0;
    dscpQosMap.dp     = color;
    dscpQosMap.cos    = FCOS_2_RCOS(internal_pri);
    dscpQosMap.fcos   = internal_pri;
    dscpQosMap.mefcos = internal_pri;


    
    

    rv = _bcm_caladan3_g3p1_port_qos_profile_manage(unit, port, vlan, TRUE,
                                                  0,0,dscp,TRUE,
                                                  &qosMap,&dscpQosMap );
    return rv;

}


int
_bcm_caladan3_g3p1_port_dscp_map_set(int unit,
                                   bcm_port_t port,
                                   int dscp,
                                   int internal_pri,
                                   bcm_color_t color)
{
    int rv;
    soc_sbx_g3p1_qos_t qosMap;
    soc_sbx_g3p1_dscpqos_t dscpQosMap;

    PORT_PARAM_CHECK(unit, port,          0, SBX_MAX_PORTS-1);
    PORT_PARAM_CHECK(unit, dscp,          0, 63);
    PORT_PARAM_CHECK(unit, internal_pri,  0, 7);
    PORT_PARAM_CHECK(unit, color,         0, 3);

    soc_sbx_g3p1_qos_t_init(&qosMap);
    soc_sbx_g3p1_dscpqos_t_init(&dscpQosMap);

    dscpQosMap.e      = 0;
    dscpQosMap.dp     = color;
    dscpQosMap.cos    = FCOS_2_RCOS(internal_pri);
    dscpQosMap.fcos   = internal_pri;
    dscpQosMap.mefcos = 0;

    
    

    rv = _bcm_caladan3_g3p1_port_qos_profile_manage(unit, port, 0, FALSE,
                                                  0,0,dscp,TRUE,
                                                  &qosMap,&dscpQosMap );
    return rv;

}


int
_bcm_caladan3_g3p1_port_vlan_dscp_map_get(int unit,
                                        bcm_port_t port,
                                        bcm_vlan_t vid,
                                        int pkt_dscp,
                                        int *internal_pri,
                                        bcm_color_t *color)
{
    int rv;
    soc_sbx_g3p1_dscpqos_t qos;

    rv = _bcm_caladan3_g3p1_port_dscp_qos_map_get(unit, port, vid,
                                                TRUE /* isPortVid */,
                                                pkt_dscp, &qos);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get dscp_qos map for port %d dscp=%d/$d "
                               ": %d %s\n"), port, pkt_dscp, rv, bcm_errmsg(rv)));
        return rv;
    }

    *internal_pri = qos.fcos;
    *color        = qos.dp;

    return rv;
}


int
_bcm_caladan3_g3p1_port_dscp_map_get(int unit,
                                   bcm_port_t port,
                                   int pkt_dscp,
                                   int *internal_pri,
                                   int *color)
{
    int rv;
    soc_sbx_g3p1_dscpqos_t qos;

    rv = _bcm_caladan3_g3p1_port_dscp_qos_map_get(unit, port, 0,
                                                FALSE /* isPortVid */,
                                                pkt_dscp, &qos);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Failed to get dscp_qos map for port %d dscp=%d/$d "
                               ": %d %s\n"), port, pkt_dscp, rv, bcm_errmsg(rv)));
        return rv;
    }

    *internal_pri = qos.fcos;
    *color        = qos.dp;

    return rv;
}


int
_bcm_caladan3_g3p1_port_dscp_unmap_set(int unit,
                                     bcm_port_t port,
                                     int internal_pri,
                                     bcm_color_t color,
                                     int pkt_dscp)
{
    int                   rv;
    uint32              etei;
    soc_sbx_g3p1_ete_t    sbx_ete;
    soc_sbx_g3p1_remark_t zEgrRemark;

    PORT_PARAM_CHECK(unit, pkt_dscp,      0, 63);
    PORT_PARAM_CHECK(unit, internal_pri,  0, 7);
    PORT_PARAM_CHECK(unit, color,         0, 3);

    /* Get the port ETE and then set the dscpremark so that
       DSCP get remarked with the given map */

    etei = SOC_SBX_PORT_ETE(unit, port);

    rv = soc_sbx_g3p1_ete_get(unit, etei, &sbx_ete);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Read tag %d:ete[%08X]\n"),
                 unit, etei));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "port_dscp_unmap_set: unit=%d port=%d soc_sbx_g1p1_ete_get\n"),
                   unit, port));
        return rv;
    }

    sbx_ete.dscpremark = 1;
    rv = soc_sbx_g3p1_ete_set(unit, etei, &sbx_ete);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Write tag %d:ete[%08X]\n"),
                 unit, etei));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "port_dscp_unmap_set: unit=%d port=%d soc_sbx_g3p1_ete_set\n"),
                   unit, port));
        return rv;
    }

    /* Now write the Egress remark table to set the mapping of COS.Dp to DSCP */

#if 0
    if (PORT(unit, port).egr_remark_table_idx == 
        SBX_QOS_DEFAULT_REMARK_IDX)
    {
        int flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
        rv = _bcm_caladan3_port_qos_map_create(unit, flags, port);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "Failed to create qos map: %s\n"),
                       bcm_errmsg (rv)));
            return rv;
        }
    }
#endif

    rv = soc_sbx_g3p1_remark_get(unit,
                                 0,
                                 color,
                                 internal_pri,
                                 PORT(unit,port).egr_remark_table_idx,
                                 &zEgrRemark);
    if (BCM_E_NONE == rv) {
        zEgrRemark.dscp = pkt_dscp;
        rv = soc_sbx_g3p1_remark_set(unit,
                                     0,
                                     color,
                                     internal_pri,
                                     PORT(unit,port).egr_remark_table_idx,
                                     &zEgrRemark);
    }
    if (BCM_E_NONE != rv) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to update %d:remark[%02X][%d,%d,%d]: %d (%s)\n"),
                   unit,
                   PORT(unit,port).egr_remark_table_idx,
                   0,
                   color,
                   internal_pri,
                   rv,
                   _SHR_ERRMSG(rv)));
    }

    return rv;

}


int
_bcm_caladan3_g3p1_port_dscp_unmap_get(int unit,
                                     bcm_port_t port,
                                     int internal_pri,
                                     bcm_color_t color,
                                     int *pkt_dscp)
{
    int                   rv;
    uint32              etei;
    soc_sbx_g3p1_ete_t    sbx_ete;
    soc_sbx_g3p1_remark_t zEgrRemark;

    /* Get the port ETE and check if dscp remark is configured */

    etei = SOC_SBX_PORT_ETE(unit, port);

    rv = soc_sbx_g3p1_ete_get(unit, etei, &sbx_ete);
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Read tag %d:ete[%08X]\n"),
                 unit, etei));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "port_dscp_unmap_get: unit=%d port=%d soc_sbx_g3p1_ete_get\n"),
                   unit, port));
        return rv;
    }

    /* check if dscp remark is configured. Else return error */
    if( ! sbx_ete.dscpremark)
    {
      LOG_VERBOSE(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "DSCP remark not configure for unit %d, port %d\n"),
                   unit, port));
      return BCM_E_DISABLED;
    }

    rv = soc_sbx_g3p1_remark_get(unit,
                                 0,
                                 color,
                                 internal_pri,
                                 PORT(unit,port).egr_remark_table_idx,
                                 &zEgrRemark);
    if (BCM_E_NONE == rv) {
        *pkt_dscp  = zEgrRemark.dscp;
    } else {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to read %d:remark[%02X][%d,%d,%d]: %d (%s)\n"),
                   unit,
                   PORT(unit,port).egr_remark_table_idx,
                   0,
                   color,
                   internal_pri,
                   rv,
                   _SHR_ERRMSG(rv)));
    }

    return rv;

}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_dscp_map_mode_get
 * Description:
 *    Gets the DSCP map mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - Output - BCM_PORT_DSCP_MAP_NONE,   BCM_PORT_DSCP_MAP_ALL,
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */


int
_bcm_caladan3_g3p1_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode)
{
    int rv;
    soc_sbx_g3p1_lp_t lp;

        rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to read lp for port %d: %d (%s)\n"), 
                       port, rv, bcm_errmsg(rv)));
            return rv;
        }

        if(lp.usedscp ==1)
             *mode = BCM_PORT_DSCP_MAP_ALL;
         else
             *mode = BCM_PORT_DSCP_MAP_NONE;

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_port_dscp_map_mode_set
 * Description:
 *    Disables DSCP mapping to QOS for IPv4 packets.
 *     This function provides handle only to disable. To enable
 *     one needs to use the dscp_map_set function and that takes care
 *     of automatically enabling the dscp mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - FLAG - BCM_PORT_DSCP_MAP_NONE
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */

int
_bcm_caladan3_g3p1_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode)
{
    int rv;
    soc_sbx_g3p1_lp_t lp;

    if(mode != BCM_PORT_DSCP_MAP_NONE)
        return BCM_E_PARAM;
    
    rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read lp for port %d: %d (%s)\n"), 
                   port, rv, bcm_errmsg(rv)));
        return rv;
    }
    /* Set the usedscp for the lp to zero */
    
    
    
    lp.usedscp = 0;
    rv = soc_sbx_g3p1_lp_set(unit, port, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to write lp for port %d: %d (%s)\n"), 
                   port, rv, bcm_errmsg(rv)));
        return rv;
    }
    return rv;

}


/*
 *   Function
 *      _bcm_caladan3_g3p1_port_qosmap_set
 *   Purpose
 *      Set QoS mapping behaviour on a physical port
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_port_t gport = physical port
 *      (in) int ingrMap     = ingress map
 *      (in) int egrMap      = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     Little parameter checking is done here.
 */
int
_bcm_caladan3_g3p1_port_qosmap_set(int unit, bcm_port_t port,
                                 int ingrMap, int egrMap,
                                 uint32 ingFlags, uint32 egrFlags)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g3p1_lp_t       lp;
    soc_sbx_g3p1_pv2e_t     pv2e;
    int                     i;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "(%d,%d,%d) enter\n"),
                 port, ingrMap, egrMap));

    if (ingrMap >= 0) {
        bcm_vlan_t vid;
        uint32   logicalPort = 0, ltype = BCM_GPORT_INVALID;
        uint32 *fastLpi = sal_alloc(sizeof(uint32) * (BCM_VLAN_MAX + 1),
                            "fastLpi");
        if (fastLpi == NULL) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to alloc %d bytes for"
                                   " unit %d port temp data\n"),
                       sizeof(uint32) * (BCM_VLAN_MAX + 1),unit));
            return BCM_E_INTERNAL;
        }

        if (SAL_BOOT_BCMSIM) {
            for (i = 0; i <= 0xFFF; i++) {
                rv = SOC_SBX_G3P1_PV2E_GET(unit, port, i, &pv2e);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "failed to get pv2e for port/vid=%d/0x%x: %d (%s)\n"),
                               port, i, rv, bcm_errmsg(rv)));
                } else {
                    fastLpi[i] = pv2e.lpi;
                }

            }
        } else {
            rv = soc_sbx_g3p1_pv2e_lpi_fast_get(unit, 0, port,
                                                0xFFF, port,
                                                fastLpi, BCM_VLAN_MAX + 1);
        }
        if(BCM_SUCCESS(rv)) {
            /* If LPi != Port, its a logical port.
             * If the Logical port is associated with valid Gport, 
             * dont update QOS profile on the LP since it will overrite
             * logical port QOS.
             * If the logical port is not associated with any Gport
             * (i.e.) case where vlan statistics are enabled on TB port
             * update the QOS profile on the LP */
            for(vid=0; vid < BCM_VLAN_MAX + 1 && rv == BCM_E_NONE; vid++) {

                logicalPort = fastLpi[vid];

                if(logicalPort != 0) {
                    ltype = SBX_LPORT_TYPE(unit, logicalPort);
                                                                                           
                    if((ltype == BCM_GPORT_INVALID) || 
                       (ltype == BCM_GPORT_TYPE_NONE) ||
                       (ltype == BCM_GPORT_TYPE_LOCAL) ||
                       (ltype == BCM_GPORT_TYPE_MODPORT)) {

                        /* set QOS on this lp */
                        rv = soc_sbx_g3p1_lp_get(unit, logicalPort, &lp);
                        if (BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_PORT,
                                      (BSL_META_U(unit,
                                                  "failed to read lp for port %d: %d (%s)\n"), 
                                       port, rv, bcm_errmsg(rv)));
                        } else {
                            lp.qos = ingrMap;
                            lp.usedscp = (ingFlags & BCM_QOS_MAP_L3)?1:0;
                            lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS)?1:0;

                            rv = soc_sbx_g3p1_lp_set(unit, logicalPort, &lp);
                            if (BCM_FAILURE(rv)) {
                                LOG_ERROR(BSL_LS_BCM_PORT,
                                          (BSL_META_U(unit,
                                                      "failed to write lp for port %d: %d (%s)\n"), 
                                           port, rv, bcm_errmsg(rv)));
                            }                    
                        }    
                    }
                } 
            } /* for loop end */
            
            if(BCM_SUCCESS(rv)) {

                rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "failed to read lp for port %d: %d (%s)\n"), 
                               port, rv, bcm_errmsg(rv)));

                } else {
                    lp.qos = ingrMap;
                    lp.usedscp = (ingFlags & BCM_QOS_MAP_L3)?1:0;
                    lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS)?1:0;
                    rv = soc_sbx_g3p1_lp_set(unit, port, &lp);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_PORT,
                                  (BSL_META_U(unit,
                                              "failed to write lp for port %d: %d (%s)\n"), 
                                   port, rv, bcm_errmsg(rv)));
                    }       
                }       
            }   

            if(fastLpi) {
                sal_free(fastLpi);
            }

            if(BCM_FAILURE(rv)) {                    
                return rv;
            }
        } else {
            if(fastLpi) {
                sal_free(fastLpi);
            }
        }
    }

    rv = _bcm_caladan3_g3p1_port_egr_remark_set(unit, port, egrMap, 
                                             !!(egrFlags & BCM_QOS_MAP_L3));
    
    return rv;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_port_qosmap_get
 *   Purpose
 *      Get QoS mapping on a physical port
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_port_t gport = physical port
 *      (out) int ingrMap     = ingress map
 *      (out) int egrMap      = egress map
 *   Returns
 *      BCM_E_*
 */
int
_bcm_caladan3_g3p1_port_qosmap_get(int unit,bcm_port_t  port, 
                                 int *ing_map, int *egr_map,
                                 uint32 *ing_flags, uint32 *egr_flags)
{
    int                  rv = BCM_E_NONE;
    uint32               etei;
    soc_sbx_g3p1_ete_t   ete;
    soc_sbx_g3p1_lp_t    lp;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 port));

    /* Get the ingress qos map
     */
    rv = soc_sbx_g3p1_lp_get(unit, port, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read lp for port %d: %d (%s)\n"),
                   port, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    *ing_map = lp.qos;
    *ing_flags  = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2;
    if (lp.usedscp) {
        *ing_flags |= BCM_QOS_MAP_L3;
    }
    if (lp.useexp) {
        *ing_flags |=  BCM_QOS_MAP_MPLS;   
    }

    /* Get the Egress qos map
     */
    etei = SOC_SBX_PORT_ETE(unit, port);
    rv = soc_sbx_g3p1_ete_get(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    *egr_map = ete.remark;
    *egr_flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
    if (ete.dscpremark) {
        *egr_flags |= BCM_QOS_MAP_L3;
    }

    return rv;
}

/*
 *   Function
 *      _bcm_caladan3_g3p1_port_vlan_qosmap_set
 *   Purpose
 *      Set the qos profile and remark index for the given port,vid
 *   Parameters
 *      (in)  unit       - BCM device number
 *      (in)  port       - The port to change
 *      (in)  vid        - The vid to change
 *      (in)  ing_idx    - The new qos profile
 *      (in)  egr_idx    - The new remark index
 *      (in)  ing_flags  - Flags specifing qos attributes (BCM_QOS_MAP_*)
 *      (in)  egr_flags  - Flags specifing qos attributes (BCM_QOS_MAP_*)
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 *     Modifies the globally configured Port ETE.
 *
 *     C3 Logical ports are allocated when:
 *        A new qos profile is added, and the logical port,vid shares the 
 *        physical port's logical port.
 *
 *     C3 Logical ports are freed when:
 *        The qos profile is removed, AND the logical port,vid matches
 *        the physical port's logical port OR the default (empty) logical port.
 *
 *      Else - the logical port's qos is set to the default profile.
 */
int
_bcm_caladan3_g3p1_port_vlan_qosmap_set(int unit, bcm_port_t port,
                                      bcm_vlan_t vid, 
                                      int ing_idx, int egr_idx,
                                      uint32 ing_flags, uint32 egr_flags)
{
    int                    rv = BCM_E_NONE;
    soc_sbx_g3p1_lp_t      lp, port_lp, default_lp;
    soc_sbx_g3p1_pv2e_t    pv2e;
    uint32                 lpi, alloc_lp;
    
    alloc_lp = FALSE;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "(%d, 0x%03x, %d, %d) enter\n"),
                 port, vid, ing_idx, egr_idx));

    if (ing_idx >= 0) {

        rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &pv2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to read pv2e[%d,0x%03x]:"
                                   " %d (%s)\n"), 
                       port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = soc_sbx_g3p1_lp_get(unit, port, &port_lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to port lp[%d]: %d (%s)\n"),
                       port, rv, bcm_errmsg(rv)));
            return rv;
        }

        /* If the port,vid uses the port's logical port, then one must be 
         * allocated to support the new non-default qos mapping.
         *
         * If the caller is removing the qos mapping, AND the lp is default, or
         * is equivilent to the port lp, then free the logical port.
         * */
        lpi = pv2e.lpi ? pv2e.lpi : port;

        if (ing_idx == 0 && lpi != port) {
            /* ing_idx == 0 -> removing the qos profile,
             * conditionally free the logical port
             */
            soc_sbx_g3p1_lp_t_init(&default_lp);

            rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "failed to read lp for port,vid"
                                       " %d,0x%03x: %d (%s)\n"),
                           port, vid, rv, bcm_errmsg(rv)));
                return rv;
            }
    
            /* compare everything, except qos params */
            lp.qos     = port_lp.qos     = 0;
            lp.useexp  = port_lp.useexp  = 0;
            lp.usedscp = port_lp.usedscp = 0;
            if ((sal_memcmp(&lp, &port_lp, sizeof(lp)) == 0) ||
                (sal_memcmp(&lp, &default_lp, sizeof(lp)) == 0))
            {

                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "Freeing Logical Port 0x%0x\n"),
                             lpi));
                rv = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_LPORT,
                                            1, &lpi, 0);
                pv2e.lpi = 0;

            } else {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "Logical Port %d is non-default, keeping.\n"), 
                             lpi));
            }
            
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "failed to free logical port 0x%x"
                                       ": %d (%s)\n"),
                           lpi, rv, bcm_errmsg(rv)));  
                return rv;
            }

        } else if (ing_idx != 0 && lpi == port) {
            
            /* ing_idx != 0 -> setting a new qos profile, 
             * allocate a logical port
             */
            rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_LPORT,
                                         1, &lpi, 0);
            
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "failed to allocate logical port"
                                       ": %d (%s)\n"),
                           rv, bcm_errmsg(rv)));
                return rv;
            }

            /* inherit port lpi settings */
            sal_memcpy (&lp, &port_lp, sizeof(lp));

            pv2e.lpi = lpi;
            alloc_lp = TRUE;
            
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "Allocated Logical Port 0x%0x\n"), 
                         lpi));
        }  /* ing_idx != 0 && lpi == port */
        
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Modifying Logical Port 0x%0x\n"),
                     lpi));

        if (alloc_lp == FALSE) {
            rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "failed to read lp for port,vid"
                                       " %d,0x%03x: %d (%s)\n"),
                           port, vid, rv, bcm_errmsg(rv)));
                return rv;
            }
        }
        
        /* set QOS on this lp */
        lp.qos     = ing_idx;
        lp.usedscp = !!(ing_flags & BCM_QOS_MAP_L3);
        lp.useexp  = !!(ing_flags & BCM_QOS_MAP_MPLS);
        
        rv = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to write lp[0x%x] for port,vid"
                                   " %d,0x%03x: %d (%s)\n"),
                       lpi, port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
        
        rv = SOC_SBX_G3P1_PV2E_SET(unit, port, vid, &pv2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "failed to read pv2e[%d,0x%03x]:"
                                   " %d (%s)\n"), 
                       port, vid, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* Currently, remarking is available per-port, not per-port,vid */
    rv = _bcm_caladan3_g3p1_port_egr_remark_set(unit, port, egr_idx,
                                              !!(egr_flags & BCM_QOS_MAP_L3));

    return rv;
}


/*
 *   Function
 *      _bcm_caladan3_g3p1_port_vlan_qosmap_get
 *   Purpose
 *      Set the qos profile and remark index for the given port,vid
 *   Parameters
 *      (in)  unit       - BCM device number
 *      (in)  port       - The port to get
 *      (in)  vid        - The vid to get
 *      (out) ing_idx    - qos profile
 *      (out) egr_idx    - remark index
 *      (out) ing_flags  - Flags specifing qos attributes (BCM_QOS_MAP_*)
 *      (out) egr_flags  - Flags specifing qos attributes (BCM_QOS_MAP_*)
 *   Notes:
 *     Assumes params are valid, and port lock has been taken
 */
int
_bcm_caladan3_g3p1_port_vlan_qosmap_get(int unit, bcm_port_t port,
                                      bcm_vlan_t vid, 
                                      int *ing_idx, int *egr_idx,
                                      uint32 *ing_flags, uint32 *egr_flags)
{
    int                    rv = BCM_E_NONE;
    soc_sbx_g3p1_lp_t      lp;
    soc_sbx_g3p1_pv2e_t    pv2e;
    soc_sbx_g3p1_ete_t     ete;
    uint32                 lpi, etei;
    
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "(%d, 0x%03x) enter\n"),
                 port, vid));

    rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &pv2e);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read pv2e[%d,0x%03x]: %d (%s)\n"),
                   port, vid, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    lpi = pv2e.lpi ? pv2e.lpi : port;
    rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read lp[%d]: %d (%s)\n"),
                   lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    *ing_idx    = lp.qos;
    *ing_flags  = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2;
    if (lp.usedscp) {
        *ing_flags |= BCM_QOS_MAP_L3;
    }
    if (lp.useexp) {
        *ing_flags |=  BCM_QOS_MAP_MPLS;   
    }


    /* Currently, remarking is available per-port, not per-port,vid */
    etei = SOC_SBX_PORT_ETE(unit, port);
    rv = soc_sbx_g3p1_ete_get(unit, etei, &ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "failed to read ete 0x%04x: %d (%s)\n"), 
                   etei, rv, bcm_errmsg(rv)));
        return rv;
    }

    *egr_idx   = ete.remark;
    *egr_flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
    if (ete.dscpremark) {
        *egr_flags |= BCM_QOS_MAP_L3;
    }


    return rv;
}

#endif /* BCM_CALADAN3_G3P1_SUPPORT */
