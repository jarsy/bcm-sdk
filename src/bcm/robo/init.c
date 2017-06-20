/*
 * $Id: init.c,v 1.76 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM Library Initialization
 *
 *   This module calls the initialization routine of each BCM module.
 *
 * Initial System Configuration
 *
 *   Each module should initialize itself without reference to other BCM
 *   library modules to avoid a chicken-and-the-egg problem.  To do
 *   this, each module should initialize its respective internal state
 *   and hardware tables to match the Initial System Configuration.  The
 *   Initial System Configuration is:
 *
 *   STG 1 containing VLAN 1
 *   STG 1 all ports in the DISABLED state
 *   VLAN 1 with
 *  PBMP = all switching Ethernet ports (non-fabric) and the CPU.
 *  UBMP = all switching Ethernet ports (non-fabric).
 *   No trunks configured
 *   No mirroring configured
 *   All L2 and L3 tables empty
 *   Ingress VLAN filtering disabled
 *   BPDU reception enabled
 */

#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/robo.h>
#include <soc/phyctrl.h>
#include <soc/arl.h>

#include <bcm_int/control.h>
#include <bcm/init.h>
#include <bcm/error.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/common/family.h>

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
#include <bcm_int/common/fcmap_cmn.h>
#endif /* INCLUDE_FCMAP */

#include <bcm_int/robo/switch.h>
#include <bcm_int/robo/stat.h>
#include <bcm_int/robo/stg.h>
#include <bcm_int/robo/mcast.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/tx.h>
#include <bcm_int/robo/rx.h>
#include <bcm_int/robo/mirror.h>
#include <bcm_int/robo/vlan.h>

#include <bcm_int/robo_dispatch.h>

/* ASSUMES unit PARAMETER which is not in macro's list. */
#define CLEAR_CALL(_rtn, _name) {                                       \
        int rv;                                                         \
        rv = (_rtn)(unit);                                              \
        if (rv < 0 && rv != BCM_E_UNAVAIL) {                            \
            LOG_ERROR(BSL_LS_BCM_COMMON, \
                      (BSL_META("bcm_clear %d: %s failed %d. %s\n"),    \
                       unit, _name, rv, bcm_errmsg(rv)));              \
            return rv;                                                  \
        }                                                               \
}

/*
 * Function:
 *  _bcm_robo_lock_init
 * Purpose:
 *  Allocate BCM_LOCK.
 */

STATIC int
_bcm_robo_lock_init(int unit)
{
    if (_bcm_lock[unit] == NULL) {
    _bcm_lock[unit] = sal_mutex_create("bcm_config_lock");
    }

    if (_bcm_lock[unit] == NULL) {
    return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_robo_lock_deinit
 * Purpose:
 *	De-allocate BCM_LOCK.
 */

STATIC int
_bcm_robo_lock_deinit(int unit)
{
    if (_bcm_lock[unit] != NULL) {
        sal_mutex_destroy(_bcm_lock[unit]);
        _bcm_lock[unit] = NULL;
    }
    return BCM_E_NONE;
}

#define _ROBO_DEINIT_INFO_VERB(_mod) \
    LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                (BSL_META("bcm_detach: Deinitializing %s...\n"), \
                 _mod));

#define _ROBO_DEINIT_CHECK_ERR(_rv, _mod) \
    if (_rv != BCM_E_NONE && _rv != BCM_E_UNAVAIL) { \
        LOG_WARN(BSL_LS_BCM_COMMON, \
                 (BSL_META("Warning: Deinitializing %s returned %d\n"), \
                  _mod, _rv)); \
    }

/*
 * Function:
 *	   _bcm_robo_modules_deinit
 * Purpose:
 *	   De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_robo_modules_deinit(int unit)
{
    
    int rv;  

    _ROBO_DEINIT_INFO_VERB("auth");
    rv = bcm_robo_auth_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "auth");

    _ROBO_DEINIT_INFO_VERB("rx");
    rv = bcm_robo_rx_deinit(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "rx");

    _ROBO_DEINIT_INFO_VERB("tx");
    rv = bcm_robo_tx_deinit(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "tx");

    _ROBO_DEINIT_INFO_VERB("mirror");
    rv = bcm_robo_mirror_deinit(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "mirror");

#ifdef BCM_TB_SUPPORT
    if (soc_feature(unit, soc_feature_subport)) {
        _ROBO_DEINIT_INFO_VERB("subport");
        rv = bcm_robo_subport_cleanup(unit);
        _ROBO_DEINIT_CHECK_ERR(rv, "subport");
    }
#endif

#ifdef BCM_FIELD_SUPPORT
    if (soc_feature(unit, soc_feature_field)) {
        _ROBO_DEINIT_INFO_VERB("field");
        rv = bcm_robo_field_detach(unit);
        _ROBO_DEINIT_CHECK_ERR(rv, "field");
    }
#endif /* BCM_FIELD_SUPPORT */

#ifdef INCLUDE_KNET
        _ROBO_DEINIT_INFO_VERB("knet");
        rv = bcm_robo_knet_cleanup(unit);
        _ROBO_DEINIT_CHECK_ERR(rv, "knet");
#endif

    _ROBO_DEINIT_INFO_VERB("stat");
    rv = _bcm_robo_stat_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "stat");

    _ROBO_DEINIT_INFO_VERB("linkscan");
    rv = bcm_robo_linkscan_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "linkscan");

    _ROBO_DEINIT_INFO_VERB("mcast");
    rv = _bcm_robo_mcast_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "mcast");

    _ROBO_DEINIT_INFO_VERB("multicast");
    rv = bcm_robo_multicast_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "multicast");

    _ROBO_DEINIT_INFO_VERB("cosq");
    rv = bcm_robo_cosq_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "cosq");

    _ROBO_DEINIT_INFO_VERB("trunk");
    rv = bcm_robo_trunk_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "trunk");

    _ROBO_DEINIT_INFO_VERB("vlan");
    rv = bcm_robo_vlan_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "vlan");

    _ROBO_DEINIT_INFO_VERB("stg");
    rv = bcm_robo_stg_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "stg");

    _ROBO_DEINIT_INFO_VERB("l2");
    rv = bcm_robo_l2_detach(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "l2");

    _ROBO_DEINIT_INFO_VERB("port");
    rv = _bcm_robo_port_deinit(unit);
    _ROBO_DEINIT_CHECK_ERR(rv, "port");

    BCM_UNLOCK(unit);
    _bcm_robo_lock_deinit(unit);

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_robo_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit. 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_robo_threads_shutdown(int unit)
{
    int rv = BCM_E_NONE;
    rv = bcm_linkscan_enable_set(unit, 0);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    rv = soc_robo_arl_mode_set(unit, ARL_MODE_NONE);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    if (soc_feature(unit, soc_feature_hw_dos_report)) {
        rv = soc_robo_dos_monitor_deinit(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }
    
    rv = soc_robo_counter_detach(unit);

    return (rv);
}

STATIC int
_bcm_robo_modules_init(int unit)
{
    /*
     * Initialize each bcm module that requires it.
     */

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(bcm_chip_family_set(unit, BCM_FAMILY_ROBO)); 

    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_port_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_l2_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_stg_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_vlan_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_trunk_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_cosq_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_mcast_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_multicast_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_linkscan_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_stat_init(unit));
#ifdef INCLUDE_KNET
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_knet_init(unit));
#endif

#ifdef BCM_TB_SUPPORT
    if (soc_feature(unit, soc_feature_subport)) {
        BCM_IF_ERROR_RETURN(bcm_subport_init(unit));
    }
#endif    
#ifdef BCM_FIELD_SUPPORT
    if (soc_feature(unit, soc_feature_field)) {
        BCM_IF_ERROR_RETURN(bcm_field_init(unit));
    }
#endif
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_policer_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_mirror_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_tx_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_rx_init(unit));
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(bcm_auth_init(unit));
    
    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_init
 * Purpose:
 *  Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *  unit - RoboSwitch unit #.
 * Returns:
 *  BCM_E_XXX
 */

STATIC int 
_bcm_robo_init(int unit)     /* Initialize chip and BCM layer */
{

    BCM_IF_ERROR_RETURN(_bcm_robo_lock_init(unit));

    /* If linkscan is running, disable it. */
    BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0));

#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit)); 
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit)); 
#endif /* INCLUDE_FCMAP */

    BCM_IF_ERROR_RETURN(_bcm_robo_modules_init(unit));

    BCM_IF_ERROR_RETURN(_bcm_robo_switch_init(unit));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_init
 * Purpose:
 * 	Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *	unit - RoboSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_robo_init(int unit)
{
    if (0 == SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    return _bcm_robo_init(unit);
}    

/*      _bcm_robo_attach
 * Purpose:
 *      Attach and initialize bcm device
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_robo_attach(int unit, char *subtype)
{
    int dunit;
    int rv;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;

    /* Initialize soc layer */
    if ((NULL == SOC_CONTROL(dunit)) || 
        (0 == (SOC_CONTROL(dunit)->soc_flags & SOC_F_ATTACHED))) {
        return (BCM_E_INIT);
    }

    if (SAL_THREAD_ERROR == SOC_CONTROL(dunit)->counter_pid) {
        rv = soc_robo_counter_attach(unit);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (soc_feature(unit, soc_feature_hw_dos_report)) {
        rv = soc_robo_dos_monitor_init(unit);
        BCM_IF_ERROR_RETURN(rv);
    }
    
    /* Initialize bcm layer */
    BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
    BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
    BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH;
    
    rv = _bcm_robo_init(unit);
    return rv;
}

/*      _bcm_robo_match
 * Purpose:
 *      match BCM control subtype strings for ROBO subtypes 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    0 match
 *    !0 no match
 */
int
_bcm_robo_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}

/*
 * Function:
 *  bcm_robo_init_selective
 * Purpose:
 *  Initialize specific bcm modules as desired.
 * Parameters:
 *  unit - StrataSwitch unit #.
 *  module_number - Indicate module number
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_init_selective(int unit, uint32 module_number)
{
    switch (module_number) {
        case BCM_MODULE_PORT     :
            BCM_IF_ERROR_RETURN(bcm_port_init(unit));
            break;
        case BCM_MODULE_L2       :
            BCM_IF_ERROR_RETURN(bcm_l2_init(unit));
            break;
        case BCM_MODULE_VLAN     :   
            BCM_IF_ERROR_RETURN(bcm_vlan_init(unit));
            break;
        case BCM_MODULE_TRUNK    :
            BCM_IF_ERROR_RETURN(bcm_trunk_init(unit));
            break;
        case BCM_MODULE_COSQ     :
            BCM_IF_ERROR_RETURN(bcm_cosq_init(unit));
            break;
        case BCM_MODULE_MCAST        :
            BCM_IF_ERROR_RETURN(bcm_mcast_init(unit));
            break;
        case BCM_MODULE_LINKSCAN  :
            BCM_IF_ERROR_RETURN(bcm_linkscan_init(unit));
            break;
        case BCM_MODULE_STAT     :
            BCM_IF_ERROR_RETURN(bcm_stat_init(unit));
            break;
        case BCM_MODULE_MIRROR   :
            BCM_IF_ERROR_RETURN(bcm_mirror_init(unit));
            break;
        case BCM_MODULE_SUBPORT	:
            BCM_IF_ERROR_RETURN(bcm_subport_init(unit));
            break;
        case BCM_MODULE_STG      :
            BCM_IF_ERROR_RETURN(bcm_stg_init(unit));
            break;
        case BCM_MODULE_TX       :
            BCM_IF_ERROR_RETURN(bcm_tx_init(unit));
            break;
        case BCM_MODULE_AUTH     :
            BCM_IF_ERROR_RETURN(bcm_auth_init(unit));
            break;
        case BCM_MODULE_RX       :
            BCM_IF_ERROR_RETURN(bcm_rx_init(unit));
            break;
#ifdef BCM_FIELD_SUPPORT
        case BCM_MODULE_FIELD    :
            BCM_IF_ERROR_RETURN(bcm_field_init(unit));
            break;
#endif /* BCM_FIELD_SUPPORT */
        case BCM_MODULE_POLICER  :
            BCM_IF_ERROR_RETURN(bcm_policer_init(unit));
            break;
        case BCM_MODULE_MULTICAST:
            BCM_IF_ERROR_RETURN(bcm_multicast_init(unit));
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}   

/*
 * Function:
 *  bcm_robo_init_check
 * Purpose:
 *  Return TRUE if bcm_init_bcm has already been called and succeeded
 * Parameters:
 *  unit- RoboSwitch unit #.
 * Returns:
 *  TRUE or FALSE
 */
int 
bcm_robo_init_check(int unit)   /* Return true BCM layer init done */
{
    return BCM_E_UNAVAIL;
}   


/*
 * Function:
 *      _bcm_robo_detach
 * Purpose:
 *      Clean up bcm layer when unit is detached
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *  BCM_E_XXX
 */
int 
_bcm_robo_detach(int unit)
{
    int rv;                    /* Operation return status. */

    /* Shut down all the spawned threads. */
    rv = _bcm_robo_threads_shutdown(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    /* 
     *  Don't move up, holding lock or disabling hw operations 
     *  might prevent theads clean exit.
     */
    BCM_LOCK(unit);

    rv = _bcm_robo_modules_deinit(unit);

    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    return BCM_E_NONE;

}   

/*
 * Function:
 *  bcm_robo_info_get
 * Purpose:
 *  Provide unit information to caller
 * Parameters:
 *  unit    - switch device
 *  info    - (OUT) bcm unit info structure
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_info_get(int unit, bcm_info_t *info)
{
    uint16  dev_id = 0;
    uint8   rev_id = 0;
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    if (info == NULL) {
        return BCM_E_PARAM;
    }
    
    soc_cm_get_id(unit, &dev_id, &rev_id);
    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = dev_id;
    info->revision = rev_id;
    info->capability = 0;
    /* All ROBO chips are of switch capability now */
    info->capability |= BCM_INFO_SWITCH;
    if (soc_feature(unit, soc_feature_l3)) {
        info->capability |= BCM_INFO_L3;
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        info->capability |= BCM_INFO_IPMC;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_clear
 * Purpose:
 *      Initialize a device without a full reset
 * Parameters:
 *      unit        - The unit number of the device to clear
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      For each module, call the underlieing init/clear operation
 */

int 
bcm_robo_clear(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    CLEAR_CALL(bcm_port_clear, "port");
    CLEAR_CALL(bcm_l2_clear, "L2");
    CLEAR_CALL(bcm_stg_clear, "STG");
    CLEAR_CALL(bcm_vlan_init, "VLAN");
    CLEAR_CALL(bcm_trunk_init, "trunk");
    CLEAR_CALL(bcm_cosq_init, "COSQ");
    CLEAR_CALL(bcm_mcast_init, "MCast");
    CLEAR_CALL(bcm_multicast_init, "Multicast");

    /* Linkscan init is not called; assumed running as configured */
    /* Stats init is not called; assumed running as configured */

#ifdef BCM_FIELD_SUPPORT
    CLEAR_CALL(bcm_field_init, "field");
#endif
    CLEAR_CALL(bcm_mirror_init, "mirror");

    /* TX should not need clearing */

    /* Stacking calls will go away */

    CLEAR_CALL(bcm_auth_init, "AUTH");

    return BCM_E_NONE;
}

    
