/*
 * $Id: l2.c,v 1.18 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     l2.c
 * Purpose:
 *
 */
#include <shared/bsl.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/types.h>
#include <bcm/l2.h>
#include <bcm_int/tk371x_dispatch.h>
#include <bcm/error.h>

#include <soc/ea/tk371x/brg.h>
#include <soc/ea/tk371x/onu.h>


static int tk371x_l2_freezed[SOC_MAX_NUM_DEVICES] = {0};
static int tk371x_l2_detached[SOC_MAX_NUM_DEVICES] = {0};

#define STATIC_MAC_ADDR_ENTRY_MAX_NUM	256
static int _tk371x_clear_all_staticmacs(int unit){
	int rv;
	uint8 entryCount; 
	int port;
    MacAddr macs[STATIC_MAC_ADDR_ENTRY_MAX_NUM];
    uint8 link = 0;
    int i = 0;
    
    PBMP_E_ITER(unit, port) {
        sal_memset(macs, 0, sizeof(macs));
        rv = _soc_ea_static_mac_entries_get(unit, 0, port, &entryCount, macs); 
        LOG_INFO(BSL_LS_BCM_L2TABLE,
                 (BSL_META_U(unit,
                             "_tk371x_clear_all_staticmacs: port=%d, entryCount=%d\n"), port, entryCount));
        if (rv == OK) {
        	for (i = 0; i < entryCount; i++){
        		_soc_ea_static_mac_entry_del(unit, link, port, &macs[i]);
            }
        }
    }
    PBMP_E_ITER(unit, port) {
        sal_memset(macs, 0, sizeof(macs));
        rv = _soc_ea_static_mac_entries_get(unit, 0, port, &entryCount, macs); 
        LOG_INFO(BSL_LS_BCM_L2TABLE,
                 (BSL_META_U(unit,
                             "port=%d, entryCount=%d\n"), port, entryCount));
        if ((rv == ERROR) || (entryCount > 0)) {
        	return BCM_E_FAIL;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_add
 * Description:
 *	Add a MAC address to the Switch Address Resolution Logic (ARL)
 *	port with the given VLAN ID and parameters.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 * Notes:
 *	Use CMIC_PORT(unit) to associate the entry with the CPU.
 *	Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 */
int
bcm_tk371x_l2_addr_add(
	    int unit,
	    bcm_l2_addr_t *l2addr)
{
    int rv;
    int link;
    int port;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }   
    
    if (l2addr == NULL) {
        return BCM_E_PARAM;   
    }
       
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, l2addr->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_PON_PORT(unit, l2addr->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_E_PORT(unit, l2addr->port)) {
        link = 0;
        port = l2addr->port;
        
    } else {
        link = l2addr->port - SOC_PORT_MIN(unit, llid);
        port = 0;
    } 
    
    rv = _soc_ea_static_mac_entry_add(unit, link, 
                                      port, (MacAddr *)&l2addr->mac);
    
    if (rv != OK) {
        return BCM_E_FAIL;
    }
     
	return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_delete
 * Description:
 *	Remove an L2 address from the device's ARL
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	mac - MAC address to remove
 *	vid - associated VLAN ID
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_addr_delete(
	    int unit,
	    bcm_mac_t mac,
	    bcm_vlan_t vid)
{
	return bcm_tk371x_l2_addr_delete_by_mac(unit, mac, BCM_L2_DELETE_STATIC);
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_delete_by_mac
 * Description:
 *	Delete L2 entries associated with a MAC address.
 * Parameters:
 *	unit  - device unit
 *	mac   - MAC address
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	L2 aging and learning are disabled during this operation.
 */
int
bcm_tk371x_l2_addr_delete_by_mac(
	    int unit,
	    bcm_mac_t mac,
	    uint32 flags)
{
	int rv;
	bcm_l2_addr_t l2Addr;
	
	if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
	
	if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }
    
    if ((flags & BCM_L2_DELETE_STATIC) != BCM_L2_DELETE_STATIC) {
        return BCM_E_UNAVAIL;
    }

    rv = bcm_tk371x_l2_addr_get(unit, mac, 0, &l2Addr);
    
    if (rv == BCM_E_NOT_FOUND) {
        return BCM_E_NONE;
    } else if (rv != BCM_E_NONE) {
        return BCM_E_FAIL;
    }
         
    rv = _soc_ea_static_mac_entry_del(unit, 0, l2Addr.port, (MacAddr *)&mac[0]);    
    if (rv != OK) {
        return BCM_E_FAIL;
    }  
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_delete_by_mac_port
 * Description:
 *	Delete L2 entries associated with a MAC address and
 *	a destination module/port
 * Parameters:
 *	unit  - device unit
 *	mac   - MAC address
 *	mod   - module id
 *	port  - port
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	
 */
int
bcm_tk371x_l2_addr_delete_by_mac_port(
	    int unit,
	    bcm_mac_t mac,
	    bcm_module_t mod,
	    bcm_port_t port,
	    uint32 flags)
{
    int rv;
    int link;
    int real_port;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }

    if ((flags & BCM_L2_DELETE_STATIC) != BCM_L2_DELETE_STATIC) {
        return BCM_E_UNAVAIL;
    }
    
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_PON_PORT(unit, port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_E_PORT(unit, port)) {
        link = 0;
        real_port = port;
        
    } else {
        link = port - SOC_PORT_MIN(unit, llid);
        real_port = 0;
    } 
    
    rv = _soc_ea_static_mac_entry_del(unit, link, real_port, (MacAddr *)&mac[0]);
    if (rv != OK) {
        return BCM_E_FAIL;
    }
	return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_delete_by_port
 * Description:
 *	Remove all L2 (MAC) addresses associated with the port.
 * Parameters:
 *	unit  - EA PCI device unit number (driver internal).
 *	mod   - module id (or -1 for local unit)
 *	pbmp  - bitmap of ports to affect
 *	flags - BCM_L2_REMOVE_XXX
 * Returns:
 *	BCM_E_NONE		Success.
 *	BCM_E_INTERNAL		Chip access failure.
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *	ARL aging and learning on all ports is disabled during this
 *	operation.   If these weren't disabled, the hardware could
 *	shift around the contents of the ARL table during the remove
 *	operation, causing some entries that should be removed to remain
 *	in the table.
 */
int
bcm_tk371x_l2_addr_delete_by_port(
	    int unit,
	    bcm_module_t mod,
	    bcm_port_t port,
	    uint32 flags)
{
    int rv;
    int link;
    int real_port;
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_PON_PORT(unit, port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_E_PORT(unit, port)) {
        link = 0;
        real_port = port;
        
    } else {
        link = port - SOC_PORT_MIN(unit, llid);
        real_port = 0;
    } 
    
    if ((flags & BCM_L2_DELETE_STATIC) == BCM_L2_DELETE_STATIC) {
        rv = _soc_ea_mac_table_flush(unit, link, real_port, FlushStaticMac);
        if (rv != OK) {
            return BCM_E_FAIL;
        } 
    } else {
        rv = _soc_ea_mac_table_flush(unit, link, real_port, FlushStaticMac);
        if (rv != OK) {
            return BCM_E_FAIL;
        } 
        rv = _soc_ea_mac_table_flush(unit, link, real_port, FlushDynamicMac); 
        if (rv != OK) {
            return BCM_E_FAIL;
        } 
    }
	return BCM_E_NONE;
}

int 
bcm_tk371x_l2_addr_delete_by_trunk(
        int unit, 
        bcm_trunk_t tid, 
        uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_tk371x_l2_addr_delete_by_vlan(
        int unit, 
        bcm_vlan_t vid, 
        uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_tk371x_l2_addr_delete_by_vlan_port(
        int unit, 
        bcm_vlan_t vid, 
        bcm_module_t mod, 
        bcm_port_t port, 
        uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_tk371x_l2_addr_delete_by_vlan_trunk(
        int unit, 
        bcm_vlan_t vid, 
        bcm_trunk_t tid, 
        uint32 flags)
{
    return BCM_E_UNAVAIL;
}



/*
 * Temporarily stop L2 table from changing (learning, aging, CPU, etc)
 */

/*
 * Function:
 *	bcm_tk371x_l2_addr_freeze
 * Description:
 *	Temporarily quiesce ARL from all activity (learning, aging)
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_addr_freeze(int unit)
{
    int rv;
    int port;
    uint16 age = 0;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }  
    PBMP_E_ITER(unit, port) {
        rv = _soc_ea_mac_learning_set(unit, 0, port, ArlDisLearning);  
        if (rv != OK) {
            return BCM_E_FAIL;
        } 
        rv = _soc_ea_dyna_mac_tab_age_set(unit, 0, port, age);    
        if (rv != OK) {
            return BCM_E_FAIL;
        }    
    } 
    tk371x_l2_freezed[unit] = 1;
    return BCM_E_NONE;
}


static int
_bcm_tk371x_l2_mac_match(
        bcm_mac_t mac_addr,
        MacAddr  *macs,
        uint8     cnt)
{
    uint8 i;
    for (i = 0; i < cnt; i++) {
        if (sal_memcmp(mac_addr, &macs[i], sizeof(bcm_mac_t)) == 0) {
            return BCM_E_NONE; 
        }   
    }
    return BCM_E_FAIL;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_get
 * Description:
 *	Given a MAC address and VLAN ID, check if the entry is present
 *	in the L2 table, and if so, return all associated information.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	mac - input MAC address to search
 *	vid - input VLAN ID to search, no meaning for tk371x
 *	l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *	BCM_E_NONE		Success (l2addr filled in)
 *	BCM_E_PARAM		Illegal parameter (NULL pointer)
 *	BCM_E_INTERNAL		Chip access failure
 *	BCM_E_NOT_FOUND	Address not found (l2addr not filled in)
 */
int
bcm_tk371x_l2_addr_get(
	    int unit,
	    bcm_mac_t mac_addr,
	    bcm_vlan_t vid,
	    bcm_l2_addr_t *l2addr)
{
    int rv;
    int port;
    uint8 entryCount; 
    MacAddr macs[256];
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (l2addr == NULL) {
        return BCM_E_PARAM;
    }
    
    PBMP_E_ITER(unit, port) {  
        sal_memset(macs, 0, sizeof(macs)); 
        rv = _soc_ea_dyna_mac_entries_get(unit, 0, port, &entryCount, macs); 
        if (rv == OK) {
            if (_bcm_tk371x_l2_mac_match(mac_addr, macs ,entryCount) == 
                BCM_E_NONE) {
                bcm_l2_addr_t_init(l2addr, mac_addr, vid);
                l2addr->port = port;
                return BCM_E_NONE;
            }
        }
        sal_memset(macs, 0, sizeof(macs));
        rv = _soc_ea_static_mac_entries_get(unit, 0, port, &entryCount, macs); 
        if (rv == OK) {
            if (_bcm_tk371x_l2_mac_match(mac_addr, macs ,entryCount) == 
                BCM_E_NONE) {
                bcm_l2_addr_t_init(l2addr, mac_addr, vid);
                l2addr->port = port;
                l2addr->flags |= BCM_L2_STATIC;
                return BCM_E_NONE;
            }
        }
    }   
	return BCM_E_NOT_FOUND;
}
/*
 * Function:
 *	bcm_tk371x_l2_addr_register
 * Description:
 *	Register a callback routine that will be called whenever
 *	an entry is inserted into or deleted from the L2 address table.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	fn - Callback function of type bcm_l2_addr_callback_t.
 *	fn_data - Arbitrary value passed to callback along with messages
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_addr_register(
	    int unit,
	    bcm_l2_addr_callback_t callback,
	    void *userdata)
{
	return BCM_E_UNAVAIL;
}


/*
 * Function:
 *	bcm_tk371x_l2_addr_thaw
 * Description:
 *	Restore normal ARL activity.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_addr_thaw(int unit)
{
	int rv;
    int port;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    } 
    PBMP_E_ITER(unit, port) {
        rv = _soc_ea_mac_learning_set(unit, 0, port, ArlHwLearning);  
        if (rv != OK) {
            return BCM_E_FAIL;
        }   
    } 
    tk371x_l2_freezed[unit] = 0;
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tk371x_l2_addr_unregister
 * Description:
 *	Unregister a previously registered callback routine.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	fn - Same callback function used to register callback
 *	fn_data - Same arbitrary value used to register callback
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 * Notes:
 *	Both callback and userdata must match from original register call.
 */
int
bcm_tk371x_l2_addr_unregister(
	    int unit,
	    bcm_l2_addr_callback_t callback,
	    void *userdata)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *	bcm_tk371x_l2_age_timer_get
 * Description:
 *	Returns the current age timer value.
 *	The value is 0 if aging is not enabled.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	age_seconds - Place to store returned age timer value in seconds
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_age_timer_get(
	    int unit,
	    int *age_seconds)
{
    uint16 age;
    int rv;
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    rv = _soc_ea_dyna_mac_tab_age_get(unit, 0, SOC_PORT_MIN(unit, ge), &age);
    if (rv != OK) {
        return BCM_E_FAIL;
    }
    * age_seconds =  age / 114;
	return BCM_E_NONE;
}

/*
 * Set L2 table aging time
 */

/*
 * Function:
 *	bcm_tk371x_l2_age_timer_set
 * Description:
 *	Set the age timer for all blocks.
 *	Setting the value to 0 disables the age timer.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 *	age_seconds - Age timer value in seconds
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_tk371x_l2_age_timer_set(
	    int unit,
	    int age_seconds)
{
    int rv;
    int port;
    uint16 age = age_seconds * 114;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }
    PBMP_E_ITER(unit, port) {  
        rv = _soc_ea_dyna_mac_tab_age_set(unit, 0, port,  age);    
        if (rv != OK) {
            return BCM_E_FAIL;
        }
    }
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_l2_clear
 * Purpose:
 *  Clear the L2 layer
 * Parameters:
 *  unit  - BCM unit number
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_l2_clear(int unit)
{   
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }      
    return _tk371x_clear_all_staticmacs(unit);	
}
/*
 * Function:
 *  bcm_tk371x_l2_detach
 * Purpose:
 *  Clean up l2 bcm layer when unit is detached
 * Parameters:
 *  unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_tk371x_l2_detach(int unit)
{    
    int rv;
    int port;
    
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }
    
    PBMP_E_ITER(unit, port) {  
        rv = _soc_ea_mac_table_flush(unit, 0, port, FlushAllMac);    
        if (rv != OK) {
            return BCM_E_FAIL;
        }
    }
    tk371x_l2_detached[unit] = 1;
	return _tk371x_clear_all_staticmacs(unit);

}

/*
 * Function:
 *	bcm_tk371x_l2_init
 * Description:
 *	Perform required initializations to L2 table.
 * Parameters:
 *	unit - EA PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_tk371x_l2_init(int unit)
{
	if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;
    }
	tk371x_l2_detached[unit] = 0;
	return _tk371x_clear_all_staticmacs(unit);
}

/*
 * Function:
 *	bcm_tk371x_l2_key_dump
 * Purpose:
 *	Dump the key (VLAN+MAC) portion of a hardware-independent
 *	L2 address for debugging
 * Parameters:
 *	unit - Unit number
 *	pfx - String to print before output
 *	entry - Hardware-independent L2 entry to dump
 *	sfx - String to print after output
 */
int
bcm_tk371x_l2_key_dump(
	    int unit,
	    char *pfx,
	    bcm_l2_addr_t *entry,
	    char *sfx)
{
    LOG_CLI((BSL_META_U(unit,
                        "%s"), pfx));
    LOG_CLI((BSL_META_U(unit,
                        "%02x:%02x:%02x:%02x:%02x:%02x"),
             entry->mac[0],entry->mac[1],entry->mac[2],
             entry->mac[3],entry->mac[4],entry->mac[5]));
    LOG_CLI((BSL_META_U(unit,
                        "%s"), sfx));
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_l2_learn_limit_get
 * Description:
 *  Get the L2 MAC learning limit
 * Parameters:
 *  unit        device number
 *  limit       learn limit control info
 *              limit->flags - qualifiers bits and action bits
 *              limit->vlan - vlan identifier
 *              limit->port - port number
 *              limit->trunk - trunk identifier
 *              limit->limit - max number of learned entry, -1 for unlimit
 * Return:
 *  BCM_E_XXX
 */
int
bcm_tk371x_l2_learn_limit_get(
	    int unit,
	    bcm_l2_learn_limit_t *limit)
{
    int link;
    int real_port;
    int rv;
    int limit_size;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (limit == NULL) {
        return BCM_E_PARAM;   
    }
    
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, limit->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_PON_PORT(unit, limit->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_E_PORT(unit, limit->port)) {
        link = 0;
        real_port = limit->port;
    } else {
        link = limit->port - SOC_PORT_MIN(unit, llid);
        real_port = 0;
    } 
    
    rv = _soc_ea_dyna_mac_tab_size_get(unit, link, real_port, &limit_size);
    if (rv != OK) {
        return BCM_E_FAIL;
    }
    limit->limit = limit_size;
    limit->flags = BCM_L2_LEARN_LIMIT_PORT;
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_l2_learn_limit_set
 * Description:
 *  Set the L2 MAC learning limit
 * Parameters:
 *  unit        device number
 *  limit       learn limit control info
 *              limit->flags - qualifiers bits and action bits
 *              limit->vlan - vlan identifier
 *              limit->port - port number
 *              limit->trunk - trunk identifier
 *              limit->limit - max number of learned entry, -1 for unlimit
 * Return:
 *     BCM_E_XXX
 */
int
bcm_tk371x_l2_learn_limit_set(
	    int unit,
	    bcm_l2_learn_limit_t *limit)
{
    int link;
    int real_port;
    int rv;
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (tk371x_l2_freezed[unit]) {
        return BCM_E_FAIL;   
    }
    
    if (limit == NULL) {
        return BCM_E_PARAM;
    }
    
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, limit->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_PON_PORT(unit, limit->port)) {
        return BCM_E_PARAM;
    }
    
    if (IS_E_PORT(unit, limit->port)) {
        link = 0;
        real_port = limit->port;
    } else {
        link = limit->port - SOC_PORT_MIN(unit, llid);
        real_port = 0;
    } 
    
    rv = _soc_ea_dyna_mac_tab_size_set(unit, link, real_port, limit->limit);
    if (rv != OK) {
        return BCM_E_FAIL; 
    }
           
	return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_tk371x_l2_traverse
 * Description:
 *  To traverse the L2 table and call provided callback function with matched
 *  entry
 * Parameters:
 *  unit         device number
 *  trav_fn      User specified callback function
 *  user_data    User specified cookie
 * Return:
 *  BCM_E_XXX
 */
int
bcm_tk371x_l2_traverse(
	    int unit,
	    bcm_l2_traverse_cb trav_fn,
	    void *user_data)
{
    int rv;
    int port;
    int i;
    uint8 entryCount; 
    bcm_l2_addr_t addr;
    MacAddr macs[256];
    
    if (tk371x_l2_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (trav_fn == NULL) {
        return BCM_E_PARAM;   
    }
       
    PBMP_E_ITER(unit, port) {  
        sal_memset(macs, 0, sizeof(macs)); 
        rv = _soc_ea_dyna_mac_entries_get(unit, 0, port, &entryCount, macs); 
        if (rv == OK) {
            for (i = 0; i < entryCount; i++ ) {
                sal_memset(&addr, 0, sizeof(bcm_l2_addr_t));
                addr.port = port;
                sal_memcpy( &addr.mac[0], &macs[i], sizeof(bcm_mac_t)); 
                (void)(* trav_fn)(unit, &addr, user_data);               
            }
        }
        sal_memset(macs, 0, sizeof(macs));
        rv = _soc_ea_static_mac_entries_get(unit, 0, port, &entryCount, macs); 
        if (rv == OK) {
            for (i = 0; i < entryCount; i++ ) {
                sal_memset(&addr, 0, sizeof(bcm_l2_addr_t));
                addr.port = port;
                sal_memcpy(&addr.mac[0], &macs[i], sizeof(bcm_mac_t));
                addr.flags |= BCM_L2_STATIC;
                (void)(* trav_fn)(unit, &addr, user_data);               
            }
        }
    } 
    return BCM_E_NONE;
}

