/*
 * $Id: mim.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    mim.c
 * Purpose: Manages MIM Provider Back bone Bridging functions for CALADAN3
 */
#if defined(INCLUDE_L3)

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/vlan.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/mim.h>
#include <bcm_int/sbx/caladan3/trunk.h>
#include <bcm/qos.h>
#include <soc/sbx/sbDq.h>
#include <shared/avl.h>
#include <shared/idxres_fl.h>
#include <bcm/policer.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/wb_db_mim.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <shared/idxres_afl.h>
#include <bcm_int/sbx/caladan3/policer.h>
 
#define MAC_FMT    "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_PFMT(m) m[0], m[1], m[2], m[3], m[4], m[5]


bcm_caladan3_mim_trunk_association_t caladan3_mim_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

/* data needed to convert AVL traversal to bcm traversal */
typedef struct _bcm_fe200_mim_vpn_cb_data_s {
    int                     unit;
    bcm_mim_vpn_traverse_cb cb;
    void                    *userData;
} _bcm_caladan3_mim_vpn_cb_data_t;

_bcm_caladan3_mimgp_info_t caladan3_gportinfo[BCM_MAX_NUM_UNITS];
shr_avl_t   *caladan3_mim_vpn_db[BCM_MAX_NUM_UNITS];
sal_mutex_t  mim_mlock[BCM_MAX_NUM_UNITS];

#define _MIM_INVALID_VPN_ID (0)
#define _MIM_INVALID_ISID   (-1)

#define _MIM_UNIT_INVALID(unit) \
    (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS))

#define _MIM_PORT_TYPE_VALID_CHECK(type) \
    (((type) >= _BCM_CALADAN3_MIM_ACCESS_PORT) || ((type) < _BCM_CALADAN3_MIM_PORT_MAX))


#define _MIM_PORT_INVALID(unit, port) \
           (((port) < 0) || ((port) >= SBX_MAX_PORTS) || \
             ((port) >= SOC_SBX_CFG_CALADAN3(unit)->numUcodePorts))

#define _MIM_LOCK_CREATED_ALREADY(unit) \
    (mim_mlock[(unit)] != NULL)

#define _MIM_LOCK(unit)       \
    (_MIM_LOCK_CREATED_ALREADY(unit)? \
     sal_mutex_take(mim_mlock[(unit)], sal_mutex_FOREVER)?BCM_E_UNIT:BCM_E_NONE: \
     BCM_E_UNIT)

#define _MIM_UNLOCK(unit)     \
    sal_mutex_give(mim_mlock[(unit)])


#define _BCM_CALADAN3_MIM_SUPPORTED_FLAGS (BCM_MIM_VPN_MIM | BCM_MIM_VPN_WITH_ID)

#define G3P1_ONLY_SUPPORT_CHECK(unit)                                                \
    if(!SOC_IS_SBX_G3P1((unit))) {                                                   \
        LOG_WARN(BSL_LS_BCM_MIM, \
                 (BSL_META_U(unit, \
                             "WARNING %s is supported only for G2P3(%s,%d)\n"), \
                  FUNCTION_NAME(), __FILE__, __LINE__));                 \
        return BCM_E_UNAVAIL;                                                        \
    }

#define _MIM_VPN_INIT_CHECK(unit)                                                   \
    if(!caladan3_mim_vpn_db[(unit)]) {                                                       \
        _MIM_UNLOCK(unit);                                                          \
        LOG_WARN(BSL_LS_BCM_MIM, \
                 (BSL_META_U(unit, \
                             "WARNING %s VPN sussystem not initialized(%s,%d)\n"), \
                  FUNCTION_NAME(), __FILE__, __LINE__));                \
        return BCM_E_UNAVAIL;                                                       \
    }


#define _BCM_CALADAN3_FTIDX_2_MIM_PORTID(unit, ftidx) ((ftidx) - SBX_GLOBAL_GPORT_FTE_BASE(unit))
#define _BCM_CALADAN3_MIM_PORTID_2_FTIDX(unit, mimportid) ((mimportid) + SBX_GLOBAL_GPORT_FTE_BASE(unit))

#define G3P1_GPORT_RANGE_CHECK(unit, gportid)                                 \
    do {                                                                      \
        if(BCM_GPORT_IS_MIM_PORT(gportid)) {                                  \
            uint32 _pid = BCM_GPORT_MIM_PORT_ID_GET(gportid);                 \
            if(_pid > (SBX_GLOBAL_GPORT_FTE_END(unit) -                       \
                        SBX_GLOBAL_GPORT_FTE_BASE(unit))) {                   \
                LOG_WARN(BSL_LS_BCM_MIM, \
                         (BSL_META_U(unit, \
                                     "WARNING %s BAD MiM GPORT ID specified(%s,%d)\n"), \
                          FUNCTION_NAME(), __FILE__, __LINE__));          \
                return BCM_E_PARAM;                                           \
            }                                                                 \
        } else {                                                              \
            LOG_WARN(BSL_LS_BCM_MIM, \
                     (BSL_META_U(unit, \
                                 "WARNING %s Not Mim Gport (%s,%d)\n"), \
                      FUNCTION_NAME(), __FILE__, __LINE__));          \
            return BCM_E_PARAM;                                               \
        }                                                                     \
    } while(0)

#define  _BCM_CALADAN3_MIM_SUPPORTED_PORT_FLAGS (BCM_MIM_PORT_TYPE_BACKBONE |            \
                                               BCM_MIM_PORT_TYPE_ACCESS   |            \
                                               BCM_MIM_PORT_WITH_ID       |            \
                                               BCM_MIM_PORT_ENCAP_WITH_ID |            \
                                               BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD)

#define  _BCM_CALADAN3_MIM_SUPPORTED_CRITERIA   (BCM_MIM_PORT_MATCH_PORT|                \
                                               BCM_MIM_PORT_MATCH_PORT_VLAN|           \
                                               BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC)

#define  _BCM_CALADAN3_MIM_BB_SUPPORTED_FLAGS   (BCM_MIM_PORT_TYPE_BACKBONE |            \
                                               BCM_MIM_PORT_WITH_ID       |            \
                                               BCM_MIM_PORT_ENCAP_WITH_ID )

#define  _BCM_CALADAN3_MIM_BB_SUPPORTED_CRITERIA (BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC)

#define  _BCM_CALADAN3_MIM_ACCESS_SUPPORTED_FLAGS (BCM_MIM_PORT_TYPE_ACCESS|             \
                                                 BCM_MIM_PORT_WITH_ID    |             \
                                                 BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD |\
                                                 BCM_MIM_PORT_ENCAP_WITH_ID )

#define  _BCM_CALADAN3_MIM_ACCESS_SUPPORTED_CRITERIA (BCM_MIM_PORT_MATCH_PORT|           \
                                                    BCM_MIM_PORT_MATCH_PORT_VLAN)

#define  _BCM_CALADAN3_MIM_SUPPORTED_UPDATE_FLAGS (BCM_MIM_PORT_WITH_ID | \
                                                 BCM_MIM_PORT_REPLACE | \
                                                 BCM_MIM_PORT_ENCAP_WITH_ID)


#define _BCM_CALADAN3_IS_MAC_EQUAL(m1,m2) \
            ((((m1)[0] == (m2)[0]) && \
             ((m1)[1] == (m2)[1]) && \
             ((m1)[2] == (m2)[2]) && \
             ((m1)[3] == (m2)[3]) && \
             ((m1)[4] == (m2)[4]) && \
             ((m1)[5] == (m2)[5])) ? 1:0)

#define  _BCM_CALADAN3_G3P1_ADJUST_TB_OFFSET(ohi) ((ohi) - 8*1024)

#define  _BCM_CALADAN3_MIM_ETYPE (0x88e7)
#define  _BCM_CALADAN3_DEFAULT_BTAG_TPID (0x88A8)

extern int bcm_sbx_stk_modid_get(int,int *);

#define _AVL_EMPTY(avl) ((shr_avl_count(avl) > 0)?0:1)

/*-------- Static Functions Start -------*/

STATIC int
_bcm_caladan3_mim_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data);

int 
_bcm_caladan3_mim_set_egress_remarking(int unit, 
                                     uint32 eteidx, 
                                     uint32 egrMap,
                                     uint32 egrFlags);

/* Translate a bcm_caladan3_mim_vpn_control_t to a bcm_mim_vpn_config_t */
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static void
_bcm_caladan3_mim_vpn_control_xlate(bcm_caladan3_mim_vpn_control_t *vpnCtl,
                                  bcm_mim_vpn_config_t  *vpnConfig)
{
    
    vpnConfig->vpn       = vpnCtl->vpnid;
    vpnConfig->lookup_id = vpnCtl->isid;
    vpnConfig->broadcast_group         = vpnCtl->broadcast_group;
    vpnConfig->unknown_unicast_group   = vpnCtl->unknown_unicast_group;
    vpnConfig->unknown_multicast_group = vpnCtl->unknown_multicast_group;
}
#endif

/* Translate a bcm_caladan3_mim_port_control_t to a bcm_mim_port_t */
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static void
_bcm_caladan3_mim_port_control_xlate(bcm_caladan3_mim_port_control_t *portCtl,
                                   bcm_mim_port_t *mimPort)
{
    
    mimPort->mim_port_id = portCtl->gport;
    mimPort->port        = portCtl->port;
    sal_memcpy(mimPort->egress_tunnel_srcmac, portCtl->smac, sizeof(bcm_mac_t));
    sal_memcpy(mimPort->egress_tunnel_dstmac, portCtl->dmac, sizeof(bcm_mac_t));

    mimPort->match_vlan            = portCtl->vlan;
    mimPort->egress_service_vlan   = portCtl->vlan;
    mimPort->egress_service_tpid   = portCtl->tpid;
    mimPort->egress_tunnel_service = portCtl->isid;
    mimPort->encap_id              = SOC_SBX_ENCAP_ID_FROM_OHI(portCtl->hwinfo.ohi);
    mimPort->criteria              = portCtl->criteria;
    mimPort->flags                 = portCtl->flags;
}
#endif

/* VPN Database Tree compare function using VPN ID */
/* VPNDB Insert compare function */
#if defined(BCM_CALADAN3_MIM_WARM_BOOT_DEBUG) || defined(BCM_WARM_BOOT_SUPPORT)
int _bcm_caladan3_mim_insert_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_caladan3_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_caladan3_mim_vpn_control_t*)datum1;
    data2 = (bcm_caladan3_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        return -1;
    }

    if(data1->vpnid > data2->vpnid) {
        return 1;
    }

    return(0);
}
#endif


/* AVL lookup compare function. Compares both VPNID and ISID
 * to see if there is another entry on the Tree using either the
 * same VPNID or ISID */
#if defined(BCM_CALADAN3_MIM_WARM_BOOT_DEBUG) || defined(BCM_WARM_BOOT_SUPPORT)
int _bcm_caladan3_mim_vpn_lookup_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2,
                                          void *lkupdata)
{
    bcm_caladan3_mim_vpn_control_t *data1, *data2;
    void *vp = datum2; /* grab the pointer to AVL tree datum element */

    data1 = (bcm_caladan3_mim_vpn_control_t*)datum1;
    data2 = (bcm_caladan3_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        if(data1->isid == data2->isid){
            /* the VPN we are looking for exists, get its address */
            *(((_bcm_caladan3_mim_lookup_data_t*)lkupdata)->datum) = \
                ( bcm_caladan3_mim_vpn_control_t*)vp;
            return 0;
        } else {
            return -1;
        }
    } else if(data1->vpnid > data2->vpnid) {
        if(data1->isid == data2->isid){
            /* the VPN we are looking for exists, get its address */
            *(((_bcm_caladan3_mim_lookup_data_t*)lkupdata)->datum) = \
                ( bcm_caladan3_mim_vpn_control_t*)vp;
            return 0;
        } else {
            return 1;
        }
    } else { /* data1->vpnid == data2->vpnid */
        /* the VPN we are looking for exists, get its address */
        *(((_bcm_caladan3_mim_lookup_data_t*)lkupdata)->datum) = \
            ( bcm_caladan3_mim_vpn_control_t*)vp;
        return 0;
    }
}
#endif

static int _bcm_caladan3_mim_vpn_copy_datum(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_caladan3_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_caladan3_mim_vpn_control_t*)datum1;
    data2 = (bcm_caladan3_mim_vpn_control_t*)datum2;

    sal_memcpy(datum1, datum2, sizeof(bcm_caladan3_mim_vpn_control_t));

    /* carefully manipute list headers */
    if(DQ_EMPTY(&data2->vpn_access_sap_head)) {
        DQ_INIT(&data1->vpn_access_sap_head);
    } else {
        DQ_SWAP_HEAD(&data1->vpn_access_sap_head, &data2->vpn_access_sap_head);
    }

    if(DQ_EMPTY(&data2->vpn_bbone_sap_head)) {
        DQ_INIT(&data1->vpn_bbone_sap_head);
    } else {
        DQ_SWAP_HEAD(&data1->vpn_bbone_sap_head, &data2->vpn_bbone_sap_head);
    }

    if(DQ_EMPTY(&data2->def_bbone_plist)) {
        DQ_INIT(&data1->def_bbone_plist);
    } else {
        DQ_SWAP_HEAD(&data1->def_bbone_plist, &data2->def_bbone_plist);
    }
    return 0;
}

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static int _bcm_caladan3_mim_vpn_lookup_compare_nodata(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_caladan3_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_caladan3_mim_vpn_control_t*)datum1;
    data2 = (bcm_caladan3_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        if(data1->isid == data2->isid){
            return 0;
        } else {
            return -1;
        }
    } else if(data1->vpnid > data2->vpnid) {
        if(data1->isid == data2->isid){
            return 0;
        } else {
            return 1;
        }
    } else { /* data1->vpnid == data2->vpnid */
        return 0;
    }
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static int _bcm_caladan3_mim_port_check(int unit,
                                      bcm_mim_port_t *mim_port,
                                      bcm_caladan3_mim_vpn_control_t *vpncb,
                                      bcm_caladan3_mim_port_control_t **defbbportcb)
{
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    dq_p_t                       port_elem;
    uint8                        isdefbbone = 0;

    if((!mim_port) || (!vpncb) || (!defbbportcb)) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(mim_port->flags & ~_BCM_CALADAN3_MIM_SUPPORTED_PORT_FLAGS) {
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
        /* verify if mim-port id is valid */
        G3P1_GPORT_RANGE_CHECK(unit, mim_port->mim_port_id);
    }

    if(mim_port->criteria & ~_BCM_CALADAN3_MIM_SUPPORTED_CRITERIA) {
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
        /* Verify if user is requesting for only Supported PBBN interface types */
        /* Supported Service Interfaces:
         * [1] Port Mode
         * [2] 1:1 STAG Mode
         */
        if(mim_port->flags & ~_BCM_CALADAN3_MIM_ACCESS_SUPPORTED_FLAGS) {
            return BCM_E_PARAM;
        }
        if(mim_port->criteria & ~_BCM_CALADAN3_MIM_ACCESS_SUPPORTED_CRITERIA) {
            return BCM_E_PARAM;
        }
    }

    /* verify if the port is MODPORT or TRUNK gport type */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port) &&
       !BCM_GPORT_IS_TRUNK(mim_port->port)){
         LOG_ERROR(BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "WARNING %s Only modport Gport ID supported !!!!(%s,%d)\n"),
                    FUNCTION_NAME(), __FILE__, __LINE__));
         return BCM_E_PARAM;
    }


    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       _MIM_PORT_INVALID(unit, BCM_GPORT_MODPORT_PORT_GET(mim_port->port))) {
        return BCM_E_PARAM;
    }

    /* if port is created with an ID verify Encap ID and Mim port ID */
    if(mim_port->flags & BCM_MIM_PORT_WITH_ID){
        if(!BCM_GPORT_IS_MIM_PORT(mim_port->mim_port_id)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "WARNING %s Invalid MIM port ID !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }
        
        if((mim_port->flags & BCM_MIM_PORT_ENCAP_WITH_ID) &&
           (!SOC_SBX_IS_VALID_ENCAP_ID(mim_port->encap_id))) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "WARNING %s Invalid Encap ID !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
         return BCM_E_PARAM;
       }
    }

    /* Verify no back bone port is created before default back bone port is created */
    if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

        if(mim_port->flags & ~_BCM_CALADAN3_MIM_BB_SUPPORTED_FLAGS) {
            return BCM_E_PARAM;
        }

        if(mim_port->criteria & ~_BCM_CALADAN3_MIM_BB_SUPPORTED_CRITERIA) {
            return BCM_E_PARAM;
        }

        if(BCM_MAC_IS_ZERO(mim_port->egress_tunnel_srcmac)){
           LOG_ERROR(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "WARNING %s Zero SMAC invalid for Back bone port !!!!(%s,%d)\n"),
                      FUNCTION_NAME(), __FILE__, __LINE__));
           return BCM_E_PARAM;
        }

        /* if default back bone port is trying to be created verify if no default
         * back bone port exists */
        if(BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac)){

           /* Verify if the Default Back Bone Port Exists */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

               /* Cant create more than one default back bone port on same modport gport */
               if(portcb->port == mim_port->port){
                   LOG_ERROR(BSL_LS_BCM_MIM,
                             (BSL_META_U(unit,
                                         "WARNING %s Duplicate Default Back bone port creation !!!!(%s,%d)\n"),
                              FUNCTION_NAME(), __FILE__, __LINE__));
                   return BCM_E_PARAM;
               }
           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
           isdefbbone = 1;
        } else {
           int found = 0;
           /* VIP(ISID) can be associated with only one PIP or BSMAC */
           /* Verify if PIP or BSMAC are same for default and added back bone port */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

               if(_BCM_CALADAN3_IS_MAC_EQUAL(portcb->smac, mim_port->egress_tunnel_srcmac)){
                   found = 1;
                   *defbbportcb = portcb;
               }
           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

           if(!found) {
               LOG_ERROR(BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "WARNING %s Default Back bone not found !!!!(%s,%d)\n"),
                          FUNCTION_NAME(), __FILE__, __LINE__));
               return BCM_E_PARAM;
           }
        }
    }

    if(!isdefbbone) {
        /* verify is no other mim acces port is created on this port */
        if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {

        /* Check if there is a Mim Port using this port */
            DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {

            _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

            if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
                /* verify if a virtual port exists with this ID */
                if(portcb->gport == mim_port->mim_port_id) {
                    return BCM_E_EXISTS;
                }
            }

                if(portcb->port == mim_port->port){
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "WARNING %s Access port Exists !!!!(%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                    return BCM_E_EXISTS;
                }
                
            } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);
        }

            /* compare the mac addresses and see if ports exist with the same mac */
            if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

            /* Check if there is a Mim Port using this port */
            DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {

                _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

                if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
                    /* verify if a virtual port exists with this ID */
                    if(portcb->gport == mim_port->mim_port_id) {
                    return BCM_E_EXISTS;
                }
            }

                /* compare the mac addresses and see if ports exist with the same mac */
                if(_BCM_CALADAN3_IS_MAC_EQUAL(portcb->dmac, mim_port->egress_tunnel_dstmac)) {
                    
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "WARNING %s Back bone port Exists !!!!(%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                    return BCM_E_EXISTS;
                }

            } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);
            }
    }

    return BCM_E_NONE;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static int _bcm_caladan3_mim_port_update_check(int unit,
                                             bcm_mim_port_t *mim_port,
                                             bcm_caladan3_mim_vpn_control_t *vpncb,
                                             bcm_caladan3_mim_port_control_t **updateportcb,
                                             bcm_caladan3_mim_port_control_t **newdefportcb,
                                             bcm_caladan3_mim_port_control_t **olddefportcb)
{
    uint32 portid;
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb=NULL, *defbbportcb=NULL;
    dq_p_t port_elem;
    int status = BCM_E_NONE;

    if(!mim_port || !vpncb || !updateportcb || !newdefportcb || !olddefportcb) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(!(mim_port->flags & _BCM_CALADAN3_MIM_SUPPORTED_UPDATE_FLAGS)) {
        return BCM_E_PARAM;
    }

    /* verify if the port is MODPORT or TRUNK gport type */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port) &&
       !BCM_GPORT_IS_TRUNK(mim_port->port)){
         LOG_ERROR(BSL_LS_BCM_MIM,
                   (BSL_META_U(unit,
                               "WARNING %s Only modport Gport ID supported !!!!(%s,%d)\n"),
                    FUNCTION_NAME(), __FILE__, __LINE__));
         return BCM_E_PARAM;
    }


    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       _MIM_PORT_INVALID(unit, BCM_GPORT_MODPORT_PORT_GET(mim_port->port))) {
        return BCM_E_PARAM;
    }

    /* Only update supported now is for Station Movement update of Back Bone port */
    /* obtain portcb using gport id */
    portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port->mim_port_id);

    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, mim_port->mim_port_id);

    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid - SBX_GLOBAL_GPORT_FTE_BASE(unit)];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;

    } else {
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        /* This is not a station movement update flag error */
        if((portcb->type == _BCM_CALADAN3_MIM_BACKBONE_PORT) &&
           (portcb->port == mim_port->port)) {

            *newdefportcb = NULL;
            *olddefportcb = NULL;

           /* Verify if there is Default Back Bone Port for the moved BMAC to associate to */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

               if(defbbportcb->port == portcb->port) {
                   *olddefportcb = defbbportcb;
               }

               if(defbbportcb->port == mim_port->port){
                   *newdefportcb = defbbportcb;
               }

               if(*newdefportcb && *olddefportcb) {
                   break;
               }

           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

           if((defbbportcb) && (defbbportcb->port == mim_port->port)) {
               *updateportcb = portcb;
           } else {

               LOG_ERROR(BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "ERROR: %s Default Back bone not found !!!!(%s,%d)\n"),
                          FUNCTION_NAME(), __FILE__, __LINE__));
               status = BCM_E_PARAM;
           }
        } else {

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Not a Back Bone Station Movement Update !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));

            status = BCM_E_PARAM;
        }
    }
    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_SUPPORT
static int _bcm_caladan3_mim_update_bbone_port_movement(int unit,
                                                      bcm_mim_port_t *mim_port,
                                                      bcm_caladan3_mim_port_control_t *portcb,
                                                      bcm_caladan3_mim_port_control_t *defbbportcb,
                                                      bcm_caladan3_mim_port_control_t *olddefbbportcb)
{
    int status = BCM_E_NONE;
    int    fabport, fabunit, node, module, port;

    if(!portcb || !mim_port || !defbbportcb) {
        return BCM_E_PARAM;
    }

    /* port can be moved from
       1. non-trunk to trunk
       2. trunk to non-trunk
       3. non-trunk to non-trunk
       4. trunk to trunk
    */

    /* case 4 */
    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       BCM_GPORT_IS_MODPORT(portcb->port)) {
        /* update FTIDX QID for the new modport gport */
        soc_sbx_g3p1_ft_t fte;

        status = soc_sbx_g3p1_ft_get(unit, portcb->hwinfo.ftidx, &fte);
        if(BCM_SUCCESS(status)) {
            module = BCM_GPORT_MODPORT_MODID_GET(portcb->port);
            port   = BCM_GPORT_MODPORT_PORT_GET(portcb->port);


            /* obtain fabric info to program QID */
            status = soc_sbx_node_port_get(unit, module, port,
                                           &fabunit, &node, &fabport);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "Error %s Failed to Get Fabric Port Node info !!!!(%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));
            } else {
   	        fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,node,fabport, NUM_COS(unit));
                LOG_VERBOSE(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "unit %d: [%s] soc_sbx_node_port_get mod(0x%x) port(%d)"
                                         "-> node(0x%x) port (%d) map to qid (0x%05x) unit(%d)\n"),
                             unit, FUNCTION_NAME(), module, port, node,
                             fabport, fte.qid, fabunit));

                status = soc_sbx_g3p1_ft_set(unit,portcb->hwinfo.ftidx, &fte);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in programming FTE [0x%x] in HW\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ftidx));
                } else {
                    /* update bsmac pid */
                    soc_sbx_g2p3_bmac_t bsmac;
                    status = soc_sbx_g2p3_bmac_get(unit, portcb->dmac,
                                                   portcb->vlan, &bsmac);
                    if(BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "unit %d: [%s] error %s Getting BSMAC"MAC_FMT"\n"),
                                   unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                    } else {
                        int fabport, fabunit, fabnode;

                        status = soc_sbx_node_port_get(unit, module, port,
                                                       &fabunit, &fabnode, &fabport);
                        if(BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "unit %d: [%s] error %s Mapping FE to QE port information"),
                                       unit, FUNCTION_NAME(), bcm_errmsg(status)));
                        } else {

                            /* update port of entry on BSMAC */
                            bsmac.bpid = SOC_SBX_PORT_SID(unit, fabnode,
                                                          fabport);

                            status = soc_sbx_g2p3_bmac_set(unit, portcb->dmac,
                                                           portcb->vlan, 
                                                           &bsmac);
                            if(BCM_FAILURE(status)) {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n"),
                                           unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                            }
                        }
                    }
                }
            }

        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Getting FTE idx [0x%x] !!!!(%s,%d)\n"),
                       FUNCTION_NAME(),portcb->hwinfo.ftidx, __FILE__, __LINE__));
        }
    } else {
        /* add support */
        status = BCM_E_UNAVAIL;
    }

    if(BCM_SUCCESS(status)) {
        portcb->port = mim_port->port; /* update the underneath modport gport */
        defbbportcb->refcount++;
        olddefbbportcb->refcount--;
    }

    return status;
}
#endif

/* Terminology
 * INSEG (aka) Insegment - Refers to Ingress Hardware Resources required for SBX forwarding
 *                       - eg., FT, LP
 * OUTSEG (aka) Outsegment - Refers to Egress Hardware Resources required for SBX forwarding
 */

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_alloc_inseg(int           unit,
                                     int          flags,
                                     bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb){
        status = BCM_E_PARAM;
    } else {
        if(_BCM_CALADAN3_MIM_DEF_BACKBONE_PORT == portcb->type) {
            /* Allocate a LSM entry */
            status = _sbx_caladan3_ismac_idx_alloc(unit,
                                              0,
                                              portcb->smac,
                                              _SBX_CALADAN3_RES_UNUSED_PORT,
                                              &portcb->hwinfo.ismacidx);
        }

        if(BCM_SUCCESS(status)) {

            /* Allocate logical port */
            /* Logical Ports for Back bone ports are dummy, they are only used now for quick access
             * of obtaining Port information given a GPORT ID which is O(1) access */
            status = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_LPORT,
                                         1,
                                         &portcb->hwinfo.lport,
                                         0);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error(%s) could not allocate LP index\n"),
                           unit, FUNCTION_NAME(), bcm_errmsg(status)));
            } else {

                status = _sbx_caladan3_resource_alloc(unit,
                                                 SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                                 1,
                                                 &portcb->hwinfo.ftidx,
                                                 (flags & BCM_MIM_PORT_WITH_ID)?
                                                 _SBX_CALADAN3_RES_FLAGS_RESERVE:0);

                if(flags & BCM_MIM_PORT_WITH_ID) {
                   if(status == BCM_E_RESOURCE) {
                       LOG_VERBOSE(BSL_LS_BCM_MIM,
                                   (BSL_META_U(unit,
                                               "DEBUG Reserved FTE (0x%x) (%s,%d)\n"),
                                    portcb->hwinfo.ftidx, __FILE__, __LINE__));
                       status = BCM_E_NONE;
                   } else {
                       status = BCM_E_FULL;
                   }
                }

                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error(%s) could not allocate fte index\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status)));
                } else {
                    /* validate if FTIDX doesnt exceed PID number of bits */
                    int result;
                    uint32 min, max;

                    result = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_LPORT,
                                                      &min, &max);

                    if((result == BCM_E_NONE) && (portcb->hwinfo.ftidx > max)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "Error %s FTIDX(%d) greater than PID range(%d<->%d) supported !!!!(%s,%d)\n"),
                                   FUNCTION_NAME(),portcb->hwinfo.ftidx, min, max, __FILE__,__LINE__));
                        status = BCM_E_PARAM;
                    } else {
                        /* Set Global FT to GPORT Type mapping */
                        if(_sbx_caladan3_set_global_fte_type(unit, portcb->hwinfo.ftidx, 
                                                        BCM_GPORT_MIM_PORT) != BCM_E_NONE) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "Error %s Setting Global FTE Type"),
                                       FUNCTION_NAME()));
                            status = BCM_E_INTERNAL;
                        }
                    }
                }

            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) could not allocate SMAC index\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status)));
        }
    }
    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_free_inseg(int           unit,
                                    bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb) {
        return BCM_E_PARAM;
    }

    if (portcb->hwinfo.ismacidx) {
        status = _sbx_caladan3_ismac_idx_free(unit,
                                         portcb->dmac,
                                         _SBX_CALADAN3_RES_UNUSED_PORT,
                                         &portcb->hwinfo.ismacidx);
        if (status == BCM_E_EMPTY) {
            soc_sbx_g3p1_lsmac_t lsmac;
            soc_sbx_g3p1_lsmac_t_init(&lsmac);

            status = soc_sbx_g3p1_lsmac_set (unit, portcb->hwinfo.ismacidx, &lsmac);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "Failed to clear LSM: %d %s\n"),
                           status, bcm_errmsg(status)));
            } else {
                LOG_VERBOSE(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "Freed Back LSM[0x%x] for MAC"MAC_FMT"\n"),
                             portcb->hwinfo.ismacidx, MAC_PFMT(portcb->smac)));
            }

        } else if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "error(%s) could not free SMAC IDX [%d]\n"),
                       bcm_errmsg(status), portcb->hwinfo.ismacidx));
        }
    }

    if(portcb->hwinfo.lport){
        /* Free logical port */
        status = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_LPORT,
                                        1, &portcb->hwinfo.lport, 0);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "error(%s) could not free LP[%d]\n"),
                       bcm_errmsg(status), portcb->hwinfo.lport));
        }
    }


    if(portcb->hwinfo.ftidx) {
        /* Free FTE port */
        status = _sbx_caladan3_resource_free(unit,
                                        SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                        1,
                                        &portcb->hwinfo.ftidx,
                                        0);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) could not free FTE[%d]\n"),
                       unit, FUNCTION_NAME(),bcm_errmsg(status), portcb->hwinfo.ftidx));
        }

        /* Invalidate Global FT to GPORT Type mapping */
        if(_sbx_caladan3_set_global_fte_type(unit, portcb->hwinfo.ftidx, 
                                        BCM_GPORT_TYPE_NONE) != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Setting Global FTE Type"),
                       FUNCTION_NAME()));
            status = BCM_E_INTERNAL;
        }
    }

    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_alloc_outseg(int                          unit,
                                      int                          modid,
                                      int                          flags,
                                      bcm_if_t                    *encap,
                                      bcm_caladan3_mim_port_control_t *portcb)
{
    int status=BCM_E_NONE, result=0;
    int encapfail=0;

    if(!portcb || !encap) {
        status = BCM_E_PARAM;
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "WARNING %s Bad Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
    } else {

        /* Assumption, Encap ID is validated on port check */
        /* Allocate OI */
        if(flags & BCM_MIM_PORT_ENCAP_WITH_ID) {
            portcb->hwinfo.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(*encap);
        }

        result = _sbx_caladan3_resource_alloc(unit,
                     SBX_CALADAN3_USR_RES_OHI,
                     1,
                     &portcb->hwinfo.ohi,
                     (flags & BCM_MIM_PORT_ENCAP_WITH_ID)?_SBX_CALADAN3_RES_FLAGS_RESERVE:0);

        if(flags & BCM_MIM_PORT_ENCAP_WITH_ID) {
           if(result == BCM_E_RESOURCE) {
               LOG_VERBOSE(BSL_LS_BCM_MIM,
                           (BSL_META_U(unit,
                                       "DEBUG Reserved EncapID (0x%x) (%s,%d)\n"),
                            portcb->hwinfo.ohi, __FILE__, __LINE__));
               result = BCM_E_NONE;
           } else {
               result = BCM_E_FULL;
           }
        }

        if(BCM_FAILURE(result)) {
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "WARNING %s Encap ID allocation Failed !!!!(%s,%d)\n"),
                         FUNCTION_NAME(), __FILE__, __LINE__));
            encapfail = 1;
            status = BCM_E_FULL;
        } else {
            /* Dont allocate ETE if this is not the Home Unit */

            result = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_ETE,
                                         1,
                                         &portcb->hwinfo.ete,
                                         0);
            if(BCM_FAILURE(result)) {
                LOG_VERBOSE(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "WARNING %s ETE Encap allocation Failed !!!!(%s,%d)\n"),
                             FUNCTION_NAME(), __FILE__, __LINE__));
                status = BCM_E_FULL;
            } else {
                /* If back bone port allocate smac idx */
                if(portcb->type == _BCM_CALADAN3_MIM_BACKBONE_PORT ||
                   portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {

                    result = _sbx_caladan3_esmac_idx_alloc(unit, 0,
                                                      portcb->smac,
                                                      &portcb->hwinfo.esmacidx);
                    if(BCM_E_NONE != result) {
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "WARNING %s ESMAC allocation Failed !!!!(%s,%d)\n"),
                                     FUNCTION_NAME(), __FILE__, __LINE__));
                        status = BCM_E_FULL;
                    }
                }

            } /* else of ete failure check */
        } /* else of oi allocation failure check */

        if(BCM_E_FULL == status){

            if(portcb->hwinfo.ete) {
                if(BCM_E_NONE != _sbx_caladan3_resource_free(unit,
                                                        SBX_CALADAN3_USR_RES_ETE,
                                                        1,
                                                        &portcb->hwinfo.ete,
                                                        0)) {
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "WARNING %s ETE L2 Free Failed !!!!(%s,%d)\n"),
                                 FUNCTION_NAME(), __FILE__, __LINE__));
                }
            }

            if(!encapfail) {
                if(BCM_E_NONE != _sbx_caladan3_resource_free(unit,
                                                        SBX_CALADAN3_USR_RES_OHI,
                                                        1,
                                                        (uint32*)encap,
                                                        0)) {
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "WARNING %s ETE Encap Free Failed !!!!(%s,%d)\n"),
                                 FUNCTION_NAME(), __FILE__, __LINE__));
                }
            }

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Enacpsulation Resource not available !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
        } else {
            *encap = SOC_SBX_ENCAP_ID_FROM_OHI(portcb->hwinfo.ohi);
        }

    }
    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_free_outseg(int                          unit,
                                     bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb) {
        status = BCM_E_PARAM;
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "WARNING %s Bad Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
    } else {
        if(portcb->hwinfo.ohi){
            /* Free logical port */
            status = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_OHI,
                                            1, &portcb->hwinfo.ohi, 0);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "error(%s) could not free OI[%d]\n"),
                           bcm_errmsg(status), portcb->hwinfo.ohi));
            }
        }

        if (portcb->hwinfo.ete) {
            /* Free logical port */
            status = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_ETE,
                                            1, &portcb->hwinfo.ete, 0);

            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "error(%s) could not free Encap L2[%d]\n"),
                           bcm_errmsg(status), portcb->hwinfo.ete));
            }
        }

        if(portcb->hwinfo.esmacidx) {
            /* Free logical port */
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "Freeing esmac idx 0x%x mac=" MAC_FMT "\n"),
                         portcb->hwinfo.esmacidx, MAC_PFMT(portcb->smac)));
            status = _sbx_caladan3_esmac_idx_free(unit, portcb->smac,
                                             &portcb->hwinfo.esmacidx);

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
            if (status == BCM_E_EMPTY) {
                soc_sbx_g2p3_esmac_t esmac;
                int esmacIdx = portcb->hwinfo.esmacidx;

                soc_sbx_g2p3_esmac_t_init(&esmac);
                status = soc_sbx_g2p3_esmac_set(unit, esmacIdx, &esmac);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "failed to clear esmac idx=%d: %d (%s)\n"),
                               esmacIdx, status, bcm_errmsg(status)));
                }
            } else if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "error(%s) could not free ESMAC[%d]\n"),
                           bcm_errmsg(status), portcb->hwinfo.esmacidx));
            }
#endif
        }
    }
    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_map_inseg_outseg(int      unit,
                                          bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;
    soc_sbx_g3p1_ft_t fte;
    int    fabport, fabunit, node;
    int    module, port;

    soc_sbx_g3p1_ft_t_init(&fte);

    if(portcb) {
        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            fte.lag = 1;
#define TRUNK_SBX_HASH_SIZE  3 
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)
#define TRUNK_INDEX_SET(tid, offset)                \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?          \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) :  \
     -1
            fte.lagbase = TRUNK_INDEX_SET(BCM_GPORT_TRUNK_GET(portcb->port), 0);
            fte.lagsize = TRUNK_SBX_HASH_SIZE;
        } else {
            module = _SHR_GPORT_MODPORT_MODID_GET(portcb->port);
            port   = _SHR_GPORT_MODPORT_PORT_GET(portcb->port);


            /* obtain fabric info to program QID */
            status = soc_sbx_node_port_get(unit, module, port,
                                           &fabunit, &node,
                                           &fabport);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "Error %s Failed to Get Fabric Port Node info !!!!(%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));
            } else {
 	        fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,node,fabport, NUM_COS(unit));
                LOG_VERBOSE(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "unit %d: [%s] soc_sbx_node_port_get mod(0x%x) port(%d)"
                                         "-> node(0x%x) port (%d) map to qid (0x%05x) unit(%d)\n"),
                             unit, FUNCTION_NAME(), module, port, node,
                             fabport, fte.qid, fabunit));

            }
        }

        if(BCM_E_NONE == status) {
            /* map ohi to fte */
            fte.oi          = portcb->hwinfo.ohi;

            status = soc_sbx_g3p1_ft_set(unit,
                                         portcb->hwinfo.ftidx,
                                         &fte);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error %s in programming FTE 0x%x in HW\n"),
                           unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ftidx));
            } else {
                /* LP except for  default  back bone ports are dummy, so set could be eliminated ..*/
                /* provision lp */
                soc_sbx_g3p1_lp_t lp;

                soc_sbx_g3p1_lp_t_init(&lp);

                lp.pid = portcb->hwinfo.ftidx;

                if(portcb->policer_id > 0) {
                    /* for Back bone ports Allow policing only on ISID
                     * i.e., only on vpn isid2e LP */
                    if(portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT ||
                       portcb->type == _BCM_CALADAN3_MIM_BACKBONE_PORT) {
                        soc_sbx_g3p1_lp_t isidlp;

                        status = soc_sbx_g3p1_lp_get(unit, portcb->isidlport, &isidlp);
                        if(BCM_SUCCESS(status)) {
                            status = _bcm_caladan3_g3p1_policer_lp_program(unit, 
                                                                     portcb->policer_id,
                                                                     &isidlp);              
                            if(BCM_SUCCESS(status)) {
                                status =  soc_sbx_g3p1_lp_set(unit, portcb->isidlport, &isidlp);

                                if(BCM_SUCCESS(status)) {
                                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                                (BSL_META_U(unit,
                                                            "Debug %s Provisioned Policer %d on ISID LP 0x%x\n"),
                                                 FUNCTION_NAME(),portcb->policer_id,portcb->isidlport));
                                } else {
                                    LOG_ERROR(BSL_LS_BCM_MIM,
                                              (BSL_META_U(unit,
                                                          "unit %d: [%s] error %s in setting ISID LP 0x%x in HW\n"),
                                               unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->isidlport));
                                }
                            } else {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "unit %d: [%s] error %s Configuring Policer %d to  ISID LP 0x%x\n"),
                                           unit, FUNCTION_NAME(), bcm_errmsg(status), 
                                           portcb->policer_id,
                                           portcb->isidlport));
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "unit %d: [%s] error %s in reading ISID LP 0x%x in HW\n"),
                                       unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->isidlport));
                        }
                        
                    } else {
                        status = _bcm_caladan3_g3p1_policer_lp_program(unit, 
                                                                     portcb->policer_id,
                                                                     &lp);
                        if(BCM_SUCCESS(status)) {
                            status = soc_sbx_g3p1_lp_set(unit, portcb->hwinfo.lport, &lp);
                            if(BCM_FAILURE(status)) {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n"),
                                           unit, portcb->gport, portcb->hwinfo.lport));
                            } else {
                                LOG_DEBUG(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n"),
                                           unit, portcb->gport, portcb->hwinfo.lport, portcb->policer_id));                            
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Configured Policer %d on Access LP 0x%x\n"),
                                       FUNCTION_NAME(),portcb->policer_id,portcb->hwinfo.lport));
                        }
                    }
                }

                if(BCM_SUCCESS(status)) {
                    status = soc_sbx_g3p1_lp_set(unit,
                                                 portcb->hwinfo.lport,
                                                 &lp);
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "Set lpi 0x%x pid=0x%x\n"),
                                 portcb->hwinfo.lport, lp.pid));
                }

                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in programming LP 0x%x\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.lport));
                }
            }
        }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_g3p1_provision_outseg(int                          unit,
                                          int                          mymodid,
                                          bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;
    uint8 isRemote = TRUE;

    if(portcb && _MIM_PORT_TYPE_VALID_CHECK(portcb->type)) {
        bcm_trunk_add_info_t *tdata;
        int index=0;

        /* If MIM port is over Trunk, Trunk table would have already allocated
         * required OI and OI would have already been reserved. So skip Encap allocation
         * completely */
        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            tdata = sal_alloc(sizeof(bcm_trunk_add_info_t), "trunk add info");
            if (tdata == NULL) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] Unable to allocate trunk info structure\n"),
                           unit, FUNCTION_NAME()));
                return BCM_E_MEMORY;
            }
            
            /* Compare mymodid with trunk module array, if any match allocate
             * egress resource else skip */
            status = bcm_caladan3_trunk_get_old(unit, 
                                   BCM_GPORT_TRUNK_GET(portcb->port),
                                   tdata);
            if(BCM_SUCCESS(status)) {
                for(index=0; index < tdata->num_ports; index++) {
                    if(tdata->tm[index] == mymodid) {
                        isRemote = FALSE;
                        break;
                    }
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] Failed to get trunk information\n"),
                           unit, FUNCTION_NAME()));
            }
            sal_free(tdata);
        } else if(BCM_GPORT_MODPORT_MODID_GET(portcb->port) != mymodid) {
            /* program OI & ETE only if the mim port is on our Home unit */
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "Debug %s Non-Home Units doesnt require OI/ETE provisioning (%s,%d)\n"),
                         FUNCTION_NAME(), __FILE__, __LINE__));
        } else {
            isRemote = FALSE;
        }

        if(!isRemote) {

             soc_sbx_g3p1_ete_t         ete;

             /* start from lowel hierarchy to upper so there is no incomplete pointer */
             soc_sbx_g3p1_ete_t_init(&ete);

             switch(portcb->type) {

                 case _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT:
                     
                     /* Provision ETE for Default back bone port */
                     ete.tunnelenter = 1;
                     ete.encapmac    = 1;
                     ete.encaplen    = 22;
#if 0
                     ete.btag_vid    = portcb->vlan;
                     ete.btag_pricfi = 0; 
                     ete.btag_tpid   = (portcb->tpid > 0)?portcb->tpid:_BCM_CALADAN3_DEFAULT_BTAG_TPID;
                     ete.isid        = portcb->isid;
                     ete.mimtype     = _BCM_CALADAN3_MIM_ETYPE;
#endif
                     ete.etepid      = 0;
                     ete.nosplitcheck = 0;

                     /* if trunk do skout through ete pid */
                     if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                         ete.etepid = 1;
                         ete.pid    = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                     }

                     /* Set BDMAC to default back bone group bmac based on isid */
                     /*01_1E_83*/
                     ete.dmacset = 1;
                     ete.dmac5   = portcb->isid & 0xFF;
                     ete.dmac4   = (portcb->isid >> 8) & 0xFF;
                     ete.dmac3   = (portcb->isid >> 16) & 0xFF;
                     ete.dmac2   = 0x83;
                     ete.dmac1   = 0x1E;
                     ete.dmac0   = 0x01;

                     /* copy tunnel mac */
                     ete.smacset = 1;

                     
                     ete.stpcheck  = 1;
                     ete.usevid    = 1;
                     ete.vid       = 0xFFF;
                     ete.smacindex = portcb->hwinfo.esmacidx;
                     ete.mtu       = SBX_DEFAULT_MTU_SIZE;
                     break;

                 case _BCM_CALADAN3_MIM_BACKBONE_PORT:
                     
                     ete.tunnelenter = 1;
                     ete.encapmac    = 1;
                     ete.encaplen    = 22;
#if 0
                     ete.btag_vid    = portcb->vlan;
                     ete.btag_pricfi = 0; 
                     ete.btag_tpid   = (portcb->tpid > 0)?portcb->tpid:_BCM_CALADAN3_DEFAULT_BTAG_TPID;
                     ete.isid        = portcb->isid;
                     ete.mimtype     = 1;
#endif
                     ete.etepid      = 0;
#if 0
                     ete.mimtype     = _BCM_CALADAN3_MIM_ETYPE;
#endif
                     ete.nosplitcheck= 0;

                     /* if trunk do skout through ete pid */
                     if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                         ete.etepid = 1;
                         ete.pid    = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                     }

                     /* copy tunnel mac */
                     ete.smacset = 1;
                     ete.dmacset = 1;
                     ete.dmac5   = portcb->dmac[5];
                     ete.dmac4   = portcb->dmac[4];
                     ete.dmac3   = portcb->dmac[3];
                     ete.dmac2   = portcb->dmac[2];
                     ete.dmac1   = portcb->dmac[1];
                     ete.dmac0   = portcb->dmac[0];

                     ete.stpcheck  = 1;
                     ete.usevid    = 1;
                     ete.vid       = 0xFFF;
                     ete.smacindex = portcb->hwinfo.esmacidx;
                     ete.mtu       = SBX_DEFAULT_MTU_SIZE;
                 break;

                 /* Access port */
                 case _BCM_CALADAN3_MIM_ACCESS_PORT:

                     /* Setting will vary based on STAG interface or Port Interface */
                     ete.tunnelenter = 0;
                     ete.encapmac    = 0;
                     ete.dmacset     = 0;
                     ete.smacset     = 0;
                     ete.nosplitcheck = 0;
                     ete.etepid = 1;
                     ete.pid    = portcb->hwinfo.ftidx;

                     ete.stpcheck   = 1;
                     ete.mtu        = SBX_DEFAULT_MTU_SIZE;

                     /* Based on Service Interface Configure VID on ETE */
                     if(portcb->service == _BCM_CALADAN3_MIM_PORT) {
                         soc_sbx_g3p1_p2e_t  p2e;
                         /* use native vid on ete */

                         /* update native vid from p2e */
                         status = soc_sbx_g3p1_p2e_get(unit,
                                                       BCM_GPORT_MODPORT_PORT_GET(portcb->port),
                                                       &p2e);
                         if(BCM_SUCCESS(status)) {
                             ete.usevid    = 1;
                             ete.vid       = p2e.nativevid;
                         }

                     } else if(portcb->service == _BCM_CALADAN3_MIM_STAG_1_1){
                         /* use stag on ete */
                         ete.usevid    = 1;
                         ete.vid       = portcb->vlan;
                     } else if(portcb->service == _BCM_CALADAN3_MIM_STAG_BUNDLED){
                         /* do nothing */
                     } else {
                         LOG_ERROR(BSL_LS_BCM_MIM,
                                   (BSL_META_U(unit,
                                               "unit %d: [%s] Unsupported Service type\n"),
                                    unit, FUNCTION_NAME()));
                         status = BCM_E_PARAM;
                     }
                     break;

                 default:
                     /* should never hit here - just to keep compiler happy*/
                     LOG_ERROR(BSL_LS_BCM_MIM,
                               (BSL_META_U(unit,
                                           "unit %d: [%s] BAD port type\n"),
                                unit, FUNCTION_NAME()));
                     status = BCM_E_PARAM;
                     break;
             }

             
             ete.ttlcheck = 0;
             ete.ipttldec = 0;

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
             if (ete.smacset) {
                 soc_sbx_g2p3_esmac_t esmac;
                 int esmacIdx = portcb->hwinfo.esmacidx;

                 esmac.smac5 = portcb->smac[5];  esmac.smac4 = portcb->smac[4];
                 esmac.smac3 = portcb->smac[3];  esmac.smac2 = portcb->smac[2];
                 esmac.smac1 = portcb->smac[1];  esmac.smac0 = portcb->smac[0];

                 status = soc_sbx_g2p3_esmac_set(unit, esmacIdx, &esmac);
                 if (BCM_FAILURE(status)) {
                     LOG_ERROR(BSL_LS_BCM_MIM,
                               (BSL_META_U(unit,
                                           "failed to set esmac idx=%d: %d (%s)\n"),
                                esmacIdx, status, bcm_errmsg(status)));
                 }
                 LOG_VERBOSE(BSL_LS_BCM_MIM,
                             (BSL_META_U(unit,
                                         "Set esmac idx %d " MAC_FMT " \n"),
                              esmacIdx, MAC_PFMT(portcb->smac)));
             }
#endif


             if(BCM_SUCCESS(status)) {
                 status = soc_sbx_g3p1_ete_set(unit,
                                                 portcb->hwinfo.ete,
                                                 &ete);
                 if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in programming L2 ETE 0x%x in HW\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ete));
                 } else {
                     status = soc_sbx_g3p1_ete_set(unit,
                                                        portcb->hwinfo.ete,
                                                        &ete);
                     if(BCM_FAILURE(status)) {
                         LOG_ERROR(BSL_LS_BCM_MIM,
                                   (BSL_META_U(unit,
                                               "unit %d: [%s] error %s in programming ENACP ETE 0x%x in HW\n"),
                                    unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ete));
                     } else {
                         soc_sbx_g3p1_oi2e_t        ohi2etc;
                         soc_sbx_g3p1_oi2e_t_init(&ohi2etc);
                         ohi2etc.eteptr = portcb->hwinfo.ete;
                         status = soc_sbx_g3p1_oi2e_set(unit,
                                                        _BCM_CALADAN3_G3P1_ADJUST_TB_OFFSET(portcb->hwinfo.ohi),
                                                        &ohi2etc);
                         if(BCM_E_NONE != status) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "unit %d: [%s] error %s in programming OI 0x%x in HW\n"),
                                       unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ohi));
                         }
                     }
                 }
             }
        }

    } else {
        status = BCM_E_PARAM;
    }

    return status;
}
#endif


#define MIM_ACCESS_PORT_MODE       (1)
#define MIM_ACCESS_1_1_MODE        (2)
#define MIM_ACCESS_BUNDLE_MODE     (3)
#define MIM_BBONE_PORT             (4)

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_config_port_interface(int             unit,
                                          bcm_mim_port_t *mim_port,
                                          bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port=0, index, mymodid;
    soc_sbx_g3p1_p2e_t  p2e;
    soc_sbx_g3p1_ep2e_t ep2e;
    soc_sbx_g3p1_pv2e_t pv2e;
    soc_sbx_g3p1_lp_t lp;
    soc_sbx_g3p1_epv2e_t epv2e;
    uint32             *vlanvector;
    uint32             port_mirror = 0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid gport:  gport=0x%08x\n"),
                   mim_port->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META_U(unit,
                                "Skipping Port configuration since module is remote")));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid port: %d, gport=0x%08x\n"),
                   port, mim_port->port));
        return BCM_E_PARAM;
    }

    vlanvector = sal_alloc(sizeof(uint32) * (BCM_VLAN_MAX + 1), "work buffer");

    if (vlanvector == NULL) {
        return BCM_E_PARAM;
    }
    sal_memset(vlanvector, 0, sizeof(uint32) * (BCM_VLAN_MAX + 1));


    portcb->service = _BCM_CALADAN3_MIM_PORT;

    /* GNATS 35584
     *   Need to clean pvv2e when  aport mode pbb access port is created
    status = soc_sbx_g3p1_pvv2e_delete(unit, 0xFFF, 0x000, 0, 0, port);
     */
    status = soc_sbx_g3p1_util_pvv2e_remove(unit, port, 0, 0);
    if (BCM_E_NOT_FOUND == status) {
       /*  Don't really care that we couldn't delete the pvv2e
        *  entry if it's not here to be deleted...
        */
        status = BCM_E_NONE;
    }

    /* set mim mode on p2e */
    if(((soc_sbx_g3p1_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
       ((soc_sbx_g3p1_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

        p2e.mim         = MIM_ACCESS_PORT_MODE;
        p2e.customer    = 0;
        p2e.provider    = 1;
        p2e.defstrip    = 1;
        p2e.pstrip      = 0;
        p2e.pbb         = 1;

        ep2e.customer   = 1;
        ep2e.mim        = MIM_ACCESS_PORT_MODE;
        ep2e.pbb         = 1;
        ep2e.stpid0     = 0;
        ep2e.stpid1     = 0;

        soc_sbx_g3p1_pv2e_t_init(&pv2e);
        soc_sbx_g3p1_epv2e_t_init(&epv2e);

#if 0
        pv2e.untagged_strip = 1;
#endif
        pv2e.vlan = portcb->isidvlan; /* Native VID resolves to ISID VSI */
        pv2e.lpi  = portcb->hwinfo.lport;

	/* GNATS 33852 */
        
            soc_sbx_g3p1_lp_t_init(&lp);
            status = soc_sbx_g3p1_lp_get(unit, port, &lp);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error %s in reading LP 0x%x\n"),
                           unit, FUNCTION_NAME(), bcm_errmsg(status),port));
            }
            if (lp.mirror) {
                port_mirror = lp.mirror;
                soc_sbx_g3p1_lp_t_init(&lp);
                status = soc_sbx_g3p1_lp_get(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in reading LP 0x%x\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
                lp.mirror = port_mirror;
                status = soc_sbx_g3p1_lp_set(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in programming LP 0x%x\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
            }

        epv2e.strip = 1; /* Native VID is added and then stripped back on egress */

        for(index=0; index < BCM_VLAN_MAX; index++) {
            vlanvector[index] = 0;
        }

        status = soc_sbx_g3p1_pv2e_vlan_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                                 NULL, &vlanvector[0], BCM_VLAN_MAX + 1);
        if(BCM_SUCCESS(status)) {

            /* Program Native VID */
            status = soc_sbx_g3p1_pv2e_set(unit, p2e.nativevid, port, &pv2e);

            if(BCM_SUCCESS(status)) {
                /* Set EPV2E */
                status = soc_sbx_g3p1_epv2e_set(unit, p2e.nativevid, port, &epv2e);

                if(BCM_SUCCESS(status)) {
                    /* program P2E */
                    status = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

                    if(BCM_SUCCESS(status)) {
                        /* program EP2E */
                        status = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);

                        if(BCM_FAILURE(status)) {

                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Programming EP2E %d : %s in (%s,%d)\n"),
                                       FUNCTION_NAME(), status, bcm_errmsg(status),
                                       __FILE__,__LINE__));
                        }

                    } else {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Programming P2E %d : %s in (%s,%d)\n"),
                                   FUNCTION_NAME(), status, bcm_errmsg(status),
                                   __FILE__,__LINE__));
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Programming EPV2E %d : %s in (%s,%d)\n"),
                               FUNCTION_NAME(), status, bcm_errmsg(status),
                               __FILE__,__LINE__));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s Programming PV2E %d : %s in (%s,%d)\n"),
                           FUNCTION_NAME(), status, bcm_errmsg(status),
                           __FILE__,__LINE__));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Programming PV2E Vector %d : %s in (%s,%d)\n"),
                       FUNCTION_NAME(), status, bcm_errmsg(status),
                       __FILE__,__LINE__));
        }
    }

    sal_free(vlanvector);
    return status;
}
#endif

static
int _bcm_caladan3_mim_reset_port_interface(int             unit,
                                         bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port, index, mymodid;
    uint32  *vector;

    if(!portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(portcb->port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid gport: gport=0x%08x\n"),
                   portcb->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(portcb->port)) {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META_U(unit,
                                "Skipping Port configuration since module is remote")));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid port: %d, gport=0x%08x\n"),
                   port, portcb->port));
        return BCM_E_PARAM;
    }

    vector = sal_alloc(sizeof(uint32) * (BCM_VLAN_MAX + 1), "work buffer");

    if (vector == NULL) {
        return BCM_E_PARAM;
    }

    for(index=0; index < BCM_VLAN_MAX; index++) {
        vector[index] = index;
    }
    
    status = soc_sbx_g3p1_pv2e_vlan_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                             NULL, &vector[0], BCM_VLAN_MAX + 1);
    if(BCM_SUCCESS(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Resetting PV2E Vector VSI for Port Interface on port %d in (%s,%d)"), 
                   FUNCTION_NAME(),port, __FILE__, __LINE__));
    } else {
        for(index=0; index < BCM_VLAN_MAX; index++) {
            vector[index] = 0;
        }
        status = soc_sbx_g3p1_pv2e_lpi_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                                 NULL, &vector[0], BCM_VLAN_MAX + 1);      
        if(BCM_SUCCESS(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Resetting PV2E Vector LPI for Port Interface on port %d in (%s,%d)"), 
                       FUNCTION_NAME(),port, __FILE__, __LINE__));
        } 
    }

    sal_free(vector);
    return status;
}

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_config_stag_interface(int             unit,
                                          bcm_mim_port_t *mim_port,
                                          bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port, mymodid;
    soc_sbx_g3p1_p2e_t  p2e;
    soc_sbx_g3p1_ep2e_t ep2e;
    soc_sbx_g3p1_pv2e_t pv2e;
    soc_sbx_g3p1_lp_t lp;
    soc_sbx_g3p1_epv2e_t epv2e;
    uint32             port_mirror = 0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid gport: gport=0x%08x\n"),
                   mim_port->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META_U(unit,
                                "Skipping Port configuration since module is remote")));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid port: %d, gport=0x%08x\n"),
                   port, mim_port->port));
        return BCM_E_PARAM;
    }

    /* GNATS 35584
     * need to delete pvv2e when a pbb access port is created
     * need to clean pv2e when a tag-mode pbb access port is created
    status = soc_sbx_g3p1_pvv2e_delete(unit, 0xFFF, 0x000, 0, 0, port);
     */
    status = soc_sbx_g3p1_util_pvv2e_remove(unit, port, 0, 0);
    if (BCM_E_NOT_FOUND == status) {
       /*  Don't really care that we couldn't delete the pvv2e
        *  entry if it's not here to be deleted...
        */
        status = BCM_E_NONE;
    }
        

    /* set mim mode on p2e */
    if(((soc_sbx_g3p1_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
       ((soc_sbx_g3p1_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

        p2e.mim         = MIM_ACCESS_1_1_MODE;
        p2e.customer    = 0;
        p2e.provider    = 1;
        p2e.defstrip    = 0;
        p2e.pstrip      = 1;
        p2e.pbb         = 1;

        ep2e.customer   = 0; /* for Bundled mode, customer bit has to be one */
        ep2e.stpid0     = 1;
        ep2e.stpid1     = 0;
        ep2e.mim        = MIM_ACCESS_1_1_MODE;
        ep2e.pbb         = 1;

        soc_sbx_g3p1_pv2e_t_init(&pv2e);
        soc_sbx_g3p1_epv2e_t_init(&epv2e);

        /* bundle mode */
        if (portcb->service == _BCM_CALADAN3_MIM_STAG_BUNDLED) {
           p2e.mim         = MIM_ACCESS_BUNDLE_MODE;
           ep2e.customer   = 1; /* customer bit has to be one */
           ep2e.mim        = MIM_ACCESS_BUNDLE_MODE;
        }

        pv2e.vlan = portcb->isidvlan;
        pv2e.lpi  = portcb->hwinfo.lport;

	/* GNATS 33852 */
        
            soc_sbx_g3p1_lp_t_init(&lp);
            status = soc_sbx_g3p1_lp_get(unit, port, &lp);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error %s in reading LP 0x%x\n"),
                           unit, FUNCTION_NAME(), bcm_errmsg(status),port));
            }
            if (lp.mirror) {
                port_mirror = lp.mirror;
                soc_sbx_g3p1_lp_t_init(&lp);
                status = soc_sbx_g3p1_lp_get(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in reading LP 0x%x\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
                lp.mirror = port_mirror;
                status = soc_sbx_g3p1_lp_set(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "unit %d: [%s] error %s in programming LP 0x%x\n"),
                               unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
            }
    
        epv2e.strip = 0; /* dont strip tags at egress */

        /* Program Native VID */
        status = soc_sbx_g3p1_pv2e_set(unit, mim_port->match_vlan, port, &pv2e);

        if(BCM_SUCCESS(status)) {
            /* Set EPV2E */
            status = soc_sbx_g3p1_epv2e_set(unit, mim_port->egress_service_vlan, port, &epv2e);

            if(BCM_SUCCESS(status)) {
                /* program P2E */
                status = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

                if(BCM_SUCCESS(status)) {
                    /* program EP2E */
                    status = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);

                    if(BCM_FAILURE(status)) {

                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Programming EP2E %d : %s in (%s,%d)\n"),
                                   FUNCTION_NAME(), status, bcm_errmsg(status),
                                   __FILE__,__LINE__));
                    }

                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Programming P2E %d : %s in (%s,%d)\n"),
                               FUNCTION_NAME(), status, bcm_errmsg(status),
                               __FILE__,__LINE__));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s Programming EPV2E %d : %s in (%s,%d)\n"),
                           FUNCTION_NAME(), status, bcm_errmsg(status),
                           __FILE__,__LINE__));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Programming PV2E %d : %s in (%s,%d)\n"),
                       FUNCTION_NAME(), status, bcm_errmsg(status),
                       __FILE__,__LINE__));
        }

    } else {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Getting P2E & EP2E in (%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__,__LINE__));
    }
    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_config_bbone_interface(int             unit,
                                           bcm_mim_port_t *mim_port,
                                           bcm_caladan3_mim_port_control_t *portcb)
{
    soc_sbx_g3p1_p2e_t  p2e;
    soc_sbx_g3p1_ep2e_t ep2e;
    soc_sbx_g3p1_pv2e_t pv2e;
    int port=0, status = BCM_E_NONE;
    soc_sbx_g3p1_lsmac_t lsmmac;
    bcm_trunk_add_info_t *tdata = NULL;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    int index=0, mymodid, tnumports=1, pindex=0, idx=0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    /* Only allowed for Default Back Bone ports */
    if(_BCM_CALADAN3_MIM_DEF_BACKBONE_PORT != portcb->type) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    if(BCM_GPORT_IS_TRUNK(portcb->port)) {
        tdata = sal_alloc(sizeof(bcm_trunk_add_info_t), "trunk add info");
        if (tdata == NULL) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not allocate trunk add info structure !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_MEMORY;
        }

        /* Compare mymodid with trunk module array, if any match allocate
         * egress resource else skip */
        status = bcm_caladan3_trunk_get_old(unit, 
                               BCM_GPORT_TRUNK_GET(portcb->port),
                               tdata);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Trunk Get Failure: %d, gport=0x%08x\n"),
                       status, mim_port->port));
            sal_free(tdata);
            return BCM_E_INTERNAL;
        }
        tnumports = tdata->num_ports;
        for(index=0; index <BCM_TRUNK_MAX_PORTCNT; index++) {
            tp[index] = -1;
        }
 
    } else {

        /* If this is a remote module skip port mode configuration */
        if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "Skipping Port configuration since module is remote")));
            return BCM_E_NONE;
        }
        
        port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
        if (_MIM_PORT_INVALID(unit, port)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Invalid port: %d, gport=0x%08x\n"),
                       port, mim_port->port));
            return BCM_E_PARAM;
        }
        tnumports=1;
    }

    /* set local station match entry on ppe */
    soc_sbx_g3p1_lsmac_t_init(&lsmmac);

    lsmmac.useport = 0;
    lsmmac.mac[0] = portcb->smac[0];
    lsmmac.mac[1] = portcb->smac[1];
    lsmmac.mac[2] = portcb->smac[2];
    lsmmac.mac[3] = portcb->smac[3];
    lsmmac.mac[4] = portcb->smac[4];
    lsmmac.mac[5] = portcb->smac[5];

    status = soc_sbx_g3p1_lsmac_set (unit, portcb->hwinfo.ismacidx, &lsmmac);
    /* set mim mode on p2e */
    if(BCM_SUCCESS(status)) {
        for(index=0; index < tnumports; index++) {

            if(BCM_GPORT_IS_TRUNK(portcb->port)) {
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
                } else {
                    continue;
                }
            }

            if(((soc_sbx_g3p1_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
               ((soc_sbx_g3p1_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

                p2e.mim         = MIM_BBONE_PORT;
                p2e.customer    = 0;
                p2e.provider    = 1;
                p2e.defstrip    = 0;
                p2e.pstrip      = 0;
                p2e.pbb         = 1;

                /* if trunk set trunk base FTE as PID (or) set SID as PID */
                ep2e.customer   = 1;
                ep2e.mim        = MIM_BBONE_PORT;
                ep2e.pbb        = 1;

                /* program P2E */
                status = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

                if(BCM_SUCCESS(status)) {
                    /* program EP2E */
                    status = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);

                    if(BCM_FAILURE(status)) {

                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Programming EP2E %d : %s in (%s,%d)\n"),
                                   FUNCTION_NAME(), status, bcm_errmsg(status),
                                   __FILE__,__LINE__));
                    } else {
                        soc_sbx_g3p1_pv2e_t_init(&pv2e);

                        status = soc_sbx_g3p1_pv2e_get(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                        if(BCM_FAILURE(status)) {

                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Getting PV2E %d : %s in (%s,%d)\n"),
                                       FUNCTION_NAME(), status, bcm_errmsg(status),
                                       __FILE__,__LINE__));
                        } else {
                            /* Program Bvlan, port 2 Etc -> LPI */
                            /* lpi on pv2e for back bone must be 0 to use physical port as lp */
                            pv2e.lpi = 0;

                            status =  soc_sbx_g3p1_pv2e_set(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                            if(BCM_FAILURE(status)) {

                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "ERROR %s Programming PV2E %d : %s in (%s,%d)\n"),
                                           FUNCTION_NAME(), status, bcm_errmsg(status),
                                           __FILE__,__LINE__));
                            }
                        }
                    }

                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Programming P2E %d : %s in (%s,%d)\n"),
                               FUNCTION_NAME(), status, bcm_errmsg(status),
                               __FILE__,__LINE__));
                }

            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s Getting P2E & EP2E in (%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__,__LINE__));
            }
        }
    }
    if (tdata != NULL) {
        sal_free(tdata);
    }

    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_create_access_port(int                          unit,
                                       bcm_mim_port_t              *mim_port,
                                       bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, result;
    int mymodid = 0;

    if(!mim_port || !portcb) {
        status = BCM_E_PARAM;
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "WARNING %s Bad Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
    } else {
        result = bcm_sbx_stk_modid_get(unit, &mymodid);

        if(BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            status = BCM_E_INTERNAL;
        } else {
            if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT) {
                /* Configure Port Based Interface */
                portcb->service = _BCM_CALADAN3_MIM_PORT;

            } else if ((mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) &&
                           (mim_port->flags & BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD)){
                /* Configure 1:1 STAG Based Interface */
                portcb->service = _BCM_CALADAN3_MIM_STAG_1_1;
                portcb->vlan    = mim_port->egress_service_vlan;

            } else if (mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) {
                /* Configure Bundled STAG Based Interface */
                portcb->service = _BCM_CALADAN3_MIM_STAG_BUNDLED;
                portcb->vlan    = mim_port->match_vlan;

            } else {
                /* should never hit here */
            }

            /* ALLOCATE LP, FT, ENCAPID (OI), ENCAP ETE AND L2 ETE */
            status = _bcm_caladan3_mim_g3p1_alloc_inseg(unit,
                                                      mim_port->flags,
                                                      portcb);
            if(BCM_SUCCESS(status)) {
                status = _bcm_caladan3_mim_g3p1_alloc_outseg(unit,
                                                           mymodid,
                                                           mim_port->flags,
                                                           &mim_port->encap_id,
                                                           portcb);
                if(BCM_SUCCESS(status)) {
                    /* Map OI to Ete  */
                    status = _bcm_caladan3_mim_g3p1_provision_outseg(unit,
                                                                   mymodid,
                                                                   portcb);
                    if(BCM_SUCCESS(status)) {
                        /* Map FT to Encap */
                        status = _bcm_caladan3_mim_g3p1_map_inseg_outseg(unit,
                                                                       portcb);
                    }
                }/* alloc_outseg */
            }/* alloc_inseg */

            if(BCM_SUCCESS(status)) {
                /* Provision access interface setting */
                switch(portcb->service) {
                case _BCM_CALADAN3_MIM_PORT:
                    /* Configure Port Based Interface */
                    status = _bcm_caladan3_mim_config_port_interface(unit, mim_port, portcb);
                    break;

                case _BCM_CALADAN3_MIM_STAG_1_1:
                    /* Configure 1:1 STAG Based Interface */
                case _BCM_CALADAN3_MIM_STAG_BUNDLED:
                    /* Configure Bundled STAG Based Interface */
                    status = _bcm_caladan3_mim_config_stag_interface(unit, mim_port, portcb);
                    break;

                default:
                    /* Should never hit here since port is validated for unsupported
                     * flags & criteria */
                    status = BCM_E_PARAM;
                    break;
                }
            }

            /* Free back resources if allocation failed */
            if(BCM_FAILURE(status)){
                _bcm_caladan3_mim_g3p1_free_inseg(unit,
                                                portcb);

                _bcm_caladan3_mim_g3p1_free_outseg(unit,
                                                 portcb);
            }
        }/* my mod id get */
    }

    return status;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_mim_create_back_bone_port(int                          unit,
                                          bcm_mim_port_t              *mim_port,
                                          bcm_caladan3_mim_port_control_t *portcb,
                                          bcm_caladan3_mim_port_control_t *defbbportcb)
{
    int status = BCM_E_NONE, result;
    int mymodid = 0;

    if(!mim_port || !portcb) {
        status = BCM_E_PARAM;
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "WARNING %s Bad Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
    } else {

        result = bcm_sbx_stk_modid_get(unit, &mymodid);

        if(BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            status = BCM_E_INTERNAL;
        }

        if((_BCM_CALADAN3_MIM_DEF_BACKBONE_PORT != portcb->type) && 
           (!defbbportcb)) {
            status = BCM_E_PARAM;
        }

        if(BCM_SUCCESS(status)) {

            /* ALLOCATE LP, FT, ENCAPID (OI), ENCAP ETE AND L2 ETE */
            status = _bcm_caladan3_mim_g3p1_alloc_inseg(unit,
                                                      mim_port->flags,
                                                      portcb);

            if(BCM_SUCCESS(status)) {

                status = _bcm_caladan3_mim_g3p1_alloc_outseg(unit,
                                                           mymodid,
                                                           mim_port->flags,
                                                           &mim_port->encap_id,
                                                           portcb);
                if(BCM_SUCCESS(status)) {
                    /* Map OI to Ete */
                    status = _bcm_caladan3_mim_g3p1_provision_outseg(unit,
                                                                   mymodid,
                                                                   portcb);
                    if(BCM_SUCCESS(status)) {
                        /* Map FT to Encap */
                        status = _bcm_caladan3_mim_g3p1_map_inseg_outseg(unit,
                                                                       portcb);
                    }
                }/* alloc_outseg */
            }/* alloc_inseg */

            /* configure default back bone port */
            if(BCM_SUCCESS(status)) {
               if(_BCM_CALADAN3_MIM_DEF_BACKBONE_PORT == portcb->type) {
                   status = _bcm_caladan3_mim_config_bbone_interface(unit, mim_port, portcb);
               } else {
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
                    /* create bsmac */
                    soc_sbx_g2p3_bmac_t bsmac;
                    int module, port, fabport, fabunit, fabnode;

                    soc_sbx_g2p3_bmac_t_init(&bsmac);

                    if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                        bsmac.bpid = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                    } else {
                        /* obtain physical SID */
                        module = BCM_GPORT_MODPORT_MODID_GET(portcb->port);
                        port   = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
                        status = soc_sbx_node_port_get(unit, module, port,
                                                       &fabunit, &fabnode, &fabport);
                        if(BCM_SUCCESS(status)) {
                            bsmac.bpid = SOC_SBX_PORT_SID(unit, fabnode, fabport);
                        }
                    }
                    if(BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "unit %d: [%s] error %s Mapping FE to QE port information"),
                                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
                    } else {
                        bsmac.btid = portcb->hwinfo.ftidx;
                        status = soc_sbx_g2p3_bmac_set(unit,
                                                       portcb->dmac,
                                                       portcb->vlan, &bsmac);

                        if(BCM_FAILURE(status)) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n"),
                                       unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                        } else {
                            LOG_VERBOSE(BSL_LS_BCM_MIM,
                                        (BSL_META_U(unit,
                                                    "unit %d: [%s] Added %s Setting BSMAC"MAC_FMT"\n"),
                                         unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                        }
                   }
#endif
                   /* Inherit Parnet Default Back Bone Properties if applicable */
                    if(defbbportcb->hwinfo.egrremarkidx > 0) {

                        status = _bcm_caladan3_mim_set_egress_remarking(unit,
                                                                      portcb->hwinfo.ete,
                                                                      defbbportcb->hwinfo.egrremarkidx,
                                                                      defbbportcb->hwinfo.egrremarkflags);
                        if(BCM_SUCCESS(status)) {
                            LOG_VERBOSE(BSL_LS_BCM_MIM,
                                        (BSL_META_U(unit,
                                                    "Inheritting Egress remarking=0x%x from default backbone\n"),
                                         defbbportcb->hwinfo.egrremarkidx));
                            portcb->hwinfo.egrremarkidx = defbbportcb->hwinfo.egrremarkidx;
                            portcb->hwinfo.egrremarkflags = defbbportcb->hwinfo.egrremarkflags;
                        }  else {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "Error[%d] %s Inheritting Egress remarking !!!!(%s,%d)\n"),
                                       status, FUNCTION_NAME(), __FILE__, __LINE__));
                        }
                    }
               }
            }

            /* Free back resources if allocation failed */
            if(BCM_FAILURE(status)) {
                _bcm_caladan3_mim_g3p1_free_inseg(unit,
                                                portcb);

                _bcm_caladan3_mim_g3p1_free_outseg(unit,
                                                 portcb);
            }
        }/* my mod id get */
    }

    return status;
}
#endif

static int _bcm_caladan3_mim_port_free(int unit,
                                     bcm_caladan3_mim_port_control_t **ppPortCtl)
{
    int status = BCM_E_NONE;
#ifdef  BCM_CALADAN3_MIM_SUPPORT 
    int mymodid, tnumports=1, index, idx=0, pindex=0, port=0;
    soc_sbx_g3p1_pv2e_t pv2e;
    bcm_caladan3_mim_port_control_t *portcb;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    uint32 portid;

    if(!ppPortCtl || !*ppPortCtl) {
        return BCM_E_PARAM;
    } 
    portcb = (*ppPortCtl);
    portid = BCM_GPORT_MIM_PORT_ID_GET(portcb->gport);

    if ((portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) ||
        (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT)) {

        status = bcm_sbx_stk_modid_get(unit, &mymodid);
            if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_INTERNAL;
        }

        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            bcm_trunk_add_info_t *tdata;

            tdata = sal_alloc(sizeof(bcm_trunk_add_info_t), "trunk add info");
            if (tdata == NULL) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] Unable to allocate trunk info structure\n"),
                           unit, FUNCTION_NAME()));
                return BCM_E_MEMORY;
            }

            /* Compare mymodid with trunk module array, if any match allocate
             * egress resource else skip */
            status = bcm_caladan3_trunk_get_old(unit, 
                                   BCM_GPORT_TRUNK_GET(portcb->port),
                                   tdata);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "Trunk Get Failure: %d, gport=0x%08x\n"),
                           status, portcb->port));
                sal_free(tdata);
                return BCM_E_INTERNAL;
            }   
         
            tnumports = tdata->num_ports;

            for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
                tp[index] = -1;
            }
            
            for(index=0; index < tnumports; index++) {

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
                } else {
                    continue;
                }
            }

            tnumports = pindex;

            sal_free(tdata);

        } else {
            tnumports = 0;

            if(mymodid == BCM_GPORT_MODPORT_MODID_GET(portcb->port)) {
                tnumports = 1;
                tp[0] = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
            }
        }

        for(index=0; index < tnumports; index++) {
            if(portcb->hwinfo.lport) {
                /* since BVLAN uses TB, clear LP on pv2e */
                soc_sbx_g3p1_pv2e_t_init(&pv2e);
                pv2e.vlan = portcb->vlan;
                status = soc_sbx_g3p1_pv2e_set(unit, portcb->vlan, tp[index], &pv2e);

                LOG_VERBOSE(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "Clearing pv2e[%d,0x%03x]\n"),
                             BCM_GPORT_MODPORT_PORT_GET(portcb->port), portcb->vlan));

                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Cound not Clear PV2E Port[%d] VID[0x%x] Error[%s] (%s,%d)\n"),
                               FUNCTION_NAME(), tp[index],
                               portcb->vlan, bcm_errmsg(status),
                               __FILE__,__LINE__));
                }
            }
        }               
    }

    _bcm_caladan3_mim_g3p1_free_inseg(unit,
                                    portcb);

    _bcm_caladan3_mim_g3p1_free_outseg(unit,
                                     portcb);

    if(_BCM_CALADAN3_MIM_BACKBONE_PORT == portcb->type) {
        /* delete bsmac */
        status = soc_sbx_g2p3_bmac_remove(unit, portcb->dmac, portcb->vlan);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status),
                       MAC_PFMT(portcb->dmac)));
        }
    }

    /* invalidate Gport to Lport mapping */
    SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = NULL;
    SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_INVALID;
    caladan3_gportinfo[unit].lpid[portid] = 0;

    sal_free(portcb);
    portcb = NULL;

#endif
    return status;
}

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_dump_portcb(bcm_caladan3_mim_port_control_t *portcb)
{
    if(portcb) {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META(" Mim Gport Dump ##: \n")));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("GPORT ID : [0x%x] Mod Port ID[0x%x] \n"),
                     portcb->gport, portcb->port));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("Vlan: [0x%x] ISID-Vlan:[0x%x] ISID[0x%x]"),
                     portcb->vlan, portcb->isidvlan, portcb->isid));

        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("FTIDX:[0x%x] OI:[0x%x] ETE[0x%x] \n"),
                     portcb->hwinfo.ftidx, portcb->hwinfo.ohi,
                     portcb->hwinfo.ete));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("LPORT[0x%x] ISMAC:[0x%x] ESMAC:[0x%x] \n"),
                     portcb->hwinfo.lport,portcb->hwinfo.ismacidx, portcb->hwinfo.esmacidx));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("Type[%d], Service[%d], RefCount[%d], Criteria[0x%x] \n"),
                     portcb->type, portcb->service, portcb->refcount, portcb->criteria));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("DMac[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x \n"),
                     portcb->dmac[0], portcb->dmac[1], portcb->dmac[2],
                     portcb->dmac[3], portcb->dmac[4], portcb->dmac[5]));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("SMac[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x \n"),
                     portcb->smac[0], portcb->smac[1], portcb->smac[2],
                     portcb->smac[3], portcb->smac[4], portcb->smac[5]));
    }
    return BCM_E_NONE;
}
#endif

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static
int _bcm_caladan3_dump_vpncb(bcm_caladan3_mim_vpn_control_t *vpncb)
{
    if(vpncb) {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("VPN Dump ##\n")));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("VPNID[0x%x] ISID[0x%x]\n"),
                     vpncb->vpnid, vpncb->isid));
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META("Bcast Group[0x%x] Unk.Ucast[0x%x] Unk.Mcast[0x%x]\n"),
                     vpncb->broadcast_group, vpncb->unknown_unicast_group,
                     vpncb->unknown_multicast_group));
    }
    return BCM_E_NONE;
}
#endif

static
int _bcm_caladan3_mim_g3p1_setup_learning(int unit, int disable)
{
    soc_sbx_g3p1_xt_t xt;
    uint32 xtidx=0;
    int rv=0;

    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_exc_bmac_learn_idx_get(unit, &xtidx));

    rv = soc_sbx_g3p1_xt_get(unit, xtidx, &xt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Failed to get BMAC learn exception entry "
                               "unit=%d rv=%d(%s)\n"),
                   unit, rv, bcm_errmsg(rv)));
        return rv;
    }

    xt.forward = (disable)?0:1;
    rv = soc_sbx_g3p1_xt_set(unit, xtidx, &xt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Failed to set BMAC learn exception entry "
                               "unit=%d rv=%d(%s)\n"),
                   unit, rv, bcm_errmsg(rv)));
        return rv;
    }
    return BCM_E_NONE;
}


int _bcm_caladan3_mim_port_delete(int                          unit,
                                bcm_caladan3_mim_vpn_control_t  *vpncb,
                                bcm_caladan3_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(portcb && vpncb) {

      if(_BCM_CALADAN3_MIM_DEF_BACKBONE_PORT == portcb->type) {
          /* verify if all back bone ports over this default bbone port were deleted */
          if(portcb->refcount) {
              LOG_ERROR(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "ERROR %s Default Back bone port has non-zero Backbone ports associated !!!!(%s,%d)\n"),
                         FUNCTION_NAME(), __FILE__, __LINE__));
              status = BCM_E_PARAM;
          } else {
              DQ_REMOVE(&portcb->listnode);

              if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                  DQ_REMOVE(&portcb->trunklistnode);
              }
              
              status = _bcm_caladan3_mim_port_free(unit, &portcb);
              if(BCM_FAILURE(status)) {
                  LOG_ERROR(BSL_LS_BCM_MIM,
                            (BSL_META_U(unit,
                                        "ERROR %s Count not Free back Default Backbone Port[0x%x] !!!!(%s,%d)\n"),
                             FUNCTION_NAME(),portcb->gport, __FILE__, __LINE__));
              }
          }
      } else {
          /* if back bone de-reference it with default back bone port */
          if(_BCM_CALADAN3_MIM_BACKBONE_PORT == portcb->type) {
              dq_p_t port_elem;
              bcm_caladan3_mim_port_control_t *defbbportcb = NULL;

              /* find the port on default BB List*/
              DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

                  _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

                  /* if this back bone port is associated to this default back bone port deref */
                  if(defbbportcb->port ==  portcb->port){
                      defbbportcb->refcount--;
                      break;
                  }
              } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

          } else if(_BCM_CALADAN3_MIM_ACCESS_PORT == portcb->type) {
              /* if port mode is access and Port Based interface, set pv2e to default */
              if(_BCM_CALADAN3_MIM_PORT == portcb->service) {
                  _bcm_caladan3_mim_reset_port_interface(unit, portcb);
              }
          }

          /* remove this port from port list of vpn */
          DQ_REMOVE(&portcb->listnode);

          status = _bcm_caladan3_mim_port_free(unit, &portcb);
      }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

/*-------- Static Functions End ----------*/
/* BCM API - Dispatcher Functions */

/*
 * Function:
 *   bcm_caladan3_mim_init
 * Purpose:
 *     Initializes Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int bcm_caladan3_mim_init(int unit)
{
    uint32 min, max;
    int result, i, j;

    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if ((mim_mlock[unit] = sal_mutex_create("bcm_mim_lock")) == NULL) {
        return BCM_E_MEMORY;
    } else {
        /* unit re-initialized without detach flag error */
        /* return BCM_E_EXISTS; */
    }

    result = _bcm_caladan3_mim_g3p1_setup_learning(unit, 0);
    if(BCM_SUCCESS(result)) {

        if(caladan3_mim_vpn_db[unit]){
            
            
            shr_avl_destroy(caladan3_mim_vpn_db[unit]);
            caladan3_mim_vpn_db[unit] = NULL;
            _bcm_caladan3_mim_g3p1_setup_learning(unit, 1);
        }

        /* Determine Maximum VPN that can be allocated */
        result = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_VSI,
                                      &min, &max);
        if(BCM_FAILURE(result)) {
            /* free back allocated mutex */
            sal_mutex_destroy(mim_mlock[unit]);
            mim_mlock[unit] = NULL;

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not allocate VPN VSI (%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_INTERNAL;
        } else {
            caladan3_mim_vpn_db[unit] = NULL;

            /* Create a VPN database on this unit */
            result = shr_avl_create(&caladan3_mim_vpn_db[unit], INT_TO_PTR(unit),
                                    sizeof(bcm_caladan3_mim_vpn_control_t),(max-min+1));
            if(BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s Could not create VPN database (%s, %d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));
                return result;
            }

            if(NULL == caladan3_mim_vpn_db[unit]){
                /* free back allocated mutex */
                sal_mutex_destroy(mim_mlock[unit]);
                mim_mlock[unit] = NULL;

                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s Could not allocate memory(%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));

                return BCM_E_MEMORY;
            } else {
                sal_memset(&caladan3_gportinfo[unit], 0, sizeof(_bcm_caladan3_mimgp_info_t));
                caladan3_mim_vpn_db[unit]->datum_copy_fn = _bcm_caladan3_mim_vpn_copy_datum;

                /* clear trunk association structure */
                for(i=0; i < BCM_MAX_NUM_UNITS; i++) {
                    for(j=0; j < SBX_MAX_TRUNKS; j++) {
                        caladan3_mim_trunk_assoc_info[i][j].mimType = _BCM_CALADAN3_MIM_PORT_MAX; /*invalid*/
                        /* initialize the port list */
                        DQ_INIT(&caladan3_mim_trunk_assoc_info[i][j].plist);
                    }
                }

                result = bcm_caladan3_trunk_change_register(unit, _bcm_caladan3_mim_trunk_cb, NULL);
                if(BCM_FAILURE(result)) {

                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Could not register for trunk change Error:%d  (%s,%d)\n"),
                               FUNCTION_NAME(),result, __FILE__, __LINE__));

                    /* free vpndb */
                    shr_avl_destroy(caladan3_mim_vpn_db[unit]);
                    caladan3_mim_vpn_db[unit] = NULL;

                    /* free back allocated mutex */
                    sal_mutex_destroy(mim_mlock[unit]);
                    mim_mlock[unit] = NULL;
                }
            }
        }
#ifdef BCM_WARM_BOOT_SUPPORT
        result = bcm_caladan3_wb_mim_state_init(unit);
#endif

        return BCM_E_NONE;
    } else {
        return BCM_E_INTERNAL;
    }
}

/*
 * Function:
 *   bcm_caladan3_mim_detach
 * Purpose:
 *     UnInitializes Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int bcm_caladan3_mim_detach(int unit)
{
#ifdef  BCM_CALADAN3_MIM_SUPPORT 
    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    
    /* free the vpn database */
    if(caladan3_mim_vpn_db[unit]){
        
        
        shr_avl_destroy(caladan3_mim_vpn_db[unit]);
        caladan3_mim_vpn_db[unit] = NULL;
        _bcm_caladan3_mim_g3p1_setup_learning(unit, 1);
    }
    _MIM_UNLOCK(unit);
    /* free back allocated mutex */
    sal_mutex_destroy(mim_mlock[unit]);
#endif /* BCM_CALADAN3_MIM_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *   bcm_caladan3_mim_vpn_create
 * Purpose:
 *     Creates a  Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *   unit  - (IN)     fe unit to initialize
 *   info  - (IN)     vpn configuration
 * Returns:
 *   BCM_E_XX
 */
int bcm_caladan3_mim_vpn_create(int                   unit,
                              bcm_mim_vpn_config_t *info)
{
    int                         status = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
    uint32                      alloc_vsi = ~0;
    int                         res_flags;
    int                         result;
    bcm_caladan3_mim_vpn_control_t  vpninfo, *vpncb = NULL;
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
    soc_sbx_g3p1_v2e_t          vpnv2e;
#endif

    status = BCM_E_NONE;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(!info) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(info->flags & ~_BCM_CALADAN3_MIM_SUPPORTED_FLAGS) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* if BCM_MIM_VPN_WITH_ID, verify if the vpn id is free */
    if (info->flags & BCM_MIM_VPN_WITH_ID) {
        alloc_vsi = info->vpn;
        res_flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
    } else {
        res_flags = 0;
    }

    /* if BCM_MIM_VPN_REPLACE, obtain necessary information of
     * vpn & update */
    status = _sbx_caladan3_resource_alloc(unit,
                                     SBX_CALADAN3_USR_RES_VSI,
                                     1,
                                     &alloc_vsi,
                                     res_flags);

#if 0
    /* Code currently not functional 
    if(res_flags & BCM_MIM_PORT_WITH_ID) {
        if(status == BCM_E_RESOURCE) {
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "DEBUG Reserved VPNID (0x%x) (%s,%d)\n"),
                         alloc_vsi, __FILE__, __LINE__));
            status = BCM_E_NONE;
        } else {
            status = BCM_E_FULL;
        }
    }
    */
#endif

    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not allocate VSI for VPN (%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        alloc_vsi = _MIM_INVALID_VPN_ID;
    } else {
        uint32 lport=0;

        /* allocate Logical port for the VPN used for instrumentation + counting */
        status = _sbx_caladan3_resource_alloc(unit,
                                         SBX_CALADAN3_USR_RES_LPORT,
                                         1,
                                         &lport,
                                         0);
        if(BCM_SUCCESS(status)) {
            /* allocate vpn db element */
            sal_memset(&vpninfo, 0, sizeof(bcm_caladan3_mim_vpn_control_t));
            vpninfo.vpnid = alloc_vsi;
            vpninfo.isid  = info->lookup_id;

            /* Verify if the ISID & VPNID is not part of any other VPN */
            result = shr_avl_lookup(caladan3_mim_vpn_db[unit],
                                    _bcm_caladan3_mim_vpn_lookup_compare_nodata,
                                    (shr_avl_datum_t*)&vpninfo);
            /* validate is the vpn or isid exists on the tree */
            if(result){
                status = BCM_E_EXISTS;
            } else {
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
                soc_sbx_g2p3_isid2e_t isid2e;
                soc_sbx_g3p1_ft_t fte;
                uint32 ftidx=0, unknown_mc_ft_offset=0;
#endif

                vpninfo.lport = lport;
                
                vpninfo.broadcast_group         = info->broadcast_group;
                vpninfo.unknown_unicast_group   = info->unknown_unicast_group;
                vpninfo.unknown_multicast_group = info->unknown_multicast_group;
                vpninfo.policer_id              = -1;
                /* info -> match_service_tpid -> not used */

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
                /* Make sure Learning is Enabled on the VPN VSI */
                soc_sbx_g3p1_v2e_t_init(&vpnv2e);
                result = soc_sbx_g3p1_v2e_set(unit, alloc_vsi, &vpnv2e);

                if(BCM_SUCCESS(result)) {
                    /* provision the Unknown Unicast and unknown Multicast FTE */
                    soc_sbx_g3p1_ft_t_init(&fte);
                    result = soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx);
                    if(BCM_SUCCESS(result)){
                        fte.oi  = info->broadcast_group;
                        fte.mc  = 1;
                        fte.qid = SBX_MC_QID_BASE;
                        ftidx += vpninfo.vpnid;
                        result = soc_sbx_g3p1_ft_set(unit, ftidx, &fte);
                        if(BCM_SUCCESS(result)){
                            result =  soc_sbx_g3p1_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
                            if(BCM_SUCCESS(result)) {
                                fte.oi  = info->unknown_multicast_group;
                                fte.mc  = 1;
                                fte.qid = SBX_MC_QID_BASE;
                                result = soc_sbx_g3p1_ft_set(unit,
                                                             ftidx + unknown_mc_ft_offset,
                                                             &fte);
                                if(BCM_SUCCESS(result)) {
                                    soc_sbx_g3p1_lp_t lp;
                                    soc_sbx_g3p1_lp_t_init(&lp);
                                    /* In future Set Policer + Stats on VPN service LP */
                                    /* PID is dont care */
                                    result = soc_sbx_g3p1_lp_set(unit,
                                                                 lport,
                                                                 &lp);
                                    if(BCM_FAILURE(result)) {
                                        LOG_ERROR(BSL_LS_BCM_MIM,
                                                  (BSL_META_U(unit,
                                                              "unit %d: [%s] error %s in programming LP 0x%x\n"),
                                                   unit, FUNCTION_NAME(), bcm_errmsg(status),lport));
                                    } else {

                                        /* Add a ISID2E entry to resolve to the above VSI */
                                        soc_sbx_g2p3_isid2e_t_init(&isid2e);
                                        isid2e.lpi  = lport;
                                        isid2e.vlan = vpninfo.vpnid;
                                        result = soc_sbx_g2p3_isid2e_set(unit, vpninfo.isid, &isid2e);
                                    }
                                } else {
                                    LOG_ERROR(BSL_LS_BCM_MIM,
                                              (BSL_META_U(unit,
                                                          "ERROR %s Setting Unknown Multcast Flood Group  (%s,%d)\n"),
                                               FUNCTION_NAME(), __FILE__, __LINE__));
                                }
                            } else {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "ERROR %s Getting Unknown Multcast Offset (%s,%d)\n"),
                                           FUNCTION_NAME(), __FILE__, __LINE__));
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Setting Unknown Unicast Flood Group  (%s,%d)\n"),
                                       FUNCTION_NAME(), __FILE__, __LINE__));
                        }
                    } else {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Getting Unknown Unicast Offset (%s,%d)\n"),
                                   FUNCTION_NAME(), __FILE__, __LINE__));
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Setting VPN VSI V2E (%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                }
#endif

                if(BCM_FAILURE(result)) {

                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Setting Forwarding Path for VPN (%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                    status = BCM_E_INTERNAL;
                } else {

                    /* If not error, insert ot the vpn db tree */
                    result = shr_avl_insert(caladan3_mim_vpn_db[unit],
                                            _bcm_caladan3_mim_insert_compare,
                                            (shr_avl_datum_t*)&vpninfo);
                    if(BCM_FAILURE(result)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Could not insert into VPN database tree (%s,%d)\n"),
                                   FUNCTION_NAME(), __FILE__, __LINE__));
                        status = BCM_E_INTERNAL;
                    } else {
                        /* since avl allocates memory for vpncb,
                           the tree has to be looked again to get real vpncb*/
                        _bcm_caladan3_mim_lookup_data_t hdl;

                        /* obtain the VPN from database */
                        hdl.key = &vpninfo;
                        hdl.datum = &vpncb;

                        if(!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
                            result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                                           _bcm_caladan3_mim_vpn_lookup_compare,
                                                           (shr_avl_datum_t*)&vpninfo,       
                                                           (void*)&hdl);
                            /* If vpn was found on the database */
                            if(result > 0){
                                /* initialize the port list */
                                DQ_INIT(&vpncb->def_bbone_plist);
                                DQ_INIT(&vpncb->vpn_access_sap_head);
                                DQ_INIT(&vpncb->vpn_bbone_sap_head);
                            } else {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "ERROR %s Could not find vpncb on VPN database tree (%s,%d)\n"),
                                           FUNCTION_NAME(), __FILE__, __LINE__));
                                status = BCM_E_INTERNAL;
                            }
                        }
                    }
                }
            }
        } else {

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not allocate LPORT for VPN (%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
        }
    }

    if(BCM_SUCCESS(status)) {
        /* set vsi type */
        status = _sbx_caladan3_set_vsi_type(unit, alloc_vsi, BCM_GPORT_MIM_PORT);
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not Set VSI type (%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));            
            status = BCM_E_INTERNAL;
        }
    }

    if(BCM_FAILURE(status)) {

        if(vpncb){
            /* free back logical port */
            result = _sbx_caladan3_resource_free(unit,
                                            SBX_CALADAN3_USR_RES_LPORT,
                                            1,
                                            &vpncb->lport,
                                            0);
            if(BCM_FAILURE(result)) {
                /* cant do much just log this error */
                LOG_WARN(BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "WARNING %s Could not Free back LPORT(%s,%d)\n"),
                          FUNCTION_NAME(), __FILE__, __LINE__));
            }

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
            /* Delete ISID2E */
            result = soc_sbx_g2p3_isid2e_delete(unit,
                                                vpncb->isid);
            if(BCM_FAILURE(result)) {
                /* cant do much just log this error */
                LOG_WARN(BSL_LS_BCM_MIM,
                         (BSL_META_U(unit,
                                     "WARNING %s Could not Delete ISID2E[0x%x] (%s,%d)\n"),
                          FUNCTION_NAME(), vpncb->isid, __FILE__,__LINE__));
            }
#endif
        }

        /* Free back VSI */
        result = _sbx_caladan3_resource_free(unit,
                                        SBX_CALADAN3_USR_RES_VSI,
                                        1,
                                        &alloc_vsi,
                                        res_flags);
        if(BCM_FAILURE(result)) {

            /* cant do much just log this error */
             LOG_WARN(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "WARNING %s Could not Free back VSI(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
        }

    } else {
        info->vpn = alloc_vsi;

    }

    if(BCM_SUCCESS(status)) {
        _bcm_caladan3_dump_vpncb(vpncb);
    }

    _MIM_UNLOCK(unit);
#endif /* BCM_CALADAN3_MIM_SUPPORT */
    return status;
}

int bcm_caladan3_mim_vpn_destroy(int           unit,
                               bcm_mim_vpn_t  info)
{
    int                         status=BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_SUPPORT
    int                         result;
    bcm_caladan3_mim_vpn_control_t  dummycb, *vpncb;
    _bcm_caladan3_mim_lookup_data_t hdl;
    status = BCM_E_NONE;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    dummycb.vpnid = info;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    if(!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                       _bcm_caladan3_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb, 
                                       (void*)&hdl);
        if(result > 0)/*found*/{
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "DEBUG: %s VPN ID [%d] Found on database !!!!(%s,%d)\n"),
                         FUNCTION_NAME(),info, __FILE__, __LINE__));

            
            /* Verify if all port controls or SAP are empty before the vpn can be destroyed */
            if((DQ_EMPTY(&vpncb->vpn_access_sap_head)) &&
               (DQ_EMPTY(&vpncb->def_bbone_plist)) && 
               (DQ_EMPTY(&vpncb->vpn_bbone_sap_head))) {
                /* free back the VPN VSI */
                uint32 alloc_vsi = vpncb->vpnid;

                result = _sbx_caladan3_resource_free(unit,
                                            SBX_CALADAN3_USR_RES_VSI,
                                            1,
                                            &alloc_vsi,
                                            0);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    LOG_WARN(BSL_LS_BCM_MIM,
                             (BSL_META_U(unit,
                                         "WARNING %s Could not Free back VSI[0x%x] (%s,%d)\n"),
                              FUNCTION_NAME(), alloc_vsi, __FILE__,__LINE__));
                }

                /* free back the VPN LPORT */
                result = _sbx_caladan3_resource_free(unit,
                                            SBX_CALADAN3_USR_RES_LPORT,
                                            1,
                                            &vpncb->lport,
                                            0);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    LOG_WARN(BSL_LS_BCM_MIM,
                             (BSL_META_U(unit,
                                         "WARNING %s Could not Free back LPORT[0x%x] (%s,%d)\n"),
                              FUNCTION_NAME(), vpncb->lport, __FILE__,__LINE__));
                }

                /* Delete ISID2E */
                result = soc_sbx_g2p3_isid2e_delete(unit,
                                                    vpncb->isid);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    LOG_WARN(BSL_LS_BCM_MIM,
                             (BSL_META_U(unit,
                                         "WARNING %s Could not Delete ISID2E[0x%x] (%s,%d)\n"),
                              FUNCTION_NAME(), vpncb->isid, __FILE__,__LINE__));
                }

                /* remove and destroy vpn tree node */
                result = shr_avl_delete(caladan3_mim_vpn_db[unit],
                                        _bcm_caladan3_mim_insert_compare,
                                        (shr_avl_datum_t*)&dummycb);
                if(BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Could not delete from VPN database tree (%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                    status = BCM_E_INTERNAL;
                }

                if(BCM_SUCCESS(status)) {
                    /* set vsi type */
                    status = _sbx_caladan3_set_vsi_type(unit, alloc_vsi, BCM_GPORT_TYPE_NONE);
                    if(BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR %s Could not Set VSI type (%s,%d)\n"),
                                   FUNCTION_NAME(), __FILE__, __LINE__));            
                        status = BCM_E_INTERNAL;
                    }
                }

            } else {
                dq_p_t port_elem;
                bcm_caladan3_mim_port_control_t *portcb = NULL;

                if (!(DQ_EMPTY(&vpncb->vpn_access_sap_head))) {
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "ACCESS SAP:\n")));
                    DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                                 " DMAC=" MAC_FMT "\n"), portcb->gport, portcb->isid,
                                     MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END (&vpncb->vpn_access_sap_head, port_elem);
                }

                if (!(DQ_EMPTY(&vpncb->vpn_bbone_sap_head))) {
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "BACKBONE SAP:\n")));
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {
                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                                " DMAC=" MAC_FMT "\n"), portcb->gport, portcb->isid,
                                     MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END (&vpncb->vpn_bbone_sap_head, port_elem);
                }

                if (!(DQ_EMPTY(&vpncb->def_bbone_plist))) {
                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                (BSL_META_U(unit,
                                            "Backbone:\n")));
                    DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {
                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                                " DMAC=" MAC_FMT "\n"), portcb->gport, portcb->isid,
                                     MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
                }

                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR %s VPN Service Access Points are not empty (%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));
                status = BCM_E_PARAM;

            }
        } else {
            /* If vpn was found on the database */
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not find VPN in VPN database tree (%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            status = BCM_E_PARAM;
        }
    } else {
        /* If vpn was found on the database */
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s VPN not found !! VPN Tree Empty (%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        status = BCM_E_PARAM;
    }

    _MIM_UNLOCK(unit);
#endif
    return status;
}

int bcm_caladan3_mim_vpn_destroy_all(int unit)
{
  return BCM_E_UNAVAIL;
}


#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
/* Assumes locks have been taken */
static int
_bcm_caladan3_mim_vpn_control_get(int                   unit,
                                bcm_mim_vpn_t         vpn,
                                bcm_caladan3_mim_vpn_control_t **vpnCtl)
{
    int result;
    bcm_caladan3_mim_vpn_control_t  dummycb;
    _bcm_caladan3_mim_lookup_data_t hdl;

    /* obtain the VPN from database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = vpnCtl;

    if(!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                       _bcm_caladan3_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        if (result > 0)/*found*/{
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "DEBUG: %s VPN ID [%d] Found on database !!!!(%s,%d)\n"),
                         FUNCTION_NAME(), vpn , __FILE__, __LINE__));

        } else {
            /* If vpn was found on the database */
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR %s Could not find VPN in VPN database tree (%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }
    } else {
        /* If vpn was found on the database */
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not find VPN - VPN tree empty (%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}
#endif

int bcm_caladan3_mim_vpn_get(int                   unit,
                           bcm_mim_vpn_t         vpn,
                           bcm_mim_vpn_config_t *info)
{
    int    rv = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_SUPPORT
    bcm_caladan3_mim_vpn_control_t *vpncb;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(!info) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_caladan3_mim_vpn_control_get(unit, vpn, &vpncb);

    if (BCM_SUCCESS(rv)) {
        /* fill up the vpn info */
        info->vpn                     = vpncb->vpnid;
        info->lookup_id               = vpncb->isid;
        info->broadcast_group         = vpncb->broadcast_group;
        info->unknown_unicast_group   = vpncb->unknown_unicast_group;
        info->unknown_multicast_group = vpncb->unknown_multicast_group;
    }

    _MIM_UNLOCK(unit);
#endif
    return rv;
}

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
static int
_bcm_caladan3_mim_vpn_bcm_visit(void *userdata,
                              shr_avl_datum_t *datum,
                              void *travdata)
{
    bcm_caladan3_mim_vpn_control_t   *vpnCtl;
    _bcm_caladan3_mim_vpn_cb_data_t *cbData;
    bcm_mim_vpn_config_t         vpnConfig;
    int rv, unit;

    cbData = (_bcm_caladan3_mim_vpn_cb_data_t*)travdata;
    vpnCtl = (bcm_caladan3_mim_vpn_control_t*)datum;

    unit = cbData->unit;

    sal_memset(&vpnConfig, 0, sizeof(bcm_mim_vpn_config_t));
    _bcm_caladan3_mim_vpn_control_xlate(vpnCtl, &vpnConfig);

    rv = cbData->cb(cbData->unit, &vpnConfig, cbData->userData);
    LOG_DEBUG(BSL_LS_BCM_MIM,
              (BSL_META_U(unit,
                          "User callback returned: %d\n"),
               rv));

    return rv;
}
#endif /* BCM_CALADAN3_MIM_SUPPORT */

int
bcm_caladan3_mim_vpn_traverse(int unit,
                            bcm_mim_vpn_traverse_cb cb,
                            void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
    _bcm_caladan3_mim_vpn_cb_data_t  cbData;
    cbData.cb = cb;
    cbData.unit = unit;
    cbData.userData = user_data;

    /* don't allow changes to the vpn tree during traversal */
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = shr_avl_traverse(caladan3_mim_vpn_db[unit],
                          _bcm_caladan3_mim_vpn_bcm_visit,
                          (void*)&cbData);

    _MIM_UNLOCK(unit);
#endif
    return rv;
}


/*
 * Function:
 *   bcm_caladan3_mim_port_add
 * Purpose:
 *     Creates a MiM ports and inserts into the VPN configuration
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     vpn   - (IN)     vpn id
 *     mim_port - (IN)  MiM port to create
 * Returns:
 *   BCM_E_XX
 */
int bcm_caladan3_mim_port_add(int            unit,
                            bcm_mim_vpn_t   vpn,
                            bcm_mim_port_t *mim_port)
{
    int status = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
    int result;
    bcm_caladan3_mim_vpn_control_t dummycb, *vpncb;
    _bcm_caladan3_mim_lookup_data_t hdl;
    uint32 portid = 0;

    status = BCM_E_NONE;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid Unit\n")));
        return BCM_E_PARAM;
    }

    if( (_MIM_INVALID_VPN_ID == vpn) || (!mim_port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid VPN (0x%04x) or mimPort(0x%08x)\n"),
                   vpn, (uint32)mim_port));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    if (!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                       _bcm_caladan3_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        /* If vpn was found on the database */
        if (result > 0) {
            bcm_caladan3_mim_port_control_t *defbboneportcb = NULL, *olddefbboneportcb = NULL;
            bcm_caladan3_mim_port_control_t *portcb = NULL;

            if (mim_port->flags & BCM_MIM_PORT_REPLACE) {

                /* validate input for mim port update */
                status = _bcm_caladan3_mim_port_update_check(unit, mim_port, vpncb, &portcb,
                                                           &defbboneportcb, &olddefbboneportcb);

#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG_DISABLE
                /* update mim port */
                if (BCM_SUCCESS(status)) {
                    status = _bcm_caladan3_mim_update_bbone_port_movement(unit, mim_port, portcb,
                                                                        defbboneportcb, olddefbboneportcb);
                }
#endif
            } else {
                /* If so, verify if the Mim Port exists on the vpn control base */
                status = _bcm_caladan3_mim_port_check(unit, mim_port, vpncb, &defbboneportcb);
                if(BCM_SUCCESS(status)) {

                    portcb = sal_alloc(sizeof(bcm_caladan3_mim_port_control_t),"MiM Port control");

                    if (portcb) {
                        sal_memset(portcb, 0, sizeof(bcm_caladan3_mim_port_control_t));
                        portcb->port        = mim_port->port;
                        portcb->isidvlan    = vpncb->vpnid;
                        portcb->isidlport   = vpncb->lport;
                        portcb->isid        = vpncb->isid;
                        portcb->tpid        = mim_port->egress_service_tpid;
                        portcb->criteria    = mim_port->criteria;
                        portcb->flags       = mim_port->flags;
                        portcb->policer_id  = mim_port->policer_id;

                        /* failover id not used until protection is enabled on PBBN */
                        /*portcb->failover_id = mim_port->failover_id;*/
                        if(mim_port->flags & BCM_MIM_PORT_WITH_ID){
                            portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port->mim_port_id);
                            /* reserve the mim port id */
                            portcb->hwinfo.ftidx = _BCM_CALADAN3_MIM_PORTID_2_FTIDX(unit, portid);
                        }

                        if((mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) &&
                           (BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac))) {

                            portcb->type = _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT;
                            portcb->vlan = mim_port->egress_service_vlan; /* BVLAN */

                            sal_memcpy(&portcb->smac, mim_port->egress_tunnel_srcmac, sizeof(bcm_mac_t));

                            status = _bcm_caladan3_mim_create_back_bone_port(unit,
                                                                           mim_port,
                                                                           portcb,
                                                                           NULL);
                            if(BCM_SUCCESS(status)) {
                                DQ_INSERT_HEAD(&vpncb->def_bbone_plist, &portcb->listnode);

                                /* If Backbone is created over trunk, add the portcb to trunk association list */
                                if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                                    bcm_caladan3_mim_trunk_association_t *trunkAssoc;

                                    trunkAssoc = &caladan3_mim_trunk_assoc_info[unit][BCM_GPORT_TRUNK_GET(portcb->port)];
                                    DQ_INSERT_HEAD(&trunkAssoc->plist, &portcb->trunklistnode);
                                    trunkAssoc->mimType = _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT;

                                    LOG_VERBOSE(BSL_LS_BCM_MIM,
                                                (BSL_META_U(unit,
                                                            "DEBUG: BackBone Port (0x%x) Added to Trunk(%d) Assocation (%s,%d)\n"),
                                                 portcb->gport, BCM_GPORT_TRUNK_GET(portcb->port), __FILE__, __LINE__));
                                }
                            }
                        } else {
                            if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

                                portcb->type = _BCM_CALADAN3_MIM_BACKBONE_PORT;
                                portcb->vlan = mim_port->egress_service_vlan; /* BVLAN */
                                sal_memcpy(&portcb->smac, mim_port->egress_tunnel_srcmac, sizeof(bcm_mac_t));
                                sal_memcpy(&portcb->dmac, mim_port->egress_tunnel_dstmac, sizeof(bcm_mac_t));

                                status = _bcm_caladan3_mim_create_back_bone_port(unit,
                                                                               mim_port,
                                                                               portcb,
                                                                               defbboneportcb);
                                if(BCM_SUCCESS(status)) {
                                    /* Insert the port cb to vpndb sap list */
                                    DQ_INSERT_HEAD(&vpncb->vpn_bbone_sap_head, &portcb->listnode);
                                }

                            } else if (mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS){
                                portcb->type = _BCM_CALADAN3_MIM_ACCESS_PORT;
                                portcb->vlan = mim_port->match_vlan;
                                status = _bcm_caladan3_mim_create_access_port(unit,
                                                                            mim_port,
                                                                            portcb);
                                if(BCM_SUCCESS(status)) {
                                    /* Insert the port cb to vpndb sap list */
                                    DQ_INSERT_HEAD(&vpncb->vpn_access_sap_head, &portcb->listnode);
                                }

                            } else {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "WARNING %s Unknown Port Type !!!!(%s,%d)\n"),
                                           FUNCTION_NAME(), __FILE__, __LINE__));
                                status = BCM_E_PARAM;
                            }


                        }

                        /* if successful, allocate a gport Id */
                        if(BCM_SUCCESS(status)) {

                            if(!(mim_port->flags & BCM_MIM_PORT_WITH_ID)) {
                                portid = _BCM_CALADAN3_FTIDX_2_MIM_PORTID(unit, portcb->hwinfo.ftidx);
                                /* set the mim port id */
                                BCM_GPORT_MIM_PORT_ID_SET(mim_port->mim_port_id, portid);
                            }

                            /* Setup Gport to Lport mapping */
                            SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = (void*)portcb;
                            SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_MIM_PORT;
                            caladan3_gportinfo[unit].lpid[portid] = portcb->hwinfo.lport;

                            /* increase reference count on default back bone port */
                            if((defbboneportcb) && (portcb->type == _BCM_CALADAN3_MIM_BACKBONE_PORT)) {
                                defbboneportcb->refcount++;
                            }

                            portcb->gport = mim_port->mim_port_id;

                        } else {
                            /* if error encountered on port create free up the portcb */
                            sal_free(portcb);
                            portcb = NULL;
                        }

                    } else {
                        status = BCM_E_MEMORY;
                    } /* portcb */

                    if(BCM_SUCCESS(status)) {
                        _bcm_caladan3_dump_portcb(portcb);
                    }
                }  /* port check */
            }
        } else {
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "ERROR: %s VPN ID [%d] Not  Found on database !!!!(%s,%d)\n"),
                         FUNCTION_NAME(),vpn, __FILE__, __LINE__));

            status = BCM_E_NOT_FOUND;
        }
    } else {
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META_U(unit,
                                "ERROR: %s VPN ID [%d] Not  Found - VPN Tree Empty !!!!(%s,%d)\n"),
                     FUNCTION_NAME(),vpn, __FILE__, __LINE__));

        status = BCM_E_NOT_FOUND;
    }

    _MIM_UNLOCK(unit);
#endif
     return status;
}

#ifdef BCM_CALADAN3_MIM_SUPPORT
/*
 *   Function
 *      bcm_caladan3_mim_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided MiM GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = MiM GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_mim_port_get_lpid(int unit,
                             bcm_gport_t gport,
                             uint32 *lpid,
                             bcm_port_t *pport)
{
    uint32                 logicalPort;          /* logical port number */
    uint32                 gportId;
    bcm_caladan3_mim_port_control_t *mimPortData = NULL;
    int                    result;

    if ((!lpid) || (!pport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "NULL not acceptable for out argument\n")));
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_MIM_PORT(gport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "GPORT %08X is not MiM GPORT\n"),
                   gport));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    gportId = BCM_GPORT_MIM_PORT_ID_GET(gport);
    logicalPort = caladan3_gportinfo[unit].lpid[gportId];
    result = BCM_E_NONE;
    mimPortData = (bcm_caladan3_mim_port_control_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
    if (!mimPortData) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "gport %08X is not valid on unit %d\n"),
                   gport,
                   unit));
        result = BCM_E_NOT_FOUND;
    }
    if (BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "unit %d gport %08X disagrees with stored type %02X\n"),
                   unit,
                   gport,
                   SBX_LPORT_TYPE(unit, logicalPort)));
        result = BCM_E_CONFIG;
    }
    if (BCM_E_NONE == result) {
        *lpid = logicalPort;
        *pport = mimPortData->port;
    }

    _MIM_UNLOCK(unit);
    return result;
}
#endif /* BCM_CALADAN3_MIM_SUPPORT */

int bcm_caladan3_mim_port_delete(int            unit,
                                bcm_mim_vpn_t  vpn,
                                bcm_gport_t    mim_port_id)
{
    int status = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_SUPPORT
    int result;
    bcm_caladan3_mim_vpn_control_t dummycb, *vpncb;
    _bcm_caladan3_mim_lookup_data_t hdl;
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    uint32 portid=0;
    uint16 lport;

    status = BCM_E_NONE;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if((_MIM_INVALID_VPN_ID == vpn) ||
       (!BCM_GPORT_IS_MIM_PORT(mim_port_id))) {
      return BCM_E_PARAM;
    }

    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, mim_port_id);

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);
    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;
    } else {
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        /* verify if the vpn exisits on the database */
        dummycb.vpnid = vpn;
        dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
        hdl.key = &dummycb;
        hdl.datum = &vpncb;

        if(!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
            /* verify is the VPN ID prevails on the vpn database */
            result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                           _bcm_caladan3_mim_vpn_lookup_compare,
                                           (shr_avl_datum_t*)&dummycb,
                                           (void*)&hdl);
            /* If vpn was found on the database */
            if(result > 0) {
                status = _bcm_caladan3_mim_port_delete(unit, vpncb, portcb);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR %s Count not Free back MiM Port[0x%x] !!!!(%s,%d)\n"),
                               FUNCTION_NAME(),mim_port_id, __FILE__, __LINE__));
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR: %s VPN ID [%d] Not  Found in database !!!!(%s,%d)\n"),
                           FUNCTION_NAME(),vpn, __FILE__, __LINE__));

                status = BCM_E_NOT_FOUND;
            }


        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s VPN ID [%d] Not  Found in database - VPN Tree empty !!!!(%s,%d)\n"),
                       FUNCTION_NAME(),vpn, __FILE__, __LINE__));

            status = BCM_E_NOT_FOUND;
        }
    }

    _MIM_UNLOCK(unit);
#endif
     return status;
}

int
bcm_caladan3_mim_port_delete_all(int            unit,
                               bcm_mim_vpn_t  vpn)
{
    int lastErr = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_SUPPORT
    int rv;
    bcm_caladan3_mim_vpn_control_t *vpnCtl;
    bcm_caladan3_mim_port_control_t *portCtl = NULL;
    bcm_gport_t gport;
    dq_p_t dqE;

    lastErr = BCM_E_NONE;
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = _bcm_caladan3_mim_vpn_control_get(unit, vpn, &vpnCtl);

    if (BCM_FAILURE(rv)) {
        _MIM_UNLOCK(unit);
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Failed to retrieve VPN Control data 0x%08x info: "
                              "%d %s\n"), vpn, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Grab the configured access ports */
    DQ_TRAVERSE(&vpnCtl->vpn_access_sap_head, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;

        rv = _bcm_caladan3_mim_port_delete(unit, vpnCtl, portCtl);

        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_access_sap_head, dqE);

    /* Grab the configured back bone ports */
    DQ_TRAVERSE(&vpnCtl->vpn_bbone_sap_head, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;

        rv = _bcm_caladan3_mim_port_delete(unit, vpnCtl, portCtl);

        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_bbone_sap_head, dqE);

    /* Delete the default backbone ports */
    DQ_TRAVERSE (&vpnCtl->def_bbone_plist, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;
        rv = bcm_caladan3_mim_port_delete(unit, vpn, portCtl->gport);

        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }

    } DQ_TRAVERSE_END(&vpnCtl->def_bbone_plist, dqE);

    _MIM_UNLOCK(unit);
#endif /* BCM_CALADAN3_MIM_SUPPORT */
    return lastErr;
}

int bcm_caladan3_mim_port_get(int            unit,
                            bcm_mim_vpn_t  vpn,
                            bcm_mim_port_t *mim_port)
{
    int status = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_SUPPORT 
    int result;
    bcm_caladan3_mim_vpn_control_t    dummycb, *vpncb;
    _bcm_caladan3_mim_lookup_data_t hdl;
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    dq_p_t port_elem;

    status = BCM_E_NONE;
    /* GPORT for Back bone ports by Tunnel Encap MAC (B-DMAC) and B-SMAC */
    /* GPORT for Access port obtained by match criteria */
    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
        /* Verify if user is requesting for only Supported PBBN interface types */
        /* Supported Service Interfaces:
         * [1] Port Mode
         * [2] 1:1 STAG Mode
         */
        if(mim_port->flags & ~_BCM_CALADAN3_MIM_ACCESS_SUPPORTED_FLAGS) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s Bad Access Port Flag !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));

            return BCM_E_PARAM;
        }
        if(mim_port->criteria & ~_BCM_CALADAN3_MIM_ACCESS_SUPPORTED_CRITERIA) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s Bad Access Port Criteria !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

        if(mim_port->flags & ~_BCM_CALADAN3_MIM_BB_SUPPORTED_FLAGS) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s Bad Back bone Port Flag !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }

        if(mim_port->criteria & ~_BCM_CALADAN3_MIM_BB_SUPPORTED_CRITERIA) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s Bad Bone port criteria !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }

        if(BCM_MAC_IS_ZERO(mim_port->match_tunnel_srcmac)){
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s Bad Back bone Port SMAC zero !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            return BCM_E_PARAM;
        }
    }

    if((_MIM_INVALID_VPN_ID == vpn) || (!mim_port)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR: %s No vpn or mim_port pointer specified !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
      return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    mim_port->mim_port_id = 0;

    if(!_AVL_EMPTY(caladan3_mim_vpn_db[unit])) {
        result=shr_avl_lookup_lkupdata(caladan3_mim_vpn_db[unit],
                                       _bcm_caladan3_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        /* If vpn was found on the database */
        if(result > 0) {

            if((mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) &&
               (BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac))) {
                /* find the port on default BB List*/
                DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

                    _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

                    if(_BCM_CALADAN3_IS_MAC_EQUAL(mim_port->match_tunnel_srcmac, portcb->smac) &&
                       (portcb->vlan == mim_port->egress_service_vlan)){
                        mim_port->mim_port_id = portcb->gport;
                        break;
                    }
                } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
            } else {

                    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
                    /* access port */
                    DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
                        
                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

                            if(mim_port->criteria == portcb->criteria) {

                                if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT) {

                                    if(mim_port->port == portcb->port) {
                                        mim_port->mim_port_id = portcb->gport;
                                        break;
                                    }
                                } else if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) {

                                    if((mim_port->port == portcb->port) &&
                                       (mim_port->match_vlan == portcb->vlan)) {
                                        mim_port->mim_port_id = portcb->gport;
                                        break;
                                    }
                                } else {

                                    LOG_ERROR(BSL_LS_BCM_MIM,
                                              (BSL_META_U(unit,
                                                          "ERROR %s Bad Match Criteria for Access port !!!!(%s,%d)\n"),
                                               FUNCTION_NAME(), __FILE__, __LINE__));
                                    status = BCM_E_PARAM;
                                }
                            }

                    } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);

                    } else {
                        /* back bone port */
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {

                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);
                            if(_BCM_CALADAN3_IS_MAC_EQUAL(mim_port->egress_tunnel_dstmac, portcb->dmac) &&
                               (portcb->vlan == mim_port->egress_service_vlan)){

                                mim_port->mim_port_id = portcb->gport;
                                break;
                            }
                    } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);
                        }
                    }
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: %s VPN ID [%d] Not  Found on database !!!!(%s,%d)\n"),
                       FUNCTION_NAME(),vpn, __FILE__, __LINE__));

            status = BCM_E_NOT_FOUND;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR: %s VPN ID [%d] Not  Found - VPN Tree empty !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),vpn, __FILE__, __LINE__));

        status = BCM_E_NOT_FOUND;
    }

    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR: %s MiM Port Not  Found on database !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        status = BCM_E_NOT_FOUND;
    } else if(mim_port->mim_port_id == 0){
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR: %s MiM Port Not  Found on database !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        status = BCM_E_NOT_FOUND;
    }

    _MIM_UNLOCK(unit);
#endif /* BCM_CALADAN3_MIM_SUPPORT */
     return status;
}

int
bcm_caladan3_mim_port_get_all(int            unit,
                            bcm_mim_vpn_t  vpn,
                            int            port_max,
                            bcm_mim_port_t *mim_array,
                            int            *port_count)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_CALADAN3_MIM_WARM_BOOT_DEBUG
    int portCount = 0;
    bcm_caladan3_mim_vpn_control_t *vpnCtl;
    bcm_caladan3_mim_port_control_t *portCtl = NULL;
    dq_p_t dqE;

    if (mim_array == NULL || port_count == NULL || port_max <= 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = _bcm_caladan3_mim_vpn_control_get(unit, vpn, &vpnCtl);

    if (BCM_FAILURE(rv)) {
        _MIM_UNLOCK(unit);
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Failed to retrieve VPN Control data 0x%08x info: "
                              "%d %s\n"),
                   vpn, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Grab any default backbone ports on the VPN */
    DQ_TRAVERSE (&vpnCtl->def_bbone_plist, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_caladan3_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Insufficient memory to return port info, "
                                 "skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->def_bbone_plist, dqE);

    /* Grab the configured access ports */
    DQ_TRAVERSE(&vpnCtl->vpn_access_sap_head, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_caladan3_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Insufficient memory to return port info, "
                                 "skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_access_sap_head, dqE);


    /* Grab the configured back bone ports */
    DQ_TRAVERSE(&vpnCtl->vpn_bbone_sap_head, dqE) {

        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_caladan3_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            LOG_WARN(BSL_LS_BCM_MIM,
                     (BSL_META_U(unit,
                                 "Insufficient memory to return port info, "
                                 "skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_bbone_sap_head, dqE);

    _MIM_UNLOCK(unit);
    *port_count = portCount;
#endif /* BCM_CALADAN3_MIM_SUPPORT */
    return rv;
}
#ifndef BCM_CALADAN3_MIM_SUPPORT

STATIC int
_bcm_caladan3_mim_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data)
{
    int status = BCM_E_NONE;
    soc_sbx_g3p1_p2e_t  p2e;
    soc_sbx_g3p1_ep2e_t ep2e;
    soc_sbx_g3p1_pv2e_t pv2e;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    int index=0, mymodid, pindex=0, idx=0, port=0;
    bcm_caladan3_mim_trunk_association_t *trunkAssoc;
 
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Unknown unit %d !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),unit, __FILE__, __LINE__));
        return BCM_E_PARAM;
    }

    if(tid >= SBX_MAX_TRUNKS) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Trunk ID  %d !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),tid, __FILE__, __LINE__));
        return BCM_E_PARAM;
    }

    if(!tdata) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Input Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_PARAM;
    }
    
    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    if(tdata->num_ports == 0) {
        /* nothing to do */
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
        tp[index] = -1;
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

    trunkAssoc = &caladan3_mim_trunk_assoc_info[unit][tid];

    switch(trunkAssoc->mimType) {

        case _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT:
            for(index=0; index < pindex; index++) {

                port = tp[index];

                /* configure back bone ports mim mode */
                if((port >= 0) && 
                   ((soc_sbx_g3p1_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
                   ((soc_sbx_g3p1_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

                    p2e.mim         = MIM_BBONE_PORT;
                    p2e.customer    = 0;
                    p2e.provider    = 1;
                    p2e.defstrip    = 0;
                    p2e.pstrip      = 0;
                    p2e.pbb         = 1;

                    ep2e.customer   = 1;
                    ep2e.mim        = MIM_BBONE_PORT;
                    ep2e.pbb        = 1;                

                    /* program P2E */
                    status = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

                    if(BCM_SUCCESS(status)) {
                        /* program EP2E */
                        status = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);

                        if(BCM_FAILURE(status)) {

                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Programming EP2E %d : %s in (%s,%d)\n"),
                                       FUNCTION_NAME(), status, bcm_errmsg(status),
                                       __FILE__,__LINE__));
                        } else {
                            dq_p_t port_elem;
                            bcm_caladan3_mim_port_control_t *portcb = NULL;

                            /* For each back bone port on port list, reset lp on bvlan pv2e */
                            DQ_TRAVERSE(&trunkAssoc->plist, port_elem) {

                                port_elem -= _MIM_PCB_TRUNK_NODE_POS;
                                _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

                                soc_sbx_g3p1_pv2e_t_init(&pv2e);

                                status = soc_sbx_g3p1_pv2e_get(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                if(BCM_FAILURE(status)) {

                                    LOG_ERROR(BSL_LS_BCM_MIM,
                                              (BSL_META_U(unit,
                                                          "ERROR %s Getting PV2E %d : %s in (%s,%d)\n"),
                                               FUNCTION_NAME(), status, bcm_errmsg(status),
                                               __FILE__,__LINE__));
                                } else {
                                    /* Program Bvlan, port 2 Etc -> LPI */
                                    /* lpi on pv2e for back bone must be 0 to use physical port as lp */
                                    pv2e.lpi = 0;

                                    status =  soc_sbx_g3p1_pv2e_set(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                    if(BCM_FAILURE(status)) {

                                        LOG_ERROR(BSL_LS_BCM_MIM,
                                                  (BSL_META_U(unit,
                                                              "ERROR %s Programming PV2E %d : %s in (%s,%d)\n"),
                                                   FUNCTION_NAME(), status, bcm_errmsg(status),
                                                   __FILE__,__LINE__));
                                    }
                                }
                            } DQ_TRAVERSE_END(&trunkAssoc->plist, port_elem);
                        }
                    }

                } else {
                    status = BCM_E_INTERNAL;
                }
            }
            break;

        default:
            status = BCM_E_INTERNAL;
            break;
    }

    _MIM_UNLOCK(unit);

    return status;
}
#endif /*ifndef BCM_CALADAN3_MIM_SUPPORT */

#ifdef BCM_CALADAN3_MIM_SUPPORT
/*
 * Function:
 *   _bcm_caladan3_multicast_mim_encap_get
 * Purpose:
 *     Returns Encap ID for Access or Default Back bone
 *     MiM port. Non-Default Back bone ports are invalid parameter
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     group - (IN)     Multicast group
 *     port  - (IN)     physical port
 *     mim_port_id - (IN)  MiM port
 *     encap_id    - (IN/OUT) Encap ID
 * Returns:
 *   BCM_E_XX
 */
int _bcm_caladan3_multicast_mim_encap_get(int unit,
                                        bcm_multicast_t group,
                                        bcm_gport_t port,
                                        bcm_gport_t mim_port_id,
                                        bcm_if_t *encap_id)
{
    int status = BCM_E_NONE;

    if(!encap_id) {

        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s No Encap ID pointer specified !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;

    } else {
        uint16 lport;
        bcm_caladan3_mim_port_control_t *portcb;
        uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);


        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G3P1_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G3P1_GPORT_RANGE_CHECK(unit, mim_port_id);

        BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

        /* obtain logical port from mim gport */
        lport = caladan3_gportinfo[unit].lpid[portid];

        if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));

            status = BCM_E_PARAM;

        } else {
            portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
            *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(portcb->hwinfo.ohi);
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    " %s ENCAP-ID[0x%x] \n"),
                         FUNCTION_NAME(), *encap_id));
        }

        _MIM_UNLOCK(unit);
    }

    return status;
}

/*
 * Function:
 *   _bcm_caladan3_mim_fte_gport_get
 * Purpose:
 *     Given MIM Gport, returns FTIDX for the port
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     ftidx    - (IN/OUT) Forwarding index
 * Returns:
 *   BCM_E_XX
 */
int _bcm_caladan3_mim_fte_gport_get(int unit,
                                  bcm_gport_t mim_port_id,
                                  uint32 *ftidx)
{
    int status = BCM_E_NONE;
    uint32 portid = 0;

    if(!ftidx) {

        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s No FTIDX pointer specified !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;

    } else {
        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G3P1_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G3P1_GPORT_RANGE_CHECK(unit, mim_port_id);
        portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);
        *ftidx = _BCM_CALADAN3_MIM_PORTID_2_FTIDX(unit, portid);
 
        LOG_VERBOSE(BSL_LS_BCM_MIM,
                    (BSL_META_U(unit,
                                " %s FTE[0x%x] \n"),
                     FUNCTION_NAME(), *ftidx));
    }
    return status;
}

bcm_gport_t _bcm_caladan3_mim_fte_to_gport_id(int unit,
                                    uint32 ftidx)
{
    bcm_gport_t gportid = 0;
    uint32 portid = _BCM_CALADAN3_FTIDX_2_MIM_PORTID(unit, ftidx);

    BCM_GPORT_MIM_PORT_ID_SET(gportid, portid);
    return gportid;
}

/* System Test Support Function */
int soc_sbx_g3p1_mim_lsm_get(int unit,
                            int mim_port_id,
                            int  *lsm)
{
    int status = BCM_E_NONE;

    if(!lsm) {

        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s No LSM pointer specified !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;

    } else {
        uint16 lport;
        bcm_caladan3_mim_port_control_t *portcb;
        uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);


        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G3P1_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G3P1_GPORT_RANGE_CHECK(unit, mim_port_id);

        BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

        /* obtain logical port from mim gport */
        lport = caladan3_gportinfo[unit].lpid[portid];

        if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));

            status = BCM_E_PARAM;

        } else {
            portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
            *lsm = portcb->hwinfo.ismacidx;
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    " %s LSM-ID[0x%x] \n"),
                         FUNCTION_NAME(), *lsm));
        }

        _MIM_UNLOCK(unit);
    }

    return status;
}
#endif

int
_bcm_caladan3_mim_set_egress_remarking(int unit, 
                                     uint32 eteidx, 
                                     uint32 egrMap,
                                     uint32 egrFlags)
{
    int status = BCM_E_NONE;
    soc_sbx_g3p1_ete_t      ete;
    
    /* If explicit egress qos marking was not set on
     * this bdmac gport, dont disturb the qos remarking */  
    status = soc_sbx_g3p1_ete_get(unit,
                                  eteidx,
                                  &ete);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error[%d] %s Setting ETEEncap Remark !!!!(%s,%d)\n"),
                   status, FUNCTION_NAME(), __FILE__, __LINE__));
    } else {
        ete.remark = egrMap;
        ete.dscpremark = (egrFlags & BCM_QOS_MAP_L3)?1:0;
        status = soc_sbx_g3p1_ete_set(unit,
                                      eteidx,
                                      &ete);   
        if(BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error[%d] %s Setting ETEEncap Remark !!!!(%s,%d)\n"),
                       status, FUNCTION_NAME(), __FILE__, __LINE__));
        } else {
            LOG_VERBOSE(BSL_LS_BCM_MIM,
                        (BSL_META_U(unit,
                                    "Updated ete=0x%x remark=0x%x\n"),
                         eteidx, egrMap));
        }
    }
    return status;
}

#ifdef BCM_CALADAN3_MIM_SUPPORT
int 
_bcm_caladan3_mim_bbone_egress_qosmap_set(int unit, 
                                        bcm_caladan3_mim_port_control_t *bbportcb,
                                        int egrMap, uint32 egrFlags)
{
    bcm_caladan3_mim_vpn_control_t *vpncb;
    bcm_caladan3_mim_port_control_t *tmpportcb;
    dq_p_t port_elem;
    int status = BCM_E_NONE;
    uint32 oldremarkidx=0;

    if(egrMap >= 0) {

        if(!bbportcb ||
           (bbportcb->type != _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT &&
            bbportcb->type != _BCM_CALADAN3_MIM_BACKBONE_PORT)) {
            status = BCM_E_PARAM;
        } else {

            status  = _bcm_caladan3_mim_vpn_control_get(unit, bbportcb->isidvlan, &vpncb);
            if (BCM_FAILURE(status)) {
                
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR: Could not find VPN control for this VPN: unit %d  vpn  0x%x\n"),
                           unit, bbportcb->isidvlan));
                
                status = BCM_E_INTERNAL;

            } else {

                if (bbportcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {

                    /* new qos profile is trying to be applied */
                    if (bbportcb->hwinfo.egrremarkidx > 0) {
                        oldremarkidx = bbportcb->hwinfo.egrremarkidx;  
                    }

                    /* Check if there is a Mim Port using this port */
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {
                        
                        _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, tmpportcb);

                        if(_BCM_CALADAN3_IS_MAC_EQUAL(bbportcb->smac, tmpportcb->smac)) {

                            if((oldremarkidx == 0 && tmpportcb->hwinfo.egrremarkidx == 0) ||
                               (oldremarkidx && oldremarkidx == tmpportcb->hwinfo.egrremarkidx)) {
                                /* set remarking index */
                                status = _bcm_caladan3_mim_set_egress_remarking(unit, 
                                                                              tmpportcb->hwinfo.ete,
                                                                              egrMap,
                                                                              egrFlags);
                                if(BCM_FAILURE(status)) {
                                    /* Failed to pass on remarking properties to childs */
                                    break;
                                } else {
                                    tmpportcb->hwinfo.egrremarkidx = egrMap;
                                    tmpportcb->hwinfo.egrremarkflags = egrFlags;
                                }
                            }
                        }
                        
                    } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);

                    if(BCM_SUCCESS(status)) {
                        /* set remarking index */
                        status = _bcm_caladan3_mim_set_egress_remarking(unit, 
                                                                      bbportcb->hwinfo.ete,
                                                                      egrMap,
                                                                      egrFlags);
                        if(BCM_SUCCESS(status)) {
                            bbportcb->hwinfo.egrremarkidx = egrMap;
                            bbportcb->hwinfo.egrremarkflags = egrFlags;
                        }
                    }
                } else {

                    /* set Remarking for Back bone gports */
                    /* If egress map is 0, try to inherit remark property from default backbone port */
                    if(egrMap == 0) {
                        bcm_caladan3_mim_port_control_t *defbbportcb = NULL;
                        DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {
                            _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

                            if(_BCM_CALADAN3_IS_MAC_EQUAL(defbbportcb->smac, bbportcb->smac)) {
                                if(defbbportcb->hwinfo.egrremarkidx > 0) {
                                    status = _bcm_caladan3_mim_set_egress_remarking(unit, 
                                                                        bbportcb->hwinfo.ete,
                                                                        defbbportcb->hwinfo.egrremarkidx,
                                                                        defbbportcb->hwinfo.egrremarkflags);
                                    if(BCM_FAILURE(status)) {
                                        break;
                                    } else {
                                        /* inherit from default backbone port */
                                        bbportcb->hwinfo.egrremarkidx = defbbportcb->hwinfo.egrremarkidx;
                                        bbportcb->hwinfo.egrremarkflags = defbbportcb->hwinfo.egrremarkflags;
                                        break;
                                    }
                                }
                            }
                        } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
                    } else {
                        /* if non-zero, use it */
                       status = _bcm_caladan3_mim_set_egress_remarking(unit, 
                                                                      bbportcb->hwinfo.ete,
                                                                      egrMap,
                                                                      egrFlags);
                        if(BCM_SUCCESS(status)) {
                            bbportcb->hwinfo.egrremarkidx = egrMap;
                            bbportcb->hwinfo.egrremarkflags = egrFlags;
                        }
                    }
                }        
            }
        }     
    }
    return status;
}

/*
 * Function:
 *   _bcm_caladan3_mim_qosmap_set
 * Purpose:
 *     Given MIM Gport, set qos mapping profile to it
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_caladan3_mim_qosmap_set(int unit, bcm_gport_t port, 
                          int ing_idx, int egr_idx,
                          uint32 ingFlags, uint32 egrFlags)
{
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int status = BCM_E_NONE;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        
        status = BCM_E_PARAM;
        
    } else {
        /* works only for access & default back bone ports !!! */
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if(ing_idx >= 0) {
            soc_sbx_g3p1_lp_t lp;
            int lpi = -1;

        if (portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "failed to read isid2e[0x%06x]:%d %s\n")) 
                           portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unexpected physical port for logicial "
                                      "port on isid 0x%06x\n"),
                           portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Works only for Access and Default Backbone MiM "
                                   "ports !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            
            status = BCM_E_PARAM;
        }
        
        /* Valid port type an have necessary information */
        if (BCM_SUCCESS(status)) {
            if(ing_idx >= 0) {
                status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);        
                
                if(BCM_SUCCESS(status)) {
                    lp.qos = ing_idx;
                    lp.usedscp = (ingFlags & BCM_QOS_MAP_L3) ? 1 : 0;
                    lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS) ? 1 : 0;
                    status = soc_sbx_g3p1_lp_set(unit, lpi, &lp); 
                    if(BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "Error[%d] %s Setting Logical Port !!!!(%s,%d)\n"),
                                   status, FUNCTION_NAME(), __FILE__, __LINE__));
                    } else {
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "Updated lpi=%d qos=%d\n"),
                                     lpi, lp.qos));
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "Error[%d] %s Reading Logical Port !!!!(%s,%d)\n"),
                               status, FUNCTION_NAME(), __FILE__, __LINE__));
                }
            }
            }     
        }

            if (BCM_SUCCESS(status) && (egr_idx >= 0)) {
                
            if (portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT ||
                portcb->type == _BCM_CALADAN3_MIM_BACKBONE_PORT) {
                status = _bcm_caladan3_mim_bbone_egress_qosmap_set(unit, portcb, egr_idx, egrFlags);
                    } else {
                status = _bcm_caladan3_mim_set_egress_remarking(unit, 
                                                              portcb->hwinfo.ete,
                                                              egr_idx,
                                                              egrFlags);
                if(BCM_SUCCESS(status)) {
                    portcb->hwinfo.egrremarkidx = egr_idx;
                    portcb->hwinfo.egrremarkflags = egrFlags;
                        LOG_VERBOSE(BSL_LS_BCM_MIM,
                                    (BSL_META_U(unit,
                                                "Updated ete=0x%x remark=0x%x\n"),
                                     portcb->hwinfo.ete, egr_idx));
                } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "Error[%d] %s Setting egress remarking !!!!(%s,%d)\n"),
                               status, FUNCTION_NAME(), __FILE__, __LINE__));
                }
            }
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *   _bcm_caladan3_g3p1_mim_qosmap_get
 * Purpose:
 *   Get a mim gports qos mappings, for g3p1;
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 *
 * notes:
 *   no param checking
 *   lock must be taken by caller
 */
static int
_bcm_caladan3_g3p1_mim_qosmap_get(int unit, bcm_gport_t port, 
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags)
{
    int                          rv;
    bcm_caladan3_mim_port_control_t *port_data;
    soc_sbx_g3p1_lp_t            lp;
    soc_sbx_g3p1_ete_t           ete;
    uint32                       lport, lpi;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);

    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if (SBX_LPORT_TYPE(unit, lport) != BCM_GPORT_MIM_PORT) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid GPORT type found at 0x%x\n"),
                   lport));
        return BCM_E_PARAM;
    } 

    /* works only for access & default back bone ports */
    port_data = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
    
    if (port_data->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {
        soc_sbx_g2p3_isid2e_t isid2e;

        rv = soc_sbx_g2p3_isid2e_get(unit, port_data->isid, &isid2e);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "failed to read isid2e[0x%06x]:%d %s\n"), 
                       port_data->isid, rv, bcm_errmsg(rv)));
            return rv;
        }
        
        if (isid2e.lpi == 0) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "unexpected physical port for logicial port "
                                  "on isid 0x%06x\n"),
                       port_data->isid));
            return BCM_E_INTERNAL;
        }
        
        lpi = isid2e.lpi;

    } else if (port_data->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
        lpi = port_data->hwinfo.lport;

    } else {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Invalid port type found: expected Access or"
                              " Default backbone\n")));
        return BCM_E_PARAM;
    }
        
    rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Failed to read lp[0x%x]: %d %s\n"),
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

    /* Get Egress mapping 
     */
    if (port_data->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT ||
        port_data->type == _BCM_CALADAN3_MIM_BACKBONE_PORT) {

        rv = soc_sbx_g3p1_ete_get(unit, port_data->hwinfo.ete, &ete);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Failed to read ete[0x%x]: %d %s\n"),
                       port_data->hwinfo.ete, rv, bcm_errmsg(rv)));
            return rv;
        }
        *egr_idx   = ete.remark;
        *egr_flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
        if (ete.dscpremark) {
            *egr_flags |= BCM_QOS_MAP_L3;
        }
    } else {
        rv = BCM_E_PARAM;
    }

    return rv;

}

/*
 * Function:
 *   _bcm_caladan3_mim_qosmap_get
 * Purpose:
 *   Get a mim gports qos mappings;
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_caladan3_mim_qosmap_get(int unit, bcm_gport_t port, 
                           int *ing_idx, int *egr_idx,
                           uint32 *ing_flags, uint32 *egr_flags)
{
    int rv = BCM_E_UNAVAIL;
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    rv = _bcm_caladan3_g3p1_mim_qosmap_get(unit, port, ing_idx, egr_idx,
                                         ing_flags, egr_flags);
    
    _MIM_UNLOCK(unit);
    return rv;

}

/*
 * Function:
 *   _bcm_caladan3_mim_stp_update
 * Purpose:
 *     Given MIM Gport, set stp state on it
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     stp state
 * Returns:
 *   BCM_E_XX
 */
int _bcm_caladan3_mim_stp_update(int unit, 
                               bcm_gport_t port, 
                               int *stp_state,
                               int set)
{
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int status = BCM_E_NONE;
    soc_sbx_g3p1_pv2e_t   pv2e;
    soc_sbx_g3p1_epv2e_t  epv2e;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        
        status = BCM_E_PARAM;
        
    } else {
        bcm_vlan_t vid=0;
        uint32 phy_port=0;

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
        phy_port = BCM_GPORT_MODPORT_PORT_GET(portcb->port);

        if (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
            
            if(portcb->service == _BCM_CALADAN3_MIM_PORT) {
                /* set STP on port + All VID */
                vid = 0;
            } else if (portcb->service == _BCM_CALADAN3_MIM_STAG_1_1) {
                /* set STP on port + stag */
                vid = portcb->vlan;
            } else {
                status = BCM_E_INTERNAL;
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "Error %s Unknown Access Port Service Type !!!!(%s,%d)\n"),
                           FUNCTION_NAME(), __FILE__, __LINE__));
            }
            if(BCM_SUCCESS(status)) {

                status = soc_sbx_g3p1_pv2e_get(unit, vid, phy_port, &pv2e);
                if(BCM_SUCCESS(status)) {

                    status = soc_sbx_g3p1_epv2e_get(unit, vid, phy_port, &epv2e);
                    if(BCM_SUCCESS(status)) {

                        if(set) {
                            pv2e.stpstate = _bcm_caladan3_g3p1_stg_stp_translate(unit, *stp_state);
                            epv2e.drop = ((*stp_state == BCM_STG_STP_FORWARD ||
                                                  *stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);

                            status = soc_sbx_g3p1_pv2e_set(unit, vid, phy_port, &pv2e);
                            if(BCM_SUCCESS(status)) {
                                /* for port mode set stp on all vid's since packet
                                 * can exit with CTAG on it and STP has to be based on CTAG */
                                if(portcb->service == _BCM_CALADAN3_MIM_PORT) {
                                    int *fastSets = NULL;
                                    uint32  *fastDrops = NULL;

                                    fastSets = sal_alloc(BCM_VLAN_COUNT * sizeof(int),
                                                          "fastSets temp");
                                    if (fastSets == NULL) {
                                        status = BCM_E_MEMORY;
                                    }
                                    
                                    if(fastSets) {
                                    
                                    fastDrops = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32),
                                                          "fastDrops temp");
                                    if (fastDrops == NULL) {
                                        status = BCM_E_MEMORY;
                                            sal_free(fastSets);
                                        }
                                    }

                                    if(fastSets && fastDrops && BCM_SUCCESS(status)) {
                                        bcm_vlan_t vididx;
                                        for (vididx = BCM_VLAN_MIN;  vididx < BCM_VLAN_COUNT; vididx++) {
                                            fastSets[vididx] = 1;
                                            fastDrops[vididx] = epv2e.drop;
                                        }

                                        status = soc_sbx_g3p1_epv2e_drop_fast_set(unit, BCM_VLAN_MIN, phy_port,
                                                                              BCM_VLAN_MAX, phy_port,
                                                                              &fastSets[BCM_VLAN_MIN],
                                                                              &fastDrops[BCM_VLAN_MIN],
                                                                              BCM_VLAN_COUNT);
                                        if(BCM_FAILURE(status)) {
                                            status = BCM_E_INTERNAL;
                                            LOG_ERROR(BSL_LS_BCM_MIM,
                                                      (BSL_META_U(unit,
                                                                  "Error %s Could not set EPV2E !!!!(%s,%d)\n"),
                                                       FUNCTION_NAME(), __FILE__, __LINE__));
                                        }

                                        sal_free(fastDrops);
                                        sal_free(fastSets);
                                    } else {
                                        status = BCM_E_INTERNAL;
                                        LOG_ERROR(BSL_LS_BCM_MIM,
                                                  (BSL_META_U(unit,
                                                              "Error %s Could not set EPV2E !!!!(%s,%d)\n"),
                                                   FUNCTION_NAME(), __FILE__, __LINE__));
                                    }
                                } else {
                                    status = soc_sbx_g3p1_epv2e_set(unit, vid, phy_port, &epv2e);
                                    if(BCM_FAILURE(status)) {
                                        status = BCM_E_INTERNAL;
                                        LOG_ERROR(BSL_LS_BCM_MIM,
                                                  (BSL_META_U(unit,
                                                              "Error %s Could not set EPV2E !!!!(%s,%d)\n"),
                                                   FUNCTION_NAME(), __FILE__, __LINE__));
                                    }
                                }
                            } else {
                                status = BCM_E_INTERNAL;
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "Error %s Could not set PV2E !!!!(%s,%d)\n"),
                                           FUNCTION_NAME(), __FILE__, __LINE__));
                            }
                        } else {
                            *stp_state = _bcm_caladan3_g3p1_stg_stp_translate_to_bcm(unit, pv2e.stpstate);
                        }
                    } else {
                        status = BCM_E_INTERNAL;
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "Error %s Could not read EPV2E !!!!(%s,%d)\n"),
                                   FUNCTION_NAME(), __FILE__, __LINE__));
                    }
                } else {
                    status = BCM_E_INTERNAL;
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "Error %s Could not read PV2E !!!!(%s,%d)\n"),
                               FUNCTION_NAME(), __FILE__, __LINE__));
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Works only for Access MiM "
                                   "ports !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            
            status = BCM_E_PARAM;
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *   _bcm_caladan3_mim_vpn_policer_set
 * Purpose:
 *    Set Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     verify - if set, verify if policer is set on all logical port
 *              of vpn required to accumulate statistics
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_caladan3_mim_vpn_policer_set(int unit, 
                               bcm_mim_vpn_t vpn,
                               bcm_policer_t pol_id,
                               uint8       verify)
{
    int    rv;
    bcm_caladan3_mim_vpn_control_t *vpncb;
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    dq_p_t                       port_elem;
    soc_sbx_g3p1_lp_t lp;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_caladan3_mim_vpn_control_get(unit, vpn, &vpncb);
    if (BCM_SUCCESS(rv)) {
        /* Set policer on Access MiM Gport's LP */
        /* Check if there is a Mim Port using this port */
        DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
            
            _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

            if(!portcb->hwinfo.lport) {
                /* access ports lport must be non zero */
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR: BAD Access LPORT: unit %d gport %08X LpIndex %d\n"),
                           unit, portcb->gport, portcb->hwinfo.lport));
            } else {
                rv = soc_sbx_g3p1_lp_get(unit, portcb->hwinfo.lport, &lp);
                if(BCM_SUCCESS(rv)) {
                    /* only verify if Logical ports has associate policer */
                    if(verify) {
                        if(lp.policer) {
                            LOG_DEBUG(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "Policer %d exists on gport %08x\n"),
                                       lp.policer, portcb->gport));
                        } else {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "No Policer exists on gport %08x\n"),
                                       portcb->gport));
                            rv = BCM_E_INTERNAL;
                        }
                    } else {
                        /* set new policer id */
                        rv = _bcm_caladan3_g3p1_policer_lp_program(unit, pol_id, &lp);
                        if(BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n"),
                                       unit, portcb->gport, portcb->hwinfo.lport));
                        } else {
                            
                            rv = soc_sbx_g3p1_lp_set(unit, portcb->hwinfo.lport, &lp);
                            if(BCM_FAILURE(rv)) {
                                LOG_ERROR(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n"),
                                           unit, portcb->gport, portcb->hwinfo.lport));
                            } else {
                                LOG_DEBUG(BSL_LS_BCM_MIM,
                                          (BSL_META_U(unit,
                                                      "Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n"),
                                           unit, portcb->gport, portcb->hwinfo.lport, pol_id));                            
                            }
                        }
                    }
                 } else {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR: Getting LPORT: unit %d gport %08X LpIndex %d\n"),
                               unit, portcb->gport, portcb->hwinfo.lport));
                }
            }

            if(BCM_FAILURE(rv)) {
                break;
            }

        } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);
        
        if(BCM_SUCCESS(rv)) {
            /* Set policer on ISID2E LP - Backbone does not have dedicated LP */
            rv = soc_sbx_g3p1_lp_get(unit, vpncb->lport, &lp);
            if(BCM_SUCCESS(rv)) {
                /* only verify if Logical ports has associate policer */
                if(verify) {
                    if(lp.policer) {
                        LOG_DEBUG(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "Policer %d exists on ISID2E LP \n"),
                                   lp.policer));
                    } else {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "No Policer exists on ISID2E LP \n")));
                        rv = BCM_E_INTERNAL;
                    }
                } else {
                    /* set new policer id */
                    rv = _bcm_caladan3_g3p1_policer_lp_program(unit, pol_id, &lp);
                    if(BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_MIM,
                                  (BSL_META_U(unit,
                                              "ERROR: Setting Back Bone LPORT: unit %d  LpIndex %d\n"),
                                   unit, vpncb->lport));
                    } else {
                        rv = soc_sbx_g3p1_lp_set(unit, vpncb->lport, &lp);
                        if(BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n"),
                                       unit, portcb->gport, vpncb->lport));
                        } else {
                            LOG_DEBUG(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n"),
                                       unit, portcb->gport, vpncb->lport, pol_id));                            
                        }
                    }
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR: Getting Back Bone LPORT: unit %d LpIndex %d\n"),
                           unit, vpncb->lport));
            }        
        }
    }

    if(BCM_SUCCESS(rv) && (!verify)) {
        vpncb->policer_id = pol_id;
    }

    _MIM_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *   _bcm_caladan3_mim_vpn_policer_get
 * Purpose:
 *    Get Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_caladan3_mim_vpn_policer_get(int unit, 
                               bcm_mim_vpn_t vpn,
                               bcm_policer_t *pol_id)
{
    int    rv;
    bcm_caladan3_mim_vpn_control_t *vpncb;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit) || (!pol_id)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_caladan3_mim_vpn_control_get(unit, vpn, &vpncb);
    if (BCM_SUCCESS(rv)) {
        /* Set policer on Access MiM Gport's LP */
        if(vpncb->policer_id < 0) {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "ERROR: No Policer Set on this VPN: unit %d  vpn  0x%x\n"),
                       unit, vpn));
            rv = BCM_E_PARAM;
        } else {
            *pol_id = vpncb->policer_id;
        }
    }

    _MIM_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *   _bcm_caladan3_mim_policer_set
 * Purpose:
 *    Set Policer ID for a MiM Gport
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_caladan3_mim_policer_set(int unit, 
                            bcm_gport_t port,
                            bcm_policer_t pol_id)
{
    int status = BCM_E_NONE;
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int lpi = -1;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;
        
    } else {

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "failed to read isid2e[0x%06x]:%d %s\n"), 
                           portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unexpected physical port for logicial "
                                      "port on isid 0x%06x\n"),
                           portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Works only for Access and Default Backbone MiM "
                                   "ports !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            
            status = BCM_E_PARAM;
        }
    }
        
    if (BCM_SUCCESS(status)) {
        soc_sbx_g3p1_lp_t lp;
        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
        if(BCM_SUCCESS(status)) {
            /* set new policer id */
            status = _bcm_caladan3_g3p1_policer_lp_program(unit, pol_id, &lp);
            if(BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "ERROR: Setting Policer to LPORT: unit %d gport %08X LpIndex %d\n"),
                           unit, portcb->gport, lpi));
            } else {
                status = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
                if(BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n"),
                               unit, portcb->gport, lpi));
                } else {
                    LOG_DEBUG(BSL_LS_BCM_MIM,
                              (BSL_META_U(unit,
                                          "Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n"),
                               unit, portcb->gport, lpi, pol_id));                            
                }                           
            }
        }  else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error[%d] %s Reading Logical Port !!!!(%s,%d)\n"),
                       status, FUNCTION_NAME(), __FILE__, __LINE__));
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *   _bcm_caladan3_mim_policer_get
 * Purpose:
 *    Get Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_caladan3_mim_policer_get(int unit, 
                                bcm_gport_t port,
                               bcm_policer_t *pol_id)
{
    int                          status = BCM_E_NONE;
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb = NULL;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int lpi = -1;

    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit) || (!pol_id)){
        return BCM_E_PARAM;
    }

    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, port);

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;

    } else {

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "failed to read isid2e[0x%06x]:%d %s\n"), 
                           portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                LOG_ERROR(BSL_LS_BCM_MIM,
                          (BSL_META_U(unit,
                                      "unexpected physical port for logicial "
                                      "port on isid 0x%06x\n"),
                           portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s Works only for Access and Default Backbone MiM "
                                   "ports !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            
            status = BCM_E_PARAM;
             }
         }

    if (BCM_SUCCESS(status)) {
        soc_sbx_g3p1_lp_t lp;

        status = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
        if(BCM_SUCCESS(status)) {
            /* set new policer id */
            *pol_id = lp.policer;
        }  else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error[%d] %s Reading Logical Port !!!!(%s,%d)\n"),
                       status, FUNCTION_NAME(), __FILE__, __LINE__));
         }
    }

    _MIM_UNLOCK(unit);
    return status;
}


/* Support Bundle Mode */

int
_bcm_caladan3_mim_port_vlan_vector_internal(int unit,
                                           bcm_gport_t gport,
                                           bcm_port_t *phy_port,
                                           bcm_vlan_t *vsi,
                                           uint32   *logicalPort)
{
    uint16 lport;
    bcm_caladan3_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(gport);
    int status = BCM_E_NONE;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G3P1_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit) || (!phy_port) || (!vsi) || (!logicalPort)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G3P1_GPORT_RANGE_CHECK(unit, gport);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = caladan3_gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "Error %s LP GPORT not a MiM port !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));

        status = BCM_E_PARAM;
        
    } else {
        /* only for access ports !!! */
        portcb = (bcm_caladan3_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_CALADAN3_MIM_ACCESS_PORT) {
           *phy_port    = SOC_GPORT_MODPORT_PORT_GET(portcb->port);
           *vsi         = portcb->isidvlan;
           *logicalPort = portcb->hwinfo.lport;
        } else {
            LOG_ERROR(BSL_LS_BCM_MIM,
                      (BSL_META_U(unit,
                                  "Error %s only for Access MiM ports !!!!(%s,%d)\n"),
                       FUNCTION_NAME(), __FILE__, __LINE__));
            
            status = BCM_E_PARAM;
        }
        
    }
    
    _MIM_UNLOCK(unit);
    return status;
}

int
_bcm_caladan3_mim_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;

    soc_sbx_g3p1_pv2e_t          pv2e;
    uint32                       logicalPort = ~0;

    BCM_IF_ERROR_RETURN
       (_bcm_caladan3_mim_port_vlan_vector_internal(unit,
                                                  gport,
                                                  &phy_port,
                                                  &vpn,
                                                  &logicalPort));

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
         /* Always need to read the entry */
         soc_sbx_g3p1_pv2e_t_init(&pv2e);

         status = soc_sbx_g3p1_pv2e_get(unit, vid, phy_port, &pv2e);
         if (BCM_FAILURE(status)) {
             break;
         }

         if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
             /* this VID is a member of the vector, set it up */
             pv2e.vlan = vpn;
             pv2e.lpi = logicalPort;
         } else if (!BCM_VLAN_VEC_GET(vlan_vec, vid) &&
                    (pv2e.vlan == vpn) &&
		    (pv2e.lpi == logicalPort)) {
             pv2e.vlan = 0;
             pv2e.lpi = 0;
         }

         status = soc_sbx_g3p1_pv2e_set(unit, vid, phy_port, &pv2e);
         if (BCM_FAILURE(status)) {
             break;
         }
    }

    return status;
}

int
bcm_caladan3_mim_frame_max_access(int unit, bcm_gport_t gport,
                              int *size, int set)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g3p1_ft_t       ft;
    soc_sbx_g3p1_oi2e_t     oh;
    soc_sbx_g3p1_ete_t      ete;
    uint32                  fti, etei;

    G3P1_GPORT_RANGE_CHECK(unit, gport);

    fti = BCM_GPORT_MIM_PORT_ID_GET(gport);
    fti = _BCM_CALADAN3_MIM_PORTID_2_FTIDX(unit, fti);

    rv = _bcm_caladan3_egr_path_get(unit, fti, &ft, &oh, &ete, &etei);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Failed to get Egress path from "
                               "FT[0x%08x]: %d %s\n"),
                   FUNCTION_NAME(), fti, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (set) {
        ete.mtu = *size - 4;
        rv = soc_sbx_g3p1_ete_set(unit, etei, &ete);
    } else {
        *size = ete.mtu + 4;
    }

    return rv;
}

int
_bcm_caladan3_mim_port_vlan_vector_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;
    uint32                       logicalPort = 0;

    soc_sbx_g3p1_pv2e_t          pv2e;

    BCM_IF_ERROR_RETURN
       (_bcm_caladan3_mim_port_vlan_vector_internal(unit,
                                                  gport,
                                                  &phy_port,
                                                  &vpn,
                                                  &logicalPort));

    BCM_VLAN_VEC_ZERO(vlan_vec);

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

         soc_sbx_g3p1_pv2e_t_init(&pv2e);
         status = soc_sbx_g3p1_pv2e_get(unit, vid, phy_port, &pv2e);
         if (BCM_E_NONE == status) {
             if (pv2e.vlan == vpn) {
                 BCM_VLAN_VEC_SET(vlan_vec, vid);
             }
         }

         if (BCM_FAILURE(status)) {
             break;
         }
    }

    return status;
}

/* Trunk Important Notes:
 *
 * Only Homogeneous MiM ports are allowed on trunks.
 * i.e., If Default back bone port is created over trunk, only
 * more default back bone ports can exist. 
 * There cannot be Access & Backbone over same Trunk. It is a bad
 * configuration. Checking it on API is expensive.
 *
 * Currently MiM Supports only Back bone ports over Trunk */

/*
 * Function:
 *   _bcm_caladan3_mim_trunk_cb
 * Purpose:
 *     Call back function for Trunk Membership change
 * Returns:
 *   BCM_E_XX
 */ 
STATIC int
_bcm_caladan3_mim_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data)
{
    int status = BCM_E_NONE;
    soc_sbx_g3p1_p2e_t  p2e;
    soc_sbx_g3p1_ep2e_t ep2e;
    soc_sbx_g3p1_pv2e_t pv2e;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    int index=0, mymodid, pindex=0, idx=0, port=0;
    bcm_caladan3_mim_trunk_association_t *trunkAssoc;
 
    G3P1_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Unknown unit %d !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),unit, __FILE__, __LINE__));
        return BCM_E_PARAM;
    }

    if(tid >= SBX_MAX_TRUNKS) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Trunk ID  %d !!!!(%s,%d)\n"),
                   FUNCTION_NAME(),tid, __FILE__, __LINE__));
        return BCM_E_PARAM;
    }

    if(!tdata) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Bad Input Parameter !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_PARAM;
    }
    
    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_MIM,
                  (BSL_META_U(unit,
                              "ERROR %s Could not obtain my module ID !!!!(%s,%d)\n"),
                   FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_INTERNAL;
    }

    if(tdata->num_ports == 0) {
        /* nothing to do */
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
        tp[index] = -1;
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

    trunkAssoc = &caladan3_mim_trunk_assoc_info[unit][tid];

    switch(trunkAssoc->mimType) {

        case _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT:
            for(index=0; index < pindex; index++) {

                port = tp[index];

                /* configure back bone ports mim mode */
                if((port >= 0) && 
                   ((soc_sbx_g3p1_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
                   ((soc_sbx_g3p1_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

                    p2e.mim         = MIM_BBONE_PORT;
                    p2e.customer    = 0;
                    p2e.provider    = 1;
                    p2e.defstrip    = 0;
                    p2e.pstrip      = 0;
                    p2e.pbb         = 1;

                    ep2e.customer   = 1;
                    ep2e.mim        = MIM_BBONE_PORT;
                    ep2e.pbb        = 1;                

                    /* program P2E */
                    status = soc_sbx_g3p1_p2e_set(unit, port, &p2e);

                    if(BCM_SUCCESS(status)) {
                        /* program EP2E */
                        status = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);

                        if(BCM_FAILURE(status)) {

                            LOG_ERROR(BSL_LS_BCM_MIM,
                                      (BSL_META_U(unit,
                                                  "ERROR %s Programming EP2E %d : %s in (%s,%d)\n"),
                                       FUNCTION_NAME(), status, bcm_errmsg(status),
                                       __FILE__,__LINE__));
                        } else {
                            dq_p_t port_elem;
                            bcm_caladan3_mim_port_control_t *portcb = NULL;

                            /* For each back bone port on port list, reset lp on bvlan pv2e */
                            DQ_TRAVERSE(&trunkAssoc->plist, port_elem) {

                                port_elem -= _MIM_PCB_TRUNK_NODE_POS;
                                _BCM_CALADAN3_GET_PORTCB_FROM_LIST(port_elem, portcb);

                                soc_sbx_g3p1_pv2e_t_init(&pv2e);

                                status = soc_sbx_g3p1_pv2e_get(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                if(BCM_FAILURE(status)) {

                                    LOG_ERROR(BSL_LS_BCM_MIM,
                                              (BSL_META_U(unit,
                                                          "ERROR %s Getting PV2E %d : %s in (%s,%d)\n"),
                                               FUNCTION_NAME(), status, bcm_errmsg(status),
                                               __FILE__,__LINE__));
                                } else {
                                    /* Program Bvlan, port 2 Etc -> LPI */
                                    /* lpi on pv2e for back bone must be 0 to use physical port as lp */
                                    pv2e.lpi = 0;

                                    status =  soc_sbx_g3p1_pv2e_set(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                    if(BCM_FAILURE(status)) {

                                        LOG_ERROR(BSL_LS_BCM_MIM,
                                                  (BSL_META_U(unit,
                                                              "ERROR %s Programming PV2E %d : %s in (%s,%d)\n"),
                                                   FUNCTION_NAME(), status, bcm_errmsg(status),
                                                   __FILE__,__LINE__));
                                    }
                                }
                            } DQ_TRAVERSE_END(&trunkAssoc->plist, port_elem);
                        }
                    }

                } else {
                    status = BCM_E_INTERNAL;
                }
            }
            break;

        default:
            status = BCM_E_INTERNAL;
            break;
    }

    _MIM_UNLOCK(unit);

    return status;
}
#endif /* BCM_CALADAN3_MIM_SUPPORT */
#endif /* INCLUDE_L3 */
