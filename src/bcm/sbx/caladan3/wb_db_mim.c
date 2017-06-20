/*
 * $Id: wb_db_mim.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: MIM APIs
 *
 * Purpose:
 *     MIM API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/sbDq.h>

#include <bcm_int/common/debug.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/mim.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_mim.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_MIM


extern bcm_caladan3_mim_trunk_association_t
    caladan3_mim_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

extern _bcm_caladan3_mimgp_info_t caladan3_gportinfo[BCM_MAX_NUM_UNITS];

extern shr_avl_t   *caladan3_mim_vpn_db[BCM_MAX_NUM_UNITS];

extern int _bcm_caladan3_mim_insert_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2);

static bcm_caladan3_wb_mim_state_scache_info_t * _bcm_caladan3_wb_mim_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_mim_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_mim_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_mim_state_scache_info_p[unit]->init_done == TRUE))

STATIC int
_bcm_caladan3_wb_mim_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_mim_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_mim_state_scache_info_t), "Scache for MIM warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_mim_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_mim_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_mim_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    int rc = BCM_E_NONE;
    uint32  vsi_min=0, vsi_max=0;
    uint32  gport_min=0, gport_max=0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_MIM_VERSION_1_0:
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            SBX_WB_DB_LAYOUT_INIT_NV(_bcm_caladan3_mimgp_info_t, BCM_MAX_NUM_UNITS);

            SBX_WB_DB_LAYOUT_INIT_NV(uint32, 1);

            rc = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_VSI,
                                      &vsi_min, &vsi_max);
            SBX_SCACHE_INFO_PTR(unit)->avl_tree_size = (vsi_max - vsi_min + 1);

            SBX_WB_DB_LAYOUT_INIT_NV(bcm_caladan3_mim_vpn_control_t, (vsi_max-vsi_min+1));

            SBX_WB_DB_LAYOUT_INIT_NV(int16, (vsi_max-vsi_min+1)*3);
            
            rc = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                      &gport_min, &gport_max);
            SBX_WB_DB_LAYOUT_INIT_NV(bcm_caladan3_mim_port_control_t, (gport_max-gport_min+1));

            SBX_SCACHE_INFO_PTR(unit)->scache_len = *scache_len;
            break;


        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}


/*
 *  This function is called by the AVL tree traversal function
 * and writes out one element on the tree.
 */
int
_bcm_caladan3_wb_mim_vpn_db_write(void *userdata,
                                 shr_avl_datum_t *datum,
                                 void *travdata)
{
    bcm_caladan3_mim_vpn_control_t  *vpninfo;
    int                             unit;
    int16                           access_count = 0;
    int16                           bbone_count = 0;
    int16                           def_bbone_count = 0;
    dq_p_t                          dqE;
    bcm_caladan3_mim_port_control_t *portcb;

    vpninfo = (bcm_caladan3_mim_vpn_control_t *)datum;
    unit = *(int*)travdata;

    /* Write the VPN control data */
    SBX_WB_DB_SYNC_VARIABLE(bcm_caladan3_mim_vpn_control_t, 1, *vpninfo);

    /* Get the count of ports attached to the VPN */
    DQ_TRAVERSE(&vpninfo->vpn_access_sap_head, dqE) {
        access_count++;
    } DQ_TRAVERSE_END(&vpninfo->vpn_access_sap_head, dqE);
    DQ_TRAVERSE(&vpninfo->vpn_bbone_sap_head, dqE) {
        bbone_count++;
    } DQ_TRAVERSE_END(&vpninfo->vpn_bbone_sap_head, dqE);
    DQ_TRAVERSE(&vpninfo->def_bbone_plist, dqE) {
        def_bbone_count++;
    } DQ_TRAVERSE_END(&vpninfo->def_bbone_plist, dqE);

    /* Write the port counts */
    SBX_WB_DB_SYNC_VARIABLE(int16, 1, access_count);
    SBX_WB_DB_SYNC_VARIABLE(int16, 1, bbone_count);
    SBX_WB_DB_SYNC_VARIABLE(int16, 1, def_bbone_count);

    /* Write the port data */
    DQ_TRAVERSE(&vpninfo->vpn_access_sap_head, dqE) {
        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portcb);
        SBX_WB_DB_SYNC_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
    } DQ_TRAVERSE_END(&vpninfo->vpn_access_sap_head, dqE);
    DQ_TRAVERSE(&vpninfo->vpn_bbone_sap_head, dqE) {
        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portcb);
        SBX_WB_DB_SYNC_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
    } DQ_TRAVERSE_END(&vpninfo->vpn_bbone_sap_head, dqE);
    DQ_TRAVERSE(&vpninfo->def_bbone_plist, dqE) {
        /* coverity [uninit_use] */
        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portcb);
        SBX_WB_DB_SYNC_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
    } DQ_TRAVERSE_END(&vpninfo->def_bbone_plist, dqE);

    return BCM_E_NONE;
}

int
bcm_caladan3_wb_mim_state_sync (int unit)
{
    int                              rc = BCM_E_NONE;
    int                             version = 0;
    uint8                           *scache_ptr_orig = NULL;
    int                             count;

    BCM_INIT_FUNC_DEFS;
    
    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "Sync not allowed during warmboot on unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
         LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                           "Warm boot scache not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_MIM_VERSION_1_0:
            
            SBX_WB_DB_SYNC_VARIABLE(_bcm_caladan3_mimgp_info_t,
                1, caladan3_gportinfo[unit]);

            count = shr_avl_count(caladan3_mim_vpn_db[unit]);
            SBX_WB_DB_SYNC_VARIABLE(uint32, 1, count);
            shr_avl_traverse(caladan3_mim_vpn_db[unit],
                             _bcm_caladan3_wb_mim_vpn_db_write,
                             (void*)&unit);

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            break;


        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}

int _bcm_caladan3_wb_mim_state_restore_resource(int unit, bcm_caladan3_mim_port_control_t *portcb)
{
    int rc;

    /* inseg */
    rc = _sbx_caladan3_resource_alloc(unit,
                                      SBX_CALADAN3_USR_RES_LPORT,
                                      1,
                                      &portcb->hwinfo.lport,
                                      _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if(rc != BCM_E_NONE)
    {
        LOG_ERROR(BSL_LS_BCM_MIM,
            (BSL_META_U(unit,
            "%s: resource_alloc failed(lp:0x%08x)\n"),
            __FUNCTION__, portcb->hwinfo.lport));
        return rc;
    }

    rc = _sbx_caladan3_resource_alloc(unit,
                                      SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                      1,
                                      &portcb->hwinfo.ftidx,
                                      _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if((rc != BCM_E_NONE) && (rc != BCM_E_RESOURCE))
    {
        LOG_ERROR(BSL_LS_BCM_MIM,
            (BSL_META_U(unit,
            "%s: resource_alloc failed(ggport:0x%08x)\n"),
            __FUNCTION__, portcb->hwinfo.ftidx));
        return rc;
    }


    /* outseg */
    rc = _sbx_caladan3_resource_alloc(unit,
                                      SBX_CALADAN3_USR_RES_OHI,
                                      1,
                                      &portcb->hwinfo.ohi,
                                      _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if((rc != BCM_E_NONE) && (rc != BCM_E_RESOURCE))
    {
        LOG_ERROR(BSL_LS_BCM_MIM,
            (BSL_META_U(unit,
            "%s: resource_alloc failed(ohi:0x%08x)\n"),
            __FUNCTION__, portcb->hwinfo.ohi));
        return rc;
    }


    rc = _sbx_caladan3_resource_alloc(unit,
                                      SBX_CALADAN3_USR_RES_ETE,
                                      1,
                                      &portcb->hwinfo.ete,
                                      _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if(rc != BCM_E_NONE)
    {
        LOG_ERROR(BSL_LS_BCM_MIM,
            (BSL_META_U(unit,
            "%s: resource_alloc failed(ete:0x%08x)\n"),
            __FUNCTION__, portcb->hwinfo.ete));
        return rc;
    }

    return rc;
}

STATIC int
_bcm_caladan3_wb_mim_state_restore (int unit)
{
    int                                     rc = BCM_E_NONE;
    int                                     version = 0;
    int                                     count;
    int16 i;
    bcm_caladan3_mim_vpn_control_t          vpninfo, *vpncb;
    _bcm_caladan3_mim_lookup_data_t         hdl;
    int16                                   access_count = 0;
    int16                                   bbone_count = 0;
    int16                                   def_bbone_count = 0;
    bcm_caladan3_mim_port_control_t         *portcb;
    bcm_caladan3_mim_trunk_association_t    *trunkAssoc;
    uint32                                  vpnid, lport;


    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    switch(version)
    {
        case BCM_CALADAN3_WB_MIM_VERSION_1_0:

            SBX_WB_DB_RESTORE_VARIABLE(_bcm_caladan3_mimgp_info_t,
                1, caladan3_gportinfo[unit]);

            SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, count);
            for (i = 0; i < count; i++) {
                SBX_WB_DB_RESTORE_VARIABLE(bcm_caladan3_mim_vpn_control_t, 1, vpninfo);
                hdl.key = &vpninfo;
                hdl.datum = &vpncb;

                rc = shr_avl_insert(caladan3_mim_vpn_db[unit],
                                            _bcm_caladan3_mim_insert_compare,
                                            (shr_avl_datum_t*)&vpninfo);
                if (rc < 0) {
                    LOG_ERROR (BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                        "%s: shr_avl_insert failed\n"), __FUNCTION__));
                }
                rc = shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                                           _bcm_caladan3_mim_vpn_lookup_compare,
                                                           (shr_avl_datum_t*)&vpninfo,
                                                           (void*)&hdl);
                if (rc < 0) {
                    LOG_ERROR (BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                        "%s: shr_avl_lookup_lkupdata failed\n"), __FUNCTION__));
                }
                vpnid = vpninfo.vpnid;
                rc = _sbx_caladan3_resource_alloc(unit,
                                     SBX_CALADAN3_USR_RES_VSI,
                                     1,
                                     &vpnid,
                                     _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if(rc == BCM_E_RESOURCE) {
                    rc = BCM_E_NONE;
                }
                if (rc != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit, "Failed to allocate vpnid %u, %d\n"),
                            vpninfo.vpnid, rc));
                }
                lport = vpninfo.lport;
                rc = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_LPORT,
                                         1,
                                         &lport,
                                         _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (rc != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit, "Failed to allocate lport %u, %d\n"),
                            vpninfo.lport, rc));
                }
                DQ_INIT(&vpncb->vpn_access_sap_head);
                DQ_INIT(&vpncb->vpn_bbone_sap_head);
                DQ_INIT(&vpncb->def_bbone_plist);

                SBX_WB_DB_RESTORE_VARIABLE(int16, 1, access_count);
                SBX_WB_DB_RESTORE_VARIABLE(int16, 1, bbone_count);
                SBX_WB_DB_RESTORE_VARIABLE(int16, 1, def_bbone_count);

                while(access_count-- > 0) {
                    portcb = sal_alloc(sizeof(bcm_caladan3_mim_port_control_t),"MiM Port control");
                    SBX_WB_DB_RESTORE_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
                    portcb->listnode.flink = 0;
                    portcb->listnode.blink = 0;
                    DQ_INSERT_HEAD(&vpncb->vpn_access_sap_head, &portcb->listnode);
                    SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = (void*)portcb;
                    SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_MIM_PORT;
                    rc = _bcm_caladan3_wb_mim_state_restore_resource(unit, portcb);
                    if (rc != BCM_E_NONE){
                        LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                            "%s: resource_alloc failed(gport:0x%08x, port:0x%08x)\n"),
                            __FUNCTION__, portcb->gport,portcb->port));
                        return rc;
                    }
                }
                while(bbone_count-- > 0) {
                    portcb = sal_alloc(sizeof(bcm_caladan3_mim_port_control_t),"MiM Port control");
                    SBX_WB_DB_RESTORE_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
                    portcb->listnode.flink = 0;
                    portcb->listnode.blink = 0;
                    DQ_INSERT_HEAD(&vpncb->vpn_bbone_sap_head, &portcb->listnode);
                    SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = (void*)portcb;
                    SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_MIM_PORT;
                    rc = _bcm_caladan3_wb_mim_state_restore_resource(unit, portcb);
                    if (rc != BCM_E_NONE){
                        LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                            "%s: resource_alloc failed(gport:0x%08x, port:0x%08x)\n"),
                            __FUNCTION__, portcb->gport,portcb->port));
                        return rc;
                    }
                }
                while(def_bbone_count-- > 0) {
                    portcb = sal_alloc(sizeof(bcm_caladan3_mim_port_control_t),"MiM Port control");
                    SBX_WB_DB_RESTORE_VARIABLE(bcm_caladan3_mim_port_control_t, 1, *portcb);
                    portcb->listnode.flink = 0;
                    portcb->listnode.blink = 0;
                    DQ_INSERT_HEAD(&vpncb->def_bbone_plist, &portcb->listnode);
                    if (BCM_GPORT_IS_TRUNK(portcb->port)) {
                        trunkAssoc = &caladan3_mim_trunk_assoc_info[unit]
                            [BCM_GPORT_TRUNK_GET(portcb->port)];
                        DQ_INSERT_HEAD(&trunkAssoc->plist, &portcb->trunklistnode);
                        trunkAssoc->mimType = _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT;
                    }
                    SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = (void*)portcb;
                    SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_MIM_PORT;
                    rc = _bcm_caladan3_wb_mim_state_restore_resource(unit, portcb);
                    if (rc != BCM_E_NONE){
                        LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                            "%s: resource_alloc failed(gport:0x%08x, port:0x%08x)\n"),
                            __FUNCTION__, portcb->gport,portcb->port));
                        return rc;
                    }
                }

            }

            break;


        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_mim_state_init (int unit)
{
    int result = BCM_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_MIM_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit))
    {
        _bcm_caladan3_wb_mim_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mim_state_scache_alloc (unit));

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_MIM, 0);

    if (SOC_WARM_BOOT (unit))
    {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRetrieve, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == result)
        {
            LOG_VERBOSE (BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                "unit %d loading MIRROR state\n"),
                      unit));
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mim_state_layout_init (unit, version, &scache_len));
            if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len)
            {
                LOG_ERROR (BSL_LS_BCM_MIM,
                       (BSL_META_U(unit,
                                   "Scache length %d is not same as stored length %d\n"),
                       scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
            }
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mim_state_restore (unit));
            if (BCM_E_NONE == result)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mim_state_layout_init (unit, version, &scache_len));
                    LOG_VERBOSE (BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "unit %d reallocate %d bytes warm boot "
                                     "backing store space\n"),
                          unit, scache_len));
                    /* reallocate the warm boot space */
                    result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRealloc, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
                    if (BCM_E_NONE != result)
                    {
                        LOG_VERBOSE (BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "unit %d reallocate %d bytes warm boot "
                                     "backing store space\n"),
                          unit, scache_len));
                        BCM_EXIT;
                    }
                    SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == result) */

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_mim_state_scache_info_p[unit]->init_done = TRUE;
        }
        else
        {			/* if (BCM_E_NONE == result) */
            LOG_ERROR (BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                        "unable to get current warm boot state for"
                        " unit %d MIM instance: %d (%s)\n"),
                    unit, result, _SHR_ERRMSG (result)));
        }			/* if (BCM_E_NONE == result) */
    }
    else
    {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mim_state_layout_init (unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE (BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "unit %d MIRROR: allocate %d bytes warm boot "
                                 "backing store space\n"),
                      unit, scache_len));
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheCreate, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != result)
        {
            LOG_ERROR (BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d mim instance: %d (%s)\n"),
                         scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }
        else
        {
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_mim_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
