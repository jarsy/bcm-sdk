/*
 * $Id: l2cache.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * L2 Cache - Layer 2 control protocols support
 */

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/stack.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/cosq.h>
#include <bcm/pkt.h>

#include <soc/sbx/sbx_drv.h>

#include <shared/idxres_fl.h>
#include <shared/gport.h>

#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <bcm_int/sbx/error.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/l2cache.h>
#include <bcm_int/sbx/caladan3/wb_db_l2cache.h>
#include <bcm_int/sbx/l2.h>

_l2_cache_t _l2_cache[BCM_MAX_NUM_UNITS];
static sal_mutex_t _l2_cache_glob_lock; /* glob l2 cache lock */
static uint32 max_l2cp_types=0;
static uint32 max_l2cp_subtypes=0;

#define L2CACHE_L2CPSLOW_BASE (SBX_MAX_PORTS * L2CACHE_L2CP_TYPES)

#define L2CACHE_PORT_LSB_TO_INDEX(p,lsb) ((p * L2CACHE_L2CP_TYPES) + lsb)
#define L2CACHE_PORT_SUBTYPE_TO_INDEX(p,st) (L2CACHE_L2CPSLOW_BASE + (p * L2CACHE_L2CP_SUBTYPES) + st)

#define L2CACHE_INDEX_TO_PORT(idx) \
    ( (idx < L2CACHE_L2CPSLOW_BASE) ? (idx / L2CACHE_L2CP_TYPES) : ((idx - L2CACHE_L2CPSLOW_BASE)/ L2CACHE_L2CP_SUBTYPES) )
#define L2CACHE_INDEX_TO_SUBTYPE(idx) \
    ((idx < L2CACHE_L2CPSLOW_BASE) ? 0xff : ( (idx - L2CACHE_L2CPSLOW_BASE) % L2CACHE_L2CP_SUBTYPES) )
#define L2CACHE_INDEX_TO_DMACLSB(idx) \
    ((idx < L2CACHE_L2CPSLOW_BASE) ? (idx % L2CACHE_L2CP_TYPES) : 2)

#ifdef WARM_BOOT_TODO
#include <bcm/module.h>

typedef struct caladan3_l2cache_mac_entry_s {
    uint16 vlan;
    bcm_mac_t mac;
}caladan3_l2cache_mac_entry_t;

/* Utilize invalid entries in L2CP Slow table to store WB info */
#define L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE 11


#define L2CACHE_WB_MAX_MAC_ENTRIES (L2CACHE_MAX_IDX_DEFAULT - L2CACHE_MAX_L2CP_IDX)
typedef struct caladan3_l2cache_wb_mem_layout_s {
    uint16 flags[L2CACHE_MAX_IDX_DEFAULT];
    caladan3_l2cache_mac_entry_t mac_entry[L2CACHE_WB_MAX_MAC_ENTRIES];
}caladan3_l2cache_wb_mem_layout_t;

#define L2CACHE_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define L2CACHE_WB_CURRENT_VERSION            L2CACHE_WB_VERSION_1_0

/* Is Level 2 warmboot configured - */
#define L2CACHE_WB_L2_CONFIGURED(unit) (_l2_cache[unit].scache_size != 0)

#endif 

#define L2CACHE_UNIT_CHECK(unit) \
do { \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        LOG_ERROR(BSL_LS_BCM_L2, \
                  (BSL_META_U(unit, \
                              "%s: Invalid unit \n"), FUNCTION_NAME())); \
        return BCM_E_UNIT; \
    } \
} while (0)

#define L2CACHE_INIT_CHECK(unit) \
do { \
    L2CACHE_UNIT_CHECK(unit); \
    if (!_l2_cache[unit].lock) { \
        LOG_ERROR(BSL_LS_BCM_L2, \
                  (BSL_META_U(unit, \
                              "%s: l2_cache unitialized on unit:%d \n"), \
                   FUNCTION_NAME(), unit)); \
        return BCM_E_INIT; \
    } \
} while (0)

#define L2CACHE_UNIT_LOCK(unit) \
do { \
    if (sal_mutex_take(_l2_cache[unit].lock, sal_mutex_FOREVER)) { \
        LOG_ERROR(BSL_LS_BCM_L2, \
                  (BSL_META_U(unit, \
                              "%s: sal_mutex_take failed for unit %d. \n"), \
                   FUNCTION_NAME(), unit)); \
        return BCM_E_INTERNAL; \
    } \
} while (0)

#define L2CACHE_UNIT_UNLOCK(unit) \
do { \
    if (sal_mutex_give(_l2_cache[unit].lock)) { \
        LOG_ERROR(BSL_LS_BCM_L2, \
                  (BSL_META_U(unit, \
                              "%s: sal_mutex_give failed for unit %d. \n"), \
                   FUNCTION_NAME(), unit)); \
        return BCM_E_INTERNAL; \
    } \
} while (0)

/* check if the l2cpmac is zero */
#define L2CPMAC_NULL(l2cpmac, result) \
do { \
    result = 0; \
    if ((!l2cpmac.mac[0]) && (!l2cpmac.mac[1]) && (!l2cpmac.mac[2]) && \
        (!l2cpmac.mac[3]) && (!l2cpmac.mac[4]) && (!l2cpmac.mac[5])) { \
        result = 1; \
    } \
} while (0)

/* copy from dmac to l2cpmac */
#define MAC_TO_L2CPMAC(dmac, l2cpmac) \
do { \
    l2cpmac.mac[0] = dmac[0]; \
    l2cpmac.mac[1] = dmac[1]; \
    l2cpmac.mac[2] = dmac[2]; \
    l2cpmac.mac[3] = dmac[3]; \
    l2cpmac.mac[4] = dmac[4]; \
    l2cpmac.mac[5] = dmac[5]; \
} while (0)


#ifdef WARM_BOOT_TODO
static int
_bcm_l2cache_wb_layout_get(int unit, caladan3_l2cache_wb_mem_layout_t **layout)
{
    int rv;
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    rv = soc_scache_ptr_get(unit, _l2_cache[unit].wb_hdl, 
                            (uint8**)&wbc, &size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }
    *layout = (caladan3_l2cache_wb_mem_layout_t *)wbc->cache;
    return rv;
}
#endif 

#ifdef BCM_WARM_BOOT_SUPPORT
int bcm_sbx_caladan3_l2_cache_state_dump(int unit)
{
    int                 rv = BCM_E_NONE;
    int                 index = 0;
    _l2_cache_entry     *entry;
    uint32 first=0,last=0,valid_low=0,valid_high=0,free_count=0,alloc_count=0;

    L2CACHE_UNIT_CHECK(unit);
    for (index=0; index<L2CACHE_MAX_IDX; index++) {
        L2CACHE_ENTRY_GET_RV(unit, index, entry, rv);
        if (!entry || (rv == BCM_E_PARAM)) {
            break;
        }

        if (entry->flags & L2CACHE_ENTRY_VALID) {
            LOG_INFO(BSL_LS_BCM_L2,
                     (BSL_META_U(unit,
                                 "l2cache entry index: 0x%x flags:0x%08x, cookie: 0x%x, fte_idx: 0x%x, encap_id: 0x%x, src_port: 0x%x\n"), 
                index, entry->flags, entry->cookie, entry->fte_idx, entry->encap_id, entry->src_port));
        }

    }
    LOG_INFO(BSL_LS_BCM_L2,
             (BSL_META_U(unit,
                         "_l2_cache pass_thru_ohi 0x%x, cpu_fte 0x%x, l2_ete 0x%x, _l2cpmac_table 0x%x, max_idx 0x%x max_l2cp_types 0x%x, max_l2cp_subtypes 0x%x\n"), 
              _l2_cache[unit].pass_thru_ohi, _l2_cache[unit].cpu_fte, _l2_cache[unit].l2_ete, _l2_cache[unit]._l2cpmac_table, _l2_cache[unit].max_idx, max_l2cp_types, max_l2cp_subtypes));
   
    rv = shr_idxres_list_state(_l2_cache[unit].idlist, &first, &last, &valid_low, &valid_high, &free_count, &alloc_count);
    LOG_INFO(BSL_LS_BCM_L2,
             (BSL_META_U(unit,
                         "_l2_cache idlist first=0x%x,last=0x%x,valid_low=0x%x,valid_high=0x%x,free_count=0x%x alloc_count=0x%x\n"),
              first, last, valid_low, valid_high, free_count, alloc_count));
    return rv;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

int
_bcm_caladan3_g3p1_l2cpmac_alloc(int unit, bcm_mac_t mac, 
                               int *index_used, int verify)
{
    int                     rv = BCM_E_NONE, l2cpmac_idx;
    int                     max_l2cpmac_idx = 0;
    soc_sbx_g3p1_l2cpmac_t  l2cpmac;
    _l2cpmac_entry          *entry;

    /* First see if the same address is programmed in PPE */
    rv = soc_sbx_g3p1_l2cpmac_table_size_get(unit, 
                                          (uint32 *) &max_l2cpmac_idx);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    for (l2cpmac_idx=0; l2cpmac_idx < max_l2cpmac_idx; l2cpmac_idx++) {
        soc_sbx_g3p1_l2cpmac_t_init(&l2cpmac);
        rv = soc_sbx_g3p1_l2cpmac_get(unit, l2cpmac_idx, &l2cpmac);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        /* if index is being used...the mac will be non-zero */
        L2CPMAC_NULL(l2cpmac, rv);
        if (!rv) {
            /* index being used: if same mac address just incr ref cnt */
            if (!sal_memcmp(l2cpmac.mac, mac, 5)) {
                entry = L2CACHE_L2CPMAC_TABLE_ENTRY(unit, l2cpmac_idx);
                entry->ref_cnt++;
                *index_used = l2cpmac_idx;
                return BCM_E_NONE;
            }
        }
    }

    /* did not find this address programmed in the l2cpmac table
       Add a new entry */
    if (verify) {
        return BCM_E_NOT_FOUND;
    }

    for (l2cpmac_idx=0; l2cpmac_idx < max_l2cpmac_idx; l2cpmac_idx++) {
        soc_sbx_g3p1_l2cpmac_t_init(&l2cpmac);
        rv = soc_sbx_g3p1_l2cpmac_get(unit, l2cpmac_idx, &l2cpmac);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        /* if index is being used...the mac will be non-zero */
        L2CPMAC_NULL(l2cpmac, rv);
        if (!rv) {
            continue; /* index already being used */
        }
        /* found a free l2cpmac index...program it*/
        soc_sbx_g3p1_l2cpmac_t_init(&l2cpmac);
        MAC_TO_L2CPMAC(mac, l2cpmac);
        l2cpmac.useport = 0; /* not port specific */
        l2cpmac.mac[5] = 0; /* lsb is 0 */
        rv = soc_sbx_g3p1_l2cpmac_set(unit, l2cpmac_idx, &l2cpmac);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        entry = L2CACHE_L2CPMAC_TABLE_ENTRY(unit, l2cpmac_idx);
        entry->ref_cnt = 1;
        break;
    }
    if (l2cpmac_idx >= max_l2cpmac_idx) {
        /* all of l2cpmac entries already used */
        return BCM_E_RESOURCE;
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_l2cpmac_free(int unit, bcm_mac_t mac)
{
    int                     rv = BCM_E_NONE, l2cpmac_idx;
    int                     max_l2cpmac_idx = 0;
    soc_sbx_g3p1_l2cpmac_t  l2cpmac;
    _l2cpmac_entry          *entry;

    rv = soc_sbx_g3p1_l2cpmac_table_size_get(unit, 
                                          (uint32 *) &max_l2cpmac_idx);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    for (l2cpmac_idx=0; l2cpmac_idx < max_l2cpmac_idx; l2cpmac_idx++) {
        soc_sbx_g3p1_l2cpmac_t_init(&l2cpmac);
        rv = soc_sbx_g3p1_l2cpmac_get(unit, l2cpmac_idx, &l2cpmac);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        /* if index is being used...the mac will be non-zero */
        L2CPMAC_NULL(l2cpmac, rv);
        if (!rv) {
            /* index being used: if same mac address decr ref cnt */
            if (!sal_memcmp(l2cpmac.mac, mac, 5)) {
                entry = L2CACHE_L2CPMAC_TABLE_ENTRY(unit, l2cpmac_idx);
                entry->ref_cnt--;
                if (entry->ref_cnt <= 0) {
                    /* free if ref_cnt <= 0 */
                    soc_sbx_g3p1_l2cpmac_t_init(&l2cpmac);
                    rv = soc_sbx_g3p1_l2cpmac_set(unit, l2cpmac_idx, &l2cpmac);
                }
                return BCM_E_NONE;
            }
        }
    }

    /* should not reach here */
    return BCM_E_INTERNAL;
}

#define _G3P1_L2CACHE_L2CP_FTE  1
#define _G3P1_L2CACHE_GPORT_FTE 2

int
_bcm_caladan3_g3p1_fte_allocate(int unit, bcm_l2_cache_addr_t *addr,
                              uint32 *pfte, int fte_type)
{
    int                 rv = BCM_E_NONE;
    uint32              fte_idx = ~0, ohi_idx;
    int                 fab_unit, fab_node_id, fab_port, qid;
    soc_sbx_g3p1_ft_t   fte;
    int                 numcos, mc=0;
    const char         *fteTypeStr = "<UNKNOWN>";

    if (!pfte || !addr) {
        return BCM_E_PARAM;
    }

    if (addr->flags & BCM_L2_CACHE_MULTICAST) {
        ohi_idx = addr->group;
        qid = SBX_MC_QID_BASE; 
        mc = 1;
    } else {
        if (SOC_SBX_IS_VALID_L2_ENCAP_ID(addr->encap_id)) {
            /* encap_id is already provided */
            ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(addr->encap_id);
        } else {
            /* use the pass thru ohi */
            ohi_idx = L2CACHE_DEFAULT_OHI(unit);
        }

        rv = bcm_cosq_config_get(unit, &numcos);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }

        /* get dest queue info */
        rv = soc_sbx_node_port_get(unit, addr->dest_modid, addr->dest_port, 
                                   &fab_unit, &fab_node_id, &fab_port); 
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: invalid (dest_modid, dest_port)\n"),
                       FUNCTION_NAME()));
            return rv;
        }
        qid = SOC_SBX_NODE_PORT_TO_QID(unit,fab_node_id, fab_port, numcos);
    }

    if (fte_type == _G3P1_L2CACHE_L2CP_FTE) {
        rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 
                                     1, &fte_idx, 0);
        fteTypeStr = "UNICAST";
    } else if (fte_type == _G3P1_L2CACHE_GPORT_FTE) {
        rv = _sbx_caladan3_resource_alloc(unit, 
                                     SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                     1, &fte_idx, 0);
        fteTypeStr = "GPORT";
    }
 
    if (rv != BCM_E_NONE) {
#if defined(BROADCOM_DEBUG)
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed to allocate an %s (%d) FTE.\n"), 
                   FUNCTION_NAME(), fteTypeStr, fte_type));
#endif
        return rv;
    }

    /* initialize the FTE */
    soc_sbx_g3p1_ft_t_init(&fte);
    fte.oi  = (uint32)ohi_idx;
    fte.qid = qid;
    fte.mc = mc;

#ifdef INCLUDE_SBX_HIGIG
    
    /*
    zFte.ulDestMod  = addr->dest_modid;
    zFte.ulDestPort = addr->dest_port;
    zFte.ulOpcode   = BCM_HG_OPCODE_UC;
    */
#endif

    rv = soc_sbx_g3p1_ft_set(unit, fte_idx, &fte);
    
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed (%s)\n"),
                   FUNCTION_NAME(), _SHR_ERRMSG(rv)));
        if (fte_type == _G3P1_L2CACHE_L2CP_FTE) {
            _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                                   &fte_idx, 0);
        } else if (fte_type == _G3P1_L2CACHE_GPORT_FTE) {
            _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT, 1,
                                   &fte_idx, 0);
        } 
    } else {
        *pfte = fte_idx;
    }

    return rv;
}

int
_bcm_caladan3_g3p1_fte_free(int unit, uint32 fte_idx, int fte_type)
{
    int                 rv = BCM_E_NONE;
    soc_sbx_g3p1_ft_t   fte;
    
    soc_sbx_g3p1_ft_t_init(&fte);
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_set(unit, fte_idx, &fte));
    if (fte_type == _G3P1_L2CACHE_L2CP_FTE) {
        rv = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 
                                    1, &fte_idx, 0);
    } else if (fte_type == _G3P1_L2CACHE_GPORT_FTE) {
        rv = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT, 1,
                                   &fte_idx, 0);
    }
    return rv;
}

#ifdef WARM_BOOT_TODO
int
_bcm_caladan3_l2cache_entry_store(int unit, int index, int subtype) 
{
    _l2_cache_entry              *entry;
    caladan3_l2cache_wb_mem_layout_t *layout;
    int                          rv = BCM_E_NONE;
    _l2_cache_key               *pL2Key;

    if (L2CACHE_WB_L2_CONFIGURED(unit) == FALSE) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_l2cache_wb_layout_get(unit, &layout));
    soc_scache_handle_lock(unit, _l2_cache[unit].wb_hdl);

    L2CACHE_ENTRY_GET(unit, index, entry);
    layout->flags[index] = entry->flags;

    /* store some additional information in unused portion of flags field */
    if (entry->src_port == BCM_L2_SRCPORT_MASK_ALL) {
        layout->flags[index] |= L2CACHE_WB_ENTRY_PORT_ALL;
    }
    if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
        if (subtype == 0xff) {
            layout->flags[index] |= L2CACHE_WB_ENTRY_SUBTYPE_ALL;
        }
    }

    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "L2CP_CACHE entry stored, index: 0x%x flags: 0x%08x\n"),
               index, layout->flags[index]));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->flags: 0x%08x\n"),
               entry->flags));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->cookie: 0x%x\n"),
               entry->cookie));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->fte_idx: 0x%x\n"),
               entry->fte_idx));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->encap_id: 0x%x\n"),
               entry->encap_id));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->src_port: 0x%x\n"),
               entry->src_port));

    if (index >= L2CACHE_MAX_L2CP_IDX) {
        index = index - L2CACHE_MAX_L2CP_IDX;
        if (entry->flags & L2CACHE_ENTRY_DMAC) {
            if (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP){
                layout->mac_entry[index].mac[5]=0;
            }
            pL2Key = (_l2_cache_key *) entry->cookie;
            L2KEY_TO_MAC(pL2Key, layout->mac_entry[index].mac);
            layout->mac_entry[index].vlan = pL2Key->ulVlan;
            LOG_DEBUG(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "   entry->vlan: 0x%x\n"),
                       layout->mac_entry[index].vlan));
            LOG_DEBUG(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "   entry->mac: %02x:%02x:%02x:%02x:%02x:%02x\n"),
                       layout->mac_entry[index].mac[0],
                       layout->mac_entry[index].mac[1],
                       layout->mac_entry[index].mac[2],
                       layout->mac_entry[index].mac[3],
                       layout->mac_entry[index].mac[4],
                       layout->mac_entry[index].mac[5]));

        }
    }

    soc_scache_handle_unlock(unit, _l2_cache[unit].wb_hdl);

    return rv;

}

int
_bcm_caladan3_l2cache_entry_recover(int unit, int index,
                                  caladan3_l2cache_wb_mem_layout_t *layout)
{
    _l2_cache_entry              *entry;
    int                         rv = BCM_E_NONE;
    soc_sbx_g3p1_l2cpslow_t     l2cpslow;
    soc_sbx_g3p1_l2cp_t         l2cp;
    soc_sbx_g3p1_ft_t           fte;
    int                         port;
    int                         subtype=0;
    int                         dmac;
    _l2_cache_key               *pL2Key;
    soc_sbx_g3p1_mac_t          sbx_mac_data;
    int                         mac_index=0;
    int                         l2cpmac_idx=0;
    int                         rsvd_mac=0;

    L2CACHE_ENTRY_GET(unit, index, entry);

    entry->flags = layout->flags[index];

    port = L2CACHE_INDEX_TO_PORT(index);
    if (entry->flags & L2CACHE_WB_ENTRY_PORT_ALL) {
        entry->src_port = BCM_L2_SRCPORT_MASK_ALL;
    }else{
        entry->src_port = port;
    }

    dmac = L2CACHE_INDEX_TO_DMACLSB(index);

    if (!(entry->flags & L2CACHE_ENTRY_DMAC) ){
        if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
            subtype = L2CACHE_INDEX_TO_SUBTYPE(index);
            rv = soc_sbx_g3p1_l2cpslow_get(unit, subtype, port, &l2cpslow);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error reading l2cpslow table, subtype 0x%x, port 0x%x, \n"),
                           FUNCTION_NAME(), subtype, port));
                return rv;
            }
            entry->fte_idx = l2cpslow.ftidx;
        }else{
            subtype=0xff;
            rv = soc_sbx_g3p1_l2cp_get(unit, dmac, port, &l2cp);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error reading l2cp table, dmac 0x%x port 0x%x\n"),
                           FUNCTION_NAME(), dmac, port));
                return rv;
            }
            entry->fte_idx = l2cp.ftidx;
        }
    }else{
        /* recover mac entry */
        entry->src_port = 0;
        if (entry->flags & L2CACHE_ENTRY_DMAC) {
            if (index < L2CACHE_MAX_L2CP_IDX) {
                return BCM_E_INTERNAL;
            }
            mac_index = index - L2CACHE_MAX_L2CP_IDX;
            pL2Key = (_l2_cache_key*) sal_alloc(sizeof(_l2_cache_key), "l2 key");
            if (!pL2Key) {
                return BCM_E_MEMORY;
            }
            sal_memset(pL2Key, 0, sizeof(_l2_cache_key));
            pL2Key->ulVlan = layout->mac_entry[mac_index].vlan;
            MAC_TO_L2KEY(layout->mac_entry[mac_index].mac, pL2Key);
            entry->cookie = (uint32)pL2Key;

            L2CACHE_RESERVED_DMAC_CHECK(layout->mac_entry[mac_index].mac, rsvd_mac);
            if ((entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) && !(rsvd_mac)) {
                /* verify entry is present in l2cpmac table */
                rv = _bcm_caladan3_g3p1_l2cpmac_alloc(unit, layout->mac_entry[mac_index].mac, &l2cpmac_idx, 1);
                if (rv != BCM_E_NONE){
                    LOG_ERROR(BSL_LS_BCM_L2,
                              (BSL_META_U(unit,
                                          "%s: Entry not found in L2CPMac table\n"),
                               FUNCTION_NAME()));
                    return BCM_E_INTERNAL;
                }
            }

            soc_sbx_g3p1_mac_t_init(&sbx_mac_data);
            rv = soc_sbx_g3p1_mac_get(unit, layout->mac_entry[mac_index].mac, (pL2Key->ulVlan & 0xfff),
                                      &sbx_mac_data);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error reading mac from L2 table\n"),
                           FUNCTION_NAME()));
                return rv;
            }
            entry->fte_idx = sbx_mac_data.ftidx;
        }else{
            return BCM_E_INTERNAL;
        }

    }

    if ( entry->flags & L2CACHE_ENTRY_FTE_VALID) {
        soc_sbx_g3p1_ft_t_init(&fte);
        rv = soc_sbx_g3p1_ft_get(unit, entry->fte_idx, &fte);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Error getting ft entry:%d)\n"),
                       FUNCTION_NAME(), entry->fte_idx));
            return rv;
        }
        /* for mpls gports, fte and egress was allocated via MPLS module */
        if ( !(entry->flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) ) {
            if ( (fte.oi == L2CACHE_DEFAULT_OHI(unit)) ||
                 (entry->flags & L2CACHE_ENTRY_MULTICAST) ) {
                entry->encap_id = 0;
            }else{
                entry->encap_id = SOC_SBX_L2_ENCAP_ID_FROM_OHI(fte.oi);
            }
            if ( (entry->fte_idx >= SBX_GLOBAL_GPORT_FTE_BASE(unit)) &&
                 (entry->fte_idx <= SBX_GLOBAL_GPORT_FTE_END(unit)) ) {
                rv = _sbx_caladan3_resource_alloc(unit,
                                             SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                             1, &entry->fte_idx, _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_L2,
                              (BSL_META_U(unit,
                                          "%s: Error re-allocating fte 0x%x\n"),
                               FUNCTION_NAME(), entry->fte_idx));
                    return rv;
                }
            }else{
                rv = _sbx_caladan3_resource_alloc(unit,
                                             SBX_CALADAN3_USR_RES_FTE_UNICAST,
                                             1, &entry->fte_idx, _SBX_CALADAN3_RES_FLAGS_RESERVE);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_L2,
                              (BSL_META_U(unit,
                                          "%s: Error re-allocating fte 0x%x\n"),
                               FUNCTION_NAME(), entry->fte_idx));
                    return rv;
                }
            }
        }
    }else{
        entry->fte_idx = 0;
    }

    if (!(entry->flags & L2CACHE_ENTRY_DMAC)){
        if ( (entry->src_port == BCM_L2_SRCPORT_MASK_ALL) ||
            (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) ||
            ((entry->flags & L2CACHE_ENTRY_SUBTYPE) && 
             (entry->flags & L2CACHE_WB_ENTRY_SUBTYPE_ALL))) {
            /* maskable entry. cookie contains subtype & lsb */
            if (entry->flags & L2CACHE_WB_ENTRY_SUBTYPE_ALL) {
                subtype = 0xff;
            }
            entry->cookie = (((subtype & 0xff) << 24) | (dmac & 0xff));
        } else {
            /* entry is a non-maskable entry. cookie is the l2cp table index */
            if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
                entry->cookie = (L2CACHE_L2CP_SUBTYPES_OFFSET +
                                 (port * L2CACHE_L2CP_SUBTYPES) + subtype);
            } else {
                entry->cookie = (port * L2CACHE_L2CP_TYPES) + dmac;
            }
        }
    }

    /* clear WB flags */
    entry->flags &= ~(L2CACHE_WB_ENTRY_PORT_ALL | L2CACHE_WB_ENTRY_SUBTYPE_ALL);

    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "L2CP_CACHE entry recovered, index: 0x%x flags: 0x%08x\n"),
               index, layout->flags[index]));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->flags: 0x%08x\n"),
               entry->flags));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->cookie: 0x%x\n"),
               entry->cookie));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->fte_idx: 0x%x\n"),
               entry->fte_idx));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->encap_id: 0x%x\n"),
               entry->encap_id));
    LOG_DEBUG(BSL_LS_BCM_L2,
              (BSL_META_U(unit,
                          "   entry->src_port: 0x%x\n"),
               entry->src_port));
    if (entry->flags & L2CACHE_ENTRY_DMAC){
        LOG_DEBUG(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "   entry->vlan: 0x%x\n"),
                   layout->mac_entry[mac_index].vlan));
        LOG_DEBUG(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "   entry->mac: %02x:%02x:%02x:%02x:%02x:%02x\n"),
                   layout->mac_entry[mac_index].mac[0],
                   layout->mac_entry[mac_index].mac[1],
                   layout->mac_entry[mac_index].mac[2],
                   layout->mac_entry[mac_index].mac[3],
                   layout->mac_entry[mac_index].mac[4],
                   layout->mac_entry[mac_index].mac[5]));
    }

    return rv;
}


int
_bcm_caladan3_g3p1_l2_cache_init_recover(int unit, uint32 *pass_thru_ohi, 
                                       uint32 *cpu_fte, uint32 *l2cpmac_table,
                                       caladan3_l2cache_wb_mem_layout_t *layout)
{
    int                         rv = BCM_E_NONE;
    uint32                      max_idx;
    soc_sbx_g3p1_l2cpslow_t     l2cpslow;
    soc_sbx_g3p1_oi2e_t         sbx_ohi;
    soc_sbx_g3p1_ete_t          ete;
    uint32                      ete_idx;
    int                         flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
    int                         idx = 0;


    if (L2CACHE_WB_L2_CONFIGURED(unit) == FALSE) {
        return BCM_E_INTERNAL;
    }

    rv = soc_sbx_g3p1_max_l2cp_types_get(unit, &max_l2cp_types);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_max_l2cp_subtypes_get(unit, &max_l2cp_subtypes);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_l2cpmac_table_size_get(unit, &max_idx);
    if (rv == BCM_E_NONE) {
        *l2cpmac_table = (uint32) sal_alloc(max_idx * sizeof(_l2cpmac_entry), 
                                            "l2cpmac table");
        if (!(*l2cpmac_table)) {
            rv = BCM_E_RESOURCE;
			COMPILER_REFERENCE(rv);
        }
    }
    /* reconstruction of _l2_cache_t for unit */
    /* max_idx, determined above */
    /* _l2_cache_t.local, created in bcm_fe2000_l2_cache_init */
    /* subcodes 11-255 are invalid (by spec).  G2P3 l2cp table is 16 entries so
     11-15 may be used to store recovery info */
    /* recover _l2_cache_t.cpu_fte from l2cpslow table entry 11 */
    soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);
    rv = soc_sbx_g3p1_l2cpslow_get(unit, L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE, 0, &l2cpslow);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error getting L2CP SLOW entry (port:%d "
                               "subtype:%d)\n"), FUNCTION_NAME(), 0, 11));
        return rv;
    }
    *cpu_fte = l2cpslow.ftidx;
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 
                                 1, cpu_fte, flags);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating fte 0x%x\n"),
                   FUNCTION_NAME(), *cpu_fte));
        return rv;
    }

    /* recover _l2_cache_t.pass_thru_ohi from l2cpslow table entry 12 */
    rv = soc_sbx_g3p1_l2cpslow_get(unit, L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE + 1, 0, &l2cpslow);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error getting L2CP SLOW entry (port:%d "
                               "subtype:%d)\n"), FUNCTION_NAME(), 0, 12));
        return rv;
    }
    *pass_thru_ohi = l2cpslow.ftidx;
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_OHI, 
                                 1, pass_thru_ohi, flags);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating pass-thru ohi 0x%x\n"),
                   FUNCTION_NAME(), *pass_thru_ohi));
        return rv;
    }

    /* get encap ete and l2 ete based on ohi */
    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);
    rv = soc_sbx_g3p1_oi2e_get(unit, *pass_thru_ohi - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error getting oi2e entry:%d)\n"),
                   FUNCTION_NAME(), *pass_thru_ohi));
        return rv;
    }
    ete_idx = sbx_ohi.eteptr;
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 
                                 1, &ete_idx, flags);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating ete 0x%x\n"),
                   FUNCTION_NAME(), ete_idx));
        return rv;
    }

    soc_sbx_g3p1_ete_t_init(&ete);
    rv = soc_sbx_g3p1_ete_get(unit, ete_idx, &ete);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error getting ete entry:%d)\n"),
                   FUNCTION_NAME(), ete_idx));
        return rv;
    }

    /* walk L2CP and L2CP slow tables.  for each non-default entry create appropriate l2 cache entry */
    /* l2_cache_entries, created in bcm_fe2000_l2_cache_init, must be populated in this function */
    for (idx = 0; idx < L2CACHE_MAX_IDX; idx++) {
        if (layout->flags[idx] & L2CACHE_ENTRY_VALID) {
            _bcm_caladan3_l2cache_entry_recover(unit, idx, layout);
        }else{
            continue;
        }
    }

    return rv;
}
#endif 
#ifdef  BCM_WARM_BOOT_SUPPORT
int
bcm_caladan3_l2_cache_resource_restore(int unit )
{
    int                         rv = BCM_E_NONE;
    int                 index = 0;
    _l2_cache_entry     *entry;


    rv = soc_sbx_g3p1_max_l2cp_types_get(unit, &max_l2cp_types);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_max_l2cp_subtypes_get(unit, &max_l2cp_subtypes);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 
                                 1, &_l2_cache[unit].cpu_fte, _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating cpu fte 0x%x\n"),
                   FUNCTION_NAME(), _l2_cache[unit].cpu_fte));
        return rv;
    }

    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_OHI, 
                                 1, &_l2_cache[unit].pass_thru_ohi, _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating pass-thru ohi 0x%x\n"),
                   FUNCTION_NAME(), _l2_cache[unit].pass_thru_ohi));
        return rv;
    }

    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 
                                 1, &_l2_cache[unit].l2_ete, _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error re-allocating l2 ete 0x%x\n"),
                   FUNCTION_NAME(), _l2_cache[unit].l2_ete));
        return rv;
    }

    for (index = 0; index < L2CACHE_MAX_IDX; index++) {
        L2CACHE_ENTRY_GET_RV(unit, index, entry, rv);
        if (!entry || (rv == BCM_E_PARAM)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Error getting l2cache entry index %d\n"),
                       FUNCTION_NAME(), index));
            rv = BCM_E_INTERNAL;
            break;
        }
        if (entry->flags & L2CACHE_ENTRY_VALID) {
            if (index >= L2CACHE_MAX_L2CP_IDX) {
                rv = shr_idxres_list_reserve(_l2_cache[unit].idlist, index, index);
                if (rv != BCM_E_NONE) {
                    return rv;
                }
            }

            if (entry->flags & L2CACHE_ENTRY_FTE_VALID) {
                /* for mpls gports, fte and egress was allocated via MPLS module */
                if ( !(entry->flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) ) {
                    if ( (entry->fte_idx >= SBX_GLOBAL_GPORT_FTE_BASE(unit)) &&
                         (entry->fte_idx <= SBX_GLOBAL_GPORT_FTE_END(unit)) ) {
                        rv = _sbx_caladan3_resource_alloc(unit,
                                             SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                             1, &entry->fte_idx, _SBX_CALADAN3_RES_FLAGS_RESERVE);
                        if (rv != BCM_E_NONE) {
                            LOG_ERROR(BSL_LS_BCM_L2,
                                      (BSL_META_U(unit,
                                                  "%s: Error re-allocating fte 0x%x\n"),
                                       FUNCTION_NAME(), entry->fte_idx));
                            return rv;
                        }
                    }
                    else {
                        rv = _sbx_caladan3_resource_alloc(unit,
                                             SBX_CALADAN3_USR_RES_FTE_UNICAST,
                                             1, &entry->fte_idx, _SBX_CALADAN3_RES_FLAGS_RESERVE);
                        if (rv != BCM_E_NONE) {
                            LOG_ERROR(BSL_LS_BCM_L2,
                                      (BSL_META_U(unit,
                                                  "%s: Error re-allocating fte 0x%x\n"),
                                       FUNCTION_NAME(), entry->fte_idx));
                            return rv;
                        }
                    }

                }
            }
        }
    }
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT*/
int
_bcm_caladan3_g3p1_l2cp_table_set(int unit, uint32 fte_idx, uint8 dmac_lsb, 
                                int subtype_in, int idx)
{
    int                     rv = BCM_E_NONE;
    int                     port, dmac, subtype;
    int                     port_min, port_max, dmac_min, dmac_max;
    int                     st_min, st_max;
    uint32                  m_forward = 0, m_copy = 0;
    _l2_cache_entry         *entry;
    soc_sbx_g3p1_l2cp_t     l2cp;
    soc_sbx_g3p1_l2cpslow_t l2cpslow;

    L2CACHE_ENTRY_GET(unit, idx, entry);
    dmac = (int) dmac_lsb;
    port = (int) entry->src_port;

    if ((port != BCM_L2_SRCPORT_MASK_ALL) && (port > SBX_MAX_PORTS)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid port (%d) \n"),
                   FUNCTION_NAME(), port));
        return BCM_E_PARAM;
    }

    if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
        if (dmac != 0x02 ) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Invalid DMAC LSB(0x%x) specified for l2cp slow "
                                   "protocol. should be 0x02\n"), FUNCTION_NAME(), dmac));
            return BCM_E_PARAM;
        }
        if (subtype_in == 0xff) {
            st_min = 0;
#ifdef WARM_BOOT_TODO
            st_max = L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE;
#else
            st_max = L2CACHE_L2CP_SUBTYPES;
#endif 
        } else {
            st_min = subtype_in;
            st_max = subtype_in + 1;
        }
    } else {
        st_min = st_max = 0;
    }

    if (port == BCM_L2_SRCPORT_MASK_ALL) {
        port_min = 0;
        port_max = SBX_MAX_PORTS;
    } else {
        port_min = port;
        port_max = port + 1;
    }

    if (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) {
        dmac_min = 0;
        dmac_max = 0x100;
    } else {
        dmac_min = dmac;
        dmac_max = dmac + 1;
    }

    if (entry->flags & L2CACHE_ENTRY_FTE_VALID) {
        m_forward = 1;
    }

    if (entry->flags & L2CACHE_ENTRY_COPY_TO_CPU) {
        m_copy = 1;
    }
    if (entry->flags & L2CACHE_ENTRY_DROP) {
        m_copy = 0;
        m_forward = 0;
    }

    soc_sbx_g3p1_l2cp_t_init(&l2cp);
    soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);

    /* fill the l2cp and l2cp table entries */
    l2cp.forward = l2cpslow.forward = 1;
    if (m_forward) {
        l2cp.ftidx = l2cpslow.ftidx = fte_idx; /* Being tunnelled */
        if (m_copy) {
            l2cp.copy = l2cpslow.copy = 1; /* copy exception to cpu as well */
        }
    } else {
        /* not being tunnelled. Use CPU fte (for copy) or drop fte*/
        if (m_copy) {
            l2cp.ftidx = l2cpslow.ftidx = L2CACHE_CPU_FTE(unit);
        } else {
            l2cp.ftidx = l2cpslow.ftidx = SBX_DROP_FTE(unit);
        }
    }
    l2cpslow.passstp = 1; /* slow protocol - ignore stp */
    if (entry->flags & L2CACHE_ENTRY_BYPASS) {
        /* overrides all flags. Sets forward & copy to 0 */
        soc_sbx_g3p1_l2cp_t_init(&l2cp);
        soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);
    }

    /* now write to HW */
    for (port = port_min; port < port_max; port++) {
        for (dmac = dmac_min; dmac < dmac_max; dmac++) {
            if (entry->flags & L2CACHE_ENTRY_BPDU) {
                l2cp.passstp = 1;
            } else {
                l2cp.passstp = 0;
            }
            rv = soc_sbx_g3p1_l2cp_set(unit, dmac, port, &l2cp);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error setting L2CP entry (port:%d lsb:%d)\n"),
                           FUNCTION_NAME(), port, dmac));
                return rv;
            }
        }
        for (subtype = st_min; subtype < st_max; subtype++) {
            rv = soc_sbx_g3p1_l2cpslow_set(unit, subtype, port, &l2cpslow);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error setting L2CP SLOW entry (port:%d "
                                       "subtype:%d)\n"), FUNCTION_NAME(), port, subtype));
                return rv;
            }
        }
    }

    if (rv == BCM_E_NONE) {
        if ((entry->src_port == BCM_L2_SRCPORT_MASK_ALL) ||
            (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) ||
            ((entry->flags & L2CACHE_ENTRY_SUBTYPE) && 
             (subtype_in == 0xff))) {
            /* maskable entry. cookie contains subtype & lsb */
            entry->cookie = (((subtype_in & 0xff) << 24) | (dmac_lsb & 0xff));
        } else {
            /* entry is a non-maskable entry. cookie is the l2cp table index */
            if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
                entry->cookie = (L2CACHE_L2CP_SUBTYPES_OFFSET +
                                 (port_min * L2CACHE_L2CP_SUBTYPES) + st_min);
            } else {
                entry->cookie = (port_min * L2CACHE_L2CP_TYPES) + dmac_min;
            }
        }
        if (fte_idx) {
            entry->fte_idx = fte_idx;
        }
    }

    return rv;
}

int
_bcm_caladan3_g3p1_l2_cache_hw_entry_delete(int unit, int idx)
{
    int                         rv = BCM_E_NONE;
    uint32                      flags, l2cp_idx;
    int                         port, dmac, subtype;
    soc_sbx_g3p1_l2cp_t         l2cp;
    soc_sbx_g3p1_l2cpslow_t     l2cpslow;
    _l2_cache_entry             *entry;
    soc_sbx_g3p1_6_byte_t       mac;
    _l2_cache_key               *pL2Key;
    int                         rsvd_mac;
    uint8                     dmac_lsb;
    
    L2CACHE_ENTRY_GET(unit, idx, entry);
    flags = entry->flags;

    if (flags & L2CACHE_ENTRY_FTE_VALID) {
        /* FTE entry is valid, delete the entry */
        if (flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) {
            entry->flags &= (~L2CACHE_ENTRY_FTE_MPLS_GPORT);
        } else {
            if (flags & L2CACHE_ENTRY_L2CP) {
                rv = _bcm_caladan3_g3p1_fte_free(unit, entry->fte_idx, 
                                               _G3P1_L2CACHE_L2CP_FTE);
            } else {
                rv = _bcm_caladan3_g3p1_fte_free(unit, entry->fte_idx,
                                               _G3P1_L2CACHE_GPORT_FTE);
            }
            if (rv != BCM_E_NONE) {
                return rv;
            }
        }
        entry->flags &= (~L2CACHE_ENTRY_FTE_VALID);
        entry->fte_idx = 0;
    }

    if ((entry->src_port == BCM_L2_SRCPORT_MASK_ALL) || 
        ((flags & L2CACHE_ENTRY_L2CP) && 
         (flags & L2CACHE_ENTRY_PREFIX5_LOOKUP)) ||
        ((flags & L2CACHE_ENTRY_SUBTYPE) && 
         (((entry->cookie & 0xff000000) >> 24) == 0xff))) {
        entry->flags = 0;
        if (flags & L2CACHE_ENTRY_SUBTYPE) {
            entry->flags |= L2CACHE_ENTRY_SUBTYPE;
        }
        if (flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) {
            entry->flags |= L2CACHE_ENTRY_PREFIX5_LOOKUP;
        }
        /* mark the entry as CPU...which is the default */
        entry->flags |= L2CACHE_ENTRY_COPY_TO_CPU;
        dmac_lsb = (entry->cookie & 0xff);
        if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
            subtype = ((entry->cookie & 0xff000000) >> 24);
        } else {
            subtype = 0xff;
        }
        rv = _bcm_caladan3_g3p1_l2cp_table_set(unit, 0, dmac_lsb, subtype, idx);
        if (rv == BCM_E_NONE) {
            sal_memset(entry, 0x0, sizeof(_l2_cache_entry));
        }
#ifdef WARM_BOOT_TODO
        /* update cache */
        rv = _bcm_caladan3_l2cache_entry_store(unit, idx, 0);
#endif 

        return rv;
    }

    if (flags & L2CACHE_ENTRY_L2CP) {
        /* entry is a L2CP entry */
        l2cp_idx = entry->cookie;
        if (l2cp_idx < L2CACHE_L2CP_SUBTYPES_OFFSET) {
            /* non-slow protocol */
            dmac = (l2cp_idx & (max_l2cp_types - 1));
            port = l2cp_idx/max_l2cp_types;
            subtype = -1;
        } else {
            /* slow protocol */
            l2cp_idx -= L2CACHE_L2CP_SUBTYPES_OFFSET;
            dmac = 0x2;
            port = l2cp_idx/max_l2cp_subtypes;
            subtype = (l2cp_idx & (max_l2cp_subtypes - 1));
        }
        if (subtype < 0) {
            soc_sbx_g3p1_l2cp_t_init(&l2cp);
            l2cp.forward = 1;
            l2cp.ftidx = L2CACHE_CPU_FTE(unit);
            if (entry->flags & L2CACHE_ENTRY_BPDU) {
                l2cp.passstp = 1;
            } else {
                l2cp.passstp = 0;
            }
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_l2cp_set(unit, dmac, port, &l2cp));
        } else {
            soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);
            l2cpslow.forward = 1;
            l2cpslow.ftidx = L2CACHE_CPU_FTE(unit);
            l2cpslow.passstp = 1; /* slow protocol - ingore stp */
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_l2cpslow_set(unit, subtype, port, 
                                                          &l2cpslow));
        }
        if (rv == BCM_E_NONE) {
            sal_memset(entry, 0x0, sizeof(_l2_cache_entry));
        }
#ifdef WARM_BOOT_TODO
        /* update cache */
        rv = _bcm_caladan3_l2cache_entry_store(unit, idx, 0);
#endif 
    }

    if (flags & L2CACHE_ENTRY_DMAC) {
        /* Entry is present in a regular DMAC table */
        pL2Key = (_l2_cache_key *) entry->cookie;
        if (!pL2Key) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Invalid l2 key\n"),
                       FUNCTION_NAME()));
            return BCM_E_INTERNAL;
        }
        L2KEY_TO_MAC(pL2Key, mac);
        rsvd_mac = 0;
        L2CACHE_RESERVED_DMAC_CHECK(mac, rsvd_mac);

        if ((flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) && !rsvd_mac) {
            /* free the l2cpmac table entry */
            rv = _bcm_caladan3_g3p1_l2cpmac_free(unit, mac);
            if (rv != BCM_E_NONE) {
                return rv;
            }
        }
        rv = soc_sbx_g3p1_mac_remove(unit, mac, pL2Key->ulVlan);
        if (rv != BCM_E_NONE) {
            LOG_DEBUG(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Error (%s) removing L2 entry from DMAC table\n"),
                       FUNCTION_NAME(), bcm_errmsg(rv)));
            return rv;
        }
    
        sal_free(pL2Key); /* delete the key */
        sal_memset(entry, 0x0, sizeof(_l2_cache_entry));
#ifdef WARM_BOOT_TODO
        /* update cache */
        rv = _bcm_caladan3_l2cache_entry_store(unit, idx, 0);
#endif 
    }


    return rv;
}

int
_bcm_caladan3_g3p1_l2_cache_hw_entry_config_get(int unit, int idx,
                                              bcm_l2_cache_addr_t *addr)
{
    int                         rv = BCM_E_NONE;
    uint32                      flags, cookie, qid, fte_idx;
    _l2_cache_entry             *entry;
    int                         fab_node_id, fab_port;
    int                         numcos;
    _l2_cache_key               *pL2Key;
    soc_sbx_g3p1_ft_t           fte;

    L2CACHE_ENTRY_GET(unit, idx, entry);
    
    rv = bcm_cosq_config_get(unit, &numcos);
    if (rv != BCM_E_NONE) {
        return BCM_E_INTERNAL;
    }

    flags = entry->flags;
    
    if (flags & L2CACHE_ENTRY_L2CP) {
        /* entry is a L2CP entry. Derive the mac address based on index */
        cookie = entry->cookie;
        L2CACHE_RESERVED_DMAC_SET(addr->mac);
        if ((entry->src_port == BCM_L2_SRCPORT_MASK_ALL) || 
            (flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) ||
            ((flags & L2CACHE_ENTRY_SUBTYPE) && 
             (((cookie & 0xff000000) >> 24) == 0xff))) {
            /* for src_port_mask, cookie contains last byte of dmac & subtype*/
            addr->mac[5] = (cookie & 0xff);
            if (flags & L2CACHE_ENTRY_SUBTYPE) {
                addr->subtype = ((cookie & 0xff000000) >> 24);
                addr->flags |= BCM_L2_CACHE_SUBTYPE;
            }
        } else {
            if (cookie < L2CACHE_L2CP_SUBTYPES_OFFSET) {
                /* non-slow protocol */
                addr->mac[5] = (cookie & (max_l2cp_types - 1));
            } else {
                /* slow protocol */
                cookie -= L2CACHE_L2CP_SUBTYPES_OFFSET;
                addr->mac[5] = 0x2;
                addr->subtype = (cookie & (max_l2cp_subtypes - 1));
                addr->flags |= BCM_L2_CACHE_SUBTYPE;
            }
        }
    }

    if (flags & L2CACHE_ENTRY_DMAC) {
        /* entry is a regular DMAC entry */
        pL2Key = (_l2_cache_key *)entry->cookie;
        if (!pL2Key) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Invalid L2CACHE entry\n"),
                       FUNCTION_NAME()));
            return BCM_E_INTERNAL;
        }
        L2KEY_TO_MAC(pL2Key, addr->mac);
        addr->vlan = pL2Key->ulVlan;
    }
    if (flags & L2CACHE_ENTRY_FTE_VALID) {
        /* There is a valid FTE */
        fte_idx = entry->fte_idx;
        if (flags & L2CACHE_ENTRY_MULTICAST) {
            addr->flags |= BCM_L2_CACHE_MULTICAST;
        } else {
            addr->flags |= BCM_L2_CACHE_TUNNEL;
        }
        
        if (flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) {
            /* for MPLS gport, fte_idx is gport instead of fte idx */
            BCM_GPORT_MPLS_PORT_ID_SET(addr->dest_port, entry->fte_idx);
        } else if (flags & L2CACHE_ENTRY_MULTICAST){
            soc_sbx_g3p1_ft_t_init(&fte);
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_get(unit, fte_idx, &fte));
            addr->group = fte.oi;
        } else {
            soc_sbx_g3p1_ft_t_init(&fte);
            BCM_IF_ERROR_RETURN(soc_sbx_g3p1_ft_get(unit, fte_idx, &fte));
            qid = fte.qid;
            SOC_SBX_NODE_PORT_FROM_QID(unit, qid, fab_node_id, fab_port, numcos);
            rv = soc_sbx_modid_get(unit, fab_node_id, fab_port, 
                                   &addr->dest_modid);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Tunnel dest_modid get failed. \n"), 
                           FUNCTION_NAME()));
                return rv;
            }
            addr->dest_port = fab_port;
        }
    }
    addr->encap_id = entry->encap_id;
    if (entry->src_port == BCM_L2_SRCPORT_MASK_ALL) {
        addr->src_port = 0;
        addr->src_port_mask = BCM_L2_SRCPORT_MASK_ALL;
    } else {
        addr->src_port = entry->src_port;
    }
    
    return rv;
}

int
_bcm_caladan3_g3p1_l2_cache_hw_entry_set(int unit, bcm_l2_cache_addr_t *addr, 
                                       int idx)
{
    int                         rv = BCM_E_NONE;
    int                         rsvd_dmac, subtype;
    int                         l2cpmac_idx=0;
    uint32                      fte_idx = 0;
    _l2_cache_entry             *entry;
    uint32                      m_ftidx=0;
    _l2_cache_key               *pL2Key;
    soc_sbx_g3p1_mac_t          sbx_mac_data;
    bcm_mac_t                   temp_mac;
    int                         node, fab_unit, fab_port;

    L2CACHE_ENTRY_GET(unit, idx, entry);
    L2CACHE_RESERVED_DMAC_CHECK(addr->mac, rsvd_dmac);

    if (rsvd_dmac && !(entry->flags & L2CACHE_ENTRY_DMAC)) {
        if (entry->flags & L2CACHE_ENTRY_FTE_VALID) {
            if (entry->flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) {
                /* for MPLS gport, fte_idx is gport instead of fte idx */
                m_ftidx = BCM_GPORT_MPLS_PORT_ID_GET(entry->fte_idx);
            } else {
                rv = _bcm_caladan3_g3p1_fte_allocate(unit, addr, &fte_idx,
                                                   _G3P1_L2CACHE_L2CP_FTE);
                if (rv != BCM_E_NONE) {
                    return rv;
                }
                m_ftidx = fte_idx;
            }

        }
        if (entry->flags & L2CACHE_ENTRY_SUBTYPE) {
            subtype = addr->subtype;
        } else {
            subtype = 0xff;
        }

        /* entry is l2cp entry. Program l2cp table */
        rv = _bcm_caladan3_g3p1_l2cp_table_set(unit, m_ftidx, addr->mac[5], 
                                             subtype, idx);
        if (rv != BCM_E_NONE) {
            if (fte_idx) {
                _bcm_caladan3_g3p1_fte_free(unit, fte_idx, 
                                          _G3P1_L2CACHE_GPORT_FTE);
            }
        }else{
#ifdef WARM_BOOT_TODO
            rv = _bcm_caladan3_l2cache_entry_store(unit, idx, subtype);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error storing warmboot information for entry 0x%x"),
                           FUNCTION_NAME(), idx));
            }
#endif 
        }
        return rv;
    } else {
        sal_memcpy(temp_mac, addr->mac, sizeof(bcm_mac_t));
        pL2Key = (_l2_cache_key*) sal_alloc(sizeof(_l2_cache_key), "l2 key");
        if (!pL2Key) {
            return BCM_E_MEMORY;
        }
        sal_memset(pL2Key, 0, sizeof(_l2_cache_key));

        if (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) {
            temp_mac[5] = 0; /* reset the lsb */
            if (rsvd_dmac) {
                /* do not add rsvd mac addresses to PPE l2cpmac table */
                l2cpmac_idx = -1;
            } else {
                rv = _bcm_caladan3_g3p1_l2cpmac_alloc(unit, addr->mac, &l2cpmac_idx, 0);
                if (rv != BCM_E_NONE) {
                    sal_free(pL2Key);
                    return rv;
                }
            }
        }
        MAC_TO_L2KEY(addr->mac, pL2Key);
        pL2Key->ulVlan = (addr->vlan & 0xfff);
        if (entry->flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) {
            pL2Key->ulMac0 = 0; /* reset the LSB */
            pL2Key->cookie = (uint32) l2cpmac_idx; /* l2cpmac table index */
        }
        soc_sbx_g3p1_mac_t_init(&sbx_mac_data);
        /* sbx_mac_data.dontage = 1; */
        if (entry->flags & L2CACHE_ENTRY_COPY_TO_CPU) {
            sbx_mac_data.dcopy = 1;
        }
        if (entry->flags & L2CACHE_ENTRY_FTE_VALID) {
            if (entry->flags & L2CACHE_ENTRY_FTE_MPLS_GPORT) {
                /* for MPLS gport, fte_idx is gport instead of fte idx */
                sbx_mac_data.ftidx = BCM_GPORT_MPLS_PORT_ID_GET(entry->fte_idx);
            } else {
                rv = _bcm_caladan3_g3p1_fte_allocate(unit, addr, &fte_idx, 
                                                   _G3P1_L2CACHE_GPORT_FTE);
                if (rv != BCM_E_NONE) {
		  if (!rsvd_dmac && l2cpmac_idx) {
                    _bcm_caladan3_g3p1_l2cpmac_free(unit, temp_mac);
		  }
                  sal_free(pL2Key);
		  return rv;
                }
                sbx_mac_data.ftidx = fte_idx;
            }
        }
        if (entry->flags & L2CACHE_ENTRY_DROP) {
            sbx_mac_data.ftidx = SBX_DROP_FTE(unit);
            if (fte_idx) {
                _bcm_caladan3_g3p1_fte_free(unit, fte_idx,
                                          _G3P1_L2CACHE_GPORT_FTE);
                fte_idx = 0;
            }
        } else if ((entry->flags & L2CACHE_ENTRY_BYPASS) &&
                   !(entry->flags & L2CACHE_ENTRY_FTE_VALID)) {
            /*
             * check for multicast group was done above.. 
             * using L2CACHE_ENTRY_FTE_VALID
             * now check for unicast destination and return if invalid params
             */
            if (!SOC_SBX_MODID_ADDRESSABLE(unit, addr->dest_modid)) {
                sal_free(pL2Key); 
                return BCM_E_PARAM;
            }

            if (!SOC_SBX_PORT_ADDRESSABLE(unit, addr->dest_port)) {
                sal_free(pL2Key);
                return BCM_E_PARAM;
            }

            rv = soc_sbx_node_port_get(unit, addr->dest_modid, addr->dest_port,
                                       &fab_unit, &node, &fab_port);
            if (rv != BCM_E_NONE) {
                sal_free(pL2Key); 
                return rv;
            }
            sbx_mac_data.ftidx = SOC_SBX_PORT_FTE(unit, node, fab_port);
        }
        rv = soc_sbx_g3p1_mac_set(unit, temp_mac, (addr->vlan & 0xfff),
                                  &sbx_mac_data);
        if (rv == BCM_E_EXISTS) {
            rv = BCM_E_NONE;
        }
 
        if (rv == BCM_E_NONE) {
            entry->cookie = (uint32)pL2Key;
            if (fte_idx) {
                entry->fte_idx = fte_idx;
            }
        } else {
            if (fte_idx) {
                _bcm_caladan3_g3p1_fte_free(unit, fte_idx, _G3P1_L2CACHE_GPORT_FTE);
            }
            if (!rsvd_dmac && l2cpmac_idx) {
            _bcm_caladan3_g3p1_l2cpmac_free(unit, temp_mac);
            }
            sal_free(pL2Key);
        }
#ifdef WARM_BOOT_TODO
        if (rv == BCM_E_NONE) {
            rv = _bcm_caladan3_l2cache_entry_store(unit, idx, 0);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error storing warmboot information for entry 0x%x"),
                           FUNCTION_NAME(), idx));
            }
        }
#endif 
    }

    return rv;
}

int
_bcm_caladan3_g3p1_l2_cache_entries_init(int unit, uint32 fte_idx)
{
    int                         rv = BCM_E_NONE;
    soc_sbx_g3p1_l2cp_t         l2cp;
    soc_sbx_g3p1_l2cpslow_t     l2cpslow;
    int                         port, dmac, subtype;

    soc_sbx_g3p1_l2cp_t_init(&l2cp);
    l2cp.forward = 1;
    l2cp.ftidx = fte_idx;
    soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);
    l2cpslow.forward = 1;
    l2cpslow.ftidx = fte_idx; 
    l2cpslow.passstp = 1; /* slow protocol - ignore stp */

    if (BCM_L2_SRCPORT_MASK_ALL <= SBX_MAX_PORTS) {
        LOG_WARN(BSL_LS_BCM_L2,
                 (BSL_META_U(unit,
                             "%s: BCM_L2_SRCPORT_MASK_ALL is less than SBX_MAX_PORTS> "
                              "This results in unknown behavior\n"), FUNCTION_NAME()));
    }

    for (port=0; ((port<SBX_MAX_PORTS) && (rv == BCM_E_NONE)); port++) {
        for (dmac=0; ((dmac < L2CACHE_L2CP_TYPES) && (rv == BCM_E_NONE)); 
             dmac++) {
            /* Bridge & Provider Bridge Group Addresses, skip STP check */
            l2cp.passstp = (((dmac == 0) || (dmac == 8))? 1 : 0); 
            rv = soc_sbx_g3p1_l2cp_set(unit, dmac, port, &l2cp);
            if (rv != BCM_E_NONE)
                   LOG_CLI((BSL_META_U(unit,
                                       " soc_sbx_g3p1_l2cp_set failed dmac %d port %d\n"),
                            dmac, port));
                        
        }
        for (subtype=0; ((subtype< L2CACHE_L2CP_SUBTYPES) && 
                         (rv == BCM_E_NONE)); subtype++) {
            rv = soc_sbx_g3p1_l2cpslow_set(unit, subtype, port, &l2cpslow);
            if (rv != BCM_E_NONE)
                   LOG_CLI((BSL_META_U(unit,
                                       " soc_sbx_g3p1_l2cpslow_set failed subtype %d port %d\n"),
                            subtype, port));
        }
    }

    /* OAM range, initilize as bypass */
    /* overrides all flags. Sets forward & copy to 0 */
    soc_sbx_g3p1_l2cp_t_init(&l2cp);
    for (port=0; ((port<SBX_MAX_PORTS) && (rv == BCM_E_NONE)); port++) {
        for (dmac=0x30; ((dmac < 0x40) && (rv == BCM_E_NONE));
             dmac++) {
           rv = soc_sbx_g3p1_l2cp_set(unit, dmac, port, &l2cp);
        }
    }

    return rv;
}

int 
_bcm_caladan3_g3p1_l2_cache_init(int unit, uint32 *pass_thru_ohi, 
                               uint32 *cpu_fte, uint32 *l2_ete, uint32 *l2cpmac_table)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx = ~0;
    uint32                      fte_idx = ~0;
    soc_sbx_g3p1_oi2e_t         sbx_ohi;
    soc_sbx_g3p1_ete_t          ete;
    soc_sbx_g3p1_ft_t           ft;
    soc_sbx_g3p1_xt_t           xt;
    uint32                      exc_idx;
    uint32                      max_idx, ete_idx = ~0;
#ifdef WARM_BOOT_TODO
    soc_sbx_g3p1_l2cpslow_t l2cpslow;
    uint32                  size;
    soc_wb_cache_t          *wbc;

    /* on cold boot, setup scache */
    SOC_SCACHE_HANDLE_SET(_l2_cache[unit].wb_hdl, unit, 
                          BCM_MODULE_L2, L2_WB_L2CACHE);
    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, (int*)&size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }
    
    /* Allocate a new chunk of the scache during a cold boot */
    if (!SOC_WARM_BOOT(unit) && (size > 0)) {
        LOG_DEBUG(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "L2CACHE: allocating 0x%x (%d) bytes of scache:"),  
                   (sizeof(caladan3_l2cache_wb_mem_layout_t) + 
                   SOC_WB_SCACHE_CONTROL_SIZE),  
                   (sizeof(caladan3_l2cache_wb_mem_layout_t) + 
                   SOC_WB_SCACHE_CONTROL_SIZE)));
        rv = soc_scache_alloc(unit, _l2_cache[unit].wb_hdl, 
                              (sizeof(caladan3_l2cache_wb_mem_layout_t) + 
                               SOC_WB_SCACHE_CONTROL_SIZE));
        if (rv == BCM_E_EXISTS) {
            rv = BCM_E_NONE;
        }
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    /* Get the pointer for the Level 2 cache */
    wbc = NULL;
    _l2_cache[unit].scache_size = 0;
    if (size > 0) {
        rv = soc_scache_ptr_get(unit, _l2_cache[unit].wb_hdl, (uint8**)&wbc,
                                &_l2_cache[unit].scache_size);
        if (BCM_FAILURE(rv)) {
            return rv; 
        }

        /* adjust the scache size for the control word */
        _l2_cache[unit].scache_size = SOC_WB_SCACHE_SIZE(_l2_cache[unit].scache_size);
    }

    if (wbc) {
        if (!SOC_WARM_BOOT(unit)) {
            wbc->version = L2CACHE_WB_CURRENT_VERSION;
        }

        LOG_VERBOSE(BSL_LS_BCM_L2,
                    (BSL_META_U(unit,
                                "Obtained scache pointer=0x%08x, %d bytes, "
                                "version=%d.%d\n"),
                     (int)wbc->cache, _l2_cache[unit].scache_size,
                     SOC_SCACHE_VERSION_MAJOR(wbc->version),
                     SOC_SCACHE_VERSION_MINOR(wbc->version)));

        if (wbc->version > L2CACHE_WB_CURRENT_VERSION) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Upgrade scenario not supported.  "
                                   "Current version=%d.%d  found %d.%d\n"),
                       SOC_SCACHE_VERSION_MAJOR(L2CACHE_WB_CURRENT_VERSION),
                       SOC_SCACHE_VERSION_MINOR(L2CACHE_WB_CURRENT_VERSION),
                       SOC_SCACHE_VERSION_MAJOR(wbc->version),
                       SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;

        } else if (wbc->version < L2CACHE_WB_CURRENT_VERSION) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Downgrade scenario not supported.  "
                                   "Current version=%d.%d  found %d.%d\n"),
                       SOC_SCACHE_VERSION_MAJOR(L2CACHE_WB_CURRENT_VERSION),
                       SOC_SCACHE_VERSION_MINOR(L2CACHE_WB_CURRENT_VERSION),
                       SOC_SCACHE_VERSION_MAJOR(wbc->version),
                       SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;
        }

        if (rv != BCM_E_NONE) {
            return rv;
        }

        /* if warmboot, perform recovery */
        if (SOC_WARM_BOOT(unit)) {
            caladan3_l2cache_wb_mem_layout_t *layout;
            
            BCM_IF_ERROR_RETURN(
                _bcm_l2cache_wb_layout_get(unit, &layout));
            soc_scache_handle_lock(unit, _l2_cache[unit].wb_hdl);
            rv = _bcm_caladan3_g3p1_l2_cache_init_recover(unit, pass_thru_ohi, 
                                                        cpu_fte, 
                                                        l2cpmac_table, layout);
            soc_scache_handle_unlock(unit, _l2_cache[unit].wb_hdl);

            return rv;
        }

    }
#endif 

#ifdef BCM_WARM_BOOT_SUPPORT 
        rv = bcm_caladan3_wb_l2_cache_state_init (unit);
        if (SOC_WARM_BOOT(unit)) {
            return rv;
        }
#endif /* BCM_WARM_BOOT_SUPPORT */

    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_exc_l2cp_copy_idx_get(unit, &exc_idx));
    soc_sbx_g3p1_xt_t_init(&xt);
    xt.qid      = SBX_EXC_QID_BASE(unit);
    xt.forward  = TRUE;
    rv = soc_sbx_g3p1_xt_set(unit, exc_idx, &xt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: programming exception table failed (%s)\n"),
                   FUNCTION_NAME(), _SHR_ERRMSG(rv)));
        return rv;
    }

    rv = soc_sbx_g3p1_max_l2cp_types_get(unit, &max_l2cp_types);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g3p1_max_l2cp_subtypes_get(unit, &max_l2cp_subtypes);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* program the CPU FTE for L2CP */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                                 &fte_idx, 0);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed to allocate an FTE. \n"),
                   FUNCTION_NAME()));
        return rv;
    }
    soc_sbx_g3p1_ft_t_init(&ft);
    ft.excidx = exc_idx;
    rv = soc_sbx_g3p1_ft_set(unit, fte_idx, &ft);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed to set cpu FTE 0x%x for l2cp\n"), 
                   FUNCTION_NAME(), fte_idx));
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                               &fte_idx, 0);
        return rv;
    }

    rv = _bcm_caladan3_g3p1_l2_cache_entries_init(unit, fte_idx);
    
    if (rv != BCM_E_NONE) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                       &fte_idx, 0);
        return rv;
    }
    
    /* create a default pass thru ohi */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_OHI, 1, &ohi_idx, 0);
    if (rv != BCM_E_NONE) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                       &fte_idx, 0);
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed to allocate an OHI\n"),
                   FUNCTION_NAME()));
        return rv;
    }

    /* get an L2 ETE */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 1, &ete_idx,0);
    if (rv != BCM_E_NONE) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                       &fte_idx, 0);
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_OHI, 1, &ohi_idx, 0);
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: failed to allocate an ETE\n"),
                   FUNCTION_NAME()));
        return rv;
    }

    /* initialize the OHI */
    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);
    sbx_ohi.eteptr  = ete_idx;

    /* initialize the ete */
    soc_sbx_g3p1_ete_t_init(&ete);
    ete.dmacset = 0;
    ete.ipttldec = 0;
    ete.ttlcheck = 0;
    ete.smacset = 0;
    ete.etype = 0;

    ete.mtu = SBX_DEFAULT_MTU_SIZE;
    
    /* default to untagged */
    ete.usevid       = 1;
    ete.vid          = _BCM_VLAN_G3P1_UNTAGGED_VID;

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_ete_set(unit, ete_idx, &ete);
    }
    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_oi2e_set(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    }

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_l2cpmac_table_size_get(unit, &max_idx);
        if (rv == BCM_E_NONE) {
            *l2cpmac_table = (uint32) sal_alloc(max_idx * sizeof(_l2cpmac_entry), 
                                                "l2cpmac table");
            if (!(*l2cpmac_table)) {
                rv = BCM_E_RESOURCE;
            }
        }
    }

    if (rv != BCM_E_NONE) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_UNICAST, 1, 
                       &fte_idx, 0);
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_OHI, 1, &ohi_idx, 0);
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_ETE, 1, &ete_idx, 0);
    } else {
        *pass_thru_ohi = ohi_idx;
        *cpu_fte = fte_idx;
        *l2_ete = ete_idx;
#ifdef WARM_BOOT_TODO
        /* Protocol subcodes 11-16 in the l2cp slow table are invalid.  Store 
         * To-CPU and pass-thru ohi information for warm boot recovery
         */
            soc_sbx_g3p1_l2cpslow_t_init(&l2cpslow);
            l2cpslow.forward = 0;
            l2cpslow.ftidx = *cpu_fte;
            rv = soc_sbx_g3p1_l2cpslow_set(unit, L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE, 0, &l2cpslow);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error setting L2CP SLOW entry (port:%d "
                                       "subtype:%d)\n"), FUNCTION_NAME(), 0, 11));
                return rv;
            }
            l2cpslow.ftidx = *pass_thru_ohi;
            rv = soc_sbx_g3p1_l2cpslow_set(unit, L2CACHE_WB_MAX_VALID_L2CP_SUBTYPE + 1, 0, &l2cpslow);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Error setting L2CP SLOW entry (port:%d "
                                       "subtype:%d)\n"), FUNCTION_NAME(), 0, 12));
                return rv;
            }
#endif 

    }

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_cache_entry_delete
 * Purpose:
 *      Delete the l2_cache entry pointed by the index
 * Parameters:
 *      unit        - Device number
 *      idx         - idx of the l2 cache entry to be deleted
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
_bcm_caladan3_l2_cache_entry_delete(int unit, int idx)
{
    int                         rv = BCM_E_NONE;
    _l2_cache_entry             *entry;
    uint32                      flags;

    L2CACHE_ENTRY_GET(unit, idx, entry);

    flags = entry->flags;
    if (!(flags & L2CACHE_ENTRY_VALID)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid L2CP entry at idx %d\n"),
                   FUNCTION_NAME(), idx));
        return BCM_E_PARAM;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_cache_hw_entry_delete(unit, idx);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    sal_memset(entry, 0, sizeof(_l2_cache_entry));
    shr_idxres_list_free(_l2_cache[unit].idlist, idx);

    return rv;
}


/*
 * Function:
 *      _bcm_caladan3_l2_cache_entry_config_get
 * Purpose:
 *      Read the FTE/OHI/ETE entries and populate l2_cache_addr_t info
 * Parameters:
 *      unit        - Device number
 *      idx         - l2 cache entry idx from which to read the config
 *      addr (OUT)  - l2 cache address config to be read into
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
_bcm_caladan3_l2_cache_entry_config_get(int unit, 
                                      uint32 idx,
                                      bcm_l2_cache_addr_t *addr)
{
    int                 rv = BCM_E_NONE;
    _l2_cache_entry     *entry;
    uint32              flags;
    
    L2CACHE_ENTRY_GET(unit, idx, entry);

    flags = entry->flags;
    if (!(flags & L2CACHE_ENTRY_VALID)) {
        LOG_DEBUG(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid entry at idx %d\n"),
                   FUNCTION_NAME(), idx));
        return BCM_E_PARAM;
    }

    if (flags & L2CACHE_ENTRY_DROP) {
        addr->flags |= BCM_L2_CACHE_DISCARD;
    }
    if (flags & L2CACHE_ENTRY_BYPASS) {
        addr->flags |= BCM_L2_CACHE_LOOKUP;
    }
    if (flags & L2CACHE_ENTRY_BPDU) {
        addr->flags |= BCM_L2_CACHE_BPDU;
    }
    if (flags & L2CACHE_ENTRY_COPY_TO_CPU) {
        addr->flags |= BCM_L2_CACHE_CPU;
    }
    if (flags & L2CACHE_ENTRY_PREFIX5_LOOKUP) {
        sal_memset(addr->mac_mask, 0xff, 5);
    }else if (flags & L2CACHE_ENTRY_DMAC){
        sal_memset(addr->mac_mask, 0xff, 6);
    }
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_cache_hw_entry_config_get(unit, idx, addr);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_cache_entry_config_set
 * Purpose:
 *      Create an FTE entry. If the dest modid is remote, use the provided 
 *      encap_id. Use the pass thru ohi if dest_modid id local & encap_id 
 *      is not provided
 * Parameters:
 *      unit        - Device number
 *      idx         - index of the l2 cache entry to be programmed
 *      addr        - config of the l2_cache address
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
_bcm_caladan3_l2_cache_entry_config_set(int unit, int idx, 
                                      bcm_l2_cache_addr_t *addr)
{
    int                         rv = BCM_E_NONE;
    uint32                      fte_idx = ~0;
    uint32                      flags;
    int                         mymodid;
    int                         allocate_fte = 0;
    _l2_cache_entry             *entry;
    int                         rsvd_dmac, mask_null, mask_em, mask_prefix5;
    
    L2CACHE_RESERVED_DMAC_CHECK(addr->mac, rsvd_dmac);
    L2CACHE_ENTRY_GET(unit, idx, entry);

    flags = addr->flags;
    if (flags == 0) {
        /* empty flags is not valid */
        return BCM_E_PARAM;
    }
    IS_NULL_MASK(addr->mac_mask, mask_null);
    IS_EXACT_MATCH_MASK(addr->mac_mask, mask_em);
    IS_PREFIX5_MASK(addr->mac_mask, mask_prefix5);
    if (!(mask_null || mask_em || mask_prefix5)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Unsupported mask null:%d em:%d p5:%d\n"),
                   FUNCTION_NAME(), mask_null, mask_em, mask_prefix5));
        return BCM_E_PARAM; /* unsupported mask */
    }
    if ((rsvd_dmac) && !(mask_null || mask_em || mask_prefix5)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Unsupported mask null:%d em:%d p5:%d rsvd_dmac:%d\n"), 
                   FUNCTION_NAME(), mask_null, mask_em, mask_prefix5, rsvd_dmac));
        return BCM_E_PARAM;
    }

    /* validity checks on flags */
    if ((flags & BCM_L2_CACHE_DISCARD) &&
        ((flags & BCM_L2_CACHE_TUNNEL) || (flags & BCM_L2_CACHE_MULTICAST))) {
        /* Discard is mutually exclusive with tunnel & multicast flags */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid flag combination (%x) with discard \n"),
                   FUNCTION_NAME(), flags));
        return BCM_E_PARAM;
    }
    if ((flags & BCM_L2_CACHE_LOOKUP) && (!rsvd_dmac)) {
        /* LOOKUP is valid only for rsvd-dmac packets */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: BCM_L2_CACHE_LOOKUP is valid only on rsvd dmacs \n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if ((flags & BCM_L2_CACHE_TUNNEL) && (flags & BCM_L2_CACHE_MULTICAST)) {
        /* tunnel & multicast are mutually exclusive */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: BCM_L2_CACHE_TUNNEL & BCM_L2_CACHE_MULTICAST are "
                               "mutually exclusive \n"), FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if (addr->src_port_mask && (flags & (~(BCM_L2_CACHE_CPU | 
                                           BCM_L2_CACHE_DISCARD |
					   BCM_L2_CACHE_LOOKUP |
                                           BCM_L2_CACHE_SUBTYPE)))) {
        /* src_port_mask is valid only with CPU or DISCARD options */
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: src_port_mask supported only with BCM_L2_CACHE_CPU,"
                               " BCM_L2_CACHE_DISCARD, BCM_L2_CACHE_LOOKUP & BCM_L2_CACHE_SUBTYPE options \n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if (addr->src_port_mask && (!rsvd_dmac)) {
        /* src_port_mask is support only with rsvd dmac addresses */
        
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: src_port_mask supported only with rsvd_dmac "
                               "addresses \n"), FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if (flags & BCM_L2_CACHE_LOOKUP) {
        /* requesting l2 lookup...mark l2cp table entry as bypass */
        /* Note: we will enter here only for rsvd (ieee) addresses */
        sal_memset(entry, 0, sizeof(_l2_cache_entry));
        entry->flags |= L2CACHE_ENTRY_VALID;
        if (!mask_prefix5) {
            entry->flags |= L2CACHE_ENTRY_L2CP;
        } else {
            entry->flags |= L2CACHE_ENTRY_DMAC;
            entry->flags |= L2CACHE_ENTRY_PREFIX5_LOOKUP;
        }
        if (addr->src_port_mask) {
            entry->src_port = addr->src_port_mask;
        } else {
            entry->src_port = addr->src_port;
        }
        entry->flags |= L2CACHE_ENTRY_BYPASS;

        if (addr->flags & BCM_L2_CACHE_MULTICAST) {
            if (!addr->group) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Group not specified with BCM_L2_CACHE_MULTICAST"
                                       "flag \n"), FUNCTION_NAME()));
                return BCM_E_PARAM;
            }
            /* encap_id is ignored */
            if (addr->encap_id) {
                LOG_DEBUG(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Ignoring encap_id. \n"),
                           FUNCTION_NAME()));
            }
            entry->flags |= L2CACHE_ENTRY_FTE_VALID;
            entry->flags |= L2CACHE_ENTRY_MULTICAST;
        }
        if (flags & BCM_L2_CACHE_SUBTYPE) {
            entry->flags |= L2CACHE_ENTRY_SUBTYPE;
        }
        if (flags & BCM_L2_CACHE_BPDU) {
            entry->flags |= L2CACHE_ENTRY_BPDU;
        }

        return BCM_E_NONE;
    }

    if (flags & BCM_L2_CACHE_DISCARD){
        /* discard flag set */
        sal_memset(entry, 0, sizeof(_l2_cache_entry));
        entry->flags |= L2CACHE_ENTRY_VALID;
        entry->flags |= L2CACHE_ENTRY_DROP;
        if (rsvd_dmac) {
            entry->flags |= L2CACHE_ENTRY_L2CP;
        }
        if (mask_prefix5) {
            entry->flags |= L2CACHE_ENTRY_PREFIX5_LOOKUP;
        }
        if (addr->src_port_mask) {
            entry->src_port = addr->src_port_mask;
        } else {
            entry->src_port = addr->src_port;
        }
        if (flags & BCM_L2_CACHE_SUBTYPE) {
            entry->flags |= L2CACHE_ENTRY_SUBTYPE;
        }

        return BCM_E_NONE; 
    }

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &mymodid));

    if (addr->flags & BCM_L2_CACHE_TUNNEL) {
        /* check if mpls gport */
        if (BCM_GPORT_IS_MPLS_PORT(addr->dest_port)) {
            fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(addr->dest_port);
            if (!_CALADAN3_MPLS_PORT_FTE_VALID(unit, fte_idx)) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Invalid mpls gport in dest_port (%d) \n"),
                           FUNCTION_NAME(), addr->dest_port));
                return BCM_E_PARAM;
            }
        } else {
            allocate_fte = 1;
            /* check if dest modid is local when encap_id is not specified */
            if ((addr->dest_modid != mymodid) && 
                (!SOC_SBX_IS_VALID_L2_ENCAP_ID(addr->encap_id))) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s:encap_id not specified for remote "
                                       "dest_modid\n"), FUNCTION_NAME()));
                return BCM_E_PARAM;
            }
        }
    }

    if (addr->flags & BCM_L2_CACHE_MULTICAST) {
        if (!addr->group) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Group not specified with BCM_L2_CACHE_MULTICAST"
                                   "flag \n"), FUNCTION_NAME()));
            return BCM_E_PARAM;
        }
        /* encap_id is ignored */
        if (addr->encap_id) {
            LOG_DEBUG(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Ignoring encap_id. \n"),
                       FUNCTION_NAME()));
        }
        allocate_fte = 1;
    }

    if (rv == BCM_E_NONE) {
        sal_memset(entry, 0, sizeof(_l2_cache_entry));
        entry->flags |= L2CACHE_ENTRY_VALID;
        entry->encap_id = addr->encap_id;
        if (rsvd_dmac) {
            entry->flags |= L2CACHE_ENTRY_L2CP;
        } else {
            entry->flags |= L2CACHE_ENTRY_DMAC;
        }
        if (allocate_fte) {
            entry->flags |= L2CACHE_ENTRY_FTE_VALID;
        }
        if (addr->flags & BCM_L2_CACHE_CPU) {
            entry->flags |= L2CACHE_ENTRY_COPY_TO_CPU;
        }
        if (addr->flags & BCM_L2_CACHE_MULTICAST) {
            entry->flags |= L2CACHE_ENTRY_MULTICAST;
        }
        if (flags & BCM_L2_CACHE_SUBTYPE) {
            entry->flags |= L2CACHE_ENTRY_SUBTYPE;
        }
        if (flags & BCM_L2_CACHE_BPDU) {
            entry->flags |= L2CACHE_ENTRY_BPDU;
        }
        if (mask_prefix5) {
            entry->flags |= L2CACHE_ENTRY_PREFIX5_LOOKUP;
        }
        if (BCM_GPORT_IS_MPLS_PORT(addr->dest_port)) {
            entry->flags |= L2CACHE_ENTRY_FTE_VALID;
            entry->flags |= L2CACHE_ENTRY_FTE_MPLS_GPORT;
            /* for MPLS gport, fte_idx is gport instead of fte idx */
            entry->fte_idx = addr->dest_port;
        }
        if (addr->src_port_mask) {
            entry->src_port = addr->src_port_mask;
        } else {
            entry->src_port = addr->src_port;
        }
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_cache_index_get
 * Purpose:
 *      Map from l2addr to index into l2_cache_entries table
 * Parameters:
 *      unit        - Device number
 *      addr        - l2 cache address to be programmed
 *      index_used  - index mapped (return)
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
_bcm_caladan3_l2_cache_index_get(int unit, bcm_l2_cache_addr_t *addr, int *real_idx)
{
    int rv = BCM_E_NONE;
    int port, subtype = 0;
    int rsvd_dmac;
    int mask_null, mask_em, mask_prefix5;

    COMPILER_REFERENCE(mask_null);
    COMPILER_REFERENCE(mask_em);
    COMPILER_REFERENCE(mask_prefix5);
    IS_NULL_MASK(addr->mac_mask, mask_null);
    IS_EXACT_MATCH_MASK(addr->mac_mask, mask_em);
    IS_PREFIX5_MASK(addr->mac_mask, mask_prefix5);
    L2CACHE_RESERVED_DMAC_CHECK(addr->mac, rsvd_dmac);

    /* for srcmask = ALLPORTS, return port 0 index
     * for subtype = ALLSUBTYPES, return subtype 0 index
     */
    if (addr->src_port_mask) {
        port = 0;
    }else{
        port = addr->src_port;
    }
    if (addr->flags & BCM_L2_CACHE_SUBTYPE) {
        if (addr->subtype == 0xff) {
            subtype = 0;
        }else{
            subtype = addr->subtype;
        }
    }

    if ((rsvd_dmac) & !(addr->flags & BCM_L2_CACHE_LOOKUP)){
        /* rsvd (ieee) address */
        if (addr->flags & BCM_L2_CACHE_SUBTYPE) {
            /* l2cpslow */
            *real_idx = L2CACHE_PORT_SUBTYPE_TO_INDEX(port, subtype);
        }else{
            /* l2cp */
            *real_idx = L2CACHE_PORT_LSB_TO_INDEX(port, addr->mac[5]);
        }
    }else{
        /* prefix5 or normal DMAC entry, assign a free index */
        rv = shr_idxres_list_alloc(_l2_cache[unit].idlist, 
                                   (uint32 *)real_idx);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }
        if ((*real_idx < L2CACHE_MAX_L2CP_IDX) || (*real_idx >= L2CACHE_MAX_IDX)) {
            return BCM_E_INTERNAL;
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_caladan3_l2_cache_entry_set
 * Purpose:
 *      Set an L2 cache entry using the input config
 *      This handles set to both L2CP entries & non-L2CP (DMAC) entries...which
 *      are used for reverse-dmac translation
 * Parameters:
 *      unit        - Device number
 *      index       - index to be used.
 *      addr        - l2 cache address to be programmed
 *      index_used  - if index is -1, index is assigned.
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
_bcm_caladan3_l2_cache_entry_set(int unit, int index, bcm_l2_cache_addr_t *addr, 
                               int *index_used)
{
    int                     real_idx = 0;
    int                     rv = BCM_E_NONE;
    _l2_cache_entry         *entry = NULL;

    if (addr->flags == 0) {
        /* empty flags is invalid*/
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid flags. \n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (index < 0) {
        /* calculate index */
        rv = _bcm_caladan3_l2_cache_index_get(unit, addr, &real_idx);
        if (rv != BCM_E_NONE) {
            return rv;
        }
        L2CACHE_ENTRY_GET(unit, real_idx, entry);
    } else {
        real_idx = index;
        if (real_idx >= L2CACHE_MAX_IDX) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Invalid index %d\n"),
                       FUNCTION_NAME(), real_idx));
            return BCM_E_PARAM;
        }
        L2CACHE_ENTRY_GET(unit, real_idx, entry);

        if (entry->flags & L2CACHE_ENTRY_VALID) {
            rv = _bcm_caladan3_l2_cache_entry_delete(unit, real_idx);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                         (BSL_META_U(unit,
                                "%s: Delete cache entry failed, index %d, rv %d\n"),
                          FUNCTION_NAME(), real_idx,rv));
            }
        }
    }

    /* _config_set API sets up the contents of entry */
    rv = _bcm_caladan3_l2_cache_entry_config_set(unit, real_idx, addr);
    if (rv != BCM_E_NONE) {
        if (index < 0) {
            shr_idxres_list_free(_l2_cache[unit].idlist, real_idx);
        }
        return rv;
    }
    if (index >= L2CACHE_MAX_L2CP_IDX) {
        rv = shr_idxres_list_reserve(_l2_cache[unit].idlist, real_idx, 
                                     real_idx);
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_l2_cache_hw_entry_set(unit, addr, real_idx);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    if (rv != BCM_E_NONE) {
        sal_memset(entry, 0, sizeof(_l2_cache_entry));
        shr_idxres_list_free(_l2_cache[unit].idlist, real_idx);
        return rv;
    } else {
        *index_used = real_idx;
    }

    return rv;
}


/*
 * Function:
 *      bcm_caladan3_l2_cache_delete
 * Purpose:
 *      Delete the l2 cache entry with the given index
 * Parameters:
 *      unit            - Device number
 *      index           - Index of the entry to retrieve
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
bcm_caladan3_l2_cache_delete(int unit, int index)
{
    int                         rv = BCM_E_NONE;
    
    if ((index < 0) || (index >= L2CACHE_MAX_IDX)) {
        return BCM_E_PARAM;
    }

    L2CACHE_INIT_CHECK(unit);
    
    L2CACHE_UNIT_LOCK(unit);
    rv = _bcm_caladan3_l2_cache_entry_delete(unit, index);
    L2CACHE_UNIT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_cache_delete_all
 * Purpose:
 *      Delete all l2 cache entries on the specified unit
 *      Note that this deletes only the user deleted entries
 * Parameters:
 *      unit            - Device number
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
bcm_caladan3_l2_cache_delete_all(int unit)
{
    int                 rv = BCM_E_NONE;
    int                 index = 0;
    _l2_cache_entry     *entry;

    L2CACHE_INIT_CHECK(unit);
    
    L2CACHE_UNIT_LOCK(unit);
    for (index=0; index<L2CACHE_MAX_IDX; index++) {
        L2CACHE_ENTRY_GET_RV(unit, index, entry, rv);
        if (!entry || (rv == BCM_E_PARAM)) {
            rv = BCM_E_INTERNAL;
            break;
        }
        if (entry->flags & L2CACHE_ENTRY_VALID) {
            rv = _bcm_caladan3_l2_cache_entry_delete(unit, index);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: Failed deleting entry with index - %d\n"), 
                           FUNCTION_NAME(), index));
                break;
            }
        }
    }
    L2CACHE_UNIT_UNLOCK(unit);

    return rv;
}

int
bcm_caladan3_l2_cache_detach(int unit)
{
    int rv = BCM_E_NONE;

#ifdef WARM_BOOT_TODO
    if (!SOC_WARM_BOOT(unit)) {
#endif 
    rv = bcm_caladan3_l2_cache_delete_all(unit);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error deleting existing entries. Init failed.\n"),
                   FUNCTION_NAME()));
        return rv;
    }
#ifdef WARM_BOOT_TODO
    }
#endif 

    if (_l2_cache[unit].idlist) {
        rv = shr_idxres_list_destroy(_l2_cache[unit].idlist);
        _l2_cache[unit].idlist = NULL;
    }

    if (rv == BCM_E_NONE && _l2_cache[unit].l2_cache_entries) {
        sal_free(_l2_cache[unit].l2_cache_entries);
        _l2_cache[unit].l2_cache_entries = NULL;
    }

    if (rv == BCM_E_NONE) {
        sal_mutex_destroy(_l2_cache[unit].lock);
        _l2_cache[unit].lock = NULL;
    } 

    /* sal_mutex_destroy(_l2_cache_glob_lock); */

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_cache_init
 * Purpose:
 *      Initialize the L2 Cache. By default, all entries in PortL2CP2Etc
 *      are valid and have copy-to-cpu set     
 * Parameters:
 *      unit   - Device number
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_UNIT      - Invalid unit
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_caladan3_l2_cache_init(int unit)
{
    int                         rv = BCM_E_NONE;
    _l2_cache_entry             *entry;
    sal_mutex_t                 local_lock;
    int                         idx;

    L2CACHE_UNIT_CHECK(unit);

    if (_l2_cache[unit].lock) {
        rv = bcm_caladan3_l2_cache_detach(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    if (!_l2_cache_glob_lock) {
        /* create and initialize the global lock */
        local_lock = sal_mutex_create("_l2_cache_glob_lock");
        if (!local_lock) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_create failed. \n"),
                       FUNCTION_NAME()));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(local_lock, sal_mutex_FOREVER) == 0) {
            /* initialize the global struct */
            sal_memset(&(_l2_cache), 0, sizeof(_l2_cache));
            if (sal_mutex_give(local_lock) != 0) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: sal_mutex_give failed.\n"),
                           FUNCTION_NAME()));
                sal_mutex_destroy(local_lock);
                return BCM_E_INTERNAL;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_take failed. \n"),
                       FUNCTION_NAME()));
            sal_mutex_destroy(local_lock);
            return BCM_E_INTERNAL;
        }
        _l2_cache_glob_lock = local_lock;
    }

    /* get global lock */
    if (sal_mutex_take(_l2_cache_glob_lock, sal_mutex_FOREVER) != 0) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: sal_mutex_take failed. \n"),
                   FUNCTION_NAME()));
        sal_mutex_destroy(_l2_cache_glob_lock);
        return BCM_E_INTERNAL;
    }
    
    if (_l2_cache[unit].lock) {
        /* re-init clears the existing entries */
        LOG_WARN(BSL_LS_BCM_L2,
                 (BSL_META_U(unit,
                             "%s: Re-initializing. Clearing all existing entries\n"),
                  FUNCTION_NAME()));
        rv = bcm_caladan3_l2_cache_delete_all(unit);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Error deleting existing entries. Init failed.\n"),
                       FUNCTION_NAME()));
        }
    } else {
        local_lock = sal_mutex_create("_l2_cache_unit_lock");
        if (!local_lock) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: mutex create failed. \n"),
                       FUNCTION_NAME()));
            rv = BCM_E_RESOURCE;
        }

        /* initialize L2CACHE_MAX_IDX */
        _l2_cache[unit].max_idx = SOC_SBX_CFG_CALADAN3(unit)->l2_cache_max_idx;

        if (rv == BCM_E_NONE) {
            /* first time. So, initialize the l2_cache_entries table */
            _l2_cache[unit].l2_cache_entries = 
                        sal_alloc((sizeof(_l2_cache_entry) * L2CACHE_MAX_IDX), 
                                  "l2_cache_entries");
            if (!_l2_cache[unit].l2_cache_entries) {
                rv = BCM_E_MEMORY;
            } else {
                /* clear all entries */
                for (idx = 0; ((idx < L2CACHE_MAX_IDX) && (rv != BCM_E_PARAM));
                     idx++) {
                    L2CACHE_ENTRY_GET_RV(unit, idx, entry, rv);
                    sal_memset(entry, 0, sizeof(_l2_cache_entry));
                }
            }
        }
        if (rv == BCM_E_NONE) {
            rv = shr_idxres_list_create(& _l2_cache[unit].idlist, L2CACHE_MAX_L2CP_IDX, 
                                        L2CACHE_MAX_IDX-1, 0, L2CACHE_MAX_IDX-1,
                                        "User created L2CACHE entries");
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "%s: list create for user L2CACHE entries failed."
                                       " Unit %d\n"), FUNCTION_NAME(), unit));
            }
        }

        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            if (_l2_cache[unit]._l2cpmac_table) {
                sal_free((void *)_l2_cache[unit]._l2cpmac_table);
            }
            rv = _bcm_caladan3_g3p1_l2_cache_init(unit, 
                                                &_l2_cache[unit].pass_thru_ohi,
                                                &_l2_cache[unit].cpu_fte,
                                                &_l2_cache[unit].l2_ete,
                                                &_l2_cache[unit]._l2cpmac_table);
            break;
#endif
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_CONFIG;
        }

        if (rv != BCM_E_NONE) {
            /* cleanup resources */
            if (local_lock) {
                sal_mutex_destroy(local_lock);
            }
            if (_l2_cache[unit].idlist) {
                shr_idxres_list_destroy(_l2_cache[unit].idlist);
                _l2_cache[unit].idlist = NULL;
            }
            if (_l2_cache[unit].l2_cache_entries) {
                sal_free(_l2_cache[unit].l2_cache_entries);
                _l2_cache[unit].l2_cache_entries = NULL;
            }
        } else {
            /* init succeeded */
            _l2_cache[unit].lock = local_lock;
        }
    }

    if (sal_mutex_give(_l2_cache_glob_lock) != 0) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: sal_mutex_give failed. \n"),
                   FUNCTION_NAME()));
        rv = BCM_E_INTERNAL;
    }

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_cache_size_get
 * Purpose:
 *      Get number of L2 cache entries
 * Parameters:
 *      unit - device number
 *      size - (OUT) number of entries in cache
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_caladan3_l2_cache_size_get(int unit, int *size)
{
    if (!size) {
        return BCM_E_PARAM;
    }

    *size = L2CACHE_MAX_IDX;
    

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_caladan3_l2_cache_set
 * Purpose:
 *      Set the l2 cache entry with the given config
 * Parameters:
 *      unit            - Device number
 *      index           - Index to be used
 *      addr            - l2 cache entry config
 *      index_used(OUT) - Actual index used.
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */

int 
bcm_caladan3_l2_cache_set(int unit, int index, bcm_l2_cache_addr_t *addr,
                        int *index_used)
{
    int                     rv = BCM_E_NONE;
    int                     rsvd_mac = 0;
    bcm_port_t              port;
    
    L2CACHE_UNIT_CHECK(unit);
    
    if ((!addr) || (!index_used)) {
        return BCM_E_PARAM;
    }

    L2CACHE_RESERVED_DMAC_CHECK(addr->mac, rsvd_mac);
    if ((addr->flags & BCM_L2_CACHE_SUBTYPE) && 
        (!rsvd_mac || (addr->mac[5] != 0x02))) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Invalid Mac address for IEEE slow protocol. \n"), 
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (!addr->src_port_mask) {
        /* src_port_mask is not specified */
        if (BCM_GPORT_IS_MPLS_PORT(addr->src_port)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Source GPort is not supported.\n")));
            return BCM_E_PARAM;
        }
        
        port = addr->src_port;
        if ((port < 0) || (port >= SBX_MAX_PORTS)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "%s: Invalid port %d. \n"),
                       FUNCTION_NAME(), port));
            return BCM_E_PARAM;
        }
    } else if (addr->src_port_mask == BCM_L2_SRCPORT_MASK_ALL) {
        if (addr->src_port) {
            /* src_port specified along with src_port_mask is ignored */
            LOG_WARN(BSL_LS_BCM_L2,
                     (BSL_META_U(unit,
                                 "%s: Ignoring the src_port specified \n"), 
                      FUNCTION_NAME()));
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Un-supported src_port_mask specified. \n")));
        return BCM_E_PARAM;
    }

    L2CACHE_UNIT_LOCK(unit);
    rv = _bcm_caladan3_l2_cache_entry_set(unit, index, addr, index_used);
    L2CACHE_UNIT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_caladan3_l2_cache_get
 * Purpose:
 *      Get the l2 cache entry with the given index
 * Parameters:
 *      unit            - Device number
 *      index           - Index of the entry to retrieve
 *      addr(OUT)       - l2 cache entry config of the index specified.
 * Returns:
 *      BCM_E_XXX       - Appropriately
 */
int
bcm_caladan3_l2_cache_get(int unit, int index, bcm_l2_cache_addr_t *addr)
{
    int                         rv = BCM_E_NONE;
    
    if ((addr == NULL) || (index >= L2CACHE_MAX_IDX) || (index < 0)) {
        return BCM_E_PARAM;
    }

    L2CACHE_INIT_CHECK(unit);
    bcm_l2_cache_addr_t_init(addr);
    
    L2CACHE_UNIT_LOCK(unit);
    rv = _bcm_caladan3_l2_cache_entry_config_get(unit, index, addr);
    L2CACHE_UNIT_UNLOCK(unit);

    if (rv != BCM_E_NONE) {
        LOG_DEBUG(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "%s: Error reading entry at index %d \n"), 
                   FUNCTION_NAME(), index));
        return rv;
    }

    return rv;
}
