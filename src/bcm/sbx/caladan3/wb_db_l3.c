/* $SDK/src/bcm/sbx/caladan3/wb_db_l3.c */
/*
 * $Id: wb_db_l3.c,v Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: L3 APIs
 *
 * Purpose:
 *     Warm boot support for L3 API for Caladan3 Packet Processor devices
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <sal/core/sync.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <soc/sbx/wb_db_cmn.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_l3.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_L3

/*
 *  Set _BCM_CALADAN3_MPLS_WARMBOOT_WRITE_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time the MPLS backing store is written.
 */
#define _BCM_CALADAN3_L3_WARMBOOT_WRITE_TRACKING FALSE

/*
 *  Set _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time L3 backing store is read.
 */
#define _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING FALSE


/*
 *  Maximum number of l3 interfaces
 *  The max count 64K comes from RES_IFID Resource count
 */
#define WB_L3_MAX_INTFS        (64 * 1024)
/*
 *  Maximum number of l3_egress interfaces
 *  The max count 128K comes from RES_FTE_L3 Resource count
 */
#define WB_L3_MAX_EGR_INTFS    (128 * 1024)

/*
 *  Externs for L3
 */
extern void _bcm_caladan3_l3_debug(int unit);
extern void
_bcm_caladan3_l3_caladan3_l3_ete_t_print(int unit, 
                                         _caladan3_l3_ete_t *l3_sw_ete,
                                         int printHdr);

/*
extern 
_caladan3_l3_fe_instance_t * bcm_sbx_caladan3_l3_cntl_ptr_get(int unit);
*/

/*
 *  Locals for L3
 */
static bcm_caladan3_wb_l3_state_scache_info_t 
        *_bcm_caladan3_wb_l3_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for L3
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_l3_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_l3_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_l3_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Warmboot APIs implementation for L3
 */
STATIC int
_bcm_caladan3_wb_l3_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_l3_state_scache_info_p[unit], 
               sizeof (bcm_caladan3_wb_l3_state_scache_info_t),
                                        "Scache for L3 warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_l3_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_l3_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}


int
_bcm_caladan3_wb_l3_ete_layout_init(int unit, int version, unsigned int *scache_len)
{
    SBX_WB_DB_LAYOUT_INIT(bcm_if_t                , 1, _caladan3_l3_ete_t.l3_intf_id);
    SBX_WB_DB_LAYOUT_INIT(_caladan3_l3_ete_key_t  , 1, _caladan3_l3_ete_t.l3_ete_key);
    SBX_WB_DB_LAYOUT_INIT(_caladan3_ohi_t         , 1, _caladan3_l3_ete_t.l3_ohi);
    SBX_WB_DB_LAYOUT_INIT(_caladan3_ete_idx_t     , 1, _caladan3_l3_ete_t.l3_ete_hw_idx);
    SBX_WB_DB_LAYOUT_INIT(_caladan3_ohi_t         , 1, _caladan3_l3_ete_t.l3_mpls_ohi);
    SBX_WB_DB_LAYOUT_INIT(uint32                  , 1, _caladan3_l3_ete_t.l3_ete_hw_stat_idx);
    SBX_WB_DB_LAYOUT_INIT(int32                   , 1, _caladan3_l3_ete_t.l3_alloced_ue);
    SBX_WB_DB_LAYOUT_INIT(int32                   , 1, _caladan3_l3_ete_t.l3_inuse_ue);

    /*
     * _caladan3_l3_ete_t.u is a union with pointers to fte_t and ipmt_t.
     *  Allocate memory for the maximum size
     * L3/MPLS don't use fte/ipmc count > _CALADAN3_ETE_USER_SLAB_SIZE.
     * So it is acceptable to limit the count to _CALADAN3_ETE_USER_SLAB_SIZE.
     */
    if (sizeof(_caladan3_l3_ete_fte_t) > sizeof(_caladan3_l3_ete_ipmc_t)) {
        SBX_WB_DB_LAYOUT_INIT(_caladan3_l3_ete_fte_t,
                              _CALADAN3_ETE_USER_SLAB_SIZE,
                              _caladan3_l3_ete_t.u.l3_fte);
    } else {
        SBX_WB_DB_LAYOUT_INIT(_caladan3_l3_ete_ipmc_t,
                              _CALADAN3_ETE_USER_SLAB_SIZE,
                              _caladan3_l3_ete_t.u.l3_ipmc);
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_wb_l3_intf_layout_init(int unit, int version, unsigned int *scache_len)
{
    SBX_WB_DB_LAYOUT_INIT(int                   , 1, l3_intf.if_ip_ete_count);
    SBX_WB_DB_LAYOUT_INIT(bcm_l3_intf_t         , 1, l3_intf.if_info);
    SBX_WB_DB_LAYOUT_INIT(uint32                , 1, l3_intf.if_flags);
    SBX_WB_DB_LAYOUT_INIT(uint8                 , 1,
                                              (l3_intf.if_tunnel_info != NULL));
    SBX_WB_DB_LAYOUT_INIT(bcm_tunnel_initiator_t, 1, (*l3_intf.if_tunnel_info));
    SBX_WB_DB_LAYOUT_INIT(uint32                , 1, l3_intf.if_lmac_idx);
    SBX_WB_DB_LAYOUT_INIT(bcm_mpls_label_t      , 1, l3_intf.tunnel_egr_label);
    return BCM_E_NONE;
}


int
_bcm_caladan3_wb_l3_instance_layout_init(int unit, int version, unsigned int *scache_len)
{
            /* Layout for scache length and offset calculation */
    SBX_WB_DB_LAYOUT_INIT(int   , 1, _caladan3_l3_fe_instance_t.fe_ipmc_enabled);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.fe_mp_set_size);
    SBX_WB_DB_LAYOUT_INIT(int   , 1, _caladan3_l3_fe_instance_t.fe_ipv4_vrf_bits);
    SBX_WB_DB_LAYOUT_INIT(int   , 1, _caladan3_l3_fe_instance_t.fe_unit);
    SBX_WB_DB_LAYOUT_INIT(int   , 1, _caladan3_l3_fe_instance_t.fe_my_modid);
    SBX_WB_DB_LAYOUT_INIT(int   , 1, _caladan3_l3_fe_instance_t.fe_cosq_config_numcos);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.vlan_ft_base);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.umc_ft_offset);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.vpws_uni_ft_offset);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.max_pids);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.fe_vsi_default_vrf);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.fe_drop_vrf);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.fe_raw_ete_idx);
    SBX_WB_DB_LAYOUT_INIT(uint32, 1, _caladan3_l3_fe_instance_t.fe_flags);
    return BCM_E_NONE;

}


STATIC int
_bcm_caladan3_wb_l3_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    uint32       max_intf    = WB_L3_MAX_INTFS;
    uint32       max_l3_egr  = WB_L3_MAX_EGR_INTFS;
    unsigned int l3_intf_len = 0;
    unsigned int l3_egr_len  = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_L3_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            _bcm_caladan3_wb_l3_instance_layout_init(unit, version, scache_len);


	    /*------------------------------------------------*/
            /* Store Maximum number of l3 interfaces */
	    /* Store number of l3 interfaces created *
	     * Traverse through l3 interface hash table and dump each one.
	     */

            SBX_WB_DB_LAYOUT_INIT(uint32, 1, l3_intf_count);

            _bcm_caladan3_wb_l3_intf_layout_init(unit, version, &l3_intf_len);
            *scache_len += (l3_intf_len * max_intf);

            SBX_WB_DB_LAYOUT_INIT(uint32, 1, num_l3_egress); 
            _bcm_caladan3_wb_l3_ete_layout_init(unit, version, &l3_egr_len);
            *scache_len += (l3_egr_len * max_l3_egr);

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


void
_bcm_caladan3_wb_l3_ete_sync(int unit, _caladan3_l3_ete_t *l3_sw_ete)
{
    int array_idx;

    SBX_WB_DB_SYNC_VARIABLE(bcm_if_t              , 1, l3_sw_ete->l3_intf_id);
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_l3_ete_key_t, 1, l3_sw_ete->l3_ete_key);
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_ohi_t       , 1, l3_sw_ete->l3_ohi);
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_ete_idx_t   , 1, l3_sw_ete->l3_ete_hw_idx);
    SBX_WB_DB_SYNC_VARIABLE(_caladan3_ohi_t       , 1, l3_sw_ete->l3_mpls_ohi);
    SBX_WB_DB_SYNC_VARIABLE(uint32                , 1,
                                                 l3_sw_ete->l3_ete_hw_stat_idx);
    SBX_WB_DB_SYNC_VARIABLE(int32                 , 1, l3_sw_ete->l3_alloced_ue);
    SBX_WB_DB_SYNC_VARIABLE(int32                 , 1, l3_sw_ete->l3_inuse_ue);

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {

        SBX_WB_DB_SYNC_ARRAY(_caladan3_l3_ete_fte_t,
                             _CALADAN3_ETE_USER_SLAB_SIZE,
                             *(l3_sw_ete->u.l3_fte));

    } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type
                          == _CALADAN3_L3_ETE__MCAST_IP) {

        SBX_WB_DB_SYNC_ARRAY(_caladan3_l3_ete_ipmc_t,
                             1,
                             *(l3_sw_ete->u.l3_ipmc));

    }
}

void
_bcm_caladan3_wb_l3_intf_sync(int unit, _caladan3_l3_intf_t *l3_intf)
{
    uint8                  tunnel_info_exists = 0;
    bcm_tunnel_initiator_t if_tunnel_info;

    SBX_WB_DB_SYNC_VARIABLE(int             , 1, l3_intf->if_ip_ete_count);
    SBX_WB_DB_SYNC_VARIABLE(bcm_l3_intf_t   , 1, l3_intf->if_info);
    SBX_WB_DB_SYNC_VARIABLE(uint32          , 1, l3_intf->if_flags);

    if (l3_intf->if_tunnel_info != NULL) {
        tunnel_info_exists = 1;
    }
    SBX_WB_DB_SYNC_VARIABLE(uint8           , 1, tunnel_info_exists);
    if (l3_intf->if_tunnel_info) {
        if_tunnel_info = *(l3_intf->if_tunnel_info);
        SBX_WB_DB_SYNC_VARIABLE(bcm_tunnel_initiator_t, 1, if_tunnel_info);
    }
    SBX_WB_DB_SYNC_VARIABLE(uint32          , 1, l3_intf->if_lmac_idx);
    SBX_WB_DB_SYNC_VARIABLE(bcm_mpls_label_t, 1, l3_intf->tunnel_egr_label);
}

void
_bcm_caladan3_wb_l3_instance_sync(int unit, _caladan3_l3_fe_instance_t *l3_fe)
{
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, l3_fe->fe_ipmc_enabled);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->fe_mp_set_size);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, l3_fe->fe_ipv4_vrf_bits);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, l3_fe->fe_unit);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, l3_fe->fe_my_modid);
    SBX_WB_DB_SYNC_VARIABLE(int   , 1, l3_fe->fe_cosq_config_numcos);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->vlan_ft_base);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->umc_ft_offset);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->vpws_uni_ft_offset);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->max_pids);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->fe_vsi_default_vrf);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->fe_drop_vrf);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->fe_raw_ete_idx);
    SBX_WB_DB_SYNC_VARIABLE(uint32, 1, l3_fe->fe_flags);
}

int
bcm_caladan3_wb_l3_state_sync (int unit)
{
    int                  version = 0;
    uint8 *scache_ptr_orig = NULL;
    uint32               sw_ete_count = 0;
    uint32               l3_intf_count = 0;
    dq_p_t               l3_intf_elem;   
    dq_p_t               hash_bucket;
    uint32               i;
    uint32               j;
    uint8               *sw_ete_count_ptr  = NULL;
    uint8               *l3_intf_count_ptr = NULL;

    _caladan3_l3_intf_t        *l3_intf   = NULL;
    _caladan3_l3_fe_instance_t *l3_fe     = NULL;
    _caladan3_l3_ete_t         *l3_sw_ete = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L3,
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
        case BCM_CALADAN3_WB_L3_VERSION_1_0:
            /* Sync state to the scache */
            _bcm_caladan3_wb_l3_instance_sync(unit, l3_fe);

            /* Store the pointer for intf_count, to write it later */
            SBX_WB_DB_GET_SCACHE_PTR(l3_intf_count_ptr);
            SBX_WB_DB_MOVE_SCACHE_PTR(uint32, 1);

            /* Iterate through the hash table, and store the l3intf */
            for (j=0; j < _CALADAN3_INTF_ID_HASH_SIZE; j++) {
                hash_bucket = &l3_fe->fe_intf_by_id[j];
                DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
                    _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
                    _bcm_caladan3_wb_l3_intf_sync(unit, l3_intf);
                    l3_intf_count++;
                } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);
	    }
            /* Write the intf_count using the previously stored pointer */
            SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(uint32,
                                             1,
                                             l3_intf_count,
                                             l3_intf_count_ptr);

#if _BCM_CALADAN3_L3_WARMBOOT_WRITE_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                    "L3 WB sync: intf_count %d \n"),
                         l3_intf_count));
#endif


            /* Store the pointer for ete_count, to write it later */
            SBX_WB_DB_GET_SCACHE_PTR(sw_ete_count_ptr);
            SBX_WB_DB_MOVE_SCACHE_PTR(uint32, 1);

            /* Iterate through the hash table, and store the l3ete */
            for (i=0; i < _CALADAN3_INTF_ID_HASH_SIZE; i++) {
                hash_bucket = &l3_fe->fe_intf_by_id[i];
                DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
                    _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
                    for ( j = 0; j < _CALADAN3_INTF_L3ETE_HASH_SIZE; j++) {
                        _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, j, l3_sw_ete) {
                            _bcm_caladan3_wb_l3_ete_sync(unit, l3_sw_ete);
                             sw_ete_count++;
                        } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, j, l3_sw_ete);
                    }
                } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);
            }

            /* Write the ete_count at the previously stored pointer */
            SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(uint32,
                                             1,
                                             sw_ete_count,
                                             sw_ete_count_ptr);

#if _BCM_CALADAN3_L3_WARMBOOT_WRITE_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                    "L3 WB sync: sw_ete_count %d \n"),
                         sw_ete_count));
#endif
            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_L3 | BSL_INFO)) {
                _bcm_caladan3_l3_sw_dump(unit);
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
_bcm_caladan3_wb_l3_ete_restore(_caladan3_l3_fe_instance_t *l3_fe)
{
    int     status = BCM_E_NONE;
    int     array_idx;
    int     unit;
    uint32  ete_hash_idx;
    _caladan3_l3_ete_t    *l3_sw_ete;
    _caladan3_l3_intf_t   *l3_intf;

    unit = l3_fe->fe_unit;

    l3_sw_ete = (_caladan3_l3_ete_t *) 
                   sal_alloc(sizeof(_caladan3_l3_ete_t), "WB Intf-Data");
    if (l3_sw_ete) {
        sal_memset(l3_sw_ete, 0, sizeof(_caladan3_l3_ete_t));
    } else {
        return BCM_E_MEMORY;
    }


    SBX_WB_DB_RESTORE_VARIABLE(bcm_if_t              , 1, l3_sw_ete->l3_intf_id);
    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_l3_ete_key_t, 1, l3_sw_ete->l3_ete_key);
    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_ohi_t       , 1, l3_sw_ete->l3_ohi);
    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_ete_idx_t   , 1,
                                                   l3_sw_ete->l3_ete_hw_idx);
    SBX_WB_DB_RESTORE_VARIABLE(_caladan3_ohi_t, 1, l3_sw_ete->l3_mpls_ohi);
    SBX_WB_DB_RESTORE_VARIABLE(uint32      , 1, l3_sw_ete->l3_ete_hw_stat_idx);
    SBX_WB_DB_RESTORE_VARIABLE(int32       , 1, l3_sw_ete->l3_alloced_ue);
    SBX_WB_DB_RESTORE_VARIABLE(int32       , 1, l3_sw_ete->l3_inuse_ue);

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {
        l3_sw_ete->u.l3_fte = (_caladan3_l3_ete_fte_t *)
                   sal_alloc(sizeof(_caladan3_l3_ete_fte_t) *
                              _CALADAN3_ETE_USER_SLAB_SIZE, "WB: Ip-ete-fte");
        SBX_WB_DB_RESTORE_ARRAY(_caladan3_l3_ete_fte_t,
                                _CALADAN3_ETE_USER_SLAB_SIZE,
                                (*l3_sw_ete->u.l3_fte));
        for (array_idx = 0; array_idx < l3_sw_ete->l3_inuse_ue; array_idx++) {
            status = _bcm_caladan3_alloc_l3_or_mpls_fte(l3_fe,
                                               BCM_L3_WITH_ID,
                                               0,
                                               SBX_CALADAN3_USR_RES_FTE_L3,
                                               &(l3_sw_ete->u.l3_fte[array_idx].fte_idx.fte_idx));
            if (BCM_FAILURE(status)) {
                sal_free(l3_sw_ete);
                return status;
            }
        }
    } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) {
        l3_sw_ete->u.l3_ipmc = (_caladan3_l3_ete_ipmc_t *)
                   sal_alloc(sizeof(_caladan3_l3_ete_ipmc_t) *
                              _CALADAN3_ETE_USER_SLAB_SIZE, "WB: Ip-ete-ipmc");
        SBX_WB_DB_RESTORE_ARRAY(_caladan3_l3_ete_ipmc_t,
                                _CALADAN3_ETE_USER_SLAB_SIZE,
                                (*l3_sw_ete->u.l3_ipmc));
    }


    /* Init the links, add to hash tables */

    DQ_INIT(&l3_sw_ete->l3_vc_ete_head);

    /* 
     * 1. Add the l3_sw_ete to the hash table l3intf->if_ete_hash,
                            l3_fe->fe_eteidx2_l3ete, l3_fe->fe_ohi2_l3_ete
     * 2. Alloc ete resource
     */

    /* Add the l3_sw_ete to l3_intf ete hash list */
    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                l3_sw_ete->l3_intf_id,
                                                &l3_intf);
    if (BCM_SUCCESS(status)) {
        _CALADAN3_CALC_INTF_L3ETE_HASH(ete_hash_idx,
                                       l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                                       l3_sw_ete->l3_ete_key.l3_ete_hk.dmac);
        DQ_INSERT_HEAD(&l3_intf->if_ete_hash[ete_hash_idx],
                                   &l3_sw_ete->l3_ieh_link);
    } else {
#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(unit,
                             "L3 WB restore: Couldn't find l3intf[ID:0x%x]\n"),
                  l3_sw_ete->l3_intf_id));
#endif
    }

    /* Add the l3_sw_ete to l3fe eteidx2 lete ete */
    ete_hash_idx = _CALADAN3_GET_ETE_HASH_IDX(l3_sw_ete->l3_ete_hw_idx.ete_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_eteidx2_l3_ete[ete_hash_idx],
                                      &l3_sw_ete->l3_ete_link);

     /* Reserve ETE: l3_sw_ete.l3_ete_hw_idx.ete_idx */
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                 SBX_CALADAN3_USR_RES_ETE,
                                 1,
                                &l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (BCM_FAILURE(status)) {
        sal_free(l3_sw_ete);
        return status;
    }

    /* Reserve MPLS OHI l3_sw_ete.l3_mpls_ohi.ohi */
    if (l3_sw_ete->l3_ohi.ohi != _CALADAN3_INVALID_OHI) {
        ete_hash_idx = _CALADAN3_GET_OHI2ETE_HASH_IDX(l3_sw_ete->l3_ohi.ohi);
#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "L3 WB restore: inserting sw ete to ohi2_l3_ete "
                                "ohi:0x%x\n"), l3_sw_ete->l3_ohi.ohi));
#endif
        DQ_INSERT_HEAD(&l3_fe->fe_ohi2_l3_ete[ete_hash_idx],
                                      &l3_sw_ete->l3_ohi_link);

        /* Reserve OHI l3_sw_ete.l3_ohi.ohi */
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_OHI,
                                     1,
                                    &l3_sw_ete->l3_ohi.ohi,
                                    _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (BCM_FAILURE(status)) {
            sal_free(l3_sw_ete);
            return status;
        }
    }

#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "L3 WB restore: sw_ete intfid: 0x%x\n"),
                 l3_sw_ete->l3_intf_id));
#endif

 /*               _bcm_caladan3_l3_caladan3_l3_ete_t_print(unit, l3_sw_ete); */
    return status;
}

int
_bcm_caladan3_wb_l3_intf_restore(_caladan3_l3_fe_instance_t *l3_fe)
{
    int          status = BCM_E_NONE;
    int          ii;
    dq_p_t       hash_bucket;
    uint32       hash_index, ifid = ~0 ;
    int          unit;
    uint8        tunnel_info_exists;
    _caladan3_l3_intf_t *l3_intf;

    unit = l3_fe->fe_unit;

    /* Allocate memory for the l3intf */
    l3_intf = (_caladan3_l3_intf_t *) 
                        sal_alloc(sizeof(_caladan3_l3_intf_t), "WB Intf-Data");
    if (l3_intf) {
        sal_memset(l3_intf, 0, sizeof(_caladan3_l3_intf_t));
    } else {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "L3 WB restore: Couldn't allocate memory for "
                              "l3intf\n")));
        return BCM_E_MEMORY;
    }

    SBX_WB_DB_RESTORE_VARIABLE(int          , 1, l3_intf->if_ip_ete_count);
    SBX_WB_DB_RESTORE_VARIABLE(bcm_l3_intf_t, 1, l3_intf->if_info);
    SBX_WB_DB_RESTORE_VARIABLE(uint32       , 1, l3_intf->if_flags);
    SBX_WB_DB_RESTORE_VARIABLE(uint8        , 1, tunnel_info_exists);

    if (tunnel_info_exists) {
        l3_intf->if_tunnel_info = (bcm_tunnel_initiator_t *)
                 sal_alloc(sizeof(bcm_tunnel_initiator_t), "WB L3 TunnelInfo");
        if (l3_intf->if_tunnel_info) {
            sal_memset(l3_intf->if_tunnel_info, 0, sizeof (bcm_tunnel_initiator_t));
        } else {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(unit,
                                  "L3 WB restore: Couldn't allocate memory for tunnel initiator\n")));
            sal_free(l3_intf);
            return BCM_E_MEMORY;
        }
        SBX_WB_DB_RESTORE_VARIABLE(bcm_tunnel_initiator_t,
                                      1, *(l3_intf->if_tunnel_info));
    }
    SBX_WB_DB_RESTORE_VARIABLE(uint32          , 1, l3_intf->if_lmac_idx);
    SBX_WB_DB_RESTORE_VARIABLE(bcm_mpls_label_t, 1, l3_intf->tunnel_egr_label);

    for (ii = 0; ii < _CALADAN3_INTF_L3ETE_HASH_SIZE; ii++) {
        DQ_INIT(&l3_intf->if_ete_hash[ii]);
    }
    DQ_INIT(&l3_intf->if_oam_ep_list);

#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "L3 WB restore: l3intf intfid: 0x%x\n"),
                 l3_intf->if_info.l3a_intf_id));
#endif

    /* 
     * 1. Alloc l3intf resource
     * 2. Alloc lmac_idx
     * 3. Add the l3intf to the hash table l3_fe_instance (byVid, byID )
     */

    ifid = l3_intf->if_info.l3a_intf_id;
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                 SBX_CALADAN3_USR_RES_IFID,
                                 1,
                                 &ifid,
                                 _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "L3 WB restore: alloc IFID(0x%x) failed: %d\n"),
                   ifid, status));
        if (l3_intf->if_tunnel_info != NULL) {
            sal_free(l3_intf->if_tunnel_info);
        }
        sal_free(l3_intf);
        return status;
    }

    status = _sbx_caladan3_ismac_idx_alloc(l3_fe->fe_unit,
                                  _SBX_CALADAN3_RES_FLAGS_RESERVE,
                                  l3_intf->if_info.l3a_mac_addr, 
                                  _SBX_CALADAN3_RES_UNUSED_PORT,
                                  &l3_intf->if_lmac_idx);
    if (BCM_FAILURE(status)) {
        status = BCM_E_NONE;
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "L3 WB restore: ismac idx alloc failed: %d\n"),
                   status));

        if (l3_intf->if_tunnel_info != NULL) {
            sal_free(l3_intf->if_tunnel_info);
        }
        sal_free(l3_intf);
        return status;
    }

    /* Add l3intf to l3fe intf by id list */
    hash_index    = _CALADAN3_GET_INTF_ID_HASH(l3_intf->if_info.l3a_intf_id);
    hash_bucket   = &l3_fe->fe_intf_by_id[hash_index];
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_ifid_link);

    /* Add l3intf to l3fe intf by vid list */
    hash_index    = _CALADAN3_GET_INTF_VID_HASH(l3_intf->if_info.l3a_vid);
    hash_bucket   = &l3_fe->fe_intf_by_vid[hash_index];
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_vid_link);

   /* Add l3intf to l3fe fe_intf_by_egr_lbl */
    if (l3_intf->tunnel_egr_label) {
        hash_index    = _CALADAN3_GET_INTF_EGR_LBL_HASH(l3_intf->tunnel_egr_label);
        hash_bucket   = &l3_fe->fe_intf_by_egr_lbl[hash_index];
        DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_egr_lbl_link);
    }

    return status;
}

int
_bcm_caladan3_wb_l3_instance_restore(int unit, _caladan3_l3_fe_instance_t *l3_fe)
{
    int status = BCM_E_NONE;
            /* Restore state from scache */
    SBX_WB_DB_RESTORE_VARIABLE(int   , 1, l3_fe->fe_ipmc_enabled);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->fe_mp_set_size);
    SBX_WB_DB_RESTORE_VARIABLE(int   , 1, l3_fe->fe_ipv4_vrf_bits);
    SBX_WB_DB_RESTORE_VARIABLE(int   , 1, l3_fe->fe_unit);
    SBX_WB_DB_RESTORE_VARIABLE(int   , 1, l3_fe->fe_my_modid);
    SBX_WB_DB_RESTORE_VARIABLE(int   , 1, l3_fe->fe_cosq_config_numcos);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->vlan_ft_base);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->umc_ft_offset);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->vpws_uni_ft_offset);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->max_pids);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->fe_vsi_default_vrf);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->fe_drop_vrf);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->fe_raw_ete_idx);
    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_fe->fe_flags);

    /* Remove the mpls init flag from l3_fe->fe_flags.
     * It will be re-added in mpls init */
    l3_fe->fe_flags = (l3_fe->fe_flags & ~_CALADAN3_L3_FE_FLG_MPLS_INIT);
    return status;
}


STATIC int
_bcm_caladan3_wb_l3_state_restore (int unit)
{
    int     status COMPILER_ATTRIBUTE((unused));
    int     version = 0;
    uint32  sw_ete_count;
    uint32  l3_intf_count;
    uint32  count;
    _caladan3_l3_fe_instance_t *l3_fe; 

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (NULL == l3_fe) {
        return BCM_E_UNIT;
    }

#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "L3 WB restore: Printing STATE Before RESTORE\n")));
    _bcm_caladan3_l3_sw_dump(unit);
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "L3 WB restore: Printing over -- to -- RESTORE\n")));
#endif

    switch(version)
    {
        case BCM_CALADAN3_WB_L3_VERSION_1_0:

            _bcm_caladan3_wb_l3_instance_restore(unit, l3_fe);
            SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, l3_intf_count);
#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                    "L3 WB restore: intf_count %d \n"),
                         l3_intf_count));
#endif
	    for (count = 0; count < l3_intf_count; count++) {
                status = _bcm_caladan3_wb_l3_intf_restore(l3_fe);
            }
	    
            /* Read ete_count
             *  Restre l3_ete (ete_count times) 
             */
            SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, sw_ete_count);
#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                    "L3 WB restore: sw_ete_count %d \n"),
                         sw_ete_count));
#endif
               
            for (count = 0; count < sw_ete_count; count++) {
                _bcm_caladan3_wb_l3_ete_restore(l3_fe);
            }

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_L3 | BSL_INFO)) {
#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
                 _bcm_caladan3_l3_sw_dump(unit);
#endif
            }
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
            /* coverity[dead_error_line] */
            break;
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_l3_state_init (int unit)
{
    int     flags   = SOC_CALADAN3_SCACHE_DEFAULT;
    int     exists  = 0;
    uint16  version = BCM_CALADAN3_WB_L3_VERSION_CURR;
    uint16  recovered_version = 0;
    uint8  *scache_ptr        = NULL;
    unsigned int        scache_len = 0;
    soc_scache_handle_t handle     = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit)) {
        _bcm_caladan3_wb_l3_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l3_state_scache_alloc (unit));

    if(SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_L3, 0);

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
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d L3 instance: %d (%s)\n"),
                       unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }

#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "unit %d loading L3 backing store state\n"),
                     unit));
#endif
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;

        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l3_state_layout_init(unit, 
                                                  version, &scache_len));

        if (scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(unit,
                                  "Scache length %d is not same as stored "
                                  "length %d\n"),
                       scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
        }
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l3_state_restore (unit));

        if (version != recovered_version) {
            /* set up layout for the preferred version */
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l3_state_layout_init (unit,
                                                    version, &scache_len));

#if _BCM_CALADAN3_L3_WARMBOOT_READ_TRACKING
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                    "unit %d reallocate %d bytes warm boot "
                                    "backing store space\n"), unit, scache_len));
#endif

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
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(unit,
                                      "unable to reallocate %d bytes"
                                      " warm boot space for unit %d"
                                      " L3 instance: %d (%s)\n"),
                           scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
                BCM_EXIT;
            }

        }		/* if (version != recovered_version) */

        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_l3_state_scache_info_p[unit]->init_done = TRUE;

    } else {

        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l3_state_layout_init (unit, 
                                                  version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "unit %d L3: allocate %d bytes warm boot "
                                "backing store space\n"), unit, scache_len));
        _rv = bcm_caladan3_scache_ptr_get (unit,
                                           handle,
                                           socScacheCreate,
                                           flags,
                                          &scache_len,
                                          &scache_ptr,
                                           version,
                                          &recovered_version,
                                          &exists);
        if (BCM_E_NONE != _rv) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_l3_state_scache_info_p[unit]->init_done = TRUE;
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
