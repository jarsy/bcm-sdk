/*
 * $Id: l3.c,v 1.28.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    l3.c
 * Purpose: Manages L3 interface table, forwarding table, routing table
 */

#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/enet.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#endif

#include <bcm_int/sbx/error.h>

#include <shared/gport.h>
#include <bcm/types.h>
#include <bcm/module.h>
#include <bcm/error.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <bcm/pkt.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/state.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/l3_priv.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#endif

#include <bcm_int/sbx/caladan3/wb_db_l3.h>

#define MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                      (mac)[3], (mac)[4], (mac)[5]


/*
 * XXX: Move these
 */


#define V4UC_STR_SEL_RULE 0
/*
 * Local Defines
 */
#define CALADAN3_TUNNEL_SUPPORTED_FLAGS  (BCM_TUNNEL_REPLACE)
#define L3_INTF_SUPPORTED_FLAGS  (BCM_L3_UNTAG      | \
                                  BCM_L3_ADD_TO_ARL | \
                                  BCM_L3_WITH_ID    | \
                                  BCM_L3_REPLACE    | \
                                  BCM_L3_IP6        | \
                                  BCM_L3_RPE)

#define L3_ROUTE_SUPPORTED_FLAGS (BCM_L3_REPLACE      | \
                                  BCM_L3_RPF          | \
                                  BCM_L3_TGID         | \
                                  BCM_L3_MULTIPATH    | \
                                  BCM_L3_IP6          | \
                                  BCM_L3_SRC_DISCARD)

#define L3_INTF_LOCATE_BY_IFID                1
#define L3_INTF_LOCATE_BY_VID                 2

#define L3_EGR_INTF_CHG                    0x01
#define L3_EGR_DMAC_CHG                    0x02
#define L3_EGR_MODULE_CHG                  0x04
#define L3_EGR_PORT_CHG                    0x08
#define L3_EGR_TRUNK_CHG                   0x10
#define L3_EGR_TGID_CHG                    0x20
#define L3_EGR_OHI_CHG                     0x40


#define _CALADAN3_INCR_FTE_REFCNT                 1
#define _CALADAN3_DECR_FTE_REFCNT                 2


#define L3_CRC_INFO_DONT_FETCH_FTE_DATA       0
#define L3_CRC_INFO_FETCH_FTE_DATA            1

#define L3_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define L3_WB_CURRENT_VERSION            L3_WB_VERSION_1_0

/* Assume that bcm param is in network order */
#define _CALADAN3_L3_CMP_BCM_SB_IPV4(b4, s4)                  \
    (((s4).uByte0 == ((uint8 *)&(b4))[0]) &&              \
     ((s4).uByte1 == ((uint8 *)&(b4))[1]) &&              \
     ((s4).uByte2 == ((uint8 *)&(b4))[2]) &&              \
     ((s4).uByte3 == ((uint8 *)&(b4))[3]))

#define _CALADAN3_L3_BCM_TO_SB_IPV4(b4, s4)                   \
    do                                  {                 \
        (s4).uByte0 = ((uint8 *)&(b4))[0];                \
        (s4).uByte1 = ((uint8 *)&(b4))[1];                \
        (s4).uByte2 = ((uint8 *)&(b4))[2];                \
        (s4).uByte3 = ((uint8 *)&(b4))[3];                \
    } while (0)

#define _CALADAN3_L3_SB_IPV4_TO_BCM(s4, b4)                   \
    do                                  {                 \
        ((uint8 *)&(b4))[0] = (s4).uByte0;                \
        ((uint8 *)&(b4))[1] = (s4).uByte1;                \
        ((uint8 *)&(b4))[2] = (s4).uByte2;                \
        ((uint8 *)&(b4))[3] = (s4).uByte3;                \
    } while (0)

#define _CALADAN3_L3_GET_ETE_RESOURCE_TYPE(ete_type, resource_type)  \
    do {                                                             \
       if (((ete_type) == _CALADAN3_L3_ETE__UCAST_IP) ||             \
           ((ete_type) == _CALADAN3_L3_ETE__MCAST_IP) ||             \
           ((ete_type) == _CALADAN3_L3_ETE__ENCAP_IP) ||             \
           ((ete_type) == _CALADAN3_L3_ETE__ENCAP_MPLS)) {           \
           (resource_type) = SBX_CALADAN3_USR_RES_ETE;               \
       } else {                                                      \
            (resource_type)  = SBX_CALADAN3_USR_RES_MAX;             \
       }                                                             \
    } while (0);

#define _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(hinfo, rinfo)   \
    do {                                                       \
        (rinfo)->l3a_flags      =  (hinfo)->l3a_flags;         \
        (rinfo)->l3a_vrf        =  (hinfo)->l3a_vrf;           \
        (rinfo)->l3a_subnet     =  (hinfo)->l3a_ip_addr;       \
        sal_memcpy((rinfo)->l3a_ip6_net, (hinfo)->l3a_ip6_addr,\
                   sizeof(bcm_ip6_t));                         \
        (rinfo)->l3a_ip_mask    =  0xffffffff;                 \
        sal_memset((rinfo)->l3a_ip6_mask, 0xff,                \
                   sizeof(bcm_ip6_t));                         \
        (rinfo)->l3a_intf       =  (hinfo)->l3a_intf;          \
        (rinfo)->l3a_pri        =  (hinfo)->l3a_pri;           \
        (rinfo)->l3a_modid      =  (hinfo)->l3a_modid;         \
        (rinfo)->l3a_port_tgid  = (hinfo)->l3a_port_tgid;      \
    } while (0);


/*
 * recovery restrictions go here - we may not want support all possible
 * resources, maybe we do.   Let's allow a compile time option.
 */
#define L3_RCVR_MAX_IFID    (_SBX_CALADAN3_MAX_VALID_IFID >> 5)

/*
 *   Version 1.0 of the scache layout for l3 level-2 recovery
 *  The version is stored in l3_fe->wb_cache->version
 *  This memory map is applied to l3_fe->wb_cache->cache;
 *  This memory map is used to define the amount of memory allocated for the 
 *    purposes of warmboot - if its not here - it's not stored.
 */
typedef struct caladan3_l3_wb_mem_layout_s {

    /* data required to recover l3 interfaces */
    /* Upgrade:  condense 17b entities; saves (max_ifid * 2 * 14) bits!  */
    struct ifdata {
        uint32  tbd;
    } rif[L3_RCVR_MAX_IFID];

} caladan3_l3_wb_mem_layout_t;


/* total amount of persistent storage requiredf for l3 recovery */
#define L3_WB_SCACHE_SIZE  \
  (sizeof(caladan3_l3_wb_mem_layout_t) + SOC_WB_SCACHE_CONTROL_SIZE)

/* Is Level 2 warmboot configured - */
#define L3_WB_L2_CONFIGURED(l3_fe_) ((l3_fe_)->scache_size != 0)


/**
 * GLOBAL variables
 */
_l3_device_unit_t _l3_units[BCM_MAX_NUM_UNITS];
sal_mutex_t       _l3_mlock[BCM_MAX_NUM_UNITS];

STATIC int
_bcm_caladan3_l3_cmp_uint32(void *a, void *b)
{
    uint32 first;                  /* First compared integer. */
    uint32 second;                 /* Second compared integer. */

    first = *(uint32 *)a;
    second = *(uint32 *)b;

    if (first < second) {
        return (BCM_L3_CMP_LESS);
    } else if (first > second) {
        return (BCM_L3_CMP_GREATER);
    }
    return (BCM_L3_CMP_EQUAL);
}

_caladan3_l3_fe_instance_t *
_bcm_caladan3_get_l3_instance_from_unit(int unit)
{
    if (L3_UNIT_INVALID(unit)) {
        return NULL;
    }
    return (_l3_units[unit].device_pvt_data);
}

STATIC int
_bcm_caladan3_set_l3_instance_for_unit(int unit,
                                     _caladan3_l3_fe_instance_t *fe)
{
    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }
    _l3_units[unit].device_pvt_data = fe;
    return BCM_E_NONE;
}

const char *
_caladan3_ete_type_string_get(_caladan3_l3_ete_types_t type) {
    switch (type)
    {
    case _CALADAN3_L3_ETE__UCAST_IP: return   "_CALADAN3_L3_ETE__UCAST_IP";
    case _CALADAN3_L3_ETE__MCAST_IP: return   "_CALADAN3_L3_ETE__MCAST_IP";
    case _CALADAN3_L3_ETE__ENCAP_IP: return   "_CALADAN3_L3_ETE__ENCAP_IP";
    case _CALADAN3_L3_ETE__ENCAP_MPLS: return "_CALADAN3_L3_ETE__ENCAP_MPLS";
    default:
        return "unknown";
    }
}

STATIC int
_bcm_caladan3_ip_is_valid(bcm_l3_route_t *info)
{
    
    if (info->l3a_flags & BCM_L3_IP6) {
        bcm_ip6_t zero;
        int len = bcm_ip6_mask_length(info->l3a_ip6_mask);

        sal_memset(zero, 0, sizeof(bcm_ip6_t));

        return ((len == 128 || len <= 64)
                && ((BCM_IP6_ADDR_EQ(info->l3a_ip6_mask, zero) != 0) ||
                    (BCM_IP6_ADDR_EQ(info->l3a_ip6_net, zero) == 0)));
    } else  {
        return ((info->l3a_ip_mask != 0) || (info->l3a_subnet == 0));
    }
}


int
_bcm_caladan3_l3_do_lpm_commit(int unit, bcm_l3_route_t *info)
{

    /* For IPv4 we have only LPM and IPv6 maintains
       two seperate tables - LPM and host for subnet
       and host routest 
    */

    if (info->l3a_flags & BCM_L3_IP6) {
        bcm_ip6_t allOnes;
        sal_memset(allOnes, 0xFF, sizeof(bcm_ip6_t));
        if(BCM_IP6_ADDR_EQ(info->l3a_ip6_mask, allOnes)) {
            return ((SOC_SBX_STATE(unit)->cache_l3host == 0) ? TRUE: FALSE);
        } else {
            return ((SOC_SBX_STATE(unit)->cache_l3route == 0) ? TRUE: FALSE);
        }
    } else {
        return ((SOC_SBX_STATE(unit)->cache_l3route == 0) ? TRUE: FALSE);
    } 
}

void
_bcm_caladan3_l3_bcm_intf_qos_t_print(int unit, bcm_l3_intf_qos_t *qos)
{
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "\t flags: 0x%x  map_id: 0x%x "
                            " pri: 0x%x cfi: 0x%x  dscp: 0x%x\n"),
                  qos->flags,
                  qos->qos_map_id,
                  qos->pri,
                  qos->cfi,
                  qos->dscp));
}

void
_bcm_caladan3_l3_caladan3_l3_intf_t_print(int unit,
                                          _caladan3_l3_intf_t *l3_intf,
                                          int printHdr)
{
    if (l3_intf == NULL) {
        return;
    }
    if (printHdr) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "IntfId   ipEteCnt flags    lmacIdx  tunEgLbl "
                  "l3aFlags vrf      vid      iVlan    tunIdx   ttl      mtu"
                  "      oTpid    ifFlags\n")));
    }
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "%8x %8x %8x %8x %8x %8x %8x "
                            "%8x %8x %8x %8x %8x %8x %8x\n"),
                 l3_intf->if_info.l3a_intf_id,
                 l3_intf->if_ip_ete_count,
                 l3_intf->if_flags,
                 l3_intf->if_lmac_idx,
                 l3_intf->tunnel_egr_label,
                 l3_intf->if_info.l3a_flags,
                 l3_intf->if_info.l3a_vrf,
                 l3_intf->if_info.l3a_vid,
                 l3_intf->if_info.l3a_inner_vlan,
                 l3_intf->if_info.l3a_tunnel_idx,
                 l3_intf->if_info.l3a_ttl,
                 l3_intf->if_info.l3a_mtu,
                 l3_intf->if_info.outer_tpid,
                 l3_intf->if_info.l3a_intf_flags));

/*
    if (l3_intf->if_flags & _CALADAN3_L3_MPLS_TUNNEL_SET) {
        L3_VERB((_SBX_D(l3_fe->fe_unit,(bcm_tunnel_initiator_t, 1, *(l3_intf->if_tunnel_info));
    }
*/
}

void
_bcm_caladan3_l3_bcm_l3_intf_t_print(int unit, bcm_l3_intf_t *if_info)
{
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                         "\tl3a_intf_id: 0x%x l3a_flags: 0x%x l3a_vrf  : 0x%x "
                         "l3a_intf_id: 0x%x l3a_vid: 0x%x l3a_inner_vlan: 0x%x "
                         "l3a_tunnel_idx: 0x%x  l3a_ttl: 0x%x  l3a_mtu: 0x%x "
                         "l3a_group: 0x%x l3a_intf_class: 0x%x "
                         "l3a_ip4_options_profile_id: 0x%x outer_tpid: 0x%x "
                         "l3a_intf_flags: 0x%x\n"),
             if_info->l3a_intf_id,  if_info->l3a_flags, if_info->l3a_vrf,
             if_info->l3a_intf_id,  if_info->l3a_vid, if_info->l3a_inner_vlan,
             if_info->l3a_tunnel_idx, if_info->l3a_ttl, if_info->l3a_mtu,
             if_info->l3a_group, if_info->l3a_intf_class,
             if_info->l3a_ip4_options_profile_id, if_info->outer_tpid,
             if_info->l3a_intf_flags));
    
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "\tvlan_qos:")));
    _bcm_caladan3_l3_bcm_intf_qos_t_print(unit, &if_info->vlan_qos);
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "\tinner_vlan_qos:")));
    _bcm_caladan3_l3_bcm_intf_qos_t_print(unit, &if_info->inner_vlan_qos);
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "\tdscp_qos:")));
    _bcm_caladan3_l3_bcm_intf_qos_t_print(unit, &if_info->dscp_qos);
}

void
_bcm_caladan3_l3_caladan3_l3_ete_key_t_print(int unit,
                                             _caladan3_l3_ete_key_t *key)
{
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                           "\tl3_ete_hk[type:0x%x dmac:], l3_ete_ttl: 0x%x\n"),
                 key->l3_ete_hk.type, key->l3_ete_ttl));
}


void
_bcm_caladan3_l3_caladan3_l3_ete_t_print(int                 unit,
                                         _caladan3_l3_ete_t *l3_sw_ete,
                                         int                 printHdr)
{
    int i;
    if (printHdr) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                       "intfId   eteKeyTy eteTtl   OHI      mplsOHI  "
                       "eteHwIdx statIdx  allocUE  inuseUE  ftModid  ftIdx  "
                       "  Ipmcidx  ipmcPort\n")));
    }
    if (l3_sw_ete->l3_inuse_ue) {
        if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP){
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                "%8x %8x %8x %8x %8x %8x %8x %8x %8x %8x %8x "
                                "%8x %8x\n"),
                         l3_sw_ete->l3_intf_id,
                         l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                         l3_sw_ete->l3_ete_key.l3_ete_ttl,
                         l3_sw_ete->l3_ohi.ohi,
                         l3_sw_ete->l3_mpls_ohi.ohi,
                         l3_sw_ete->l3_ete_hw_idx.ete_idx,
                         l3_sw_ete->l3_ete_hw_stat_idx,
                         l3_sw_ete->l3_alloced_ue,
                         l3_sw_ete->l3_inuse_ue,
                         l3_sw_ete->u.l3_fte[0].mod_id,
                         l3_sw_ete->u.l3_fte[0].fte_idx.fte_idx,
                         0, 0));
        } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type ==
                                      _CALADAN3_L3_ETE__MCAST_IP) {
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(unit,
                                 "%8x %8x %8x %8x %8x %8x %8x %8x %8x %8x %8x"
                                 " %8x %8x\n"),
                         l3_sw_ete->l3_intf_id,
                         l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                         l3_sw_ete->l3_ete_key.l3_ete_ttl,
                         l3_sw_ete->l3_ohi.ohi,
                         l3_sw_ete->l3_mpls_ohi.ohi,
                         l3_sw_ete->l3_ete_hw_idx.ete_idx,
                         l3_sw_ete->l3_ete_hw_stat_idx,
                         l3_sw_ete->l3_alloced_ue,
                         l3_sw_ete->l3_inuse_ue,
                         0, 0,
                         l3_sw_ete->u.l3_ipmc[0].ipmc_index,
                         l3_sw_ete->u.l3_ipmc[0].port));
        }
        if (l3_sw_ete->l3_inuse_ue > 1) {
            if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == 
                                       _CALADAN3_L3_ETE__UCAST_IP) {
                for (i=1; i < l3_sw_ete->l3_inuse_ue; i++) {
                    LOG_VERBOSE(BSL_LS_BCM_L3,
                                (BSL_META_U(unit,
                                            "\tl3_fte[0x%x].modid:0x%x "
                                            "fte_idx: 0x%x\n"),
                                 i,
                                 l3_sw_ete->u.l3_fte[i].mod_id,
                                 l3_sw_ete->u.l3_fte[i].fte_idx.fte_idx));
                }
            } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == 
                                        _CALADAN3_L3_ETE__MCAST_IP) {
                for (i=1; i < l3_sw_ete->l3_inuse_ue; i++) {
                    LOG_VERBOSE(BSL_LS_BCM_L3,
                                (BSL_META_U(unit,
                                            "\tl3_ipmc[0x%x].ipmc_idx:0x%x  "
                                            "port:0x%x\n"),
                                 i,
                                 l3_sw_ete->u.l3_ipmc[i].ipmc_index,
                                 l3_sw_ete->u.l3_ipmc[i].port));
                }
            }
        }
    } else {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "%8x %8x %8x %8x %8x %8x %8x %8x %8x\n"),
                     l3_sw_ete->l3_intf_id,
                     l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                     l3_sw_ete->l3_ete_key.l3_ete_ttl,
                     l3_sw_ete->l3_ohi.ohi,
                     l3_sw_ete->l3_mpls_ohi.ohi,
                     l3_sw_ete->l3_ete_hw_idx.ete_idx,
                     l3_sw_ete->l3_ete_hw_stat_idx,
                     l3_sw_ete->l3_alloced_ue,
                     l3_sw_ete->l3_inuse_ue));
    }
}

void
_bcm_caladan3_l3_sw_dump(int unit)
{
#ifdef BROADCOM_DEBUG
    int i, j, k COMPILER_ATTRIBUTE((unused));
    dq_p_t                        l3_intf_elem;
    dq_p_t                        hash_bucket;
    _caladan3_l3_fe_instance_t   *l3_fe      = NULL;
    _caladan3_l3_intf_t          *l3_intf    = NULL;
    _caladan3_l3_ete_t           *l3_sw_ete  = NULL;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (NULL == l3_fe) {
        return;
    }
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "L3 Debug Dump\n")));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "L3 Fe Instance: \n")));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_ipmc_enabled : 0x%x\n"),
                 l3_fe->fe_ipmc_enabled));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_mp_set_size  : 0x%x\n"),
                 l3_fe->fe_mp_set_size));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_ipv4_vrf_bits: 0x%x\n"),
                 l3_fe->fe_ipv4_vrf_bits));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_unit         : 0x%x\n"),
                 l3_fe->fe_unit));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_my_modid     : 0x%x\n"),
                 l3_fe->fe_my_modid));

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tfe_cosq_numcos  : 0x%x\n"),
                 l3_fe->fe_cosq_config_numcos));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tvlan_ft_base    : 0x%x\n"),
                 l3_fe->vlan_ft_base));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tumc_ft_offset   : 0x%x\n"),
                 l3_fe->umc_ft_offset));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tvpws_uni_ft_off : 0x%x\n"),
                 l3_fe->vpws_uni_ft_offset));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tmax_pids        : 0x%x\n"),
                 l3_fe->max_pids));

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tvsi_def_vrf     : 0x%x\n"),
                 l3_fe->fe_vsi_default_vrf));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tdrop_vrf        : 0x%x\n"),
                 l3_fe->fe_drop_vrf));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\traw_ete_idx     : 0x%x\n"),
                 l3_fe->fe_raw_ete_idx));
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\tflags           : 0x%x\n"),
                 l3_fe->fe_flags));


    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "\nL3 Interfaces\n")));
    i = 0;
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "\t\t\tIntfId   ipEteCnt flags    lmacIdx "
             "tunEgLbl l3aFlags vrf      vid      iVlan    tunIdx   ttl      "
             "mtu      oTpid    ifFlags\n")));

    for (j=0; j < _CALADAN3_INTF_ID_HASH_SIZE; j++) {
        hash_bucket = &l3_fe->fe_intf_by_id[j];
        DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
            _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
           _bcm_caladan3_l3_caladan3_l3_intf_t_print(unit, l3_intf, 0);
        } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);
    }

    /* The max count 128K comes from RES_FTE_L3 Resource count */
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                   "\t\t\tIntfId   eteKeyTy eteTtl   OHI      mplsOHI  "
                   "eteHwIdx statIdx  allocUE  inuseUE  ftModid  ftIdx    "
                   "Ipmcidx  ipmcPort\n")));
    for (i=0; i < _CALADAN3_INTF_ID_HASH_SIZE; i++) {
        k = 0;
        hash_bucket = &l3_fe->fe_intf_by_id[i];
        DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
            _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
            for ( j = 0; j < _CALADAN3_INTF_L3ETE_HASH_SIZE; j++) {
                _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, j, l3_sw_ete) {
                    _bcm_caladan3_l3_caladan3_l3_ete_t_print(unit,
                                                             l3_sw_ete, 0);
                } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, j, l3_sw_ete);
            }
        } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);
    }


#endif
}

/*
 * Function:
 *      _bcm_caladan3_get_ete_by_type_on_intf
 * Purpose:
 *      Get the first of given type
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 *      ete_type    - (IN)  the type of ete we are looking for
 *      l3_ete      - (OUT) l3 ete, if found
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_caladan3_get_ete_by_type_on_intf(_caladan3_l3_fe_instance_t  *l3_fe,
                                    _caladan3_l3_intf_t         *l3_intf,
                                    _caladan3_l3_ete_types_t     ete_type,
                                    _caladan3_l3_ete_t         **l3_ete)
{
    int               i;
    _caladan3_l3_ete_t   *tmp_ete;

    *l3_ete = NULL;

    if (ete_type == _CALADAN3_L3_ETE__UCAST_IP) {
       if (l3_intf->if_ip_ete_count > 1) {
          return BCM_E_INTERNAL;
       }
    } else if (ete_type != _CALADAN3_L3_ETE__ENCAP_MPLS) {
        return BCM_E_INTERNAL;
    }

    tmp_ete = NULL;
    for (i = 0; i < _CALADAN3_INTF_L3ETE_HASH_SIZE; i++) {
        if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
            _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, i, tmp_ete) {
                if (tmp_ete->l3_ete_key.l3_ete_hk.type == ete_type) {
                    *l3_ete = tmp_ete;
                    return BCM_E_NONE;
                }
            } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, i, tmp_ete);
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_l3_get_sw_fte
 * Purpose:
 *     Given an fte_idx, return fte_hash elem
 *
 * Parameters:
 *     l3_fe           - (IN)     l3 fe instance
 *     fte_idx         - (IN)     Fte Index in HW
 *     hash_elem       - (OUT)    the FTE hash elem
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an Fte index, use the global fteHash table
 *     to locate the element
 */
int
_bcm_caladan3_l3_get_sw_fte(_caladan3_l3_fe_instance_t      *l3_fe,
                          uint32                      fte_idx,
                          _caladan3_l3_fte_t            **sw_fte)
{
    uint32                           fte_hash_idx;
    dq_p_t                           l3_fte_head, l3_fte_elem;
    _caladan3_l3_fte_t                  *elem;

    *sw_fte      = NULL;
    elem         = NULL;
    fte_hash_idx = _CALADAN3_GET_FTE_HASH_IDX(fte_idx);

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "fte_idx 0x%x, hashIndex %d\n"),
               fte_idx, fte_hash_idx));

    l3_fte_head    = &l3_fe->fe_fteidx2_fte[fte_hash_idx];

    DQ_TRAVERSE(l3_fte_head, l3_fte_elem) {
        _CALADAN3_L3FTE_FROM_FTE_HASH_DQ(l3_fte_elem, elem);

        if (elem->fte_idx == fte_idx) {
            *sw_fte = elem;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_fte_head, l3_fte_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_get_l3_ete_context_by_index
 * Purpose:
 *     Given an ete index, return the sw ete structure
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     ete_idx     - (IN)     Ete Index in HW
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an Ete index, use the global eteidx2ETE hash table
 *     to locate the SW ete context
 */
int
_bcm_caladan3_get_l3_ete_context_by_index(_caladan3_l3_fe_instance_t *l3_fe,
                                        uint32                  ete_idx,
                                        _caladan3_l3_ete_t        **sw_ete)
{
    uint32                           ete_hash_idx;
    dq_p_t                           l3_ete_head, l3_ete_elem;
    _caladan3_l3_ete_t                  *l3_ete = NULL;

    *sw_ete      = NULL;
    ete_hash_idx = _CALADAN3_GET_ETE_HASH_IDX(ete_idx);

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter (Ete Addr 0x%x, hashIndex %d)\n"),
               ete_idx, ete_hash_idx));

    l3_ete_head    = &l3_fe->fe_eteidx2_l3_ete[ete_hash_idx];

    DQ_TRAVERSE(l3_ete_head, l3_ete_elem) {
        _CALADAN3_L3ETE_FROM_L3ETE_HASH_DQ(l3_ete_elem, l3_ete);

        if (l3_ete->l3_ete_hw_idx.ete_idx == ete_idx) {
            *sw_ete = l3_ete;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_ete_head, l3_ete_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *
 * Purpose:
 *     Given an ohi index, return the sw ete structure
 *
 * Parameters:
 *     l3_fe      - (IN)      l3 fe instance
 *     ohi        - (IN)      out header index
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an ohi, use the global ohi2ETE hash table
 *     to locate the SW ete context. This will always
 *     return the the ETE that was created with the egress
 *     object. Even if the HW ohi pointing to a Tunnel, the
 *     SW copy maintains a pointer to the original ETE
 */
int
_bcm_caladan3_l3_sw_ete_find_by_ohi(_caladan3_l3_fe_instance_t *l3_fe,
                                  _caladan3_ohi_t            *ohi,
                                  _caladan3_l3_ete_t        **sw_ete)
{
    uint32           ohi_hash_idx;
    dq_p_t           l3_ohi_head, l3_ete_elem;
    _caladan3_l3_ete_t  *l3_ete = NULL;

    *sw_ete      = NULL;
    ohi_hash_idx = _CALADAN3_GET_OHI2ETE_HASH_IDX(ohi->ohi);

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "(OHI 0x%x, hashIndex %d)\n"),
               ohi->ohi, ohi_hash_idx));

    l3_ohi_head    = &l3_fe->fe_ohi2_l3_ete[ohi_hash_idx];
    DQ_TRAVERSE(l3_ohi_head, l3_ete_elem) {
        _CALADAN3_L3ETE_FROM_L3OHI_HASH_DQ(l3_ete_elem, l3_ete);

        if (l3_ete->l3_ohi.ohi  == ohi->ohi) {
            *sw_ete = l3_ete;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_ohi_head, l3_ete_elem);

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *     _bcm_caladan3_l3_sw_ete_find
 * Purpose:
 *     Given an ete_key, return the sw ete structure
 *
 * Parameters:
 *     unit        - (IN)     the fe unit number
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf    -  (IN)     l3 interface
 *     ete_key     - (IN)     the ete key comprises of
 *                            <ete_type, dmac, vidop, ttlop>
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given the ete key and a l3_intf, find the hash bucket based on
 *     <ete_type + dmac> and then traverse all the ete contexts to find
 *     the the one that matches the key passed in
 */
int
_bcm_caladan3_l3_sw_ete_find(int                     unit,
                           _caladan3_l3_fe_instance_t *l3_fe,
                           _caladan3_l3_intf_t        *l3_intf,
                           _caladan3_l3_ete_key_t     *ete_key,
                           _caladan3_l3_ete_t        **sw_ete)
{
    uint32                            hash_idx;
    _caladan3_l3_ete_t                   *l3_sw_ete;

    *sw_ete   = NULL;
    l3_sw_ete = NULL;
    _CALADAN3_CALC_INTF_L3ETE_HASH(hash_idx,  ete_key->l3_ete_hk.type,
                               ete_key->l3_ete_hk.dmac);
    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "ete hash index %d\n"),
               hash_idx));

    if (DQ_EMPTY(&l3_intf->if_ete_hash[hash_idx])) {
        return BCM_E_NOT_FOUND;
    }

    _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, hash_idx, l3_sw_ete) {
        if (l3_sw_ete &&
            (ete_key->l3_ete_hk.type  == 
             l3_sw_ete->l3_ete_key.l3_ete_hk.type)                           &&
            (ete_key->l3_ete_ttl      == l3_sw_ete->l3_ete_key.l3_ete_ttl)   &&
            (ENET_CMP_MACADDR(ete_key->l3_ete_hk.dmac, 
                              l3_sw_ete->l3_ete_key.l3_ete_hk.dmac) == 0)) {
            LOG_DEBUG(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Found matching ETE at 0x%x\n"),
                       l3_sw_ete->l3_ete_hw_idx.ete_idx));
            *sw_ete = l3_sw_ete;
            return BCM_E_NONE;
        }
    } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, hash_idx, l3_sw_ete);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_with_vidop
 * Purpose:
 *     Find the tunnel ete on the intf that matches the vidop
 *
 * Parameters:
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf     - (IN)     l3 interface
 *     mac         - (IN)     tunnel end point mac
 *     vidop       - (IN)     vidop to match
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
_bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_with_vidop( _caladan3_l3_fe_instance_t *l3_fe,
                                                    _caladan3_l3_intf_t        *l3_intf,
                                                    bcm_mac_t               mac,
                                                    _caladan3_l3_ete_t        **sw_ete)
{
    uint32                            hash_idx;
    _caladan3_l3_ete_t                   *l3_sw_ete;

    *sw_ete   = NULL;
    l3_sw_ete = NULL;
    _CALADAN3_CALC_INTF_L3ETE_HASH(hash_idx,  _CALADAN3_L3_ETE__ENCAP_IP, mac);
    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "ete hash index 0x%x\n"),
               hash_idx));

    if (DQ_EMPTY(&l3_intf->if_ete_hash[hash_idx])) {
        return BCM_E_NOT_FOUND;
    }

    _CALADAN3_ALL_L3ETE_PER_IEH_BKT(l3_intf, hash_idx, l3_sw_ete) {
        if ((_CALADAN3_L3_ETE__ENCAP_IP == l3_sw_ete->l3_ete_key.l3_ete_hk.type) &&
            (ENET_CMP_MACADDR(mac, l3_sw_ete->l3_ete_key.l3_ete_hk.dmac) == 0))
        {
            *sw_ete = l3_sw_ete;
            LOG_DEBUG(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Found matching Tunnel ete at 0x%x\n"),
                       l3_sw_ete->l3_ete_hw_idx.ete_idx));
            return BCM_E_NONE;
        }
    } _CALADAN3_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, hash_idx, l3_sw_ete);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_by_intf
 * Purpose:
 *     Find a tunnel ete on the intf
 *
 * Parameters:
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf     - (IN)     l3 interface
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     For V4 Encap tunnels, we always create two etes.
 *     One ete with vidop = NOP and the other with vidop = ADD.
 *     So either one will suffice
 */
int
_bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_by_intf(_caladan3_l3_fe_instance_t *l3_fe,
                                                _caladan3_l3_intf_t        *l3_intf,
                                                _caladan3_l3_ete_t        **sw_ete)
{

    return (_bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_with_vidop(l3_fe,
                                                               l3_intf,
                                                               l3_intf->if_tunnel_info->dmac,
                                                               sw_ete));
}

/*
 * Function:  _bcm_caladan3_l3_get_egrif_from_fte
 *
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     ul_fte     - (IN)     the fte index
 *     flags      - (IN)     L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY
 *                           L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY
 *     bcm_egr    - (OUT)    the results
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int _bcm_caladan3_l3_get_egrif_from_fte(_caladan3_l3_fe_instance_t *l3_fe,
                                  uint32                  fte_idx,
                                  uint32                  flags,
                                  bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;
    
    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_l3_get_egrif_from_fte(l3_fe, fte_idx,
                                                        flags, bcm_egr);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    return status;
}


/*
 * Function:
 *     _bcm_caladan3_get_local_l3_egress_from_ohi
 * Purpose:
 *     Given an ohi, get bcm l3 egress info
 *
 * Parameters:
 *     l3_fe      - (IN)    l3 fe instance
 *     bcm_egr    - (IN/OUT) bcm_egress object.
 *                           (IN) encap_id
 *                           (OUT)ifid, mac addr, vlan
 * Returns:
 *     BCM_E_XXX
 *
 * Note:
 *     This is a GET function. So we go: FTE -> OHI -> Egress data
 */
int
_bcm_caladan3_get_local_l3_egress_from_ohi(_caladan3_l3_fe_instance_t *l3_fe,
                                         bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_get_local_l3_egress_from_ohi(l3_fe, bcm_egr);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_free_l3_ete
 * Purpose:
 *     free all resources that were allocated by alloc routine
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_sw_ete  - (IN)  SW context for l3 ete
 * Returns:
 *     None
 *
 * Asumption:
 *     This assumes that the SW context data struct is not in
 *     any link list.
 */
STATIC int
_bcm_caladan3_free_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                        _caladan3_l3_ete_t         **sw_ete)
{
    _caladan3_l3_ete_t         *l3_sw_ete;
    uint32                  ete_resource_type;
    int                     status;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    status    = BCM_E_NONE;
    l3_sw_ete = *sw_ete;

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type ==  _CALADAN3_L3_ETE__UCAST_IP) {
        if (l3_sw_ete->u.l3_fte) {
            sal_free(l3_sw_ete->u.l3_fte);
        }
    } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) 
    {
        if (l3_sw_ete->u.l3_ipmc) {
            sal_free(l3_sw_ete->u.l3_ipmc);
        }
    }

    if (_CALADAN3_L3_OHI_DYNAMIC_RANGE(l3_sw_ete->l3_ohi.ohi)) {
        /*
         * This means that the OHI was allocated by us and was not some
         * reserved by user
         */
        status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                        SBX_CALADAN3_USR_RES_OHI,
                                        1,
                                        &l3_sw_ete->l3_ohi.ohi,
                                        0);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not free ohi %d\n"),
                       bcm_errmsg(status), l3_sw_ete->l3_ohi.ohi));
            return status;
        }
    }

    /*
     * Free the L3 ete resource
     */
    if (l3_sw_ete->l3_ete_hw_idx.ete_idx != _CALADAN3_INVALID_ETE_IDX) {
        _CALADAN3_L3_GET_ETE_RESOURCE_TYPE(l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                                       ete_resource_type);
        if (ete_resource_type != SBX_CALADAN3_USR_RES_MAX) {
            status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                            ete_resource_type,
                                            1,
                                            &l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                            0);

            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) could not free L3 ETE %d\n"),
                           bcm_errmsg(status),
                           l3_sw_ete->l3_ete_hw_idx.ete_idx));
                return status;
            }
        } else {
            return (BCM_E_INTERNAL);
        }
    }

    sal_free(*sw_ete);
    *sw_ete = NULL;
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_undo_l3_ete_alloc
 * Purpose:
 *     Remove from all link lists and then
 *     free the HW indices and the SW instance
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface context for the ete
 *     sw_ete     - (IN)  sw ete instance
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     XXX: There is currently no way to invalidate an ETE
 *     We just make sure that not FTEs point to it. Point the
 *     OHI to ete #0.
 */
int
_bcm_caladan3_undo_l3_ete_alloc(_caladan3_l3_fe_instance_t *l3_fe,
                              _caladan3_l3_intf_t        *l3_intf,
                              _caladan3_l3_ete_t         **p_l3_sw_ete)
{
    _caladan3_l3_ete_t    *l3_sw_ete;
    const char        *cptr;
    int                status;

    l3_sw_ete   = *p_l3_sw_ete;
    cptr = _caladan3_ete_type_string_get(l3_sw_ete->l3_ete_key.l3_ete_hk.type);
    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter : Free ete (0x%x) type %s ohi (0x%x)\n"),
               l3_sw_ete->l3_ete_hw_idx.ete_idx,
               cptr, l3_sw_ete->l3_ohi.ohi));

    if (l3_sw_ete->l3_inuse_ue) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Attempted to allocate an in-use ete=0x%x "
                               "type %s ohi=0x%x\n"),
                   l3_sw_ete->l3_ete_hw_idx.ete_idx,
                   cptr, l3_sw_ete->l3_ohi.ohi));
        return BCM_E_INTERNAL;
    }

    DQ_REMOVE(&l3_sw_ete->l3_ete_link);
    DQ_REMOVE(&l3_sw_ete->l3_ieh_link);

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {
        /* keep track of v4 etes on a intf. In case of mpls tunnels on an
         * intf, we can only have one v4-ete
         */
        l3_intf->if_ip_ete_count--;
    }

    if (_CALADAN3_ETE_TYPE_NEEDS_OHI(l3_sw_ete->l3_ete_key.l3_ete_hk.type)) {

        DQ_REMOVE(&l3_sw_ete->l3_ohi_link);
        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case  SOC_SBX_UCODE_TYPE_G3P1:
        {
            soc_sbx_g3p1_oi2e_t  ohi2etc;

            soc_sbx_g3p1_oi2e_t_init(&ohi2etc);
            ohi2etc.eteptr = SOC_SBX_INVALID_L2ETE(l3_fe->fe_unit);
            status = soc_sbx_g3p1_oi2e_set(l3_fe->fe_unit,
                                           l3_sw_ete->l3_ohi.ohi,
                                           &ohi2etc);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error %s in soc_sbx_g3p1_oi2e_set"),
                           bcm_errmsg(status)));
                return status;
            }
            break;
        }
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
            return status;
        }
        
    }

    status = _bcm_caladan3_free_l3_ete(l3_fe, p_l3_sw_ete);

    return status;
}


/*
 * Function:
 *     _bcm_caladan3_alloc_default_l3_ete
 * Purpose:
 *     allocate an l3 ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface
 *     module_fte - (IN)  <mod,fte> which points to this ete
 *     hw_ete     - (OUT) default values in HW ete struct
 *     ohi        - (IN)  ohi
 *     sw_ete     - (OUT) allocate and return
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     Allocate a SW instance and link it to
 *     a) ete
 *     b) eteIdx2ETE hash list
 *     Allocate HW ete_idx, and  a HW ohi (if needed). Finally fill in
 *     default values in the HW ete
 */
int
_bcm_caladan3_alloc_default_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                 _caladan3_l3_intf_t        *l3_intf,
                                 _caladan3_l3_ete_key_t     *l3_ete_key,
                                 uint32                  ohi,
                                 _caladan3_l3_ete_t        **sw_ete)

{
    uint32                           ete_hash_idx;
    uint32                           size, i;
    _sbx_caladan3_usr_res_types_t         ete_resource_type;
    int                              status;
    _caladan3_l3_ete_t                  *l3_sw_ete;
    _caladan3_l3_ete_fte_t              *ete_fte;
    _caladan3_l3_ete_ipmc_t             *ete_ipmc;
    _caladan3_l3_ete_t                  *tmp_l3_ete;
    _caladan3_ohi_t                      tmp_ohi;

    *sw_ete     = NULL;
    ete_fte     = NULL;
    ete_ipmc    = NULL;
    status      = BCM_E_NONE;
    tmp_l3_ete  = NULL;

    /*
     * if user has specified an OHI, it should not be in use
     */
    if (ohi) {
        tmp_ohi.ohi = ohi;
        status = _bcm_caladan3_l3_sw_ete_find_by_ohi(l3_fe,
                                                   &tmp_ohi,
                                                   &tmp_l3_ete);
        if (status == BCM_E_NONE) {
            return BCM_E_EXISTS;
        }
    }

    size        = sizeof(_caladan3_l3_ete_t);
    l3_sw_ete   = (_caladan3_l3_ete_t *) sal_alloc(size, "l3-ete");
    if (l3_sw_ete == NULL) {
        status = BCM_E_MEMORY;
        return status;
    }

    sal_memset((l3_sw_ete), 0, size);

    ENET_COPY_MACADDR(l3_ete_key->l3_ete_hk.dmac,
                      l3_sw_ete->l3_ete_key.l3_ete_hk.dmac);

    l3_sw_ete->l3_ete_key.l3_ete_hk.type  = l3_ete_key->l3_ete_hk.type;
    l3_sw_ete->l3_ete_key.l3_ete_ttl      = l3_ete_key->l3_ete_ttl;
    l3_sw_ete->l3_ohi.ohi                 = _CALADAN3_INVALID_OHI;
    l3_sw_ete->l3_mpls_ohi.ohi            = _CALADAN3_INVALID_OHI;
    l3_sw_ete->l3_ete_hw_idx.ete_idx      = _CALADAN3_INVALID_ETE_IDX;

    if (l3_ete_key->l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {
        size     = (sizeof(_caladan3_l3_ete_fte_t)) * _CALADAN3_ETE_USER_SLAB_SIZE;
        ete_fte  = (_caladan3_l3_ete_fte_t *)
            sal_alloc(size, "Ip-ete-fte");
        if (ete_fte == NULL) {
            _bcm_caladan3_free_l3_ete(l3_fe, &l3_sw_ete);
            status = BCM_E_MEMORY;
            return status;
        }
        sal_memset(ete_fte, 0, size);

        l3_sw_ete->u.l3_fte   = ete_fte;
        /*
         * Initial ETE user size. This is a growable array
         */
        l3_sw_ete->l3_alloced_ue = _CALADAN3_ETE_USER_SLAB_SIZE;
        for (i = 0; i < l3_sw_ete->l3_alloced_ue; i++) {
            ete_fte[i].mod_id = SBX_INVALID_MODID;
        }
    } else if (l3_ete_key->l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) {
        size     = (sizeof(_caladan3_l3_ete_ipmc_t)) * _CALADAN3_ETE_USER_SLAB_SIZE;
        ete_ipmc = (_caladan3_l3_ete_ipmc_t *) sal_alloc(size, "Ip-ete-ipmc");
        if (ete_ipmc == NULL) {
            _bcm_caladan3_free_l3_ete(l3_fe, &l3_sw_ete);
            status = BCM_E_MEMORY;
            return status;
        }
        sal_memset(ete_ipmc, -1, size);
        l3_sw_ete->u.l3_ipmc  = ete_ipmc;
        /*
         * Initial ETE user size. This is a growable array
         */
        l3_sw_ete->l3_alloced_ue = _CALADAN3_ETE_USER_SLAB_SIZE;
    }

    if ((ohi == 0) &&
        (_CALADAN3_ETE_TYPE_NEEDS_OHI(l3_ete_key->l3_ete_hk.type))) {
        status =  _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                          SBX_CALADAN3_USR_RES_OHI,
                                          1,
                                          &l3_sw_ete->l3_ohi.ohi,
                                          0);
        if (status != BCM_E_NONE) {
            /*
             * Make sure that resource allocator is in sync with us
             */
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Could not allocate ohi: %s\n"),
                       bcm_errmsg(status)));
            _bcm_caladan3_free_l3_ete(l3_fe, &l3_sw_ete);
            return BCM_E_RESOURCE;
        }
    } else if (ohi) {
        l3_sw_ete->l3_ohi.ohi = ohi;
    }

    _CALADAN3_L3_GET_ETE_RESOURCE_TYPE(l3_ete_key->l3_ete_hk.type,
                                   ete_resource_type);
    if (ete_resource_type == SBX_CALADAN3_USR_RES_MAX) {
        _bcm_caladan3_free_l3_ete(l3_fe, &l3_sw_ete);
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Unknown ete type %d\n"),
                   l3_ete_key->l3_ete_hk.type));
        return BCM_E_PARAM;
    }

    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     ete_resource_type,
                                     1,
                                     &l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                     0);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not allocate L3 Ete: %s\n"), 
                   bcm_errmsg(status)));
         _bcm_caladan3_free_l3_ete(l3_fe, &l3_sw_ete);
         return status;
    }

    _CALADAN3_CALC_INTF_L3ETE_HASH(ete_hash_idx, l3_ete_key->l3_ete_hk.type,
                               l3_ete_key->l3_ete_hk.dmac);

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Successfully allocated ete index (0x%x) intf_hash (%d)\n"),
               l3_sw_ete->l3_ete_hw_idx.ete_idx, ete_hash_idx));

    /*
     * Per interface ETEs
     */
    DQ_INSERT_HEAD(&l3_intf->if_ete_hash[ete_hash_idx], &l3_sw_ete->l3_ieh_link);
    if (l3_ete_key->l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {
        /* keep track of v4 etes on a intf. In case of mpls tunnels on an
         * intf, we can only have one v4-ete
         */
        l3_intf->if_ip_ete_count++;
    }

    /*
     * Global L3 Ete Index to ETE
     */
    ete_hash_idx = _CALADAN3_GET_ETE_HASH_IDX(l3_sw_ete->l3_ete_hw_idx.ete_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_eteidx2_l3_ete[ete_hash_idx],
                   &l3_sw_ete->l3_ete_link);

    if (l3_sw_ete->l3_ohi.ohi != _CALADAN3_INVALID_OHI) {
        /*
         * Global OHI to ETE
         */
        ete_hash_idx = _CALADAN3_GET_OHI2ETE_HASH_IDX(l3_sw_ete->l3_ohi.ohi);
        DQ_INSERT_HEAD(&l3_fe->fe_ohi2_l3_ete[ete_hash_idx],
                   &l3_sw_ete->l3_ohi_link);
    }

    *sw_ete = l3_sw_ete;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_add_ipv4_ete
 * Purpose:
 *     Create the neccesary state in HW and SW for an ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     ete_key    - (IN)  key to be added
 *     l3_intf    - (IN)  l3 interface
 *     ohi        - (IN)  ohi
 *     port       - (IN)  egress port
 *     flags      - (IN)  bcm_egr flags
 *     sw_ete     - (IN/OUT) updated sw ete
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This function assumes that the ete does not exist
 *     either in HW or SW. In other words the caller has
 *     done a find before. This function returns after
 *     the following have been programmed in HW and the
 *     associated SW state has been updated.
 *     i)   ete
 *     ii) outheader2etc
 *     If there is a Tunnel on the interface, set the ohi to point
 *     to the corressponding tunnel ete
 */
int
_bcm_caladan3_add_ipv4_ete(_caladan3_l3_fe_instance_t *l3_fe,
                         _caladan3_l3_ete_key_t  *ete_key,
                         _caladan3_l3_intf_t     *l3_intf,
                         uint32                  ohi,
                         bcm_port_t              port,
                         uint32                  flags,
                         uint32                  vid,
                         _caladan3_l3_ete_t      **sw_ete)
{
    int                                status;
    _caladan3_l3_ete_t                    *l3_ete, *l3_tnnl_ete;

    soc_sbx_g3p1_ete_t                  g3p1_hw_ete;
    soc_sbx_g3p1_oi2e_t                 g3p1_hw_ohi2etc;

    *sw_ete    = NULL;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter ohi(0x%x)\n"),
               ohi));

    status = _bcm_caladan3_alloc_default_l3_ete(l3_fe,
                                              l3_intf,
                                              ete_key,
                                              ohi,
                                              &l3_ete);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) could not allocate l3 ete\n"),
                   bcm_errmsg(status)));
        return status;
    }

    /*
     * Store the intf id, because given an ETE we can get back to
     * the l3 intf
     */
    l3_ete->l3_intf_id    = l3_intf->if_info.l3a_intf_id;

    /* Fill in ete */
    status = _bcm_caladan3_g3p1_map_l3_ete(l3_fe, l3_ete,
                                           l3_intf,
                                           port, flags, vid, &g3p1_hw_ete,
                                           &g3p1_hw_ohi2etc);
    if (BCM_FAILURE(status)) {
        _bcm_caladan3_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
        return status;
    }

    l3_tnnl_ete = NULL;

    /*
     * If there is a Encap Tunnel, point the OHI
     * to the corressponding tunnel ete (with correct VIDOP).
     *
     * GNATS 15353 (code review comments): If we cannot find
     * the Encap ETE, fail the L3 ETE add. This behavior is
     * debateable, but was agreed upon
     */
    if ((l3_intf->if_tunnel_info) &&
        (l3_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP)) {

        /*
         * If there is a tunnel on the interface find the tunnel ete
         * Mcast pkts are not allowed on tunnels
         */
        status = _bcm_caladan3_l3_sw_find_v4encap_tnnl_ete_with_vidop(l3_fe,
                                                  l3_intf,
                                                  l3_intf->if_tunnel_info->dmac,
                                                  &l3_tnnl_ete);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Could not find matching l3 encap "
                                   "(tunnel) ETE. Failing L3 ete ADD: %s\n"),
                       bcm_errmsg(status)));
            _bcm_caladan3_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
            return status;

        } else {
            g3p1_hw_ohi2etc.eteptr = l3_tnnl_ete->l3_ete_hw_idx.ete_idx;
        }
    }

    /* Write ete to hardware */
    status = _bcm_caladan3_g3p1_set_l3_ete(l3_fe, l3_ete, &g3p1_hw_ete,
                                             &g3p1_hw_ohi2etc);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not set L3 ETE %d"),
                   l3_ete->l3_ete_hw_idx.ete_idx));

        _bcm_caladan3_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
        return status;
    }

    *sw_ete = l3_ete;

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_ete_create
 * Purpose:
 *     Find or create an l3 ete
 *
 * Parameters:
 *     unit         - (IN)  FE unit number
 *     l3_fe        - (IN)  l3 fe instance
 *     l3_intf      - (IN)  l3 interface context
 *     etekey       - (IN)  l3 ete key to be created
 *     ohi          - (IN)  ohi
 *     flags        - (IN)  bcm_egr flags
 *     port         - (IN)  bcm_egr port
 *     l3_ete       - (OUT) l3 ete (either found or added)
 * Returns:
 *     BCM_E_EXIST - found an existing l2encap ete
 *     BCM_E_NONE  - successfully created a new l2encap ete
 *
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. It also create an l2encap ete if one is required.
 */
int
_bcm_caladan3_l3_sw_ete_create(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             _caladan3_l3_intf_t        *l3_intf,
                             _caladan3_l3_ete_key_t     *etekey,
                             uint32                  ohi,
                             uint32                  flags,
                             bcm_port_t              port,
                             uint32                  vid,
                             _caladan3_l3_ete_t        **l3_ete)
{
    int                            status, ignore_status = BCM_E_NONE;
    int                            max_permissible_v4_etes;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    status = _bcm_caladan3_l3_sw_ete_find(unit,
                                        l3_fe,
                                        l3_intf,
                                        etekey,
                                        l3_ete);

    if (status == BCM_E_NONE) {
        return BCM_E_EXISTS;
    }

    /*
     * We are about to add a new ucast v4 ete. However, if
     * there is a mpls tunnel on the intf we cannot have more
     * than one ucast  ete on the intf. This is because we
     * get the dmac from the v4 ete and the mpls tunnel will
     * point to that dmac. Since in the bcm world all etes that
     * are present on the intf need to go over the tunnel, therefore
     * by implication we can only have one v4 ete.
     *
     * There is one transient case however, when there will be two etes.
     * This is when the ete is being replaced. For example if the MAC addr
     * changes, we create a new v4-ete and then destroy the old one.
     * Therefore during this transition we will have two etes
     */

    if (l3_intf->if_flags &  _CALADAN3_L3_MPLS_TUNNEL_SET) {
        max_permissible_v4_etes = (flags & BCM_L3_REPLACE) ? 1 : 0;
        if (l3_intf->if_ip_ete_count > max_permissible_v4_etes) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "mpls tunnel set on interface; "
                                   " Cannot have more than one egress "
                                   " object on the interface")));
            return BCM_E_PARAM;
        }
    }

    status = _bcm_caladan3_add_ipv4_ete(l3_fe,
                                      etekey,
                                      l3_intf,
                                      ohi,
                                      port,
                                      flags,
                                      vid,
                                      l3_ete);
    if (status == BCM_E_NONE) {
        if (l3_intf->if_flags & _CALADAN3_L3_MPLS_TUNNEL_SET) {
            /*
             * we successfully created the first ucast v4 ete.
             * (Note that we checked this above in case of mpls tunnels)
             * Since the flag is set, there must be a mpls ete. Although
             * this has a dummy mac address
             */
            ignore_status = _bcm_caladan3_g3p1_enable_mpls_tunnel(l3_fe,
                                                                  l3_intf,
                                                                  *l3_ete);
            if (ignore_status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Could not enable mpls tunnel on interface 0x%x\n"),
                           _CALADAN3_USER_HANDLE_FROM_IFID((*l3_ete)->l3_intf_id)));
            }
        }
    }
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_create_local_ucast_l3_ete
 * Purpose:
 *     Find or create an l3 ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     bcm_egr    - (IN/OUT)  bcm egress data
 *                        encap_id to contain encoded ohi
 *     module_fte - (IN)  <module,ftidx> module is where the
 *                        fte lives
 * Returns:
 *     BCM_E_EXIST - found an existing l2encap ete
 *     BCM_E_NONE  - successfully created a new l2encap ete
 *
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. It also create an l2encap ete if one is required.
 */
int
_bcm_caladan3_create_local_ucast_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                      bcm_l3_egress_t        *bcm_egr,
                                      _caladan3_l3_ete_fte_t     *module_fte)
{
    int                                status;
    _caladan3_l3_ete_t                    *l3_sw_ete;
    _caladan3_l3_intf_t                   *l3_intf;
    _caladan3_l3_ete_key_t                 tkey;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    l3_sw_ete = NULL;

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                              bcm_egr->intf,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "Could not find interfaceId %d on unit %d"),
                  bcm_egr->intf, l3_fe->fe_unit));
        return status;
    }

    _CALADAN3_MAKE_UCAST_IP_SW_ETE_KEY(&tkey, bcm_egr, l3_intf);

    /*
     * Find an existing ETE and if not found add a new one
     */
    status = _bcm_caladan3_l3_sw_ete_create(l3_fe->fe_unit,
                                          l3_fe,
                                          l3_intf,
                                          &tkey,
                                          SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id),
                                          bcm_egr->flags,
                                          bcm_egr->port,
                                          bcm_egr->vlan,
                                          &l3_sw_ete);
    if ((status == BCM_E_NONE) || (status == BCM_E_EXISTS)) {
        /*
         * add the FTE as one of the users of this ETEG
         */
        bcm_egr->encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(l3_sw_ete->l3_ohi.ohi);

        status = _bcm_caladan3_link_fte2ete(l3_sw_ete, module_fte);
        if ((bcm_egr->flags & BCM_L3_REPLACE) && (status == BCM_E_EXISTS)) {
            status = BCM_E_NONE;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_ete_destroy
 * Purpose:
 *     Destroy the L3 ete if the last user is going away
 *
 * Parameters:
 *     unit       - (IN)  FE unit #
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface
 *     sw_ete     - (IN)  L3 ete to be destroyed
 *
 * Returns:
 *     BCM_E_XXX -
 * NOTE:
 *     Must be called on the Linecard where ete lives
 */
int
_bcm_caladan3_l3_sw_ete_destroy(int                     unit,
                              _caladan3_l3_fe_instance_t *l3_fe,
                              _caladan3_l3_intf_t        *l3_intf,
                              _caladan3_l3_ete_t        **sw_ete)
{
    if ((*sw_ete)->l3_inuse_ue == 0) {
        _bcm_caladan3_undo_l3_ete_alloc(l3_fe,
                                      l3_intf,
                                      sw_ete);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_destroy_local_l3_ete
 * Purpose:
 *     Destroy an ipv4 ete if the last fte is going away
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     module_fte - (IN)  <module,ftidx> module is where the
 *                        fte lives
 * Returns:
 *     BCM_E_XXX -
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. If the FTE handle is the last one that is using the
 *     ete, then the ete is destroyed.
 */
int
_bcm_caladan3_destroy_local_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                 uint32                  ohi,
                                 _caladan3_l3_ete_fte_t     *module_fte)
{
    int                        status;
    _caladan3_l3_ete_t            *l3_sw_ete;
    _caladan3_ohi_t                tmp_ohi;
    _caladan3_l3_intf_t           *l3_intf;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter (Ohi 0x%x, module %d fte 0x%x\n"),
               ohi, module_fte->mod_id,
               module_fte->fte_idx.fte_idx));

    /*
     * This will always give the underlying real ete,
     * even if the ohi in HW points to a tunnel
     */
    tmp_ohi.ohi = ohi;
    status = _bcm_caladan3_l3_sw_ete_find_by_ohi(l3_fe, &tmp_ohi, &l3_sw_ete);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not get SW context for ohi (%d)\n"),
                   ohi));
        return status;
    }

    status = _bcm_caladan3_unlink_fte2ete(l3_sw_ete, module_fte);

    if (BCM_FAILURE(status)) {
        return status;
    }

    /* XXX: See if we can move it in */
    if (l3_sw_ete->l3_inuse_ue == 0) {
        status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                  l3_sw_ete->l3_intf_id,
                                                  &l3_intf);
        if (BCM_SUCCESS(status)) {
            status = _bcm_caladan3_undo_l3_ete_alloc(l3_fe,
                                                   l3_intf,
                                                   &l3_sw_ete);
        } else {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Could not free ete because interface 0x%08X"
                                   "not found\n"),
                       _CALADAN3_IFID_FROM_USER_HANDLE(l3_sw_ete->l3_intf_id)));
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_find_fte_by_modid
 * Purpose:
 *     Given an ete and modid find the corresponding fte
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     fte_module - (IN)     module id for the fte
 *     fte_idx    - (OUT)    fte idx
 * Returns:
 *     BCM_E_XXX
 *
 * Assumption:
 *     Only one FTE from a module can point to an ETE
 */
int
_bcm_caladan3_find_fte_by_modid(_caladan3_l3_fe_instance_t *l3_fe,
                              _caladan3_l3_ete_t *l3_ete,
                              bcm_module_t    fte_module,
                              uint32         *fte_idx)
{
    int                    status;
    int                    i;

    status        = BCM_E_NOT_FOUND;

    if (l3_ete->l3_inuse_ue < 0) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Invalid in-use count (%d) found on ete 0x%x\n"),
                   l3_ete->l3_inuse_ue,
                   l3_ete->l3_ete_hw_idx.ete_idx));
        return BCM_E_INTERNAL;
    }

    if ((l3_ete->u.l3_fte == NULL) ||
        (l3_ete->l3_inuse_ue == 0)) {
        return status;
    }

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if (l3_ete->u.l3_fte[i].mod_id == fte_module) {
            *fte_idx = l3_ete->u.l3_fte[i].fte_idx.fte_idx;
            status = BCM_E_NONE;
            break;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_local_find_l3_ete_fte
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     bcm_egr    - (IN)     Egress object properties to match.
 *                           l3a_intf_id, module, dmac and port
 *     fte_module - (IN)     module of the FTE
 *     ul_fte     - (OUT)    fte Index
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     1. This function is called on the unit where ete is local
 *     2. This function should be callable from an RPC wrapper
 */
int
_bcm_caladan3_local_find_l3_ete_fte(_caladan3_l3_fe_instance_t *l3_fe,
                                  bcm_l3_egress_t        *bcm_egr,
                                  bcm_module_t            fte_module,
                                  uint32                 *fte_idx)
{
    int                       status;
    _caladan3_l3_ete_t           *l3_ete;
    _caladan3_l3_intf_t          *l3_intf;
    _caladan3_l3_ete_key_t        tkey;

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                              bcm_egr->intf,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "Could not find interfaceId %d on unit %d"),
                  bcm_egr->intf, l3_fe->fe_unit));
        return status;
    }

    _CALADAN3_MAKE_UCAST_IP_SW_ETE_KEY(&tkey, bcm_egr, l3_intf);

    status = _bcm_caladan3_l3_sw_ete_find(l3_fe->fe_unit,
                                        l3_fe,
                                        l3_intf,
                                        &tkey,
                                        &l3_ete);

    if (status != BCM_E_NONE) {
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "Could not find l3 ete\n")));
        return status;
    }

    status = _bcm_caladan3_find_fte_by_modid(l3_fe, l3_ete, fte_module, fte_idx);

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_find_fte
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     bcm_egr    - (IN)     Egress object properties to match.
 *                           l3a_intf_id, module, dmac and port
 *     ul_fte     - (OUT)    fte Index
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_caladan3_find_fte(_caladan3_l3_fe_instance_t *l3_fe,
                     bcm_l3_egress_t        *bcm_egr,
                     uint32                 *fte_idx)
{
    int                    status = 0;

    if (bcm_egr->module == l3_fe->fe_my_modid) {
        status = _bcm_caladan3_local_find_l3_ete_fte(l3_fe,
                                                   bcm_egr,
                                                   l3_fe->fe_my_modid,
                                                   fte_idx);
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_alloc_l3_or_mpls_fte
 * Purpose:
 *     Allocate an FTE index on the given unit and
 *     fill in FTE default fields
 *
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     trunk      - (IN)     trunk id
 *     res_type   - (IN)     type of resource
 *     hw_fte     - (OUT)    default values in HW fte struct
 *     fte_idx    - (IN/OUT) IN : if flag == BCM_L3_WITH_ID
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */

int
_bcm_caladan3_alloc_l3_or_mpls_fte(_caladan3_l3_fe_instance_t   *l3_fe,
                                 int                       flags,
                                 bcm_trunk_t               trunk,
                                 _sbx_caladan3_usr_res_types_t  res_type,
                                 uint32                   *fte_idx)
{
    int                         status;
    _caladan3_l3_fte_t             *fte_hash_elem, *tmp_elem;
    uint32                      hash_idx, res_idx;

    status        = BCM_E_NONE;

    if (flags & ~(BCM_L3_WITH_ID | BCM_L3_TGID)) {
        return BCM_E_INTERNAL;
    }

    /* convert the vsi to an fti, for reserve case */
    if (res_type == SBX_CALADAN3_USR_RES_VSI) {
        *fte_idx -= l3_fe->vlan_ft_base;
    }

    fte_hash_elem = sal_alloc(sizeof(_caladan3_l3_fte_t), "FTE-Hash");
    if (fte_hash_elem == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not allocate fte hash elem\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(fte_hash_elem, 0, sizeof(_caladan3_l3_fte_t));


    res_idx = *fte_idx;
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit, res_type, 1, &res_idx,
                                         (flags & BCM_L3_WITH_ID)?_SBX_CALADAN3_RES_FLAGS_RESERVE:0);

    if (status == BCM_E_RESOURCE) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "fte 0x%08x marked as externally managed\n"),
                     res_idx));
        status = BCM_E_NONE;

    } else if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) could not allocate fte index\n"),
                   bcm_errmsg(status)));
        sal_free(fte_hash_elem);
        return status;
    }

    /* convert the vsi to an fti */
    if (res_type == SBX_CALADAN3_USR_RES_VSI) {
        *fte_idx = res_idx + l3_fe->vlan_ft_base;
    } else {
        *fte_idx = res_idx;
    }

    /*
     * As a check, make sure that this fte_idx already does not exist
     */
    status = _bcm_caladan3_l3_get_sw_fte(l3_fe, *fte_idx, &tmp_elem);
    if (status == BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "FTE idx 0x%x already exists in hash\n"),
                   tmp_elem->fte_idx));

        sal_free(fte_hash_elem);
        return BCM_E_EXISTS;
    } else if (status == BCM_E_NOT_FOUND) {
        /* Entry does not exsist  - ok */
        status = BCM_E_NONE;
    }
      

    /* All checks passed; allocate and insert the fte hash element
     */

    fte_hash_elem->ref_count = 1;
    fte_hash_elem->fte_idx   = *fte_idx;
    hash_idx = _CALADAN3_GET_FTE_HASH_IDX(fte_hash_elem->fte_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_fteidx2_fte[hash_idx],
                   &fte_hash_elem->fte_hash_link);

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "allocated fte index(0x%x) res=%s (%d)\n"),
                 fte_hash_elem->fte_idx, _sbx_caladan3_resource_to_str(res_type), 
                 res_type));

    return status;
}


/*
 * Function:
 *     _bcm_caladan3_expand_ete_users
 * Purpose:
 *     Create more space to store info about ete users
 * Parameters:
 *     l3_ete     - (IN)     the SW ete struct
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */
int
_bcm_caladan3_expand_ete_users(_caladan3_l3_ete_t         *l3_ete)
{

    uint32                num_new_entries;
    uint32                size, i;
    _caladan3_l3_ete_fte_t   *fte_info;
    _caladan3_l3_ete_ipmc_t  *ipmc_info;

    num_new_entries   = l3_ete->l3_alloced_ue + _CALADAN3_ETE_USER_SLAB_SIZE;

    if (l3_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__UCAST_IP) {
        size       = (sizeof(_caladan3_l3_ete_fte_t)) * num_new_entries;
        fte_info   = (_caladan3_l3_ete_fte_t *) sal_alloc(size, "Ip-ete-fte");
        if (!fte_info) {
            return BCM_E_MEMORY;
        }

        for (i = 0; i < num_new_entries; i++) {
            if (i < l3_ete->l3_alloced_ue) {
                fte_info[i] = l3_ete->u.l3_fte[i];
            } else {
                fte_info[i].mod_id = SBX_INVALID_MODID;
            }
        }
        sal_free(l3_ete->u.l3_fte);
        l3_ete->u.l3_fte = fte_info;
    } else if (l3_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) {
        size      = (sizeof(_caladan3_l3_ete_ipmc_t)) * num_new_entries;
        ipmc_info = (_caladan3_l3_ete_ipmc_t *) sal_alloc(size, "Ip-ete-ipmc");
        if (!ipmc_info) {
            return BCM_E_MEMORY;
        }
        sal_memset(ipmc_info, 0, size);
        for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
            ipmc_info[i] = l3_ete->u.l3_ipmc[i];
        }
        sal_free(l3_ete->u.l3_ipmc);
        l3_ete->u.l3_ipmc = ipmc_info;
    }
    l3_ete->l3_alloced_ue = num_new_entries;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_unlink_fte2ete
 * Purpose:
 *     Given an fte and the module to which it belongs, update
 *     the ete.
 * Parameters:
 *     l3_ete     - (IN/OUT) l3 ete to which the fte must should point
 *     module     - (IN)     the globally unique module id on which
 *                           the FTE is actually present
 *     fte_idx    - (IN)     FTE index of the FTE on C3 represented by
 *                           module
 * Returns:
 *     BCM_E_XXX
 *
 */
STATIC int
_bcm_caladan3_unlink_fte2ete(_caladan3_l3_ete_t         *l3_ete,
                           _caladan3_l3_ete_fte_t     *module_fte)
{
    uint32                i;

    if ( (module_fte->mod_id ==0) &&
         (module_fte->fte_idx.fte_idx == 0)) {
        return BCM_E_PARAM;
    }

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if ((l3_ete->u.l3_fte[i].mod_id == module_fte->mod_id) &&
            (l3_ete->u.l3_fte[i].fte_idx.fte_idx ==
             module_fte->fte_idx.fte_idx)) {
            l3_ete->u.l3_fte[i].mod_id          = SBX_INVALID_MODID;
            l3_ete->u.l3_fte[i].fte_idx.fte_idx = 0;
            l3_ete->l3_inuse_ue--;
            break;
        }
    }
    if (i >= l3_ete->l3_alloced_ue) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_link_fte2ete
 * Purpose:
 *     Given an fte and the module to which it belongs, update
 *     the ete.
 * Parameters:
 *     l3_ete     - (IN/OUT) l3 ete to which the fte must should point
 *     module     - (IN)     the globally unique module id on which
 *                           the FTE is actually present
 *     fte_idx    - (IN)     FTE index of the FTE on C3 represented by
 *                           module
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     The MODID/FTE pairs are allocated in chunks of
 *     _CALADAN3_ETE_USER_SLAB_SIZE to avoid a re-alloc every time.
 */
STATIC int
_bcm_caladan3_link_fte2ete(_caladan3_l3_ete_t         *l3_ete,
                         _caladan3_l3_ete_fte_t     *module_fte)
{
    _caladan3_l3_ete_fte_t   *fte_info;
    uint32                i;
    int                   status;

    status = BCM_E_NONE;

    /*
     * first make sure that the FTE/MOD pair does not already exist
     */
    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if ((l3_ete->u.l3_fte[i].mod_id == module_fte->mod_id) &&
            (l3_ete->u.l3_fte[i].fte_idx.fte_idx ==
             module_fte->fte_idx.fte_idx)) {
            return BCM_E_EXISTS;
        }
    }

    /*
     * New entry, need to add. Make sure there is space
     */
    if ((l3_ete->l3_inuse_ue > 0) &&
        (l3_ete->l3_inuse_ue == l3_ete->l3_alloced_ue)) {
        status = _bcm_caladan3_expand_ete_users(l3_ete);
    }

    BCM_IF_ERROR_RETURN(status);

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        fte_info = &l3_ete->u.l3_fte[i];
        if ((fte_info->mod_id == SBX_INVALID_MODID) &&
            (fte_info->fte_idx.fte_idx == 0)) {
            *fte_info  = *module_fte;
            l3_ete->l3_inuse_ue += 1;
            break;
        }
    }
    return status;
}


int
_bcm_caladan3_create_ucast_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                uint32                  fte_idx,
                                bcm_l3_egress_t        *bcm_egr)
{
    _caladan3_l3_ete_fte_t    ete_fte;
    int                   status;

    status = BCM_E_NONE;
    ete_fte.fte_idx.fte_idx = fte_idx;

    if ((l3_fe->fe_my_modid == bcm_egr->module) || 
        (bcm_egr->flags & BCM_L3_TGID)) {
        ete_fte.mod_id = l3_fe->fe_my_modid;

        status = _bcm_caladan3_create_local_ucast_l3_ete(l3_fe, bcm_egr, 
                                                       &ete_fte);
    } else {
        /* for Remote Egress objects, ENCAP ID is passed with Egress Interface
         * input, No RPC is issued */
        ete_fte.mod_id = l3_fe->fe_my_modid;
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_map_l3_ucast_fte
 * Purpose:
 *     Translate BCM egress values to fields in FTE
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     bcm_egr    - (IN)     egress object passed by user
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This function is called after the ETE has been created
 *     (either local or remote) OR the user has specified an
 *     encoded OHI in the encap_id field of the egress structure
 */
int
_bcm_caladan3_map_l3_ucast_fte(_caladan3_l3_fe_instance_t *l3_fe,
                             _caladan3_fte_idx_t        *fte_idx,
                             bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_map_set_l3_ucast_fte(l3_fe, fte_idx, 
                                                       bcm_egr);
        break;
#endif
    default: 
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error mapping l3 ucast fte 0x%x\n"),
                   fte_idx->fte_idx));
    } else {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "successfully programmed fte(0x%x) with encap-id(0x%x)\n"),
                     fte_idx->fte_idx, bcm_egr->encap_id));
    }

    return status;
}


STATIC int
_bcm_caladan3_destroy_l3_ete(_caladan3_l3_fe_instance_t   *l3_fe,
                           bcm_module_t              egr_modid,
                           uint32                    ohi,
                           uint32                    fte_idx)
{
    int                   status;
    _caladan3_l3_ete_fte_t    ete_fte;

    status = BCM_E_NONE;
    if (l3_fe->fe_my_modid == egr_modid) {

        ete_fte.mod_id          = egr_modid;
        ete_fte.fte_idx.fte_idx = fte_idx;

        status = _bcm_caladan3_destroy_local_l3_ete(l3_fe, ohi, &ete_fte);
    } else {
        ete_fte.mod_id = l3_fe->fe_my_modid;
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_add_l3_fte
 * Purpose:
 *     add an ipv4 fte
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     bcm_egr    - (IN)     egress object passed by user
 *     fte_idx    - (OUT)    handle that needs to be returned
 *                           to user
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
STATIC int
_bcm_caladan3_add_l3_fte(_caladan3_l3_fe_instance_t *l3_fe,
                       uint32                  flags,
                       bcm_l3_egress_t        *bcm_egr,
                       _caladan3_fte_idx_t        *fte_idx)
{
    int      status, ignore_status;
    uint32   flags_tmp;

    flags_tmp = (flags & BCM_L3_WITH_ID) | (bcm_egr->flags & BCM_L3_TGID);
    status = _bcm_caladan3_alloc_l3_or_mpls_fte(l3_fe,
                                              flags_tmp,
                                              bcm_egr->trunk,
                                              SBX_CALADAN3_USR_RES_FTE_L3,
                                              &fte_idx->fte_idx);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) could not allocate fte\n"),
                   bcm_errmsg(status)));
        return status;
    }

    /*
     * find or create an ete. Then link this FTE to the
     * ETE
     */
    status = _bcm_caladan3_create_ucast_l3_ete(l3_fe, fte_idx->fte_idx, bcm_egr);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) could not create ETE for fte 0x%x\n"),
                   bcm_errmsg(status), fte_idx->fte_idx));
        ignore_status = _bcm_caladan3_destroy_fte(l3_fe,
                                                L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                fte_idx->fte_idx,
                                                0, /* mod */
                                                0  /* ohi */);
        COMPILER_REFERENCE(ignore_status);
        return status;
    }

    status = _bcm_caladan3_map_l3_ucast_fte(l3_fe,
                                          fte_idx,
                                          bcm_egr);

    if (status != BCM_E_NONE) {
        _bcm_caladan3_destroy_fte(l3_fe,
                                L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                fte_idx->fte_idx,
                                bcm_egr->module,
                                SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id));
    }

    return status;
}


/*
 * Function:
 *     _bcm_caladan3_update_l3_fte
 * Purpose:
 *     add an l3 fte
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     bcm_egr    - (IN)     egress object passed by user
 *     intf       - (IN/OUT) fte
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_caladan3_update_l3_fte(_caladan3_l3_fe_instance_t *l3_fe,
                            uint32                  flags,
                            bcm_l3_egress_t        *bcm_egr,
                            _caladan3_fte_idx_t        *fte_data)
{
    bcm_l3_egress_t        old_egr, tmp_egr;
    int                    ignore_status, status;
    int                    new_ete_created;
    uint32                 chg_flags;
    uint32                 ohi;
    uint32                 old_fte_idx;
    uint32                 old_flags;
    _caladan3_fte_idx_t        new_fte;

    /*
     * Note: The fte could change because of port <-> trunk change
     * Therefore the old fte index is saved
     */
    status      = BCM_E_NONE;
    old_fte_idx = fte_data->fte_idx;
    old_flags = bcm_egr->flags;

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "Update fte 0x%x\n"),
                 old_fte_idx));

    /*
     * Get the old egress object (possibly from a remote line card)
     */
    bcm_l3_egress_t_init(&old_egr);
    status = _bcm_caladan3_l3_get_egrif_from_fte( l3_fe,
                                                old_fte_idx,
                                                L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                &old_egr);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) unable to get fte(0x%x)\n"),
                   bcm_errmsg(status), old_fte_idx));
        return BCM_E_NOT_FOUND;
    }

    if (old_fte_idx < SBX_DYNAMIC_FTE_BASE(l3_fe->fe_unit)) {
        /*
         * The FTE should be in the dynamic range
         */
        return (BCM_E_PARAM);
    }

    /*
     * At this point, old_fte_idx contains the FTE index and old_egr
     * contains the old egress info. Find out what changed and take
     * appropriate action. intf, mac_addr, module, port or trunk may
     * have changed. if intf, mac_addr or module changed then add
     * a new ete and delete the old one. If only port/trunk changed then only the
     * fte needs to be locally updated
     */
    chg_flags = 0;

    /*
     * intf is a reqd parameter for REPLACE. While checking
     * we made sure that it was not zero
     */
    if (bcm_egr->intf != old_egr.intf) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "interface changed from 0x%x to 0x%x\n"),
                     (int32)old_egr.intf, (int32)bcm_egr->intf));
        chg_flags |= L3_EGR_INTF_CHG;
    }

    /*
     * if dmac is not specified copy over the old one
     * so that we have a valid dmac if some other params changed
     */
    if (BCM_MAC_IS_ZERO(bcm_egr->mac_addr)) {
        ENET_COPY_MACADDR(old_egr.mac_addr, bcm_egr->mac_addr);
    } else {
        if (ENET_CMP_MACADDR(bcm_egr->mac_addr, old_egr.mac_addr)) {
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "destination mac changed\n")));
            chg_flags |= L3_EGR_DMAC_CHG;
        }
    }

    if ((bcm_egr->module != old_egr.module) && 
        (!(bcm_egr->flags & BCM_L3_TGID))) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "egress module changed from %d to %d\n"),
                     (int)old_egr.module, (int)bcm_egr->module));
        chg_flags |= L3_EGR_MODULE_CHG;
    }

    ohi =  SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id);
    if  (ohi) {
        /*
         * 1. old_egr will have the internal value of the encap_id,
         *    So to compare with user passed encap_id, we need to
         *    comapre with converted encap_id
         * 2. Another thing that needs to be checked is that if ohi
         *    has changed, it shuould not be in use on the module specified
         *    in bcm_egr.
         */
        if (ohi != SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id)) {
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "OHI changed from 0x%x -> 0x%x\n"),
                         SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id), ohi));
            chg_flags |= L3_EGR_OHI_CHG;
        }

        /*
         * encap_id specified. The encap_id itself may or may-not have changed,
         * but the module changed. In that case also need to ensure its availability
         */
        if (chg_flags & (L3_EGR_OHI_CHG | L3_EGR_MODULE_CHG)) {
            bcm_l3_egress_t_init(&tmp_egr);
            tmp_egr.encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(ohi);
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "validating new ohi(0x%x)\n"),
                         ohi));
            if (l3_fe->fe_my_modid == bcm_egr->module) {
                status = _bcm_caladan3_get_local_l3_egress_from_ohi(l3_fe,
                                                                  &tmp_egr);
            }
            if (status == BCM_E_NONE) {
                status = BCM_E_EXISTS;
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "ohi(0x%x) already in use on module %d\n"),
                           ohi, bcm_egr->module));
                return status;
            }
        }
    }

    if (bcm_egr->port != old_egr.port) {
         LOG_VERBOSE(BSL_LS_BCM_L3,
                     (BSL_META_U(l3_fe->fe_unit,
                                 "Egress port changed from %d to %d\n"),
                      old_egr.port, bcm_egr->port));
        chg_flags |= L3_EGR_PORT_CHG;
    }

    /*
     * If port/trunk behavior has changed, we will need to change the
     * FTE as well. This is because Trunk (Lag) ports have special FTEs
     * that are outside the dynamic range
     */
    if ((bcm_egr->flags & BCM_L3_TGID) &&
        (!(old_egr.flags & BCM_L3_TGID))) {
        chg_flags |= L3_EGR_TGID_CHG;
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "change from Port (%d) to Trunk (%d) \n"),
                   (int)old_egr.port, (int)bcm_egr->trunk));
    }

    if ((!(bcm_egr->flags & BCM_L3_TGID)) &&
        (old_egr.flags & BCM_L3_TGID)) {
        chg_flags |= L3_EGR_TGID_CHG;
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "change from Trunk (%d) to Port (%d)\n"),
                   (int)old_egr.trunk, (int)bcm_egr->port));
    }

    if (chg_flags == 0 && (bcm_egr->failover_id == 0)) {
        return BCM_E_NONE;
    }

    new_ete_created = 0;
    if (chg_flags & L3_EGR_TGID_CHG) {
        /*
         * We need a new FTE because we went from port to trunk or
         * vice-versa. Since the FTE changed, we need to unlink the
         * old FTE from the old ETE and relink to the ETE. The ETE
         * may or may not change. Allocate a new FTE and link it to the
         * ete and then remove the old fte
         */
        bcm_egr->flags |= BCM_L3_REPLACE;
        status = _bcm_caladan3_add_l3_fte(l3_fe, flags, bcm_egr, &new_fte);
        if (status == BCM_E_NONE) {
            ignore_status = _bcm_caladan3_destroy_fte(l3_fe,
                                                    L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                    old_fte_idx,
                                                    0, /* module */
                                                    0  /* ohi    */);
            if (ignore_status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) in destroying "
                                       "fte(0x%x) while REPLACING\n"),
                           bcm_errmsg(status), old_fte_idx));
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) in creating new FTE while REPLACING"
                                   " egress object \n"), bcm_errmsg(status)));
            return status;
        }

        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "fte changed to 0x%x\n"),
                     (int)new_fte.fte_idx));
        bcm_egr->flags = old_flags;
        return BCM_E_NONE;
    }

    /*
     * If there is no port <-> trunk change, the FTE remains the same
     */
    new_fte.fte_idx = old_fte_idx;

    if (chg_flags == L3_EGR_DMAC_CHG) {
        /* In case of only DMAC Change, 
         *    i. retain the hw resource
         *   ii. update the eteencap for dmac change
         *  iii. remove the old hash link for the sw ete
         *       add new hash link based on the new dmac
         */
        if ( (l3_fe->fe_my_modid == bcm_egr->module) || 
              (bcm_egr->flags && BCM_L3_TGID)) {
            status = _bcm_caladan3_l3_egress_dmac_update(l3_fe, bcm_egr);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error(%s) in updating dmac "),
                           bcm_errmsg(status)));
                return status;
            }
        }
        return BCM_E_NONE;
    }

    /*
     * Module, port are reqd parameters, so no need
     * to copy from old_bcm_egr
     */
    if (chg_flags &  (L3_EGR_INTF_CHG  |
                      L3_EGR_DMAC_CHG  |
                      L3_EGR_OHI_CHG   |
                      L3_EGR_MODULE_CHG)) {
        /*
         * In this case we also need a new ETE. So find or create the new ete.
         * Then link this FTE to the  ETE.  Since we are creating a ete, it will
         * take care of the mpls ete if there is a mpls tunnel
         */
        bcm_egr->flags |= BCM_L3_REPLACE;
        /* Availablity of this ohi in case of OHI_CHG has been verified 
         * for other cases, we'd be creating a new one
         */
        if (!(chg_flags & L3_EGR_OHI_CHG)) {
            bcm_egr->encap_id = 0;
        }
        status = _bcm_caladan3_create_ucast_l3_ete(l3_fe, new_fte.fte_idx, 
                                                 bcm_egr);

        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not create ete for fte(0x%x)\n"),
                       bcm_errmsg(status), new_fte.fte_idx));
            return status;
        }
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "error(%s) created new ete encap-id(0x%x), "
                                 "fte(0x%x)\n"),
                     _SHR_ERRMSG(status), bcm_egr->encap_id,
                     new_fte.fte_idx));
        new_ete_created = 1;
    } else {
        if (SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id) == 0) {
            /**
             * user did not specify and there was no new ete created
             * then use the old encap_id. Because map function will need
             * ohi
             */
            bcm_egr->encap_id = old_egr.encap_id;
        }
    }

    /*
     * we need to rewrite the old FTE with new info. We need to do this
     * regardless. Ex only the port may have changed or we may have a
     * new FTE in case port <-> tgid change happened
     */
    status = _bcm_caladan3_map_l3_ucast_fte(l3_fe, &new_fte, bcm_egr);

    if (BCM_FAILURE(status)) {
        if (new_ete_created) {
            ignore_status = _bcm_caladan3_destroy_l3_ete(l3_fe,
                                                       bcm_egr->module,
                                                       SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id),
                                                       new_fte.fte_idx);
            if (BCM_FAILURE(ignore_status)) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Could not destroy l3 ete: %s\n"),
                           bcm_errmsg(ignore_status)));
            }
        }
    }

    if (new_ete_created) {
        /*
         * Everything went well... Need to remove the old fte from the old ETE
         * If it was the last FTE on the ETE, then the ETE will be removed too.
         */
        ignore_status = _bcm_caladan3_destroy_l3_ete(l3_fe,
                                                   old_egr.module,
                                                   SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id),
                                                   old_fte_idx);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Error in removing fte 0x%x from ETE: %s\n"),
                       old_fte_idx, bcm_errmsg(ignore_status)));
        } else {
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "Successfully destroyed old ete "
                                     "(FTE 0x%x module %d encap-id(0x%x)\n"),
                         (int)old_fte_idx, (int)old_egr.module,
                         (int) old_egr.encap_id));
        }
        bcm_egr->flags = old_flags;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_l3_egress_dmac_update
 * Purpose:
 *     Update the dmac in l3 egress object
 * Parameters:
 *     l3_fe   - (IN)   l3 fe instance
 *     bcm_egr - (IN)   bcm egress object passed by user
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_caladan3_l3_egress_dmac_update(_caladan3_l3_fe_instance_t *l3_fe,
                                  bcm_l3_egress_t        *bcm_egr)
{
    int                      status = BCM_E_NONE;
    int                      ignore_status = BCM_E_NONE;
    _caladan3_l3_ete_t      *l3_sw_ete;
    soc_sbx_g3p1_ete_t       l3_hw_ete;
    _caladan3_ohi_t          ohi;
    _caladan3_l3_intf_t     *l3_intf;
    int                      ete_hash_idx;

    if ((bcm_egr == NULL) || (l3_fe == NULL)) {
        return BCM_E_PARAM;
    }

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "encap-id(0x%x)\n"),
               bcm_egr->encap_id));

    status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                              bcm_egr->intf,
                                              &l3_intf);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not find intf Id %d on unit %d"),
                   bcm_egr->intf, l3_fe->fe_unit));
        return status;
    }

    ohi.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id);

    status = _bcm_caladan3_l3_sw_ete_find_by_ohi(l3_fe, &ohi, &l3_sw_ete);
    
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error(%s) could not get sw ete from encap-id(0x%x)\n"),
                   bcm_errmsg(status), bcm_egr->encap_id));
        return status;
    }

    status = soc_sbx_g3p1_ete_get(l3_fe->fe_unit,
                                  l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                  &l3_hw_ete);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "unit %d: [%s] error(%s) reading l3-ete(0x%x)\n"),
                   l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status),
                   l3_sw_ete->l3_ete_hw_idx.ete_idx));
        return status;
    }

    _CALADAN3_MAKE_UCAST_IP_SW_ETE_KEY(&l3_sw_ete->l3_ete_key, bcm_egr, l3_intf);

    /* copy the dmac */
    l3_hw_ete.dmac5 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[5];
    l3_hw_ete.dmac4 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[4];
    l3_hw_ete.dmac3 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[3];
    l3_hw_ete.dmac2 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[2];
    l3_hw_ete.dmac1 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[1];
    l3_hw_ete.dmac0 = l3_sw_ete->l3_ete_key.l3_ete_hk.dmac[0];

    status = soc_sbx_g3p1_ete_set(l3_fe->fe_unit,
                                  l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                  &l3_hw_ete);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "unit %d: [%s] error %s in setting L3 ete in HW"),
                   l3_fe->fe_unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    } else {
        LOG_DEBUG(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Updated l3ete(0x%x) with new dmac\n"),
                   l3_sw_ete->l3_ete_hw_idx.ete_idx));
    }

    /* remove ete from the ete hash list */ 
    DQ_REMOVE(&l3_sw_ete->l3_ieh_link);
    /* compute new hash */
    _CALADAN3_CALC_INTF_L3ETE_HASH(ete_hash_idx,
                               l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                               l3_sw_ete->l3_ete_key.l3_ete_hk.dmac);
    /* add the new ete to the list */
    DQ_INSERT_HEAD(&l3_intf->if_ete_hash[ete_hash_idx],
                   &l3_sw_ete->l3_ieh_link);

    if (l3_intf->if_flags & _CALADAN3_L3_MPLS_TUNNEL_SET) {
        /*
         * we successfully updated the  v4 ete.
         * Since the tunnel flag is set, there must be a mpls ete.
         * update mpls ete
         */
        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            ignore_status = _bcm_caladan3_g3p1_enable_mpls_tunnel(l3_fe,
                                                                l3_intf,
                                                                l3_sw_ete);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            return BCM_E_CONFIG;
        }
        
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Could not enable mpls tunnel on interface 0x%x\n"),
                       _CALADAN3_USER_HANDLE_FROM_IFID((l3_sw_ete)->l3_intf_id)));
        }
    }
        
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_invalidate_l3_or_mpls_fte
 * Purpose:
 *     mark the fte as invalid in HW
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     fte_idx    - (IN)     handle that needs to be destroyed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_caladan3_invalidate_l3_or_mpls_fte(_caladan3_l3_fe_instance_t   *l3_fe,
                                      uint32                    fte_idx)
{
    int rv = BCM_E_UNAVAIL;
    
    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l3_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);
        break;
#endif
    default:
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "uCode type %d is not supported\n"),
                  SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype));
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}



/*
 * Function:
 *     _bcm_caladan3_destroy_fte
 * Purpose:
 *     create a
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     action     - (IN)     mode of operation
 *     fte_idx    - (IN)     handle that needs to be destroyed
 *     module_id  - (IN)     module in case of FTE_ONLY
 *     ohi         - (IN)     ohi in case of FTE_ONLY
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
_bcm_caladan3_destroy_fte(_caladan3_l3_fe_instance_t *l3_fe,
                        int                     action,
                        uint32                  fte_idx,
                        bcm_module_t            module_id,
                        uint32                  ohi)
{
    int                   status;
    bcm_l3_egress_t       tmp_egr;
    bcm_module_t          del_modid;
    uint32                del_ohi;


    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "ohi(0x%x) fte(0x%x)\n"),
                 ohi, fte_idx));

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        status = _bcm_caladan3_l3_get_egrif_from_fte(l3_fe,
                                        fte_idx,
                                        L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                        &tmp_egr);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not find fte(0x%x)\n"),
                       bcm_errmsg(status), fte_idx));
            return status;
        }
    }

    /* XXX: TBD: How to handle the MPATH FTEs */
    status = _bcm_caladan3_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);
    if (status != BCM_E_NONE) {
        return status;
    }

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_ONLY) {
        return BCM_E_NONE;
    } else if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        del_modid = tmp_egr.module;
        del_ohi   = SOC_SBX_OHI_FROM_ENCAP_ID(tmp_egr.encap_id);
    } else if (action  == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        del_modid = module_id;
        del_ohi   = ohi;
    } else {
        return BCM_E_INTERNAL;
    }

    status = _bcm_caladan3_destroy_l3_ete(l3_fe, del_modid, del_ohi, fte_idx);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error in destroying ete. FTE 0x%x not "
                               "being invalidated\n"), fte_idx));
        return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_egress_create_checks
 * Purpose:
 *     sanity checks before creating egress object
 * Parameters:
 *     l3_fe   - (IN)  fe instance corresponsing to unit
 *     flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                     BCM_L3_WITH_ID: intf argument is given.
 *     bcm_egr - (IN)  Egress forwarding destination.
 *     intf    - (IN/OUT) fte index interface id corresponding to the
 *                      locally allocated FTE pointing to Egress object.
 * Returns:
 *     BCM_E_XXX
 * Note:
 */
int
_bcm_caladan3_l3_egress_create_checks(_caladan3_l3_fe_instance_t *l3_fe,
                                    uint32                  flags,
                                    bcm_l3_egress_t        *bcm_egr,
                                    bcm_if_t               *intf)
{
    uint32              l3a_intf_id;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    /*
     * Note: The egress (i.e. intf + ete could be on a remote module
     * Therefore we cannot fetch the l3_intf context here
     */

    if ((NULL == bcm_egr) || (NULL == intf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "bcm_egr or intf param is NULL\n")));
        return (BCM_E_PARAM);
    }

    /*
     * In replace case, FTE must be supplied. Therefore the WITH_ID flag
     * must be set. On the other hand in the ADD case, the FTE
     * cannot be specified because we manage the FTE space.  Therefore
     * in case of ADD, WITH_ID flag is invalid
     */
    if (flags & BCM_L3_REPLACE) {
        if (!(flags & BCM_L3_WITH_ID)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "BCM_L3_REPLACE is supported only with BCM_L3_WITH_ID\n")));
             return BCM_E_PARAM;
        }
        if (!_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, *intf)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "L3 egress id is not valid\n")));
            return BCM_E_PARAM;
        }
    }

    /*
     * Interface Id is required for both Add and Replace cases
     * because egress objects are unique per interface
     */
    l3a_intf_id = bcm_egr->intf;
    if (!(_CALADAN3_L3_USER_IFID_VALID(l3a_intf_id))) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "l3a_intf_id param is invalid\n")));
        return (BCM_E_PARAM);
    }

    /*
     * For Add case, a valid dmac is reqd. In case of
     * replace, check for validity only if address is specified
     */
    if (!(flags & BCM_L3_REPLACE)) {
        if ((BCM_MAC_IS_MCAST(bcm_egr->mac_addr)) ||
            (BCM_MAC_IS_ZERO(bcm_egr->mac_addr))) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "mac_addr is invalid\n")));
            return (BCM_E_PARAM);
        }
    } else if ((!BCM_MAC_IS_ZERO(bcm_egr->mac_addr)) &&
               (BCM_MAC_IS_MCAST(bcm_egr->mac_addr))) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mac_addr is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (!(SOC_SBX_MODID_ADDRESSABLE(l3_fe->fe_unit, bcm_egr->module))) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "modid is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (bcm_egr->encap_id &&
        !SOC_SBX_IS_VALID_ENCAP_ID(bcm_egr->encap_id)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "encap id is invalid\n")));
            return BCM_E_PARAM;
    }

    if (bcm_egr->flags & BCM_L3_TGID) {
        if (!(SBX_TRUNK_VALID(bcm_egr->trunk))) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "trunk id is invalid\n")));
            return BCM_E_PARAM;
        }
    } else if ((bcm_egr->module == l3_fe->fe_my_modid) && 
               (!(SOC_PORT_VALID(l3_fe->fe_unit, bcm_egr->port)))) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "port is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (SOC_IS_SBX_G3P1(l3_fe->fe_unit)) {
        if(bcm_egr->flags & BCM_L3_UNTAG) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "BCM_L3_UNTAG not yet supported in G3P1\n")));
            return (BCM_E_PARAM);
        }
    }

    /*
     * If failover id was supplied, check for valid
     * backup egress object
     */
    if (bcm_egr->failover_id &&
        (bcm_egr->failover_id > 0 && bcm_egr->failover_id < 1024)) {
        if (bcm_egr->failover_if_id == 0 ||
            !_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, bcm_egr->failover_if_id)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failover_if_id is invalid\n")));
            return BCM_E_PARAM;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_find_intf_by_ifid
 * Purpose:
 *     lookup l3 interface given a intf id
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     intf_id:  (IN)  a unique 32 bit number
 *     l3_intf:  (OUT) interface sw context
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_caladan3_l3_find_intf_by_ifid(_caladan3_l3_fe_instance_t *l3_fe,
                                 uint32                  intf_id,
                                 _caladan3_l3_intf_t        **ret_l3_intf)
{
    int                             status;
    uint32                          hash_index;
    _caladan3_l3_intf_t                *l3_intf = NULL;
    dq_p_t                          l3_intf_elem;
    dq_p_t                          hash_bucket;

    status      = BCM_E_NOT_FOUND;
    hash_index  = _CALADAN3_GET_INTF_ID_HASH(intf_id);
    hash_bucket = &l3_fe->fe_intf_by_id[hash_index];

    DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
        _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);

        if (l3_intf->if_info.l3a_intf_id == intf_id) {
            *ret_l3_intf = l3_intf;
            status       = BCM_E_NONE;
            break;
        }

    } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_find_intf_by_vid_mac
 * Purpose:
 *     lookup l3 interface given a vid and a macaddr
 * Parameters:
 *     l3_fe:   (IN)  fe instance corresponsing to unit
 *     key  :   (IN)  contains vid and macaddr
 *     l3_intf: (OUT) l3 interface context if found
 * Returns:
 *     BCM_E_XXX
 * Note:
 *    This will return the first matching instance in case there are
 *    multiple interfaces with the same vid and macaddr.
 */
int
_bcm_caladan3_l3_find_intf_by_vid_mac(_caladan3_l3_fe_instance_t *l3_fe,
                                    bcm_l3_intf_t          *key,
                                    _caladan3_l3_intf_t        **ret_l3_intf)
{
    int                           status;
    uint32                        hash_index;
    _caladan3_l3_intf_t              *l3_intf = NULL;
    dq_p_t                        l3_intf_elem, hash_bucket;

    status       = BCM_E_NOT_FOUND;
    *ret_l3_intf = NULL;
    hash_index   =  _CALADAN3_GET_INTF_VID_HASH(key->l3a_vid);
    hash_bucket  = &l3_fe->fe_intf_by_vid[hash_index];

    DQ_TRAVERSE(hash_bucket, l3_intf_elem) {

        _CALADAN3_L3INTF_FROM_VID_DQ(l3_intf_elem, l3_intf);

        if ((l3_intf->if_info.l3a_vid == key->l3a_vid) &&
            (!(ENET_CMP_MACADDR(l3_intf->if_info.l3a_mac_addr,
                                key->l3a_mac_addr)))) {
            status       = BCM_E_NONE;
            *ret_l3_intf = l3_intf;
            return status;
        }

    } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_find_intf_by_egr_label
 * Purpose:
 *     lookup l3 interface given a mpls egr_label
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     egr_label:  (IN)  mpls egr label
 *     l3_intf:  (OUT) interface sw context
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_caladan3_l3_find_intf_by_egr_label(_caladan3_l3_fe_instance_t *l3_fe,
                                      bcm_mpls_label_t       egr_label,
                                     _caladan3_l3_intf_t       **ret_l3_intf)
{
    int                             status;
    uint32                          hash_index;
    _caladan3_l3_intf_t                *l3_intf = NULL;
    dq_p_t                          l3_intf_elem;
    dq_p_t                          hash_bucket;

    status      = BCM_E_NOT_FOUND;
    hash_index  = _CALADAN3_GET_INTF_EGR_LBL_HASH(egr_label);
    hash_bucket = &l3_fe->fe_intf_by_egr_lbl[hash_index];

    DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
        _CALADAN3_L3INTF_FROM_EGR_LBL_DQ(l3_intf_elem, l3_intf);

        if (l3_intf->tunnel_egr_label == egr_label) {
            *ret_l3_intf = l3_intf;
            status       = BCM_E_NONE;
            break;
        }

    } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);

    return status;
}


/*
 * Function:
 *     _bcm_caladan3_l3_intf_locate
 * Purpose:
 *     Find the L3 intf based on  (MAC, VLAN) or InterfaceId
 * Parameters:
 *     unit   - (IN) FE2000 unit number
 *     intf   - (IN) interface (MAC, VLAN) intf number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_caladan3_l3_intf_locate(int                  unit,
                           bcm_l3_intf_t       *bcm_intf,
                           uint32               op)
{
    _caladan3_l3_intf_t            *l3_intf;
    _caladan3_l3_fe_instance_t     *l3_fe;
    int                         status;

    if (NULL == bcm_intf) {
        return (BCM_E_PARAM);
    }

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       return  BCM_E_UNIT;
    }
    status = (op == L3_INTF_LOCATE_BY_VID) ?
        _bcm_caladan3_l3_find_intf_by_vid_mac(l3_fe,
                                            bcm_intf,
                                            &l3_intf) :
        _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                         bcm_intf->l3a_intf_id,
                                         &l3_intf);
    if (status == BCM_E_NONE) {
        *bcm_intf = l3_intf->if_info;
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_free_sw_intf
 * Purpose:
 *     Free internal software information for an l3 interface
 * Parameters:
 *     l3_fe      - (IN) fe instance
 *     IN l3_intf - (IN) l3 interface instance
 * Returns:
 *     NONE
 * Note:
 *     This function cleans up state created by
 *      _bcm_caladan3_l3_free_sw_intf.
 * Assumptions:
 *     The caller must ensure that the follwing conditions are met.
 *     This routine blindly cleans up state.
 *     1. The corresponding Smacs should have been removed
 *        prior to calling this
 *     2. It is safe to remove mcast ete and tunnel ete (if they exist)
 *     3. There are no etes hanging.
 *     4. Lock is held by caller
 */
STATIC int
_bcm_caladan3_l3_free_sw_intf(_caladan3_l3_fe_instance_t *l3_fe,
                            _caladan3_l3_intf_t        *l3_intf)
{
    int    status;

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "vrf %d Interface Id %d \n"),
                 l3_intf->if_info.l3a_vrf,
                 l3_intf->if_info.l3a_intf_id));

    if ((l3_intf->if_flags & _CALADAN3_L3_INTF_SMAC_IN_TBL)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "SMAC not removed before interface freed.\n")));
        return BCM_E_INTERNAL;
    }

    if (l3_intf->if_flags & _CALADAN3_L3_INTF_IN_IFID_LIST) {
        DQ_REMOVE(&l3_intf->if_ifid_link);
    }

    if (l3_intf->if_flags & _CALADAN3_L3_INTF_IN_VID_LIST) {
        DQ_REMOVE(&l3_intf->if_vid_link);
    }
    status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                    SBX_CALADAN3_USR_RES_IFID,
                                    1,
                                    (uint32 *)&l3_intf->if_info.l3a_intf_id,
                                    0);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not free IfId %d\n"), 
                   l3_intf->if_info.l3a_intf_id));
    }

    sal_free(l3_intf);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_uninit
 * Purpose:
 *     Free internal software information in l3 module.
 * Parameters:
 *     unit - (IN) bcm Device number
 * Returns:
 *     NONE
 * NOTE:
 *     Assumes valid unit number.
 */
int
_bcm_caladan3_l3_sw_uninit(int unit)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     ignore_status;
    uint32                  vrf_id;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "software context not found\n")));
        return BCM_E_INTERNAL;
    }

    /**
     * Whether we had allocated or not, try to
     * free the resource on this unit. The goal
     * is to have a clean state after this call.
     */
    vrf_id = BCM_L3_VRF_DEFAULT;
    ignore_status = _sbx_caladan3_resource_free(unit,
                       SBX_CALADAN3_USR_RES_VRF,
                       1,
                       &vrf_id,
                       0);

    if (l3_fe->fe_raw_ete_idx != _CALADAN3_INVALID_ETE_IDX) {
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_ETE,
                                               1,
                                               &l3_fe->fe_raw_ete_idx,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) resource free raw-ete(0x%x)\n"),
                       bcm_errmsg(ignore_status), l3_fe->fe_raw_ete_idx));
        }
    }

    if (l3_fe->fe_drop_vrf != _CALADAN3_INVALID_VRF) {
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_VRF,
                                               1,
                                               &l3_fe->fe_drop_vrf,
                                               0);
       if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) resource free drop vrf(0x%x)\n"),
                       bcm_errmsg(ignore_status), l3_fe->fe_drop_vrf));
        }
    }

    if (l3_fe->fe_vsi_default_vrf != _CALADAN3_INVALID_VRF) {
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_VSI,
                                               1,
                                               &l3_fe->fe_vsi_default_vrf,
                                               0);
       if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) resource free vsi default vrf(0x%x)\n"),
                       bcm_errmsg(ignore_status), l3_fe->fe_vsi_default_vrf));
        }
    }

    if (_l3_mlock[unit] != NULL) {
        L3_UNLOCK(unit);
        sal_mutex_destroy(_l3_mlock[unit]);
        _l3_mlock[unit] = NULL;
    }

    /* This memory may be allocated by the MPLS module */
    if (l3_fe->fe_vpn_by_vrf[BCM_L3_VRF_DEFAULT] != NULL) {
        sal_free(l3_fe->fe_vpn_by_vrf[BCM_L3_VRF_DEFAULT]);
    }

    sal_free(l3_fe);
    l3_fe = NULL;
    _bcm_caladan3_set_l3_instance_for_unit(unit, NULL);

    return BCM_E_NONE;
}




int
_bcm_caladan3_l3_hw_init(int unit, _caladan3_l3_fe_instance_t *l3_fe)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l3_hw_init(unit, l3_fe);
        break;
#endif
    default:
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "uCode type %d is not supported\n"),
                  SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype));
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *     bcm_caladan3_l3_cleanup
 * Purpose:
 *     Cleanup the L3 and bring it
 *     back to state before l3_init
 * Parameters:
 *     unit  - (IN) C3 unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_caladan3_l3_cleanup(int unit)
{
    _caladan3_l3_fe_instance_t     *l3_fe;
    int                         rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "bcm_l3_sw_uninit: "
                               "software context for unit %d not found"), unit));
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* XXX: TBD:
     * make sure no resources are being used
     */

    rv = _bcm_caladan3_l3_sw_uninit(unit);

    if (rv != BCM_E_NONE) {
        /* lock freed in uninit unless rv fails */
        L3_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_init
 * Purpose:
 *     Initialize internal software information in l3 module.
 * Parameters:
 *     unit - (IN) Device number
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Failed to allocate required lock or memory
 * NOTE:
 *     Assumes valid unit number.
 */
STATIC int
_bcm_caladan3_l3_sw_init(int unit)
{
    uint32                     ii, size;
    int                        status;
    uint32                     vrf_id;
    _caladan3_l3_fe_instance_t    *l3_fe;
    bcm_module_t               my_modid;
    uint32                     ignore;

    if (L3_LOCK_CREATED_ALREADY(unit)) {
        status = bcm_caladan3_l3_cleanup(unit);
        if (BCM_FAILURE(status)) {
            return status;
        }
    }

    /**
     * XXX: bcm_l3_init() needs to be called from thread safe
     * user code. Once init() is successful, other calls
     * have the lock to make sure access is ordered.
     */
    if (!(L3_LOCK_CREATED_ALREADY(unit))) {
        if ((_l3_mlock[unit] = sal_mutex_create("bcm_l3_lock")) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    l3_fe     = NULL;
    status    = BCM_E_NONE;

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe) {
        return BCM_E_EXISTS;
    }

    BCM_IF_ERROR_RETURN(
        bcm_stk_my_modid_get(unit,
                             &my_modid));

    size  = sizeof(_caladan3_l3_fe_instance_t);
    l3_fe = sal_alloc(size, (char*)FUNCTION_NAME());
    if (l3_fe == NULL) {
        return (BCM_E_MEMORY);
    }

    sal_memset(l3_fe, 0, size);
    l3_fe->fe_vsi_default_vrf = _CALADAN3_INVALID_VPN_ID;
    l3_fe->fe_drop_vrf        = _CALADAN3_INVALID_VRF;
    l3_fe->fe_raw_ete_idx     = _CALADAN3_INVALID_ETE_IDX;

    _bcm_caladan3_set_l3_instance_for_unit(unit, l3_fe);
    l3_fe->fe_unit     = unit;
    l3_fe->fe_my_modid = my_modid;

    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_vlan_ft_base_get(l3_fe->fe_unit, 
                                       &l3_fe->vlan_ft_base));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_mc_ft_offset_get(l3_fe->fe_unit, 
                                       &l3_fe->umc_ft_offset));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_vpws_ft_offset_get(l3_fe->fe_unit, 
                                         &l3_fe->vpws_uni_ft_offset));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_max_pids_get(l3_fe->fe_unit, 
                                   &l3_fe->max_pids));

#ifdef BCM_WARM_BOOT_SUPPORT
    BCM_IF_ERROR_RETURN
        (_sbx_caladan3_alloc_range_get(l3_fe->fe_unit, SBX_CALADAN3_USR_RES_LINE_VSI,
                                  &l3_fe->line_vsi_base, &ignore));
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* validate driver */
    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        _bcm_caladan3_l3_sw_uninit(unit);
        return BCM_E_UNIT;
    }

    for (ii = 0; ii < _CALADAN3_INTF_ID_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_intf_by_id[ii]);
    }

    for (ii = 0; ii < _CALADAN3_INTF_VID_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_intf_by_vid[ii]);
    }

    for (ii = 0; ii < _CALADAN3_INTF_EGR_LBL_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_intf_by_egr_lbl[ii]);
    }

    for (ii = 0; ii <   _CALADAN3_IPMC_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ipmc_by_id[ii]);
    }

    for (ii=0; ii < _CALADAN3_EGR_MP_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_mp_by_egr[ii]);
        DQ_INIT(&l3_fe->fe_mp_by_mpbase[ii]);
    }

    for (ii=0; ii < _CALADAN3_ETE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_eteidx2_l3_ete[ii]);
    }

    for (ii=0; ii < _CALADAN3_FTE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_fteidx2_fte[ii]);
    }

    for (ii=0; ii < _CALADAN3_OHI_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ohi2_l3_ete[ii]);
    }

    for (ii=0; ii < _CALADAN3_OHI_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ohi2_vc_ete[ii]);
    }

    for (ii=0; ii < _CALADAN3_FTE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_fteidx2_fte[ii]);
    }

    l3_fe->fe_mp_set_size    = _CALADAN3_L3_ECMP_DEFAULT;
    l3_fe->fe_flags         &= ~( _CALADAN3_L3_FE_FLG_MP_SIZE_SET);

    status = bcm_cosq_config_get(l3_fe->fe_unit,
                                 &l3_fe->fe_cosq_config_numcos);
    if (status != BCM_E_NONE) {
        _bcm_caladan3_l3_sw_uninit(unit);
        return status;
    }

    /***
     * Make all our resource allocations upfront, then go into the specialized
     * table writes based on ucode version
     */
    if (!SOC_WARM_BOOT(unit)) {
        l3_fe->fe_raw_ete_idx = _CALADAN3_INVALID_ETE_IDX;
        status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                              SBX_CALADAN3_USR_RES_ETE,
                                              1,
                                              &l3_fe->fe_raw_ete_idx,
                                              0);
    }

    if (status != BCM_E_NONE) {
      _bcm_caladan3_l3_sw_uninit(unit);
      return status;
    }

    /**
     * Allocate the default VRF-ID. This needs to be programmed
     * into the RoutedVlan2Etc in vlan.c [based on the control
     * value passed by the user to bcm_vlan_control_vlan_set()]
     *
     * Do not touch vpn structures here , mpls_init will overwrite.
     *
     */
    vrf_id = BCM_L3_VRF_DEFAULT;
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_VRF,
                                     1,
                                     &vrf_id,
                                     _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (status != BCM_E_NONE) {
        _bcm_caladan3_l3_sw_uninit(unit);
        return status;
    }

#ifdef L3_VPN_SUPPORT
    l3_fe->fe_drop_vrf = _CALADAN3_L3_VRF_DROP;
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_VRF,
                                     1,
                                     &l3_fe->fe_drop_vrf,
                                     _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (status != BCM_E_NONE) {
      _bcm_caladan3_l3_sw_uninit(unit);
      return status;
    }
#endif

    /* reserve the default VSI for VRF to simplify warm boot */
    status = _sbx_caladan3_alloc_range_get(l3_fe->fe_unit, SBX_CALADAN3_USR_RES_VSI,
                                      &l3_fe->fe_vsi_default_vrf, &ignore);

    if (status != BCM_E_NONE) {
        _bcm_caladan3_l3_sw_uninit(unit);
        return status;
    }

    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_VSI,
                                     1,
                                     &l3_fe->fe_vsi_default_vrf,
                                     _SBX_CALADAN3_RES_FLAGS_RESERVE);

    if (status != BCM_E_NONE) {
      _bcm_caladan3_l3_sw_uninit(unit);
      return status;
    }

    status = 
        soc_sbx_g3p1_ip_vrf_bits_get(unit,
                                       (uint32 *) &l3_fe->fe_ipv4_vrf_bits);
    if (BCM_FAILURE(status)) {
      _bcm_caladan3_l3_sw_uninit(unit);
      return status;
    }
    
#ifdef BCM_WARM_BOOT_SUPPORT
    status = bcm_caladan3_wb_l3_state_init(unit);
	COMPILER_REFERENCE(status);

    if (SOC_WARM_BOOT(unit)) {
        if (BCM_SUCCESS(status)) {
            status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                                  SBX_CALADAN3_USR_RES_ETE,
                                                  1,
                                                  &l3_fe->fe_raw_ete_idx,
                                                  _SBX_CALADAN3_RES_FLAGS_RESERVE);
        }
        return status;        
    }

#endif /* BCM_WARM_BOOT_SUPPORT */ 

    status = _bcm_caladan3_l3_hw_init(unit, l3_fe);
    if (status != BCM_E_NONE) {
      _bcm_caladan3_l3_sw_uninit(unit);
      return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_intf_create_replace
 * Purpose:
 *     Create or replace l3 intf
 * Parameters:
 *     unit      - (IN)  C3 unit number
 *     l3_fe     - (IN)  l3 fe instance context
 *     bcm_intf  - (IN)  user (bcm) interface data
 *     ret_intf  - (OUT) L3 internal interface structure info
 *
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */

int
_bcm_caladan3_l3_intf_create_replace(int                     unit,
                                   _caladan3_l3_fe_instance_t *l3_fe,
                                   bcm_l3_intf_t          *bcm_intf,
                                   _caladan3_l3_intf_t        **ret_intf)
{
    _caladan3_l3_intf_t            *l3_intf;
    int                         status;

    status = BCM_E_NONE;

    if (!(bcm_intf->l3a_flags & BCM_L3_REPLACE)) {

        /*
         * alloc a new sw context and copy over user data
         */
        status = _bcm_caladan3_l3_alloc_intf(l3_fe, bcm_intf, NULL, &l3_intf);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error in allocating interface: %s\n"),
                       bcm_errmsg(status)));
            return status;
        }

        status = _bcm_caladan3_l3_add_intf(l3_fe, bcm_intf, l3_intf);
        if (status != BCM_E_NONE) {
            _bcm_caladan3_l3_free_sw_intf(l3_fe, l3_intf);
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error in adding interface: %s\n"), 
                       bcm_errmsg(status)));
            l3_intf = NULL;
            return status;
        }
    } else {
        if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
            status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                      bcm_intf->l3a_intf_id,
                                                      &l3_intf);
        } else {
            status = _bcm_caladan3_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "could not find Interface: %s\n"),
                       bcm_errmsg(status)));
            return status;
        }

        status = _bcm_caladan3_l3_update_intf(l3_fe,
                                            bcm_intf,
                                            l3_intf);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error in updating interface: %s\n"),
                       bcm_errmsg(status)));
            return status;
        }
    }

    if (status == BCM_E_NONE) {
        *ret_intf = l3_intf;
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_sw_intf_create_checks
 * Purpose:
 *     Consistency checks before a interface create is allowed
 * Parameters:
 *     l3_fe     - (IN) l3 fe instance context
 *     bcm_intf  - (IN) user (bcm) interface data
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Only the following L3 flags are supported for interfaces
 *     - BCM_L3_UNTAG,
 *     - BCM_L3_ADD_TO_ARL
 *     - BCM_L3_WITH_ID
 *     - BCM_L3_REPLACE
 *     - BCM_L3_RPE
 */
int
_bcm_caladan3_l3_sw_intf_create_checks(_caladan3_l3_fe_instance_t *l3_fe,
                                     bcm_l3_intf_t          *bcm_intf)
{
    uint32                i, ifid;
    int                   status;
    _caladan3_l3_intf_t      *l3_intf;

    l3_intf  = NULL;

    if (bcm_intf->l3a_flags & ~(L3_INTF_SUPPORTED_FLAGS)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "unsupported flags 0x%x"),
                   bcm_intf->l3a_flags));
        return BCM_E_PARAM;
    }

    if (!_BCM_VRF_VALID(bcm_intf->l3a_vrf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid vrf(0x%x)\n"),
                   bcm_intf->l3a_vrf));
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[bcm_intf->l3a_vrf] == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "vrf(0x%x) not yet created\n"),
                   bcm_intf->l3a_vrf));
        return BCM_E_PARAM;
    }


    /*
     * REPLACE may not come WITH_ID because user can specify vid+mac
     */
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        if (!_CALADAN3_L3_USER_IFID_VALID(bcm_intf->l3a_intf_id)) {
            return BCM_E_PARAM;
        }
    }

    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);

    if ( bcm_intf->l3a_vid && !BCM_VLAN_VALID(bcm_intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    /*
     *  in case of add, if ttl is not specified we will assign a default
     */
    if (bcm_intf->l3a_flags & BCM_L3_REPLACE) {

        if (!BCM_MAC_IS_ZERO(bcm_intf->l3a_mac_addr)) {
            if (BCM_MAC_IS_MCAST(bcm_intf->l3a_mac_addr)) {
                return (BCM_E_PARAM);
            }
        }

        if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
            status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                      ifid,
                                                      &l3_intf);
        } else {
            status = _bcm_caladan3_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Interface not found: %s\n"),
                       bcm_errmsg(status)));
            return BCM_E_NOT_FOUND;
        }
    } else {
        if ((BCM_MAC_IS_MCAST(bcm_intf->l3a_mac_addr)) ||
            (BCM_MAC_IS_ZERO(bcm_intf->l3a_mac_addr))) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "MAC is Multicast or 0\n")));
            return (BCM_E_PARAM);
        }

        if (bcm_intf->l3a_flags & (BCM_L3_WITH_ID)) {
            /*
             * Create an interface with specified Id.
             * Make sure it does not exist
             */
            status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                      ifid,
                                                      &l3_intf);
            if (status == BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "Specified interface (Ifid) exists\n")));
                return BCM_E_EXISTS;
            }
        }
    }

    /*
     * A change in vrf is not allowed if there are ETEs hanging off this
     * interface. We cannot destroy the underlying ETEs because the user
     * will be holding references
     */
    if ((bcm_intf->l3a_flags & BCM_L3_REPLACE) &&
        (l3_intf != NULL && bcm_intf->l3a_vrf != l3_intf->if_info.l3a_vrf)) {
        for (i = 0; i < _CALADAN3_INTF_L3ETE_HASH_SIZE; i++) {
            if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
                return BCM_E_BUSY;
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_update_smac_table
 * Purpose:
 *     Update the software state and update the following tables
 *     in HW
 *     1. Local Station Match Table
 *     2. SmacIdx2Smac
 *
 * Parameters:
 *     l3_fe      - (IN) l3 fe instance
 *     intf       - (IN) user (bcm) mac address
 *     op         - (IN) ADD/DEL
 *     ul_idx     - (IN/OUT)index into local SMAC table
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     1. Lock is held by caller.
 */
int
_bcm_caladan3_l3_update_smac_table(_caladan3_l3_fe_instance_t *l3_fe,
                                  bcm_mac_t               smac,
                                  int                     op,
                                  uint32                 *ul_smac_idx)
{
    int rv = BCM_E_PARAM;

    if (op == L3_INTF_DEL_SMAC) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "freeing lsmi=%d\n"),
                     *ul_smac_idx));

        rv = _sbx_caladan3_ismac_idx_free(l3_fe->fe_unit, smac, _SBX_CALADAN3_RES_UNUSED_PORT, ul_smac_idx);
        if (rv == BCM_E_EMPTY) {
            rv = BCM_E_NONE;
        }
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error freeing idx %d, MAC addr(" MAC_FMT
                                   "): %d %s\n"),
                       *ul_smac_idx, MAC_PFMT(smac), rv, bcm_errmsg(rv)));
        }

    } else if (op == L3_INTF_ADD_SMAC) {
        int flags = 0;

        if (SOC_WARM_BOOT(l3_fe->fe_unit)) {
            flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
        }

        rv = _sbx_caladan3_ismac_idx_alloc(l3_fe->fe_unit, flags, smac, _SBX_CALADAN3_RES_UNUSED_PORT, ul_smac_idx);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failed to allocate ismac idx: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
        }

        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "%s lsmi=%d rv=%d\n"),
                     ((flags & _SBX_CALADAN3_RES_FLAGS_RESERVE) ? 
                     "reserved" : "allocated"), *ul_smac_idx, rv));

        rv = _bcm_caladan3_g3p1_l3_update_smac_table(l3_fe, smac, op,
                                                    *ul_smac_idx);

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "failed to update smac tables: %d %s\n"),
                       rv, bcm_errmsg(rv)));
        }
    }

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_l3_alloc_intf
 * Purpose:
 *     Initialize internal software information in l3 interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (OUT) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     1. Assumes valid vid
 *     2. Is called after it is ascertained that a new instance
 *        needs to be allocated. For xeample if the user has specified
 *        an ifid (and REPLACE flag is not set), it should not exist.
 *        Therefore this routine just creates the software instance
 *        and links it into the ifid list and the vid list.
 *     3. Also allocate an l2 ete for every vid op. These etee can
 *        potentially be shared by other l3 etes
 *        The l2 ete's are allocated only for warmboot
 *        Otherwise it will be allocated when first l3 egress is created
 */
STATIC int
_bcm_caladan3_l3_alloc_intf(_caladan3_l3_fe_instance_t *l3_fe,
                          bcm_l3_intf_t          *bcm_intf,
                          uint32                 *reserve_etes,
                          _caladan3_l3_intf_t        **ret_intf)
{
    int                            status, flags;
    int                            ii;
    uint32                         vid, hash_index, size, ifid = ~0;
    _caladan3_l3_intf_t               *l3_intf;
    dq_p_t                         hash_bucket;

    status       = BCM_E_NONE;
    vid          = bcm_intf->l3a_vid;
    size         = sizeof(_caladan3_l3_intf_t);
    l3_intf      = (_caladan3_l3_intf_t *)sal_alloc(size, "Intf-Data");
    if (l3_intf) {
        sal_memset(l3_intf, 0, size);
    } else {
        return BCM_E_MEMORY;
    }

    flags = 0;
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
    }

    ifid =  bcm_intf->l3a_intf_id;
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit, SBX_CALADAN3_USR_RES_IFID,
                                     1, &ifid, flags);
    if (BCM_FAILURE(status)) {
        if ((bcm_intf->l3a_flags & BCM_L3_WITH_ID) &&
            (status == BCM_E_RESOURCE)){
            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "Reserved interface id 0x%x\n"),
                         ifid));            
        } else {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) could not allocate ifid"),
                       bcm_errmsg(status)));
            sal_free(l3_intf);
            return status;
        }
    } else {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "Allocated interface id 0x%x\n"),
                     ifid));
    }

    bcm_intf->l3a_intf_id = ifid;

    if (bcm_intf->l3a_mtu == 0) {
        bcm_intf->l3a_mtu = _CALADAN3_DEFAULT_EGR_MTU;
    }

    sal_memcpy(&l3_intf->if_info, bcm_intf,
               sizeof(bcm_l3_intf_t));

    l3_intf->if_info.l3a_intf_id = ifid;

    hash_index         = _CALADAN3_GET_INTF_ID_HASH(ifid);
    hash_bucket        = &l3_fe->fe_intf_by_id[hash_index];
    l3_intf->if_flags |= _CALADAN3_L3_INTF_IN_IFID_LIST;
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_ifid_link);

    hash_index         = _CALADAN3_GET_INTF_VID_HASH(vid);
    hash_bucket        = &l3_fe->fe_intf_by_vid[hash_index];;
    l3_intf->if_flags |= _CALADAN3_L3_INTF_IN_VID_LIST;
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_vid_link);

    for (ii=0; ii < _CALADAN3_INTF_L3ETE_HASH_SIZE; ii++) {
        DQ_INIT(&l3_intf->if_ete_hash[ii]);
    }

    DQ_INIT(&l3_intf->if_oam_ep_list);

    *ret_intf = l3_intf;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_flush_cache
 * Purpose:
 *     Unconditionally flush the ilib cached transactions to hw
 * Parameters:
 *     unit - BCM Device number
 * Returns:
 *     BCM_E_NONE      - Success
 */
int _bcm_caladan3_l3_flush_cache(int unit, int flag)
{

   int                  status = BCM_E_NONE;
   _caladan3_l3_fe_instance_t      *l3_fe;

   BCM_IF_ERROR_RETURN(L3_LOCK(unit));

   l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
   if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return BCM_E_UNIT;
   }

   switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
   case SOC_SBX_UCODE_TYPE_G3P1:
       status = _bcm_caladan3_g3p1_l3_flush_cache(l3_fe, flag);
       break;
#endif
   default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
   }

   L3_UNLOCK(unit);

   return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_add_intf
 * Purpose:
 *     Update HW tables with mac addr of new interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     The caller has made sure that it does not already
 *     exist.
 */
STATIC int
_bcm_caladan3_l3_add_intf(_caladan3_l3_fe_instance_t *l3_fe,
                        bcm_l3_intf_t          *bcm_intf,
                        _caladan3_l3_intf_t        *l3_intf)
{
    int                 status;
    uint32              ul_mac_idx;

    status = _bcm_caladan3_l3_update_smac_table(l3_fe,
                                               bcm_intf->l3a_mac_addr,
                                               L3_INTF_ADD_SMAC,
                                               &ul_mac_idx);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not add MAC address into HW: %s\n"),
                   bcm_errmsg(status)));
        return status;
    }

    l3_intf->if_lmac_idx   = ul_mac_idx;
    l3_intf->if_flags          |= _CALADAN3_L3_INTF_SMAC_IN_TBL;

    /*
     * Note that if there is any error we decr the ref_cnt on the mac address
     * (which will free if neccessary). However if there is a error in map or
     * set of ete, the caller will simply free the ete indices. Therefore
     * partial failures are handled correctly
     */
    return BCM_E_NONE;
}

int
_bcm_caladan3_l3_oam_endpoint_associate(int      unit, 
                                      bcm_if_t l3_intf_id,
                                      dq_p_t  oamep,
                                      uint8  add)
{
    _caladan3_l3_intf_t            *l3_intf;
    _caladan3_l3_fe_instance_t     *l3_fe;
    int                         status = BCM_E_NONE;
    uint32                    ifid;

    if (!oamep) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(l3_intf_id);

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
       status = BCM_E_UNIT;
    } else {
        status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                  ifid,
                                                  &l3_intf);
        if (BCM_SUCCESS(status)) {
            if(add) {
                DQ_INSERT_HEAD(&l3_intf->if_oam_ep_list, oamep);
            } else {
                DQ_REMOVE(oamep);
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Could not find interface ID 0x%x: %d %s\n"),
                       l3_intf_id, status, bcm_errmsg(status)));
        }
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_handle_smac_change
 * Purpose:
 *     Handle a change in mac addr of the interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Delete the old mac addr (iff ref_count == 0) and
 *     Add the new addr
 */
int
_bcm_caladan3_l3_handle_smac_change(_caladan3_l3_fe_instance_t *l3_fe,
                                  bcm_l3_intf_t          *bcm_intf,
                                  _caladan3_l3_intf_t        *l3_intf)
{
    int       status;
    uint32    old_ul_idx;
    uint32    new_ul_idx;
    bcm_mac_t old_smac;

    old_ul_idx   = l3_intf->if_lmac_idx;

    ENET_COPY_MACADDR(l3_intf->if_info.l3a_mac_addr, old_smac);

    status = _bcm_caladan3_l3_update_smac_table(l3_fe,
                                               bcm_intf->l3a_mac_addr,
                                               L3_INTF_ADD_SMAC,
                                               &new_ul_idx);
    if (status == BCM_E_NONE) {
        ENET_COPY_MACADDR(bcm_intf->l3a_mac_addr, 
                          l3_intf->if_info.l3a_mac_addr);

        l3_intf->if_lmac_idx = new_ul_idx;
        l3_intf->if_flags        |= _CALADAN3_L3_INTF_SMAC_IN_TBL;

        status = _bcm_caladan3_l3_update_smac_table(l3_fe,
                                                   old_smac,
                                                   L3_INTF_DEL_SMAC,
                                                   &old_ul_idx);
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_update_intf
 * Purpose:
 *     Update the HW for any changes in interface properties
 *     i.e. BCM_L3_REPLACE on an interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     l3_intf  - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Need to handle changes in any of the following
 *     l3a_vrf
 *     l3a_mac_addr
 *     l3a_tunnel_idx
 *     l3a_ttl
 *     l3a_mtu
 */
STATIC int
_bcm_caladan3_l3_update_intf(_caladan3_l3_fe_instance_t *l3_fe,
                           bcm_l3_intf_t          *bcm_intf,
                           _caladan3_l3_intf_t        *l3_intf)
{
    int                             status;
    uint32                          chg_flags;
    uint32                          hash_index;
    dq_p_t                          hash_bucket;
    uint32                          flags = 0;

    chg_flags = 0;
    status    = BCM_E_NONE;

    if (ENET_CMP_MACADDR(bcm_intf->l3a_mac_addr, 
                         l3_intf->if_info.l3a_mac_addr)) {
        chg_flags |= L3_INTF_MAC_CHANGED;
    }

    /*
     * It has already been checked that there are no ETEs on this
     */
    if (bcm_intf->l3a_vrf != l3_intf->if_info.l3a_vrf) {
        chg_flags |=   L3_INTF_VRF_CHANGED;
    }

    if (bcm_intf->l3a_mtu &&
        (bcm_intf->l3a_mtu != l3_intf->if_info.l3a_mtu)) {
        chg_flags |=  L3_INTF_MTU_CHANGED;
    }

    if (bcm_intf->l3a_vid &&
        (bcm_intf->l3a_vid != l3_intf->if_info.l3a_vid)) {
        chg_flags |=  L3_INTF_VID_CHANGED;
    }

    if (chg_flags & (L3_INTF_VID_CHANGED | L3_INTF_MAC_CHANGED)) {
        if (chg_flags & (L3_INTF_MAC_CHANGED)) {
            status = _bcm_caladan3_l3_handle_smac_change(l3_fe,
                                                       bcm_intf,
                                                       l3_intf);
        }
        if (status != BCM_E_NONE) {
            return status;
        }
        DQ_REMOVE(&l3_intf->if_vid_link);
    }

    /*
     * Copy over the new values
     */
    if (chg_flags & L3_INTF_VID_CHANGED) {
        l3_intf->if_info.l3a_vid = bcm_intf->l3a_vid;
    }
    if (chg_flags & L3_INTF_MAC_CHANGED) {
        ENET_COPY_MACADDR(bcm_intf->l3a_mac_addr,
                          l3_intf->if_info.l3a_mac_addr);
    }
    if (chg_flags & L3_INTF_MTU_CHANGED) {
        l3_intf->if_info.l3a_mtu = bcm_intf->l3a_mtu;
    }

    if (chg_flags & (L3_INTF_VRF_CHANGED)) {
        l3_intf->if_info.l3a_vrf = bcm_intf->l3a_vrf;
    }

    if (chg_flags & (L3_INTF_VID_CHANGED | L3_INTF_MAC_CHANGED)) {
        hash_index         =  _CALADAN3_GET_INTF_VID_HASH(l3_intf->if_info.l3a_vid);
        hash_bucket        =  &l3_fe->fe_intf_by_vid[hash_index];
        DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_vid_link);
    }


    if (chg_flags &
        (L3_INTF_VID_CHANGED |
         L3_INTF_MAC_CHANGED |
         L3_INTF_MTU_CHANGED |
         L3_INTF_TTL_CHANGED)) {

    }

    /* Save special flags other than REPLACE/WITH_ID*/
    flags = bcm_intf->l3a_flags;
    flags &= ~(BCM_L3_REPLACE | BCM_L3_WITH_ID);
    if (flags != l3_intf->if_info.l3a_flags) {
        l3_intf->if_info.l3a_flags = flags;
    }

    return status;
}

/*
 * Function:
 *    _bcm_caladan3_l3_intf_delete_one
 * Purpose:
 *     Delete L3 interface based on intf context
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    -  (IN) l3 interface
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_caladan3_l3_intf_delete_one(_caladan3_l3_fe_instance_t     *l3_fe,
                               _caladan3_l3_intf_t            **pl3_intf)
{

    uint32                      ul_smac_idx;
    uint32                      i;
    int                         status;
    _caladan3_l3_intf_t            *l3_intf;

    l3_intf = *pl3_intf;
    status  = BCM_E_NONE;

    for (i = 0; i < _CALADAN3_INTF_L3ETE_HASH_SIZE; i++) {
        if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
            return BCM_E_BUSY;
        }
    }

    ul_smac_idx = l3_intf->if_lmac_idx;

    if (l3_intf->if_flags & _CALADAN3_L3_INTF_SMAC_IN_TBL) {
        status = _bcm_caladan3_l3_update_smac_table(l3_fe,
                                                   l3_intf->if_info.l3a_mac_addr,
                                                   L3_INTF_DEL_SMAC,
                                                   &ul_smac_idx);
        if (status != BCM_E_NONE) {
            return status;
        }
        l3_intf->if_flags &= ~(_CALADAN3_L3_INTF_SMAC_IN_TBL);
    }

    _bcm_caladan3_l3_free_sw_intf(l3_fe,
                                l3_intf);

    l3_intf   = NULL;
    *pl3_intf = NULL;
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_tunnel_initiator_check_and_get_intf
 * Purpose:
 *     basic validations and get of L3 intf for tunnel
 * Parameters:
 *     l3_fe          - (IN)  l3 fe instance context
 *     intf           - (IN)  bcm intf
 *     l3_intf        - (IN)  l3 interface context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */
int
_bcm_caladan3_tunnel_initiator_check_and_get_intf(_caladan3_l3_fe_instance_t *l3_fe,
                                                bcm_l3_intf_t          *intf,
                                                _caladan3_l3_intf_t       **l3_intf)
{
    uint32                 l3a_intf_id;
    int                    status;

    status = BCM_E_NONE;

    if (intf->l3a_flags & BCM_L3_WITH_ID) {
        if (!_CALADAN3_L3_USER_IFID_VALID(intf->l3a_intf_id)) {
            return BCM_E_PARAM;
        }
    } else {
        if (!BCM_VLAN_VALID(intf->l3a_vid)) {
            return BCM_E_PARAM;
        }
        if (BCM_MAC_IS_MCAST(intf->l3a_mac_addr) ||
            BCM_MAC_IS_ZERO(intf->l3a_mac_addr)) {
            return BCM_E_PARAM;
        }
    }

    if (intf->l3a_flags & BCM_L3_WITH_ID) {
        l3a_intf_id = _CALADAN3_IFID_FROM_USER_HANDLE(intf->l3a_intf_id);

        status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                  l3a_intf_id,
                                                  l3_intf);

    } else {
        status = _bcm_caladan3_l3_find_intf_by_vid_mac(l3_fe,
                                                     intf,
                                                     l3_intf);
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_alloc_mpath_object
 * Purpose:
 *     allocate a multipath object and the base fte index
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     crc_info   - (IN)  the sorted mapped(original) fte,
 *                        with their crc16
 *     ret_mp_info- (OUT) allocated mpath object
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     alloc ecmp_set_size number of FTE indices and
 *     an object to hold the unique set passed by user
 */
int
_bcm_caladan3_l3_alloc_mpath_object(_caladan3_l3_fe_instance_t    *l3_fe,
                                  uint32                     flags,
                                  _caladan3_l3_mpath_crc_info_t *crc_info,
                                  _caladan3_egr_mp_info_t       **ret_mp_info)
{
    int                         status = 0;
    uint32                      size, i;
    uint32                      fte[_CALADAN3_L3_ECMP_MAX];
    uint32                      hash_idx;
    uint32                      allocater_flags;
    uint32                     *mapped_fte;
    _caladan3_egr_mp_info_t        *mp_info;
    _caladan3_l3_fte_t             *fte_hash_elem, *tmp_elem;
    uint32                      fte_hash_idx;

    size = sizeof(_caladan3_l3_fte_t);
    fte_hash_elem = sal_alloc(sizeof(_caladan3_l3_fte_t), "FTE-Hash");
    if (fte_hash_elem == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not allocate fte hash elem\n")));

        return BCM_E_MEMORY;
    }
    sal_memset(fte_hash_elem, 0, size);

    size    = sizeof(_caladan3_egr_mp_info_t) +
        ((sizeof(uint32)) * l3_fe->fe_mp_set_size);
    mp_info = (_caladan3_egr_mp_info_t *) sal_alloc(size, "mpath-info");
    if (mp_info == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Error in allocating memory for multipath object")));
        sal_free(fte_hash_elem);
        return BCM_E_MEMORY;
    }
    sal_memset(mp_info, 0, size);

    mp_info->fte_crc   = crc_info->crc16;
    mp_info->num_valid = crc_info->crc_data.count;

    sal_memset(fte, 0, sizeof(uint32) * _CALADAN3_L3_ECMP_MAX);
    allocater_flags = _SBX_CALADAN3_RES_FLAGS_CONTIGOUS;
    if (flags & BCM_L3_WITH_ID) {
        allocater_flags |= _SBX_CALADAN3_RES_FLAGS_RESERVE;
        for (i = 0; i < l3_fe->fe_mp_set_size; i++) {
            fte[i] = crc_info->mpbase + i;
        }
    }
    status = _sbx_caladan3_resource_alloc(l3_fe->fe_unit,
                                     SBX_CALADAN3_USR_RES_FTE_L3_MPATH,
                                     l3_fe->fe_mp_set_size,
                                     fte,
                                     allocater_flags);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Error in allocating multipath resource: %s\n"),
                   bcm_errmsg(status)));

        sal_free(mp_info);
        sal_free(fte_hash_elem);
        return status;
    }

    /*
     * As a check, make sure that this fte_idx already does not exist
     */
    status = _bcm_caladan3_l3_get_sw_fte(l3_fe,
                                       fte[0],
                                       &tmp_elem);
    if (status == BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "FTE idx 0x%x already exists in hash\n"),
                   fte[0]));
        sal_free(fte_hash_elem);
        sal_free(mp_info);
        status = _sbx_caladan3_resource_free(l3_fe->fe_unit,
                                        SBX_CALADAN3_USR_RES_FTE_L3_MPATH,
                                        l3_fe->fe_mp_set_size,
                                        fte,
                                        _SBX_CALADAN3_RES_FLAGS_CONTIGOUS);
        return BCM_E_EXISTS;
    }
    status = BCM_E_NONE;
    crc_info->mpbase = mp_info->mp_base = fte[0];

    mapped_fte = &mp_info->mapped_fte[0];
    for (i = 0; i < mp_info->num_valid; i++) {
        *mapped_fte = crc_info->crc_data.fte_indices[i];
        mapped_fte++;
    }

    fte_hash_elem->ref_count = 1;
    fte_hash_elem->fte_idx   = mp_info->mp_base;
    fte_hash_idx = _CALADAN3_GET_FTE_HASH_IDX(fte_hash_elem->fte_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_fteidx2_fte[fte_hash_idx], &fte_hash_elem->fte_hash_link);


    hash_idx = _CALADAN3_GET_HASH_EGR_MP(mp_info->fte_crc);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_egr[hash_idx], &mp_info->egr_mp_link);

    hash_idx = _CALADAN3_GET_HASH_MPBASE_MP(mp_info->mp_base);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_mpbase[hash_idx], &mp_info->mpbase_link);

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "Successfully allocated multipath object"
                             "and Indices (%d - %d)\n"),
                 mp_info->mp_base,
                 mp_info->mp_base + l3_fe->fe_mp_set_size - 1));

    *ret_mp_info = mp_info;
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_destroy_mpath_object
 * Purpose:
 *     de-alloc the mpath object and free the fte indices
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mp_info    - (IN)  mpath object to be freed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_caladan3_l3_destroy_mpath_object(_caladan3_l3_fe_instance_t    *l3_fe,
                                    _caladan3_egr_mp_info_t      **pmp_info)
{
    int                         status=BCM_E_NONE;
    uint32                      derived_fte;
    uint32                      ecmp_size, i;
    /* uint32                      fte[_CALADAN3_L3_ECMP_MAX]; */
    _caladan3_egr_mp_info_t        *mp_info;

    ecmp_size       = l3_fe->fe_mp_set_size;
    status          = BCM_E_NONE;
    mp_info         = *pmp_info;


    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "Destroying mpath object with Index %d\n"),
                 mp_info->mp_base));

    for (i = 0; i < ecmp_size; i++) {
        derived_fte        = mp_info->mp_base + i;
        /* fte[i]             = derived_fte; */


        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            status = _bcm_caladan3_g3p1_fte_op(l3_fe, derived_fte, NULL,
                                             _CALADAN3_FT_OPCODE_CLR);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error(%s) invalidating mpath fte(0x%x)\n"),
                       bcm_errmsg(status), derived_fte));
            return status;
        }
    }

    DQ_REMOVE(&mp_info->egr_mp_link);
    DQ_REMOVE(&mp_info->mpbase_link);
    sal_free(mp_info);
    *pmp_info = NULL;

    status =  _bcm_caladan3_destroy_fte(l3_fe,
                                      L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                      mp_info->mp_base,
                                      0, /* mod */
                                      0  /* ohi */);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not destroy fte 0x%x: %s\n"),
                   mp_info->mp_base, bcm_errmsg(status)));
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_set_mpath_ftes
 * Purpose:
 *     copy the contents of the original ftes to the
 *     derived ones and program HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mp_info    - (IN)  mpath object to be programmed in HW
 *     crc_info   - (IN)  provides the fte contents of the base (mapped)
 *                        FTEs
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_caladan3_l3_set_mpath_ftes(_caladan3_l3_fe_instance_t    *l3_fe,
                              _caladan3_egr_mp_info_t       *mp_info,
                              _caladan3_l3_mpath_crc_info_t *crc_info)
{
    int            status = 0;
    uint32         mapped_fte, derived_fte;
    uint32         ecmp_size, num_mapped_ftes, i, map_idx;

    ecmp_size       = l3_fe->fe_mp_set_size;
    num_mapped_ftes = mp_info->num_valid;
    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "ecmpSize %d mappedFtes %d \n"),
                 ecmp_size, num_mapped_ftes));

    /*
     * Get the mapped Ftes and copy them over
     */
    for (i = 0; i < ecmp_size; i++) {
        map_idx     = i % num_mapped_ftes;
        mapped_fte  = mp_info->mapped_fte[map_idx];
        derived_fte = mp_info->mp_base + i;

        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(l3_fe->fe_unit,
                                "[Programming mpath ftes] "
                                 "derived fte 0x%x mapped fte 0x%x)\n"),
                     (int) derived_fte, (int) mapped_fte));

        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            status = _bcm_caladan3_g3p1_fte_op(l3_fe, derived_fte,
                                             crc_info->fte_contents[map_idx],
                                             _CALADAN3_FT_OPCODE_SET);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Error in FTE programming: %s\n"
                                   "mpbase 0x%x mapped_fte 0x%x derived_fte 0x%x\n"),
                       bcm_errmsg(status),
                       mp_info->mp_base, mapped_fte, derived_fte));
            return status;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_save_fte_hash_elements
 * Purpose:
 *     save the pointers to fte_indices passed in
 *     Typically this will be used to incr/decr ref_counts
 *     if the underlying operation was successful
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     count      - (IN)  number of FTEs
 *     fte_indices- (IN)  array of indices for which we need to get hash elements
 *     elems      - (OUT) array of pointers to fte hash elements
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_caladan3_l3_save_fte_hash_elements(_caladan3_l3_fe_instance_t    *l3_fe,
                                      int                        count,
                                      uint32                    *fte_indices,
                                      _caladan3_l3_fte_t           **elems)
{
    int                         i, status, fte_idx;

    for (i = 0; i < count; i++) {
        fte_idx = fte_indices[i];
        status = _bcm_caladan3_l3_get_sw_fte(l3_fe,
                                           fte_idx,
                                           &elems[i]);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "fte index 0x%x not found in hash\n"),
                       fte_idx));
            return status;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_gen_mpath_crc
 * Purpose:
 *     setup the ftes passed by the user in a cannonical form
 *     i.e sort them and compute crc. This form is used by most
 *     functions that operate on mpath objects
 * Parameters:
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     intf_count - (IN)  number of unique FTEs in mpath
 *     intf_array - (IN)  actual FTEs passed in by user
 *     crc_info   - (OUT) sorted FTEs with crc
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_caladan3_l3_gen_mpath_crc(_caladan3_l3_fe_instance_t    *l3_fe,
                             uint32                     mpbase,
                             uint32                     intf_count,
                             bcm_if_t                  *intf_array,
                             _caladan3_l3_mpath_crc_info_t *crc_info,
                             uint32                     opcode)
{
    uint32                    crc_len;
    int                       i, status = BCM_E_NONE;


    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "Mpath base 0x%x, count %d\n"),
                 mpbase, intf_count));

    status                   = BCM_E_NONE;
    crc_info->crc_data.count = intf_count;
    crc_info->mpbase         = mpbase;

    for (i = 0; i < intf_count; i++) {
        crc_info->crc_data.fte_indices[i] = intf_array[i];
        crc_info->fte_contents[i]         = NULL;
    }
    _shr_sort(&crc_info->crc_data.fte_indices[0],  intf_count, sizeof(uint32),
              _bcm_caladan3_l3_cmp_uint32);

    /*
     * get the FTE hash elements. This will be used later
     * to update ref_counts on the FTEs. This also checks that the
     * FTE is in our hash
     */
    status = _bcm_caladan3_l3_save_fte_hash_elements(l3_fe,
                                                   crc_info->crc_data.count,
                                                   crc_info->crc_data.fte_indices,
                                                   crc_info->fte_hash_elem);
    if (status != BCM_E_NONE) {
        return status;
    }

    /*
     * The extra integer is for the count field itself
     */
    crc_len     = sizeof(uint32) * (intf_count + 1);
    crc_info->crc16 = _shr_crc16(0, (unsigned char *) &crc_info->crc_data,
                                 crc_len);

    /*
     * Get FTE contents only after sorting
     */
    if (opcode == L3_CRC_INFO_FETCH_FTE_DATA) {

        for (i = 0; i < intf_count; i++) {

            switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
            case SOC_SBX_UCODE_TYPE_G3P1:
                crc_info->fte_contents[i] = _bcm_caladan3_g3p1_alloc_fte(1);
                
                if (crc_info->fte_contents[i] == NULL) {
                    status = BCM_E_MEMORY;
                } else {
                    status = _bcm_caladan3_g3p1_fte_op(l3_fe,
                                                     crc_info->crc_data.fte_indices[i],
                                                     crc_info->fte_contents[i],
                                                     _CALADAN3_FT_OPCODE_GET);
                }
                break;
#endif
            default:
                L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
                status = BCM_E_CONFIG;
            }

            if (status != BCM_E_NONE) {
                while(--i > 0){
                    sal_free(crc_info->fte_contents[i]);
                }

                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error %s in getting FTE (0x%x) from HW\n"),
                           bcm_errmsg(status), 
                           crc_info->crc_data.fte_indices[i]));
                return status;
            }
        }
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_l3_free_mpath_crc(_caladan3_l3_mpath_crc_info_t *crc_info,
                              uint32                     intf_count)
{
    int i;
    for (i = 0; i < intf_count; i++) {
        if(crc_info->fte_contents[i]==NULL) {
            sal_free(crc_info->fte_contents[i]);
        }
    }
    sal_free(crc_info);
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_save_old_mpath_state
 * Purpose:
 *     save old math info in case the REPLACE goes awry
 * Parameters:
 *     ol_mp_info - (IN)  the current mpath object
 *     crc_info   - (OUT) the mpath object saved in cannonical form
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_caladan3_save_old_mpath_state(_caladan3_l3_fe_instance_t     *l3_fe,
                                 _caladan3_egr_mp_info_t        *old_mp_info,
                                 _caladan3_l3_mpath_crc_info_t  *crc_info)
{
    int            i;
    int            status = BCM_E_NONE;

    crc_info->crc16          = old_mp_info->fte_crc;
    crc_info->flags          = 0;
    crc_info->mpbase         = old_mp_info->mp_base;
    crc_info->crc_data.count = old_mp_info->num_valid;

    for (i = 0; i < old_mp_info->num_valid; i++) {
        crc_info->crc_data.fte_indices[i] = old_mp_info->mapped_fte[i];

        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            status = _bcm_caladan3_g3p1_fte_op(l3_fe,
                                             crc_info->crc_data.fte_indices[i],
                                             &(crc_info->fte_contents[i]),
                                             _CALADAN3_FT_OPCODE_GET);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error %s in getting FTE (0x%x) from HW"),
                       bcm_errmsg(status),
                       crc_info->crc_data.fte_indices[i]));
            return status;
        }
    }

    /*
     * get the FTE hash elements. This will be used later
     * to update ref_counts on the FTEs. This also checks that the
     * FTE is in our hash
     */
    status = _bcm_caladan3_l3_save_fte_hash_elements(l3_fe,
                                                   crc_info->crc_data.count,
                                                   crc_info->crc_data.fte_indices,
                                                   crc_info->fte_hash_elem);
    if (status != BCM_E_NONE) {
        return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_l3_mpath_find
 * Purpose:
 *     Given the original set of FTEs find the base FTE of the
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     crc_info   - (IN)  the mpath object saved in cannonical form
 *     ret_info   - (OUT) mpath object if found
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_caladan3_l3_mpath_find(_caladan3_l3_fe_instance_t     *l3_fe,
                          _caladan3_l3_mpath_crc_info_t  *crc_info,
                          _caladan3_egr_mp_info_t        **ret_info)
{
    uint32               hash_idx;
    dq_p_t               mp_by_egr_head, mp_by_egr_elem;
    _caladan3_egr_mp_info_t *mp_info = NULL;

    hash_idx =  _CALADAN3_GET_HASH_EGR_MP(crc_info->crc16);
    mp_by_egr_head = &l3_fe->fe_mp_by_egr[hash_idx];
    DQ_TRAVERSE(mp_by_egr_head, mp_by_egr_elem) {
        _CALADAN3_MP_INFO_FROM_EGR_MP_DQ(mp_by_egr_elem, mp_info);
        if (mp_info->fte_crc == crc_info->crc16) {
            /* both are sorted, hence should be identical */
            if ((mp_info->num_valid == crc_info->crc_data.count) &&
                !sal_memcmp(mp_info->mapped_fte,
                             crc_info->crc_data.fte_indices,
                             sizeof(uint32)*mp_info->num_valid)) {
                *ret_info = mp_info;
                return BCM_E_NONE;
            }
        }
    } DQ_TRAVERSE_END(mp_by_egr_head, mp_by_egr_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_l3_mpath_get
 * Purpose:
 *     Given the base FTE find the mpath object and
 *     hence the original set of FTEs.
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mpbase     - (IN)  mpath base FTE for the mpath group
 *     ret_info   - (OUT) mpath object if found
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
STATIC int
_bcm_caladan3_l3_mpath_get(_caladan3_l3_fe_instance_t *l3_fe,
                         uint32                 mpbase,
                         _caladan3_egr_mp_info_t    **ret_info)
{
    uint32                hash_idx;
    dq_p_t                mp_by_mpbase_head, mp_by_mpbase_elem;
    _caladan3_egr_mp_info_t  *mp_info = NULL;

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "get mpath object with base FTE %d\n"),
                 mpbase));

    hash_idx =  _CALADAN3_GET_HASH_MPBASE_MP(mpbase);
    mp_by_mpbase_head = &l3_fe->fe_mp_by_mpbase[hash_idx];
    DQ_TRAVERSE(mp_by_mpbase_head, mp_by_mpbase_elem) {
        _CALADAN3_MP_INFO_FROM_MPBASE_MP_DQ(mp_by_mpbase_elem, mp_info);
        if (mp_info->mp_base == mpbase) {
            *ret_info = mp_info;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(mp_by_mpbase_head, mp_by_mpbase_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_caladan3_l3_add_mpath_object
 * Purpose:
 *     Add a new mpath object in SW and HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     crc_info   - (IN)  the mpath object saved in cannonical form
 *     ret_info   - (OUT) mpath object if created
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *    It is assumed that the caller has done a find and
 *    made sure that it does not exist
 */
int
_bcm_caladan3_l3_add_mpath_object(_caladan3_l3_fe_instance_t    *l3_fe,
                                uint32                     flags,
                                _caladan3_l3_mpath_crc_info_t *crc_info,
                                _caladan3_egr_mp_info_t       **ret_mp_info)
{
    int                         i, status;
    _caladan3_egr_mp_info_t        *mp_info;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(l3_fe->fe_unit,
                          "Enter\n")));

    status = _bcm_caladan3_l3_alloc_mpath_object(l3_fe,
                                               flags,
                                               crc_info,
                                               &mp_info);
    if (status != BCM_E_NONE) {
        return status;
    }
    status = _bcm_caladan3_l3_set_mpath_ftes(l3_fe,
                                           mp_info,
                                           crc_info);
    if (status != BCM_E_NONE) {
        _bcm_caladan3_l3_destroy_mpath_object(l3_fe,
                                            &mp_info);
        mp_info = NULL;
    }

    /*
     * everything went well. Increment the ref_counts for all the
     * mapped ftes.
     */
    for (i = 0; i < crc_info->crc_data.count; i++) {
        (crc_info->fte_hash_elem[i])->ref_count++;
    }

    *ret_mp_info = mp_info;
    return status;
}

/*
 * Function:
 *     _bcm_caladan3_l3_update_mpath_object
 * Purpose:
 *     update mpath object in SW and HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     crc_info   - (IN)  the new mpath object saved in cannonical form
 *     mp_info    - (IN/OUT) mpath object to be updated
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_caladan3_l3_update_mpath_object(_caladan3_l3_fe_instance_t    *l3_fe,
                                   _caladan3_l3_mpath_crc_info_t *crc_info,
                                   _caladan3_egr_mp_info_t       *mp_info)
{
    int                         status;
    int                         i;
    uint32                     *mapped_fte;
    uint32                      hash_idx;

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "Enter\n")));

    /*
     * copy over the new sorted FTEs, their count and CRC
     * into the mpath object. The mpbase does not change
     * in case of update
     */
    mapped_fte = &mp_info->mapped_fte[0];
    for (i = 0; i < crc_info->crc_data.count; i++) {
        *mapped_fte = crc_info->crc_data.fte_indices[i];
        mapped_fte++;
    }
    mp_info->num_valid = crc_info->crc_data.count;
    mp_info->fte_crc   = crc_info->crc16;

    status = _bcm_caladan3_l3_set_mpath_ftes(l3_fe,
                                           mp_info,
                                           crc_info);
    if (status != BCM_E_NONE) {
        /*
         * caller will try to revert back to old-working fte mpath
         * if required
         */
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error in setting mpath object 0x%x: %s\n"),
                   mp_info->mp_base, bcm_errmsg(status)));
        return status;
    }

    /*
     * Since the crc may have changed, remove from old hash bkt
     * and insert into new hash bkt
     */
    DQ_REMOVE(&mp_info->egr_mp_link);
    hash_idx =  _CALADAN3_GET_HASH_EGR_MP(mp_info->fte_crc);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_egr[hash_idx],
                   &mp_info->egr_mp_link);

    return status;
}

/*
 * Function:
 *     _bcm_caladan3_handle_mpath_update
 * Purpose:
 *     Update a existing mpath object
 * Parameters:
 *     l3_fe          - (IN)  l3 fe instance
 *     mp_info        - (IN)  the existing mpath object
 *     new_intf_count - (IN)  the new number of contributing FTEs
 *     new_ftes       - (IN)  the actual indice of  contributing FTEs
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This does the initial setup and calls the real update
 *     function.  Also handles roll-back
 */
int
_bcm_caladan3_handle_mpath_update(_caladan3_l3_fe_instance_t  *l3_fe,
                                _caladan3_egr_mp_info_t     *mp_info,
                                uint32                   new_intf_count,
                                bcm_if_t                *new_ftes)
{

    _caladan3_l3_mpath_crc_info_t    *crc_info, *old_crc_info;
    int                          status, ignore_status, size, i;

    status   = ignore_status = BCM_E_NONE;
    size     = sizeof(_caladan3_l3_mpath_crc_info_t) * 2;
    crc_info = sal_alloc(size, "mpath-crc-info");
    if (crc_info == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(crc_info, 0, size);

    old_crc_info = crc_info + 1;

    /*
     * save the old info
     */
    status = _bcm_caladan3_save_old_mpath_state(l3_fe,
                                              mp_info,
                                              old_crc_info);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "could not retrieve existing FTEs: %s\n"), 
                   bcm_errmsg(status)));
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    /*
     * fill up the new FTEs in sorted order into crc_info
     */
    status = _bcm_caladan3_l3_gen_mpath_crc(l3_fe,
                                          mp_info->mp_base,
                                          new_intf_count,
                                          new_ftes,
                                          crc_info,
                                          L3_CRC_INFO_FETCH_FTE_DATA);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Error in setting up mpath Info: %s\n"),
                   bcm_errmsg(status)));
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    status = _bcm_caladan3_l3_update_mpath_object(l3_fe,
                                                crc_info,
                                                mp_info);
    if (status != BCM_E_NONE) {
        ignore_status =
            _bcm_caladan3_l3_update_mpath_object(l3_fe,
                                               old_crc_info,
                                               mp_info);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "mpbase(0x%x) neither set"
                                   " to new nor could be reverted back to "
                                   "old set of ftes\n"),
                       mp_info->mp_base));

        }

        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    /*
     * everything went well. Increment the ref_counts for all the
     * new mapped ftes and decrement the ref_count for the old ones
     * Note that there may be some ftes that are common to both the
     * sets. For those an increment followed by a decrement will
     * cancel out
     */
    for (i = 0; i < crc_info->crc_data.count; i++) {
        (crc_info->fte_hash_elem[i])->ref_count++;
    }

    for (i = 0; i < old_crc_info->crc_data.count; i++) {
        (old_crc_info->fte_hash_elem[i])->ref_count--;
    }

    if (crc_info) {
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_l3_modid_set(int unit, int modid)
{
    _caladan3_l3_fe_instance_t      *l3_fe;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe->fe_my_modid = modid;

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/************************ ONLY BCM FUNCTIONS BELOW THIS LINE  ************/

/*
 * Function:
 *     bcm_caladan3_l3_init
 * Purpose:
 *     Initialize L3 structures
 * Parameters:
 *     unit  - (IN) C3 unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_caladan3_l3_init(int unit)
{
    int         status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    status = _bcm_caladan3_l3_sw_init(unit);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error(%s)\n"),
                   bcm_errmsg(status)));
        return status;
    }

    /* Disable IPv4 caching */
    SOC_SBX_STATE(unit)->cache_l3host = FALSE;
    SOC_SBX_STATE(unit)->cache_l3route = FALSE;

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(unit,
                            "completed init successfully\n")));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_caladan3_l3_intf_create
 * Purpose:
 *     Create an L3 interface
 * Parameters:
 *     unit  - (IN) C3 unit number
 *     intf  - (IN) interface info:
 *             (IN) l3a_mac_addr - MAC address;
 *             (IN) l3a_vid - VLAN ID;
 *             (IN) flag BCM_L3_WITH_ID: use specified interface ID l3a_intf.
 *                  flag BCM_L3_REPLACE: overwrite if interface already exists.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. The L3 interface ID is automatically assigned unless a specific
 *         ID is requested with the BCM_L3_WITH_ID flag.
 *      2. Create a interface instance and insert it into the hash tables
 *         (<ifid> hash and <vid, mac> hash.
 *      3. Add the mac address to the localStation match table and Smac2Idx table.
 */
int
bcm_caladan3_l3_intf_create(int            unit,
                          bcm_l3_intf_t *bcm_intf)
{
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf;
    int                         status;

    if (bcm_intf == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = _bcm_caladan3_l3_sw_intf_create_checks(l3_fe,
                                                  bcm_intf);

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (bcm_intf->l3a_flags &  BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _CALADAN3_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);
    }

    status = _bcm_caladan3_l3_intf_create_replace(unit,
                                                l3_fe,
                                                bcm_intf,
                                                &l3_intf);

    /*
     * At this time, if things have gone well the intf_id is the
     * internal number. Convert it back to user handle
     */
    if ((status == BCM_E_NONE) ||
        (bcm_intf->l3a_flags &  BCM_L3_WITH_ID)) {
        bcm_intf->l3a_intf_id = _CALADAN3_USER_HANDLE_FROM_IFID(bcm_intf->l3a_intf_id);
    }

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *     bcm_caladan3_l3_intf_delete
 * Purpose:
 *     Delete L3 interface based on L3 interface ID
 * Parameters:
 *     unit  - (IN) Fe2000 unit number
 *     intf  - (IN) interface structure with L3 interface ID as input
 *
 * Returns:
 *     BCM_E_XXX
 *
 * Assumptions:
 *     The multicast egress object is  not in use.
 *     The HW ete will be deprogrammed and SW structures
 *     will be freed.
 *
 * Notes:
 *     If there is any non ipmcast etes dependent on this intf, then
 *     return EBUSY.
 *
 */
int
bcm_caladan3_l3_intf_delete(int unit,
                          bcm_l3_intf_t *bcm_intf)
{
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf;
    int                         status;

    if (NULL == bcm_intf) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_PARAM;
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _CALADAN3_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);
        if (_CALADAN3_IFID_VALID_RANGE(bcm_intf->l3a_intf_id)) {
            status = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                                      bcm_intf->l3a_intf_id,
                                                      &l3_intf);
        }
    } else {
        if (BCM_VLAN_VALID(bcm_intf->l3a_vid)) {
            status = _bcm_caladan3_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_l3_intf_delete_one(l3_fe,
                                            &l3_intf);
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _CALADAN3_USER_HANDLE_FROM_IFID(bcm_intf->l3a_intf_id);
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_caladan3_l3_intf_find
 * Purpose:
 *     Find the L3 intf number based on (MAC, VLAN)
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN) interface (MAC, VLAN),
 *             (OUT)intf number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_caladan3_l3_intf_find(int unit,
                        bcm_l3_intf_t *intf)
{

    int status;

    /*
     * WITH_ID should not be specified and VID,MAC needs to be valid
     */
    if ((intf->l3a_flags & BCM_L3_WITH_ID)         ||

        (BCM_MAC_IS_MCAST(intf->l3a_mac_addr))     ||
        (BCM_MAC_IS_ZERO(intf->l3a_mac_addr))      ||

        (intf->l3a_vid && (!BCM_VLAN_VALID(intf->l3a_vid)))) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));


    status = _bcm_caladan3_l3_intf_locate(unit,
                                        intf,
                                        L3_INTF_LOCATE_BY_VID);
    L3_UNLOCK(unit);

    return status;

}

/*
 * Function:
 *     bcm_caladan3_l3_intf_get
 * Purpose:
 *     Given the L3 interface id, return the MAC and VLAN
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN) L3 interface;
 *             (OUT)VLAN ID, 802.3 MAC for this L3 intf
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_caladan3_l3_intf_get(int unit,
                    bcm_l3_intf_t *intf)
{
    int       status;
    uint32    ifid;
    uint32    flags;

    if (!(intf->l3a_flags & BCM_L3_WITH_ID)) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "l3a_flags(0x%x) does not have BCM_L3_WITH_ID\n"),
                     intf->l3a_flags));
        return BCM_E_PARAM;
    }

    if (!_CALADAN3_L3_USER_IFID_VALID(intf->l3a_intf_id)) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "l3a_intf_id(0x%x) is invalid\n"),
                     intf->l3a_intf_id));
        return BCM_E_PARAM;
    }

    ifid =  _CALADAN3_IFID_FROM_USER_HANDLE(intf->l3a_intf_id);
    if (!_CALADAN3_IFID_VALID_RANGE(ifid)) {
        LOG_VERBOSE(BSL_LS_BCM_L3,
                    (BSL_META_U(unit,
                                "internal l3a_intf_id(0x%x) is invalid\n"),
                     ifid));
        return BCM_E_PARAM;
    }

    flags             = intf->l3a_flags;
    bcm_l3_intf_t_init(intf);
    intf->l3a_intf_id = ifid;
    intf->l3a_flags   = flags;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    status = _bcm_caladan3_l3_intf_locate(unit,
                                        intf,
                                        L3_INTF_LOCATE_BY_IFID);

    intf->l3a_intf_id =  _CALADAN3_USER_HANDLE_FROM_IFID(ifid);

    L3_UNLOCK(unit);

    return status;


}
/*
 * Function:
 *     bcm_caladan3_l3_intf_find_vlan
 * Purpose:
 *     Given a vid, return the first interface that matches
 *
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN/OUT) BCM L3 interface;
 *                     (IN)VLAN ID
 * Returns:
 *     BCM_E_XXX
 * Note:
 *     return first intf based on vid
 *     Can return different values at diff times
 */
int
bcm_caladan3_l3_intf_find_vlan(int unit,
                          bcm_l3_intf_t *intf)
{
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf = NULL;
    dq_p_t                      l3_intf_elem;
    uint32                      hash_index;

    if (!BCM_VLAN_VALID(intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    hash_index   =  _CALADAN3_GET_INTF_VID_HASH(intf->l3a_vid);
    if (DQ_EMPTY(&l3_fe->fe_intf_by_vid[hash_index])) {
        L3_UNLOCK(unit);
        return  BCM_E_NOT_FOUND;
    }
    l3_intf_elem = DQ_HEAD(&l3_fe->fe_intf_by_vid[hash_index], dq_p_t);
    _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);

    *intf = l3_intf->if_info;

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


int
bcm_caladan3_l3_intf_delete_all(int unit)
{
    _caladan3_l3_fe_instance_t     *l3_fe;
    _caladan3_l3_intf_t            *l3_intf = NULL;
    dq_p_t                      l3_intf_elem;
    int                         i, status, rv;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    rv = status = BCM_E_NONE;
    for (i = 0; i < _CALADAN3_INTF_ID_HASH_SIZE; i++) {
        while (!(DQ_EMPTY(&l3_fe->fe_intf_by_id[i]))) {
            l3_intf_elem = DQ_HEAD(&l3_fe->fe_intf_by_id[i], dq_p_t);
            _CALADAN3_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
            rv = _bcm_caladan3_l3_intf_delete_one(l3_fe,
                                                &l3_intf);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L3,
                          (BSL_META_U(l3_fe->fe_unit,
                                      "error in interface delete: %s\n"),
                           bcm_errmsg(rv)));
                status = rv;
            }
        }
    }


    L3_UNLOCK(unit);
    return status;
}


int
bcm_caladan3_l3_host_find(int unit,
                     bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_get(unit, &route_info));
}

int
bcm_caladan3_l3_host_add(int unit,
                    bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_add(unit, &route_info));
}

int
bcm_caladan3_l3_host_delete(int unit,
                       bcm_l3_host_t *ip_addr)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(ip_addr, &route_info);
    return (bcm_l3_route_delete(unit, &route_info));
}

int
bcm_caladan3_l3_host_delete_by_network(int unit,
                                  bcm_l3_route_t *net_addr)
{
    return (bcm_l3_route_delete(unit, net_addr));
}

int
bcm_caladan3_l3_host_delete_by_interface(int unit,
                                    bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_delete_by_interface(unit, &route_info));
}

int
bcm_caladan3_l3_host_delete_all(int unit,
                           bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _CALADAN3_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_delete_by_interface(unit, &route_info));
}

/*
 * Function:
 *      bcm_caladan3_l3_route_add
 * Purpose:
 *      Add an IP route to the routing table
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     info  - (IN) Pointer to bcm_l3_route_t containing all valid fields.
 * Returns:
 *      BCM_E_XXX
 *
 * Assumptions:
 *     info->l3a_intf must contain the FTE Index returned at egress create
 *     The following fields in the route structure are ignored for the
 *     purpose of this API (they are needed at egress create time);
 *     l3a_nexthop_ip;  Next hop IP address (XGS1/2, IPv4)
 *     l3a_nexthop_mac;
 *     l3a_modid;
 *     l3a_stack_port;  Used if modid not local (Strata Only)
 *     l3a_vid;         BCM5695 only - for per-VLAN def route
 *
 * Notes:
 *     1. if BCM_L3_RPF flag is set, then the route is added to SA table also
 *     2. if the route is to be added to SA table,
 *        - XXX: how to set ulDropMask, ulDrop, ulStatsPolicerId will not be set
 *     3. In the DA payload field, the ulProcCopy will not be set
 *        - XXX: How do we map the pri field to cos. because the FTE has already
 *               been created. Do we modify the FTE here ?
 */
int
bcm_caladan3_l3_route_add(int unit,
                        bcm_l3_route_t *info)
{
    int                      status=BCM_E_NONE;
    _caladan3_l3_fe_instance_t  *l3_fe;
    bcm_l3_egress_t          bcm_egr;
    uint32                   vid, fte_idx;
    _caladan3_egr_mp_info_t     *mp_info;
    uint32                   ecmp_size;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(unit,
                          "Enter fteHandle 0x%x\n"),
               info->l3a_intf));

    if (info->l3a_flags & ~L3_ROUTE_SUPPORTED_FLAGS) {
        LOG_CLI((BSL_META_U(unit,
                            "%x/%x failed"), info->l3a_subnet, info->l3a_ip_mask));
        return BCM_E_PARAM;
    }

    if (!_bcm_caladan3_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "invalid vrf(0x%x)\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (!(_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, info->l3a_intf))) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "invalid l3 interface: 0x%08x\n"),
                   info->l3a_intf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "vrf(0x%x) not yet created\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    vid     = 0;
    fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(info->l3a_intf);

    ecmp_size = 1;
    if (info->l3a_flags & BCM_L3_MULTIPATH) {
        /*
         *  if flags specify ecmp, make sure that the FTE is indeed mpath
         */
        status = _bcm_caladan3_l3_mpath_get(l3_fe,
                                          fte_idx,
                                          &mp_info);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        ecmp_size = mp_info->num_valid;
    }

    /*
     * In case of RPF, we need to get the VID.
     * Therefore we need to get the info from the egress
     * XXX: Do this to get RpfUnion
     */
    bcm_l3_egress_t_init(&bcm_egr);

    status = bcm_caladan3_g3p1_l3_route_add(l3_fe, info, vid, ecmp_size);

    

    /*
     * restore the user fte handle
     */
    info->l3a_intf =  _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx);
    L3_UNLOCK(unit);

    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_route_delete
 * Purpose:
 *      Delete an entry from the Default IP Routing table.
 * Parameters:
 *      unit  - (IN) FE2000 unit number
 *      info  - Pointer to bcm_l3_route_t structure with valid IP subnet & mask.
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      FTE is not removed
 */
int
bcm_caladan3_l3_route_delete(int unit, bcm_l3_route_t *info)
{
    int                     status = 0;
    _caladan3_l3_fe_instance_t *l3_fe;

    if (!_bcm_caladan3_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid vrf(0x%x)\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "vrf(0x%x) not yet created\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status = _bcm_caladan3_g3p1_ip_route_delete(l3_fe, info);

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_l3_route_get(int unit,
                        bcm_l3_route_t *info)
{
    int                     status=BCM_E_NONE;
    _caladan3_l3_fe_instance_t *l3_fe;
    bcm_l3_egress_t         bcm_egr;
    _caladan3_g3p1_da_route_t   junkDa;
    _caladan3_g3p1_sa_route_t   junkSa;
    

    if (!_bcm_caladan3_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid vrf(0x%x)\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "vrf(0x%x) not yet created\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /*
     * Tgid filled directly into info
     */
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_ip_da_route_get(l3_fe, info, &junkDa);
        
        if (BCM_SUCCESS(status) && (info->l3a_flags & BCM_L3_RPF)) {
            status = _bcm_caladan3_g3p1_ip_sa_route_get(l3_fe, info, &junkSa);
        }
        break;
#endif
    default: 
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    bcm_l3_egress_t_init(&bcm_egr);
    status = 
        _bcm_caladan3_l3_get_egrif_from_fte(l3_fe,
                                          info->l3a_intf,
                                          L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                          &bcm_egr);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    ENET_COPY_MACADDR(bcm_egr.mac_addr, info->l3a_nexthop_mac);
    info->l3a_modid = bcm_egr.module;
    info->l3a_vid   = bcm_egr.vlan;
    if (bcm_egr.flags & BCM_L3_TGID) {
        info->l3a_port_tgid = bcm_egr.trunk;
    } else {
        info->l3a_port_tgid = bcm_egr.port;
    }
    info->l3a_intf =  _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(info->l3a_intf);

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_l3_route_multipath_get(int             unit,
                                  bcm_l3_route_t *info,
                                  bcm_l3_route_t *path_array,
                                  int             max_path,
                                  int            *path_count)
{
    int                          status;
    _caladan3_l3_fe_instance_t      *l3_fe;
    uint32                       base_fte=0;
    bcm_l3_egress_t              bcm_egr;
    _caladan3_egr_mp_info_t         *mp_info;
    uint32                       i;

    status = BCM_E_NONE;

    if ((path_count == NULL) || (info == NULL) ||
        (path_array == NULL) || (max_path <= 0)) {
        return (BCM_E_PARAM);
    }

    if (info->l3a_flags & BCM_L3_RPF) {
        return (BCM_E_PARAM);
    }

    if (!_bcm_caladan3_ip_is_valid(info)      ||
        BCM_IP4_MULTICAST(info->l3a_subnet) ||
        BCM_IP6_MULTICAST(info->l3a_ip6_net)) {
        return BCM_E_PARAM;
    }

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "invalid vrf(0x%x)\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "vrf(0x%x) not yet created\n"),
                   info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /*
     * This is the base FTE of the mpath object. To get the
     * unique set of FTEs, we need the orginal FTEs from which
     * this mpath was derived. So using this base fte, "get" the
     * mpath object. Then for each FTE get the egress info
     */

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_ip_ecmp_route_get(l3_fe, info, &base_fte);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_l3_mpath_get(l3_fe,base_fte, &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (max_path < mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    for (i = 0; i < mp_info->num_valid; i++) {
        bcm_l3_egress_t_init(&bcm_egr);
        status = _bcm_caladan3_l3_get_egrif_from_fte(l3_fe,
                                                   mp_info->mapped_fte[i],
                                                   L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                   &bcm_egr);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }

        path_array->l3a_modid     = bcm_egr.module;
        path_array->l3a_intf      = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mapped_fte[i]);
        ENET_COPY_MACADDR(bcm_egr.mac_addr, path_array->l3a_nexthop_mac);
        path_array++;
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_l3_route_delete_by_interface(int unit,
                                        bcm_l3_route_t *info)
{
    int                     status=BCM_E_NONE;
    _caladan3_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;


    fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(info->l3a_intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!(_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, info->l3a_intf))) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        status = _bcm_caladan3_g3p1_ipv4_route_delete_all(l3_fe,
                                                        fte_idx,
                                                        L3_ROUTE_DELETE_BY_INTF);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_l3_route_delete_all(int unit,
                            bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_caladan3_l3_route_traverse(int unit,
                          uint32 flags,
                          uint32 start,
                          uint32 end,
                          bcm_l3_route_traverse_cb trav_fn,
                          void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_caladan3_l3_route_age(int unit,
                     uint32 flags,
                     bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_caladan3_l3_egress_create
 * Purpose:
 *      Create an Egress forwarding object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                      BCM_L3_WITH_ID: intf argument is given.
 *      egr     - (IN)  Egress forwarding destination.
 *      intf    - (OUT) fte index interface id corresponding to the
 *                      locally allocated FTE pointing to Egress object.
 *                      encap_id containg OHI
 * Assumptions
 *      1. The interface object needs to be created prior to this
 *         call.
 *      2. if trunk field is set, it implies ingress lag is set and
 *         encap_id contains OHI
 *      3. Ignore the vlan field.
 * Implementation Notes:
 *      Case (i):: Module is local
 *         1. Check if the egress object already exists. The interface
 *            object has the list of all egress objects that belong to
 *            it.  <type, mac, vidop, ttl> identify a unique egress object.
 *            There are two cases here
 *            a) Egress object and FTE both exist: Return FTEIdx + OHI
 *            b) Egress object exits but no FTE for local module.
 *                  Allocate FteIdx, populate in SW ETE. Program
 *                  HW with OHI copy in SW ETE. Return FTEIdx + OHI
 *            c) Egress object does not exist
 *         2. Case (c) above. Allocate the following HW resources
 *            a) OHI : In case of ingress LAG (BCM_L3_TGID flag set or
 *                     the trunk field is set), the OHI is passed by the
 *                     user in the encap_id field.
 *                     In some other cases (TBD) also the OHI may be passed
 *                     by user. In such cases we need to use the same. This is
 *                     more likely in the remote module case.
 *            b) ETE Index
 *            c) FTE Index
 *         4. Allocate an egress object and link it to the per intf list of
 *            egress objects. Populate with the HW indices and set up the
 *            OHI and ETE arrays to point to it
 *         5. Program the FTE, OHI and ETE in HW and return the FTE index as
 *            an output param and OHI in encap_id
 *      If there is a tunnel_initiator set pending on the interface for then create the
 *      tunnel ete and point the OHI to the tunnel ETE
 */
int
bcm_caladan3_l3_egress_create(int              unit,
                            uint32           flags,
                            bcm_l3_egress_t *egr,
                            bcm_if_t        *intf)
{
    int                     status;
    _caladan3_l3_fe_instance_t *l3_fe;
    _caladan3_fte_idx_t         fte_idx;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    /*
     * checks common to add/replace
     */
    status = _bcm_caladan3_l3_egress_create_checks(l3_fe, flags, egr, intf);

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * interface id is required in either  case. We made sure that
     * a valid user interface handle is passed in the check routines
     * Now we need to convert it to the internal representation
     */
    egr->intf =  _CALADAN3_IFID_FROM_USER_HANDLE(egr->intf);

    fte_idx.fte_idx = 0;

    if (flags & BCM_L3_REPLACE) {
        fte_idx.fte_idx = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(*intf);
        status = _bcm_caladan3_update_l3_fte(l3_fe, flags, egr, &fte_idx);
    } else {
        if (flags & BCM_L3_WITH_ID) {
            fte_idx.fte_idx = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(*intf);
        }
        status = _bcm_caladan3_add_l3_fte(l3_fe, flags, egr, &fte_idx);
    }

    egr->intf =  _CALADAN3_USER_HANDLE_FROM_IFID(egr->intf);
    if (status == BCM_E_NONE) {
        *intf = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx.fte_idx);
    }

    LOG_VERBOSE(BSL_LS_BCM_L3,
                (BSL_META_U(l3_fe->fe_unit,
                            "fte(0x%x) encap-id(0x%x)\n"),
                 *intf, egr->encap_id));

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_destroy
 * Purpose:
 *      Destroy an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN)  FTE index pointing to Egress object.
 * Returns:
 *      BCM_E_BUSY - Some other FTEs still using this object
 *
 * Assumptions:
 *      The egress object (FTE-ETE pair) is not being used by
 *      any multipath group.
 *
 * Implementation Notes:
 *      1. Fetch the contents of FTE and get node(module) and OHI from HW
 *      2. Module is local
 *         i)   Get SW ete from OHI.
 *         iii) There are remote FTEs present:
 *                 remove local FTE from SW ete
 *                 Deprogram local and return FTE to allocator
 *         iii) There are no remote FTEs using the ete
 *                 Deprogram  HW ete, HW OHI and HW FTE and free the
 *                 corresponding indices in the allocators
 *                 Remove SW ete structure from interface list
 *                 Clean up SW ETEptr array and OHI2ETE array
 *          iv) If the ete is being removed and this is the last ete
 *              on the interface, then remove the tunnel ete as well.
 */
int
bcm_caladan3_l3_egress_destroy(int      unit,
                             bcm_if_t intf)
{
    int                              status;
    _caladan3_l3_fe_instance_t          *l3_fe;
    uint32                           fte_idx;
    _caladan3_l3_fte_t                  *fte_hash_elem;

    LOG_DEBUG(BSL_LS_BCM_L3,
              (BSL_META_U(unit,
                          "Enter\n")));

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!(_CALADAN3_L3_FTE_VALID(l3_fe->fe_unit, intf))) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_caladan3_l3_get_sw_fte(l3_fe,
                                       fte_idx,
                                       &fte_hash_elem);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "fte 0x%x not found in hash table\n"),
                   fte_idx));
        L3_UNLOCK(unit);
        return status;
    }
    if (fte_hash_elem->ref_count > 1) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "fte 0x%x in use"),
                   fte_idx));
        L3_UNLOCK(unit);
        return BCM_E_BUSY;
    }
    status =  _bcm_caladan3_destroy_fte(l3_fe,
                                      L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE,
                                      fte_idx,
                                      0, /* mod */
                                      0  /* ohi */);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_get
 * Purpose:
 *      Get an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 fteIdx retruned at egress create time
 *      egr     - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 *
 * Implementation Notes:
 *
 *     1. Fetch the contents of FTE and get node(module) and OHI from HW
 *     2. Module is local.
 *        Get the SW ete pointer from OHI. From the SW ete ptr get HW eteIdx
 *        Based on HW eteIdx, fetch the contents of ete.
 *     3. Module is remote: pass the OHI to remote module.
 */
int
bcm_caladan3_l3_egress_get(int              unit,
                         bcm_if_t         intf,
                         bcm_l3_egress_t *egr)
{
    int                     status = BCM_E_NONE;
    _caladan3_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if (!(_CALADAN3_L3_FTE_VALID(unit, intf)) || (egr == NULL)) {
        status = BCM_E_PARAM;
    } else {
        l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
        if (l3_fe == NULL) {
            status = BCM_E_UNIT;

        } else {
            fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf);
            bcm_l3_egress_t_init(egr);
            status = _bcm_caladan3_l3_get_egrif_from_fte(l3_fe,fte_idx,
                                                       L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                       egr);
            egr->intf     =  _CALADAN3_USER_HANDLE_FROM_IFID(egr->intf);
        }
    }
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_find
 * Purpose:
 *      Find an egress forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      egr        - (IN) Egress object properties to match.
 *                        l3a_intf_id, module, dmac and port
 *      intf       - (OUT) FteIdx if found
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 *
 * Implementation Notes:
 *      1. From the module determine the node, which could be
 *         local or remote.
 *      2. For local node,
 *         i)  determine the l3_intf from the intf_id
 *         ii) Walk the list of egress objects on the intf matching for
 *             dmac and port.
 *         iii)On a match, walk locate the fte by in the <modid, fte>
 *             array by matching the modid.
 *      3. For remote node
 *         Same as above, except when locating the fte in the egress
 *         object , we need to match on the calling units modid.
 *
 */
int
bcm_caladan3_l3_egress_find(int              unit,
                          bcm_l3_egress_t *egr,
                          bcm_if_t        *intf)
{
    int                     status;
    _caladan3_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;
    bcm_l3_egress_t         bcm_egr;

    if (egr == NULL || intf == NULL) {
        return BCM_E_PARAM;
    }
    if (!(_CALADAN3_L3_USER_IFID_VALID(egr->intf))) {
        return (BCM_E_PARAM);
    }

    if ((BCM_MAC_IS_MCAST(egr->mac_addr)) ||
        (BCM_MAC_IS_ZERO(egr->mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!(SOC_SBX_MODID_ADDRESSABLE(unit, egr->module))) {
        return (BCM_E_PARAM);
    }

    if (egr->encap_id && !SOC_SBX_IS_VALID_ENCAP_ID(egr->encap_id)) {
            return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (egr->flags & BCM_L3_TGID) {
        if (!SBX_TRUNK_VALID(egr->trunk)) {
            L3_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    } else if ((egr->module == l3_fe->fe_my_modid) &&
               (!(SOC_PORT_VALID(unit, egr->port)))) {
        L3_UNLOCK(unit);
        return (BCM_E_PARAM);
    }

    bcm_egr          = *egr;
    bcm_egr.intf     =  _CALADAN3_IFID_FROM_USER_HANDLE(egr->intf);
    bcm_egr.encap_id =  egr->encap_id;

    status = _bcm_caladan3_find_fte(l3_fe,
                                  &bcm_egr,
                                  &fte_idx);
    if (status == BCM_E_NONE) {
        *intf = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx);
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_caladan3_l3_egress_traverse(int                     unit,
                           bcm_l3_egress_traverse_cb  trav_fn,
                           void                      *user_data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_caladan3_l3_route_max_ecmp_set
 * Purpose:
 *    Set the maximum ECMP paths allowed for a route
 * Parameters:
 *    unit       - (IN) bcm device.
 *    max        - (IN) MAX number of paths for ECMP
 * Returns:
 *      BCM_E_XXX
 * Note:
 *    This function can be called before ECMP routes are added,
 *    normally at the beginning.  Once ECMP routes exist, cannot be reset.
 */
int
bcm_caladan3_l3_route_max_ecmp_set(int unit, int max)
{
    int                      status;
    _caladan3_l3_fe_instance_t  *l3_fe;

    if (max >= _CALADAN3_L3_ECMP_MAX) {
        return  BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;
    if (l3_fe->fe_flags & _CALADAN3_L3_FE_FLG_MP_SIZE_SET) {
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(l3_fe->fe_unit,
                             "Cannot change ecmp size, Multipath routes exist\n")));
        status = BCM_E_EXISTS;
    } else {
        l3_fe->fe_mp_set_size = max;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *    bcm_caladan3_l3_route_max_ecmp_get
 * Purpose:
 *    Get the maximum ECMP paths allowed for a route
 * Parameters:
 *    unit       - (IN) bcm device.
 *    max        - (OUT) MAX number of paths for ECMP
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_l3_route_max_ecmp_get(int unit, int *max)
{
    int                      status;
    _caladan3_l3_fe_instance_t  *l3_fe;

    if (max == NULL) {
        return  BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;
    *max   = l3_fe->fe_mp_set_size;

    L3_UNLOCK(unit);
    return status;
}

/* Function:
 *      bcm_caladan3_l3_egress_multipath_create
 * Purpose:
 *      Create an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      flags      - (IN) BCM_L3_REPLACE: replace existing.
 *                        BCM_L3_WITH_ID: mpintf argument is given.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of Egress forwarding objects. (FteIndices)
 *      mpintf     - (OUT) BaseFteIndex of the newly allocated mpath Group.
 *                         This is an IN argument if either BCM_L3_REPLACE
 *                         or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      1. Sort the FteIndices passed in and work with the sorted list
 *      2. Case I: REPLACE flag not set
 *         i) lookup the list of ftes in the hash table. If found
 *         return error
 *         ii) allocate a contigous set of intf_count ftes (beginning
 *             with mpintf if BCM_L3_WITH_ID if given) .
 *             The first in the array is the base fte that will be
 *             returned in mpintf
 *         iii) For each mapped fte in the list, get HW info and clone
 *              a new Fte.
 *         iv) Insert the list of mapped FTEs in a the hash table.
 *      3. Case II: REPLACE flag set
 *         i)  The FTE list must be in hash table
 *         ii) From mpintf fetch each old fte index and deprogram HW
 *         iii)Clone each FTE in the group as above
 */
int
bcm_caladan3_l3_egress_multipath_create(int       unit,
                                      uint32    flags,
                                      int       intf_count,
                                      bcm_if_t *intf_array,
                                      bcm_if_t *mpintf)
{
    int                          status;
    _caladan3_l3_fe_instance_t      *l3_fe;
    _caladan3_l3_mpath_crc_info_t   *crc_info;
    _caladan3_egr_mp_info_t         *mp_info;
    uint32                       i;
    uint32                       mpbase;
    bcm_if_t                     new_ftes[_CALADAN3_L3_ECMP_MAX];

    if ((intf_array == NULL) || (mpintf == NULL)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if ((intf_count < 0) ||
        (intf_count > l3_fe->fe_mp_set_size) ||
        (intf_count > _CALADAN3_L3_ECMP_MAX)) {
        L3_UNLOCK(unit);
        return  BCM_E_PARAM;
    }

    /*
     * Validate and translate the FTEs
     */
    for (i = 0; i < intf_count; i++) {
        if (!(_CALADAN3_L3_FTE_VALID(unit, intf_array[i]))) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        new_ftes[i] =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf_array[i]);
    }

    if ((flags & BCM_L3_REPLACE) || (flags & BCM_L3_WITH_ID)) {
        /*
         * in either case mpintf must contain a valid FTE
         */
        if (!_CALADAN3_L3_FTE_VALID(unit, *mpintf)) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
    }

    /*
     * crc info contains all the FTEs in sorted order and
     * the calculated CRC16 for hashing
     */
    status   = BCM_E_NONE;
    crc_info = NULL;

    if (flags & BCM_L3_REPLACE) {
        /*
         * case:: update mpath
         */
        mpbase = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(*mpintf);

        /*
         * The mpath object is returned in mp_info
         */
        status = _bcm_caladan3_l3_mpath_get(l3_fe,
                                          mpbase,
                                          &mp_info);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Multipath object (index = %d) to"
                                   " Replace not found\n"),
                       *mpintf));
            L3_UNLOCK(unit);
            return  status;
        }

        status = _bcm_caladan3_handle_mpath_update(l3_fe,
                                                 mp_info,    /* existing mpath object */
                                                 intf_count,
                                                 new_ftes);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    } else {
        /*
         * case:: new mpath
         */
        if (flags & BCM_L3_WITH_ID) {
            /*
             * do we have the full range of fte's available ?
             */
            mpbase =   _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(*mpintf);

            for (i = mpbase; i < (mpbase+l3_fe->fe_mp_set_size); i++) {
                status = _sbx_caladan3_resource_test(unit,
                                                SBX_CALADAN3_USR_RES_FTE_L3_MPATH,
                                                i);
                if (status != BCM_E_NOT_FOUND) {
                    LOG_ERROR(BSL_LS_BCM_L3,
                              (BSL_META_U(l3_fe->fe_unit,
                                          "FTE Index Range (%d - %d) in use\n"),
                               mpbase,
                               mpbase + l3_fe->fe_mp_set_size - 1));
                    L3_UNLOCK(unit);
                    return BCM_E_EXISTS;
                }
            }
        }
        else {
            mpbase = _CALADAN3_INVALID_FTE;
        }

        crc_info = sal_alloc(sizeof(_caladan3_l3_mpath_crc_info_t), "mpath-crc-info");
        if (crc_info == NULL) {
            L3_UNLOCK(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(crc_info, 0, sizeof(_caladan3_l3_mpath_crc_info_t));
        status = _bcm_caladan3_l3_gen_mpath_crc(l3_fe,
                                              mpbase,
                                              intf_count,
                                              new_ftes,
                                              crc_info,
                                              L3_CRC_INFO_FETCH_FTE_DATA);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "error in setting up Mpath Ftes: %s\n"),
                       bcm_errmsg(status)));
            sal_free(crc_info);
            crc_info = NULL;
            L3_UNLOCK(unit);
            return status;
        }

        /*
         * mpbase is either reserved or allocated based on
         * BCM_L3_WITH_ID flag inside add_mpath_object.
         */
        status = _bcm_caladan3_l3_add_mpath_object(l3_fe,
                                                 flags,
                                                 crc_info,
                                                 &mp_info);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            sal_free(crc_info);
            crc_info = NULL;
            return status;
        }
    }

    l3_fe->fe_flags |= _CALADAN3_L3_FE_FLG_MP_SIZE_SET;
    *mpintf =  _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mp_base);

    if (crc_info) {
        _bcm_caladan3_l3_free_mpath_crc(crc_info, intf_count);
        crc_info = NULL;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_multipath_destroy
 * Purpose:
 *      Destroy an Egress Multipath forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      Free the Derived FTEs and remove from Hash table and
 *      unlink from the mp_base2info array
 */
int
bcm_caladan3_l3_egress_multipath_destroy(int unit,
                                       bcm_if_t mpintf)
{
    int                          status = 0;
    _caladan3_l3_fe_instance_t      *l3_fe;
    _caladan3_egr_mp_info_t         *mp_info;
    uint32                       i, mp_base, count;
    _caladan3_l3_fte_t              *fte_hash_elems[_CALADAN3_L3_ECMP_MAX];
    _caladan3_l3_fte_t              *mp_base_hash_elem;

    if (!_CALADAN3_L3_FTE_VALID(unit, mpintf)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    mp_base =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);

    status = _bcm_caladan3_l3_mpath_get(l3_fe,
                                      mp_base,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return  status;
    }

    /*
     * Make sure that the ref count on the base fte is not > 1
     */
    status = _bcm_caladan3_l3_get_sw_fte(l3_fe,
                                       mp_base,
                                       &mp_base_hash_elem);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "fte idx 0x%x not found in hash table\n"),
                   mp_base));
        L3_UNLOCK(unit);
        return status;
    } else {
        /*
         * if the base fte has a ref count more than one -- in use error
         */
        if (mp_base_hash_elem->ref_count > 1) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "fte idx 0x%x has a refcount of %d\n"),
                       mp_base, mp_base_hash_elem->ref_count));
             L3_UNLOCK(unit);
            return BCM_E_BUSY;
        }
    }

    /*
     * save the fte hash elemnts for the mapped ftes. This
     * info will be used to decr the ref_counts later on.
     * We do this now so that we can bail out before we start
     * mucking with the ftes
     */
    for (i = 0; i < _CALADAN3_L3_ECMP_MAX; i++) {
        fte_hash_elems[i] = NULL;
    }
    count  = mp_info->num_valid;
    status = _bcm_caladan3_l3_save_fte_hash_elements(l3_fe,
                                                   count,
                                                   mp_info->mapped_fte,
                                                   fte_hash_elems);

    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "mapped ftes not found in hash table: %s\n"),
                   bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }
    for (i = 0; i < count; i++) {
        /*
         * The original FTEs should have a ref_count of atleast 2
         * On creation we have ref_count = 1 and when the mpath
         * was created it should have been incremeted by 1
         */
        if ((fte_hash_elems[i])->ref_count < 2) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "Invalid refCount %d on mapped fte 0x%x\n"),
                       (fte_hash_elems[i])->ref_count,
                       mp_info->mapped_fte[i]));
            L3_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
    }

    status = _bcm_caladan3_l3_destroy_mpath_object(l3_fe,
                                                 &mp_info);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not destroy multipath object %d: %s\n"),
                   mpintf, bcm_errmsg(status)));
    }

    /*
     * Now decrement the ref counts on the mapped ftes
     */
    for (i = 0; i < count; i++) {
        (fte_hash_elems[i])->ref_count--;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_multipath_get
 * Purpose:
 *      Get an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      mpintf     - (IN) BaseFteIndex that was returned at create time
 *      intf_size  - (IN) Size of allocated entries in intf_array.
 *      intf_array - (OUT) Array of mapped FTE indices
 *      intf_count - (OUT) Number of entries of intf_count actually filled in.
 *
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      Using mpintf and the mp_base2info array, get the mpath group
 *      This contains a list of mapped FTEs that are returned
 */
int
bcm_caladan3_l3_egress_multipath_get(int       unit,
                                   bcm_if_t  mpintf,
                                   int       intf_size,
                                   bcm_if_t *intf_array,
                                   int      *intf_count)
{
    int                           status;
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_egr_mp_info_t          *mp_info;
    int                           i;
    uint32                        mp_base;

    if ((intf_array == NULL) || (intf_count == NULL) || (intf_size == 0)) {
        return BCM_E_PARAM;
    }

    if (!_CALADAN3_L3_FTE_VALID(unit, mpintf)) {
        return BCM_E_PARAM;
    }

    mp_base =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }
    mp_info = NULL;
    status  = _bcm_caladan3_l3_mpath_get(l3_fe,
                                       mp_base,
                                       &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (intf_size < mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *intf_count = mp_info->num_valid;
    for (i = 0; i < mp_info->num_valid; i++) {
        intf_array[i] = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mapped_fte[i]);
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_multipath_add
 * Purpose:
 *      Add an Egress forwarding object to an Egress Multipath
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex that was returned at create time
 *      intf    - (IN) A new mapped FteIndex that needs to be added
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      1. Fetch the mpath group based on mpath_intf
 *      2. Check to make sure that we are within the MAX_ECMP limit.
 *      3. remove the group from egr_mp hash.
 *      4. allocate a new FTE and clone the mapped FTE.
 *      5. Re-program the entire FTE set in HW because the FTE arrangement
 *         in the set may have changed. For example if MAX_ECMP was 4 and
 *         the group had two FTEs (A & B) then the HW FTEs would be {A B A B}
 *         With the addition of a new FTE the pattern must become {A B C A}
 *      6. Add the new FTE to the group, recompute hash and insert back
 */
int
bcm_caladan3_l3_egress_multipath_add(int unit,
                                   bcm_if_t mpintf,
                                   bcm_if_t intf)
{
    int                          status, ignore_status;
    _caladan3_egr_mp_info_t         *mp_info;
    _caladan3_l3_fe_instance_t      *l3_fe;
    uint32                       mpbase, new_fte_idx;
    bcm_if_t                     new_ftes[_CALADAN3_L3_ECMP_MAX];
    uint32                       i;

    if (!_CALADAN3_L3_FTE_VALID(unit, mpintf) ||
        !_CALADAN3_L3_FTE_VALID(unit, intf)) {
          return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    status = ignore_status = BCM_E_NONE;

    mpbase      =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);
    new_fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_caladan3_l3_mpath_get(l3_fe,
                                      mpbase,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not find mpath object with base 0x%x: %s\n"),
                   mpintf, bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return  status;
    }

    if (mp_info->num_valid == l3_fe->fe_mp_set_size) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Max ECMP set size (%d) exceeded\n"),
                   l3_fe->fe_mp_set_size));
        L3_UNLOCK(unit);
        return BCM_E_FULL;
    }

    for (i = 0; i < mp_info->num_valid; i++) {
        new_ftes[i] = mp_info->mapped_fte[i];
    }
    new_ftes[mp_info->num_valid] = new_fte_idx;

    status = _bcm_caladan3_handle_mpath_update(l3_fe,
                                             mp_info,
                                             mp_info->num_valid+1,
                                             new_ftes);

    L3_UNLOCK(unit);
    return  status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_multipath_delete
 * Purpose:
 *      Delete an Egress forwarding object to an Egress Multipath
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex that was returned at create time
 *      intf    - (IN) A mapped FteIndex that needs to be deleted
 * Returns:
 *      BCM_E_XXX
 * Implementation Notes:
 *      Follows logic similar to  bcm_caladan3_l3_egress_multipath_add
 *      except that the mapped FTE is deleted from the set
 */
int
bcm_caladan3_l3_egress_multipath_delete(int unit,
                                   bcm_if_t mpintf,
                                   bcm_if_t intf)
{
    int                          status;
    _caladan3_egr_mp_info_t         *mp_info;
    _caladan3_l3_fe_instance_t      *l3_fe;
    uint32                       mpbase, del_fte_idx;
    bcm_if_t                     new_ftes[_CALADAN3_L3_ECMP_MAX];
    uint32                       i, new_num_valid;

    if (!_CALADAN3_L3_FTE_VALID(unit, mpintf) ||
        !_CALADAN3_L3_FTE_VALID(unit, intf)) {
          return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;

    mpbase      =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);
    del_fte_idx =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_caladan3_l3_mpath_get(l3_fe,
                                      mpbase,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Could not find mpath object with base %d"),
                   mpintf));
        L3_UNLOCK(unit);
        return  status;
    }

    /*
     * remove the deleted fte
     */
    for (i = 0, new_num_valid = 0; i < mp_info->num_valid; i++) {
        if (mp_info->mapped_fte[i] != del_fte_idx) {
            new_ftes[new_num_valid] = mp_info->mapped_fte[i];
            new_num_valid++;
        }
    }

    if (new_num_valid == mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    status = _bcm_caladan3_handle_mpath_update(l3_fe,
                                             mp_info,
                                             new_num_valid,
                                             new_ftes);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "error in updating _mpath_ftes: %s\n"), 
                   bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return  status;
}

/*
 * Function:
 *      bcm_caladan3_l3_egress_multipath_find
 * Purpose:
 *      Find an egress multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of egress forwarding objects.
 *      mpintf     - (OUT) base fte of mpath object
 *
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_l3_egress_multipath_find(int       unit,
                                    int       intf_count,
                                    bcm_if_t *intf_array,
                                    bcm_if_t *mpintf)
{
    int                           status;
    _caladan3_l3_fe_instance_t       *l3_fe;
    _caladan3_egr_mp_info_t          *mp_info;
    int                           i;
    _caladan3_l3_mpath_crc_info_t    *crc_info;
    bcm_if_t                      ftes[_CALADAN3_L3_ECMP_MAX];

    if ((mpintf == NULL) || (intf_array == NULL) || (intf_count == 0)) {
        return BCM_E_PARAM;
    }


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (intf_count > l3_fe->fe_mp_set_size) {
        L3_UNLOCK(unit);
        return  BCM_E_PARAM;
    }

    /*
     * Validate and translate the FTEs
     */
    for (i = 0; i < intf_count; i++) {
        if (!_CALADAN3_L3_FTE_VALID(unit, intf_array[i])) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        ftes[i] =  _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf_array[i]);
    }

    crc_info = sal_alloc(sizeof(_caladan3_l3_mpath_crc_info_t), "mpath-crc-info");
    if (crc_info == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(crc_info, 0, sizeof(_caladan3_l3_mpath_crc_info_t));

    status = _bcm_caladan3_l3_gen_mpath_crc(l3_fe,
                                          _CALADAN3_INVALID_FTE,
                                          intf_count,
                                          ftes,
                                          crc_info,
                                          L3_CRC_INFO_DONT_FETCH_FTE_DATA);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(l3_fe->fe_unit,
                              "Error in setting up Mpath Ftes:  %s\n"),
                   bcm_errmsg(status)));
        L3_UNLOCK(unit);
        sal_free(crc_info);
        return  BCM_E_PARAM;
    }
    status = _bcm_caladan3_l3_mpath_find(l3_fe,
                                       crc_info,
                                       &mp_info);

    if (status == BCM_E_NONE) {
        *mpintf = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mp_base);
    }

    L3_UNLOCK(unit);
    _bcm_caladan3_l3_free_mpath_crc(crc_info, intf_count);
    return status;
}


/* VRRP APIs */
int
bcm_caladan3_l3_vrrp_add(int unit,
                       bcm_vlan_t vlan,
                       uint32 vrid)
{
    soc_sbx_g3p1_vrid2e_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (vrid == 0) {
        return BCM_E_PARAM;
    }
    status = soc_sbx_g3p1_vrid2e_get(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_get"), 
                   bcm_errmsg(status)));
        return status;
    }

    /* check whether the vrid is already in the entry */
    if (v2e1.v4_vrid0 == vrid)    return BCM_E_EXISTS;
    if (v2e1.v4_vrid1 == vrid)    return BCM_E_EXISTS;
    if (v2e1.v4_vrid0 == 0) {
        v2e1.v4_vrid0 = vrid;
        v2e1.v6_vrid0 = vrid;
    } else if (v2e1.v4_vrid1 == 0) {
        v2e1.v4_vrid1 = vrid;
        v2e1.v6_vrid1 = vrid;
    } else {
      return BCM_E_RESOURCE;
    }

    status = soc_sbx_g3p1_vrid2e_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_set"),
                   bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_l3_vrrp_delete(int unit,
                       bcm_vlan_t vlan,
                       uint32 vrid)
{
    soc_sbx_g3p1_vrid2e_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (vrid == 0) {
        return BCM_E_PARAM;
    }
    status = soc_sbx_g3p1_vrid2e_get(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_get"),
                   bcm_errmsg(status)));
        return status;
    }
    /* check whether the vrid is already in the entry */
    if(v2e1.v4_vrid0 == vrid) {
        v2e1.v4_vrid0 = 0;
        v2e1.v6_vrid0 = 0;
    } else if (v2e1.v4_vrid1 == vrid) {
        v2e1.v4_vrid1 = 0;
        v2e1.v6_vrid1 = 0;
    } else return BCM_E_NONE;

    status = soc_sbx_g3p1_vrid2e_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_set"),
                   bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_l3_vrrp_delete_all(int unit,
                      bcm_vlan_t vlan)
{
    soc_sbx_g3p1_vrid2e_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    v2e1.v4_vrid0 = 0;
    v2e1.v4_vrid1 = 0;
    v2e1.v6_vrid0 = 0;
    v2e1.v6_vrid1 = 0;
    status = soc_sbx_g3p1_vrid2e_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_set"),
                   bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_caladan3_l3_vrrp_get(int unit,
                       bcm_vlan_t vlan,
                       int alloc_size,
                       int * vrid_array,
                       int * count)
{
    soc_sbx_g3p1_vrid2e_t v2e1;
    int status, vrid_count;

    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (alloc_size < 4) {
        return BCM_E_PARAM;
    }
    if (vrid_array == NULL) {
        return BCM_E_PARAM;
    }
    if (count == NULL) {
        return BCM_E_PARAM;
    }

    status = soc_sbx_g3p1_vrid2e_get (unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L3,
                  (BSL_META_U(unit,
                              "error %s in soc_sbx_g3p1_vrid2e_get"),
                   bcm_errmsg(status)));
        return status;
    }

    vrid_count = 0;
    if (v2e1.v4_vrid0 != 0) {
        vrid_array[vrid_count] = v2e1.v4_vrid0;
        vrid_count++;
    }
    if (v2e1.v4_vrid1 != 0) {
        vrid_array[vrid_count] = v2e1.v4_vrid1;
        vrid_count++;
    }
    *count = vrid_count;

    return BCM_E_NONE;
}
/* VRRP APIs END */


int
bcm_caladan3_l3_egress_multipath_traverse(int unit,
                                        bcm_l3_egress_multipath_traverse_cb trav_fn,
                                        void *user_data)
{
    return BCM_E_UNAVAIL;
}


/* Provided an L3 Interface, reference it. Typically used in cases as 
 * associating OAM on mpls tunnel interface. This will ensure the mpls
 * tunnel is not deleted before the OAM is disassociated 
 * reference > 0 -> References the interface
 * reference <= 0 -> Dereferences the interface
 */
int 
_bcm_caladan3_l3_reference_interface(int unit, bcm_if_t intf, int reference)
{
    _caladan3_l3_fe_instance_t *l3_fe;    
    _caladan3_l3_fte_t         *fte_hash_elem;
    int status = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
        status = BCM_E_UNIT;

    } else {
        status = _bcm_caladan3_l3_get_sw_fte(l3_fe, 
                                           _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf),
                                           &fte_hash_elem);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_L3,
                      (BSL_META_U(l3_fe->fe_unit,
                                  "fte_idx 0x%x not found in hash table\n"), 
                       _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf)));
            status = BCM_E_NOT_FOUND;

        } else {
            if (reference > 0) {
                fte_hash_elem->ref_count++;
            } else {
                fte_hash_elem->ref_count--;
            }

            LOG_VERBOSE(BSL_LS_BCM_L3,
                        (BSL_META_U(l3_fe->fe_unit,
                                    "L3 Interface 0x%x fte_idx 0x%x referenced Count[%d]\n"), 
                         intf,_CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(intf), 
                         fte_hash_elem->ref_count));
        }
    }

    L3_UNLOCK(unit);
    return status;
}

#endif  /* INCLUDE_L3 */


