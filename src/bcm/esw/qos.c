/*
 * $Id: qos.c,v 1.32 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * QoS module
 */
#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2u.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/ipmc.h>
#include <bcm/qos.h>
#include <bcm/stack.h>
#include <bcm/topo.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/qos.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/xgs3.h>
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
#include <bcm_int/esw/triumph.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIDENT2_SUPPORT)
#include <bcm_int/esw/trident2.h>
#endif /* BCM_TRIDENT2_SUPPORT */
#if defined(BCM_GREYHOUND_SUPPORT)
#include <bcm_int/esw/greyhound.h>
#endif /* BCM_GREYHOUND_SUPPORT */
#if defined(BCM_TOMAHAWK_SUPPORT)
#include <bcm_int/esw/tomahawk.h>
#endif /* BCM_TOMAHAWK_SUPPORT */

/* Initialize the QoS module. */
int 
bcm_esw_qos_init(int unit)
{
    int rv = BCM_E_NONE;

#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TRIDENT2X(unit)) {
        rv = bcm_td2_qos_init(unit);
    }
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        rv = bcm_th_qos_init(unit);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {

        rv = bcm_tr2_qos_init(unit);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_init(unit);
    }
#endif
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANAX(unit) ||
        SOC_IS_TRIUMPH3(unit)) {
        egr_map_mh_entry_t *mapmh = NULL;
        soc_mem_t mem;
        uint32 min, max, idx;
        int alloc_sz;
        uint8 *mbuf;

        mem = EGR_MAP_MHm;
        min = soc_mem_index_min(unit, mem);
        max = soc_mem_index_max(unit, mem);
        alloc_sz = sizeof(egr_map_mh_entry_t) * soc_mem_index_count(unit, mem);

        /* Program the table */
        mbuf = soc_cm_salloc(unit, alloc_sz, SOC_MEM_NAME(unit, mem));
        if (NULL == mbuf) {
            rv = BCM_E_MEMORY;
        }
        soc_mem_lock(unit, mem);
        if (BCM_SUCCESS(rv)) {
            sal_memset(mbuf, 0, alloc_sz);
        }
        if (BCM_SUCCESS(rv)) {
            for (idx = min; idx <= max; idx++) {
                mapmh = soc_mem_table_idx_to_pointer(unit, mem, 
                                         egr_map_mh_entry_t *, mbuf, idx);
                soc_mem_field32_set(unit, mem, mapmh, HG_TCf, idx & 0xf);
            }
        }
        if (BCM_SUCCESS(rv)) {
            rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ANY, min, max, mbuf);
        }
        soc_mem_unlock(unit, mem);
        if (mbuf != NULL) {
            soc_cm_sfree(unit, mbuf);
        }
    }
#endif
#if defined(BCM_GREYHOUND_SUPPORT)
    if (SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_gh_qos_init(unit);
    }
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
#if defined BCM_KATANA2_SUPPORT || defined BCM_TRIDENT2_SUPPORT
    if ((SOC_IS_KATANA2(unit)) || SOC_IS_TRIDENT2X(unit)) {
        rv = _bcm_notify_qos_reinit_to_fp (unit);
    }
#endif /* BCM_KATANA2_SUPPORT, BCM_TRIDENT2_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */


    return rv;
}

/* Detach the QoS module. */
int 
bcm_esw_qos_detach(int unit)
{
    int rv = BCM_E_NONE;
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_detach(unit);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_detach(unit);
    }
#endif
#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TRIDENT2(unit)) {
        rv = bcm_td2_qos_detach(unit);
    }
#endif
#if defined(BCM_GREYHOUND_SUPPORT)
    if (SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_gh_qos_detach(unit);
    }
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        rv = bcm_th_qos_detach(unit);
    }
#endif


    return rv;
}

/* Create a QoS map profile */
int 
bcm_esw_qos_map_create(int unit, uint32 flags, int *map_id)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_HURRICANE_SUPPORT)
    if ((SOC_IS_HURRICANEX(unit)) && (flags & BCM_QOS_MAP_MPLS)) {
        return rv;
    }
#endif

#if defined(BCM_GREYHOUND_SUPPORT)
    if ((SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) && 
        (flags & BCM_QOS_MAP_MPLS)) {
        return rv;
    }
#endif


#ifdef BCM_TRIDENT2_SUPPORT
    if ((QOS_FLAGS_ARE_FCOE(flags) || (flags & BCM_QOS_MAP_L2_VLAN_ETAG)) 
        && SOC_IS_TRIDENT2X(unit)) {
        return bcm_td2_qos_map_create(unit, flags, map_id);
    }
#endif

#ifdef BCM_GREYHOUND_SUPPORT
    if ((flags & BCM_QOS_MAP_L2_VLAN_ETAG) 
        && (SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
            SOC_IS_GREYHOUND2(unit))) {
        return bcm_gh_qos_map_create(unit, flags, map_id);
    }
#endif

#if defined(BCM_TOMAHAWK_SUPPORT)
    if ((flags & BCM_QOS_MAP_L2_VLAN_ETAG) &&
        SOC_IS_TOMAHAWKX(unit)) {
        return bcm_th_qos_map_create(unit, flags, map_id);
    }
#endif

#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_map_create(unit, flags, map_id);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_map_create(unit, flags, map_id);
    }
#endif
    return rv;
}

/* Destroy a QoS map profile */
int 
bcm_esw_qos_map_destroy(int unit, int map_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)&& (QOS_MAP_IS_FCOE(map_id)
        || QOS_MAP_IS_L2_VLAN_ETAG(map_id))) {
        return bcm_td2_qos_map_destroy(unit, map_id);
    }
#endif
#ifdef BCM_GREYHOUND_SUPPORT
    if ((SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit) ||
        SOC_IS_GREYHOUND2(unit))&& 
        QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_gh_qos_map_destroy(unit, map_id);
    }
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)&& QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_th_qos_map_destroy(unit, map_id);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_map_destroy(unit, map_id);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_map_destroy(unit, map_id);
    }
#endif
    return rv;
}

/* Add an entry to a QoS map */
int 
bcm_esw_qos_map_add(int unit, uint32 flags, bcm_qos_map_t *map, int map_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)&& (QOS_MAP_IS_FCOE(map_id)
        || QOS_MAP_IS_L2_VLAN_ETAG(map_id))) {
        return bcm_td2_qos_map_add(unit, flags, map, map_id);
    }
#endif
#ifdef BCM_GREYHOUND_SUPPORT
    if ((SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit))&& 
        QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_gh_qos_map_add(unit, flags, map, map_id);
    }
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)&& QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_th_qos_map_add(unit, flags, map, map_id);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_map_add(unit, flags, map, map_id);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_map_add(unit, flags, map, map_id);
    }
#endif
    return rv;
}

/* bcm_esw_qos_map_multi_get */
int
bcm_esw_qos_map_multi_get(int unit, uint32 flags,
                          int map_id, int array_size, 
                          bcm_qos_map_t *array, int *array_count)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)&& QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_td2_qos_map_multi_get(unit, flags, map_id, array_size, array, 
                                       array_count);
    }
#endif

#ifdef BCM_GREYHOUND_SUPPORT
    if ((SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit))&& 
        QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_gh_qos_map_multi_get(unit, flags, map_id, array_size, array, 
                                       array_count);
    }
#endif

#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)&& QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_th_qos_map_multi_get(unit, flags, map_id, array_size, array,
                                       array_count);
    }
#endif
    
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_map_multi_get(unit, flags, map_id, array_size, array, 
                                       array_count);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_map_multi_get(unit, flags, map_id, array_size, array, 
                                       array_count);
    }
#endif
    return rv;
}

/* Clear an entry from a QoS map */
int 
bcm_esw_qos_map_delete(int unit, uint32 flags, bcm_qos_map_t *map, int map_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)&& (QOS_MAP_IS_FCOE(map_id)
        || QOS_MAP_IS_L2_VLAN_ETAG(map_id))) {
        return bcm_td2_qos_map_delete(unit, flags, map, map_id);
    }
#endif
#ifdef BCM_GREYHOUND_SUPPORT
    if ((SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit))&& 
        QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_gh_qos_map_delete(unit, flags, map, map_id);
    }
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)&& QOS_MAP_IS_L2_VLAN_ETAG(map_id)) {
        return bcm_th_qos_map_delete(unit, flags, map, map_id);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_map_delete(unit, flags, map, map_id);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_map_delete(unit, flags, map, map_id);
    }
#endif
    return rv;
}

/* Attach a QoS map to an object (Gport) */
int 
bcm_esw_qos_port_map_set(int unit, bcm_gport_t port, int ing_map, int egr_map)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)
        && (QOS_MAP_IS_FCOE(ing_map) || QOS_MAP_IS_L2_VLAN_ETAG(ing_map) 
        || QOS_MAP_IS_FCOE(egr_map))) {
        return bcm_td2_qos_port_map_set(unit, port, ing_map, egr_map);
    }
#endif
#ifdef BCM_GREYHOUND_SUPPORT
    if ((SOC_IS_GREYHOUND(unit)||SOC_IS_HURRICANE3(unit)||
        SOC_IS_GREYHOUND2(unit))
        && (QOS_MAP_IS_L2_VLAN_ETAG(ing_map) || QOS_MAP_IS_L2_VLAN_ETAG(egr_map))) {
        return bcm_gh_qos_port_map_set(unit, port, ing_map, egr_map);
    }
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit) &&
        (QOS_MAP_IS_L2_VLAN_ETAG(ing_map) ||
        QOS_MAP_IS_L2_VLAN_ETAG(egr_map))) {
        return bcm_th_qos_port_map_set(unit, port, ing_map, egr_map);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {

        rv = bcm_tr2_qos_port_map_set(unit, port, ing_map, egr_map);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_port_map_set(unit, port, ing_map, egr_map);
    }
#endif
    return rv;
}


/* bcm_qos_port_map_type_get*/
int
bcm_esw_qos_port_map_type_get(int unit, bcm_gport_t port, uint32 flags,
                          int *map_id)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TRIDENT2X(unit)
        && (QOS_FLAGS_ARE_FCOE(flags) || (flags & BCM_QOS_MAP_L2_VLAN_ETAG))) {
       
        return bcm_td2_qos_port_map_type_get(unit, port, flags, map_id);
    }
#endif

#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit) &&
        (flags & BCM_QOS_MAP_L2_VLAN_ETAG)) {
        return bcm_th_qos_port_map_type_get(unit, port, flags, map_id);
    }
#endif

#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit)) {

        rv = bcm_tr2_qos_port_map_type_get(unit, port, flags, map_id);
    }
#endif


    return rv;
}


/* bcm_qos_port_map_get */
int
bcm_esw_qos_port_map_get(int unit, bcm_gport_t port, 
                         int *ing_map, int *egr_map)
{
    return BCM_E_UNAVAIL;
}

/* bcm_qos_port_vlan_map_set */
int
bcm_esw_qos_port_vlan_map_set(int unit,  bcm_port_t port, bcm_vlan_t vid,
                              int ing_map, int egr_map)
{
    int rv = BCM_E_UNAVAIL;
    
#if defined(BCM_TRIUMPH2_SUPPORT)
    if ( SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit) || SOC_IS_TD_TT(unit) || \
         SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) || \
         SOC_IS_HURRICANE2(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_port_vlan_map_set(unit, port, vid, ing_map, egr_map);
    }
#endif

    return rv;
}

/* bcm_qos_port_vlan_map_get */
int
bcm_esw_qos_port_vlan_map_get(int unit, bcm_port_t port, bcm_vlan_t vid,
                              int *ing_map, int *egr_map)
{
    int rv = BCM_E_UNAVAIL;
    
#if defined(BCM_TRIUMPH2_SUPPORT)
    if ( SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit) || SOC_IS_TD_TT(unit) || \
         SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) || \
         SOC_IS_HURRICANE2(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_port_vlan_map_get(unit, port, vid, ing_map, egr_map);
    }
#endif
    return rv;

}

/* bcm_qos_multi_get */
int
bcm_esw_qos_multi_get(int unit, int array_size, int *map_ids_array, 
                      int *flags_array, int *array_count)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = bcm_tr2_qos_multi_get(unit, array_size, map_ids_array, 
                                   flags_array, array_count);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = bcm_tr_qos_multi_get(unit, array_size, map_ids_array, 
                                   flags_array, array_count);
    }
#endif
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_esw_qos_sync(int unit)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        rv = _bcm_tr2_qos_sync(unit);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        rv = _bcm_tr_qos_sync(unit);
    }
#endif
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
_bcm_esw_qos_sw_dump(int unit)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANEX(unit) ||
        SOC_IS_KATANAX(unit) || SOC_IS_TRIUMPH3(unit)||
        SOC_IS_GREYHOUND(unit) || SOC_IS_GREYHOUND2(unit)) {
        _bcm_tr2_qos_sw_dump(unit);
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        _bcm_tr_qos_sw_dump(unit);
    }
#endif
#if defined (BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TRIDENT2X(unit)) {
        _bcm_td2_qos_sw_dump(unit);
    }
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        _bcm_th_qos_sw_dump(unit);
    }
#endif
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
