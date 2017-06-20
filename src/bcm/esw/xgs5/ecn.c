/*
 * $Id: ecn.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    ecn.c
 * Purpose: Manages Tomahawk2 MPLS ECN functions
 */

#include <soc/defs.h>
#include <sal/core/libc.h>
#include <shared/bsl.h>
#include <bcm/error.h>

#ifdef INCLUDE_L3
#ifdef BCM_TOMAHAWK_SUPPORT
#include <soc/drv.h>
#include <soc/hash.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/triumph.h>
#include <soc/scache.h>

#include <bcm/mpls.h>
#include <bcm/ecn.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/xgs5.h>
#include <bcm_int/esw/mpls.h>
#include <bcm_int/esw/ecn.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw/trx.h>
/*
 * Software book keeping for ECN related information
 */
typedef struct _bcm_xgs5_ecn_bookkeeping_s {
    int         initialized;        /* Set to TRUE when ECN module initialized */
    SHR_BITDCL  *ing_tunnel_term_ecn_map_bitmap; /* Ingress tunnel term ecn map usage bitmap */
    uint32      *ing_tunnel_term_ecn_map_hw_idx;   /* Actual profile number used */    
    SHR_BITDCL  *ing_exp_to_ip_ecn_map_bitmap; /* Ingress_EXP_TO_ECN map usage bitmap */
    uint32      *ing_exp_to_ip_ecn_map_hw_idx;   /* Actual profile number used */
    SHR_BITDCL  *egr_ip_ecn_to_exp_map_bitmap; /* Egress_IP_ECN_TO_EXP map usage bitmap */
    uint32      *egr_ip_ecn_to_exp_map_hw_idx;   /* Actual profile number used */
    SHR_BITDCL  *egr_int_cn_to_exp_map_bitmap; /* Egress_INT_CN_TO_EXP map usage bitmap */
    uint32      *egr_int_cn_to_exp_map_hw_idx;   /* Actual profile number used */
    sal_mutex_t ecn_mutex;                     /* Protection mutex. */
} _bcm_xgs5_ecn_bookkeeping_t;

static _bcm_xgs5_ecn_bookkeeping_t  _bcm_xgs5_ecn_bk_info[BCM_MAX_NUM_UNITS] = {{ 0 }};
#define ECN_INFO(_unit_)   (&_bcm_xgs5_ecn_bk_info[_unit_])

#define _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP    \
            (soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / \
            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP)
/*
 * Ingress Tunnel term ECN map usage bitmap operations
 */
#define _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(_u_, _map_) \
        SHR_BITGET(ECN_INFO(_u_)->ing_tunnel_term_ecn_map_bitmap, (_map_))
#define _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_SET(_u_, _map_) \
        SHR_BITSET(ECN_INFO((_u_))->ing_tunnel_term_ecn_map_bitmap, (_map_))
#define _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_CLR(_u_, _map_) \
        SHR_BITCLR(ECN_INFO((_u_))->ing_tunnel_term_ecn_map_bitmap, (_map_))
/*
 * Ecn resource lock
 */
#define ECN_LOCK(unit) \
        sal_mutex_take(ECN_INFO(unit)->ecn_mutex, sal_mutex_FOREVER);

#define ECN_UNLOCK(unit) \
        sal_mutex_give(ECN_INFO(unit)->ecn_mutex);
        
#define ECN_INIT(unit)                                    \
    do {                                                  \
        if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {  \
            return BCM_E_UNIT;                            \
        }                                                 \
        if (!ECN_INFO(unit)->initialized) {                           \
            return BCM_E_INIT;                            \
        }                                                 \
    } while (0)

#define _INVALID_ECN_MAP_INDEX 0xffffffff

STATIC void bcmi_xgs5_ecn_free_resource(int unit);

#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT)
/*
 * Ingress EXP->IP ECN map usage bitmap operations
 */
#define _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(_u_, _map_) \
        SHR_BITGET(ECN_INFO(_u_)->ing_exp_to_ip_ecn_map_bitmap, (_map_))
#define _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_SET(_u_, _map_) \
        SHR_BITSET(ECN_INFO((_u_))->ing_exp_to_ip_ecn_map_bitmap, (_map_))
#define _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_CLR(_u_, _map_) \
        SHR_BITCLR(ECN_INFO((_u_))->ing_exp_to_ip_ecn_map_bitmap, (_map_))


/*
 * Egress IP ECN->EXP map usage bitmap operations
 */
#define _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(_u_, _map_) \
        SHR_BITGET(ECN_INFO(_u_)->egr_ip_ecn_to_exp_map_bitmap, (_map_))
#define _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_SET(_u_, _map_) \
        SHR_BITSET(ECN_INFO((_u_))->egr_ip_ecn_to_exp_map_bitmap, (_map_))
#define _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_CLR(_u_, _map_) \
        SHR_BITCLR(ECN_INFO((_u_))->egr_ip_ecn_to_exp_map_bitmap, (_map_))


/*
 * Egress INT CN->EXP map usage bitmap operations
 */
#define _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(_u_, _map_) \
        SHR_BITGET(ECN_INFO(_u_)->egr_int_cn_to_exp_map_bitmap, (_map_))
#define _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_SET(_u_, _map_) \
        SHR_BITSET(ECN_INFO((_u_))->egr_int_cn_to_exp_map_bitmap, (_map_))
#define _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_CLR(_u_, _map_) \
        SHR_BITCLR(ECN_INFO((_u_))->egr_int_cn_to_exp_map_bitmap, (_map_))

#define _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP    \
            (soc_mem_index_count(unit, ING_EXP_TO_IP_ECN_MAPPINGm) / \
            _BCM_ECN_MAX_ENTRIES_PER_MAP)

#define _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP    \
            (soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / \
            _BCM_ECN_MAX_ENTRIES_PER_MAP)

#define _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP    \
            (soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / \
            _BCM_ECN_MAX_ENTRIES_PER_MAP)

STATIC int
bcmi_xgs5_mpls_exp_to_ecn_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int table_num;
    int num_ecn_map;
    ing_exp_to_ip_ecn_mapping_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP];
    void *entries = NULL;
    int rv;
    uint32 index = _INVALID_ECN_MAP_INDEX;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    sal_memset(table, 0, sizeof(table));

    if (NULL == ecn_map_id) {
        return BCM_E_PARAM;
    }  
    num_ecn_map = soc_mem_index_count(unit, ING_EXP_TO_IP_ECN_MAPPINGm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (flags & BCM_ECN_MAP_WITH_ID) {
        int ecn_map_type;
        table_num = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK);
        ecn_map_type = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK);
        if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN) {
            return BCM_E_PARAM;
        }        
        if (table_num >= num_ecn_map) {
            return BCM_E_PARAM;
        }
        if (_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
            return BCM_E_EXISTS;
        }
        _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_SET(unit, table_num);
    } else {
        for (table_num = 0; table_num < num_ecn_map; table_num++) {
            if (!_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
                _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_SET(unit, table_num);
                *ecn_map_id = (table_num | _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN);
                break;
            }
        }
        if (table_num == num_ecn_map) {
            return BCM_E_FULL;
        }        
    }
    entries = &table;
    /* coverity[callee_ptr_arith : FALSE] */
    rv = _bcm_ing_exp_to_ip_ecn_map_entry_add(unit, &entries, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              &index);
    if (BCM_SUCCESS(rv)) {
        ecn_info->ing_exp_to_ip_ecn_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;
    }
    return rv;   
}

STATIC int
bcmi_xgs5_mpls_exp_to_ecn_map_destroy(int unit, int ecn_map_id)
{
    int num_ecn_map, table_num;    
    int base_idx;
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    num_ecn_map = soc_mem_index_count(unit, ING_EXP_TO_IP_ECN_MAPPINGm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }
    base_idx = ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP;
    BCM_IF_ERROR_RETURN(_bcm_ing_exp_to_ip_ecn_map_entry_delete(unit, base_idx));   
    _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_CLR(unit, table_num); 
    return BCM_E_NONE;
}

STATIC int
bcmi_xgs5_mpls_exp_to_ecn_map_update_index(int unit,int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    void    *entries[1];
    int     rv = BCM_E_NONE;
    uint32  index;   
    int     offset;
    ing_exp_to_ip_ecn_mapping_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP]; 
    int     table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    sal_memset(table, 0, sizeof(table));
    index = ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[table_num] *
            _BCM_ECN_MAX_ENTRIES_PER_MAP;

    entries[0] = &table;
    rv = _bcm_ing_exp_to_ip_ecn_map_entry_get(unit, index, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              entries);
    offset = (ecn_map->ecn << 3) + ecn_map->exp; 
    if (BCM_SUCCESS(rv)) {
        ing_exp_to_ip_ecn_mapping_entry_t *entry = &table[offset];
        soc_ING_EXP_TO_IP_ECN_MAPPINGm_field32_set(unit, entry, 
                                                   ECNf, ecn_map->new_ecn);
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_INGRESS_DROP) {
            soc_ING_EXP_TO_IP_ECN_MAPPINGm_field32_set(unit, entry, 
                                                       DROPf, 1);
        } else {
            soc_ING_EXP_TO_IP_ECN_MAPPINGm_field32_set(unit, entry, 
                                                       DROPf, 0);
        }
        rv = _bcm_ing_exp_to_ip_ecn_map_entry_delete(unit, (int)index);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_ing_exp_to_ip_ecn_map_entry_add(unit, entries,
                                                _BCM_ECN_MAX_ENTRIES_PER_MAP,
                                                &index);
        ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;  
    }
    return rv;
}

STATIC int
bcmi_xgs5_mpls_exp_to_ecn_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int table_num;

    /* Check validity of ecn_map_id */
    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    if (ecn_map == NULL) {
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
       
    if (!_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    if ((ecn_map->ecn > 3) || (ecn_map->exp > 7) || (ecn_map->new_ecn > 3)) {
        return BCM_E_PARAM;
    }   
    return bcmi_xgs5_mpls_exp_to_ecn_map_update_index(unit, ecn_map_id, ecn_map);;
}


STATIC int
bcmi_xgs5_mpls_exp_to_ecn_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int num_ecn_map, index, table_num;
    ing_exp_to_ip_ecn_mapping_entry_t ing_exp_to_ip_ecn_map;

    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;

    num_ecn_map = soc_mem_index_count(unit, ING_EXP_TO_IP_ECN_MAPPINGm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;

    if (ecn_map == NULL || table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    /* Get the base index for this ECN map */
    index = (ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP) + 
            (ecn_map->ecn << 3) + ecn_map->exp;  
    
    /* Commit to HW */
    BCM_IF_ERROR_RETURN(READ_ING_EXP_TO_IP_ECN_MAPPINGm(unit, MEM_BLOCK_ANY,
                                                        index, 
                                                        &ing_exp_to_ip_ecn_map));
    ecn_map->new_ecn = soc_mem_field32_get(unit, ING_EXP_TO_IP_ECN_MAPPINGm, 
                                           &ing_exp_to_ip_ecn_map, ECNf);
    if (soc_mem_field32_get(unit, ING_EXP_TO_IP_ECN_MAPPINGm,
                            &ing_exp_to_ip_ecn_map, DROPf)) {
        ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_INGRESS_DROP;           
    }
    
    return BCM_E_NONE;
}


STATIC int
bcmi_xgs5_mpls_ecn_to_exp_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int table_num;
    int num_ecn_map;
    egr_ip_ecn_to_exp_mapping_table_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP];
    void       *entries = NULL;
    int rv;
    uint32 index = _INVALID_ECN_MAP_INDEX;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    sal_memset(table, 0, sizeof(table)); 
    /* Input parameters check. */
    if (NULL == ecn_map_id) {
        return BCM_E_PARAM;
    }   
    num_ecn_map = soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (flags & BCM_ECN_MAP_WITH_ID) {
        int ecn_map_type;
        table_num = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK);
        ecn_map_type = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK);
        if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
            return BCM_E_PARAM;
        }
        if (table_num >= num_ecn_map) {
            return BCM_E_PARAM;
        }
        if (_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_EXISTS;
        }
        _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_SET(unit, table_num);
    } else {
        for (table_num = 0; table_num < num_ecn_map; table_num++) {
            if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
                _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_SET(unit, table_num);
                *ecn_map_id = (table_num | _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP);
                break;
            }
        }
        if (table_num == num_ecn_map) {
            return BCM_E_FULL;
        }           
    }
    entries = &table;
    /* coverity[callee_ptr_arith : FALSE] */
    rv = _bcm_egr_ip_ecn_to_exp_map_entry_add(unit, &entries, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              &index);
    if (BCM_SUCCESS(rv)) {
        ecn_info->egr_ip_ecn_to_exp_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;
    }
    return rv; 

}


STATIC int
bcmi_xgs5_mpls_ecn_to_exp_map_destroy(int unit, int ecn_map_id)
{
    int num_ecn_map, table_num;   
    int base_idx; 
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    num_ecn_map = soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }
    base_idx = ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP;
    BCM_IF_ERROR_RETURN(_bcm_egr_ip_ecn_to_exp_map_entry_delete(unit, base_idx));   
    _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_CLR(unit, table_num); 
    return BCM_E_NONE;
}

STATIC int
bcmi_xgs5_mpls_ecn_to_exp_map_update_index(int unit,int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    void    *entries[1];
    int     rv = BCM_E_NONE;
    uint32  index;   
    int     offset;
    egr_ip_ecn_to_exp_mapping_table_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP]; 
    int     table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    sal_memset(table, 0, sizeof(table));
    index = ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[table_num] *
            _BCM_ECN_MAX_ENTRIES_PER_MAP;

    entries[0] = &table;
    rv = _bcm_egr_ip_ecn_to_exp_map_entry_get(unit, index, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              entries);
    offset = (ecn_map->exp << 2) + ecn_map->ecn;  
    if (BCM_SUCCESS(rv)) {
        egr_ip_ecn_to_exp_mapping_table_entry_t *entry = &table[offset];
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_RESPONSIVE) {
            soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, entry, 
                                                             RESPONSIVE_EXPf, 
                                                             ecn_map->new_exp);
            if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP) {    
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 RESPONSIVE_DROPf, 
                                                                 1);
            } else {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 RESPONSIVE_DROPf, 
                                                                 0);        
            }
            if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING) {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 RESPONSIVE_CHANGE_PACKET_EXPf, 
                                                                 1);
            } else {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 RESPONSIVE_CHANGE_PACKET_EXPf, 
                                                                 0);
            }
        
        } else {
            soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, entry, 
                                                             EXPf, ecn_map->new_exp);
            if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP) {    
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 DROPf, 1);
            } else {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 DROPf, 0);
            }
            if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING) {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 CHANGE_PACKET_EXPf, 
                                                                 1);
            } else {
                soc_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                                 entry, 
                                                                 CHANGE_PACKET_EXPf, 
                                                                 0);
            }
        }
        rv = _bcm_egr_ip_ecn_to_exp_map_entry_delete(unit, (int)index);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_egr_ip_ecn_to_exp_map_entry_add(unit, entries,
                                                _BCM_ECN_MAX_ENTRIES_PER_MAP,
                                                &index);
        ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;  
    }
    return rv;
}

STATIC int
bcmi_xgs5_mpls_ecn_to_exp_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int table_num;
    /* Check validity of exp_map_id */
    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    if (ecn_map == NULL) {
        /* invalid pointer to map data */
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
       
    if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    if ((ecn_map->ecn > 3) || (ecn_map->exp > 7) || 
        (ecn_map->new_exp > 7)) {
        return BCM_E_PARAM;
    }
    return bcmi_xgs5_mpls_ecn_to_exp_map_update_index(unit, ecn_map_id, ecn_map);
}

STATIC int
bcmi_xgs5_mpls_ecn_to_exp_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int num_ecn_map, index, table_num;
    egr_ip_ecn_to_exp_mapping_table_entry_t egr_ip_ecn_to_exp_map;

    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;

    num_ecn_map = soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;

    if ((ecn_map == NULL) || (table_num >= num_ecn_map)) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    /* Get the base index for this ECN map */
    index = (ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP) + 
            (ecn_map->exp << 2) + ecn_map->ecn;  

    /* Commit to HW */
    BCM_IF_ERROR_RETURN(READ_EGR_IP_ECN_TO_EXP_MAPPING_TABLEm(unit, MEM_BLOCK_ANY,
                                                        index, 
                                                        &egr_ip_ecn_to_exp_map));
    if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_RESPONSIVE) {
        ecn_map->new_exp = soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm, 
                                               &egr_ip_ecn_to_exp_map, RESPONSIVE_EXPf);
        if (soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm,
                                &egr_ip_ecn_to_exp_map, RESPONSIVE_DROPf)) {
            ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP;
        }
        if (soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm,
                                &egr_ip_ecn_to_exp_map, RESPONSIVE_CHANGE_PACKET_EXPf)) {
            ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING;
        }   

    } else {
        ecn_map->new_exp = soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm, 
                                               &egr_ip_ecn_to_exp_map, EXPf);
        if (soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm,
                                &egr_ip_ecn_to_exp_map, DROPf)) {
            ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP;
        }
        if (soc_mem_field32_get(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm,
                                &egr_ip_ecn_to_exp_map, CHANGE_PACKET_EXPf)) {
            ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING;
        }   
    }
    return BCM_E_NONE;
}


STATIC int
bcmi_xgs5_mpls_int_cn_to_exp_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int table_num;
    int num_ecn_map;
    egr_int_cn_to_exp_mapping_table_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP];
    void *entries = NULL;
    int rv;
    uint32 index = _INVALID_ECN_MAP_INDEX;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);    
    sal_memset(table, 0, sizeof(table));
    /* Input parameters check. */
    if (NULL == ecn_map_id) {
        return BCM_E_PARAM;
    }   
    num_ecn_map = soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (flags & BCM_ECN_MAP_WITH_ID) {
        int ecn_map_type;
        table_num = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK);
        ecn_map_type = (*ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK);
        if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
            return BCM_E_PARAM;
        }
        if (table_num >= num_ecn_map) {
            return BCM_E_PARAM;
        }
        if (_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_EXISTS;
        }
        _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_SET(unit, table_num);
    } else {
        for (table_num = 0; table_num < num_ecn_map; table_num++) {
            if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
                _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_SET(unit, table_num);
                *ecn_map_id = (table_num | _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP);
                break;
            }
        }
        if (table_num == num_ecn_map) {
            return BCM_E_FULL;
        }        
    }
    entries = &table;
    /* coverity[callee_ptr_arith : FALSE] */
    rv = _bcm_egr_int_cn_to_exp_map_entry_add(unit, &entries, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              &index);
    if (BCM_SUCCESS(rv)) {
        ecn_info->egr_int_cn_to_exp_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;
    }
    return rv;   

}


STATIC int
bcmi_xgs5_mpls_int_cn_to_exp_map_destroy(int unit, int ecn_map_id)
{
    int num_ecn_map, table_num;   
    int base_idx; 
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    num_ecn_map = soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;
    if (table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }
    base_idx = ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP;
    BCM_IF_ERROR_RETURN(_bcm_egr_int_cn_to_exp_map_entry_delete(unit, base_idx));     
    _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_CLR(unit, table_num); 
    return BCM_E_NONE;
}

STATIC int
bcmi_xgs5_mpls_int_cn_to_exp_map_update_index(int unit,int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    void    *entries[1];
    int     rv = BCM_E_NONE;
    uint32  index;   
    int     offset;
    egr_int_cn_to_exp_mapping_table_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_MAP]; 
    int     table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    sal_memset(table, 0, sizeof(table));
    index = ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[table_num] *
            _BCM_ECN_MAX_ENTRIES_PER_MAP;

    entries[0] = &table;
    rv = _bcm_egr_int_cn_to_exp_map_entry_get(unit, index, 
                                              _BCM_ECN_MAX_ENTRIES_PER_MAP, 
                                              entries);
    offset = (ecn_map->exp << 2) + ecn_map->int_cn; 
    if (BCM_SUCCESS(rv)) {
        egr_int_cn_to_exp_mapping_table_entry_t *entry = &table[offset];
        soc_EGR_INT_CN_TO_EXP_MAPPING_TABLEm_field32_set(unit, entry, 
                                                         EXPf, ecn_map->new_exp);
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP) { 
            soc_EGR_INT_CN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                             entry, 
                                                             DROPf, 1);
        } else {
            soc_EGR_INT_CN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                             entry, 
                                                             DROPf, 0);
        }
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING) { 
            soc_EGR_INT_CN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                             entry, 
                                                             CHANGE_PACKET_EXPf, 
                                                             1);
        } else {
            soc_EGR_INT_CN_TO_EXP_MAPPING_TABLEm_field32_set(unit, 
                                                             entry, 
                                                             CHANGE_PACKET_EXPf, 
                                                             0);
        }

        rv = _bcm_egr_int_cn_to_exp_map_entry_delete(unit, (int)index);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_egr_int_cn_to_exp_map_entry_add(unit, entries,
                                                  _BCM_ECN_MAX_ENTRIES_PER_MAP,
                                                  &index);
        ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_MAP;  
    }
    return rv;
}

STATIC int
bcmi_xgs5_mpls_int_cn_to_exp_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int table_num;
    /* Check validity of exp_map_id */
    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    if (ecn_map == NULL) {
        /* invalid pointer to map data */
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
       
    if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    if ((ecn_map->int_cn > 3) || (ecn_map->exp > 7) || (ecn_map->new_exp > 7)) {
        return BCM_E_PARAM;
    }
    return bcmi_xgs5_mpls_int_cn_to_exp_map_update_index(unit, ecn_map_id, ecn_map);
}

STATIC int
bcmi_xgs5_mpls_int_cn_to_exp_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int num_ecn_map, index, table_num;
    egr_int_cn_to_exp_mapping_table_entry_t egr_int_cn_to_exp_map;

    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;

    num_ecn_map = soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_MAP;

    if (ecn_map == NULL || table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    /* Get the base index for this ECN map */
    index = (ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_MAP) + 
            (ecn_map->exp << 2) + ecn_map->int_cn;  

    /* Commit to HW */
    BCM_IF_ERROR_RETURN(READ_EGR_INT_CN_TO_EXP_MAPPING_TABLEm(unit, MEM_BLOCK_ANY,
                                                              index, 
                                                              &egr_int_cn_to_exp_map));
    ecn_map->new_exp = soc_mem_field32_get(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm, 
                                           &egr_int_cn_to_exp_map, EXPf);
    if (soc_mem_field32_get(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm,
                            &egr_int_cn_to_exp_map, DROPf)) {
        ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_DROP;
    }
    if (soc_mem_field32_get(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm,
                            &egr_int_cn_to_exp_map, CHANGE_PACKET_EXPf)) {
        ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_EGRESS_EXP_MARKING;
    }
    return BCM_E_NONE;
}



int
bcmi_xgs5_mpls_ecn_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int rv = BCM_E_PARAM;
    /* Input parameters check. */
    ECN_INIT(unit);
    if (NULL == ecn_map_id) {
        return rv;
    } 
    ECN_LOCK(unit);   
    if (flags & BCM_ECN_MAP_INGRESS) {
        if (flags & BCM_ECN_MAP_MPLS) {
            rv = bcmi_xgs5_mpls_exp_to_ecn_map_create(unit, flags, ecn_map_id);
        }
    } else if (flags & BCM_ECN_MAP_EGRESS) {
        if (flags & BCM_ECN_MAP_MPLS) {
            rv = bcmi_xgs5_mpls_ecn_to_exp_map_create(unit, flags, ecn_map_id);
        } else if (flags & BCM_ECN_MAP_MPLS_INT_CN_TO_EXP) {
            rv = bcmi_xgs5_mpls_int_cn_to_exp_map_create(unit, flags, ecn_map_id);
        }
    }
    ECN_UNLOCK(unit);
    return rv;
}
int 
bcmi_xgs5_mpls_ecn_map_destroy(int unit, int ecn_map_id)
{
    int rv = BCM_E_PARAM; 
    ECN_INIT(unit); 
    if (ecn_map_id < 0) {
        return rv;
    }
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN){
        rv = bcmi_xgs5_mpls_exp_to_ecn_map_destroy(unit, ecn_map_id);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
        rv = bcmi_xgs5_mpls_ecn_to_exp_map_destroy(unit, ecn_map_id);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
        rv = bcmi_xgs5_mpls_int_cn_to_exp_map_destroy(unit, ecn_map_id);
    }
    ECN_UNLOCK(unit);
    return rv;
}

int 
bcmi_xgs5_mpls_ecn_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int rv = BCM_E_PARAM; 
    ECN_INIT(unit); 
    if (ecn_map_id < 0) {
        return rv;
    }
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN) {
        rv = bcmi_xgs5_mpls_exp_to_ecn_map_set(unit, ecn_map_id, ecn_map);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
        rv = bcmi_xgs5_mpls_ecn_to_exp_map_set(unit, ecn_map_id, ecn_map);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
        rv = bcmi_xgs5_mpls_int_cn_to_exp_map_set(unit, ecn_map_id, ecn_map);
    }
    ECN_UNLOCK(unit);
    return rv;

}

int 
bcmi_xgs5_mpls_ecn_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int rv = BCM_E_PARAM;
    ECN_INIT(unit);
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN) {
        rv = bcmi_xgs5_mpls_exp_to_ecn_map_get(unit, ecn_map_id, ecn_map);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
        rv = bcmi_xgs5_mpls_ecn_to_exp_map_get(unit, ecn_map_id, ecn_map);
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
        rv = bcmi_xgs5_mpls_int_cn_to_exp_map_get(unit, ecn_map_id, ecn_map);
    }
    ECN_UNLOCK(unit);
    return rv;    
}

int 
bcmi_xgs5_mpls_ecn_port_map_info_get(int unit, bcm_port_t port, 
                                     bcm_ecn_port_map_t *ecn_map)

{  
    int mymodid = -1;
    int modbase = -1;
    int index = -1;
    egr_gpp_attributes_modbase_entry_t modbase_entry;
    egr_gpp_attributes_entry_t egr_gpp_attributes;
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
    
    BCM_IF_ERROR_RETURN(
        READ_EGR_GPP_ATTRIBUTES_MODBASEm(unit, MEM_BLOCK_ANY, mymodid, 
                                         &modbase_entry));
    modbase = 
        soc_EGR_GPP_ATTRIBUTES_MODBASEm_field32_get(unit, &modbase_entry, 
                                                    BASEf);
    index = port + modbase;
    BCM_IF_ERROR_RETURN(
        READ_EGR_GPP_ATTRIBUTESm(unit, MEM_BLOCK_ANY, index, 
                                 &egr_gpp_attributes));
    
    if (ecn_map->flags == BCM_ECN_EGRESS_PORT_ECN_TO_EXP_MAP) {
        ecn_map->ecn_map_id = 
            soc_EGR_GPP_ATTRIBUTESm_field32_get(unit, &egr_gpp_attributes, 
                                                IP_ECN_TO_EXP_MAPPING_PTRf);
        if (_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, ecn_map->ecn_map_id)) {
            ecn_map->ecn_map_id |= _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP;
        } else {
            return BCM_E_PARAM;
        }
        
    } else if (ecn_map->flags == BCM_ECN_EGRESS_PORT_INT_CN_TO_EXP_MAP) {
        ecn_map->ecn_map_id = 
            soc_EGR_GPP_ATTRIBUTESm_field32_get(unit, &egr_gpp_attributes, 
                                                INT_CN_TO_EXP_MAPPING_PTRf); 
        if (_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, ecn_map->ecn_map_id)) {
            ecn_map->ecn_map_id |= _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP;
        } else {
            return BCM_E_PARAM;
        }
    }  
    return BCM_E_NONE;
}

int
bcmi_xgs5_mpls_ecn_port_map_get(int unit, bcm_gport_t port, 
                                bcm_ecn_port_map_t *ecn_map)
{
    int rv = BCM_E_PORT;
    ECN_INIT(unit);
    ECN_LOCK(unit);
    if (BCM_GPORT_IS_SET(port) && BCM_GPORT_IS_MODPORT(port)) {
        rv = bcm_esw_port_local_get(unit, port, &port);
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PORT;
            goto clean_up;
        }
        rv = bcmi_xgs5_mpls_ecn_port_map_info_get(unit, port, ecn_map);
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
    }
clean_up:
    ECN_UNLOCK(unit);
    return rv;    
}

int
bcmi_xgs5_mpls_ecn_port_map_info_set(int unit, bcm_port_t port, 
                                     bcm_ecn_port_map_t *ecn_map)
{
    int ecn_map_index = -1;
    int ecn_map_type = -1;
    int ecn_map_num = -1;
    int mymodid = -1;
    int modbase = -1;
    int index = -1;
    egr_gpp_attributes_modbase_entry_t modbase_entry;
    egr_gpp_attributes_entry_t egr_gpp_attributes;
    ecn_map_type = ecn_map->ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK;
    ecn_map_index = ecn_map->ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    if (ecn_map->flags == BCM_ECN_EGRESS_PORT_ECN_TO_EXP_MAP) {
        if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
            return BCM_E_PARAM;
        }
        ecn_map_num = 
            soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / 
            _BCM_ECN_MAX_ENTRIES_PER_MAP;
        if (ecn_map_index >= ecn_map_num) {
            return BCM_E_PARAM;
        }
        if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit,ecn_map_index)) {
            return BCM_E_PARAM;
        }
        /* Commit to HW */
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
        
        BCM_IF_ERROR_RETURN(
            READ_EGR_GPP_ATTRIBUTES_MODBASEm(unit, MEM_BLOCK_ANY, mymodid, 
                                             &modbase_entry));
        modbase = 
            soc_EGR_GPP_ATTRIBUTES_MODBASEm_field32_get(unit, &modbase_entry, 
                                                        BASEf);
        index = port + modbase;
        BCM_IF_ERROR_RETURN(
            READ_EGR_GPP_ATTRIBUTESm(unit, MEM_BLOCK_ANY, index, 
                                     &egr_gpp_attributes));
        
        
        soc_EGR_GPP_ATTRIBUTESm_field32_set(unit, &egr_gpp_attributes, 
                                            IP_ECN_TO_EXP_MAPPING_PTRf, 
                                            ecn_map_index);
        soc_EGR_GPP_ATTRIBUTESm_field32_set(unit, &egr_gpp_attributes, 
                                            IP_ECN_TO_EXP_PRIORITYf, 
                                            1);    
    } else if (ecn_map->flags == BCM_ECN_EGRESS_PORT_INT_CN_TO_EXP_MAP) {
        if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
            return BCM_E_PARAM;
        }
        ecn_map_num = 
            soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / 
            _BCM_ECN_MAX_ENTRIES_PER_MAP;
        if (ecn_map_index >= ecn_map_num) {
            return BCM_E_PARAM;
        }
        if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit,ecn_map_index)) {
            return BCM_E_PARAM;
        }
        /* Commit to HW */
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
        
        BCM_IF_ERROR_RETURN(
            READ_EGR_GPP_ATTRIBUTES_MODBASEm(unit, MEM_BLOCK_ANY, mymodid, 
                                             &modbase_entry));
        modbase = 
            soc_EGR_GPP_ATTRIBUTES_MODBASEm_field32_get(unit, &modbase_entry, 
                                                        BASEf);
        index = port + modbase;
        BCM_IF_ERROR_RETURN(
            READ_EGR_GPP_ATTRIBUTESm(unit, MEM_BLOCK_ANY, index, 
                                     &egr_gpp_attributes));
        
        
        soc_EGR_GPP_ATTRIBUTESm_field32_set(unit, &egr_gpp_attributes, 
                                            INT_CN_TO_EXP_MAPPING_PTRf, 
                                            ecn_map_index);
        soc_EGR_GPP_ATTRIBUTESm_field32_set(unit, &egr_gpp_attributes, 
                                            INT_CN_TO_EXP_PRIORITYf, 
                                            1);
    } else {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(
        WRITE_EGR_GPP_ATTRIBUTESm(unit, MEM_BLOCK_ALL, index, 
                                  &egr_gpp_attributes));
    return BCM_E_NONE;

}

int
bcmi_xgs5_mpls_ecn_port_map_set(int unit, bcm_gport_t port, 
                                bcm_ecn_port_map_t *ecn_map)
{
    int rv = BCM_E_PORT;
    ECN_INIT(unit);
    ECN_LOCK(unit);
    if (BCM_GPORT_IS_SET(port) && BCM_GPORT_IS_MODPORT(port)) {
        rv = bcm_esw_port_local_get(unit, port, &port); 
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PORT;
            goto clean_up;
        }
        rv =  bcmi_xgs5_mpls_ecn_port_map_info_set(unit, port, ecn_map);
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
    }
clean_up:    
    ECN_UNLOCK(unit);
    return rv;    
}
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */

STATIC int
bcmi_xgs5_tunnel_term_ecn_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int table_num;
    int num_ecn_map;
    ing_tunnel_ecn_decap_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP];
    void       *entries = NULL;
    int rv;
    uint32 index = _INVALID_ECN_MAP_INDEX;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    sal_memset(table, 0, sizeof(table));
    if (NULL == ecn_map_id) {
        return BCM_E_PARAM;
    }  
    num_ecn_map = soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
    if (flags & BCM_ECN_MAP_WITH_ID) {
        int ecn_map_type;
        table_num = (*ecn_map_id & _BCM_XGS5_ECN_MAP_NUM_MASK);
        ecn_map_type = (*ecn_map_id & _BCM_XGS5_ECN_MAP_TYPE_MASK);
        if (ecn_map_type != _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
            return BCM_E_PARAM;
        }        
        if (table_num >= num_ecn_map) {
            return BCM_E_PARAM;
        }
        if (_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
            return BCM_E_EXISTS;
        }
        _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_SET(unit, table_num);
    } else {
        for (table_num = 0; table_num < num_ecn_map; table_num++) {
            if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
                _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_SET(unit, table_num);
                *ecn_map_id = (_BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM | table_num);
                break;
            }
        }
        if (table_num == num_ecn_map) {
            return BCM_E_FULL;
        }
    }

    entries = &table;
    /* coverity[callee_ptr_arith : FALSE] */
    rv = _bcm_ing_tunnel_term_map_entry_add(unit, &entries, 
                                            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP, 
                                            &index);
    if (BCM_SUCCESS(rv)) {
        ecn_info->ing_tunnel_term_ecn_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
    }
    return rv;
}

STATIC int 
bcmi_xgs5_tunnel_term_ecn_map_destroy(int unit, int ecn_map_id)
{
    int num_ecn_map, table_num;  
    int base_idx;
    table_num = ecn_map_id & _BCM_XGS5_ECN_MAP_NUM_MASK;
    num_ecn_map = soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
    if (table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }
    base_idx = ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[table_num] * _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
    BCM_IF_ERROR_RETURN(_bcm_ing_tunnel_term_map_entry_delete(unit, base_idx));
    _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_CLR(unit, table_num);
    return BCM_E_NONE;
}

/*
 * Function: 
 *     bcmi_xgs5_tunnel_term_map_update_index
 * Purpose:
 *     Update an existing profile (or a newly created one) in the hardware's
 *     table with a new mapping.  
 *   
 *     The structure is: (1) get existing profile for this map from hardware, 
 *     (2) update the profile in RAM, (3) delete the old profile from hardware, 
 *     and (4) push the updated profile in RAM to hardware.
 * Parameters:
 *     unit   - (IN) The unit being operated on
 *     ecn_map    - (IN) Map structure sent in by app to be applied to hardware
 *     ecn_map_id - (IN) Map ID sent in by app to be applied to hardware
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
bcmi_xgs5_tunnel_term_map_update_index(int unit,int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    void    *entries[1];
    int     rv = BCM_E_NONE;
    uint32  index;   
    int     offset;
    ing_tunnel_ecn_decap_entry_t table[_BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP]; 
    int     table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
    index = ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[table_num] *
            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
    sal_memset(table, 0, sizeof(table));
    entries[0] = &table;
    rv = _bcm_ing_tunnel_term_map_entry_get(unit, index, 
                                            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP, 
                                            entries);
    offset = (ecn_map->ecn << 2) + ecn_map->inner_ecn;
    if (BCM_SUCCESS(rv)) {
        ing_tunnel_ecn_decap_entry_t *entry = &table[offset];
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_INGRESS_DROP) { 
            soc_ING_TUNNEL_ECN_DECAPm_field32_set(unit, entry, 
                                                  DROPf, 1);
        } else {
            soc_ING_TUNNEL_ECN_DECAPm_field32_set(unit, entry, 
                                                  DROPf, 0);
        }
        if (ecn_map->action_flags & BCM_ECN_TRAFFIC_ACTION_INGRESS_ECN_MARKING) { 
            soc_ING_TUNNEL_ECN_DECAPm_field32_set(unit, entry, 
                                                  CHANGE_INNER_ECNf, 1);
        } else {
            soc_ING_TUNNEL_ECN_DECAPm_field32_set(unit, entry, 
                                                  CHANGE_INNER_ECNf, 0);
        }  
        soc_ING_TUNNEL_ECN_DECAPm_field32_set(unit, entry, 
                                              INNER_ECNf, ecn_map->new_ecn);      
        rv = _bcm_ing_tunnel_term_map_entry_delete(unit, (int)index);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_ing_tunnel_term_map_entry_add(unit, entries,
                                                _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP,
                                                &index);
        ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[table_num] = 
            index / _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;  
    }
    return rv;
}

STATIC int
bcmi_xgs5_tunnel_term_ecn_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map)
{
    int table_num;
    /* Check validity of exp_map_id */
    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    if (ecn_map == NULL) {
        /* invalid pointer to map data */
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_ECN_MAP_NUM_MASK;
       
    if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    if ((ecn_map->ecn > 3) || (ecn_map->inner_ecn > 3) || (ecn_map->new_ecn > 3)) {
        return BCM_E_PARAM;
    }

    return bcmi_xgs5_tunnel_term_map_update_index(unit, ecn_map_id, ecn_map);
}

STATIC int
bcmi_xgs5_tunnel_term_ecn_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map) 
{
    int num_ecn_map, index, table_num;
    ing_tunnel_ecn_decap_entry_t ing_tunnel_ecn_decap_map;

    if (ecn_map_id < 0) {
        return BCM_E_PARAM;
    }
    table_num = ecn_map_id & _BCM_XGS5_ECN_MAP_NUM_MASK;

    num_ecn_map = soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / 
                  _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;

    if (ecn_map == NULL || table_num >= num_ecn_map) {
        return BCM_E_PARAM;
    }
    if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
        return BCM_E_NOT_FOUND;
    }

    /* Get the base index for this ECN map */
    index = (ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[table_num] *
            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP) + 
            (ecn_map->ecn << 2) + ecn_map->inner_ecn;  

    /* Commit to HW */
    BCM_IF_ERROR_RETURN(READ_ING_TUNNEL_ECN_DECAPm(unit, MEM_BLOCK_ANY,
                                                   index, 
                                                   &ing_tunnel_ecn_decap_map));
    ecn_map->new_ecn = soc_mem_field32_get(unit, ING_TUNNEL_ECN_DECAPm, 
                                           &ing_tunnel_ecn_decap_map, INNER_ECNf);
    if (soc_mem_field32_get(unit, ING_TUNNEL_ECN_DECAPm,
                            &ing_tunnel_ecn_decap_map, DROPf)) {
        ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_INGRESS_DROP;
    }
    if (soc_mem_field32_get(unit, ING_TUNNEL_ECN_DECAPm,
                            &ing_tunnel_ecn_decap_map, CHANGE_INNER_ECNf)) {
        ecn_map->action_flags |= BCM_ECN_TRAFFIC_ACTION_INGRESS_ECN_MARKING;
    }
    return BCM_E_NONE;
}


int
bcmi_xgs5_ecn_map_create(int unit, uint32 flags, int *ecn_map_id)
{
    int rv = BCM_E_PARAM;
    /* Input parameters check. */
    ECN_INIT(unit);
    if (NULL == ecn_map_id) {
        return rv;
    } 
    ECN_LOCK(unit);   
    if (flags & BCM_ECN_MAP_INGRESS) {
        if (flags & BCM_ECN_MAP_TUNNEL_TERM) {
            rv = bcmi_xgs5_tunnel_term_ecn_map_create(unit, flags, ecn_map_id);
        }
    }
    ECN_UNLOCK(unit);
    return rv;
}

int
bcmi_xgs5_ecn_map_destroy(int unit, int ecn_map_id)
{
    int rv = BCM_E_PARAM; 
    ECN_INIT(unit); 
    if (ecn_map_id < 0) {
        return rv;
    }
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM){
        rv = bcmi_xgs5_tunnel_term_ecn_map_destroy(unit, ecn_map_id);
    }
    ECN_UNLOCK(unit);
    return rv;
}

int
bcmi_xgs5_ecn_map_set(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map) 
{
    int rv = BCM_E_PARAM; 
    ECN_INIT(unit); 
    if (ecn_map_id < 0) {
        return rv;
    }
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
        rv = bcmi_xgs5_tunnel_term_ecn_map_set(unit, ecn_map_id, ecn_map);
    }
    ECN_UNLOCK(unit);
    return rv;

}

int
bcmi_xgs5_ecn_map_get(int unit, int ecn_map_id, bcm_ecn_map_t *ecn_map) 
{
    int rv = BCM_E_PARAM;
    ECN_INIT(unit);
    ECN_LOCK(unit);
    if ((ecn_map_id & _BCM_XGS5_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
        rv = bcmi_xgs5_tunnel_term_ecn_map_get(unit, ecn_map_id, ecn_map);
    }
    ECN_UNLOCK(unit);
    return rv;    
}

STATIC int
bcmi_xgs5_tunnel_term_ecn_vp_map_set(int unit, int vp, int ecn_map_id)
{
    int rv = BCM_E_UNAVAIL;
    source_vp_entry_t svp_entry;

    if (SOC_MEM_FIELD_VALID(unit, SOURCE_VPm, ECN_DECAP_MAPPING_PTRf)) {
        SOC_IF_ERROR_RETURN
            (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
        soc_SOURCE_VPm_field32_set(unit, &svp_entry, ECN_DECAP_MAPPING_PTRf, 
                                   ecn_map_id);
        SOC_IF_ERROR_RETURN
            (WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry));
        rv = BCM_E_NONE;
    }
    return rv;
}

STATIC int
bcmi_xgs5_tunnel_term_ecn_vp_map_get(int unit, int vp, int *ecn_map_id)  
{
    int rv = BCM_E_UNAVAIL;
    source_vp_entry_t svp_entry;
    if (SOC_MEM_FIELD_VALID(unit, SOURCE_VPm, ECN_DECAP_MAPPING_PTRf)) {
        SOC_IF_ERROR_RETURN
            (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
        *ecn_map_id = soc_SOURCE_VPm_field32_get(unit, &svp_entry, 
                                                 ECN_DECAP_MAPPING_PTRf);
        rv = BCM_E_NONE;
    }
    return rv;
}


int
bcmi_xgs5_ecn_port_map_set(int unit, bcm_gport_t port, bcm_ecn_port_map_t *ecn_map)
{
    int rv = BCM_E_PARAM;
    int vp = -1;
    ECN_INIT(unit);
    /* Get the VP index from the gport */
    if (BCM_GPORT_IS_L2GRE_PORT(port)) {
        vp = BCM_GPORT_L2GRE_PORT_ID_GET(port);
    } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
    } else if (BCM_GPORT_IS_VXLAN_PORT(port)) {
        vp = BCM_GPORT_VXLAN_PORT_ID_GET(port);
    } else { 
        return BCM_E_PORT;
    }
    if (ecn_map->ecn_map_id < 0) {
        return rv;
    }
    ECN_LOCK(unit);
    if (ecn_map->flags & BCM_ECN_INGRESS_PORT_TUNNEL_TERM_MAP) {
        int ecn_map_index;
        int ecn_map_type;
        int ecn_map_num;
        ecn_map_type = ecn_map->ecn_map_id & 
                        _BCM_XGS5_ECN_MAP_TYPE_MASK;
        ecn_map_index = ecn_map->ecn_map_id & 
                        _BCM_XGS5_ECN_MAP_NUM_MASK;
        ecn_map_num = 
            soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / 
            _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
        if (ecn_map_type != _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
            goto clean_up;
        }
        if (ecn_map_index >= ecn_map_num) {
            goto clean_up;
        }          
        if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, ecn_map_index)) {
            goto clean_up;
        }
        rv =  bcmi_xgs5_tunnel_term_ecn_vp_map_set(unit, vp, ecn_map_index);
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
    }
clean_up:    
    ECN_UNLOCK(unit);
    return rv;    
}



int
bcmi_xgs5_ecn_port_map_get(int unit, bcm_gport_t port, bcm_ecn_port_map_t *ecn_map)
{
    int rv = BCM_E_PARAM;
    int vp = -1;
    int ecn_map_index;
    ECN_INIT(unit);
    /* Get the VP index from the gport */
    if (BCM_GPORT_IS_L2GRE_PORT(port)) {
        vp = BCM_GPORT_L2GRE_PORT_ID_GET(port);
    } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
    } else if (BCM_GPORT_IS_VXLAN_PORT(port)) {
        vp = BCM_GPORT_VXLAN_PORT_ID_GET(port);
    } else { 
        return BCM_E_PORT;
    }
    ECN_LOCK(unit);
    if (ecn_map->flags & BCM_ECN_INGRESS_PORT_TUNNEL_TERM_MAP) {
        rv =  bcmi_xgs5_tunnel_term_ecn_vp_map_get(unit, vp, &ecn_map_index);
        if (BCM_FAILURE(rv)) {
            goto clean_up;
        }
        if (_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, ecn_map_index)) {
            ecn_map->ecn_map_id = 
                ecn_map_index | _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM;
            ecn_map->flags |= BCM_ECN_INGRESS_PORT_TUNNEL_TERM_MAP;            
        } else {
            return BCM_E_PARAM;
        }
    }
clean_up:    
    ECN_UNLOCK(unit);
    return rv;    
}

STATIC
void bcmi_xgs5_ecn_free_resource(int unit)
{
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (ecn_info->ing_exp_to_ip_ecn_map_bitmap) {
        sal_free(ecn_info->ing_exp_to_ip_ecn_map_bitmap);
        ecn_info->ing_exp_to_ip_ecn_map_bitmap = NULL;
    }
    if (ecn_info->ing_exp_to_ip_ecn_map_hw_idx) {
        sal_free(ecn_info->ing_exp_to_ip_ecn_map_hw_idx);
        ecn_info->ing_exp_to_ip_ecn_map_hw_idx = NULL;
    }
    
    if (ecn_info->egr_ip_ecn_to_exp_map_bitmap) {
        sal_free(ecn_info->egr_ip_ecn_to_exp_map_bitmap);
        ecn_info->egr_ip_ecn_to_exp_map_bitmap = NULL;
    }
    if (ecn_info->egr_ip_ecn_to_exp_map_hw_idx) {
        sal_free(ecn_info->egr_ip_ecn_to_exp_map_hw_idx);
        ecn_info->egr_ip_ecn_to_exp_map_hw_idx = NULL;
    }

    if (ecn_info->egr_int_cn_to_exp_map_bitmap) {
        sal_free(ecn_info->egr_int_cn_to_exp_map_bitmap);
        ecn_info->egr_int_cn_to_exp_map_bitmap = NULL;
    } 
    if (ecn_info->egr_int_cn_to_exp_map_hw_idx) {
        sal_free(ecn_info->egr_int_cn_to_exp_map_hw_idx);
        ecn_info->egr_int_cn_to_exp_map_hw_idx = NULL;
    }
#endif /* BCM_MPLS_SUPPORT */
#endif /* BCM_TOMAHAWK2_SUPPORT */    
    if (ecn_info->ing_tunnel_term_ecn_map_bitmap) {
        sal_free(ecn_info->ing_tunnel_term_ecn_map_bitmap);
        ecn_info->ing_tunnel_term_ecn_map_bitmap = NULL;
    }
    if (ecn_info->ing_tunnel_term_ecn_map_hw_idx) {
        sal_free(ecn_info->ing_tunnel_term_ecn_map_hw_idx);
        ecn_info->ing_tunnel_term_ecn_map_hw_idx = NULL;
    }
    
    /* Destroy the mutex */
    if (ecn_info->ecn_mutex) {
        sal_mutex_destroy(ecn_info->ecn_mutex);
        ecn_info->ecn_mutex = NULL;
    }
    ecn_info->initialized = 0;
    return;
}


int 
bcmi_xgs5_ecn_init(int unit)
{
    int rv = BCM_E_NONE;
    int num_ing_tunnel_term_ecn_map = 0;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    if (ecn_info->initialized) {
        bcmi_xgs5_ecn_free_resource(unit);
    }
    num_ing_tunnel_term_ecn_map =  _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP;
    ecn_info->ing_tunnel_term_ecn_map_bitmap =
        sal_alloc(SHR_BITALLOCSIZE(num_ing_tunnel_term_ecn_map), 
                  "ing_tunnel_term_ecn_map_bitmap");
    if (ecn_info->ing_tunnel_term_ecn_map_bitmap == NULL) {
        bcmi_xgs5_ecn_free_resource(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(ecn_info->ing_tunnel_term_ecn_map_bitmap, 0, 
               SHR_BITALLOCSIZE(num_ing_tunnel_term_ecn_map));
    ecn_info->ing_tunnel_term_ecn_map_hw_idx = 
        sal_alloc(sizeof(uint32) * num_ing_tunnel_term_ecn_map, 
                  "ing_tunnel_term_ecn_map_hw_idx");
    if (ecn_info->ing_tunnel_term_ecn_map_hw_idx == NULL) {
        bcmi_xgs5_ecn_free_resource(unit);
        return BCM_E_MEMORY;
    }     
    sal_memset(ecn_info->ing_tunnel_term_ecn_map_hw_idx, 0, 
               num_ing_tunnel_term_ecn_map * sizeof(uint32));
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT) 
    if (soc_feature(unit, soc_feature_mpls_ecn)) {
        int num_ing_ecn_map = 0;
        int num_egr_ecn_map = 0;
        int num_egr_int_cn_map = 0;
        num_ing_ecn_map =  _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP;
        num_egr_ecn_map =  _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP;
        num_egr_int_cn_map = _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP;
        ecn_info->ing_exp_to_ip_ecn_map_bitmap =
            sal_alloc(SHR_BITALLOCSIZE(num_ing_ecn_map), "ing_ecn_map_bitmap");
        if (ecn_info->ing_exp_to_ip_ecn_map_bitmap == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }
        
        ecn_info->ing_exp_to_ip_ecn_map_hw_idx = 
            sal_alloc(sizeof(uint32) * num_ing_ecn_map, 
                      "ing_exp_to_ip_ecn_map_hw_idx");
        if (ecn_info->ing_exp_to_ip_ecn_map_hw_idx == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }       

        ecn_info->egr_ip_ecn_to_exp_map_bitmap =
            sal_alloc(SHR_BITALLOCSIZE(num_egr_ecn_map), "egr_ecn_map_bitmap");
        if (ecn_info->egr_ip_ecn_to_exp_map_bitmap == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }
        
        ecn_info->egr_ip_ecn_to_exp_map_hw_idx = 
            sal_alloc(sizeof(uint32) * num_egr_ecn_map, 
                      "egr_ip_ecn_to_exp_map_hw_idx");
        if (ecn_info->egr_ip_ecn_to_exp_map_hw_idx == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }

        ecn_info->egr_int_cn_to_exp_map_bitmap =
            sal_alloc(SHR_BITALLOCSIZE(num_egr_int_cn_map), "egr_int_cn_map_bitmap");
        if (ecn_info->egr_int_cn_to_exp_map_bitmap == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }

        ecn_info->egr_int_cn_to_exp_map_hw_idx = 
            sal_alloc(sizeof(uint32) * num_egr_int_cn_map, 
                      "egr_int_cn_to_exp_map_hw_idx");
        if (ecn_info->egr_int_cn_to_exp_map_hw_idx == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }        
        sal_memset(ecn_info->ing_exp_to_ip_ecn_map_bitmap, 0, 
                   SHR_BITALLOCSIZE(num_ing_ecn_map));
        sal_memset(ecn_info->ing_exp_to_ip_ecn_map_hw_idx, 0, 
                   num_ing_ecn_map * sizeof(uint32));         
        sal_memset(ecn_info->egr_ip_ecn_to_exp_map_bitmap, 0, 
                   SHR_BITALLOCSIZE(num_egr_ecn_map));
        sal_memset(ecn_info->egr_ip_ecn_to_exp_map_hw_idx, 0, 
                   num_egr_ecn_map * sizeof(uint32));         
        sal_memset(ecn_info->egr_int_cn_to_exp_map_bitmap, 0, 
                   SHR_BITALLOCSIZE(num_egr_int_cn_map));
        sal_memset(ecn_info->egr_int_cn_to_exp_map_hw_idx, 0, 
                   num_egr_int_cn_map * sizeof(uint32));                
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */
    /* Create mutex */
    if (NULL == ecn_info->ecn_mutex) {
        ecn_info->ecn_mutex = sal_mutex_create("ecn mutex");
        if (ecn_info->ecn_mutex == NULL) {
            bcmi_xgs5_ecn_free_resource(unit);
            return BCM_E_MEMORY;
        }
    }
    ecn_info->initialized = 1;
    return rv;
}

/* 
 * Function:
        bcmi_xgs5_ecn_scache_size_get
 * Purpose:
 *      Helper utility to determine scache details.
 * Parameters:
 *      unit        : (IN) Device Unit Number
 *      scache_size  : (OUT) Total required scache length
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_ecn_scache_size_get(int unit, uint32 *scache_size)
{
    if (scache_size == NULL) {
        return BCM_E_PARAM;
    }
    *scache_size = 0;
    *scache_size += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP);
    *scache_size += _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP * sizeof(uint32);
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT) 
    if (soc_feature(unit, soc_feature_mpls_ecn)) {    
        *scache_size += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP);
        *scache_size += _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP * sizeof(uint32);
        *scache_size += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP);
        *scache_size += _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP * sizeof(uint32);
        *scache_size += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP);
        *scache_size += _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP * sizeof(uint32);
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_ecn_map_used_get
 * Purpose:
 *      Check whether a ecn map id is used or not
 * Parameters:
 *      unit    -  (IN) Device number.
 *      vp      -  (IN) map_id
 * Returns:
 *      Boolean
 */
int
bcmi_xgs5_ecn_map_used_get(int unit, int map_index, _bcm_ecn_map_type_e type)
{
    int rv = BCM_E_NONE;
    switch (type) {
#ifdef BCM_MPLS_SUPPORT        
#ifdef BCM_TOMAHAWK2_SUPPORT        
        case _bcmEcnmapTypeExp2Ecn:
            rv = _BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, map_index);
            break;
        case _bcmEcnmapTypeIpEcn2Exp:
            rv = _BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, map_index);
            break;
        case _bcmEcnmapTypeIntcn2Exp:
            rv = _BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, map_index);
            break;
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */
        case _bcmEcnmapTypeTunnelTerm:
            rv = _BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, map_index);
            break;
        default:
            break;
    }
    return rv;
}

int
bcmi_ecn_map_id2hw_idx (int unit,int ecn_map_id,int *hw_idx)
{
    int rv = BCM_E_PARAM;
    int table_num = -1;
    ECN_INIT(unit);
    ECN_LOCK(unit);
#ifdef BCM_TOMAHAWK2_SUPPORT
    if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
        _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN){
        table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
        if (!_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, table_num)) {
            ECN_UNLOCK(unit);
            return BCM_E_PARAM;
        }
        *hw_idx = ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[table_num];
        rv = BCM_E_NONE;
    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
        table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
        if (!_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, table_num)) {
            ECN_UNLOCK(unit);
            return BCM_E_PARAM;
        }
        *hw_idx = ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[table_num];
        rv = BCM_E_NONE;

    } else if ((ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
        table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
        if (!_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, table_num)) {
            ECN_UNLOCK(unit);
            return BCM_E_PARAM;
        }
        *hw_idx = ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[table_num];
        rv = BCM_E_NONE;
    } else 
#endif
    if ((ecn_map_id & _BCM_XGS5_ECN_MAP_TYPE_MASK) == 
               _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
        table_num = ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
        if (!_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, table_num)) {
            ECN_UNLOCK(unit);
            return BCM_E_PARAM;
        }
        *hw_idx = ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[table_num];
        rv = BCM_E_NONE;
    }
    ECN_UNLOCK(unit);
    return rv;
}


int
bcmi_ecn_map_hw_idx2id (int unit, int hw_idx, int type, int *ecn_map_id)
{
    int id, num_map;
    int rv = BCM_E_NOT_FOUND;
    ECN_INIT(unit);
    ECN_LOCK(unit);
    switch (type) {
    case _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM:
        num_map = _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP;
        for (id = 0; id < num_map; id++) {
            if (_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, id)) {
                if (ECN_INFO(unit)->ing_tunnel_term_ecn_map_hw_idx[id] == hw_idx) {
                    *ecn_map_id = type | id;
                    rv = BCM_E_NONE;
                    break;
                }
            }
        }
        break;     
#ifdef BCM_TOMAHAWK2_SUPPORT   
    case _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN:
        num_map = _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP;
        for (id = 0; id < num_map; id++) {
            if (_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, id)) {
                if (ECN_INFO(unit)->ing_exp_to_ip_ecn_map_hw_idx[id] == hw_idx) {
                    *ecn_map_id = type | id;
                    rv = BCM_E_NONE;
                    break;
                }
            }
        }
        break;
    case _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP:
        num_map = _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP;
        for (id = 0; id < num_map; id++) {
            if (_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, id)) {
                if (ECN_INFO(unit)->egr_ip_ecn_to_exp_map_hw_idx[id] == hw_idx) {
                    *ecn_map_id = type | id;
                    rv = BCM_E_NONE;
                    break;
                }
            }
        }
        break;
    case _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP:
        num_map = _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP;
        for (id = 0; id < num_map; id++) {
            if (_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, id)) {
                if (ECN_INFO(unit)->egr_int_cn_to_exp_map_hw_idx[id] == hw_idx) {
                    *ecn_map_id = type | id;
                    rv = BCM_E_NONE;
                    break;
                }
            }
        }
        break;        
#endif
    default:
        rv = BCM_E_NOT_FOUND;
        break;
    }
    ECN_UNLOCK(unit);
    return rv;
}

void
bcmi_ecn_map_clean_up(int unit)
{
    bcmi_xgs5_ecn_free_resource(unit);
    return;
}


#ifdef BCM_WARM_BOOT_SUPPORT
int 
bcmi_xgs5_ecn_sync(int unit, uint8 **scache_ptr) 
{
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    SHR_BITCOPY_RANGE((SHR_BITDCL *)(*scache_ptr), 0,
                      ecn_info->ing_tunnel_term_ecn_map_bitmap, 0, 
                      _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP);
    *scache_ptr += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP);
    sal_memcpy(*scache_ptr, ecn_info->ing_tunnel_term_ecn_map_hw_idx,
               (_BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP * sizeof(uint32)));
    *scache_ptr += (_BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP * sizeof(uint32));
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_ecn)) { 
        /* ing_exp_to_ip_ecn_map */
        SHR_BITCOPY_RANGE((SHR_BITDCL *)(*scache_ptr), 0,
                          ecn_info->ing_exp_to_ip_ecn_map_bitmap, 0, 
                          _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP);
        *scache_ptr += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP);
        sal_memcpy(*scache_ptr, ecn_info->ing_exp_to_ip_ecn_map_hw_idx,
               (_BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP * sizeof(uint32)));
        *scache_ptr += (_BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP * sizeof(uint32));
        /* egr_ip_ecn_to_exp_map */
        SHR_BITCOPY_RANGE((SHR_BITDCL *)(*scache_ptr), 0,
                          ecn_info->egr_ip_ecn_to_exp_map_bitmap, 0, 
                          _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP);
        *scache_ptr += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP);
        sal_memcpy(*scache_ptr, ecn_info->egr_ip_ecn_to_exp_map_hw_idx,
               (_BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP * sizeof(uint32)));
        *scache_ptr += (_BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP * sizeof(uint32));        
        /* egr_int_cn_to_exp_map */
        SHR_BITCOPY_RANGE((SHR_BITDCL *)(*scache_ptr), 0,
                          ecn_info->egr_int_cn_to_exp_map_bitmap, 0, 
                          _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP);
        *scache_ptr += SHR_BITALLOCSIZE(_BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP);
        sal_memcpy(*scache_ptr, ecn_info->egr_int_cn_to_exp_map_hw_idx,
               (_BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP * sizeof(uint32)));
        *scache_ptr += (_BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP * sizeof(uint32));        
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */    
    return BCM_E_NONE;
}
int
bcmi_xgs5_ecn_reinit(int unit, uint8 **scache_ptr) 
{
    int                 num_ing_tunnel_term_ecn_map = 0;
    int                 i = 0;
    int                 rv;
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);
    num_ing_tunnel_term_ecn_map = _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP;
    SHR_BITCOPY_RANGE(ecn_info->ing_tunnel_term_ecn_map_bitmap, 0,
                      (SHR_BITDCL *)(*scache_ptr), 0, num_ing_tunnel_term_ecn_map);
    *scache_ptr += SHR_BITALLOCSIZE(num_ing_tunnel_term_ecn_map);
    sal_memcpy(ecn_info->ing_tunnel_term_ecn_map_hw_idx, *scache_ptr,
               (num_ing_tunnel_term_ecn_map * sizeof(uint32)));
    
    *scache_ptr += (num_ing_tunnel_term_ecn_map * sizeof(uint32));
    
    for (i = 0; i < num_ing_tunnel_term_ecn_map; i++) {
        if (_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, i)) {
            rv = _bcm_ing_tunnel_term_map_entry_reference(unit, 
                    ((ecn_info->ing_tunnel_term_ecn_map_hw_idx[i]) * 16), 16);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
    }
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_ecn)) {  
        int                 num_ing_ecn_map = 0;
        int                 num_egr_ecn_map = 0;
        int                 num_egr_int_cn_map = 0;
        num_ing_ecn_map =  _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP;
        num_egr_ecn_map =  _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP;
        num_egr_int_cn_map = _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP;
        /* ing_exp_to_ip_ecn_map */
        SHR_BITCOPY_RANGE(ecn_info->ing_exp_to_ip_ecn_map_bitmap, 0,
                          (SHR_BITDCL *)(*scache_ptr), 0, num_ing_ecn_map);
        *scache_ptr += SHR_BITALLOCSIZE(num_ing_ecn_map);
        sal_memcpy(ecn_info->ing_exp_to_ip_ecn_map_hw_idx, *scache_ptr,
                   (num_ing_ecn_map * sizeof(uint32)));
        
        *scache_ptr += (num_ing_ecn_map * sizeof(uint32));
        
        for (i = 0; i < num_ing_ecn_map; i++) {
            if (_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, i)) {
                rv = _bcm_ing_exp_to_ip_ecn_map_entry_reference(unit, 
                        ((ecn_info->ing_exp_to_ip_ecn_map_hw_idx[i]) * 32), 32);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }
            }
        } 
        /* egr_ip_ecn_to_exp_map */
        SHR_BITCOPY_RANGE(ecn_info->egr_ip_ecn_to_exp_map_bitmap, 0,
                          (SHR_BITDCL *)(*scache_ptr), 0, num_egr_ecn_map);
        *scache_ptr += SHR_BITALLOCSIZE(num_egr_ecn_map);
        
        sal_memcpy(ecn_info->egr_ip_ecn_to_exp_map_hw_idx, *scache_ptr,
                   (num_egr_ecn_map * sizeof(uint32)));
        
        *scache_ptr += (num_egr_ecn_map * sizeof(uint32));
        
        for (i = 0; i < num_egr_ecn_map; i++) {
            if (_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, i)) {
                rv = _bcm_egr_ip_ecn_to_exp_map_entry_reference(unit, 
                        ((ecn_info->egr_ip_ecn_to_exp_map_hw_idx[i]) * 32), 32);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }
            }
        }        
        /* egr_int_cn_to_exp_map */
        SHR_BITCOPY_RANGE(ecn_info->egr_int_cn_to_exp_map_bitmap, 0,
                          (SHR_BITDCL *)(*scache_ptr), 0, num_egr_int_cn_map);
        *scache_ptr += SHR_BITALLOCSIZE(num_egr_int_cn_map);
        sal_memcpy(ecn_info->egr_int_cn_to_exp_map_hw_idx, *scache_ptr,
                   (num_egr_int_cn_map * sizeof(uint32)));
        
        *scache_ptr += (num_egr_int_cn_map * sizeof(uint32));
        
        for (i = 0; i < num_egr_int_cn_map; i++) {
            if (_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, i)) {
                rv = _bcm_egr_int_cn_to_exp_map_entry_reference(unit, 
                        ((ecn_info->egr_int_cn_to_exp_map_hw_idx[i]) * 32), 32);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }
            }
        }
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */    
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
bcmi_xgs5_ecn_sw_dump(int unit)
{
    int     i;  
    _bcm_xgs5_ecn_bookkeeping_t *ecn_info = ECN_INFO(unit);   
    if (!ecn_info->initialized) {
        LOG_CLI((BSL_META_U(unit,
                            "ERROR: ECN module not initialized\n")));
        return;
    }  
    LOG_CLI((BSL_META_U(unit, "ECN: ING_TUNNEL_TERM_ECN_MAP info \n")));
    for (i = 0; i < _BCM_ECN_MAP_LEN_ING_TUNNEL_TERM_ECN_MAP; i++) {
        if (_BCM_ING_TUNNEL_TERM_ECN_MAP_USED_GET(unit, i)) {
            LOG_CLI((BSL_META_U(unit, "    map id:%4d HW index:%4d\n"), i, ecn_info->ing_tunnel_term_ecn_map_hw_idx[i]));
        }
    }
#ifdef BCM_MPLS_SUPPORT
#if defined(BCM_TOMAHAWK2_SUPPORT) 
    if (soc_feature(unit, soc_feature_mpls_ecn)) {    
        LOG_CLI((BSL_META_U(unit, "ECN: ING_EXP_TO_IP_ECN_MAP info \n")));
        for (i = 0; i < _BCM_ECN_MAP_LEN_ING_EXP_TO_IP_ECN_MAP; i++) {
            if (_BCM_MPLS_ING_EXP_TO_IP_ECN_MAP_USED_GET(unit, i)) {
                LOG_CLI((BSL_META_U(unit, "    map id:%4d HW index:%4d\n"), i, ecn_info->ing_exp_to_ip_ecn_map_hw_idx[i]));
            }
        }
        
        LOG_CLI((BSL_META_U(unit, "ECN: EGR_IP_ECN_TO_EXP_MAP info \n")));
        for (i = 0; i < _BCM_ECN_MAP_LEN_EGR_IP_ECN_TO_EXP_MAP; i++) {
            if (_BCM_MPLS_EGR_IP_ECN_TO_EXP_MAP_USED_GET(unit, i)) {
                LOG_CLI((BSL_META_U(unit, "    map id:%4d HW index:%4d\n"), i, ecn_info->egr_ip_ecn_to_exp_map_hw_idx[i]));
            }
        }
        
        LOG_CLI((BSL_META_U(unit, "ECN: EGR_INT_CN_TO_EXP_MAP info \n")));
        for (i = 0; i < _BCM_ECN_MAP_LEN_EGR_INT_CN_TO_EXP_MAP; i++) {
            if (_BCM_MPLS_EGR_INT_CN_TO_EXP_MAP_USED_GET(unit, i)) {
                LOG_CLI((BSL_META_U(unit, "    map id:%4d HW index:%4d\n"), i, ecn_info->egr_int_cn_to_exp_map_hw_idx[i]));
            }
        }
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
#endif /* BCM_MPLS_SUPPORT */
    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
#endif /* BCM_TOMAHAWK_SUPPORT */
#endif /* INCLUDE_L3 */

