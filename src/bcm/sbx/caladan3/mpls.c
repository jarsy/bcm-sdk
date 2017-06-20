/*
 * $Id: mpls.c,v 1.39 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    mpls.c
 * Purpose: Manages MPLS functions
 */

#define CALADAN3_UNPORTED 1
#if defined(INCLUDE_L3)

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_cmu.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/caladan3.h>

#include <shared/idxres_afl.h>
#include <shared/gport.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <bcm_int/sbx/caladan3/trunk.h>
#include <bcm/vlan.h>
#include <bcm/pkt.h>
#include <bcm/policer.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/vlan.h>

#include <bcm_int/sbx/caladan3/g3p1.h>
#include <bcm_int/sbx/caladan3/stat.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stat.h>
#include <shared/idxres_fl.h>
#include <bcm_int/sbx/caladan3/policer.h>
#include <bcm_int/sbx/caladan3/qos.h>
#include <bcm_int/sbx/stack.h>

#include <bcm_int/sbx/caladan3/mpls.h>
#include <shared/hash_tbl.h>

#include <bcm_int/sbx/caladan3/wb_db_mpls.h>

/* Enables the tunnel_id usage for tunnel_switch_add.
 * This tunnel_id is useful in linking PW to LSP ingress configuration.
 */
/*
#define USE_TUNNEL_ID
*/

/* ulVplsColor is 44:38 have a mapping for _NETWORK flag
 * until the user managed vpls-color is enabled in the
 * mpls-port-parameters
 */
#define _CALADAN3_VPLS_COLOR_WAN  0x3F
#define _CALADAN3_VPLS_COLOR_LAN  0x00

#define _CALADAN3_G3P1_MAX_LABELS  (64 * 1024)

#ifdef USE_TUNNEL_ID
#define _BCM_MPLS_SWITCH_SUPPORTED_FLAGS             \
    (BCM_MPLS_SWITCH_OUTER_TTL      |                \
     BCM_MPLS_SWITCH_INNER_TTL      |                \
     BCM_MPLS_SWITCH_TTL_DECREMENT  |                \
     BCM_MPLS_SWITCH_INT_PRI_MAP    |                \
     BCM_MPLS_SWITCH_INT_PRI_SET    |                \
     BCM_MPLS_SWITCH_INNER_EXP      |                \
     BCM_MPLS_SWITCH_DROP           |                \
     BCM_MPLS_SWITCH_OUTER_EXP      |                \
     BCM_MPLS_SWITCH_WITH_ID        |                \
     BCM_MPLS_SWITCH_REPLACE)
#else
#define _BCM_MPLS_SWITCH_SUPPORTED_FLAGS             \
    (BCM_MPLS_SWITCH_OUTER_TTL      |                \
     BCM_MPLS_SWITCH_INNER_TTL      |                \
     BCM_MPLS_SWITCH_TTL_DECREMENT  |                \
     BCM_MPLS_SWITCH_INT_PRI_MAP    |                \
     BCM_MPLS_SWITCH_INT_PRI_SET    |                \
     BCM_MPLS_SWITCH_INNER_EXP      |                \
     BCM_MPLS_SWITCH_DROP           |                \
     BCM_MPLS_SWITCH_OUTER_EXP)
#endif

/**
 * BCM_MPLS_PORT_DROP
 *   There is no way to drop pkts on per vpn-sap basis
 *   for p2etc, pvid2etc ...
 *
 * BCM_MPLS_PORT_SEQUENCED
 * BCM_MPLS_PORT_CONTROL_WORD
 *
 * BCM_MPLS_PORT_INT_PRI_SET
 * BCM_MPLS_PORT_INT_PRI_MAP
 * BCM_MPLS_PORT_COLOR_MAP
 *   cos related work-item
 *
 */
#define _BCM_MPLS_PORT_SUPPORTED_FLAGS          \
    (BCM_MPLS_PORT_REPLACE                  |   \
     BCM_MPLS_PORT_WITH_ID                  |   \
     BCM_MPLS_PORT_NETWORK                  |   \
     BCM_MPLS_PORT_INT_PRI_MAP              |   \
     BCM_MPLS_PORT_INT_PRI_SET              |   \
     BCM_MPLS_PORT_EGRESS_TUNNEL            |   \
     BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP   |   \
     BCM_MPLS_PORT_SERVICE_TAGGED           |   \
     BCM_MPLS_PORT_SERVICE_VLAN_ADD         |   \
     BCM_MPLS_PORT_SERVICE_VLAN_REPLACE     |   \
     BCM_MPLS_PORT_SERVICE_VLAN_DELETE      |   \
     BCM_MPLS_PORT_INNER_VLAN_PRESERVE      |   \
     BCM_MPLS_PORT_ENCAP_WITH_ID            |   \
     BCM_MPLS_PORT_EGRESS_UNTAGGED          |   \
     BCM_MPLS_PORT_SERVICE_VLAN_TPID_REPLACE |  \
     BCM_MPLS_PORT_CONTROL_WORD             |   \
     BCM_MPLS_PORT_DROP                     |   \
     BCM_MPLS_PORT_FAILOVER                 |   \
     BCM_MPLS_PORT_PW_FAILOVER              |   \
     BCM_MPLS_PORT_INNER_VLAN_ADD)

#define _CALADAN3_IS_MPLS_INITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags & _CALADAN3_L3_FE_FLG_MPLS_INIT)

#define _CALADAN3_SET_MPLS_INITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags |= _CALADAN3_L3_FE_FLG_MPLS_INIT)

#define _CALADAN3_SET_MPLS_UNINITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags &= ~_CALADAN3_L3_FE_FLG_MPLS_INIT)


/* Caladan3 g3p1 supports 16bits of mpls label for vsi matching.  However, the
 * MSb signals to the PPE that the label is a PWE3 label
 */
#define _BCM_LABEL_VALID(_label_) \
  ((_label_ & ~_CALADAN3_L3_MPLS_PWE3_LBL_ID) < _CALADAN3_L3_MPLS_LBL_MAX)

#define _BCM_CALADAN3_MPLSTP_LABEL_VALID(_label_)\
  ((_label_) <= _CALADAN3_L3_MPLSTP_LBL_MASK)


#define MPLS_MAX_GPORT (8192) 

#define SB_COMMIT_COMPLETE   0xffffffff

#define _MPLSTP_ILM_FT_SHIFT (14)

int
bcm_caladan3_mpls_tunnel_switch_delete(int                       unit,
                                     bcm_mpls_tunnel_switch_t *info);


/* Gport Database */
STATIC shr_htb_hash_table_t mpls_gport_db[BCM_MAX_NUM_UNITS];

/* Tunnel Switch Info data base */
STATIC shr_htb_hash_table_t mpls_switch_info_db[BCM_MAX_NUM_UNITS];
#ifndef PLATFORM_LABEL
#ifdef USE_TUNNEL_ID
#define MPLS_SWITCH_INFO_KEY_SIZE (4)
#else
#define MPLS_SWITCH_INFO_KEY_SIZE (12)
#endif
#else
#define MPLS_SWITCH_INFO_KEY_SIZE (4)
#endif

bcm_caladan3_mpls_trunk_association_t 
mpls_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

uint32 _sbx_mplstp_lbl_opcode[BCM_MAX_NUM_UNITS][MAX_LBL_OPCODE];


/**
 * Static function declarations.
 */

/*
 * Function:
 *   _bcm_caladan3_mpls_trunk_cb
 * Purpose:
 *     Call back function for Trunk Membership change
 * Returns:
 *   BCM_E_XX
 */ 
STATIC int
_bcm_caladan3_mpls_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data)
{
    int                           status = BCM_E_NONE;
    soc_sbx_g3p1_pv2e_t           pv2e;
    soc_sbx_g3p1_pvv2edata_t      pvv2e;
    bcm_port_t                    tp[BCM_TRUNK_MAX_PORTCNT];
    bcm_port_t                    *tp_rmvd = NULL;
    bcm_port_t                    *tp_added = NULL;
    int                           index=0, mymodid, pindex=0, idx=0, port=0;
    bcm_caladan3_mpls_trunk_association_t *trunkAssoc;
    bcm_trunk_add_info_t         *old_info;
    dq_p_t                        port_elem;
    _caladan3_vpn_sap_t              *mpls_vpn_sap = NULL;
    _caladan3_vpn_control_t          *mpls_vc_vpnc = NULL;
    bcm_mpls_port_t              *mpls_port = NULL;
    uint32                        vsi = 0;
    bcm_vlan_t                    vid, ivid, ovid;
    int                           portMode, rindex=0, aindex=0;

    _caladan3_l3_fe_instance_t       *l3_fe = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    if(!SOC_IS_SBX_G3P1((unit))) {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(unit,
                             "WARNING %s is supported only for G3P1(%s,%d)\n"),
                  FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_UNAVAIL;
    }
    
    if(L3_UNIT_INVALID(unit)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %sUnknown Unit %d\n"),
                   FUNCTION_NAME(), unit));
        return BCM_E_PARAM;
    }
       if(tid >= SBX_MAX_TRUNKS) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Trunk ID  %d !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),tid,__FILE__,__LINE__));
        return BCM_E_PARAM;
    }

    if(!tdata) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Input Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_PARAM;
    }
 
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }

    
    status = bcm_stk_my_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    trunkAssoc = &mpls_trunk_assoc_info[unit][tid];
    old_info = &trunkAssoc->add_info;
    if(tdata->num_ports == 0 && old_info->num_ports == 0) {
        /* nothing to do */
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
 
    tp_added = sal_alloc(sizeof(bcm_port_t) * BCM_TRUNK_MAX_PORTCNT, "tp_added");
    if (tp_added == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %s Could not allocate tp_added memory (%s,%d)\n"),
                   FUNCTION_NAME(),__FILE__,__LINE__));
        L3_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }

    tp_rmvd = sal_alloc(sizeof(bcm_port_t) * BCM_TRUNK_MAX_PORTCNT, "tp_rmvd");
    if (tp_added == NULL) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR %s Could not allocate tp_rmvd memory (%s,%d)\n"),
                   FUNCTION_NAME(),__FILE__,__LINE__));
        sal_free(tp_added);
        L3_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }
    sal_memset(tp_added, 0, sizeof(bcm_port_t));
    sal_memset(tp_rmvd, 0, sizeof(bcm_port_t));

    for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
        tp[index] = -1;
        tp_added[index] = -1;
        tp_rmvd[index] = -1;
    }

    for(index=0; index < tdata->num_ports; index++) {
        if(tdata->tm[index] == mymodid) {
            port = tdata->tp[index];
            idx = 0;
            /* verify if this port was already taken care due to 
             * duplicate trunk distribution */
            while(tp[idx] >= 0) {
                if(port == tp[idx]) {
                    break;
                }
                idx++;
            }

            if(port == tp[idx]) {
                continue;
            }

            tp[pindex++] = port;
        }
    }

    /* remove duplicate from old ports list and add to rmvd list*/
    for(index=0; index < old_info->num_ports; index++) {
        if(old_info->tm[index] == mymodid) {
            port = old_info->tp[index];
            idx = 0;
            /* verify if this port was already taken care due to 
             * duplicate trunk distribution */
            while(tp_rmvd[idx] >= 0) {
                if(port == tp_rmvd[idx]) {
                    break;
                }
                idx++;
            }

            if(port == tp_rmvd[idx]) {
                continue;
            }

            tp_rmvd[rindex++] = port;
        }
    }

    /* find removed and added ports */
    for (index = 0; index < pindex; index++) {
        port = tp[index];
        for (idx = 0; idx < rindex; idx++) {
            if (port == tp_rmvd[idx]) {
                /* port exists in both list, remove from rmvd */
                tp_rmvd[idx] = -1;
                break;
            }
        }
        if (idx != rindex) {
        /* port newly added to the trunk */
            tp_added[aindex++] = port;
        }
    }

    /* copy the trunk add info data */
    trunkAssoc->add_info = *tdata;

    /* Traverse each element of the trunk and update the MPLS port info */
    /* Check if queue is non-empty */
    if(!DQ_EMPTY(&trunkAssoc->plist))
    {
        DQ_TRAVERSE(&trunkAssoc->plist,port_elem) {
            _CALADAN3_VPN_SAP_FROM_TRUNK_LINK(port_elem, mpls_vpn_sap);
           
             /* coverity [dead_error_begin] */ 
             if(mpls_vpn_sap == NULL) {
                sal_free(tp_added);
                sal_free(tp_rmvd);
                 L3_UNLOCK(unit);
                 return BCM_E_INTERNAL;
             }

            /* now the SAP could be of VPLS or VPWS service*/
            /* Handle each case seperately */
            mpls_vc_vpnc = mpls_vpn_sap->vc_vpnc;
            mpls_port = &mpls_vpn_sap->vc_mpls_port;
            /* Extract the VSI as the case may be */
            if(mpls_vc_vpnc->vpn_flags == BCM_MPLS_VPN_VPWS) {
                vsi = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id)
                            - l3_fe->vlan_ft_base;
            } else if(mpls_vc_vpnc->vpn_flags == BCM_MPLS_VPN_VPLS) {
                vsi = mpls_vc_vpnc->vpn_id;
            }

            /* Take action based on the match criteria */
            switch(mpls_port->criteria) {
                case BCM_MPLS_PORT_MATCH_PORT:
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        /* Check if the port mode for MATCH_PORT is set to transparent*/
                        status = bcm_port_dtag_mode_get(l3_fe->fe_unit, 
                                                        port, &portMode);
                        if(portMode != BCM_PORT_DTAG_MODE_TRANSPARENT) {
                             LOG_ERROR(BSL_LS_BCM_MPLS,
                                       (BSL_META_U(unit,
                                                   "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                                        FUNCTION_NAME(),__FILE__,__LINE__));
                             sal_free(tp_added);
                             sal_free(tp_rmvd);
                             L3_UNLOCK(unit);
                             return BCM_E_INTERNAL;
                        }

                       /* get the native VID so we can set the 
                        * proper VID's pv2e entry */

                        status = bcm_port_untagged_vlan_get(l3_fe->fe_unit,
                                                             port, &vid);

                       if (BCM_E_NONE != status) {

                           LOG_ERROR(BSL_LS_BCM_MPLS,
                                     (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) reading nativeVid for port "
                                                  "%d\n"),
                                      bcm_errmsg(status), port));

                           sal_free(tp_added);
                           sal_free(tp_rmvd);
                           L3_UNLOCK(unit);
                           return status;

                       }
                       soc_sbx_g3p1_pv2e_t_init(&pv2e);
                       status = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit,
                                                      port, vid, &pv2e);
                       if (BCM_FAILURE(status)) {
                           LOG_ERROR(BSL_LS_BCM_MPLS,
                                     (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) reading PVid2Etc for port "
                                                  "%d vid %d\n"),
                                      bcm_errmsg(status),
                                      port, vid));
                           sal_free(tp_added);
                           sal_free(tp_rmvd);
                           L3_UNLOCK(unit);
                           return status;
                       }
                       if (index < aindex) {
                           pv2e.vlan = vsi;
                           pv2e.lpi = mpls_vpn_sap->logicalPort;
                       } else {
                           /* delete pv2e */
                           pv2e.vlan = vid;
                           pv2e.lpi = 0;
                       }
                       status = _soc_sbx_g3p1_pv2e_set(l3_fe->fe_unit, port,
                                                       vid, &pv2e);

                       if (BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                      (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) writing pv2e(0x%x,0x%x)\n"),
                                       bcm_errmsg(status), port, vid));
                             sal_free(tp_added);
                             sal_free(tp_rmvd);
                             L3_UNLOCK(unit);
                             return status;
                       }

                    }
                break;

                case BCM_MPLS_PORT_MATCH_PORT_VLAN:
                    vid = mpls_port->match_vlan;
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        soc_sbx_g3p1_pv2e_t_init(&pv2e);
                        status = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, 
                                                       port, vid, &pv2e);
                        if (BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                      (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) reading PVid2Etc for port "
                                                  "%d vid %d\n"),
                                       bcm_errmsg(status), 
                                       port, vid));
                            sal_free(tp_added);
                            sal_free(tp_rmvd);
                            L3_UNLOCK(unit);
                            return status;
                        }
                        if (index < aindex) {
                            pv2e.vlan = vsi;
                            pv2e.lpi = mpls_vpn_sap->logicalPort;
                        } else {
                           /* delete pv2e */
                           pv2e.vlan = vid;
                           pv2e.lpi = 0;
                        }
                        status = _soc_sbx_g3p1_pv2e_set(l3_fe->fe_unit,
                                                        port, vid, &pv2e);

                       if (BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                      (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) writing pv2e(0x%x,0x%x)\n"),
                                       bcm_errmsg(status), port, vid));
                             sal_free(tp_added);
                             sal_free(tp_rmvd);
                             L3_UNLOCK(unit);
                             return status;
                       }

                    }

                break;

                case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
                    ivid = mpls_port->match_inner_vlan;
                    ovid = mpls_port->match_vlan;
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        soc_sbx_g3p1_pvv2edata_t_init(&pvv2e);
                        if (index < aindex) {
                            pvv2e.lpi = mpls_vpn_sap->logicalPort;
                            pvv2e.vlan = vsi;
                            status = soc_sbx_g3p1_util_pvv2e_add(l3_fe->fe_unit,
                                                            port, ovid, ivid,
                                                            &pvv2e);
                        } else {
                            /* delete pv2e */
                            status = soc_sbx_g3p1_util_pvv2e_remove(l3_fe->fe_unit,
                                                        port, ovid, ivid);
                        }
                        if (BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                      (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) writing pvv2e(0x%x,0x%x,0x%x)\n"),
                                       bcm_errmsg(status), port, ivid, ovid));
                            sal_free(tp_added);
                            sal_free(tp_rmvd);
                            L3_UNLOCK(unit);
                            return status;
                        }
#if !CALADAN3_UNPORTED
                        status = soc_sbx_g3p1_pvv2e_commit(l3_fe->fe_unit, 
                                                        SB_COMMIT_COMPLETE);
#else
                        status = BCM_E_NONE;
#endif /* !CALADAN3_UNPORTED */
                        if (BCM_FAILURE(status)) {
                            /* coverity[dead_error_begin] */
                            LOG_ERROR(BSL_LS_BCM_MPLS,
                                      (BSL_META_U(l3_fe->fe_unit,
                                                  "error(%s) calling soc_sbx_g3p1_pvv2e_commit "
                                                  "(port=%d, innerVid=%d outerVid=%d)\n"),
                                       bcm_errmsg(status), port, ivid, ovid));
                 
                            sal_free(tp_added);
                            sal_free(tp_rmvd);
                            L3_UNLOCK(unit);
                            return status;
                        }

                    }
                break;

                default:
                break;
            }

        }DQ_TRAVERSE_END(&trunkAssoc->plist,port_elem);
    }
    
    sal_free(tp_added);
    sal_free(tp_rmvd);
    L3_UNLOCK(unit);    
    return status;
}


STATIC int
_bcm_caladan3_invalidate_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t *l3_fe,
                                            _caladan3_vpn_sap_t    *vpn_sap);
STATIC int
_bcm_caladan3_mpls_get_fte(_caladan3_l3_fe_instance_t *l3_fe,
                         uint32                  fte_idx,
                         int                     action,
                         _caladan3_l3_or_mpls_egress_t *egr);

STATIC int
_bcm_caladan3_link_vpn_sap_ufte(_caladan3_l3_fe_instance_t  *l3_fe,
                              _caladan3_vpn_sap_t         *vpn_sap,
                              bcm_module_t             caller_module);
STATIC int
_bcm_caladan3_update_vpn_sap_hw(_caladan3_l3_fe_instance_t  *l3_fe,
                              bcm_module_t             caller_module,
                              _caladan3_vpn_sap_t         *vpn_sap,
                              bcm_mpls_port_t         *mpls_port);
STATIC int
_bcm_caladan3_mpls_port_delete(int                      unit,
                             _caladan3_l3_fe_instance_t  *l3_fe,
                             _caladan3_vpn_control_t     *vpnc,
                             _caladan3_vpn_sap_t        **vpn_sap);
STATIC int
_bcm_caladan3_fill_mpls_vpn_config(int                     unit,
                                 _caladan3_vpn_control_t    *vpnc,
                                 bcm_mpls_vpn_config_t  *info);

STATIC int
_bcm_caladan3_alloc_mpls_vpn_sap(_caladan3_l3_fe_instance_t  *l3_fe,
                               _caladan3_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _caladan3_vpn_sap_t      **vpn_sap);


STATIC int
_bcm_caladan3_free_mpls_vpn_sap(_caladan3_l3_fe_instance_t  *l3_fe,
                              _caladan3_vpn_sap_t      **vpn_sap);

int
bcm_caladan3_mpls_vpn_id_create(int                    unit,
                              bcm_mpls_vpn_config_t *info);
int
bcm_caladan3_mpls_port_add(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port);


STATIC void
_bcm_caladan3_mpls_switch_key_get(void *key, 
                                  bcm_mpls_label_t label1, 
                                  bcm_mpls_label_t label2,
                                  bcm_gport_t port,
                                  bcm_gport_t tunnel_id)
{
    uint8 *loc = key;
#ifndef PLATFORM_LABEL
#ifdef USE_TUNNEL_ID
    assert (MPLS_SWITCH_INFO_KEY_SIZE >= sizeof(bcm_gport_t));
    sal_memcpy(loc, &tunnel_id, sizeof(bcm_gport_t));
#else
    assert (MPLS_SWITCH_INFO_KEY_SIZE 
                >= (2*sizeof(bcm_mpls_label_t) + sizeof(bcm_gport_t)));
    sal_memcpy(loc, &label1, sizeof(bcm_mpls_label_t));
    loc = loc + sizeof(bcm_mpls_label_t);
    sal_memcpy(loc, &label2, sizeof(bcm_mpls_label_t));
    loc = loc + sizeof(bcm_mpls_label_t);
    sal_memcpy(loc, &port, sizeof(bcm_gport_t));
#endif
#else
    assert (MPLS_SWITCH_INFO_KEY_SIZE 
                >= (sizeof(bcm_mpls_label_t)));
    sal_memcpy(loc, &label1, sizeof(bcm_mpls_label_t));
#endif
}

/*
 * Function:
 *      _bcm_caladan3_fill_mpls_label_array_from_ete_idx
 * Purpose:
 *      fill label array based on the ete-idx
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      ete_idx     - h/w mpls ete index
 *      label_max   - label_array size
 *      label_array - egress label array
 *      label_count - filled number of entries
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */
static int
_bcm_caladan3_fill_mpls_label_array_from_ete_idx(_caladan3_l3_fe_instance_t *l3_fe,
                                               uint32                   ete_idx,
                                               int                      label_max,
                                               bcm_mpls_egress_label_t *label_array,
                                               int                     *label_count)
{
    int                      rv;
    soc_sbx_g3p1_ete_t  encap_ete;

    *label_count = 0;
    rv = soc_sbx_g3p1_ete_get(l3_fe->fe_unit, ete_idx, &encap_ete);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error reading ete encap(0x%x): %s\n"),
                   ete_idx, bcm_errmsg(rv)));
        return rv;
    }
    
#define BCM_CALADAN3_G3P1_LABEL_ADD(_label_num_)  \
    if (encap_ete.label##_label_num_ && (*label_count < label_max)) {   \
        label_array->label  = encap_ete.label##_label_num_;             \
        label_array->exp    = encap_ete.exp##_label_num_;               \
        label_array->ttl    = encap_ete.ttl##_label_num_;               \
        label_array->flags  = 0;                                        \
        if (encap_ete.exp##_label_num_##remark) {                       \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_EXP_REMARK;     \
        } else {                                                        \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;        \
        }                                                               \
                                                                        \
        if (encap_ete.ttl##_label_num_##dec) {                          \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;  \
        } else {                                                        \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_TTL_SET;        \
        }                                                               \
                                                                        \
        LOG_VERBOSE(BSL_LS_BCM_MPLS, \
                    (BSL_META_U(l3_fe->fe_unit, \
                                "Added label%d at %d:0x%x\n"),          \
                     _label_num_, *label_count, label_array->label));   \
        label_array++;                                                  \
        (*label_count)++;                                               \
    }

    BCM_CALADAN3_G3P1_LABEL_ADD(0);
    BCM_CALADAN3_G3P1_LABEL_ADD(1);
    BCM_CALADAN3_G3P1_LABEL_ADD(2);

#undef BCM_CALADAN3_G3P1_LABEL_ADD

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_update_vpn_sap_hw
 * Purpose:
 *      allocate if reqd, map and set ohi, ete for vpn sap
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      caller_module - modid of remote caller or fe_my_modid
 *      vpn_sap     -  vpn service attachment point
 *      mpls_port   -  bcm provided port info
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_update_vpn_sap_hw(_caladan3_l3_fe_instance_t  *l3_fe,
                              bcm_module_t             caller_module,
                              _caladan3_vpn_sap_t         *vpn_sap,
                              bcm_mpls_port_t         *mpls_port)
{
    int status = 0, ignore_status;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));
    vpn_sap->vc_res_alloced = 0;

    /**
     * XXX: TBD: How will back 2 back router with PHP support work with
     * VC labels ?
     * (8/2/07)
     * new design for mpls-ete via mpls-switch-add,
     * see if that solves this
     * ucode reqd for having vsi <--> fte
     *
     */

    if (_BCM_CALADAN3_IS_PWE3_TUNNEL(mpls_port->flags)) {

        /**
         * This routine programs the P side for VPLS and VPWS
         * i.e. the vc_label over tunnel.
         */

        switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
        {
            
        case SOC_SBX_UCODE_TYPE_G3P1:
            status = _bcm_caladan3_g3p1_mpls_update_vpxs_tunnel_hw(l3_fe,
                                                                 vpn_sap,
                                                                 mpls_port);
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_INTERNAL;
            break;
        }

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) updating ucode tables\n"),
                       bcm_errmsg(status)));
            goto error_out;
        }

    } else {

        /**
         * (i)  LAN side of VPLS/VPWS
         * (ii) L1 port
         */
        switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
        {
        case SOC_SBX_UCODE_TYPE_G3P1:
            status = _bcm_caladan3_g3p1_mpls_update_vpxs_hw(l3_fe,
                                                          vpn_sap,
                                                          mpls_port);
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_INTERNAL;
            break;
        }

        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) updating ucode tables\n"),
                       bcm_errmsg(status)));
            goto error_out;
        }

    }

    status = _bcm_caladan3_link_vpn_sap_ufte(l3_fe,
                                           vpn_sap,
                                           caller_module);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) linking vpn-sap ufte\n"),
                   bcm_errmsg(status)));
        goto error_out;
    }

    return BCM_E_NONE;

error_out:

    if (vpn_sap->vc_res_alloced) {
        ignore_status =
            _bcm_caladan3_invalidate_vpn_sap_hw_resources(l3_fe,
                                                        vpn_sap);
        COMPILER_REFERENCE(ignore_status);
    }

    return status;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel ete (add/update)
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 *      l3_ete      - (IN)  l3 ete for which the tunnel ete is being made
 *      num_labels  - (IN)  Number of labels in the array
 *      label_array - (IN)  Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_mpls_tunnel_initiator_set(_caladan3_l3_fe_instance_t  *l3_fe,
                                      _caladan3_l3_intf_t         *l3_intf,
                                      _caladan3_l3_ete_t          *l3_ete, /* may be NULL */
                                      int                      num_labels,
                                      bcm_mpls_egress_label_t *label_array)
{
    _caladan3_l3_ete_t            *mpls_sw_ete;
#if 0
    int                        status, ignore_status;
#else
    int                        status;
#endif
    _caladan3_l3_ete_key_t         tkey;
    int                        ete_allocated;
    bcm_mac_t                  mac_addr;
    int                        i;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    /*
     * Note that the v4-ete may not exist (i.e. the l3_ete parameter may be NULL)
     */
    if (l3_ete == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "Mpls tunnel initiator set called on interface (0x%x), "
                                 " but no v4 ete present\n"),
                     _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
    }

    mpls_sw_ete   = NULL;
    ete_allocated = 0;
    sal_memset(mac_addr, 0, sizeof(bcm_mac_t)); /* dummy mac if v4-ete is not there */

    /*
     * first, see if a mpls ete exists. i.e. figure out the add/update case
     */
    status   = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe, l3_intf,
                                                   _CALADAN3_L3_ETE__ENCAP_MPLS,
                                                   &mpls_sw_ete);
    if (status == BCM_E_NOT_FOUND) {
        /*
         * 1. mpls ete does not exists => Add case: But there are 2 sub cases.
         *    case a: v4 ete exists
         *    case b: v4 ete does not exist - In this case we will use
         *            all-zero mac address and a NOP for vidop.
         *            When the v4-ete is created, the modify function
         *            will do the fix-up.
         */
        if (l3_ete != NULL) {
            _CALADAN3_MAKE_ENCAP_MPLS_SW_ETE_KEY(&tkey,
                                             l3_ete->l3_ete_key.l3_ete_hk.dmac);
        } else {
            _CALADAN3_MAKE_ENCAP_MPLS_SW_ETE_KEY(&tkey,
                                             mac_addr);
        }

        status = _bcm_caladan3_alloc_default_l3_ete(l3_fe, l3_intf, &tkey,
                                                  0,        /* OHI */
                                                  &mpls_sw_ete);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not allocate MPLS encap ete on "
                                   "interface (0x%x)\n"),
                       bcm_errmsg(status),
                       _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
            return status;
        }

        mpls_sw_ete->l3_intf_id = l3_intf->if_info.l3a_intf_id;
        ete_allocated  = 1;
        DQ_INIT(&mpls_sw_ete->l3_vc_ete_head);
    }
    /* 
     * Tunnel Ingress Drop
     * If any of the labels in the array has DROP set, flag the interface
     * Clear existing flag, set again if required
     */
    l3_intf->if_flags &= ~_CALADAN3_L3_MPLS_TUNNEL_DROP;
    for (i=0; i< num_labels; i++) {
        if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_DROP) {
            l3_intf->if_flags |= _CALADAN3_L3_MPLS_TUNNEL_DROP;
        }
    }

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G3P1:
        return (_bcm_caladan3_g3p1_mpls_tunnel_ete_set(l3_fe->fe_unit,
                                                     l3_fe,
                                                     l3_intf,
                                                     l3_ete,
                                                     num_labels,
                                                     label_array,
                                                     mpls_sw_ete,
                                                     ete_allocated));

   default:
       SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
       return BCM_E_CONFIG;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_tunnel_initiator_clear
 * Purpose:
 *       to remove the mpls tunnel ete from the l3_intf
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_caladan3_mpls_l3_initiator_clear(_caladan3_l3_fe_instance_t  *l3_fe,
                                      _caladan3_l3_intf_t         *l3_intf)
{
    int                       rv;
    _caladan3_l3_ete_t           *mpls_sw_ete;
    _caladan3_l3_ete_t           *l3_ete;
    bcm_if_t                  ifid;

    ifid = _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id);
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    /* validate the interface deleting */
    mpls_sw_ete = NULL;
    rv =_bcm_caladan3_get_ete_by_type_on_intf(l3_fe, l3_intf,
                                              _CALADAN3_L3_ETE__ENCAP_MPLS,
                                            &mpls_sw_ete);

    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(l3_fe->fe_unit,
                             "mpls tunnel does not exist on "
                              "interface id (0x%x): %s\n"),
                  ifid, bcm_errmsg(rv)));
        return rv;
    }

    if (!DQ_EMPTY(&mpls_sw_ete->l3_vc_ete_head)) {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(l3_fe->fe_unit,
                             "mpls tunnel is on interface id (0x%x) "
                              "is being used by VCs\n"),
                  ifid));
        return BCM_E_BUSY;
    }

    l3_intf->if_flags &= ~_CALADAN3_L3_MPLS_TUNNEL_SET;

    /*
     * Finally free the mpls ete
     */
    rv = _bcm_caladan3_undo_l3_ete_alloc(l3_fe, l3_intf, &mpls_sw_ete);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls tunnel on Interface Id (0x%x) could not "
                               "be destroyed\n"),
                   ifid));
        return rv;
    }

    if (l3_intf->if_flags & _CALADAN3_L3_INTF_IN_EGR_LBL_LIST) {
        DQ_REMOVE(&l3_intf->if_egr_lbl_link);
    }

    /*
     * If there is a valid v4 ete, revert to that
     */
    rv = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                             l3_intf,
                                             _CALADAN3_L3_ETE__UCAST_IP,
                                             &l3_ete);
    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) finding ipv4 ete on intf(0x%x)\n"),
                   bcm_errmsg(rv), ifid));
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_check_mpls_initiator_params
 * Purpose:
 *      Validate params given for mpls initiator
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      l3_intf     - the egress L3 interface context
 *      num_labels  - number of labels in label_array
 *      label_array - label array
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_check_mpls_initiator_params(_caladan3_l3_fe_instance_t  *l3_fe,
                                          _caladan3_l3_intf_t         *l3_intf,
                                          int                      num_labels,
                                          bcm_mpls_egress_label_t *label_array)
{
    int      i;
    int      qosMapId;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    if ((num_labels <= 0) || (num_labels > _CALADAN3_MAX_MPLS_TNNL_LABELS)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid  number of labels (%d)\n"),
                   num_labels));
        return BCM_E_PARAM;
    }

    if (label_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "label_array is null\n")));
        return BCM_E_PARAM;
    }

    if ((label_array->label == _BCM_MPLS_IMPLICIT_NULL_LABEL) &&
        (num_labels != 1)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid number of labels(%d) for label(0x%x)\n"),
                   num_labels, label_array->label));
        return BCM_E_PARAM;
    }

    /*
     *  For right now, since we can only have one qosMapId, this value must be
     *  the same (thence, both qos_map_id and flags.remark must be the same for
     *  all labels in the stack.  We'll check that later; this just loads the
     *  value for the outer label.
     */
    qosMapId = label_array->qos_map_id;

    for (i = 0; i < num_labels; i++) {

        /**
         * Two modes are supported for TTL ops
         * ERH.ttl - 1 OR setting a specific value
         */
        if (label_array->flags & BCM_MPLS_EGRESS_LABEL_TTL_COPY) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "TTL_COPY is not supported\n")));
            return BCM_E_PARAM;
        }

        if ((label_array->flags & (BCM_MPLS_EGRESS_LABEL_EXP_SET    |
                                   BCM_MPLS_EGRESS_LABEL_EXP_COPY)) &&
            !_BCM_EXP_VALID(label_array->exp)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "label_array[%d] invalid exp\n"),
                       i));
            return BCM_E_PARAM;
        }

        if (label_array->qos_map_id != qosMapId) {
            /* qos_map_id disagrees with outer layer */
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "label_array[%d] qos_map_id (%08X)"
                                   " disagrees with label_array[0] qos_map_id (0x%08X)\n"),
                       i,label_array->qos_map_id, qosMapId));
            return BCM_E_PARAM;
        }

        if ((label_array->flags & BCM_MPLS_EGRESS_LABEL_PRI_SET) &&
            !_BCM_PKT_PRI_VALID(label_array->pkt_pri)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "label_array[%d] invalid pri\n"),
                       i));
            return BCM_E_PARAM;
        }

        if (!_BCM_PKT_CFI_VALID(label_array->pkt_cfi)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "label_array[%d] invalid pkt_cfi\n"),
                       i));
            return BCM_E_PARAM;
        }

        if (!_CALADAN3_QOS_MAP_ID_VALID(label_array->qos_map_id)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "label_array[%d] invalid qos_map_id\n"),
                       i));
            return BCM_E_PARAM;
        }

        label_array++;
    }

    /**
     * Only one tunnel is allowed on an interface at one time
     * so if the interface has a v4 encap tunnel configured, return
     * an error
     */
    if (l3_intf->if_tunnel_info) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "interface (0x%x) already has a v4encap tunnel\n"),
                   _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
        return BCM_E_PARAM;
    }

    /**
     * If there is more than one egress object on this intf then return an error.
     */
    if (l3_intf->if_ip_ete_count > 1) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "cannot have more than one egress object on interface "
                               "and enable mpls-tunnel\n")));
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_caladan3_mpls_alloc_vpncb(_caladan3_l3_fe_instance_t  *l3_fe,
                             _caladan3_vpn_control_t    **vpnc)
{

    *vpnc = sal_alloc(sizeof(_caladan3_vpn_control_t),
                      "MPLS-vpncb");
    if (*vpnc == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(*vpnc, 0, sizeof(_caladan3_vpn_control_t));

    (*vpnc)->vpn_id   = _CALADAN3_INVALID_VPN_ID;
    (*vpnc)->vpn_vrf  = _CALADAN3_INVALID_VRF;

    DQ_INIT(&(*vpnc)->vpn_sap_head);
    DQ_INIT(&(*vpnc)->vpn_fe_link);

    return BCM_E_NONE;
}



int
_bcm_caladan3_mpls_free_vpncb(_caladan3_l3_fe_instance_t  *l3_fe,
                            _caladan3_vpn_control_t    **vpnc)
{
    int rv = BCM_E_UNAVAIL;
    
    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_free_vpncb(l3_fe, vpnc);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

STATIC int
_bcm_caladan3_destroy_mpls_vpn_id(int                    unit,
                                _caladan3_l3_fe_instance_t  *l3_fe,
                                _caladan3_vpn_control_t    **vpnc)
{
    int                           status;
    int                           vrf;

    vrf    = (*vpnc)->vpn_vrf;
    status = _bcm_caladan3_mpls_free_vpncb(l3_fe, vpnc);
    if (status != BCM_E_NONE) {
        return status;
    }

    if (vrf != BCM_L3_VRF_DEFAULT && _BCM_VRF_VALID(vrf)) {
        l3_fe->fe_vpn_by_vrf[vrf] = NULL;
    }

    return BCM_E_NONE;
}

static int
_bcm_caladan3_mplstp_init(int unit)
{
    int index=0;
    uint32 xt_index=0;
    soc_sbx_g3p1_v2e_t v2e;
    soc_sbx_g3p1_xt_t  xt;    /* exception table */  
#ifdef BCM_WARM_BOOT_SUPPORT
    int status;
#endif
    
    for(index=0;index<MAX_LBL_OPCODE;index++) {
        _sbx_mplstp_lbl_opcode[unit][index] = MAX_LBL_OPCODE;
    }

    /* fill in supported outer label opcode */
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_label_ler_get(unit,&_BCM_CALADAN3_LABEL_LER(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_label_lsr_get(unit,&_BCM_CALADAN3_LABEL_LSR(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_label_eth_pwe3_get(unit,&_BCM_CALADAN3_LABEL_PWE(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_label_ces_pwe3_get(unit,&_BCM_CALADAN3_LABEL_CES(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_exc_dcn_idx_get(unit, &xt_index));

    /* initialize reserved mplstp vpws vsi */
    BCM_IF_ERROR_RETURN(
       soc_sbx_g3p1_vpws_vlan_set(unit, _SBX_CALADAN3_MPLSTP_RSVD_VSI));

    soc_sbx_g3p1_v2e_t_init(&v2e);
    v2e.dontlearn = 1;
    SOC_IF_ERROR_RETURN(soc_sbx_g3p1_v2e_set(unit, _SBX_CALADAN3_MPLSTP_RSVD_VSI, &v2e));

    soc_sbx_g3p1_xt_t_init(&xt);
    BCM_IF_ERROR_RETURN
      (soc_sbx_g3p1_xt_get(unit, xt_index, &xt));

    xt.forward = 1;

    BCM_IF_ERROR_RETURN
      (soc_sbx_g3p1_xt_set(unit, xt_index, &xt));

#ifdef BCM_WARM_BOOT_SUPPORT

    status = bcm_caladan3_wb_mpls_state_init(unit);

    if (SOC_WARM_BOOT(unit)) {
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) warmboot init MPLSTP\n"),
                       bcm_errmsg(status)));
        }
        return status;        
    }

#endif   

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_init
 * Purpose:
 *      Initialize mpls module
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_mpls_init(int                     unit,
                      _caladan3_l3_fe_instance_t *l3_fe)
{
    int                           idx, status;
    int                           hidx;
    _caladan3_vpn_control_t          *vpnc;
    int i,j, rv;
    uint32 vpws_vpn_low, vpws_vpn_high;
    uint32 vpws_pw_gport_low, vpws_pw_gport_high, vpws_pw_gport;

    /**
     * Initialize the various DQ
     */
    for (idx = 0; idx < _CALADAN3_OHI_IDX_HASH_SIZE; idx++) {
        DQ_INIT(&l3_fe->fe_ohi2_vc_ete[idx]);
    }

    for (idx = 0; idx < _CALADAN3_VPN_HASH_SIZE; idx++) {
        DQ_INIT(&l3_fe->fe_vpn_hash[idx]);
    }

    sal_memset(&l3_fe->fe_vpn_by_vrf[0],
               (int)NULL,
               sizeof(_caladan3_vpn_control_t *) * SBX_MAX_VRF);

    /*
     * Now create the vpnc for BCM_L3_VRF_DEFAULT. This way
     * other code in mpls does not need to special case
     * for info->vpn being BCM_L3_VRF_DEFAULT
     *
     * For Warmboot, the default VRF VPN will be retrieved
     * instead of alloc
     */
    if (!SOC_WARM_BOOT(unit)) {
        status = _bcm_caladan3_mpls_alloc_vpncb(l3_fe,
                                              &vpnc);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) allocating new vpn control block\n"),
                       bcm_errmsg(status)));
            return status;
        }

        /* init vpnc and insert into fe_vpn_hash */
        vpnc->vpn_id    = l3_fe->fe_vsi_default_vrf;
        vpnc->vpn_vrf   = BCM_L3_VRF_DEFAULT;
        vpnc->vpn_flags = BCM_MPLS_VPN_L3;

        hidx = _CALADAN3_GET_MPLS_VPN_HASH(vpnc->vpn_id);
        DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx],
                       &vpnc->vpn_fe_link);
        if (_BCM_VRF_VALID(vpnc->vpn_vrf)) {
            l3_fe->fe_vpn_by_vrf[vpnc->vpn_vrf] = vpnc;
        }
    }

    /* initialize mpls gport database */
    if (mpls_gport_db[unit] != NULL) {
        status = shr_htb_destroy(&mpls_gport_db[unit], NULL);
        mpls_gport_db[unit] = NULL;
    }
    status = shr_htb_create(&mpls_gport_db[unit],
                   MPLS_MAX_GPORT,
                   sizeof(uint32),
                   "MPLS Gport Dbase");
    if( BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) allocating MPLS gport data base\n"),
                   bcm_errmsg(status)));
        return status;
    }
    /* initialize mpls switch_info database */
    if (mpls_switch_info_db[unit] != NULL) {
        status = shr_htb_destroy(&mpls_switch_info_db[unit], NULL);
        mpls_switch_info_db[unit] = NULL;
    }
    status = shr_htb_create(&mpls_switch_info_db[unit],
                   (8*1024),
                   MPLS_SWITCH_INFO_KEY_SIZE,
                   "MPLS switch_info Dbase");
    if( BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) allocating MPLS switch_info data base\n"),
                   bcm_errmsg(status)));
        return status;
    }


    /* clear trunk association structure */
    for(i=0; i < BCM_MAX_NUM_UNITS; i++) {
        for(j=0; j < SBX_MAX_TRUNKS; j++) {
            /* initialize the port list */
            DQ_INIT(&mpls_trunk_assoc_info[i][j].plist);
            bcm_trunk_add_info_t_init(&mpls_trunk_assoc_info[i][j].add_info);
        }
    }

    /* register callback for trunk change */
    status = bcm_caladan3_trunk_change_register(unit, _bcm_caladan3_mpls_trunk_cb, NULL);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) registering trunk change callback\n"),
                   bcm_errmsg(status)));
        return status;
    }
     
    status = _bcm_caladan3_mplstp_init(unit);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) initializing MPLSTP\n"),
                   bcm_errmsg(status)));
        return status;
    }

    vpws_vpn_low       = soc_property_get(unit, spn_SBX_VPWS_VPN_LOW , 0); 
    vpws_vpn_high      = soc_property_get(unit, spn_SBX_VPWS_VPN_HIGH, 0); 
    vpws_pw_gport_low  = soc_property_get(unit, spn_SBX_VPWS_PW_GPORTS_LOW, 0); 
    vpws_pw_gport_high = soc_property_get(unit, spn_SBX_VPWS_PW_GPORTS_HIGH, 0);
    if (vpws_vpn_low && vpws_vpn_high) {
        LOG_CLI((BSL_META_U(unit,
                            "Reserving vpws_vpn (unit:%d): 0x%x - 0x%x\n"),
                 unit, vpws_vpn_low, vpws_vpn_high));
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_LINE_VSI,
                               FALSE, (uint32) vpws_vpn_low);
        if (rv != BCM_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "VPWS VPN Reserve failed:")));
            LOG_CLI((BSL_META_U(unit,
                                " unit %d vpn_low 0x%x Error %d\n"),
                     unit, vpws_vpn_low, rv));
        } else {
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_LINE_VSI,
                                        TRUE, (uint32) vpws_vpn_high);
            if (rv != BCM_E_NONE) {
                LOG_CLI((BSL_META_U(unit,
                                    "VPWS VPN Reserve failed:")));
                LOG_CLI((BSL_META_U(unit,
                                    " unit %d vpn_high 0x%x Error %d\n"),
                         unit, vpws_vpn_high, rv));
                _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_LINE_VSI,
                                       FALSE, 0);
            }
        }
    }
    if (vpws_pw_gport_low && vpws_pw_gport_high) {
        LOG_CLI((BSL_META_U(unit,
                            "Reserving vpws_pw_gport (unit:%d): 0x%x - 0x%x\n"),
                 unit, vpws_pw_gport_low, vpws_pw_gport_high));
        vpws_pw_gport = BCM_GPORT_MPLS_PORT_ID_GET(vpws_pw_gport_low);
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_MPLS,
                               FALSE, (uint32) vpws_pw_gport);
        if (rv != BCM_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "VPWS PW Gports Reserve failed:")));
            LOG_CLI((BSL_META_U(unit,
                                " unit %d vpn_pw_gport_low 0x%x Error %d\n"),
                     unit, vpws_pw_gport_low, rv));
        } else {
           vpws_pw_gport = BCM_GPORT_MPLS_PORT_ID_GET(vpws_pw_gport_high);
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_MPLS,
                                        TRUE, (uint32) vpws_pw_gport);
            if (rv != BCM_E_NONE) {
                LOG_CLI((BSL_META_U(unit,
                                    "VPWS PW Gports Reserve failed:")));
                LOG_CLI((BSL_META_U(unit,
                                    " unit %d vpn_pw_gport_high 0x%x Error %d\n"),
                         unit, vpws_pw_gport_high, rv));
                rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_MPLS,
                                            FALSE, 0);

            }
        }
    }

    _CALADAN3_SET_MPLS_INITIALIZED(l3_fe);

    return status;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_cleanup
 * Purpose:
 *      Cleanup mpls module
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_mpls_cleanup(int                     unit,
                         _caladan3_l3_fe_instance_t *l3_fe)
{
    int status,idx;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    /**
     * XXX: Cleanup all the memory allocated and resources
     */
    for (idx = 0; idx < _CALADAN3_OHI_IDX_HASH_SIZE; idx++) {
        if (!DQ_EMPTY(&l3_fe->fe_ohi2_vc_ete[idx])) {
            LOG_WARN(BSL_LS_BCM_MPLS,
                     (BSL_META_U(l3_fe->fe_unit,
                                 "Found unfreed vc ete at idx %d\n"),
                      idx));
            /* return BCM_E_BUSY; */
        }
    }

    for (idx = 0; idx < SBX_MAX_VRF; idx++) {
        if (l3_fe->fe_vpn_by_vrf[idx] != NULL) {
            LOG_WARN(BSL_LS_BCM_MPLS,
                     (BSL_META_U(l3_fe->fe_unit,
                                 "Found unfreed vpn vrf at idx %d\n"), 
                      idx));
            /* return BCM_E_BUSY;*/
        }
    }

    for (idx = 0; idx < _CALADAN3_VPN_HASH_SIZE; idx++) {
        if (!DQ_EMPTY(&l3_fe->fe_vpn_hash[idx])) {
            LOG_WARN(BSL_LS_BCM_MPLS,
                     (BSL_META_U(l3_fe->fe_unit,
                                 "Found unfreed vpn hash idx %d\n"),
                      idx));
            /* return BCM_E_BUSY; */
        }
    }

    sal_memset(l3_fe->fe_vpn_by_vrf,
               (int)NULL,
               sizeof(_caladan3_vpn_control_t *)*SBX_MAX_VRF);

    /* initialize mpls gport database */
    status = shr_htb_destroy(&mpls_gport_db[unit], NULL);
    if( BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) freeing MPLS gport data base\n"),
                   bcm_errmsg(status)));
    }
    /* initialize mpls switch_info database */
    status = shr_htb_destroy(&mpls_switch_info_db[unit], NULL);
    if( BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) freeing MPLS switch_info data base\n"),
                   bcm_errmsg(status)));
    }


    _CALADAN3_SET_MPLS_UNINITIALIZED(l3_fe);

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_caladan3_mpls_find_vpn_sap_by_gport
 * Purpose:
 *      Find vpn sap for a gport given gport
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      gport       - gport to find
 *      vpn_sap     - sap found
 * Note:
 *    The _CALADAN3_ALL_VPNC and _CALADAN3_ALL_VPN_SAP_PER_VPNC macros CANNOT be
 *   used in conjunction with 'break' statements due to the implicit
 *   loops defined by the macros.
 *
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_caladan3_mpls_find_vpn_sap_by_gport(_caladan3_l3_fe_instance_t *l3_fe, 
                                       bcm_gport_t             gport,
                                       _caladan3_vpn_sap_t       **vpn_sap) 
{
    int status = BCM_E_NONE;
/*
    _caladan3_vpn_control_t          *vpnc = NULL;
    
    _CALADAN3_ALL_VPNC(l3_fe, vpnc) {
        _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, (*vpn_sap)) {
            if (gport == (*vpn_sap)->vc_mpls_port_id) {
                return TRUE;
            }
        } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, (*vpn_sap));
    } _CALADAN3_ALL_VPNC_END(l3_fe, vpnc);
*/
    status = bcm_caladan3_mpls_gport_get(l3_fe->fe_unit, gport, vpn_sap);
    if (BCM_FAILURE(status)) {
        if (vpn_sap) {
            *vpn_sap = NULL;
        }
        return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_free_vpncb_for_vlan_vsi
 * Purpose:
 *      Free as VPNCB for the vlan
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_free_vpncb_for_vlan_vsi(int                     unit,
                                    _caladan3_l3_fe_instance_t  *l3_fe,
                                    _caladan3_vpn_control_t     **vpnc)
{

    DQ_REMOVE(&(*vpnc)->vpn_fe_link);
    sal_free(*vpnc);
    *vpnc = NULL;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_caladan3_alloc_vpncb_for_vlan_vsi
 * Purpose:
 *      Allocates as VPNCB for the vlan
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vlan_vsi    - VLAN ID
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

static int
_bcm_caladan3_alloc_vpncb_for_vlan_vsi(int                     unit,
                                  _caladan3_l3_fe_instance_t  *l3_fe,
                                  bcm_vlan_t              vlan_vsi,
                                  _caladan3_vpn_control_t     **vpnc)
{
    int status;
    int hidx;

    status = _bcm_caladan3_mpls_alloc_vpncb(l3_fe, vpnc);
    if (status != BCM_E_NONE) {
        return status;
    }

    (*vpnc)->vpn_bc_mcg = vlan_vsi;
    (*vpnc)->vpn_id     = vlan_vsi;
    (*vpnc)->vpn_flags  = BCM_MPLS_VPN_VPLS; /* Only VPLS */
    (*vpnc)->vpls_color =  SBX_VPLS_COLOR_FTE_BASE(l3_fe->fe_unit);
    hidx = _CALADAN3_GET_MPLS_VPN_HASH(vlan_vsi);
    
    DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx], &((*vpnc)->vpn_fe_link));
    /*
     * Nothing more to do:
     * In this case, the vlan id was already created externally
     * and mcast ftes were set up at that time.
     */

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_find_mpls_vpncb_by_id
 * Purpose:
 *      Find vpn control block based on vpn-id
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vpn_id      - vpn identifier
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_find_mpls_vpncb_by_id(int                      unit,
                                  _caladan3_l3_fe_instance_t  *l3_fe,
                                  bcm_vpn_t                vpn_id,
                                  _caladan3_vpn_control_t    **vpnc)
{
    int                           hidx;
    _caladan3_vpn_control_t          *vpnc_temp = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter (vpn_id=0x%x)\n"),
               (int)vpn_id));

    *vpnc = NULL;
    hidx = _CALADAN3_GET_MPLS_VPN_HASH(vpn_id);

    _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

        if (vpnc_temp->vpn_id == vpn_id) {
            *vpnc = vpnc_temp;
            return BCM_E_NONE;
        }

    } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_caladan3_find_mpls_vpncb_by_mcg
 * Purpose:
 *      Find vpn control block based on broadcast group
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      group       - broadcast group
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

static int
_bcm_caladan3_find_mpls_vpncb_by_mcg(_caladan3_l3_fe_instance_t  *l3_fe,
                                   bcm_multicast_t          group,
                                   _caladan3_vpn_control_t    **vpnc)
{
    _caladan3_vpn_control_t          *vpnc_temp = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter group(0x%x)\n"),
               (int)group));

    *vpnc = NULL;

    _CALADAN3_ALL_VPNC(l3_fe, vpnc_temp) {
        if (vpnc_temp->vpn_bc_mcg == group) {
            *vpnc = vpnc_temp;
            return BCM_E_NONE;
        }
    } _CALADAN3_ALL_VPNC_END(l3_fe, vpnc_temp);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_caladan3_find_vpn_sap_by_id
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port identifier
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      mpls_port_id- global mpls port identifier
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_find_vpn_sap_by_id(_caladan3_l3_fe_instance_t *l3_fe,
                               _caladan3_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _caladan3_vpn_sap_t      **vpn_sap)
{
    _caladan3_vpn_sap_t              *temp_vpn_sap = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter mpls-port-id(0x%x)\n"),
               (int)mpls_port_id));

    *vpn_sap = NULL;

    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if (mpls_port_id == temp_vpn_sap->vc_mpls_port_id) {
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _bcm_caladan3_find_vpn_sap_by_port
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      port        - port as a gport
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_find_vpn_sap_by_port(_caladan3_l3_fe_instance_t *l3_fe,
                                 _caladan3_vpn_control_t   *vpnc,
                                 bcm_gport_t            port,
                                 _caladan3_vpn_sap_t      **vpn_sap)
{
    _caladan3_vpn_sap_t              *temp_vpn_sap = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter port(0x%x)\n"),
               (int)port));

    *vpn_sap = NULL;

    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if (port == temp_vpn_sap->vc_mpls_port.port) {
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_caladan3_find_vpn_sap_by_port_vlan
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      port        - port as a gport
 *      vlan        - vlan
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

static int
_bcm_caladan3_find_vpn_sap_by_port_vlan(_caladan3_l3_fe_instance_t *l3_fe,
                                      _caladan3_vpn_control_t   *vpnc,
                                      bcm_gport_t            port,
                                      bcm_vlan_t             vlan,
                                      _caladan3_vpn_sap_t      **vpn_sap)
{
    _caladan3_vpn_sap_t              *temp_vpn_sap = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter port(0x%x) vlan (0x%0x)\n"),
               (int)port, (int)vlan));

    *vpn_sap = NULL;

    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if ( (port == temp_vpn_sap->vc_mpls_port.port) &&
             (vlan == temp_vpn_sap->vc_mpls_port.match_vlan) ){
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_caladan3_find_vpn_sap_by_label
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      mpls_port   - mpls port passed by user (get match label from this)
 *      is_failover - failover flag
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_find_vpn_sap_by_label(_caladan3_l3_fe_instance_t *l3_fe,
                                  _caladan3_vpn_control_t   *vpnc,
                                  bcm_mpls_port_t           *mpls_port,
                                  _caladan3_vpn_sap_t      **vpn_sap,
                                  int is_failover)
{
    _caladan3_vpn_sap_t              *temp_vpn_sap = NULL;
#ifdef PLATFORM_LABEL
    int                           failover_flag;
#endif
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter label(0x%x) \n"),
               (int)(mpls_port->match_label)));

    *vpn_sap = NULL;


    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if ((mpls_port->match_label == temp_vpn_sap->vc_mpls_port.match_label)){
#ifndef PLATFORM_LABEL
            if ( (mpls_port->port == temp_vpn_sap->vc_mpls_port.port) &&
                 (mpls_port->egress_tunnel_if == 
                              temp_vpn_sap->vc_mpls_port.egress_tunnel_if)) {
                *vpn_sap = temp_vpn_sap;
                return BCM_E_NONE;
            }
#else
            failover_flag = temp_vpn_sap->vc_mpls_port.flags \
                                 & BCM_MPLS_PORT_FAILOVER;
            if (failover_flag == is_failover) {
                *vpn_sap = temp_vpn_sap;
                return BCM_E_NONE;
            }
#endif
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_caladan3_create_mpls_vpn_sap
 * Purpose:
 *
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        -  vpn control
 *      mpls_port   -  bcm provided port info
 *      vpn_sap     -  allocated vpn service attachment point
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_create_mpls_vpn_sap(_caladan3_l3_fe_instance_t *l3_fe,
                                _caladan3_vpn_control_t   *vpnc,
                                bcm_mpls_port_t       *mpls_port,
                                _caladan3_vpn_sap_t      **vpn_sap)
{
    int                           status;

    *vpn_sap = NULL;

    status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe, vpnc, 
                                            mpls_port->mpls_port_id, vpn_sap);

    if ((status == BCM_E_NONE) || (status != BCM_E_NOT_FOUND)) {
        return (status == BCM_E_NONE) ? BCM_E_EXISTS : status;
    }

    status = _bcm_caladan3_alloc_mpls_vpn_sap(l3_fe, vpnc,
                                            mpls_port->mpls_port_id, vpn_sap);
    if (status != BCM_E_NONE) {
        return status;
    }

    /* keep copy of user params */
    (*vpn_sap)->vc_mpls_port = *mpls_port;

    /* gport for this vpn-sap */
    (*vpn_sap)->vc_mpls_port_id = mpls_port->mpls_port_id;

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_caladan3_fill_mpls_port_from_vpn_sap
 * Purpose:
 *      Fill mpls port information based on mpls vpn sap
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn_sap     - vpn service attachment point
 *      mpls_port   - mpls port parameters
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_fill_mpls_port_from_vpn_sap(_caladan3_l3_fe_instance_t  *l3_fe,
                                        _caladan3_vpn_sap_t         *vpn_sap,
                                        bcm_mpls_port_t         *mpls_port)
{

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    bcm_mpls_port_t_init(mpls_port);

    /* This has to be the same */
    mpls_port->mpls_port_id        = vpn_sap->vc_mpls_port.mpls_port_id;

    /* Below *may* be derived */
    mpls_port->flags               = vpn_sap->vc_mpls_port.flags;
    mpls_port->if_class            = vpn_sap->vc_mpls_port.if_class;
    mpls_port->exp_map             = vpn_sap->vc_mpls_port.exp_map;
    mpls_port->int_pri             = vpn_sap->vc_mpls_port.int_pri;
    mpls_port->service_tpid        = vpn_sap->vc_mpls_port.service_tpid;

    /* Below *cannot* be derived */
    mpls_port->port                = vpn_sap->vc_mpls_port.port;
    mpls_port->criteria            = vpn_sap->vc_mpls_port.criteria;
    mpls_port->match_vlan          = vpn_sap->vc_mpls_port.match_vlan;
    mpls_port->match_inner_vlan    = vpn_sap->vc_mpls_port.match_inner_vlan;
    mpls_port->match_label         = vpn_sap->vc_mpls_port.match_label;

    /* Below *can* be derived */
    mpls_port->egress_tunnel_if    = vpn_sap->vc_mpls_port.egress_tunnel_if;
    mpls_port->egress_label        = vpn_sap->vc_mpls_port.egress_label;
    mpls_port->egress_service_vlan = vpn_sap->vc_mpls_port.egress_service_vlan;

    /* Always retrieve the encapId from sw state.  VPWS is implemented by
     * swapping ohi's & ftes, so this ohi may be the hw.ohi for the peer.
     */
    mpls_port->encap_id            = vpn_sap->vc_mpls_port.encap_id;

    mpls_port->failover_id         = vpn_sap->vc_mpls_port.failover_id;
    mpls_port->failover_port_id    = vpn_sap->vc_mpls_port.failover_port_id;
    mpls_port->pw_failover_id      = vpn_sap->vc_mpls_port.pw_failover_id;
    mpls_port->pw_failover_port_id = vpn_sap->vc_mpls_port.pw_failover_port_id;
    mpls_port->policer_id          = vpn_sap->vc_mpls_port.policer_id;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_fill_mpls_vpn_config
 * Purpose:
 *      Fill vpn config params based on vpn control
 * Parameters:
 *      unit        - FE unit
 *      vpnc        - vpn control
 *      info        - vpn config
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_fill_mpls_vpn_config(int                     unit,
                                 _caladan3_vpn_control_t    *vpnc,
                                 bcm_mpls_vpn_config_t  *info)
{
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    info->flags = _BCM_MPLS_VPN_TYPE(vpnc->vpn_flags);
    if (info->flags == BCM_MPLS_VPN_L3) {
        info->lookup_id = vpnc->vpn_vrf;
    } else {
        info->lookup_id = vpnc->vpn_id;
    }

    info->vpn                     = vpnc->vpn_id;
    info->broadcast_group         = vpnc->vpn_bc_mcg;
    info->unknown_unicast_group   = vpnc->vpn_bc_mcg;
    info->unknown_multicast_group = vpnc->vpn_bc_mcg;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_validate_tunnel_switch_add
 * Purpose:
 *      validate mpls_tunnel_switch_add params
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_mpls_validate_tunnel_switch_add(_caladan3_l3_fe_instance_t   *l3_fe,
                                            bcm_mpls_tunnel_switch_t *info)
{
    int                status;
    uint32             fte_idx;
    bcm_l3_egress_t    bcm_egr;
    int                flags, unit = l3_fe->fe_unit;

    status = BCM_E_NONE;

#ifdef PLATFORM_LABEL
    if (BCM_GPORT_IS_SET(info->port)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "incoming port (0x%x) not valid for "
                               " bcm_mpls_tunnel_switch_t\n"), info->port));
        return BCM_E_PARAM;
    }
#else
    if (!(BCM_GPORT_IS_SET(info->port))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Gport not set (0x%x)\n"),
                   info->port));
        return BCM_E_PARAM;
    }
#ifdef USE_TUNNEL_ID
    if ((info->flags & BCM_MPLS_SWITCH_WITH_ID) && 
         !(BCM_GPORT_IS_TUNNEL(info->tunnel_id))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid tunnel ID\n")));
        return BCM_E_PARAM;
    }
    if ((info->flags & BCM_MPLS_SWITCH_REPLACE) && 
         !(info->flags & BCM_MPLS_SWITCH_WITH_ID)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "BCM_MPLS_SWITCH_REPLACE needs BCM_MPLS_SWITCH_WITH_ID\n")));
        return BCM_E_PARAM;
    }
#endif
#endif

    if (info->flags & ~_BCM_MPLS_SWITCH_SUPPORTED_FLAGS) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid flags 0x%x\n"),
                   info->flags));
        return BCM_E_PARAM;
    }

    /* We do not care about inner_label */

    if ( (info->action != BCM_MPLS_SWITCH_ACTION_SWAP) &&
         (info->action != BCM_MPLS_SWITCH_ACTION_PHP)  &&
         (info->action != BCM_MPLS_SWITCH_ACTION_POP)  &&
         (info->action != BCM_MPLS_SWITCH_ACTION_POP_DIRECT)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid label switch action %d\n"),
                   info->action));
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, info->vpn)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "invalid vpn (0x%0x)\n"),
                   info->vpn));
        return BCM_E_PARAM;
    }

    /* XXX: TBD: info->egress_label.label  needs to be valid for SWAP */
    fte_idx = 0;
    if (info->action == BCM_MPLS_SWITCH_ACTION_SWAP) {
        bcm_l3_egress_t_init(&bcm_egr);
        fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(info->egress_if);
        /* XXX: TBD: what is the point of label not being there ? */
        flags  = info->egress_label.label ?
            L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH : L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY;
        status = _bcm_caladan3_l3_get_egrif_from_fte(l3_fe,
                                                   fte_idx,
                                                   flags,
                                                   &bcm_egr);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) invalid egress object "
                                   "(egress_if == 0x%x) \n"),
                       bcm_errmsg(status), (int)info->egress_if));
            return status;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_tunnel_switch_delete
 * Purpose:
 *      de-program the label2etc table for l3 vpn/global vrf
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_caladan3_mpls_tunnel_switch_delete(_caladan3_l3_fe_instance_t   *l3_fe,
                                      bcm_mpls_tunnel_switch_t *info)
{
    int                         rv, idx;
    int                         unit = l3_fe->fe_unit;
    _sbx_caladan3_usr_res_types_t    restype;
    soc_sbx_g3p1_labels_t      label2e;
    soc_sbx_g3p1_ft_t           fte;
    soc_sbx_g3p1_v2e_t          v2e;
    uint32                      ftidx, ftcnt, flags;
    bcm_vlan_t vlan;
    uint32 lpidx;
    int                          label1 = 0, label2 = 0, label3 = 0;
    int                          local_port = -1;
#ifndef PLATFORM_LABELS
    bcm_module_t                modid = -1;
    uint32                      trunkid;
    bcm_trunk_add_info_t       *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;
#endif


    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

#ifndef PLATFORM_LABELS
    if (!BCM_GPORT_IS_TRUNK(info->port)) {
        rv = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                      info->port,
                                                      &modid,
                                                      &local_port);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) failure to get mod-port\n"),
                       bcm_errmsg(rv)));
            return rv;
        }
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(info->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
        for (index = 0; index < num_ports; index++) {
            if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
                local_port = trunk_info->tp[index];
                break;
            }
        }
    }

    if ((l3_fe->fe_my_modid != modid && (!is_trunk)) || (local_port == -1)) {
        /**
         * ADD or DELETE needs to modify the match
         * criteria only on local module.
         * If port is -1, then there are no local port for the LAG
         */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "Remote Module:No update needed\n")));

        return BCM_E_NONE;
    }


    label1 = _CALADAN3_MPLSTP_LABEL(info->label);
    label2 = info->second_label;
#else
    local_port = 0;
    label1 = _CALADAN3_MPLSTP_LABEL(info->label);
#endif

    rv = _bcm_caladan3_g3p1_mpls_labels_get(unit, local_port,
                                label1, label2, label3,
                                &label2e);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to read l2e[0x%x]: %s\n"),
                   info->label, bcm_errmsg(rv)));
        return rv;
    }

    ftidx = label2e.ftidx;
        vlan  = label2e.vlan;
        lpidx = label2e.lpi;
        ftcnt = 1;
        flags = 0;

    if (((info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT &&
          info->egress_if))                                    ||
        (info->action == BCM_MPLS_SWITCH_ACTION_SWAP)          ||
        (info->action == BCM_MPLS_SWITCH_ACTION_PHP)) 
    {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "Clearing FTEs[0x%x-0x%x]\n"),
                     ftidx, ftidx + ftcnt - 1));
        soc_sbx_g3p1_ft_t_init(&fte);
        for (idx = 0; idx < ftcnt; idx++) {
            rv = soc_sbx_g3p1_ft_set(unit, ftidx + idx, &fte);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "failed to clear FTE[0x%x]: %s\n\n"),
                           ftidx + idx, bcm_errmsg(rv)));
            }
        }
    }

    
    switch (info->action) {
    case BCM_MPLS_SWITCH_ACTION_SWAP:
    case BCM_MPLS_SWITCH_ACTION_PHP:
        if (_BCM_CALADAN3_IS_LABEL_LER(unit, label2e.opcode) == 0 &&
            (ftidx != 0 && ftidx != SBX_DROP_FTE(unit)))
        {
            restype = SBX_CALADAN3_USR_RES_FTE_MPLS;
            
            rv = _sbx_caladan3_resource_free(unit, restype,
                                        ftcnt,
                                        &ftidx,
                                        flags);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Action %d Failed to free resource %d %s: %s\n"),
                           info->action, restype, 
                           _sbx_caladan3_resource_to_str(restype), 
                           bcm_errmsg(rv)));
                return rv;
            }
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "Freed %s 0x%x + 0x%x\n"),
                         _sbx_caladan3_resource_to_str(restype), 
                         ftidx, ftcnt));
        }

        if (lpidx != 0) {
            restype = SBX_CALADAN3_USR_RES_MPLS_LPORT;
            rv = _sbx_caladan3_resource_free(unit, restype,
                                        1,
                                        &lpidx,
                                        0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Action %d Failed to free resource %d %s: %s\n"),
                           info->action, restype, 
                           _sbx_caladan3_resource_to_str(restype), 
                           bcm_errmsg(rv)));
                return rv;
            }
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "Freed %s 0x%x\n"),
                         _sbx_caladan3_resource_to_str(restype), 
                         lpidx));
        }
        break;
    case BCM_MPLS_SWITCH_ACTION_POP_DIRECT:
    case BCM_MPLS_SWITCH_ACTION_POP:
        if (info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT) {
            restype = SBX_CALADAN3_USR_RES_FTE_MPLS;
        } else {
            restype = SBX_CALADAN3_USR_RES_VSI;
        }

        if (_BCM_CALADAN3_IS_LABEL_LER(unit, label2e.opcode) &&
            (vlan != BCM_VLAN_NONE && ftidx != SBX_DROP_FTE(unit)) &&
            (ftidx != l3_fe->fe_vsi_default_vrf)) 
        {
            rv = _sbx_caladan3_resource_free(unit, restype, 
                                        ftcnt,
                                        &ftidx,
                                        flags);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Action %d Failed to free resource %d %s: %s\n"),
                           info->action, restype, 
                           _sbx_caladan3_resource_to_str(restype), 
                           bcm_errmsg(rv)));
                return rv;
            }
        }

        if (info->action == BCM_MPLS_SWITCH_ACTION_POP && 
                   ftidx != l3_fe->fe_vsi_default_vrf) {
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "Clearing v2e[0x%x-0x%x]\n"),
                         vlan, vlan + ftcnt - 1));

            soc_sbx_g3p1_v2e_t_init(&v2e);
            for (idx = 0; idx < ftcnt; idx++) {
                rv = soc_sbx_g3p1_v2e_set(unit, vlan + idx, &v2e);

                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Failed to clear v2e[0x%x] on vpn 0x%x:"
                                           " %s\n"),
                               vlan + idx, info->vpn, bcm_errmsg(rv)));
                    return rv;
                }
            }
        }
        if (lpidx != 0) {
            restype = SBX_CALADAN3_USR_RES_MPLS_LPORT;
            rv = _sbx_caladan3_resource_free(unit, restype,
                                        1,
                                        &lpidx,
                                        0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                       "Action %d Failed to free resource %d %s: %s\n"),
                           info->action, restype, 
                           _sbx_caladan3_resource_to_str(restype), 
                           bcm_errmsg(rv)));
                return rv;
            }
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "Freed %s 0x%x\n"),
                         _sbx_caladan3_resource_to_str(restype), 
                         lpidx));
        }
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid action: %d\n"),
                   info->action));
        return BCM_E_PARAM;
    }

#ifndef PLATFORM_LABELS
    for (index = 0; index < num_ports; index++) {
        if (is_trunk == 1) {
            local_port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }
#endif

        rv = _bcm_caladan3_g3p1_mpls_labels_delete(unit, local_port,
                                                   label1, label2, label3);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to clear l2e[0x%x]: %s\n"),
                       info->label, bcm_errmsg(rv)));
            return rv;
         }
#ifndef PLATFORM_LABELS
    }
#endif

#ifdef PLATFORM_LABELS
    if (BCM_SUCCESS(status)) {
        rv = _bcm_caladan3_g3p1_mpls_property_table_set(unit,
                                                   info->label,
                                                   0);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to clear property table entry "
                                   " [0x%x]: %s\n"), info->label, bcm_errmsg(rv)));
        }
    }
#endif

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_tunnel_switch_delete_all
 * Purpose:
 *      LER and LSR ranges needs to be invalidated
 * Parameters:
 *      l3_fe       - l3 fe context
 *
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_caladan3_mpls_tunnel_switch_delete_all(_caladan3_l3_fe_instance_t   *l3_fe)
{
    int                         unit = l3_fe->fe_unit;
#ifndef USE_TUNNEL_ID
    int                         rv;
    int                         curr =0, next=0;
    int                         label2 = 0, nlabel2 = 0;
    int                         port = 0, nport = 0;
#else
    bcm_gport_t                 tunnel_id;
    bcm_gport_t                 max_tunnel_id;
    bcm_mpls_tunnel_switch_t    info;
#endif

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

#ifdef USE_TUNNEL_ID
    BCM_GPORT_TUNNEL_ID_SET(tunnel_id, SBX_CALADAN3_TUNNEL_ID_BASE);
    BCM_GPORT_TUNNEL_ID_SET(max_tunnel_id, SBX_CALADAN3_TUNNEL_ID_MAX);
    
    for (; tunnel_id < max_tunnel_id; tunnel_id++) {
        bcm_mpls_tunnel_switch_t_init(&info);
        info.tunnel_id = tunnel_id;
        bcm_caladan3_mpls_tunnel_switch_delete(unit, &info);
    }

#else

    rv = _bcm_caladan3_g3p1_mpls_labels_first(unit, &port, &curr, &label2, 0);
    while (curr != 0)
      {
          rv = _bcm_caladan3_g3p1_mpls_labels_next(unit,
                     port, curr, label2, 0,
                     &nport, &next, &nlabel2, 0);
          rv = _bcm_caladan3_g3p1_mpls_labels_delete(unit,
                     port, curr, label2, 0);
          COMPILER_REFERENCE(rv);
          curr = next;
          label2 = nlabel2,
          port = nport;
      }
#endif
    return BCM_E_NONE;
}



STATIC int
_bcm_caladan3_mpls_map_set_vlan2etc(_caladan3_l3_fe_instance_t   *l3_fe,
                                  _caladan3_vpn_control_t      *vpnc,
                                  int                       program_vsi)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_map_set_vlan2etc(l3_fe, vpnc, program_vsi);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}


/*
 * Function:
 *      _bcm_caladan3_mpls_tunnel_switch_update
 * Purpose:
 *      program the label2etc table for l3 vpn/global vrf
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_mpls_tunnel_switch_update(_caladan3_l3_fe_instance_t   *l3_fe,
                                      bcm_mpls_tunnel_switch_t *info)
{

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    if (SOC_IS_SBX_G3P1(l3_fe->fe_unit)) {
        return (_bcm_caladan3_g3p1_mpls_tunnel_switch_update(l3_fe->fe_unit,
                                                           l3_fe,
                                                           info));
    }

    return BCM_E_UNAVAIL;
}



/*
 * Function:
 *      _bcm_caladan3_map_set_vpn_bc_fte
 * Purpose:
 *      map and then set the broadcast fte
 *      for the vpn
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_map_set_vpn_bc_fte(_caladan3_l3_fe_instance_t  *l3_fe,
                               _caladan3_vpn_control_t     *vpnc)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_map_set_vpn_bc_fte(l3_fe, vpnc);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_update_mpls_vpn_id
 * Purpose:
 *      Update information for already existing
 *      vpn config
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      info        - vpn config
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_update_mpls_vpn_id(int                     unit,
                               _caladan3_l3_fe_instance_t *l3_fe,
                               _caladan3_vpn_control_t    *vpnc,
                               bcm_mpls_vpn_config_t  *info)
{
    int                           status;
    int                           old_bc_mcg;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    /**
     * Cannot modify the key
     */
    if (_BCM_MPLS_VPN_TYPE(vpnc->vpn_flags) !=
        _BCM_MPLS_VPN_TYPE(info->flags)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "cannot modify type of vpn 0x%x --> 0x%x\n"),
                   vpnc->vpn_flags, info->flags));
        return BCM_E_PARAM;
    }

    switch (_BCM_MPLS_VPN_TYPE(info->flags)) {
    case BCM_MPLS_VPN_L3:
        if (info->lookup_id && (info->lookup_id != vpnc->vpn_vrf)) {
            return BCM_E_PARAM;
        }
        break;

    default:
        if (info->lookup_id && (info->lookup_id != vpnc->vpn_id)) {
            return BCM_E_PARAM;
        }
    }

    if (vpnc->vpn_bc_mcg != info->broadcast_group) {
        old_bc_mcg       = vpnc->vpn_bc_mcg;
        vpnc->vpn_bc_mcg = info->broadcast_group;
        status = _bcm_caladan3_map_set_vpn_bc_fte(l3_fe,
                                                vpnc);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) setting bcast fte for vpn(0x%x)\n"),
                       bcm_errmsg(status), vpnc->vpn_id));
            vpnc->vpn_bc_mcg = old_bc_mcg;
            return status;
        }
    }

    /* coverity [end_of_scope] */
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_gport_get_mod_port
 * Purpose:
 *      Given gport decipher module-id and port values
 * Parameters:
 *      unit        - FE unit number
 *      gport       - global port value
 *      modid       - module id
 *      port        - port
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_caladan3_mpls_gport_get_mod_port(int            unit,
                                    bcm_gport_t    gport,
                                    bcm_module_t  *modid,
                                    bcm_port_t    *port)
{
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    if (SOC_GPORT_IS_LOCAL(gport)) {
        BCM_IF_ERROR_RETURN(
            bcm_stk_my_modid_get(unit, modid));
        *port = SOC_GPORT_LOCAL_GET(gport);
        return BCM_E_NONE;
    } else if (SOC_GPORT_IS_MODPORT(gport)) {
        *modid = SOC_GPORT_MODPORT_MODID_GET(gport);
        *port  = SOC_GPORT_MODPORT_PORT_GET(gport);
        return BCM_E_NONE;
    }

    return BCM_E_PARAM;
}

/*
 * Function:
 *      _bcm_caladan3_expand_vpn_sap_ufte
 * Purpose:
 *      Expand the user area to keep <modid,fte-id>
 * Parameters:
 *      vpn_sap     -  vpn service attachment point
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_expand_vpn_sap_ufte(_caladan3_vpn_sap_t   *vpn_sap)
{
    int                           num_new_entries, size, ii;
    _caladan3_vc_ete_fte_t           *vc_ete_fte;

    num_new_entries = vpn_sap->vc_alloced_ue + _CALADAN3_ETE_USER_SLAB_SIZE;
    size = sizeof(*vc_ete_fte) * num_new_entries;

    vc_ete_fte =
        sal_alloc(size, "MPLS-ete-fte");
    if (vc_ete_fte == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(vc_ete_fte, 0, size);
    for (ii = 0; ii < vpn_sap->vc_alloced_ue; ii++) {
        vc_ete_fte[ii] = vpn_sap->u.vc_fte[ii];
    }
    sal_free(vpn_sap->u.vc_fte);
    vpn_sap->u.vc_fte      = vc_ete_fte;
    vpn_sap->vc_alloced_ue = num_new_entries;

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_caladan3_find_vpn_sap_ufte
 * Purpose:
 *      find in the user area if the <modid,fte-idx>
 *      is already present
 * Parameters:
 *      l3_fe         -  fe instance structure
 *      vpn_sap       -  vpn service attachment point
 *      caller_module -  module id
 *      slot          -  slot if we find entry
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_find_vpn_sap_ufte(_caladan3_l3_fe_instance_t  *l3_fe,
                                _caladan3_vpn_sap_t         *vpn_sap,
                                bcm_module_t             caller_module,
                                int                     *slot)
{
    int          ii;
    uint32       fte_idx;

    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "mpls_port=0x%08x  fte=0x%04x\n"),
               vpn_sap->vc_mpls_port_id, fte_idx));

    *slot   = -1;
    for (ii = 0; ii < vpn_sap->vc_inuse_ue; ii++) {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "compare mod/fte:  %d/0x%04x %d/0x%04x slot=%d\n"),
                   vpn_sap->u.vc_fte[ii].mod_id,
                   vpn_sap->u.vc_fte[ii].fte_idx.fte_idx,
                   caller_module, fte_idx, ii));

        if ((vpn_sap->u.vc_fte[ii].mod_id == caller_module) &&
            (vpn_sap->u.vc_fte[ii].fte_idx.fte_idx == fte_idx)) {
            *slot = ii;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_9caladan3_link_vpn_sap_ufte
 * Purpose:
 *      link given <mod,fte-id> into user area
 * Parameters:
 *      l3_fe          - l3 fe instance
 *      vpn_sap        -  vpn service attachment point
 *      caller_module  -  module id
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_link_vpn_sap_ufte(_caladan3_l3_fe_instance_t  *l3_fe,
                                _caladan3_vpn_sap_t         *vpn_sap,
                                bcm_module_t             caller_module)
{
    int status;

    if (vpn_sap->vc_alloced_ue == vpn_sap->vc_inuse_ue) {
        status = _bcm_caladan3_expand_vpn_sap_ufte(vpn_sap);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failure(%s) expanding vpn-sap\n"),
                       bcm_errmsg(status)));
            return status;
        }
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "insert gport 0x%08x mod/fte %d/0x%04x to "
                           "slot %d\n"),
               vpn_sap->vc_mpls_port_id, caller_module,
               BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id),
               vpn_sap->vc_inuse_ue));

    /**
     * FTE-idx has to be globally same
     * Please never do a find and add. Temporarily during
     * update --> delete handling for bcm_mpls_vpn_port_udate()
     * there will be 2 of the same <modid, fte-idx>
     */
    vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue].mod_id  = caller_module;
    vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue].fte_idx.fte_idx =
        BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
    vpn_sap->vc_inuse_ue++;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_unlink_vpn_sap_ufte
 * Purpose:
 *      unlink given <mod,fte-id> into user area
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn_sap     -  vpn service attachment point
 *      fte_idx     -  h/w fte idx
 *      module      -  module id
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_unlink_vpn_sap_ufte(_caladan3_l3_fe_instance_t  *l3_fe,
                                _caladan3_vpn_sap_t         *vpn_sap,
                                bcm_module_t             caller_module)
{
    int                           status, slot;

    status = _bcm_caladan3_find_vpn_sap_ufte(l3_fe,
                                           vpn_sap,
                                           caller_module,
                                           &slot);
    if (status != BCM_E_NONE) {
        return status;
    }

    vpn_sap->vc_inuse_ue--;
    vpn_sap->u.vc_fte[slot] =
        vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue];

    return BCM_E_NONE;
}


STATIC int
_bcm_caladan3_invalidate_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t *l3_fe,
                                            _caladan3_vpn_sap_t    *vpn_sap)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {

    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_invalidate_vpn_sap_hw_resources(l3_fe, vpn_sap);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

int
_bcm_caladan3_destroy_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t *l3_fe,
                                         bcm_module_t            caller_module,
                                         _caladan3_vpn_sap_t       **vpn_sap)
{
    int                               status;

    status = _bcm_caladan3_unlink_vpn_sap_ufte(l3_fe, *vpn_sap, caller_module);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) unlinking vpn-sap\n"),
                   bcm_errmsg(status)));
        /* do not return */
    }

    if ((*vpn_sap)->vc_inuse_ue == 0) {
        /*
         * Invalidate and free the resources
         */
        status = _bcm_caladan3_invalidate_vpn_sap_hw_resources(l3_fe, *vpn_sap);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) invalidating vpn-sap resources\n"),
                       bcm_errmsg(status)));
            return status;
        }

        status = _bcm_caladan3_free_mpls_vpn_sap(l3_fe, vpn_sap);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) freeing vpn-sap\n"), 
                       bcm_errmsg(status)));
            return status;
        }
    }

    return BCM_E_NONE;
}




/*
 * Function:
 *     _bcm_caladan3_mpls_get_fte
 * Purpose:
 *     Given an fte index get mpls port information
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     ul_fte     - (IN)     the fte index
 *     action     - (IN)     L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY
 *                           L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY
 *                           L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH
 *     fte_mpls_port - (OUT) mpls-port information from fte
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */

STATIC int
_bcm_caladan3_mpls_get_fte(_caladan3_l3_fe_instance_t *l3_fe,
                         uint32                  fte_idx,
                         int                     action,
                         _caladan3_l3_or_mpls_egress_t *egr)
{
    int rv = BCM_E_INTERNAL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_get_fte(l3_fe, fte_idx, action, egr);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *     _bcm_caladan3_destroy_mpls_fte
 * Purpose:
 *     destroy gport fte, optionally ohi and ete
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     action     - (IN)     mode of operation
 *     fte_idx    - (IN)     handle that needs to be destroyed
 *     module_id  - (IN)     module in case of FTE_ONLY
 *     encap_id   - (IN)     ohi in case of FTE_ONLY
 *     vpnc       - (IN)     vpn control block
 *     is_trunk   - (IN)     whether the egress port is trunk
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     L3_OR_MPLS_DESTROY_FTE__FTE_ONLY
 *        Invalidate FTE and return
 *     L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE
 *        Get FTE, Invalidate FTE and delete OHI-->ETE
 *        based on what we got earlier from H/W
 *     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI
 *        Invalidate FTE and delete OHI-->ETE
 *        based on user params
 */

STATIC int
_bcm_caladan3_destroy_mpls_fte(_caladan3_l3_fe_instance_t *l3_fe,
                             int                     action,
                             uint32                  fte_idx,
                             bcm_module_t            param_modid,
                             bcm_if_t                param_encap_id,
                             _caladan3_vpn_control_t    *vpnc,
                             int8                  is_trunk)
{
    int                              status;
    _caladan3_l3_or_mpls_egress_t        egr;
    bcm_if_t                         encap_id;
    bcm_module_t                     exit_modid;
    _caladan3_vpn_sap_t                 *vpn_sap;
    bcm_gport_t                      mpls_port_id;


    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE ||
        action == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        status = _bcm_caladan3_mpls_get_fte(l3_fe,
                                          fte_idx,
                                          L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                          &egr);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not find fte(0x%x)\n"),
                       bcm_errmsg(status), fte_idx));
            return status;
        }
    }

    /*
     * In any case, invalidate the FTE and free the resource
     */
    status = _bcm_caladan3_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) invalidating gport-fte(0x%x)\n"),
                   bcm_errmsg(status), fte_idx));
        return status;
    }

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_ONLY) {
        return BCM_E_NONE;
    } else if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        exit_modid = egr.fte_modid;
        encap_id   = egr.encap_id;
    } else if (action  == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        exit_modid = param_modid;
        encap_id   = param_encap_id;
    } else {
        return BCM_E_INTERNAL;
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "myMod=%d exitMod=%d encap_id=0x%08x"
                           " ohi=0x%04x\n"),
               l3_fe->fe_my_modid, exit_modid,
               encap_id,  SOC_SBX_OHI_FROM_ENCAP_ID(encap_id)));


    BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, fte_idx);

    status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe, vpnc, mpls_port_id,
                                            &vpn_sap);

    if  (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) finding vpn for fte (0x%x)\n"),
                   bcm_errmsg(status), fte_idx));
        return status;
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "sap found id 0x%08x\n"),
               vpn_sap->vc_mpls_port_id));

    if (l3_fe->fe_my_modid == exit_modid || is_trunk) {
        status = _bcm_caladan3_destroy_vpn_sap_hw_resources(l3_fe,
                                                          l3_fe->fe_my_modid,
                                                          &vpn_sap);
        if  (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) in destroying egr resources for fte(0x%x)\n"),
                       bcm_errmsg(status), fte_idx));
            return status;
        }
    } else {
        /* since vpn sw state is always created, it must always be destroyed */
        status = _bcm_caladan3_free_mpls_vpn_sap(l3_fe, &vpn_sap);
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_caladan3_map_set_mpls_vpn_fte(_caladan3_l3_fe_instance_t  *l3_fe,
                                 _caladan3_vpn_control_t     *vpnc,
                                 _caladan3_vpn_sap_t         *vpn_sap,
                                 bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv =  _bcm_caladan3_g3p1_mpls_map_set_mpls_vpn_fte(l3_fe, vpnc,
                                                         vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}


int
_bcm_caladan3_mpls_free_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t  *l3_fe,
                                           _caladan3_vpn_sap_t        *vpn_sap)
{
    int  last_error_status;

    last_error_status = BCM_E_NONE;

    if (vpn_sap->vc_ohi.ohi != _CALADAN3_INVALID_OHI) {

        /* de-allocated only if owned but still unlink from vpn db 
         */
        if (vpn_sap->vc_res_alloced) {
            last_error_status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                                       SBX_CALADAN3_USR_RES_OHI,
                                                       1,
                                                       &vpn_sap->vc_ohi.ohi,
                                                       0);
            if (last_error_status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) freeing ohi(0x%x)\n"),
                           bcm_errmsg(last_error_status),
                           vpn_sap->vc_ohi.ohi));
            }
        }

        DQ_REMOVE(&vpn_sap->vc_ohi_link);
    }

    /* deallocate etes, even when exteranlly managed; only the OHIs are actually
     * managed exteranlly.  The SDK still manages the ETEs & LPs
     */
    if (vpn_sap->vc_ete_hw_idx.ete_idx != _CALADAN3_INVALID_ETE_IDX) {

        if (vpn_sap->vc_res_alloced) {
            last_error_status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                                       SBX_CALADAN3_USR_RES_ETE,
                                                       1,
                                                       &vpn_sap->vc_ete_hw_idx.ete_idx,
                                                       0);
            if (last_error_status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) freeing encap-ete(0x%x)\n"),
                           bcm_errmsg(last_error_status),
                           vpn_sap->vc_ete_hw_idx.ete_idx));
            }
        }
    }

    vpn_sap->vc_ete_hw_idx.ete_idx   = _CALADAN3_INVALID_ETE_IDX;
    vpn_sap->vc_ohi.ohi              = _CALADAN3_INVALID_OHI;

    return last_error_status;
}


int
_bcm_caladan3_mpls_alloc_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t  *l3_fe,
                                            _caladan3_vpn_sap_t         *vpn_sap,
                                            bcm_mpls_port_t         *mpls_port)
{
    int                           hidx, status = BCM_E_NONE;
    int                           ignore_status;

    /* is an OHI needed - */
    if (SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id)) {
        vpn_sap->vc_ohi.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
    } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id)) {
        vpn_sap->vc_ohi.ohi = 
            SOC_SBX_OHI_FROM_L2_ENCAP_ID(mpls_port->encap_id);
    }

    if (!mpls_port->encap_id) {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                         SBX_CALADAN3_USR_RES_OHI,
                                         1,
                                         &vpn_sap->vc_ohi.ohi,
                                         0);

        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failure(%s) allocating ohi\n"),
                       bcm_errmsg(status)));
            goto error_out;
        }
    }
    
    /* ensure the externally managed encap id is reserved */
    if (mpls_port->flags & BCM_MPLS_PORT_ENCAP_WITH_ID) {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                         SBX_CALADAN3_USR_RES_OHI,
                                         1,
                                         &vpn_sap->vc_ohi.ohi,
                                         _SBX_CALADAN3_RES_FLAGS_RESERVE);
        if (status != BCM_E_RESOURCE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "encapId 0x%08x passed as reserved, but is "
                                   "not allocated\n"), mpls_port->encap_id));
        } else {
            status = BCM_E_NONE;
        }
    }

    /* Allocate ETE Encap */
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_ETE,
                                     1,
                                     &vpn_sap->vc_ete_hw_idx.ete_idx,
                                     0);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "failure(%s) allocating mpls-encap ete\n"),
                   bcm_errmsg(status)));
        goto error_out;
    }

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit,
                            "vpn(0x%x) sap(0x%x) having "
                             "ohi(0x%x) vc-ete(0x%x) allocated\n"),
                 vpn_sap->vc_vpnc->vpn_id, vpn_sap->vc_mpls_port_id,
                 vpn_sap->vc_ohi.ohi,
                 vpn_sap->vc_ete_hw_idx.ete_idx));

    /**
     * OHI is always reqd on vpn_sap if we are exiting out of
     * fe_my_modid
     */
    hidx = _CALADAN3_GET_OHI2ETE_HASH_IDX(vpn_sap->vc_ohi.ohi);
    DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx],
                   &vpn_sap->vc_ohi_link);
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit,
                            "Insert ohi=0x%08x into sw hash idx=%d\n"),
                 vpn_sap->vc_ohi.ohi, hidx));

    mpls_port->encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);
    return BCM_E_NONE;

error_out:
    if (!mpls_port->encap_id || \
        _BCM_CALADAN3_IS_PWE3_TUNNEL(mpls_port->flags)) {
        ignore_status = _bcm_caladan3_mpls_free_vpn_sap_hw_resources(l3_fe,
                                                                   vpn_sap);
        COMPILER_REFERENCE(ignore_status);
    }

    return status;
}

static int
_bcm_caladan3_match_port2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                           int                      action,
                           int                      logicalPort,
                           _caladan3_vpn_control_t     *vpnc,
                           _caladan3_vpn_sap_t         *vpn_sap,
                           bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    if ((action != _CALADAN3_MPLS_PORT_MATCH_ADD) &&
        (mpls_port->port != vpn_sap->vc_mpls_port.port)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "cannot change mod-port or match criteria, user "
                               "needs to delete and then re-add\n")));
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_match_port2etc(l3_fe, action, logicalPort,
                                                  vpnc, vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

STATIC int
_bcm_caladan3_match_pvid2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                           int                      action,
                           uint32                 logicalPort,
                           _caladan3_vpn_control_t     *vpnc,
                           _caladan3_vpn_sap_t         *vpn_sap,
                           bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_match_pvid2etc(l3_fe, action, logicalPort,
                                                  vpnc, vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

static int
_bcm_caladan3_match_pstackedvid2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                                  int                      action,
                                  uint32                 logicalPort,
                                  _caladan3_vpn_control_t     *vpnc,
                                  _caladan3_vpn_sap_t         *vpn_sap,
                                  bcm_mpls_port_t         *mpls_port)
{

    int rv = BCM_E_UNAVAIL;


    if ((action != _CALADAN3_MPLS_PORT_MATCH_ADD) &&
        ((mpls_port->port != vpn_sap->vc_mpls_port.port) ||
         (mpls_port->match_vlan != vpn_sap->vc_mpls_port.match_vlan) ||
         (mpls_port->match_inner_vlan != vpn_sap->vc_mpls_port.match_inner_vlan))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "cannot change mod-port or match criteria, user "
                               "needs to delete and then re-add\n")));
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_mpls_match_pstackedvid2etc(l3_fe, action,
                                                         logicalPort,
                                                         vpnc, vpn_sap,
                                                         mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/* 
 * Populate the vpn_sap.mpls_psn_ing_label with LSP labels
 * by reading hash entry using mpls_port.tunnel_id
 */
#ifdef USE_TUNNEL_ID
int
_bcm_caladan3_mpls_populate_lsp_labels(int unit,
                                  _caladan3_vpn_sap_t *vpn_sap,
                                  bcm_mpls_port_t *mpls_port)
{
    int status = BCM_E_NONE;
    bcm_mpls_tunnel_switch_t *info;
    uint8                     key[MPLS_SWITCH_INFO_KEY_SIZE];
    _bcm_caladan3_mpls_switch_key_get(&key,
                                      0, 0, 0, mpls_port->tunnel_id);
                                     
    status = shr_htb_find(mpls_switch_info_db[unit],
                          (shr_htb_key_t) &key,
                          (shr_htb_data_t *) &info, 0);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Tunnel switch data not found: %d (%s)\n"),
                   status, bcm_errmsg(status)));
    } else {
        vpn_sap->mpls_psn_ing_label  = info->label;
        vpn_sap->mpls_psn_ing_label2 = info->second_label;
    }

    return status;
}
#endif

int
_bcm_caladan3_match_label2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                            int                      action,
                            uint32                 logicalPort,
                            _caladan3_vpn_control_t     *vpnc,
                            _caladan3_vpn_sap_t         *vpn_sap,
                            bcm_mpls_port_t         *mpls_port)
{
    int                           status = BCM_E_NONE;
    int                           vlan;
    uint32                        fteIdx=0;
    soc_sbx_g3p1_labels_t         ilm;
#ifdef PLATFORM_LABELS
    int                           property_value;
#else
    bcm_module_t                  modid = -1;
    uint32                        trunkid;
    bcm_trunk_add_info_t         *trunk_info = NULL;
    uint8                         index, is_trunk = 0, num_ports = 0;
#endif
    bcm_port_t                    port = -1;
    int                           label1 = 0, label2 = 0, label3 = 0;
    int                           old_label1 = 0, old_label2 = 0, old_label3 = 0;

    if(action < _CALADAN3_MPLS_PORT_MATCH_ADD ||
       action >= _CALADAN3_MPLS_PORT_MATCH_MAX) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Bad Action\n")));
        return BCM_E_PARAM;
    }

    soc_sbx_g3p1_labels_t_init(&ilm);

    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype != SOC_SBX_UCODE_TYPE_G3P1) {
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        return BCM_E_INTERNAL;
    }

#ifndef PLATFORM_LABELS
    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                      mpls_port->port,
                                                      &modid,
                                                      &port);
        
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) failure to get mod-port\n"),
                       bcm_errmsg(status)));
            return status;
        }
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
        for (index = 0; index < num_ports; index++) {
            if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
                port = trunk_info->tp[index];
                break;
            }
        }
    }

    if ((l3_fe->fe_my_modid != modid && (!is_trunk)) || (port == -1)) {
        /**
         * ADD or DELETE needs to modify the match
         * criteria only on local module.
         * If port is -1, then there are no local port for the LAG
         */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "Remote Module:No update needed\n")));

        return BCM_E_NONE;
    }
#endif

#ifdef PLATFORM_LABELS
    port = 0;
    label1 = _CALADAN3_MPLSTP_LABEL(mpls_port->match_label);
    old_label1 = _CALADAN3_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label);
#else
#ifdef USE_TUNNEL_ID
    _bcm_caladan3_mpls_populate_lsp_labels(l3_fe->fe_unit, vpn_sap, mpls_port);
    label1 = vpn_sap->mpls_psn_ing_label;
    if (vpn_sap->mpls_psn_ing_label2) {
        label2 = vpn_sap->mpls_psn_ing_label2;
        label3 = _CALADAN3_MPLSTP_LABEL(mpls_port->match_label);
    } else {
        label2 = _CALADAN3_MPLSTP_LABEL(mpls_port->match_label);
    }
    old_label1 = vpn_sap->mpls_psn_ing_label;
    if (vpn_sap->mpls_psn_ing_label2) {
        old_label2 = vpn_sap->mpls_psn_ing_label2;
        old_label3 = _CALADAN3_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label);
    } else {
        old_label2 = _CALADAN3_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label);
    }
#else    
    
    label1 = vpn_sap->mpls_psn_label;
    if (vpn_sap->mpls_psn_label2) {
        label2 = vpn_sap->mpls_psn_label2;
        label3 = _CALADAN3_MPLSTP_LABEL(mpls_port->match_label);
    } else {
        label2 = _CALADAN3_MPLSTP_LABEL(mpls_port->match_label);
    }
    old_label1 = vpn_sap->mpls_psn_label;
    if (vpn_sap->mpls_psn_label2) {
        old_label2 = vpn_sap->mpls_psn_label2;
        old_label3 = _CALADAN3_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label);
    } else {
        old_label2 = _CALADAN3_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label);
    }
#endif
#endif

        
    if((_CALADAN3_MPLS_PORT_MATCH_DELETE == action) ||
       (_CALADAN3_MPLS_PORT_MATCH_UPDATE == action)) {
        status = _bcm_caladan3_g3p1_mpls_labels_get(l3_fe->fe_unit, port,
                                                    label1, label2, label3,
                                                   &ilm);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) reading g3p1 labels table (0x%x)\n"),
                       bcm_errmsg(status),
                       mpls_port->match_label));
            return status;
        }
    }

    if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
        /* update lpi in software state */
        vpn_sap->logicalPort = 0;
        if (vpn_sap->vc_mpls_port.match_label != mpls_port->match_label) {
            return BCM_E_PARAM;
        }
    } else {

        
        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            vlan  = (BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id) -
                     l3_fe->vlan_ft_base);
            fteIdx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
        } else {
            vlan = vpnc->vpn_id;
        }
        
        
        ilm.pipe     = 1; 
        ilm.vlan     = vlan;
        if (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP) {
            ilm.elsp = 1;
        }
        if (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET) {
            ilm.elsp = 0;
            ilm.cos = mpls_port->int_pri;
        }

         ilm.keep_sdtag = 0;
         /* if raw mode pseudowire, do not touch customer tags under label stack */
        if (_BCM_CALADAN3_IS_PWE3_TUNNEL(mpls_port->flags) && 
            (!(mpls_port->flags & BCM_MPLS_PORT_SERVICE_TAGGED))) {
            ilm.keep_sdtag = 1;
        }
 
        if (_BCM_CALADAN3_IS_PWE3_TUNNEL(mpls_port->flags)) {
            if (mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) {
                ilm.opcode  = _BCM_CALADAN3_LABEL_PWE(l3_fe->fe_unit);

            } else if (mpls_port->flags & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP) {
                ilm.opcode  = _BCM_CALADAN3_LABEL_CES(l3_fe->fe_unit);

            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Bad Tunnel Flag 0x%x\n"),
                           mpls_port->flags));
                return status;
            }
        } else {
            /* only CES PW handoff AC is supported currently, they are faked
             * as LSR */
            ilm.opcode = _BCM_CALADAN3_LABEL_LSR(l3_fe->fe_unit);
        }

        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            ilm.vlan   = 0;
            ilm.ftidx  = fteIdx;
            ilm.vpws   = 1;
        }

        
 
        
#if 0
        ilm.ulQosProfile = _MPLS_EXPMAP_HANDLE_DATA(mpls_port->exp_map);

        if (mpls_port->flags & BCM_MPLS_PORT_NETWORK) {
            ilm.ulVplsColor  = _CALADAN3_VPLS_COLOR_WAN;
        } else {
            ilm.ulVplsColor  = _CALADAN3_VPLS_COLOR_LAN;
        }
#endif
        status = _bcm_caladan3_g3p1_mpls_lp_write(l3_fe->fe_unit, logicalPort,
                                                  vpn_sap, mpls_port, action);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failed to write logical port 0x%x: %d (%s)\n"),
                       logicalPort, status, bcm_errmsg(status)));
            return status;
        }
        
        ilm.lpi = logicalPort;

        /* update lpi in software state */
        vpn_sap->logicalPort  = logicalPort;
    }
   

#ifndef PLATFORM_LABELS
    for (index = 0; index < num_ports; index++) {
        if (is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }
#endif
        if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
            _bcm_caladan3_g3p1_mpls_labels_delete(l3_fe->fe_unit, port,
                                                  label1, label2, label3);
            
        } else {
            if (action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) {
                status = _bcm_caladan3_g3p1_mpls_labels_update(
                                                       l3_fe->fe_unit, port,
                                                       label1, label2, label3,
                                                      &ilm);
                if (vpn_sap->vc_mpls_port.match_label != mpls_port->match_label) {

                    /* To avoid traffic loss during update, update old label after new label changes
                     * are commited */
                    /* delete old entry */
                    _bcm_caladan3_g3p1_mpls_labels_delete(l3_fe->fe_unit, port,
                                           old_label1, old_label2, old_label3);
    
                    if (BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(l3_fe->fe_unit,
                                              "error(%s) invalidating "
                                               "using soc_sbx_g3p1_labels_delete "
                                               "for old label(0x%x)\n"),
                                   bcm_errmsg(status),
                                   vpn_sap->vc_mpls_port.match_label));
                        return status;
                    }
                }
            } else {
                status = _bcm_caladan3_g3p1_mpls_labels_set(l3_fe->fe_unit,
                                             port,
                                             label1, label2, label3, 
                                             &ilm);
                             
            }
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) calling soc_sbx_g3p1_labels_set "
                                       "for label(0x%x)\n"),
                           bcm_errmsg(status),
                           mpls_port->match_label));
                return status;
            }
        }
#ifndef PLATFORM_LABELS
    } /* for (index = 0; index < num_ports; index++) */
#endif

#ifdef PLATFORM_LABELS
    
    if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
            property_value = 0;
    } else {
        if (mpls_port->flags & BCM_MPLS_PORT_CONTROL_WORD) {
            /* ACH */
            property_value = 3;
        } else {
            if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
                /* NO_ACH - L3 */
                property_value = 1;
            } else {
                /* NO_ACH - L2 */
                property_value = 2;
            }
        }           
    }
    status = _bcm_caladan3_g3p1_mpls_property_table_set(l3_fe->fe_unit,
                                                        mpls_port->match_label,
                                                        property_value);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Failed to set property table entry "
                               " [0x%x]: %s\n"), mpls_port->match_label, bcm_errmsg(status)));
    }
#endif

    return BCM_E_NONE;
}

STATIC int
_bcm_caladan3_alloc_mpls_vpn_sap(_caladan3_l3_fe_instance_t  *l3_fe,
                               _caladan3_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _caladan3_vpn_sap_t      **vpn_sap)
{
    int                           size;
    _caladan3_vc_ete_fte_t           *vc_ete_fte;
    uint32                        fte_idx;

    *vpn_sap = sal_alloc(sizeof(_caladan3_vpn_sap_t),
                         "MPLS-vpn-sap");
    if (*vpn_sap == NULL) {
        return BCM_E_MEMORY;
    }
    size  = sizeof(_caladan3_vc_ete_fte_t) * _CALADAN3_ETE_USER_SLAB_SIZE;
    vc_ete_fte = sal_alloc(size, "MPLS-ete-fte");
    if (vc_ete_fte == NULL) {
        sal_free(*vpn_sap);
        *vpn_sap = NULL;
        return BCM_E_MEMORY;
    }

    sal_memset(*vpn_sap, 0, sizeof(_caladan3_vpn_sap_t));
    sal_memset(vc_ete_fte, 0, size);

    (*vpn_sap)->vc_vpnc       = vpnc;
    (*vpn_sap)->u.vc_fte      = vc_ete_fte;
    (*vpn_sap)->vc_alloced_ue = _CALADAN3_ETE_USER_SLAB_SIZE;
    (*vpn_sap)->vc_inuse_ue   = 0;

    /* These are reserved upfront */
    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port_id);
    (*vpn_sap)->vc_ohi.ohi              = _CALADAN3_INVALID_OHI;
    (*vpn_sap)->vc_ete_hw_idx.ete_idx   = _CALADAN3_INVALID_ETE_IDX;

    DQ_INSERT_HEAD(&vpnc->vpn_sap_head,
                   &(*vpn_sap)->vc_vpn_sap_link);
    DQ_INIT(&(*vpn_sap)->vc_mpls_ete_link);
    DQ_INIT(&(*vpn_sap)->vc_ohi_link);

    (*vpn_sap)->mpls_psn_label     = BCM_MPLS_LABEL_INVALID;
    (*vpn_sap)->mpls_psn_label_exp = 0;
    (*vpn_sap)->mpls_psn_label_ttl = 0;


    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit,
                            "vpn(0x%x) allocated fte(0x%x), ohi(0x%x), ete(0x%x)\n"),
                 vpnc->vpn_id, fte_idx,
                 (*vpn_sap)->vc_ohi.ohi,
                 (*vpn_sap)->vc_ete_hw_idx.ete_idx));

    return BCM_E_NONE;
}

STATIC int
_bcm_caladan3_free_mpls_vpn_sap(_caladan3_l3_fe_instance_t  *l3_fe,
                              _caladan3_vpn_sap_t      **vpn_sap)
{
    int  rv;

    sal_free((*vpn_sap)->u.vc_fte);
    DQ_REMOVE(&(*vpn_sap)->vc_mpls_ete_link);
    DQ_REMOVE(&(*vpn_sap)->vc_vpn_sap_link);

    rv = _bcm_caladan3_mpls_free_vpn_sap_hw_resources(l3_fe, *vpn_sap);
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(l3_fe->fe_unit,
                             "Error freeing vpn sap hw resources: %d %s\n"),
                  rv, bcm_errmsg(rv)));
    }

    sal_free(*vpn_sap);
    *vpn_sap = NULL;

    return BCM_E_NONE;
}

static int
_bcm_caladan3_add_mpls_vpn_id(int                     unit,
                            _caladan3_l3_fe_instance_t *l3_fe,
                            bcm_mpls_vpn_config_t  *info)
{
    _caladan3_vpn_control_t          *vpnc;
    int                           status, hidx;
    int                           res_flags;
    uint32                        alloc_vrf, alloc_vsi;

    vpnc      = NULL;
    alloc_vrf = _CALADAN3_INVALID_VRF;
    alloc_vsi = _CALADAN3_INVALID_VPN_ID;
    res_flags = 0;

    if (info->flags & BCM_MPLS_VPN_L3) {

        if (info->lookup_id) {
            alloc_vrf = info->lookup_id;
            res_flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
        } else {
            res_flags = 0;
        }

        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                         SBX_CALADAN3_USR_RES_VRF,
                                         1,
                                         &alloc_vrf,
                                         res_flags);
        if (status != BCM_E_NONE) {
            alloc_vrf = _CALADAN3_INVALID_VRF;
            goto error_out;
        }
    }

    /* VSI is vpn-id for L3, VPLS; no VSI allocated for VPWS */
    if (info->flags & BCM_MPLS_VPN_WITH_ID) {
        alloc_vsi = info->vpn;
        res_flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
    } else {
        res_flags = 0;
    }

    if (info->flags & BCM_MPLS_VPN_VPWS) {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                         SBX_CALADAN3_USR_RES_LINE_VSI,
                                         1,
                                         &alloc_vsi,
                                         res_flags);
    } else {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                         SBX_CALADAN3_USR_RES_VSI,
                                         1,
                                         &alloc_vsi,
                                         res_flags);
    }

    /* if this VPN is in the reserved range, convert the error code to NONE */
    if (status == BCM_E_RESOURCE) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "vpnId 0x%04x found to be in the "
                                 "reserved range, OK.\n"), alloc_vsi));
        status = BCM_E_NONE;
    }

    if (status != BCM_E_NONE) {
        alloc_vsi = _CALADAN3_INVALID_VPN_ID;
        goto error_out;
    }

    status = _bcm_caladan3_mpls_alloc_vpncb(l3_fe, &vpnc);
    if (status != BCM_E_NONE) {
        goto error_out;
    }

    vpnc->vpn_bc_mcg = info->broadcast_group;
    vpnc->vpn_id     = alloc_vsi;
    vpnc->vpn_vrf    = alloc_vrf;
    vpnc->vpn_flags  = info->flags;
    vpnc->vpls_color = (info->flags & BCM_MPLS_VPN_VPLS) ? \
                         SBX_VPLS_COLOR_FTE_BASE(l3_fe->fe_unit):0;
    hidx = _CALADAN3_GET_MPLS_VPN_HASH(vpnc->vpn_id);
    DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx], &vpnc->vpn_fe_link);

    if (!(info->flags & BCM_MPLS_VPN_VPWS)) {
        status = _bcm_caladan3_map_set_vpn_bc_fte(l3_fe, vpnc);
        if (status != BCM_E_NONE) {
            goto error_out;
        }

        status = _bcm_caladan3_mpls_map_set_vlan2etc(l3_fe,
                                                   vpnc,
                                                   vpnc->vpn_id);
        if (status != BCM_E_NONE) {
            goto error_out;
        }
    }

    info->vpn       = vpnc->vpn_id;

    if (_BCM_VRF_VALID(vpnc->vpn_vrf)) {
        l3_fe->fe_vpn_by_vrf[vpnc->vpn_vrf] = vpnc;
    }

    return BCM_E_NONE;

error_out:
    if (vpnc) {
        _bcm_caladan3_mpls_free_vpncb(l3_fe, &vpnc);
    } else {
        if (alloc_vrf != _CALADAN3_INVALID_VRF) {
            _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                   SBX_CALADAN3_USR_RES_VRF,
                                   1,
                                   &alloc_vrf,
                                   0);
        }

        if (alloc_vsi != _CALADAN3_INVALID_VPN_ID) {
            if (info->flags & BCM_MPLS_VPN_VPWS) {
                _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                       SBX_CALADAN3_USR_RES_LINE_VSI,
                                       1,
                                       &alloc_vsi,
                                       0);
            } else {
                _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                       SBX_CALADAN3_USR_RES_VSI,
                                       1,
                                       &alloc_vsi,
                                       0);
            }
        }
    }

    return status;
}

static int
_bcm_caladan3_destroy_all_mpls_vpn_id(int                      unit,
                                    _caladan3_l3_fe_instance_t  *l3_fe)
{
    int                           hidx, status;
    int                           last_error_status;
    _caladan3_vpn_control_t          *vpnc_temp = NULL;

    /**
     * Check to see if all the sap on each of the vpn
     * is deleted already.
     */
    for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
        _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

            if (!DQ_EMPTY(&vpnc_temp->vpn_sap_head)) {
                return BCM_E_BUSY;
            }

        } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);
    }

    last_error_status = BCM_E_NONE;

    /**
     * Actual deletion of the vpn
     */
    for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
        _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

            status =
                _bcm_caladan3_destroy_mpls_vpn_id(unit,
                                                l3_fe,
                                                &vpnc_temp);
            if (status != BCM_E_NONE) {
                last_error_status = status;
            }

        } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);
    }

    return last_error_status;
}

STATIC int
_bcm_caladan3_validate_mpls_vpn_id_create(int                    unit,
                                        bcm_mpls_vpn_config_t *info)
{
    if (!_BCM_MPLS_VPN_VALID_TYPE(info->flags)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid VPN type\n")));
        return BCM_E_PARAM;
    }

    if ((info->flags & (BCM_MPLS_VPN_WITH_ID | BCM_MPLS_VPN_REPLACE)) &&
        !_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, info->vpn)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid vpn id (0x%x) \n"),
                   info->vpn));
        return BCM_E_PARAM;
    }

    /**
     * Caladan3 does not currently support more than one class for
     * broadcast on L2 domain.
     */
    if ((info->broadcast_group != info->unknown_unicast_group) ||
        (info->unknown_unicast_group != info->unknown_multicast_group)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Different Broadcast groups supplied for "
                               "broadcast, unknown unicast and unknown multicast\n")));
        return BCM_E_PARAM;
    }

    if (info->flags & BCM_MPLS_VPN_L3) {
        /* L3-vpn and VPWS do not need l2 bcast */
        if (info->broadcast_group       ||
            info->unknown_unicast_group ||
            info->unknown_multicast_group) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "L3-vpn do not need l2 bcast\n")));
            return BCM_E_PARAM;
        }
    } else if (info->flags & BCM_MPLS_VPN_VPWS) {
        /* L3-vpn and VPWS do not need l2 bcast */
        if (info->broadcast_group       ||
            info->unknown_unicast_group ||
            info->unknown_multicast_group) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "VPWS vpn do not need l2 bcast\n")));
            return BCM_E_PARAM;
        }
        if (info->lookup_id) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "VPWS vpn do not need lookup id\n")));
            return BCM_E_PARAM;
        }
    } else if (info->flags & BCM_MPLS_VPN_VPLS) {
        if (!info->broadcast_group       ||
            !info->unknown_unicast_group ||
            !info->unknown_multicast_group) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "VPLS vpn needs l2 bcast group\n")));
            return BCM_E_PARAM;
        }
        if (info->lookup_id) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "VPLS vpn do not need lookup id\n")));
            return BCM_E_PARAM;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Unsupported VPN type\n")));
        return BCM_E_PARAM;
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_validate_mpls_port_add
 * Purpose:
 *      Validate parameters for adding port to vpn
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn         - vpn identifier
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_validate_mpls_port_add(_caladan3_l3_fe_instance_t *l3_fe,
                                   bcm_vpn_t               vpn,
                                   bcm_mpls_port_t        *mpls_port)
{

    soc_sbx_g3p1_ft_t ft;
    int valid = 0, status = BCM_E_NONE;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));
    if (mpls_port == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls_port param is NULL\n")));
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(l3_fe->fe_unit, vpn)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid VPN (0x%x)\n"),
                   vpn));
        return BCM_E_PARAM;
    }

    if ((vpn > 0) && (vpn < BCM_VLAN_MAX)) {
        /* Ensure the vlan is existing */
        status = soc_sbx_g3p1_ft_get(l3_fe->fe_unit, 
                                     l3_fe->vlan_ft_base + vpn,
                                     &ft);
        if (BCM_SUCCESS(status)) {
            valid = (ft.excidx != VLAN_INV_FTE_EXC) ? 1 : 0;
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Unable to read FTE for Vlan id 0x%x\n"),
                       vpn));
            return BCM_E_PARAM;
        }
            
        if (!valid) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                   "VLAN ID 0x%x does not exists\n"),
                       vpn));
            return BCM_E_PARAM;
        }
    }

    /**
     * YYY: TBD:
     * Since this port specifies the Egress port and in
     * some cases the Ingress port, it has to be of type
     * MOD-PORT.
     */
    if (!BCM_GPORT_IS_SET(mpls_port->port) ||
        !(BCM_GPORT_IS_MODPORT(mpls_port->port) ||
          BCM_GPORT_IS_TRUNK(mpls_port->port))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Unexpected port (0x%x); must be GPORT_MODPORT\n"),
                   mpls_port->port));
        return BCM_E_PARAM;
    }

    if (mpls_port->flags & ~_BCM_MPLS_PORT_SUPPORTED_FLAGS) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid flag.  Supported=0x%x Passed=0x%x\n"),
                   _BCM_MPLS_PORT_SUPPORTED_FLAGS, mpls_port->flags));
        return BCM_E_PARAM;
    }
    if ((mpls_port->flags & BCM_MPLS_PORT_FAILOVER) &&
        (mpls_port->flags & BCM_MPLS_PORT_PW_FAILOVER)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid flags (0x%x).  Both BCM_MPLS_PORT_FAILOVER and "
                              "BCM_MPLS_PORT_PW_FAILOVER are set\n"),
                   mpls_port->flags));
        return BCM_E_PARAM;
    }
    /* MPLS-TP only supports Pseudowires with CW option 
       For VPXS, CW will be added at the entry PSN tunnel and 
       at the egress-PE the CW will be deleted before switching
       the packet*/
/*
     if(((mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
         !(mpls_port->flags & BCM_MPLS_PORT_CONTROL_WORD))){
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "MPLS-TP supports PW only with CW\n")));
        return BCM_E_PARAM;
     }
*/      

    
    if ((mpls_port->service_tpid != 0) && 
             (mpls_port->service_tpid != 0x88a8) &&
             (mpls_port->service_tpid != 0x9100) &&
             (mpls_port->service_tpid != 0x8100)) {
         LOG_ERROR(BSL_LS_BCM_MPLS,
                   (BSL_META_U(l3_fe->fe_unit,
                               "Invalid service_tpid param \n")));
         return BCM_E_PARAM;
    }

        if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_ADD) &&
            (mpls_port->egress_service_vlan >= BCM_VLAN_INVALID)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid service vlan param \n")));
            return BCM_E_PARAM;        
        }
        
        if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_REPLACE) &&
            (mpls_port->flags & BCM_MPLS_PORT_NETWORK)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "SD TAG Translation supported only on Attachment circuits \n")));
            return BCM_E_PARAM;    
        }

        if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_REPLACE) &&
            (mpls_port->egress_service_vlan >= BCM_VLAN_INVALID)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid Translation service vlan param \n")));
            return BCM_E_PARAM;        
        }

        if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_REPLACE) &&
            (mpls_port->flags & BCM_MPLS_PORT_INNER_VLAN_PRESERVE)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid Translation operation with Preservation \n")));
            return BCM_E_PARAM;  
        }


    /* egress tunnel is used for Ethernet transport & tunnel without encap flag is 
     * used for PW handoff, the flags are mutually exclusive */
    if((mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) &&
       (mpls_port->flags & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP)) {
         LOG_ERROR(BSL_LS_BCM_MPLS,
                   (BSL_META_U(l3_fe->fe_unit,
                               "Invalid Egress Tunnel Flags \n")));
         return BCM_E_PARAM;
    }


    if(_BCM_CALADAN3_IS_PWE3_TUNNEL(mpls_port->flags) && 
       !_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, mpls_port->egress_tunnel_if)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Egress tunnel requested with invalid "
                               " egress tunnel interface (0x%x)\n"),
                   mpls_port->egress_tunnel_if));
        return BCM_E_PARAM;
    }


    if (((mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP) ||
          (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET)) &&
         (!_MPLS_EXPMAP_HANDLE_IS_INGR(mpls_port->exp_map))) {
        /* The exp_map is enabled and bogus */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls_port has invalid exp_map %08x\n"),
                   mpls_port->exp_map));
        return BCM_E_PARAM;
    }

    if (((0 == (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP)) &&
         (0 == (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET))) &&
         (mpls_port->exp_map)) {
        /* the exp_map is disabled and nonzero */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls_port has invalid exp_map %08x\n"),
                   mpls_port->exp_map));
        return BCM_E_PARAM;
    }

    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        /* XXX: TBD: Trunk ?? */
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        /* XXX: TBD: Trunk ?? */
        if (mpls_port->match_vlan > BCM_VLAN_MAX) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid vlan passed for MATCH_PORT_VLAN "
                                   "vlan=0x%x\n"), mpls_port->match_vlan));

            return BCM_E_PARAM;
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        if (!BCM_VLAN_VALID(mpls_port->match_vlan)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid outer vlan passed for "
                                   "MATCH_PORT_VLAN_STACKED vlan=0x%x\n"),
                       mpls_port->match_vlan));

            return BCM_E_PARAM;
        }
        if (!BCM_VLAN_VALID(mpls_port->match_inner_vlan)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid inner vlan passed for "
                                   "MATCH_PORT_VLAN_STACKED vlan=0x%x\n"),
                       mpls_port->match_inner_vlan));
            return BCM_E_PARAM;
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
        if (!_BCM_CALADAN3_MPLSTP_LABEL_VALID(mpls_port->match_label)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid label passed for "
                                   "MATCH_LABEL label=0x%x\n"),
                       mpls_port->match_label));

            return BCM_E_PARAM;
        }
        break;

    case BCM_MPLS_PORT_MATCH_COUNT:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid Match criteria=%d\n"),
                   mpls_port->criteria));

        return BCM_E_PARAM;
    default:
        return BCM_E_PARAM;
    }


    /*
     * If failover port id is supplied, check for valid port id 
     */
    if (mpls_port->failover_port_id) {
        if (BCM_GPORT_IS_MPLS_PORT(mpls_port->failover_port_id)) {
            uint32 failover_port_fte;

            failover_port_fte = 
                     BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->failover_port_id);
            if (!_CALADAN3_MPLS_PORT_FTE_VALID(l3_fe->fe_unit, failover_port_fte)) {

                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                       "Invalid failover fte (0x%x)\n"),
                           failover_port_fte));
                return BCM_E_PARAM;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                   "Invalid failover port id (0x%x)\n"),
                       mpls_port->failover_port_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * If failover id was supplied, check for valid
     * backup egress object
     */
    if (mpls_port->failover_id) {
        if (mpls_port->failover_id > 0 && mpls_port->failover_id < 1024) {
            if (!mpls_port->failover_port_id) {
                /* Validity of failover port id is already checked */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Failover port id not supplied (0x%x)\n"),
                           mpls_port->failover_port_id));
                return BCM_E_PARAM;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid failover id (0x%x)\n"),
                       mpls_port->failover_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * If pw failover port id is supplied, check for valid port id 
     */
    if (mpls_port->pw_failover_port_id) {
        if (BCM_GPORT_IS_MPLS_PORT(mpls_port->pw_failover_port_id)) {
            uint32 failover_pw_port_fte;

            failover_pw_port_fte = 
                    BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->pw_failover_port_id);
            if (!_CALADAN3_MPLS_PORT_FTE_VALID(l3_fe->fe_unit, failover_pw_port_fte)) {

                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Invalid failover pw fte (0x%x)\n"),
                           failover_pw_port_fte));
                return BCM_E_PARAM;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid failover pw port id (0x%x)\n"),
                       mpls_port->pw_failover_port_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * If failover pw id was supplied, check for valid
     * backup egress object
     */
    if (mpls_port->pw_failover_id) {
        
        if (mpls_port->pw_failover_id >= 1024 && mpls_port->pw_failover_id < (33*1024)) {
            if (!mpls_port->pw_failover_port_id) {
                /* Validity of pw failover port id is already checked */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Failover pw port id not supplied (0x%x)\n"),
                           mpls_port->pw_failover_port_id));
                return BCM_E_PARAM;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid failover pw id (0x%x)\n"),
                       mpls_port->pw_failover_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * Validate encap id (or OHI)
     */
    if ( mpls_port->encap_id &&
         !(SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id) ||
           SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id) )
        ) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls_port has invalid encap id %08x\n"),
                   mpls_port->encap_id));
        return BCM_E_PARAM;
    }

    /* XXX: TBD: other fields to be validated */

    return BCM_E_NONE;
}


STATIC int
_bcm_caladan3_mpls_port_update(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_vpn_t               vpn_id,
                             _caladan3_vpn_control_t    *vpnc,
                             bcm_mpls_port_t        *mpls_port,
                             _caladan3_vpn_sap_t        *vpn_sap)
{
    int                           status;
    _caladan3_l3_fte_t                gport_fte;
    bcm_port_t                    new_exit_port;
    bcm_module_t                  new_exit_modid = -1, old_exit_modid = -1;
    _caladan3_l3_or_mpls_egress_t     gport_egr;
    uint32                      logicalPort = 0, is_trunk = 0;
#ifdef PLATFORM_LABEL
    uint32                      is_failover = 0;
    uint8                       is_same_label = 0;
    _caladan3_vpn_sap_t               *vpn_sap_tmp = NULL;
#endif
    
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        is_trunk = 1;
    } else {

        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     mpls_port->port,
                                                     &new_exit_modid,
                                                     &new_exit_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) extracting mod-port from gport (0x%x)\n"),
                       bcm_errmsg(status), mpls_port->port));
            return status;
        }
    }

    /**
     * Any match criteria change implies that the mpls-port
     * needs to be deleted and added as new addition.
     */
    if (vpn_sap->vc_mpls_port.criteria != mpls_port->criteria) {
        return BCM_E_PARAM;
    }

    gport_fte.fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
    
    /**
     * read the gport fte to get current (old) exit_modid
     */

    status = _bcm_caladan3_mpls_get_fte(l3_fe,
                                      gport_fte.fte_idx,
                                      L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                      &gport_egr);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) reading gport-fte(0x%x)\n"),
                   bcm_errmsg(status), gport_fte.fte_idx));
        return status;
    }

    old_exit_modid = gport_egr.fte_modid;

    /**
     * update the vpn-sap either locally or remote
     * for the new_exit_modid first
     */
    if (l3_fe->fe_my_modid == new_exit_modid || (is_trunk == 1)) {
        status = _bcm_caladan3_update_vpn_sap_hw(l3_fe,
                                               l3_fe->fe_my_modid,
                                               vpn_sap,
                                               mpls_port);
        if  (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) updating gport-fte(0x%x)\n"),
                       bcm_errmsg(status), gport_fte.fte_idx));
            return status;
        }
    }

    /**
     * Every scenario for updating the egress portion of GPORT-FTE
     * is done by now.
     * We update the GPORT-FTE here before destroy from old ete
     */
    status   = _bcm_caladan3_map_set_mpls_vpn_fte(l3_fe,
                                                vpnc,
                                                vpn_sap,
                                                mpls_port);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) map-set fte(0x%x)\n"),
                   bcm_errmsg(status), gport_fte.fte_idx));
        return status;
    }

    /* store the logical port before deleting vpn_sap */
    logicalPort = vpn_sap->logicalPort;
    /**
     * Delete the vpn-sap from the old-exit-modid,
     * this is deliberately done even when the new and old
     * exit_modid is local. If old and new is same modid,
     * we just link and then unlink
     */
    if (l3_fe->fe_my_modid == old_exit_modid || (is_trunk == 1)) {
        status = _bcm_caladan3_destroy_vpn_sap_hw_resources(l3_fe,
                                                          l3_fe->fe_my_modid,
                                                          &vpn_sap);
        if  (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) destroying ohi-ete for fte(0x%x)\n"),
                       bcm_errmsg(status), gport_fte.fte_idx));
            return status;
        }
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Using lp(%d)\n"),
               logicalPort));

    /**
     * Now we need to program the ingress portion
     * of mpls-port
     */
    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_caladan3_match_port2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_UPDATE,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) updating port2etc\n"),
                       bcm_errmsg(status)));
            return status;
        }

        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_caladan3_match_pvid2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_UPDATE,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) updating pvid2etc\n"), 
                       bcm_errmsg(status)));
            return status;
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
            return BCM_E_PARAM;
        }

        status = _bcm_caladan3_match_pstackedvid2etc(l3_fe,
                                                   _CALADAN3_MPLS_PORT_MATCH_UPDATE,
                                                   logicalPort,
                                                   vpnc,
                                                   vpn_sap,
                                                   mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) updating pstackedvid2etc\n"),
                       bcm_errmsg(status)));
            return status;
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
#ifdef PLATFORM_LABEL
        is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
        status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port,
                                            &vpn_sap_tmp,
                                            (!is_failover));
        if (BCM_SUCCESS(status)) {
            /* port with same vc label exits */
            is_same_label = 1;
        }

        /* dont update label2e if same label and failover */
        if (!(is_same_label && is_failover ) )
#endif
        {
            status = _bcm_caladan3_match_label2etc(l3_fe,
                                                 _CALADAN3_MPLS_PORT_MATCH_UPDATE,
                                                 logicalPort,
                                                 vpnc,
                                                 vpn_sap,
                                                 mpls_port);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) updating label2etc\n"),
                           bcm_errmsg(status)));
                return status;
            }
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        return BCM_E_PARAM;
    }

    /* update the user params in vpn_sap */
    vpn_sap->vc_mpls_port = *mpls_port;

    return BCM_E_NONE;
}


/* Get the requested type of VPWS port 
 */
int
_bcm_caladan3_mpls_vpws_port_sap_get(_caladan3_vpn_control_t *vpnc, 
                                   _bcm_caladan3_mpls_vpws_port_type_t port_type,
                                   _caladan3_vpn_sap_t **vpn_sap)
{
    uint32 search_flag;
    uint32 mask_flags;
    _caladan3_vpn_sap_t *tmp_vpn_sap = NULL;
    _caladan3_vpn_sap_t *vpws_vpn_sap = NULL;
    
    mask_flags = (BCM_MPLS_PORT_FAILOVER |
                 BCM_MPLS_PORT_NETWORK  |
                 BCM_MPLS_PORT_PW_FAILOVER);
    switch (port_type) {
    case _BCM_CALADAN3_MPLS_VPWS_UNI_PORT:
        search_flag = 0;
        break;
    case _BCM_CALADAN3_MPLS_VPWS_UNI_PORT_FO:
        search_flag = BCM_MPLS_PORT_FAILOVER;
        break;
    case _BCM_CALADAN3_MPLS_VPWS_NW_PORT:
        search_flag = BCM_MPLS_PORT_NETWORK;
        break;
    case _BCM_CALADAN3_MPLS_VPWS_NW_PORT_FO:
        
        search_flag = BCM_MPLS_PORT_NETWORK | BCM_MPLS_PORT_FAILOVER;
        break;
    case _BCM_CALADAN3_MPLS_VPWS_NW_PORT_PW_FO:
        search_flag = BCM_MPLS_PORT_NETWORK | BCM_MPLS_PORT_PW_FAILOVER;
        break;
    case _BCM_CALADAN3_MPLS_VPWS_NW_PORT_PW_FO2:
        
        search_flag = BCM_MPLS_PORT_NETWORK | BCM_MPLS_PORT_FAILOVER;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
        if ((tmp_vpn_sap->vc_mpls_port.flags & mask_flags) == search_flag) {
            vpws_vpn_sap = tmp_vpn_sap;
        }
    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

    if (vpws_vpn_sap == NULL) {
        return BCM_E_NOT_FOUND;
    } else {
        *vpn_sap = vpws_vpn_sap;
    }
    return BCM_E_NONE;
}                              

int
_bcm_caladan3_mpls_vpws_fte_connect(_caladan3_l3_fe_instance_t *l3_fe,
                                  _caladan3_vpn_sap_t* vpnSaps[_BCM_CALADAN3_VPWS_MAX_SAP],
                                  _caladan3_mpls_port_action_type action,
                                  uint32 connect)
{
    int           rv = BCM_E_UNAVAIL;
    uint32        fteIdx[_BCM_CALADAN3_VPWS_MAX_SAP];
    int           tmp, idx, hidx, localOh[_BCM_CALADAN3_VPWS_MAX_SAP];
    int           acIndex=-1, peIndex=-1;
    soc_sbx_g3p1_labels_t ilm[_BCM_CALADAN3_VPWS_MAX_SAP];
    int           label1 = 0, label2 = 0, label3 = 0, port = -1;

#define _MPLS_VPN_SAP_SWAP(f)  \
  tmp = vpnSaps[0]->f; vpnSaps[0]->f = vpnSaps[1]->f; vpnSaps[1]->f = tmp;

    for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {
        fteIdx[idx] = BCM_GPORT_MPLS_PORT_ID_GET(vpnSaps[idx]->vc_mpls_port_id);
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter.  p1Fte=0x%x p2Fte=0x%x\n"),
               fteIdx[0], fteIdx[1]));

    if (SOC_WARM_BOOT(l3_fe->fe_unit) == 0) { 
        rv = _bcm_caladan3_g3p1_mpls_vpws_fte_connect(l3_fe, 
                                      fteIdx[0], fteIdx[1], connect);
    } else {
        _MPLS_VPN_SAP_SWAP(vc_mpls_port.port);
        _MPLS_VPN_SAP_SWAP(vc_mpls_port.encap_id);
        return BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv)) {

        /* only locally managed OHIs are in the list, track which are
         * and swap as appropriate below
         */
        for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {
            localOh[idx] = !DQ_NULL(&vpnSaps[idx]->vc_ohi_link);
        }

        /* Update the vpn_sap struct to reflect the swapped ohi, common for
         * all known ucodes
         */
        
        /* swap all sw-state  */
        _MPLS_VPN_SAP_SWAP(vc_ohi.ohi);
        _MPLS_VPN_SAP_SWAP(vc_res_alloced);
        _MPLS_VPN_SAP_SWAP(vc_ete_hw_idx.ete_idx);

        /* update internal state -
         *   remove the sap from the OHI to VC ETE hash table
         *   insert only if the other OHI is local, that is, it
         *   was previously inserted
         */
        for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {
            /* locate the AC label match sap */
            if (!(_BCM_CALADAN3_IS_PWE3_TUNNEL(vpnSaps[idx]->vc_mpls_port.flags))) {
                acIndex = idx;
            } else {
                peIndex = idx;
            }

            DQ_REMOVE(&vpnSaps[idx]->vc_ohi_link);
            DQ_INIT(&vpnSaps[idx]->vc_ohi_link);

            if (localOh[!idx]) {
                hidx = _CALADAN3_GET_OHI2ETE_HASH_IDX(vpnSaps[idx]->vc_ohi.ohi);
                DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx],
                               &vpnSaps[idx]->vc_ohi_link);
                LOG_VERBOSE(BSL_LS_BCM_MPLS,
                            (BSL_META_U(l3_fe->fe_unit,
                                        "Insert ohi=0x%08x into sw hash idx=%d "
                                         "(id=0x%08x)\n"),
                             vpnSaps[idx]->vc_ohi.ohi, hidx,
                             vpnSaps[idx]->vc_mpls_port_id));
            }
        }

        /* If CES PW handoff provision appropriate ILM setting */
        if(vpnSaps[acIndex]->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_LABEL) {

            if(acIndex < 0 || peIndex < 0) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "failed to find Attachment Circuit\n")));            
                return BCM_E_INTERNAL;
            }

            /* If the AC has an Label match criteria, connect ILM of AC
             * to PE Gport */
            for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP && BCM_SUCCESS(rv); idx++) {
#if 0
                rv = soc_sbx_g3p1_labels_get(l3_fe->fe_unit, 
                                              vpnSaps[idx]->vc_mpls_port.match_label,
                                              &ilm[idx]); 
#endif
#ifdef PLATFORM_LABELS
                port = 0;
                label1 = vpnSaps[idx]->vc_mpls_port.match_label;
#else
                
                port = 0;
#ifdef USE_TUNNEL_ID
                label1 = vpnSaps[idx]->mpls_psn_ing_label;
#else
                label1 = vpnSaps[idx]->mpls_psn_label;
#endif
                label2 = vpnSaps[idx]->vc_mpls_port.match_label;
#endif

                rv = _bcm_caladan3_g3p1_mpls_labels_get(l3_fe->fe_unit, port,
                               label1, label2, label3,
                               &ilm[idx]); 
            }

            /* link ILM to PE gport */
            if (BCM_SUCCESS(rv)) {
                switch(action) {
                case _CALADAN3_MPLS_PORT_MATCH_ADD:
                case _CALADAN3_MPLS_PORT_MATCH_UPDATE:   

                    for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) { 
                        ilm[idx].ftidx = fteIdx[idx];
                    }

                    ilm[peIndex].opcode = _BCM_CALADAN3_LABEL_CES(l3_fe->fe_unit);
                    ilm[peIndex].vpws   = 1;
                    break;

                case _CALADAN3_MPLS_PORT_MATCH_DELETE:
                    for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) { 
                        ilm[idx].ftidx = 0;
                    }
                    ilm[peIndex].opcode = 0;
                    ilm[peIndex].vpws   = 0;
                    break;

                default:
                    /* never hits here since action is valided by caller */
                    rv = BCM_E_PARAM;
                    break;
                }
                for(idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP && BCM_SUCCESS(rv); idx++) {
                    rv = _bcm_caladan3_g3p1_mpls_labels_set(l3_fe->fe_unit,
                                             port, label1, label2, label3,
                                             &ilm[idx]); 
                }
                if (BCM_FAILURE(rv)) { 
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit,
                                          "error(%s) Failed to Set Label2e(ILM)\n"), 
                               bcm_errmsg(rv)));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) Failed to Get Label2e(ILM)\n"), 
                           bcm_errmsg(rv)));
            }
        }
    }
    return rv;
}

/*
 * Function:
 *    _bcm_caladan3_mpls_port_is_failover_update
 * Purpose:
 *    Check whether the two bcm_mpls_port_t are differed only by
 *                   i. failover details
 *                  ii. dont care - BCM_MPLS_PORT_REPLACE flag
 * Returns :
 *    1 - If the above condition is true
 *    0 - Otherwise
 */
int _bcm_caladan3_mpls_port_is_failover_update(bcm_mpls_port_t *mpls_port_old,
                                             bcm_mpls_port_t *mpls_port)
{
    uint32 tmp_flags , is_same_flags;
    bcm_mpls_egress_label_t *egress_label, *egress_label_old;
    egress_label = &mpls_port->egress_label;
    egress_label_old = &mpls_port_old->egress_label;
    tmp_flags = mpls_port_old->flags ^ mpls_port->flags;
    is_same_flags = (tmp_flags == 0 ) || (tmp_flags == BCM_MPLS_PORT_REPLACE);
    if ( mpls_port->mpls_port_id        == mpls_port_old->mpls_port_id     &&
         mpls_port->if_class            == mpls_port_old->if_class         &&
         mpls_port->exp_map             == mpls_port_old->exp_map          &&
         mpls_port->int_pri             == mpls_port_old->int_pri          &&
         mpls_port->pkt_pri             == mpls_port_old->pkt_pri          &&
         mpls_port->service_tpid        == mpls_port_old->service_tpid     &&
         mpls_port->port                == mpls_port_old->port             &&
         mpls_port->criteria            == mpls_port_old->criteria         &&
         mpls_port->match_vlan          == mpls_port_old->match_vlan       &&
         mpls_port->match_inner_vlan    == mpls_port_old->match_inner_vlan &&
         mpls_port->match_label         == mpls_port_old->match_label      &&
         mpls_port->egress_tunnel_if    == mpls_port_old->egress_tunnel_if &&
         mpls_port->mtu                 == mpls_port_old->mtu              && 
         mpls_port->egress_service_vlan == mpls_port_old->egress_service_vlan &&
         mpls_port->encap_id            == mpls_port_old->encap_id         &&
         mpls_port->policer_id          == mpls_port_old->policer_id       &&
         egress_label->flags            == egress_label_old->flags        &&
         egress_label->label            == egress_label_old->label        && 
         egress_label->qos_map_id       == egress_label_old->qos_map_id   &&
         egress_label->exp              == egress_label_old->exp          &&
         egress_label->ttl              == egress_label_old->ttl          &&
         egress_label->pkt_pri          == egress_label_old->pkt_pri      &&
         egress_label->pkt_cfi          == egress_label_old->pkt_cfi      &&
         is_same_flags                  == 1) {

         if (mpls_port->failover_id       != mpls_port_old->failover_id ||
              mpls_port->failover_port_id != mpls_port_old->failover_port_id) {
             return 1;
         }

         /* Due to the constraint of pw_failover_id to be a 1 to 1 mapping with 
            VPWS AC gport or VPLS PW gport, application may pass pw_failover_id as 0.
            So only consider pw_failover_port_id
          */
         if (mpls_port->pw_failover_port_id != mpls_port_old->pw_failover_port_id) {

         }
    }
    return 0;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_port_replace
 * Purpose:
 *      Replace or update mpls port
 */
STATIC int 
_bcm_caladan3_mpls_port_replace(int                     unit,
                              _caladan3_l3_fe_instance_t *l3_fe,
                              bcm_mpls_port_t        *mpls_port,
                              bcm_vpn_t               vpn,
                              _caladan3_vpn_control_t    *vpnc,
                              _caladan3_vpn_sap_t        *vpn_sap) 
{
    int status = BCM_E_NONE;
    uint32 fte_idx, fte_idx_fo, fte_idx_to_update;
    _caladan3_vpn_sap_t   *tmp_vpn_sap = NULL, *vpws_ac_sap = NULL;
    int fo_update;
    uint8 is_pw_fo_exists = 0;

    if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
        int                  reconnectPorts = 0;
        _caladan3_vpn_sap_t     *vpnSaps[2];

        /* if this is a VPWS & has two ports in the VPN, un-connect them
         * so each port own's its own FTE information before updating
         */
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "vpn_flags=0x%x\n"),
                   vpnc->vpn_flags));

        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            /* check whether it is only an update for failover port */
            fo_update = _bcm_caladan3_mpls_port_is_failover_update(
                                     &vpn_sap->vc_mpls_port, mpls_port);
            if (fo_update) {
                LOG_DEBUG(BSL_LS_BCM_MPLS, \
                          (BSL_META_U(unit, \
                                      "only failover update."
                                       "Update fte without breaking vpws connection \n")));
                fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
                fte_idx_fo = BCM_GPORT_MPLS_PORT_ID_GET(
                                                 mpls_port->failover_port_id);
                /* the vpws service may have two ports and cross connected,
                   start by assuming only one VPWS port is configured */
                fte_idx_to_update = fte_idx;
            
                /* if there is one more non failover port,
                 *   then the ft of other port should be updated
                 *   as the fte's are cross connected
                 */  
                _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
                    if (!(tmp_vpn_sap->vc_mpls_port.flags & 
                                    BCM_MPLS_PORT_ALL_FAILOVERS)) {
                        if (tmp_vpn_sap->vc_mpls_port.mpls_port_id 
                                    != mpls_port->mpls_port_id) {
                            fte_idx_to_update = BCM_GPORT_MPLS_PORT_ID_GET(
                                       tmp_vpn_sap->vc_mpls_port.mpls_port_id);
                        }
                    }
                } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

                /* update the lp and pid for failover port */
                if ((mpls_port->failover_id != 0) || 
                    (mpls_port->failover_port_id != 0) ) {
                    status = _bcm_caladan3_g3p1_mpls_failover_update(l3_fe, vpnc,
                                                      vpn_sap, mpls_port, 0);
                    if (status != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                   "Updating MPLS with failover failed\n")));
                    }
                }
                /* Update the saved mpls_port in vpn_sap */
                vpn_sap->vc_mpls_port.failover_id = mpls_port->failover_id;
                vpn_sap->vc_mpls_port.failover_port_id =
                                                mpls_port->failover_port_id;
                /* update the ft with failover details */
                status = _bcm_caladan3_g3p1_mpls_vpws_port_failover_update(
                                                       l3_fe->fe_unit,
                                                       fte_idx_to_update,
                                                       fte_idx_fo,
                                                       mpls_port->failover_id);
                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Updating ft with failover failed\n")));
                }
                return status;
            }
        }

        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            int                  num_vpn_ports = 0;
            _caladan3_vpn_sap_t     *tmp_vpn_sap = NULL;
            
            _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
                if (!(tmp_vpn_sap->vc_mpls_port.flags & 
                                 BCM_MPLS_PORT_ALL_FAILOVERS)) {
                    if (num_vpn_ports < _BCM_CALADAN3_VPWS_MAX_SAP) {
                        vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                    }
                    num_vpn_ports++;
                    if ((tmp_vpn_sap->vc_mpls_port.flags & 
                                                   BCM_MPLS_PORT_NETWORK))
                    {
                        if(tmp_vpn_sap->vc_mpls_port.pw_failover_port_id != 0) {
                            is_pw_fo_exists = 1;
                        }
                    } else {
                        vpws_ac_sap = tmp_vpn_sap;
                    }
                }
            } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);
            
            if (num_vpn_ports == 2) {
                status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                            _CALADAN3_MPLS_PORT_MATCH_UPDATE, 0);
                reconnectPorts = 1;
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "error(%s) failed to unlink VPWS\n"),
                               bcm_errmsg(status)));
                }
                if ((is_pw_fo_exists == 1) && (vpws_ac_sap != NULL)) {
                    status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe,
                                                   vpnc, vpws_ac_sap, NULL, 0);
                    if (BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                 (BSL_META_U(unit,
                                      "error(%s) failed to unlink VPWS(PW)\n"),
                                  bcm_errmsg(status)));
                    }
                } 
            }
        }

        status = _bcm_caladan3_mpls_port_update(unit,
                                              l3_fe,
                                              vpn,
                                              vpnc,
                                              mpls_port,
                                              vpn_sap);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "MPLS Port Update Failed error:%s\n"),
                       bcm_errmsg(status)));            

        } else if (reconnectPorts) {
            status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                            _CALADAN3_MPLS_PORT_MATCH_UPDATE, 1);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "error(%s) failed to unlink VPWS\n"),
                           bcm_errmsg(status)));
            }
            if ((is_pw_fo_exists == 1) && (vpws_ac_sap != NULL)) {
                /* If VPWS PW port, check whether the pw failover
                 * attachment is removed.
                 * Reconnect PW FO, if the updated port is
                 *   i. any kind of FO port
                 *  ii. AC port
                 * iii. pw port & pw fo port is not zero.
                 *      If both (i & ii) is not satisfied then it is PW port.
                 */
                if ((mpls_port->flags & BCM_MPLS_PORT_ALL_FAILOVERS) ||
                    (!(mpls_port->flags & BCM_MPLS_PORT_NETWORK)) || 
                    ( mpls_port->pw_failover_port_id != 0)) {
                    status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe,
                                                vpnc, vpws_ac_sap, NULL, 1);
                }
            } 
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "MPLS flag not set to replace\n")));
        status = BCM_E_PARAM;
    }

    return status;
}


/*
 * Function:
 *      _bcm_caladan3_mpls_port_add
 * Purpose:
 *      Add port into existing VPN
 * Parameters:
 *      unit        - FE unit
 *      l3_fe       - l3 fe instance
 *      vpn_id      - vpn identifier
 *      vpnc        - vpn control block
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_caladan3_mpls_port_add(int                     unit,
                          _caladan3_l3_fe_instance_t *l3_fe,
                          bcm_vpn_t               vpn_id,
                          _caladan3_vpn_control_t    *vpnc,
                          bcm_mpls_port_t        *mpls_port,
                          int                     num_vpn_ports)
{
    int                           status, ignore_status;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    _caladan3_fte_idx_t               gport_fte;
    int                           exit_modid = -1, exit_port;
    uint32                      logicalPort = ~0;
    _sbx_caladan3_usr_res_types_t      resType = SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT;
    char                         *serviceName = "<none>";
    _caladan3_vpn_sap_t              *vpn_saps[2], *vpws_ac_sap = NULL;
    bcm_caladan3_mpls_trunk_association_t *trunkAssoc=NULL;
    uint32                      trunkid = -1;
    uint8                       is_trunk = 0, is_pw_fo_exists = 0;
    uint8                       is_mplstp_vpwsuni = 0;
#ifdef PLATFORM_LABEL
    uint8                         is_same_label = 0, is_failover = 0;
    _caladan3_vpn_sap_t          *vpn_sap_tmp = NULL;
#endif
 
    vpn_saps[0] = vpn_saps[1] = NULL;
    gport_fte.fte_idx = 0;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     mpls_port->port,
                                                     &exit_modid,
                                                     &exit_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) unable to get mod-port from port\n"),
                       bcm_errmsg(status)));
            return status;
        }
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
        trunkAssoc = &mpls_trunk_assoc_info[unit][trunkid];
    }

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && 
        !(mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL)) {
        is_mplstp_vpwsuni = 1;
    }

    /**
     * Allocation of gport. Assumption is that the same gport value will be
     * allocated since user will do port_add() on all units one by one.
     *
     * We suggest strongly that the user create GPORT on first unit and then
     * do mpls_port_add() WITH_ID on other unit(s)
     */
    if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
        gport_fte.fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
    }

    resType = (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)? \
        SBX_CALADAN3_USR_RES_FTE_MPLS:SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT;

    if (is_mplstp_vpwsuni) {
        resType = SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT;
    }

    if (mpls_port->flags & BCM_MPLS_PORT_FAILOVER) {
        resType = SBX_CALADAN3_USR_RES_FTE_MPLS;
    }
    if (mpls_port->flags & BCM_MPLS_PORT_PW_FAILOVER) {
        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
            resType = SBX_CALADAN3_USR_RES_FTE_VPXS_PW_FO;
        } else if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            /* When creating VPWS PW Redundant port, RES_FTE_MPLS is used.
               On attaching this to primary PW (or) on creation of VPWS AC Port,
               a FTE in FTE_VPXS_PW_FO is allocated and the Redundant ports FTE 
               content is copied into that */ 
            resType = SBX_CALADAN3_USR_RES_FTE_MPLS;
        }
    }
    if ((mpls_port->flags & BCM_MPLS_PORT_PW_FAILOVER) &&
        !(mpls_port->flags & BCM_MPLS_PORT_WITH_ID) &&
        (vpnc->vpn_flags  & BCM_MPLS_VPN_VPLS)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                               "Creation of VPLS PW_FAILOVER Port is supported only with "
                              "BCM_MPLS_PORT_WITH_ID\n")));
       
        return BCM_E_PARAM;
    }

    status  = _bcm_caladan3_alloc_l3_or_mpls_fte(l3_fe,
                                                 (mpls_port->flags & BCM_MPLS_PORT_WITH_ID)?BCM_L3_WITH_ID:0,
                                                 0,  /* XXX: trunk */
                                                 resType,
                                                 &gport_fte.fte_idx);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) could not allocate mpls-gport-fte\n"),
                   bcm_errmsg(status)));
        return status;
    }

    /* Do not move this setting of mpls_port_id */
    BCM_GPORT_MPLS_PORT_ID_SET(mpls_port->mpls_port_id, gport_fte.fte_idx);

    /**
     * Create the vpn-sap locally irrespective of where the actual egress 
     * for this gport is.  Soft-state only.
     */
    status = _bcm_caladan3_create_mpls_vpn_sap(l3_fe, vpnc, mpls_port, &vpn_sap);
    
    if (status != BCM_E_NONE) {
        ignore_status = _bcm_caladan3_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                     gport_fte.fte_idx,
                                                     0, 0, vpnc, is_trunk);
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) could not create vpn-sap for gport(0x%x)\n"),
                   bcm_errmsg(status), mpls_port->mpls_port_id));
        return status;
    }

    if (exit_modid == l3_fe->fe_my_modid || is_trunk == 1) {
        /* vpn-sap exit is local. */
        status = _bcm_caladan3_update_vpn_sap_hw(l3_fe, l3_fe->fe_my_modid,
                                               vpn_sap, mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) creating mpls-vpn-ete on vpn(0x%x)\n"),
                       bcm_errmsg(status), vpnc->vpn_id));
            ignore_status = _bcm_caladan3_destroy_mpls_fte(l3_fe,
                                                         L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                         gport_fte.fte_idx,
                                                         0,
                                                         0,
                                                         vpnc,
                                                         is_trunk);
            return status;
        }
    } else {
        if (SOC_SBX_IS_VALID_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id)) {
            vpn_sap->vc_ohi.ohi =
                SOC_SBX_OHI_FROM_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id);

        } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id)) {
            vpn_sap->vc_ohi.ohi=
                SOC_SBX_OHI_FROM_L2_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id);
        }

    }

    /**
     * program the gport/fte with exit info
     */
    status = _bcm_caladan3_map_set_mpls_vpn_fte(l3_fe,
                                              vpnc,
                                              vpn_sap,
                                              mpls_port);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) writing fte(0x%x)\n"),
                   bcm_errmsg(status), gport_fte.fte_idx));
        ignore_status = _bcm_caladan3_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                     gport_fte.fte_idx,
                                                     exit_modid,
                                                     SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi),
                                                     vpnc,
                                                     is_trunk);
        return status;
    }

    vpn_sap->vc_mpls_port.encap_id =
        SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);


    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype == SOC_SBX_UCODE_TYPE_G3P1) {
        if ( (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT               ||
              mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN          ||
              mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED ) &&
              (exit_modid == l3_fe->fe_my_modid || is_trunk == 1))
        {
            /* Alloc LPORT only if the port is local or trunk */

            resType = SBX_CALADAN3_USR_RES_LPORT;

            if (is_mplstp_vpwsuni || 
                 ((mpls_port->flags & BCM_MPLS_PORT_FAILOVER)
                  && (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)) ) {
                resType = SBX_CALADAN3_USR_RES_VPWS_UNI_LPORT;
            }

            status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit, resType,
                                             1, &logicalPort, 0);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "failed to allocate Type(%d) logical port: %d (%s)\n"), 
                           resType, status, bcm_errmsg(status)));
                return status;
            }

            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Allocated lp(%d)\n"),
                       logicalPort));
            vpn_sap->logicalPort = logicalPort;

        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {
#ifdef PLATFORM_LABEL
                is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
                status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port,
                                            &vpn_sap_tmp,
                                            (!is_failover));
                if (BCM_SUCCESS(status)) {
                    /* port with same vc label exits */
                    is_same_label = 1;
                    logicalPort = vpn_sap_tmp->logicalPort;
                } else
#endif
                {
                    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                 SBX_CALADAN3_USR_RES_MPLS_LPORT,
                                                 1, &logicalPort, 0);
                    if (BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                               "failed to allocate logical port: %d (%s)\n"),
                                   status, bcm_errmsg(status)));
                        return status;
                    }
                    LOG_DEBUG(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Allocated lp(%d)\n"),
                               logicalPort));
                }
        }
    }

    /**
     * Program the match/ingress information
     */
    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_caladan3_match_port2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_ADD,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        serviceName = "port2etc";
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_caladan3_match_pvid2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_ADD,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        serviceName = "pvid2etc";
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        status = _bcm_caladan3_match_pstackedvid2etc(l3_fe,
                                                   _CALADAN3_MPLS_PORT_MATCH_ADD,
                                                   logicalPort,
                                                   vpnc,
                                                   vpn_sap,
                                                   mpls_port);
        serviceName = "pstackedvid2etc";
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
#ifdef PLATFORM_LABEL
        /* dont update label2e if same label and failover */
        if (!(is_same_label && is_failover ) )
#endif
        {

            status = _bcm_caladan3_match_label2etc(l3_fe,
                                                 _CALADAN3_MPLS_PORT_MATCH_ADD,
                                                 logicalPort,
                                                 vpnc,
                                                 vpn_sap,
                                                 mpls_port);
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        status = BCM_E_PARAM;
    }

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) updating %s\n"),
                   bcm_errmsg(status), serviceName));
        goto error_done;
    }

    /* If port is of TRUNK add SAP to the trunk association list */
    if(BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        DQ_INSERT_HEAD(&trunkAssoc->plist, &vpn_sap->trunk_port_link);
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "MPLS_PORT (0x%x) added to trunk %d\n"),
                     mpls_port->mpls_port_id, trunkid));
    }


    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && (num_vpn_ports == 1) &&
            !(mpls_port->flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {
        _caladan3_vpn_sap_t *vpn_sap_loop;
        int              idx = 0;

        vpn_saps[0] = vpn_saps[1] = NULL;

        _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap_loop) {
            /* cross connect only primary port */
            if (!(vpn_sap_loop->vc_mpls_port.flags & 
                                     BCM_MPLS_PORT_ALL_FAILOVERS)) {
                if (vpn_saps[idx] == NULL) {
                    vpn_saps[idx++] = vpn_sap_loop;
                }
                if ((vpn_sap_loop->vc_mpls_port.flags & BCM_MPLS_PORT_NETWORK))
                {
                    if(vpn_sap_loop->vc_mpls_port.pw_failover_port_id != 0) {
                        is_pw_fo_exists = 1;
                    } else {
                        vpws_ac_sap = vpn_sap_loop;
                    }
                }
            }
        } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap_loop);

        if (vpn_saps[0] == NULL || vpn_saps[1] == NULL) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid vpn saps supplied\n")));
            status = BCM_E_INTERNAL;
            goto error_done;
        }

        /* now we have both the ports to make
         * connection b/w them.
         */
        status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpn_saps, 
                                                _CALADAN3_MPLS_PORT_MATCH_ADD, 1);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "VPWS cross connect failed!! Error: %d/%s\n"),
                       status, bcm_errmsg(status)));
        }

        if (is_pw_fo_exists && (vpn_sap == vpws_ac_sap)) {
            status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe, 
                                                     vpnc, vpn_sap, NULL, 0);
        }
    }

error_done:
    if (BCM_FAILURE(status)) {
        ignore_status = _bcm_caladan3_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                     gport_fte.fte_idx,
                                                     exit_modid,
                                                     SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi),
                                                     vpnc,
                                                     is_trunk);
        COMPILER_REFERENCE(ignore_status);
    } else {
        /* if mpls port add successful, insert the gport to dbase */
        status = shr_htb_insert(mpls_gport_db[unit],
                            (shr_htb_key_t) &vpn_sap->vc_mpls_port_id, 
                            (void*) vpn_sap);        
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) Updating Gport Dbase (0x%x)\n"),
                       bcm_errmsg(status), BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id)));
        }
    }

    return status;
}

STATIC int
_bcm_caladan3_mpls_port_delete(int                      unit,
                             _caladan3_l3_fe_instance_t  *l3_fe,
                             _caladan3_vpn_control_t     *vpnc,
                             _caladan3_vpn_sap_t        **vpn_sap)
{
    int                         status;
    _caladan3_fte_idx_t             fte_idx;
    bcm_mpls_port_t            *mpls_port;
    int                         freeLp = 0;
    uint32                      logicalPort = 0;
    _sbx_caladan3_usr_res_types_t    restype;
    shr_htb_data_t              datum = NULL;
    int8                      is_trunk = 0;
    uint8                     is_mplstp_vpwsuni = 0;
#ifdef PLATFORM_LABEL
    uint32                      is_failover = 0;
    uint8                       is_same_label = 0;
    _caladan3_vpn_sap_t            *vpn_sap_tmp = NULL;
#endif

    mpls_port        = &(*vpn_sap)->vc_mpls_port;
    fte_idx.fte_idx  =
        BCM_GPORT_MPLS_PORT_ID_GET((*vpn_sap)->vc_mpls_port_id);

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        is_trunk = 1;
        logicalPort = (*vpn_sap)->logicalPort;
    }

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && 
        !(mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL)) {
        is_mplstp_vpwsuni = 1;
    }
    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype == SOC_SBX_UCODE_TYPE_G3P1) {

        freeLp = 1;

        logicalPort = (*vpn_sap)->logicalPort;
    }

    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_caladan3_match_port2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_DELETE,
                                            logicalPort,
                                            vpnc,
                                            *vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) deleting port2etc\n"), 
                       bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_caladan3_match_pvid2etc(l3_fe,
                                            _CALADAN3_MPLS_PORT_MATCH_DELETE,
                                            logicalPort,
                                            vpnc,
                                            *vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) deleting pvid2etc\n"),
                       bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        status = _bcm_caladan3_match_pstackedvid2etc(l3_fe,
                                                   _CALADAN3_MPLS_PORT_MATCH_DELETE,
                                                   logicalPort,
                                                   vpnc,
                                                   *vpn_sap,
                                                   mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) deleting pstackedvid2etc\n"),
                       bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
#ifdef PLATFORM_LABEL
        is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
        status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port,
                                            &vpn_sap_tmp,
                                            (!is_failover));
        if (BCM_SUCCESS(status)) {
            /* port with same vc label exits */
            is_same_label = 1;
        }

        /* update label2e and logical port only if
         *  no other port exist with same label */
        if (is_same_label == 0 ) {
#endif
            status = _bcm_caladan3_match_label2etc(l3_fe,
                                                 _CALADAN3_MPLS_PORT_MATCH_DELETE,
                                                 logicalPort,
                                                 vpnc,
                                                 *vpn_sap,
                                                 mpls_port);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "error(%s) deleting label2etc\n"),
                           bcm_errmsg(status)));
            }
#ifdef PLATFORM_LABEL
        } else {
            freeLp = 0;
        }
#endif
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        return BCM_E_PARAM;
    }

    if (is_mplstp_vpwsuni) {
        restype = SBX_CALADAN3_USR_RES_VPWS_UNI_LPORT;
    } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {
        restype = SBX_CALADAN3_USR_RES_MPLS_LPORT;
    } else {
        restype = SBX_CALADAN3_USR_RES_LPORT;
    }

    if (freeLp && (logicalPort != 0)){
        status = _sbx_caladan3_resource_free(l3_fe->fe_unit, restype,
                                        1, &logicalPort, 0);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "!! leak: failed to free logical port[0x%x]: %d (%s)\n"),
                       logicalPort, status, bcm_errmsg(status)));
        }
    }

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        DQ_REMOVE(&(*vpn_sap)->trunk_port_link);
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                 "MPLS_PORT (0x%x) removed from trunk association\n"),
                     mpls_port->mpls_port_id));
    }

    /* find and destroy the gport from gport dbase */
    status = shr_htb_find(mpls_gport_db[unit], 
                          (shr_htb_key_t) &(*vpn_sap)->vc_mpls_port_id,
                          (shr_htb_data_t *)&datum, 1);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "failed to free MPLS Gport dbase element: %d (%s)\n"),
                   status, bcm_errmsg(status)));
    }

    /*
     * destroy FTE, and OHI/ETE based on values in HW FTE
     */
    status = _bcm_caladan3_destroy_mpls_fte(l3_fe,
                                          L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE,
                                          fte_idx.fte_idx,
                                          0,/* module */
                                          0,/* ohi    */
                                          vpnc,
                                          is_trunk);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) destroying mpls-fte(0x%x)\n"),
                   bcm_errmsg(status), fte_idx.fte_idx));
        return status;
    }

    return BCM_E_NONE;
}


int
_bcm_caladan3_mpls_port_vlan_vector_internal(int unit,
                                             bcm_gport_t gport,
                                             bcm_port_t *phy_port,
                                             bcm_vlan_t *match_vlan,
                                             bcm_vlan_t *vsi,
                                             uint32   *logicalPort,
                                             bcm_mpls_port_t *mpls_port,
                                             uint8    *vpwsuni,
                                             uint16   *num_ports) /*for trunk*/
{
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    int                          status = BCM_E_NONE;
    int                          exit_modid, mymodid;
    uint8                      is_vpws_uni=0;
    uint16                     index, pindex = 0;
    uint32                     fte_idx=0, lpi=0;
    bcm_trunk_t                  trunk_id = -1;
    bcm_trunk_add_info_t         *trunk_info = NULL;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    if ((status = bcm_stk_my_modid_get(unit, &mymodid)) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "bcm_stk_my_modid_get failed with %d: %s\n"),
                   status, bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * Note: this can be excruciatingly slow if there are
     * a lot of active VPNs and multiple ports in each VPN
     */
    status = 
        _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if ( BCM_FAILURE(status) || vpn_sap == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls gport 0x%x not found\n"),
                   gport));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (!(vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT ||
          vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls gport 0x%x cannot be updated\n"),
                   gport));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MODPORT(vpn_sap->vc_mpls_port.port)) {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     vpn_sap->vc_mpls_port.port,
                                                     &exit_modid,
                                                     phy_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) extracting mod-port from gport 0x%x\n"),
                       bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
            L3_UNLOCK(unit);
            return status;
        }
        if (num_ports != NULL) {
            *num_ports = 1;
        }

        if (exit_modid != mymodid) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) attempting to update MPLS gport"
                                   " of a remote port\n"), bcm_errmsg(status)));
            L3_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    } else if (BCM_GPORT_IS_LOCAL(vpn_sap->vc_mpls_port.port)) {
        *phy_port = BCM_GPORT_LOCAL_GET(vpn_sap->vc_mpls_port.port);
        if (num_ports != NULL) {
            *num_ports = 1;
        }
    } else if (BCM_GPORT_IS_TRUNK(vpn_sap->vc_mpls_port.port)) {
        if (num_ports == NULL || phy_port == NULL) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "NULL Parameters")));
            L3_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
        trunk_id = BCM_GPORT_TRUNK_GET(vpn_sap->vc_mpls_port.port);
        trunk_info = &(mpls_trunk_assoc_info[unit][trunk_id].add_info);
        for (index = 0; index < trunk_info->num_ports; index++) {
            if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
               if (*num_ports > pindex) {
                   phy_port[pindex++] = trunk_info->tp[index];
               }
            }
        }
        *num_ports = pindex;
        if (num_ports == 0) {
            /* no port on the local module */
            L3_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    } else if (SOC_PORT_VALID(unit, vpn_sap->vc_mpls_port.port)) {
        *phy_port = vpn_sap->vc_mpls_port.port;
        if (num_ports != NULL) {
            *num_ports = 1;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) attempting to update an "
                               "invalid MPLS gport (0x%x)\n"),
                   bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
        L3_UNLOCK(unit);
        return BCM_E_PORT;
    }

    fte_idx =  (BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id)); 
    lpi     = vpn_sap->logicalPort;

    if ((fte_idx >= SBX_VPWS_UNI_FTE_BASE(unit)) &&
        (fte_idx <= SBX_VPWS_UNI_FTE_END(unit))) {
        is_vpws_uni = 1;
    }

    if (vpwsuni) {
        *vpwsuni = is_vpws_uni;
    } 
    
    if (is_vpws_uni) {
        fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
        status  = _bcm_caladan3_g3p1_mplstp_vpws_ft_lp_offset(unit, 
                                                           vpn_sap->vc_mpls_port_id,
                                                           &lpi,
                                                           &fte_idx);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) to update "
                                   "vpws uni offset for MPLS gport (0x%x)\n"),
                       bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
            L3_UNLOCK(unit);
            return status;
        }
    } else {
        fte_idx -= l3_fe->vlan_ft_base;
    }

    /*
     * Assumed that this macro always gives the actual VSI/vlan
     * programmed in pv2e
     */
    if (vsi) {
        if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
            *vsi = vpn_sap->vc_vpnc->vpn_id;
        } else {
            *vsi = fte_idx;
        }
    }

    if (logicalPort) {
        *logicalPort = lpi;
    }

    if (match_vlan) {
        *match_vlan = vpn_sap->vc_mpls_port.match_vlan;
    }

    if (mpls_port) {
        status = _bcm_caladan3_fill_mpls_port_from_vpn_sap(l3_fe,
                                                         vpn_sap,
                                                         mpls_port);
    }

    L3_UNLOCK(unit);

    return status;
}

int
_bcm_caladan3_mpls_port_gport_attr_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_port_t *phy_port,
                                     bcm_vlan_t *match_vlan,
                                     bcm_vlan_t *vsi)
{
    uint16 num_ports = 1;
    return (_bcm_caladan3_mpls_port_vlan_vector_internal(unit,
                                                       gport,
                                                       phy_port,
                                                       match_vlan,
                                                       vsi,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       &num_ports));
}

int
_bcm_caladan3_mpls_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, match_vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   *phy_port;
    soc_sbx_g3p1_pv2e_t          p3pv2e;
    uint32                       logicalPort = ~0;
    bcm_mpls_port_t              mpls_port;
    uint8                      vpwsuni=0;
    uint16                     num_ports = 0, index = 0;

    phy_port = sal_alloc(sizeof(bcm_port_t) * BCM_TRUNK_MAX_PORTCNT, "port vlan vector set");
    if (phy_port == NULL) {
        return BCM_E_MEMORY;
    }

    num_ports = BCM_TRUNK_MAX_PORTCNT;
    
    status = _bcm_caladan3_mpls_port_vlan_vector_internal(unit,
                                                          gport,
                                                          phy_port,
                                                          &match_vid,
                                                          &vpn,
                                                          &logicalPort,
                                                          &mpls_port,
                                                          &vpwsuni,
                                                          &num_ports);
    if (BCM_SUCCESS(status)) {
        for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
            for (index = 0; index < num_ports; index++) {
                /* Always need to read the entry */
                soc_sbx_g3p1_pv2e_t_init(&p3pv2e);
                status = SOC_SBX_G3P1_PV2E_GET(unit, phy_port[index],
                                               vid, &p3pv2e);
                if (BCM_FAILURE(status)) {
                    break;
                }


                if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
                    /* this VID is a member of the vector, set it up */
                    p3pv2e.vlan = vpn;
                    p3pv2e.lpi = logicalPort;
                    p3pv2e.vpws = (vpwsuni)?1:0;
                } else if (!BCM_VLAN_VEC_GET(vlan_vec, vid) &&
                           (p3pv2e.vlan == vpn) && (p3pv2e.lpi == logicalPort)) {
                    /* match vid must be reset through mpls port delete */
                    if (!((mpls_port.criteria & BCM_MPLS_PORT_MATCH_PORT_VLAN) &&
                          (mpls_port.match_vlan == vid))) {
                        p3pv2e.vlan = 0;
                        p3pv2e.lpi = 0;
                        p3pv2e.vpws = 0;
                    }
                }
                status = _soc_sbx_g3p1_pv2e_set(unit, phy_port[index], vid,
                                                &p3pv2e);
        
                if (BCM_FAILURE(status)) {
                    break;
                }
            }
        }
    }


    if (phy_port != NULL) {
        sal_free(phy_port);
    }
    
    return status;
}

int
_bcm_caladan3_mpls_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;
    soc_sbx_g3p1_pv2e_t          p3pv2e;
    uint8                      vpwsuni=0;
    uint16                     num_ports = 1;

    BCM_IF_ERROR_RETURN
        (_bcm_caladan3_mpls_port_vlan_vector_internal(unit,
                                                    gport,
                                                    &phy_port,
                                                    NULL,
                                                    &vpn,
                                                    NULL,
                                                    NULL,
                                                    &vpwsuni,
                                                    &num_ports));

    BCM_VLAN_VEC_ZERO(vlan_vec);

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
        /* If trunk, only one port is used to get the vlan vector.
         * It is assumed that all trunk port are configured for same vector.
         */
        soc_sbx_g3p1_pv2e_t_init(&p3pv2e);
        status = soc_sbx_g3p1_pv2e_get(unit, vid, phy_port,
                                       &p3pv2e);
        if (BCM_E_NONE == status) {
            if (p3pv2e.vlan == vpn && p3pv2e.vpws == vpwsuni) {
                BCM_VLAN_VEC_SET(vlan_vec, vid);
            }
        }

        if (BCM_FAILURE(status)) {
            break;
        }
    }
    
    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_init
 * Description:
 *      initialize mpls structures and h/w
 * Parameters:
 *      unit         - fe unit
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_XXX    - on failure
 * Assumption:
 */

int
bcm_caladan3_mpls_init(int unit)
{
    int                           status;
    _caladan3_l3_fe_instance_t       *l3_fe;
    
    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!L3_LOCK_CREATED_ALREADY(unit)) {
        return BCM_E_INTERNAL;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        status = _bcm_caladan3_mpls_cleanup(unit, l3_fe);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "mpls detach failed\n")));
            L3_UNLOCK(unit);
            return status;
        }
    }

    status = _bcm_caladan3_mpls_init(unit,
                                   l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_caladan3_mpls_cleanup
 * Description:
 *      cleanup mpls related items
 * Parameters:
 *      unit         - fe unit
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_XXX    - on failure
 * Assumption:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_caladan3_mpls_cleanup(int unit)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /**
     * For some reason the LOCK was not
     * created, then there is *really*
     * nothing to clean. Hence return
     * BCM_E_NONE
     */
    if (!L3_LOCK_CREATED_ALREADY(unit)) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /**
     * Do not call _cleanup() without _init()
     */
    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_mpls_cleanup(unit,
                                      l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_caladan3_mpls_vpn_id_create(int                    unit,
                              bcm_mpls_vpn_config_t *info)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_validate_mpls_vpn_id_create(unit, info);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid parameters\n")));
        L3_UNLOCK(unit);
        return status;
    }

    if (info->flags & BCM_MPLS_VPN_REPLACE) {
        status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                                   l3_fe,
                                                   info->vpn,
                                                   &vpnc);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return BCM_E_NOT_FOUND;
        }
        status = _bcm_caladan3_update_mpls_vpn_id(unit,
                                                l3_fe,
                                                vpnc,
                                                info);
    } else {

        if (info->flags & BCM_MPLS_VPN_WITH_ID) {
            status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                                       l3_fe,
                                                       info->vpn,
                                                       &vpnc);
            if (status == BCM_E_NONE) {
                L3_UNLOCK(unit);
                return BCM_E_EXISTS;
            }
        }

        status = _bcm_caladan3_add_mpls_vpn_id(unit, l3_fe, info);
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_mpls_vpn_id_destroy(int       unit,
                               bcm_vpn_t vpn)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    if (!DQ_EMPTY(&vpnc->vpn_sap_head)) {
        L3_UNLOCK(unit);
        return BCM_E_BUSY;
    }

    status = _bcm_caladan3_destroy_mpls_vpn_id(unit, l3_fe, &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_caladan3_mpls_vpn_id_destroy_all(int unit)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_destroy_all_mpls_vpn_id(unit,
                                                 l3_fe);
    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_mpls_vpn_id_get(int                    unit,
                             bcm_vpn_t              vpn,
                             bcm_mpls_vpn_config_t *info)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_fill_mpls_vpn_config(unit,
                                              vpnc,
                                              info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_caladan3_mpls_vpn_traverse(int   unit,
                             bcm_mpls_vpn_traverse_cb cb,
                             void  *user_data)
{
    bcm_mpls_vpn_config_t info;
    _caladan3_vpn_control_t          *vpnc = NULL;
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (cb == NULL) {
        return BCM_E_PARAM;
    }
  
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    _CALADAN3_ALL_VPNC(l3_fe, vpnc) {
        bcm_mpls_vpn_config_t_init(&info);
        status = _bcm_caladan3_fill_mpls_vpn_config(unit, vpnc, &info);
        if (BCM_SUCCESS(status)) {
            cb(unit, &info, user_data);
        }
    } _CALADAN3_ALL_VPNC_END(l3_fe, vpnc);

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

int
bcm_caladan3_mpls_remote_update_vpn_sap_hw(int              unit,
                                         bcm_module_t     caller_module,
                                         bcm_vpn_t        vpn_id,
                                         bcm_mpls_port_t *mpls_port)
{
    int                           status, ignore_status;
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap;

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn_id)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid user handle 0x%x\n"),
                   (int)vpn_id));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }
    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn_id,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Could not find vpn id 0x%x\n"),
                   vpn_id));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_create_mpls_vpn_sap(l3_fe,
                                             vpnc,
                                             mpls_port,
                                             &vpn_sap);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) creating vpn-sap\n"),
                   bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_update_vpn_sap_hw(l3_fe,
                                           caller_module,
                                           vpn_sap,
                                           mpls_port);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) updating vpn-sap\n"),
                   bcm_errmsg(status)));
        if (vpn_sap->vc_inuse_ue == 0) {
            ignore_status = _bcm_caladan3_free_mpls_vpn_sap(l3_fe,
                                                          &vpn_sap);
            COMPILER_REFERENCE(ignore_status);
        }
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_mpls_vpls_encap_get
 * Purpose:
 *      Given VPN hint(group) and the mpls_port_id
 *      we need to return the encap_id associated
 * Parameters:
 *      unit         - FE unit
 *      group        - broadcast group for the vpn
 *      port         - port
 *      mpls_port_id - gport of port in vpn
 *      encap_id     - encap_id returned
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This is called from outside the MPLS module
 *      and hence needs to LOCK/UNLOCK before accessing
 *      internal structures
 */

int
_bcm_caladan3_mpls_vpls_encap_get(int              unit,
                                bcm_multicast_t  group,
                                bcm_gport_t      port,
                                bcm_gport_t      mpls_port_id,
                                bcm_if_t        *encap_id)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap;

    if (!group || !encap_id || !mpls_port_id) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_mcg(l3_fe,
                                                group,
                                                &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe,
                                            vpnc,
                                            mpls_port_id,
                                            &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (vpn_sap->vc_ohi.ohi != _CALADAN3_INVALID_OHI) {
        *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    L3_UNLOCK(unit);
    return BCM_E_NOT_FOUND;
}


static int
_bcm_caladan3_mpls_vpn_stp_internal(int unit,
                                  bcm_vpn_t  vpn,
                                  bcm_port_t port,
                                  _caladan3_l3_fe_instance_t **l3_fe,
                                  _caladan3_vpn_control_t **vpnc)
{
    int status = BCM_E_NONE;

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_SET(port) ||
        !BCM_GPORT_IS_MPLS_PORT(port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    *l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (*l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(*l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               *l3_fe,
                                               vpn,
                                               vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if ((*vpnc)->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    L3_UNLOCK(unit);
    return status;
}

static int
_bcm_caladan3_g3p1_mpls_vpn_stp_set_get(int                 unit,
                                      bcm_vpn_t           vpn,
                                      bcm_port_t          port,
                                      int                 *stp_state,
                                      _caladan3_vpn_control_t *vpnc,
                                      int                 set)
{
    bcm_vlan_t           vid;
    bcm_mpls_port_t      *mpls_port = NULL;
    bcm_port_t           phy_port;
    soc_sbx_g3p1_p2e_t   p2e;
    int                  rv = BCM_E_NONE;
    _caladan3_vpn_sap_t      *vpn_sap = NULL;
    int                  mymodid = 0;
    int                  gmodid;
    uint8              is_trunk = 0;
    uint16             num_ports = 0, index;
    bcm_trunk_add_info_t *trunk_info = NULL;
    uint32             trunkid;

    L3_LOCK(unit);
    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
        if (vpn_sap->vc_mpls_port_id == port) {
            vid = BCM_VLAN_INVALID;
            mpls_port = &vpn_sap->vc_mpls_port;
            phy_port = mpls_port->port;

            if (BCM_GPORT_IS_SET(phy_port) &&
                BCM_GPORT_IS_MODPORT(phy_port)) {

                rv = bcm_stk_my_modid_get(unit, &mymodid);
                if (rv != BCM_E_NONE) {
                    L3_UNLOCK(unit);
                    return rv;
                }

                gmodid = BCM_GPORT_MODPORT_MODID_GET(phy_port);
                if (gmodid == mymodid) {
                    phy_port = BCM_GPORT_MODPORT_PORT_GET(phy_port);
                }
                num_ports = 1;
            } else if(BCM_GPORT_IS_TRUNK(phy_port)) {
                is_trunk = 1;
                trunkid = BCM_GPORT_TRUNK_GET(phy_port);
                trunk_info = &(mpls_trunk_assoc_info[unit][trunkid].add_info);
                num_ports = trunk_info->num_ports;
            }

            switch (mpls_port->criteria) {
            case BCM_MPLS_PORT_MATCH_PORT:
            {
                for(index = 0; index < num_ports; index++) {
                    if (is_trunk == 1) {
                        phy_port = trunk_info->tp[index];
                        if (trunk_info->tm[index] != mymodid) {
                            continue;
                        }
                    }
                    soc_sbx_g3p1_p2e_t_init(&p2e);

                    rv = soc_sbx_g3p1_p2e_get(unit,
                                              phy_port,
                                              &p2e);
                    if (rv == BCM_E_NONE) {
                        vid = p2e.nativevid;
                        if (set) {
                            rv = _bcm_caladan3_g3p1_stg_vid_stp_set(unit, vid,
                                                         phy_port, *stp_state);
                            if (rv != BCM_E_NONE) {
                                break;
                            }
                        } else {
                            rv = _bcm_caladan3_g3p1_stg_vid_stp_get(unit, vid,
                                                          phy_port, stp_state);
                            /* It is assumed that all the trunk ports will
                               have the same stp, so break out of the loop */
                            if (rv == BCM_E_NONE) {
                                break;
                            }
                        }
                    }
                }
            }
            break;
            case BCM_MPLS_PORT_MATCH_PORT_VLAN:
            {
                for(index = 0; index < num_ports; index++) {
                    if (is_trunk == 1) {
                        phy_port = trunk_info->tp[index];
                        if (trunk_info->tm[index] != mymodid) {
                            continue;
                        }
                    }
                    vid = mpls_port->match_vlan;
                    if (set) {
                        rv = _bcm_caladan3_g3p1_stg_vid_stp_set(unit, vid,
                                                          phy_port, *stp_state);
                        if (rv != BCM_E_NONE) {
                            break;
                        }
                    } else {
                        rv = _bcm_caladan3_g3p1_stg_vid_stp_get(unit, vid,
                                                          phy_port, stp_state);
                        /* It is assumed that all the trunk ports will
                           have the same stp, so break out of the loop */
                        if (rv == BCM_E_NONE) {
                            break;
                        }
                    }
                }
            }
            break;
            case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
            {
                for(index = 0; index < num_ports; index++) {
                    if (is_trunk == 1) {
                        phy_port = trunk_info->tp[index];
                        if (trunk_info->tm[index] != mymodid) {
                            continue;
                        }
                    }
                    rv = _bcm_caladan3_g3p1_stg_stacked_vid_stp_set_get(unit,
                                                                  mpls_port->match_vlan,
                                                                  mpls_port->match_inner_vlan,
                                                                  phy_port,
                                                                  stp_state,
                                                                  set);
                     
                    if (((rv != BCM_E_NONE) && (set == 1)) ||
                        ((rv == BCM_E_NONE) && (set == 0))) {
                        /* Break out of trunk port loop, if either the set fails or get pass */
                        break;
                    }
                }
            }
            break;
            case BCM_MPLS_PORT_MATCH_LABEL:
            {
                rv = _bcm_caladan3_g3p1_stg_label_stp_set_get(unit,
                                                            mpls_port->match_label,
                                                            stp_state,
                                                            set);
            }
            break;
            default:
                rv = BCM_E_PARAM;
                break;
            }

            break;
        }
    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

    L3_UNLOCK(unit);
    return rv;
}

/*
 * Internal access routine
 *
 * vpn:   VPN
 * port:  MPLS port
 * stp_state:  STG stp state
 */
int
_bcm_caladan3_mpls_vpn_stp_set(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        stp_state)
{
    _caladan3_l3_fe_instance_t *l3_fe = NULL;
    int                    status = BCM_E_NONE;
    _caladan3_vpn_control_t    *vpnc = NULL;
    int                    stpState = stp_state;

    BCM_IF_ERROR_RETURN
        (_bcm_caladan3_mpls_vpn_stp_internal(unit, vpn, port,
                                           &l3_fe, &vpnc));

    if (SOC_IS_SBX_G3P1(unit)) {
        return (_bcm_caladan3_g3p1_mpls_vpn_stp_set_get(unit, vpn,
                                                      port, &stpState,
                                                      vpnc, TRUE));
    } 
    else {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(unit,
                             "WARNING %s has not been ported for this microcode (%s:%d)\n"),
                  FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_UNAVAIL;
    }
    return status;
}

int
_bcm_caladan3_mpls_vpn_stp_get(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        *stp_state)
{
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    int                          status = BCM_E_NONE;
    _caladan3_vpn_control_t          *vpnc = NULL;

    BCM_IF_ERROR_RETURN
        (_bcm_caladan3_mpls_vpn_stp_internal(unit, vpn, port,
                                           &l3_fe, &vpnc));

    if (SOC_IS_SBX_G3P1(unit)) {
        return (_bcm_caladan3_g3p1_mpls_vpn_stp_set_get(unit, vpn,
                                                      port, stp_state,
                                                      vpnc, FALSE));
    } else {
        LOG_WARN(BSL_LS_BCM_MPLS,
                 (BSL_META_U(unit,
                             "WARNING %s has not been ported for this microcode (%s:%d)\n"),
                  FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_UNAVAIL;
    }

    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_port_add
 * Purpose:
 *      Add port into existing VPN
 * Parameters:
 *      unit        - FE unit
 *      vpn         - vpn where port is added
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

int
bcm_caladan3_mpls_port_add(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status = BCM_E_NONE;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    int                           is_failover;
    int                           is_vlan_vsi = 0;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }
    if (( vpn > 0) && (vpn < BCM_VLAN_MAX)) {
        is_vlan_vsi = 1;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        status = BCM_E_UNIT;

    } else {
        if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "mpls is not initialized\n")));
            status = BCM_E_INIT;
        } else {
            status = _bcm_caladan3_validate_mpls_port_add(l3_fe,
                                                        vpn,
                                                        mpls_port);
            if (BCM_SUCCESS(status)) {
                status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                                           l3_fe,
                                                           vpn,
                                                           &vpnc);

                if (status == BCM_E_NOT_FOUND) {
                    if (is_vlan_vsi) {
                        status = _bcm_caladan3_alloc_vpncb_for_vlan_vsi(unit, 
                                                                      l3_fe,
                                                                      vpn,
                                                                      &vpnc);
                    } else if (vpn == BCM_VLAN_INVALID) {
                        
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "Not supported, use vlan id "
                                               "instead of VPN id to add mpls_port to a vlan\n")));
                        status = BCM_E_PARAM;
                         
                    }
                }
                if (BCM_SUCCESS(status)) {
                    /* */
                    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "MPLS Port functions apply only L2 VPN i.e., VPWS AND VPLS\n")));
                        status = BCM_E_PARAM;

                    } else {
                        if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
                            status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe,
                                                                    vpnc,
                                                                    mpls_port->mpls_port_id,
                                                                    &vpn_sap);
                        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN) {
                            status = _bcm_caladan3_find_vpn_sap_by_port_vlan(l3_fe,
                                                                           vpnc,
                                                                           mpls_port->port,
                                                                           mpls_port->match_vlan,
                                                                           &vpn_sap);
                        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {
                            is_failover = mpls_port->flags & \
                                           BCM_MPLS_PORT_FAILOVER;
                            status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                                        vpnc,
                                                        mpls_port,
                                                        &vpn_sap,
                                                        is_failover);
                        } else {
                            status = _bcm_caladan3_find_vpn_sap_by_port(l3_fe,
                                                                      vpnc,
                                                                      mpls_port->port,
                                                                      &vpn_sap);
                        }

                        if (BCM_SUCCESS(status)) {

                            if (BCM_SUCCESS(status)) {
                                if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
                                    status = _bcm_caladan3_mpls_port_replace(unit, l3_fe, mpls_port, 
                                                                           vpn, vpnc, vpn_sap);
                                } else {
                                    LOG_ERROR(BSL_LS_BCM_MPLS,
                                              (BSL_META_U(unit,
                                                          "MPLS Port Exists - use replace to update it\n")));
                                    status = BCM_E_EXISTS;
                                }
                            }

                        } else {
                            if (!(mpls_port->flags & BCM_MPLS_PORT_REPLACE)) {
                                
                                int num_vpn_ports = 0;
                                status = BCM_E_NONE;
                                /* Do not allow more than 2 ports to be in VPWS */
                                if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS &&
                                       !(mpls_port->flags &
                                                BCM_MPLS_PORT_ALL_FAILOVERS)) {
                                    /* VPWS are configured such that two VSIs are allocated an pointed
                                     * at each other.  VPLS, allocates from the local_gport pool so the
                                     * fti may be used as a PID.
                                     */
                                    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                                        if (!(vpn_sap->vc_mpls_port.flags & 
                                                BCM_MPLS_PORT_ALL_FAILOVERS)) {
                                            if (++num_vpn_ports >= 2) {
                                                LOG_ERROR(BSL_LS_BCM_MPLS,
                                                          (BSL_META_U(unit,
                                                                       "2 ports already exist for vpn(0x%x)\n"),
                                                           vpnc->vpn_id));
                                                status =  BCM_E_FULL;
                                                break;
                                            }
                                        }
                                    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);
                                }
                                
                                if (BCM_SUCCESS(status)) {
                                    status = _bcm_caladan3_mpls_port_add(unit, l3_fe,
                                                                       vpn, vpnc,
                                                                       mpls_port, num_vpn_ports);
                                }
                            }
                            /* replace an !found error handled implcit */
                        } /* check for mpls gport */
                    } /* l3 vpn check */
                } /* invalid vpn */
            } /* valid mpls port */
        } /* INITIALIZED(l3_fe) */
    } /* !l3_fe */

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_port_delete
 * Purpose:
 *      Delete port from existing VPN
 * Parameters:
 *      unit        - FE unit
 *      vpn         - vpn where port is deleted
 *      mpls_port_id- port identifier, gport
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

int
bcm_caladan3_mpls_port_delete(int               unit,
                            bcm_vpn_t         vpn,
                            bcm_gport_t       mpls_port_id)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap;
    int8                        is_pw_fo_exists = 0;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_MPLS_PORT(mpls_port_id)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* Port functions are only for VPWS AND VPLS */
    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe,
                                            vpnc,
                                            mpls_port_id,
                                            &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    /* if this is a VPWS & has two ports in the VPN, un-connect them
     * so each port own's its own FTE information before deleting
     */
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "vpn_flags=0x%x\n"),
               vpnc->vpn_flags));
    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        int                  num_vpn_ports = 0;
        _caladan3_vpn_sap_t     *tmp_vpn_sap = NULL, *vpnSaps[_BCM_CALADAN3_VPWS_MAX_SAP];

        _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
            if (!(tmp_vpn_sap->vc_mpls_port.flags & 
                                 BCM_MPLS_PORT_ALL_FAILOVERS)) {
                if (num_vpn_ports < _BCM_CALADAN3_VPWS_MAX_SAP) {
                    vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                }
                num_vpn_ports++;
                if ((tmp_vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_NETWORK) &&
                    (tmp_vpn_sap->vc_mpls_port.pw_failover_port_id != 0)) {
                    is_pw_fo_exists = 1;
                }
            }
        } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

        if (num_vpn_ports == 2 && !(vpn_sap->vc_mpls_port.flags & 
                                               BCM_MPLS_PORT_ALL_FAILOVERS)) {
            
            status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                               _CALADAN3_MPLS_PORT_MATCH_DELETE, 0);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "error(%s) failed to unlink VPWS\n"),
                           bcm_errmsg(status)));
            }
            if (is_pw_fo_exists == 1) {
                if (!(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_NETWORK)) {
                    status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe,
                                                     vpnc, vpn_sap, NULL, 0);
                } else {
                    /* If VPWS PW port is deleted without delinkin PW FO port */
                    status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe,
                                     vpnc, vpn_sap, &vpn_sap->vc_mpls_port, 0);
                }
            } 
        }
    }


    status = _bcm_caladan3_mpls_port_delete(unit,
                                          l3_fe,
                                          vpnc,
                                          &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) && (vpn > 0) && (vpn < BCM_VLAN_MAX)) {
        if (DQ_EMPTY(&vpnc->vpn_sap_head)) {
            /* No more reference to the vlan vsi based VpnCb, free it */
            _bcm_caladan3_free_vpncb_for_vlan_vsi(unit, l3_fe, &vpnc);
        }
    }


    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_caladan3_mpls_port_delete_all(int               unit,
                                bcm_vpn_t         vpn)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;
    int                           last_error_status;
    _caladan3_vpn_control_t          *vpnc = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* Port functions are only for VPWS AND VPLS */
    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    last_error_status = BCM_E_NONE;

    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {

        status = _bcm_caladan3_mpls_port_delete(unit,
                                              l3_fe,
                                              vpnc,
                                              &vpn_sap);
        if (status != BCM_E_NONE) {
            last_error_status = status;
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) && (vpn > 0) && (vpn < BCM_VLAN_MAX)) {
        if (DQ_EMPTY(&vpnc->vpn_sap_head)) {
            /* No more reference to the vlan vsi based VpnCb, free it */
            _bcm_caladan3_free_vpncb_for_vlan_vsi(unit, l3_fe, &vpnc);
        }
    }


    L3_UNLOCK(unit);
    return last_error_status;
}

int
bcm_caladan3_mpls_port_get(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap;
    int                          is_failover;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!mpls_port) {
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
        if(!BCM_GPORT_IS_MPLS_PORT(mpls_port->mpls_port_id)) {
            return BCM_E_PARAM;
        }
    } else {
        switch (mpls_port->criteria) {
        case BCM_MPLS_PORT_MATCH_PORT:
            /* XXX: TBD: Trunk ?? */
            break;
            
        case BCM_MPLS_PORT_MATCH_PORT_VLAN:
            /* XXX: TBD: Trunk ?? */
            if (mpls_port->match_vlan > BCM_VLAN_MAX) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Invalid vlan passed for MATCH_PORT_VLAN "
                                       "vlan=0x%x\n"), mpls_port->match_vlan));
                
                return BCM_E_PARAM;
            }
            break;

        case BCM_MPLS_PORT_MATCH_LABEL:
            if (!_BCM_CALADAN3_MPLSTP_LABEL_VALID(mpls_port->match_label)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Invalid label passed for "
                                       "MATCH_LABEL label=0x%x\n"),
                           mpls_port->match_label));

                return BCM_E_PARAM;
            }
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Invalid Match criteria=%d\n"),
                       mpls_port->criteria));
            
            return BCM_E_PARAM;
            break;
        }
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (BCM_SUCCESS(status)) {
        /* Port functions are only for VPLS */
        if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
            status = BCM_E_PARAM;
        } else {
            if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
                status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe,
                                                        vpnc,
                                                        mpls_port->mpls_port_id,
                                                        &vpn_sap);
            } else {
                is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
                status = _bcm_caladan3_find_vpn_sap_by_label(l3_fe,
                                                           vpnc,
                                                           mpls_port,
                                                           &vpn_sap,
                                                           is_failover);
            }
            if (BCM_SUCCESS(status)) {
                status = _bcm_caladan3_fill_mpls_port_from_vpn_sap(l3_fe,
                                                                 vpn_sap,
                                                                 mpls_port);
            }
        }
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_mpls_port_get_all(int               unit,
                             bcm_vpn_t         vpn,
                             int               port_max,
                             bcm_mpls_port_t  *port_array,
                             int              *port_count)
{
    _caladan3_l3_fe_instance_t       *l3_fe;
    int                           status;
    int                           ii;
    _caladan3_vpn_control_t          *vpnc;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if ((port_max <= 0) || (port_array == NULL)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    ii = 0;
    _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {

        status =
            _bcm_caladan3_fill_mpls_port_from_vpn_sap(l3_fe,
                                                    vpn_sap,
                                                    &port_array[ii++]);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }

        if (ii == port_max) {
            *port_count = ii;
            L3_UNLOCK(unit);
            return BCM_E_NONE;
        }

    } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

    *port_count = ii;

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear
 * Purpose:
 *      destroy the mpls tunnel on the interface
 * Parameters:
 *      unit        - Device Number
 *      intf        - The egress L3 interface
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_mpls_tunnel_initiator_clear(int           unit,
                                       bcm_if_t      intf)
{
    int                         status;
    uint32                      ifid;
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_L3_USER_IFID_VALID(intf)) {
        return (BCM_E_PARAM);
    }

    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe, ifid, &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid interface Id 0x%x\n"), 
                   _CALADAN3_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status  = _bcm_caladan3_mpls_l3_initiator_clear(l3_fe, l3_intf);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_get
 * Purpose:
 *      given a l3a_intf_id, get the mpls tunnel params
 * Parameters:
 *      unit        - (IN)  Device Number
 *      intf        - (IN)  The egress L3 interface Id
 *      label_max   - (IN)  max labels that can be filled up
 *      label_array - (OUT) label array to be returned
 *      label_count - (OUT) number of labels returned
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_mpls_tunnel_initiator_get(int                      unit,
                                     bcm_if_t                 intf,
                                     int                      label_max,
                                     bcm_mpls_egress_label_t *label_array,
                                     int                     *label_count)
{
    int                         status;
    uint32                      ifid;
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf;
    _caladan3_l3_ete_t             *mpls_sw_ete;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if ((intf        == 0)     ||
        (label_max   <= 0)     ||
        (label_array == NULL)  ||
        (label_count == NULL)) {
        return BCM_E_PARAM;
    }

    if (!_CALADAN3_L3_USER_IFID_VALID(intf)) {
        return (BCM_E_PARAM);
    }

    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                              ifid,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid interface Id 0x%x\n"),
                   _CALADAN3_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    mpls_sw_ete = NULL;
    status      = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                                      l3_intf,
                                                      _CALADAN3_L3_ETE__ENCAP_MPLS,
                                                      &mpls_sw_ete);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls tunnel does not exist on "
                               "interface id (0x%x)\n"),
                   _CALADAN3_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_fill_mpls_label_array_from_ete_idx(l3_fe,
                                                            mpls_sw_ete->l3_ete_hw_idx.ete_idx,
                                                            label_max,
                                                            label_array,
                                                            label_count);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear_all
 * Purpose:
 *      destroy all the mpls tunnels on the unit
 * Parameters:
 *      unit        - Device Number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_caladan3_mpls_tunnel_initiator_clear_all(int unit)
{
    int                         last_error_status, status;
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    last_error_status = BCM_E_NONE;
    _CALADAN3_ALL_L3INTF(l3_fe, l3_intf) {
        status = _bcm_caladan3_mpls_l3_initiator_clear(l3_fe,
                                                     l3_intf);
        if ((status != BCM_E_NOT_FOUND) || (status != BCM_E_NONE)) {
            last_error_status = status;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not clear mpls tunnel on "
                                   "interface(0x%x)\n"),
                       bcm_errmsg(status),
                       _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
        }
    } _CALADAN3_ALL_L3INTF_END(l3_fe, l3_intf);

    L3_UNLOCK(unit);
    return last_error_status;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit        - Device Number
 *      intf        - The egress L3 interface
 *      num_labels  - Number of labels in the array
 *      label_array - Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_caladan3_mpls_tunnel_initiator_set(int                      unit,
                                     bcm_if_t                 intf,
                                     int                      num_labels,
                                     bcm_mpls_egress_label_t *label_array)
{

    int                         status;
    uint32                      ifid;
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf;
    _caladan3_l3_ete_t             *l3_ete;
    uint32                      i = 0, hash_index;
    dq_p_t                      hash_bucket;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_L3_USER_IFID_VALID(intf)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "invalid intf(0x%x)\n"),
                   intf));
        return (BCM_E_PARAM);
    }

    /**
     * Since there is no module in the parameters, therefore function
     * must be called locally
     */
    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                              ifid,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) finding interface 0x%x\n"),
                   bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status = _bcm_caladan3_check_mpls_initiator_params(l3_fe,
                                                     l3_intf,
                                                     num_labels,
                                                     label_array);
    if (status != BCM_E_NONE) {
        /* error printed already */
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * We have already made sure that there is at most
     * one l3-ete (i.e. no l3-ete or one l3-ete).
     */
    status = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                                 l3_intf,
                                                 _CALADAN3_L3_ETE__UCAST_IP,
                                                 &l3_ete);
    if ((status != BCM_E_NONE) && (status != BCM_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) finding ipv4 ete on intf(0x%x)\n"),
                   bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * If a v4-ete has not been created yet, we save the label info.
     * dmac and vidop needs to come from the v4-ete
     */
    status = _bcm_caladan3_mpls_tunnel_initiator_set(l3_fe,
                                                   l3_intf,
                                                   l3_ete, /* Could be NULL */
                                                   num_labels,
                                                   label_array);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "error(%s) enabling mpls tunnel on intf(0x%x)\n"),
                   bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return status;
    }

    l3_intf->if_flags |=_CALADAN3_L3_MPLS_TUNNEL_SET;

    l3_intf->if_flags |= _CALADAN3_L3_INTF_IN_EGR_LBL_LIST;
/*    for (i = 0; i < num_labels; i++) { */
        i = 0;
        l3_intf->if_flags |= _CALADAN3_L3_INTF_IN_EGR_LBL_LIST;
        hash_index = _CALADAN3_GET_INTF_EGR_LBL_HASH(label_array[i].label);
        hash_bucket = &l3_fe->fe_intf_by_egr_lbl[hash_index];
        DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_egr_lbl_link);
        l3_intf->tunnel_egr_label = label_array[i].label;
/*    } */
    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_caladan3_mpls_tunnel_switch_add
 * Purpose:
 *      destroy all the mpls tunnels on the unit
 * Parameters:
 *      unit        - Device Number
 *      info        - label switching info (only LSR/Egress LER for L3 VPN)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *     Check info->label for routing label range in PPE config;
 *     Index into Label2Etc table using label :
 *     LSR Case:
 *       Set labelMode to lsr
 *       Set labelQos to ??
 *       Derive real FTidx from info->egress_if and
 *       populate in ftidx field
 *
 *       The MPLS egress object handle passed to this API must
 *       be created as mentioned above; effectively egress_if gives
 *       the FTE handle that points to an MPLS ETE that contains the swap
 *       label (swap_label). For PHP case, the MPLS ETE must be populated to
 *       set Php field to 1
 *
 *     LER case:
 *       Set labelMode to ler
 *       Set labelUnion (vlan) tbo a default Bridging context (VSI) -
 *       aka SBX 16b VLAN space - which has the default/global VRF
 */

int
bcm_caladan3_mpls_tunnel_switch_add(int                       unit,
                                  bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _caladan3_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *swinfo;
    bcm_mpls_tunnel_switch_t   *datum;
#ifdef USE_TUNNEL_ID
    int                         rv;
    uint                        res_flags = 0;
    uint32                      tunnel_id = 0;
#endif
    uint8                       key[MPLS_SWITCH_INFO_KEY_SIZE];
    

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (info == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    swinfo = sal_alloc(sizeof(*swinfo),
                      "MPLS-tunnel-switch-info");
    if (swinfo == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_MEMORY;
    }


    /* User does not create BCM_L3_VRF_DEFAULT */
    if (info->vpn == BCM_L3_VRF_DEFAULT) {
        info->vpn = l3_fe->fe_vsi_default_vrf;
    }

    status = _bcm_caladan3_mpls_validate_tunnel_switch_add(l3_fe, info);
    if (status != BCM_E_NONE) {
        if (info->vpn == l3_fe->fe_vsi_default_vrf) {
            info->vpn = BCM_L3_VRF_DEFAULT;
        }
        sal_free(swinfo);
        L3_UNLOCK(unit);
        return  status;
    }

#ifdef USE_TUNNEL_ID
    if (info->flags & BCM_MPLS_SWITCH_WITH_ID) {
        _bcm_caladan3_mpls_switch_key_get(&key,
                                          info->label,
                                          info->second_label,
                                          info->port,
                                          info->tunnel_id);
        status = shr_htb_find(mpls_switch_info_db[unit], 
                              (shr_htb_key_t) &key,
                              (shr_htb_data_t*)&datum, 0);
        if (status == BCM_E_NOT_FOUND) {
            if (info->flags & BCM_MPLS_SWITCH_REPLACE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Tunnel switch not found for tunnel id (0x%x)\n"),
                           info->tunnel_id));
                sal_free(swinfo);
                L3_UNLOCK(unit);
                return  status;
            }
        } else if (status == BCM_E_NONE) {
            if (!(info->flags & BCM_MPLS_SWITCH_REPLACE)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Tunnel Id Exists\n")));
                sal_free(swinfo);
                L3_UNLOCK(unit);
                return  status;
            }
        }
    }
    if (info->flags & BCM_MPLS_SWITCH_REPLACE) {
        tunnel_id = BCM_GPORT_TUNNEL_ID_GET(info->tunnel_id);
        if ((info->label != datum->label) || 
            (info->second_label != datum->second_label) ||
            (info->port != datum->port)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Changing port or labels are not supported\n")));
            sal_free(swinfo);
            L3_UNLOCK(unit);
            return  status;
        }
    } else {         
        if (info->flags & BCM_MPLS_SWITCH_WITH_ID) {
            res_flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
            tunnel_id = BCM_GPORT_TUNNEL_ID_GET(info->tunnel_id);
        }
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                              SBX_CALADAN3_USR_RES_TUNNEL_ID,
                                              1,
                                              &tunnel_id,
                                              res_flags);
    
        if(status == BCM_E_NONE) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Tunnel Id Allocated 0x%x\n"),
                       tunnel_id));
        } else if(status == BCM_E_RESOURCE) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Tunnel Id (0x%x) marked as externally managed\n"),
                       tunnel_id));
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) Couldn't allocate tunnel_id\n"),
                       bcm_errmsg(status)));
            sal_free(swinfo);
            L3_UNLOCK(unit);
            return  status;
        }
    }

#endif

    status = _bcm_caladan3_mpls_tunnel_switch_update(l3_fe, info);
    if (info->vpn == l3_fe->fe_vsi_default_vrf) {
        info->vpn = BCM_L3_VRF_DEFAULT;
    }

    if (BCM_SUCCESS(status)) {
#ifdef USE_TUNNEL_ID         
        BCM_GPORT_TUNNEL_ID_SET(info->tunnel_id, tunnel_id);
#endif
        _bcm_caladan3_mpls_switch_key_get(&key,
                                          info->label,
                                          info->second_label,
                                          info->port,
                                          info->tunnel_id);
        /* if successful, insert into the db */
        status = shr_htb_find(mpls_switch_info_db[unit], 
                              (shr_htb_key_t) &key,
                              (shr_htb_data_t*)&datum, 0);
        if (status  == BCM_E_NOT_FOUND) {
            *swinfo = *info;
            status = shr_htb_insert(mpls_switch_info_db[unit],
                                     (shr_htb_key_t) &key,
                                     (void*) swinfo);        
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "error(%s) Updating switch_info Dbase (0x%x,0x%x,0x%x)\n"),
                           bcm_errmsg(status), info->label, 
                           info->second_label, info->port));
                sal_free(swinfo);
            }
        } else {
            /* overwrite existing */
            *datum = *info;
#ifdef USE_TUNNEL_ID
            /* Remove Replace label when storing */
            datum->flags &= ~(BCM_MPLS_SWITCH_REPLACE);           
#endif
            sal_free(swinfo);
        }
    } else {
#ifdef USE_TUNNEL_ID
        /* _bcm_caladan3_mpls_tunnel_switch_update failed.
         * Free tunnel_id if it is add case*/
        if (!(info->flags & BCM_MPLS_SWITCH_REPLACE)) {
            rv = _sbx_caladan3_resource_free(unit, 
                                                SBX_CALADAN3_USR_RES_TUNNEL_ID,
                                                1, &tunnel_id, 0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                       "failed to free tunnel_id (0x%x) Error: %d (%s) \n"),
                           tunnel_id, rv, bcm_errmsg(rv)));
            } else {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "freed tunnel_id (0x%x)\n"),
                           tunnel_id));
            }
        }
#endif
        sal_free(swinfo);
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_tunnel_switch_delete
 * Purpose:
 *      delete earlier setup label switch path
 * Parameters:
 *      unit        - Device Number
 *      info        - label switching info (only LSR/Egress LER for L3 VPN)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_caladan3_mpls_tunnel_switch_delete(int                       unit,
                                     bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _caladan3_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *datum;
    uint8                       key[MPLS_SWITCH_INFO_KEY_SIZE];
#ifdef USE_TUNNEL_ID
    uint32                      tunnel_id;
#endif

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (info == NULL) {
        return BCM_E_PARAM;
    }

#ifdef USE_TUNNEL_ID
    if (!(BCM_GPORT_IS_TUNNEL(info->tunnel_id))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid tunnel id\n")));
        return BCM_E_PARAM;
    }
    tunnel_id = BCM_GPORT_TUNNEL_ID_GET(info->tunnel_id);
    if (!tunnel_id) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "tunnel id is 0\n")));
        return BCM_E_PARAM;
    }

#else
    if (info->label == 0) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Label is 0\n")));
        return BCM_E_PARAM;
    }
#endif


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    /* find and destroy the tunnel_switch info */
    _bcm_caladan3_mpls_switch_key_get(&key,
                                      info->label,
                                      info->second_label,
                                      info->port,
                                      info->tunnel_id);
        
    status = shr_htb_find(mpls_switch_info_db[unit], 
                          (shr_htb_key_t) &key,
                          (shr_htb_data_t*)&datum, 1);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "failed to find MPLS switch_info dbase element: %d (%s) \n"),
                   status, bcm_errmsg(status)));
#ifdef USE_TUNNEL_ID
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Tunnel_Id (0x%x) \n"),
                   info->tunnel_id));
#endif
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    status = _bcm_caladan3_mpls_tunnel_switch_delete(l3_fe,
                                                   datum);

    if (BCM_SUCCESS(status)) {
        sal_free(datum);
    }

#ifdef USE_TUNNEL_ID
    if (BCM_SUCCESS(status)) {
        if (tunnel_id) {
            status = _sbx_caladan3_resource_free(unit, 
                                                SBX_CALADAN3_USR_RES_TUNNEL_ID,
                                                1, &tunnel_id, 0);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "failed to free tunnel_id (0x%x) Error: %d (%s) \n"),
                           tunnel_id, status, bcm_errmsg(status)));
            } else {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "freed tunnel_id (0x%x)\n"),
                           tunnel_id));
            }
        }
    }
#endif

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_tunnel_switch_delete_all
 * Purpose:
 *      delete all earlier setup label switch paths
 * Parameters:
 *      unit        - Device Number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_caladan3_mpls_tunnel_switch_delete_all(int unit)
{
    int                         status = BCM_E_UNAVAIL;
    _caladan3_l3_fe_instance_t     *l3_fe;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_caladan3_mpls_tunnel_switch_delete_all(l3_fe);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_mpls_tunnel_switch_get
 * Purpose:
 *      get label switch path information
 * Parameters:
 *      unit        - Device Number
 *      info        - tunnel switch info
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_caladan3_mpls_tunnel_switch_get(int               unit,
                                  bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _caladan3_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *datum;
    uint8                       key[MPLS_SWITCH_INFO_KEY_SIZE];

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (info == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Info is NULL\n")));
        return BCM_E_PARAM;
    }

#ifdef USE_TUNNEL_ID
    if (!(BCM_GPORT_IS_TUNNEL(info->tunnel_id))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Tunnel id (%x) is not valid GPORT\n"),
                   info->tunnel_id));
        return BCM_E_PARAM;
    }
#else
    if (info->label == 0) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Label value is 0\n")));
        return BCM_E_PARAM;
    }

#endif

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    /* find the tunnel switch info */
    _bcm_caladan3_mpls_switch_key_get(&key,
                                      info->label,
                                      info->second_label,
                                      info->port,
                                      info->tunnel_id);
    status = shr_htb_find(mpls_switch_info_db[unit], 
                          (shr_htb_key_t) &key,
                          (shr_htb_data_t *)&datum, 0);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Tunnel switch data not found for label: %d (%s)\n"),
                   status, bcm_errmsg(status)));
    } else {
        *info = *datum;
    }
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_traverse
 * Purpose:
 *      traverse label switch(LSR/LER) information
 * Parameters:
 *      unit        - Device Number
 *      cb          - function to be called
 *      user_data   - user provided info sent to cb
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_mpls_tunnel_switch_traverse(int               unit,
                                       bcm_mpls_tunnel_switch_traverse_cb  cb,
                                       void             *user_data)
{
    int                         rv;
    _caladan3_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t    info;
#ifndef USE_TUNNEL_ID
    bcm_mpls_label_t            label;
#else
    bcm_gport_t                 tunnel_id, max_tunnel_id;
#endif
    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (cb == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

#ifdef USE_TUNNEL_ID
    BCM_GPORT_TUNNEL_ID_SET(tunnel_id, SBX_CALADAN3_TUNNEL_ID_BASE);
    BCM_GPORT_TUNNEL_ID_SET(max_tunnel_id, SBX_CALADAN3_TUNNEL_ID_MAX);
    
    for (; tunnel_id < max_tunnel_id; tunnel_id++) {
        bcm_mpls_tunnel_switch_t_init(&info);
        info.tunnel_id = tunnel_id;
        rv = bcm_caladan3_mpls_tunnel_switch_get(unit, &info);
        if (BCM_SUCCESS(rv)) {
            cb(unit, &info, user_data);
        }
    }
#else
    for (label = 0; label < _CALADAN3_G3P1_MAX_LABELS; label++) {
        bcm_mpls_tunnel_switch_t_init(&info);
        info.label = label;
        rv = bcm_caladan3_mpls_tunnel_switch_get(l3_fe->fe_unit, &info);
        if (BCM_SUCCESS(rv)) {
            cb(unit, &info, user_data);
        }
    }
#endif

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 *  Function
 *      bcm_mpls_exp_map_create
 *  Purpose
 *      Allocate a QOS_MAP or EGR_REMARK resource for configuration of MPLS
 *      PRI based translation on ingress or egress.
 *  Parameters
 *      int unit = the unit number
 *      uint32 flags = flags for the operation
 *      int *exp_map_id = pointer to where to put the ID or the desired ID
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      Resource pools conflict with port_vlan and IPv4 priority maps.  Unless
 *      the same requests are made of all units at the same time, both of this
 *      API and the port version and the IPv4 version, it is possible that
 *      BCM_MPLS_EXP_MAP_WITH_ID requests will fail because other functions
 *      have already used an item.
 *      Does not support allocating both BCM_MPLS_EXP_MAP_INGRESS and
 *      BCM_MPLS_EXP_MAP_EGRESS at the same time, since they come from
 *      different pools and may therefore not have the same element number
 *      available in both pools.
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (they are always the same here).
 */
int
bcm_caladan3_mpls_exp_map_create(int unit,
                               uint32 flags,
                               int *exp_map_id)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    uint32                  expProfile = 0;
    uint32                  expMap = 0;
    soc_sbx_g3p1_qos_t      p3qosProfile;
    soc_sbx_g3p1_remark_t   p3egrRemark;
    unsigned int            pri,cfi;
    int                     result = BCM_E_INTERNAL;
    int                     index;
    int                     freeAllocs = TRUE;

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "(0x%08X,*): enter\n"),
                 flags));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!exp_map_id) {
        /* NULL pointer is unacceptable */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "NULL pointer to exp_map_id\n")));
        return BCM_E_PARAM;
    }
    if (flags & (~(BCM_MPLS_EXP_MAP_WITH_ID |
                   BCM_MPLS_EXP_MAP_INGRESS |
                   BCM_MPLS_EXP_MAP_EGRESS ))) {
        /* An unsupported flag is set */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "an unexpected flag is set\n")));
        return BCM_E_PARAM;
    }
    if (((flags & (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)) ==
         (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)) ||
        (0 == (flags & (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)))) {
        /*
         *  Caller wanted either both of or neither of ingress and egress; we
         *  don't support either case here.
         */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "must specify exactly one of ingress,egress\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* allocate the map as requested */
    if (flags & BCM_MPLS_EXP_MAP_INGRESS) {
        result = BCM_E_NONE; /* be optimistic */

        /* now need a QosMap */
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "allocate any available QosMap\n")));
        result = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                         1,
                                         &expMap,
                                         0);
        if (BCM_E_NONE == result) {
            /* got the map */
            /* set the needed values to the profile */
            
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {

            case SOC_SBX_UCODE_TYPE_G3P1:
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "allocated QosMap[0x%08X]; prep it\n"),
                           expMap));
                /* initialise the profile */
                soc_sbx_g3p1_qos_t_init(&p3qosProfile);
                for (pri = 0;
                     (pri < 8) && (BCM_E_NONE == result);
                     pri++) {
                    for (cfi = 0;
                         (cfi < 2) && (BCM_E_NONE == result);
                         cfi++) {
                        result = soc_sbx_g3p1_qos_set(unit,
                                                      cfi,
                                                      pri,
                                                      expMap,
                                                      &p3qosProfile);
                    } /* for (all valid CFI values) */
                } /* for (all valid PRI values) */
                if (BCM_E_NONE == result) {
                    *exp_map_id = _MPLS_EXPMAP_HANDLE_MAKE_INGR(expMap);
                } else { /* if (BCM_E_NONE == result) */
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to clear :QosMap[0x%08X]: %d (%s)\n"),
                               expMap, result, _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
                break;

            default:
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unknown microcode %d\n"),
                           SOC_SBX_CONTROL(unit)->ucodetype));
                result = BCM_E_UNAVAIL;
            } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE != result) && freeAllocs) {
            /* something went wrong; free anything we managed to acquire */
            if (expProfile) {
                /* allocated a qosProfile; free it */
	        /* coverity[dead_error_line] */
                _sbx_caladan3_resource_free(unit,
                                       SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                       1,
                                       &expProfile,
                                       0);
            }
            if (expMap) {
                /* allocated a qosMap; free it */
                _sbx_caladan3_resource_free(unit,
                                       SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                       1,
                                       &expMap,
                                       0);
            }
        } /* if (BCM_E_NONE == result) */
    } else if (flags & BCM_MPLS_EXP_MAP_EGRESS) {
        /* Requested an egress map */
        if (flags & BCM_MPLS_EXP_MAP_WITH_ID) {
            /* Requested a specific egress map */
            if (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) {
                /* requested handle is valid egress ID */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(*exp_map_id);
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "allocate specific egrRemark[0x%08X]\n"),
                           *exp_map_id));
                result = _sbx_caladan3_resource_alloc(unit,
                                                 SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                                 1,
                                                 &expMap,
                                                 _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (BCM_E_NONE != result) {
                    /* something went wrong reserving the element */
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to reserve egrRemark[0x%08X]: %d (%s)\n"),
                               *exp_map_id,
                               result,
                               _SHR_ERRMSG(result)));
                }
            } else { /* (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) */
                /* requested handle is not valid egress ID */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "requested ID Ox%08X is not valid for egress\n"),
                           *exp_map_id));
                result = BCM_E_PARAM;
            } /* (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) */
            /* whatever was passed in should not be freed */
            freeAllocs = FALSE;
        } else { /* if (flags & BCM_MPLS_EXP_MAP_WITH_ID) */
            /* Happy with any free egress map */
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "allocate any available egrRemark\n")));
            result = _sbx_caladan3_resource_alloc(unit,
                                             SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                             1,
                                             &expMap,
                                             0);
            if (BCM_E_NONE != result) {
                /* something went wrong trying to allocate the map */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to allocate an egrRemark\n")));
            }
        } /* if (flags & BCM_MPLS_EXP_MAP_WITH_ID) */

        if (BCM_E_NONE == result) {
            /* got the map */
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "allocated egrRemark[0x%08X]; prepare it\n"),
                       expMap));
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {

            case SOC_SBX_UCODE_TYPE_G3P1:
                soc_sbx_g3p1_remark_t_init(&p3egrRemark);
                for (pri = 0;
                     (pri < 8) && (BCM_E_NONE == result);
                     pri++) {
                    for (index = 0;
                         (index < 4) && (BCM_E_NONE == result);
                         index++) {
                        for (cfi = 0;
                             (cfi < 2) && (BCM_E_NONE == result);
                             cfi++) {
                            result = soc_sbx_g3p1_remark_set(unit,
                                                             cfi,
                                                             index,
                                                             pri,
                                                             expMap,
                                                             &p3egrRemark);
                        } /* for (all ECN value) */
                    } /* for (all DP values) */
                } /* for (all CoS values) */
                if (BCM_E_NONE == result) {
                    *exp_map_id = _MPLS_EXPMAP_HANDLE_MAKE_EGR(expMap);
                } else { /* if (BCM_E_NONE == result) */
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to clear :egrRemark[0x%08X]: %d (%s)\n"),
                               expMap, result, _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
                break;

            default:
                SBX_UNKNOWN_UCODE_WARN(unit);
                result = BCM_E_UNAVAIL;
            } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE != result) && freeAllocs) {
            /* something went wrong; free any acquired resources */
            if (expMap) {
                /* got an egressRemarkEntry; free it */
                _sbx_caladan3_resource_free(unit,
                                       SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                       1,
                                       &expMap,
                                       0);
            }
        } /* if (BCM_E_NONE == result) */
    } /* flags contains ingress or egress */

    
    L3_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "(%08X,&(%08X)): return %d (%s)\n"),
               flags,
               *exp_map_id,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_caladan3_mpls_exp_map_destroy
 *  Purpose
 *      Free a QOS_MAP or EGR_REMARK resource for configuration of MPLS
 *      PRI based translation on ingress or egress.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be freed
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 */
int
bcm_caladan3_mpls_exp_map_destroy(int unit,
                                int exp_map_id)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    uint32                  expMap = 0;
    uint32                  expProfile = 0;
    int                     result = BCM_E_INTERNAL;
    int                     tmpRes;

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "(0x%08X): enter\n"),
                 exp_map_id));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "0x%08X is not a valid MPLS EXP map ID\n"),
                   exp_map_id));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* free the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* freeing an ingress map */
        expProfile = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G3P1:
            /* don't need to do anything to find map -- single layer */
            expMap = expProfile;
            expProfile = 0;
            result = BCM_E_NONE;
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        if ((BCM_E_NONE == result) && expMap) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "free qosMap[0x%08X]\n"),
                       expMap));
            tmpRes = _sbx_caladan3_resource_free(unit,
                                            SBX_CALADAN3_USR_RES_QOS_PROFILE,
                                            1,
                                            &expMap,
                                            0);
            if (BCM_E_NONE != tmpRes) {
                /* something went wrong */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to free qosMap[0x%08X]: %d (%s)\n"),
                           expMap, result, _SHR_ERRMSG(result)));
                if (BCM_E_NONE == result) {
                    /* don't have an error yet, so set it */
                    result = tmpRes;
                }
            } /* if (BCM_E_NONE != tmpRes) */
        } /* if ((BCM_E_NONE == result) && expMap) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* freeing an egress map */
        expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "free egrRemap[0x%08X]\n"),
                   expMap));
        /*
         *  This one's easy, since there is only a single resource to manage,
         *  and there's no chaining, so either getting rid of it works or not.
         */
        result = _sbx_caladan3_resource_free(unit,
                                        SBX_CALADAN3_USR_RES_QOS_EGR_REMARK,
                                        1,
                                        &expMap,
                                        0);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unable to free egrRemark[0x%08X]: %d (%s)\n"),
                       expMap, result, _SHR_ERRMSG(result)));
        }
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "(0x%08X): return %d (%s)\n"),
               exp_map_id,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_mpls_exp_map_set
 *  Purpose
 *      Adjust a qosMap or egrRemark entry to reflect the specified settings.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be updated
 *      bcm_mpls_exp_map_t *exp_map = pointer to the new setting
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 *
 *      On G2P2, if setting for ingress, and qosProfile does not indicate to
 *      use the qosMap for MPLS, this will enable using the qosMap for MPLS and
 *      rewrite the qosProfile.  If the qosProfile already indicates to use the
 *      qosMap, this will not write back to the qosProfile.
 *
 *      When setting ingress: exp_map->exp is the key; exp_map->priority and
 *      exp_map->color are the data that are set (exp_map->pkt_pri and
 *      exp_map->pkt_cfi are ignored).
 *
 *      When setting egress: exp_map->priority and exp_map->color are the key;
 *      exp_map->pri, exp_map->pkt_pri, exp_map->pkt_cfi are the data to set.
 */
int
bcm_caladan3_mpls_exp_map_set(int unit,
                            int exp_map_id,
                            bcm_mpls_exp_map_t *exp_map)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    uint32                  expMap;
    uint32                  expProfile = 0;
    soc_sbx_g3p1_qos_t      p3qosProfile;
    soc_sbx_g3p1_remark_t   p3egrRemark;
    int                     result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "(0x%08X,0x%08X): enter\n"),
                 exp_map_id, (unsigned int)exp_map));


    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "0x%08X is not a valid MPLS EXP map ID\n"),
                   exp_map_id));
        return BCM_E_PARAM;
    }
    if (!exp_map) {
        /* invalid pointer to map data */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "NULL is not a valid exp_map pointer\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* adjust the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* setting ingress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value 0x%x for exp\n"),
                       exp_map->exp));
        }
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for priority\n"), 
                       exp_map->priority));
        }
        /* verify that colour is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  " invalid value %d for color\n"),
                       exp_map->color));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G3P1:
            if (BCM_E_NONE == result) {
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                soc_sbx_g3p1_qos_t_init(&p3qosProfile);
                
                p3qosProfile.mefcos = exp_map->priority;
                p3qosProfile.fcos = exp_map->priority;
                p3qosProfile.cos = exp_map->priority;
                p3qosProfile.dp = exp_map->color;
                
                p3qosProfile.e = FALSE;
                result = soc_sbx_g3p1_qos_set(unit,
                                              0,
                                              exp_map->exp,
                                              expMap,
                                              &p3qosProfile);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to set qosMap[0x%08X].exp[0x%X]:"
                                           " %d (%s)\n"),
                               expMap, exp_map->exp,
                               result, _SHR_ERRMSG(result)));
                }
            }
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* setting egress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value 0x%X for exp\n"),
                       exp_map->exp));
        }
        /* make sure pri is valid */
        if (7 < exp_map->pkt_pri) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for pkt_pri\n"),
                       exp_map->pkt_pri));
        }
        /* make sure cfi is valid */
        if (1 < exp_map->pkt_cfi) {
            /* invalid CFI value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for pkt_cfi\n"),
                       exp_map->pkt_cfi));
        }
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for priority\n"),
                       exp_map->priority));
        }
        /* make sure color is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for color\n"),
                       exp_map->color));
        }
        /* extract the egress map ID from the handle */
        expProfile = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        expMap = (exp_map->priority << 2) | (exp_map->color);
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G3P1:
            if (BCM_E_NONE == result) {
                /* fill in the data */
                soc_sbx_g3p1_remark_t_init(&p3egrRemark);
                p3egrRemark.exp = exp_map->exp;
                p3egrRemark.cfi = exp_map->pkt_cfi;
                p3egrRemark.pri = exp_map->pkt_pri;
                for (expMap = 0;
                     (expMap < 2) && (BCM_E_NONE == result);
                     expMap++) {
                    result = soc_sbx_g3p1_remark_set(unit,
                                                     expMap,
                                                     exp_map->color,
                                                     exp_map->priority,
                                                     expProfile,
                                                     &p3egrRemark);
                } /* for (both ECN values) */
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to write"
                                           " egrRemark[0x%08X].[%d].[%d].[%d]:"
                                           " %d (%s)\n"),
                               expMap,
                               exp_map->priority,
                               exp_map->color,
                               expProfile,
                               result,
                               _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE != result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "(0x%08X,0x%08X->(%d,%d,%d,%d,%d)): return %d (%s)\n"),
               exp_map_id,
               (unsigned int)exp_map,
               exp_map->priority,
               exp_map->color,
               exp_map->exp,
               exp_map->pkt_pri,
               exp_map->pkt_cfi,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_caladan3_mpls_exp_map_get
 *  Purpose
 *      Fetch a qosMap or egrRemark entry to read the specified settings.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be updated
 *      bcm_mpls_exp_map_t *exp_map = pointer to the setting buffer
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 *
 *      On G2P2, if getting for ingress, and qosProfile does not indicate to
 *      use the qosMap for MPLS, this will return BCM_E_EMPTY.
 *
 *      When getting ingress: exp_map->exp is the key; exp_map->priority and
 *      exp_map->color are the data that are read (exp_map->pkt_pri and
 *      exp_map->pkt_cfi are ignored).
 *
 *      When getting egress: exp_map->priority and exp_map->color are the key;
 *      exp_map->pri, exp_map->pkt_pri, exp_map->pkt_cfi are the data read.
 */
int
bcm_caladan3_mpls_exp_map_get(int unit,
                            int exp_map_id,
                            bcm_mpls_exp_map_t *exp_map)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    uint32                  expMap;
    soc_sbx_g3p1_qos_t      p3qosProfile;
    soc_sbx_g3p1_remark_t   p3egrRemark;
    int                     result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "(0x%08X,0x%08X): enter\n"),
                 exp_map_id, (unsigned int)exp_map));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "0x%08X is not a valid MPLS EXP map ID\n"),
                   exp_map_id));
        return BCM_E_PARAM;
    }
    if (!exp_map) {
        /* invalid pointer to map data */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "NULL is not a valid exp_map pointer\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* fetch the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* getting ingress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for exp\n"),
                       exp_map->exp));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G3P1:
            if (BCM_E_NONE == result) {
                /* extract the ingress map ID from the handle */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "get current qosMap[0x%08X].exp[%d]\n"),
                           expMap,
                           exp_map->exp));
                result = soc_sbx_g3p1_qos_get(unit,
                                              0,
                                              exp_map->exp,
                                              expMap,
                                              &p3qosProfile);
                if (BCM_E_NONE == result) {
                    exp_map->priority = p3qosProfile.cos;
                    exp_map->color = p3qosProfile.dp;
                } else { /* if (BCM_E_NONE == result) */
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to read qosMap[0x%08X].exp[%d]:"
                                           " %d (%s)\n"),
                               expMap,
                               exp_map->exp,
                               result,
                               _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* getting egress mapping */
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for priority\n"),
                       exp_map->priority));
        }
        /* make sure color is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "invalid value %d for color\n"),
                       exp_map->color));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G3P1:
            if (BCM_E_NONE == result) {
                /* extract the egress map ID from the handle */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                result = soc_sbx_g3p1_remark_get(unit,
                                                 0,
                                                 exp_map->color,
                                                 exp_map->priority,
                                                 expMap,
                                                 &p3egrRemark);
                if (BCM_E_NONE == result) {
                    exp_map->pkt_cfi = p3egrRemark.cfi;
                    exp_map->pkt_pri = p3egrRemark.pri;
                    exp_map->exp = p3egrRemark.exp;
                } else { /* if (BCM_E_NONE == result) */
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unable to read"
                                           " egrRemark[0x%08X].[%d].[%d].[0]: "
                                           "%d (%s)\n"),
                               expMap,
                               exp_map->priority,
                               exp_map->color,
                               result,
                               _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "(0x%08X,0x%08X->(%d,%d,%d,%d,%d)): return %d (%s)\n"),
               exp_map_id,
               (unsigned int)exp_map,
               exp_map->priority,
               exp_map->color,
               exp_map->exp,
               exp_map->pkt_pri,
               exp_map->pkt_cfi,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

int
_bcm_caladan3_mpls_validate_vpn_id(bcm_vpn_t vpn)
{
    return BCM_E_NONE;
}

/*
 *  Function:
 *    _bcm_caladan3_mpls_policer_stat_rw
 *  Description:
 *    Translate MPLS port stat to Pol stat
 *    Get/Clear the Pol Stat counters
 *  Parameters:
 *    in  clear          - Get (0) / clear (1) Stat
 *    in  policerId      - Policer Id
 *    in  statCos        - 
 *    in  mplsStat       - MPLS Port statistic ID
 *    in  base_counter   - base counter
 *    out *val           - Pointer to store counter value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

static int
_bcm_caladan3_mpls_policer_stat_rw(int unit, 
                                 int clear,
                                 uint32 policerId,
                                 bcm_cos_t statCos,
                                 bcm_mpls_port_stat_t mplsStat,
                                 uint32 base_counter,
                                 uint64 *val)
{
#define BCM_CALADAN3_G3P1_MAX_STAT_CONV 24
    int                      status = BCM_E_NONE;
    bcm_policer_stat_t       polStats[BCM_CALADAN3_G3P1_MAX_STAT_CONV];
    int                      numStats = 0, statIdx;
    bcm_policer_group_mode_t groupMode;
    uint64                   uuVal;
    /*int                      allCos = FALSE;*/

    status = _bcm_caladan3_policer_group_mode_get(unit, policerId, &groupMode);
    if (BCM_SUCCESS(status)) {

        switch (groupMode) {
        case bcmPolicerGroupModeSingle:

            switch (mplsStat) {
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
            default:
                break;
            }
            break;
        case bcmPolicerGroupModeTyped:

            switch (mplsStat) {
            case bcmMplsPortStatUnicastPackets:
                polStats[numStats++] = bcmPolicerStatUnicastPackets;
                break;
            case bcmMplsPortStatUnicastBytes:
                polStats[numStats++] = bcmPolicerStatUnicastBytes;
                break;
            case bcmMplsPortStatNonUnicastPackets:
                polStats[numStats++] = bcmPolicerStatMulticastPackets;
                break;
            case bcmMplsPortStatNonUnicastBytes:
                polStats[numStats++] = bcmPolicerStatMulticastBytes;
                break;
            case bcmMplsPortStatFloodDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                break;
            case bcmMplsPortStatFloodPackets:
                polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodBytes:
                polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
                break;
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
            default:
                break;
            }
            break;
        case bcmPolicerGroupModeTypedIntPri: /* fall thru intentional */
        case bcmPolicerGroupModeTypedAll:
            switch (mplsStat) {
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
            case bcmMplsPortStatFloodDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                break;
            case bcmMplsPortStatGreenPackets:
                polStats[numStats++] = bcmPolicerStatGreenPackets;
                break;
            case bcmMplsPortStatGreenBytes:
                polStats[numStats++] = bcmPolicerStatGreenBytes;
                break;
            case bcmMplsPortStatYellowPackets:
                polStats[numStats++] = bcmPolicerStatYellowPackets;
                break;
            case bcmMplsPortStatYellowBytes:
                polStats[numStats++] = bcmPolicerStatYellowBytes;
                break;
            case bcmMplsPortStatRedPackets:
                polStats[numStats++] = bcmPolicerStatRedPackets;
                break;
            case bcmMplsPortStatRedBytes:
                polStats[numStats++] = bcmPolicerStatRedBytes;
                break;
            default:
                break;
            }
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Unsupported groupMode: %d %s\n"),
                       groupMode,
                       _bcm_caladan3_policer_group_mode_to_str(groupMode)));
            status = BCM_E_CONFIG;
            break;
        }

        assert (numStats < BCM_CALADAN3_G3P1_MAX_STAT_CONV);

        if (numStats <= 0 ) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "MplsPortStatType %d not supported by "
                                   "policer group mode %d\n"),
                       mplsStat, groupMode));
            return BCM_E_PARAM;
        }

        COMPILER_64_ZERO(uuVal);
        for (statIdx = 0; statIdx < numStats; statIdx++) {
            uint64 uuTmp = COMPILER_64_INIT(0,0);
            int cos, cosStart, cosEnd;

            /*            if (allCos) {
                cosStart = 0;
                cosEnd = NUM_COS(unit);
            } else {*/
                cosStart = statCos;
                cosEnd = statCos + 1;
            /*}*/

            for (cos = cosStart; cos < cosEnd; cos++) {
                if (clear) {
                    status = _bcm_caladan3_g3p1_policer_stat_set(unit, policerId,
                                           polStats[statIdx], cos,
                                           0, base_counter, uuTmp);
                } else {
                    status = _bcm_caladan3_g3p1_policer_stat_get(unit, policerId,
                                           polStats[statIdx], cos,
                                           0, base_counter,
                                           0, &uuTmp);
                }
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Failed to get mplsPortStat=%d; "
                                           "policerStat=%d: %d %s\n"), mplsStat,
                               polStats[statIdx], status, bcm_errmsg(status)));
                    return status;
                }
                COMPILER_64_ADD_64(uuVal, uuTmp);
            }
        }

        if (!clear) {
            *val = uuVal;
        }
    }    

    return status;
}

static int
_bcm_caladan3_mpls_label_stat_get(int unit,
                                bcm_mpls_label_t label,
                                bcm_gport_t port,
                                bcm_mpls_stat_t stat,
                                int clear,
                                uint64 *val)
{
    int                          status = BCM_E_NONE;
    bcm_policer_group_mode_t     grp_mode;
    int                          num_ctrs = 0, pkts = 0;
    uint32                       counter, cur_ctr, i, ohi;
    soc_sbx_g3p1_turbo64_count_t soc_val, soc_val_tot;
    soc_sbx_g3p1_oi2e_t          oi2e;
    soc_sbx_g3p1_lp_t            lp;
    uint32                       lpi = 0;
    bcm_policer_t                policer_id;
    _caladan3_l3_fe_instance_t      *l3_fe = NULL;
    _caladan3_vpn_sap_t             *vpn_sap = NULL;
    soc_sbx_g3p1_labels_t       label2etc;
    _caladan3_l3_intf_t             *l3_intf = NULL;
    _caladan3_l3_ete_t              *l3_ete = NULL;
    int                          label1 = 0, label2 = 0, label3 = 0;
    int                          local_port = -1;
#ifndef PLATFORM_LABELS
    bcm_module_t                  modid = -1;
    uint32                        trunkid;
    bcm_trunk_add_info_t         *trunk_info = NULL;
    uint8                         index, is_trunk = 0, num_ports = 0;
#endif

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "ENTER \n")));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    if (label == 0) {
        status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                   "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d port %08X\n"),
                       unit, port));
            L3_UNLOCK(unit);
            return status;
        }
    }

#ifndef PLATFORM_LABELS
    if (label == 0) {
        if (!BCM_GPORT_IS_TRUNK(vpn_sap->vc_mpls_port.port)) {
            status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                         vpn_sap->vc_mpls_port.port,
                                                         &modid,
                                                         &local_port);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) failure to get mod-port\n"),
                           bcm_errmsg(status)));
                return status;
            }
            num_ports = 1;
            } else {
                is_trunk = 1;
                trunkid = BCM_GPORT_TRUNK_GET(vpn_sap->vc_mpls_port.port);
            trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
            num_ports = trunk_info->num_ports;
            for (index = 0; index < num_ports; index++) {
                if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
                    local_port = trunk_info->tp[index];
                    break;
                }
            }
        }
        if ((l3_fe->fe_my_modid != modid && (!is_trunk)) || (local_port == -1)) {
            /**
             * If mpls port is not local, do nothing.
             * If local_port is -1, then there are no local port for the LAG
             */
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "Remote Module: Can't get stat\n")));

            return BCM_E_NONE;
        }

#ifdef USE_TUNNEL_ID
        label1 = vpn_sap->mpls_psn_ing_label;
        if (vpn_sap->mpls_psn_label2) {
            label2 = vpn_sap->mpls_psn_ing_label2;
            label3 = label;
        } else {
            label2 = label;
        }
#else
        label1 = vpn_sap->mpls_psn_label;
        if (vpn_sap->mpls_psn_label2) {
            label2 = vpn_sap->mpls_psn_label2;
            label3 = label;
        } else {
            label2 = label;
        }
#endif
    } else {
        local_port = port;
        /* assume it is lsp label */
        label1 = label;
    }
#else
    local_port = 0;
    label1 = label;
#endif


    switch (stat) {
    /* coverity[unterminated_case] */
    case bcmMplsInPkts:  pkts = 1;
    case bcmMplsInBytes:
        if (label != 0) {
            soc_sbx_g3p1_labels_t_init(&label2etc);
            status = _bcm_caladan3_g3p1_mpls_labels_get(unit, local_port,
                               label1, label2, label3,
                               &label2etc);
            if (!BCM_SUCCESS(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                             "ERROR: Failed to get lable-etc localport(%d),label(%d/%d/%d): "),
                           local_port, label1,label2,label3)); 
                return status;
            }
            lpi = label2etc.lpi;
        } else {
            lpi = vpn_sap->logicalPort;
        }
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
        if (BCM_SUCCESS(status)) {
        
            counter    = lp.counter;
            policer_id = lp.policer;
            COMPILER_64_ZERO(soc_val_tot.packets);
            COMPILER_64_ZERO(soc_val_tot.bytes);
            if (counter & policer_id) {
                status = _bcm_caladan3_policer_group_mode_get(unit, 
                                                            policer_id,
                                                           &grp_mode);
                
                if (BCM_SUCCESS(status)) {
                    status = _bcm_caladan3_g3p1_num_counters_get(unit,
                                                               grp_mode,
                                                              &num_ctrs);
                    if (BCM_SUCCESS(status)) {
                        for (i = 0; i < num_ctrs; i++) {
                            cur_ctr = counter + i;
                            status = soc_sbx_g3p1_ingctr_read(unit, cur_ctr, 1, 
                                                              clear, &soc_val);
                            if (BCM_SUCCESS(status)) {
                               /* add the counters */
                                if (pkts) {
                                    COMPILER_64_ADD_64(soc_val_tot.packets,
                                                      soc_val.packets); 
                                } else {
                                    COMPILER_64_ADD_64(soc_val_tot.bytes,
                                                      soc_val.bytes); 
                                }
                            } else {
                                LOG_ERROR(BSL_LS_BCM_MPLS,
                                          (BSL_META_U(unit,
                                                      "ERROR: Failed to read "
                                                       "ingress counter 0x%x \n"), cur_ctr));
                                break;
                            }
                        }
                        if (BCM_SUCCESS(status)) {
                            if (pkts) {
                                COMPILER_64_SET(*val, 
                                    COMPILER_64_HI(soc_val_tot.packets),
                                    COMPILER_64_LO(soc_val_tot.packets));
                            } else {
                                COMPILER_64_SET(*val, 
                                    COMPILER_64_HI(soc_val_tot.bytes),
                                    COMPILER_64_LO(soc_val_tot.bytes));
                            }
                        }
                    }
                }    
            } else {
                status = BCM_E_CONFIG;
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: counter or policer missing: "
                                       "gport %08X lpi %d counter %d policer 0x%x\n"),
                           port, lpi, counter, policer_id));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read lport entry: "
                                   "gport %08X lp index %d\n"), port, lpi));
        }
        break;
    /* coverity[unterminated_case] */
    case bcmMplsOutPkts:  pkts = 1;
    case bcmMplsOutBytes:
        COMPILER_64_ZERO(soc_val.packets);
        COMPILER_64_ZERO(soc_val.bytes);
        if (label) {
            status = _bcm_caladan3_l3_find_intf_by_egr_label(l3_fe,
                                                          label,
                                                         &l3_intf);
            if (BCM_SUCCESS(status)) {
                status = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                                      l3_intf,
                                                       _CALADAN3_L3_ETE__UCAST_IP,
                                                       &l3_ete);
                if (BCM_SUCCESS(status)) {
                    ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(l3_ete->l3_ohi.ohi);
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to find l3ete"
                                           " on l3intf for label: 0x%05X\n"), label));
                    return BCM_E_PARAM;
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to find l3intf"
                                       " for label: 0x%05X\n"), label));
                return BCM_E_PARAM;
            }
        } else {
            ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
        }
        status = soc_sbx_g3p1_oi2e_get(unit, ohi, &oi2e);
        if (BCM_SUCCESS(status)) {
            counter = oi2e.counter;
            if (counter) {
                status = soc_sbx_g3p1_egrctr_read(unit, counter, 1, 
                                                  clear, &soc_val);
                if (BCM_SUCCESS(status)) {
                    if (pkts) {
                        COMPILER_64_SET(*val, 
                                COMPILER_64_HI(soc_val.packets),
                                COMPILER_64_LO(soc_val.packets));
                    } else {
                        COMPILER_64_SET(*val, 
                                COMPILER_64_HI(soc_val.bytes),
                                COMPILER_64_LO(soc_val.bytes));
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to read "
                                           "egress counter 0x%x \n"), counter));
                }
            } else {
                status = BCM_E_CONFIG;
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: counter missing in ohi: "
                                       "gport %08X ohi %d counter %d\n"),
                           port, ohi, counter));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read ohi entry: "
                                   "gport %08X ohi %d\n"), port, ohi));
        }
        break;
    default:
        break;
    }

    return status;
}


int
bcm_caladan3_mpls_label_stat_clear(int unit,
                                 bcm_mpls_label_t label,
                                 bcm_gport_t port,
                                 bcm_mpls_stat_t stat)
{
    int          status = BCM_E_NONE;
    uint64       val = COMPILER_64_INIT(0,0);
    int          clear = 1;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!BCM_GPORT_IS_MPLS_PORT(port) && (label == 0)) {
        return BCM_E_PARAM;
    }

    status = _bcm_caladan3_mpls_label_stat_get(unit, label, port,
                                             stat, clear, &val);
    return status;
}

int
bcm_caladan3_mpls_label_stat_get(int unit,
                               bcm_mpls_label_t label,
                               bcm_gport_t port,
                               bcm_mpls_stat_t stat,
                               uint64 *val)
{
    int status = BCM_E_NONE;
    int clear = 0;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!BCM_GPORT_IS_MPLS_PORT(port) && (label == 0)) {
        return BCM_E_PARAM;
    }

    COMPILER_64_ZERO(*val);
    status = _bcm_caladan3_mpls_label_stat_get(unit, label, port,
                                             stat, clear, val);

    return status;
}

int 
bcm_caladan3_mpls_label_stat_get32(
    int unit, 
    bcm_mpls_label_t label, 
    bcm_gport_t port, 
    bcm_mpls_stat_t stat, 
    uint32 *val)
{
    int status = BCM_E_NONE;
    uint64          value64;
    
    status = bcm_caladan3_mpls_label_stat_get(unit, label, port, stat, &value64);
    if (BCM_E_NONE == status) {
        if (COMPILER_64_HI(value64) > 0) {
            /* the value is too large */
            *val = 0xFFFFFFFF;
        } else {
            /* the value will fit */
            *val = u64_L(value64);
        }
    }
    return status;
}

static int
_bcm_caladan3_mpls_egress_stat_enable(int unit,
                                    uint32 ohi,
                                    int enable)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g3p1_oi2e_t          oi2e;
    uint32                       egr_counter;
    uint32                       stat_flags = 0;

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "%s: Invalid enable value (%d). valid values - 0,1 \n"),
                   FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    /* Get the oi2e to enable / disable egress counter */
    status = soc_sbx_g3p1_oi2e_get(unit, ohi, &oi2e);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: Failed to read oi2e entry: "
                               " ohi %d\n"), ohi));
        L3_UNLOCK(unit);
        return status;
    }

    egr_counter = oi2e.counter;

    if (enable == 0) {
        if (egr_counter) {
            /* egress counter available - reset oi2e.counter
             * and free the counters */

            oi2e.counter = 0;
            status = soc_sbx_g3p1_oi2e_set(unit, ohi, &oi2e);
            if (BCM_SUCCESS(status)) {  
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Clearing counter in ohi entry : "
                                       " ohi index %d\n"), ohi));
                status = _bcm_caladan3_stat_block_free(unit,
                                   CALADAN3_G3P1_COUNTER_EGRESS, egr_counter);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to free egress counters: "
                                          "ohi %d ctr %d\n"),
                               ohi, egr_counter));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to set ohi entry: "
                                       "ohi %d\n"), ohi));
            }
        } else if (!egr_counter) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "No egress counter found: "
                                   "ohi %d\n"), ohi));
        }
    } else if (enable == 1) {
        /* enable stat for egress */
        if (egr_counter == 0) {
            
#ifdef BCM_WARM_BOOT_SUPPORT
            if (SOC_WARM_BOOT(unit)) {
                /*
                 * stat_flags = BCM_CALADAN3_STAT_WITH_ID;
                 * egr_counter = restored egr_counter id
                 */
            }
#endif /* BCM_WARM_BOOT_SUPPORT */
            status = _bcm_caladan3_stat_block_alloc(unit,
                                                    CALADAN3_G3P1_COUNTER_EGRESS, &egr_counter, 1, stat_flags);
            if (BCM_SUCCESS(status)) {
                oi2e.counter =  egr_counter;
                status = soc_sbx_g3p1_oi2e_set(unit, ohi, &oi2e);
                if (BCM_SUCCESS(status)) {
                    LOG_DEBUG(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Updating counter in ohi entry : "
                                           "ohi index %d\n"), ohi));
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set oi2e: "
                                           "ohi %d\n"), ohi));

                    _bcm_caladan3_stat_block_free(unit,
                                                CALADAN3_G3P1_COUNTER_EGRESS,
                                                egr_counter);
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to alloc"
                                       " egress counters")));
            }
        } else if (egr_counter != 0) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Egress counter is already attached: "
                                   "ohix %d\n"), ohi));
        }
        /* enable stat for egress */
        if (BCM_SUCCESS(status) && (egr_counter == 0)) {
            
#ifdef BCM_WARM_BOOT_SUPPORT
            if (SOC_WARM_BOOT(unit)) {
                /*
                 * stat_flags = BCM_CALADAN3_STAT_WITH_ID;
                 * egr_counter = restored egr_counter id
                 */
            }
#endif /* BCM_WARM_BOOT_SUPPORT */
            status = _bcm_caladan3_stat_block_alloc(unit,
                                                    CALADAN3_G3P1_COUNTER_EGRESS, &egr_counter, 1, stat_flags);
            if (BCM_SUCCESS(status)) {
                oi2e.counter =  egr_counter;
                status = soc_sbx_g3p1_oi2e_set(unit, ohi, &oi2e);
                if (BCM_SUCCESS(status)) {
                    LOG_DEBUG(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Updating counter in ohi entry : "
                                           "ohi index %d\n"), ohi));
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set oi2e: "
                                           "ohi %d\n"), ohi));

                    _bcm_caladan3_stat_block_free(unit,
                                                CALADAN3_G3P1_COUNTER_EGRESS,
                                                egr_counter);
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to alloc"
                                       " egress counters")));
            }
        } else if (egr_counter != 0) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Egress counter is already attached: "
                                   "ohix %d\n"), ohi));
        }
    }
    return status;
}
                                   

static int
_bcm_caladan3_mpls_ingress_stat_enable(int unit,
                                     uint32 lpi,
                                     int enable)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g3p1_lp_t            lp;
    bcm_policer_t                policer_id;
    bcm_policer_config_t         pol_cfg;
    uint32                       counter;

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "%s: Invalid enable value (%d). valid values - 0,1 \n"),
                   FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: Failed to read lport entry: "
                               "lp index %d\n"), lpi));
        return status;
    }
    policer_id  = lp.policer;
    counter     = lp.counter;
    if (enable == 0) {
        /* disable the gport counters - ingress and egress */
        if (counter) {
            /* ingress counter available - reset lp.counter
             * and free the counters */
            lp.counter = 0;
            status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
            if (BCM_SUCCESS(status)) {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Clearing counter in lport entry: "
                                       "lp index %d policerId 0x%x\n"),
                           lpi, policer_id));
                /* free counters */
                status = _bcm_caladan3_g3p1_free_counters(unit,
                                                        policer_id, 0,
                                                        counter);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to free ingress counters: "
                                          "lp index %d base_ctr %d\n"),
                               lpi, counter));
                }
            } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set lport entry: "
                                          "unit %d lp index %d\n"),
                               unit, lpi));
            }
        } else {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: No ingress counter found: "
                                   "lpi %d\n"), lpi));
        }

    } else if (enable == 1) {
        /* enable the gport counters - ingress and egress */
        if (policer_id == 0) {
            status = BCM_E_CONFIG; 
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: No policer attached to gport: "
                                   "lp index %d\n"), lpi));
        }
        if (BCM_SUCCESS(status) && (counter == 0)) {
            /* check policer type and alloc correct number of counters
             * update lp.counter */

            /* check if policer exists */
            status = bcm_policer_get(unit, policer_id, &pol_cfg);
            if (BCM_SUCCESS(status)) {
                status = _bcm_caladan3_g3p1_alloc_counters(unit, 
                                                         policer_id, 0,
                                                         &counter);
                if (BCM_SUCCESS(status)) {
                    lp.counter = counter;
                    status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);

                    if (BCM_SUCCESS(status)) {
                        LOG_DEBUG(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "Updating lport entry with "
                                               " counter: lpi %d policerId 0x%x\n"),
                                   lpi, policer_id));
                    } else {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "ERROR: Failed to set lport "
                                               "entry: lp index %d\n"), lpi));
                        /* failed to write lp, so free counters */
                        _bcm_caladan3_g3p1_free_counters(unit,
                                                       policer_id, 0,
                                                       counter);
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to alloc "
                                           " ingress counters")));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to get policer: "
                                       "lp index %d policerId 0x%x\n"),
                           lpi, policer_id));
            }
        } else if (counter != 0) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Ingress counter is already attached: "
                                   "lp index %d\n"), lpi));
        }

    }
    return status;

}


int
bcm_caladan3_mpls_label_stat_enable_set(int unit, 
                                      bcm_mpls_label_t label,
                                      bcm_gport_t port,
                                      int enable)
{
    int                          status = BCM_E_NONE;
    uint32                       lpi, ohi;
    soc_sbx_g3p1_labels_t       label2etc;
    _caladan3_l3_fe_instance_t      *l3_fe = NULL;
    _caladan3_l3_intf_t             *l3_intf = NULL;
    _caladan3_l3_ete_t              *l3_ete = NULL;
    int                          ingress_done = 0;

    int                          label1 = 0, label2 = 0, label3 = 0;
    int                          local_port = -1;


    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "ENTER \n")));

    if (label == 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL || label == 0) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

#ifndef PLATFORM_LABEL
    local_port = port;
    /* assume the label is lsp label */
    label1 = label;
#else
    label1 = label;
    local_port = 0;
#endif

    /* enable stats for ingress */
    soc_sbx_g3p1_labels_t_init(&label2etc);
    status = _bcm_caladan3_g3p1_mpls_labels_get(unit, local_port,
                               label1, label2, label3,
                               &label2etc); 
    if (BCM_SUCCESS(status)) {
        lpi = label2etc.lpi;
        status = _bcm_caladan3_mpls_ingress_stat_enable(unit, lpi, enable);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Error in enabling/disabling ingress stat\n")));
            L3_UNLOCK(unit);
            return status;
        }
        ingress_done = 1;
    } else {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Could not find label2e entry"
                               " for label: 0x%05X\n"), label));
        /* No Label found, try egress stat enable */
        status = BCM_E_NONE;
    }

    /* enable stats for egress */
    /* find the l3 interface by label */
    status = _bcm_caladan3_l3_find_intf_by_egr_label(l3_fe,
                                                  label,
                                                 &l3_intf);
    if (BCM_SUCCESS(status)) {
        status = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                              l3_intf,
                                               _CALADAN3_L3_ETE__UCAST_IP,
                                               &l3_ete);
        if (BCM_SUCCESS(status)) {
            /* get the ohi from l3 ete */
            ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(l3_ete->l3_ohi.ohi);
            status = _bcm_caladan3_mpls_egress_stat_enable(unit, ohi, enable);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Error in enabling/disabling egress stat\n")));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find l3ete"
                                   " on l3intf for label: 0x%05X\n"), label));
            status = BCM_E_PARAM;
        }
    } else {
        if (ingress_done == 0) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Could not find label2e or l3intf"
                                   " for label: 0x%05X\n"), label));
            status = BCM_E_CONFIG;
        } else {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find l3intf"
                                   " for label: 0x%05X\n"), label));
            status = BCM_E_NONE;
        }
    }
    L3_UNLOCK(unit);
    return status;
}
                                      
/*
 *  Function:
 *    bcm_caladan3_mpls_port_stat_enable_set
 *  Description:
 *    Enable/Disable statistics on the indicated MPLS gport
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in int enable - nonzero to enable stats, zero to disable stats
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit
 */
int
bcm_caladan3_mpls_port_stat_enable_set(int unit, bcm_gport_t port, int enable)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g3p1_lp_t            lp;
    soc_sbx_g3p1_oi2e_t          oi2e;
    uint32                       lpi, ohi;
    bcm_policer_t                policer_id;
    bcm_policer_config_t         pol_cfg;
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    uint32                       counter, egr_counter;
    uint32                       stat_flags = 0;

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "%s: Invalid enable value (%d). valid values - 0,1 \n"),
                   FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    /* Get the vpn sap using the gport id*/
    status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: Failed to find vpn sap for gport: "
                              "unit %d port %08X\n"),
                   unit, port));
        L3_UNLOCK(unit);
        return status;
    }

    /* Get the lp entry to enable / disable ingress counter */
    lpi = vpn_sap->logicalPort;
    status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: Failed to read lport entry: "
                               "gport %08X lp index %d\n"), port, lpi));
        L3_UNLOCK(unit);
        return status;
    }

    /* Get the oi2e to enable / disable egress counter */
    ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
    status = soc_sbx_g3p1_oi2e_get(unit, ohi, &oi2e);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: Failed to read oi2e entry: "
                               "gport %08X ohi %d\n"), port, ohi));
        L3_UNLOCK(unit);
        return status;
    }

    policer_id  = lp.policer;
    counter     = lp.counter;
    egr_counter = oi2e.counter;

    if (enable == 0) {
        /* disable the gport counters - ingress and egress */
        if (counter) {
            /* ingress counter available - reset lp.counter
             * and free the counters */
            lp.counter = 0;
            status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
            if (BCM_SUCCESS(status)) {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Clearing counter in lport entry: "
                                       " gport %08X lp index %d policerId 0x%x\n"),
                           port, lpi, policer_id));
                /* free counters */
                status = _bcm_caladan3_g3p1_free_counters(unit,
                                                        policer_id, 0,
                                                        counter);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                           "ERROR: Failed to free ingress counters: "
                                           "gport %08X lp index %d base_ctr %d\n"),
                               port, lpi, counter));
                }
            } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set lport entry: "
                                          "unit %d gport %08X lp index %d\n"),
                               unit, port, lpi));
            }
        } else {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: No ingress counter found: "
                                   "gport %08X ohi %d\n"), port, ohi));
        }

        if (BCM_SUCCESS(status) && egr_counter) {
            /* egress counter available - reset oi2e.counter
             * and free the counters */

            oi2e.counter = 0;
            status = soc_sbx_g3p1_oi2e_set(unit, ohi, &oi2e);
            if (BCM_SUCCESS(status)) {  
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Clearing counter in ohi entry : "
                                       "gport %08X ohi index %d\n"), port, ohi));
                status = _bcm_caladan3_stat_block_free(unit,
                                   CALADAN3_G3P1_COUNTER_EGRESS, egr_counter);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to free egress counters: "
                                          "gport %08X ohi %d ctr %d\n"),
                               port, ohi, egr_counter));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to set ohi entry: "
                                       "gport %08X lp index %d\n"), port, lpi));
            }
        } else if (!egr_counter) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "No egress counter found: "
                                   "gport %08X ohi %d\n"), port, ohi));
        }
    } else if (enable == 1) {
        /* enable the gport counters - ingress and egress */
        if (policer_id == 0) {
            status = BCM_E_CONFIG; 
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: No policer attached to gport: "
                                   "gport %08X lp index %d\n"), port, lpi));
        }
        if (BCM_SUCCESS(status) && (counter == 0)) {
            /* check policer type and alloc correct number of counters
             * update lp.counter */

            /* check if policer exists */
            status = bcm_policer_get(unit, policer_id, &pol_cfg);
            if (BCM_SUCCESS(status)) {
                status = _bcm_caladan3_g3p1_alloc_counters(unit, 
                                                         policer_id, 0,
                                                         &counter);
                if (BCM_SUCCESS(status)) {
                    lp.counter = counter;
                    status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);

                    if (BCM_SUCCESS(status)) {
                        LOG_DEBUG(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "Updating lport entry with "
                                               " counter: gport %08X lpi %d policerId 0x%x\n"),
                                   port, lpi, policer_id));
                    } else {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "ERROR: Failed to set lport "
                                               "entry: gport %08X lp index %d\n"), port, lpi));
                        /* failed to write lp, so free counters */
                        _bcm_caladan3_g3p1_free_counters(unit,
                                                       policer_id, 0,
                                                       counter);
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to alloc "
                                           " ingress counters")));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to get policer: "
                                       "gport %08X lp index %d policerId 0x%x\n"),
                           port, lpi, policer_id));
            }
        } else if (counter != 0) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Ingress counter is already attached: "
                                   "gport %08X lp index %d\n"), port, lpi));
        }

        /* enable stat for egress */
        if (BCM_SUCCESS(status) && (egr_counter == 0)) {
#ifdef BCM_WARM_BOOT_SUPPORT
            if (SOC_WARM_BOOT(unit)) {
                /*
                 * stat_flags = BCM_CALADAN3_STAT_WITH_ID;
                 * egr_counter = restored egr_counter
                 */
            }
#endif /* BCM_WARM_BOOT_SUPPORT */
            status = _bcm_caladan3_stat_block_alloc(unit,
                                                    CALADAN3_G3P1_COUNTER_EGRESS, &egr_counter, 1, stat_flags);
            if (BCM_SUCCESS(status)) {
                oi2e.counter =  egr_counter;
                status = soc_sbx_g3p1_oi2e_set(unit, ohi, &oi2e);
                if (BCM_SUCCESS(status)) {
                    LOG_DEBUG(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Updating counter in ohi entry : "
                                           "gport %08X ohi index %d\n"), port, ohi));
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set oi2e: "
                                           "gport %08X ohi %d\n"), port, ohi));

                    _bcm_caladan3_stat_block_free(unit,
                                                CALADAN3_G3P1_COUNTER_EGRESS,
                                                egr_counter);
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to alloc"
                                       " egress counters")));
            }

        } else if (egr_counter != 0) {
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Egress counter is already attached: "
                                   "gport %08X ohix %d\n"), port, ohi));
        }
    }

    L3_UNLOCK(unit);

    return status;
}

/*
 *  Function:
 *    bcm_caladan3_mpls_port_stat_set
 *  Description:
 *    Set the specified MPLS gport statistic to the indicated value
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint64 val - the new value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_caladan3_mpls_port_stat_set(int unit,
                              bcm_gport_t port,
                              bcm_cos_t cos,
                              bcm_mpls_port_stat_t stat,
                              uint64 val)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g3p1_lp_t            lp;
    uint32                       lpi;
    bcm_policer_t                policer_id;
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    uint32                       counter;
    int                          clear = 1;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    LOG_DEBUG(BSL_LS_BCM_COUNTER,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, 0x%08X %08X) - Enter\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat,
               u64_H(val),
               u64_L(val)));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            counter    = lp.counter;
            policer_id = lp.policer;
            if (counter & policer_id) {
                  status = _bcm_caladan3_mpls_policer_stat_rw(unit, clear,
                                                            policer_id,
                                                            cos, stat,
                                                            counter, &val);
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: No policer or coutner attached to gport: "
                                      "unit %d gport %08X counter %d policerId 0x%x\n"),
                           unit, port, counter, policer_id));
                status = BCM_E_CONFIG;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read lport entry: "
                                  "unit %d port %08X lp index %d\n"),
                       unit, port, lpi));
        }
        
    } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d port %08X\n"),
                       unit, port));
    }

    LOG_DEBUG(BSL_LS_BCM_COUNTER,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, 0x%08X %08X) - Exit %d (%s)\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat,
               u64_H(val),
               u64_L(val),
               status,
               bcm_errmsg(status)));

    return status;
}

/*
 *  Function:
 *    bcm_caladan3_mpls_port_stat_set32
 *  Description:
 *    Set the specified MPLS gport statistic to the indicated value (32b)
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint32 val - the new value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_caladan3_mpls_port_stat_set32(int unit,
                                bcm_gport_t port,
                                bcm_cos_t cos,
                                bcm_mpls_port_stat_t stat,
                                uint32 val)
{
    uint64          value64;
    int             result;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    LOG_DEBUG(BSL_LS_BCM_COUNTER,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, %08X) - Enter\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat,
               val));

    STAT_CHECK_STAT_VALUE(val);     /* Only allowed to set to zero */

    COMPILER_64_SET(value64, 0, val);
    result = bcm_caladan3_mpls_port_stat_set(unit, port, cos, stat, value64);

    LOG_DEBUG(BSL_LS_BCM_COUNTER,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, %08X) - Exit %d (%s)\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat,
               val,
               result,
               bcm_errmsg(result)));
    return result;
}

/*
 *  Function:
 *    bcm_caladan3_mpls_port_stat_get
 *  Description:
 *    Get the specified MPLS gport statistic value
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint64 *val - where to put the value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_caladan3_mpls_port_stat_get(int unit,
                              bcm_gport_t port,
                              bcm_cos_t cos,
                              bcm_mpls_port_stat_t stat,
                              uint64 *val)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g3p1_lp_t            lp;
    uint32                       lpi;
    bcm_policer_t                policer_id;
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    uint32                       counter;
    int                          clear = 0;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, *) - Enter\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            counter    = lp.counter;
            policer_id = lp.policer;
            if (counter & policer_id) {
                status = _bcm_caladan3_mpls_policer_stat_rw(unit, clear,
                                                          policer_id,
                                                          cos, stat,
                                                          counter, val);
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: No policer or coutner attached to gport: "
                                      "unit %d gport %08X counter %d policerId 0x%x\n"),
                           unit, port, counter, policer_id));
                status = BCM_E_CONFIG;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read lport entry: "
                                  "unit %d port %08X lp index %d\n"),
                       unit, port, lpi));
        }
        
    } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d port %08X\n"),
                       unit, port));
    }


    if (val) {
        LOG_DEBUG(BSL_LS_BCM_COUNTER,
                  (BSL_META_U(unit,
                              "%s(%d, %08X, %d, %d, &(0x%08X %08X)) - Exit %d (%s)\n"),
                   FUNCTION_NAME(),
                   unit,
                   port,
                   cos,
                   stat,
                   u64_H(*val),
                   u64_L(*val),
                   status,
                   bcm_errmsg(status)));
    }

    return status;
}

/*
 *  Function:
 *    bcm_caladan3_mpls_port_stat_get32
 *  Description:
 *    Get the specified MPLS gport statistic value (32b)
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint32 *val - where to put the value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_caladan3_mpls_port_stat_get32(int unit,
                                bcm_gport_t port,
                                bcm_cos_t cos,
                                bcm_mpls_port_stat_t stat,
                                uint32 *val)
{
    int             result;
    uint64          value64 = COMPILER_64_INIT(0,0);

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    LOG_DEBUG(BSL_LS_BCM_COUNTER,
              (BSL_META_U(unit,
                          "%s(%d, %08X, %d, %d, *) - Enter\n"),
               FUNCTION_NAME(),
               unit,
               port,
               cos,
               stat));

    result = bcm_caladan3_mpls_port_stat_get(unit, port, cos, stat, &value64);
    if (BCM_E_NONE == result) {
        if (COMPILER_64_HI(value64) > 0) {
            /* the value is too large */
            *val = 0xFFFFFFFF;
        } else {
            /* the value will fit */
            *val = u64_L(value64);
        }
    }

    if (val) {
        LOG_DEBUG(BSL_LS_BCM_COUNTER,
                  (BSL_META_U(unit,
                              "%s(%d, %08X, %d, %d, &(%08X)) - Exit %d (%s)\n"),
                   FUNCTION_NAME(),
                   unit,
                   port,
                   cos,
                   stat,
                   *val,
                   result,
                   bcm_errmsg(result)));
    }

    return result;
}


int
_bcm_caladan3_mpls_port_info_get(int unit, int port, int vid,
                               bcm_vlan_t vsi, uint8 vpwsuni, 
                               int *keepUntagged)
{
    _caladan3_l3_fe_instance_t *l3_fe = NULL;
    bcm_gport_t            mpls_port_id = 0;
    int                    rv;
    bcm_module_t           lcl_module;
    _caladan3_vpn_control_t    *vpncb = NULL;
    _caladan3_vpn_sap_t        *vpnsap = NULL; 


    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }

    rv = bcm_stk_my_modid_get(unit, &lcl_module);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) failed to get mod id\n"),
                   bcm_errmsg(rv)));
        return rv;
    }

        if (vpwsuni) {
            /* This functionality is only supported on AC port-vid match */
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, 
                                       l3_fe->vpws_uni_ft_offset + vsi);
            rv = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, mpls_port_id, &vpnsap); 
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) failed to get vpn SAP block\n"),
                           bcm_errmsg(rv)));
                return rv;
            }
        } else {

            bcm_gport_t modgport = 0;
            BCM_GPORT_MODPORT_SET(modgport, lcl_module, port);

            rv = _bcm_caladan3_find_mpls_vpncb_by_id(unit, l3_fe, 
                                                   vsi, &vpncb);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) failed to get vpn control block\n"),
                           bcm_errmsg(rv)));
                return rv;
            } else {
                /* for now assume only mod-gport */
                /* try to find gport by port + vid match */
                rv = _bcm_caladan3_find_vpn_sap_by_port_vlan(l3_fe, vpncb, 
                                                           modgport, vid, &vpnsap);
                if (BCM_FAILURE(rv)) {
                    /* try to find gport with port only match */
                    rv = _bcm_caladan3_find_vpn_sap_by_port(l3_fe, vpncb, 
                                                           modgport, &vpnsap);
                }
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit,
                                          "error(%s) failed to get vpn SAP block\n"),
                               bcm_errmsg(rv)));
                    return rv;
                }
            }
        }
        /* keepUntagged specifies the behavior in ucode where a tag is added to
         * untagged traffic when the port is set to keep tags.  Setting this flag
         * results in untagged traffic remaining untagged as it traverses the fabric,
         * clearing it would result in the port's native vid to be inserted.
         * For mpls ports, we ALWAYS want to keep the packet unchanged, so we set it
         * whenever inner_vlan_preserve is set.
         */
        *keepUntagged = !(vpnsap->vc_mpls_port.flags & BCM_MPLS_PORT_INNER_VLAN_ADD);

    return BCM_E_NONE;
}

/*
 *  Function:
 *    bcm_caladan3_mpls_gport_get
 *  Description:
 *    Get the specified MPLS gport information
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    _caladan3_vpn_sap_t * 
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */
int
bcm_caladan3_mpls_gport_get(int unit,
                          bcm_gport_t gport,
                          _caladan3_vpn_sap_t **vpn_sap)
{
    int status = BCM_E_NONE;



    if(L3_UNIT_INVALID(unit) || !vpn_sap) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Bad input parameter\n")));
    } else{
        BCM_IF_ERROR_RETURN(L3_LOCK(unit));
        status = shr_htb_find(mpls_gport_db[unit], 
                          (shr_htb_key_t) &gport,
                          (shr_htb_data_t*)vpn_sap, 0);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed:(%d)(%s) to obtain Gport information Gport[0x%x] \n"),
                       status,bcm_errmsg(status),gport));            
        }
        L3_UNLOCK(unit);                             
    }
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_mpls_policer_set
 * Purpose:
 *     Set Policer ID for a Mpls Gport
 * Returns:
 *     BCM_E_XX
 */
int
_bcm_caladan3_mpls_policer_set(int unit,
                             bcm_gport_t gport,
                             bcm_policer_t pol_id)
{
    int status = BCM_E_NONE;
    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    soc_sbx_g3p1_lp_t lp;
    uint32 lpi;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if (BCM_SUCCESS(status)) {

        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {

            status = _bcm_caladan3_g3p1_policer_lp_program(unit, pol_id, &lp);

            if (BCM_SUCCESS(status)) {
                status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);

                if (BCM_SUCCESS(status)) {
                    LOG_DEBUG(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "Updating lport entry with policer: unit %d"
                                          " gport %08X lp index %d policerId 0x%x\n"),
                               unit, gport, lpi, pol_id));
                } else {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "ERROR: Failed to set lport entry: "
                                          "unit %d gport %08X lp index %d\n"),
                               unit, gport, lpi));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "ERROR: Failed setting policer to lport: "
                                      "unit %d gport %08X lp index %d\n"),
                           unit, gport, lpi));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read lport entry: "
                                  "unit %d gport %08X lp index %d\n"),
                       unit, gport, lpi));
        }
    } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d gport %08X\n"),
                       unit, gport));
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_mpls_policer_get
 * Purpose:
 *     Get Policer ID for a Mpls Gport
 * Returns:
 *     BCM_E_XX
 */
int
_bcm_caladan3_mpls_policer_get(int unit,
                             bcm_gport_t gport,
                             bcm_policer_t *pol_id)
{
    int status = BCM_E_NONE;

    _caladan3_l3_fe_instance_t       *l3_fe = NULL;
    _caladan3_vpn_sap_t              *vpn_sap = NULL;
    soc_sbx_g3p1_lp_t lp;
    uint32 lpi;

    if (!pol_id) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Invalid parameter: NULL pointer for pol_id\n")));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            *pol_id = lp.policer;
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read lport entry: "
                                  "unit %d gport %08X lp index %d\n"),
                       unit, gport, lpi));
        }
    } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d gport %08X\n"),
                       unit, gport));
    }

    L3_UNLOCK(unit);
    return status;
}

int
_bcm_caladan3_mpls_label_commit_enable_set(int unit, uint32 enable)
{
    int status;
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
#if !CALADAN3_UNPORTED
    status = soc_sbx_g3p1_labels_commit_set(unit, enable);
#else 
    status = BCM_E_NONE;
#endif /* !CALADAN3_UNPORTED */
    if (BCM_FAILURE(status)) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: failed to set labe2e_commit")));
    }
    L3_UNLOCK(unit);
    return status;
}

int
_bcm_caladan3_mpls_label_commit_enable_get(int unit, uint32 *enable)
{
    int status;
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
#if !CALADAN3_UNPORTED
    status = soc_sbx_g3p1_labels_commit_get(unit, enable);
#else 
    status = BCM_E_NONE;
#endif /* !CALADAN3_UNPORTED */

    if (BCM_FAILURE(status)) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: failed to get labe2e_commit")));
    }
    L3_UNLOCK(unit);
    return status;
}


int
_bcm_caladan3_mpls_label_commit(int unit)
{
    int status;
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
#if !CALADAN3_UNPORTED
    status = soc_sbx_g3p1_labels_commit(unit, -1);
#else 
    status = BCM_E_NONE;
#endif /* !CALADAN3_UNPORTED */
    if (BCM_FAILURE(status)) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "ERROR: failed to commit label2e")));
    }
    L3_UNLOCK(unit);
    return status;
}

int
_bcm_caladan3_mpls_switch_ft_lp_get(_caladan3_l3_fe_instance_t *l3_fe,
                                    bcm_mpls_tunnel_switch_t   *info,
                                    uint32                     *ftidx,
                                    uint32                     *lpidx)
{
    int                   status;
    soc_sbx_g3p1_labels_t label2e;
    bcm_port_t            port  = -1;
    int                   label1 = 0;
    int                   label2 = 0;
    int                   label3 = 0;
#ifndef PLATFORM_LABELS
    bcm_module_t          modid = -1;
    uint32                trunkid;
    bcm_trunk_add_info_t *trunk_info = NULL;
    uint8                 index, is_trunk = 0;
#endif

#ifndef PLATFORM_LABELS
   if (!BCM_GPORT_IS_TRUNK(info->port)) {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                      info->port,
                                                      &modid,
                                                      &port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                     (BSL_META_U(l3_fe->fe_unit,
                                 "error(%s) failure to get mod-port\n"),
                      bcm_errmsg(status)));
            return status;
        }
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(info->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        for (index = 0; index < trunk_info->num_ports; index++) {
            if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
                port = trunk_info->tp[index];
                break;
            }
        }
    }

    if ((l3_fe->fe_my_modid != modid && (!is_trunk)) || (port == -1)) {
        /**
         * If port is -1, then there are no local port for the LAG
         */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit,
                                "Remote Module:No update needed\n")));
        return BCM_E_NOT_FOUND;
    }

    label1 = info->label;
    label2 = info->second_label;
#else
    port = 0;
    label1 = info->label;
#endif
    
    status = _bcm_caladan3_g3p1_mpls_labels_get(l3_fe->fe_unit, port,
                                label1, label2, label3,
                                &label2e);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                 (BSL_META_U(l3_fe->fe_unit,
                           "Labels Get failed:%d "
                           "(port:0x%x, label1:0x%x label2:0x%x label3:0x%x\n"),
                  status, port, label1, label2, label3));
        return status;
    }
    *lpidx = label2e.lpi;
    if (info->action != BCM_MPLS_SWITCH_ACTION_POP) {
        *ftidx = label2e.ftidx;
    } else {
        *ftidx = SBX_DROP_FTE(l3_fe->fe_unit);
    }
    return BCM_E_NONE;
}


void
_bcm_caladan3_mpls_switch_info_db_get(int                   unit,
                                      shr_htb_hash_table_t *switch_info_db)
{
    *switch_info_db = mpls_switch_info_db[unit];
}

int
_bcm_caladan3_mpls_switch_info_db_add(int unit,
                                      bcm_mpls_tunnel_switch_t *switch_info)
{
    uint8 key[MPLS_SWITCH_INFO_KEY_SIZE];
    int status;
    _bcm_caladan3_mpls_switch_key_get(&key,
                                      switch_info->label,
                                      switch_info->second_label,
                                      switch_info->port,
                                      switch_info->tunnel_id);
    status = shr_htb_insert(mpls_switch_info_db[unit],
                            (shr_htb_key_t) &key,
                            (void*) switch_info);
    return status;
}

int
_bcm_caladan3_mpls_gport_info_db_add(int unit, _caladan3_vpn_sap_t *vpn_sap)
{
    int status;
    status = shr_htb_insert(mpls_gport_db[unit],
                            (shr_htb_key_t) &vpn_sap->vc_mpls_port_id,
                            (void*) vpn_sap);
    return status;
}

int
_bcm_caladan3_mpls_trunk_assoc_gport_add(int unit, _caladan3_vpn_sap_t *vpn_sap)
{
    int status = BCM_E_NONE;
    bcm_caladan3_mpls_trunk_association_t *trunkAssoc = NULL;
    uint32 trunkid = -1;
    trunkid = BCM_GPORT_TRUNK_GET(vpn_sap->vc_mpls_port.port);
    trunkAssoc = &mpls_trunk_assoc_info[unit][trunkid];
    DQ_INSERT_HEAD(&trunkAssoc->plist, &vpn_sap->trunk_port_link);
    return status;
}


void
_bcm_caladan3_mpls_vpnc_dump(int                      unit, 
                             _caladan3_vpn_control_t *vpnc,
                             int                      printHdr)
{
    if (printHdr) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "--VPN_ID---VRF----bcMcg"
                                "-----Flags-----Color\n")));
    }
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "%6x  %6x   %6x   %8x    %2x\n"),
                 vpnc->vpn_id,
                 vpnc->vpn_vrf,
                 vpnc->vpn_bc_mcg,
                 vpnc->vpn_flags,
                 vpnc->vpls_color));
}

void
_bcm_caladan3_mpls_port_dump(int unit, bcm_mpls_port_t *mpls_port, int printHdr)
{
    if (printHdr) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "MPLS_PORT:\n")));
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                  "  portId   flags  eMap iPri pPri pCfi sTpid"
                  "  port    crit  mVlan  mIVlan  mLabel  eTun eLbl MTU eSVlan"
                  "    encapId    foid    fopid    policer    PWFoId PWFoPid"
                  " VccvT Tid\n")));
    }
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "%8x %8x  %2x   %1x   %1x   %1x   %4x   %2x   %1x"
                            "   %4x   %4x   %5x   %5x   %5x   %2x  %3x  %5x  "
                            "%4x  %8x  %5x  %4x  %8x  %3x  %5x\n"),
                 mpls_port->mpls_port_id,
                 mpls_port->flags,
                 mpls_port->exp_map,
                 mpls_port->int_pri,
                 mpls_port->pkt_pri,
                 mpls_port->pkt_cfi,
                 mpls_port->service_tpid,
                 mpls_port->port,
                 mpls_port->criteria,
                 mpls_port->match_vlan,
                 mpls_port->match_inner_vlan,
                 mpls_port->match_label,
                 mpls_port->egress_tunnel_if,
                 mpls_port->egress_label.label,
                 mpls_port->mtu,
                 mpls_port->egress_service_vlan,
                 mpls_port->encap_id,
                 mpls_port->failover_id,
                 mpls_port->failover_port_id,
                 mpls_port->policer_id,
                 mpls_port->pw_failover_id,
                 mpls_port->pw_failover_port_id,
                 mpls_port->vccv_type,
                 mpls_port->tunnel_id));
/* fields not printed */
/*
                        mpls_port->if_class,
                        mpls_port->pw_seq_number,
                        mpls_port->failover_mc_group,
*/

}


void
_bcm_caladan3_mpls_vpn_sap_dump(int unit, 
                                _caladan3_vpn_sap_t *vpn_sap,
                                int printHdr)
{
    int i;
    if (printHdr) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "VPN_SAP:   ohi  ete   statidx aUE  uUE  aVC"
                                "    fte_mod fte_idx\n")));
    }
    if (vpn_sap->vc_inuse_ue == 0) {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                               "VPN_SAP:  %5x   %5x   %5x   %2x   %2x  %2x\n"),
                     vpn_sap->vc_ohi.ohi,
                     vpn_sap->vc_ete_hw_idx.ete_idx,
                     vpn_sap->vc_ete_hw_stat_idx,
                     vpn_sap->vc_alloced_ue,
                     vpn_sap->vc_inuse_ue,
                        vpn_sap->vc_res_alloced));
    } else {
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(unit,
                                "VPN_SAP:  %5x   %5x   %5x   %2x   %2x  %2x  "
                                "%5x    %5x\n"),
                     vpn_sap->vc_ohi.ohi,
                     vpn_sap->vc_ete_hw_idx.ete_idx,
                     vpn_sap->vc_ete_hw_stat_idx,
                     vpn_sap->vc_alloced_ue,
                     vpn_sap->vc_inuse_ue,
                     vpn_sap->vc_res_alloced,
                     vpn_sap->u.vc_fte[0].mod_id,
                     vpn_sap->u.vc_fte[0].fte_idx.fte_idx));
        /* MPLS always uses vc_fte */
        for (i = 1; i < vpn_sap->vc_inuse_ue; i++) {
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "VPN_SAP -fte_mod: 0x%x  fte_idx: 0x%x\n"),
                         vpn_sap->u.vc_fte[i].mod_id,
                         vpn_sap->u.vc_fte[i].fte_idx.fte_idx));
        }
    }
}


STATIC int
_bcm_caladan3_mpls_switch_info_dump(int unit,
                                    shr_htb_key_t key,
                                    shr_htb_data_t data)
{
    bcm_mpls_tunnel_switch_t * info;
    info = (bcm_mpls_tunnel_switch_t *) data;
/*
    MPLS_VERB((_SBX_D(unit, "TID     Flags   label   label2  port    act "
                            "mcgroup expM iPri PolId  VPN     egrLbl  EgrIf"
                            "   IngIf   MTU   QosId   TunIf   EgrPort\n")));
*/
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "%5x   %7x %5x   %5x   %5x   %1x   %5x   %2x  "
                            "%1x    %5x    %5x   %5x   %5x   %5x   %3x   %5x"
                            "   %5x   %5x\n"),
                 info->tunnel_id,
                 info->flags,
                 info->label,
                 info->second_label,
                 info->port,
                 info->action,
                 info->mc_group,
                 info->exp_map,
                 info->int_pri,
                 info->policer_id,
                 info->vpn,
                 info->egress_label.label,
                 info->egress_if,
                 info->ingress_if,
                 info->mtu,
                 info->qos_map_id,
                 info->tunnel_if,
                 info->egress_port));
    return BCM_E_NONE;
}


void
_bcm_caladan3_mpls_sw_dump(int unit)
{
    _caladan3_l3_fe_instance_t *l3_fe = NULL;
    int hidx;
    _caladan3_vpn_control_t *vpnc     = NULL;
    _caladan3_vpn_sap_t     *vpn_sap  = NULL;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (NULL == l3_fe) {
        return;
    }

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "MPLS Debug\n")));
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "MPLS VPN-List\n")));
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "--VPN_ID---VRF----bcMcg-----Flags-----Color\n")));
    for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
        _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc) {
            _bcm_caladan3_mpls_vpnc_dump(unit, vpnc, 0);
        } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_Fe, hidx, vpnc);
    }

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "VPN_SAP:   ohi  ete   statidx aUE  uUE  aVC    "
                            "fte_mod fte_idx\n")));
    for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
        _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc) {
            _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                _bcm_caladan3_mpls_vpn_sap_dump(unit, vpn_sap, 0);
            } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);
        } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_Fe, hidx, vpnc);
    }

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "MPLS_PORT:\n")));
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "  portId   flags  eMap iPri pPri pCfi sTpid  "
                "port    crit  mVlan  mIVlan  mLabel  eTun eLbl MTU eSVlan    "
                "encapId    foid    fopid    policer    PWFoId PWFoPid "
                "VccvT Tid\n")));
    for (hidx = 0; hidx < _CALADAN3_VPN_HASH_SIZE; hidx++) {
        _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc) {
            _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                _bcm_caladan3_mpls_port_dump(unit, &vpn_sap->vc_mpls_port, 0);
            } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);
        } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_Fe, hidx, vpnc);
    }

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(unit,
                            "\t    TID     Flags   label   label2  port    "
             "act mcgroup expM iPri PolId  VPN     egrLbl  EgrIf   IngIf   "
             "MTU   QosId   TunIf   EgrPort\n")));
    shr_htb_iterate(unit, mpls_switch_info_db[unit],
                                 _bcm_caladan3_mpls_switch_info_dump);
}


#else   /* INCLUDE_L3 && BCM_MPLS_SUPPORT */
int bcm_caladan3_mpls_not_empty;
#endif  /* INCLUDE_L3 && BCM_MPLS_SUPPORT */

