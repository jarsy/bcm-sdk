/*
 * $Id: port.c,v 1.148 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.c
 * Purpose:     Tracks and manages ports.
 *              P-VLAN table is managed directly.
 *              MAC/PHY interfaces are managed through respective drivers.
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/sbFabCommon.h>
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/qe2000.h>
#endif
#include <bcm_int/sbx/mbcm.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/link.h>
#include <bcm/vlan.h>
#include <bcm/rate.h>
#include <bcm/stg.h>
#include <bcm/stack.h>
#include <bcm/module.h>

#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>
#include <soc/phy.h>
#include <soc/phyctrl.h>
#include <soc/phyreg.h>
#ifdef BCM_QE2000_SUPPORT
#include <bcm_int/sbx/qe2000.h>
#endif
#ifdef BCM_BME3200_SUPPORT
#include <bcm_int/sbx/bm3200.h>
#endif
#ifdef BCM_BM9600_SUPPORT
#include <bcm_int/sbx/bm9600.h>
#endif
#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/cosq.h>
#endif
#include <bcm_int/sbx_dispatch.h>

static bcm_sbx_port_shaper_state_t *shaper_state[SOC_MAX_NUM_DEVICES];
static bcm_sbx_port_state_t *port_state[SOC_MAX_NUM_DEVICES];
/* This points to an array of per node abilities for the given logical crossbar */
static uint8 *ability[SOC_MAX_NUM_DEVICES][SB_FAB_DEVICE_MAX_PHYSICAL_SERDES];

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * General Utility Macros
 *    Warning: Don't define macros which returns, it's error-prone when locks are used
 */
#define SBX_PORT(unit, port)    (port_state[unit]->port_info[port])

#define UNIT_INIT_DONE(unit)    ((port_state[unit] != NULL) && (port_state[unit]->init == TRUE))

#ifdef BCM_QE2000_SUPPORT
static int
_bcm_sbx_port_shaper_state_init(int unit);
#endif

static int
_bcm_sbx_port_egress_shaperid_get(int unit, bcm_port_t port, int * nShaperId);

static int
_bcm_sbx_port_check_egress_shapers(int unit,
                                   bcm_port_t port,
                                   int nShaperType,
                                   uint32 uShaperSrc,
                                   int * nShaperId,
                                   int  * uShaperConflicts,
                                   uint32 * uMatchControl);

#ifdef BCM_WARM_BOOT_SUPPORT
static int
_bcm_sbx_wb_port_state_init(int unit);
#endif


int
bcm_sbx_port_enable_set(int unit,
                        bcm_port_t port,
                        int enable)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_enable_set, (unit, port, enable));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port enable set failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_enable_get(int unit,
                        bcm_port_t port,
                        int *enable)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_enable_get, (unit, port, enable));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port enable get failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_init(int unit)
{
    int rv = BCM_E_NONE;
    int ability_num_entries;
    int sfi_port;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    /* allocate port state */
    if (port_state[unit] != NULL) {
        if (port_state[unit]->port_info != NULL) {
            sal_free(port_state[unit]->port_info);
        }
        sal_free(port_state[unit]);
        port_state[unit] = NULL;
    }

    if (port_state[unit] == NULL) {
        port_state[unit] = sal_alloc(sizeof(bcm_sbx_port_state_t),
                                        "port_state");
        if (port_state[unit] == NULL) {
            return BCM_E_MEMORY;
        }

        /* per port info is allocated by the mcm layer */
    }
    sal_memset(port_state[unit], 0, sizeof(bcm_sbx_port_state_t));
    SOC_SBX_STATE(unit)->port_state = port_state[unit];

    if (!soc_feature(unit, soc_feature_standalone)) {

        /* Allocate the logical to physical crossbar memory area */
        ability_num_entries = sizeof(uint8) * SB_FAB_DEVICE_BM9600_MAX_NODES;

        for (sfi_port = 0; sfi_port < SB_FAB_DEVICE_MAX_PHYSICAL_SERDES; sfi_port++) {

            if (ability[unit][sfi_port] != NULL) {
                sal_free(ability[unit][sfi_port]);
            }
            ability[unit][sfi_port] = sal_alloc(ability_num_entries, "port_ability");

            if (ability[unit][sfi_port] == NULL) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, sal_alloc ability,  Unit(%d)\n"),
                           FUNCTION_NAME(), unit));
                rv = BCM_E_MEMORY;
                return rv;
            }

            sal_memset(ability[unit][sfi_port], BCM_PORT_ABILITY_DUAL_SFI, ability_num_entries);

            SOC_SBX_STATE(unit)->port_state->ability[sfi_port] = ability[unit][sfi_port];
        }
    }

    BCM_SBX_LOCK(unit);

#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
        _bcm_sbx_port_shaper_state_init(unit);
    }
#endif

#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE(unit)) {
        port_state[unit]->init = TRUE;
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit)) {
        port_state[unit]->init = TRUE;
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }
#endif

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_init, (unit));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port init failed error(%d)\n"),
                   rv));
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    rv = _bcm_sbx_wb_port_state_init(unit);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: error in WarmBoot port state init \n"), 
                   FUNCTION_NAME()));
    }
#endif

    port_state[unit]->init = TRUE;

    BCM_SBX_UNLOCK(unit);
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/* This function initializes the WB support for port module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the fabric state from stable cache
 * 
 *  
 * bcm_sbx_port_state_t *port_state
 *                 init
 *                 ability (96 * 72)
 *                 uPrbsModeSi
 *                 uPrbsForceTxError (96 serdes max)
 *                 uDriverEqualizationFarEnd (96 serdes max)
 *                 uDriverPriorPostCursorNegative (96 serdes max)
 *                 cpu_fabric_port
 *                 fabric_header_format
 *                 *port_info     (SS only)
 *                 *subport_info  (SS only)
 * 
 * bcm_sbx_port_shaper_state_t *shaper_state[unit]
 *                   0-3   is_free
 *                   4-7   shaper_type
 *                   8-11  shaper_src
 *                   12-15 hi_side
 *                   16-19 port
 *
 * ability[MAX_DEVICES 18][SB_FAB_DEVICE_MAX_PHYSICAL_SERDES 96]
 *               
 */
static int
_bcm_sbx_wb_port_state_init(int unit)
{
    int                     rv = BCM_E_NONE;
    uint32                  scache_len;
    int                     stable_size;
    uint8                   *scache_ptr = NULL;
    uint8                   *ptr, *end_ptr;
    bcm_sbx_port_state_t   *p_ps;
    bcm_sbx_port_shaper_state_t *p_shaper;
    int nSi, xbar, node, port, eg, cosq, mask = 0;
    int nShaperId;
    soc_scache_handle_t scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;

    p_ps = SOC_SBX_STATE(unit)->port_state;
    if (p_ps == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid port state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }
    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_PORT, 0);

    if (SOC_WARM_BOOT(unit)) {

        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "In warm boot, recover SBX port state from external memory cache unit(%d)\n"),
                     unit));

        /* If device is during warm-boot, recover the state from scache */
        rv = soc_versioned_scache_ptr_get(unit, scache_handle, FALSE,
                                          &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            return rv;
        }

        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "In warm boot, cache ptr(0x%08x) length(%d)\n"),
                     (uint32)scache_ptr, scache_len));

        if (SOC_IS_SBX_QE2000(unit)) {
            if (shaper_state[unit] == NULL) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Memory for egress port shaper state is not allocated for unit(%d). \n"), 
                           FUNCTION_NAME(), unit));
                return BCM_E_MEMORY;
            }
        }

    } else {
        /* During cold-boot. Allocate a stable cache */
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Cold boot, allocate stable cache\n")));
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/

    /* now de-compress to port state */
    __WB_DECOMPRESS_SCALAR(uint32, p_ps->init);
    
    if (!soc_feature(unit, soc_feature_standalone)) {
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_PHYSICAL_SERDES; xbar++) {
            if (p_ps->ability[xbar] == NULL) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Memory for port ability of xbar %d is not allocated for unit(%d).\n"), 
                           FUNCTION_NAME(), xbar, unit));
                return BCM_E_MEMORY;
            }
            for (node=0; node<SB_FAB_DEVICE_BM9600_MAX_NODES; node++) {
                __WB_DECOMPRESS_SCALAR(uint8, p_ps->ability[xbar][node]);
            }
        }
    }
    
    __WB_DECOMPRESS_SCALAR(uint32, p_ps->uPrbsModeSi);
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_DECOMPRESS_SCALAR(uint8, p_ps->uPrbsForceTxError[nSi]);
    }
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_DECOMPRESS_SCALAR(uint32, p_ps->uDriverEqualizationFarEnd[nSi]);
    }
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_DECOMPRESS_SCALAR(uint32, p_ps->uDriverPriorPostCursorNegative[nSi]);
    }
    
    __WB_DECOMPRESS_SCALAR(uint32, p_ps->cpu_fabric_port);
    
    __WB_DECOMPRESS_SCALAR(uint8, p_ps->fabric_header_format);
    
    if (SOC_IS_SBX_QE2000(unit)) {
        p_shaper = shaper_state[unit];

        for (nShaperId=0; nShaperId<SOC_SBX_CFG(unit)->nShaperCount; nShaperId++) {
            
            __WB_DECOMPRESS_SCALAR(uint8, p_shaper[nShaperId].is_free);
            
            if (p_shaper[nShaperId].is_free == FALSE) {
                __WB_DECOMPRESS_SCALAR(uint32, p_shaper[nShaperId].shaper_type);
                __WB_DECOMPRESS_SCALAR(uint32, p_shaper[nShaperId].shaper_src);
                __WB_DECOMPRESS_SCALAR(uint32, p_shaper[nShaperId].hi_side);
                __WB_DECOMPRESS_SCALAR(uint32, p_shaper[nShaperId].port);
            }
        }
    } else if (SOC_IS_SIRIUS(unit)) {
        /*  bcm_sbx_port_info_t
         *    only need to store flags
         */
        for (port=0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_DECOMPRESS_SCALAR(uint32, p_ps->port_info[port].flags);
        }
        
        /*  bcm_sbx_subport_info_t
         */
        for (port=0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
            __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].valid);
            if (!SOC_WARM_BOOT(unit) || (p_ps->subport_info[port].valid == TRUE)) {
                __WB_DECOMPRESS_SCALAR(uint32, p_ps->subport_info[port].flags);
                __WB_DECOMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].parent_gport);
                __WB_DECOMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].original_gport);
                __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].group_shaper);
                if (p_ps->subport_info[port].group_shaper == 0xFF) {
                    p_ps->subport_info[port].group_shaper = -1;
                }
                __WB_DECOMPRESS_SCALAR(int, p_ps->subport_info[port].port_offset);
                __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].ts_scheduler_level);
                if (p_ps->subport_info[port].ts_scheduler_level == 0xFF) {
                    p_ps->subport_info[port].ts_scheduler_level = -1;
                }
                __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].ts_scheduler_node);
                if (p_ps->subport_info[port].ts_scheduler_node == 0xFF) {
                    p_ps->subport_info[port].ts_scheduler_node = -1;
                }
                __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].es_scheduler_level2_node);
                if (p_ps->subport_info[port].es_scheduler_level2_node == 0xFF) {
                    p_ps->subport_info[port].es_scheduler_level2_node = -1;
                }
                for (eg=0; eg < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; eg++) {
                    __WB_DECOMPRESS_SCALAR(uint8, p_ps->subport_info[port].egroup[eg].num_fifos);
                    if (!SOC_WARM_BOOT(unit) || (p_ps->subport_info[port].egroup[eg].num_fifos != 0)) {
                        __WB_DECOMPRESS_SCALAR(int, p_ps->subport_info[port].es_scheduler_level1_node[eg]);
                        __WB_DECOMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].es_scheduler_level0_node);
                        __WB_DECOMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].ef_fcd);
                        __WB_DECOMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].nef_fcd);
                        __WB_DECOMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].egroup[eg].egroup_gport);
                        /* fcd */
                        __WB_DECOMPRESS_SCALAR(uint16, mask);
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (!SOC_WARM_BOOT(unit) || (mask & (1<<cosq))) {
                                __WB_DECOMPRESS_SCALAR(uint32, p_ps->subport_info[port].egroup[eg].fcd[cosq]);
                            } else {
                                p_ps->subport_info[port].egroup[eg].fcd[cosq] = -1;
                            }
                        }
                        __WB_DECOMPRESS_SCALAR(uint16, mask);
                        /* port speed */
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (!SOC_WARM_BOOT(unit) || (mask & (1 << cosq))) {
                                __WB_DECOMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].port_speed[cosq]);
                            } else {
                                p_ps->subport_info[port].egroup[eg].port_speed[cosq] = 0;                            }
                        }
                        __WB_DECOMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].port_speed[cosq]);
                    }
                }
            }
        }
        
#if defined(BCM_SIRIUS_SUPPORT)
        /* Need to restore congestion_info table */
        rv = _bcm_sirius_wb_port_state_sync(unit, &scache_len, &ptr, &end_ptr, (SOC_WARM_BOOT(unit) ? _WB_OP_DECOMPRESS : _WB_OP_SIZE));
        if (rv != BCM_E_NONE) {
            return rv;
        }
#endif
        
        /*  
         *  Only need to restore phy_flags from soc_phy_info_t
         */
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_DECOMPRESS_SCALAR(uint32, phy_port_info[unit][port].phy_flags);
        }   
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /*  
         *  Only need to restore phy_flags from soc_phy_info_t
         */
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_DECOMPRESS_SCALAR(uint32, phy_port_info[unit][port].phy_flags);
        }
    }
    
    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    if (!SOC_WARM_BOOT(unit)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Cold boot,  handle(0x%x) cache length(%d) bytes\n"),
                     scache_handle, scache_len));

        rv = soc_versioned_scache_ptr_get(unit, scache_handle, TRUE,
                                          &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) allocating WB scache handle on unit:%d \n"),
                       FUNCTION_NAME(), soc_errmsg(rv), unit));
            return rv;
        }
    }

    return rv;
}

/* This function takes bcm_sbx_port_state_t and stores it
 * to stable memory.
 * Input param: sync --> indicates whether to sync scache to Persistent memory
 */
int
bcm_sbx_wb_port_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr;
    uint8                   *ptr, *end_ptr;
    uint32                  scache_len;
    int                     stable_size;
    int                     rv;
    bcm_sbx_port_state_t    *p_ps;
    int                     xbar, nSi, node, port, eg, cosq, mask;
    bcm_sbx_port_shaper_state_t *p_shaper;
    int nShaperId;
    soc_scache_handle_t scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Sync SBX PORT data to cache unit(%d)\n"),
                 unit));

    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }

    p_ps = SOC_SBX_STATE(unit)->port_state;
    if (p_ps == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_PORT, 0);

    rv = soc_versioned_scache_ptr_get(unit, scache_handle, FALSE,
                                      &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                   FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
        return rv;
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr+scache_len;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "sync, cache ptr(0x%08x) length(%d)\n"),
                 (uint32)scache_ptr, scache_len));

    /* now compress and store in the scache location */
    __WB_COMPRESS_SCALAR(uint32, p_ps->init);

    if (!soc_feature(unit, soc_feature_standalone)) {
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_PHYSICAL_SERDES; xbar++) {
            if (p_ps->ability[xbar] == NULL) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Memory for port ability of xbar %d is not allocated for unit(%d).\n"), 
                           FUNCTION_NAME(), xbar, unit));
                return BCM_E_MEMORY;
            }
            for (node=0; node<SB_FAB_DEVICE_BM9600_MAX_NODES; node++) {
                __WB_COMPRESS_SCALAR(uint8, p_ps->ability[xbar][node]);
            }
        }
    }

    __WB_COMPRESS_SCALAR(uint32, p_ps->uPrbsModeSi);
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_COMPRESS_SCALAR(uint8, p_ps->uPrbsForceTxError[nSi]);
    }
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_COMPRESS_SCALAR(uint32, p_ps->uDriverEqualizationFarEnd[nSi]);
    }
    
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        __WB_COMPRESS_SCALAR(uint32, p_ps->uDriverPriorPostCursorNegative[nSi]);
    }
    
    __WB_COMPRESS_SCALAR(uint32, p_ps->cpu_fabric_port);
    
    __WB_COMPRESS_SCALAR(uint8, p_ps->fabric_header_format);
    
    if (SOC_IS_SBX_QE2000(unit)) {
        p_shaper = shaper_state[unit];

        for (nShaperId=0; nShaperId<SOC_SBX_CFG(unit)->nShaperCount; nShaperId++) {

            __WB_COMPRESS_SCALAR(uint8, p_shaper[nShaperId].is_free);

            if (p_shaper[nShaperId].is_free == FALSE) {
                __WB_COMPRESS_SCALAR(uint32, p_shaper[nShaperId].shaper_type);
                __WB_COMPRESS_SCALAR(uint32, p_shaper[nShaperId].shaper_src);
                __WB_COMPRESS_SCALAR(uint32, p_shaper[nShaperId].hi_side);
                __WB_COMPRESS_SCALAR(uint32, p_shaper[nShaperId].port);
            }
        }
    } else if (SOC_IS_SIRIUS(unit)) {
        /*  bcm_sbx_port_info_t
         *    only need to store flags
         */
        for (port=0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_COMPRESS_SCALAR(uint32, p_ps->port_info[port].flags);
        }

        /*  bcm_sbx_subport_info_t
         */
        for (port=0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
            __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].valid);
            if (p_ps->subport_info[port].valid == TRUE) {
                __WB_COMPRESS_SCALAR(uint32, p_ps->subport_info[port].flags);
                __WB_COMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].parent_gport);
                __WB_COMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].original_gport);
                if (p_ps->subport_info[port].group_shaper == -1) {
                    __WB_COMPRESS_SCALAR(uint8,0xFF);
                } else {
                    __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].group_shaper);
                }
                __WB_COMPRESS_SCALAR(int, p_ps->subport_info[port].port_offset);
                if (p_ps->subport_info[port].ts_scheduler_level == -1) {
                    __WB_COMPRESS_SCALAR(uint8,0xFF);
                } else {
                    __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].ts_scheduler_level);
                }
                if (p_ps->subport_info[port].ts_scheduler_node == -1) {
                    __WB_COMPRESS_SCALAR(uint8,0xFF);
                } else {
                    __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].ts_scheduler_node);
                }
                if (p_ps->subport_info[port].es_scheduler_level2_node == -1) {
                    __WB_COMPRESS_SCALAR(uint8,0xFF);
                } else {
                    __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].es_scheduler_level2_node);
                }
                
                for (eg=0; eg < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; eg++) {
                    __WB_COMPRESS_SCALAR(uint8, p_ps->subport_info[port].egroup[eg].num_fifos);
                    if (p_ps->subport_info[port].egroup[eg].num_fifos != 0) {
                        __WB_COMPRESS_SCALAR(int, p_ps->subport_info[port].es_scheduler_level1_node[eg]);
                        __WB_COMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].es_scheduler_level0_node);
                        __WB_COMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].ef_fcd);
                        __WB_COMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].nef_fcd);
                        __WB_COMPRESS_SCALAR(bcm_gport_t, p_ps->subport_info[port].egroup[eg].egroup_gport);
                        /* fcd */
                        mask = 0;
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (p_ps->subport_info[port].egroup[eg].fcd[cosq] != -1) {
                                mask |= (1<<cosq);
                            }
                        }
                        __WB_COMPRESS_SCALAR(uint16,mask);
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (mask & (1<<cosq)) {
                                __WB_COMPRESS_SCALAR(uint32, p_ps->subport_info[port].egroup[eg].fcd[cosq]);
                            }
                        }
                        /* Port speed */
                        mask = 0;
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (p_ps->subport_info[port].egroup[eg].port_speed[cosq] > 0) {
                                mask |= (1<<cosq);
                            }
                        }                        
                        __WB_COMPRESS_SCALAR(uint16,mask);
                        for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                            if (mask & (1<<cosq)) {
                                __WB_COMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].port_speed[cosq]);
                            }
                        }
                        __WB_COMPRESS_SCALAR(uint16, p_ps->subport_info[port].egroup[eg].port_speed[cosq]);
                    }
                }
            }
        }

#if defined(BCM_SIRIUS_SUPPORT)
        /* Need to save congestion_info table */
        rv = _bcm_sirius_wb_port_state_sync(unit, &scache_len, &ptr, &end_ptr, _WB_OP_COMPRESS);
        if (rv != BCM_E_NONE) {
            return rv;
        }
#endif
        /*  
         *  Store phy_flags to soc_phy_info_t
         */
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_COMPRESS_SCALAR(uint32, phy_port_info[unit][port].phy_flags);
        }
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /*  
         *  Store phy_flags from soc_phy_info_t
         */
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            __WB_COMPRESS_SCALAR(uint32, phy_port_info[unit][port].phy_flags);
        }
    }
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "commit to cache\n")));    
    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) sync'ing scache to Persistent memory. \n"),
                       FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
    }

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_port_sw_dump(int unit)
{
    bcm_sbx_port_state_t    *p_ps;
    int                     xbar, nSi, node, port, eg, cosq;
    bcm_sbx_port_shaper_state_t *p_shaper;
    int nShaperId;

    LOG_CLI((BSL_META_U(unit,
                        "Dump SBX PORT data, unit(%d)\n"), unit));

    p_ps = SOC_SBX_STATE(unit)->port_state;
    if (p_ps == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return;
    }

    /* now dump port state */
    LOG_CLI((BSL_META_U(unit,
                        "Port State Init: %x\n"), p_ps->init));

    if (!soc_feature(unit, soc_feature_standalone)) {
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_PHYSICAL_SERDES; xbar++) {
            if (p_ps->ability[xbar] != NULL) {
                LOG_CLI((BSL_META_U(unit,
                                    "XBAR: %d\n\t"),xbar));
                for (node=0; node<SB_FAB_DEVICE_BM9600_MAX_NODES; node++) {
                    LOG_CLI((BSL_META_U(unit,
                                        "node[%d]: 0x%x "), node, p_ps->ability[xbar][node]));
                    if ((node%8) == 7) {
                        LOG_CLI((BSL_META_U(unit,
                                            "\n\t")));
                    }
                }
            }
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "uPrbsModeSi: 0x%x\n"), p_ps->uPrbsModeSi));
    LOG_CLI((BSL_META_U(unit,
                        "uPrbsForceTxError:\n")));
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        LOG_CLI((BSL_META_U(unit,
                            "nSi: %2d 0x%x "), nSi, p_ps->uPrbsForceTxError[nSi]));
        if ((nSi%8) == 7) {
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "uDriverEqualizationFarEnd:\n")));
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        LOG_CLI((BSL_META_U(unit,
                            "nSi: %2d 0x%x "), nSi, p_ps->uDriverEqualizationFarEnd[nSi]));
        if ((nSi%8) == 7) {
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "uDriverPriorPostCursorNegative:\n")));
    for (nSi=0; nSi<SB_FAB_USER_MAX_NUM_SERIALIZERS; nSi++) {
        LOG_CLI((BSL_META_U(unit,
                            "nSi: %2d 0x%x "), nSi, p_ps->uDriverPriorPostCursorNegative[nSi]));
        if ((nSi%8) == 7) {
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "CPU fabric port: 0x%x\n"), p_ps->cpu_fabric_port));
    LOG_CLI((BSL_META_U(unit,
                        "Fabric Header Format: %d\n"), p_ps->fabric_header_format));
    
    if (SOC_IS_SBX_QE2000(unit)) {
        p_shaper = shaper_state[unit];
        
        for (nShaperId=0; nShaperId<SOC_SBX_CFG(unit)->nShaperCount; nShaperId++) {

            LOG_CLI((BSL_META_U(unit,
                                "shaper: is_free %d\t"), p_shaper[nShaperId].is_free));
            
            if (p_shaper[nShaperId].is_free == FALSE) {
                LOG_CLI((BSL_META_U(unit,
                                    "shaper_type 0x%x "), p_shaper[nShaperId].shaper_type));
                LOG_CLI((BSL_META_U(unit,
                                    "shaper_src 0x%x "), p_shaper[nShaperId].shaper_src));
                LOG_CLI((BSL_META_U(unit,
                                    "hi_side 0x%x "), p_shaper[nShaperId].hi_side));
                LOG_CLI((BSL_META_U(unit,
                                    "port 0x%x "), p_shaper[nShaperId].port));
            }
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    } else if (SOC_IS_SIRIUS(unit)) {
        /*  bcm_sbx_port_info_t
         *    only need to store flags
         */
        for (port=0; port < SOC_MAX_NUM_PORTS; port++) {
            LOG_CLI((BSL_META_U(unit,
                                "p:%2d flags 0x%x "), port, p_ps->port_info[port].flags));
            if ((port%8) == 7) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
            }
        }

        /*  bcm_sbx_subport_info_t
         */
        LOG_CLI((BSL_META_U(unit,
                            "\nSubport_info:\n")));
        for (port=0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
            LOG_CLI((BSL_META_U(unit,
                                "Subport[%d] "), port));
            if (!LOG_CHECK(BSL_LS_BCM_PORT | BSL_INFO) && !p_ps->subport_info[port].valid) {
                LOG_CLI((BSL_META_U(unit,
                                    "not allocated\n")));
                continue;
            }
            LOG_CLI((BSL_META_U(unit,
                                "valid: %d flags: 0x%x\n"), p_ps->subport_info[port].valid, p_ps->subport_info[port].flags));
            LOG_CLI((BSL_META_U(unit,
                                "parent_gport 0x%x "), p_ps->subport_info[port].parent_gport));
            LOG_CLI((BSL_META_U(unit,
                                "original_gport 0x%x "), p_ps->subport_info[port].original_gport));
            LOG_CLI((BSL_META_U(unit,
                                "group shaper 0x%x\n"), p_ps->subport_info[port].group_shaper));
            LOG_CLI((BSL_META_U(unit,
                                "Port Offset %d "), p_ps->subport_info[port].port_offset));
            LOG_CLI((BSL_META_U(unit,
                                "TS Sched Level: %d "), p_ps->subport_info[port].ts_scheduler_level));
            LOG_CLI((BSL_META_U(unit,
                                "Node: %d "), p_ps->subport_info[port].ts_scheduler_node));
            LOG_CLI((BSL_META_U(unit,
                                "Es_Sched_L2_node: %d\n"), p_ps->subport_info[port].es_scheduler_level2_node));
                
            for (eg=0; eg < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; eg++) {
                LOG_CLI((BSL_META_U(unit,
                                    "Egress Group[%d] "),eg));
                if (!LOG_CHECK(BSL_LS_BCM_PORT | BSL_INFO) && !p_ps->subport_info[port].egroup[eg].num_fifos) {
                    LOG_CLI((BSL_META_U(unit,
                                        " not configured\n")));
                    continue;
                }
                LOG_CLI((BSL_META_U(unit,
                                    "Egress Gport: 0x%x\n"), p_ps->subport_info[port].egroup[eg].egroup_gport));
                LOG_CLI((BSL_META_U(unit,
                                    "\tEs_Sched_L1_node: %d "), p_ps->subport_info[port].es_scheduler_level1_node[eg]));
                LOG_CLI((BSL_META_U(unit,
                                    "Es_Sched_L0_node: %d\n"), p_ps->subport_info[port].egroup[eg].es_scheduler_level0_node));
                LOG_CLI((BSL_META_U(unit,
                                    "\tNum Fifos: %d "), p_ps->subport_info[port].egroup[eg].num_fifos));
                LOG_CLI((BSL_META_U(unit,
                                    "EF FCD: %d "), p_ps->subport_info[port].egroup[eg].ef_fcd));
                LOG_CLI((BSL_META_U(unit,
                                    "NEF FCD: %d\n"), p_ps->subport_info[port].egroup[eg].nef_fcd));
                LOG_CLI((BSL_META_U(unit,
                                    "Attached FCDs: ")));
                for (cosq=0; cosq<SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                    if (p_ps->subport_info[port].egroup[eg].fcd[cosq] == -1) {
                        LOG_CLI((BSL_META_U(unit,
                                            "%d:%d "), cosq, p_ps->subport_info[port].egroup[eg].fcd[cosq])); 
                    } else {
                        LOG_CLI((BSL_META_U(unit,
                                            "%d:%d/%d "), cosq, 
                                 ATTACH_ID_FCD_GET(p_ps->subport_info[port].egroup[eg].fcd[cosq]), 
                                 ATTACH_ID_SYSPORT_GET(p_ps->subport_info[port].egroup[eg].fcd[cosq])));
                    }
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\nPort Speed: ")));
                for (cosq=0; cosq<=SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; cosq++) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%d:%d "), cosq, p_ps->subport_info[port].egroup[eg].port_speed[cosq]));
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
            }
        }

#if defined (BCM_SIRIUS_SUPPORT) && defined(BCM_WARM_BOOT_SUPPORT)
        if (SOC_IS_SIRIUS(unit)) {
            /* Dump sirius port structures */
            _bcm_sirius_wb_port_state_sync(unit, NULL, NULL, NULL, _WB_OP_DUMP);
        }
#endif

        /*  
         *   phy_flags 
         */
        LOG_CLI((BSL_META_U(unit,
                            "phy_flags:\n")));
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            LOG_CLI((BSL_META_U(unit,
                                "p:%d f:0x%x "), port, phy_port_info[unit][port].phy_flags));
            if ((port%8) == 7) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

int
bcm_sbx_port_clear(int unit)
{
    if (port_state[unit] != NULL) {
        if (port_state[unit]->port_info != NULL) {
            sal_free(port_state[unit]->port_info);
        }
        sal_free(port_state[unit]);
        port_state[unit] = NULL;
    }

    return BCM_E_NONE;
}

int
bcm_sbx_port_probe(int unit,
                   pbmp_t pbmp,
                   pbmp_t *okay_pbmp)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_probe, (unit, pbmp, okay_pbmp));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port probe failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_port_detach(int unit,
                    pbmp_t pbmp,
                    pbmp_t *detached)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_sbx_port_config_get
 * Purpose:
 *      Get port configuration of a device
 * Parameters:
 *      unit   - Device unit number
 *      config - (OUT) Structure returning port configuration
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_port_config_get(int unit, bcm_port_config_t *config)
{
    /* Check params */
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }

    if (config == NULL) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    config->ge          = PBMP_GE_ALL(unit);
    config->xe          = PBMP_XE_ALL(unit);
    config->hg          = PBMP_HG_ALL(unit);
    config->e           = PBMP_E_ALL(unit);
    config->sci         = PBMP_SCI_ALL(unit);
    config->sfi         = PBMP_SFI_ALL(unit);
    config->spi         = PBMP_SPI_ALL(unit);
    config->spi_subport = PBMP_SPI_SUBPORT_ALL(unit);
    config->port        = PBMP_PORT_ALL(unit);
    config->cpu         = PBMP_CMIC(unit);
    config->all         = PBMP_ALL(unit);

    /* Clear bitmap for port types not supported in SBX */
    BCM_PBMP_CLEAR(config->fe);
    BCM_PBMP_CLEAR(config->stack_ext);
    BCM_PBMP_CLEAR(config->stack_int);

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_port_update(int unit,
                    bcm_port_t port,
                    int link)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        return (bcm_sirius_port_update(unit, port, link));
#endif
    }

    /* returning an error here will cause linkscan to fail */
    return BCM_E_NONE;
}


int
bcm_sbx_port_linkscan_get(int unit,
                          bcm_port_t port,
                          int *linkscan)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    if (!UNIT_INIT_DONE(unit)) {
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port) ||
        IS_GX_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
        rv = bcm_linkscan_mode_get(unit, port, linkscan);
    }

    return rv;
}

int
bcm_sbx_port_linkscan_set(int unit,
                          bcm_port_t port,
                          int linkscan)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (linkscan != BCM_LINKSCAN_MODE_SW &&
        linkscan != BCM_LINKSCAN_MODE_NONE)
    {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_CONFIG;
    }

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port) ||
        IS_GX_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
        rv = bcm_linkscan_mode_set(unit, port, linkscan);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_autoneg_get(int unit,
                         bcm_port_t port,
                         int *autoneg)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port) || 
        IS_GX_PORT(unit, port) || IS_SPI_PORT(unit, port) || 
        IS_CPU_PORT(unit, port) || IS_REQ_PORT(unit, port) ||
        IS_SPI_SUBPORT_PORT(unit, port)) {
        /* Support Higig interface only, no autoneg */
        *autoneg = FALSE;
        rv = BCM_E_NONE;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_autoneg_set(int unit,
                         bcm_port_t port,
                         int autoneg)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_speed_get(int unit,
                       bcm_port_t port,
                       int *speed)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_speed_get, (unit, port, speed));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port speed get failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_speed_max(int unit,
                       bcm_port_t port,
                       int *speed)
{
    bcm_port_ability_t  ability;
    int rv;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
        rv = bcm_sbx_port_ability_local_get(unit, port, &ability);

        if (BCM_SUCCESS(rv)) {
            *speed = BCM_PORT_ABILITY_SPEED_MAX(ability.speed_full_duplex);
            if (10000 == *speed) {
                if (IS_GX_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
                    *speed = SOC_INFO(unit).port_speed_max[port];
                }
            }
        }
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_port_speed_max: u=%d p=%d speed=%d rv=%d\n"),
                  unit, port, *speed, rv));
    } else {
        rv = bcm_sbx_port_speed_get(unit, port, speed);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_port_speed_set(int unit,
                       bcm_port_t port,
                       int speed)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_IS_SIRIUS(unit)) {
        if (!SOC_PORT_VALID(unit, port)) {
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PORT;
        }
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_speed_set, (unit, port, speed));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port speed set failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int 
bcm_sbx_port_interface_config_get(
    int unit, 
    bcm_port_t port, 
    bcm_port_interface_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_port_interface_config_set(
    int unit, 
    bcm_port_t port, 
    bcm_port_interface_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_interface_get(int unit,
                           bcm_port_t port,
                           bcm_port_if_t *intf)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
        rv = soc_phyctrl_interface_get(unit, port, intf);

        
    } else if (IS_SCI_PORT(unit, port) || IS_SFI_PORT(unit, port)) {
        if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_BM9600(unit)) {
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_BM9600_SUPPORT)
            rv = soc_phyctrl_interface_get(unit, port, intf); 
#endif
        } else {
            rv = BCM_E_NONE;
        }
    } else {
        rv = BCM_E_NONE;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_port_interface_set(int unit,
                           bcm_port_t port,
                           bcm_port_if_t intf)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
        rv = soc_phyctrl_interface_set(unit, port, intf);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_loopback_set(int unit,
                          bcm_port_t port,
                          int loopback)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if ((loopback != BCM_PORT_LOOPBACK_NONE) &&
#if defined(BCM_SIRIUS_SUPPORT)
        (SOC_IS_SIRIUS(unit) && (loopback != BCM_PORT_LOOPBACK_MAC)) &&
#endif        
        (loopback != BCM_PORT_LOOPBACK_PHY)) {
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_loopback_set, (unit, port, loopback));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port loopback set failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_loopback_get(int unit,
                          bcm_port_t port,
                          int *loopback)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_loopback_get, (unit, port, loopback));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port loopback get failed error(%d)\n"),
                   rv));
    }

    if (*loopback) {
        *loopback = BCM_PORT_LOOPBACK_PHY;
    } else {
        *loopback = BCM_PORT_LOOPBACK_NONE;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_duplex_get(int unit,
                        bcm_port_t port,
                        int *duplex)
{
    int  rv = BCM_E_NONE;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port) || IS_GX_PORT(unit, port) ||
        IS_SPI_PORT(unit, port) || IS_CPU_PORT(unit, port) || IS_SPI_SUBPORT_PORT(unit,port)) {
        *duplex = SOC_PORT_DUPLEX_FULL;
    } else {
        rv = BCM_E_PORT;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_duplex_set(int unit,
                        bcm_port_t port,
                        int duplex)
{
    int  rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port) || IS_GX_PORT(unit, port) ||
        IS_SPI_PORT(unit, port) || IS_CPU_PORT(unit, port) || IS_SPI_SUBPORT_PORT(unit, port)) {
        /* duplex setting not allowed */
    } else {
        rv = BCM_E_PORT;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_pause_get(int unit,
                       bcm_port_t port,
                       int *pause_tx,
                       int *pause_rx)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        return(bcm_sirius_port_pause_get(unit, port, pause_tx, pause_rx));
#endif
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_pause_set(int unit,
                       bcm_port_t port,
                       int pause_tx,
                       int pause_rx)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        return(bcm_sirius_port_pause_set(unit, port, pause_tx, pause_rx));
#endif
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_advert_get(int unit,
                        bcm_port_t port,
                        bcm_port_abil_t *ability_mask)
{
    int                 rv;
    bcm_port_ability_t  ability;

    BCM_SBX_LOCK(unit);
    rv = soc_phyctrl_ability_advert_get(unit, port, &ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&ability, ability_mask);
    }
    BCM_SBX_UNLOCK(unit);



    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_advert_get: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, *ability_mask, rv));

    return rv;
}

int
bcm_sbx_port_advert_set(int unit,
                        bcm_port_t port,
                        bcm_port_abil_t ability_mask)
{
    int                 rv;
    bcm_port_ability_t  given_ability, port_ability;


    BCM_IF_ERROR_RETURN
        (bcm_sbx_port_ability_local_get(unit, port, &port_ability));

    BCM_IF_ERROR_RETURN(
        soc_port_mode_to_ability(ability_mask, &given_ability));

    /* make sure that the ability advertising in PHY is supported by MAC */
    given_ability.flags &= port_ability.flags;
    given_ability.loopback &= port_ability.loopback;
    given_ability.medium &= port_ability.medium;
    given_ability.eee    &= port_ability.eee;
    given_ability.pause &= port_ability.pause;
    given_ability.speed_full_duplex &= port_ability.speed_full_duplex;
    given_ability.speed_half_duplex &= port_ability.speed_half_duplex;

    if (IS_HG_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
        if (SOC_INFO(unit).port_speed_max[port] < 16000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_16GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_16GB);
        }

        if (SOC_INFO(unit).port_speed_max[port] < 13000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_13GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_13GB);
        }

        if (SOC_INFO(unit).port_speed_max[port] < 12000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_12GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_12GB);
        } 

        if (!(given_ability.speed_full_duplex & (BCM_PORT_ABILITY_16GB |
              BCM_PORT_ABILITY_13GB | BCM_PORT_ABILITY_12GB |
              BCM_PORT_ABILITY_10GB))) {
            return BCM_E_CONFIG;
        }
    }

    BCM_SBX_LOCK(unit);
    rv = soc_phyctrl_ability_advert_set(unit, port, &given_ability);
    BCM_SBX_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_advert_set: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, ability_mask, rv));

    return rv;
}

int
bcm_sbx_port_ability_advert_get(int unit,
                                bcm_port_t port,
                                bcm_port_ability_t *ability_mask)
{
    int rv = BCM_E_UNAVAIL;

    if (IS_GX_PORT(unit, port) || 
        IS_HG_PORT(unit, port) ||
        IS_XE_PORT(unit, port)) {
        BCM_SBX_LOCK(unit);
        rv = soc_phyctrl_ability_advert_get(unit, port, ability_mask);
        BCM_SBX_UNLOCK(unit);
    }

    return rv;
}

int
bcm_sbx_port_ability_advert_set(int unit,
                                bcm_port_t port,
                                bcm_port_ability_t *ability_mask)
{
    int             rv;
    bcm_port_ability_t port_ability;

    BCM_IF_ERROR_RETURN
        (bcm_sbx_port_ability_local_get(unit, port, &port_ability));

    /* Make sure to advertise only abilities supported by the port */
    port_ability.speed_half_duplex   &= ability_mask->speed_half_duplex;
    port_ability.speed_full_duplex   &= ability_mask->speed_full_duplex;
    port_ability.pause      &= ability_mask->pause;
    port_ability.interface  &= ability_mask->interface;
    port_ability.medium     &= ability_mask->medium;
    port_ability.eee        &= ability_mask->eee;
    port_ability.loopback   &= ability_mask->loopback;
    port_ability.flags      &= ability_mask->flags;

    BCM_SBX_LOCK(unit);
    rv = soc_phyctrl_ability_advert_set(unit, port, &port_ability);
    BCM_SBX_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_advert_set: u=%d p=%d rv=%d\n"),
              unit, port, rv));
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x\n"
                            "Interface=0x%08x Medium=0x%08x EEE=0x%08x Loopback=0x%08x Flags=0x%08x\n"),
                 port_ability.speed_half_duplex,
                 port_ability.speed_full_duplex,
                 port_ability.pause, port_ability.interface,
                 port_ability.medium, port_ability.eee,
                 port_ability.loopback, port_ability.flags));

    return rv;
}

int
bcm_sbx_port_ability_remote_get(int unit,
                                bcm_port_t port,
                                bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_ability_local_get(int unit,
                               bcm_port_t port,
                               bcm_port_ability_t *ability_mask)
{
    soc_port_ability_t             mac_ability, phy_ability;

    if (!IS_GX_PORT(unit, port) && 
        !IS_HG_PORT(unit, port) &&
        !IS_XE_PORT(unit, port)) {
        return BCM_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_ability_local_get(unit, port, &phy_ability));

    SOC_IF_ERROR_RETURN
        (MAC_ABILITY_LOCAL_GET(SBX_PORT(unit, port).p_mac, unit, 
                               port, &mac_ability));

    /* Combine MAC and PHY abilities */
    ability_mask->speed_half_duplex  = mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
    ability_mask->speed_full_duplex  = mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
    ability_mask->pause     = mac_ability.pause & phy_ability.pause;
    if (phy_ability.interface == 0) {
        ability_mask->interface = mac_ability.interface;
    } else {
        ability_mask->interface = phy_ability.interface;
    }
    ability_mask->medium    = phy_ability.medium;

    /* mac_ability.eee without phy_ability.eee makes no sense */
    ability_mask->eee    = phy_ability.eee;

    ability_mask->loopback  = mac_ability.loopback | phy_ability.loopback |
                               BCM_PORT_ABILITY_LB_NONE;
    ability_mask->flags     = mac_ability.flags | phy_ability.flags;

    return BCM_E_NONE;
}

int
bcm_sbx_port_advert_remote_get(int unit,
                               bcm_port_t port,
                               bcm_port_abil_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_ability_get(int unit,
                         bcm_port_t port,
                         bcm_port_abil_t *ability_mask)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_ability_get, (unit, port, ability_mask));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port ability get failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_port_link_status_get(int unit,
                             bcm_port_t port,
                             int *up)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    if (!UNIT_INIT_DONE(unit)) {
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_link_status_get, (unit, port, up));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port link status get failed error(%d)\n"),
                   rv));
    }

    return rv;
}


/*
 * Function:
 *     bcm_port_selective_get
 * Purpose:
 *     Get requested port parameters.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - (IN/OUT) port information structure
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     The action_mask field of the info argument is used as an input
 */
int
bcm_sbx_port_selective_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int     rv = BCM_E_NONE;
    uint32  mask;

    /* Check params */
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }
    if (info == NULL) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        rv = bcm_port_encap_get(unit, port, &info->encap_mode);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_encap_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_ENABLE_MASK) {
        rv = bcm_port_enable_get(unit, port, &info->enable);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_enable_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LINKSTAT_MASK) {
        rv = bcm_port_link_status_get(unit, port, &info->linkstatus);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_link_status_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK) {
        rv = bcm_port_autoneg_get(unit, port, &info->autoneg);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_autoneg_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
        rv = bcm_port_advert_get(unit, port, &info->local_advert);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_advert_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK) {
        if ((rv = bcm_port_advert_remote_get(unit, port,
                                             &info->remote_advert)) < 0) {
            info->remote_advert = 0;
            info->remote_advert_valid = FALSE;
        } else {
            info->remote_advert_valid = TRUE;
        }
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK) {
        if ((rv = bcm_port_speed_get(unit, port, &info->speed)) < 0) {
            if (rv != BCM_E_BUSY) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_speed_get failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            } else {
                info->speed = 0;
            }
        }
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK) {
        if ((rv = bcm_port_duplex_get(unit, port, &info->duplex)) < 0) {
            if (rv != BCM_E_BUSY) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_duplex_get failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            } else {
                info->duplex = 0;
            }
        }
    }

    /* Get both if either mask bit set */
    if (mask & (BCM_PORT_ATTR_PAUSE_TX_MASK |
                BCM_PORT_ATTR_PAUSE_RX_MASK)) {
        rv = bcm_port_pause_get(unit, port,
                                &info->pause_tx, &info->pause_rx);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        rv = bcm_port_pause_addr_get(unit, port, info->pause_mac);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_addr_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK) {
        rv = bcm_port_linkscan_get(unit, port, &info->linkscan);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_linkscan_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK) {
        rv = bcm_port_learn_get(unit, port, &info->learn);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_learn_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK) {
        rv = bcm_port_discard_get(unit, port, &info->discard);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_discard_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK) {
        rv = bcm_port_vlan_member_get(unit, port, &info->vlanfilter);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_vlan_member_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
        rv = bcm_port_untagged_priority_get(unit, port,
                                            &info->untagged_priority);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_untagged_priority_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK) {
        rv = bcm_port_untagged_vlan_get(unit, port, &info->untagged_vlan);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_untagged_vlan_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK) {
        rv = bcm_port_stp_get(unit, port, &info->stp_state);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_stp_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        rv = bcm_port_pfm_get(unit, port, &info->pfm);
        if (BCM_FAILURE(rv)) {
            if (rv != BCM_E_UNAVAIL) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_pfm_get failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
        }
    }

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK) {
        rv = bcm_port_loopback_get(unit, port, &info->loopback);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_loopback_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        rv = bcm_port_master_get(unit, port, &info->phy_master);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_master_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK) {
        rv = bcm_port_interface_get(unit, port, &info->interface);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_interface_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        rv = bcm_rate_mcast_get(unit, &info->mcast_limit,
                                &info->mcast_limit_enable, port);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_mcast_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
        rv = bcm_rate_bcast_get(unit, &info->bcast_limit,
                                &info->bcast_limit_enable, port);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_bcast_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
        rv = bcm_rate_dlfbc_get(unit, &info->dlfbc_limit,
                                &info->dlfbc_limit_enable, port);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_dlfbc_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_SPEED_MAX_MASK) {
        rv = bcm_port_speed_max(unit, port, &info->speed_max);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_speed_max failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_ABILITY_MASK) {
        rv = bcm_port_ability_get(unit, port, &info->ability);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_ability_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        rv = bcm_port_frame_max_get(unit, port, &info->frame_max);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_frame_max_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        rv = bcm_port_mdix_get(unit, port, &info->mdix);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_mdix_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_MDIX_STATUS_MASK) {
        rv = bcm_port_mdix_status_get(unit, port, &info->mdix_status);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_mdix_status_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_MEDIUM_MASK) {
        rv = bcm_port_medium_get(unit, port, &info->medium);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_medium_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_FAULT_MASK) {
        rv = bcm_port_fault_get(unit, port, &info->fault);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_fault_get failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_port_selective_set
 * Purpose:
 *     Set requested port parameters.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - Port information structure
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Does not set spanning tree state.
 */
int
bcm_sbx_port_selective_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int     rv = BCM_E_NONE;
    uint32  mask;
    int     flags = 0;

    /* Check params */
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }
    if (info == NULL) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        rv = bcm_port_encap_set(unit, port, info->encap_mode);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_encap_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_ENABLE_MASK) {
        rv = bcm_port_enable_set(unit, port, info->enable);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_enable_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        rv = bcm_port_pause_addr_set(unit, port, info->pause_mac);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_addr_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK) {
        rv = bcm_port_interface_set(unit, port, info->interface);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_interface_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        rv = bcm_port_master_set(unit, port, info->phy_master);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_master_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK) {
        rv = bcm_port_linkscan_set(unit, port, info->linkscan);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_linkscan_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK) {
        rv = bcm_port_learn_set(unit, port, info->learn);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_learn_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK) {
        rv = bcm_port_discard_set(unit, port, info->discard);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_discard_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK) {
        rv = bcm_port_vlan_member_set(unit, port, info->vlanfilter);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_vlan_member_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
        rv = bcm_port_untagged_priority_set(unit, port,
                                            info->untagged_priority);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_untagged_priority_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK) {
        rv = bcm_port_untagged_vlan_set(unit, port, info->untagged_vlan);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_untagged_vlan_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        rv = bcm_port_pfm_set(unit, port, info->pfm);
        if (BCM_FAILURE(rv) && (rv != BCM_E_UNAVAIL)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_pfm_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    /*
     * Set loopback mode before setting the speed/duplex, since it may
     * affect the allowable values for speed/duplex.
     */

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK) {
        rv = bcm_port_loopback_set(unit, port, info->loopback);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_loopback_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
        rv = bcm_port_advert_set(unit, port, info->local_advert);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_advert_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK) {
        rv = bcm_port_autoneg_set(unit, port, info->autoneg);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_autoneg_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK) {
        rv = bcm_port_speed_set(unit, port, info->speed);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_speed_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK) {
        rv = bcm_port_duplex_set(unit, port, info->duplex);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_duplex_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & (BCM_PORT_ATTR_PAUSE_TX_MASK |
                BCM_PORT_ATTR_PAUSE_RX_MASK)) {
        int     tpause, rpause;

        tpause = rpause = -1;
        if (mask & BCM_PORT_ATTR_PAUSE_TX_MASK) {
            tpause = info->pause_tx;
        }
        if (mask & BCM_PORT_ATTR_PAUSE_RX_MASK) {
            rpause = info->pause_rx;
        }
        rv = bcm_port_pause_set(unit, port, tpause, rpause);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        flags = (info->mcast_limit_enable) ? BCM_RATE_MCAST : 0;
        rv = bcm_rate_mcast_set(unit, info->mcast_limit, flags, port);
        if (rv == BCM_E_UNAVAIL) {
            rv = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_mcast_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
        flags = (info->bcast_limit_enable) ? BCM_RATE_BCAST : 0;
        rv = bcm_rate_bcast_set(unit, info->bcast_limit, flags, port);
        if (rv == BCM_E_UNAVAIL) {
            rv = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_bcast_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
        flags = (info->dlfbc_limit_enable) ? BCM_RATE_DLF : 0;
        rv = bcm_rate_dlfbc_set(unit, info->dlfbc_limit, flags, port);
        if (rv == BCM_E_UNAVAIL) {
            rv = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_rate_dlfbc_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK) {
        rv = bcm_port_stp_set(unit, port, info->stp_state);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_stp_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        rv = bcm_port_frame_max_set(unit, port, info->frame_max);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_frame_max_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        rv = bcm_port_mdix_set(unit, port, info->mdix);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_mdix_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_port_info_get(int unit,
                      bcm_port_t port,
                      bcm_port_info_t *info)
{
    int rv;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!UNIT_INIT_DONE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PORT;
    }

    if (info == NULL) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    bcm_port_info_t_init(info);


    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);

    } else if (IS_SPI_PORT(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);
        /* What other attributes applicable ? for SPI bus ports */
    } else if (IS_GX_PORT(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_SPEED_MASK |
                             BCM_PORT_ATTR_LINKSCAN_MASK |
                             BCM_PORT_ATTR_LOOPBACK_MASK |
                             BCM_PORT_ATTR_AUTONEG_MASK |
                             BCM_PORT_ATTR_ENCAP_MASK |
                             BCM_PORT_ATTR_SPEED_MAX_MASK |
                             BCM_PORT_ATTR_FRAME_MAX_MASK |
                             BCM_PORT_ATTR_INTERFACE_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);
    } else {
        info->action_mask = BCM_PORT_ATTR_ALL_MASK;
    }

    rv = bcm_port_selective_get(unit, port, info);
    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_port_info_set(int unit,
                      bcm_port_t port,
                      bcm_port_info_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_info_save(int unit,
                       bcm_port_t port,
                       bcm_port_info_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_info_restore(int unit,
                          bcm_port_t port,
                          bcm_port_info_t *info)
{
    int rv;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = bcm_port_selective_set(unit, port, info);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_phy_drv_name_get(int unit,
                              bcm_port_t port,
                              char *name, int len)
{
    int str_len;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_INIT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        str_len = sal_strlen("invalid port");
        if (str_len <= len) {
          /* coverity[secure_coding] */
            sal_strcpy(name, "invalid port");
        }
        return BCM_E_PORT;
    }
    return (soc_phyctrl_drv_name_get(unit, port, name, len)); 
}

int
bcm_sbx_port_encap_set(int unit,
                       bcm_port_t port,
                       int mode)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_SIRIUS_SUPPORT
    uint64 val64;
#endif

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

#ifdef BCM_SIRIUS_SUPPORT
    COMPILER_64_ZERO(val64);
#endif

    if (IS_GX_PORT(unit, port)) {
        switch (mode) {
            case SOC_ENCAP_SBX:
                SOC_SBX_CFG(unit)->uInterfaceProtocol = SOC_SBX_IF_PROTOCOL_SBX;
                rv = BCM_E_NONE;
                break;
            case SOC_ENCAP_HIGIG2:
                SOC_SBX_CFG(unit)->uInterfaceProtocol = SOC_SBX_IF_PROTOCOL_XGS;
                rv = BCM_E_NONE;
                break;
            case SOC_ENCAP_HIGIG:
            case SOC_ENCAP_HIGIG2_L2:
            case SOC_ENCAP_HIGIG2_IP_GRE:
            case SOC_ENCAP_IEEE:
            case SOC_ENCAP_B5632:
            default:
                break;
        }
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SBX_SIRIUS(unit)) {
        if (rv == BCM_E_NONE) {
            rv = soc_sirius_hw_update_trt(unit);
            if (rv != BCM_E_NONE)
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_encap_set failed: %s\n"),
                             bcm_errmsg(rv)));
        }
        if (rv == BCM_E_NONE) {
            rv = soc_sirius_hw_update_crt(unit);
            if (rv != BCM_E_NONE)
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_encap_set failed: %s\n"),
                             bcm_errmsg(rv)));
        }

        if (!SAL_BOOT_BCMSIM && (rv == BCM_E_NONE) && IS_GX_PORT(unit, port)) {
          uint64 tmp0 = COMPILER_64_INIT(0,0), tmp1 = COMPILER_64_INIT(0,1);
            /* update the 51b_mode */

            if (mode == SOC_ENCAP_SBX) {
                rv = READ_MAC_CTRLr(unit, port, &val64);
                if (rv != BCM_E_NONE) {
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
                soc_reg64_field_set(unit, MAC_CTRLr, &val64, RUNT_51B_MODEf, tmp1);
                rv = WRITE_MAC_CTRLr(unit, port, val64);
                if (rv != BCM_E_NONE) {
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
            } else {
                rv = READ_MAC_CTRLr(unit, port, &val64);
                if (rv != BCM_E_NONE) {
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
                soc_reg64_field_set(unit, MAC_CTRLr, &val64, RUNT_51B_MODEf, tmp0);
                rv = WRITE_MAC_CTRLr(unit, port, val64);
                if (rv != BCM_E_NONE) {
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
            }
        }
    }
#endif /* BCM_SIRIUS_SUPPORT */

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int
bcm_sbx_port_encap_get(int unit,
                       bcm_port_t port,
                       int *mode)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_SIRIUS_SUPPORT
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if ((IS_GX_PORT(unit, port) || IS_REQ_PORT(unit, port)) &&
        SOC_IS_SBX_SIRIUS(unit)) {
        if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
            *mode = SOC_ENCAP_SBX;
        } else {
            *mode = SOC_ENCAP_HIGIG2;
        }
        rv = BCM_E_NONE;
    }

    BCM_SBX_UNLOCK(unit);
#endif /* BCM_SIRIUS_SUPPORT */

    return rv;
}

int
bcm_sbx_port_encap_config_set(int unit,
                              bcm_gport_t gport,
                              bcm_port_encap_config_t *config)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_SIRIUS_SUPPORT
    bcm_port_t port = -1;
    bcm_module_t module = -1;
    bcm_module_t mymodid;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SIRIUS(unit)) {
        if (!BCM_GPORT_IS_SET(gport)) {
            /* raw local port */
            port = gport;
        } else if (BCM_GPORT_IS_MODPORT(gport)) {
            port = BCM_GPORT_MODPORT_PORT_GET(gport);
            module = BCM_GPORT_MODPORT_MODID_GET(gport);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
            module = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
        } else if (BCM_GPORT_IS_LOCAL(gport)) {
            port = BCM_GPORT_LOCAL_GET(gport);
        }
        if ((0 <= port) && (BCM_PBMP_PORT_MAX > port)) {
            if (0 <= module) {
                rv = bcm_stk_my_modid_get(unit, &mymodid);
                if ((BCM_E_NONE == rv) && (module != mymodid)) {
                    /* don't support remote lookup here */
                    rv = BCM_E_PARAM;
                }
            } else { /* if (0 <= module) */
                /* module is [implied] local; just keep going */
                rv = BCM_E_NONE;
            } /* if (0 <= module) */
            if (BCM_E_NONE == rv) {
                if (IS_GX_PORT(unit, port)) {
                    /* do this in the same place, rather than duplicating */
                    rv = bcm_sbx_port_encap_set(unit,
                                                port,
                                                config->encap);
                }
            } /* if (BCM_E_NONE == rv) */
        } /* if ((0 <= port) && (BCM_PBMP_PORT_MAX > port)) */
    } /* if (SOC_IS_SIRIUS(unit)) */

    BCM_SBX_UNLOCK(unit);

#endif /* BCM_SIRIUS_SUPPORT */
    return rv;
}

int
bcm_sbx_port_encap_config_get(int unit,
                              bcm_gport_t gport,
                              bcm_port_encap_config_t *config)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_SIRIUS_SUPPORT
    bcm_port_t port = -1;
    bcm_module_t module = -1;
    bcm_module_t mymodid;
    int mode = 0;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SIRIUS(unit)) {
        if (!BCM_GPORT_IS_SET(gport)) {
            /* raw local port */
            port = gport;
        } else if (BCM_GPORT_IS_MODPORT(gport)) {
            port = BCM_GPORT_MODPORT_PORT_GET(gport);
            module = BCM_GPORT_MODPORT_MODID_GET(gport);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
            module = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
        } else if (BCM_GPORT_IS_LOCAL(gport)) {
            port = BCM_GPORT_LOCAL_GET(gport);
        }
        if ((0 <= port) && (BCM_PBMP_PORT_MAX > port)) {
            if (0 <= module) {
                rv = bcm_stk_my_modid_get(unit, &mymodid);
                if ((BCM_E_NONE == rv) && (module != mymodid)) {
                    /* don't support remote lookup here */
                    rv = BCM_E_PARAM;
                }
            } else { /* if (0 <= module) */
                /* module is [implied] local; just keep going */
                rv = BCM_E_NONE;
            } /* if (0 <= module) */
            if (BCM_E_NONE == rv) {
                if (IS_GX_PORT(unit, port)) {
                    sal_memset(config, 0x00, sizeof(*config));
                    /* do this in the same place, rather than duplicating */
                    rv = bcm_sbx_port_encap_get(unit,
                                                port,
                                                &mode);
                    config->encap = mode;
                }
            } /* if (BCM_E_NONE == rv) */
        } /* if ((0 <= port) && (BCM_PBMP_PORT_MAX > port)) */
    } /* if (SOC_IS_SIRIUS(unit)) */

    BCM_SBX_UNLOCK(unit);

#endif /* BCM_SIRIUS_SUPPORT */
    return rv;
}

int
bcm_sbx_port_queued_count_get(int unit,
                              bcm_port_t port,
                              uint32 *count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_rate_egress_set(int unit,
                             bcm_port_t port,
                             uint32 kbits_sec,
                             uint32 kbits_burst)
{
    int rv = BCM_E_UNAVAIL;
    uint32 uShaperSrc;
    int nShaperId;
    int nShaperConflicts;
    uint32 uMatchControl;
    bcm_sbx_port_shaper_state_t *p_shaper_state;
#ifdef BCM_SIRIUS_SUPPORT
    int level = 0, node = 0;
#endif /* BCM_SIRIUS_SUPPORT */

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    /* Get System Parameters */
    p_shaper_state = shaper_state[unit];

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "%s\n"),
                 FUNCTION_NAME()));

    if (soc_feature(unit, soc_feature_egress_metering)) {

        if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
            if (kbits_sec > 33538048) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "%s: Kbits_sec too large, cannot exceed 33538048 unit %d\n"),
                             FUNCTION_NAME(), unit));                
            }

            if (BCM_GPORT_IS_CHILD(port)) {
                BCM_GPORT_EGRESS_CHILD_SET(port, BCM_GPORT_CHILD_MODID_GET(port),
                                           BCM_GPORT_CHILD_PORT_GET(port));
            }

            if (!BCM_GPORT_IS_EGRESS_CHILD(port)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set requires egress child gport format on unit %d\n"),
                             unit));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }

            level = SB_FAB_DEVICE_GROUP_SHAPER_LEVEL;
            rv = bcm_sbx_port_get_scheduler(unit, port, &level, &node);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_shaper_params, (unit,
                                      level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                      SOC_SIRIUS_API_PARAM_NO_CHANGE, kbits_sec, kbits_burst * 1000));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: %s\n"),
                             bcm_errmsg(rv)));
            }
            BCM_SBX_UNLOCK(unit);
            return rv;
#endif /* BCM_SIRIUS_SUPPORT */
        }

        uShaperSrc = ( BCM_PORT_RATE_TRAFFIC_UC_EF |
                       BCM_PORT_RATE_TRAFFIC_UC_NON_EF |
                       BCM_PORT_RATE_TRAFFIC_NON_UC_EF |
                       BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF );

        rv = _bcm_sbx_port_check_egress_shapers(unit, port,
                                                BCM_SBX_PORT_EGRESS_SHAPER_PORT,
                                                uShaperSrc,
                                                &nShaperId,
                                                &nShaperConflicts,
                                                &uMatchControl);

        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_rate_egress_set failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        /*
          LOG_CLI((BSL_META_U(unit,
                              " *** check egress shapers Conflicts %d, Id %d, MatchControl 0x%x\n"),
                   nShaperConflicts, nShaperId, uMatchControl));
        */

        if ((nShaperConflicts == 0) && (nShaperId == -1) && (uMatchControl == 0)) {

            /* There are no existing shapers assigned to this port and fifo configuration, so get a new one */
            /* The shaper controls are not in use so take first available */

            rv = _bcm_sbx_port_egress_shaperid_get(unit, port, &nShaperId);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: No Shapers available %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            LOG_CLI((BSL_META_U(unit,
                                "Assigned new Port Shaper ID %d\n"), nShaperId));
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_shaper_set, (unit, port, nShaperId, kbits_sec, kbits_burst));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: %s\n"),
                             bcm_errmsg(rv)));
            }

            if (rv == BCM_E_NONE) {
#ifdef SHAPER_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "1>Setting Shaper %d is_free = FALSE"), nShaperId));
#endif
                p_shaper_state[nShaperId].is_free = FALSE;
                p_shaper_state[nShaperId].shaper_type = BCM_SBX_PORT_EGRESS_SHAPER_PORT;
                p_shaper_state[nShaperId].shaper_src = uShaperSrc;
                p_shaper_state[nShaperId].port = port;
                p_shaper_state[nShaperId].hi_side = 0;
            }
            BCM_SBX_UNLOCK(unit);
            return rv;
        } else if ((nShaperConflicts == 0) && (nShaperId != -1) &&
                   (uMatchControl == BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH)) {

            /* There is an existing shaper assigned to this port and fifo configuration, so use it or */
            /* release if kb_sec is zero */

#if 0
            rv = _bcm_sbx_port_egress_shaperid_get(unit, port, &nShaperId);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: No Shapers available %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            LOG_CLI((BSL_META_U(unit,
                                "Shaper ID %d\n"), nShaperId));
#endif

            /* If we are reloading and the shaper is already in use and matched, we need to set */
            /* the state up accordingly                                                         */
            if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
                p_shaper_state[nShaperId].is_free = FALSE;
                p_shaper_state[nShaperId].shaper_type = BCM_SBX_PORT_EGRESS_SHAPER_PORT;
                p_shaper_state[nShaperId].shaper_src = uShaperSrc;
                p_shaper_state[nShaperId].port = port;
                p_shaper_state[nShaperId].hi_side = 0;
#endif /* BCM_EASY_RELOAD_SUPPORT */
            }

            /* LOG_CLI((BSL_META_U(unit,
                                   "Modify exiting PORT shaper\n"))); */
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_shaper_set, (unit, port, nShaperId,
                                      (int)kbits_sec, (int)kbits_burst));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: %s\n"),
                             bcm_errmsg(rv)));
            }

            /* Verify sanity although we should already know this is true*/
            if (rv == BCM_E_NONE) {
                if(( p_shaper_state[nShaperId].is_free != FALSE ) &&
                   ( p_shaper_state[nShaperId].shaper_type != BCM_SBX_PORT_EGRESS_SHAPER_PORT ) &&
                   ( p_shaper_state[nShaperId].shaper_src != uShaperSrc ) &&
                   ( p_shaper_state[nShaperId].port != port ) &&
                   ( p_shaper_state[nShaperId].hi_side != 0 )) {

                    LOG_CLI((BSL_META_U(unit,
                                        "Failed shaper database consistency check 0x%x 0x%x 0x%x 0x%x 0x%x\n"),
                             p_shaper_state[nShaperId].is_free,
                             p_shaper_state[nShaperId].shaper_type,
                             p_shaper_state[nShaperId].shaper_src,
                             p_shaper_state[nShaperId].port,
                             p_shaper_state[nShaperId].hi_side));
                    rv = BCM_E_INTERNAL;
                }
            }

            BCM_SBX_UNLOCK(unit);
            return rv;
        }
    } else {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_port_rate_egress_set Unavailable on %s: %s\n"),
                     SOC_CHIP_STRING(unit), bcm_errmsg(rv)));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_LINK,
                (BSL_META_U(unit,
                            "bcm_sbx_port_rate_egress_set UNAVAILABLE\n")));
    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_rate_egress_get(int unit,
                             bcm_port_t port,
                             uint32 *kbits_sec,
                             uint32 *kbits_burst)
{
    uint32 uShaperSrc;
    int nShaperId = 0;
    int nShaperConflicts = 0;  /* Check if multiple shapers already connect the desired FIFO */
    uint32 uMatchControl = 0;
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_SIRIUS_SUPPORT
    int level = 0, node = 0;
#endif /* BCM_SIRIUS_SUPPORT */

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "bcm_sbx_port_rate_egress_get\n")));

    if (soc_feature(unit, soc_feature_egress_metering)) {

        if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
            if (BCM_GPORT_IS_CHILD(port)) {
                BCM_GPORT_EGRESS_CHILD_SET(port, BCM_GPORT_CHILD_MODID_GET(port),
                                           BCM_GPORT_CHILD_PORT_GET(port));
            }

            if (!BCM_GPORT_IS_EGRESS_CHILD(port)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_get requires egress child gport format on unit %d\n"),
                             unit));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }

            level = SB_FAB_DEVICE_GROUP_SHAPER_LEVEL;
            rv = bcm_sbx_port_get_scheduler(unit, port, &level, &node);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_get failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_shaper_params, (unit,
                                      level, node, NULL, NULL, (int*)kbits_sec, (int*)kbits_burst));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_get failed: %s\n"),
                             bcm_errmsg(rv)));
            } else {
                /* mbcm call return bits instead of kbits, convert it */
                *kbits_burst /= 1000;
            }
            BCM_SBX_UNLOCK(unit);
            return rv;
#endif /* BCM_SIRIUS_SUPPORT */
        }

        /* On are always on for PORT shaping */
        uShaperSrc = ( BCM_PORT_RATE_TRAFFIC_UC_EF |
                       BCM_PORT_RATE_TRAFFIC_UC_NON_EF |
                       BCM_PORT_RATE_TRAFFIC_NON_UC_EF |
                       BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF );

        rv = _bcm_sbx_port_check_egress_shapers(unit, port,
                                                BCM_SBX_PORT_EGRESS_SHAPER_PORT,
                                                uShaperSrc,
                                                &nShaperId,
                                                &nShaperConflicts,
                                                &uMatchControl);

        if ((nShaperConflicts == 0) && (nShaperId != -1) &&
            (uMatchControl == BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH)) {

            /* Command available on QE2000 */
            rv = MBCM_SBX_DRIVER_CALL(unit,
                                      mbcm_port_rate_egress_shaper_get,
                                      (unit,
                                      port,
                                      nShaperId,
                                      kbits_sec,
                                      kbits_burst));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_get failed: %s\n"),
                             bcm_errmsg(rv)));
            }
        } else {
#ifdef SHAPER_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "Port & Shaper Match Not found. Conflicts %d, MatchControl 0x%x\n"),
                     nShaperConflicts, uMatchControl));
#endif
            rv = BCM_E_INTERNAL;
        }
    } else {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_port_rate_egress_get Unavailable on %s: %s\n"),
                     SOC_CHIP_STRING(unit), bcm_errmsg(rv)));
    }

    if (rv == BCM_E_UNAVAIL) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_sbx_port_rate_egress_get Unavailable\n")));
    }

    BCM_SBX_UNLOCK(unit);

    return rv;
}


/* This function is used to set and clear shaping to between FIFOs and Shapers */
int
bcm_sbx_port_rate_egress_traffic_set(int unit,
                                     bcm_port_t port,
                                     uint32 traffic_types,
                                     uint32 kbits_sec,
                                     uint32 kbits_burst)
{
    int rv = BCM_E_UNAVAIL;
    int nShaperType;  /* Is this a Port or Fifo Shaper */
    uint32 uShaperSrc;   /* The fifo to shaper select bits */
    int nShaperId;
    int nShaperConflicts;  /* Check if multiple shapers already connect the desired FIFO */
    uint32 uMatchControl;
    bcm_sbx_port_shaper_state_t *p_shaper_state;
#ifdef BCM_SIRIUS_SUPPORT
    int level = 0, node = 0;
#endif /* BCM_SIRIUS_SUPPORT */

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    /* Get System Parameters */
    p_shaper_state = shaper_state[unit];

    if (soc_feature(unit, soc_feature_egress_metering)) {

        if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
            /* in case of port create at init time, allow user to pass in child/egress child type */
            if (BCM_GPORT_IS_CHILD(port)) {
                BCM_GPORT_EGRESS_GROUP_SET(port, BCM_GPORT_CHILD_MODID_GET(port),
                                           BCM_GPORT_CHILD_PORT_GET(port));
            } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
                BCM_GPORT_EGRESS_GROUP_SET(port, BCM_GPORT_EGRESS_CHILD_MODID_GET(port),
                                           BCM_GPORT_EGRESS_CHILD_PORT_GET(port));                
            }
            
            if (!BCM_GPORT_IS_EGRESS_GROUP(port)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_set requires egress group gport format on unit %d\n"),
                             unit));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }

            rv = bcm_sbx_port_get_scheduler(unit, port, &level, &node);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_set failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            switch (traffic_types) {
                case BCM_PORT_RATE_TRAFFIC_UC_EF:
                    break;
                case BCM_PORT_RATE_TRAFFIC_UC_NON_EF:
                    node++;
                    break;
                case BCM_PORT_RATE_TRAFFIC_NON_UC_EF:
                    node += 2;
                    break;
                case BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF:
                    node += 3;
                    break;
                default:
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "bcm_port_rate_egress_traffic_set unrecognized traffic type on unit %d\n"),
                                 unit));
                    BCM_SBX_UNLOCK(unit);
                    return BCM_E_PARAM;
            }

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_shaper_params, (unit,
                                      level, node, 0, 0, (int)kbits_sec, (int)(kbits_burst * 1000)));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_set failed: %s\n"),
                             bcm_errmsg(rv)));
            }
            BCM_SBX_UNLOCK(unit);
            return rv;
#endif /* BCM_SIRIUS_SUPPORT */
        }

        nShaperType = BCM_SBX_PORT_EGRESS_SHAPER_FIFO;
        uShaperSrc = traffic_types;

        rv = _bcm_sbx_port_check_egress_shapers(unit, port,
                                                nShaperType,
                                                uShaperSrc,
                                                &nShaperId,
                                                &nShaperConflicts,
                                                &uMatchControl);

        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_check_egress_shapers failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        if ((nShaperConflicts == 0) && (nShaperId == -1) && (uMatchControl == 0)) {

            /* There are no existing shapers assigned to this port and fifo configuration, so get a new one */
            /* The shaper controls are not in use so take first available */

            rv = _bcm_sbx_port_egress_shaperid_get(unit, port, &nShaperId);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_set failed: No Shapers available %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            /*
            LOG_CLI((BSL_META_U(unit,
                                "Shaper ID %d\n"), nShaperId));
            */


            /* if kbits_sec is 0, are are turning shaper off */
            if(kbits_sec != 0) {

                /*
                LOG_CLI((BSL_META_U(unit,
                                    "Setting rate\n")));
                */

                /* assign selected shaper to fifo configuration */
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_traffic_set, (unit, port, nShaperId, traffic_types,
                                          (int )kbits_sec, (int )kbits_burst));
                if (BCM_FAILURE(rv)) {
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "bcm_port_rate_egress_set failed: %s\n"),
                                 bcm_errmsg(rv)));
                }
                if (rv == BCM_E_NONE) {
#ifdef SHAPER_DEBUG
                    LOG_CLI((BSL_META_U(unit,
                                        "2>Setting Shaper %d is_free = FALSE"), nShaperId));
#endif
                    p_shaper_state[nShaperId].is_free = FALSE;
                    p_shaper_state[nShaperId].shaper_type = nShaperType;
                    p_shaper_state[nShaperId].shaper_src = traffic_types;
                    p_shaper_state[nShaperId].port = port;
                    p_shaper_state[nShaperId].hi_side = 0;
                }
                BCM_SBX_UNLOCK(unit);
                return rv;
            } else {

#ifdef SHAPER_DEBUG
                LOG_CLI((BSL_META_U(unit,
                                    "** nShaperConflicts %d nShaperId %d uMatchControl %d\n"), nShaperConflicts, nShaperId, uMatchControl));
                LOG_CLI((BSL_META_U(unit,
                                    "Unexpected clear rate on unassigned port %d & fifo 0x%x\n"), port, traffic_types));
#endif
                /* We did not end up using the Shaper after all */
                    p_shaper_state[nShaperId].is_free = TRUE;
                BCM_SBX_UNLOCK(unit);
              return BCM_E_INTERNAL;

            }

        } else if ((nShaperConflicts == 0) && (nShaperId != -1)
                && (uMatchControl == BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH)) {

            /* Here we have a direct match so we can modify or disable */
            /* if kbits_sec is 0, are are turning shaper off */
            if(kbits_sec != 0) {

#ifdef SHAPER_DEBUG
                LOG_CLI((BSL_META_U(unit,
                                    "Modifying rate\n")));
#endif

                /* modify selected shaper to fifo configuration */
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_traffic_set, (unit, port, nShaperId, traffic_types, kbits_sec, kbits_burst));
                if (BCM_FAILURE(rv)) {
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "bcm_port_rate_egress_set failed: %s\n"),
                                 bcm_errmsg(rv)));
                }

            /* If we are reloading and the shaper is already in use and matched, we need to set */
            /* the state up accordingly                                                         */
            if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
                p_shaper_state[nShaperId].is_free = FALSE;
                p_shaper_state[nShaperId].shaper_type = BCM_SBX_PORT_EGRESS_SHAPER_PORT;
                p_shaper_state[nShaperId].shaper_src = uShaperSrc;
                p_shaper_state[nShaperId].port = port;
                p_shaper_state[nShaperId].hi_side = 0;
#endif /* BCM_EASY_RELOAD_SUPPORT */
            }

                /* Verify sanity although we should already know this is true*/
                if (rv == BCM_E_NONE) {
                    if(( p_shaper_state[nShaperId].is_free != FALSE ) &&
                       ( p_shaper_state[nShaperId].shaper_type != nShaperType ) &&
                       ( p_shaper_state[nShaperId].shaper_src != traffic_types ) &&
                       ( p_shaper_state[nShaperId].port != port ) &&
                       ( p_shaper_state[nShaperId].hi_side != 0 )) {

                          LOG_CLI((BSL_META_U(unit,
                                              "Failed shaper database consistency check 0x%x 0x%x 0x%x 0x%x 0x%x\n"),
                                   p_shaper_state[nShaperId].is_free,
                                   p_shaper_state[nShaperId].shaper_type,
                                   p_shaper_state[nShaperId].shaper_src,
                                   p_shaper_state[nShaperId].port,
                                   p_shaper_state[nShaperId].hi_side));
                          rv = BCM_E_INTERNAL;
                       }
                }
                BCM_SBX_UNLOCK(unit);
                return rv;
            } else {

                /* clear and release selected shaper to fifo configuration */
                kbits_burst = 0;
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_traffic_set, (unit, port, nShaperId, traffic_types, kbits_sec, kbits_burst));
                if (rv == BCM_E_NONE) {
                    p_shaper_state[nShaperId].is_free = TRUE;
                    p_shaper_state[nShaperId].shaper_type =  BCM_SBX_PORT_EGRESS_SHAPER_UNKNOWN;
                    p_shaper_state[nShaperId].shaper_src = 0;
                    p_shaper_state[nShaperId].port = -1;
                    p_shaper_state[nShaperId].hi_side = -1;
#ifdef SHAPER_DEBUG
                    LOG_CLI((BSL_META_U(unit,
                                        "Clear rate and release shaperId %d on  port %d & fifo 0x%x\n"),nShaperId, port, traffic_types));
#endif

                } else {
#ifdef SHAPER_DEBUG
                    LOG_CLI((BSL_META_U(unit,
                                        "Unexpected bcm_qe2000_port_rate_egress_traffic_set error %x ShaperId %d Freed"), rv,nShaperId));
#endif
                /* We did not end up using the Shaper after all */
                    p_shaper_state[nShaperId].is_free = TRUE;
                }
                BCM_SBX_UNLOCK(unit);
              return rv;

            }


        } else if ((nShaperConflicts == 0) && (nShaperId == -1)) {

            LOG_CLI((BSL_META_U(unit,
                                "Conflicts %d, MatchControl 0x%x\n"), nShaperConflicts, uMatchControl));
            LOG_CLI((BSL_META_U(unit,
                                "Need to implement existing shaper modification\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_INTERNAL;

        } else {

            LOG_CLI((BSL_META_U(unit,
                                "Conflicts %d, MatchControl 0x%x\n"), nShaperConflicts, uMatchControl));
            LOG_CLI((BSL_META_U(unit,
                                "Need to implement conflict resolution\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_INTERNAL;

        }

    }
    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_port_rate_egress_traffic_get(int unit,
                                     bcm_port_t port,
                                     uint32 *traffic_types,
                                     uint32 *kbits_sec,
                                     uint32 *kbits_burst)
{
    int rv = BCM_E_UNAVAIL;
    uint32 uShaperSrc;   /* The fifo to shaper select bits */
    int nShaperConflicts;  /* Check if multiple shapers already connect the desired FIFO */
    uint32 uMatchControl;
    int shaper_id;
#ifdef BCM_SIRIUS_SUPPORT
    int level = 0, node = 0;
#endif /* BCM_SIRIUS_SUPPORT */

#ifdef SHAPER_DEBUG
    int i;
    uint32 uData0, uData1, uData2;
#endif

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    uShaperSrc = *traffic_types;

    if (soc_feature(unit, soc_feature_egress_metering)) {

        if (SOC_IS_SBX_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
            /* in case of port create at init time, allow user to pass in child/egress child type */
            if (BCM_GPORT_IS_CHILD(port)) {
                BCM_GPORT_EGRESS_GROUP_SET(port, BCM_GPORT_CHILD_MODID_GET(port),
                                           BCM_GPORT_CHILD_PORT_GET(port));
            } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
                BCM_GPORT_EGRESS_GROUP_SET(port, BCM_GPORT_EGRESS_CHILD_MODID_GET(port),
                                           BCM_GPORT_EGRESS_CHILD_PORT_GET(port));                
            }

            if (!BCM_GPORT_IS_EGRESS_GROUP(port)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_get requires egress group gport format on unit %d\n"),
                             unit));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }

            rv = bcm_sbx_port_get_scheduler(unit, port, &level, &node);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_get failed: %s\n"),
                             bcm_errmsg(rv)));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            switch (*traffic_types) {
                case BCM_PORT_RATE_TRAFFIC_UC_EF:
                    break;
                case BCM_PORT_RATE_TRAFFIC_UC_NON_EF:
                    node++;
                    break;
                case BCM_PORT_RATE_TRAFFIC_NON_UC_EF:
                    node += 2;
                    break;
                case BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF:
                    node += 3;
                    break;
                default:
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "bcm_port_rate_egress_traffic_get unrecognized traffic type on unit %d\n"),
                                 unit));
                    BCM_SBX_UNLOCK(unit);
                    return BCM_E_PARAM;
            }

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_shaper_params, (unit,
                                      level, node, NULL, NULL, (int *)kbits_sec, (int *)kbits_burst));
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_PORT,
                            (BSL_META_U(unit,
                                        "bcm_port_rate_egress_traffic_get failed: %s\n"),
                             bcm_errmsg(rv)));
            } else {
                /* mbcm call return bits instead of kbits, convert it */
                *kbits_burst /= 1000;
            }
            BCM_SBX_UNLOCK(unit);
            return rv;
#endif /* BCM_SIRIUS_SUPPORT */
        }

#ifdef SHAPER_DEBUG
        for( i = 0; i < 30; i++) {
            rv = soc_qe2000_eg_mem_read(unit, i, 0x2, &uData0, &uData1, &uData2);
            LOG_CLI((BSL_META_U(unit,
                                "*** _eg_mem_read: Table %d: %d Data 0x%x 0x%x 0x%x\n"), 2, i, uData0, uData1, uData2));
        }

        for( i = 0; i < 30; i++) {
            rv = soc_qe2000_eg_mem_read(unit, i, 0x5, &uData0, &uData1, &uData2);
            LOG_CLI((BSL_META_U(unit,
                                "*** _eg_mem_read: Table %d: %d Data 0x%x 0x%x 0x%x\n"), 5, i, uData0, uData1, uData2));
        }

        for( i = 100; i < 106; i++) {
            rv = soc_qe2000_eg_mem_read(unit, i, 0x5, &uData0, &uData1, &uData2);
            LOG_CLI((BSL_META_U(unit,
                                "*** _eg_mem_read: Table %d: %d Data 0x%x 0x%x 0x%x\n"), 5, i, uData0, uData1, uData2));
        }

        for( i = 150; i < 156; i++) {
            rv = soc_qe2000_eg_mem_read(unit, i, 0x5, &uData0, &uData1, &uData2);
            LOG_CLI((BSL_META_U(unit,
                                "*** _eg_mem_read: Table %d: %d Data 0x%x 0x%x 0x%x\n"), 5, i, uData0, uData1, uData2));
        }
#endif


        rv = _bcm_sbx_port_check_egress_shapers(unit, port,
                                                BCM_SBX_PORT_EGRESS_SHAPER_FIFO,
                                                uShaperSrc,
                                                &shaper_id,
                                                &nShaperConflicts,
                                                &uMatchControl);
#ifdef SHAPER_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "_bcm_sbx_port_check_egress_shapers rv 0x%x ShaperSrc 0x%x ShpId %d Confl %d MatchCtl 0x%x\n"),
                 rv, uShaperSrc, shaper_id, nShaperConflicts, uMatchControl));
#endif

        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "Shaper Database Check failed: %s\n"),
                         bcm_errmsg(rv)));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        if ((nShaperConflicts == 0) && (shaper_id != -1) &&
                (uMatchControl == BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH)) {

        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_rate_egress_traffic_get, (unit, port, shaper_id, traffic_types, kbits_sec, kbits_burst));
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "bcm_port_rate_egress_get failed: %s\n"),
                         bcm_errmsg(rv)));
        }
        BCM_SBX_UNLOCK(unit);
        return rv;

        } else {
#ifdef SHAPER_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "Port & Shaper Match Not found. Conflicts %d, MatchControl 0x%x\n"),
                     nShaperConflicts, uMatchControl));
#endif
            BCM_SBX_UNLOCK(unit);
            return (BCM_E_INTERNAL);
        }
    }
    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_frame_max_set(int unit,
                           bcm_port_t port,
                           int size)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
    rv = BCM_E_NONE;
    if (!SAL_BOOT_BCMSIM)
        /* BCMSIM model doesn't support 64bits registers access for now,
         * and MAC_INIT has some 64bits registers access. Disable it
         * for now
         */
#endif
    {
        rv = MAC_FRAME_MAX_SET(SBX_PORT(unit, port).p_mac, unit, port, size);
    }
    } else {
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_frame_max_set, (unit, port, size));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_frame_max_get(int unit,
                           bcm_port_t port,
                           int *size)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
    *size = 0x5ee; /* use reset value */
    rv = BCM_E_NONE;
    if (!SAL_BOOT_BCMSIM)
        /* BCMSIM model doesn't support 64bits registers access for now,
         * and MAC_INIT has some 64bits registers access. Disable it
         * for now
         */
#endif
    {
        rv = MAC_FRAME_MAX_GET(SBX_PORT(unit, port).p_mac, unit, port, size);
    }
    } else {
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_frame_max_get, (unit, port, size));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_ifg_set(int unit,
                     bcm_port_t port,
                     int speed,
                     bcm_port_duplex_t duplex,
                     int ifg)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);
    if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
    rv = BCM_E_NONE;
    if (!SAL_BOOT_BCMSIM)
        /* BCMSIM model doesn't support 64bits registers access for now,
         * and MAC_INIT has some 64bits registers access. Disable it
         * for now
         */
#endif
    {
        rv = MAC_IFG_SET(SBX_PORT(unit, port).p_mac, unit, port, speed, duplex, ifg);
    }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_ifg_get(int unit,
                     bcm_port_t port,
                     int speed,
                     bcm_port_duplex_t duplex,
                     int *ifg)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
    *ifg = (0xc * 8); /* use reset value */
    rv = BCM_E_NONE;
    if (!SAL_BOOT_BCMSIM)
        /* BCMSIM model doesn't support 64bits registers access for now,
         * and MAC_INIT has some 64bits registers access. Disable it
         * for now
         */
#endif
    {
        rv = MAC_IFG_GET(SBX_PORT(unit, port).p_mac, unit, port, speed, duplex, ifg);
    }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}
/*
 * Function:
 *      bcm_port_phy_set
 * Description:
 *      General PHY register write
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_port_phy_set(int unit, bcm_port_t port, uint32 flags,
                     uint32 phy_reg_addr, uint32 phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

#if 000 /* gport unsupported for now */
    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_sbx_port_gport_validate(unit, port, &port));
    }
#endif
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_sbx_port_phy_set: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x phy_data=0x%08x\n"),
              unit, port, flags, phy_reg_addr, phy_data));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        BCM_SBX_LOCK(unit);
        rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
        BCM_SBX_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & 0xffff);
        BCM_SBX_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_write(unit, phy_id, phy_devad, 
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
        }
        BCM_SBX_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *      bcm_port_phy_get
 * Description:
 *      General PHY register read
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - (OUT) Data that was read
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_port_phy_get(int unit, bcm_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 *phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint32 reg_flag;
    int    rv;

#if 000 /* gport unsupported for now */
    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_sbx_port_gport_validate(unit, port, &port));
    }
#endif
    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        BCM_SBX_LOCK(unit);
        rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
        BCM_SBX_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        BCM_SBX_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad, 
                                  phy_reg, &phy_rd_data);

        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
        }
        BCM_SBX_UNLOCK(unit);

        if (BCM_SUCCESS(rv)) {
           *phy_data = phy_rd_data;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_sbx_port_phy_get: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x, phy_data=0x%08x, rv=%d\n"),
              unit, port, flags, phy_reg_addr, *phy_data, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_modify
 * Description:
 *      General PHY register modify
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 *      phy_mask - Bits to modify using phy_data
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_port_phy_modify(int unit, bcm_port_t port, uint32 flags,
                        uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

#if 000 /* gport unsupported for now */
    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_sbx_port_gport_validate(unit, port, &port));
    }
#endif
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_sbx_port_phy_modify: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x phy_data=0x%08x phy_mask=0x%08x\n"),
              unit, port, flags, phy_reg_addr, phy_data, phy_mask));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        BCM_SBX_LOCK(unit);
        rv = soc_phyctrl_reg_modify(unit, port, flags, phy_reg_addr,
                                    phy_data, phy_mask);
        BCM_SBX_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & phy_mask & 0xffff);
        BCM_SBX_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad, 
                                  phy_reg, &phy_rd_data);
            phy_wr_data |= (phy_rd_data & ~phy_mask);
            rv = soc_miimc45_write(unit, phy_id, phy_devad, 
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
            if (BCM_SUCCESS(rv)) {
                phy_wr_data |= (phy_rd_data & ~phy_mask);
                rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
            }
        }
        BCM_SBX_UNLOCK(unit);
    }
    return rv;
}

int
bcm_sbx_port_phy_reset(int unit,
                       bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_phy_reset_register(int unit,
                                bcm_port_t port,
                                bcm_port_phy_reset_cb_t callback,
                                void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_phy_reset_unregister(int unit,
                                  bcm_port_t port,
                                  bcm_port_phy_reset_cb_t callback,
                                  void *user_data)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_port_fault_get(int unit,
                       bcm_port_t port,
                       uint32 *flags)
{
    int rv = BCM_E_UNAVAIL;
    uint64 val64;
    
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if( flags == NULL) {
        return BCM_E_PARAM;
    }

    if (!IS_HG_PORT(unit, port) &&
        !IS_XE_PORT(unit, port)) {
        return BCM_E_UNAVAIL;
    }

    *flags = 0; 

    if (SOC_IS_SIRIUS(unit)) {

        SOC_IF_ERROR_RETURN(READ_MAC_RXLSSSTATr(unit, port, &val64));
        
        if (soc_reg64_field32_get(unit, MAC_RXLSSSTATr, val64, REMOTEFAULTSTATf)) { 
            *flags |= BCM_PORT_FAULT_REMOTE; 
        }
        
        if (soc_reg64_field32_get(unit, MAC_RXLSSSTATr, val64, LOCALFAULTSTATf)) { 
            *flags |= BCM_PORT_FAULT_LOCAL; 
        }
        
        rv = BCM_E_NONE;
    }
    
    return rv;
}


int
bcm_sbx_port_control_set(int unit,
                         bcm_port_t port,
                         bcm_port_control_t type,
                         int value)
{
    int rv = BCM_E_UNAVAIL;
    int node, modid;
    int sfi_port;
    
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    
    BCM_SBX_LOCK(unit);

    node = SOC_SBX_CONTROL(unit)->node_id;    

    if ((type == bcmPortControlAbility) && (SOC_IS_SIRIUS(unit))) {
        switch (value) {
        
        case BCM_PORT_ABILITY_SFI:
        case BCM_PORT_ABILITY_SCI:
        case BCM_PORT_ABILITY_DUAL_SFI:
        case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:
        case BCM_PORT_ABILITY_SFI_LOOPBACK:
        case BCM_PORT_ABILITY_SFI_SCI:
            
            if (BCM_GPORT_IS_MODPORT(port)) {
                modid = BCM_GPORT_MODPORT_MODID_GET(port);
                node = BCM_STK_MOD_TO_NODE(modid);
                port = BCM_GPORT_MODPORT_PORT_GET(port);
            }
            
            rv = bcm_sbx_port_to_sfi_base_get(unit, node, port, &sfi_port);
            if (rv != BCM_E_NONE) { 
                rv = BCM_E_PARAM;
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
            SOC_SBX_STATE(unit)->port_state->ability[sfi_port][node] = value;
            break;
            
        default:
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: unknown ability(%d)\n"),
                       value));
            rv = BCM_E_PARAM;
            break;
            
        }
      
        /* If setting the ability for a different, node, there is no configuration required */
        if (SOC_SBX_CONTROL(unit)->node_id != node) {
            BCM_SBX_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_control_set, (unit, port, type, value));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: port control set failed error(%d)\n"),
                   rv));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }
    
    BCM_SBX_UNLOCK(unit);
    return rv;
}
/* Return the ability for the given node/port - port is the base sfi_port */
int
bcm_sbx_port_node_ability_get(int unit, 
                              int sfi_port, 
                              int node, 
                              int *ability)
{
    
   *ability = SOC_SBX_STATE(unit)->port_state->ability[sfi_port][node];
   return (BCM_E_NONE);
}

int
bcm_sbx_port_control_get(int unit,
                         bcm_port_t port,
                         bcm_port_control_t type,
                         int *value)
{
    int rv = BCM_E_UNAVAIL;
    int modid;
    int node;
    int sfi_port;


    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    /* Determine whether the port is a gport - the port can be overlaid with a gport */
    /* this is required for setting the ability so that we can store the abilities   */
    /* for all nodes so that we can get the set of logical crossbars between nodes.  */
    /* The function bcm_fabric_crossbar_connection_status_get() and                  */
    /* bcm_sbx_fabric_connection_bytes_get() won't work unless the user plays the    */
    /* bcm_port_control_set(ability) command on all units using a gport overlaying   */
    /* the port field.                                                               */
    if (BCM_GPORT_IS_MODPORT(port) && (type == bcmPortControlAbility)) {

        modid = BCM_GPORT_MODPORT_MODID_GET(port);
        node = modid - BCM_MODULE_FABRIC_BASE;

        rv = bcm_sbx_port_to_sfi_base_get(unit, node, BCM_GPORT_MODPORT_PORT_GET(port), &sfi_port);
        if (rv != BCM_E_NONE) { 
            rv = BCM_E_PARAM;
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        *value = SOC_SBX_STATE(unit)->port_state->ability[sfi_port][node];
        /* LOG_CLI((BSL_META_U(unit,
                               "ability[sfi_port %d][node %d] = ability %d\n"), port, node, value)); */
        
        rv = BCM_E_NONE;
        
    } else {    

        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_control_get, (unit, port, type, value));
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: port control get failed error(%d)\n"),
                       rv));
        }
    }
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_port_phy_control_set(int unit,
                             bcm_port_t port,
                             bcm_port_phy_control_t type,
                             uint32 value)
{
    int rv = BCM_E_UNAVAIL;

    if (SOC_IS_SBX_BM9600(unit) || SOC_IS_SBX_SIRIUS(unit)) {
        /* coverity[mixed_enums] */
        rv = soc_phyctrl_control_set(unit, port, type, value);
    }
    return rv;
}

int
bcm_sbx_port_phy_control_get(int unit,
                             bcm_port_t port,
                             bcm_port_phy_control_t type,
                             uint32 *value)
{
    int rv = BCM_E_UNAVAIL;

    if (SOC_IS_SBX_BM9600(unit) || SOC_IS_SBX_SIRIUS(unit)) {
        /* coverity[mixed_enums] */
        rv = soc_phyctrl_control_get(unit, port, type, value);
    }
    return rv;
}

int
bcm_sbx_port_phy_firmware_set(int unit,
                              bcm_port_t port,
                              uint32 flags,
                              int offset,
                              uint8 *array,
                              int length)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_port_get_scheduler(int unit,
                           bcm_gport_t gport,
                           int *scheduler_level,
                           int *scheduler_node)
{
    int rv = BCM_E_NONE;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_scheduler_get, (unit, gport, scheduler_level, scheduler_node));

    return rv;
}

/* 
 * Mapping function to get interface port and port offset for a fabric port
 * In:  subport      - a fabric port id
 *                   - or a MODPORT gport
 *                   - or a EGRESS_MODPORT gport
 * Out: port         - interface port (type of bcm_port_t)
 *      port_offset  - subport offset within the interface port (or -1 if a gport is specified)
 */
int
bcm_sbx_port_get_port_portoffset(int unit,
                                 int subport,
                                 bcm_port_t *port,
                                 int *port_offset)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info;
    bcm_gport_t parent_gport;

    if (SOC_IS_SBX_SIRIUS(unit)) {
        if ( BCM_GPORT_IS_MODPORT(subport) || BCM_GPORT_IS_EGRESS_MODPORT(subport) ) {
            if ( BCM_GPORT_IS_MODPORT(subport) ) {
                /* MODPORT gport */
                *port = BCM_GPORT_MODPORT_PORT_GET(subport);
            } else {
                /* EGRESS_MODPORT gport */
                *port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(subport);
            }
            if (!SOC_PORT_VALID(unit, *port)) {
                return BCM_E_PORT;
            }
            *port_offset = -1;
        } else {
            if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, DIAG_EMULATOR_PARTIAL_INIT)) {
                /* For DV ONLY */
                *port = subport;
                *port_offset = 0;
                return BCM_E_NONE;
            }

            /* subport id */
            if ((subport < 0) || (subport >=  SB_FAB_DEVICE_MAX_FABRIC_PORTS)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "ERROR: %s, subport 0x%x out of range, unit %d\n"),
                           FUNCTION_NAME(), subport, unit));
                return BCM_E_PARAM;
            }
            
            if ( (SOC_SBX_STATE(unit)->port_state == NULL) ||
                 (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) ) {
                return BCM_E_INIT;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if (sp_info->valid == FALSE) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "ERROR: %s, invalid fabric port 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, unit));
                return BCM_E_PARAM;
            }        

            if ((sp_info->flags & (SBX_SUBPORT_FLAG_ON_TS | SBX_SUBPORT_FLAG_INGRESS_MCAST)) ==
                                     (SBX_SUBPORT_FLAG_ON_TS | SBX_SUBPORT_FLAG_INGRESS_MCAST)) {
                return BCM_E_PORT;
            }
            
            parent_gport = sp_info->parent_gport;

            if (BCM_GPORT_IS_EGRESS_MODPORT(parent_gport)) {
                /* if parent gport is egress, convert to ingress */
                BCM_GPORT_MODPORT_SET(parent_gport, 
                                      BCM_GPORT_EGRESS_MODPORT_MODID_GET(parent_gport),
                                      BCM_GPORT_EGRESS_MODPORT_PORT_GET(parent_gport));
            }
            
            if (!BCM_GPORT_IS_MODPORT(parent_gport)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "ERROR: %s, invalid parent interface port 0x%x, unit %d\n"),
                           FUNCTION_NAME(), parent_gport, unit));
                return BCM_E_PARAM;
            } else {
                *port = BCM_GPORT_MODPORT_PORT_GET(parent_gport);
                if (!(IS_GX_PORT(unit, *port) || 
                      IS_CPU_PORT(unit, *port) ||
                      IS_REQ_PORT(unit, *port))) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "ERROR: %s, Invalid gport 0x%x, unit %d\n"),
                               FUNCTION_NAME(), parent_gport, unit));
                    return BCM_E_PARAM;
                }
            }
            
            *port_offset = sp_info->port_offset;
        }
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

#ifdef BCM_SBX_SUPPORT
/* 
 * Mapping function to get hardware interface ID and port offset for a fabric port
 * In:  subport      - a fabric port id
 *                   - or a MODPORT gport
 *                   - or a EGRESS_MODPORT gport
 * Out: intf         - hardware interface ID
 *      port_offset  - subport offset within the interface (or -1 if a gport is specified)
 */
int
bcm_sbx_port_get_intf_portoffset(int unit,
                                 int subport,
                                 int *intf,
                                 int *port_offset)
{
    int rv = BCM_E_NONE;
    int port;

    rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, port_offset);
    if (rv == BCM_E_NONE) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }

#ifdef BCM_SIRIUS_SUPPORT
        /* for now, only support sirius device */
        if (SOC_IS_SIRIUS(unit)) {
            if (IS_GX_PORT(unit, port)) {
                *intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + SOC_PORT_OFFSET(unit,port);
            } else if (IS_CPU_PORT(unit, port)) {
                *intf = SB_FAB_DEVICE_SIRIUS_CPU_INTF;
            } else if (IS_REQ_PORT(unit, port)) { 
                *intf = SB_FAB_DEVICE_SIRIUS_RQ0_INTF + SOC_PORT_OFFSET(unit,port);
            } else {
                return BCM_E_PARAM;
            }
        }
        else
#endif 
        {
            *intf = -1;
            *port_offset = -1;
            return BCM_E_UNAVAIL;
        }
    } else {
        *intf = -1;
        *port_offset = -1;
    }

    return rv;
}
#endif /* BCM_SBX_SUPPORT */

#ifdef BCM_QE2000_SUPPORT
static int
_bcm_sbx_port_shaper_state_init(int unit)
{
    int rv = BCM_E_NONE;
    int shaper_count, i;


    /* determine if shaper functionality is supported */
    if (!(soc_feature(unit, soc_feature_egress_metering))) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Shaper Not Supported Unit: %d\n"),unit ));
        return(rv);
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Shaper Supported Unit: %d\n"),unit ));

    /* allocate Shaper Control data structures */
    shaper_count = SOC_SBX_CFG(unit)->nShaperCount;
    shaper_state[unit] = sal_alloc(sizeof(bcm_sbx_port_shaper_state_t) * shaper_count, "shaper_control_memory");
    if (shaper_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    /* initialize data structures */
    for (i = 0; i < shaper_count; i++) {
        sal_memset((shaper_state[unit] + i), 0, sizeof(bcm_sbx_port_shaper_state_t));
        (shaper_state[unit] + i)->shaper_type = BCM_SBX_PORT_EGRESS_SHAPER_UNKNOWN;
        (shaper_state[unit] + i)->is_free = TRUE;
        (shaper_state[unit] + i)->shaper_src = 0;
        (shaper_state[unit] + i)->hi_side = -1;
        (shaper_state[unit] + i)->port = -1;
    }

    return(rv);
}
#endif /* BCM_QE2000_SUPPORT */


static int
_bcm_sbx_port_egress_shaperid_get(int unit,
                                  bcm_port_t port,
                                  int32 * nShaperId)

{
    bcm_sbx_port_shaper_state_t *p_shaper_state;
    int i, shaper_count;

    p_shaper_state = shaper_state[unit];

    shaper_count = SOC_SBX_CFG(unit)->nShaperCount;

    for ( i = 0 ; i < shaper_count; i++) {

        if(p_shaper_state[i].is_free == TRUE) {
            *nShaperId = i;
#ifdef SHAPER_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "Found Shaper %d Free\n"), i));
            LOG_CLI((BSL_META_U(unit,
                                "3>Setting Shaper %d is_free = FALSE\n"), i));
#endif
            p_shaper_state[i].is_free = FALSE;
            return (BCM_E_NONE);

        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "No Shaper Found %d Free\n"), i));

    /* We are out of shapers */
    return (BCM_E_INTERNAL);


}

static int
_bcm_sbx_port_check_egress_shapers(int unit,
                                   bcm_port_t port,
                                   int nShaperType,
                                   uint32 uShaperSrc,
                                   int *nShaperId,
                                   int *nShaperConflicts,
                                   uint32 *uMatchControl)

{
    int rv = BCM_E_NONE;
#ifdef BCM_QE2000_SUPPORT
    bcm_sbx_port_shaper_state_t *p_shaper_state;
    int i, shaper_count;
#ifdef BCM_EASY_RELOAD_SUPPORT
    uint32 shaperIdHi, shaperIdLo;
    int32 nFifoParmIndex;
#endif
    /* Set some default values */

    *nShaperId = -1;
    *nShaperConflicts = 0;
    *uMatchControl = 0;

    /* Get System Parameters */
    p_shaper_state = shaper_state[unit];
    shaper_count = SOC_SBX_CFG(unit)->nShaperCount;


    /* If we are in easy_reload, first check whether the port has already been */
    /* allocated a shaper (port or fifo)                                       */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT

        if (nShaperType == BCM_SBX_PORT_EGRESS_SHAPER_PORT) {

            nFifoParmIndex = port * 2;

            rv = bcm_qe2000_fifo_egress_shaper_get(unit, nFifoParmIndex,
                                                   &shaperIdHi, &shaperIdLo);

            if (rv != 0) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "ERROR: port FIFO params read for shaper failed FIFO(%d)\n"),
                           nFifoParmIndex ));
                return rv;
            }
            if (shaperIdHi != 0xff) {
                LOG_CLI((BSL_META_U(unit,
                                    "Port shaper(%d) in use on port(%d) returning this shaper\n"), shaperIdHi, port));
                *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                *nShaperId = shaperIdHi;
                return BCM_E_NONE;
            }else {
                return rv;
            }
        } else { /* BCM_SBX_PORT_EGRESS_SHAPER_FIFO */

            if (uShaperSrc & BCM_PORT_RATE_TRAFFIC_UC_EF) {

                nFifoParmIndex = port * 2;

                rv = bcm_qe2000_fifo_egress_shaper_get(unit, nFifoParmIndex,
                                                       &shaperIdHi, &shaperIdLo);
                if (rv != 0) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "ERROR: port FIFO params read for shaper failed FIFO(%d)\n"),
                               nFifoParmIndex ));
                    return rv;
                }

                if (shaperIdLo != 0xff) {
                    LOG_CLI((BSL_META_U(unit,
                                        "FIFO shaper(%d) in use on port(%d) EF unicast FIFO returning this shaper\n"), shaperIdLo, port));
                    *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                    *nShaperId = shaperIdLo;
                }
            }
            else if (uShaperSrc & BCM_PORT_RATE_TRAFFIC_UC_NON_EF) {

                nFifoParmIndex = (port * 2) + 1;

                rv = bcm_qe2000_fifo_egress_shaper_get(unit, nFifoParmIndex,
                                                       &shaperIdHi, &shaperIdLo);
                if (rv != 0) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "ERROR: port FIFO params read for shaper failed FIFO(%d)\n"),
                               nFifoParmIndex ));
                    return rv;
                }

                if (shaperIdLo != 0xff) {
                    LOG_CLI((BSL_META_U(unit,
                                        "FIFO shaper(%d) in use on port(%d) non EF unicast FIFO returning this shaper\n"), shaperIdLo, port));
                    *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                    *nShaperId = shaperIdLo;
                }
            }
            else if (uShaperSrc & BCM_PORT_RATE_TRAFFIC_NON_UC_EF) {

                nFifoParmIndex = port + 100;

                rv = bcm_qe2000_fifo_egress_shaper_get(unit, nFifoParmIndex,
                                                       &shaperIdHi, &shaperIdLo);

                if (rv != 0) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "ERROR: port FIFO params read for shaper failed FIFO(%d)\n"),
                               nFifoParmIndex ));
                    return rv;
                }

                if (shaperIdLo != 0xff) {
                    LOG_CLI((BSL_META_U(unit,
                                        "FIFO shaper(%d) in use on port(%d) EF multicast FIFO returning this shaper\n"), shaperIdLo, port));
                    *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                    *nShaperId = shaperIdLo;
                }
            }
            else if (uShaperSrc & BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF) {

                nFifoParmIndex = port + 150;

                rv = bcm_qe2000_fifo_egress_shaper_get(unit, nFifoParmIndex,
                                                       &shaperIdHi, &shaperIdLo);
                if (rv != 0) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "ERROR: port FIFO params read for shaper failed FIFO(%d)\n"),
                               nFifoParmIndex ));
                    return rv;
                }

                if (shaperIdLo != 0xff) {
                    LOG_CLI((BSL_META_U(unit,
                                        "FIFO shaper(%d) in use on port(%d) non EF multicast FIFO returning this shaper\n"), shaperIdLo, port));
                    *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                    *nShaperId = shaperIdLo;
                }
            }
        }
#endif /* BCM_EASY_RELOAD_SUPPORT */
    } else {
        /* only go through remaining code, if in EASY_RELOAD and *not* reloading */


    /* Check the shaper control table */
    for ( i = 0 ; i < shaper_count; i++) {
#ifdef SHAPER_DEBUG
        if((p_shaper_state[i].is_free != TRUE) ||
           (p_shaper_state[i].port != -1) ||
           (p_shaper_state[i].shaper_type != BCM_SBX_PORT_EGRESS_SHAPER_UNKNOWN)) {
            LOG_CLI((BSL_META_U(unit,
                                "> Shaper %d Free:%d Type:%d Src 0x%x Port %d Hi:%d.\n"),
                     i,
                     p_shaper_state[i].is_free,
                     p_shaper_state[i].shaper_type,
                     p_shaper_state[i].shaper_src,
                     p_shaper_state[i].port,
                     p_shaper_state[i].hi_side
                     ));
        }
#endif
         if( p_shaper_state[i].is_free == FALSE  ) {

             if( port == p_shaper_state[i].port) {

                 if((nShaperType == BCM_SBX_PORT_EGRESS_SHAPER_FIFO )
                    && (nShaperType == p_shaper_state[i].shaper_type)) {

                     LOG_VERBOSE(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "FIFO Shaper Supported Unit: %d\n"),unit ));

                     if( uShaperSrc == p_shaper_state[i].shaper_src ) {

                         *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                         LOG_VERBOSE(BSL_LS_BCM_PORT,
                                     (BSL_META_U(unit,
                                                 "Shaper Direct Match Unit: %d\n"),unit ));

                         if(  *nShaperId == -1) {
                             *nShaperId = i;  /* Set Id, continue to look for conflicts */
                         } else {
                             (*nShaperConflicts)++;
                         }
                     } else if ( uShaperSrc & p_shaper_state[i].shaper_src) {

                         *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_PARTIAL_MATCH;

                         if(  *nShaperId == -1) {
                             *nShaperId = i;  /* Set Id, continue to look for conflicts */
                         } else {
                             (*nShaperConflicts)++;
                         }

                     } else {
#ifdef SHAPER_DEBUG
                         LOG_CLI((BSL_META_U(unit,
                                             "*P no shaper type match found 0x%x 0x%x\n"), uShaperSrc, p_shaper_state[i].shaper_src));
#endif
                     }


                 } else if((nShaperType == BCM_SBX_PORT_EGRESS_SHAPER_PORT)
                           && (nShaperType  == p_shaper_state[i].shaper_type)) {
                     LOG_VERBOSE(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "Port Shaper Supported Unit: %d\n"),unit ));
                     /* LOG_CLI((BSL_META_U(unit,
                                            "BCM_SBX_PORT_EGRESS_SHAPER_PORT found\n")));
                      */

                     if( uShaperSrc == p_shaper_state[i].shaper_src  ) {
                         /* LOG_CLI((BSL_META_U(unit,
                                                "Match Replace!\n")));
                          */
                         *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH;
                         LOG_VERBOSE(BSL_LS_BCM_PORT,
                                     (BSL_META_U(unit,
                                                 "Shaper Direct Match Unit: %d\n"),unit ));
                         if(  *nShaperId == -1) {
                             *nShaperId = i;  /* Set Id, continue to look for conflicts */
                         } else {
                             (*nShaperConflicts)++;
                         }
                     } else if ( uShaperSrc & p_shaper_state[i].shaper_src) {
                         /* This should not happen as port is always configured with all fifos */
                         LOG_CLI((BSL_META_U(unit,
                                             "Partial Match! 0x%x\n"), *uMatchControl));
                         *uMatchControl |= BCM_SBX_PORT_EGRESS_SHAPER_PARTIAL_MATCH;
                         if(  *nShaperId == -1) {
                             *nShaperId = i;  /* Set Id, continue to look for conflicts */
                         } else {
                             (*nShaperConflicts)++;
                         }
                     }

                 } else {
#ifdef SHAPER_DEBUG
                     LOG_CLI((BSL_META_U(unit,
                                         "F> no shaper type match found 0x%x 0x%x\n"), uShaperSrc, p_shaper_state[i].shaper_src));
                     /* If we have good house keeping this should not happen */
                     LOG_VERBOSE(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "Shaper Unknown Unit: %d\n"),unit ));
                     return (BCM_E_INTERNAL);
#endif
                 }
             } else {
                 /* LOG_CLI((BSL_META_U(unit,
                                        "port mismatch\n"))); */
             }

         } else {
             /* is_free is true */
         }
    }
    }

#ifdef SHAPER_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "First ShaperId Found %d Conflicts %d Control 0x%x Type %d\n"),
             *nShaperId, *nShaperConflicts, *uMatchControl, nShaperType));
#endif

#endif /* BCM_QE2000_SUPPORT */

    return rv;
}

int
bcm_sbx_port_congestion_config_get(int unit, bcm_gport_t port, bcm_port_congestion_config_t *config)
{
    int rv = BCM_E_UNAVAIL;


    BCM_SBX_LOCK(unit);

#if 0
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PORT);
    }
#endif /* 0 */

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_congestion_config_get, (unit, port, config, NULL));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion get failed, unit(%d) error(%d)\n"),
                   unit, rv));
    }

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_port_congestion_config_set(int unit, bcm_gport_t port, bcm_port_congestion_config_t *config)
{
    int rv = BCM_E_UNAVAIL;


    BCM_SBX_LOCK(unit);

#if 0
    if (!SOC_PORT_VALID(unit, port)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PORT);
    }
#endif /* 0 */

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_congestion_config_set, (unit, port, config, NULL));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion set failed, unit(%d) error(%d)\n"),
                   unit, rv));
    }

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_port_congestion_get(int unit,
                            bcm_gport_t congestion_port,
                            bcm_gport_t port,
                            uint32 flags,
                            int *channel_id)
{
    int rv = BCM_E_UNAVAIL;

    if (!(BCM_GPORT_IS_MODPORT(congestion_port) ||
          BCM_GPORT_IS_EGRESS_MODPORT(congestion_port) ||
          BCM_GPORT_IS_CONGESTION(congestion_port))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion set failed, unsupported congestion_port gport type unit(%d)\n"),
                   unit));
        return BCM_E_PARAM;        
    }        

    if (!(BCM_GPORT_IS_CHILD(port) ||
          BCM_GPORT_IS_EGRESS_CHILD(port) ||
          BCM_GPORT_IS_EGRESS_GROUP(port))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion set failed, unsupported port gport type unit(%d)\n"),
                   unit));
        return BCM_E_PARAM;        
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_port_congestion_get, (unit, congestion_port,
                              port, flags, channel_id));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion set failed, unit(%d) error(%d)\n"),
                   unit, rv));
    }

    return(rv);
}

int
bcm_sbx_port_congestion_set(int unit,
                            bcm_gport_t congestion_port,
                            bcm_gport_t port,
                            uint32 flags,
                            int channel_id)
{
    int rv = BCM_E_UNAVAIL;
    bcm_port_congestion_config_t config;

    if (!(BCM_GPORT_IS_MODPORT(congestion_port) ||
          BCM_GPORT_IS_EGRESS_MODPORT(congestion_port) ||
          BCM_GPORT_IS_CONGESTION(congestion_port))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion get failed, unsupported congestion_port gport type unit(%d)\n"),
                   unit));
        return BCM_E_PARAM;        
    }        

    if (!(BCM_GPORT_IS_CHILD(port) ||
          BCM_GPORT_IS_EGRESS_CHILD(port) ||
          BCM_GPORT_IS_EGRESS_GROUP(port))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion get failed, unsupported port gport type unit(%d)\n"),
                   unit));
        return BCM_E_PARAM;        
    }

    config.flags = BCM_PORT_CONGESTION_CONFIG_RX;

    rv = bcm_sbx_port_congestion_config_get(unit, congestion_port, &config);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion config set failed, unit(%d) error(%d)\n"),
                   unit, rv));
        return rv;
    }

    /* for now this API is for HCFC RX only */
    if (!(config.flags & BCM_PORT_CONGESTION_CONFIG_HCFC)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: bcm_port_congestion_set requires bcm_port_congestion_config_set API issued,"
                               " and configed as HCFC flow control on unit(%d)\n"), unit));
        return BCM_E_PARAM;
    }
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_port_congestion_set, (unit, congestion_port,
                              port, flags, config, channel_id));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: congestion set failed, unit(%d) error(%d)\n"),
                   unit, rv));
    }

    return(rv);
}


int
bcm_sbx_port_any_is_encap_higig(int unit, int *is_encap_higig)
{
    int rv = BCM_E_UNAVAIL;
    int port, mode;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!SOC_IS_SBX_SIRIUS(unit)) {
        (*is_encap_higig) = FALSE;
        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    (*is_encap_higig) = FALSE;
    for (port = SOC_PORT_MIN(unit, hg); port < SOC_PORT_MAX(unit, hg); port++) {
        rv = bcm_sbx_port_encap_get(unit, port, &mode);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: bcm_sbx_port_encap_get failed, unit(%d) port(%d) error(%d)\n"),
                       unit, port, rv));
            continue;
        }
        if (mode == SOC_ENCAP_HIGIG2) {
            (*is_encap_higig) = TRUE;
            break;
        }
        else {
            (*is_encap_higig) = FALSE;
            break;
        }
    }

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
_bcm_sbx_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{
    int     rv = BCM_E_NONE;
    int     oldstate,newstate;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_port_link_get(unit, port, up);
#endif
    }
    else {

        if (hw) {
            pbmp_t hw_linkstat;
            
            rv = soc_linkscan_hw_link_get(unit, &hw_linkstat);
            
            *up = PBMP_MEMBER(hw_linkstat, port);
            
            /*
             * We need to confirm link down because we may receive false link
             * change interrupts when hardware and software linkscan are mixed.
             * Processing a false link down event is known to cause packet
             * loss, which is obviously unacceptable.
             */
            if(!(*up)) {
                rv = soc_phyctrl_link_get(unit, port, up);
            }
        } else {
            rv = soc_phyctrl_link_get(unit, port, up);
        }

        if (BCM_SUCCESS(rv)) {
            /* make callouts if state changed either by flag or measurement */
            oldstate = (0 != (SBX_PORT(unit, port).flags & SBX_PORT_STATE_IS_UP));
            newstate = (0 != (*up));
            if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE) ||
                (oldstate != newstate)) {
                soc_port_medium_t  medium;

                soc_phyctrl_medium_get(unit, port, &medium);
                soc_phy_medium_status_notify(unit, port, medium);
#ifdef BCM_SIRIUS_SUPPORT
                if ((SOC_IS_SIRIUS(unit)) && !(*up)) {
                    int rc;
                    /* link went down on Sirius; notify aggregate code */
                    rc = bcm_sirius_trunk_port_down(unit, port);
                    COMPILER_REFERENCE(rc);
                    /* but we don't really care about the result here... */
                }
#endif /* def BCM_SIRIUS_SUPPORT */
            }
            /* adjust 'current' state to reflect the measured state */
            if (newstate) {
                SBX_PORT(unit, port).flags |= SBX_PORT_STATE_IS_UP;
            } else {
                SBX_PORT(unit, port).flags &= (~(SBX_PORT_STATE_IS_UP));
            }
        }
        LOG_VERBOSE(BSL_LS_BCM_LINK,
                    (BSL_META_U(unit,
                                "_bcm_port_link_get: u=%d p=%d hw=%d up=%d rv=%d\n"),
                     unit, port, hw, *up, rv));
    }

    return rv;
}

int
bcm_sbx_port_local_get(int unit, bcm_gport_t gport, bcm_port_t *local_port)
{
    bcm_module_t my_mod, mod_out;
    bcm_port_t port_out;

    if (BCM_GPORT_IS_LOCAL(gport)) {
         *local_port = BCM_GPORT_LOCAL_GET(gport);
    } else if (BCM_GPORT_IS_LOCAL_CPU(gport)) {
        *local_port = CMIC_PORT(unit);
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        BCM_IF_ERROR_RETURN(
            bcm_stk_my_modid_get(unit, &my_mod));
        mod_out = BCM_GPORT_MODPORT_MODID_GET(gport);
        port_out = BCM_GPORT_MODPORT_PORT_GET(gport);

        if (mod_out == my_mod){
            *local_port = port_out;
        }

        if (!SOC_PORT_VALID(unit, *local_port)) {
            return BCM_E_PORT;
        }
    } else {
        return BCM_E_PORT;
    }

    return BCM_E_NONE;
}

/* Return a base sfi port (either QE or Sirius) based upon pbmp port given */
/* If BM9600 device, just return the SFI port                              */
int
bcm_sbx_port_to_sfi_base_get(int unit, int node, bcm_port_t port, int *sfi_port)
{
    int rv = BCM_E_NONE;
    
    if (node >= SB_FAB_DEVICE_MAX_NODES) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: node(%d) out of range\n"),
                   FUNCTION_NAME(), node));        
        return BCM_E_PARAM;
    }
    if (sfi_port == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: sfi_port null pointer passed\n"),
                   FUNCTION_NAME()));        
        return BCM_E_PARAM;
    }

    if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) ||
        (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2)) {
        /* QE2000 node */
        if (!BCM_PORT_IS_QE2000_SFI(port)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: port(%d) is not a valid QE2000 port \n"),
                       FUNCTION_NAME(), port));        
            return BCM_E_PARAM;
        }
        *sfi_port = port - SB_FAB_DEVICE_QE2000_SFI_PBMP_BASE;
    } else {
        /* Sirius node */
        if (!BCM_PORT_IS_SIRIUS_SFI(port) && !BCM_PORT_IS_SIRIUS_SCI(port)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: port(%d) is not a valid Sirius port \n"),
                       FUNCTION_NAME(), port));        
            return BCM_E_PARAM;
        }
        /* Sirius node */
        if (BCM_PORT_IS_SIRIUS_SFI(port)) {
            *sfi_port = port - SB_FAB_DEVICE_SIRIUS_SFI_PBMP_BASE;
        } else {
            /* SCI links will start right after last SFI in sfi_port arrays */
            *sfi_port = port - SB_FAB_DEVICE_SIRIUS_SCI_PBMP_BASE + SB_FAB_DEVICE_SIRIUS_SFI_LINKS;
        }
    }
    return rv;
}
int
bcm_sbx_sfi_base_to_port_get(int unit, int node, int sfi_port, bcm_port_t *port)
{
    int rv = BCM_E_NONE;
    
    if (node >= SB_FAB_DEVICE_MAX_NODES) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: node(%d) out of range\n"),
                   FUNCTION_NAME(), node));        
        return BCM_E_PARAM;
    }

    if (port == NULL) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s: port null pointer passed\n"),
                   FUNCTION_NAME()));        
        return BCM_E_PARAM;
    }
    if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) ||
        (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2)) {
        if (sfi_port > SB_FAB_DEVICE_QE2000_SFI_LINKS) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: qe2000 base sfi_port(%d) out of range\n"),
                       FUNCTION_NAME(), sfi_port));        
            return BCM_E_PARAM;
        }
        *port = sfi_port + SB_FAB_DEVICE_QE2000_SFI_PBMP_BASE; 
    } else { 
        if (sfi_port > SB_FAB_DEVICE_SIRIUS_SFI_LINKS) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s: qe2000 base sfi_port(%d) out of range\n"),
                       FUNCTION_NAME(), sfi_port));        
            return BCM_E_PARAM;
        }
        *port = sfi_port + SB_FAB_DEVICE_SIRIUS_SFI_PBMP_BASE;
    } 
    return rv;
}



#ifdef BCM_SIRIUS_SUPPORT
int
bcm_sbx_port_qinfo_get(int unit, bcm_gport_t gport, int *eg_n,
                       bcm_sbx_subport_info_t *sp_info)
{
    int rv = BCM_E_NONE, subport = -1;
    
    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, eg_n, NULL);
    if (rv == BCM_E_NONE) {
        sal_memcpy(sp_info, 
                   &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]), 
                   sizeof(bcm_sbx_subport_info_t));
    }

    return rv;
}
#endif


#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_port_get_state(int unit, char *pbuf)
{
    int rv;
    char *pbuf_current = pbuf;
    int nShaperId;
    bcm_sbx_port_shaper_state_t *p_shaper_state;

    p_shaper_state = shaper_state[unit];

    for (nShaperId = 0; nShaperId < SOC_SBX_CFG(unit)->nShaperCount; nShaperId++) {

        if (!p_shaper_state[nShaperId].is_free) {
            rv = sal_sprintf(pbuf_current, "egress shaper id(%d) is in use\n", nShaperId);
            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;

            rv = sal_sprintf(pbuf_current, "  type(%s) port(%d)\n",
                             (p_shaper_state[nShaperId].shaper_type==BCM_SBX_PORT_EGRESS_SHAPER_PORT)? "port":"fifo",
                             p_shaper_state[nShaperId].port);
            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;
        }

    }
    return BCM_E_NONE;
}
#endif /* EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* EASY_RELOAD_SUPPORT */
