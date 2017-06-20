/*
 * $Id: mpls.c,v 1.31 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mpls.c
 * Purpose:     BCM mpls API
 */

#ifdef BCM_CALADAN3_G3P1_SUPPORT

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>

#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <shared/gport.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <bcm/vlan.h>
#include <bcm/pkt.h>
#include <bcm/policer.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <bcm_int/sbx/caladan3/qos.h>

#include <bcm_int/sbx/stat.h>

#include <bcm_int/sbx/caladan3/vlan.h>
#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>
#include <bcm_int/sbx/caladan3/policer.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/caladan3/g3p1.h>

#define _CALADAN3_IS_MPLS_INITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags & _CALADAN3_L3_FE_FLG_MPLS_INIT)

#define SB_COMMIT_COMPLETE   0xffffffff

#define TRUNK_SBX_HASH_SIZE  3 
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)
#define TRUNK_INDEX_SET(tid, offset)                \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?          \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) :  \
     -1

#define VLAN_INVALID  ((uint32)0)

/******************************************/

/******************************************/
#ifdef BCM_FE2000_SUPPORT
#include <soc/sbx/g2p3/c2_g2p3_mplstp_defs.h>
#else
int soc_sbx_g2p3_redundant_vpls_pw_ft_base_get(int unit, uint32 *vp)
{
    return BCM_E_UNAVAIL;
}

int soc_sbx_g2p3_redundant_vpws_pw_ft_base_get(int unit, uint32 *vp)
{
    return BCM_E_UNAVAIL;
}

int soc_sbx_g2p3_vpws_ft_offset_get(int unit, uint32 *vp)
{
    return BCM_E_UNAVAIL;
}

int soc_sbx_g2p3_mpls_stag_bundled_mode_get(int unit, uint32 *vp)
{
    return BCM_E_UNAVAIL;
}

int soc_sbx_g2p3_pvv2e_commit(int unit, int runlength)
{
    return BCM_E_UNAVAIL;
}
    

/*****************************************************/

/*****************************************************/
#endif /* BCM_FE2000_SUPPORT */


int
_bcm_caladan3_g3p1_mpls_update_vpxs_hw(_caladan3_l3_fe_instance_t  *l3_fe,
                                       _caladan3_vpn_sap_t         *vpn_sap,
                                     bcm_mpls_port_t         *mpls_port)
{
    int                           status = BCM_E_UNAVAIL;
    soc_sbx_g3p1_oi2e_t           ohi_ent;
    soc_sbx_g3p1_ete_t       ete;
    bcm_port_t                    port;
    int                           modid = -1;
    int                           hidx;
    uint32                        ohi = 0;
    int8                        is_trunk = 0;
    int                           max_frame_size = SBX_DEFAULT_MTU_SIZE;

    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     mpls_port->port,
                                                     &modid,
                                                     &port);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error %d (%s) extracting mod-port from gport\n"),
                       status, bcm_errmsg(status)));
            return status;
        }
    } else {
        is_trunk = 1;
    }

    /* if replace, need to remove old _ohi hash entry, it will be replaced
     * by the logic below.  The replacement may be the same ohi.
     */
    if (mpls_port->flags & BCM_MPLS_PORT_REPLACE &&
        !(vpn_sap->vc_ohi.ohi == 0 ||
          vpn_sap->vc_ohi.ohi == _CALADAN3_INVALID_OHI))
    {
        DQ_REMOVE(&vpn_sap->vc_ohi_link);

        hidx = _CALADAN3_GET_OHI2ETE_HASH_IDX(vpn_sap->vc_ohi.ohi);
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Removed ohi=0x%04x from sw hash idx=%d\n"),
                     vpn_sap->vc_ohi.ohi, hidx));
        /* the OHI to ETE link will be re-added below */
    }

    /* update internal sw state, allocate resources, if necessary; */
    if (vpn_sap->vc_ohi.ohi == _CALADAN3_INVALID_OHI ||
        mpls_port->encap_id == 0)
    {

        if ((mpls_port->flags & BCM_MPLS_PORT_ENCAP_WITH_ID) ||
            (mpls_port->encap_id == 0)) {
            vpn_sap->vc_res_alloced = 1;
        }

        status = _bcm_caladan3_mpls_alloc_vpn_sap_hw_resources(l3_fe,
                                                             vpn_sap,
                                                             mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error %d (%s) allocating vpn-sap resources for "
                       " LAN/L1 ports\n"),
                       status, bcm_errmsg(status)));
            vpn_sap->vc_res_alloced = 0;
            return status;
        }


    }  else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id)) {
        vpn_sap->vc_ohi.ohi =
            SOC_SBX_OHI_FROM_L2_ENCAP_ID(mpls_port->encap_id);
        status  = BCM_E_NONE;

    } else if (SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id)) {

        vpn_sap->vc_ohi.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
        status  = BCM_E_NONE;

    }

    if ((l3_fe->fe_my_modid == modid || is_trunk) &&
           !vpn_sap->vc_res_alloced) {

        /* track only OHI on the local unit, otherwise we risk the possibility
         * of collisions with other modules having the same OHI!
         */
        hidx = _CALADAN3_GET_OHI2ETE_HASH_IDX(vpn_sap->vc_ohi.ohi);
        DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx],
                       &vpn_sap->vc_ohi_link);
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Insert ohi=0x%08x into sw hash idx=%d\n"),
                     vpn_sap->vc_ohi.ohi, hidx));

        soc_sbx_g3p1_oi2e_t_init(&ohi_ent);

        ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
        status = soc_sbx_g3p1_oi2e_get(l3_fe->fe_unit, ohi, &ohi_ent);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error %d (%s) reading oi2e(0x%x)/(0x%x)\n"),
                       status, bcm_errmsg(status), ohi, 
                       SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id)));
            return status;
        }
        vpn_sap->vc_ete_hw_idx.ete_idx = ohi_ent.eteptr;
    }

    soc_sbx_g3p1_oi2e_t_init(&ohi_ent);
    
    /* do not call soc_sbx_..._ete_t_init(&encap_ete), it set some 
     * some attributes not needed on the CEP
     */
    sal_memset(&ete, 0, sizeof(ete));

    if (vpn_sap->vc_res_alloced || (mpls_port->flags & BCM_MPLS_PORT_REPLACE)) {

        /* setup common attributes */
        if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
            if (!(mpls_port->flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {
                /* FAILOVER port uses dynamic ft range (ft > 16K),
                 * learning is not available. So FAILOVERs should use
                 * primary port's pid.
                 * This will be configured on updating primary port with 
                 * failover details. So skip this if failover port or set the 
                 * pid if primary.
                 */
                ete.nosplitcheck = 1;
                ete.etepid = 1;
                ete.pid    = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
            }
        } else {
            /* VPWS - No learning. So no need to set the pid */
            ete.nosplitcheck = 0;
            ete.etepid = 0;
        }
        if (!is_trunk) {
            status = bcm_port_frame_max_get(l3_fe->fe_unit, port, &max_frame_size);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error %d (%s) in frame_max_get"),
                           status, bcm_errmsg(status)));
                return status;
            }
            if (mpls_port->mtu > 16000) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "port mtu is too big")));
                return BCM_E_PARAM;
            }
            ete.mtu = (mpls_port->mtu > 0) ? mpls_port->mtu : max_frame_size;
        } else {
            /* Do we have to walk the list of ports in trunk and check the lowest?
             * how to handle trunk updates... lets keep it simple for now!
             */
            ete.mtu = SBX_DEFAULT_MTU_SIZE;
        }

        if (vpn_sap->vc_ete_hw_idx.ete_idx) {
            ohi_ent.eteptr  = vpn_sap->vc_ete_hw_idx.ete_idx;
        } else {
            return BCM_E_INTERNAL;
        }

        switch (mpls_port->criteria) {
        case BCM_MPLS_PORT_MATCH_PORT:
            /* Customer owns the complete port, the other side
             * could be pvid or pstackedvid. We let the frame
             * go raw into this port for egress.
             */
            ete.stpcheck     = 1; /* XXX: */
            ete.nostrip      = 0;

            /* p3 always pushes a vid, force the vid to be the 'always strip'
             * vid for port match
             */
            ete.usevid       = 1;
            ete.vid          = _BCM_VLAN_G3P1_UNTAGGED_VID;
            break;

        case BCM_MPLS_PORT_MATCH_PORT_VLAN:

            ete.stpcheck     = 1;
            ete.usevid       = 1;

            if (mpls_port->flags & BCM_MPLS_PORT_EGRESS_UNTAGGED) {
                ete.vid = _BCM_VLAN_G3P1_UNTAGGED_VID;
            } else {
                ete.vid  = mpls_port->match_vlan;
                ete.mtu -= 4; /* Adjust MTU for vid */
            }
            break;

        case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED not supported\n")));
            return BCM_E_UNAVAIL;
            break;

        /* Currently match label is supported only for stitching CES PW to Eth PW */
        case BCM_MPLS_PORT_MATCH_LABEL:
            {
                /* TBD: investigate if its worth to cache tunnel information on vpn sap */
                uint32 ohi = 0, fte_idx;
                _caladan3_l3_or_mpls_egress_t     mpls_tunnel_egr;
                soc_sbx_g3p1_ete_t       mpls_tunnel_hw_ete;
                soc_sbx_g3p1_oi2e_t           mpls_tunnel_ohi2etc;

                /**
                 * egress_tunnel_if is the egr object on LOCAL unit
                 * that is pointing to the OHI on exit_module.
                 */
                fte_idx = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpls_port->egress_tunnel_if);
                LOG_VERBOSE(BSL_LS_BCM_MPLS,
                            (BSL_META_U(l3_fe->fe_unit, "getting fte=0x%04x from tunnel %x\n"),
                             fte_idx, mpls_port->egress_tunnel_if));
                
                status = _bcm_caladan3_g3p1_mpls_get_fte(l3_fe, fte_idx,
                                                       L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                                       &mpls_tunnel_egr);
                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "error(%s) reading mpls-fte egress-tunnel-if(0x%x)\n"),
                               bcm_errmsg(status), mpls_port->egress_tunnel_if));
                    return status;
                }
                
                ohi  = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(SOC_SBX_OHI_FROM_ENCAP_ID(mpls_tunnel_egr.encap_id));
                
                status = soc_sbx_g3p1_oi2e_get(l3_fe->fe_unit, ohi, &mpls_tunnel_ohi2etc);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "failure(%s) reading oi2e(0x%x)\n"),
                               bcm_errmsg(status), ohi));
                    return status;
                }
                
                status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                                   mpls_tunnel_ohi2etc.eteptr,
                                                   &mpls_tunnel_hw_ete);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "failure(%s) reading encap-ete-get(0x%x)\n"),
                               bcm_errmsg(status),
                               mpls_tunnel_ohi2etc.eteptr));
                    return status;
                }
                
                /* provision l2 ete */
                sal_memcpy(&ete, &mpls_tunnel_hw_ete, sizeof(ete));
                ete.vid    = _BCM_VLAN_G3P1_UNTAGGED_VID;
                ete.usevid = 1;            
                if (mpls_port->mtu > ete.mtu) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "error mpls-port mtu greater than underlying mtu")));
                    return BCM_E_PARAM;
                }
                if (mpls_port->mtu) {
                    ete.mtu = mpls_port->mtu - (ete.encaplen - mpls_tunnel_hw_ete.encaplen);
                } else {
                    ete.mtu -= (ete.encaplen - mpls_tunnel_hw_ete.encaplen);
                }
    
                /* prepare encap ete */
                ete.dmacset   = 1;
                ete.smacset   = 1;
                ete.dmacsetlsb= 1;

                ete.dmac5 = mpls_tunnel_hw_ete.dmac5;
                ete.dmac4 = mpls_tunnel_hw_ete.dmac4;
                ete.dmac3 = mpls_tunnel_hw_ete.dmac3;
                ete.dmac2 = mpls_tunnel_hw_ete.dmac2;
                ete.dmac1 = mpls_tunnel_hw_ete.dmac1;
                ete.dmac0 = mpls_tunnel_hw_ete.dmac0;

                ete.s2       = 1;
                ete.exp2     = mpls_port->egress_label.exp;
                ete.label2   = mpls_port->egress_label.label;
                ete.etype    = 0x8847;
                ete.encaplen = 6;
                
                if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
                    ete.ttl2 = mpls_port->egress_label.ttl;
                }
                if (mpls_port->egress_label.flags & 
                                 BCM_MPLS_EGRESS_LABEL_EXP_REMARK) {
                    ete.exp2remark = 1;
                }
                if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_EXP_SET) {
                    ete.exp2 = mpls_port->egress_label.exp;
                }
            }
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "criteria(%d) illegal here\n"),
                       mpls_port->criteria));
            return BCM_E_INTERNAL;
        }

        if (modid == l3_fe->fe_my_modid || is_trunk) {
            
            if (vpn_sap->vc_ete_hw_idx.ete_idx) {
                status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                              vpn_sap->vc_ete_hw_idx.ete_idx,
                                              &ete);
                
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "error(%s) writing eteencap(0x%x)\n"),
                               bcm_errmsg(status),
                               vpn_sap->vc_ete_hw_idx.ete_idx));
                    return status;
                }
            }
        }

        if (SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id)) {
            ohi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
        } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id)) {
            ohi = SOC_SBX_OHI_FROM_L2_ENCAP_ID(mpls_port->encap_id);
        }

        if (modid == l3_fe->fe_my_modid || is_trunk) {

            if (ohi >= SBX_RAW_OHI_BASE) {
                ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(ohi);
            }

            status = soc_sbx_g3p1_oi2e_set(l3_fe->fe_unit, ohi, &ohi_ent);

            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error(%s) writing oi2e(0x%x)/(0x%x)\n"),
                           bcm_errmsg(status), ohi,
                           SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id)));

            }
        }
    }

    return status;
}

int
_bcm_caladan3_g3p1_mpls_free_vpncb(_caladan3_l3_fe_instance_t  *l3_fe,
                                   _caladan3_vpn_control_t    **vpnc)
{
    int                rv;
    soc_sbx_g3p1_v2e_t sbxVlan;
    soc_sbx_g3p1_ft_t  sbxFt;
    uint32           vrf_vsi;

    /* Extra check */
    if (!DQ_EMPTY(&(*vpnc)->vpn_sap_head)) {
        return BCM_E_BUSY;
    }

    if ((*vpnc)->vpn_id != _CALADAN3_INVALID_VPN_ID) {

        if ((*vpnc)->vpn_flags & BCM_MPLS_VPN_VPWS) {
            uint32 vsi = (*vpnc)->vpn_id;

            rv = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                        SBX_CALADAN3_USR_RES_LINE_VSI,
                                        1, &vsi, 0);
        } else {
            uint32 fte_idx;

            vrf_vsi = (*vpnc)->vpn_id;
            fte_idx = vrf_vsi + l3_fe->vlan_ft_base;

            soc_sbx_g3p1_v2e_t_init(&sbxVlan);
            rv = soc_sbx_g3p1_v2e_set(l3_fe->fe_unit, vrf_vsi, &sbxVlan);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error(%s) invalidating v2e(0x%x)\n"),
                           bcm_errmsg(rv), vrf_vsi));
            }

            /**
             * VSI <--> FTE for broadcast
             */
            soc_sbx_g3p1_ft_t_init(&sbxFt);
            rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, fte_idx, &sbxFt);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error(%s) invalidating fte(0x%x)\n"),
                           bcm_errmsg(rv), fte_idx));
            }
            /* Clear the unknown MC group fte */
            if (l3_fe->umc_ft_offset) {
                fte_idx += l3_fe->umc_ft_offset;
                rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, fte_idx, &sbxFt);
            }

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error(%s) invalidating fte(0x%x)\n"),
                           bcm_errmsg(rv), fte_idx));
            }

            rv = _sbx_caladan3_resource_free(l3_fe->fe_unit, SBX_CALADAN3_USR_RES_VSI,
                                        1, &vrf_vsi, 0);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "error(%s) freeing vsi(0x%x\n"),
                           bcm_errmsg(rv), vrf_vsi));
            }
        }

    }

    DQ_REMOVE(&(*vpnc)->vpn_fe_link);
    sal_free(*vpnc);
    *vpnc = NULL;

    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_mpls_map_set_vlan2etc(_caladan3_l3_fe_instance_t   *l3_fe,
                                         _caladan3_vpn_control_t      *vpnc,
                                         int                       program_vsi)
{
    int                rv;
    soc_sbx_g3p1_v2e_t sbxVlan;

    soc_sbx_g3p1_v2e_t_init(&sbxVlan);

    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
        if (vpnc->vpn_vrf != _CALADAN3_INVALID_VRF) {
            sbxVlan.vrf = vpnc->vpn_vrf;
        } else {
            sbxVlan.vrf = l3_fe->fe_drop_vrf;
        }
        sbxVlan.dontlearn = 1;
        sbxVlan.v4uc = 1;
        sbxVlan.v6uc = 1;
    }

    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        sbxVlan.dontlearn = 1;
        sbxVlan.v4uc = 0;
        sbxVlan.v6uc = 0;
    } else if (vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
        sbxVlan.dontlearn = 0;
        sbxVlan.v4uc = 0;
        sbxVlan.v6uc = 0;
    }
    
    rv = soc_sbx_g3p1_v2e_set(l3_fe->fe_unit, program_vsi, &sbxVlan);

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit, 
                 "vpn_flags=0x%x updating v2e[0x%x]: %s\n"), 
                 vpnc->vpn_flags, program_vsi, bcm_errmsg(rv)));

    return rv;
}



int
_bcm_caladan3_g3p1_mpls_map_set_vpn_bc_fte(_caladan3_l3_fe_instance_t  *l3_fe,
                                         _caladan3_vpn_control_t     *vpnc)
{
    int               rv;
    uint32          ftIdx;
    soc_sbx_g3p1_ft_t sbxFt;
    int unit = l3_fe->fe_unit;

    ftIdx = vpnc->vpn_id + l3_fe->vlan_ft_base;

    soc_sbx_g3p1_ft_t_init(&sbxFt);

    sbxFt.qid = SBX_MC_QID_BASE;
    sbxFt.mc = 1;
    sbxFt.oi  = vpnc->vpn_bc_mcg;

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit,
                            "Setting vsi=0x%x fte 0x%x oi=0x%x qid=0x%x (mc)\n"),
                 vpnc->vpn_id, ftIdx, sbxFt.oi, sbxFt.qid));

    /**
     * VSI <--> FTE for broadcast
     */
    rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, ftIdx, &sbxFt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "[%s] error(%s) writing fte(0x%x)\n"),
                   FUNCTION_NAME(), bcm_errmsg(rv), ftIdx));
        return rv;
    }
    /* FTE for the unknown mc group in this VSI */
    if (BCM_SUCCESS(rv)) {
        if (l3_fe->umc_ft_offset) {
            ftIdx += l3_fe->umc_ft_offset;
            rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, ftIdx, &sbxFt);
        }
    }
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "[%s] error(%s) writing fte(0x%x)\n"),
                   FUNCTION_NAME(), bcm_errmsg(rv), ftIdx));
        return rv;
    }


    return BCM_E_NONE;
}

STATIC int
_bcm_caladan3_mpls_g3p1_map_tunnel_to_vc_egress(_caladan3_l3_fe_instance_t *l3_fe,
                                                _caladan3_vpn_sap_t *vpn_sap,
                                                bcm_mpls_port_t  *mpls_port,
                                                soc_sbx_g3p1_ete_t *mpls_tunnel_hw_ete)
{
    soc_sbx_g3p1_ete_t       mpls_vc_hw_ete;
    int status = BCM_E_NONE;

    /**
     * map onto vc-ete params
     */
    soc_sbx_g3p1_ete_t_init(&mpls_vc_hw_ete);

    mpls_vc_hw_ete.mtu = mpls_tunnel_hw_ete->mtu;

    
    mpls_vc_hw_ete.vid    = _BCM_VLAN_G3P1_UNTAGGED_VID;

    if (mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_ADD) {
        mpls_vc_hw_ete.vid = mpls_port->egress_service_vlan;
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Egress Service Vlan %x added to tunnel"), 
                     mpls_port->egress_service_vlan));
    }

    mpls_vc_hw_ete.usevid = 1;
    if (mpls_port->flags & BCM_MPLS_PORT_DROP) {
        mpls_vc_hw_ete.droptagged = 1;
        mpls_vc_hw_ete.dropuntagged = 1;
    }

    /* Copy stuff from mpls-tunnel ete */
    if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
        mpls_vc_hw_ete.nosplitcheck    = 0;
        mpls_vc_hw_ete.etepid          = 1;
        /* Split horizon check with vpls color only on NW ports. */
        if((mpls_port->flags & BCM_MPLS_PORT_NETWORK)) {
            mpls_vc_hw_ete.pid             = vpn_sap->vc_vpnc->vpls_color; 
        } else {
            mpls_vc_hw_ete.pid             = 
               BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
        }
    }else{
        mpls_vc_hw_ete.nosplitcheck    = 1;
    }

    mpls_vc_hw_ete.ipttldec    = 0;
    mpls_vc_hw_ete.ttlcheck    = 0;
    if (mpls_port->egress_label.qos_map_id) {
        mpls_vc_hw_ete.remark = 
                  _MPLS_EXPMAP_HANDLE_DATA(mpls_port->egress_label.qos_map_id);
    } else {
        mpls_vc_hw_ete.remark      = mpls_tunnel_hw_ete->remark;
    }
    mpls_vc_hw_ete.tunnelenter = 1;
        
    if (!(vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_L3)) {
        mpls_vc_hw_ete.dmacset       = 1;
        mpls_vc_hw_ete.dmacsetlsb    = 1;
        mpls_vc_hw_ete.encapmac      = (mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL)?1:0;
    } else {
        mpls_vc_hw_ete.dmacset       = 0;
        mpls_vc_hw_ete.dmacsetlsb    = 0;
    }
    mpls_vc_hw_ete.dmac5 = mpls_tunnel_hw_ete->dmac5;
    mpls_vc_hw_ete.dmac4 = mpls_tunnel_hw_ete->dmac4;
    mpls_vc_hw_ete.dmac3 = mpls_tunnel_hw_ete->dmac3;
    mpls_vc_hw_ete.dmac2 = mpls_tunnel_hw_ete->dmac2;
    mpls_vc_hw_ete.dmac1 = mpls_tunnel_hw_ete->dmac1;
    mpls_vc_hw_ete.dmac0 = mpls_tunnel_hw_ete->dmac0;

    mpls_vc_hw_ete.smac5 = mpls_tunnel_hw_ete->smac5;
    mpls_vc_hw_ete.smac4 = mpls_tunnel_hw_ete->smac4;
    mpls_vc_hw_ete.smac3 = mpls_tunnel_hw_ete->smac3;
    mpls_vc_hw_ete.smac2 = mpls_tunnel_hw_ete->smac2;
    mpls_vc_hw_ete.smac1 = mpls_tunnel_hw_ete->smac1;
    mpls_vc_hw_ete.smac0 = mpls_tunnel_hw_ete->smac0;
    mpls_vc_hw_ete.smacset = mpls_tunnel_hw_ete->smacset;

    /* copy psn tunnel information into the sap */
    vpn_sap->mpls_psn_label     = mpls_tunnel_hw_ete->label2;
    vpn_sap->mpls_psn_label_exp = mpls_tunnel_hw_ete->exp2;
    vpn_sap->mpls_psn_label_ttl = mpls_tunnel_hw_ete->ttl2;

    vpn_sap->mpls_psn_label2    = mpls_tunnel_hw_ete->label1;

    /* Tagged or untagged tunnel - */
    if (mpls_tunnel_hw_ete->usevid &&
        (mpls_tunnel_hw_ete->vid < BCM_VLAN_INVALID)  &&
        (mpls_tunnel_hw_ete->vid != 0) &&
        (mpls_tunnel_hw_ete->vid != _BCM_VLAN_G3P1_UNTAGGED_VID)) {


        /* tagged tunnel */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Creating tagged vc-tunnel. TunnelVID=0x%x\n"),
                     mpls_tunnel_hw_ete->vid));

        /*  <l2 header><v-tag><label>  <label>   [vid]  <data> 
         *  |-------- tunnel -------|  |-vc--|   |-?-|   pkt
         *   ete_encap.vid, .label1    .label0   l2_ete
         *
         * Vid and label2 are overlay's in the encap ete, which is why a 
         * tagged tunnel label stats at label1, while an untagged starts 
         * at label 2
         */

        /* add the tunnel tag; the outermost tag */
        if (mpls_port->service_tpid) {
           mpls_vc_hw_ete.etype = mpls_port->service_tpid;
        } else {
           mpls_vc_hw_ete.etype = 0x8100;
        }
        mpls_vc_hw_ete.encap_vid   = mpls_tunnel_hw_ete->vid;
        mpls_vc_hw_ete.pricfi      = 0;

        /* Copy label stack from mpls-tunnel ete into vc-ete */
        mpls_vc_hw_ete.ttl1        =  mpls_tunnel_hw_ete->ttl2;
        mpls_vc_hw_ete.s1          =  0;
        mpls_vc_hw_ete.exp1remark  =  mpls_tunnel_hw_ete->exp2remark;
        mpls_vc_hw_ete.exp1        =  mpls_tunnel_hw_ete->exp2;
        mpls_vc_hw_ete.label1      =  mpls_tunnel_hw_ete->label2;
        mpls_vc_hw_ete.etype2      =  0x8847;

        /* Add the vc label; the inner most label */
        mpls_vc_hw_ete.s0     = 1;
        mpls_vc_hw_ete.label0 = mpls_port->egress_label.label;
            
        if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            mpls_vc_hw_ete.ttl0 = mpls_port->egress_label.ttl;
        }
        if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_EXP_SET) {
            mpls_vc_hw_ete.exp0 = mpls_port->egress_label.exp;
        } else {
            mpls_vc_hw_ete.exp0remark = 1;
        }

        if(mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) {
            mpls_vc_hw_ete.encaplen    =  mpls_tunnel_hw_ete->encaplen + 4 + 4 + 12;
        
            /* MPLS-TP PW - mandatory CW needs to be present */
            /* Add control word only for ethernet to PW traffic, not for 
             * CES PW handoff since ACH is retained from ingress*/
            if (mpls_port->flags & BCM_MPLS_PORT_CONTROL_WORD) {
                mpls_vc_hw_ete.add_pwcw = 1;
                mpls_vc_hw_ete.encaplen   += 4;
            }
        } else {
            mpls_vc_hw_ete.encaplen    =  mpls_tunnel_hw_ete->encaplen + 4 + 4;
        }
    } else {

        /* untagged tunnel */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Creating untagged vc-tunnel\n")));

        /*  <l2 header><label>  <label>   [vid]  <data> 
         *  |--- tunnel -----|  |-vc--|   |-?-|   pkt
         *   ete_encap.label2   .label1   l2_ete
         */

        /* Copy label stack from mpls-tunnel ete into vc-ete */
        mpls_vc_hw_ete.ttl2        =  mpls_tunnel_hw_ete->ttl2;
        mpls_vc_hw_ete.s2          =  0;
        mpls_vc_hw_ete.exp2remark  =  mpls_tunnel_hw_ete->exp2remark;
        mpls_vc_hw_ete.exp2        =  mpls_tunnel_hw_ete->exp2;
        mpls_vc_hw_ete.label2      =  mpls_tunnel_hw_ete->label2;
        mpls_vc_hw_ete.etype       =  0x8847;

        /* If the TUNNEL has two labels */
        if (mpls_tunnel_hw_ete->label1) {
            mpls_vc_hw_ete.s1          = 0;
            mpls_vc_hw_ete.label1      = mpls_tunnel_hw_ete->label1;
            mpls_vc_hw_ete.ttl1        =  mpls_tunnel_hw_ete->ttl1;
            mpls_vc_hw_ete.exp1remark  =  mpls_tunnel_hw_ete->exp1remark;
            mpls_vc_hw_ete.exp1        =  mpls_tunnel_hw_ete->exp1;

            /* inner most label is vc label */
            mpls_vc_hw_ete.s0     = 1;
            mpls_vc_hw_ete.label0 = mpls_port->egress_label.label;
            
            if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
                mpls_vc_hw_ete.ttl0 = mpls_port->egress_label.ttl;
            }
            if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_EXP_SET) {
                mpls_vc_hw_ete.exp0 = mpls_port->egress_label.exp;
            } else {
                mpls_vc_hw_ete.exp0remark = 1;
            }
        } else {
            /* inner most label is vc label */
            mpls_vc_hw_ete.s1     = 1;
            mpls_vc_hw_ete.label1 = mpls_port->egress_label.label;
            
            if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
                mpls_vc_hw_ete.ttl1 = mpls_port->egress_label.ttl;
            }
            if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_EXP_SET) {
                mpls_vc_hw_ete.exp1 = mpls_port->egress_label.exp;
            } else {
                mpls_vc_hw_ete.exp1remark = 1;
            }
        }

        if(mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) {
            mpls_vc_hw_ete.encaplen    =  mpls_tunnel_hw_ete->encaplen + 4 + 12;
        
            /* MPLS-TP PW - mandatory CW needs to be present */
            /* Add control word only for ethernet to PW traffic, not for 
             * CES PW handoff since ACH is retained from ingress*/
            if (mpls_port->flags & BCM_MPLS_PORT_CONTROL_WORD) {
                mpls_vc_hw_ete.add_pwcw = 1;
                mpls_vc_hw_ete.encaplen   += 4;
            }
        } else {
            mpls_vc_hw_ete.encaplen    =  mpls_tunnel_hw_ete->encaplen + 4;
        }            
    } /* tagged or untagged tunnel */

    /* Encaplen during header compression */
    /* comp_encaplen can be calculated in ucode.
       This field is added in ete to save ucode instruction.
     */
    if (mpls_vc_hw_ete.encapmac) {
        /* If header compression enabled, knock off mac & etype */
        mpls_vc_hw_ete.comp_encaplen = mpls_vc_hw_ete.encaplen - 14;
    } else {
        /* If header compression enabled, knock off etype */
        mpls_vc_hw_ete.comp_encaplen = mpls_vc_hw_ete.encaplen - 2;
    }

    if (mpls_port->mtu > mpls_vc_hw_ete.mtu) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error mpls-port mtu greater than underlying tunnel mtu")));
        return BCM_E_PARAM;
    }
    if (mpls_port->mtu) {
        mpls_vc_hw_ete.mtu = mpls_port->mtu - (mpls_vc_hw_ete.encaplen - mpls_tunnel_hw_ete->encaplen);
    } else {
        mpls_vc_hw_ete.mtu -= (mpls_vc_hw_ete.encaplen - mpls_tunnel_hw_ete->encaplen);
    }
    status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                       vpn_sap->vc_ete_hw_idx.ete_idx,
                                       &mpls_vc_hw_ete);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) writing mpls-vc-ete(0x%x)"),
                   bcm_errmsg(status), vpn_sap->vc_ete_hw_idx.ete_idx));
        return status;
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_mpls_update_vpxs_tunnel_hw(_caladan3_l3_fe_instance_t  *l3_fe,
                                              _caladan3_vpn_sap_t         *vpn_sap,
                                              bcm_mpls_port_t         *mpls_port)
{
    int                           status = 0;
    int                           numlabels;
    uint32                        ohi = 0, fte_idx;
    _caladan3_l3_or_mpls_egress_t     mpls_tunnel_egr;
    soc_sbx_g3p1_ete_t       mpls_vc_hw_ete;
    soc_sbx_g3p1_oi2e_t           mpls_vc_ohi2etc;

    soc_sbx_g3p1_ete_t       mpls_tunnel_hw_ete;
    soc_sbx_g3p1_oi2e_t           mpls_tunnel_ohi2etc;
    int8                        is_trunk = 0;
    _caladan3_l3_ete_t           *mpls_tunnel_sw_ete;

    
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit, "Enter\n")));

    vpn_sap->vc_res_alloced = 0;

    /**
     * This routine programs the P side for VPLS and VPWS
     * i.e. the vc_label over tunnel.
     */

    /**
     * egress_tunnel_if is the egr object on LOCAL unit
     * that is pointing to the OHI on exit_module.
     */
    fte_idx = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpls_port->egress_tunnel_if);

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit, "getting fte=0x%04x from tunnel %x\n"),
                 fte_idx, mpls_port->egress_tunnel_if));

    status = _bcm_caladan3_g3p1_mpls_get_fte(l3_fe, fte_idx,
                                           L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                           &mpls_tunnel_egr);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) reading mpls-fte egress-tunnel-if(0x%x)\n"),
                   bcm_errmsg(status), mpls_port->egress_tunnel_if));
        return status;
    }

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        is_trunk = 1;
    }


    /**
     * vc exit_modid should be the same as the tunnel exit_modid
     */
    if (l3_fe->fe_my_modid != mpls_tunnel_egr.fte_modid && (is_trunk == 0)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) mpls-port-modid(0x%x) "
                   "mpls_tunnel_egr.module(0x%x) for mpls-tunnel(0x%x)\n"),
                   bcm_errmsg(BCM_E_PARAM),
                   l3_fe->fe_my_modid, mpls_tunnel_egr.fte_modid,
                   mpls_port->egress_tunnel_if));
        status = BCM_E_PARAM;
        return status;
    }

    if (vpn_sap->vc_ohi.ohi == _CALADAN3_INVALID_OHI) {
        status = _bcm_caladan3_mpls_alloc_vpn_sap_hw_resources(l3_fe,
                                                             vpn_sap,
                                                             mpls_port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) allocating vpn-sap resources\n"),
                       bcm_errmsg(status)));
            return status;
        }

        vpn_sap->vc_res_alloced = 1;
    }

    /*
     * Get mpls-tunnel-ohi --> mpls-tunnel-ete
     * --> mpls-tunnel-l2-ete
     */
    ohi  = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(SOC_SBX_OHI_FROM_ENCAP_ID(mpls_tunnel_egr.encap_id));

    status = soc_sbx_g3p1_oi2e_get(l3_fe->fe_unit, ohi, &mpls_tunnel_ohi2etc);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "failure(%s) reading oi2e(0x%x)\n"),
                   bcm_errmsg(status), ohi));
        return status;
    }

    status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                       mpls_tunnel_ohi2etc.eteptr,
                                       &mpls_tunnel_hw_ete);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "failure(%s) reading encap-ete-get(0x%x)\n"),
                   bcm_errmsg(status),
                   mpls_tunnel_ohi2etc.eteptr));
        return status;
    }
    /* link MPLS Label ports to the tunnel interface */
    /*mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) */
    if (!(mpls_port->flags & BCM_MPLS_PORT_REPLACE) && 
         ((mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) || 
         (mpls_port->flags & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP)) && 
        _CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, mpls_port->egress_tunnel_if)) {
            status = _bcm_caladan3_get_l3_ete_context_by_index(l3_fe,
                                                               mpls_tunnel_ohi2etc.eteptr,
                                                               &mpls_tunnel_sw_ete);
            if (status == BCM_E_NONE) {
                DQ_INSERT_HEAD(&mpls_tunnel_sw_ete->l3_vc_ete_head, &vpn_sap->vc_mpls_ete_link);
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "Linked vc ete to tunnel sw ete \n")));
            } else {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "failure(%s) getting tunnel sw ete from eteptr 0x%x\n"),
                           bcm_errmsg(status),
                           mpls_tunnel_ohi2etc.eteptr));
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, "failed to link vc to tunnel \n")));
            }
    }

    
    numlabels = 2;
    if (numlabels >= _CALADAN3_MAX_MPLS_TNNL_LABELS) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "mpls-tunnel-ete already has %d labels\n"),
                   numlabels));
        status = BCM_E_INTERNAL;
        return status;
    }

    status = _bcm_caladan3_mpls_g3p1_map_tunnel_to_vc_egress(l3_fe, vpn_sap,
                                                           mpls_port,
                                                             &mpls_tunnel_hw_ete);
    if (status != BCM_E_NONE) {
        return status;
    }

    /**
     * Write the ohi2etc for vc-etc
     */
    soc_sbx_g3p1_oi2e_t_init(&mpls_vc_ohi2etc);
    mpls_vc_ohi2etc.eteptr   = vpn_sap->vc_ete_hw_idx.ete_idx;
    mpls_vc_ohi2etc.counter  = 0;

    ohi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
        
    if (ohi >= SBX_RAW_OHI_BASE) {
        ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(ohi);
    }

    status = soc_sbx_g3p1_oi2e_set(l3_fe->fe_unit, ohi, &mpls_vc_ohi2etc);
    if (BCM_FAILURE(status)) {
        int tmp;
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) writing ohi2etc(0x%x)\n"),
                   bcm_errmsg(status),
                   _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id))));

        soc_sbx_g3p1_ete_t_init(&mpls_vc_hw_ete);
        tmp = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                        vpn_sap->vc_ete_hw_idx.ete_idx,
                                        &mpls_vc_hw_ete);
        if (BCM_FAILURE(tmp)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) setting invalidated mpls-vc-ete(0x%x)"),
                       bcm_errmsg(tmp),
                       vpn_sap->vc_ete_hw_idx.ete_idx));
        }

    }

    return status;
}


int
_bcm_caladan3_g3p1_mpls_invalidate_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t *l3_fe,
                                                        _caladan3_vpn_sap_t    *vpn_sap)
{
    int                 rv;
    soc_sbx_g3p1_oi2e_t oh_ent;
    soc_sbx_g3p1_ete_t  mpls_vc_ete_encap;
    uint32              ohi;

    /* only free the OHI & ETEs if mpls_port allocated them */
    if (vpn_sap->vc_res_alloced == 0) {
        return BCM_E_NONE;
    }

    ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
    rv = soc_sbx_g3p1_oi2e_get(l3_fe->fe_unit, ohi, &oh_ent);

    if (BCM_FAILURE(rv)) {
        goto free_resources;
    }

    if (oh_ent.eteptr != l3_fe->fe_raw_ete_idx) {
        if (vpn_sap->vc_ete_hw_idx.ete_idx) {
            soc_sbx_g3p1_ete_t_init(&mpls_vc_ete_encap);
            rv = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                           vpn_sap->vc_ete_hw_idx.ete_idx,
                                           &mpls_vc_ete_encap);
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(l3_fe->fe_unit, "invalidate eteencap=0x%x\n"),
                         vpn_sap->vc_ete_hw_idx.ete_idx));
            if (BCM_FAILURE(rv)) {
                goto free_resources;
            }
        }
    }

    /*
     * In both types of ete (i.e. mpls vc and L2, we need to nuke the
     * Ohi2etc
     */
    soc_sbx_g3p1_oi2e_t_init(&oh_ent);
    rv = soc_sbx_g3p1_oi2e_set(l3_fe->fe_unit, ohi, &oh_ent);

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit, "invalidate ohi=0x%x(0x%x)\n"), 
                 ohi, vpn_sap->vc_ohi.ohi));
    if (BCM_FAILURE(rv)) {
        goto free_resources;
    }

    rv = BCM_E_NONE;

free_resources:
    _bcm_caladan3_mpls_free_vpn_sap_hw_resources(l3_fe, vpn_sap);
    return rv;
}


int
_bcm_caladan3_g3p1_mpls_map_set_mpls_vpn_fte(_caladan3_l3_fe_instance_t  *l3_fe,
                                             _caladan3_vpn_control_t     *vpnc,
                                             _caladan3_vpn_sap_t         *vpn_sap,
                                             bcm_mpls_port_t         *mpls_port)
{
    int                           status;
    uint32                        fte_idx;
    int                           modid, port;
    int                           fab_node, fab_unit, fab_port;
    soc_sbx_g3p1_ft_t             ft_ent;
    
    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);

    soc_sbx_g3p1_ft_t_init(&ft_ent);
    
    if (SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id)) {
        ft_ent.oi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
    } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id)) {
        ft_ent.oi = SOC_SBX_OHI_FROM_L2_ENCAP_ID(mpls_port->encap_id);
    }

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        ft_ent.lag = 1;
        ft_ent.lagbase = TRUNK_INDEX_SET(BCM_GPORT_TRUNK_GET(mpls_port->port), 0);
        ft_ent.lagsize = TRUNK_SBX_HASH_SIZE;

    } else {
        /* common to all mpls port types - */
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     vpn_sap->vc_mpls_port.port,
                                                     &modid, &port);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) extracting mod-port from gport\n"),
                       bcm_errmsg(status)));
            return status;
        }

        /* convert to QE node, port */
        BCM_IF_ERROR_RETURN(soc_sbx_node_port_get(l3_fe->fe_unit, modid, port,
                                                  &fab_unit, &fab_node, &fab_port));
        ft_ent.qid = SOC_SBX_NODE_PORT_TO_QID(l3_fe->fe_unit,
                                              fab_node,
                                              fab_port,
                                              l3_fe->fe_cosq_config_numcos);
    }
    
    /* GNATS 31668 adding rridx */
    /* failover id 0 with valid failover port id
     * could be used to update the port in non ingress modules to save
     * failover id consumption */
    if (mpls_port->failover_id || mpls_port->failover_port_id) {
        uint32               prot_fte_idx;
        soc_sbx_g3p1_ft_t    prot_ft_ent;
        
        prot_fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->failover_port_id);
        
        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_ft_get(l3_fe->fe_unit, prot_fte_idx, &prot_ft_ent));
        
        ft_ent.oib = prot_ft_ent.oi;
        ft_ent.qidb = prot_ft_ent.qid;
        ft_ent.rridx = mpls_port->failover_id;
        _bcm_caladan3_g3p1_mpls_failover_update(l3_fe,
                                              vpnc,
                                              vpn_sap,
                                              mpls_port, 0);
    }
    if (mpls_port->pw_failover_port_id) {
        /* If VPLS, pw_failover_id is derived from primary_pw_id,
         *          ft is already populated.
         * If VPWS, pw_failover_id is derived from AC gport id,
         *          new ft needs to be populated. If AC is not created,
         *          then populating the new ft needs to be done,
         *          when AC is created.
         */
        _bcm_caladan3_g3p1_mpls_failover_update(l3_fe,
                                                vpnc,
                                                vpn_sap,
                                                mpls_port, 1);
    }

    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);

    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit, "writing fte_idx=0x%04x\n"), fte_idx));

    if (fte_idx == -1) {
        return BCM_E_INTERNAL;
    }

    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        /* Set the exception id in ft entry for VPWS
           and clear it when cross connecting two ports */
        ft_ent.excidx = 0x7f;
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "setting ft(%d).excidx to 0x7f\n"),
                     fte_idx));
    }

    status = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, fte_idx, &ft_ent);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) writing fte(0x%x)\n"),
                   bcm_errmsg(status), fte_idx));
        return status;
    }

    return status;

}

STATIC int
_bcm_caladan3_g3p1_mpls_failover_lp_update(int unit,
                                           uint32 logicalPort,
                                           int pid)
{
    int status;
    soc_sbx_g3p1_lp_t hw_logicalPort;

    soc_sbx_g3p1_lp_t_init(&hw_logicalPort);
    status = soc_sbx_g3p1_lp_get(unit, logicalPort, &hw_logicalPort);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to get lp 0x%x: %d (%s)\n"),
                   logicalPort, status, bcm_errmsg(status)));
        return status;
    }
    hw_logicalPort.pid = pid;
    status = soc_sbx_g3p1_lp_set(unit, logicalPort, &hw_logicalPort);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to set lp 0x%x: %d (%s)\n"),
                   logicalPort, status, bcm_errmsg(status)));
        return status;
    }
    return BCM_E_NONE;
}
                                            

int
_bcm_caladan3_g3p1_mpls_failover_update(_caladan3_l3_fe_instance_t *l3_fe,
                                        _caladan3_vpn_control_t *vpnc,
                                        _caladan3_vpn_sap_t *vpn_sap,
                                        bcm_mpls_port_t *mpls_port,
                                        uint8  is_pw_failover)
{
    int                     status = BCM_E_NONE;
    _caladan3_vpn_sap_t        *fo_vpn_sap;
    int                     pid, modid = -1;
    soc_sbx_g3p1_ete_t hw_ete;
    soc_sbx_g3p1_pv2e_t     pv2e;
    uint32                fte, lport;
    bcm_port_t              port;
    bcm_vlan_t              vid;
    bcm_trunk_t             trunkid;
    bcm_trunk_add_info_t   *trunk_info = NULL;
    uint16                 index, is_trunk = 0, num_ports = 0;
    bcm_gport_t             failover_port_id;

    if (is_pw_failover) {
        failover_port_id = mpls_port->pw_failover_port_id;
    } else {
        failover_port_id = mpls_port->failover_port_id;
    }
    status = _bcm_caladan3_find_vpn_sap_by_id(l3_fe,
                                   vpnc,
                                   failover_port_id,
                                  &fo_vpn_sap);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                   "error(%s) finding vpn sap for failover gport (0x%x)\n"),
                   bcm_errmsg(status), failover_port_id));
        return status;
    }

    if (BCM_GPORT_IS_TRUNK(fo_vpn_sap->vc_mpls_port.port)) {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(fo_vpn_sap->vc_mpls_port.port);
        trunk_info = 
                 &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
        for (index = 0; index < num_ports; index++) {
            if (trunk_info->tm[index] == l3_fe->fe_my_modid) {
                modid = trunk_info->tm[index];
            }
        }
    } else {
        status = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                         fo_vpn_sap->vc_mpls_port.port,
                                         &modid,
                                         &port);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) extracting mod-port from gport (0x%x)\n"),
                       bcm_errmsg(status), fo_vpn_sap->vc_mpls_port.port));
            return status;
        }
        num_ports = 1;
    }

    if (l3_fe->fe_my_modid != modid) {
        return status;
    }
    if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
        /* Set the pid for the VPLS failover port.
           The failover port uses primary ports pid as the failover port's fte
           is allocated in non pid range (> 16K : in dynamic fte)
         */
        pid = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);

        if (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL) {
            status = _bcm_caladan3_g3p1_mpls_failover_lp_update(l3_fe->fe_unit,
                                                     fo_vpn_sap->logicalPort,
                                                     pid);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                         (BSL_META_U(l3_fe->fe_unit, "error(%s) mpls failover lp update lpi(0x%x),pid(0x%x)"),
                          bcm_errmsg(status),
                          fo_vpn_sap->logicalPort,pid));
            }
        }


        status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                       fo_vpn_sap->vc_ete_hw_idx.ete_idx,
                                       &hw_ete);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) getting failover mpls vc ete(0x%x)"),
                       bcm_errmsg(status),
                       fo_vpn_sap->vc_ete_hw_idx.ete_idx));
        }



        hw_ete.nosplitcheck = 0;
        hw_ete.etepid = 1;
        if ((mpls_port->flags & BCM_MPLS_PORT_NETWORK)) {
            hw_ete.pid = vpn_sap->vc_vpnc->vpls_color;
        } else {
            hw_ete.pid = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
        }

        status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                       fo_vpn_sap->vc_ete_hw_idx.ete_idx,
                                       &hw_ete);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) setting failover mpls vc ete(0x%x)"),
                       bcm_errmsg(status),
                       fo_vpn_sap->vc_ete_hw_idx.ete_idx));
        }
    }
    if ((vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) 
           && (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT ||
               mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN)) {

        for (index = 0; index < num_ports; index++) {
            if ( is_trunk == 1) {
                port = trunk_info->tp[index];
                if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                    continue;
                }
            }

            if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT) {

                status = bcm_port_untagged_vlan_get(l3_fe->fe_unit,
                                                    port, &vid);

                if (BCM_E_NONE != status) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit,
                               "error(%s) reading nativeVid for port\
                               %d\n"),bcm_errmsg(status), port));

                    return status;

                }
            } else {
                vid = fo_vpn_sap->vc_mpls_port.match_vlan;
            }
            soc_sbx_g3p1_pv2e_t_init(&pv2e);
            status = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit,
                                           port, vid, &pv2e);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) reading PVid2Etc for port\
                           %d vid %d\n"),bcm_errmsg(status),
                           port, vid));
                return status;
            }

            lport = fo_vpn_sap->logicalPort;
            status = _bcm_caladan3_g3p1_mplstp_vpws_ft_lp_offset(l3_fe->fe_unit,
                                                      mpls_port->mpls_port_id,
                                                     &lport, &fte);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) on getting ft_lp offset\
                           (port_id 0x%x, lp 0x%x)\n"),\
                           bcm_errmsg(status),
                           mpls_port->mpls_port_id, lport));
                return status;
            }

            pv2e.vlan = fte;
            pv2e.lpi = lport;
            pv2e.vpws = 1;
            status = _soc_sbx_g3p1_pv2e_set(l3_fe->fe_unit, port,
                                            vid, &pv2e);

            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) writing pv2e(0x%x,0x%x)\n"),\
                           bcm_errmsg(status), port, vid));
                return status;
            }

        }

    }

    if ((vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && is_pw_failover) {
        status = _bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(l3_fe, vpnc, vpn_sap,
                                                        mpls_port, 1);
    }
    return status;
}

/*
 * Function:
 *   _bcm_caladan3_g3p1_mpls_vpws_port_failover_update
 * Purpose:
 *     i. Copy details from failover fte and update primary fte
 *    ii. Update failover id in primary fte
 *
 */
int 
_bcm_caladan3_g3p1_mpls_vpws_port_failover_update(int          unit,
                                              uint32       fte_idx_to_update,
                                              uint32       fte_idx_fo,
                                              bcm_failover_t failover_id)
{
    int               status;
    soc_sbx_g3p1_ft_t fte_to_update, fte_fo;
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "fte(update):0x%x fte(fo):0x%x fo_id:0x%x\n"),
               fte_idx_to_update, fte_idx_fo, failover_id));
    soc_sbx_g3p1_ft_t_init(&fte_to_update);
    soc_sbx_g3p1_ft_t_init(&fte_fo);

    status = soc_sbx_g3p1_ft_get(unit, fte_idx_to_update, &fte_to_update);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Could not read fte :0x%x \n"),
                   fte_idx_to_update));
        return status;
    }

    /* if failover_id == 0, then it is failover removal case
       as ft is not read (when failover_id = 0),
       it will use initialized ft to update the primary ft */
    if (failover_id != 0) {
        status = soc_sbx_g3p1_ft_get(unit, fte_idx_fo, &fte_fo);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Could not read fte :0x%x \n"),
                       fte_idx_fo));
            return status;
        }
    }

    fte_to_update.rridx      = failover_id;
    fte_to_update.oib        = fte_fo.oi;
    fte_to_update.qidb       = fte_fo.qid;
    fte_to_update.lagbaseb   = fte_fo.lagbase;
    fte_to_update.lagsizeb   = fte_fo.lagsize;
    fte_to_update.lagb       = fte_fo.lag;

    status = soc_sbx_g3p1_ft_set(unit, fte_idx_to_update, &fte_to_update);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Could not set fte :0x%x \n"),
                   fte_idx_to_update));
        return status;
    } else {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "successfully updated fte with failover\n")));
    }
    return status;
}

int
_bcm_caladan3_g3p1_mpls_vpws_pw_fo_connect(_caladan3_l3_fe_instance_t *l3_fe,
                                          _caladan3_vpn_control_t    *vpnc,
                                          _caladan3_vpn_sap_t        *vpn_sap,
                                          bcm_mpls_port_t        *mpls_pw_port,
                                          uint8                 connect)
{
    _caladan3_vpn_sap_t  *vpws_uni_vpn_sap, *mpls_pw_sap;
    uint32          vpws_uni_fti, fo_fti, vpws_uni_ft_offset = 0;
    uint32          vpws_pw_fo_ft_base = 0, vpws_pw_fo_ft, vpls_pw_fo_ft_base=0;
    uint32          fo_size = 0;
    bcm_failover_t    pw_fo_id;
    bcm_gport_t       pw_fo_port_id;
    soc_sbx_g3p1_ft_t vpws_pw_fo_fte;
    int               status = BCM_E_NONE;
    
    if (mpls_pw_port == NULL) {
        /* mpls_pw_port is NULL, this denotes this function is called 
         *  during AC port creation.
         */
        vpws_uni_vpn_sap = vpn_sap;
        status = _bcm_caladan3_mpls_vpws_port_sap_get(vpnc,
                                             _BCM_CALADAN3_MPLS_VPWS_NW_PORT,
                                             &mpls_pw_sap);
        pw_fo_port_id = mpls_pw_sap->vc_mpls_port.pw_failover_port_id;
    } else {
        /* mpls_pw_port is not NULL, this denotes this function is called 
         *  during updation of PW port with PW Failover port.
         */
        mpls_pw_sap = vpn_sap;
        status = _bcm_caladan3_mpls_vpws_port_sap_get(vpnc,
                                             _BCM_CALADAN3_MPLS_VPWS_UNI_PORT,
                                             &vpws_uni_vpn_sap);
        pw_fo_port_id = mpls_pw_port->pw_failover_port_id;

    }

    if (status == BCM_E_NONE) {
        vpws_uni_fti = BCM_GPORT_MPLS_PORT_ID_GET(
                           vpws_uni_vpn_sap->vc_mpls_port_id);
        soc_sbx_g2p3_redundant_vpws_pw_ft_base_get(l3_fe->fe_unit, 
                                                   &vpws_pw_fo_ft_base);
        soc_sbx_g2p3_redundant_vpls_pw_ft_base_get(l3_fe->fe_unit, 
                                                   &vpls_pw_fo_ft_base);
        soc_sbx_g2p3_vpws_ft_offset_get(l3_fe->fe_unit, &vpws_uni_ft_offset);
        vpws_pw_fo_ft = vpws_pw_fo_ft_base + 
                         (vpws_uni_fti - vpws_uni_ft_offset);
        fo_fti        = BCM_GPORT_MPLS_PORT_ID_GET(pw_fo_port_id);
        soc_sbx_g3p1_ft_t_init(&vpws_pw_fo_fte);

        if (connect) {
            soc_sbx_g3p1_ft_t_init(&vpws_pw_fo_fte);
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_get(l3_fe->fe_unit, vpws_uni_fti,  &vpws_pw_fo_fte));
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_set(l3_fe->fe_unit, vpws_pw_fo_ft, &vpws_pw_fo_fte));
            fo_size = soc_sbx_g3p1_rrt_table_size_get(l3_fe->fe_unit);
            pw_fo_id = fo_size + (vpws_pw_fo_ft_base - vpls_pw_fo_ft_base)
                                     + (vpws_uni_fti - vpws_uni_ft_offset);
            if (mpls_pw_port != NULL) {
                mpls_pw_port->pw_failover_id = pw_fo_id;
            } else {
                mpls_pw_sap->vc_mpls_port.pw_failover_id = pw_fo_id;
            }
            _bcm_caladan3_g3p1_mpls_vpws_fte_connect(l3_fe, vpws_pw_fo_ft, fo_fti, 1);
        } else {
            _bcm_caladan3_g3p1_mpls_vpws_fte_connect(l3_fe, vpws_pw_fo_ft, fo_fti, 0);
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_set(l3_fe->fe_unit, vpws_pw_fo_ft, &vpws_pw_fo_fte));
        }
    } 
    return status;
}
                                     
int
_bcm_caladan3_g3p1_mpls_lp_write(int unit, uint32 logicalPort,
                               _caladan3_vpn_sap_t   *vpn_sap,
                               bcm_mpls_port_t   *mpls_port,
                               int                action)
{
    soc_sbx_g3p1_lp_t  hw_logicalPort;
    int                status, pid, modid;
    uint32           trunkid;
    int                fab_unit, fab_node, fab_port;
    bcm_port_t         port;

    soc_sbx_g3p1_lp_t_init(&hw_logicalPort);
    if (action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) {
        status = soc_sbx_g3p1_lp_get(unit, logicalPort, &hw_logicalPort);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to get lp 0x%x: %d %s\n"),
                       logicalPort, status, bcm_errmsg(status)));
            return status;
        }
    }

    status = _bcm_caladan3_g3p1_policer_lp_program(unit, mpls_port->policer_id,
                                                   &hw_logicalPort);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to program lp 0x%x policer 0x%x"
                               ": %d (%s)\n"),
                   logicalPort, mpls_port->policer_id,
                   status, bcm_errmsg(status)));
        return status;
    }

    /* VPLS is always in logical model; use the FTE as the SID */
    if (vpn_sap->vc_vpnc->vpn_flags & BCM_MPLS_VPN_VPLS) {
        /* Program color for only the network facing tunnel port */
        if((mpls_port->flags & BCM_MPLS_PORT_NETWORK)) {
            hw_logicalPort.usecolor = 1;
            hw_logicalPort.color = vpn_sap->vc_vpnc->vpls_color;
        } 
 
        pid = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
        if ((mpls_port->flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {
            pid = 0;
        }
    } else {
        if(BCM_GPORT_IS_TRUNK(mpls_port->port)) {
            trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
            pid = SOC_SBX_TRUNK_TO_SID(unit, trunkid);
            
        } else {

            BCM_IF_ERROR_RETURN
                (_bcm_caladan3_mpls_gport_get_mod_port(unit, 
                                                     mpls_port->port,
                                                     &modid, &port));

            BCM_IF_ERROR_RETURN
                (soc_sbx_node_port_get(unit, modid, port, &fab_unit,
                                       &fab_node, &fab_port));

            pid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
       } 
    }

    hw_logicalPort.pid = pid;
    hw_logicalPort.qos = _MPLS_EXPMAP_HANDLE_DATA(mpls_port->exp_map);
    if ((mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP) ||
        (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET)) {
        hw_logicalPort.useexp = 1;
    }

    status = soc_sbx_g3p1_lp_set(unit, logicalPort, &hw_logicalPort);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to set lp 0x%x: %d (%s)\n"),
                   logicalPort, status, bcm_errmsg(status)));
        return status;
    }
    
    return status;
}

#if 0
/* This function is not used */
int
_bcm_caladan3_g3p1_mpls_lp_get(int unit, bcm_mpls_port_t *mpls_port,
                             uint32 *lp)
{
    soc_sbx_g3p1_pv2e_t      hw_portVid;
    soc_sbx_g3p1_pvv2edata_t hw_pvv;
    soc_sbx_g3p1_labels_t    hw_label2e;
    int                      rv = BCM_E_UNAVAIL;
    bcm_module_t             modid;
    bcm_port_t               port;
    bcm_vlan_t               vlan;
    int                      vid;
    int                      label1 = 0, label2 = 0, label3 = 0;

    rv = _bcm_caladan3_mpls_gport_get_mod_port(unit, mpls_port->port,
                                             &modid, &port);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                               "error %d (%s) extracting mod-port from gport\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    vid =  mpls_port->match_vlan;

    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        rv = bcm_port_untagged_vlan_get(unit, port, &vlan);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) failed to read native vid for port %d\n"),
                       bcm_errmsg(rv), port));
            return rv;
        }
        vid = vlan;
        /* fall thru intentional */
    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &hw_portVid);
        *lp = hw_portVid.lpi;
        break;
    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        rv = soc_sbx_g3p1_util_pvv2e_get
            (unit, port, mpls_port->match_vlan, mpls_port->match_inner_vlan, &hw_pvv);
        *lp = hw_pvv.lpi;
        break;
    case BCM_MPLS_PORT_MATCH_LABEL:
#ifndef PLATFORM_LABELS
        rv = _bcm_caladan3_mpls_find_vpn_sap_by_gport(l3_fe, 
                                                          mpls_port->mpls_port_id,
                                                         &vpn_sap);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to find vpn sap for gport: "
                                  "unit %d port %08X\n"), unit, port));
            return status;
        }
        
        label1 = vpn_sap->mpls_psn_label;
        label2 = mpls_port->match_label;
#else
        port = 0;
        label1 = mpls_port->match_label;
#endif
        rv = _bcm_caladan3_g3p1_mpls_labels_get(unit, port,
                               label1, label2, label3,
                               &hw_label2e); 
        *lp = hw_label2e.lpi;
        break;
    default:
        rv = BCM_E_NOT_FOUND;
        break;
    }

    return rv;
}
#endif


int 
_bcm_caladan3_g3p1_mplstp_vpws_ft_lp_offset(int         unit,
                                             bcm_gport_t gport,
                                             uint32   *lport,
                                             uint32   *fte)
{
    int status = BCM_E_NONE;

    if (!lport || !fte || 
        !BCM_GPORT_IS_MPLS_PORT(gport)) {
        status = BCM_E_PARAM;
    } else {
        uint32 vpws_uni_ft_offset = 0;
        
        status = soc_sbx_g3p1_vpws_ft_offset_get(unit, &vpws_uni_ft_offset);
        if (status == BCM_E_NONE) {
            /* set uni ft into pv2e.vlan. Covert it to 0 based offset so range
             * falls between 0 - 16k */
            *fte = BCM_GPORT_MPLS_PORT_ID_GET(gport) - vpws_uni_ft_offset;
            if (*fte >= _BCM_CALADAN3_VPWS_UNI_OFFSET) {
                status = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "error(%s) Bad internal vpws uni offset FT[%x]\n"),
                           bcm_errmsg(status), vpws_uni_ft_offset));                    
            } else {
                *lport -= _BCM_CALADAN3_VPWS_UNI_OFFSET;
            }
        }
    }
    return status;
}

static int 
_bcm_caladan3_g3p1_mplstp_vpws_offset(int unit, 
                                    _caladan3_vpn_control_t *vpnc,
                                    _caladan3_vpn_sap_t     *vpn_sap,
                                    uint32            *lport,
                                    uint32            *fte)
{
    int status = BCM_E_NONE;

    if (!lport || !fte || !vpnc || !vpn_sap) {
        status = BCM_E_PARAM;
    } else {
        if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) &&
            (!(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_NETWORK))) {

            status = _bcm_caladan3_g3p1_mplstp_vpws_ft_lp_offset(unit, 
                                                               vpn_sap->vc_mpls_port_id, 
                                                               lport, fte);
        } else {
            status = BCM_E_PARAM;
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error(%s) vpws offset adjust applicable only for vpws uni ports\n"),
                       bcm_errmsg(status)));
        }
    }

    return status;
}

int
_bcm_caladan3_g3p1_mpls_match_port2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                                       int                      action,
                                       int                      logicalPort,
                                       _caladan3_vpn_control_t     *vpnc,
                                       _caladan3_vpn_sap_t         *vpn_sap,
                                       bcm_mpls_port_t         *mpls_port)
{
    int                           rv;
    bcm_vlan_t                    vid;
    bcm_port_t                    port;
    int                           modid = -1;
    int                           portMode;
    int                           ifilter, efilter;
    soc_sbx_g3p1_pv2e_t           hw_pvid;
    soc_sbx_g3p1_pvv2edata_t      hw_pvv2e;
    bcm_mpls_port_t               null_mpls_port;
    bcm_mpls_port_t               *lp_mpls_port = mpls_port;
    uint32                      trunkid;
    bcm_trunk_add_info_t         *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;


    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit, "Enter\n")));

    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        rv = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit, mpls_port->port,
                                             &modid, &port);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) extracting mod-port from gport\n"),
                       bcm_errmsg(rv)));
            return rv;
        }
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
    }
    

    if (l3_fe->fe_my_modid != modid && (!is_trunk)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "nothing to be done for modid(0x%x) my_modid(0x%x)\n"),
                   modid, l3_fe->fe_my_modid));
        return BCM_E_NONE;
    }

    bcm_mpls_port_t_init(&null_mpls_port);
    null_mpls_port.port = mpls_port->port;

    /* update the logical port */
    if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
        lp_mpls_port = &null_mpls_port;
    }
    rv = _bcm_caladan3_g3p1_mpls_lp_write(l3_fe->fe_unit, logicalPort,
                                        vpn_sap, lp_mpls_port, action);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                   "failed to write logical port 0x%x: %d (%s)\n"),
                   logicalPort, rv, bcm_errmsg(rv)));
        return rv;
    }

    for (index = 0; index < num_ports; index++) {
        if ( is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }

        rv = bcm_port_dtag_mode_get(l3_fe->fe_unit, port, &portMode);

        /* port must be in port mode */
        if (portMode != BCM_PORT_DTAG_MODE_TRANSPARENT) {
            char *portModeStr = "<unknown>";

            switch (portMode) {
            case BCM_PORT_DTAG_MODE_NONE:
                portModeStr = "BCM_PORT_DTAG_MODE_NONE"; break;
            case BCM_PORT_DTAG_MODE_INTERNAL:
                portModeStr = "BCM_PORT_DTAG_MODE_INTERNAL"; break;
            case BCM_PORT_DTAG_MODE_EXTERNAL:
                portModeStr = "BCM_PORT_DTAG_MODE_EXTERNAL"; break;
            }

            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, 
                       "Port must be in BCM_PORT_DTAG_MODE_TRANSPARENT for "
                       "mpls port match criteria BCM_MPLS_PORT_MATCH_PORT.\nPort "
                       " found to be in mode %d %s\n"),
                       portMode, portModeStr));

            return BCM_E_PARAM;
        }

        soc_sbx_g3p1_pvv2edata_t_init(&hw_pvv2e);

        /**
         * XXX: TBD:
         * Callback from L2 for change in ulNativeVid
         */

        /* get the native VID so we can set the proper VID's pv2e entry */
        rv = bcm_port_untagged_vlan_get(l3_fe->fe_unit, port, &vid);
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) reading nativeVid for port %d\n"),
                       bcm_errmsg(rv), port));
            return rv;
        }

        /* vlan will replicate this to VID 0 automatically so untagged works */
        rv = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, port, vid, &hw_pvid);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, 
                       "error(%s) reading PVid2Etc for port %d vid %d\n"),
                       bcm_errmsg(rv), port, vid));
            return rv;
        }

        hw_pvid.lpi = logicalPort;

        if ((action == _CALADAN3_MPLS_PORT_MATCH_ADD) ||
            (action == _CALADAN3_MPLS_PORT_MATCH_UPDATE)) {

            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                /* VPWS FTEs are VSIs, convert the port id to the DLF FTE
                 * then convert the DLF FTE to the VSI
                 */
                hw_pvid.vlan = 
                    BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
                hw_pvid.vlan -= l3_fe->vlan_ft_base;

                if (!(mpls_port->flags & BCM_MPLS_PORT_FAILOVER)) {
                    rv = _bcm_caladan3_g3p1_mplstp_vpws_offset(l3_fe->fe_unit,vpnc,
                                                               vpn_sap,
                                                               &hw_pvid.lpi,
                                                               &hw_pvid.vlan);
                    hw_pvid.vpws = 1;
                } else {
                    /* If failover port & vpws,
                     *      temporarily set lpi and vlan
                     * On attaching the failover with primary port,
                     *      the lpi and vlan will be updated with primary
                     *      port's lpi and vid
                     */

                    hw_pvid.lpi = 0;
                    hw_pvid.vlan = 0;
                }

            } else {
                hw_pvid.vlan = vpnc->vpn_id;
            }

            hw_pvv2e.lpi          = hw_pvid.lpi;
            hw_pvv2e.keeporstrip  = 1;
            hw_pvv2e.vlan         = hw_pvid.vlan;
            hw_pvv2e.vpws         = hw_pvid.vpws;

            /* update lpi in software state */
            vpn_sap->logicalPort  = logicalPort;
        } else if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
            /* reset the port/vid to the original setting */
            hw_pvid.lpi  = 0;

            _bcm_caladan3_vlan_port_filter_get(l3_fe->fe_unit, port,
                                             &ifilter, &efilter);
            if (ifilter) {
                hw_pvid.vlan = 0;
            } else {
                hw_pvid.vlan = vid;
            }

            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                hw_pvid.vpws = 0;
            }

            /* update lpi in software state */
            vpn_sap->logicalPort  = 0;
        } else {
            return BCM_E_INTERNAL;
        }

        if (BCM_FAILURE(rv)) {
            return rv;
        }

        rv = _soc_sbx_g3p1_pv2e_set(l3_fe->fe_unit, port, vid, &hw_pvid);

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) writing pv2e(0x%x,0x%x)\n"),
                       bcm_errmsg(rv), port, vid));
            return rv;
        }
        if (mpls_port->flags & BCM_MPLS_PORT_INNER_VLAN_ADD
            && action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {

            rv = soc_sbx_g3p1_util_pvv2e_remove(l3_fe->fe_unit, port, 0, 0xFFF);
            LOG_DEBUG(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "Deleting pvv2e(0x%x,0xFFF,0x%x)\n"),
                       port, 0));

            rv = _bcm_caladan3_g3p1_vlan_nvid_pvv2e_flags_set(
                                               l3_fe->fe_unit,
                                               port, 0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) setting vlan nvid pvv2e flags for port (0x%x),action(0x%x)\n"),
                           bcm_errmsg(rv), port,action));
                return rv;
            }

        } else if (mpls_port->flags & BCM_MPLS_PORT_INNER_VLAN_ADD) {
            hw_pvv2e.vid = vid;
            hw_pvv2e.replace = 1;

            rv = _bcm_caladan3_g3p1_vlan_nvid_pvv2e_flags_set(
                                               l3_fe->fe_unit,
                                               port,
                                              (BCM_CALADAN3_NVID_OVERRIDE_PVV2E |
                                               BCM_CALADAN3_NVID_USE_PVV2E |
                                               BCM_CALADAN3_NVID_SET_REPLACE));

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) setting vlan nvid pvv2e flags for port (0x%x)\n"),
                           bcm_errmsg(rv), port));
                return rv;
            }

            rv = soc_sbx_g3p1_util_pvv2e_set(l3_fe->fe_unit, port, 0, 0xFFF, &hw_pvv2e);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) writing pvv2e(0x%x,0xFFF,0x%x)\n"),
                           bcm_errmsg(rv),
                           port, 0));
                return rv;
            } else {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "Set pvv2e(0x%x,0xFFF,0x%x)\n"),
                           port, 0));
            }
        }
    }

    return BCM_E_NONE;

}

int
_bcm_caladan3_g3p1_mpls_match_pvid2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                                       int                      action,
                                       uint32                 logicalPort,
                                       _caladan3_vpn_control_t     *vpnc,
                                       _caladan3_vpn_sap_t         *vpn_sap,
                                       bcm_mpls_port_t         *mpls_port)
{

    int                           status=0, stripTag=0, mode=0; 
    bcm_port_t                    port;
    bcm_module_t                  modid = -1;
    soc_sbx_g3p1_pv2e_t           hw_pvid2etc;
    int                           ifilter, efilter; 
    bcm_mpls_port_t               null_mpls_port;
    bcm_mpls_port_t              *lp_mpls_port = mpls_port;
    soc_sbx_g3p1_pvv2edata_t      hw_pvv2e;
    uint32                      trunkid;
    bcm_trunk_add_info_t         *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;


    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit,
               "Enter Port=0x%x, Vlan=0x%x, lp=0x%x\n"),
               mpls_port->port, mpls_port->match_vlan, logicalPort));

    if ((action != _CALADAN3_MPLS_PORT_MATCH_ADD) &&
        ((mpls_port->port != vpn_sap->vc_mpls_port.port) ||
         (mpls_port->match_vlan != vpn_sap->vc_mpls_port.match_vlan))) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "cannot change mod-port or match criteria, user needs to delete and then re-add\n")));
        return BCM_E_PARAM;
    }

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
    }

    bcm_mpls_port_t_init(&null_mpls_port);
    null_mpls_port.port = mpls_port->port;


    if ((l3_fe->fe_my_modid != modid && (!is_trunk))) {
        /**
         * ADD or DELETE needs to modify the match
         * criteria only on local module.
         * We expect the user to call mpls_port_add()
         * on all the modules and that will take care
         * of converging to user expectation.
         */
        LOG_VERBOSE(BSL_LS_BCM_MPLS,
                    (BSL_META_U(l3_fe->fe_unit, "Remote Module:No update needed\n")));

        return BCM_E_NONE;
    }

    /* update the logical port */
    if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
        lp_mpls_port     = &null_mpls_port;
    }
    status = _bcm_caladan3_g3p1_mpls_lp_write(l3_fe->fe_unit,
                                            logicalPort,
                                            vpn_sap, lp_mpls_port,
                                            action);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                   "failed to write logical port 0x%x: %d (%s)\n"),
                   logicalPort, status, bcm_errmsg(status)));
        return status;
    }

    for (index = 0; index < num_ports; index++) {
        if ( is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }

        soc_sbx_g3p1_pv2e_t_init(&hw_pvid2etc);
        soc_sbx_g3p1_pvv2edata_t_init(&hw_pvv2e);

        if (action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) {
            status = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, port,
                                           vpn_sap->vc_mpls_port.match_vlan,
                                           &hw_pvid2etc);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit,
                           "error(%s) reading pv2e(0x%x,0x%x)\n"),
                           bcm_errmsg(status),
                           port, vpn_sap->vc_mpls_port.match_vlan));
                return status;
            }
        }

        if ((action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) ||
            (action == _CALADAN3_MPLS_PORT_MATCH_ADD)) {

            hw_pvid2etc.lpi = logicalPort;

            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                /* VPWS FTEs are VSIs, convert the port id to the DLF FTE
                 * then convert the DLF FTE to the VSI
                 */
                hw_pvid2etc.vlan =
                    BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
                hw_pvid2etc.vlan -= l3_fe->vlan_ft_base;

                if (!(mpls_port->flags & BCM_MPLS_PORT_FAILOVER)) {
                    status = _bcm_caladan3_g3p1_mplstp_vpws_offset(l3_fe->fe_unit,vpnc,
                                                                   vpn_sap,
                                                                   &hw_pvid2etc.lpi,
                                                                   &hw_pvid2etc.vlan);
                    hw_pvid2etc.vpws = 1;
                } else {
                    /* If failover port & vpws,
                     *      temporarily set lpi and vlan
                     * On attaching the failover with primary port,
                     *      the lpi and vlan will be updated with primary
                     *      port's lpi and vid
                     */

                    hw_pvid2etc.lpi = 0;
                    hw_pvid2etc.vlan = mpls_port->match_vlan;
                }

            } else {
                hw_pvid2etc.vlan = vpnc->vpn_id;
            }

            /* setup a pvv lookup for untagged traffic such that outer vid
             * p2e.nativevid (always 0 for p3), and the inner vid = 0xfff
             */
            hw_pvv2e.lpi          = hw_pvid2etc.lpi;
            hw_pvv2e.keeporstrip  = 1;
            hw_pvv2e.vlan         = hw_pvid2etc.vlan;
            hw_pvv2e.vpws         = hw_pvid2etc.vpws;

            /* update lpi in software state */
            vpn_sap->logicalPort  = logicalPort;
        } else if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {

            /* Reset the port/vid to the original setting */
            hw_pvid2etc.lpi  = 0;
            _bcm_caladan3_vlan_port_filter_get(l3_fe->fe_unit, port, 
                                             &ifilter, &efilter);
            if (ifilter) {
                hw_pvid2etc.vlan = 0;
            } else {
                hw_pvid2etc.vlan = mpls_port->match_vlan;
            }

            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                hw_pvid2etc.vpws = 0;
            }

            /* update lpi in software state */
            vpn_sap->logicalPort  = 0;
        } else {
            return BCM_E_INTERNAL;
        }

        if (BCM_FAILURE(status)) {
            return status;
        }

        if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
            stripTag = 1;
        } else {
            /* Tag preservation is performed on a per-port basis in p3 */
            stripTag = !(mpls_port->flags & BCM_MPLS_PORT_INNER_VLAN_PRESERVE);
        }

        status = bcm_port_dtag_mode_get(l3_fe->fe_unit, port, &mode);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) reading port mode (0x%x)\n"),
                       bcm_errmsg(status), port));
            return status;
        }

        /* If Stag bundling set p2e bundling bits */
        if (mode == BCM_PORT_DTAG_MODE_INTERNAL) {
            if (mpls_port->flags & BCM_MPLS_PORT_INNER_VLAN_PRESERVE) {
                soc_sbx_g3p1_p2e_t hw_p2etc;

                status = soc_sbx_g3p1_p2e_get(l3_fe->fe_unit, port, &hw_p2etc);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, 
                               "failed to get port2e table port=%d: %d (%s)\n"),
                               port, status, bcm_errmsg(status)));
                    return status;
                }

                if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
                    hw_p2etc.mplstp = 1;
                } else {
                    uint32 bundled_mode = 0;
                    soc_sbx_g2p3_mpls_stag_bundled_mode_get(l3_fe->fe_unit, &bundled_mode);
                    hw_p2etc.mplstp = bundled_mode;
                }

                status = soc_sbx_g3p1_p2e_set(l3_fe->fe_unit, port, &hw_p2etc);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, 
                               "failed to set port2e table port=%d: %d (%s)\n"),
                               port, status, bcm_errmsg(status)));
                    return status;
                }
            }
        } else {
            status = bcm_caladan3_port_strip_tag(l3_fe->fe_unit, port, stripTag);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(l3_fe->fe_unit, 
                           "failed to set port_strip_tag port=%d: %d (%s)\n"),
                           port, status, bcm_errmsg(status)));
                return status;
            }
        }

        /**
         * Write pvid2etc for the new match
         * <port, vid> key
         */
        status = _soc_sbx_g3p1_pv2e_set(l3_fe->fe_unit,
                                        port, mpls_port->match_vlan,
                                        &hw_pvid2etc);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) writing pv2e(0x%x,0x%x)\n"),
                       bcm_errmsg(status),
                       port, mpls_port->match_vlan));
            return status;
        }

        /* TBD: if the port is in provider mode, and we're adding a pvid match,
         * we need to consider the case where an stagged only packet arrives
         */
        if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_REPLACE) &&
            (mode == BCM_PORT_DTAG_MODE_EXTERNAL)) {

            if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
                status = soc_sbx_g3p1_util_pvv2e_remove
                    (l3_fe->fe_unit, port, mpls_port->match_vlan, 0xFFF);
            } else {
                hw_pvv2e.replace     = 1;
                hw_pvv2e.vid         = mpls_port->egress_service_vlan;
                hw_pvv2e.keeporstrip = 0;
                status = soc_sbx_g3p1_util_pvv2e_set
                    (l3_fe->fe_unit, port, mpls_port->match_vlan, 0xFFF, &hw_pvv2e);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit,
                               "error(%s) writing pvv2e(0x%x,0xFFF,0x%x)\n"),
                               bcm_errmsg(status),
                               port, mpls_port->match_vlan));
                    return status;
                }
            }
        }
    }

    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_mpls_match_pstackedvid2etc(_caladan3_l3_fe_instance_t  *l3_fe,
                                            int                      action,
                                            uint32                 logicalPort,
                                            _caladan3_vpn_control_t     *vpnc,
                                            _caladan3_vpn_sap_t         *vpn_sap,
                                            bcm_mpls_port_t         *mpls_port)
{
    int                           rv;
    bcm_port_t                    port = 0;
    int                           modid = -1;
    soc_sbx_g3p1_pvv2edata_t      hw_pvv;
    int                           ivid=0, ovid=0;
    char                         *func_called = "<unknown>";
    bcm_mpls_port_t               null_mpls_port;
    bcm_mpls_port_t              *lp_mpls_port = mpls_port;
    uint32                      trunkid;
    bcm_trunk_add_info_t         *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;

    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        rv = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                 mpls_port->port,
                                                 &modid, &port);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) extracting mod-port from gport\n"),
                       bcm_errmsg(rv)));
            return rv;
        }
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
    }
    
    if (modid != l3_fe->fe_my_modid && (is_trunk == 0 )) {
        /* nothing to be done for remote port */
        return BCM_E_NONE;
    }

    bcm_mpls_port_t_init(&null_mpls_port);
    null_mpls_port.port = mpls_port->port;

    if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
        lp_mpls_port     = &null_mpls_port;
    }
    rv = _bcm_caladan3_g3p1_mpls_lp_write(l3_fe->fe_unit, logicalPort,
                                        vpn_sap, lp_mpls_port, action);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                   "error(%s) failed to write logical port "
                   "(port=%d, innerVid=%d, outerVid=%d)\n"),
                   bcm_errmsg(rv), port, ivid, ovid));
        return rv;
    }


    ivid  = mpls_port->match_inner_vlan;
    ovid  = mpls_port->match_vlan;

    for (index = 0; index < num_ports; index++) {
        if ( is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }
        soc_sbx_g3p1_pvv2edata_t_init(&hw_pvv);

        hw_pvv.lpi = logicalPort;

        if (action == _CALADAN3_MPLS_PORT_MATCH_DELETE) {
            hw_pvv.lpi   = 0;
            rv = soc_sbx_g3p1_util_pvv2e_remove(l3_fe->fe_unit, port, ovid, ivid);
            func_called = "soc_util_pvv2e_remove";
            /* update lpi in software state */
            vpn_sap->logicalPort  = 0;
        } else if ((action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) ||
                   action == _CALADAN3_MPLS_PORT_MATCH_ADD) {

            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                /* VPWS FTEs are VSIs, convert the port id to the DLF FTE
                 * then convert the DLF FTE to the VSI
                 */
                hw_pvv.vlan = 
                         BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
                hw_pvv.vlan -= l3_fe->vlan_ft_base;

            } else {
                hw_pvv.vlan = vpnc->vpn_id;
            }

            /* update lpi in software state */
            vpn_sap->logicalPort  = logicalPort;

            if (action == _CALADAN3_MPLS_PORT_MATCH_UPDATE) {
                rv = soc_sbx_g3p1_util_pvv2e_update(l3_fe->fe_unit, port, ovid, ivid,
                                               &hw_pvv);
                func_called = "soc_util_pvv2e_update";
            } else {
                rv = soc_sbx_g3p1_util_pvv2e_add(l3_fe->fe_unit, port, ovid, ivid,
                                            &hw_pvv);
                func_called = "soc_util_pvv2e_add";

            }
        } else {
            return BCM_E_PARAM;
        }

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) calling %s "
                       "(port=%d, innerVid=%d, outerVid=%d)\n"),
                       bcm_errmsg(rv), func_called,
                       port, ivid, ovid));
            return rv;
        }

        rv = soc_sbx_g2p3_pvv2e_commit(l3_fe->fe_unit, SB_COMMIT_COMPLETE);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) calling soc_sbx_g3p1_pvv2e_commit "
                       "(port=%d, innerVid=%d outerVid=%d)\n"),
                       bcm_errmsg(rv), port, ivid, ovid));
            return rv;
        }
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_mpls_program_vpn_sap_vlan2etc(_caladan3_l3_fe_instance_t   *l3_fe,
                                                 _caladan3_vpn_control_t      *vpnc,
                                                 _caladan3_vpn_sap_t          *vpn_sap)
{

    int                   rv, vix;
    soc_sbx_g3p1_v2e_t    v2e;


    soc_sbx_g3p1_v2e_t_init(&v2e);

    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        /* VPWS uses two vsi's the FTE for the VSI is the port Id
         * to get the VSI, subtract the vlan base from the fte
         */
        vix = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
        vix -= l3_fe->vlan_ft_base;
    } else {        
        /* all other vpns store the vsi as the vpn id */
        vix = vpnc->vpn_id;
    }

    rv = _bcm_caladan3_g3p1_mpls_map_set_vlan2etc(l3_fe, vpnc, vix);
    
    return rv;
}


int
_bcm_caladan3_g3p1_mpls_vpws_fte_connect(_caladan3_l3_fe_instance_t *l3_fe,
                                    uint32 p1_fte_idx,
                                    uint32 p2_fte_idx,
                                    uint32 connect)
{

    soc_sbx_g3p1_ft_t  sbxFt[_BCM_CALADAN3_VPWS_MAX_SAP];
    int                fteIdx[_BCM_CALADAN3_VPWS_MAX_SAP];
    int                rv, idx;
    uint32           ohi, qid, ohib, qidb, rridx, fti, lag, lagbase, lagsize;

    fteIdx[0] = p1_fte_idx;
    fteIdx[1] = p2_fte_idx;

    for (idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {
 
        fti = fteIdx[idx];

        soc_sbx_g3p1_ft_t_init(&sbxFt[idx]);
        rv = soc_sbx_g3p1_ft_get(l3_fe->fe_unit, fti, &sbxFt[idx]);

        /* clear union vars so the write doesn't OR them together! */
        if (sbxFt[idx].lag == 0) { 
            sbxFt[idx].lagbase = sbxFt[idx].lagsize = 0;
        } else {
            sbxFt[idx].qid = 0;
        }

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) reading port%d fte(0x%x)\n"),
                       bcm_errmsg(rv), idx+1, fti));
            return rv;
        }

        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                              "orig: fteIdx[%d]=0x%x to oi=0x%x qid=0x%x lag=%d lagbase=0x%x\n"
                              "\t\t oib=0x%x qidb=0x%x rridx=%d lagsize=%d"),
                   idx, fti, sbxFt[idx].oi, sbxFt[idx].qid,
                   sbxFt[idx].lag, sbxFt[idx].lagbase, sbxFt[idx].oib,
                   sbxFt[idx].qidb, sbxFt[idx].rridx, sbxFt[idx].lagsize));

    }

    /* set the FTEs to point to each other */
    ohi     = sbxFt[0].oi;
    qid     = sbxFt[0].qid;
    ohib    = sbxFt[0].oib;
    qidb    = sbxFt[0].qidb;
    rridx   = sbxFt[0].rridx;
    lag     = sbxFt[0].lag;
    lagbase = sbxFt[0].lagbase;
    lagsize = sbxFt[0].lagsize;
    
    sbxFt[0].oi      = sbxFt[1].oi;
    sbxFt[0].qid     = sbxFt[1].qid;
    sbxFt[0].oib     = sbxFt[1].oib;
    sbxFt[0].qidb    = sbxFt[1].qidb;
    sbxFt[0].rridx   = sbxFt[1].rridx;
    sbxFt[0].lag     = sbxFt[1].lag;
    sbxFt[0].lagbase = sbxFt[1].lagbase;
    sbxFt[0].lagsize = sbxFt[1].lagsize;

    sbxFt[1].oi      = ohi;
    sbxFt[1].qid     = qid;
    sbxFt[1].oib     = ohib;
    sbxFt[1].qidb    = qidb;
    sbxFt[1].rridx   = rridx;
    sbxFt[1].lag     = lag;
    sbxFt[1].lagbase = lagbase;
    sbxFt[1].lagsize = lagsize;

    if (connect) {
        sbxFt[0].excidx = 0;
        sbxFt[1].excidx = 0;
    } else {
        sbxFt[0].excidx  = 0x7f;
        sbxFt[1].excidx  = 0x7f;
    }

    
    for (idx=0; idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {

        fti = fteIdx[idx];
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit,
                   "BC: swapped: fteIdx[%d]=0x%x to oi=0x%x qid=0x%x lag=%d "
                   "lagbase=0x%x\n\t\t oib=0x%x qidb=0x%x rridx=%d lagsize=%d"),
                   idx, fti, sbxFt[idx].oi, sbxFt[idx].qid,
                   sbxFt[idx].lag, sbxFt[idx].lagbase, sbxFt[idx].oib,
                   sbxFt[idx].qidb, sbxFt[idx].rridx, sbxFt[idx].lagsize));

        rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, fti, &sbxFt[idx]);

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit, "error(%s) writing port%d fte(0x%x)\n"),
                       bcm_errmsg(rv), idx+1, fti));
            return rv;
        }
    }

    for (idx=0; l3_fe->umc_ft_offset && idx < _BCM_CALADAN3_VPWS_MAX_SAP; idx++) {
        /* update the unknown mc ft as well */
        fti = fteIdx[idx] + l3_fe->umc_ft_offset;
        rv = soc_sbx_g3p1_ft_set(l3_fe->fe_unit, fti, &sbxFt[idx]);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(l3_fe->fe_unit,
                       "error(%s) writing port%d UMC fte(0x%x) for unknown mc\n"),
                       bcm_errmsg(rv), idx+1, fti));
            return rv;
        }
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_mpls_get_fte(_caladan3_l3_fe_instance_t *l3_fe,
                              uint32                  fte_idx,
                              int                     action,
                              _caladan3_l3_or_mpls_egress_t *egr)
{
    int                rv;
    soc_sbx_g3p1_ft_t  fte;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit, "Enter fte-idx(0x%x)\n"), fte_idx));

    sal_memset(egr, 0, sizeof(*egr));

    rv = soc_sbx_g3p1_ft_get(l3_fe->fe_unit, fte_idx, &fte);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) reading fte-idx(0x%x)\n"),
                   bcm_errmsg(rv), fte_idx));
        return rv;
    }

    if (action == L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY) {
        return BCM_E_NONE;
    }

    rv = BCM_E_NONE;

    if (!fte.lag) {
        SOC_SBX_NODE_PORT_FROM_QID(l3_fe->fe_unit, fte.qid, egr->fte_node,
                               egr->fte_port,
                               l3_fe->fe_cosq_config_numcos);
    }

    egr->encap_id   = SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi);
    if (!fte.lag) {
        rv = soc_sbx_modid_get(l3_fe->fe_unit, egr->fte_node,
                           egr->fte_port, &egr->fte_modid);
    }
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(l3_fe->fe_unit, "error(%s) soc-sbx-modid-get\n"),
                   bcm_errmsg(rv)));
        return rv;
    }

    if (action == L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY) {
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

int
_bcm_caladan3_g3p1_mpls_port_discard_set(int unit, bcm_port_t port, int mode)
{
    soc_sbx_g3p1_p2e_t   p2e;
    soc_sbx_g3p1_pv2e_t  pv2e;
    soc_sbx_g3p1_pvv2edata_t pvv2e;
    int                  rv = BCM_E_NONE;
    int                  vid;
    bcm_gport_t          mpls_port_id = 0;
    bcm_port_t           phy_port;
    bcm_vlan_t           match_vlan, mpls_vsi;
    uint32             logical_port;
    bcm_mpls_port_t      mpls_port;
    _caladan3_l3_fe_instance_t *l3_fe = NULL;
    uint8              vpwsuni=0;
    uint16             num_ports = 1;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }

    /* if port is not in provider, nothing to do.  Only need to
     * handle the case of STAG only, where there's a pvv2e entry for
     * SVID,0xFFF
     */
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_p2e_get(unit, port, &p2e));
    if (p2e.provider == 0) {
        return BCM_E_NONE;
    }

    /* scan all pv2e's; if maps to an mpls_port,
     * then set pvv2e.vlan to 0, or vsi, based on mode
     */
    for (vid = 0; vid < BCM_VLAN_MAX; vid++) {
        vpwsuni = 0;
        rv = SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &pv2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "error %d (%s) failed to read pv2e"
                                   " p/v=%d/0x%x\n"),
                       rv, bcm_errmsg(rv), port, vid));
            break;
        }

        if (pv2e.vpws) {
            vpwsuni = 1;
        }

        if (vpwsuni || pv2e.vlan > BCM_VLAN_MAX) {
            if (vpwsuni) {
                BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, 
                                           l3_fe->vpws_uni_ft_offset + pv2e.vlan);
            } else {
                /*  Use the VSI as a hint to the gport */
                BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, 
                                           pv2e.vlan + l3_fe->vlan_ft_base);
            }

            bcm_mpls_port_t_init(&mpls_port);
            rv = _bcm_caladan3_mpls_port_vlan_vector_internal(unit, mpls_port_id,
                                                            &phy_port,
                                                            &match_vlan,
                                                            &mpls_vsi,
                                                            &logical_port,
                                                            &mpls_port, NULL,
                                                            &num_ports);

            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "rv=%d vsi=0x%x --> \nmplsPort=0x%x"
                                     " phy_port=%d match_vlan=0x%x mpls_vsi=0x%x lp=0x%x\n"),
                         rv, pv2e.vlan,
                         mpls_port_id, phy_port, match_vlan, mpls_vsi, logical_port));

            if (BCM_SUCCESS(rv)) {
                /* this is an mpls_port, update pvv2e accordingly */
                rv = soc_sbx_g3p1_util_pvv2e_get(unit, port, vid, 0xFFF, &pvv2e);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "error %d (%s) failed to read"
                                           " pvv2e p/v/v=%d/0x%x/0xFFF\n"),
                               rv, bcm_errmsg(rv), port, vid));
                    break;
                }

                if (mode == BCM_PORT_DISCARD_UNTAG) {
                    pvv2e.vlan = 0;
                } else if (mode == BCM_PORT_DISCARD_NONE ||
                           mode == BCM_PORT_DISCARD_TAG) {
                    pvv2e.vlan =  mpls_vsi;
                    if (pv2e.vpws) {
                        pvv2e.vpws = vpwsuni;
                    }
                }
                rv = soc_sbx_g3p1_util_pvv2e_set(unit, port, vid, 0xFFF, &pvv2e);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "error %d (%s) failed to write"
                                           " pvv2e p/v/v=%d/0x%x/0xFFF\n"),
                               rv, bcm_errmsg(rv), port, vid));
                    break;
                }

            }
        }
        rv = BCM_E_NONE;

    }

    return rv;
}

int
_bcm_caladan3_g3p1_mpls_tunnel_ete_set(int                        unit,
                                     _caladan3_l3_fe_instance_t  *l3_fe,
                                     _caladan3_l3_intf_t         *l3_intf,
                                     _caladan3_l3_ete_t          *v4_ete, /* maybe NULL */
                                     int                          num_labels,
                                     bcm_mpls_egress_label_t     *label_array,
                                     _caladan3_l3_ete_t          *mpls_sw_ete,
                                     int                          ete_allocated)
{
    int                      ignore_status, status = BCM_E_NONE;
    soc_sbx_g3p1_ete_t  mpls_tunnel_hw_ete;
    soc_sbx_g3p1_oi2e_t       ohi2etc;
    bcm_mpls_egress_label_t *label_info;
    _caladan3_vpn_sap_t           *vpn_sap = NULL;
    int                            ttlcheck = 0;

    soc_sbx_g3p1_ete_t_init(&mpls_tunnel_hw_ete);

    label_info                = label_array;

    soc_sbx_g3p1_ete_t_init(&mpls_tunnel_hw_ete);

    mpls_tunnel_hw_ete.smacindex = l3_intf->if_lmac_idx;

    /* If a VLAN was configured use it otherwise set to untagged */
    if (l3_intf->if_info.l3a_vid != 0) {
        mpls_tunnel_hw_ete.vid = l3_intf->if_info.l3a_vid;
    } else {
        mpls_tunnel_hw_ete.vid = _BCM_VLAN_G3P1_UNTAGGED_VID;
    }
    mpls_tunnel_hw_ete.mtu = l3_intf->if_info.l3a_mtu;
    mpls_tunnel_hw_ete.usevid = 1;
    mpls_tunnel_hw_ete.dscpremark = 1;

    mpls_tunnel_hw_ete.remark = _MPLS_EXPMAP_HANDLE_DATA(label_info->qos_map_id);

    /* tunnel entry */
    mpls_tunnel_hw_ete.tunnelenter = 1;

    if (!(label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_SET)) {
        ttlcheck = 1;
    }

    if (label_info->label == _BCM_MPLS_IMPLICIT_NULL_LABEL) {
        /**
         * XXX:
         * This needs changes in ucode to auto gen eType
         * back-to-back as well as non back-to-back.
         * The packet can be either with VPN-label or
         * data over L2.
         */
        mpls_tunnel_hw_ete.etype           = 0x0800;
        /* mpls_tunnel_hw_ete.ulMplsPop         =  1; */
    } else {
        mpls_tunnel_hw_ete.etype          = 0x8847;
        mpls_tunnel_hw_ete.encaplen       = num_labels * 4 + 2;
        mpls_tunnel_hw_ete.ipttldec       = 0;

        /*
         * In the BCM API, lalel[0] is the innermost, i.e closest
         * to the payload.
         */
        if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            mpls_tunnel_hw_ete.ttl2 = label_info->ttl;
        } else if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT) {
            /* by default the erh.ttl - 1 is put into mpls.ttl */
        /*    mpls_tunnel_hw_ete.mplsttldec = 1; */
            mpls_tunnel_hw_ete.ttl2dec = 1;
        } else {
            /* default: need erh.ttl - 1 in mpls.ttl */
        /*    mpls_tunnel_hw_ete.mplsttldec = 1; */
            mpls_tunnel_hw_ete.ttl2dec = 1;
        }

        if (label_info->flags & BCM_MPLS_EGRESS_LABEL_EXP_REMARK) {
            mpls_tunnel_hw_ete.exp2remark = 1;
        }
        if (num_labels == 1) {
            mpls_tunnel_hw_ete.s2                = 1;
        }
        mpls_tunnel_hw_ete.exp2              = label_info->exp;
        mpls_tunnel_hw_ete.label2            = label_info->label;
    }

    if (num_labels > 1) {
        label_info                  =  label_array + 1;

        mpls_tunnel_hw_ete.label1   = mpls_tunnel_hw_ete.label2;
        mpls_tunnel_hw_ete.s1       = 1;
        mpls_tunnel_hw_ete.exp1     = mpls_tunnel_hw_ete.exp2;
        mpls_tunnel_hw_ete.exp1remark = mpls_tunnel_hw_ete.exp2remark;
        mpls_tunnel_hw_ete.ttl1     = mpls_tunnel_hw_ete.ttl2;
        mpls_tunnel_hw_ete.ttl1dec  = mpls_tunnel_hw_ete.ttl2dec;

        if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            mpls_tunnel_hw_ete.ttl2 = label_info->ttl;
            mpls_tunnel_hw_ete.ttl2dec = 0;
        } else if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT) {
            /* by default the erh.ttl - 1 is put into mpls.ttl */
         /*   mpls_tunnel_hw_ete.mplsttldec = 1; */
            mpls_tunnel_hw_ete.ttl2dec = 1;
            ttlcheck = 1;
        }

        if (num_labels == 2) {
            mpls_tunnel_hw_ete.s2                = 0;
        }
        mpls_tunnel_hw_ete.exp2              = label_info->exp;
        mpls_tunnel_hw_ete.label2            = label_info->label;
        mpls_tunnel_hw_ete.exp2remark        = mpls_tunnel_hw_ete.exp1remark;
    }

    if (num_labels > 2) {
        label_info                  =  label_array + 2;

        mpls_tunnel_hw_ete.label0 = mpls_tunnel_hw_ete.label1;
        mpls_tunnel_hw_ete.exp0   = mpls_tunnel_hw_ete.exp1;
        mpls_tunnel_hw_ete.exp0remark = mpls_tunnel_hw_ete.exp1remark;
        mpls_tunnel_hw_ete.ttl0 =   mpls_tunnel_hw_ete.ttl1;
        mpls_tunnel_hw_ete.ttl0dec = mpls_tunnel_hw_ete.ttl1dec;
        mpls_tunnel_hw_ete.s0                = 1;

        mpls_tunnel_hw_ete.label1   = mpls_tunnel_hw_ete.label2;
        mpls_tunnel_hw_ete.exp1     = mpls_tunnel_hw_ete.exp2;
        mpls_tunnel_hw_ete.exp1remark = mpls_tunnel_hw_ete.exp2remark;
        mpls_tunnel_hw_ete.ttl1     = mpls_tunnel_hw_ete.ttl2;
        mpls_tunnel_hw_ete.ttl1dec  = mpls_tunnel_hw_ete.ttl2dec;


        if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            mpls_tunnel_hw_ete.ttl2 = label_info->ttl;
            mpls_tunnel_hw_ete.ttl2dec = 0;
        } else if (label_info->flags & BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT) {
            /* by default the erh.ttl - 1 is put into mpls.ttl */
         /*   mpls_tunnel_hw_ete.mplsttldec = 1; */
            mpls_tunnel_hw_ete.ttl2dec = 1;
            ttlcheck = 1;
        }

        mpls_tunnel_hw_ete.exp2              = label_info->exp;
        mpls_tunnel_hw_ete.label2            = label_info->label;
        mpls_tunnel_hw_ete.exp2remark        = mpls_tunnel_hw_ete.exp1remark;

        if (num_labels == 3) {
            mpls_tunnel_hw_ete.s2                = 0;
            mpls_tunnel_hw_ete.s1                = 0;
        }
    }

    mpls_tunnel_hw_ete.ttlcheck = ttlcheck;

    if (v4_ete) {
        mpls_tunnel_hw_ete.dmac5 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[5]);
        mpls_tunnel_hw_ete.dmac4 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[4]);
        mpls_tunnel_hw_ete.dmac3 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[3]);
        mpls_tunnel_hw_ete.dmac2 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[2]);
        mpls_tunnel_hw_ete.dmac1 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[1]);
        mpls_tunnel_hw_ete.dmac0 =   (v4_ete->l3_ete_key.l3_ete_hk.dmac[0]);
        mpls_tunnel_hw_ete.dmacset = 1;
        mpls_tunnel_hw_ete.dmacsetlsb = 1;
    }


    mpls_tunnel_hw_ete.smac5 = l3_intf->if_info.l3a_mac_addr[5];
    mpls_tunnel_hw_ete.smac4 = l3_intf->if_info.l3a_mac_addr[4];
    mpls_tunnel_hw_ete.smac3 = l3_intf->if_info.l3a_mac_addr[3];
    mpls_tunnel_hw_ete.smac2 = l3_intf->if_info.l3a_mac_addr[2];
    mpls_tunnel_hw_ete.smac1 = l3_intf->if_info.l3a_mac_addr[1];
    mpls_tunnel_hw_ete.smac0 = l3_intf->if_info.l3a_mac_addr[0];
    mpls_tunnel_hw_ete.smacset = 1;

    /* Encaplen during header compression.
       It is assumed that there will be no tagged tunnel on the port where 
       the Header compression is enabled.
     */
    /* comp_encaplen can be calculated in ucode.
       This field is added in ete to save ucode instruction.
     */
    if (mpls_tunnel_hw_ete.encapmac) {
        /* If header compression enabled, knock off mac & etype */
        mpls_tunnel_hw_ete.comp_encaplen = mpls_tunnel_hw_ete.encaplen - 14;
    } else {
        /* If header compression enabled, knock off etype */
        mpls_tunnel_hw_ete.comp_encaplen = mpls_tunnel_hw_ete.encaplen - 2;
    }

    status = soc_sbx_g3p1_ete_set(unit,
                                  mpls_sw_ete->l3_ete_hw_idx.ete_idx,
                                  &mpls_tunnel_hw_ete);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s]  error(%s) writing mpls-ete(0x%x) on "
                               "interface(0x%x)"),
                   l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status),
                   mpls_sw_ete->l3_ete_hw_idx.ete_idx,
                   _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
        if (ete_allocated) {
            ignore_status = _bcm_caladan3_undo_l3_ete_alloc(l3_fe,
                                                            l3_intf,
                                                            &mpls_sw_ete);
        }
        return status;
    }

    /*
     * Remap the ipv4 ohi to point to the mpls tunnel ete
     */
    if (v4_ete && ete_allocated) {
        /*
         * If there was a v4-ete and the mpls ete was just added,
         * point the ohi to the mpls ete
         */
        soc_sbx_g3p1_oi2e_t_init(&ohi2etc);
        ohi2etc.eteptr = mpls_sw_ete->l3_ete_hw_idx.ete_idx;
        status = soc_sbx_g3p1_oi2e_set(unit,
                                       _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(v4_ete->l3_ohi.ohi),
                                       &ohi2etc);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s]  error %s in setting OutHdrIndex2Etc to mpls ete "
                                   "Interface Id (0x%x) \n"),
                       l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status),
                       _CALADAN3_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
            ignore_status = _bcm_caladan3_undo_l3_ete_alloc(l3_fe,
                                                          l3_intf,
                                                          &mpls_sw_ete);
            COMPILER_REFERENCE(ignore_status);
            return status;
        }
    }

    if (ete_allocated == 0) {
        /**
         * This is the update case,
         * we need to update all VC etes that depend
         * on this mpls encap ete
         *
         * handle the case the mpls-ete (tunnel) has implicit
         * label
         */
        _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE(mpls_sw_ete, vpn_sap) {

           _caladan3_vpn_control_t  *vpnc;
           int                       num_vpn_ports = 0;
           int                       reconnect = 0;
           _caladan3_vpn_sap_t      *tmp_vpn_sap = NULL, *vpnSaps[_BCM_CALADAN3_VPWS_MAX_SAP];

            vpnc = vpn_sap->vc_vpnc;
            
            if (vpnc && (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) &&
                !(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {

                _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
                    if (!(tmp_vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {
                        if (num_vpn_ports < _BCM_CALADAN3_VPWS_MAX_SAP) {
                            vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                        }
                        num_vpn_ports++;
                    }
                } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

                if (num_vpn_ports == 2 ) {
                    status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                               _CALADAN3_MPLS_PORT_MATCH_UPDATE, 0);
                    if (BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "error(%s) failed to unlink VPWS\n"),
                                   bcm_errmsg(status)));
                    } else {
                        reconnect = 1;
                    }
                }
            }
            _bcm_caladan3_mpls_g3p1_map_tunnel_to_vc_egress(l3_fe, vpn_sap,
                                                            &vpn_sap->vc_mpls_port,
                                                            &mpls_tunnel_hw_ete);

           if (reconnect) {
               /* Cross-connect again after update !*/
               status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                              _CALADAN3_MPLS_PORT_MATCH_UPDATE, 1);
               if (BCM_FAILURE(status)) {
                   LOG_ERROR(BSL_LS_BCM_MPLS,
                             (BSL_META_U(unit,
                                         "error(%s) failed to link VPWS\n"),
                              bcm_errmsg(status)));
               }
           }

        } _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE_END(mpls_sw_ete, vpn_sap);
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_caladan3_g3p1_mpls_fte_copy_from_l3_egr(_caladan3_l3_fe_instance_t   *l3_fe,
                                           bcm_mpls_tunnel_switch_t *info,
                                           uint32                    base_vsi,
                                           uint32                    count)
{
    soc_sbx_g3p1_ft_t           egr_hw_fte;
    uint32                      fte_idx;
    int                         status;
    int                         idx;

    /* Read FTE contents for the given L3 egress */
    fte_idx =
        _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(info->egress_if);
    status = soc_sbx_g3p1_ft_get(l3_fe->fe_unit,
                                 fte_idx,
                                 &egr_hw_fte);

    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) reading l3-fte(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status), fte_idx));
        goto error_done;
    }

    /**
     * XXX: TBD:
     * if l3 egress changes after this call, that
     * will not be updated in the mpls portion.
     * user needs to explicitly call switch_update.
     */
    for (idx = 0; idx < count; idx++) {
        status = soc_sbx_g3p1_ft_set(l3_fe->fe_unit,
                                     base_vsi + idx,
                                     &egr_hw_fte);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META("unit %d: [%s] error(%s) programming mpls-fte(0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_idx));
            goto error_done;
        }
    }

    return BCM_E_NONE;

error_done:
    return status;
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_mpls_egress_ttl_update
 * Purpose:
 *      update the ttl processing fields on egress
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *      caller_modid- module that is calling to update info
 *      encap_id    - ohi,ete being updated
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_g3p1_mpls_egress_ttl_update(_caladan3_l3_fe_instance_t   *l3_fe,
                                        bcm_mpls_tunnel_switch_t *info,
                                        int                       caller_modid,
                                        bcm_if_t                  encap_id)
{
    soc_sbx_g3p1_oi2e_t         hw_ohi2etc;
    soc_sbx_g3p1_ete_t     mpls_hw_ete;
    int                         status;

    status = soc_sbx_g3p1_oi2e_get(l3_fe->fe_unit,
                                   _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(SOC_SBX_OHI_FROM_ENCAP_ID(encap_id)),
                                   &hw_ohi2etc);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) reading ohi2etc(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status),
                   SOC_SBX_OHI_FROM_ENCAP_ID(encap_id)));
        return BCM_E_NONE;
    }

    status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                       hw_ohi2etc.eteptr,
                                       &mpls_hw_ete);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) reading encap-ete(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status), hw_ohi2etc.eteptr));
        return BCM_E_NONE;
    }

#if 0  /* inherited, commented out, from g2p3 */
    if ( (info->action == BCM_MPLS_SWITCH_ACTION_POP) ||
         (info->action == BCM_MPLS_SWITCH_ACTION_PHP) ){
        mpls_hw_ete.ulMplsPop = 1;
    } else {
        mpls_hw_ete.ulMplsPop = 0;
    }
#endif

    if ( (info->action == BCM_MPLS_SWITCH_ACTION_SWAP) ){
        /* do i really need this */
        /* mpls_hw_ete.s2 = 1; */
        mpls_hw_ete.mplsttldec = 0;
        mpls_hw_ete.tunnelenter = 0;
    }

    if (info->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
        mpls_hw_ete.ttl0 = info->egress_label.ttl;
    } else {
        mpls_hw_ete.ttl0 = 0;
    }

    
    if (info->flags & BCM_MPLS_SWITCH_OUTER_EXP) {
        mpls_hw_ete.exp2remark = 1;
    }

    /* TTL handling */
    if (info->action == BCM_MPLS_SWITCH_ACTION_PHP) {
        mpls_hw_ete.tunnelenter = 0;
        mpls_hw_ete.ipttldec = 0;
        if (info->flags & BCM_MPLS_SWITCH_INNER_TTL) {
            mpls_hw_ete.mplsttldec = 0;
        }else{
            mpls_hw_ete.mplsttldec = 1;
        }
    }

    status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                       hw_ohi2etc.eteptr,
                                       &mpls_hw_ete);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) writing encap-ete(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status), hw_ohi2etc.eteptr));
        return BCM_E_NONE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_mpls_ttl_update
 * Purpose:
 *      update the ttl processing fields on egress
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_caladan3_g3p1_mpls_ttl_update(_caladan3_l3_fe_instance_t   *l3_fe,
                                   bcm_mpls_tunnel_switch_t *info)
{
    soc_sbx_g3p1_ft_t           egr_hw_fte;
    uint32                      fte_idx;
    int                         status;
    int                         numcos;
    int                         fte_node, fte_modid, fte_port;

    /* Read FTE contents for the given L3 egress */
    fte_idx =
        _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(info->egress_if);
    status = soc_sbx_g3p1_ft_get(l3_fe->fe_unit,
                                 fte_idx,
                                 &egr_hw_fte);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) reading l3-fte(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status), fte_idx));
        return status;
    }

    if (egr_hw_fte.lag) {
        return BCM_E_NONE;
    } else {
        numcos = l3_fe->fe_cosq_config_numcos;
        SOC_SBX_NODE_PORT_FROM_QID(l3_fe->fe_unit, egr_hw_fte.qid,
                                   fte_node,
                                   fte_port,
                                   numcos);
        status = soc_sbx_modid_get(l3_fe->fe_unit, fte_node,
                                   fte_port, &fte_modid);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META("unit %d: [%s] error(%s) getting modid for node(0x%x) port(0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_node, fte_port));
            return status;
        }
    }

    if (l3_fe->fe_my_modid == fte_modid) {
        status = _bcm_caladan3_g3p1_mpls_egress_ttl_update(
            l3_fe,
            info,
            l3_fe->fe_my_modid,
            SOC_SBX_ENCAP_ID_FROM_OHI(egr_hw_fte.oi));
        if (status != BCM_E_NONE) {
            return status;
        }
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_caladan3_g3p1_mpls_map_set_lsr_info(_caladan3_l3_fe_instance_t   *l3_fe,
                                       bcm_mpls_tunnel_switch_t *info,
                                       uint32                    base_vsi,
                                       uint32                    lpidx)
{
    soc_sbx_g3p1_labels_t      lsr_label_info;
    int                         status;
    int                           unit = l3_fe->fe_unit;
    soc_sbx_g3p1_lp_t           logical_port;
    soc_sbx_control_t           *sbx;
    bcm_port_t                    port = -1;
    int                           label1 = 0, label2 = 0, label3 = 0;
#ifndef PLATFORM_LABELS
    bcm_module_t                modid = -1;
    uint32                      trunkid;
    bcm_trunk_add_info_t       *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;
#endif

    status = BCM_E_NONE;
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    
    soc_sbx_g3p1_labels_t_init(&lsr_label_info);
    lsr_label_info.opcode      =  _BCM_CALADAN3_LABEL_LSR(unit);
    lsr_label_info.ftidx        =  base_vsi;
    if (info->flags & BCM_MPLS_SWITCH_DROP) {
        /* Trigger forced drop by using STP drop */
        lsr_label_info.stpstate = sbx->stpblock;
    }

    if (lpidx == 0) {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                              SBX_CALADAN3_USR_RES_MPLS_LPORT,
                                              1, &lsr_label_info.lpi, 0);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "failed to allocate logical port: %d (%s)\n"),
                       status, bcm_errmsg(status)));
            return status;
        }
        soc_sbx_g3p1_lp_t_init(&logical_port);
    } else {
        lsr_label_info.lpi = lpidx;
        soc_sbx_g3p1_lp_t_init(&logical_port);

        status = soc_sbx_g3p1_lp_get(l3_fe->fe_unit,
                                     lsr_label_info.lpi,
                                     &logical_port);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to get lp 0x%x: %d %s\n"),
                       lsr_label_info.lpi, status, bcm_errmsg(status)));
            return status;
        }
    }
    /* XXX: TBD:
     * L-LSP needs work here.
     */
    /* lsr_label_info.elsp   =   1;
       lsr_label_info.pipe   =   1; */

    /* Update MPLS LPORT */
    /* 1. Set qos exp_map
       2. Set pid (Not needed?)
       3. how to set the policer?
    */

    if (info->flags & BCM_MPLS_SWITCH_INT_PRI_MAP) {
        lsr_label_info.elsp = 1;
        logical_port.qos = _MPLS_EXPMAP_HANDLE_DATA(info->exp_map);
        logical_port.useexp = 1;
    }
    if (info->flags & BCM_MPLS_SWITCH_INT_PRI_SET) {
        lsr_label_info.elsp = 0;
        lsr_label_info.cos = info->int_pri;
        logical_port.qos = _MPLS_EXPMAP_HANDLE_DATA(info->exp_map);
        logical_port.useexp = 1;
    }

    if(info->flags & BCM_MPLS_SWITCH_INNER_EXP) {
        logical_port.useexp = 0;
    }

    status = _bcm_caladan3_g3p1_policer_lp_program(unit,
                                                   info->policer_id,
                                                   &logical_port);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to program lp 0x%x policer 0x%x"
                               ": %d (%s)\n"),
                   lsr_label_info.lpi, info->policer_id,
                   status, bcm_errmsg(status)));
        return status;
    }

    status = soc_sbx_g3p1_lp_set(l3_fe->fe_unit, 
                                 lsr_label_info.lpi,
                                 &logical_port);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to set lp 0x%x: %d (%s)\n"),
                   lsr_label_info.lpi, status, bcm_errmsg(status)));
        return status;
    }

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
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(info->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
    }

    /*
     * No update is needed on remote module.
     * The calling function will call this function only if the port is
     * local. So no need to check again.
     */

    label1 = info->label;
    label2 = info->second_label;
#else
    port = 0;
    label1 = info->label;
#endif

#ifndef PLATFORM_LABELS
    for (index = 0; index < num_ports; index++) {
        if (is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }
#endif
        status = _bcm_caladan3_g3p1_mpls_labels_set(l3_fe->fe_unit,
                                                    port, 
                                                    label1, label2, label3,
                                                   &lsr_label_info);
        
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) setting label(0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(status), (int)info->label));
        }
#ifndef PLATFORM_LABELS
    } /* for (index = 0; index < num_ports; index++) */
#endif
    return status;
}

STATIC int
_bcm_caladan3_g3p1_mpls_map_set_ler_info(_caladan3_l3_fe_instance_t   *l3_fe,
                                         bcm_mpls_tunnel_switch_t *info,
                                         uint32                    base_vsi,
                                         uint32                    lpidx)
{
    soc_sbx_g3p1_labels_t       ler_info;
    int                         status;
    int                         unit = l3_fe->fe_unit;
    soc_sbx_g3p1_lp_t           logical_port;
    soc_sbx_control_t           *sbx;
    bcm_port_t                  port = -1;
    int                         label1 = 0, label2 = 0, label3 = 0;
#ifndef PLATFORM_LABELS
    bcm_module_t                modid = -1;
    uint32                      trunkid;
    bcm_trunk_add_info_t       *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;
#endif

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    status = BCM_E_NONE;
    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Setting l2e for label 0x%x as LER\n"),
               info->label));

    soc_sbx_g3p1_labels_t_init(&ler_info);
    if (lpidx == 0) {
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                              SBX_CALADAN3_USR_RES_MPLS_LPORT,
                                              1, &ler_info.lpi, 0);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                   "failed to allocate logical port: %d (%s)\n"),
                       status, bcm_errmsg(status)));
            return status;
        }
        soc_sbx_g3p1_lp_t_init(&logical_port);
    } else {
        ler_info.lpi = lpidx;
        soc_sbx_g3p1_lp_t_init(&logical_port);

        status = soc_sbx_g3p1_lp_get(l3_fe->fe_unit,
                                     ler_info.lpi,
                                     &logical_port);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to get lp 0x%x: %d %s\n"),
                       ler_info.lpi, status, bcm_errmsg(status)));
            return status;
        }
    }
    ler_info.opcode    =   _BCM_CALADAN3_LABEL_LER(unit);
    ler_info.vlan      =   base_vsi;
    if (info->flags & BCM_MPLS_SWITCH_DROP) {
        /* 
         * Note: Trigger forced drop by using STP drop
         *   This is a tunnel Lock, inorder to lock all pwe running
         *   over this, we set the opcode to lsr(Swap) and stpdrop enabled
         */
        ler_info.stpstate = sbx->stpblock;
        ler_info.opcode  = _BCM_CALADAN3_LABEL_LSR(unit);
        /*
         * Oam packets must be processed in a locked lsp
         * Since a locked ler resembles a lsr to the ucode
         * we need to provide additional info that its ler locked case.
         */
        ler_info.lock   = 0x3;
    }

    /* XXX: TBD:
     * L-LSP needs work here.
     */
    /* ler_info.elsp   =   1; */
    /* ler_info.pipe   =   1; */

    if (info->flags & BCM_MPLS_SWITCH_INT_PRI_MAP) {
        ler_info.elsp = 1;
        logical_port.qos = _MPLS_EXPMAP_HANDLE_DATA(info->exp_map);
        logical_port.useexp = 1;
    }
    if (info->flags & BCM_MPLS_SWITCH_INT_PRI_SET) {
        ler_info.elsp = 0;
        ler_info.cos = info->int_pri;
        logical_port.qos = _MPLS_EXPMAP_HANDLE_DATA(info->exp_map);
        logical_port.useexp = 1;
    }

    if (info->flags & BCM_MPLS_SWITCH_INNER_TTL) {
        ler_info.pipe = 1;
    }

    if(info->flags & BCM_MPLS_SWITCH_INNER_EXP) {
        logical_port.useexp = 0;
    }

    status = _bcm_caladan3_g3p1_policer_lp_program(unit,
                                                   info->policer_id,
                                                   &logical_port);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to program lp 0x%x policer 0x%x"
                               ": %d (%s)\n"),
                   logical_port.pid, info->policer_id,
                   status, bcm_errmsg(status)));
        return status;
    }

    status = soc_sbx_g3p1_lp_set(l3_fe->fe_unit, 
                                 ler_info.lpi,
                                 &logical_port);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Failed to set lp 0x%x: %d (%s)\n"),
                   ler_info.lpi, status, bcm_errmsg(status)));
        /* return status; */
    }

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
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(info->port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        num_ports = trunk_info->num_ports;
    }

    /*
     * No update is needed on remote module.
     * The calling function will call this function only if the port is
     * local. So no need to check again.
     */

    label1 = info->label;
    label2 = info->second_label;
#else
    port = 0;
    label1 = info->label;
#endif

#ifndef PLATFORM_LABELS
    for (index = 0; index < num_ports; index++) {
        if (is_trunk == 1) {
            port = trunk_info->tp[index];
            if (trunk_info->tm[index] != l3_fe->fe_my_modid) {
                continue;
            }
        }
#endif
        status = _bcm_caladan3_g3p1_mpls_labels_set(l3_fe->fe_unit,
                                          port, 
                                          label1, label2, label3,
                                          &ler_info);

        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) soc_sbx_g3p1_label2e_set label(0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(status), (int)info->label));
        } 
#ifndef PLATFORM_LABELS
    }
#endif

    if (BCM_SUCCESS(status)) {
        status = _bcm_caladan3_g3p1_mpls_property_table_set(l3_fe->fe_unit,
                                                            info->label,
                                                            1);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "Failed to set property table entry "
                                   " [0x%x]: %s\n"), info->label, bcm_errmsg(status)));
        }

    }

    return status;
}

int
_bcm_caladan3_g3p1_mpls_tunnel_switch_update(int                         unit,
                                             _caladan3_l3_fe_instance_t *l3_fe,
                                             bcm_mpls_tunnel_switch_t   *info)
{
    int                         status, ignore_status;
    int                         ii;
    int                         vsi_allocated;
    uint32                      *base_vsi = NULL;
    _caladan3_vpn_control_t     *vpnc;
    soc_sbx_g3p1_labels_t       mpls_label2etc;
    uint32                      restype = 0;
    uint32                      leren, ftidx, vlan, lpidx = 0, ftcnt = 0, flags;
    soc_sbx_control_t           *sbx;
    bcm_port_t                  port = -1;
    int                         label1 = 0, label2 = 0, label3 = 0;
#ifndef PLATFORM_LABELS
    bcm_module_t                modid = -1;
    uint32                      trunkid;
    bcm_trunk_add_info_t       *trunk_info = NULL;
    uint8                       index, is_trunk = 0, num_ports = 0;
#endif

    /* temporary fix warnings */
    leren = vlan = 0;
    

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter\n")));

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    vpnc          = NULL;
    base_vsi      = _CALADAN3_INVALID_VPN_ID;
    status        = BCM_E_NONE;
    vsi_allocated = 0;

    base_vsi = sal_alloc(sizeof(uint32) * l3_fe->fe_cosq_config_numcos,
                         "VSI array");
    if (base_vsi == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(base_vsi, 0, (sizeof(uint32) * l3_fe->fe_cosq_config_numcos));

    /*
     * in order to determine add vs. update case
     */

    soc_sbx_g3p1_labels_t_init(&mpls_label2etc);

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
            sal_free(base_vsi);
            return status;
        }
        num_ports = 1;
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(info->port);
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
        sal_free(base_vsi);
        return BCM_E_NONE;
    }

    label1 = info->label;
    label2 = info->second_label;
#else
    port = 0;
    label1 = info->label;
#endif

    status = _bcm_caladan3_g3p1_mpls_labels_get(unit, port,
                               label1, label2, label3,
                               &mpls_label2etc); 

    /*
     * Mapping between action and label2etc types
     *
     *  BCM_MPLS_SWITCH_ACTION_SWAP       --> SB_G2_FE_LABELTYPE_LSR
     *
     *  BCM_MPLS_SWITCH_ACTION_PHP        --> SB_G2_FE_LABELTYPE_LSR
     *                                        with ete.php = 1 and
     *                                        phpdecapethertype
     *
     *     labels table (formerly label2e)
     *           needs to have context for lookup VLAN (vrf 0)
     *
     *  BCM_MPLS_SWITCH_ACTION_POP        +-> SB_G2_FE_LABELTYPE_LER
     *                                  OR|
     *                                    +-> SB_G2_FE_LABELTYPE_PWE
     *     label-range will say if it is in the ler or pwe
     *
     */

    /**
     * XXX: TBD:
     * Counter api hookup needs to be implemented for
     * BCM_MPLS_SWITCH_COUNTED
     *
     */

    /*
     * MPLSTP LSP Lock
     *   When a lock is enabled on the LSP, we set the lock bit
     *   and change operation from LER to LSR, we need to revert that.
     *   This is required because LER operations ignores stpstate on
     *   outer tunnel label during POP
     */
    if ((mpls_label2etc.stpstate == sbx->stpblock) &&
        (info->action == BCM_MPLS_SWITCH_ACTION_POP) &&
        (mpls_label2etc.opcode == _BCM_CALADAN3_LABEL_LSR(unit))) {

        mpls_label2etc.opcode = _BCM_CALADAN3_LABEL_LER(unit);
    }
         
    leren = _BCM_CALADAN3_IS_LABEL_LER(unit, mpls_label2etc.opcode);
    vlan = mpls_label2etc.vlan;
    ftidx = mpls_label2etc.ftidx;
    lpidx = mpls_label2etc.lpi;
    ftcnt = 1;
    flags = 0;

    switch (info->action) {
    case BCM_MPLS_SWITCH_ACTION_SWAP:
    case BCM_MPLS_SWITCH_ACTION_PHP:
        if (leren == 0 &&
            (ftidx == 0 || 
             ftidx == SBX_DROP_FTE(unit)))
        {
            restype = SBX_CALADAN3_USR_RES_FTE_MPLS;
            status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                             restype,
                                             ftcnt,
                                             base_vsi,
                                             flags);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] failure(%s) to allocate vsi\n"),
                           l3_fe->fe_unit, FUNCTION_NAME(),
                           bcm_errmsg(status)));
                goto error_done;
            }

            vsi_allocated = 1;
            LOG_VERBOSE(BSL_LS_BCM_MPLS,
                        (BSL_META_U(unit,
                                    "base_vsi(0x%x) allocated\n"),
                         *base_vsi));
        } else if (leren == 0 &&
                   (ftidx  != SBX_DROP_FTE(unit))) {
            *base_vsi = ftidx;
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] invalid label type (%d) "
                                   "for label (0x%x) with action(%d)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       leren, (int)info->label,
                       info->action));
            status = BCM_E_PARAM;
            goto error_done;
        }

        status = _bcm_caladan3_g3p1_mpls_fte_copy_from_l3_egr(l3_fe,
                                                            info,
                                                            *base_vsi,
                                                            ftcnt);
        if (status != BCM_E_NONE) {
            goto error_done;
        }

        status = _bcm_caladan3_g3p1_mpls_ttl_update(l3_fe, info);
        if (status != BCM_E_NONE) {
            goto error_done;
        }

        status = _bcm_caladan3_g3p1_mpls_map_set_lsr_info(l3_fe,
                                                        info,
                                                        *base_vsi,
                                                        lpidx);
        if (status != BCM_E_NONE) {
            goto error_done;
        }
        break;
    case BCM_MPLS_SWITCH_ACTION_POP_DIRECT:
    case BCM_MPLS_SWITCH_ACTION_POP:
        if (info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT) {
            restype = SBX_CALADAN3_USR_RES_FTE_MPLS;
        } else {
            restype = SBX_CALADAN3_USR_RES_VSI;
        }

        if (leren == 0 &&
            (vlan == VLAN_INVALID || 
             ftidx == SBX_DROP_FTE(unit))) 
        {
            if (!((info->action == BCM_MPLS_SWITCH_ACTION_POP) && 
                  (info->vpn == l3_fe->fe_vsi_default_vrf))) {
                status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                 restype,
                                                 ftcnt,
                                                 base_vsi,
                                                 flags);
                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] failure(%s) to allocate vsi\n"),
                               l3_fe->fe_unit, FUNCTION_NAME(),
                               bcm_errmsg(status)));
                    goto error_done;
                }
                vsi_allocated = 1;
            } else {
                *base_vsi = l3_fe->fe_vsi_default_vrf;
            }
        } else if (leren) {
            if (info->action == BCM_MPLS_SWITCH_ACTION_POP) {
                *base_vsi = vlan;
            } else {
                *base_vsi = ftidx;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] invalid label type (%d) "
                                   "for label (0x%x) with action(%d)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       leren, (int)info->label,
                       info->action));
            status = BCM_E_PARAM;
            goto error_done;
        }

        if (info->action == BCM_MPLS_SWITCH_ACTION_POP && 
            info->vpn != l3_fe->fe_vsi_default_vrf) {
            status = _bcm_caladan3_find_mpls_vpncb_by_id(l3_fe->fe_unit,
                                                       l3_fe,
                                                       info->vpn,
                                                       &vpnc);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error(%s) finding vpn-control for vpn(0x%x)\n"),
                           l3_fe->fe_unit, FUNCTION_NAME(),
                           bcm_errmsg(status), info->vpn));
                goto error_done;
            }

            for (ii = 0; ii < ftcnt; ii++) {
                status = _bcm_caladan3_g3p1_mpls_map_set_vlan2etc(l3_fe,
                                                                vpnc,
                                                                (*base_vsi)+ii);
                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error(%s) setting vlan2etc for vsi(0x%x) vpn(0x%x)\n"),
                               l3_fe->fe_unit, FUNCTION_NAME(),
                               bcm_errmsg(status), (*base_vsi)+ii, info->vpn));
                    goto error_done;
                }
            }
        }

        if (info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT &&
            info->egress_if) {
            status = _bcm_caladan3_g3p1_mpls_fte_copy_from_l3_egr(l3_fe,
                                                                info,
                                                                *base_vsi,
                                                                ftcnt);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error(%s) fte-copy-from-l3-egress base-vsi(0x%x) vpn(0x%x)\n"),
                           l3_fe->fe_unit, FUNCTION_NAME(),
                           bcm_errmsg(status), *base_vsi, info->vpn));
                goto error_done;
            }
        }

        status = _bcm_caladan3_g3p1_mpls_map_set_ler_info(l3_fe,
                                                        info,
                                                        *base_vsi, lpidx);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) map-set-ler information base-vsi(0x%x) vpn(0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(status), *base_vsi, info->vpn));
            goto error_done;
        }
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s]  error(%s) in reading label (0x%x) "
                               "info from HW\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   bcm_errmsg(status), (int)info->label));
        status = BCM_E_PARAM;
        goto error_done;
    }

    sal_free(base_vsi);
    return BCM_E_NONE;

error_done:
    if (vsi_allocated) {
        ignore_status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                               restype,
                                               ftcnt,
                                               base_vsi,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit: %d: [%s] failure(%s) releasing %d allocated vsi(0x%x-0x%x)\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(ignore_status),
                       ftcnt,
                       (*base_vsi),
                       (*base_vsi) + ftcnt - 1));
        }
    }

    sal_free(base_vsi);
    return status;
}

STATIC int
_bcm_caladan3_g3p1_mpls_tunnel_initiator_modify(_caladan3_l3_fe_instance_t *l3_fe,
                                                _caladan3_l3_intf_t      *l3_intf,
                                                _caladan3_l3_ete_t        *v4_ete,
                                                int                  action_flags,
                                                _caladan3_l3_ete_t      *mpls_ete)
{
    int                        ete_hash_idx, status;
    soc_sbx_g3p1_ete_t         mpls_hw_ete;
    _caladan3_vpn_sap_t       *vpn_sap = NULL;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit, "Enter\n")));
    /*
     * Note: This function is used to modify the following attrs of mpls ete
     * 1. dmac
     * 2. intf related params.
     * For any label related changes, we use the initiator_set API
     */

    if (mpls_ete == NULL) {
        return BCM_E_PARAM;
    }

    status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                       mpls_ete->l3_ete_hw_idx.ete_idx,
                                       &mpls_hw_ete);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) reading mpls-ete(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status),
                   mpls_ete->l3_ete_hw_idx.ete_idx));
        return status;
    }

    if (action_flags & _CALADAN3_MPLS_ETE_MODIFY__UCAST_IP_ETE_CHANGED) {
        mpls_hw_ete.dmac5 = v4_ete->l3_ete_key.l3_ete_hk.dmac[5];
        mpls_hw_ete.dmac4 = v4_ete->l3_ete_key.l3_ete_hk.dmac[4];
        mpls_hw_ete.dmac3 = v4_ete->l3_ete_key.l3_ete_hk.dmac[3];
        mpls_hw_ete.dmac2 = v4_ete->l3_ete_key.l3_ete_hk.dmac[2];
        mpls_hw_ete.dmac1 = v4_ete->l3_ete_key.l3_ete_hk.dmac[1];
        mpls_hw_ete.dmac0 = v4_ete->l3_ete_key.l3_ete_hk.dmac[0];

        /*
         * First update HW. If that is successful, only then update SW state
         */
        status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                           mpls_ete->l3_ete_hw_idx.ete_idx,
                                           &mpls_hw_ete);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META("unit %d: [%s] error %s in setting MPLS ete in HW"),
                       l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }

        /*
         * The dmac and hence the has key has changed. Remove from
         * old hash bucket and add to the new one
         */

        DQ_REMOVE(&mpls_ete->l3_ieh_link);
        _CALADAN3_MAKE_ENCAP_MPLS_SW_ETE_KEY(&mpls_ete->l3_ete_key,
                                         v4_ete->l3_ete_key.l3_ete_hk.dmac);
        _CALADAN3_CALC_INTF_L3ETE_HASH(ete_hash_idx,
                                   mpls_ete->l3_ete_key.l3_ete_hk.type,
                                   mpls_ete->l3_ete_key.l3_ete_hk.dmac);
        DQ_INSERT_HEAD(&l3_intf->if_ete_hash[ete_hash_idx],
                       &mpls_ete->l3_ieh_link);

    }

    /*
     * we need to update all VC etes that depend
     * on this mpls encap ete
     */
    _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE(mpls_ete, vpn_sap) {
        _caladan3_vpn_control_t  *vpnc;
        int                  num_vpn_ports = 0;
        int                  reconnect = 0;
        _caladan3_vpn_sap_t      *tmp_vpn_sap = NULL, *vpnSaps[_BCM_CALADAN3_VPWS_MAX_SAP];

        vpnc = vpn_sap->vc_vpnc;
        
        if (vpnc && (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) &&
             !(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_ALL_FAILOVERS)) {

            _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
                if (!(tmp_vpn_sap->vc_mpls_port.flags & 
                                         BCM_MPLS_PORT_ALL_FAILOVERS)) {
                    if (num_vpn_ports < _BCM_CALADAN3_VPWS_MAX_SAP) {
                        vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                    }
                    num_vpn_ports++;
                }
            } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

            if (num_vpn_ports == 2) {
                status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                             _CALADAN3_MPLS_PORT_MATCH_UPDATE, 0);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MPLS,
                              (BSL_META_U(l3_fe->fe_unit, "error(%s) failed to unlink VPWS\n"),
                               bcm_errmsg(status)));
                } else {
                    reconnect = 1;
                }
            }
        }
        _bcm_caladan3_mpls_g3p1_map_tunnel_to_vc_egress(l3_fe, vpn_sap,
                                                      &vpn_sap->vc_mpls_port,
                                                        &mpls_hw_ete); 
        /* Cross-connect vpws again after update !*/
        if (reconnect) {
              status = _bcm_caladan3_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                           _CALADAN3_MPLS_PORT_MATCH_UPDATE, 1);
              if (BCM_FAILURE(status)) {
                  LOG_ERROR(BSL_LS_BCM_MPLS,
                            (BSL_META_U(l3_fe->fe_unit, "error(%s) failed to link VPWS\n"),
                             bcm_errmsg(status)));
              }
        }

    } _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE_END(mpls_ete, vpn_sap);

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_enable_mpls_tunnel(_caladan3_l3_fe_instance_t *l3_fe,
                                      _caladan3_l3_intf_t        *l3_intf,
                                      _caladan3_l3_ete_t         *v4_ete)
{
    int                            status;
    _caladan3_l3_ete_t                *mpls_ete;
    soc_sbx_g3p1_oi2e_t            ohi2etc;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(l3_fe->fe_unit, "Enter\n")));

    status = _bcm_caladan3_get_ete_by_type_on_intf(l3_fe,
                                                   l3_intf,
                                                   _CALADAN3_L3_ETE__ENCAP_MPLS,
                                                   &mpls_ete);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] mpls tunnel ete not found on interface\n"),
                   l3_fe->fe_unit, FUNCTION_NAME()));
        return status;
    }
    LOG_VERBOSE(BSL_LS_BCM_MPLS,
                (BSL_META_U(l3_fe->fe_unit, "enabling mpls tunnel for v4 ete (0x%x) on "
                 "Interface (0x%x) \n"),
                 (v4_ete)->l3_ete_hw_idx.ete_idx,
                 l3_intf->if_info.l3a_intf_id));

    status = _bcm_caladan3_g3p1_mpls_tunnel_initiator_modify(l3_fe,
                                                           l3_intf,
                                                           v4_ete,
                                     _CALADAN3_MPLS_ETE_MODIFY__UCAST_IP_ETE_CHANGED,
                                                           mpls_ete);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] tunnel ete not be enabled (could not set DMAC)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME()));
        return status;
    }

    soc_sbx_g3p1_oi2e_t_init(&ohi2etc);
    ohi2etc.eteptr = mpls_ete->l3_ete_hw_idx.ete_idx;
    status = soc_sbx_g3p1_oi2e_set(l3_fe->fe_unit,
                                   _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(v4_ete->l3_ohi.ohi),
                                   &ohi2etc);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META("unit %d: [%s] error(%s) in mapping ohi (0x%x) "
                   "to MPLS ete (0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status),
                   v4_ete->l3_ohi.ohi, ohi2etc.eteptr));
    }

    return status;
}

/*
 *   Function
 *      bcm_caladan3_mpls_port_qosmap_set
 *   Purpose
 *      Set QoS mapping behaviour on an MPLS GPORT
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_gport_t gport = MPLS GPORT
 *      (in) int ingrMap    = ingress map
 *      (in) int egrMap     = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     Little parameter checking is done here.
 */
int
bcm_caladan3_mpls_port_qosmap_set(int unit,
                                bcm_gport_t gport,
                                int ingrMap,
                                int egrMap,
                                uint32 ingFlags, 
                                uint32 egrFlags)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    _caladan3_vpn_control_t *vpnc = NULL;
    _caladan3_vpn_sap_t *vpn_sap = NULL;
    _caladan3_vpn_sap_t *vpn_sap_temp = NULL;
    soc_sbx_g3p1_lp_t p3lp;
    soc_sbx_g3p1_ft_t p3ft;
    soc_sbx_g3p1_oi2e_t p3oi2e;
    soc_sbx_g3p1_ete_t p3eteEncap;
    bcm_port_t phy_port;
    bcm_module_t mymodid;
    bcm_module_t exit_modid = -1;
    uint32     trunkid;
    bcm_trunk_add_info_t *trunk_info;
    int rv = BCM_E_INTERNAL;
    int found = FALSE;
    uint32 ohi;
    int i;

    LOG_DEBUG(BSL_LS_BCM_MPLS,
              (BSL_META_U(unit,
                          "Enter : gport 0x%x, ingrMap 0x%x, egrMap 0x%x\n"),
               gport, ingrMap, egrMap));

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s] mpls is not initialized\n"),
                   unit, FUNCTION_NAME()));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    rv = bcm_stk_my_modid_get(unit, &mymodid);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s] bcm_stk_my_modid_get failed with %d: %s\n"),
                   unit, FUNCTION_NAME(), rv, bcm_errmsg(rv)));
        L3_UNLOCK(unit);
        return rv;
    }

    _CALADAN3_ALL_VPNC(l3_fe, vpnc) {
        _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap_temp) {
            if (gport == vpn_sap_temp->vc_mpls_port_id) {
                vpn_sap = vpn_sap_temp;
                found = TRUE;
                break;
            }
        } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap_temp);
        if (found) {
            break;
        }
    } _CALADAN3_ALL_VPNC_END(l3_fe, vpnc);

    if (!found || vpn_sap == NULL) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s] mpls gport 0x%x not found\n"),
                   unit, FUNCTION_NAME(), gport));
        L3_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    } else {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "found vpnsap for gport: 0x%x, "
                               "vpnsap->vc_mpls_port_id:0x%x, vpnsap->logicalPort:0x%x"
                               "vpnsap->vc_vpnc->vpn_id:0x%x\n"),
                   gport, vpn_sap->vc_mpls_port_id,
                   vpn_sap->logicalPort, vpn_sap->vc_vpnc->vpn_id));

    }

    if (BCM_GPORT_IS_MODPORT(vpn_sap->vc_mpls_port.port)) {
        rv = _bcm_caladan3_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                 vpn_sap->vc_mpls_port.port,
                                                 &exit_modid,
                                                 &phy_port);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) extracting mod-port from gport\n"),
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       bcm_errmsg(rv)));
            L3_UNLOCK(unit);
            return rv;
        }
    } else if (BCM_GPORT_IS_LOCAL(vpn_sap->vc_mpls_port.port)) {
        phy_port = BCM_GPORT_LOCAL_GET(vpn_sap->vc_mpls_port.port);
        exit_modid = mymodid;
    } else if (SOC_PORT_VALID(unit, vpn_sap->vc_mpls_port.port)) {
        phy_port = vpn_sap->vc_mpls_port.port;
        exit_modid = mymodid;
    } else if (BCM_GPORT_IS_TRUNK(vpn_sap->vc_mpls_port.port)) {
        trunkid = BCM_GPORT_TRUNK_GET(vpn_sap->vc_mpls_port.port);
        trunk_info = &(mpls_trunk_assoc_info[l3_fe->fe_unit][trunkid].add_info);
        for (i = 0; i < trunk_info->num_ports; i++) {
            if (trunk_info->tm[i] == mymodid) {
                exit_modid = mymodid;
                break;
            }
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s] attempting to update an invalid MPLS gport\n"),
                   l3_fe->fe_unit, FUNCTION_NAME()));
        L3_UNLOCK(unit);
        return BCM_E_PORT;
    }
    

    /* follow the egress path */
    if ((BCM_E_NONE == rv) && (egrMap >= 0) && (mymodid == exit_modid)) {
        rv = soc_sbx_g3p1_ft_get(unit,
                                 BCM_GPORT_MPLS_PORT_ID_GET(gport),
                                 &p3ft);
        if (BCM_E_NONE == rv) {
            ohi = _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(p3ft.oi);
            rv = soc_sbx_g3p1_oi2e_get(unit, ohi, &p3oi2e);
            if (BCM_E_NONE == rv) {
                if (p3oi2e.eteptr) {
                    /* has egress path; follow it */
                    rv = soc_sbx_g3p1_ete_get(unit,
                                              p3oi2e.eteptr,
                                              &p3eteEncap);
                    if (BCM_E_NONE != rv) {
                        LOG_ERROR(BSL_LS_BCM_MPLS,
                                  (BSL_META_U(unit,
                                              "unable to read %d:ete[%08X].encap:"
                                               " %d (%s)\n"),
                                   unit,
                                   p3oi2e.eteptr,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                    }
                } else {
                    /* no egress path -- hasn't been added yet? */
                    LOG_WARN(BSL_LS_BCM_MPLS,
                             (BSL_META_U(unit,
                                         "unable to follow egress path for unit %d"
                                          " gport 0x%08X because it  has no egress path;"
                                          " not added yet?\n"),
                              unit, gport));
                }
            } else { /* if (BCM_E_NONE == rv) */
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to read %d:oi2e[%08X]: %d (%s)\n"),
                           unit,
                           p3ft.oi,
                           rv,
                           _SHR_ERRMSG(rv)));
            } /* if (BCM_E_NONE == rv) */
        } else { /* if (BCM_E_NONE == rv) */
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unable to read %d:ft[%08X]: %d (%s)\n"),
                       unit,
                       BCM_GPORT_MPLS_PORT_ID_GET(gport),
                       rv,
                       _SHR_ERRMSG(rv)));
        } /* if (BCM_E_NONE == rv) */
        if ((BCM_E_NONE == rv) && p3oi2e.eteptr) {
            /* update if successfully followed egress path */
            p3eteEncap.remark = egrMap;
            p3eteEncap.dscpremark = (egrFlags & BCM_QOS_MAP_L3)?1:0;
            rv = soc_sbx_g3p1_ete_set(unit, p3oi2e.eteptr, &p3eteEncap);
            if (BCM_E_NONE != rv) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to write %d:ete[%08X].l2: %d (%s)\n"),
                           unit,
                           p3oi2e.eteptr,
                           rv,
                           _SHR_ERRMSG(rv)));
            }
        } /* if (BCM_E_NONE == rv) */
    } /* if ((BCM_E_NONE == rv) && (egrMap >=0 ) && (mymodid == exit_modid)) */

    /* update the ingress path */
    if ((BCM_E_NONE == rv) && (ingrMap >= 0) && (mymodid == exit_modid)) {
        /* get the logical port (ingress map is here) */
        rv = soc_sbx_g3p1_lp_get(unit, vpn_sap->logicalPort, &p3lp);
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_MPLS,
                      (BSL_META_U(unit,
                                  "unable to read %d:lp[%08X]: %d (%s)\n"),
                       unit,
                       vpn_sap->logicalPort,
                       rv,
                       _SHR_ERRMSG(rv)));
        }

        if (BCM_E_NONE == rv) {
            p3lp.qos = ingrMap;
            p3lp.usedscp = (ingFlags & BCM_QOS_MAP_L3)?1:0;
            p3lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS)?1:0;
            rv = soc_sbx_g3p1_lp_set(unit, vpn_sap->logicalPort, &p3lp);
            if (BCM_E_NONE != rv) {
                LOG_ERROR(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "unable to write %d:lp[%08X]: %d (%s)\n"),
                           unit,
                           vpn_sap->logicalPort,
                           rv,
                           _SHR_ERRMSG(rv)));
            } else {
                LOG_DEBUG(BSL_LS_BCM_MPLS,
                          (BSL_META_U(unit,
                                      "Updated lp [0x%x] with qos [0x%x] map\n"),
                           vpn_sap->logicalPort, ingrMap));
            }
        } /* if (BCM_E_NONE == rv) */

    } /* if ((BCM_E_NONE == rv && (ingrMap >= 0) && (mymodid == exit_modid)) */

    L3_UNLOCK(unit);
    return rv;
}


/*
 *   Function
 *      bcm_caladan3_mpls_port_qosmap_get
 *   Purpose
 *      Set QoS mapping on an MPLS GPORT
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_gport_t gport = MPLS GPORT
 *      (out) int ing_idx    = qos profile
 *      (out) int egr_idx     = remark index
 *   Returns
 *      BCM_E_*
 */
int
bcm_caladan3_mpls_port_qosmap_get(int unit, bcm_gport_t gport,
                                  int *ing_idx, int *egr_idx,
                                  uint32 *ing_flags, uint32 *egr_flags)
{
    int rv = BCM_E_UNAVAIL;
    
    return rv;
}

/*
 *   Function
 *      bcm_caladan3_g3p1_mpls_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided MPLS GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = MPLS GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

int
bcm_caladan3_g3p1_mpls_port_get_lpid(int unit,
                                   bcm_gport_t gport,
                                   uint32 *lpid,
                                   bcm_port_t *pport)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    _caladan3_vpn_control_t *vpnc = NULL;
    _caladan3_vpn_sap_t *vpn_sap = NULL;
    int result = BCM_E_NOT_FOUND;

    if ((!lpid) || (!pport)) {
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_MPLS_PORT(gport)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        result = BCM_E_UNIT;
    } else if (!_CALADAN3_IS_MPLS_INITIALIZED(l3_fe)) {
        LOG_ERROR(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "unit %d: [%s] mpls is not initialized\n"),
                   unit, FUNCTION_NAME()));
        result = BCM_E_INIT;
    } else {
        _CALADAN3_ALL_VPNC(l3_fe, vpnc) {
            _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                if (gport == vpn_sap->vc_mpls_port_id) {
                    *lpid = vpn_sap->logicalPort;
                    *pport = vpn_sap->vc_mpls_port.port;
                    result = BCM_E_NONE;
                    break;
                }
            } _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);
            if (BCM_E_NONE == result) {
                break;
            }
        } _CALADAN3_ALL_VPNC_END(l3_fe, vpnc);
    }

    L3_UNLOCK(unit);
    return result;
}

/*
 * Set the MPLS property table
 */

int
_bcm_caladan3_g3p1_mpls_property_table_set(int unit,
                                           bcm_mpls_label_t label,
                                           int value)
{
    return soc_sbx_g3p1_ppe_property_table_segment_set(unit, 0, label, value);
}

int
_bcm_caladan3_g3p1_mpls_labels_set(int unit,
                                   int port,
                                   int label1,
                                   int label2,
                                   int label3,
                                   soc_sbx_g3p1_labels_t *labels)
{
    int rv = 0;
    if (label3 == 0) {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Adding labels - %x, %x, %x\n"),
                   port, label1, label2));
        rv = soc_sbx_g3p1_labels_set(unit, label2, 0, port, label1, labels);
    } else {
        LOG_DEBUG(BSL_LS_BCM_MPLS,
                  (BSL_META_U(unit,
                              "Adding labels3 - %x, %x, %x, %x\n"),
                   port, label1, label2, label3));
        rv = soc_sbx_g3p1_labels3_set(unit, label3, label2, label1, 0, port,
                                            (soc_sbx_g3p1_labels3_t *)labels);
    }
    return rv;
}

int
_bcm_caladan3_g3p1_mpls_labels_get(int unit,
                                   int port,
                                   int label1,
                                   int label2,
                                   int label3,
                                   soc_sbx_g3p1_labels_t *labels)
{
    int rv = 0;
    if (label3 == 0) {
        rv = soc_sbx_g3p1_labels_get(unit, label2, 0, port, label1, labels);
    } else {
        rv = soc_sbx_g3p1_labels3_get(unit, label3, label2, label1, 0, port,
                                            (soc_sbx_g3p1_labels3_t *)labels);
    }
    return rv;
}

int
_bcm_caladan3_g3p1_mpls_labels_delete(int unit,
                                      int port,
                                      int label1,
                                      int label2,
                                      int label3)
{
    int rv = 0;
    if (label3 == 0) {
        rv = soc_sbx_g3p1_labels_delete(unit, label2, 0, port, label1);
    } else {
        rv = soc_sbx_g3p1_labels3_delete(unit, label3, label2, label1, 0, port);
    }
    return rv;
}

int
_bcm_caladan3_g3p1_mpls_labels_update(int unit,
                                      int port,
                                      int label1,
                                      int label2,
                                      int label3,
                                      soc_sbx_g3p1_labels_t *labels)
{
    int rv = 0;
    if (label3 == 0) {
        rv = soc_sbx_g3p1_labels_update(unit, label2, 0, port, label1, labels);
    } else {
        rv = soc_sbx_g3p1_labels3_update(unit, label3, label2, label1, 0, port,
                                            (soc_sbx_g3p1_labels3_t *) labels);
    }
    return rv;
}

int
_bcm_caladan3_g3p1_mpls_labels_first(int unit,
                                     int *port,
                                     int *label1,
                                     int *label2,
                                     int *label3)
{
    int rv = 0;
    int dummy = 0;
    if (label3 == 0) {
        rv = soc_sbx_g3p1_labels_first(unit, label2, &dummy, port, label1);
    } else {
        rv = soc_sbx_g3p1_labels3_first(unit, 
                                 label3, label2, label1, &dummy, port);
    }
    return rv;
}

int
_bcm_caladan3_g3p1_mpls_labels_next(int unit,
                          int port,   int   label1, int   label2, int   label3,
                          int *nport, int *nlabel1, int *nlabel2, int *nlabel3)
{
    int rv = 0;
    int dummy = 0;
    if (label3 == 0) {
        rv = soc_sbx_g3p1_labels_next(unit, label2, 0, port, label1, 
                                      nlabel2, &dummy, nport, nlabel1);
    } else {
        rv = soc_sbx_g3p1_labels3_next(unit,
                                       label3,  label2,  label1,   0,    port, 
                                      nlabel3, nlabel2, nlabel1, &dummy, nport);
    }
    return rv;
}


#endif
