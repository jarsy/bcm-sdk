/*
 * $Id: port.c,v 1.35 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     port.c
 * Purpose:
 *
 */

#include <shared/bsl.h>

#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm_int/tk371x_dispatch.h>
#include <bcm_int/ea/init.h>
#include <bcm_int/ea/port.h>
#include <bcm_int/ea/tk371x/port.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/onu.h>
#include <soc/ea/tk371x/ea_drv.h>

#define DRV_TK371X_PORT_STATUS_AUTONEG_ENABLE	2
#define DRV_TK371X_PORT_STATUS_AUTONEG_DISABLE	1


/*
 * Function:
 *  bcm_tk371x_port_loopback_get
 * Purpose:
 *  Recover the current loopback operation for the specified port.
 * Parameters:
 *  unit - EA tk371x Unit #.
 *  port - tk371x port #.
 *  loopback - (OUT) one of:
 *      BCM_PORT_LOOPBACK_NONE
 *      BCM_PORT_LOOPBACK_MAC
 *      BCM_PORT_LOOPBACK_PHY
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_loopback_get(
		int unit,
		bcm_port_t port,
		int *loopback)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortLoopback, loopback);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
    switch (*loopback){
		case socEaLoopbackPhy:
			*loopback = BCM_PORT_LOOPBACK_PHY;
			break;
		case socEaLoopbackMac:
			*loopback = BCM_PORT_LOOPBACK_MAC;
			break;
		case socEaLoopBackNone:
			*loopback = BCM_PORT_LOOPBACK_NONE;
			break;
		default:
			*loopback = 0;
			return BCM_E_FAIL;
    }
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_port_loopback_set
 * Purpose:
 *  Setting the speed for a given port
 * Parameters:
 *  unit - EA tk371x Unit #.
 *  port - EA tk371x port #.
 *  loopback - one of:
 *      BCM_PORT_LOOPBACK_NONE
 *      BCM_PORT_LOOPBACK_MAC
 *      BCM_PORT_LOOPBACK_PHY
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_loopback_set(
		int unit,
		bcm_port_t port,
		int loopback)
{
	int rv;
	bcm_port_loopback_t lp = (bcm_port_loopback_t)loopback;
	
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
        switch (lp){
    		case BCM_PORT_LOOPBACK_MAC:
    			loopback = socEaLoopbackMac;
    			break;
    		case BCM_PORT_LOOPBACK_PHY:
    			loopback = socEaLoopbackPhy;
    			break;
    		case BCM_PORT_LOOPBACK_NONE:
    			loopback = socEaLoopBackNone;
    			break;
    		default:
    			return BCM_E_PARAM;;
        }
    	rv = _soc_ea_port_info_set(unit, port, socEaPortLoopback, loopback);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
	return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_tk371x_port_pause_get
 * Purpose:
 *  Get the pause state for a given port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  pause_tx - (OUT) Boolean value
 *  pause_rx - (OUT) Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_pause_get(
		int unit,
		bcm_port_t port,
		int *pause_tx,
		int *pause_rx)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortPauseTx, pause_tx);
    	rv = _soc_ea_port_info_get(unit, port, socEaPortPauseRx, pause_rx);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}


/*
 * Function:
 *  bcm_tk371x_port_pause_set
 * Purpose:
 *  Set the pause state for a given port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  pause_tx - Boolean value
 *  pause_rx - Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Symmetric pause requires the two "pause" values to be the same.
 */
int
bcm_tk371x_port_pause_set(
		int unit,
		bcm_port_t port,
		int pause_tx,
		int pause_rx)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortPauseTx, pause_tx);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortPauseRx, pause_rx);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *      bcm_tk371x_port_medium_get
 * Description:
 *      Get the medium-specific configuration for a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 to get the config for
 *      config   - per-medium configuration
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_tk371x_port_medium_get(
		int unit,
		bcm_port_t port,
		bcm_port_medium_t *medium)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityMedium, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
    *medium = (bcm_port_medium_t)value;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_port_mdix_get
 * Description:
 *  Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - (Out) One of:
 *            BCM_PORT_MDIX_AUTO
 *          Enable auto-MDIX when autonegotiation is enabled
 *            BCM_PORT_MDIX_FORCE_AUTO
 *          Enable auto-MDIX always
 *      BCM_PORT_MDIX_NORMAL
 *          Disable auto-MDIX
 *      BCM_PORT_MDIX_XOVER
 *          Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *  BCM_E_UNAVAIL - feature unsupported by hardware
 *  BCM_E_XXX - other error
 */
int
bcm_tk371x_port_mdix_get(
		int unit,
		bcm_port_t port,
		bcm_port_mdix_t *midx)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortMdix, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
    *midx = (bcm_port_mdix_t)value;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_port_mdix_set
 * Description:
 *  Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - One of:
 *            BCM_PORT_MDIX_AUTO
 *          Enable auto-MDIX when autonegotiation is enabled
 *            BCM_PORT_MDIX_FORCE_AUTO
 *          Enable auto-MDIX always
 *      BCM_PORT_MDIX_NORMAL
 *          Disable auto-MDIX
 *      BCM_PORT_MDIX_XOVER
 *          Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *  BCM_E_UNAVAIL - feature unsupported by hardware
 *  BCM_E_XXX - other error
 */
int
bcm_tk371x_port_mdix_set(
		int unit,
		bcm_port_t port,
		bcm_port_mdix_t mdix)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortMdix, (int)mdix);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_mdix_status_get
 * Description:
 *  Get the current MDIX status on a port/PHY
 * Parameters:
 *  unit    - Device number
 *  port    - Port number
 *  status  - (OUT) One of:
 *            BCM_PORT_MDIX_STATUS_NORMAL
 *          Straight connection
 *            BCM_PORT_MDIX_STATUS_XOVER
 *          Crossover has been performed
 * Return Value:
 *  BCM_E_UNAVAIL - feature unsupported by hardware
 *  BCM_E_XXX - other error
 */
int
bcm_tk371x_port_mdix_status_get(
		int unit,
		bcm_port_t port,
		bcm_port_mdix_status_t *midx_status)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortMdixState, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
    *midx_status = (bcm_port_mdix_status_t)value;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_port_interface_get
 * Purpose:
 *  Getting the interface type of a port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  intf - (OUT) BCM_PORT_IF_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcm_tk371x_port_interface_get(
		int unit,
		bcm_port_t port,
		bcm_port_if_t *interface)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortInterface, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    *interface = (bcm_port_if_t)value;
    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_tk371x_port_interface_set
 * Purpose:
 *  Setting the interface type for a given port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  if - BCM_PORT_IF_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */
int
bcm_tk371x_port_interface_set(
		int unit,
		bcm_port_t port,
		bcm_port_if_t interface)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortInterface, (int)interface);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_master_get
 * Purpose:
 *  Getting the master status of the port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  ms - (OUT) BCM_PORT_MS_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */
int
bcm_tk371x_port_master_get(
		int unit,
		bcm_port_t port,
		int *master)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortPhyMaster, master);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_master_set
 * Purpose:
 *  Setting the master status for a given port
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  ms - BCM_PORT_MS_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Ignored if not supported on port.
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */
int
bcm_tk371x_port_master_set(
		int unit,
		bcm_port_t port,
		int master)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortPhyMaster, master);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *      bcm_tk371x_port_ability_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - tk371x Unit #.
 *      port - tk371x port #.
 *      ability_mask - (OUT) Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_tk371x_port_ability_advert_get(
		int unit,
		bcm_port_t port,
		bcm_port_ability_t *ability)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortInterface, &value);
    	ability->interface = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityEncap, &value);
    	ability->encap = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityLoopback, &value);
    	ability->loopback = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityMedium, &value);
    	ability->medium = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityPause, &value);
    	ability->pause = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedFullDuplex, &value);
    	ability->speed_full_duplex = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedHalfDuplex, &value);
    	ability->speed_half_duplex = value;
    	ability->flags = 0;
    	ability->eee = 0;
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_port_ability_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - tk371x Unit #.
 *      port - tk371x port #.
 *      ability_mask - Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */
int
bcm_tk371x_port_ability_advert_set(
		int unit,
		bcm_port_t port,
		bcm_port_ability_t *ability)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortInterface, ability->interface);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityEncap, ability->encap);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityLoopback, ability->loopback);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityMedium, ability->medium);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityPause, ability->pause);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilitySpeedFullDuplex, ability->speed_full_duplex);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilitySpeedHalfDuplex, ability->speed_half_duplex);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_ability_get
 * Purpose:
 *  Retrieve the local port abilities.
 * Parameters:
 *  unit - tk371x Unit #.
 *  port - tk371x port #.
 *  ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *      ability of the MAC/PHY.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_tk371x_port_ability_get(
		int unit,
		bcm_port_t port,
		bcm_port_abil_t *abil)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityInterface, &value);
    }else{
       	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    *abil = (bcm_port_abil_t)value;
    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_tk371x_port_ability_remote_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - tk371x Unit #.
 *      port - tk371x port #.
 *      ability_mask - (OUT) Remote advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_tk371x_port_ability_remote_get(
		int unit,
		bcm_port_t port,
		bcm_port_ability_t *ability)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortInterface, &value);
    	ability->interface = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityEncap, &value);
    	ability->encap = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityLoopback, &value);
    	ability->loopback = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityMedium, &value);
    	ability->medium = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityPause, &value);
    	ability->pause = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedFullDuplex, &value);
    	ability->speed_full_duplex = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedHalfDuplex, &value);
    	ability->speed_half_duplex = value;
    	ability->flags = 0;
    	ability->eee = 0;
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_port_ability_local_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit - Tk371x Unit #.
 *      port - tk371x port #.
 *      ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *              ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_tk371x_port_ability_local_get(
		int unit,
		bcm_port_t port,
		bcm_port_ability_t *ability)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortInterface, &value);
    	ability->interface = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityEncap, &value);
    	ability->encap = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityLoopback, &value);
    	ability->loopback = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityMedium, &value);
    	ability->medium = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityPause, &value);
    	ability->pause = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedFullDuplex, &value);
    	ability->speed_full_duplex = value;
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilitySpeedHalfDuplex, &value);
    	ability->speed_half_duplex = value;
    	ability->flags = 0;
    	ability->eee = 0;
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return BCM_E_FAIL;
    }

    return BCM_E_NONE;
}

int
bcm_tk371x_port_ability_local_set(
		int unit,
		bcm_port_t port,
		bcm_port_ability_t ability)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortInterface, ability.interface);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityEncap, ability.encap);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityLoopback, ability.loopback);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityMedium, ability.medium);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityPause, ability.pause);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilitySpeedFullDuplex, ability.speed_full_duplex);
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilitySpeedHalfDuplex, ability.speed_half_duplex);
    }else{
    	return BCM_E_UNAVAIL;
    }
    return rv;
}


/*
 * Function:
 *  bcm_tk371x_port_advert_get
 * Purpose:
 *  Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  ability_mask - (OUT) Local advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_advert_get(
		int unit,
		bcm_port_t port,
		bcm_port_abil_t *abilty)
{
	return bcm_tk371x_port_ability_get(unit, port, abilty);
}

/*
 * Function:
 *  bcm_tk371x_port_advert_remote_get
 * Purpose:
 *  Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  ability_mask - (OUT) Remote advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_tk371x_port_advert_remote_get(
		int unit,
		bcm_port_t port,
		bcm_port_abil_t *abil)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAbilityInterface, &value);
    }else{
       	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    *abil = (bcm_port_abil_t)value;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_port_advert_set
 * Purpose:
 *  Set the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  ability_mask - Local advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Does NOT restart autonegotiation.
 *  To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */
int
bcm_tk371x_port_advert_set(
		int unit,
		bcm_port_t port,
		bcm_port_abil_t abilty)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAbilityInterface, (int)abilty);
    }else{
       	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_port_autoneg_get
 * Purpose:
 *  Get the autonegotiation state of the port
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  autoneg - (OUT) Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int bcm_tk371x_port_autoneg_get(
	    int unit,
	    bcm_port_t port,
	    int *autoneg)
{
	int rv;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortAutoneg, autoneg);
    }else{
    	return BCM_E_UNAVAIL;
    }
	return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_autoneg_set
 * Purpose:
 *  Set the autonegotiation state for a given port
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  autoneg - Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_autoneg_set(
          int unit,
          bcm_port_t port,
          int autoneg)
{
	int rv;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortAutoneg, autoneg);
    }else{
    	return BCM_E_UNAVAIL;
    }
	return rv;
}

/* General routines on which most port routines are built */

/*
 * Function:
 *  bcm_tk371x_port_config_get
 * Purpose:
 *  Get port configuration of a device
 * Parameters:
 *  unit - EA Unit #.
 *  config - (OUT) Structure returning configuration
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_config_get(
		int unit,
		bcm_port_config_t *config)
{
	char pbuf[SOC_PBMP_FMT_LEN + 1];

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_tk371x_port_config_get()..\n")));
    if (0 == BCM_TK371X_UNIT_VALID(unit)){
    	return BCM_E_UNIT;
    }
    if (NULL == config){
    	return BCM_E_PARAM;
    }
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "before call soc marcos:\n")));
    sal_memset(pbuf, 0, sizeof(pbuf));
    SOC_PBMP_FMT(config->fe, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->fe=%s\n"), pbuf));
    SOC_PBMP_FMT(config->ge, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->ge=%s\n"), pbuf));
    SOC_PBMP_FMT(config->port, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->port=%s\n"), pbuf));
    SOC_PBMP_FMT(config->all, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->all=%s\n"), pbuf));
    SOC_PBMP_FMT(config->llid, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->llid=%s\n"), pbuf));
    SOC_PBMP_FMT(config->pon, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->pon=%s\n"), pbuf));
	config->fe      = PBMP_FE_ALL(unit);
    config->ge      = PBMP_GE_ALL(unit);
    config->port    = PBMP_PORT_ALL(unit);
    config->all     = PBMP_ALL(unit);
    config->llid	= PBMP_LLID_ALL(unit);
    config->pon		= PBMP_PON_ALL(unit);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "after call soc marcos:\n")));
    sal_memset(pbuf, 0, sizeof(pbuf));
    SOC_PBMP_FMT(config->fe, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->fe=%s\n"), pbuf));
    SOC_PBMP_FMT(config->ge, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->ge=%s\n"), pbuf));
    SOC_PBMP_FMT(config->port, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->port=%s\n"), pbuf));
    SOC_PBMP_FMT(config->all, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->all=%s\n"), pbuf));
    SOC_PBMP_FMT(config->llid, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->llid=%s\n"), pbuf));
    SOC_PBMP_FMT(config->pon, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "config->pon=%s\n"), pbuf));
    return BCM_E_NONE;
}

/*
 * Function
 *  bcm_tk371x_port_config_t_init
 * Purpose:
 *  Initialize a Port Configuration structure.
 * Parameters:
 * 	pconfig
 * Returns:
 *  BCM_E_XXX
 **/
void
bcm_tk371x_port_config_t_init(bcm_port_config_t *pconfig)
{
    if (pconfig != NULL){
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
}

/*
 * Function:
 *      bcm_tk371x_port_control_get
 * Description:
 *      Return the current value of the port feature identified by <type> parameter.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - Identifies the port feature to be controlled
 *      value - Value of the bit field in port table
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_port_control_get(
        int unit,
        bcm_port_t port,
        bcm_port_control_t type,
        int *value)
{
    int rv = BCM_E_INTERNAL;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_control_get(),"
                             " unit=%d, port=%d, type=%d\n"), unit, port, (int)type));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return value: BCM_E_PORT, Invaild PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    rv = _bcm_ea_port_control_get(unit, port, type, value);

    return rv;
}

/*
 * Function:
 *      bcm_tk371x_port_control_set
 * Description:
 *      Enable/Disable various features at the port level.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - Identifies the port feature to be controlled
 *      value - Value of the bit field in port table
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_port_control_set(
	    int unit,
	    bcm_port_t port,
	    bcm_port_control_t type,
	    int value)
{
	int rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_phy_control_get()..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	rv = _bcm_ea_port_control_set(unit, port, type, value);
	return rv;
}

/*
 * Function:
 *      bcm_port_duplex_get
 * Purpose:
 *      Get the port duplex settings
 * Parameters:
 *      unit - Tk371x EPON Unit #.
 *      port - Tk371x port #.
 *      duplex - (OUT) Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_tk371x_port_duplex_get(
	    int unit,
	    bcm_port_t port,
	    int *duplex)
{
	int rv;
	int drv_act_value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortDuplex, &drv_act_value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != SOC_E_NONE){
    	return rv;
    }
    *duplex = (drv_act_value== DRV_PORT_STATUS_DUPLEX_FULL) ?
                    BCM_PORT_DUPLEX_FULL : BCM_PORT_DUPLEX_HALF;
	return rv;
}

/*
 * Function:
 *      bcm_port_duplex_set
 * Purpose:
 *      Set the port duplex settings.
 * Parameters:
 *      unit - Tk371x EPON Unit #.
 *      port - Tk371x port #.
 *      duplex - Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Turns off autonegotiation.  Caller must make sure other forced
 *      parameters (such as speed) are set.
 */
int
bcm_tk371x_port_duplex_set(
	    int unit,
	    bcm_port_t port,
	    int duplex)
{
	int rv;
	int drv_act_value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	if (duplex == BCM_PORT_DUPLEX_FULL){
    		drv_act_value = DRV_PORT_STATUS_DUPLEX_FULL;
    	}else if(duplex == BCM_PORT_DUPLEX_HALF){
    		drv_act_value = DRV_PORT_STATUS_DUPLEX_HALF;
    	}else{
    		return BCM_E_PARAM;
    	}
    	rv = _soc_ea_port_info_set(unit, port, socEaPortDuplex, drv_act_value);
    }else{
    	return BCM_E_UNAVAIL;
    }
	return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_enable_get
 * Purpose:
 *  Gets the enable state as defined by bcm_port_enable_set()
 * Parameters:
 *  unit - EA unit #.
 *  port - EA port #.
 *  enable - (OUT) TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *  BCM_E_XXXX
 * Notes:
 *  The PHY enable holds the port enable state set by the user.
 *  The MAC enable transitions up and down automatically via linkscan
 *  even if user port enable is always up.
 */
int
bcm_tk371x_port_enable_get(
	    int unit,
	    bcm_port_t port,
	    int *enable)
{
    int         rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_enable_get..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    rv = _bcm_ea_port_enable_get(unit, port, enable);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_enable_get: u=%d p=%d rv=%d enable=%d\n"),
              unit, port, rv, *enable));
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_enable_set
 * Purpose:
 *  Physically enable/disable the MAC/PHY on this port.
 * Parameters:
 *  unit - EA unit #.
 *  port - EA port #.
 *  enable - TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *  BCM_E_XXXX
 * Notes:
 *  If linkscan is running, it also controls the MAC enable state.
 */
int
bcm_tk371x_port_enable_set(
	    int unit,
	    bcm_port_t port,
	    int enable)
{
    int         rv = BCM_E_NONE ;
    int 		state;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_enable_set.., unit=%d\n"), unit));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	/* check the enable param is valid?*/
    if ((enable == 0) || (enable == 1)){
    	state = enable;
    }
    else{
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PARAM, invalid value : %d\n"), (enable)));
    	return BCM_E_PARAM;
    }
    rv = _bcm_ea_port_enable_set(unit, port, state);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "_bcm_ea_port_enable_set:unit=%d port=%d rv=%d enable=%d\n"),
              unit, port, rv, enable));
    return rv;
}


/*
 * Function:
 *  bcm_tk371x_port_frame_max_get
 * Description:
 *  Set the maximum receive frame size for the port
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  size - Maximum frame size in bytes
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 *      Depending on chip or port type the actual maximum receive frame size
 *      might be slightly higher.
 *
 *      For GE ports that use 2 separate MACs (one for GE and another one for
 *      10/100 modes) the function returns the maximum rx frame size set for
 *      the current mode.
 */
int
bcm_tk371x_port_frame_max_get(
	    int unit,
	    bcm_port_t port,
	    int *size)
{
	int rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_frame_max_get.., unit=%d\n"), unit));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	if (TK371X_PON_PORT_VALID(port)){
	    rv = _bcm_ea_port_frame_max_get(unit, port, size);
	}else if (TK371X_UNI_PORT_VALID(port)){
	    rv = _bcm_ea_port_frame_max_get(unit, port, size);
	}else{
		return BCM_E_UNAVAIL;
	}
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "_bcm_ea_port_frame_max_get:unit=%d port=%d rv=%d size=%d\n"),
              unit, port, rv,*size));
	return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_frame_max_set
 * Description:
 *  Set the maximum receive frame size for the port
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  size - Maximum frame size in bytes
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 *      Depending on chip or port type the actual maximum receive frame size
 *      might be slightly higher.
 *
 *      It looks like this operation is performed the same way on all the chips
 *      and the only depends on the port type.
 */
int
bcm_tk371x_port_frame_max_set(
	    int unit,
	    bcm_port_t port,
	    int size)
{
	int rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_frame_max_set.., unit=%d\n"), unit));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

	if (TK371X_PON_PORT_VALID(port)){
		rv = _bcm_ea_port_frame_max_set(unit, port, size);

	}else if (TK371X_UNI_PORT_VALID(port)){
		rv = _bcm_ea_port_frame_max_set(unit, port, size);
	}else{
		return BCM_E_UNAVAIL;
	}
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_frame_max_set:unit=%d port=%d rv=%d size=%d\n"),
                  unit, port, rv, size));
	return rv;
}

/*
 * Each field in the bcm_port_info_t structure has a corresponding
 * mask bit to control whether to get or set that value during the
 * execution of the bcm_port_selective_get/_set functions.
 * The OR of all requested masks should be stored in the action_mask
 * field of the bcm_port_info_t before calling the functions.
 */


/*
 * Routines to set or get port parameters in one call.
 *
 * The "action mask" indicates which values should be set/gotten.
 */


/*
 * Function:
 *  bcm_tk371x_port_info_get
 * Purpose:
 *  Get all information on the port
 * Parameters:
 *  unit - EA unit #
 *  port - EA port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_info_get(
	    int unit,
	    bcm_port_t port,
	    bcm_port_info_t *info)
{
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		return BCM_E_PORT;
	}
    info->action_mask = 0;
    info->action_mask |= BCM_PORT_ATTR_ENABLE_MASK;
    info->action_mask |= BCM_PORT_ATTR_LINKSTAT_MASK;
    info->action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
    info->action_mask |= BCM_PORT_ATTR_SPEED_MASK;
    info->action_mask |= BCM_PORT_ATTR_SPEED_MAX_MASK;
    info->action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    info->action_mask |= BCM_PORT_ATTR_LEARN_MASK;
    info->action_mask |= BCM_PORT_ATTR_LOOPBACK_MASK;
    info->action_mask |= BCM_PORT_ATTR_INTERFACE_MASK;
    info->action_mask |= BCM_PORT_ATTR_PAUSE_TX_MASK;
    info->action_mask |= BCM_PORT_ATTR_PAUSE_RX_MASK;
    info->action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_ABILITY_MASK;
    info->action_mask |= BCM_PORT_ATTR_REMOTE_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_FRAME_MAX_MASK;
    info->action_mask |= BCM_PORT_ATTR_MEDIUM_MASK;
    info->action_mask2 = 0;
    info->action_mask2 |= BCM_PORT_ATTR2_PORT_ABILITY;

    return _bcm_tk371x_port_selective_get(unit, port, info);
}

/*
 * Function:
 *  bcm_tk371x_port_info_restore
 * Purpose:
 *  Restore port settings saved by info_save
 * Parameters:
 *  unit - EA unit #
 *  port - EA port #
 *  info - Pointer to structure with info from port_info_save
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  bcm_port_info_save has done all the work.
 *  We just call port_selective_set.
 */

/*
 * Function:
 *  bcm_tk371x_port_info_save
 * Purpose:
 *  Save the current settings of a port
 * Parameters:
 *  unit - EA unit #
 *  port - EA port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  The action_mask will be adjusted so that the
 *  proper values will be set when a restore is made.
 *  This mask should not be altered between these calls.
 */
int
bcm_tk371x_port_info_save(
	    int unit,
	    bcm_port_t port,
	    bcm_port_info_t *info)
{
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_info_save..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "UNIT : %d\n"), (unit)));
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    info->action_mask = BCM_PORT_ATTR_ALL_MASK;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_info_save()..\n")));
    BCM_IF_ERROR_RETURN(_bcm_tk371x_port_selective_get(unit, port, info));

    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_tk371x_port_info_set
 * Purpose:
 *  Set all information on the port
 * Parameters:
 *  unit - EA unit #
 *  port - EA port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  Checks if AN is on, and if so, clears the
 *  proper bits in the action mask.
 */
int
bcm_tk371x_port_info_set(
	    int unit,
	    bcm_port_t port,
	    bcm_port_info_t *info)
{
	int rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_info_set..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "UNIT : %d\n"), (unit)));
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    rv = bcm_tk371x_port_info_restore(unit, port, info);

	return rv;
}

/* Initialize a Port Configuration structure. */
/*
 * Function:
 *  bcm_tk371x_port_info_t_init
 * Purpose:
 *  Set all information on the port
 * Parameters:
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  Checks if AN is on, and if so, clears the
 *  proper bits in the action mask.
 */
void
bcm_tk371x_port_info_t_init(bcm_port_info_t *info)
{
	if (NULL != info){
		sal_memset(info, 0, sizeof(*info));
	}
}

/* Module one-time initialization routine */

/*
 * Function:
 *      bcm_tk371x_port_init
 * Purpose:
 *      Initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *      unit - EA unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_MEMORY - failed to allocate required memory.
 * Notes:
 *      All ports set in disabled state.
 *      Default PVID initialized.
 */
int
bcm_tk371x_port_init(int unit)
{
	int rv = BCM_E_NONE;

	if (0 == BCM_TK371X_UNIT_VALID(unit)){
		return BCM_E_UNIT;
	}
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_tk371x_port_init, unit = %d\n"), unit));
	rv = _bcm_tk371x_port_detach(unit);

	return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_learn_get
 * Purpose:
 *  Get the ARL hardware learning options for this port.
 *  This defines what the hardware will do when a packet
 *  is seen with an unknown address.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  flags - (OUT) Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_learn_get(
		int unit,
	    bcm_port_t port,
	    uint32 *flags)
{
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_learn_get..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

	return _bcm_ea_port_learn_get(unit, port, flags);
}

/*
 * Function:
 *  bcm_tk371x_port_learn_modify
 * Purpose:
 *  Modify the port learn flags, adding add and removing remove flags.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  add  - Flags to set.
 *      remove - Flags to clear.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_learn_modify(
	    int unit,
	    bcm_port_t port,
	    uint32 add,
	    uint32 remove)
{
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_learn_modify..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	return _bcm_ea_port_learn_modify(unit, port, add, remove);
}

/*
 * Function:
 *  bcm_tk371x_port_learn_set
 * Purpose:
 *  Set the ARL hardware learning options for this port.
 *  This defines what the hardware will do when a packet
 *  is seen with an unknown address.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  flags - Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_learn_set(
	    int unit,
	    bcm_port_t port,
	    uint32 flags)
{
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_learn_set..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

	switch (flags){
		case 0:
		case BCM_PORT_LEARN_ARL:
		case BCM_PORT_LEARN_FWD:
		case (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD):
			break;
		default:
			return BCM_E_PARAM;
	}
	return _bcm_ea_port_learn_set(unit, port, flags);
}

/*
 * Function:
 *  bcm_tk371x_port_link_status_get
 * Purpose:
 *  Return current Link up/down status, queries linkscan, if unable to
 *  retrieve status queries the PHY.
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  up - (OUT) Boolean value, FALSE for link down and TRUE for link up
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_link_status_get(
	    int unit,
	    bcm_port_t port,
	    int *status)
{
	int rv = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_link_status_get..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		return BCM_E_PORT;
	}

	rv = _bcm_ea_port_link_status_get(unit, port, status);

	return rv;
}

/*
 * Function:
 *     bcm_tk371x_port_phy_control_get
 * Description:
 *     Set PHY specific properties
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_tk371x_port_phy_control_get(
	    int unit,
	    bcm_port_t port,
	    bcm_port_phy_control_t type,
	    uint32 *value)
{
    int rv = BCM_E_INTERNAL;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_phy_control_get..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "UNIT : %d\n"), (unit)));
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    rv = _bcm_ea_port_phy_control_get(unit, port, type, value);

    return rv;
}

/*
 * Function:
 *     bcm_tk371x_port_phy_control_set
 * Description:
 *     Set PHY specific properties
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       new value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_tk371x_port_phy_control_set(
	    int unit,
	    bcm_port_t port,
	    bcm_port_phy_control_t type,
	    uint32 value)
{
    int rv = BCM_E_INTERNAL;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_phy_control_set..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "UNIT : %d\n"), (unit)));
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    rv = _bcm_ea_port_phy_control_set(unit, port, type, value);
    return rv;
}

/*
 * Function:
 *  bcm_tk371x_port_speed_get
 * Purpose:
 *  Getting the speed of the port
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  If port is in MAC loopback, the speed of the loopback is returned.
 *  On chips with a PHYMOD field:
 *      If PHYMOD=0, the speed is hardwired to 100Mb/s.
 *      If PHYMOD=1, the speed is obtained directly from the PHY.
 *  In either case, FE_SUPP.SPEED is completely ignored.
 */
int
bcm_tk371x_port_speed_get(
	    int unit,
	    bcm_port_t port,
	    int *speed)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port, socEaPortSpeed, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    switch (value){
    	case DRV_PORT_STATUS_SPEED_10M:
    		*speed = 10;
    		break;
    	case DRV_PORT_STATUS_SPEED_100M:
    		*speed = 100;
    		break;
    	case DRV_PORT_STATUS_SPEED_1G:
    		*speed = 1000;
    		break;
    	case DRV_PORT_STATUS_SPEED_2500M:
    		*speed = 2500;
    		break;
    	default:
    		*speed = 0;
    		return BCM_E_INTERNAL;
    }
	return BCM_E_NONE;
}


int bcm_tk371x_port_speed_max(
		int unit,
		bcm_port_t port,
		int *speed)
{
	int rv;
	int value;
    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }

    if (0 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_get(unit, port,socEaPortAbilitySpeedFullDuplex, &value);
    }else{
    	return BCM_E_UNAVAIL;
    }
    if (rv != BCM_E_NONE){
    	return rv;
    }
    switch (value){
    	case DRV_PORT_STATUS_SPEED_10M:
    		*speed = 10;
    		break;
    	case DRV_PORT_STATUS_SPEED_100M:
    		*speed = 100;
    		break;
    	case DRV_PORT_STATUS_SPEED_1G:
    		*speed = 1000;
    		break;
    	case DRV_PORT_STATUS_SPEED_2500M:
    		*speed = 2500;
    		break;
    	default:
    		*speed = 0;
    		return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *  bcm_tk371x_port_speed_set
 * Purpose:
 *  Getting the maximum speed of the port
 * Parameters:
 *  unit - EA Unit #.
 *  port - EA port #.
 *  speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_tk371x_port_speed_set(
	    int unit,
	    bcm_port_t port,
	    int speed)
{
	int rv;
	int value;

    if (0 == BCM_TK371X_UNIT_VALID((unit))){
    	return BCM_E_UNIT;
    }
    if (0 == TK371X_PORT_VALID( port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Return value: BCM_E_PORT: invalid PORT : %d\n"), (port)));
        return BCM_E_PORT;
    }
    switch (speed){
		case 10:
			value = DRV_PORT_STATUS_SPEED_10M;
			break;
		case 100:
			value = DRV_PORT_STATUS_SPEED_100M;
			break;
		case 1000:
			value = DRV_PORT_STATUS_SPEED_1G;
			break;
		case 2500:
			value = DRV_PORT_STATUS_SPEED_2500M;
			break;
		default:
			return BCM_E_PARAM;
    }
    if (1 == TK371X_UNI_PORT_VALID(port)){
    	rv = _soc_ea_port_info_set(unit, port, socEaPortSpeed, value);
    }else{
    	return BCM_E_UNAVAIL;
    }
	return rv;
}

int bcm_tk371x_port_probe(
		int unit,
		bcm_pbmp_t ibp,
		bcm_pbmp_t *obp)
{
	bcm_port_t port;
	int okay = 0;

	char pbuf[100];
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "ibp:\n")));
    sal_memset(pbuf, 0, sizeof(pbuf));
    SOC_PBMP_FMT(ibp, pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "ibp=%s\n"), pbuf));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_probe()..\n")));
	BCM_PBMP_CLEAR(*obp);

	PBMP_ITER(ibp, port){
		/* do port_probe process */
		bcm_tk371x_port_link_status_get(unit, port, &okay);
		if (okay){
			BCM_PBMP_PORT_ADD(*obp, port);
		}
	}
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "obp:\n")));
    sal_memset(pbuf, 0, sizeof(pbuf));
    SOC_PBMP_FMT((*obp), pbuf);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "obp=%s\n"), pbuf));
	return BCM_E_NONE;
}

int
bcm_tk371x_port_flood_block_get(
		int unit,
		bcm_port_t iport,
		bcm_port_t oport,
		uint32 *value)
{
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_port_flood_block_set(
		int unit,
		bcm_port_t iport,
		bcm_port_t oport,
		uint32 value)
{
	return BCM_E_UNAVAIL;
}


int
bcm_tk371x_port_info_restore(
	    int unit,
	    bcm_port_t port,
	    bcm_port_info_t *info)
{
    uint32      mask = 0;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_tk371x_port_info_restore..\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENABLE_MASK){
    	bcm_tk371x_port_enable_set(unit, port, info->enable);
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK){
    	bcm_tk371x_port_autoneg_set(unit, port, info->autoneg);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK){
    	bcm_tk371x_port_speed_set(unit, port, info->speed);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK){
    	bcm_tk371x_port_learn_set(unit, port, info->learn);
    }

    /*
     * Set loopback mode before setting the speed/duplex, since it may
     * affect the allowable values for speed/duplex.
     */
    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK){
		bcm_tk371x_port_loopback_set(unit, port, info->loopback);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK){
    	bcm_tk371x_port_interface_set(unit, port, info->interface);
    }

    if ((mask & BCM_PORT_ATTR_PAUSE_TX_MASK) &&
    		(mask & BCM_PORT_ATTR_PAUSE_RX_MASK)){
    	bcm_tk371x_port_pause_set(unit, port, info->pause_tx, info->pause_tx);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK){
    	bcm_tk371x_port_advert_set(unit, port, info->local_advert);
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK){
        bcm_tk371x_port_duplex_set(unit, port, info->duplex);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        bcm_tk371x_port_frame_max_set(unit, port, info->frame_max);
    }

	return BCM_E_NONE;
}



int
_bcm_tk371x_port_selective_get(
		int unit,
		bcm_port_t port,
		bcm_port_info_t *info)
{
    uint32      mask = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : _bcm_tk371x_port_selective_get()..\n")));
    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENABLE_MASK){
    	bcm_tk371x_port_enable_get(unit, port, &info->enable);
    }

    if (mask & BCM_PORT_ATTR_LINKSTAT_MASK){
    	bcm_tk371x_port_link_status_get(unit, port, &info->linkstatus);
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK){
    	bcm_tk371x_port_autoneg_get(unit, port, &info->autoneg);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK){
    	bcm_tk371x_port_speed_get(unit, port, &info->speed);
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK){
    	bcm_tk371x_port_duplex_get(unit, port, &info->duplex);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK){
    	bcm_tk371x_port_learn_get(unit, port, &info->learn);
    }

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK){
    	bcm_tk371x_port_loopback_get(unit, port, &info->loopback);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK){
    	bcm_tk371x_port_interface_get(unit, port, &info->interface);
    }

    if ((mask & BCM_PORT_ATTR_PAUSE_TX_MASK) &&
    		(mask & BCM_PORT_ATTR_PAUSE_RX_MASK)){
    	bcm_tk371x_port_pause_get(unit, port, &info->pause_tx, &info->pause_rx);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK){
    	bcm_tk371x_port_advert_get(unit, port, &info->local_advert);
    }

    if (mask & BCM_PORT_ATTR2_PORT_ABILITY){
    	bcm_tk371x_port_ability_local_get(unit, port, &info->local_ability);
    }

    if (mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK){
    	bcm_tk371x_port_advert_remote_get(unit, port, &info->remote_advert);
    }

    if ((mask & BCM_PORT_ATTR2_PORT_ABILITY) &&
    		(mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK)){
    	bcm_tk371x_port_ability_remote_get(unit, port, &info->remote_ability);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MAX){
    	bcm_tk371x_port_speed_max(unit, port, &info->speed_max);
    }

    if (mask & BCM_PORT_ATTR_ABILITY_MASK){
    	bcm_tk371x_port_ability_get(unit, port, &info->ability);
    }

    if ((mask & BCM_PORT_ATTR_ABILITY_MASK) &&
    		(mask & BCM_PORT_ATTR2_PORT_ABILITY)){
    	bcm_tk371x_port_ability_local_get(unit, port, &info->local_ability);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK){
    	bcm_tk371x_port_frame_max_get(unit, port, &info->frame_max);
    }

    if (mask & BCM_PORT_ATTR_MEDIUM_MASK){
    	bcm_tk371x_port_medium_get(unit, port, &info->medium);
    }
    return BCM_E_NONE;
}

int bcm_tk371x_port_pon_info_get(
		int unit,
		bcm_port_t port,
		bcm_port_pon_info_t *pon)
{
	int rv = BCM_E_NONE;
	TkEponStatus pon_state;
	OamEponPortInfo pon_port_info;
	int i = 0, llid;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_tk371x_port_pon_info_get...\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	if (1 == TK371X_UNI_PORT_VALID(port)){
		return BCM_E_UNAVAIL;
	}
	rv = TkExtOamGetMLLIDLinkStatus(unit, 0, &pon_state);
	if (rv != OK){
		return BCM_E_FAIL;
	}
	rv = TkExtOamGetEponPortInfo(unit, 0, &pon_port_info);
	if (rv != OK){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_soc_ea_pon_reg_status_get Return BCM_E_INTERNAL, invalid value=%d\n"), rv));
		return BCM_E_FAIL;
	}
	if (1 == TK371X_PON_PORT_VALID(port)){
		pon->enable = pon_state.connection;
		pon->linkstatus = pon_port_info.EponLosState;
		pon->llid = 0;
		pon->loopback = 0;
		pon->oam_discovery_status = 0;
		sal_memcpy((void*)(pon->olt_mac_addr),
				(void*)pon_state.Olt_MAC_addr, sizeof(bcm_mac_t));
	}
	if (1 == TK371X_LLID_PORT_VALID(port)){
		llid = port - 2;
		if (llid > pon_state.linkNum){
			return BCM_E_INTERNAL;
		}
		pon->enable = pon_state.connection;
		pon->linkstatus = pon_port_info.LinkInfo[llid - 1].RegState - 1;
		pon->llid = pon_state.linkInfo[llid - 1].linkLlid;
		pon->oam_discovery_status = pon_state.linkInfo[llid - 1].oamLinkEstablished;
		pon->loopback = pon_state.linkInfo[llid - 1].loopBack;
		sal_memcpy((void*)(pon->olt_mac_addr),
				(void*)pon_state.Olt_MAC_addr, sizeof(bcm_mac_t));
	}
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "_soc_ea_pon_reg_status_get:unit=%d port=%d rv=%d\n"),
              unit, port, rv));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "pon->enable=%d\n"), pon->enable));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "pon->linkstatus=%d\n"), pon->linkstatus));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "pon->llid=%d\n"), pon->llid));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "pon->loopback=%d\n"), pon->loopback));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "pon->oam_discovery_status=%d\n"), pon->oam_discovery_status));
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "pon->olt_mac_addr=")));
	for (i = 0; i < sizeof(bcm_mac_t); i++){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%02X"), pon->olt_mac_addr[i]));
	}
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "\n")));
	return BCM_E_NONE;;
}

int bcm_tk371x_port_pon_info_set(
		int unit,
		bcm_port_t port,
		bcm_port_pon_info_t *pon)
{
	int rv = BCM_E_NONE;
	int pon_state, pon_loopback;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_tk371x_port_pon_info_set...\n")));
	if (0 == BCM_TK371X_UNIT_VALID((unit))){
		return BCM_E_UNIT;
	}
	if (0 == TK371X_PORT_VALID( port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_PORT, invalid PORT : %d\n"), (port)));
		return BCM_E_PORT;
	}
	if (1 == TK371X_UNI_PORT_VALID(port)){
		return BCM_E_UNAVAIL;
	}
	if (1 == TK371X_PON_PORT_VALID(port)){ 
		if (pon->enable != 0 && pon->enable != 1){
			LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "Return BCM_E_PARAM, invalid value is %d\n"), pon->enable));
			return BCM_E_PARAM;
		}

		pon_state = (pon->enable == 1) ? 0 : 1;
		rv = TkExtOamSetEponAdmin(unit, port, pon_state);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "TkExtOamSetEponAdmin:unit=%d port=%d rv=%d, pon->enable=%d\n"),
                          unit, port, rv, pon->enable));
	    if (rv != OK){
	    	return BCM_E_FAIL;
	    }
	}
	if (1 == TK371X_LLID_PORT_VALID(port)){
		switch(pon->loopback){
			case 0:
				pon_loopback = socEaLoopBackNone;
				break;
			case 1:
				pon_loopback = socEaLoopback8023ah;
				break;
			default:
				return BCM_E_PARAM;
		}
		rv = _soc_ea_port_info_set(unit, port, socEaPortLoopback, pon_loopback);
		if (rv != BCM_E_NONE){
			return rv;
		}
	}
	return BCM_E_NONE;
}


int _bcm_tk371x_port_detach(int unit)
{
	return _bcm_ea_port_detach(unit);
}
