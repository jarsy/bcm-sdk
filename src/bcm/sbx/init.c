/*
 * $Id: init.c,v 1.127.18.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM-SBX Library Initialization
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
 *      PBMP = all switching Ethernet ports (non-fabric) and the CPU.
 *      UBMP = all switching Ethernet ports (non-fabric).
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
#include <soc/l2x.h>
#include <soc/mem.h>
#include <soc/counter.h>

#include <bcm_int/control.h>
#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/init.h>
#include <bcm/l2.h>
#include <bcm/link.h>
#include <bcm/rx.h>
#include <bcm/stat.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>
#include <bcm/vswitch.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/fabric.h>
#include <bcm/policer.h>
#include <bcm/oam.h>
#include <bcm/field.h>
#include <bcm/mirror.h>
#include <bcm/mcast.h>
#include <bcm/qos.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/error.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/common/family.h>
#include <bcm_int/sbx_dispatch.h>
#ifdef BCM_FE2000_SUPPORT
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <bcm_int/sbx/fe2000/g2p3.h>
#endif
#include <bcm_int/fe2000_dispatch.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/recovery.h>
#include <bcm_int/sbx/fe2000/switch.h>
#endif /* BCM_FE2000_SUPPORT */
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/failover.h>

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
#include <bcm_int/common/fcmap_cmn.h>
#endif /* INCLUDE_FCMAP */

#ifdef BCM_CALADAN3_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/sbx/caladan3/wb_db_init.h>
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* BCM_CALADAN3_SUPPORT */

/* See BCM_SEL_INIT flags in init.h */
STATIC bcm_sbx_state_t _sbx_state[BCM_MAX_NUM_UNITS];

#ifdef BCM_FE2000_SUPPORT

extern int bcm_fe2000_l2cache_detach(int unit);
extern int bcm_fe2000_mirror_detach(int unit);
extern int bcm_fe2000_policer_detach(int unit);
extern int bcm_fe2000_vlan_detach(int unit);
extern int _bcm_fe2000_port_deinit(int unit);
extern int bcm_fe2000_mcast_detach(int unit);
extern int bcm_fe2000_stg_detach(int unit);
#endif

#ifdef BCM_CALADAN3_SUPPORT
extern int _sbx_caladan3_resource_init(int unit);
extern int _sbx_caladan3_alloc_wellknown_resources(int unit);
extern int _bcm_caladan3_switch_control_init(int unit);
extern int _bcm_caladan3_g3p1_igmp_snooping_init(int unit);
#endif
#ifdef BCM_SIRIUS_SUPPORT
extern int bcm_sirius_port_deinit(int unit);
#endif /* def BCM_SIRIUS_SUPPORT */
extern int bcm_sbx_fabric_detach(int unit);

STATIC int
_bcm_sbx_lock_init(int unit)
{
    if (_bcm_lock[unit] == NULL) {
        _bcm_lock[unit] = sal_mutex_create("bcm_sbx_config_lock");
    }

    if (_bcm_lock[unit] == NULL) {
        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}

/* assumes unit and rv are declared in local scope */
#define INIT_CALL_CHK( m )             \
  rv = bcm_##m##_init(unit);                                            \
  if (BCM_FAILURE(rv)) {                                               \
      LOG_ERROR(BSL_LS_BCM_COMMON, \
                (BSL_META_U(unit, \
                            "bcm_init(%d): " #m " failed (%d -- %s)\n"), \
                 unit, rv, bcm_errmsg(rv)));                            \
      return rv;                                                       \
  }

#ifdef BCM_CALADAN3_SUPPORT
int
bcm_sbx_caladan3_init(int unit)
{
    int rv = BCM_E_NONE;
    int bypass = 0;

    /* skip bcm init for PCID only configuration */
    bypass = soc_property_get(unit, spn_LRP_BYPASS, 0);

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_IF_ERROR_RETURN(soc_mem_cache_scache_init(unit));
#endif  /* BCM_WARM_BOOT_SUPPORT */
    
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(tx);
    INIT_CALL_CHK(rx);
    INIT_CALL_CHK(linkscan);
    
    if (SOC_IS_SBX_G3P1(unit)) {
        if (!(bypass) ) {
            INIT_CALL_CHK(trunk);
            INIT_CALL_CHK(failover);
            INIT_CALL_CHK(qos);
            INIT_CALL_CHK(stg);
            INIT_CALL_CHK(vswitch);
            /* for warm boot support, stat must be before mpls/vlan/policer */
            INIT_CALL_CHK(stat);
            INIT_CALL_CHK(vlan);
            INIT_CALL_CHK(l2);
            INIT_CALL_CHK(cosq);
            INIT_CALL_CHK(l3);
            INIT_CALL_CHK(mpls);
            INIT_CALL_CHK(ipmc);
            INIT_CALL_CHK(policer);

            INIT_CALL_CHK(field);
            INIT_CALL_CHK(mcast);
            INIT_CALL_CHK(mirror);
#if  defined(BCM_CALADAN3_MIM_SUPPORT) || defined(BCM_CALADAN3_MIM_WARM_BOOT_DEBUG)
            INIT_CALL_CHK(mim);
#endif
            INIT_CALL_CHK(time);
            INIT_CALL_CHK(oam);
        }
    }
    
    if (SOC_IS_SBX_T3P1(unit)) {
        INIT_CALL_CHK(stat);
	}
    
    if (SOC_IS_SBX_G3P1(unit) && !bypass) {
        BCM_IF_ERROR_RETURN(_bcm_caladan3_g3p1_igmp_snooping_init(unit));
    }

    return rv;
}
#endif /* BCM_CALADAN3_SUPPORT */


#ifdef BCM_BME3200_SUPPORT
int
bcm_sbx_bm3200_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    return BCM_E_NONE;
}
#endif /* BCM_BME3200_SUPPORT */


#ifdef BCM_BM9600_SUPPORT
int
bcm_sbx_bm9600_init(int unit)
{
    int rv;
    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    return BCM_E_NONE;
}
#endif /* BCM_BM9600_SUPPORT */


#ifdef BCM_QE2000_SUPPORT
int
bcm_sbx_qe2000_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(vlan);
    INIT_CALL_CHK(trunk);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    INIT_CALL_CHK(rx);
    INIT_CALL_CHK(stat);
    INIT_CALL_CHK(tx);        
    
    return BCM_E_NONE;
}
#endif /* BCM_QE2000_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
int
bcm_sbx_sirius_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(trunk);
    INIT_CALL_CHK(linkscan);
    INIT_CALL_CHK(rx);
    INIT_CALL_CHK(stat);
    INIT_CALL_CHK(multicast);
    INIT_CALL_CHK(tx);

    return BCM_E_NONE;
}
#endif /* BCM_SIRIUS_SUPPORT */


int
bcm_sbx_init(int unit)
{
#ifdef BCM_CALADAN3_SUPPORT
    int bypass = 0;
#endif
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* init state pointer */
    SOC_SBX_CONTROL(unit)->state = &(_sbx_state[unit]);

#ifdef BCM_CALADAN3_SUPPORT
    bypass = soc_property_get(unit, spn_LRP_BYPASS, 0);
    if (SOC_IS_SBX_CALADAN3(unit) && !(bypass) && SOC_IS_SBX_G3P1(unit)) {
        BCM_IF_ERROR_RETURN(_sbx_caladan3_resource_init(unit));
        BCM_IF_ERROR_RETURN(_sbx_caladan3_alloc_wellknown_resources(unit));
        BCM_IF_ERROR_RETURN(_bcm_caladan3_switch_control_init(unit));
    }
#endif /* BCM_CALADAN3_SUPPORT */

#ifdef BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif /* BCM_BME3200_SUPPORT */


#ifdef BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif  /* BCM_BM9600_SUPPORT */


#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));

    }
#endif /* BCM_QE2000_SUPPORT */


#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif  /* BCM_SIRIUS_SUPPORT */


    BCM_IF_ERROR_RETURN(_bcm_sbx_lock_init(unit));

    /* If linkscan is running, disable it. */
    BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0));

#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit));
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit));
#endif /* INCLUDE_FCMAP */


    /*  The call sequence below replaces bcm_init_selective calls */
    bcm_chip_family_set(unit, BCM_FAMILY_SBX);
    if (0) {
    }
#ifdef BCM_CALADAN3_SUPPORT
    else if (SOC_IS_SBX_CALADAN3(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_caladan3_init(unit));
#ifdef BCM_WARM_BOOT_SUPPORT
        BCM_IF_ERROR_RETURN(bcm_caladan3_wb_init_state_init (unit));
#endif /* BCM_WARM_BOOT_SUPPORT */
    }
#endif
#ifdef BCM_BME3200_SUPPORT
    else if (SOC_IS_SBX_BME3200(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_bm3200_init(unit));
    }
#endif
#ifdef BCM_BM9600_SUPPORT
    else if (SOC_IS_SBX_BM9600(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_bm9600_init(unit));
    }
#endif
#ifdef BCM_QE2000_SUPPORT
    else if (SOC_IS_SBX_QE2000(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_qe2000_init(unit));
    }
#endif
#ifdef BCM_SIRIUS_SUPPORT
    else if (SOC_IS_SIRIUS(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_sirius_init(unit));
    }
#endif

    if (!SOC_IS_SBX_FE(unit)) {
        BCM_IF_ERROR_RETURN
            (bcm_sbx_failover_init(unit));
    }

    return BCM_E_NONE;
}

int
bcm_sbx_info_get(int unit,
                 bcm_info_t *info)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    if (info == NULL) {
        return BCM_E_PARAM;
    }
    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = SOC_PCI_DEVICE(unit);
    info->revision = SOC_PCI_REVISION(unit);
    info->capability = 0;
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        info->capability |= BCM_INFO_FABRIC;
    }
#endif
    if (soc_feature(unit, soc_feature_l3)) {
        info->capability |= BCM_INFO_L3;
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        info->capability |= BCM_INFO_IPMC;
    }
    return BCM_E_NONE;
}

int
bcm_sbx_init_check(int unit)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_init_selective(int unit, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_clear(int unit)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *         _bcm_sbx_modules_deinit
 * Purpose:
 *         De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_modules_deinit(int unit)
{
    int rv = BCM_E_NONE;
    
    /* Detach, ignoring uninit errors */
#define  SBX_DETACH( detach_f )                                              \
  rv = detach_f(unit);                                                       \
  if (rv == BCM_E_INIT) { rv = BCM_E_NONE; }                                 \
  if (BCM_FAILURE(rv)) {                                                     \
      LOG_CLI((BSL_META_U(unit,                                              \
                          #detach_f " failed: %s\n"), bcm_errmsg(rv)));      \
  }                                                                          \
  BCM_IF_ERROR_RETURN(rv); 


    if (SOC_IS_SBX_CALADAN3(unit)) {
#ifdef  BCM_CALADAN3_SUPPORT
        LOG_CLI((BSL_META_U(unit,
                            "WARM_BOOT_TODO: %s"),__FUNCTION__));
#endif
    } else {
        SBX_DETACH(bcm_sbx_multicast_detach);
        SBX_DETACH(bcm_sbx_trunk_detach);
        SBX_DETACH(bcm_sbx_cosq_detach);
#ifdef BCM_SIRIUS_SUPPORT
        if (SOC_IS_SIRIUS(unit)) {
            SBX_DETACH(bcm_sirius_port_deinit);
        }
#endif /* def BCM_SIRIUS_SUPPORT */
        SBX_DETACH(bcm_sbx_fabric_detach);
    }
    if (!SOC_IS_SBX_FE(unit)) {
        SBX_DETACH(bcm_sbx_failover_deinit);
    }

    SBX_DETACH(bcm_common_linkscan_detach);
    SBX_DETACH(bcm_sbx_rx_clear);


#undef SBX_DETACH
    return rv;
}
int
_bcm_sbx_attach(int unit, char *subtype)
{
    int         dunit;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;
    if (SOC_UNIT_VALID(dunit)) {
        BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
        BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
        BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);

        if (SOC_IS_SBX_FE(dunit)) {
            BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH_SBX;
        } else if (SOC_IS_SBX_QE(dunit) ||
                   SOC_IS_SBX_BME(dunit)) {
            BCM_CONTROL(unit)->capability |= BCM_CAPA_FABRIC_SBX;
        }
#ifdef  BCM_CALADAN3_SUPPORT
        else if (SOC_IS_SBX_CALADAN3(dunit)) {
            BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH_SBX;
        }
#endif
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_sbx_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit.
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_sbx_threads_shutdown(int unit)
{
    int rv;     /* Operation return status. */

    rv = bcm_linkscan_enable_set(unit, 0);
    BCM_IF_ERROR_RETURN(rv);

    return rv;
}

int
_bcm_sbx_detach(int unit)
{
    int rv;                    /* Operation return status. */

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        SOC_DETACH(unit, TRUE);
    }
#endif

    /* Shut down all the spawned threads. */
    rv = _bcm_sbx_threads_shutdown(unit);
    BCM_IF_ERROR_RETURN(rv);

    /* 
     *  Don't move up, holding lock or disabling hw operations
     *  might prevent theads clean exit.
     */
    BCM_LOCK(unit);

    rv = _bcm_sbx_modules_deinit(unit);
    
    BCM_UNLOCK(unit);

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        SOC_DETACH(unit, FALSE);
    }
#endif
    return rv;
}

/*      _bcm_sbx_match
 * Purpose:
 *      match BCM control subtype strings for SBX types 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    0 match
 *    !0 no match
 */
int
_bcm_sbx_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}

