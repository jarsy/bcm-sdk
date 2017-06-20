/*
 * $Id: l2cache.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * L2 Cache - Layer 2 BPDU and overflow address cache
 */

#include <shared/bsl.h>

#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/l2.h>
#include <bcm/error.h>
#include <bcm/types.h>

#include <bcm_int/robo_dispatch.h>

/* define l2 cache entries size for robo */
#define L2_CACHE_ENTRIES_NUM            2
#define L2_CACHE_ENTRIES_NUM_VULCAN     6 

#if defined(BCM_HARRIER_SUPPORT)
#define _BCM_HARRIER_USER_ADDR_MODE_BPDU    0
#define _BCM_HARRIER_USER_ADDR_MODE_L2      1

static int
_bcm_robo_harrier_user_addr_mode(int unit, int *mode)
{
    int         ctrl_val = 0;
    uint32      ctrl_cnt = 1, ctrl_type = DRV_DEV_CTRL_L2_USERADDR;    

    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_GET(unit, 
            &ctrl_cnt, &ctrl_type, &ctrl_val));
    *mode = (ctrl_val == _BCM_HARRIER_USER_ADDR_MODE_BPDU) ? 
            _BCM_HARRIER_USER_ADDR_MODE_BPDU : _BCM_HARRIER_USER_ADDR_MODE_L2;
    
    return BCM_E_NONE;
}

static int
_bcm_robo_harrier_free_user_addr_count(int unit, int *count)
{
    bcm_pbmp_t  null_pbmp;
    bcm_mac_t   tmp_addr;
    int         tmp_id = 0, user_addr_free_cnt = 0;
    
    for (tmp_id = 0; tmp_id < 2; tmp_id++){
        BCM_IF_ERROR_RETURN(DRV_MAC_GET(unit, (tmp_id + 1),
                DRV_MAC_CUSTOM_BPDU, &null_pbmp, tmp_addr));
        if (BCM_MAC_IS_ZERO(tmp_addr)) { 
            user_addr_free_cnt ++;
        }
    }
    
    *count = user_addr_free_cnt;
    return BCM_E_NONE;
}
#endif  /* BCM_HARRIER_SUPPORT */

/*
 * Function:
 *      bcm_robo_l2_cache_delete
 * Purpose:
 *      Clear an L2 cache multiport address and vector with index
 * Parameters:
 *      unit - device number
 *      index - multiport address register index
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l2_cache_delete(int unit, int index)
{
    bcm_pbmp_t  mport_vctr;
    bcm_mac_t   mport_addr;
    uint32 mac_type;
#if defined(BCM_HARRIER_SUPPORT)
    int         useraddr_mode = 0;
#endif /* BCM_HARRIER_SUPPORT */

    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_cache_delete()..\n")));
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

#if defined(BCM_HARRIER_SUPPORT)
    if (SOC_IS_HARRIER(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_harrier_user_addr_mode(unit, 
                &useraddr_mode)); 
        if (useraddr_mode == _BCM_HARRIER_USER_ADDR_MODE_BPDU) {
            return BCM_E_NOT_FOUND;
        }
    }
#endif /* BCM_HARRIER_SUPPORT */

    BCM_PBMP_CLEAR(mport_vctr);
    ENET_SET_MACADDR(mport_addr, _soc_mac_all_zeroes);

    switch (index) {
        case 0:
            mac_type = DRV_MAC_MULTIPORT_0;
            break;
        case 1:             
            mac_type = DRV_MAC_MULTIPORT_1;
            break;
        case 2:
            mac_type = DRV_MAC_MULTIPORT_2;
            break;
        case 3:
            mac_type = DRV_MAC_MULTIPORT_3;
            break;
        case 4:               
            mac_type = DRV_MAC_MULTIPORT_4;
            break;
        case 5:
            mac_type = DRV_MAC_MULTIPORT_5;
            break;
        default:
            return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mac_set)
        (unit, mport_vctr, mac_type, mport_addr, 0));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_l2_cache_delete_all
 * Purpose:
 *      Clear all L2 cache multiport address and vector
 * Parameters:
 *      unit - device number
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l2_cache_delete_all(int unit) 
{  
    int index, size;
    int rv;
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_cache_delete_all()..\n")));

    BCM_IF_ERROR_RETURN(bcm_l2_cache_size_get(unit, &size));

    for (index = 0 ; index < size ; index++) {
        rv = bcm_l2_cache_delete(unit, index);
        /* Allow the delete on the invalid MAC address */
        if (rv == BCM_E_NOT_FOUND) {
            rv = BCM_E_NONE;
        } 
        BCM_IF_ERROR_RETURN(rv);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_cache_get
 * Purpose:
 *    get L2 cache multiport address and vector register with index
 *    index = 0 : DRV_MAC_MULTIPORT_0,
 *    index = 1 : DRV_MAC_MULTIPORT_1,
 *    index = 2 : DRV_MAC_MULTIPORT_2,
 *    index = 3 : DRV_MAC_MULTIPORT_3,
 *    index = 4 : DRV_MAC_MULTIPORT_4,
 *    index = 5 : DRV_MAC_MULTIPORT_5,
 * Parameters:
 *    unit        - RoboSwitch unit #.
 *    index      - multiport address register index
 *    addr       - l2 cache address
 * Returns:
 *    BCM_E_XXXX
 */
int 
bcm_robo_l2_cache_get(int unit, int index, bcm_l2_cache_addr_t *addr) 
{
    uint32      mac_type;
    uint32      addr_count = 0;
    bcm_pbmp_t  null_pbmp;
#if defined(BCM_HARRIER_SUPPORT)
    int         useraddr_mode = 0;
#endif  /* BCM_HARRIER_SUPPORT */
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_cache_get()..\n")));
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

#if defined(BCM_HARRIER_SUPPORT)
    if (SOC_IS_HARRIER(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_harrier_user_addr_mode(unit, 
                &useraddr_mode)); 
    }
#endif /* BCM_HARRIER_SUPPORT */

    if (addr->flags & BCM_L2_CACHE_BPDU) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_BPDU_NUM, &addr_count));
        if (index >= addr_count || index < 0){
            return BCM_E_BADID;
        }

        BCM_PBMP_CLEAR(null_pbmp);
        BCM_IF_ERROR_RETURN(DRV_MAC_GET(unit, index,
                DRV_MAC_CUSTOM_BPDU, &null_pbmp, addr->mac));
        if (!BCM_MAC_IS_ZERO(addr->mac)) {
#if defined(BCM_HARRIER_SUPPORT)
            if (SOC_IS_HARRIER(unit)) {
                if ((useraddr_mode == _BCM_HARRIER_USER_ADDR_MODE_L2) && 
                        (index > 0)) {
                    sal_memset(addr->mac, 0, sizeof(bcm_mac_t));
                    return BCM_E_NOT_FOUND;
                }
            }
#endif /* BCM_HARRIER_SUPPORT */
            return BCM_E_NONE;
        } else {
            return BCM_E_NOT_FOUND;
        }
    } else {
        switch (index) {
            case 0:
                mac_type = DRV_MAC_MULTIPORT_0;
                break;
            case 1:             
                mac_type = DRV_MAC_MULTIPORT_1;
                break;
            case 2:
                mac_type = DRV_MAC_MULTIPORT_2;
                break;
            case 3:
                mac_type = DRV_MAC_MULTIPORT_3;
                break;
            case 4:               
                mac_type = DRV_MAC_MULTIPORT_4;
                break;
            case 5:
                mac_type = DRV_MAC_MULTIPORT_5;
                break;
            default:
                return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_MAC_GET(unit, 0, 
                mac_type, &addr->dest_ports, addr->mac));
        
        if (!BCM_MAC_IS_ZERO(addr->mac)) {
            addr->flags |= BCM_L2_CACHE_DESTPORTS;
#if defined(BCM_HARRIER_SUPPORT)
            if (SOC_IS_HARRIER(unit)) {
                if (useraddr_mode == _BCM_HARRIER_USER_ADDR_MODE_BPDU) {
                    sal_memset(addr->mac, 0, sizeof(bcm_mac_t));
                    return BCM_E_NOT_FOUND;
                }
            }
#endif /* BCM_HARRIER_SUPPORT */
            return BCM_E_NONE;
        } else {
            /* MAC_Addr = 0 means invalid for ROBO */
            return BCM_E_NOT_FOUND;
        }
    }
    
}

/*
 * Function:
 *      bcm_robo_l2_cache_init
 * Purpose:
 *      Initialize the L2 cache
 * Parameters:
 *      unit - device number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Clears all entries and preloads a few BCM_L2_CACHE_BPDU
 *      entries to match previous generation of devices.
 */
int 
bcm_robo_l2_cache_init(int unit) 
{  
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    BCM_IF_ERROR_RETURN(bcm_l2_cache_delete_all(unit));
    
    return BCM_E_NONE;
}

/* user address search :
 *
 *  - for found case, return BCM_E_NONE and return id at this found index.
 *  - for not found case,
 *      a. Full condition, return BCM_E_FULL and id = -1.
 *      b. Not Full condition, return BCM_E_NOT_FOUND and return the first
 *          free index to id.
 */
int 
_bcm_robo_l2_cache_search(int unit, bcm_l2_cache_addr_t *addr, int *id)
{
    int i, rv, tmp_id = -1;
    bcm_l2_cache_addr_t temp_l2u_addr;
    int         addr_count = 0;
    bcm_pbmp_t  null_pbmp;
    bcm_mac_t   tmp_addr;

    if (addr == NULL) {
        *id = -1;
        return BCM_E_INTERNAL;
    }
    
    rv = BCM_E_NOT_FOUND;
    BCM_PBMP_CLEAR(null_pbmp);
    
    if (addr->flags & BCM_L2_CACHE_BPDU){
    /* process for BPDU addr */
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_BPDU_NUM, 
                (uint32 *) &addr_count));
        
        for (i = 0; i < addr_count; i++){
            BCM_IF_ERROR_RETURN(DRV_MAC_GET(unit, i,
                    DRV_MAC_CUSTOM_BPDU, &null_pbmp, tmp_addr));
            if (BCM_MAC_IS_ZERO(tmp_addr)) {
                /* assigning the founded free id */
                if (tmp_id == -1) {
                    tmp_id = i;
                }
            } else {
                if (!sal_memcmp(tmp_addr, addr->mac, sizeof(tmp_addr))){
                    tmp_id = i;
                    break;  /* early break for found case only */
                }
            }
        }
        
        if (i == addr_count) {
            /* addr not found condition */
            rv = (tmp_id == -1) ? BCM_E_FULL : BCM_E_NOT_FOUND;
        } else {
            /* addr found coundition */
            rv = BCM_E_NONE;
        }
        
    } else {    /* addr->flags & BCM_L2_CACHE_BPDU */
        /* process for L2 user addr */
        BCM_IF_ERROR_RETURN(bcm_robo_l2_cache_size_get(unit, 
                &addr_count)); 
        
        for (i = 0; i < addr_count; i++){
            sal_memset(&temp_l2u_addr, 0, sizeof (bcm_l2_cache_addr_t));
            rv = bcm_robo_l2_cache_get( unit, i, &temp_l2u_addr);
            if (rv == BCM_E_NONE){

                /* keep the founded free id */
                if (BCM_MAC_IS_ZERO(temp_l2u_addr.mac)) {
                    if (tmp_id == -1){
                        tmp_id = i;
                    }
                }

                /* l2 user addr, ports and flag will be compared */
                if (!sal_memcmp(addr, &temp_l2u_addr, 
                        sizeof (bcm_l2_cache_addr_t))){
                    *id = i;
                    rv = BCM_E_NONE;
                    break;
                }
            } else if (rv == BCM_E_NOT_FOUND){
                /* keep the founded free id */
                if (tmp_id == -1){
                    tmp_id = i;
                }
            } else {
                /* unexpect erorr */
                LOG_INFO(BSL_LS_BCM_ARL,
                         (BSL_META_U(unit,
                                     "%s,failed on check #%d L2 user address\n"),
                          FUNCTION_NAME(), i));
                return rv;
            }
        }

        if ((i == addr_count) && (tmp_id == -1)){
            rv = BCM_E_FULL;
        } else {
            rv = BCM_E_NOT_FOUND;
        }
    }   /* addr->flag & BCM_L2_CACHE_BPDU */
    *id = tmp_id;
    
    return rv;
}

/*
 * Function:
 *    bcm_robo_l2_cache_set
 * Purpose:
 *  There are two major function purpose for ROBO device on this API.
 *  1. set custome BPDU addresses.
 *     - to set the user configurable BPDU addresses
 *          index = 0 : BPDU_0
 *          index = 1 : BPDU_1
 *          index = 2 : BPDU_2
 *  2. set L2 user addresses. (the original API purpose)
 *       - to set L2 cache multiport address and vector register with index
 *          index = 0 : DRV_MAC_MULTIPORT_0,
 *          index = 1 : DRV_MAC_MULTIPORT_1,
 *          index = 2 : DRV_MAC_MULTIPORT_2,
 *          index = 3 : DRV_MAC_MULTIPORT_3,
 *          index = 4 : DRV_MAC_MULTIPORT_4,
 *          index = 5 : DRV_MAC_MULTIPORT_5,
 *
 * Parameters:
 *    unit        - RoboSwitch unit #.
 *    index     - multiport address register index
 *    addr - l2 cache address
 *    index_used - (OUT) l2 cache entry number actually used
 *
 * Returns:
 *    BCM_E_XXXX
 *
 * Note :
 *  index = -1 means user request to add this address on a free index.
 */
int 
bcm_robo_l2_cache_set(int unit, int index, bcm_l2_cache_addr_t *addr, int *index_used) 
{
    int         rv = BCM_E_NONE, search_id = -1;
    uint32      mac_type = 0;
    uint32      addr_count = 0;
    bcm_pbmp_t  null_pbmp;

#if defined(BCM_HARRIER_SUPPORT)
    int         ctrl_val = 0, useraddr_mode = 0;
    int         useraddr_free_cnt = 0;
    uint32      ctrl_cnt = 1, ctrl_type = DRV_DEV_CTRL_L2_USERADDR;    
#endif /* BCM_HARRIER_SUPPORT */
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_cache_set()..\n")));
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* SDK-29951 : API guide comply for index = -1 
     *  1. avoid the existed L2 user address and select a free index.
     *  2. return FULL if no free index.
     */
    if (index == -1){
        if (BCM_MAC_IS_ZERO(addr->mac)){
            /* assigning MAC address at zero for search but index=-1 */
            *index_used = index;
            return BCM_E_PARAM;
        }

        rv = _bcm_robo_l2_cache_search(unit, addr, &search_id);
        if (rv == BCM_E_NONE){
            /* existed already, report the founded index */
            *index_used = search_id;
            return rv;
        } else if (rv == BCM_E_FULL){
            return rv;
        } else if (rv == BCM_E_NOT_FOUND) {
            /* search_id here is expected to represent the free index */
            if (search_id == -1){
                return BCM_E_INTERNAL;
            }
            /* assign the foudned free index(addr at this index is zero) */
            index = search_id;
        } else {
            return BCM_E_INTERNAL;
        }
    }
    
#if defined(BCM_HARRIER_SUPPORT)
    if (SOC_IS_HARRIER(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_harrier_user_addr_mode(unit, 
                &useraddr_mode)); 
        BCM_IF_ERROR_RETURN(_bcm_robo_harrier_free_user_addr_count(unit, 
                &useraddr_free_cnt));
    }
#endif /* BCM_HARRIER_SUPPORT */

    if(addr->flags & BCM_L2_CACHE_BPDU) {
        BCM_PBMP_CLEAR(null_pbmp);
        
        if (index != -1){
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_BPDU_NUM, &addr_count));
            if (index >= addr_count || index < -1){
                *index_used = index;
                return BCM_E_BADID;
            }
        }

#if defined(BCM_HARRIER_SUPPORT)
        if (SOC_IS_HARRIER(unit)) {
            if ((useraddr_mode == _BCM_HARRIER_USER_ADDR_MODE_L2) && 
                    (index == 1 || index == 2)) {
                if (useraddr_free_cnt != 2) {
                    *index_used = index;
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "Failed on set #%d BPDU address, please clear L2 user address first!\n"),
                              index));
                    return BCM_E_CONFIG;
                }
                
                if (!BCM_MAC_IS_ZERO(addr->mac)) {  /* for add/update a valid address */
                    /* set multi_bpdu been enabled */
                    ctrl_val = _BCM_HARRIER_USER_ADDR_MODE_BPDU;
                    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_SET(unit, 
                            &ctrl_cnt, &ctrl_type, &ctrl_val));
                } 
            }            
       }
#endif /* BCM_HARRIER_SUPPORT */
        
        /* set address to target index */
        BCM_IF_ERROR_RETURN(DRV_MAC_SET(unit, null_pbmp, 
                DRV_MAC_CUSTOM_BPDU, addr->mac, index));
        *index_used = index;
        rv = BCM_E_NONE;
    } else if (addr->flags & BCM_L2_CACHE_DESTPORTS) {
#if defined(BCM_HARRIER_SUPPORT)
        if (SOC_IS_HARRIER(unit)) {
            if (useraddr_mode == _BCM_HARRIER_USER_ADDR_MODE_BPDU) {
                if (useraddr_free_cnt != 2) {
                    *index_used = index;
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "Failed on set #%d user L2 address, please clear user external BPDU address first!\n"),
                              index));
                    return BCM_E_CONFIG;
                }
        
                if (!BCM_MAC_IS_ZERO(addr->mac)) {  /* for add/update a valid address */
                    /* set multi_bpdu been disable */
                    ctrl_val = _BCM_HARRIER_USER_ADDR_MODE_L2;
                    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_SET(unit, 
                            &ctrl_cnt, &ctrl_type, &ctrl_val));
                } 
            }
        }
#endif /* BCM_HARRIER_SUPPORT */

        /* routine for set user L2 addr */
        switch (index) {
            case 0:
                mac_type = DRV_MAC_MULTIPORT_0;
                break;
            case 1:             
                mac_type = DRV_MAC_MULTIPORT_1;
                break;
            case 2:
                mac_type = DRV_MAC_MULTIPORT_2;
                break;
            case 3:
                mac_type = DRV_MAC_MULTIPORT_3;
                break;
            case 4:               
                mac_type = DRV_MAC_MULTIPORT_4;
                break;
            case 5:
                mac_type = DRV_MAC_MULTIPORT_5;
                break;
            default:
                *index_used = -1;
                return BCM_E_PARAM;
        }
        rv = DRV_MAC_SET(unit, addr->dest_ports, mac_type, addr->mac, 0);
        BCM_IF_ERROR_RETURN(rv);
        
        *index_used = index;
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_l2_cache_size_get
 * Purpose:
 *      Get number of L2 cache entries
 * Parameters:
 *      unit - device number
 *      size - (OUT) number of entries in cache
 *               return L2_CACHE_ENTRIES_NUM in it's size field for robo.
 * Returns:
 *      L2_CACHE_ENTRIES_NUM
 */
int 
bcm_robo_l2_cache_size_get(int unit, int *size) 
{
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_cache_size_get()..\n")));
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        *size = L2_CACHE_ENTRIES_NUM_VULCAN;
    } else {
        *size = L2_CACHE_ENTRIES_NUM;
    }
    
    return BCM_E_NONE;
}
