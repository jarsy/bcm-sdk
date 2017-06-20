/*
 * $Id: init.c,v 1.197 Broadcom SDK $
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
 *	PBMP = all switching Ethernet ports (non-fabric) and the CPU.
 *	UBMP = all switching Ethernet ports (non-fabric).
 *   No trunks configured
 *   No mirroring configured
 *   All L2 and L3 tables empty
 *   Ingress VLAN filtering disabled
 *   BPDU reception enabled
 */

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>
#include <shared/bsl.h>
#include <soc/cmext.h>
#include <soc/counter.h>
#include <soc/l2x.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/phyctrl.h>

#include <bcm/init.h>
#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm/pkt.h>
#include <bcm/ipfix.h>

#ifdef SW_AUTONEG_SUPPORT
#include <bcm_int/common/sw_an.h>
#endif 
 
#include <bcm_int/api_xlate_port.h>
#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#ifdef BCM_RCPU_SUPPORT
#include <bcm_int/esw/rcpu.h>
#endif
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/stat.h>
#include <bcm_int/esw/mcast.h>
#include <bcm_int/esw/range.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/ipfix.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/stg.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/cosq.h>
#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/rate.h>
#ifdef BCM_INSTRUMENTATION_SUPPORT
#include <bcm_int/esw/instrumentation.h>
#endif /* BCM_INSTRUMENTATION_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
#include <bcm_int/esw/katana.h>
#endif
#if defined(BCM_KATANA_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
#include <bcm_int/esw/policer.h>
#endif
#ifdef BCM_TRX_SUPPORT
#include <bcm_int/esw/trx.h>
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_HURRICANE2_SUPPORT
#include <bcm_int/esw/hurricane2.h>
#endif

#if defined(BCM_SABER2_SUPPORT)
#include <bcm_int/esw/saber2.h>
#endif /* BCM_SABER2_SUPPORT */

#if defined(BCM_SABER2_SUPPORT)
#include <bcm_int/esw/saber2.h>
#include <bcm_int/common/sat.h>
#endif /* BCM_SABER2_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/mcast.h>
#include <bcm_int/esw/port.h>
#endif

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
#include <bcm_int/common/fcmap_cmn.h>
#endif /* INCLUDE_FCMAP */

#ifdef BCM_TOMAHAWK_SUPPORT
#include <soc/tomahawk.h>
#include <bcm_int/esw/tomahawk.h>
#endif

#include <bcm_int/esw_dispatch.h>
#include <shared/shr_bprof.h>

#define BCM_CHECK_ERROR_RETURN(rv, dispname)                         \
    if ((rv < 0) && (rv != BCM_E_UNAVAIL)) {                         \
        LOG_WARN(BSL_LS_BCM_INIT,                                    \
                    (BSL_META_U(unit, "bcm_init failed in %s\n"),    \
                 shr_bprof_stats_name(dispname)));                   \
        return (rv);                                                 \
    }

#if defined(BCM_TOMAHAWK_SUPPORT)
extern int soc_ctr_evict_start(int unit, uint32 flags, sal_usecs_t interval);
#endif
#if defined(BCM_TRIUMPH3_SUPPORT)
extern int _bcm_esw_tr3_port_lanes_init(int unit);
#endif

/*
 * Function:
 *	_bcm_lock_init
 * Purpose:
 *	Allocate BCM_LOCK.
 */

STATIC int
_bcm_lock_init(int unit)
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
 *	_bcm_lock_deinit
 * Purpose:
 *	De-allocate BCM_LOCK.
 */

STATIC int
_bcm_lock_deinit(int unit)
{
    if (_bcm_lock[unit] != NULL) {
        sal_mutex_destroy(_bcm_lock[unit]);
        _bcm_lock[unit] = NULL;
    }
    return BCM_E_NONE;
}

#define _DEINIT_INFO_VERB(_mod) \
    LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                (BSL_META_U(unit, \
                            "bcm_detach: Deinitializing %s...\n"),   \
                 _mod));

#define _DEINIT_CHECK_ERR(_rv, _mod) \
    if (_rv != BCM_E_NONE && _rv != BCM_E_UNAVAIL) { \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "Warning: Deinitializing %s returned %d\n"), \
                  _mod, _rv)); \
    }

/*
 * Function:
 *	   _bcm_esw_modules_deinit
 * Purpose:
 *     bcm_detach_late_txrx() will deinit all modules except TX and RX.
 *     A regular bcm_detach() must follow to detach the remaining modules.
 *	   De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_modules_deinit(int unit)
{
    int rv;    /* Operation return status. */
    /* Only TX/RX deinit should be done here */
    if (BCM_CONTROL(unit)->attach_state == _bcmControlStateDeinitAll) {
        _DEINIT_INFO_VERB("rx");
        rv = bcm_esw_rx_deinit(unit);
        /* There is no tx deinit defined currently */
        BCM_UNLOCK(unit);
        _bcm_lock_deinit(unit);
        return rv;
    }
#ifdef INCLUDE_BFD
    if (soc_feature(unit, soc_feature_bfd)) {
        _DEINIT_INFO_VERB("bfd");
        rv =  bcm_esw_bfd_detach(unit);
        _DEINIT_CHECK_ERR(rv, "bfd");
    }
#endif
#ifdef INCLUDE_PTP
    if (soc_feature(unit, soc_feature_ptp)) {
        _DEINIT_INFO_VERB("ptp");
	    rv =  bcm_esw_ptp_detach(unit);
        _DEINIT_CHECK_ERR(rv, "ptp");
    }
#endif
    if (soc_feature(unit, soc_feature_oam))
    {
        _DEINIT_INFO_VERB("oam");
        rv = bcm_esw_oam_detach(unit);
        _DEINIT_CHECK_ERR(rv, "oam");
    }

#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_failover))
    {
        _DEINIT_INFO_VERB("failover");
        rv = bcm_esw_failover_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "failover");
    }
#endif

    if (soc_feature(unit, soc_feature_time_support))
    {
        _DEINIT_INFO_VERB("time");
        rv = bcm_esw_time_deinit(unit);
        _DEINIT_CHECK_ERR(rv, "time");
    }

    _DEINIT_INFO_VERB("udf");
    rv = bcm_esw_udf_detach(unit);
    _DEINIT_CHECK_ERR(rv, "udf");

#ifdef BCM_FIELD_SUPPORT
    _DEINIT_INFO_VERB("auth");
    rv = bcm_esw_auth_detach(unit);
    _DEINIT_CHECK_ERR(rv, "auth");

    _DEINIT_INFO_VERB("field");
    rv = bcm_esw_field_detach(unit);
    _DEINIT_CHECK_ERR(rv, "field");
#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)) {
        if (SOC_MEM_IS_VALID(unit, IFP_RANGE_CHECKm)) {
            _DEINIT_INFO_VERB("range");
            rv = bcm_esw_range_detach(unit);
            _DEINIT_CHECK_ERR(rv, "range");
        }
    }
#endif
#endif /* BCM_FIELD_SUPPORT */

#ifdef INCLUDE_L3
    _DEINIT_INFO_VERB("proxy");
    rv = bcm_esw_proxy_cleanup(unit);
    _DEINIT_CHECK_ERR(rv, "proxy");

    _DEINIT_INFO_VERB("multicast");
    rv = bcm_esw_multicast_detach(unit);
    _DEINIT_CHECK_ERR(rv, "multicast");

    if (soc_feature(unit, soc_feature_port_extension)) {
        _DEINIT_INFO_VERB("extender");
        rv =  bcm_esw_extender_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "extender");
    }

    if (soc_feature(unit, soc_feature_vxlan)) {
        _DEINIT_INFO_VERB("vxlan");
        rv =  bcm_esw_vxlan_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "vxlan");
    }

    if (soc_feature(unit, soc_feature_l2gre)) {
        _DEINIT_INFO_VERB("l2gre");
        rv = bcm_esw_l2gre_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "l2gre");
    }

    if (soc_feature(unit, soc_feature_niv)) {
        _DEINIT_INFO_VERB("niv");
        rv = bcm_esw_niv_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "niv");
    }

    if (soc_feature(unit, soc_feature_trill)) {
        _DEINIT_INFO_VERB("trill");
        rv = bcm_esw_trill_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "trill");
    }

    _DEINIT_INFO_VERB("subport");
    rv = bcm_esw_subport_cleanup(unit);
    _DEINIT_CHECK_ERR(rv, "subport");

#ifdef BCM_MPLS_SUPPORT
    _DEINIT_INFO_VERB("mpls");
    rv = bcm_esw_mpls_cleanup(unit);
    _DEINIT_CHECK_ERR(rv, "mpls");
#endif /* BCM_MPLS_SUPPORT */

    _DEINIT_INFO_VERB("ipmc");
    rv = bcm_esw_ipmc_detach(unit);
    _DEINIT_CHECK_ERR(rv, "ipmc");

    _DEINIT_INFO_VERB("l3");
    /*
     * COVERITY
     *
     * Coverity reports a call chain of depth greater than 20, but well
     * before the stack overflows, it calls bcm_esw_switch_control_get
     * with one control selection, then follows another control's path
     * to find more levels of stack.  This is spurious.
     */
    /* coverity[stack_use_overflow : FALSE] */
    rv = bcm_esw_l3_cleanup(unit);
    _DEINIT_CHECK_ERR(rv, "l3");
#endif /* INCLUDE_L3 */

    /* Delaying RX deinit so the CPU can stil receive packets
     * even when other modules are deinitialized. */
    if (!(BCM_CONTROL(unit)->attach_state == _bcmControlStateDeinitLateTxRx)) {
        _DEINIT_INFO_VERB("rx");
        rv = bcm_esw_rx_deinit(unit);
        _DEINIT_CHECK_ERR(rv, "rx");
    }

#if 0
    _DEINIT_INFO_VERB("tx");
    rv = bcm_esw_tx_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "tx");
#endif 

    _DEINIT_INFO_VERB("mirror");
    rv = bcm_esw_mirror_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "mirror");

#ifdef INCLUDE_KNET
    _DEINIT_INFO_VERB("knet");
    rv = bcm_esw_knet_cleanup(unit);
    _DEINIT_CHECK_ERR(rv, "knet");
#endif

    _DEINIT_INFO_VERB("stacking");
    rv = _bcm_esw_stk_detach(unit);
    _DEINIT_CHECK_ERR(rv, "stacking");

    _DEINIT_INFO_VERB("stats");
    rv = _bcm_esw_stat_detach(unit);
    _DEINIT_CHECK_ERR(rv, "stats");

    _DEINIT_INFO_VERB("linkscan");
    rv = bcm_esw_linkscan_detach(unit);
    _DEINIT_CHECK_ERR(rv, "linkscan");

#ifdef SW_AUTONEG_SUPPORT
    if (soc_feature(unit, soc_feature_sw_autoneg)) {
        _DEINIT_INFO_VERB("SW_AN");
        rv = bcm_sw_an_module_deinit(unit);
        _DEINIT_CHECK_ERR(rv, "SW_AN");
    }
#endif    

    _DEINIT_INFO_VERB("mcast");
    rv = _bcm_esw_mcast_detach(unit);
    _DEINIT_CHECK_ERR(rv, "mcast");

    _DEINIT_INFO_VERB("cosq");
    rv = bcm_esw_cosq_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "cosq");

    _DEINIT_INFO_VERB("trunk");
    rv = bcm_esw_trunk_detach(unit);
    _DEINIT_CHECK_ERR(rv, "trunk");

    _DEINIT_INFO_VERB("vlan");
    rv = bcm_esw_vlan_detach(unit);
    _DEINIT_CHECK_ERR(rv, "vlan");

    _DEINIT_INFO_VERB("stg");
    rv = bcm_esw_stg_detach(unit);
    _DEINIT_CHECK_ERR(rv, "stg");

    _DEINIT_INFO_VERB("l2");
    rv = bcm_esw_l2_detach(unit);
    _DEINIT_CHECK_ERR(rv, "l2");

    _DEINIT_INFO_VERB("port");
    rv = _bcm_esw_port_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "port");

    _DEINIT_INFO_VERB("ipfix");
    rv = _bcm_esw_ipfix_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "ipfix");

    _DEINIT_INFO_VERB("mbcm");
    rv = mbcm_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "mbcm");

#ifdef INCLUDE_L3
    _DEINIT_INFO_VERB("wlan");
    rv = bcm_esw_wlan_detach(unit);
    _DEINIT_CHECK_ERR(rv, "wlan");

    _DEINIT_INFO_VERB("mim");
    rv = bcm_esw_mim_detach(unit);
    _DEINIT_CHECK_ERR(rv, "mim");
#endif

    _DEINIT_INFO_VERB("qos");
    rv = bcm_esw_qos_detach(unit);
    _DEINIT_CHECK_ERR(rv, "qos");

    _DEINIT_INFO_VERB("switch");
    rv = _bcm_esw_switch_detach(unit);
    _DEINIT_CHECK_ERR(rv, "switch");

#if defined(BCM_TSN_SUPPORT)
    if (soc_feature(unit, soc_feature_tsn)){
        _DEINIT_INFO_VERB("tsn");
        rv = bcm_esw_tsn_detach(unit);
        _DEINIT_CHECK_ERR(rv, "tsn");
    }
#endif /* BCM_TSN_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_virtual_switching) ||
        soc_feature(unit, soc_feature_int_common_init)) {
        _DEINIT_INFO_VERB("common");
        rv = _bcm_common_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "common");
    }
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_KATANA_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    /* Service meter */
    if (soc_feature(unit, soc_feature_global_meter)) {
        _DEINIT_INFO_VERB("gmeter");
        rv = _bcm_esw_global_meter_cleanup(unit);
        _DEINIT_CHECK_ERR(rv, "gmeter");
    }
#endif /*BCM_KATANA_SUPPORT or BCM_TRIUMPH3_SUPPORT */

#if defined(BCM_SABER2_SUPPORT)
    if (soc_feature(unit, soc_feature_sat))
    {
        _DEINIT_INFO_VERB("sat");
        rv = bcm_sb2_sat_detach(unit);
        _DEINIT_CHECK_ERR(rv, "sat");

        _DEINIT_INFO_VERB("sat common");
        rv = bcm_common_sat_detach(unit);
        _DEINIT_CHECK_ERR(rv, "sat common");
    }
#endif

#if defined(BCM_RCPU_SUPPORT) && defined(BCM_XGS3_SWITCH_SUPPORT)
    _DEINIT_INFO_VERB("rcpu");
    rv = _bcm_esw_rcpu_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "rcpu");
#endif /* BCM_RCPU_SUPPORT && BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_TOMAHAWK_SUPPORT)
    _DEINIT_INFO_VERB("latency");
    rv = bcm_esw_switch_latency_deinit(unit);
    _DEINIT_CHECK_ERR(rv, "latency");
#endif

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "bcm_detach: All modules deinitialized.\n")));
    
    BCM_UNLOCK(unit);
      /* LOCK deinit should not be performed only during the final detach */
    if (!(BCM_CONTROL(unit)->attach_state == _bcmControlStateDeinitLateTxRx)) {
        _bcm_lock_deinit(unit);
    }

    return rv;
}

#define _THREAD_STOP_CHECK_ERR(_rv, _mod) \
    if (_rv != BCM_E_NONE && _rv != BCM_E_UNAVAIL) { \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "Warning: Stopping %s thread returned %d\n"), \
                  _mod, _rv)); \
    }

/*
 * Function:
 *      _bcm_esw_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit. 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_esw_threads_shutdown(int unit)
{
    int rv;     /* Operation return status. */

    rv = _bcm_esw_port_mon_stop(unit);
    _THREAD_STOP_CHECK_ERR(rv, "portmon");
#ifdef BCM_TRIUMPH3_SUPPORT
    if SOC_IS_TRIUMPH3(unit) {
        rv = _bcm_esw_ibod_sync_recovery_stop(unit);
        _THREAD_STOP_CHECK_ERR(rv, "ibod sync");
    }
#endif /* BCM_TRIUMPH3_SUPPORT */
    rv = bcm_esw_linkscan_enable_set(unit, 0);
    _THREAD_STOP_CHECK_ERR(rv, "linkscan");

#ifdef SW_AUTONEG_SUPPORT
    if (soc_feature(unit, soc_feature_sw_autoneg))  {
        rv = bcm_sw_an_enable_set(unit, 0);
        _THREAD_STOP_CHECK_ERR(rv, "SW_AN");
    }    
#endif
    
#ifdef BCM_XGS_SWITCH_SUPPORT
    rv = soc_l2x_stop(unit);
    _THREAD_STOP_CHECK_ERR(rv, "l2x");
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#ifdef BCM_TRIUMPH3_SUPPORT
    if SOC_IS_TRIUMPH3(unit) {
        rv = soc_tr3_l2_bulk_age_stop(unit);
        _THREAD_STOP_CHECK_ERR(rv, "l2 age");
    }
#endif /* BCM_TRIUMPH3_SUPPORT */
#ifdef BCM_TRIDENT2_SUPPORT
    if SOC_IS_TRIDENT2X(unit) {
        rv = soc_td2_l2_bulk_age_stop(unit);
        _THREAD_STOP_CHECK_ERR(rv, "l2 age");
    }
#endif /* BCM_TRIDENT2_SUPPORT */
    rv = soc_counter_detach(unit);
    _THREAD_STOP_CHECK_ERR(rv, "counter");

    return BCM_E_NONE;
}

#ifdef INCLUDE_TCL
#define _BCM_MOD_INIT_CER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: (%s)\n"),               \
                                shr_bprof_stats_name(dispname)));           \
        stime = sal_time_usecs();                                           \
        SHR_BPROF_STATS_TIME(dispname) {                                    \
            rv = rtn(unit);                                                 \
            BCM_CHECK_ERROR_RETURN(rv, dispname);                                     \
        }                                                                   \
        if (SOC_WARM_BOOT(unit)) {                                          \
            LOG_CLI((BSL_META_U(unit, \
                                "bcm_init: %8s   took %10d usec\n"),                  \
                     shr_bprof_stats_name(dispname),                      \
                     SAL_USECS_SUB(sal_time_usecs(), stime)));             \
        } else {                                                            \
            LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                        (BSL_META_U(unit, \
                                    "bcm_init: %8s   took %10d usec\n"),                 \
                         shr_bprof_stats_name(dispname),                     \
                         SAL_USECS_SUB(sal_time_usecs(), stime)));            \
        }                                                                   \
    } else {                                                                \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "bcm_init: skipped %s init\n"),         \
                             shr_bprof_stats_name(dispname)));                           \
    }                                                                       \
}
#else
#define _BCM_MOD_INIT_CER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: (%s)\n"),               \
                                shr_bprof_stats_name(dispname)));           \
        stime = sal_time_usecs();                                           \
        SHR_BPROF_STATS_TIME(dispname) {                                    \
            rv = rtn(unit);                                                 \
            BCM_CHECK_ERROR_RETURN(rv, dispname);                                     \
        }                                                                   \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: %8s   took %10d usec\n"),\
                                shr_bprof_stats_name(dispname),                     \
                     SAL_USECS_SUB(sal_time_usecs(), stime)));            \
    } else {                                                                \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "bcm_init: skipped %s init\n"),                \
                             shr_bprof_stats_name(dispname)));                    \
    }                                                                       \
}
#endif

#ifdef INCLUDE_TCL
#define _BCM_MOD_INIT_IER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: (%s)\n"),               \
                                shr_bprof_stats_name(dispname)));                        \
        stime = sal_time_usecs();                                           \
        SHR_BPROF_STATS_TIME(dispname) {                                    \
            rv = rtn(unit);                                                 \
            BCM_IF_ERROR_RETURN(rv);                                        \
        }                                                                   \
        if (SOC_WARM_BOOT(unit)) {                                          \
            LOG_CLI((BSL_META_U(unit, \
                                "bcm_init: %8s   took %10d usec\n"),                  \
                     shr_bprof_stats_name(dispname),                      \
                     SAL_USECS_SUB(sal_time_usecs(), stime)));             \
        } else {                                                            \
            LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                        (BSL_META_U(unit, \
                                    "bcm_init: %8s   took %10d usec\n"),                 \
                         shr_bprof_stats_name(dispname),                     \
                         SAL_USECS_SUB(sal_time_usecs(), stime)));            \
        }                                                                   \
    } else {                                                                \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "bcm_init: skipped %s init\n"),         \
                             shr_bprof_stats_name(dispname)));                    \
    }                                                                       \
}
#else
#define _BCM_MOD_INIT_IER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: (%s)\n"),               \
                                shr_bprof_stats_name(dispname)));           \
        stime = sal_time_usecs();                                           \
        SHR_BPROF_STATS_TIME(dispname) {                                    \
            rv = rtn(unit);                                                 \
            BCM_IF_ERROR_RETURN(rv);                                        \
        }                                                                   \
        LOG_VERBOSE(BSL_LS_BCM_COMMON, \
                    (BSL_META_U(unit, \
                                "bcm_init: %8s   took %10d usec\n"),\
                                shr_bprof_stats_name(dispname),                     \
                     SAL_USECS_SUB(sal_time_usecs(), stime)));            \
    } else {                                                                \
        LOG_WARN(BSL_LS_BCM_INIT, \
                 (BSL_META_U(unit, \
                             "bcm_init: skipped %s init\n"),                \
                             shr_bprof_stats_name(dispname)));                    \
    }                                                                       \
}
#endif

/*
 * Function:
 *	_bcm_modules_init
 * Purpose:
 * 	Initialize bcm modules
 * Parameters:
 *	unit - StrataSwitch unit #.
 *      flags - Combination of bit selectors (see init.h)
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
_bcm_modules_init(int unit)
{
    int init_cond, init_cond2; /* init condition */
    SHR_BPROF_STATS_DECL;
    /*
     * Initialize each bcm module
     */

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    /* Only Tx and Rx modules will be initialized if attaching early
     * during warmboot to minimize CPU TX/RX inactivity time.
     */
    if (BCM_CONTROL(unit)->attach_state == _bcmControlStateTxRxInit) {
#ifdef INCLUDE_KNET
        _BCM_MOD_INIT_CER(unit, bcm_esw_knet_init, SHR_BPROF_BCM_KNET, TRUE);
#endif
        _BCM_MOD_INIT_CER(unit, bcm_esw_tx_init, SHR_BPROF_BCM_TX, TRUE);
        _BCM_MOD_INIT_CER(unit, bcm_esw_rx_init, SHR_BPROF_BCM_RX, TRUE);

        return BCM_E_NONE;
    }
    
    /* Must call mbcm init first to ensure driver properly installed */
    BCM_IF_ERROR_RETURN(mbcm_init(unit));

#if defined(BCM_WARM_BOOT_SUPPORT)    
    if (SOC_WARM_BOOT(unit) && !SOC_IS_XGS12_FABRIC(unit)) {
        /* Init local module id. */
        BCM_IF_ERROR_RETURN(bcm_esw_reload_stk_my_modid_get(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* When adding new modules, double check init condition 
     *  TRUE      - init the module always
     *  init_cond - Conditional init based on boot flags/soc properties
     */
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_virtual_switching) ||
        soc_feature(unit, soc_feature_gport_service_counters) ||
        soc_feature(unit, soc_feature_fast_init) ||
        soc_feature(unit, soc_feature_int_common_init)){
        /* Initialize the common data module here to avoid multiple
         * initializations in the required modules. */
        _BCM_MOD_INIT_CER(unit, _bcm_common_init, SHR_BPROF_BCM_COMMON, TRUE);
    }

#endif /* BCM_TRIUMPH_SUPPORT */
    _BCM_MOD_INIT_CER(unit, bcm_esw_port_init, SHR_BPROF_BCM_PORT, TRUE);
    /* switch to default miim intr mode from polling mode after port init done */
    SOC_CONTROL(unit)->miimIntrEnb = soc_property_get(unit,
            spn_MIIM_INTR_ENABLE, 1);
    init_cond = (!(SAL_BOOT_SIMULATION && 
                   soc_property_get(unit, spn_SKIP_L2_VLAN_INIT, 0)));
    _BCM_MOD_INIT_CER(unit, bcm_esw_l2_init, SHR_BPROF_BCM_L2, init_cond);
    _BCM_MOD_INIT_CER(unit, bcm_esw_stg_init, SHR_BPROF_BCM_STG, TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_vlan_init, SHR_BPROF_BCM_VLAN, init_cond);
    init_cond  = soc_property_get(unit, "init_all_modules", 
                                  (SAL_BOOT_BCMSIM) || (!SAL_BOOT_SIMULATION));
    _BCM_MOD_INIT_CER(unit, bcm_esw_trunk_init, SHR_BPROF_BCM_TRUNK, init_cond);
    init_cond2 = init_cond || (soc_property_get(unit, "skip_cosq_init", 
                                                0) ? FALSE : TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_cosq_init, SHR_BPROF_BCM_COSQ, init_cond2);
    _BCM_MOD_INIT_CER(unit, bcm_esw_mcast_init, SHR_BPROF_BCM_MCAST, init_cond);
    _BCM_MOD_INIT_CER(unit, bcm_esw_linkscan_init, SHR_BPROF_BCM_LINKSCAN, TRUE);

#ifdef SW_AUTONEG_SUPPORT
    if (soc_feature(unit, soc_feature_sw_autoneg)) {
        _BCM_MOD_INIT_CER(unit, bcm_sw_an_module_init, SHR_BPROF_BCM_SW_AN, TRUE);
    }
#endif    

#if defined(BCM_TRIUMPH3_SUPPORT)
    if (SOC_IS_TRIUMPH3(unit) && !SOC_IS_HELIX4(unit) &&
        (_tr3_port_config_id[unit] == 112 || _tr3_port_config_id[unit] == 12)) {
        _bcm_esw_tr3_port_lanes_init(unit);
    }
#endif
    _BCM_MOD_INIT_CER(unit, bcm_esw_stat_init, SHR_BPROF_BCM_STAT, TRUE);
    init_cond2 = init_cond || (soc_property_get(unit, "skip_stack_init", 
                                                0) ? FALSE : TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_stk_init, SHR_BPROF_BCM_STK, init_cond2);
    _BCM_MOD_INIT_CER(unit, _bcm_esw_rate_init, SHR_BPROF_BCM_RATE, init_cond);
#ifdef INCLUDE_KNET
    /* KNET module already initialized during early attach */
    if (BCM_CONTROL(unit)->attach_state != _bcmControlStateTxRxInited) {
        _BCM_MOD_INIT_CER(unit, bcm_esw_knet_init, SHR_BPROF_BCM_KNET, TRUE);
    }
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        if (SOC_MEM_IS_VALID(unit, FP_UDF_TCAMm)) {
            _BCM_MOD_INIT_CER(unit, bcm_esw_udf_init, SHR_BPROF_BCM_UDF, init_cond);
        }
    } else {
        _BCM_MOD_INIT_CER(unit, bcm_esw_udf_init, SHR_BPROF_BCM_UDF, init_cond);
    }
#else
    _BCM_MOD_INIT_CER(unit, bcm_esw_udf_init, SHR_BPROF_BCM_UDF, init_cond);
#endif
#ifdef BCM_FIELD_SUPPORT
#ifdef BCM_TOMAHAWK_SUPPORT
        if (SOC_IS_TOMAHAWKX(unit)) {
            if (SOC_MEM_IS_VALID(unit, IFP_RANGE_CHECKm)) {
                _BCM_MOD_INIT_IER(unit, bcm_esw_range_init, SHR_BPROF_BCM_RANGE, init_cond);
            }
        }
#endif
    if (!SOC_IS_SHADOW(unit)) {
        if (soc_feature(unit, soc_feature_field)) {
            _BCM_MOD_INIT_IER(unit, bcm_esw_field_init, SHR_BPROF_BCM_FIELD, init_cond);
        }
    }
#endif
    init_cond2 = init_cond || (soc_property_get(unit, "skip_mirror_init", 
                                                0) ? FALSE : TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_mirror_init, SHR_BPROF_BCM_MIRROR, init_cond2);
    /* TX module already initialized during early attach */
    if (BCM_CONTROL(unit)->attach_state != _bcmControlStateTxRxInited) {
        _BCM_MOD_INIT_CER(unit, bcm_esw_tx_init, SHR_BPROF_BCM_TX, TRUE);
    }

    /* RX init is done in phases during early attach */
    _BCM_MOD_INIT_CER(unit, bcm_esw_rx_init, SHR_BPROF_BCM_RX, TRUE);

#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_failover)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_failover_init, SHR_BPROF_BCM_FAILOVER, init_cond);
    }
#endif

#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_l3) && soc_property_get(unit, spn_L3_ENABLE, 1)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_l3_init, SHR_BPROF_BCM_L3, init_cond);
    }
#ifdef BCM_HURRICANE3_SUPPORT
    else if (!soc_feature(unit, soc_feature_l3) &&
        soc_feature(unit, soc_feature_miml_no_l3)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_l3_init, SHR_BPROF_BCM_L3, init_cond);
    }
#endif
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_ipmc_init, SHR_BPROF_BCM_IPMC, init_cond);
    }

#ifdef BCM_MPLS_SUPPORT
    if (soc_feature(unit, soc_feature_mpls)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_mpls_init, SHR_BPROF_BCM_MPLS, init_cond);
    }
#endif
    if (soc_feature(unit, soc_feature_mim)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_mim_init, SHR_BPROF_BCM_MIM, init_cond);
    }
    if (soc_feature(unit, soc_feature_wlan)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_wlan_init, SHR_BPROF_BCM_WLAN, init_cond);
    }
    _BCM_MOD_INIT_IER(unit, bcm_esw_proxy_init, SHR_BPROF_BCM_PROXY, init_cond);
#endif
/* KT2 doesn't need L3 support for Coe/Linkphy subport initialization
 * whereas L3 support is needed for traditional subport initialization
 */
#if defined(INCLUDE_L3) || defined(BCM_KATANA2_SUPPORT)
    if (soc_feature(unit, soc_feature_subport) ||
        soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe) ||
        soc_feature(unit, soc_feature_hgproxy_subtag_coe)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_subport_init, SHR_BPROF_BCM_SUBPORT, init_cond);
    }
#endif /* INCLUDE_L3 || BCM_KATANA2_SUPPORT */
    if (soc_feature(unit, soc_feature_qos_profile)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_qos_init, SHR_BPROF_BCM_QOS, init_cond);
    }
#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_trill)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_trill_init, SHR_BPROF_BCM_TRILL, init_cond);
    }
    if (soc_feature(unit, soc_feature_niv)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_niv_init, SHR_BPROF_BCM_NIV, init_cond);
    }
    if (soc_feature(unit, soc_feature_l2gre)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_l2gre_init, SHR_BPROF_BCM_L2GRE, init_cond);
    }
    if (soc_feature(unit, soc_feature_vxlan)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_vxlan_init, SHR_BPROF_BCM_VXLAN, init_cond);
    }
    if (soc_feature(unit, soc_feature_port_extension)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_extender_init, SHR_BPROF_BCM_EXTENDER, init_cond);
    }

    /* This must be after the modules upon which it is build, in order
     * for Warm Boot to operate correctly. */
    _BCM_MOD_INIT_IER(unit, bcm_esw_multicast_init, SHR_BPROF_BCM_MULTICAST, init_cond);
#endif
#ifdef BCM_FIELD_SUPPORT
    if (soc_feature(unit, soc_feature_field)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_auth_init, SHR_BPROF_BCM_AUTH, init_cond);
    }
#endif
#ifdef INCLUDE_REGEX
    if (soc_feature(unit, soc_feature_regex)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_regex_init, SHR_BPROF_BCM_REGEX, TRUE);
    }
#endif
    if (soc_feature(unit, soc_feature_time_support)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_time_init, SHR_BPROF_BCM_TIME, TRUE);
    }

    if (soc_feature(unit, soc_feature_oam) ||
            soc_feature(unit, soc_feature_fp_based_oam)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_oam_init, SHR_BPROF_BCM_OAM, TRUE);
    }
#ifdef BCM_INSTRUMENTATION_SUPPORT
    if (soc_feature(unit, soc_feature_visibility)) {
        BCM_IF_ERROR_RETURN(bcm_esw_pkt_trace_init(unit));
    }
#endif
#ifdef INCLUDE_PTP
    if (soc_feature(unit, soc_feature_ptp)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_ptp_init, SHR_BPROF_BCM_PTP, init_cond);
    }
#endif
#ifdef INCLUDE_BFD
    if (soc_feature(unit, soc_feature_bfd)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_bfd_init, SHR_BPROF_BCM_BFD, init_cond);
    }
#endif
#if defined(BCM_KATANA_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    /* Service meter */
    if (soc_feature(unit, soc_feature_global_meter)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_global_meter_init, SHR_BPROF_BCM_GLB_METER, TRUE);
    }
#endif /* BCM_KATANA_SUPPORT  or BCM_TRIUMPH3_SUPPORT */
#if defined(BCM_TSN_SUPPORT)
    if (soc_feature(unit, soc_feature_tsn)){
        _BCM_MOD_INIT_CER(unit, bcm_esw_tsn_init, SHR_BPROF_BCM_TSN, TRUE);
    }
#endif /* BCM_TSN_SUPPORT */

/* 
 * No need to call Port downsizer during warmboot
 */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)
#if defined(BCM_WARM_BOOT_SUPPORT)    
        && (!SOC_WARM_BOOT(unit))
#endif /* BCM_WARM_BOOT_SUPPORT */
        ) { 
        BCM_IF_ERROR_RETURN(bcm_kt_port_downsizer_chk_reinit(unit));
    }
#endif

#if defined(BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_fcoe)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_fcoe_init, SHR_BPROF_BCM_FCOE, init_cond);
    }    
#endif
#ifdef BCM_HURRICANE2_SUPPORT
    if (SOC_IS_HURRICANE2(unit)) {
        BCM_IF_ERROR_RETURN(bcm_hr2_dual_port_mode_reinit(unit));
    }
#endif
#if defined(BCM_SABER2_SUPPORT)
    if (soc_feature(unit, soc_feature_sat)) {
        BCM_IF_ERROR_RETURN(bcm_common_sat_init(unit));

        _BCM_MOD_INIT_IER(unit, bcm_sb2_sat_init, SHR_BPROF_BCM_SAT, 
                          init_cond);
    }
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        if (soc_feature(unit, soc_feature_untethered_otp)) {
            BCM_IF_ERROR_RETURN(bcmi_port_force_lb_set(unit));
        } else {
            BCM_IF_ERROR_RETURN(bcm_th_phy_lb_set(unit));
        }
    }
#endif
    return BCM_E_NONE;
}

#if defined(BCM_XGS3_SWITCH_SUPPORT)
#ifdef BCM_WARM_BOOT_SUPPORT

STATIC int
_bcm_mem_scache_init(int unit)
{
    SOC_IF_ERROR_RETURN(soc_mem_cache_scache_init(unit));
    return BCM_E_NONE;
}

int
_bcm_mem_scache_sync(int unit)
{
    SOC_IF_ERROR_RETURN(soc_mem_cache_scache_sync(unit));
    return BCM_E_NONE;
}

STATIC int 
_bcm_switch_control_scache_init(int unit)
{
    return soc_switch_control_scache_init(unit);
}

int
_bcm_switch_control_scache_sync(int unit)
{
    return soc_switch_control_scache_sync(unit);
}


#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* defined(BCM_XGS3_SWITCH_SUPPORT) */

/*
 * Function:
 *	_bcm_esw_init
 * Purpose:
 * 	Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
_bcm_esw_init(int unit)
{

    int warmboot_post_config = FALSE;

    BCM_IF_ERROR_RETURN(_bcm_lock_init(unit));

    /* Skip rest of the initialization if early attaching */
    if (BCM_CONTROL(unit)->attach_state == _bcmControlStateTxRxInit) {
        if(SOC_WARM_BOOT(unit)) {
            return (_bcm_modules_init(unit));
        } else {
            /* Not supported if not warm booting */
            return BCM_E_CONFIG;
        }
    }

    SOC_CONTROL(unit)->soc_flags &= ~SOC_F_ALL_MODULES_INITED;

    /* If linkscan is running, disable it. */
    bcm_esw_linkscan_enable_set(unit, 0);

#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit)); 
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit)); 
#endif /* INCLUDE_FCMAP */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
#ifdef BCM_WARM_BOOT_SUPPORT
    /* switch global warmboot state init/reinit */
    BCM_IF_ERROR_RETURN(_bcm_switch_control_scache_init(unit));

    /* mem cache warmboot state init/reinit */
    BCM_IF_ERROR_RETURN(_bcm_mem_scache_init(unit));

#if defined(BCM_OLP_SUPPORT)
    if (soc_feature(unit, soc_feature_olp)) {
        /* Recover switch olp status before l2 module init */
        BCM_IF_ERROR_RETURN(_bcm_esw_switch_olp_init(unit));
    }
#endif

#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* defined(BCM_XGS3_SWITCH_SUPPORT) */

    BCM_IF_ERROR_RETURN(_bcm_modules_init(unit));

#ifdef BCM_TRIDENT_SUPPORT
    /* Workaround to initialize WRED memories to known state. */
    if ((SOC_IS_TRIDENT(unit) || SOC_IS_TITAN(unit)) &&
        !SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_td_wred_mem_war(unit));
    }
#endif
#ifdef BCM_TRIUMPH3_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
        /* Helix4 XL port XMAC FIFO UR handling */
        if (SOC_IS_HELIX4(unit)) {
            /* coverity[negative_returns] */
            if (!SAL_BOOT_SIMULATION) {
                BCM_IF_ERROR_RETURN(_bcm_tr3_port_ur_chk(unit, -1, -1));
            }
        }
    }
#endif
    
#ifdef BCM_XGS3_SWITCH_SUPPORT
    BCM_IF_ERROR_RETURN(_bcm_esw_switch_init(unit));
#endif

    if (SOC_WARM_BOOT(unit)) {
        warmboot_post_config = TRUE;
        SOC_WARM_BOOT_DONE(unit);
    }

    if (warmboot_post_config == TRUE) {
        /* To avoid acctidently reconfigure hardware, write to register is
           bypassed during the warm boot until SOC_WARM_BOOT_DONE */
        /* for the cases that we do need to reconfigure hardware, we do it here */
#if defined(BCM_TOMAHAWK_SUPPORT)
        if (SOC_IS_TOMAHAWKX(unit)) { 
            if (soc_property_get(unit, "ctr_evict_enable", 1) &&
                (SOC_CONTROL(unit)->counter_interval > 0)) {
                /* Clear all previously accumulated counter value in SW */
                soc_counter_ctr_reset_to_zero(unit);
                /* WB done. Set counter to Evition mode */
                SOC_IF_ERROR_RETURN
                    (soc_ctr_evict_start(unit, 0,
                                         SOC_CONTROL(unit)->counter_interval));
            }
        }
#endif
    }

#if defined(BCM_RCPU_SUPPORT) && defined(BCM_XGS3_SWITCH_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_rcpu_init(unit)); 
#endif /* BCM_RCPU_SUPPORT && BCM_XGS3_SWITCH_SUPPORT */

    SOC_CONTROL(unit)->soc_flags |= SOC_F_ALL_MODULES_INITED;
    if (SOC_HW_RESET(unit)) {
        SOC_HW_RESET_DONE(unit);
    }

    return BCM_E_NONE;
}    

/*
 * Function:
 *	bcm_esw_init
 * Purpose:
 * 	Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_esw_init(int unit)
{
    if (0 == SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    return _bcm_esw_init(unit);
}    

int
bcm_shadow_init(int unit)
{
    return bcm_esw_init(unit);
}    

/*
 * Function:
 *	_bcm_esw_reinit_s3mii_war
 * Purpose:
 * 	Partially ReInitialize the BCM API layer only.
 * 	Used for the SW Workaround for S3MII.
 * 	Presently used in Katana Only.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

int
_bcm_esw_reinit_ka_war(int unit)
{
    BCM_IF_ERROR_RETURN(_bcm_lock_init(unit));
    bcm_esw_linkscan_enable_set(unit, 0);
#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit)); 
#endif /* INCLUDE_MACSEC */
#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit)); 
#endif /* INCLUDE_FCMAP */
    SOC_IF_ERROR_RETURN(_bcm_modules_init(unit));

    return BCM_E_NONE;
}    

/*      _bcm_esw_attach
 * Purpose:
 *      Attach and initialize bcm device
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_esw_attach(int unit, char *subtype)
{
    int  dunit;
    int  rv;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;

    /* Initialize soc layer */
    if ((NULL == SOC_CONTROL(dunit)) || 
        (0 == (SOC_CONTROL(dunit)->soc_flags & SOC_F_ATTACHED))) {
        return (BCM_E_INIT);
    }
    
    if (SAL_THREAD_ERROR == SOC_CONTROL(dunit)->counter_pid) {
        rv = soc_counter_attach(unit);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Initialize bcm layer */
    BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
    BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
    BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);
    if (SOC_IS_XGS_SWITCH(dunit)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH;
    }
    if (SOC_IS_XGS_FABRIC(dunit)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_FABRIC;
    }
    if (soc_feature(dunit, soc_feature_l3)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_L3;
    }
    if (soc_feature(dunit, soc_feature_ip_mcast)) {
        BCM_CONTROL(unit)->capability |=
            BCM_CAPA_IPMC;
    }

    /* Initialize port mappings */
    _bcm_api_xlate_port_init(unit);

    rv = _bcm_esw_init(unit);
    return (rv);
}

int
_bcm_shadow_attach(int unit, char *subtype)
{
    return _bcm_esw_attach(unit, subtype);
}

/*      _bcm_esw_match
 * Purpose:
 *      match BCM control subtype strings for ESW types 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    0 match
 *    !0 no match
 */
int
_bcm_esw_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}

int
_bcm_shadow_match(int unit, char *subtype_a, char *subtype_b)
{
    return _bcm_esw_match(unit, subtype_a, subtype_b);
}

/*
 * Function:
 *	bcm_esw_init_selective
 * Purpose:
 * 	Initialize specific bcm modules as desired.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *    module_number - Indicate module number
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_esw_init_selective(int unit, uint32 module_number)
{
    switch (module_number) {
         case BCM_MODULE_PORT     :
                   BCM_IF_ERROR_RETURN(bcm_esw_port_init(unit));
                   break;
         case BCM_MODULE_L2       :
                   BCM_IF_ERROR_RETURN(bcm_esw_l2_init(unit));
                   break;
         case BCM_MODULE_VLAN     :   
                   BCM_IF_ERROR_RETURN(bcm_esw_vlan_init(unit));
                   break;
         case BCM_MODULE_TRUNK    :
                   BCM_IF_ERROR_RETURN(bcm_esw_trunk_init(unit));
                   break;
         case BCM_MODULE_COSQ     :
                   BCM_IF_ERROR_RETURN(bcm_esw_cosq_init(unit));
                   break;
         case BCM_MODULE_MCAST        :
                   BCM_IF_ERROR_RETURN(bcm_esw_mcast_init(unit));
                   break;
         case BCM_MODULE_LINKSCAN  :
                   BCM_IF_ERROR_RETURN(bcm_esw_linkscan_init(unit));
                   break;
         case BCM_MODULE_STAT     :
                   BCM_IF_ERROR_RETURN(bcm_esw_stat_init(unit));
                   break;
         case BCM_MODULE_MIRROR   :
                   BCM_IF_ERROR_RETURN(bcm_esw_mirror_init(unit));
                   break;
#ifdef INCLUDE_L3
         case BCM_MODULE_L3       :
                   BCM_IF_ERROR_RETURN(bcm_esw_l3_init(unit));
                   BCM_IF_ERROR_RETURN(bcm_esw_proxy_init(unit));
                   break;
         case BCM_MODULE_IPMC 	:
                   BCM_IF_ERROR_RETURN(bcm_esw_ipmc_init(unit));
                   break;
#ifdef BCM_MPLS_SUPPORT
         case BCM_MODULE_MPLS	 :
                   BCM_IF_ERROR_RETURN(bcm_esw_mpls_init(unit));
                   break;
#endif /* BCM_MPLS_SUPPORT */
         case BCM_MODULE_MIM 	 :
                   BCM_IF_ERROR_RETURN(bcm_esw_mim_init(unit));
                   break;
         case BCM_MODULE_SUBPORT	:
                   BCM_IF_ERROR_RETURN(bcm_esw_subport_init(unit));
                   break;
         case BCM_MODULE_WLAN	  :
                   BCM_IF_ERROR_RETURN(bcm_esw_wlan_init(unit));
                   break;
#endif /* INCLUDE_L3 */
         case BCM_MODULE_QOS		:
                   BCM_IF_ERROR_RETURN(bcm_esw_qos_init(unit));
                   break;
         case BCM_MODULE_STACK    :
                   BCM_IF_ERROR_RETURN(bcm_esw_stk_init(unit));
                   break;
         case BCM_MODULE_STG      :
                   BCM_IF_ERROR_RETURN(bcm_esw_stg_init(unit));
                   break;
         case BCM_MODULE_TX       :
                   BCM_IF_ERROR_RETURN(bcm_esw_tx_init(unit));
                   break;
#ifdef BCM_FIELD_SUPPORT
         case BCM_MODULE_AUTH     :
                   BCM_IF_ERROR_RETURN(bcm_esw_auth_init(unit));
                   break;
#endif
         case BCM_MODULE_RX       :
                   BCM_IF_ERROR_RETURN(bcm_esw_rx_init(unit));
                   break;
         case BCM_MODULE_UDF    :
                   BCM_IF_ERROR_RETURN(bcm_esw_udf_init(unit));
                   break;
#ifdef BCM_FIELD_SUPPORT
         case BCM_MODULE_FIELD    :
                   BCM_IF_ERROR_RETURN(bcm_esw_field_init(unit));
                   break;
#endif /* BCM_FIELD_SUPPORT */
         case BCM_MODULE_TIME     :
                   BCM_IF_ERROR_RETURN(bcm_esw_time_init(unit));
                   break;
         case BCM_MODULE_FABRIC   :
                   break;
         case BCM_MODULE_POLICER  :
                   break;
         case BCM_MODULE_OAM      :
                   BCM_IF_ERROR_RETURN(bcm_esw_oam_init(unit));
                   break;
#ifdef  INCLUDE_L3
         case BCM_MODULE_FAILOVER :
                   BCM_IF_ERROR_RETURN(bcm_esw_failover_init(unit));
                   break;
#endif
         case BCM_MODULE_VSWITCH  :
                   break;
#ifdef  INCLUDE_L3
         case BCM_MODULE_MULTICAST:
                   BCM_IF_ERROR_RETURN(bcm_esw_multicast_init(unit));
                   break;
         case BCM_MODULE_TRILL    :
                   BCM_IF_ERROR_RETURN(bcm_esw_trill_init(unit));
                   break;
         case BCM_MODULE_NIV      :
                   BCM_IF_ERROR_RETURN(bcm_esw_niv_init(unit));
                   break;
         case BCM_MODULE_L2GRE    :
                   BCM_IF_ERROR_RETURN(bcm_esw_l2gre_init(unit));
                   break;
         case BCM_MODULE_VXLAN    :
                   BCM_IF_ERROR_RETURN(bcm_esw_vxlan_init(unit));
                   break;
         case BCM_MODULE_EXTENDER :
                   BCM_IF_ERROR_RETURN(bcm_esw_extender_init(unit));
                   break;
#endif
#ifdef INCLUDE_PTP
         case BCM_MODULE_PTP:
                   BCM_IF_ERROR_RETURN(bcm_esw_ptp_init(unit));
                   break;
#endif 
#ifdef INCLUDE_BFD
         case BCM_MODULE_BFD:
                   BCM_IF_ERROR_RETURN(bcm_esw_bfd_init(unit));
                   break;
#endif 
#ifdef INCLUDE_REGEX
         case BCM_MODULE_REGEX :
                   BCM_IF_ERROR_RETURN(bcm_esw_regex_init(unit));
                   break;
#endif /* BCM_MPLS_SUPPORT */
#ifdef BCM_TSN_SUPPORT
         case BCM_MODULE_TSN:
                   BCM_IF_ERROR_RETURN(bcm_esw_tsn_init(unit));
                   break;
#endif /* BCM_TSN_SUPPORT */
         default:
                   return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/*
 * ************* Deprecated API ************
 * Function:
 *	bcm_esw_init_check
 * Purpose:
 *	Return TRUE if bcm_esw_init_bcm has already been called and succeeded
 * Parameters:
 *	unit- StrataSwitch unit #.
 * Returns:
 *	TRUE or FALSE
 */
int
bcm_esw_init_check(int unit)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      _bcm_esw_detach
 * Purpose:
 *      Clean up bcm layer when unit is detached
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
int
_bcm_esw_detach(int unit)
{
    int rv;                    /* Operation return status. */

    /* Shut down all the spawned threads. */
    /* Threads should have been killed during the initial detach */
    if (!(BCM_CONTROL(unit)->attach_state == _bcmControlStateDeinitAll)) {
        rv = _bcm_esw_threads_shutdown(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }

    /* 
     *  Don't move up, holding lock or disabling hw operations 
     *  might prevent theads clean exit.
     */
    BCM_LOCK(unit);

    rv = _bcm_esw_modules_deinit(unit);

    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    return rv;
}

int
_bcm_shadow_detach(int unit)
{
    return _bcm_esw_detach(unit);
}

/*
 * Function:
 *	bcm_esw_info_get
 * Purpose:
 *	Provide unit information to caller
 * Parameters:
 *	unit	- switch device
 *	info	- (OUT) bcm unit info structure
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_esw_info_get(int unit, bcm_info_t *info)
{
    uint16 dev_id = 0;
    uint8 rev_id = 0;

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
    if (SOC_IS_XGS_FABRIC(unit) || SOC_IS_XGS_FABRIC_TITAN(unit)) {
	info->capability |= BCM_INFO_FABRIC;
    } else {
	info->capability |= BCM_INFO_SWITCH;
    }
    if (soc_feature(unit, soc_feature_l3)) {
	info->capability |= BCM_INFO_L3;
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
	info->capability |= BCM_INFO_IPMC;
    }
    info->num_pipes = NUM_PIPE(unit);
    return BCM_E_NONE;
}


/* ASSUMES unit PARAMETER which is not in macro's list. */
#define CLEAR_CALL(_rtn, _name) {                                       \
        int rv;                                                         \
        rv = (_rtn)(unit);                                              \
        if (rv < 0 && rv != BCM_E_UNAVAIL) {                            \
            LOG_ERROR(BSL_LS_BCM_INIT, \
                      (BSL_META_U(unit, \
                                  "bcm_clear %d: %s failed %d. %s\n"),    \
                                  unit, _name, rv, bcm_errmsg(rv)));              \
            return rv;                                                  \
        }                                                               \
}

/*
 * Function:
 *      bcm_esw_clear
 * Purpose:
 *      Initialize a device without a full reset
 * Parameters:
 *      unit        - The unit number of the device to clear
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      For each module, call the underlying init/clear operation
 */
int
bcm_esw_clear(int unit)
{
    return _bcm_esw_init(unit);
}


