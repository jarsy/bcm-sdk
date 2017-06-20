/*
 * $Id: stat.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom RoboSwitch SNMP Statistics API.
 */

#include <shared/bsl.h>

#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/counter.h>
#include <soc/ll.h>
#include <soc/robo_stat.h>

#include <bcm/port.h>
#include <bcm/error.h>

#include <bcm/stat.h>

#define COUNTER_FLAGS_DEFAULT	0

/*
 * Function:
 *	_bcm_robo_stat_detach
 * Description:
 *	De-initializes the BCM stat module.
 * Parameters:
 *	unit -  (IN) BCM device number.
 * Returns:
 *	BCM_E_XXX
 */

int
_bcm_robo_stat_detach(int unit)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : _bcm_robo_stat_detach()..\n")));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_stat_gport_validate
 * Description:
 *      Helper funtion to validate port/gport parameter 
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port_in  - (IN) Port / Gport to validate
 *      port_out - (OUT) Port number if valid. 
 * Return Value:
 *      BCM_E_NONE - Port OK 
 *      BCM_E_INIT - Not initialized
 *      BCM_E_PORT - Port Invalid
 */

STATIC int 
_bcm_robo_stat_gport_validate(int unit, bcm_port_t port_in, bcm_port_t *port_out)
{
    if (BCM_GPORT_IS_SET(port_in)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port_in, port_out));
    } else if (SOC_PORT_VALID(unit, port_in)) { 
        *port_out = port_in;
    } else {
        return BCM_E_PORT; 
    }

    return BCM_E_NONE;

}

/*
 * Function:
 *	bcm_robo_stat_init
 * Description:
 *	Initializes the BCM stat module.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_INTERNAL - Chip access failure.
 */
int bcm_robo_stat_init(int unit)
{
    pbmp_t          pbmp;
    sal_usecs_t     interval;
    uint32          flags;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_init()..\n")));
    if (soc_property_get_str(unit, spn_BCM_STAT_PBMP) == NULL) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        pbmp = soc_property_get_pbmp(unit, spn_BCM_STAT_PBMP, 0);
    }
    interval = soc_property_get(unit, spn_BCM_STAT_INTERVAL,
                                SOC_COUNTER_INTERVAL_DEFAULT);
    flags = soc_property_get(unit, spn_BCM_STAT_FLAGS, COUNTER_FLAGS_DEFAULT);

    SOC_PBMP_CLEAR(pbmp);
    BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));

    BCM_IF_ERROR_RETURN(DRV_COUNTER_RESET(unit));
    
    BCM_IF_ERROR_RETURN(DRV_COUNTER_THREAD_SET
                (unit, DRV_COUNTER_THREAD_START, flags, 
                interval, pbmp));

    return BCM_E_NONE;
}				


/*
 * Function:
 *	bcm_robo_stat_clear
 * Description:
 *	Clear the port based statistics from the RoboSwitch port.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_INTERNAL - Chip access failure.
 */
int bcm_robo_stat_clear(int unit, bcm_port_t port)
{
    pbmp_t	pbm;
    uint64      val64;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_stat_gport_validate(unit, port, &port));

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_clear()..\n")));

    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    
    /* Check if the chip support per port MIB clear function first */
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(
        DRV_PORT_SET(unit, pbm, DRV_PORT_PROP_MIB_CLEAR, TRUE));
    
    /*
     * Force one time synchronization before clear counters in case that
     * the content of sw and hw counter tables not the same.
     */
    BCM_IF_ERROR_RETURN(bcm_stat_sync(unit));

    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);

    COMPILER_64_ZERO(val64);
    BCM_IF_ERROR_RETURN(DRV_COUNTER_SET
                (unit, pbm, 
                DRV_SNMP_VAL_COUNT, val64));

    return(BCM_E_NONE);
}				
	
/*
 * Function:
 *	bcm_robo_stat_sync
 * Description:
 *	Synchronize software counters with hardware
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *	Makes sure all counter hardware activity prior to the call to
 *	bcm_stat_sync is reflected in all bcm_stat_get calls that come
 *	after the call to bcm_stat_sync.
 */

int bcm_robo_stat_sync(int unit)
{
    pbmp_t null_pbmp;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_sync()..\n")));

    SOC_PBMP_CLEAR(null_pbmp);
    BCM_IF_ERROR_RETURN(DRV_COUNTER_THREAD_SET
                (unit, DRV_COUNTER_THREAD_SYNC, 0, 0, null_pbmp));

    return BCM_E_NONE;
}				

/*
 * Function:
 *	bcm_robo_stat_get
 * Description:
 *	Get the specified statistic from the RoboSwitch
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 *      val - (OUT) 64-bit counter value.
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_BADID - Illegal port number.
 *	BCM_E_INTERNAL - Chip access failure.
 *	BCM_E_UNAVAIL - Counter/variable is not implemented
 *				on this current chip.
 * Notes:
 *	Some counters are implemented on a given port only when it is
 *	operating in a specific mode, for example, 10 or 100, and not 
 *	1000. If the counter is not implemented on a given port, OR, 
 *	on the port given its current operating mode, BCM_E_UNAVAIL
 *	is returned.
 */
int bcm_robo_stat_get(int unit, bcm_port_t port, 
			bcm_stat_val_t type, uint64 *value)
{
    int sync_hw = FALSE; /* Read software accumulated counters */

    BCM_IF_ERROR_RETURN(
        _bcm_robo_stat_gport_validate(unit, port, &port));

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_get()..\n")));
    BCM_IF_ERROR_RETURN(DRV_COUNTER_GET
        (unit, port, type, sync_hw, value));

    return BCM_E_NONE;
}				
			
/*
 * Function:
 *	bcm_robo_stat_get32
 * Description:
 *	Get the specified statistic from the RoboSwitch
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 *      val - (OUT) 32-bit counter value.
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_BADID - Illegal port number.
 *	BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *	Same as bcm_stat_get, except converts result to 32-bit.
 */
int bcm_robo_stat_get32(int unit, bcm_port_t port, 
			  bcm_stat_val_t type, uint32 *value)
{
    int		rv;
    uint64	val64;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_get32()..\n")));
    rv = bcm_stat_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*value, val64);

    return(rv);
}				

/*
 * Function:
 *      bcm_robo_stat_multi_get
 * Purpose:
 *      Get the specified statistics from the device.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_stat_multi_get(int unit, bcm_port_t port, int nstat, 
                       bcm_stat_val_t *stat_arr, uint64 *value_arr)
{
    int stix;

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }

    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN
            (bcm_stat_get(unit, port, stat_arr[stix],
                              &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}

/*
 * Function:
 *      bcm_robo_stat_multi_get32
 * Purpose:
 *      Get the specified statistics from the device.  The 64-bit
 *      values may be truncated to fit.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected 32-bit statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_stat_multi_get32(int unit, bcm_port_t port, int nstat, 
                         bcm_stat_val_t *stat_arr, uint32 *value_arr)
{
    int stix;

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }

    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN
            (bcm_stat_get32(unit, port, stat_arr[stix],
                                &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}

/*
 * Function:
 *    bcm_robo_stat_sync_get
 * Description:
 *    Get the specified statistic from the device
 * Parameters:
 *    unit - Unit number.
 *    port - zero-based port number
 *    type - SNMP statistics type (see stat.h)
 *    val - (OUT) 64-bit counter value.
 * Returns:
 *    BCM_E_NONE - Success.
 *    BCM_E_PARAM - Illegal parameter.
 *    BCM_E_BADID - Illegal port number.
 *    BCM_E_INTERNAL - Chip access failure.
 *    BCM_E_UNAVAIL - Counter/variable is not implemented
 *                    on this current chip.
 * Notes:
 *    Some counters are implemented on a given port only when it is
 *    operating in a specific mode, for example, 10 or 100, and not 
 *    1000. If the counter is not implemented on a given port, OR, 
 *    on the port given its current operating mode, BCM_E_UNAVAIL
 *    is returned.
 */

int
bcm_robo_stat_sync_get(int unit, bcm_port_t port, 
    bcm_stat_val_t type, uint64 *value)
{
    int sync_hw = TRUE; /* Read counter from the device */

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_sync_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_stat_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_COUNTER_GET
        (unit, port, type, sync_hw, value));

    return BCM_E_NONE;
}


/*
 * Function:
 *    bcm_robo_stat_sync_get32
 * Description:
 *    Get the specified statistic from the device
 * Parameters:
 *    unit - Unit number.
 *    port - zero-based port number
 *    type - SNMP statistics type (see stat.h)
 *    val  - (OUT) 32-bit counter value.
 * Returns:
 *    BCM_E_NONE - Success.
 *    BCM_E_PARAM - Illegal parameter.
 *    BCM_E_BADID - Illegal port number.
 *    BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *    Same as bcm_stat_get, except converts result to 32-bit.
 */

int
bcm_robo_stat_sync_get32(int unit, bcm_port_t port, 
    bcm_stat_val_t type, uint32 *value)
{
    int     rv;
    uint64  val64;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_esw_stat_sync_get32()..\n")));

    rv = bcm_stat_sync_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*value, val64);

    return(rv);
}

/*
 * Function:
 *      bcm_robo_stat_sync_multi_get
 * Purpose:
 *      Get the specified statistics from the device.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_stat_sync_multi_get(int unit, bcm_port_t port, int nstat, 
    bcm_stat_val_t *stat_arr, uint64 *value_arr)
{
    int stix;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_sync_multi_get()..\n")));

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }

    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN(bcm_stat_sync_get
            (unit, port, stat_arr[stix], &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}

/*
 * Function:
 *      bcm_robo_stat_sync_multi_get32
 * Purpose:
 *      Get the specified statistics from the device.  The 64-bit
 *      values may be truncated to fit.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected 32-bit statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_stat_sync_multi_get32(int unit, bcm_port_t port,
    int nstat, bcm_stat_val_t *stat_arr, uint32 *value_arr)
{
    int stix;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_stat_sync_multi_get32()..\n")));

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }

    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN(bcm_stat_sync_get32
            (unit, port, stat_arr[stix], &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}


#define _ROBO_DBG_CNTR_IS_VALID(unit, type)                                   \
    ((type >= snmpBcmCustomReceive0 && type <= snmpBcmCustomReceive8) || \
    (type >= snmpBcmCustomTransmit0 && type <= snmpBcmCustomTransmit14))

/*
 * Function:
 *      bcm_robo_stat_custom_set 
 * Description:
 *      Set debug counter to count certain packet types.
 * Parameters:
 *      unit  - RoboSwitch PCI device unit number.
 *      port  - Port number 
 *      type  - SNMP statistics type.
 *      flags - The counter select value (see stat.h for bit definitions). 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_stat_custom_set(int unit, bcm_port_t port,
			   bcm_stat_val_t type, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_stat_custom_get 
 * Description:
 *      Get debug counter select value.
 * Parameters:
 *      unit  - RoboSwitch PCI device unit number.
 *      port  - Port number (only applicable to Easyrider TDBGC_SELECT).
 *      type  - SNMP statistics type.
 *      flags - (OUT) The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_stat_custom_get(int unit, bcm_port_t port,
			   bcm_stat_val_t type, uint32 *flags)
{
   return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stat_custom_add 
 * Description:
 *      Add a certain packet type to debug counter to count 
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number (only applicable to Easyrider TDBGC_SELECT).
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stat_custom_add(int unit, bcm_port_t port, bcm_stat_val_t type,
                    bcm_custom_stat_trigger_t trigger)
{
   return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stat_custom_delete 
 * Description:
 *      Deletes a certain packet type from debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number (only applicable to Easyrider TDBGC_SELECT).
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stat_custom_delete(int unit, bcm_port_t port,bcm_stat_val_t type, 
                       bcm_custom_stat_trigger_t trigger)
{
   return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stat_custom_delete_all
 * Description:
 *      Deletes a all packet types from debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number (only applicable to Easyrider TDBGC_SELECT).
 *      type  - SNMP statistics type.
 *
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stat_custom_delete_all(int unit, bcm_port_t port,bcm_stat_val_t type)
{
   return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stat_custom_check
 * Description:
 *      Check if certain packet types is part of debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number (only applicable to Easyrider TDBGC_SELECT).
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 *      result - [OUT] result of a query. 0 if positive , -1 if negative
 *
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stat_custom_check(int unit, bcm_port_t port, bcm_stat_val_t type, 
                      bcm_custom_stat_trigger_t trigger, int *result)
{
   return BCM_E_UNAVAIL;
}

