/* $SDK/src/bcm/sbx/caladan3/wb_db_mpls.c */
/*
 * $Id: wb_db_mpls.c,v Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: MPLS APIs
 *
 * Purpose:
 *     Warm boot support for MPLS API for Caladan3 Packet Processor devices
 */

#include <shared/bsl.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <sal/core/sync.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <soc/sbx/wb_db_cmn.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_mpls.h>
#include <shared/hash_tbl.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_MPLS

/*
 *  Set _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time the MPLS backing store is written.
 */
#define _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING FALSE

/*
 *  Set _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time MPLS backing store is read.
 */
#define _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING FALSE


/*
 *  Maximum number of vpns that can be configured 
 *  Maximum number of L2 VPN including VPLS --> 16K
 *  Maximum number of VPWS VPN              --> 16K
 */
#define WB_MPLS_MAX_VPNS          (32 * 1024)

/*
 *  Maximum number of mpls ports 
 *  VPLS (AC & PW) - 16K
 *  VPWS (AC & PW) - 32K
 *  Maximum number of failover ports?
 */
#define WB_MPLS_MAX_VPN_SAPS      (48 * 1024)
/*
 *  Maximum number of LSPs  
 *    32K
 */
#define WB_MPLS_MAX_TUNNELS       (32 * 1024)


/*
 *  Externs for MPLS
 */
extern void _bcm_caladan3_mpls_debug(int unit);
extern void _bcm_caladan3_mpls_vpnc_dump(int unit, _caladan3_vpn_control_t *vpnc);
extern void _bcm_caladan3_mpls_sw_dump(int unit);

extern void _bcm_caladan3_mpls_switch_info_db_get(int unit,
                                     shr_htb_hash_table_t *switch_info_db);
extern int
_bcm_caladan3_mpls_switch_info_db_add(int unit, bcm_mpls_tunnel_switch_t *switch_info);
extern int
_bcm_caladan3_mpls_gport_info_db_add(int unit, _caladan3_vpn_sap_t *vpn_sap);
extern int
_bcm_caladan3_mpls_trunk_assoc_gport_add(int unit, _caladan3_vpn_sap_t *vpn_sap);

extern int
_bcm_caladan3_mpls_switch_ft_lp_get(_caladan3_l3_fe_instance_t *l3_fe,
                                    bcm_mpls_tunnel_switch_t   *info,
                                    uint32                     *ftidx,
                                    uint32                     *lpidx);


int mpls_switch_info_sync_count;
/*
extern 
_caladan3_l3_fe_instance_t * bcm_sbx_caladan3_l3_cntl_ptr_get(int unit);
*/

/*
 *  Locals for MPLS
 */
static bcm_caladan3_wb_mpls_state_scache_info_t *_bcm_caladan3_wb_mpls_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for MPLS
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_mpls_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_mpls_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_mpls_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Warmboot APIs implementation for MPLS
 */
STATIC int
_bcm_caladan3_wb_mpls_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_mpls_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_mpls_state_scache_info_t), "Scache for MPLS warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_mpls_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_mpls_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC void
_bcm_caladan3_wb_mpls_vpn_sap_layout_init(int unit, int version, unsigned int *scache_len)
{
    SBX_WB_DB_LAYOUT_INIT(_caladan3_ohi_t    , 1, _caladan3_vpn_sap_t.vc_ohi);
    SBX_WB_DB_LAYOUT_INIT(_caladan3_ete_idx_t, 1, _caladan3_vpn_sap_t.vc_ete_hw_idx);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.vc_ete_hw_stat_idx);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.vc_alloced_ue);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.vc_inuse_ue);
    SBX_WB_DB_LAYOUT_INIT(uint8              , 1, _caladan3_vpn_sap_t.vc_res_alloced);
    /* Union FTE or IPMC */
    SBX_WB_DB_LAYOUT_INIT(_caladan3_vc_ete_fte_t , 1, *(_caladan3_vpn_sap_t.u.vc_fte));
    SBX_WB_DB_LAYOUT_INIT(bcm_mpls_port_t    , 1, _caladan3_vpn_sap_t.vc_mpls_port);
    SBX_WB_DB_LAYOUT_INIT(bcm_gport_t        , 1, _caladan3_vpn_sap_t.vc_mpls_port_id);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.logicalPort);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_label);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_label_exp);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_label_ttl);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_label2);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_ing_label);
    SBX_WB_DB_LAYOUT_INIT(uint32             , 1, _caladan3_vpn_sap_t.mpls_psn_ing_label2);

}

STATIC void
_bcm_caladan3_wb_mpls_vpn_control_layout_init(int unit, int version, unsigned int *scache_len)
{
    SBX_WB_DB_LAYOUT_INIT(bcm_vpn_t, 1, _caladan3_vpn_control_t.vpn_id);
    SBX_WB_DB_LAYOUT_INIT(int      , 1, _caladan3_vpn_control_t.vpn_vrf);
    SBX_WB_DB_LAYOUT_INIT(int      , 1, _caladan3_vpn_control_t.vpn_bc_mcg);
    SBX_WB_DB_LAYOUT_INIT(uint32   , 1, _caladan3_vpn_control_t.vpn_flags);
    SBX_WB_DB_LAYOUT_INIT(uint32   , 1, _caladan3_vpn_control_t.vpls_color);
}

STATIC int
_bcm_caladan3_wb_mpls_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    unsigned int vpn_control_len = 0;
    unsigned int vpn_sap_len     = 0;
    int          vpn_control_count   = WB_MPLS_MAX_VPNS;
    int          vpn_sap_count       = WB_MPLS_MAX_VPN_SAPS;
    int          tunnel_switch_count = WB_MPLS_MAX_TUNNELS;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_MPLS_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            /* Layout for scache length and offset calculation */
            SBX_WB_DB_LAYOUT_INIT(int   , 1, _bcm_caladan3_get_l3_instance_from_unit(unit)->fe_vpn_count);
            _bcm_caladan3_wb_mpls_vpn_control_layout_init(unit,
                                                          version,
                                                         &vpn_control_len);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
            LOG_CLI((BSL_META_U(unit,
                                "Vpn control size :%d\n"),
                     vpn_control_len));
#endif
            *scache_len += (vpn_control_len * vpn_control_count);

            _bcm_caladan3_wb_mpls_vpn_sap_layout_init(unit,
                                                      version,
                                                     &vpn_sap_len);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
            LOG_CLI((BSL_META_U(unit,
                                "Vpn SAP size :%d\n"),
                     vpn_sap_len));
#endif
            *scache_len += (vpn_sap_len * vpn_sap_count);

            /* Store tunnel switch database */
            SBX_WB_DB_LAYOUT_INIT(uint32, 1, tunnel_switch_count);
            SBX_WB_DB_LAYOUT_INIT(bcm_mpls_tunnel_switch_t,
                               tunnel_switch_count, bcm_mpls_tunnel_switch_t);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
            LOG_CLI((BSL_META_U(unit,
                                "Tunnel switch size :%d\n"),
                     sizeof(bcm_mpls_tunnel_switch_t)));
#endif


            /* Update scache length */
            SBX_SCACHE_INFO_PTR(unit)->scache_len = *scache_len;
            break;
        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}


STATIC int
_bcm_caladan3_wb_mpls_switch_info_sync(int unit, shr_htb_key_t key, shr_htb_data_t data)
{
    bcm_mpls_tunnel_switch_t * info;
    info = (bcm_mpls_tunnel_switch_t *) data;
    SBX_WB_DB_SYNC_VARIABLE(bcm_mpls_tunnel_switch_t, 1, *info);
    mpls_switch_info_sync_count++;
    return BCM_E_NONE;
}

STATIC void
_bcm_caladan3_wb_mpls_vpn_sap_sync(int unit, _caladan3_vpn_sap_t *vpn_sap)
{
    int i;
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_ohi_t    , 1, vpn_sap->vc_ohi);
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_ete_idx_t, 1, vpn_sap->vc_ete_hw_idx);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->vc_ete_hw_stat_idx);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->vc_alloced_ue);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->vc_inuse_ue);
    SBX_WB_DB_SYNC_VARIABLE(uint8              , 1, vpn_sap->vc_res_alloced);
    /* MPLS always uses vc_fte */
    for (i = 0; i < vpn_sap->vc_inuse_ue; i++) {
        SBX_WB_DB_SYNC_VARIABLE(_caladan3_vc_ete_fte_t , 1, vpn_sap->u.vc_fte[i]);
    }
                        
    SBX_WB_DB_SYNC_VARIABLE(bcm_mpls_port_t    , 1, vpn_sap->vc_mpls_port);
    SBX_WB_DB_SYNC_VARIABLE(bcm_gport_t        , 1, vpn_sap->vc_mpls_port_id);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->logicalPort);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_label);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_label_exp);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_label_ttl);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_label2);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_ing_label);
    SBX_WB_DB_SYNC_VARIABLE(uint32             , 1, vpn_sap->mpls_psn_ing_label2);
}

STATIC void
_bcm_caladan3_wb_mpls_vpn_control_sync(int unit, _caladan3_vpn_control_t *vpnc)
{
    SBX_WB_DB_SYNC_VARIABLE(bcm_vpn_t , 1, vpnc->vpn_id);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, vpnc->vpn_vrf);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, vpnc->vpn_bc_mcg);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, vpnc->vpn_flags);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, vpnc->vpls_color);
}


int
bcm_caladan3_wb_mpls_state_sync (int unit)
{
    int version   = 0;
    uint8 *scache_ptr_orig = NULL;
    int vpn_count = 0;
    int sap_count = 0;
    _caladan3_vpn_control_t    *vpnc    = NULL;
    _caladan3_l3_fe_instance_t *l3_fe   = NULL;
    _caladan3_vpn_sap_t        *vpn_sap = NULL;
    int hidx;
    uint8 *vpn_count_ptr    = NULL;
    uint8 *sap_count_ptr    = NULL;
    uint8 *switch_count_ptr = NULL;
    shr_htb_hash_table_t mpls_switch_info_db;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (NULL == l3_fe) {
        return BCM_E_UNIT;
    }
    switch(version)
    {
        case BCM_CALADAN3_WB_MPLS_VERSION_1_0:
            /* Sync state to the scache */
            SBX_WB_DB_GET_SCACHE_PTR(vpn_count_ptr);
            SBX_WB_DB_MOVE_SCACHE_PTR(int, 1);

            for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
                _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc) {
                    _bcm_caladan3_wb_mpls_vpn_control_sync(unit, vpnc);

                    SBX_WB_DB_GET_SCACHE_PTR(sap_count_ptr);
                    SBX_WB_DB_MOVE_SCACHE_PTR(int, 1);

                    sap_count = 0;
                    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                        _bcm_caladan3_wb_mpls_vpn_sap_sync(unit, vpn_sap);
                         sap_count++;
                    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

                    SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(int, 1, sap_count,
                                                                 sap_count_ptr);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
                    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                                (BSL_META_U(unit,
                                            "MPLS WB: sap_count %d \n"),
                                 sap_count));
#endif

                    vpn_count++;
                } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_Fe, hidx, vpnc);
            }
            SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(int, 1, vpn_count, vpn_count_ptr);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "MPLS WB sync: vpn_count %d \n"),
                         vpn_count));
#endif

           /* Sync up all mpls_tunnel_info */
            _bcm_caladan3_mpls_switch_info_db_get(unit, &mpls_switch_info_db);
            SBX_WB_DB_GET_SCACHE_PTR(switch_count_ptr);
            SBX_WB_DB_MOVE_SCACHE_PTR(int, 1);
            mpls_switch_info_sync_count = 0;
            shr_htb_iterate(unit, mpls_switch_info_db,
                                        _bcm_caladan3_wb_mpls_switch_info_sync);
            SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(int, 1, 
                                 mpls_switch_info_sync_count, switch_count_ptr);
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "MPLS WB sync: switch_info_count %d \n"),
                         mpls_switch_info_sync_count));
#endif

            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_MPLS | BSL_INFO)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING
                _bcm_caladan3_mpls_sw_dump(unit);
#endif
            }
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_caladan3_wb_mpls_vpn_control_restore(_caladan3_l3_fe_instance_t *l3_fe, 
                                          _caladan3_vpn_control_t **vpnc_ref)
{
    int hidx;
    int unit;
    uint32 vrf, vpn_id;
    _caladan3_vpn_control_t *vpnc;
    _sbx_caladan3_usr_res_types_t res_type;
    int status = BCM_E_NONE;

    unit = l3_fe->fe_unit;
    vpnc = sal_alloc(sizeof(_caladan3_vpn_control_t), "WB MPLS vpncb");
    if (vpnc == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
            (BSL_META_U(unit, "MPLS WB vpn restore: vpnc mem alloc failed\n")));
        return BCM_E_MEMORY;
    }
    DQ_INIT(&vpnc->vpn_sap_head);
    DQ_INIT(&vpnc->vpn_fe_link);
    SBX_WB_DB_RESTORE_VARIABLE(bcm_vpn_t   , 1, vpnc->vpn_id);
    SBX_WB_DB_RESTORE_VARIABLE(int         , 1, vpnc->vpn_vrf);
    SBX_WB_DB_RESTORE_VARIABLE(int         , 1, vpnc->vpn_bc_mcg);
    SBX_WB_DB_RESTORE_VARIABLE(int         , 1, vpnc->vpn_flags);
    SBX_WB_DB_RESTORE_VARIABLE(int         , 1, vpnc->vpls_color);

    hidx = _CALADAN3_GET_MPLS_VPN_HASH(vpnc->vpn_id);
    DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx], &vpnc->vpn_fe_link);

    if (_BCM_VRF_VALID(vpnc->vpn_vrf)) {
        l3_fe->fe_vpn_by_vrf[vpnc->vpn_vrf] = vpnc;
    }

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_L3) &&
        (vpnc->vpn_vrf != BCM_L3_VRF_DEFAULT)) {

        vrf = vpnc->vpn_vrf;
        status = _sbx_caladan3_resource_alloc(unit, 
                                     SBX_CALADAN3_USR_RES_VRF,
                                     1,
                                    &vrf,
                                    _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                 "MPLS WB vpn restore: VRF alloc failed\n")));
            sal_free(vpnc);
            return status;
        }
    }

    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        res_type = SBX_CALADAN3_USR_RES_LINE_VSI;
    } else {
        res_type = SBX_CALADAN3_USR_RES_VSI;
    }
    vpn_id = vpnc->vpn_id;
    if (vpn_id != l3_fe->fe_vsi_default_vrf) {
        status = _sbx_caladan3_resource_alloc(unit, 
                                     res_type,
                                     1,
                                    &vpn_id,
                                    _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                   (BSL_META_U(unit,
                          "MPLS WB vpn restore: VSI/LINE VSI alloc failed\n")));
           sal_free(vpnc);
           return status;
        }
    }

    *vpnc_ref = vpnc;
 
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
    _bcm_caladan3_mpls_vpnc_dump(unit, vpnc);
#endif

    return BCM_E_NONE;
}


STATIC int
_bcm_caladan3_wb_mpls_vpn_sap_restore(_caladan3_l3_fe_instance_t *l3_fe,
                                      _caladan3_vpn_sap_t *vpn_sap)
{
    int k;
    uint8  is_mplstp_vpwsuni = 0;
    _sbx_caladan3_usr_res_types_t res_type = SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT;
    _caladan3_fte_idx_t  gport_fte, tunnel_fte;
    _caladan3_l3_ete_t *tunnel_sw_ete;
    int exit_modid = -1, exit_port;
    uint8 is_trunk = 0;
    int unit;
    int hidx;
    soc_sbx_g3p1_ft_t g3p1_fte_e;
    _caladan3_ohi_t ohi;
    int status = BCM_E_NONE;
#ifdef PLATFORM_LABEL
    int is_failover, is_same_label;
#endif

    unit = l3_fe->fe_unit;

    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_ohi_t, 1, vpn_sap->vc_ohi);
    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_ete_idx_t, 1, vpn_sap->vc_ete_hw_idx);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, vpn_sap->vc_ete_hw_stat_idx);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, vpn_sap->vc_alloced_ue);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, vpn_sap->vc_inuse_ue);
    SBX_WB_DB_RESTORE_VARIABLE(uint8 , 1, vpn_sap->vc_res_alloced);
    if (vpn_sap->vc_alloced_ue) {
        vpn_sap->u.vc_fte =
              sal_alloc(sizeof(_caladan3_vc_ete_fte_t) * vpn_sap->vc_alloced_ue,
                        "WB MPLS vpn-sap-ue");
        if (vpn_sap->u.vc_fte == NULL) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                "MPLS WB vpn_sap res: mem alloc failed\n")));
            return BCM_E_MEMORY;
        }
        for (k = 0; k < vpn_sap->vc_inuse_ue; k++) {
            SBX_WB_DB_RESTORE_VARIABLE(_caladan3_vc_ete_fte_t, 1, vpn_sap->u.vc_fte[k]);
        }
    }
    SBX_WB_DB_RESTORE_VARIABLE(bcm_mpls_port_t, 1, vpn_sap->vc_mpls_port);
    SBX_WB_DB_RESTORE_VARIABLE(bcm_gport_t, 1, vpn_sap->vc_mpls_port_id);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->logicalPort);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_label);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_label_exp);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_label_ttl);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_label2);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_ing_label);
    SBX_WB_DB_RESTORE_VARIABLE(uint32     , 1, vpn_sap->mpls_psn_ing_label2);

    if ((vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) &&
         !(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_NETWORK) &&
          (vpn_sap->vc_mpls_port.criteria != BCM_MPLS_PORT_MATCH_LABEL)) {
        is_mplstp_vpwsuni = 1;
    }

    if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        res_type = SBX_CALADAN3_USR_RES_FTE_MPLS;
    } else {
        res_type = SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT;
    }
    if (is_mplstp_vpwsuni) {
        res_type = SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT;
    }
    if (vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER) {
        res_type = SBX_CALADAN3_USR_RES_FTE_MPLS;
    }
    gport_fte.fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);

    /* Alloc ftidx & Add the ftidx to l3_fe fe_fteidx2_fte hash list */
    status = _bcm_caladan3_alloc_l3_or_mpls_fte(l3_fe,
                                       BCM_L3_WITH_ID,
                                       0,
                                       res_type,
                                       &gport_fte.fte_idx);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                 (BSL_META_U(unit,
                             "MPLS WB vpn_sap res: fte alloc failed\n")));
        if (vpn_sap->u.vc_fte) {
            sal_free(vpn_sap->u.vc_fte);
        }
        return status;
    }
/*   Done through alloc_l3_or_mpls_fte 
    _sbx_caladan3_resource_alloc(unit,
                                 res_type,
                                 1,
                                 &gport_fte.fte_idx,
                                 _SBX_CALADAN3_RES_FLAGS_RESERVE);
*/
    
    if (BCM_GPORT_IS_TRUNK(vpn_sap->vc_mpls_port.port)) {
        is_trunk = 1;
    } else {
        status = _bcm_caladan3_mpls_gport_get_mod_port(unit,
                                              vpn_sap->vc_mpls_port.port,
                                             &exit_modid,
                                             &exit_port);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                "gport_get_mod_port failed gport[0x%x]\n"),
                      vpn_sap->vc_mpls_port.port));
            if (vpn_sap->u.vc_fte) {
                sal_free(vpn_sap->u.vc_fte);
            }
            return status;
        }
    }
    if (vpn_sap->vc_res_alloced) {
        status = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_OHI,
                                         1,
                                        &vpn_sap->vc_ohi.ohi,
                                         _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                "OHI[0x%x] resource alloc failed [%d]\n"),
                      vpn_sap->vc_ohi.ohi, status));
            if (vpn_sap->u.vc_fte) {
                sal_free(vpn_sap->u.vc_fte);
            }
            return status;
        }

        status = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_ETE,
                                         1,
                                        &vpn_sap->vc_ete_hw_idx.ete_idx,
                                         _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                 "ete[0x%x] resource alloc failed [%d]\n"),
                          vpn_sap->vc_ete_hw_idx.ete_idx, status));
            if (vpn_sap->u.vc_fte) {
                sal_free(vpn_sap->u.vc_fte);
            }
            return status;
        }

        hidx = _CALADAN3_GET_OHI2ETE_HASH_IDX(vpn_sap->vc_ohi.ohi);
        DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx], &vpn_sap->vc_ohi_link);

        if (vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_LABEL) {
        /* Link match label ports to tunnel interface */
            tunnel_fte.fte_idx = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(
                                    vpn_sap->vc_mpls_port.egress_tunnel_if);
            status = soc_sbx_g3p1_ft_get(unit,
                                         tunnel_fte.fte_idx,
                                        &g3p1_fte_e);

            if (BCM_FAILURE(status)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                LOG_WARN(BSL_LS_BCM_MPLS,
                         (BSL_META_U(unit,
                                     "MPLS WB: Couldn't read ft :0x%x\n"),
                              tunnel_fte.fte_idx));
#endif
            }

            ohi.ohi = g3p1_fte_e.oi;

            status = _bcm_caladan3_l3_sw_ete_find_by_ohi(l3_fe,
                                                        &ohi,
                                                        &tunnel_sw_ete);
            if (status == BCM_E_NONE) {
                DQ_INSERT_HEAD(&tunnel_sw_ete->l3_vc_ete_head,
                                   &vpn_sap->vc_mpls_ete_link);   
            } else {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                LOG_WARN(BSL_LS_BCM_MPLS,
                         (BSL_META_U(unit,
                                     "MPLS WB: Couldn't find sw_ete "
                                      "OHI:0x%x\n"), ohi.ohi));
#endif
            }
        }
        if (vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT      ||
            vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN ||
            vpn_sap->vc_mpls_port.criteria == 
                                      BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED) {

            res_type = SBX_CALADAN3_USR_RES_LPORT;

            if (((vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER) &&
                 (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)) ||
                 (is_mplstp_vpwsuni)) {
                res_type = SBX_CALADAN3_USR_RES_VPWS_UNI_LPORT;
            }
            if (vpn_sap->logicalPort > 0) {
                status = _sbx_caladan3_resource_alloc(unit,
                                               res_type,
                                               1,
                                               &vpn_sap->logicalPort,
                                               _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (BCM_FAILURE(status)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                LOG_WARN(BSL_LS_BCM_MPLS,
                         (BSL_META_U(unit,
                                     "MPLS WB: Couldn't allocate "
                                      "Logical port\n")));
#endif
                    return status;
                }
            }
        } else {

#ifdef PLATFORM_LABEL
            is_failover = vpn_sap->mpls_port.flags & BCM_MPLS_PORT_FAILOVER;
            status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                                         vpn_sap->vpnc,
                                                        &vpn_sap_vc_mpls_port,
                                                         vpn_sap_temp,
                                                         (!is_failover));
            if (BCM_FAILURE(status))
#endif
            {
                status = _sbx_caladan3_resource_alloc(unit,
                                               SBX_CALADAN3_USR_RES_MPLS_LPORT,
                                                1,
                                               &vpn_sap->logicalPort,
                                               _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (BCM_FAILURE(status)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                    LOG_WARN(BSL_LS_BCM_MPLS,
                             (BSL_META_U(unit,
                                         "MPLS WB: Couldn't allocate "
                                          "MPLS Lport\n")));
#endif
                }
            }
        }
	
    }
    if (is_trunk) {
      /* Add gport to trunk info list */
        status = _bcm_caladan3_mpls_trunk_assoc_gport_add(unit, vpn_sap);
        if (BCM_FAILURE(status)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
            LOG_WARN(BSL_LS_BCM_MPLS,
                     (BSL_META_U(unit,
                                 "MPLS WB: trunk db - gport add failed\n")));
#endif
        }
    }
    DQ_INSERT_TAIL(&vpn_sap->vc_vpnc->vpn_sap_head, &vpn_sap->vc_vpn_sap_link);
    DQ_INIT(&vpn_sap->vc_mpls_ete_link);
    DQ_INIT(&vpn_sap->vc_ohi_link);
    status = _bcm_caladan3_mpls_gport_info_db_add(unit, vpn_sap);
    if (BCM_FAILURE(status)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(unit,
                             "MPLS WB: Adding gport to gport db failed\n")));
#endif
    }
    return status;
}


STATIC int
_bcm_caladan3_wb_mpls_state_restore (int unit)
{
    int version      = 0;
    int vpn_count    = 0;
    int sap_count    = 0;
    int switch_count = 0;
    int i,j;
    uint32 ftidx;
    uint32 lpidx;
    _caladan3_vpn_control_t    *vpnc = NULL;
    _caladan3_vpn_sap_t        *vpn_sap;
    bcm_mpls_tunnel_switch_t   *switch_info;
    bcm_mpls_tunnel_switch_t    switch_info_temp;
    _caladan3_l3_fe_instance_t *l3_fe;

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized - unit %d\n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (NULL == l3_fe) {
        return BCM_E_UNIT;
    }
    switch(version)
    {
        case BCM_CALADAN3_WB_MPLS_VERSION_1_0:

            /* Restore state from scache */
            SBX_WB_DB_RESTORE_VARIABLE(int   , 1, vpn_count);
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "MPLS Warm boot restore: vpn_count %d\n"),
                         vpn_count));
#endif

            for (i = 0; i < vpn_count; i++) {

                _rv = _bcm_caladan3_wb_mpls_vpn_control_restore(l3_fe, &vpnc);
                if (BCM_FAILURE(_rv)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                             (BSL_META_U(unit,
                                        "MPLS WB Restore: vpn_control_restore"
                              " failed:%d\n"), _rv));
                    return _rv;
                }

                SBX_WB_DB_RESTORE_VARIABLE(int, 1, sap_count);

#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                LOG_VERBOSE(BSL_LS_BCM_MPLS,
                            (BSL_META_U(unit,
                                        "MPLS Warm boot restore: sap_count %d\n"),
                             sap_count));
#endif
                for (j = 0; j < sap_count; j++) {
                    vpn_sap = sal_alloc(sizeof(_caladan3_vpn_sap_t),
                                                     "WB MPLS vpn-sap");
                    vpn_sap->vc_vpnc = vpnc;
                    _rv = _bcm_caladan3_wb_mpls_vpn_sap_restore(l3_fe, vpn_sap);
                    if (BCM_FAILURE(_rv)) {
                        sal_free(vpn_sap);
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                 (BSL_META_U(unit,
                                            "MPLS WB Restore: vpn_sap_restore"
                                 " failed:%d\n"), _rv));
                        return _rv;
                    }
                }
            }

            SBX_WB_DB_RESTORE_VARIABLE(int, 1, switch_count);
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "MPLS WB Restore Switch count:%d\n"),
                         switch_count));
#endif

            for (i = 0; i < switch_count; i++) {
                switch_info = (bcm_mpls_tunnel_switch_t *)
                                 sal_alloc(sizeof(bcm_mpls_tunnel_switch_t),
                                               "WB MPLS-tunnel-switch-info");
                if (switch_info) {
                    SBX_WB_DB_RESTORE_VARIABLE(bcm_mpls_tunnel_switch_t, 1,
                                                           switch_info_temp);
                    *switch_info = switch_info_temp;
#ifdef USE_TUNNEL_ID
                    _rv = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                          SBX_CALADAN3_USR_RES_TUNNEL_ID,
                                                          1,
                                                          &switch_info->tunnel_id,
                                                          _SBX_CALADAN3_RES_FLAGS_RESERVE);
                    if (BCM_FAILURE(_rv)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                            (BSL_META_U(unit,
                                "MPLS WB Restore: Tunnel Id Alloc failed:%d\n"),
                             _rv));
                        return _rv;
                    }
#endif

                    /* Get the ftidx and reserve 
                       If the Operation is POP, we wont have ftidx
                       If the Operation is SWAP, we have ftidx
                       Both cases we have LP.
                       1. Read label2e 
                       2. Reserve ftidx
                       3. Reserve lpi
                     */
                    _rv = _bcm_caladan3_mpls_switch_ft_lp_get(l3_fe,
                                                                 switch_info,
                                                                &ftidx,
                                                                &lpidx);
                    if ((_rv == BCM_E_NONE) && 
                        (ftidx != SBX_DROP_FTE(l3_fe->fe_unit))) {
                 
                        _rv = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                  SBX_CALADAN3_USR_RES_FTE_MPLS,
                                                  1,
                                                  &ftidx,
                                                  _SBX_CALADAN3_RES_FLAGS_RESERVE);
                        if (BCM_FAILURE(_rv)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                               (BSL_META_U(unit,
                                 "MPLS WB Restore: MPLS Fte Alloc failed:%d\n"),
                                _rv));
                            sal_free(switch_info);
                            return _rv;
                        }
                    }
                    if (_rv == BCM_E_NONE) {
                        _rv = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                SBX_CALADAN3_USR_RES_MPLS_LPORT,
                                                1,
                                                &lpidx,
                                                _SBX_CALADAN3_RES_FLAGS_RESERVE);
                        if (BCM_FAILURE(_rv)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                     (BSL_META_U(unit,
                                                "MPLS WB Res: MPLS Lport(0x%x)"
                                                " alloc failed:%d\n"),
                                     lpidx, _rv));
                            sal_free(switch_info);
                            return _rv;
                        }
                    } else {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                 (BSL_META_U(unit,
                                         "MPLS WB Res: FT_LP Get failed:%d\n"),
                                  _rv));
                        sal_free(switch_info);
                        return _rv;
                    }
                    _rv = _bcm_caladan3_mpls_switch_info_db_add(unit, switch_info);
                    if (BCM_FAILURE(_rv)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                 (BSL_META_U(unit,
                                             "MPLS WB Res: switch info "
                                             " db add failed:%d\n"),
                                  _rv));
                                                  
                    }
                } else {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Memory allocation for switch info "
                                          "failed. switch_count:%d(inst:%d)\n"),
                               switch_count, i));
#endif
                }
            }
            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_MPLS | BSL_INFO)) {
#if _BCM_CALADAN3_MPLS_WARMBOOT_READ_TRACKING
                 _bcm_caladan3_mpls_sw_dump(unit);
#endif
            }
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_mpls_state_init (int unit)
{
    int    flags   = SOC_CALADAN3_SCACHE_DEFAULT;
    int    exists  = 0;
    uint16 version = BCM_CALADAN3_WB_MPLS_VERSION_CURR;
    uint16 recovered_version   = 0;
    uint8 *scache_ptr          = NULL;
    unsigned int scache_len    = 0;
    soc_scache_handle_t handle = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit)) {
        _bcm_caladan3_wb_mpls_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mpls_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_MPLS, 0);

    if (SOC_WARM_BOOT (unit)) {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        _rv = bcm_caladan3_scache_ptr_get (unit,
                                           handle,
                                           socScacheRetrieve,
                                           flags,
                                          &scache_len,
                                          &scache_ptr,
                                           version,
                                          &recovered_version,
                                          &exists);
    
        if (BCM_FAILURE(_rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d MPLS instance: %d (%s)\n"),
                       unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }

        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "unit %d loading MPLS state\n"),
                     unit));
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;

        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mpls_state_layout_init (unit,
                                                  version, &scache_len));

        if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Scache length %d is not same as "
                                  "stored length %d\n"),
                       scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
        }
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mpls_state_restore (unit));
        if (version != recovered_version) {
            /* set up layout for the preferred version */
            BCM_IF_ERR_EXIT (
                      _bcm_caladan3_wb_mpls_state_layout_init (unit,
                                                 version, &scache_len));
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "unit %d reallocate %d bytes warm boot "
                                    "backing store space\n"), unit, scache_len));

             /* reallocate the warm boot space */
            _rv = bcm_caladan3_scache_ptr_get (unit,
                                               handle,
                                               socScacheRealloc,
                                               flags,
                                              &scache_len,
                                              &scache_ptr,
                                               version,
                                              &recovered_version,
                                              &exists);
            if (BCM_FAILURE(_rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to reallocate %d bytes warm boot "
                                      "space for unit %d MPLS instance: %d (%s)\n"),
                           scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
                BCM_EXIT;
            }

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_mpls_state_scache_info_p[unit]->init_done = TRUE;

        }		/* if (version != recovered_version) */
    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mpls_state_layout_init (unit, 
                                                     version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "unit %d MPLS: allocate %d bytes warm boot "
                                "backing store space\n"),
                     unit, scache_len));
        _rv = bcm_caladan3_scache_ptr_get (unit,
                                           handle,
                                           socScacheCreate,
                                           flags,
                                          &scache_len,
                                          &scache_ptr,
                                           version,
                                          &recovered_version,
                                          &exists);
        if (BCM_FAILURE(_rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_mpls_state_scache_info_p[unit]->init_done = TRUE;
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
