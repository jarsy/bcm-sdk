/*
 * $Id: mcast.c,v 1.67 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Multicast Table Management
 */
#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/l2.h>
#include <bcm_int/robo/mcast.h>
#include <bcm_int/robo/port.h>

#include <bcm_int/common/multicast.h>

#include <soc/debug.h>
#include <soc/feature.h>
#include <soc/macipadr.h>
#include <soc/drv.h>
#include <soc/mcast.h>
#include <soc/robo/mcm/memregs.h>
#include <bcm_int/robo_dispatch.h>

static int _robo_bcm_mcast_init[BCM_MAX_NUM_UNITS];

#define MCAST_INIT_RETURN(unit) \
        if (!_robo_bcm_mcast_init[unit]) {return BCM_E_INIT;}

/*
 * Function:
 *      _bcm_robo_l2mc_id_alloc
 * Description:
 *      allocate the given l2mc entry (increment use count)
 */

int
_bcm_robo_l2mc_id_alloc(int unit, int l2mc_id)
{
    L2MC_ID(unit, l2mc_id);

    L2MC_USED_SET(unit, l2mc_id);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_l2mc_id_check
 * Description:
 *      check if the given l2mc id is free or not.
 */

int
_bcm_robo_l2mc_id_check(int unit, int l2mc_id)
{
    L2MC_ID(unit, l2mc_id);

    return (L2MC_USED_ISSET(unit, l2mc_id));
}

/*
 * Function:
 *      _bcm_robo_l2mc_id_free
 * Description:
 *      Marks an l2mc index as free (decrements use count)
 */

int
_bcm_robo_l2mc_id_free(int unit, int l2mc_id)
{
    L2MC_ID(unit, l2mc_id);

    L2MC_USED_CLR(unit, l2mc_id);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_l2mc_free_index
 * Description:
 *      Find an unused l2mc index and mark it used.
 */
int
_bcm_robo_l2mc_free_index(int unit, int type, int *l2mc_id)
{
    int     rv = BCM_E_FULL;
    int     i, temp = 0, temp_id = 0, temp_max = 0;
    
    if (SOC_IS_TBX(unit)){
        rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                (uint32 *)&temp);
        if (rv){
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s, %d, device property get problem!\n"),
                       FUNCTION_NAME(), __LINE__));
            return BCM_E_INTERNAL;
        }
        if (type == _BCM_MULTICAST_TYPE_L2){
            temp_id = temp;
            temp_max = L2MC_SIZE(unit);
        } else if ((type == _BCM_MULTICAST_TYPE_VLAN) || 
                (type == _BCM_MULTICAST_TYPE_SUBPORT)){
            temp_id = 0;
            temp_max = temp;
        } else {
            temp_id = 0;
            temp_max = L2MC_SIZE(unit);
        }
    } else {
        temp_id = 0;
        temp_max = L2MC_SIZE(unit);
    }

    *l2mc_id = -1;
    for (i = temp_id; i < temp_max; i++) {
        if (!L2MC_USED_ISSET(unit, i)) {
            L2MC_USED_SET(unit, i);
            *l2mc_id = i;
            break;
        }
    }
    
    if (*l2mc_id == -1){
        rv = BCM_E_FULL;
    } else {
        rv = BCM_E_NONE;
    }

    return rv;
}


/* 
 * Initialize mcast module
 */

/*
 * Function:
 *      bcm_robo_mcast_init
 * Purpose:
 *      Initialize multicast bcm routines.
 * Parameters:
 *      unit - RoboSwitch unit #
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table write failed.
 */
int
bcm_robo_mcast_init(int unit)
{
    uint32      mcast_num = 0;
    
    if (L2MC_USED(unit) != NULL) {
        sal_free(L2MC_USED(unit));
    }

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_init()..\n")));
   /* get the number of supported Mcast gourps in this unit */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(
                    unit, DRV_DEV_PROP_MCAST_NUM, &mcast_num));
    L2MC_SIZE(unit) = mcast_num;
    L2MC_USED(unit) = sal_alloc(sizeof(int) * L2MC_SIZE(unit), "L2MC");
    if (L2MC_USED(unit) == NULL) {
        return BCM_E_MEMORY;
    }

    BCM_IF_ERROR_RETURN(DRV_MCAST_INIT(unit));
    
    sal_memset(L2MC_USED(unit), 0, sizeof(int) * L2MC_SIZE(unit));
    
    _robo_bcm_mcast_init[unit] = TRUE;

    /* return L2MC size if no error here! 
     *  for the API design changed in API guide and regression test. 
     */
    return mcast_num;
}

/*
 * Function:
 *      _bcm_robo_mcast_detach
 * Purpose:
 *      De-initialize multicast bcm routines.
 * Parameters:
 *      unit - (IN) BCM device number.
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_robo_mcast_detach(int unit)
{
    if (L2MC_USED(unit) != NULL) {
        sal_free(L2MC_USED(unit));
        L2MC_USED(unit) = NULL;
    }
    L2MC_SIZE(unit) = 0;

    _robo_bcm_mcast_init[unit] = FALSE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mcast_port_get
 * Purpose:
 *      Get membership and untagged port bit maps for a MC group
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      mac - mac address used for lookup.
 *      vid - vlan id used for lookup.
 *  mcaddr - pointer to bcm_mcast_addr_t that is fully filled in
 * Returns:
 *      BCM_E_NONE      Success (mcaddr filled in)
 *      BCM_E_INTERNAL  Chip access failure
 *      BCM_E_NOT_FOUND Address not found (mcaddr not filled in)
 */
int bcm_robo_mcast_port_get(int unit, bcm_mac_t mac, bcm_vlan_t vid, 
                  bcm_mcast_addr_t *mcaddr)
{
    bcm_mcast_addr_t    mcast_search;
    l2_marl_sw_entry_t      marl_entry, marl_result;
    int                 rv = BCM_E_NONE;

    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_port_get()..\n")));

    /* valid VLAN check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    bcm_mcast_addr_init(&mcast_search, mac, vid);

    sal_memset(&marl_entry, 0, sizeof (marl_entry));
    sal_memset(&marl_result, 0, sizeof (marl_result));
    
    BCM_IF_ERROR_RETURN(DRV_MCAST_TO_MARL(
            unit, (uint32 *)&marl_entry, (uint32 *)&mcast_search));
    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (
            uint32 *)&marl_entry, (uint32 *)&marl_result, NULL,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));
    if (rv < 0) {
        if (rv != SOC_E_EXISTS) {
            if (rv == SOC_E_TIMEOUT) {
                return BCM_E_TIMEOUT;
            } else {
               LOG_INFO(BSL_LS_BCM_ARL,
                        (BSL_META_U(unit,
                                    "[bcm.mcast]MC MAC address + VID not found\n")));
                return BCM_E_NOT_FOUND; 
            }
        }
    }
    
    BCM_PBMP_CLEAR(mcaddr->pbmp);
    BCM_IF_ERROR_RETURN(DRV_MCAST_FROM_MARL(
            unit, (uint32 *)mcaddr, (uint32 *)&marl_result));

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "bcm_robo_mcast_port_get(): mc_pbmp=0x%x\n"),
              SOC_PBMP_WORD_GET(mcaddr->pbmp, 0)));

    return BCM_E_NONE;

}   

/*
 * Function:
 *      bcm_mcast_addr_add
 * Purpose:
 *      Add an entry in the multicast table
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mcaddr - Pointer to fully filled-in bcm_mcast_addr_t
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table write failed.
 */
int bcm_robo_mcast_addr_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    l2_marl_sw_entry_t  marl_entry, marl_result_entry;
    bcm_mcast_addr_t    rtaddr;
    pbmp_t          t_pbm;
    int             rv = BCM_E_NONE;
    uint32          index = 0, max_mc_cnt = 0;

    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_addr_add()..\n")));
    /* valid VLAN check */
    if(mcaddr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid CoS check */
    if(mcaddr->cos_dst < BCM_PRIO_MIN || mcaddr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(DRV_MCAST_TO_MARL(
            unit, (uint32 *)&marl_entry, (uint32 *)mcaddr));

    /* Check if this MAC+VID is exist */
    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&marl_entry, 
                    (uint32 *)&marl_result_entry, NULL,
                    (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID 
                      | DRV_MEM_OP_REPLACE));
    if (rv == BCM_E_NOT_FOUND || rv == BCM_E_EXISTS){
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&marl_result_entry, &index));

        if (rv == BCM_E_NOT_FOUND) {
            if (mcaddr->flags & _BCM_ROBO_MCAST_FORCE_L2MC_ID){
                BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(
                        unit, DRV_DEV_PROP_MCAST_NUM, (uint32 *)&max_mc_cnt));
                if (mcaddr->l2mc_index >= max_mc_cnt) {
                    return BCM_E_PARAM;
                }
                
                /* assigning the user forced index */
                index = mcaddr->l2mc_index;
            }
        }
        /* Set new multicast index value */
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP,
                    (uint32 *)&marl_entry, &index)); 
    } else if (rv < 0) {
        return rv;
    }

    /* set the new created MCAST pbmap*/
    BCM_PBMP_CLEAR(t_pbm);
    BCM_PBMP_OR(t_pbm, mcaddr->pbmp);

    if (SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit)) {
        rv = bcm_robo_mcast_port_get(
                unit, (uint8 *)_soc_mac_all_routers, mcaddr->vid, &rtaddr);
        if (rv >= 0) {
            BCM_PBMP_OR(t_pbm, rtaddr.pbmp );
        }
    }

    BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_SET(
            unit, (uint32 *)&marl_entry, t_pbm, DRV_MCAST_INDEX_ADD));

    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_mcast_addr_remove
 * Purpose:
 *      Delete an entry from the multicast table
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mac  - mac address
 *      vid  - vlan id
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXXX
 */
int bcm_robo_mcast_addr_remove(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_mcast_addr_t    mcaddr;
    l2_marl_sw_entry_t      marl_entry, marl_result;
    int                 rv = BCM_E_NONE;
    pbmp_t              null_pbmp;

    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);

    /* valid VLAN check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_addr_remove()..\n")));
    
    bcm_mcast_addr_init(&mcaddr, mac, vid);
    
    sal_memset(&marl_entry, 0, sizeof (marl_entry));
    sal_memset(&marl_result, 0, sizeof (marl_result));

    BCM_IF_ERROR_RETURN(DRV_MCAST_TO_MARL(
            unit, (uint32 *)&marl_entry, (uint32 *)&mcaddr));

    /* check if this MAC+VID is exist */
    rv = DRV_MEM_SEARCH(
            unit, DRV_MEM_ARL, (uint32 *)&marl_entry, 
            (uint32 *)&marl_result, NULL,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));

    if (rv != SOC_E_EXISTS) {
        if (rv == SOC_E_TIMEOUT) {
            return BCM_E_TIMEOUT;
        } else {
            return BCM_E_NOT_FOUND; 
        }
    }
    
    BCM_PBMP_CLEAR(null_pbmp);
    /* delete this MC group table(MC pbmap table) */
    BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_SET(
                    unit, (uint32 *)&marl_result, null_pbmp,
                    DRV_MCAST_INDEX_REMOVE));
    
    /* delete this entry in MARL(ARL) table */
    rv = bcm_robo_l2_addr_delete(unit, mac, vid);

    return rv;
}

              
/*
 * Function:
 *  bcm_mcast_join
 * Description:
 *  This function adds the given port to the membership of the
 *  given mcast mac address.
 * Parameters:
 *  unit      - SOC unit #
 *  mcMacAddr - Multicast Address to which the new member is to be added
 *  vlanId    - Vlan ID
 *  srcPort   - port to be added as a new member of the given
 *                           multicast address
 *  mcaddr    - used for returning updated entry in mcast table with
 *                             the given mcast mac address
 *  allRoutersBmp - used for returning the all routers bitmap
 *
 * Returns:
 *  BCM_MCAST_JOIN_ADDED - If a new entry is created for the
 *                                     given mcast address.
 *  BCM_MCAST_JOIN_UPDATED - If the mcast address is found in
 *                                       the table and bitmap is updated.
 *  BCM_INTERNAL  - If L2 mcast table search/insert fails.
 * Notes:
 *  BCM_MCAST_xxx values are non-negative success codes
 */
int bcm_robo_mcast_join(int unit, bcm_mac_t mcMacAddr, bcm_vlan_t vlanId,
              int srcPort, bcm_mcast_addr_t *mcaddr,
              bcm_pbmp_t *allRouterBmp)
{
    int             rv = BCM_E_NONE, port_in = 0;
    bcm_mcast_addr_t    newmc;
     
    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_join()..\n")));

    /* valid VLAN check */
    if(vlanId > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }
    
    /* valid port check */
    rv = _bcm_robo_port_gport_validate(unit, srcPort, &port_in);
    if (rv == BCM_E_PORT){
        return BCM_E_PARAM;
    }

    /*
     * Fill in all_routers port bitmap if allRouterBmp is not NULL pointer
     */
    if (allRouterBmp != NULL) {
        rv = bcm_robo_mcast_port_get(unit, (uint8 *)_soc_mac_all_routers, 
                vlanId, &newmc);
        if (rv == BCM_E_NOT_FOUND) {
            BCM_PBMP_CLEAR(*allRouterBmp);
        } else if (rv < 0) {
            return rv;
        } else {
            BCM_PBMP_ASSIGN(*allRouterBmp, newmc.pbmp);
        }
    }

    /*
     * Now, join the actual multicast group
     */
    bcm_mcast_addr_t_init(&newmc, mcMacAddr, vlanId);
    BCM_PBMP_PORT_SET(newmc.pbmp, port_in);
    rv = bcm_robo_mcast_port_add(unit, &newmc);

    if (rv == BCM_E_NOT_FOUND) {
        if (SOC_IS_TBX(unit)){
            /* For TB, the l2mc_index = 0 is specified for Mcast replication 
             *  solution, not like other ROBO chips(example: bcm53242) can 
             *  use l2mc_index = 0 to request a new free L2MC index.
             *  
             *  To request new L2MC index on TB can be approached by assigning
             *  the l2mc_index to -1. 
             */
            newmc.l2mc_index = -1;
        }
        BCM_IF_ERROR_RETURN(bcm_robo_mcast_addr_add(unit, &newmc));
        rv = BCM_MCAST_JOIN_ADDED;
    } else if (rv < 0) {
        return rv;
    } else {
        rv = BCM_MCAST_JOIN_UPDATED;
    }
    if (mcaddr != NULL) {
        BCM_IF_ERROR_RETURN(bcm_robo_mcast_port_get(
                unit, mcMacAddr, vlanId, mcaddr));
    }
    return rv;
}             

/*
 * Function:
 *  bcm_mcast_leave
 * Description:
 *  This function removes the given port from the group membership
 *  of the given mcast mac address.
 * Parameters:
 *  unit      - SOC unit #
 *  mcMacAddr - Multicast Address from which the member is to be removed
 *  vlanId    - Vlan ID
 *  srcPort   - port to be removed from the membership of the
 *          given multicast address
 * Returns:
 *  BCM_MCAST_LEAVE_DELETED - If the entry with the given mcast
 *                                        addr is deleted from H/W table
 *  BCM_MCAST_LEAVE_UPDATED - If the mcast address is found in
 *                                        the table and bitmap is updated.
 *  BCM_MCAST_LEAVE_NOTFOUND - If the mcast address is not found in
 *                                         the table
 *  BCM_E_INTERNAL  - If L2 mcast table search/insert fails.
 * Notes:
 *  BCM_MCAST_xxx values are non-negative success codes
 */

int bcm_robo_mcast_leave(int unit, bcm_mac_t mcMacAddr, bcm_vlan_t vlanId,
               int srcPort)
{
    int rv = BCM_E_NONE, port_in = 0;
    bcm_mcast_addr_t    mcaddr;
    bcm_mcast_addr_t    rtaddr;

    MCAST_INIT_RETURN(unit);

    /* valid VLAN check */
    if(vlanId > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid port check */
    rv = _bcm_robo_port_gport_validate(unit, srcPort, &port_in);
    if (rv == BCM_E_PORT){
        return BCM_E_PARAM;
    }

    bcm_mcast_addr_t_init(&mcaddr, mcMacAddr, vlanId);
    BCM_PBMP_PORT_SET(mcaddr.pbmp, port_in);
    rv = bcm_robo_mcast_port_remove(unit, &mcaddr);
    if (rv < 0) {
        return rv;
    } else {
        rv = bcm_robo_mcast_port_get(unit, mcMacAddr, vlanId, &mcaddr);
    }

    if (rv == BCM_E_NOT_FOUND) {
        rv = BCM_MCAST_LEAVE_DELETED;
    } else if (rv < 0) {
        return rv;
    } else {
        if (SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit)) {
            rv = bcm_robo_mcast_port_get(unit, (uint8 *)_soc_mac_all_routers, 
                    vlanId, &rtaddr);
            if (rv < 0) {
                bcm_mcast_addr_t_init(
                        &rtaddr, (uint8 *)_soc_mac_all_routers, vlanId);
            }
            BCM_PBMP_REMOVE(mcaddr.pbmp, rtaddr.pbmp);
        }
        if (BCM_PBMP_IS_NULL(mcaddr.pbmp)) {
            BCM_IF_ERROR_RETURN(bcm_robo_mcast_addr_remove(
                    unit, mcMacAddr, vlanId));
            return BCM_MCAST_LEAVE_DELETED;
        }
        rv = BCM_MCAST_LEAVE_UPDATED;
    }

    return rv;
}              

/*
 * Function:
 *      bcm_robo_mcast_addr_remove_w_l2mcindex
 * Purpose:
 *      Delete an entry from the multicast table and provide l2mcindex in
 *      bcm_mcast_addr_t of the deleted entry
 * Parameters:
 *      unit - unit number.
 *      mcaddr - Pointer to  filled-in bcm_mcast_addr_t with mac addr and vid
 *               l2mc_index is provided as output
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXXX
 */
int
bcm_robo_mcast_addr_remove_w_l2mcindex(int unit, bcm_mcast_addr_t *mcaddr)
{
    int                 rv = BCM_E_NONE, mem_cnt = 0;
    bcm_l2_addr_t       l2addr;

    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_addr_remove_w_l2mcindex()..\n")));

    /* pre-processing check ...*/
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }

    /* valid VLAN check */
    if(mcaddr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid CoS check */
    if(mcaddr->cos_dst < BCM_PRIO_MIN || mcaddr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }
    
    /* valid l2mc_id check */
    if(mcaddr->l2mc_index > L2MC_SIZE(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_get(unit, 
            mcaddr->mac, mcaddr->vid, &l2addr));

    if (!(l2addr.flags & BCM_L2_MCAST)) {
        return BCM_E_NOT_FOUND;
    } else {
        /* bcm_l2_addr_get() for existed cases return BCM_E_NONE */
        if (l2addr.l2mc_group != mcaddr->l2mc_index){
            LOG_INFO(BSL_LS_BCM_L2TABLE,
                     (BSL_META_U(unit,
                                 "%s, entry found but mismatch on l2mc_group \n"),
                      FUNCTION_NAME()));
            return BCM_E_PARAM;
        }
    }

    return bcm_robo_mcast_addr_remove(unit, mcaddr->mac, mcaddr->vid);
}

/*
 * Function:
 *      bcm_robo_mcast_addr_add_w_l2mcindex
 * Purpose:
 *      Add an entry in the multicast table with l2mc index provided
 * Parameters:
 *      unit - unit number.
 *  mcaddr - Pointer to fully filled-in bcm_mcast_addr_t
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table write failed.
 * Note :
 *  - All valid mcast group index is allowed.
 */
int
bcm_robo_mcast_addr_add_w_l2mcindex(int unit, bcm_mcast_addr_t *mcaddr)
{
    int     rv = BCM_E_NONE, mem_cnt = 0;
    int     mcindex = 0;
    
    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);
    
    /* pre-processing check ...*/
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }
    
    /* valid VLAN check */
    if(mcaddr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid CoS check */
    if(mcaddr->cos_dst < BCM_PRIO_MIN || mcaddr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }
    
    /* valid l2mc_id check */
    if(mcaddr->l2mc_index > L2MC_SIZE(unit)){
        return BCM_E_PARAM;
    }

    if (_BCM_MULTICAST_IS_SET(mcaddr->l2mc_index)) {
        if (_BCM_MULTICAST_IS_L2(mcaddr->l2mc_index)) {
            mcindex = _BCM_MULTICAST_ID_GET(mcaddr->l2mc_index);
        } else {
            return BCM_E_PARAM;
        }
    } else {
        mcindex = mcaddr->l2mc_index;
    }

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API:%s,mc_id=%d..\n"), FUNCTION_NAME(), mcindex));

    mcaddr->flags |= _BCM_ROBO_MCAST_FORCE_L2MC_ID;

    return bcm_robo_mcast_addr_add(unit, mcaddr);
    
}

/*
 * Function:
 *      bcm_mcast_port_remove
 * Purpose:
 *      Remove ports from an existing entry in the multicast table
 * Parameters:
 *      unit - unit number.
 *  mcaddr - Pointer to fully filled-in bcm_mcast_addr_t
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table write failed.
 */
int bcm_robo_mcast_port_remove(int unit, bcm_mcast_addr_t *mcaddr)
{
    l2_marl_sw_entry_t  marl_entry, marl_result;
    pbmp_t          pbm, in_pbmp;
    int             rv = BCM_E_NONE;

    MCAST_INIT_RETURN(unit);
    L2MC_INIT(unit);

    if (mcaddr == NULL){
        return BCM_E_PARAM;
    }

    /* valid VLAN check */
    if(mcaddr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid CoS check */
    if(mcaddr->cos_dst < BCM_PRIO_MIN || mcaddr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }

    /* pbmp validation */
    if (BCM_PBMP_IS_NULL(mcaddr->pbmp)){
        /* no port is indicated for remove */
        return BCM_E_NONE;
    } else {
        BCM_PBMP_ASSIGN(in_pbmp, mcaddr->pbmp);
        BCM_PBMP_REMOVE(in_pbmp, PBMP_PORT_ALL(unit));
        if (BCM_PBMP_NOT_NULL(in_pbmp)){
            return SOC_E_PARAM;
        }
    }

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_port_remove()..\n")));
    sal_memset(&marl_entry, 0, sizeof (marl_entry));
    sal_memset(&marl_result, 0, sizeof (marl_result));
    
    BCM_IF_ERROR_RETURN(DRV_MCAST_TO_MARL(
            unit, (uint32 *)&marl_entry, (uint32 *)mcaddr));

    rv = DRV_MEM_SEARCH(
                    unit, DRV_MEM_ARL, (uint32 *)&marl_entry, 
                    (uint32 *)&marl_result, NULL,
                    (DRV_MEM_OP_BY_HASH_BY_MAC | 
                            DRV_MEM_OP_BY_HASH_BY_VLANID));
    if (rv < 0) {
        if (rv != SOC_E_EXISTS) {
            if (rv == SOC_E_TIMEOUT) {
                return BCM_E_TIMEOUT;
            } else {
                LOG_INFO(BSL_LS_BCM_ARL,
                         (BSL_META_U(unit,
                                     "[bcm.mcast]MC MAC address + VID not found\n")));
                return BCM_E_NOT_FOUND; 
            }
        }
    }
    
    /* Get the current MC pbmp */
    BCM_PBMP_CLEAR(pbm);
    BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_GET(
                    unit, (uint32 *)&marl_result, &pbm));

    /* check if no member changed */
    BCM_PBMP_ASSIGN(in_pbmp, pbm);
    BCM_PBMP_REMOVE(in_pbmp, mcaddr->pbmp);
    if (BCM_PBMP_EQ(in_pbmp, pbm)){
        /* no member changed */
        return BCM_E_NONE;
    } else {
        /* remove from current pbmp on indicated pbmp in mcaddr */
        BCM_PBMP_REMOVE(pbm, mcaddr->pbmp);
    }

    if (BCM_PBMP_IS_NULL(pbm)) {
        /* rewrite the mc_pbmp table */
        /* if pbmp is empty, delete this mcast pbmp entry */
        BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_SET(
                        unit, (uint32 *)&marl_result, pbm,
                        DRV_MCAST_INDEX_REMOVE));
        
        /* delete this Mcast entry in ARL */
        rv =DRV_MEM_DELETE(
                unit, DRV_MEM_ARL, (uint32 *)&marl_result,
                (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID |
                DRV_MEM_OP_DELETE_BY_STATIC));
        if (rv < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR in deleting mcast entry from ARL Table\n")));
            return BCM_E_INTERNAL;
        }
    } else {
        /* rewrite mc pbmp table */
        BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_SET(
                        unit, (uint32 *)&marl_result, pbm, 0));
    }
    return BCM_E_NONE;

}   

/*
 * Function:
 *      bcm_mcast_port_add
 * Purpose:
 *      Add ports to an existing entry in the multicast table
 * Parameters:
 *      unit - unit number.
 *  mcaddr - Pointer to fully filled-in bcm_mcast_addr_t
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table write failed.
 */
int bcm_robo_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    l2_marl_sw_entry_t  marl_entry, marl_result;
    pbmp_t          pbm, in_pbmp;
    int             rv = BCM_E_NONE;

    MCAST_INIT_RETURN(unit);

    if (mcaddr == NULL){
        return BCM_E_PARAM;
    }

    /* valid VLAN check */
    if(mcaddr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    /* valid CoS check */
    if(mcaddr->cos_dst < BCM_PRIO_MIN || mcaddr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }

    /* pbmp validation */
    if (BCM_PBMP_IS_NULL(mcaddr->pbmp)){
        /* no port is indicated for add */
        return BCM_E_NONE;
    } else {
        BCM_PBMP_ASSIGN(in_pbmp, mcaddr->pbmp);
        BCM_PBMP_REMOVE(in_pbmp, PBMP_PORT_ALL(unit));
        if (BCM_PBMP_NOT_NULL(in_pbmp)){
            return SOC_E_PARAM;
        }
    }

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_mcast_port_add()..\n")));
    sal_memset(&marl_entry, 0, sizeof (marl_entry));
    sal_memset(&marl_result, 0, sizeof (marl_result));
    
    BCM_IF_ERROR_RETURN(DRV_MCAST_TO_MARL(
            unit, (uint32 *)&marl_entry, (uint32 *)mcaddr));

    rv = DRV_MEM_SEARCH(
            unit, DRV_MEM_ARL, (uint32 *)&marl_entry, 
            (uint32 *)&marl_result, NULL,
            (DRV_MEM_OP_BY_HASH_BY_MAC | 
                    DRV_MEM_OP_BY_HASH_BY_VLANID));
    if (rv < 0) {
        if (rv != SOC_E_EXISTS) {
            if (rv == SOC_E_TIMEOUT) {
                return BCM_E_TIMEOUT;
            } else {
                LOG_INFO(BSL_LS_BCM_ARL,
                         (BSL_META_U(unit,
                                     "[bcm.mcast]MC MAC address + VID not found\n")));
                return BCM_E_NOT_FOUND; 
            }
        }
    }
                    
    /* Get the current MC pbmp */
    BCM_PBMP_CLEAR(pbm);
    BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_GET(
                    unit, (uint32 *)&marl_result, &pbm));

    /* check if no member changed */
    BCM_PBMP_ASSIGN(in_pbmp, pbm);
    BCM_PBMP_OR(in_pbmp, mcaddr->pbmp);
    if (BCM_PBMP_EQ(in_pbmp, pbm)){
        /* no member changed */
        return BCM_E_NONE;
    } else {
        /* add the desired pbmp in mcaddr into current pbmp*/
        BCM_PBMP_OR(pbm, mcaddr->pbmp);
    }

    /* rewrite the mc_pbmp table */
    BCM_IF_ERROR_RETURN(DRV_MCAST_BMP_SET(
                    unit, (uint32 *)&marl_result, pbm, 0));
                    
    return BCM_E_NONE;
}   

