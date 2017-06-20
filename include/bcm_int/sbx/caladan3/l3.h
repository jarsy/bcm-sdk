/*
 * $Id: l3.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l3.h
 * Purpose:     L3 and MPLS internal definitions
 *              specific to FE2000 BCM API implementation
 */

#ifndef _BCM_INT_SBX_CALADAN3_L3_H_
#define _BCM_INT_SBX_CALADAN3_L3_H_

#include <bcm/mpls.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm_int/sbx/state.h>

#include <soc/sbx/sbDq.h>
#include <soc/enet.h>
#include <soc/sbx/caladan3.h>

#define L3_INTF_ADD_SMAC                      1
#define L3_INTF_DEL_SMAC                      2
#define L3_INTF_FIND_SMAC                     3
#define L3_INTF_FIND_FREE_SMAC_SLOT           4

#define L3_INTF_VRF_CHANGED                0x01
#define L3_INTF_MAC_CHANGED                0x02
#define L3_INTF_VID_CHANGED                0x04
#define L3_INTF_TTL_CHANGED                0x08
#define L3_INTF_MTU_CHANGED                0x10

#define BCM_CALADAN3_L3_INTF_BASE        0xCA000000
#define BCM_CALADAN3_L3_EGR_BASE         0xDB000000
#define BCM_CALADAN3_EGR_MULTI_BASE      0xED000000
#define BCM_CALADAN3_IPMC_INDEX_BASE     0xFE000000
#define BCM_CALADAN3_L3_FTE_BASE         0xBC000000

#define BCM_CALADAN3_L3_INTF_BASE_MASK   0xFF000000
#define BCM_CALADAN3_OHI_BASE_MASK       0xFF000000
#define BCM_CALADAN3_L3_FTE_BASE_MASK    0xFF000000

/*
 * Some platforms might not get this during lib compile.
 */
#ifndef IPPROTO_GRE
#define IPPROTO_GRE    47
#endif

#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP 0x800
#endif

/**
 * XXX: MOVE THESE TO GLOBAL VISIBLE .h like l3.h
 */
#define _CALADAN3_L3_VRF_DROP                1
#define _BCM_VRF_VALID(_vrf)            (((_vrf) >= 0) && (_vrf < SBX_MAX_VRF))
#define _BCM_TTL_VALID(_ttl_)           (((_ttl_) >= 0) && ((_ttl_) <= 0xff))
#define _BCM_EXP_VALID(_exp_)           ((_exp_) <= 7)
#define _BCM_PKT_PRI_VALID(_pri_)       ((_pri_) <= 7)
#define _BCM_PKT_CFI_VALID(_cfi_)       ((_cfi_) <= 1)
#define _CALADAN3_QOS_MAP_ID_VALID(_map_)  (1)   /* XXX */
#define _CALADAN3_DEFAULT_EGR_MTU             9206
#define _CALADAN3_INTF_DEFAULT_TTL            1

#define IPV6_FMT  "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define IPV6_PFMT(ip) ip[0],ip[1],ip[2],ip[3],ip[4],ip[5],ip[6],ip[7],\
                      ip[8],ip[9],ip[10],ip[11],ip[12],ip[13],ip[14],ip[15]
 
#define _CALADAN3_IFID_VALID_RANGE(ifid) \
    (((ifid) >= _SBX_CALADAN3_MIN_VALID_IFID) && \
     ((ifid) <=  _SBX_CALADAN3_MAX_VALID_IFID))

#define _CALADAN3_IFID_FROM_USER_HANDLE(handle)     \
    ((handle) - BCM_CALADAN3_L3_INTF_BASE)

#define _CALADAN3_USER_HANDLE_FROM_IFID(ifid)               \
    ((ifid) +  BCM_CALADAN3_L3_INTF_BASE)

#define _CALADAN3_L3_USER_IFID_VALID(ifid)                                  \
     ((((ifid) & BCM_CALADAN3_L3_INTF_BASE_MASK)) == BCM_CALADAN3_L3_INTF_BASE)

#define _CALADAN3_L3_FTE_VALID(unit, fte)                                     \
    ((((fte) & BCM_CALADAN3_L3_FTE_BASE_MASK) == BCM_CALADAN3_L3_FTE_BASE) && \
     ((fte - BCM_CALADAN3_L3_FTE_BASE) >= SBX_DYNAMIC_FTE_BASE(unit)))

#define _CALADAN3_TRUNK_FTE_VALID(unit, fte)                                     \
    ((((fte) & BCM_CALADAN3_L3_FTE_BASE_MASK) == BCM_CALADAN3_L3_FTE_BASE) && \
     ((fte - BCM_CALADAN3_L3_FTE_BASE) >= SBX_TRUNK_FTE_BASE(unit)))

#define  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(handle)      \
    ((uint32)handle - (BCM_CALADAN3_L3_FTE_BASE))

#define  _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx)    \
    ((fte_idx) + (BCM_CALADAN3_L3_FTE_BASE))

#define _CALADAN3_ETE_TYPE_NEEDS_OHI(ete_type)                          \
    (((ete_type) == _CALADAN3_L3_ETE__UCAST_IP) ||                      \
     ((ete_type) == _CALADAN3_L3_ETE__MCAST_IP))

#define _CALADAN3_L3_OHI_DYNAMIC_RANGE(ohi)                             \
    (((ohi) >= SBX_DYNAMIC_OHI_BASE) && ((ohi) != _CALADAN3_INVALID_OHI))

/* set the OI Base offset - 8K
 *  to reclaim TB_VID space
 */
#define  _CALADAN3_L3_G3P1_ADJUST_TB_OFFSET(ohi) ((ohi) - SBX_RAW_OHI_BASE)

#define _CALADAN3_USER_HANDLE_FROM_MVT_INDEX(mvt)           \
    ((mvt) + (BCM_CALADAN3_IPMC_INDEX_BASE))

#define _CALADAN3_MVT_INDEX_FROM_USER_HANDLE(handle)        \
    ((handle) - (BCM_CALADAN3_IPMC_INDEX_BASE))

#define _CALADAN3_VALID_IPMC_INDEX_USER_HANDLE(handle) \
    (((handle) >= (BCM_CALADAN3_IPMC_INDEX_BASE + SBX_MVT_ID_DYNAMIC_BASE)) && \
     ((handle) < (BCM_CALADAN3_IPMC_INDEX_BASE + SBX_MVT_ID_DYNAMIC_END)))

#define _CALADAN3_GET_ECMP_BITS_FROM_ECMP_SIZE(x, n)                    \
    do {                                                            \
          int _i, _tmpx;                                            \
          (n)   = 0;                                                \
          if (x > 1) {                                              \
             _tmpx = (x);                                           \
             for (_i = 0; _i < 32; _i++) {                          \
                if (_tmpx & 1)                                      \
                   (n) = _i + 1;                                    \
               _tmpx = _tmpx >> 1;                                  \
           }                                                        \
       }                                                            \
} while (0);

/* XXX: Please separate out the HW and SW portions */

#define _CALADAN3_INTF_ID_HASH_SIZE             256
#define _CALADAN3_INTF_VID_HASH_SIZE            256
#define _CALADAN3_INTF_EGR_LBL_HASH_SIZE        256

#define _CALADAN3_ETE_IDX_HASH_SIZE             512
#define _CALADAN3_OHI_IDX_HASH_SIZE             512
#define _CALADAN3_FTE_IDX_HASH_SIZE             512
#define _CALADAN3_INTF_L3ETE_HASH_SIZE           32
#define _CALADAN3_EGR_MP_HASH_SIZE              256
#define _CALADAN3_L3_ECMP_MAX                    32
#define _CALADAN3_L3_ECMP_DEFAULT                16
#define _CALADAN3_IPMC_HASH_SIZE                128
#define _CALADAN3_VPN_HASH_SIZE                  64

#define _CALADAN3_INVALID_OHI                        -1
#define _CALADAN3_INVALID_FTE                        -1
#define _CALADAN3_INVALID_ETE_IDX                     0  /* Need a global invalid ete-idx */
#define _CALADAN3_INVALID_TUNNEL_TERMINATOR_IDX      -1
#define _CALADAN3_INVALID_IFID                       -1
#define _CALADAN3_INVALID_VPN_ID                      0
#define _CALADAN3_INVALID_VRF                        -1

/**
 * Hash function for each of the hash tables.
 */
#define _CALADAN3_GET_FTE_HASH_IDX(_fte_idx)                \
    ((_fte_idx) &  ((_CALADAN3_FTE_IDX_HASH_SIZE) - 1))

#define _CALADAN3_GET_ETE_HASH_IDX(_ete_idx)                \
    ((_ete_idx) &  ((_CALADAN3_ETE_IDX_HASH_SIZE) - 1))

#define _CALADAN3_GET_HASH_EGR_MP(_fte_crc)                 \
    ((_fte_crc) &  ((_CALADAN3_EGR_MP_HASH_SIZE) - 1))

#define _CALADAN3_GET_HASH_MPBASE_MP(_mpbase)               \
    ((_mpbase) &  ((_CALADAN3_EGR_MP_HASH_SIZE) - 1))

#define _CALADAN3_GET_INTF_L3ETE_HASH_IDX(_ete_idx)         \
    ((_ete_idx) &  ((_CALADAN3_INTF_L3ETE_HASH_SIZE) - 1))

#define _CALADAN3_GET_OHI2ETE_HASH_IDX(_ohi_idx)            \
    ((_ohi_idx) &  ((_CALADAN3_OHI_IDX_HASH_SIZE) - 1))

#define _CALADAN3_GET_INTF_ID_HASH(_l3a_intf)                       \
    ((_l3a_intf) & ((_CALADAN3_INTF_ID_HASH_SIZE) - 1))

#define _CALADAN3_GET_INTF_VID_HASH(_l3a_vid)                       \
    ((_l3a_vid) & ((_CALADAN3_INTF_VID_HASH_SIZE) - 1))

#define _CALADAN3_GET_INTF_EGR_LBL_HASH(_label)                       \
    ((_label) & ((_CALADAN3_INTF_EGR_LBL_HASH_SIZE) - 1))

#define _CALADAN3_GET_MPLS_VPN_HASH(_vpn_id) \
    ((_vpn_id) & (_CALADAN3_VPN_HASH_SIZE - 1))

typedef enum {
    _caladan3_route_op__da_add = 1,
    _caladan3_route_op__da_del,
    _caladan3_route_op__da_mod,
    _caladan3_route_op__da_get,
    _caladan3_route_op__sa_add,
    _caladan3_route_op__sa_del,
    _caladan3_route_op__sa_mod,
    _caladan3_route_op__sa_get
} _caladan3_l3_route_op_t;


#define L3_ROUTE_DELETE_BY_INTF               0x1
#define L3_ROUTE_DELETE_ALL                   0x2

/**
 * Different types under the L3 class of ete
 */
typedef enum {
    _CALADAN3_L3_ETE__UCAST_IP,
    _CALADAN3_L3_ETE__MCAST_IP,
    _CALADAN3_L3_ETE__ENCAP_IP,
    _CALADAN3_L3_ETE__ENCAP_MPLS,
    _CALADAN3_L3_ETE__MAX_TYPES
} _caladan3_l3_ete_types_t;

typedef struct {
    uint32     ttl;
    uint32     flags;
#define L3_ETE_TTL_CHANGED  0x1
} _caladan3_l3_ete_change_t;

#define _CALADAN3_MPLS_ETE_MODIFY__L2_ETE_CHANGED       0x1
#define _CALADAN3_MPLS_ETE_MODIFY__UCAST_IP_ETE_CHANGED 0x2

typedef struct {
    uint32           ohi;
} _caladan3_ohi_t;

typedef struct {
    uint32           fte_idx;
} _caladan3_fte_idx_t;

typedef struct {
    uint32           ete_idx;
} _caladan3_ete_idx_t;

typedef struct {
    uint32           ipmc_idx;
} _caladan3_ipmc_idx_t;

#define _CALADAN3_ETE_USER_SLAB_SIZE               8

typedef struct _caladan3_l3_ete_fte_s {
    bcm_module_t            mod_id;
    _caladan3_fte_idx_t         fte_idx;
} _caladan3_l3_ete_fte_t;

typedef struct _caladan3_l3_ete_ipmc_s {
    uint32       ipmc_index;
    int          port;
} _caladan3_l3_ete_ipmc_t;

typedef _caladan3_l3_ete_fte_t   _caladan3_vc_ete_fte_t;
typedef _caladan3_l3_ete_ipmc_t  _caladan3_vc_ete_ipmc_t;

typedef struct _caladan3_l2encap_ete_s {
    _caladan3_ete_idx_t    ete_idx;     /* HW: L2 ete idx */
} _caladan3_l2encap_ete_t;

#define _CALADAN3_MAKE_ENCAP_MPLS_SW_ETE_KEY(tkey, nh_mac)              \
    do {                                                                \
        (tkey)->l3_ete_hk.type =  _CALADAN3_L3_ETE__ENCAP_MPLS;         \
        (tkey)->l3_ete_ttl     = 0;                                     \
        ENET_COPY_MACADDR((nh_mac), (tkey)->l3_ete_hk.dmac);            \
    } while (0);

#define _CALADAN3_L2_ETE_SHARED(l3_ete)                                        \
    ((l3_ete)->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP)


#define _CALADAN3_CALC_INTF_L3ETE_HASH(hash_idx, etype, mac)               \
    do {                                                                   \
        _caladan3_l3_ete_hash_key_t           _hk;                         \
        sal_memset(&_hk, 0, sizeof(_hk));                                  \
        _hk.type = (etype);                                                \
        ENET_COPY_MACADDR((mac), _hk.dmac);                                \
        (hash_idx) =                                                       \
            (( _shr_crc16(0, (unsigned char *)(&_hk), sizeof(_hk))) &      \
             ( _CALADAN3_INTF_L3ETE_HASH_SIZE - 1));                       \
    } while (0);


#define _CALADAN3_MAKE_SW_ETE_KEY(tkey, etype, mac, flags, vid, ttl)        \
    do {                                                                \
        (tkey)->l3_ete_hk.type = (etype);                               \
        ENET_COPY_MACADDR((mac), (tkey)->l3_ete_hk.dmac);               \
        (tkey)->l3_ete_ttl = (ttl);                                     \
   } while (0);

#define _CALADAN3_MAKE_UCAST_IP_SW_ETE_KEY(tkey, bcm_egr, l3_intf)          \
    _CALADAN3_MAKE_SW_ETE_KEY(tkey, _CALADAN3_L3_ETE__UCAST_IP,                 \
                          (bcm_egr)->mac_addr,                          \
                          (bcm_egr)->flags,                             \
                          (l3_intf)->if_info.l3a_vid,                   \
                          (l3_intf)->if_info.l3a_ttl);


#define _CALADAN3_MAKE_ENCAP_IP_SW_ETE_KEY(tkey, tunnel, vidop)             \
    do {                                                                \
        (tkey)->l3_ete_hk.type =  _CALADAN3_L3_ETE__ENCAP_IP;               \
       ENET_COPY_MACADDR((tunnel)->dmac, (tkey)->l3_ete_hk.dmac);       \
        (tkey)->l3_ete_ttl = (tunnel)->ttl;                             \
        (tkey)->l3_ete_vidop = vidop;                                   \
   } while (0);

/* XXX Needs to go in a sbx/bcmutils */
#define _CALADAN3_ETHER_NTOA(ea, buf)                                          \
    sal_sprintf((buf),"%02x:%02x:%02x:%02x:%02x:%02x",                     \
                (unsigned char)(ea)[0]&0xff, (unsigned char)(ea)[1]&0xff,  \
                (unsigned char)(ea)[2]&0xff, (unsigned char)(ea)[3]&0xff,  \
                (unsigned char)(ea)[4]&0xff, (unsigned char)(ea)[5]&0xff); \

#define _CALADAN3_IPV4_NTOA(v4, buf)                                           \
    sal_sprintf((buf),"%03d.%03d.%%03d.%03d",                              \
                (unsigned char)(v4)[0]&0xff, (unsigned char)(v4)[1]&0xff,  \
                (unsigned char)(v4)[2]&0xff, (unsigned char)(v4)[3]&0xff);

#define  _CALADAN3_DUMP_L3_ETE_KEY(tkey)                                    \
    do {                                                                \
        _CALADAN3_ETHER_NTOA((tkey)->l3_ete_hk.dmac, _g_debug_eabuf);       \
        L3_VERB(("ETE_KEY(type %d mac %s vidop %d ttl %d\n",            \
                 (tkey)->l3_ete_hk.type, _g_debug_eabuf,                \
                 (int)(tkey)->l3_ete_vidop, (int)(tkey)->l3_ete_ttl));   \
    } while (0);


#define _CALADAN3_L3FTE_FROM_FTE_HASH_DQ(e, var)                \
    (var) = DQ_ELEMENT(_caladan3_l3_fte_t *,                    \
                       (e), (var), fte_hash_link)

#define _CALADAN3_L3ETE_FROM_L3ETE_HASH_DQ(e, var)              \
    (var) = DQ_ELEMENT(_caladan3_l3_ete_t *,                    \
                       (e), (var), l3_ete_link)

#define _CALADAN3_L3ETE_FROM_L3OHI_HASH_DQ(e, var)             \
    (var) = DQ_ELEMENT(_caladan3_l3_ete_t *,                   \
                       (e), (var), l3_ohi_link)

#define _CALADAN3_L3ETE_FROM_IEH_HASH_DQ(e, var)                \
    (var) = DQ_ELEMENT(_caladan3_l3_ete_t *,                    \
                       (e), (var), l3_ieh_link)

#define _CALADAN3_VPN_SAP_FROM_MPLSETE_DQ(e, var)           \
    (var) = DQ_ELEMENT(_caladan3_vpn_sap_t *,               \
                       (e), (var), vc_mpls_ete_link)

#define _CALADAN3_VPN_SAP_FROM_SAP_DQ(e, var)           \
    (var) = DQ_ELEMENT(_caladan3_vpn_sap_t *,           \
                       (e), (var), vc_vpn_sap_link)

#define _CALADAN3_VPN_SAP_FROM_OHI_HASH_DQ(e, var)              \
    (var) = DQ_ELEMENT(_caladan3_vpn_sap_t *,                   \
                       (e), (var), vc_ohi_link)

#define _CALADAN3_VPNC_FROM_HASH_DQ(e, var)              \
    (var) = DQ_ELEMENT(_caladan3_vpn_control_t *,        \
                       (e), (var), vpn_fe_link)

#define _CALADAN3_VPN_SAP_FROM_TRUNK_LINK(e, var)              \
    (var) = DQ_ELEMENT(_caladan3_vpn_sap_t *,                   \
                       (e), (var), trunk_port_link)
/**
 * Use goto to break out of these loop macros
 */
#define _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, hid, l3_ete)               \
    do  {                                                               \
        dq_p_t __ete_elem;                                              \
        DQ_TRAVERSE(&l3_intf->if_ete_hash[hid], __ete_elem) {           \
            _CALADAN3_L3ETE_FROM_IEH_HASH_DQ(__ete_elem, l3_ete);

#define _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, hid, l3_ete)           \
    } DQ_TRAVERSE_END(&l3_intf->if_ete_hash[hid], __ete_elem);          \
    } while (0)

/**
 * Use goto to break from these loop macros
 */
#define _CALADAN3_ALL_L3ETE_ON_INTF(l3_intf, l3_ete)                        \
    do {                                                                \
        int __hash_idx;                                                 \
        for (__hash_idx = 0;                                            \
             __hash_idx < _CALADAN3_INTF_L3ETE_HASH_SIZE; __hash_idx++) {   \
            _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, __hash_idx, l3_ete) {

#define _CALADAN3_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete)                    \
    } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, __hash_idx, l3_ete);     \
    }                                                                   \
    } while (0)


/**
 * Please use goto to break out of nested loops hidden under macros
 */
#define _CALADAN3_ALL_IPMCETE_ON_ALLINTF(l3_fe, l3_intf, m_ete)             \
    do {                                                                \
        _caladan3_l3_ete_t       *_l3_ete = NULL;                           \
        m_ete = NULL;                                                   \
        _CALADAN3_ALL_L3INTF(l3_fe, l3_intf) {                              \
            _CALADAN3_ALL_L3ETE_ON_INTF(l3_intf, _l3_ete) {                 \
                if (_l3_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) {  \
                    m_ete = _l3_ete; _l3_ete = NULL;

#define _CALADAN3_ALL_IPMCETE_ON_ALLINTF_END(l3_fe, l3_intf, m_ete)         \
    }                                                                   \
    } _CALADAN3_ALL_L3ETE_ON_INTF_END(l3_intf, _l3_ete);                    \
    } _CALADAN3_ALL_L3INTF_END(l3_fe, l3_intf);                             \
    } while (0)

typedef struct _caladan3_l3_ete_hash_key_s {
    _caladan3_l3_ete_types_t  type;
    bcm_mac_t             dmac;
} _caladan3_l3_ete_hash_key_t;

typedef struct _caladan3_l3_ete_key_s {
    _caladan3_l3_ete_hash_key_t     l3_ete_hk;
    int                         l3_ete_ttl;
} _caladan3_l3_ete_key_t;


typedef struct {
    dq_t                            l3_vc_ete_head;       /* vpn-ete linked list            */
    dq_t                            l3_ieh_link;          /* Interface ETE Hash link        */
    dq_t                            l3_ete_link;          /* linked via the eteidx2ETE hash */
    dq_t                            l3_ohi_link;          /* ohi2ete link                   */
    bcm_if_t                        l3_intf_id;           /* reference to _caladan3_l3_intf_t   */
    _caladan3_l3_ete_key_t              l3_ete_key;
    _caladan3_ohi_t                     l3_ohi;               /* HW ohi                         */
    _caladan3_ete_idx_t                 l3_ete_hw_idx;        /* HW ete index                   */
    _caladan3_ohi_t                     l3_mpls_ohi;          /* mpls derived ohi from l3 egr   */
    uint32                          l3_ete_hw_stat_idx;
    int32                           l3_alloced_ue;        /* allocated # user entries       */
    int32                           l3_inuse_ue;          /* inuse  # user entries          */
    union {
        _caladan3_l3_ete_fte_t         *l3_fte;
        _caladan3_l3_ete_ipmc_t        *l3_ipmc;
    } u;
} _caladan3_l3_ete_t;

#define L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY  0x1
#define L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY  0x2
#define L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH       0x4

/* Delete FTE only */
#define L3_OR_MPLS_DESTROY_FTE__FTE_ONLY                 0x1
/* Delete FTE, OHI from fte, then delete OHI --> ETE */
#define L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE           0x2
/* Delete FTE, module and OHI from param to delete OHI --> ETE */
#define L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI    0x4

#define _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE(mpls_sw_ete, vpn_sap)            \
    do {                                                                \
        dq_p_t _vs_elem;                                                \
        DQ_TRAVERSE(&(mpls_sw_ete)->l3_vc_ete_head, _vs_elem) {         \
            _CALADAN3_VPN_SAP_FROM_MPLSETE_DQ(_vs_elem, (vpn_sap));         \


#define _CALADAN3_ALL_VPN_SAP_PER_MPLS_ETE_END(mpls_sw_ete, vpn_sap)     \
    } DQ_TRAVERSE_END(&(mpls_sw_ete)->l3_vc_ete_head, _vs_elem); \
    } while (0)


typedef struct {
    dq_t                            fte_hash_link;
    uint32                          fte_idx;
    uint32                          ref_count;
} _caladan3_l3_fte_t;

typedef struct {
    uint32        flags;
    uint32        mpbase;
    uint32        crc16;
    struct {
        uint32                count;
        uint32                fte_indices[_CALADAN3_L3_ECMP_MAX];
    } crc_data;
    void                     *fte_contents[_CALADAN3_L3_ECMP_MAX];
    _caladan3_l3_fte_t           *fte_hash_elem[_CALADAN3_L3_ECMP_MAX];
} _caladan3_l3_mpath_crc_info_t;

typedef enum {
  _CALADAN3_FT_OPCODE_SET,
  _CALADAN3_FT_OPCODE_GET,
  _CALADAN3_FT_OPCODE_CLR,
  _CALADAN3_MAX_FT_OPCODE
}_caladan3_fte_opcode;


#define _CALADAN3_L3INTF_FROM_VID_DQ(e, var) \
    (var) = DQ_ELEMENT(_caladan3_l3_intf_t *, e, (var), if_vid_link)

#define _CALADAN3_L3INTF_FROM_IFID_DQ(e, var) \
    (var) = DQ_ELEMENT(_caladan3_l3_intf_t *, e, (var), if_ifid_link)

#define _CALADAN3_L3INTF_FROM_EGR_LBL_DQ(e, var) \
    (var) = DQ_ELEMENT(_caladan3_l3_intf_t *, e, (var), if_egr_lbl_link)

typedef struct {
    dq_t                     if_vid_link;
    dq_t                     if_ifid_link;
    dq_t                     if_egr_lbl_link;
    dq_t                     if_ete_hash[_CALADAN3_INTF_L3ETE_HASH_SIZE];
    dq_t                     if_oam_ep_list;
    int                      if_ip_ete_count;

    bcm_l3_intf_t            if_info;        /* user passed in info, required to create ETEs  */
    bcm_tunnel_initiator_t  *if_tunnel_info; /* valid if tunnel_initiator_set has been called */
    uint32                   if_flags;
    uint32                   if_lmac_idx;
    bcm_mpls_label_t         tunnel_egr_label;
#define _CALADAN3_L3_INTF_IN_VID_LIST      0x00000001
#define _CALADAN3_L3_INTF_IN_IFID_LIST     0x00000002
#define _CALADAN3_L3_INTF_SMAC_IN_TBL      0x00000004
#define _CALADAN3_L3_MPLS_TUNNEL_SET       0x00000008
#define _CALADAN3_L3_MPLS_TUNNEL_DROP      0x00000010
#define _CALADAN3_L3_INTF_IN_EGR_LBL_LIST  0x00000020
} _caladan3_l3_intf_t;

#define _CALADAN3_ALL_L3INTF(l3_fe, l3_intf)                                \
    do {                                                                \
        int     _hidx;                                                  \
        dq_p_t  l3_intf_elem;                                           \
        for (_hidx = 0; _hidx < _CALADAN3_INTF_ID_HASH_SIZE; _hidx++) {     \
            DQ_TRAVERSE(&l3_fe->fe_intf_by_id[_hidx], l3_intf_elem) {   \
                _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);

#define _CALADAN3_ALL_L3INTF_END(l3_fe, l3_intf)    \
    } DQ_TRAVERSE_END(&l3_fe->fe_intf_by_id[_hidx], l3_intf_elem);      \
    }                                                                   \
    } while (0)


typedef struct _caladan3_egr_mp_info_s {
    dq_t                 egr_mp_link;
    dq_t                 mpbase_link;
    uint32               mp_base;
    uint32               fte_crc;
    uint32               num_valid;
    uint32              *mapped_fte;
} _caladan3_egr_mp_info_t;

typedef struct _caladan3_g3p1_ipv4_key_s {
    uint32 ip;
    int ipcxt;
    int length;
} _caladan3_g3p1_ipv4_key_t;

typedef struct _caladan3_g3p1_ipv6_key_s {
    soc_sbx_g3p1_16_byte_t ip;
    int                    prefix;
} _caladan3_g3p1_ipv6_key_t;


typedef union caladan3_g3p1_da_route_u {
    soc_sbx_g3p1_v4da_t    v4;
    soc_sbx_g3p1_v6da_t    v6;
} _caladan3_g3p1_da_route_t;

typedef union caladan3_g3p1_sa_route_u {
    soc_sbx_g3p1_v4sa_t    v4;
    soc_sbx_g3p1_v6sa_t    v6;
} _caladan3_g3p1_sa_route_t;


typedef union caladan3_g3p1_lpm_key_u {
    _caladan3_g3p1_ipv4_key_t     v4;
    _caladan3_g3p1_ipv6_key_t     v6;
} _caladan3_g3p1_lpm_key_t;

#define _CALADAN3_MP_INFO_FROM_EGR_MP_DQ(e, var)  \
    (var) = DQ_ELEMENT(_caladan3_egr_mp_info_t *, (e), (var), egr_mp_link)

#define _CALADAN3_MP_INFO_FROM_MPBASE_MP_DQ(e, var)  \
    (var) = DQ_ELEMENT(_caladan3_egr_mp_info_t *, (e), (var), mpbase_link)


/* To avoid walking all the (S,G) and match ipmc_index */
#define _CALADAN3_IPMC_HASH_IDX(x) ((x) % _CALADAN3_IPMC_HASH_SIZE)

#define _CALADAN3_IPMC_RT_FROM_ID_DQ(e, var)                                \
    (var) = DQ_ELEMENT(_caladan3_ipmc_route_t *, (e), (var), id_link)

#define _CALADAN3_IPMC_RT_PER_BKT(l3_fe, hidx, ipmc_rt)                     \
    do  {                                                               \
        dq_p_t __ipmc_rt_elem;                                          \
        DQ_TRAVERSE(&l3_fe->fe_ipmc_by_id[hidx],                           \
                    __ipmc_rt_elem) {                                   \
            _CALADAN3_IPMC_RT_FROM_ID_DQ(__ipmc_rt_elem, ipmc_rt);

#define _CALADAN3_IPMC_RT_PER_BKT_END(l3_fe, hidx, ipmc_rt) \
    } DQ_TRAVERSE_END(&l3_fe->fe_ipmc_by_id[hidx], __ipmc_rt_elem);        \
    } while (0)


#define _CALADAN3_ALL_IPMC_RT(l3_fe, ipmc_rt)                               \
    do {                                                                \
        int __hash_idx;                                                 \
        for (__hash_idx = 0;                                            \
             __hash_idx < _CALADAN3_IPMC_HASH_SIZE; __hash_idx++) {         \
            _CALADAN3_IPMC_RT_PER_BKT(l3_fe, __hash_idx, ipmc_rt) {         \


#define _CALADAN3_ALL_IPMC_RT_END(l3_fe, ipmc_rt)                          \
    } _CALADAN3_IPMC_RT_PER_BKT_END(l3_fe, __hash_idx, ipmc_rt);           \
    }                                                                  \
    } while (0)

#define _CALADAN3_ALL_IPMC_RT_BREAK() \
    __hash_idx = _CALADAN3_IPMC_HASH_SIZE; break;


typedef struct {
    dq_t                     id_link;
    int                      ipmc_index;
    bcm_ipmc_addr_t          ipmc_user_info; /* user passed info */
} _caladan3_ipmc_route_t;

#define _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc)                       \
    do  {                                                               \
        dq_p_t __ete_elem;                                              \
        DQ_TRAVERSE(&(l3_fe)->fe_vpn_hash[hidx], __ete_elem) {          \
            _CALADAN3_VPNC_FROM_HASH_DQ(__ete_elem, (vpnc));

#define _CALADAN3_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc)           \
    } DQ_TRAVERSE_END(&(l3_fe)->fe_vpn_hash[hidx], __ete_elem); \
    } while (0)

#define _CALADAN3_ALL_VPNC(l3_fe, vpnc)                                     \
    do {                                                                \
        int _hidx;                                                      \
        for (_hidx = 0; _hidx < _CALADAN3_VPN_HASH_SIZE; _hidx++) {         \
            _CALADAN3_ALL_VPNC_PER_BKT(l3_fe, _hidx, vpnc) {

#define _CALADAN3_ALL_VPNC_END(l3_fe, vpnc)                 \
    } _CALADAN3_ALL_VPNC_PER_BKT_END(l3_fe, _hidx, vpnc);   \
    }                                                   \
    } while (0)

typedef struct {
    dq_t             vpn_fe_link;
    dq_t             vpn_sap_head;
    bcm_vpn_t        vpn_id;            /* internal context        */
    int              vpn_vrf;
    int              vpn_bc_mcg;
    uint32           vpn_flags;
    uint32           vpls_color;          /* VPLS Color */
} _caladan3_vpn_control_t;


#define _CALADAN3_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap)                           \
    do {                                                                \
        dq_p_t _vs_elem;                                                \
        DQ_TRAVERSE(&(vpnc)->vpn_sap_head, _vs_elem) {                  \
            _CALADAN3_VPN_SAP_FROM_SAP_DQ(_vs_elem, (vpn_sap));             \

#define _CALADAN3_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap)       \
    } DQ_TRAVERSE_END(&(vpnc)->vpn_sap_head, _vs_elem);     \
    } while (0)

typedef struct {
    dq_t                            vc_vpn_sap_link;      /* threaded from vpn_cb           */
    dq_t                            vc_mpls_ete_link;     /* threaded from mpls_tunnel_ete  */
    dq_t                            vc_ohi_link;          /* via vc_ohi2ete link            */
    dq_t                            trunk_port_link;      /* via mpls_trunk_assoc_info link */
    _caladan3_vpn_control_t            *vc_vpnc;              /* vpn control for this vpn-sap   */
    _caladan3_ohi_t                     vc_ohi;               /* HW ohi                         */
    _caladan3_ete_idx_t                 vc_ete_hw_idx;        /* HW ete index                   */
    uint32                          vc_ete_hw_stat_idx;
    uint32                          vc_alloced_ue;        /* allocated # user entries       */
    uint32                          vc_inuse_ue;          /* inuse  # user entries          */
    uint8                         vc_res_alloced;       /* resources allocated on unit    */
    union {
        _caladan3_vc_ete_fte_t         *vc_fte;
        _caladan3_vc_ete_ipmc_t        *vc_ipmc;
    } u;
    bcm_mpls_port_t                 vc_mpls_port;         /* User::  port information       */
    bcm_gport_t                     vc_mpls_port_id;      /* gport for this vpn-sap         */
    uint32                          logicalPort;          /* to remember P3 lpi             */
    uint32                          mpls_psn_label;       /* MPLS PSN LSP label, to be added*/
                                                          /* over VC label                  */
    uint32                          mpls_psn_label_exp;
    uint32                          mpls_psn_label_ttl;
    uint32                          mpls_psn_label2;
    uint32                          mpls_psn_ing_label;  /* LSP Label */
    uint32                          mpls_psn_ing_label2; /* LSP inner Label */
} _caladan3_vpn_sap_t;

/*
 * XXX: Derive from SB
 */
#define _CALADAN3_L3_MPLS_LBL_MAX         0x0FFFF
#define _CALADAN3_L3_MPLS_LBL_MASK        0x0FFFF
#define _CALADAN3_L3_MPLSTP_LBL_MASK      0x0FFFFF
#define _CALADAN3_L3_MPLS_PWE3_LBL_ID     0x80000
#define _BCM_MPLS_IMPLICIT_NULL_LABEL  (3)
#define _CALADAN3_MAX_MPLS_TNNL_LABELS 3

typedef struct {
    /* FTE contents */
    bcm_if_t                    encap_id;
    int                         fte_node;
    bcm_module_t                fte_modid;
    bcm_port_t                  fte_port;

    /* ETE contents */
    int                         label_count;
    bcm_mpls_egress_label_t     label_array[_CALADAN3_MAX_MPLS_TNNL_LABELS];
    bcm_mpls_port_t             mpls_port;
} _caladan3_l3_or_mpls_egress_t;


typedef struct {
    dq_t                 fe_vpn_hash[_CALADAN3_VPN_HASH_SIZE];
    dq_t                 fe_intf_by_id[_CALADAN3_INTF_ID_HASH_SIZE];
    dq_t                 fe_intf_by_vid[_CALADAN3_INTF_VID_HASH_SIZE];
    dq_t                 fe_intf_by_egr_lbl[_CALADAN3_INTF_EGR_LBL_HASH_SIZE];
    dq_t                 fe_ipmc_by_id[_CALADAN3_IPMC_HASH_SIZE];
    int                  fe_ipmc_enabled;
    uint32               fe_mp_set_size;
    int                  fe_ipv4_vrf_bits;
    _caladan3_vpn_control_t *fe_vpn_by_vrf[SBX_MAX_VRF];

    int                  fe_unit;
    int                  fe_my_modid;
    int                  fe_cosq_config_numcos;
    uint32               vlan_ft_base;
    uint32               umc_ft_offset;
    uint32               vpws_uni_ft_offset;
    uint32               max_pids;
#ifdef BCM_WARM_BOOT_SUPPORT 
    uint32               line_vsi_base;
    uint32               wb_hdl;
    uint32               scache_size;
#endif

    /* Global VSI allocated at init time
     * RoutedVlan2Etc.ulVrf for this VSI contains
     * BCM_L3_VRF_DEFAULT
     */
    uint32               fe_vsi_default_vrf;
    uint32               fe_drop_vrf;
    uint32               fe_raw_ete_idx;           /* Raw ETE              */
    uint32               fe_flags;
#define _CALADAN3_L3_FE_FLG_MP_SIZE_SET 0x000001
#define _CALADAN3_L3_FE_FLG_IPMC_INIT   0x000002
#define _CALADAN3_L3_FE_FLG_MPLS_INIT   0x000004

    dq_t                 fe_mp_by_egr[_CALADAN3_EGR_MP_HASH_SIZE];
    dq_t                 fe_mp_by_mpbase[_CALADAN3_EGR_MP_HASH_SIZE];
    dq_t                 fe_eteidx2_l3_ete[_CALADAN3_ETE_IDX_HASH_SIZE];
    dq_t                 fe_ohi2_l3_ete[_CALADAN3_OHI_IDX_HASH_SIZE];
    dq_t                 fe_ohi2_vc_ete[_CALADAN3_OHI_IDX_HASH_SIZE];    
    dq_t                 fe_fteidx2_fte[_CALADAN3_FTE_IDX_HASH_SIZE];
} _caladan3_l3_fe_instance_t;

typedef struct {
    void         *device_pvt_data;
} _l3_device_unit_t;

#define BCM_L3_CMP_GREATER       (1)
#define BCM_L3_CMP_EQUAL         (0)
#define BCM_L3_CMP_LESS         (-1)
#define BCM_L3_CMP_NOT_EQUAL     (2)


extern _l3_device_unit_t _l3_units[];
extern sal_mutex_t       _l3_mlock[];

#define L3_UNIT_INVALID(unit) \
    (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS))

#define L3_LOCK_CREATED_ALREADY(unit) \
(_l3_mlock[(unit)] != NULL)

#define L3_LOCK(unit)       \
    (L3_LOCK_CREATED_ALREADY(unit)? \
     sal_mutex_take(_l3_mlock[(unit)], sal_mutex_FOREVER)?BCM_E_UNIT:BCM_E_NONE: \
     BCM_E_UNIT)


#define L3_UNLOCK(unit)     \
    sal_mutex_give(_l3_mlock[(unit)])


extern _caladan3_l3_fe_instance_t *
_bcm_caladan3_get_l3_instance_from_unit(int unit);

extern int _bcm_caladan3_l3_do_lpm_commit(int unit, bcm_l3_route_t *info);

extern int _bcm_caladan3_l3_sw_ete_create(int          unit,
                                        _caladan3_l3_fe_instance_t *l3_fe,
                                        _caladan3_l3_intf_t        *l3_intf,
                                        _caladan3_l3_ete_key_t     *etekey,
                                        uint32                  ohi,
                                        uint32                  flags,
                                        bcm_port_t              port,
                                        uint32                  vid,
                                        _caladan3_l3_ete_t        **l3_ete);

extern int _bcm_caladan3_l3_sw_ete_find(int unit,
                                      _caladan3_l3_fe_instance_t *l3_fe,
                                      _caladan3_l3_intf_t        *l3_intf,
                                      _caladan3_l3_ete_key_t     *ete_key,
                                      _caladan3_l3_ete_t **sw_ete);

extern int _bcm_caladan3_l3_sw_ete_destroy(int unit,
                                         _caladan3_l3_fe_instance_t *l3_fe,
                                         _caladan3_l3_intf_t        *l3_intf,
                                         _caladan3_l3_ete_t **sw_ete);

extern int _bcm_caladan3_l3_sw_ete_find_by_ohi(_caladan3_l3_fe_instance_t *l3_fe,
                                             _caladan3_ohi_t     *ohi,
                                             _caladan3_l3_ete_t **sw_ete);

extern int _bcm_caladan3_expand_ete_users(_caladan3_l3_ete_t         *l3_ete);

extern int _bcm_caladan3_l3_find_intf_by_ifid(_caladan3_l3_fe_instance_t *l3_fe,
                                            uint32 intf_id,
                                            _caladan3_l3_intf_t **ret_l3_intf);

extern int _bcm_caladan3_l3_find_intf_by_vid_mac(_caladan3_l3_fe_instance_t *l3_fe,
                                               bcm_l3_intf_t          *key,
                                               _caladan3_l3_intf_t        **ret_l3_intf);

extern int _bcm_caladan3_l3_find_intf_by_egr_label(
                                     _caladan3_l3_fe_instance_t *l3_fe,
                                     bcm_mpls_label_t        egr_label,
                                     _caladan3_l3_intf_t       **ret_l3_intf);

extern int _bcm_caladan3_l3_get_egrif_from_fte(_caladan3_l3_fe_instance_t *l3_fe,
                                  uint32                  fte_idx,
                                  uint32                  flags,
                                  bcm_l3_egress_t        *bcm_egr);

int
_bcm_caladan3_get_local_l3_egress_from_ohi(_caladan3_l3_fe_instance_t *l3_fe,
                                         bcm_l3_egress_t        *bcm_egr);

extern int
_bcm_caladan3_l3_get_sw_fte(_caladan3_l3_fe_instance_t      *l3_fe,
                          uint32                      fte_idx,
                          _caladan3_l3_fte_t            **sw_fte);
extern int
_bcm_caladan3_alloc_l3_or_mpls_fte(_caladan3_l3_fe_instance_t   *l3_fe,
                                 int                       flag,
                                 bcm_trunk_t               trunk,
                                 _sbx_caladan3_usr_res_types_t  res_type,
                                 uint32                   *fte_idx);
extern int
_bcm_caladan3_invalidate_l3_or_mpls_fte(_caladan3_l3_fe_instance_t   *l3_fe,
                                      uint32                    fte_idx);
extern int
_bcm_caladan3_get_ete_by_type_on_intf(_caladan3_l3_fe_instance_t  *l3_fe,
                                      _caladan3_l3_intf_t         *l3_intf,
                                      _caladan3_l3_ete_types_t     ete_type,
                                      _caladan3_l3_ete_t         **l3_ete);

extern int
_bcm_fe2000_enable_mpls_tunnel(_caladan3_l3_fe_instance_t *l3_fe,
                               _caladan3_l3_intf_t        *l3_intf,
                               _caladan3_l3_ete_t         *v4_ete);

extern int
_bcm_caladan3_alloc_default_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                   _caladan3_l3_intf_t        *l3_intf,
                                   _caladan3_l3_ete_key_t     *l3_ete_key,
                                   uint32                  ohi_encap_id,
                                   _caladan3_l3_ete_t        **sw_ete);

extern int
_bcm_caladan3_undo_l3_ete_alloc(_caladan3_l3_fe_instance_t *l3_fe,
                                _caladan3_l3_intf_t        *l3_intf,
                                _caladan3_l3_ete_t         **p_l3_sw_ete);


extern int _bcm_caladan3_l3_sw_uninit(int unit);


int _bcm_caladan3_l3_flush_cache(int unit, int flag);

int
_bcm_caladan3_get_l3_ete_context_by_index(_caladan3_l3_fe_instance_t *l3_fe,
                                          uint32                      ete_idx,
                                          _caladan3_l3_ete_t        **sw_ete);

extern void
_bcm_caladan3_l3_sw_dump(int unit);

#define L3_UKNOWN_UCODE_WARN(_u) do { \
    SBX_UNKNOWN_UCODE_WARN(_u);            \
} while(0);



#endif /* _BCM_INT_SBX_CALADAN3_L3_H_ */
