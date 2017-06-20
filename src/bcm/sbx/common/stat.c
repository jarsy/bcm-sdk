/*
 * $Id: stat.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom StrataSwitch SNMP Statistics API.
 */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>

#include <bcm/error.h>
#include <bcm/stat.h>

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/stat.h>

bcm_stat_info_t stat_info[BCM_MAX_NUM_UNITS];

int
bcm_sbx_stat_init(int unit)
{
    int rv = BCM_E_NONE;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_init, (unit));

    return rv;
}

int
bcm_sbx_stat_sync(int unit)
{
    int rv = BCM_E_NONE;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_sync, (unit));

    return rv;
}

int
bcm_sbx_stat_get(int unit,
                 bcm_port_t port,
                 bcm_stat_val_t type,
                 uint64 *val)
{
    int        rv = BCM_E_NONE;


    if (port == CMIC_PORT(unit)) {
	return (BCM_E_UNAVAIL);
    } 

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_get, (unit, port, type, val));

    return rv;
}

int
bcm_sbx_stat_get32(int unit,
                   bcm_port_t port,
                   bcm_stat_val_t type,
                   uint32 *val)
{
    int       rv = BCM_E_NONE;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_get32, (unit, port, type, val));

    return (rv);
}

int
bcm_sbx_stat_multi_get(int unit, 
		       bcm_port_t port, 
		       int nstat, 
		       bcm_stat_val_t *stat_arr, 
		       uint64 *value_arr)
{
    int        rv = BCM_E_NONE;
        
    if (port == CMIC_PORT(unit)) {
	return (BCM_E_UNAVAIL);
    } 
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_multi_get, (unit, port, nstat, stat_arr, value_arr));

    return rv;
}

int
bcm_sbx_stat_multi_get32(int unit, 
			 bcm_port_t port, 
			 int nstat, 
			 bcm_stat_val_t *stat_arr, 
			 uint32 *value_arr)
{
    int       rv = BCM_E_NONE;
    
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_multi_get32, (unit, port, nstat, stat_arr, value_arr));

    return (rv);
}

int 
bcm_sbx_stat_clear(int unit,
                   bcm_port_t port)
{
    int       rv = BCM_E_NONE;

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_clear, (unit, port));

    return (rv);

}

int
bcm_sbx_stat_custom_set(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 flags)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_set, (unit, port, type, flags));

    return (rv);
}

int
bcm_sbx_stat_custom_get(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 *flags)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_get, (unit, port, type, flags));

    return (rv);
}

int 
bcm_sbx_stat_custom_add(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_add, (unit, port, type, trigger));

    return (rv);
}

int 
bcm_sbx_stat_custom_delete(int unit,
                           bcm_port_t port,
                           bcm_stat_val_t type, 
                           bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_delete, (unit, port, type, trigger));

    return (rv);
}

int 
bcm_sbx_stat_custom_delete_all(int unit,
                               bcm_port_t port,
                               bcm_stat_val_t type)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_delete_all, (unit, port, type));

    return (rv);
}

int 
bcm_sbx_stat_custom_check(int unit,
                          bcm_port_t port,
                          bcm_stat_val_t type, 
                          bcm_custom_stat_trigger_t trigger,
                          int *result)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_check, (unit, port, type, trigger, result));

    return (rv);
}

