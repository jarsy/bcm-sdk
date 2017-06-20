/*
* $Id:l3.c,v 1.0 2013/6/28 09:05:00 Jianping Exp $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains the l3 intf and egress intf defintions for the MPLS operation
*/

#include <soc/drv.h>

#include <bcm/l3.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/robo/l3.h>
#include <soc/defs.h>

#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)

static bcm_mac_t _l3_mac[BCM_MAX_NUM_UNITS];/* router source mac ,keep the first */

static _bcm_robo_l3_entry_t _bcm_robo_l3_table[BCM_MAX_NUM_UNITS] = {{0,},};

#define _BCM_ROBO_L3_LINK_ADD(head, node) do{ \
    node->p_next = head->p_next; \
    if(head->p_next){ \
        head->p_next->p_pre = node; \
    } \
    head->p_next = node; \
    node->p_pre  = head; \
} while(0)

#define _BCM_ROBO_L3_LINK_DEL(head, node) do{ \
    node->p_pre->p_next = node->p_next; \
    if(node->p_next){ \
        node->p_next->p_pre = node->p_pre; \
    } \
    sal_free(node); \
} while(0)        

#define _BCM_ROBO_L3_LINK_SRCH_1_KEY(head, field, field_val, ptr) do{ \
    ptr = head->p_next; \
    while(ptr){ \
        if(ptr->field == field_val){ \
            break; \
        } \
        ptr = ptr->p_next; \
    } \
}while(0)

/*
 * Function:
 *      _bcm_robo_l3_intf_get
 * Description:
 *      find the l3 table entry according to the l3_id and unit id
 * Parameters:
 *      unit     - (IN) BCM device number
 *      intf_id  - (IN) l3 intf id
 * Returns     : 
 *      NULL or pointer
 */
_bcm_robo_l3_entry_t*
_bcm_robo_l3_intf_get(int unit, bcm_if_t intf_id)
{
    _bcm_robo_l3_entry_t *p_entry = NULL;

    _BCM_ROBO_L3_LINK_SRCH_1_KEY((&_bcm_robo_l3_table[unit]), l3_intf_index, intf_id, p_entry);

    return p_entry;
}

/*
 * Function:
 *      _bcm_robo_l3_intf_add
 * Description:
 *      add one l3 entry to the l3 table, index by the l3 intf id
 * Parameters:
 *      unit      - (IN) BCM device number
 *      intf_id   - (IN) intf
 * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_l3_intf_add(int unit, bcm_l3_intf_t *intf)
{
    bcm_mac_t mac = {0,};
    _bcm_robo_l3_entry_t *p_entry = NULL;

    if(sal_memcmp(_l3_mac[unit], mac, sizeof(bcm_mac_t))!=0){
        if(sal_memcmp(intf->l3a_mac_addr, _l3_mac[unit], sizeof(bcm_mac_t)) != 0){/*not equal with the previous*/
            return BCM_E_PARAM;
        }
    }

    p_entry = sal_alloc(sizeof(_bcm_robo_l3_entry_t),"robo l3 entry");
    if(!p_entry){
        return BCM_E_MEMORY;
    }

    sal_memset(p_entry, 0, sizeof(_bcm_robo_l3_entry_t));
    sal_memcpy(p_entry->src_mac, intf->l3a_mac_addr, sizeof(bcm_mac_t));
    p_entry->l3_intf_index = intf->l3a_intf_id;

    _BCM_ROBO_L3_LINK_ADD((&_bcm_robo_l3_table[unit]), p_entry);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_l3_srcmac_add
 * Description:
 *      add one router src mac to the hw
 * Parameters:
 *      unit       - (IN) BCM device number
 *      router_mac - (IN) router mac addr
 * Returns     : 
 *      void
 */
static void 
_bcm_robo_l3_srcmac_add(int unit, bcm_mac_t router_mac)
{
    bcm_mac_t mac={0};

    if(sal_memcmp(_l3_mac[unit], mac, sizeof(bcm_mac_t)) == 0){/*keep the first one*/
        sal_memcpy(_l3_mac[unit], router_mac, sizeof(bcm_mac_t));
    }
}

/*
 * Function:
 *      _bcm_robo_l3_srcmac_add
 * Description:
 *      delete the src mac record
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : 
 *      void
 */
static void 
_bcm_robo_l3_srcmac_del(int unit)
{
    sal_memset(_l3_mac[unit], 0, sizeof(bcm_mac_t));
}
#endif
#endif
#endif

/*
 * Function:
 *      bcm_robo_l3_egress_create
 * Description:
 *      updat the l3 table's egress parameter including the dst mac and dst port as well as VLAN 
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) only support BCM_L3_WITH_ID 
 *      egr   - (IN)  containg the vlan,dst mac and dst port
 *      if_id - (OUT) return the l3 intf id 
 * Returns     : 
 *      BCM_E_XXX
 */
int
bcm_robo_l3_egress_create(int unit, uint32 flags,
                                     bcm_l3_egress_t *egr, bcm_if_t *if_id)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    /* found the l3 entry and keep the dst mac addr and egr port to _bcm_robo_l3_table */
    _bcm_robo_l3_entry_t *p_entry = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if(!(egr->flags& BCM_L3_WITH_ID)){
        return BCM_E_PARAM;
    }

    p_entry = _bcm_robo_l3_intf_get(unit, egr->intf);
    if(!p_entry){
        return BCM_E_NOT_FOUND;
    }

    *if_id                 = egr->intf;
    p_entry->egr_port      = egr->port;
    p_entry->vlan          = egr->vlan;
    sal_memcpy(p_entry->dst_mac, egr->mac_addr, sizeof(bcm_mac_t));

    return BCM_E_NONE;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */
}

/*
 * Function:
 *      bcm_robo_l3_egress_get
 * Purpose:
 *      Get an Egress forwarding object.
 * Parameters:
 *      unit  - (IN) bcm device.
 *      intf  - (IN) L3 interface id pointing to Egress object.
 *      egr   - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_egress_get(int unit, bcm_if_t if_id, bcm_l3_egress_t *egr)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_l3_entry_t *p_entry;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_entry = _bcm_robo_l3_intf_get(unit, if_id);
    if(!p_entry){
        return BCM_E_NOT_FOUND;
    }

    egr->port       = p_entry->egr_port;
    egr->vlan       = p_entry->vlan;
    egr->intf       = p_entry->l3_intf_index;
    sal_memcpy(egr->mac_addr, p_entry->dst_mac, sizeof(bcm_mac_t));

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */

#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */

#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */
}

/*
 * Function:
 *      bcm_robo_l3_init
 * Purpose:
 *      Initialize the L3 module.
 * Parameters:
 *      unit  - (IN) bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_init(int unit)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_l3_intf_create
 * Description:
 *      create one l3 intf object
 * Parameters:
 *      unit  - (IN) BCM device number
 *      intf  - (IN) intf struct containing the intf id and src mac addr
 * Returns     : 
 *      BCM_E_XXX
 */
int
bcm_robo_l3_intf_create(int unit, bcm_l3_intf_t *intf)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv = BCM_E_NONE;
    _bcm_robo_l3_entry_t *p_entry;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_entry = _bcm_robo_l3_intf_get(unit, intf->l3a_intf_id);
    if(p_entry){
        return BCM_E_EXISTS;
    }

    rv = _bcm_robo_l3_intf_add(unit, intf);
    if(rv == BCM_E_NONE){
        _bcm_robo_l3_srcmac_add(unit, intf->l3a_mac_addr);
    }

    return rv;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_l3_intf_delete
 * Description:
 *      delete one l3 intf object
 * Parameters:
 *      unit  - (IN) BCM device number
 *      intf  - (IN) intf struct containing the intf id and src mac addr
 * Returns     : 
 *      BCM_E_XXX
 */
int
bcm_robo_l3_intf_delete(int unit, bcm_l3_intf_t *intf)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_l3_entry_t *p_entry;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_entry = _bcm_robo_l3_intf_get(unit, intf->l3a_intf_id);
    if(!p_entry){
        return BCM_E_NOT_FOUND;
    }

    _BCM_ROBO_L3_LINK_DEL((&_bcm_robo_l3_table[unit]), p_entry);   

    if(!_bcm_robo_l3_table[unit].p_next){
        _bcm_robo_l3_srcmac_del(unit);
    }

    return BCM_E_NONE;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_l3_intf_delete_all
 * Description:
 *      delete all l3 intf object
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : 
 *      BCM_E_XXX
 */
int
bcm_robo_l3_intf_delete_all(int unit)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_l3_entry_t *p_entry = _bcm_robo_l3_table[unit].p_next;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    while(p_entry){
        _BCM_ROBO_L3_LINK_DEL((&_bcm_robo_l3_table[unit]), p_entry);

        p_entry = _bcm_robo_l3_table[unit].p_next;
    }

    _bcm_robo_l3_srcmac_del(unit);

    return BCM_E_NONE;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_l3_intf_get
 * Description:
 *      get one matched l3 intf object
 * Parameters:
 *      unit  - (IN) BCM device number
 *      intf  - (OUT) pointer to the matched object
 * Returns     : 
 *      BCM_E_XXX
 */
int
bcm_robo_l3_intf_get(int unit, bcm_l3_intf_t *intf)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_l3_entry_t *p_entry;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    _BCM_ROBO_L3_LINK_SRCH_1_KEY((&_bcm_robo_l3_table[unit]), l3_intf_index, intf->l3a_intf_id, p_entry);
    if(!p_entry){
        return BCM_E_NOT_FOUND;
    }

    sal_memset(intf, 0, sizeof(bcm_l3_intf_t));
    sal_memcpy(intf->l3a_mac_addr, p_entry->src_mac, sizeof(bcm_mac_t));
    intf->l3a_intf_id = p_entry->l3_intf_index;
    
    return BCM_E_NONE;
#else
        return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
    
#else
        return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}



