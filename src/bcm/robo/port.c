/*
 * $Id: port.c,v 1.355 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    port.c
 * Purpose:     Tracks and manages ports.
 *      MAC/PHY interfaces are managed through respective drivers.
 *
 */
#include <shared/bsl.h>

#include <soc/ll.h>
#include <soc/robo/mcm/driver.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/phy.h>
#include <soc/ptable.h>
#include <soc/phyctrl.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/stg.h>
#include <bcm/rate.h>
#include <bcm/mirror.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/link.h>
#include <bcm_int/robo/stg.h>
#include <bcm_int/robo/rate.h>
#include <bcm_int/robo/vlan.h>
#include <bcm_int/robo/cosq.h>
#include <bcm_int/robo/subport.h>

#include <bcm_int/robo_dispatch.h>

static bcm_port_cfg_t   *robo_port_config[BCM_LOCAL_UNITS_MAX];

/* Waiting time for leave MAC low power mode */ 
#define BCM_ROBO_LOW_POWER_WAIT_TIME    500000 /* 500 ms */

/*
 * Define:
 *  PORT_INIT
 * Purpose:
 *  Causes a routine to return BCM_E_INIT if port is not yet initialized.
 */

#define PORT_INIT(unit) \
    if (_bcm_robo_port_init(unit) == FALSE) { return BCM_E_INIT; }
    
/*
 * Define:
 *  PORT_PARAM_CHECK
 * Purpose:
 *  Check unit and port parameters for most bcm_port api calls
 */
#define PORT_PARAM_CHECK(unit, port) do { \
    if (!SOC_UNIT_VALID(unit)) {return BCM_E_UNIT;}\
    if (!SOC_PORT_VALID(unit, port)) { return BCM_E_PORT; } \
    PORT_INIT(unit);} while (0);

/*
 * Function:
 *      _bcm_robo_pbmp_check
 * Description:
 *      Causes a routine to return BCM_E_PARAM if the specified
 *      port bitmap is out of range.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      pbmp  - (IN) Port Bitmap
 * Returns     : BCM_E_XXX
 */
int 
_bcm_robo_pbmp_check(int unit, bcm_pbmp_t pbmp)
{
    bcm_pbmp_t pbm_t;

    BCM_PBMP_ASSIGN(pbm_t, pbmp);
    BCM_PBMP_REMOVE(pbm_t, PBMP_ALL(unit));

    if (BCM_PBMP_NOT_NULL(pbm_t)) {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int
_bcm_robo_port_init(int unit)
{
    uint32      val = FALSE;
    int         rv = BCM_E_NONE;

    rv = DRV_PORT_STATUS_GET(unit, 0, DRV_PORT_STATUS_INIT, &val);
    if (rv < 0) {
        return FALSE;
    }

    if (val) {
        return TRUE;
    }
    
    return FALSE;
    
}

/*
 * Function    : _bcm_robo_modid_is_local
 * Description : Identifies if given modid is local on a given unit
 *
 * Parameters  : (IN)   unit      - BCM device number
 *               (IN)   modnd     - Module ID 
 *               (OUT)  result    - TRUE if modid is local, FALSE otherwise
 * Returns     : BCM_E_XXX
 */
int 
_bcm_robo_modid_is_local(int unit, bcm_module_t modid, int *result)
{
    bcm_module_t    mymodid;    
    int             rv;

    /* Input parameters check. */
    if (NULL == result) {
        return (BCM_E_PARAM);
    }

    /* Get local module id. */
    rv = bcm_robo_stk_my_modid_get(unit, &mymodid);
    if (BCM_E_UNAVAIL == rv) {
        if (0 == modid) {
            *result = TRUE;
        } else {
            *result = FALSE;
        }
        return (BCM_E_NONE);
    }

    if (mymodid == modid) {
        *result = TRUE;
    } else if (NUM_MODID(unit) == 2) {
        if (modid == (mymodid +1)) {
            *result = TRUE;
        } else {
            *result = FALSE;
        }
    } else {
        *result = FALSE;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_robo_port_gport_validate
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
int
_bcm_robo_port_gport_validate(int unit, 
                bcm_port_t port_in, bcm_port_t *port_out)
{
    PORT_INIT(unit); 

    if (BCM_GPORT_IS_SET(port_in)) {
        BCM_IF_ERROR_RETURN(
            bcm_robo_port_local_get(unit, port_in, port_out));
    } else if (SOC_PORT_VALID(unit, port_in)) { 
        *port_out = port_in;
    } else {
        return BCM_E_PORT; 
    }

    return BCM_E_NONE;
}

STATIC void
_bcm_robo_port_cfg_init(int unit)
{
    int         len = 0;
    int         port = 0;
    int         rv = BCM_E_NONE;
    uint32      inner_tag, outer_tag;
    uint32 tpid;
    bcm_port_cfg_t  * pcfg;
#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
    bcm_robo_port_phy_auto_pd_t *phy_apd_war_info;
#endif  /* #if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */

    /* set default PVID on each port*/
    outer_tag = BCM_VLAN_CTRL(0, 0, BCM_VLAN_DEFAULT);
    inner_tag = BCM_VLAN_CTRL(0, 0, BCM_VLAN_DEFAULT);
    BCM_PBMP_ITER(PBMP_ALL(unit), port){
        rv = DRV_PORT_VLAN_PVID_SET(unit, port, outer_tag, inner_tag);
        if (BCM_FAILURE(rv)){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "fail to set pvid in %s\n"),FUNCTION_NAME()));            
            return;
        }
    }
    
    if (robo_port_config[unit] == NULL){
        /* Allocate module database array */
        len = SOC_MAX_NUM_PORTS * sizeof(bcm_port_cfg_t);
        robo_port_config[unit] = sal_alloc(len, "ROBO Port Database");
        sal_memset(robo_port_config[unit], 0, len);
    }
    
    BCM_PBMP_ITER(PBMP_ALL(unit), port){
        /* keep SW info for the default PVID on each port */
        pcfg = &robo_port_config[unit][port];
        pcfg->pc_vlan = BCM_VLAN_DEFAULT;

        /* Initialize default double tag mode and tpid */
        pcfg->pc_dt_mode = BCM_PORT_DTAG_MODE_NONE;
        if (DRV_PORT_GET(unit, port, 
                  DRV_PORT_PROP_DTAG_TPID, &tpid) < 0) {
            tpid = 0x8100;
        }
        pcfg->pc_dt_tpid = tpid;

#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
        /* keep SW info for GE PHY's Auto-PowerDown WAR */
        if (IS_GE_PORT(unit, port)){
            phy_apd_war_info = sal_alloc(sizeof(bcm_robo_port_phy_auto_pd_t), 
                    "PHY Auto PowerDown WAR Info");

            phy_apd_war_info->auto_pd_war_lock = 
                    sal_mutex_create("bcm_port_apd_war_LOCK");
            if (phy_apd_war_info->auto_pd_war_lock == NULL) {
                sal_free(phy_apd_war_info);
                LOG_CLI((BSL_META_U(unit,
                                    "%s, Memory issue on port%d AutoPD WAR\n"),
                         FUNCTION_NAME(), port));
            }

            phy_apd_war_info->war_enable = FALSE;
            phy_apd_war_info->current_link = FALSE;
            phy_apd_war_info->current_auto_pd_mode = 0;
            phy_apd_war_info->last_soc_time = 0;
            pcfg->ge_phy_auto_pd_war_info = phy_apd_war_info;
            
        } else {
            pcfg->ge_phy_auto_pd_war_info = NULL;
        }
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */
    }

    
}

/*
 * Function:
 *      _bcm_robo_port_deinit
 * Purpose:
 *      De-initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_robo_port_deinit(int unit)
{
    uint32 status = 0;
    
    BCM_IF_ERROR_RETURN(DRV_PORT_STATUS_GET(
            unit, -1, DRV_PORT_STATUS_DETACH, &status));
    assert(status == TRUE);
    
    BCM_IF_ERROR_RETURN(soc_phy_common_detach(unit));
    
    if (NULL != robo_port_config[unit]) {
#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
        int port = 0;
        bcm_port_cfg_t *pcfg;
        bcm_robo_port_phy_auto_pd_t *phy_apd_war_info;

        BCM_PBMP_ITER(PBMP_ALL(unit), port){
            /* free SW info for GE PHY's Auto-PowerDown WAR */
            if (IS_GE_PORT(unit, port)){
                pcfg = &robo_port_config[unit][port];
                phy_apd_war_info = pcfg->ge_phy_auto_pd_war_info;
                if (phy_apd_war_info != NULL){
                    if (phy_apd_war_info->auto_pd_war_lock != NULL) {
                        sal_mutex_destroy(phy_apd_war_info->auto_pd_war_lock);
                        phy_apd_war_info->auto_pd_war_lock = NULL;
                    }
                    sal_free(phy_apd_war_info);
                }
                pcfg->ge_phy_auto_pd_war_info = NULL;
            }
        }
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */
        sal_free(robo_port_config[unit]);
    }
    robo_port_config[unit] = NULL;

    return BCM_E_NONE;
}

#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
int
_bcm_robo53101_phy_cfg_save(int unit)
{
    uint32	temp;
    bcm_pbmp_t    pbmp;
    bcm_port_t    port;
    bcm_port_cfg_t  *pcfg;
    bcm_robo_phy_cfg_t  *phy_cfg;
    int rv = BCM_E_NONE;

    temp = 0;
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_PHY_CFG_RESTORE_PBMP, &temp);
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP, &temp);
    }
    BCM_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, temp);
    PBMP_ITER(pbmp, port) {
        pcfg = &robo_port_config[unit][port];
        phy_cfg = &pcfg->phy_cfg;
        /* Save the PHY configuration before PHY reset */
        /* AN */
        rv = bcm_robo_port_autoneg_get(unit, port, &(phy_cfg->autoneg));
        BCM_IF_ERROR_RETURN(rv);
        /* Advertisement */
        rv = bcm_robo_port_advert_get(unit, port, &(phy_cfg->local_advert));
        BCM_IF_ERROR_RETURN(rv);
        /* Speed */
        if ((rv = bcm_robo_port_speed_get(unit, port, 
            &(phy_cfg->speed))) < 0){
            if (rv != BCM_E_BUSY){
                return(rv);
            } else {
                phy_cfg->speed = 0;
            }
        }
        /* Duplex */
        if ((rv = bcm_robo_port_duplex_get(unit, port, 
            &(phy_cfg->duplex))) < 0){
            if (rv != BCM_E_BUSY){
                return(rv);
            } else {
                phy_cfg->duplex = 0;
            }
        }	

    }
    return rv;
}


STATIC int
_bcm_robo53101_phy_cfg_restore(int unit)
{
    uint32	temp;
    bcm_pbmp_t    pbmp;
    bcm_port_t    port;
    int rv = BCM_E_NONE;
    bcm_port_cfg_t  *pcfg;
    bcm_robo_phy_cfg_t  *phy_cfg, cur_phy_cfg;

    sal_memset(&cur_phy_cfg, 0, sizeof(bcm_robo_phy_cfg_t));
    temp = 0;
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_PHY_CFG_RESTORE_PBMP, &temp);
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP, &temp);
    }
    BCM_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, temp);
    PBMP_ITER(pbmp, port) {
        pcfg = &robo_port_config[unit][port];
        phy_cfg = &pcfg->phy_cfg;

        /* PHY reset */
        BCM_IF_ERROR_RETURN(
            soc_phyctrl_init(unit, port));

        /* Restore the PHY configuration. */
        if (!phy_cfg->autoneg) {
            /* Force speed */
            rv = bcm_robo_port_speed_set(unit, port, phy_cfg->speed);
            BCM_IF_ERROR_RETURN(rv);
            /* Duplex */
            rv = bcm_robo_port_duplex_get(unit, port, &(cur_phy_cfg.duplex));
            BCM_IF_ERROR_RETURN(rv);
            if (cur_phy_cfg.duplex != phy_cfg->duplex) {
                rv = bcm_robo_port_duplex_set(unit, port, phy_cfg->duplex);
            }
        } else { /* AN is enabled */
            /* Advertisement */
            rv = bcm_robo_port_advert_get(unit, port, &(cur_phy_cfg.local_advert));
            BCM_IF_ERROR_RETURN(rv);
            if (cur_phy_cfg.local_advert != phy_cfg->local_advert) {
                rv = bcm_robo_port_advert_set(unit, port, phy_cfg->local_advert);
            }
        }		

    }
    return rv;
}
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */

/*
 * Function:
 *  _bcm_robo_port_link_get
 * Purpose:
 *  Return current PHY up/down status
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  hw - If TRUE, assume hardware linkscan is active and use it
 *      to reduce PHY reads.
 *         If FALSE, do not use information from hardware linkscan.
 *  up - (OUT) TRUE for link up, FALSE for link down.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
_bcm_robo_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;
#if defined(BCM_53101)
    uint32 reg_val = 0;
#endif /* BCM_53101 */
#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
        uint32 temp = 0;
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */

    PORT_PARAM_CHECK(unit, port); 

#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
    /* If Mac is in Low Power mode */
    if (SOC_IS_LOTUS(unit) || 
        (SOC_IS_POLAR(unit) && !SOC_CONTROL(unit)->int_cpu_enabled)) {
        if ((SOC_MAC_LOW_POWER_ENABLED(unit)) && 
            (SOC_AUTO_MAC_LOW_POWER(unit))) {
            /* Check the PHY energy detection.
             * 1. If the signal is detected, disabling the Low Power mode. 
             * 2. if not, return the link status as link-down. 
             */
            if (SOC_IS_LOTUS(unit)) {
#ifdef BCM_53101
                rv = REG_READ_PHY_STSr(unit, &reg_val);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Port %s: Failed to get the PHY energy detection: %s\n"), 
                               SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
                }
                soc_PHY_STSr_field_get(unit, &reg_val, PHY_ENERGY_DETf, &temp);
#endif /* BCM_53101 */                
            } else if (SOC_IS_POLAR(unit)){
#ifdef BCM_POLAR_SUPPORT
                rv = REG_READ_ENG_DET_STSr(unit, &temp);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Port %s: Failed to get the PHY energy detection: %s\n"), 
                               SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
                }
#endif /* BCM_POLAR_SUPPORT */
            }
            if (temp) {     
                /* Disabel the Low Power Mode */
                rv = DRV_DEV_PROP_SET(unit, DRV_DEV_PROP_LOW_POWER_ENABLE, 0);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Port %s: Failed to disable the Low Power Mode: %s\n"), 
                               SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
                    *up = FALSE;
                    return rv;
                } else {
                    MAC_LOW_POWER_LOCK(unit);
                    SOC_MAC_LOW_POWER_ENABLED(unit) = 0;
                    MAC_LOW_POWER_UNLOCK(unit);
#if defined(BCM_53101) || defined (BCM_POLAR_SUPPORT)
                    if (SOC_IS_LOTUS(unit) || SOC_IS_POLAR(unit)) {
                        BCM_IF_ERROR_RETURN
                        	(_bcm_robo53101_phy_cfg_restore(unit));
                    }
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */
                }
            } else {
                *up = FALSE;
                return rv;
            }
        }
    }
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */
    
    /*
     * This routines in original code is for an potential issue on 
     *   HW the link status might not sync. with the real PHY status
     *   when the HW link up represented.
     * Now, for Robo chip in this source we force all register R/W 
     *   been hided in the Drv Service so we do same thing to get 
     *   link status through "port_status_get()" within 
     *   "DRV_PORT_STATUS_LINK_UP" flag no matter hw=1 or hw=0.
     *
     */
    if (hw) {
        rv = DRV_PORT_STATUS_GET(unit, port, 
                DRV_PORT_STATUS_LINK_UP, &drv_value);
        *up = (drv_value) ? TRUE : FALSE;

    } else {
        rv = DRV_PORT_STATUS_GET(unit, port, 
                DRV_PORT_STATUS_LINK_UP, &drv_value);
        *up = (drv_value) ? TRUE : FALSE;
    }

#if defined(BCM_53125) || defined(BCM_53128)
    if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
        if ((SOC_MAC_LOW_POWER_ENABLED(unit)) && 
            (SOC_AUTO_MAC_LOW_POWER(unit) && (*up == TRUE))) {
            /* Disabel the Low Power Mode */
            rv = DRV_DEV_PROP_SET(unit, DRV_DEV_PROP_LOW_POWER_ENABLE, 0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Port %s: Failed to disable the Low Power Mode: %s\n"), 
                           SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
            } else {
                MAC_LOW_POWER_LOCK(unit);
                SOC_MAC_LOW_POWER_ENABLED(unit) = 0;
                MAC_LOW_POWER_UNLOCK(unit);
            }
        }
    }
#endif /* BCM_53125 || BCM_53128 */

    if (BCM_SUCCESS(rv)) {
        if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE)) {
            soc_port_medium_t  medium;
            soc_phyctrl_medium_get(unit, port, &medium);
            soc_phy_medium_status_notify(unit, port, medium);
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_LINK,
                (BSL_META_U(unit,
                            "_bcm_port_link_get: u=%d p=%d hw=%d up=%d rv=%d\n"),
                 unit, port, hw, *up, rv));

    return rv;
}

void
_bcm_robo_dtag_mode_sw_update(int unit, bcm_port_t port, int is_dtag)
{
    bcm_port_cfg_t  * pcfg;
    uint32 is_isp = 0;

    if (robo_port_config[unit]) {
        pcfg = &robo_port_config[unit][port];

        DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_DTAG_ISP_PORT, &is_isp);

        if (is_dtag) {
            if (is_isp) {
                pcfg->pc_dt_mode = BCM_PORT_DTAG_MODE_INTERNAL;
            } else {
                pcfg->pc_dt_mode = BCM_PORT_DTAG_MODE_EXTERNAL;
            }
        } else {
            pcfg->pc_dt_mode = BCM_PORT_DTAG_MODE_NONE;
        }
    }
}

/* report the supported flags for SA Move */
int _bcm_robo_sa_move_valid_flag(int unit, uint32 *flags)
{
    if (SOC_IS_TBX(unit)){
        *flags = BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_CPU | BCM_PORT_LEARN_FWD;
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
        /* default action is FWD but ignore this bit due to FWD/Drop is not 
         * configurable on bcm53242/bcm53262.
         */
        *flags = BCM_PORT_LEARN_ARL;
    } else {
        *flags = 0;
    }
    
    return BCM_E_NONE;
}

int _bcm_robo_port_sa_move_flags2soc(int unit, 
        uint32 sw_flags, uint32 *soc_flags)
{
    uint32 temp_val = 0;
    uint32 valid_flags = 0;
    
    if (NULL == soc_flags) {
        return BCM_E_PARAM;
    }
    
    temp_val = sw_flags & ~(BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_CPU | 
            BCM_PORT_LEARN_FWD | BCM_PORT_LEARN_PENDING);
    if (temp_val) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Invalid LEARN flags at 0x%x!\n"), temp_val));
        return (BCM_E_PARAM);
    }

    /* check the supported flag first */
    BCM_IF_ERROR_RETURN(_bcm_robo_sa_move_valid_flag(unit, &valid_flags));
    temp_val = sw_flags & ~(valid_flags);
    if (temp_val) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Not supported LEARN flags at 0x%x!\n"), temp_val));
        return (BCM_E_UNAVAIL);
    }
    
    /* to learn */
    if (sw_flags & BCM_PORT_LEARN_ARL) {
       temp_val |= DRV_SA_MOVE_ARL;
    }

    if (sw_flags & BCM_PORT_LEARN_CPU) {
       temp_val |= DRV_SA_MOVE_CPU;
    }

    /* to drop */
    if (!SOC_IS_HARRIER(unit)) {
        if (!(sw_flags & BCM_PORT_LEARN_FWD)) {
            temp_val |= DRV_SA_MOVE_DROP;
        }
    }

    if (sw_flags & BCM_PORT_LEARN_PENDING) {
       temp_val |= DRV_SA_MOVE_PEND;
    }


    *soc_flags = temp_val;
    return (BCM_E_NONE);
}

int _bcm_robo_port_sa_move_soc2flags(int unit, 
        uint32 soc_flags, uint32 *sw_flag)
{
    uint32 temp_val = 0;
    uint32 valid_flags = 0;
    
    if (NULL == sw_flag) {
        return BCM_E_PARAM;
    }

    /* to learn */
    if (soc_flags & DRV_SA_MOVE_ARL) {
       temp_val |= BCM_PORT_LEARN_ARL;
    }

    if (soc_flags & DRV_SA_MOVE_CPU) {
       temp_val |= BCM_PORT_LEARN_CPU;
    }
    
    /* to drop */
    if (!(soc_flags & DRV_SA_MOVE_DROP)) {
       temp_val |= BCM_PORT_LEARN_FWD;
    }

    if (soc_flags & DRV_SA_MOVE_PEND) {
       temp_val |= BCM_PORT_LEARN_PENDING;
    }

    /* mask off the not supported flags */
    BCM_IF_ERROR_RETURN(_bcm_robo_sa_move_valid_flag(unit, 
            &valid_flags));
    
    *sw_flag = temp_val & valid_flags;
    return (BCM_E_NONE);
}

/*
 * Function:
 *  bcm_robo_port_probe
 * Purpose:
 *  Probe the PHY and set up the PHY and MAC for the specified ports.
 *      This is purely a discovery routine and does no configuration.
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  pbmp - Bitmap of ports to probe.
 *      okay_pbmp (OUT) - Ports which were successfully probed.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL - internal error.
 * Notes:
 *      If error is returned, the port should not be enabled.
 *      Assumes port_init done.
 *      Note that if a PHY is not present, the port will still probe
 *      successfully.  The default driver will be installed.
 */

int
bcm_robo_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int rv = BCM_E_NONE;
    bcm_port_t port;
    uint32 okay = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_probe()..\n")));
    BCM_PBMP_CLEAR(*okay_pbmp);

    PBMP_ITER(pbmp, port) 
    {
        /* do port_probe process */
        rv = DRV_PORT_STATUS_GET(unit, port, DRV_PORT_STATUS_PROBE, &okay);
        if (okay) 
        {
            BCM_PBMP_PORT_ADD(*okay_pbmp, port);
        }
        if (rv < 0) 
        {
            break;
        }
    }

    return rv;
}


/*
 * Function:
 *      _bcm_robo_port_mode_setup
 * Purpose:
 *      Set initial operating mode for a port.
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      port - RoboSwitch port #.
 *      enable - Whether to enable or disable
 * Returns:
 *      BCM_E_XXXX
 */
STATIC int
_bcm_robo_port_mode_setup(int unit, bcm_port_t port, int enable)
{
    soc_port_if_t       pif;
    bcm_port_ability_t  local_pa;

    sal_memset(&local_pa, 0, sizeof(bcm_port_ability_t));
    BCM_IF_ERROR_RETURN(
            bcm_robo_port_ability_local_get(unit, port, &local_pa));

    /* If MII supported, enable it, otherwise use TBI */

    if (local_pa.interface & (SOC_PA_INTF_MII | SOC_PA_INTF_GMII | 
                                                SOC_PA_INTF_SGMII)) {
        if (IS_GE_PORT(unit, port)) 
        {
            pif = SOC_PORT_IF_GMII;
        }
        else 
        {
            pif = SOC_PORT_IF_MII;
        }
    } else {
        pif = SOC_PORT_IF_TBI;
    }

    BCM_IF_ERROR_RETURN
        (soc_phyctrl_interface_set(unit, port, pif));

    return BCM_E_NONE;
    
}

/*
 * Function:
 *  _bcm_robo_port_detach
 * Purpose:
 *  Main part of bcm_port_detach
 */

int
_bcm_robo_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    bcm_port_t      port;

    BCM_PBMP_CLEAR(*detached);

    PBMP_ITER(pbmp, port) 
    {
        BCM_IF_ERROR_RETURN(soc_phyctrl_detach(unit, port));
        BCM_IF_ERROR_RETURN(bcm_robo_port_stp_set(unit, port, BCM_STG_STP_DISABLE));
        
        BCM_IF_ERROR_RETURN(_bcm_robo_port_mode_setup(unit, port, FALSE));
        SOC_PBMP_PORT_ADD(*detached, port);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_detach
 * Purpose:
 *  Detach a port.  Set phy driver to no connection.
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  pbmp - Bitmap of ports to detach.
 *      detached (OUT) - Bitmap of ports successfully detached.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL - internal error.
 * Notes:
 *      If a port to be detached does not appear in detached, its
 *      state is not defined.
 */

int
bcm_robo_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    int     rv;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_detach()..\n")));
    PORT_INIT(unit);
    rv = _bcm_robo_port_detach(unit, pbmp, detached);

    return rv;
}

/* General routines on which most port routines are built */

/*
 * Function:
 *  bcm_robo_port_config_get
 * Purpose:
 *  Get port configuration of a device
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  config - (OUT) Structure returning configuration
 * Returns:
 *  BCM_E_XXX
 */

int
bcm_robo_port_config_get(int unit, bcm_port_config_t *config)
{
    bcm_pbmp_t pbm;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_config_get()..\n")));
    PORT_INIT(unit);
    SOC_PBMP_CLEAR(pbm);

    config->fe         = PBMP_FE_ALL(unit);
    config->ge         = PBMP_GE_ALL(unit);
    config->xe         = PBMP_XE_ALL(unit);
    config->e         = PBMP_E_ALL(unit);
    config->hg         = PBMP_HG_ALL(unit);
    config->port     = PBMP_PORT_ALL(unit);
    config->cpu     = PBMP_CMIC(unit);
    config->all     = PBMP_ALL(unit);
    config->stack_int	  = pbm;
    config->stack_ext     = pbm;
    config->sci           = pbm;
    config->sfi           = pbm;
    config->spi           = pbm;
    config->spi_subport   = pbm;

    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_enable_set
 * Purpose:
 *  Physically enable/disable the MAC/PHY on this port.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port #.
 *  enable - TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *  BCM_E_XXXX
 * Notes:
 *  If linkscan is running, it also controls the MAC enable state.
 */

int
bcm_robo_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int     rv = BCM_E_NONE;
    pbmp_t  t_pbm;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_enable_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    /* process the MAC ENABLE & PHY ENABLE */
    rv = DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_ENABLE, 
             (enable) ? TRUE : FALSE);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_robo_port_enable_set: u=%d p=%d enable=%d rv=%d\n"),
              unit, port, enable, rv));

    return rv;
}



/*
 * Function:
 *  bcm_robo_port_enable_get
 * Purpose:
 *  Gets the enable state as defined by bcm_port_enable_set()
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port #.
 *  enable - (OUT) TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *  BCM_E_XXXX
 * Notes:
 *  The PHY enable holds the port enable state set by the user.
 *  The MAC enable transitions up and down automatically via linkscan
 *  even if user port enable is always up.
 */

int
bcm_robo_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int         rv = BCM_E_NONE ;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_enable_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = DRV_PORT_GET(unit, port, DRV_PORT_PROP_ENABLE, (uint32 *) enable);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_enable_get: u=%d p=%d rv=%d enable=%d\n"),
              unit, port, rv, *enable));

    return rv;
}

/*
 * Function:
 *      bcm_robo_port_error_symbol_count
 * Description:
 *      Get the number of |E| symbol in XAUI lanes since last read.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      count - (OUT) Number of |E| error since last read.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 *      BCM_E_INIT    - Error symbol detect feature is not enabled
 */
int 
bcm_robo_port_error_symbol_count(int unit, bcm_port_t port, int *count) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_settings_init
 * Purpose:
 *      Initialize port settings if they are to be different from the
 *      default ones
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      port - port number 
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 * Notes:
 *      This function initializes port settings based on the folowing config
 *      variables:
 *           port_init_speed
 *           port_init_duplex
 *           port_init_adv
 *           port_init_autoneg
 *      If a variable is not set, then no additional initialization of the 
 *      corresponding parameter is done (and the defaults will normally be
 *      advertize everything you can do and use autonegotiation).
 *
 *      A typical use would be to set:
 *          port_init_adv=0
 *          port_init_autoneg=1
 *      to force link down in the beginning.
 *
 *      Another setup that makes sense is something like:
 *          port_init_speed=10
 *          port_init_duplex=0
 *          port_init_autoneg=0
 *      in order to force link into a certain mode. (It is very important to
 *      disable autonegotiation in this case).
 *
 *      PLEASE NOTE: 
 *          The standard rc.soc forces autoneg=on on all the ethernet ports
 *          (FE and GE). Thus, to use the second example one has to edit rc.soc
 *          as well.
 * 
 *     This function has been declared as global, but not exported. This will
 *     make port initialization easier when using VxWorks shell. 
 */
int
bcm_robo_port_settings_init(int unit, bcm_port_t port)
{
    int             val = 0;
    bcm_port_info_t info;
 
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_settings_init()..\n")));
    info.action_mask = 0;

    val = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, -1);
    if (val != -1) {
        info.speed = val;
        info.action_mask |= BCM_PORT_ATTR_SPEED_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
    if (val != -1) {
        info.duplex = val;
        info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
    if (val != -1) {
        info.local_advert = val;
        info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_AUTONEG, -1);
    if (val != -1) {
        info.autoneg = val;
        info.action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
    }
    
    return bcm_robo_port_selective_set(unit, port, &info);
}

/*
 * Function:
 *      bcm_port_clear
 * Purpose:
 *      Initialize the PORT interface layer for the specified SOC device
 *      without resetting stacking ports.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_MEMORY - failed to allocate required memory.
 * Notes:
 *      All ports set in disabled state.
 *      PTABLE initialized.
 */
int
bcm_robo_port_clear(int unit)
{

    bcm_port_config_t port_config;
    bcm_pbmp_t reset_ports;
    bcm_port_t port;
    int rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_clear()..\n")));
    PORT_INIT(unit);

    BCM_IF_ERROR_RETURN(bcm_robo_port_config_get(unit, &port_config));

    /* Clear all non-stacking ethernet ports */
    BCM_PBMP_ASSIGN(reset_ports, port_config.e);

    PBMP_ITER(reset_ports, port) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_port_clear: unit %d port %s\n"),
                     unit, SOC_PORT_NAME(unit, port)));

        if ((rv = _bcm_robo_port_mode_setup(unit, port, TRUE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set initial mode: %s\n"),
                      unit, SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
        }

        if ((rv = bcm_robo_port_enable_set(unit, port, TRUE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to enable port: %s\n"),
                      unit, SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
        }
    }

    return BCM_E_NONE;
}

/* Module one-time initialization routine */

/*
 * Function:
 *      bcm_robo_port_init
 * Purpose:
 *      Initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_MEMORY - failed to allocate required memory.
 * Notes:
 *      All ports set in disabled state.
 *      Default PVID initialized.
 */

int
bcm_robo_port_init(int unit)
{
    int                 rv = BCM_E_NONE;
    int                 dt_mode = 0;
    bcm_port_t          p;
    pbmp_t              okay_ports, pbmp, t_pbm;
    bcm_vlan_data_t     vd;
    char                pfmtok[SOC_PBMP_FMT_LEN],
                        pfmtall[SOC_PBMP_FMT_LEN];

    assert(unit < BCM_LOCAL_UNITS_MAX);
    if ((unit < 0) || (unit >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s,Unit %d Init..."), FUNCTION_NAME(), unit));
    /*
     * Write port configuration tables to contain the Initial System
     * Configuration (see init.c).
     */
    vd.vlan_tag = BCM_VLAN_DEFAULT;
    BCM_PBMP_ASSIGN(vd.port_bitmap, PBMP_ALL(unit));
    BCM_PBMP_ASSIGN(vd.ut_port_bitmap, PBMP_ALL(unit));
    BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));

    /* for software table and setting default PVID */
    _bcm_robo_port_cfg_init(unit);
    BCM_IF_ERROR_RETURN(soc_phy_common_init(unit));

    /* Probe for ports */
    BCM_PBMP_CLEAR(okay_ports);
    
    if ((rv = bcm_robo_port_probe(unit, PBMP_PORT_ALL(unit), &okay_ports)) 
                    != BCM_E_NONE) 
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Error unit %d:  Failed port probe: %s\n"),
                   unit, bcm_errmsg(rv)));
        return rv;
    }

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Probed ports okay: %s of %s\n"),
                 SOC_PBMP_FMT(okay_ports, pfmtok),
                 SOC_PBMP_FMT(PBMP_PORT_ALL(unit), pfmtall)));

    /* force the CPU been ISP port after init */
    p = CMIC_PORT(unit);
    rv = bcm_robo_port_dtag_mode_get(unit, p, &dt_mode);
    if (rv) {
        if (!SOC_IS_DINO(unit)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to get dtag_mode : %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }
    } else {
        if ((rv = bcm_robo_port_dtag_mode_set
                    (unit, p, BCM_PORT_DTAG_MODE_INTERNAL)) < 0) {
            /* if the return code is UNAVAIL, it means this chip have no dtag 
             *  feature, this error code can be ignored here.
             */
            if ((rv != BCM_E_UNAVAIL)){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Warning: Unit %d Port %s: "
                                     "Failed to set dtag_mode_internal : %s\n"),
                          unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
            }
        }
    }
    
    /* Probe and initialize MAC and PHY drivers for ports that were OK */

    PBMP_ITER(okay_ports, p) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_port_init: unit %d port %s\n"),
                     unit, SOC_PORT_NAME(unit, p)));

        if ((rv = _bcm_robo_port_mode_setup(unit, p, TRUE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set initial mode: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        if ((rv = bcm_robo_port_settings_init(unit, p)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to configure initial settings: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        if ((rv = bcm_robo_port_discard_set
                    (unit, p, BCM_PORT_DISCARD_NONE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set discard_none : %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        rv = bcm_robo_port_dtag_mode_get(unit, p, &dt_mode);
        if (rv) {
            if (!SOC_IS_DINO(unit)) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Warning: Unit %d Port %s: "
                                     "Failed to get dtag_mode : %s\n"),
                          unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
            }
        } else {
            if (dt_mode == BCM_PORT_DTAG_MODE_INTERNAL) {
                /* set port as non-ISP port when init */
                /* coverity[CONSTANT_EXPRESSION_RESULT] : FALSE */
                if ((rv = bcm_robo_port_dtag_mode_set
                            (unit, p, BCM_PORT_DTAG_MODE_EXTERNAL)) < 0) {
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "Warning: Unit %d Port %s: "
                                         "Failed to set dtag_mode_external : %s\n"),
                              unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
                }
            }
        }

        if ((rv = bcm_robo_port_tpid_set
                    (unit, p, 0x9100)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set tpid : %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        if ((rv = bcm_robo_port_pfm_set(unit, p, 
            BCM_PORT_MCAST_FLOOD_UNKNOWN)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set pfm mode : %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        if ((rv = bcm_robo_port_enable_set(unit, p, TRUE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to enable port: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }
    }

    /* disable system based Double tagging mode :
     *  - Robo only! disable DT_Mode at one port means disable system basis.
     */
    p = CMIC_PORT(unit);
    if ((rv = bcm_robo_port_dtag_mode_set
            (unit, p, BCM_PORT_DTAG_MODE_NONE)) < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Warning: Unit %d Port %s: "
                             "Failed to set dtag_mode_none : %s\n"),
                  unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
    }

    /* Clear port-based vlan mask to default value */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));

    BCM_PBMP_ITER(pbmp, p) {
        BCM_PBMP_CLEAR(t_pbm);
        BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_vlan_get)
                            (unit, p, &t_pbm));

        if (!BCM_PBMP_EQ(pbmp, t_pbm)) {
            BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_SET(unit, p, pbmp));
        }
    }

    /* Per-port rate initialize */
    BCM_IF_ERROR_RETURN(_bcm_robo_rate_init(unit));

    if (soc_feature (unit, soc_feature_eee)) {
        BCM_IF_ERROR_RETURN(
            DRV_DEV_PROP_SET(unit, 
            DRV_DEV_PROP_EEE_INIT, TRUE));
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_port_init() Done!\n")));

    return BCM_E_NONE;    
}

/*
 * Function:
 *  bcm_robo_port_advert_get
 * Purpose:
 *  Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ability_mask - (OUT) Local advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_advert_get(int unit, bcm_port_t port, 
            bcm_port_abil_t *ability_mask)
{
    int     rv = BCM_E_NONE;
    bcm_port_ability_t  ability;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_advert_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = soc_phyctrl_ability_advert_get(unit, port, &ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&ability, ability_mask);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_advert_get: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, *ability_mask, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_ability_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - (OUT) Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_robo_port_ability_advert_get(int unit, bcm_port_t port, 
                                bcm_port_ability_t *ability_mask)
{
    int  rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = soc_phyctrl_ability_advert_get(unit, port, ability_mask);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_advert_get: u=%d p=%d rv=%d\n"),
              unit, port, rv));

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_advert_set
 * Purpose:
 *  Set the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ability_mask - Local advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Does NOT restart autonegotiation.
 *  To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */

int
bcm_robo_port_advert_set(int unit, bcm_port_t port, 
                bcm_port_abil_t ability_mask)
{
    int     rv = BCM_E_NONE;
    pbmp_t  t_pbm;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_advert_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    drv_value = ability_mask;
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    rv = DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_LOCAL_ADVERTISE, drv_value);
    return rv;
}

/*
 * Function:
 *      bcm_robo_port_ability_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */
int
bcm_robo_port_ability_advert_set(int unit, bcm_port_t port,
                                bcm_port_ability_t *ability_mask)
{
    int  rv = BCM_E_NONE;
    bcm_port_ability_t port_ability;

    sal_memset(&port_ability, 0, sizeof(bcm_port_ability_t));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_IF_ERROR_RETURN
        (bcm_robo_port_ability_local_get(unit, port, &port_ability));

    /* Make sure to advertise only abilities supported by the port */
    port_ability.speed_half_duplex   &= ability_mask->speed_half_duplex;
    port_ability.speed_full_duplex   &= ability_mask->speed_full_duplex;
    port_ability.pause      &= ability_mask->pause;
    port_ability.interface  &= ability_mask->interface;
    port_ability.medium     &= ability_mask->medium;
    port_ability.eee        &= ability_mask->eee;
    port_ability.loopback   &= ability_mask->loopback;
    port_ability.flags      &= ability_mask->flags;

    rv = soc_phyctrl_ability_advert_set(unit, port, &port_ability);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_advert_set: u=%d p=%d rv=%d\n"),
              unit, port, rv));

    return rv;
}

/*
 * Function:
 *      bcm_robo_port_autoneg_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - (OUT) Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_robo_port_autoneg_advert_get (int unit, bcm_port_t port, 
                bcm_port_ability_t *ability_mask)
{
    int         rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_autoneg_advert_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = soc_phyctrl_ability_advert_get(unit, port, ability_mask);
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_autoneg_advert_get: u=%d p=%d rv=%d\n"),
              unit, port, rv));
    
    return rv;
}

/*
 * Function:
 *      bcm_robo_port_autoneg_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */

int
bcm_robo_port_autoneg_advert_set (int unit, bcm_port_t port, 
                bcm_port_ability_t *ability_mask)
{
    int             rv = BCM_E_NONE;
    bcm_port_ability_t port_ability;

    sal_memset(&port_ability, 0, sizeof(bcm_port_ability_t));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_autoneg_advert_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_IF_ERROR_RETURN
        (bcm_robo_port_ability_local_get(unit, port, &port_ability));

    /* Make sure to advertise only abilities supported by the port */
    port_ability.speed_half_duplex   &= ability_mask->speed_half_duplex;
    port_ability.speed_full_duplex   &= ability_mask->speed_full_duplex;
    port_ability.pause      &= ability_mask->pause;
    port_ability.interface  &= ability_mask->interface;
    port_ability.medium     &= ability_mask->medium;
    port_ability.eee        &= ability_mask->eee;
    port_ability.loopback   &= ability_mask->loopback;
    port_ability.flags      &= ability_mask->flags;
    
    rv = soc_phyctrl_ability_advert_set(unit, port, &port_ability);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_autoneg_advert_set: u=%d p=%d rv=%d\n"),
              unit, port, rv));

    return rv;
}

/*
 * Function:
 *      _bcm_port_autoneg_advert_remote_get
 * Purpose:
 *      Main part of bcm_port_advert_get_remote
 */

STATIC int
_bcm_robo_port_autoneg_advert_remote_get(int unit, bcm_port_t port,
                            bcm_port_ability_t *ability_mask)
{
    int                 an = 0, an_done = 0;

    BCM_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit,  port,
                                &an, &an_done));

    if (!an) {
        return BCM_E_DISABLED;
    }

    if (!an_done) {
        return BCM_E_BUSY;
    }

    BCM_IF_ERROR_RETURN
        (soc_phyctrl_ability_remote_get(unit, port, ability_mask));

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_advert_remote_get
 * Purpose:
 *  Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ability_mask - (OUT) Remote advertisement.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_advert_remote_get(int unit, bcm_port_t port,
               bcm_port_abil_t *ability_mask)
{
    int     rv = BCM_E_NONE;    
    bcm_port_ability_t  port_ability;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_advert_remote_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_autoneg_advert_remote_get(unit, port, &port_ability);

    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&port_ability, ability_mask);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_advert_remote_get: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, *ability_mask, rv));
    return rv;
}

/*
 * Function:
 *      bcm_robo_port_ability_remote_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - (OUT) Remote advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_robo_port_ability_remote_get(int unit, bcm_port_t port,
                           bcm_port_ability_t *ability_mask)
{
    int  rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_autoneg_advert_remote_get(unit, port, ability_mask);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_remote_get: u=%d p=%d rv=%d\n"),
              unit, port, rv));

    return rv;
}

/*
 * Function:
 *      _bcm_robo_port_ability_local_get
 * Purpose:
 *      Main part of bcm_port_ability_local_get
 * Notes:
 *      Relies on the fact the soc_port_mode_t and bcm_port_abil_t have
 *      the same values.
 */

STATIC int
_bcm_robo_port_ability_local_get(int unit, bcm_port_t port,
                           bcm_port_ability_t *ability_mask)
{
    soc_port_ability_t             mac_ability, phy_ability;
    mac_driver_t    *macd;
    int rv = BCM_E_NONE;
    char *s;

    if ((rv = soc_robo_mac_probe(unit, port, &macd)) < 0) {
        LOG_WARN(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_robo_port_ability_local_get : unit %d Port %s: Failed to probe MAC: %s\n"),
                  unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }
    
    sal_memset(&mac_ability, 0, sizeof(soc_port_ability_t));
    sal_memset(&phy_ability, 0, sizeof(soc_port_ability_t));

    BCM_IF_ERROR_RETURN
        (soc_phyctrl_ability_local_get(unit, port, &phy_ability));

    BCM_IF_ERROR_RETURN
        (MAC_ABILITY_LOCAL_GET(macd, unit, port, &mac_ability));

    /* Combine MAC and PHY abilities */
    s = soc_property_get_str(unit, "board_name");
    if((s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if (IS_GE_PORT(unit, port)) {
            ability_mask->speed_half_duplex  = mac_ability.speed_half_duplex;
            ability_mask->speed_full_duplex  = mac_ability.speed_full_duplex;
            ability_mask->pause     = mac_ability.pause;
        } else {
            ability_mask->speed_half_duplex  = mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
            ability_mask->speed_full_duplex  = mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
            ability_mask->pause     = mac_ability.pause & phy_ability.pause;
        }
    } else {
        ability_mask->speed_half_duplex  = mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
        ability_mask->speed_full_duplex  = mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
        ability_mask->pause     = mac_ability.pause & phy_ability.pause;
    }

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

/*
 * Function:
 *  bcm_robo_port_ability_get
 * Purpose:
 *  Retrieve the local port abilities.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *      ability of the MAC/PHY.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_ability_get(int unit, bcm_port_t port, 
                    bcm_port_abil_t *ability_mask)
{
    int rv = BCM_E_NONE;
    bcm_port_ability_t  port_ability;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_port_ability_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_ability_local_get(unit, port, &port_ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&port_ability, ability_mask);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_get: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, *ability_mask, rv));
    return rv;
}


/*
 * Function:
 *      bcm_robo_port_ability_local_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch port #.
 *      ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *              ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_robo_port_ability_local_get(int unit, bcm_port_t port,
                         bcm_port_ability_t *ability_mask)
{
    int  rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_ability_local_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_ability_local_get(unit, port, ability_mask);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_local_get: u=%d p=%d rv=%d\n"), unit, port, rv));
    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x\n"
                            "Interface=0x%08x Medium=0x%08x EEE=0x%08x Loopback=0x%08x Flags=0x%08x\n"),
                 ability_mask->speed_half_duplex,
                 ability_mask->speed_full_duplex,
                 ability_mask->pause, ability_mask->interface,
                 ability_mask->medium, ability_mask->eee,
                 ability_mask->loopback, ability_mask->flags));
    return rv;
}

/* PVLAN functions */

/*
 * Function:
 *  bcm_robo_port_untagged_vlan_get
 * Purpose:
 *  Retrieve the default VLAN TAG for the port.
 *  This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  port - RoboSwitch port number of port to get info for
 *  vid_ptr - (OUT) Pointer to VLAN ID for return
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL if table read failed.
 */

int
bcm_robo_port_untagged_vlan_get(int unit, 
                bcm_port_t port, 
                bcm_vlan_t *vid_ptr)
{
    bcm_port_cfg_t  * pcfg;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_untagged_vlan_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    pcfg = &robo_port_config[unit][port];

    if (vid_ptr != NULL) {
        /* include the VID + priority */
        *vid_ptr = pcfg->pc_vlan;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_untagged_vlan_get: u=%d p=%d pv_tag=%d\n"),
              unit, port, pcfg->pc_vlan));

    return BCM_E_NONE;
}


/*
 * Function:
 *  _bcm_robo_port_untagged_vlan_set
 * Purpose:
 *      Main part of bcm_port_untagged_vlan_set.
 * Notes:
 *  Port must already be a member of its default VLAN.
 */

STATIC int
_bcm_robo_port_untagged_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vid)
{
    bcm_port_cfg_t     * pcfg;
    bcm_vlan_t      svid = 0, cvid = 0;
    uint32          spri = 0, scfi = 0;
    uint32          cpri = 0, ccfi = 0;
    uint32          inner_tag = 0, outer_tag = 0;

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET(
            unit, port, &outer_tag, &inner_tag));

    svid = BCM_VLAN_CTRL_ID(outer_tag);
    spri = BCM_VLAN_CTRL_PRIO(outer_tag);
    scfi = BCM_VLAN_CTRL_CFI(outer_tag);

    if (SOC_IS_TBX(unit)){
        cvid = BCM_VLAN_CTRL_ID(inner_tag);
        cpri = BCM_VLAN_CTRL_PRIO(inner_tag);
        ccfi = BCM_VLAN_CTRL_CFI(inner_tag);
        
        if (vid ^ cvid){
            inner_tag = BCM_VLAN_CTRL(cpri, ccfi, vid);
        }
    }
    
    if (vid ^ svid){
        outer_tag = BCM_VLAN_CTRL(spri, scfi, vid);
    }

    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_SET(
            unit, port, outer_tag, inner_tag));
            
    pcfg = &robo_port_config[unit][port];
    pcfg->pc_vlan = vid;
    
    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_untagged_vlan_set
 * Purpose:
 *  Set the default VLAN ID for the port.
 *  This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  port - RoboSwitch port number.
 *  vid - (OUT) Pointer to VLAN ID for return
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_NOT_FOUND if vid not in VTABLE
 *  BCM_E_INTERNAL if table read failed.
 *  BCM_E_CONFIG - port does not belong to the VLAN
 */

int
bcm_robo_port_untagged_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_untagged_vlan_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    rv = _bcm_robo_port_untagged_vlan_set(unit, port, vid);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_untagged_vlan_set: u=%d p=%d vid=%d rv=%d\n"),
              unit, port, vid, rv));

    return rv;
}
                      

/*
 * Function:
 *  _bcm_robo_port_untagged_priority_set
 * Purpose:
 *  Main part of bcm_port_untagged_priority_set.
 */

STATIC int
_bcm_robo_port_untagged_priority_set(int unit, bcm_port_t port, int priority)
{
    bcm_port_cfg_t  *pcfg;
    uint32          svid = 0, spri = 0, scfi = 0;
    uint32          cvid = 0, cpri = 0, ccfi = 0;
    uint32          inner_tag = 0, outer_tag = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (priority < 0 || priority > 7) 
    {
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET(
            unit, port, &outer_tag, &inner_tag));
    
    svid = BCM_VLAN_CTRL_ID(outer_tag);
    spri = BCM_VLAN_CTRL_PRIO(outer_tag);
    scfi = BCM_VLAN_CTRL_CFI(outer_tag);
    
    if (SOC_IS_TBX(unit)){
        cvid = BCM_VLAN_CTRL_ID(inner_tag);
        cpri = BCM_VLAN_CTRL_PRIO(inner_tag);
        ccfi = BCM_VLAN_CTRL_CFI(inner_tag);
        
        if (cpri ^ priority){
            inner_tag = BCM_VLAN_CTRL(priority, ccfi, cvid);
        }
    }
    
    if (spri ^ priority){
        outer_tag = BCM_VLAN_CTRL(priority, scfi, svid);
    }
    
    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_SET(
            unit, port, outer_tag, inner_tag));
            
    pcfg = &robo_port_config[unit][port];
    /* pc_vlan = VID + priority */
    pcfg->pc_vlan = svid;

    return SOC_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_untagged_priority_set
 * Purpose:
 *  Set the 802.1P priority for untagged packets coming in on a
 *  port.  This value will be written into the priority field of the
 *  tag that is added at the ingress.
 * Parameters:
 *  unit      - RoboSwitch Unit #.
 *  port      - RoboSwitch port #.
 *      priority  - Priority to be set in 802.1p priority tag, from 0 to 7
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_untagged_priority_set(int unit, bcm_port_t port, int priority)
{
    int     rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_untagged_priority_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    rv = _bcm_robo_port_untagged_priority_set(unit, port, priority);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ut_priority_set: u=%d p=%d pri=%d rv=%d\n"),
              unit, port, priority, rv));
    return rv;
}

/*
 * Function:
 *  bcm_robo_port_untagged_priority_get
 * Purpose:
 *      Returns priority being assigned to untagged receive packets
 * Parameters:
 *  unit      - RoboSwitch Unit #.
 *  port      - RoboSwitch port #.
 *      priority  - Pointer to an int in which priority value is returned.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_untagged_priority_get(int unit, bcm_port_t port, int *priority)
{
   
    uint32          pri = 0;
    uint32          inner_tag, outer_tag;
   
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_untagged_priority_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET(
            unit, port, &outer_tag, &inner_tag));

    pri = BCM_VLAN_CTRL_PRIO(outer_tag);

    if (priority != NULL) 
    {
        *priority = pri;

    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ut_priority_get: u=%d p=%d pri=%d\n"),
              unit, port, pri));

    return BCM_E_NONE;
}
                      

/* DSCP mapping functions */

/*
 * Function:
 *      bcm_robo_port_dscp_map_mode_get
 * Purpose:
 *      DSCP mapping status for the port.
 * Parameters:
 *      unit - switch device
 *      port - switch port or -1 to get mode from first available port
 *      mode - (OUT)
 *           - BCM_PORT_DSCP_MAP_NONE
 *           - BCM_PORT_DSCP_MAP_ZERO Map if incomming DSCP = 0
 *           - BCM_PORT_DSCP_MAP_ALL DSCP -> DSCP mapping. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode)
{
    int rv = BCM_E_NONE;
    uint8 tmp = 0;
    bcm_port_t  loc_port;

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port == -1) {
        PBMP_E_ITER(unit, loc_port) {
            break;
        }
    } else {
        if (!SOC_PORT_VALID(unit, loc_port)) { 
            return BCM_E_PORT;
        }
    }
    
    if (!IS_E_PORT(unit, loc_port)) {
        rv = BCM_E_PORT;
    } else {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_GET
                        (unit, loc_port, 
                        DRV_QUEUE_MAP_DFSV, &tmp));
    }
    
    if (tmp) {
        *mode = BCM_PORT_DSCP_MAP_ALL;
    } else {
        *mode = BCM_PORT_DSCP_MAP_NONE;
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_port_dscp_map_mode_set
 * Purpose:
 *      Set DSCP mapping for the port.
 * Parameters:
 *      unit - switch device
 *      port - switch port      or -1 to apply on all the ports.
 *      mode - BCM_PORT_DSCP_MAP_NONE
 *           - BCM_PORT_DSCP_MAP_ZERO Map if incomming DSCP = 0
 *           - BCM_PORT_DSCP_MAP_ALL DSCP -> DSCP mapping. 
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t t_pbm;
    bcm_port_t  loc_port;

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    BCM_PBMP_CLEAR(t_pbm);
    if (loc_port == -1) {
        BCM_PBMP_ASSIGN(t_pbm, PBMP_E_ALL(unit));
    } else {
        if (!SOC_PORT_VALID(unit, loc_port)) { 
            return BCM_E_PORT;
        }

        if (IS_E_PORT(unit, loc_port)) {
            BCM_PBMP_PORT_ADD(t_pbm, loc_port);
        } else {
            rv = BCM_E_PORT;
        }
    }

    switch(mode) {
        case BCM_PORT_DSCP_MAP_NONE:
            if (loc_port == -1 || IS_E_PORT(unit, loc_port)) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                            (unit, t_pbm, 
                            DRV_QUEUE_MAP_DFSV, FALSE)); 
            }

            break;
        case BCM_PORT_DSCP_MAP_ZERO:    
            rv = BCM_E_UNAVAIL;
            break;
        case BCM_PORT_DSCP_MAP_ALL:
            if (loc_port == -1 || IS_E_PORT(unit, loc_port)) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                            (unit, t_pbm, 
                            DRV_QUEUE_MAP_DFSV, TRUE));
            }

            break;
        case BCM_PORT_DSCP_MAP_UNTAGGED_ONLY:
        case BCM_PORT_DSCP_MAP_DEFAULT:
            rv = BCM_E_UNAVAIL;
            break;
        default:
            return BCM_E_PARAM;
    }

    return rv;
}


/*
 * Function:
 *  bcm_robo_port_dscp_map_set
 * Purpose:
 *  Define a mapping from diffserv code points to CoS queue.
 * Parameters:
 *  unit  - RoboSwitch unit #
 *  port  - RoboSwitch port #, ignore in Robo.
 *  srccp - src code point or -1
 *         For TB B0, BCM_DSCP_ECN can be or'd with srccp 
 *         to represent {DSCP(7:2):ECN(1:0)}. 
 *  mapcp - mapped code point or -1
 *  prio - priority value for mapped code point
 *              -1 to use port default untagged priority
 *         For TB, BCM_PRIO_GREEN/BCM_PRIO_YELLOW/
 *                  BCM_PRIO_RED/BCM_PRIO_PRESERVE is or'ed
 *                  to represet both TC and DP value
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  1. Now Only BCM5338 support this feature.
 *      srccp -1, mapcp -1: disable the DiffServ function.
 *      srccp -1, mapcp 0..3:   map all packets
 *      srccp 0,  mapcp 0..3:   map packets with cp 0
 *  2. the selective on DiffServ/TOS will impact in each other.
 *      If current device is working at TOS mode, called this API may
 *        set this device been working at DiffServ mode.
 */
int
bcm_robo_port_dscp_map_set(int unit, bcm_port_t port, int srccp, int mapcp,
               int prio)
{
#define DSCP_CODE_POINT_CNT 64
#define DSCP_CODE_POINT_MAX (DSCP_CODE_POINT_CNT - 1)

    pbmp_t      t_pbm;
    uint8 queue = 0;
#ifdef BCM_TB_SUPPORT
    bcm_color_t color;
    int dscpecn = 0, ecn = 0;
    uint8 ecn_valid = 0;    
#endif
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_dscp_map_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) { 
            return BCM_E_PORT;
        }

        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            if (!IS_ALL_PORT(unit, loc_port)) {
                return BCM_E_PORT;
            }
#endif
        } else {
            if (!IS_E_PORT(unit, loc_port)) {
                return BCM_E_PORT;
            }
        }

        BCM_PBMP_CLEAR(t_pbm); 
        BCM_PBMP_PORT_ADD(t_pbm, loc_port);
    } else {
        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));
#endif
        } else {
            BCM_PBMP_ASSIGN(t_pbm, PBMP_E_ALL(unit));
        }
    }
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
        if (srccp & BCM_DSCP_ECN) {
            ecn_valid = 1;
            dscpecn = srccp & ~BCM_DSCP_ECN;
            if (dscpecn < -1 || dscpecn > (DSCP_CODE_POINT_CNT << 2)-1) {
                return BCM_E_PARAM;
            }                        
            srccp = (dscpecn >> 2) & DSCP_CODE_POINT_MAX;
        }
    }
#endif    
    if (srccp < -1 || srccp > DSCP_CODE_POINT_MAX) {
        return BCM_E_PARAM;
    }

    
    if (mapcp < -1 || mapcp > DSCP_CODE_POINT_MAX) {
        return BCM_E_PARAM;
    }
    
    if (soc_feature(unit, soc_feature_dscp)) {
        
        if (srccp != mapcp) {        
            return BCM_E_UNAVAIL;
        }
        if (srccp < 0 && mapcp < 0) {
            /* No mapping  disable DFSV*/
            BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                            (unit, t_pbm, 
                            DRV_QUEUE_MAP_DFSV, FALSE));
                    
            return BCM_E_NONE;
        }
        
        if (prio < 0) {
            /* prio == -1 get untag priority*/
            bcm_robo_port_untagged_priority_get(unit, loc_port, &prio);
        } 

        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            /* Extract DP bit and check for valid priority. */
            if (prio & BCM_PRIO_GREEN) {
                prio &= ~BCM_PRIO_GREEN;
                color = bcmColorGreen;
            } else if (prio & BCM_PRIO_YELLOW) {
                prio &= ~BCM_PRIO_YELLOW;
                color = bcmColorYellow;
            } else if (prio & BCM_PRIO_RED) {
                prio &= ~BCM_PRIO_RED;
                color = bcmColorRed;
            } else if  (prio & BCM_PRIO_PRESERVE) {
                prio &= ~BCM_PRIO_PRESERVE;
                color = bcmColorPreserve;
            } else {
                color = bcmColorGreen;
            }
            if (!_BCM_COSQ_PRIO_VALID(unit, prio)) {
                return BCM_E_PARAM;
            }
            /* set DSCP mapping to TCDP (prio = {DP[5:4], TC[3:0]}) */
            prio |= (_BCM_COLOR_ENCODING(unit, color) << 4);

            if (SOC_IS_TB_AX(unit)) {
                /* TB A0*/
                BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_SET
                    (unit, srccp, prio));
            } else {
                /* dscpecn {dscp[7:2], ecn[1:0]} */
                if (ecn_valid) {
                    BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_SET
                        (unit, dscpecn, prio));
                } else {
                    /* When BCM_DSCP_ECN is not or'ed, 
                      * iterate ecn value to configure DSCP2TCDP table  
                      */
                    for (ecn = 0; ecn < 4; ecn ++) {
                        dscpecn = (srccp << 2 ) |ecn ;
                        BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_SET
                            (unit, dscpecn, prio));
                    }
                }            
            } 
#endif
        } else {
            /* get the mapping queue with giving prio */
            BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                    (unit, -1, prio, &queue));
            /* set DSCP mapping to queue */
            BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_SET
                    (unit, srccp, queue));
        }
    }
        
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_dscp_map_get
 * Purpose:
 *  Get a mapped CoS queue for a specific diffserv code points.
 * Parameters:
 *  unit  - RoboSwitch unit #
 *  port  - RoboSwitch port #, ignore in Robo.
 *  srccp - src code point or -1
 *         For TB, BCM_DSCP_ECN can be or'd with srccp 
 *         to represent {DSCP(7:2):ECN(1:0)}. 
 *  mapcp - (OUT) pointer to returned mapped code point
 *  prio - (OUT) Priority value for mapped code point or -1
 *         For TB, BCM_PRIO_GREEN/BCM_PRIO_YELLOW/
 *                  BCM_PRIO_RED/BCM_PRIO_PRESERVE is or'ed
 *                  to represet both TC and DP value
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  1. the "prio" parameter is used for ROBO TB only.
 */
int
bcm_robo_port_dscp_map_get(int unit, bcm_port_t port, int srccp, int *mapcp,
               int *prio)
{
    uint8 drv_value = 0, pri = 0, queue_num = 0;
    int i = 0;
#ifdef BCM_TB_SUPPORT
    uint8 dp = 0;
    int dscpecn = 0;
    uint8 ecn_valid = 0;    
#endif
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_dscp_map_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }
    
    /*
      * Global DSCP table.
      */
    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) { 
            return BCM_E_PORT;
        }

        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            if (!IS_ALL_PORT(unit, loc_port)) {
                return BCM_E_PORT;
            }
#endif
        } else {
            if (!IS_E_PORT(unit, loc_port)) {
                return BCM_E_PORT;
            }
        }
    }
   
    if ((mapcp == NULL) || (prio == NULL)) {
        return BCM_E_PARAM;
    }
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
        if (srccp & BCM_DSCP_ECN) {
            ecn_valid = 1;
            dscpecn = srccp & ~BCM_DSCP_ECN;
            if (dscpecn < -1 || dscpecn > (DSCP_CODE_POINT_CNT << 2)-1) {
                return BCM_E_PARAM;
            }                        
            srccp = (dscpecn >> 2) & DSCP_CODE_POINT_MAX;
        }
    }
#endif    
    if ((srccp < -1) || (srccp > DSCP_CODE_POINT_MAX) ){
        return BCM_E_PARAM;    
    }

    if (srccp == -1) {
        /* Not support of robo chips */
        return BCM_E_UNAVAIL;
    }
    
    if (soc_feature(unit, soc_feature_dscp)) {
        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TB_AX(unit)) {
                dscpecn = srccp;
            } else {
                /* dscpecn {dscp[7:2], ecn[1:0]} */
                if (!ecn_valid) {
                    dscpecn = (srccp << 2) & ~(0x3);
                }
            } 
            BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_GET
                                (unit, (uint8)dscpecn, &drv_value));

            /* Get DSCP mapping to TCDP (prio = {DP[5:4], TC[3:0]}) */
            *prio = (drv_value & 0x0f);
            dp = ((drv_value & 0xf0) >> 4);
            if (dp == _BCM_COLOR_ENCODING(unit, bcmColorGreen)) {
                *prio |= BCM_PRIO_GREEN;
            } else if (dp == _BCM_COLOR_ENCODING(unit, bcmColorYellow)) {
                *prio |= BCM_PRIO_YELLOW;
            } else if (dp == _BCM_COLOR_ENCODING(unit, bcmColorRed)) {
                *prio |= BCM_PRIO_RED;
            } else if (dp == _BCM_COLOR_ENCODING(unit, bcmColorPreserve)) {
                *prio |= BCM_PRIO_PRESERVE;
            }
#endif
        } else {
            BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_GET
                                (unit, (uint8)srccp, &drv_value));
    
            pri = 8;
            for (i=0; i < pri; i++) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                    (unit, -1, i, &queue_num));
                if (queue_num == drv_value){
                    break;
                }
            }
        
            *prio = i;
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    *mapcp = srccp;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_dscp_unmap_set
 * Purpose:
 *  Define a mapping from internal priority(TCDP) to diffserv code points.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number, ignore in Robo.
 *      internal_pri - (IN) Internal priority (TC)
 *      color        - (IN) color (DP)
 *      pkt_dscp   - (IN) Packet dscp value
 *         For TB B0, BCM_DSCP_ECN can be or'd with pkt_dscp 
 *              to represent {DSCP(7:2):ECN(1:0)}.
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  Now Only TB support this feature on ROBO chip 
 *  for TCDP2DSCP mapping table(Global).
 */
int
bcm_robo_port_dscp_unmap_set(int unit, bcm_port_t port,
                               int internal_pri, bcm_color_t color,
                               int pkt_dscp)
{
#ifdef BCM_TB_SUPPORT
    uint8 prio = 0;
    int ecn_valid = 0, dscpecn=0, ecn;       
#endif
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_dscp_unmap_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    /*
      * Global TCDP2DSCP table.
      */
    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
        if (pkt_dscp & BCM_DSCP_ECN) {
            ecn_valid = 1;
        }
        if (ecn_valid) {
            dscpecn = pkt_dscp & ~BCM_DSCP_ECN;
            if (dscpecn < -1 || dscpecn > (DSCP_CODE_POINT_CNT<< 2)-1) {
                return BCM_E_PARAM;
            }                        
            pkt_dscp = (dscpecn >> 2) & DSCP_CODE_POINT_MAX;
        }
    }    
#endif
    
    if (!_BCM_COSQ_PRIO_VALID(unit, internal_pri)) {
        return BCM_E_PARAM;
    }

    if ((color != bcmColorGreen) && (color != bcmColorYellow) &&
         (color != bcmColorRed) && (color != bcmColorPreserve)) {
        return BCM_E_PARAM;
    }

    if (!_BCM_DSCP_CODE_POINT_VALID(pkt_dscp)) {
        return BCM_E_PARAM;
    }

    if (soc_feature(unit, soc_feature_dscp)) {
        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            if (loc_port != -1) {
                return BCM_E_UNAVAIL;
            }
            /*  Now Only TB support this feature for TCDP2DSCP mapping table */
            /* set TCDP (prio = {DP[1:0], TC[3:0]}) mapping to DSCP */
            prio = (_BCM_COLOR_ENCODING(unit, color) << 4) | internal_pri;

            if (SOC_IS_TB_AX(unit)) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_UNMAP_SET
                    (unit, prio, pkt_dscp));
            } else {
                if (ecn_valid) {
                    BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_UNMAP_SET
                            (unit, prio, dscpecn));

                } else {
                    /* When BCM_DSCP_ECN is not or'ed, 
                      * set ecn=0
                      */
                    ecn = 0;
                    dscpecn = (pkt_dscp << 2 ) |ecn ;
                    BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_UNMAP_SET
                        (unit, prio, dscpecn));

                }
            } 
            
#endif
        } else {
            return BCM_E_UNAVAIL;
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}
#undef DSCP_CODE_POINT_MAX
#undef DSCP_CODE_POINT_CNT
/*
 * Function:
 *  bcm_robo_port_dscp_unmap_get
 * Purpose:
 *  Get a mapped diffserv code points for a specific internal priority(TCDP).
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number, ignore in Robo.
 *      internal_pri - (IN) Internal priority (TC)
 *      color        - (IN) color (DP)
 *      pkt_dscp  - (OUT) Packet dscp value
 *         For TB B0, BCM_DSCP_ECN can be or'd with pkt_dscp 
 *              to represent {DSCP(7:2):ECN(1:0)}.
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  Now Only TB support this feature on ROBO chip 
 *  for TCDP2DSCP mapping table(Global).
 */
int
bcm_robo_port_dscp_unmap_get(int unit, bcm_port_t port,
                                    int internal_pri, bcm_color_t color,
                                    int *pkt_dscp)
{
#ifdef BCM_TB_SUPPORT
    uint8 drv_value = 0, prio = 0;
#endif
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_dscp_unmap_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    /*
      * Global TCDP2DSCP table.
      */
    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }
    }
       
    if (!_BCM_COSQ_PRIO_VALID(unit, internal_pri)) {
        return BCM_E_PARAM;
    }

    if ((color != bcmColorGreen) && (color != bcmColorYellow) &&
         (color != bcmColorRed) && (color != bcmColorPreserve)) {
        return BCM_E_PARAM;
    }

    if (pkt_dscp == NULL) {
        return BCM_E_PARAM;
    }

    if (soc_feature(unit, soc_feature_dscp)) {
        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            if (loc_port != -1) {
                return BCM_E_UNAVAIL;
            }

            /*  Now Only TB support this feature for TCDP2DSCP mapping table */
            prio = (_BCM_COLOR_ENCODING(unit, color) << 4) | internal_pri;

            BCM_IF_ERROR_RETURN(DRV_QUEUE_DFSV_UNMAP_GET
                                (unit, (uint8)prio, &drv_value));

            if (SOC_IS_TB_AX(unit)){
                *pkt_dscp = drv_value;
            } else {
                if (drv_value & 0x3){
                    /* ecn_valid */
                    *pkt_dscp = drv_value | BCM_DSCP_ECN;
                } else {
                    *pkt_dscp = drv_value >> 2;
                }
            } 
#endif
        } else {
            return BCM_E_UNAVAIL;
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * General port functions
 */

/*
 * Function:
 *  bcm_robo_port_linkscan_get
 * Purpose:
 *  Get the link scan state of the port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  linkscan - (OUT) Linkscan value (None, S/W, H/W)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_linkscan_get(int unit, bcm_port_t port, int *linkscan)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_linkscan_get()..\n")));
    return bcm_robo_linkscan_mode_get(unit, port, linkscan);
}

/*
 * Function:
 *  bcm_robo_port_linkscan_set
 * Purpose:
 *  Set the linkscan state for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  linkscan - Linkscan value (None, S/W, H/W)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_linkscan_set(int unit, bcm_port_t port, int linkscan)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_linkscan_set()..\n")));
    return bcm_robo_linkscan_mode_set(unit, port, linkscan);
}
    

/*
 * Function:
 *  bcm_robo_port_autoneg_get
 * Purpose:
 *  Get the autonegotiation state of the port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  autoneg - (OUT) Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_autoneg_get(int unit, bcm_port_t port, int *autoneg)
{
    int         rv = BCM_E_NONE;
    int         drv_act_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_autoneg_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    /* Note : 
     *   - port_get(DRV_PORT_PROP_AUTONEG) will return AN status.
     *      So we need to trnslate status into boolean.
     */
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_AUTONEG, (uint32 *) &drv_act_value);
    *autoneg = (drv_act_value == DRV_PORT_STATUS_AUTONEG_ENABLE) ?
                        TRUE : FALSE;

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_autoneg_set
 * Purpose:
 *  Set the autonegotiation state for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  autoneg - Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_autoneg_set(int unit, bcm_port_t port, int autoneg)
{

    int         rv = BCM_E_NONE;
    int         drv_act_value = 0;
    pbmp_t      t_pbm;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_autoneg_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    drv_act_value = autoneg;
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    rv = DRV_PORT_SET(unit, t_pbm,  DRV_PORT_PROP_AUTONEG, drv_act_value);
    return rv;

}
    

/*
 * Function:
 *  bcm_robo_port_speed_get
 * Purpose:
 *  Getting the speed of the port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
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
bcm_robo_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int     rv = SOC_E_NONE;
    uint32  drv_value = 0U;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_speed_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_SPEED, &drv_value);

    *speed = (drv_value == DRV_PORT_STATUS_SPEED_10M) ? 10 : 
                (drv_value == DRV_PORT_STATUS_SPEED_100M) ? 100 : 
                (drv_value == DRV_PORT_STATUS_SPEED_1G) ? 1000 :
                (drv_value == DRV_PORT_STATUS_SPEED_2G) ? 2000 :
                (drv_value == DRV_PORT_STATUS_SPEED_2500M) ? 2500 :
                (drv_value == DRV_PORT_STATUS_SPEED_10G) ? 10000 :
                0;

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_speed_max
 * Purpose:
 *  Getting the maximum speed of the port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_speed_max(int unit, bcm_port_t port, int *speed)
{
    bcm_port_abil_t pm = 0;
    int  rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_speed_max()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

	rv = bcm_robo_port_ability_get(unit, port, &pm);

    if (BCM_SUCCESS(rv)) {
        if (pm & (BCM_PORT_ABIL_10GB_FD | BCM_PORT_ABIL_10GB_HD)) {
            *speed = 10000;
        } else if (pm & (BCM_PORT_ABIL_2500MB_FD | BCM_PORT_ABIL_2500MB_HD)) {
            *speed = 2500;
        } else if (pm & (BCM_PORT_ABIL_1000MB_FD | BCM_PORT_ABIL_1000MB_HD)) {
            *speed = 1000;
        } else if (pm & (BCM_PORT_ABIL_100MB_FD | BCM_PORT_ABIL_100MB_HD)) {
            *speed = 100;
        } else if (pm & (BCM_PORT_ABIL_10MB_FD | BCM_PORT_ABIL_10MB_HD)) {
            *speed = 10;
        }
    }

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_speed_set
 * Purpose:
 *  Setting the speed for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Turns off autonegotiation.  Caller must make sure other forced
 *  parameters (such as duplex) are set.
 */

int
bcm_robo_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int     rv = BCM_E_NONE;
    pbmp_t  pbm;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API:%s,p=%d,spd=%d\n"),
              FUNCTION_NAME(), port, speed));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (speed == 0) {
        /* if speed is 0, set the port speed to max */
        BCM_IF_ERROR_RETURN
           (bcm_robo_port_speed_max(unit, port, &speed));
    }

    drv_value = (speed == 10) ? DRV_PORT_STATUS_SPEED_10M : 
            (speed == 100) ? DRV_PORT_STATUS_SPEED_100M :
            (speed == 1000) ? DRV_PORT_STATUS_SPEED_1G :
            (speed == 2000) ? DRV_PORT_STATUS_SPEED_2G :
            (speed == 2500) ? DRV_PORT_STATUS_SPEED_2500M :
            DRV_PORT_STATUS_SPEED_10G;
            
    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    rv = DRV_PORT_SET(unit, pbm, DRV_PORT_PROP_SPEED, drv_value);
    if (BCM_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
        (void)bcm_robo_link_change(unit, pbm);
    }

    return rv;
}
    

/*
 * Function:
 *  bcm_robo_port_master_get
 * Purpose:
 *  Getting the master status of the port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ms - (OUT) BCM_PORT_MS_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */

int
bcm_robo_port_master_get(int unit, bcm_port_t port, int *ms)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_master_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* Get the master type */
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_MS, &drv_value);
            
    *ms = (drv_value == SOC_PORT_MS_SLAVE) ? BCM_PORT_MS_SLAVE : 
            (drv_value == SOC_PORT_MS_MASTER) ? BCM_PORT_MS_MASTER : 
            (drv_value == SOC_PORT_MS_AUTO) ? BCM_PORT_MS_AUTO : 
                    BCM_PORT_MS_NONE;

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_master_set
 * Purpose:
 *  Setting the master status for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  ms - BCM_PORT_MS_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Ignored if not supported on port.
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */

int
bcm_robo_port_master_set(int unit, bcm_port_t port, int ms)
{
    int     rv = BCM_E_NONE;
    pbmp_t  pbm;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_master_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    drv_value = (ms == BCM_PORT_MS_SLAVE) ? SOC_PORT_MS_SLAVE : 
                (ms == BCM_PORT_MS_MASTER) ? SOC_PORT_MS_MASTER : 
                (ms == BCM_PORT_MS_AUTO) ? SOC_PORT_MS_AUTO : 
                        SOC_PORT_MS_NONE;
    
    BCM_PBMP_CLEAR(pbm); 
    BCM_PBMP_PORT_ADD(pbm, port);
    /* set the master type */
    rv = DRV_PORT_SET(unit, pbm, DRV_PORT_PROP_MS, drv_value);

    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "PHY_MASTER_SET failed: %s\n"), bcm_errmsg(rv)));
    }

    if (BCM_SUCCESS(rv)) {
        (void)bcm_robo_link_change(unit, pbm);
    }

    return rv;
}
    

/*
 * Function:
 *  bcm_robo_port_interface_get
 * Purpose:
 *  Getting the interface type of a port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  intf - (OUT) BCM_PORT_IF_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcm_robo_port_interface_get(int unit, bcm_port_t port, bcm_port_if_t *intf)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_interface_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = DRV_PORT_GET(unit, port, DRV_PORT_PROP_INTERFACE, &drv_value);
            
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_INTERFACE failed: %s\n"), 
                     bcm_errmsg(rv)));
    }

    *intf = (drv_value == SOC_PORT_IF_MII) ? BCM_PORT_IF_MII : 
            (drv_value == SOC_PORT_IF_GMII) ? BCM_PORT_IF_GMII :
            (drv_value == SOC_PORT_IF_RGMII) ? BCM_PORT_IF_RGMII :
            (drv_value == SOC_PORT_IF_SGMII) ? BCM_PORT_IF_SGMII :
            (drv_value == SOC_PORT_IF_XGMII) ? BCM_PORT_IF_XGMII :
                    BCM_PORT_IF_TBI;

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_interface_set
 * Purpose:
 *  Setting the interface type for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  if - BCM_PORT_IF_*
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcm_robo_port_interface_set(int unit, bcm_port_t port, bcm_port_if_t intf)
{
    int     rv = BCM_E_NONE;
    pbmp_t  pbm;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_interface_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    switch(intf) {
        case BCM_PORT_IF_MII:
            drv_value = SOC_PORT_IF_MII;
            break;
        case BCM_PORT_IF_GMII:
            drv_value = SOC_PORT_IF_GMII;
            break;
        case BCM_PORT_IF_RGMII:
            drv_value = SOC_PORT_IF_RGMII;
            break;
        case BCM_PORT_IF_SGMII:
                drv_value = SOC_PORT_IF_SGMII;
                break;         
        default:
            drv_value = SOC_PORT_IF_TBI;
            break;
    }

    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    rv = DRV_PORT_SET(unit, pbm, DRV_PORT_PROP_INTERFACE, drv_value);
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_INTERFACE failed: %s\n"), 
                     bcm_errmsg(rv)));
    }

    if (BCM_SUCCESS(rv)) {
        (void)bcm_link_change(unit, pbm);
    }

    return rv;
}

 
/*
 * Function:
 *  bcm_robo_port_duplex_get
 * Purpose:
 *  Get the port duplex settings
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      duplex - (OUT) Duplex setting, one of BCM_PORT_DUPLEX_xxx
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_duplex_get(int unit, bcm_port_t port, int *duplex)
{
    int         rv = BCM_E_NONE;
    int         drv_act_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_duplex_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
        
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_DUPLEX, (uint32 *) &drv_act_value);
    
    *duplex = (drv_act_value== DRV_PORT_STATUS_DUPLEX_FULL) ?
                BCM_PORT_DUPLEX_FULL : BCM_PORT_DUPLEX_HALF;

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_duplex_set
 * Purpose:
 *  Set the port duplex settings.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      duplex - Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Turns off autonegotiation.  Caller must make sure other forced
 *  parameters (such as speed) are set.
 */

int
bcm_robo_port_duplex_set(int unit, bcm_port_t port, int duplex)
{
    int         rv = BCM_E_NONE;
    int         drv_act_value = 0;
    pbmp_t      t_pbm;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_duplex_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    if (duplex == BCM_PORT_DUPLEX_HALF){
        drv_act_value = DRV_PORT_STATUS_DUPLEX_HALF;
    }else {
        drv_act_value = DRV_PORT_STATUS_DUPLEX_FULL;      
    }
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    rv = DRV_PORT_SET(unit, t_pbm,
            DRV_PORT_PROP_DUPLEX, drv_act_value);
    if (BCM_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
        (void)bcm_robo_link_change(unit, t_pbm);
    }
    return rv;
}
    

/*
 * Function:
 *  bcm_robo_port_pause_get
 * Purpose:
 *  Get the pause state for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause_tx - (OUT) Boolean value
 *  pause_rx - (OUT) Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */





int
bcm_robo_port_pause_get(int unit, bcm_port_t port, 
                int *pause_tx, int *pause_rx)
{
    uint32  drv_value=0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pause_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_TX_PAUSE, &drv_value));
    *pause_tx = (drv_value) ? TRUE : FALSE;
    BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_RX_PAUSE, &drv_value));
    *pause_rx = (drv_value) ? TRUE : FALSE;
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "\t Port_Pause, tpau=%d, rpau=%d\n"),
              *pause_tx, *pause_rx));

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_pause_set
 * Purpose:
 *  Set the pause state for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause_tx - Boolean value
 *  pause_rx - Boolean value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Symmetric pause requires the two "pause" values to be the same.
 */

int
bcm_robo_port_pause_set(int unit, bcm_port_t port, 
                int pause_tx, int pause_rx)
{
    pbmp_t  t_pbm;
    uint32  drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pause_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    if (pause_tx >= 0){
        drv_value = pause_tx;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                DRV_PORT_PROP_TX_PAUSE, drv_value));
   }
    
    if (pause_rx >= 0){
        drv_value = pause_rx;

        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                DRV_PORT_PROP_RX_PAUSE, drv_value));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_pause_addr_get
 * Purpose:
 *  Get the source address for transmitted PAUSE frames.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause_tx - (OUT) Boolean value
 *  pause_rx - (OUT) Boolean value
 *  mac - (OUT) MAC address sent with pause frames.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_robo_port_pause_addr_get(int unit, bcm_port_t port, sal_mac_addr_t mac)
{
    int         rv = BCM_E_NONE;
    mac_driver_t    *macd;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    if ((rv = soc_robo_mac_probe(unit, port, &macd)) < 0) {
        LOG_WARN(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s : unit %d Port %s: Failed to probe MAC: %s\n"),
                  FUNCTION_NAME(),unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    rv = MAC_PAUSE_ADDR_GET(macd, unit, port, mac);

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_pause_addr_set
 * Purpose:
 *  Set the source address for transmitted PAUSE frames.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause_tx - Boolean value
 *  pause_rx - Boolean value
 *  mac - station address used for pause frames.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Symmetric pause requires the two "pause" values to be the same.
 */
int
bcm_robo_port_pause_addr_set(int unit, bcm_port_t port, sal_mac_addr_t mac)
{
    int         rv = BCM_E_NONE;
    mac_driver_t    *macd;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    if ((rv = soc_robo_mac_probe(unit, port, &macd)) < 0) {
        LOG_WARN(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s : unit %d Port %s: Failed to probe MAC: %s\n"),
                  FUNCTION_NAME(),unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    rv = MAC_PAUSE_ADDR_SET(macd, unit, port, mac);

    return rv;
}
    

/*
 * Function:
 *  bcm_robo_port_pause_sym_get
 * Purpose:
 *  Get the current pause setting for pause
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause - (OUT) returns a bcm_port_pause_e enum value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_pause_sym_get(int unit, bcm_port_t port, int *pause)
{
    int     rv = BCM_E_NONE;
    int     pause_rx = 0, pause_tx = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pause_sym_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = bcm_robo_port_pause_get(unit, port, &pause_tx, &pause_rx);

    BCM_IF_ERROR_RETURN(rv);
    if (pause_tx){
        if (pause_rx){
            *pause = BCM_PORT_PAUSE_SYM;
        } else {
            *pause = BCM_PORT_PAUSE_ASYM_TX;
        }
    } else if (pause_rx){
        *pause = BCM_PORT_PAUSE_ASYM_RX;
    } else {
        *pause = BCM_PORT_PAUSE_NONE;
    }

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_pause_sym_set
 * Purpose:
 *  Set the pause values for the port using single integer
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  pause - a bcm_port_pause_e enum value
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_pause_sym_set(int unit, bcm_port_t port, int pause)
{
    int     pause_rx = 0, pause_tx = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pause_sym_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    switch (pause) {
    case BCM_PORT_PAUSE_SYM:
        pause_tx = pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_RX:
        pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_TX:
        pause_tx = 1;
        break;
    case BCM_PORT_PAUSE_NONE:
        break;
    default:
        return BCM_E_PARAM;
    }

    return bcm_port_pause_set(unit, port, pause_tx, pause_rx);
}

/*
 * Function:
 *  _bcm_robo_port_update
 * Purpose:
 *  Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      link -  True if link is active, false if link is inactive.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
_bcm_robo_port_update(int unit, bcm_port_t port, int link)
{

    int                 rv = BCM_E_NONE;
    int                 duplex = 0, an = 0, an_done = 0;
    int                 mode = 0, enable = 0;
    pbmp_t pbmp;

    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_PORT_ADD(pbmp, port);   

    if (!link) {
        /* PHY is down.  Disable the MAC. */
        rv = DRV_PORT_SET(unit, pbmp, DRV_PORT_PROP_MAC_ENABLE, FALSE);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_ENABLE_SET FALSE rv=%d\n"),
                      unit, port, rv));
            return rv;
        }

        /* PHY link down event */
        rv = (soc_phyctrl_linkdn_evt(unit, port));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_linkdn_evt rv=%d\n"),unit, port, rv));
            return rv;
        }

        return BCM_E_NONE;
    }

    /* PHY link up event may not be support by all PHY driver. 
     * Just ignore it if not supported */
    rv = (soc_phyctrl_linkup_evt(unit, port));
    if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d soc_phyctrl_linkup_evt rv=%d\n"),unit, port, rv));
        return rv;
    }


    rv = DRV_PORT_SW_MAC_UPDATE(unit, pbmp);
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d DRV_PORT_SW_MAC_UPDATE rv=%d\n"),
                  unit, port, rv));
    }


    rv =   (soc_phyctrl_duplex_get(unit, port, &duplex));
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d phyctrl_duplex_get rv=%d\n"),
                  unit, port, rv));
        return rv;
    }

    /*
     * If autonegotiating, check the negotiated PAUSE values, and program
     * MACs accordingly.
     */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_done));

    if (an) {
        bcm_port_ability_t      remote_advert, local_advert;
        int                     tx_pause, rx_pause;

        sal_memset(&local_advert,  0, sizeof(bcm_port_ability_t));
        sal_memset(&remote_advert, 0, sizeof(bcm_port_ability_t));
        rv = soc_phyctrl_ability_advert_get(unit, port, &local_advert); 
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_adv_local_get rv=%d\n"),
                      unit, port, rv));
            return rv;
        }
        rv = soc_phyctrl_ability_remote_get(unit, port, &remote_advert); 
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_adv_remote_get rv=%d\n"),
                      unit, port, rv));
            return rv;
        }

        /*
         * IEEE 802.3 Flow Control Resolution.
         * Please see $SDK/doc/pause-resolution.txt for more information.
         */

        if (duplex) {
             tx_pause =                  
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      (local_advert.pause & SOC_PA_PAUSE_RX)) ||     
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      !(remote_advert.pause & SOC_PA_PAUSE_TX) &&    
                     (local_advert.pause & SOC_PA_PAUSE_TX));    
   
             rx_pause =      
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      (local_advert.pause & SOC_PA_PAUSE_RX)) ||     
                     ((local_advert.pause & SOC_PA_PAUSE_RX) &&      
                      (remote_advert.pause & SOC_PA_PAUSE_TX) &&     
                      !(local_advert.pause & SOC_PA_PAUSE_TX));
        } else {
            rx_pause = tx_pause = 0;
        }

        rv = DRV_PORT_SET(unit, pbmp, DRV_PORT_PROP_TX_PAUSE, tx_pause);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_PAUSE_SET rv=%d\n"),
                      unit, port, rv));
            return rv;
        }
        rv = DRV_PORT_SET(unit, pbmp, DRV_PORT_PROP_RX_PAUSE, rx_pause);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_PAUSE_SET rv=%d\n"),
                      unit, port, rv));
            return rv;
        }

    }    

    /* Enable the MAC. */
    enable = TRUE;
    
    /* GNATS 40307 : Unexpected override on Port discard configuration while  
     *        the link been re-established.
     *
     *  Note :
     *  1. Except TB chips, all ROBO chips use RX disable to implemente the 
     *     port discard related configuration.
     *      >> Thus the chip list for this PR to apply is :
     *          - bcm53242/53262/53115/53118/53101/53125/53128/5389/5396
     *  2. Use positive list in source code to avoid unexpected processing 
     *      flow on new ROBO chips.
     *
     *  P.S
     *    - processing flow here is for link condition only.
     */
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO(unit)) {
        BCM_IF_ERROR_RETURN(bcm_robo_port_discard_get(unit, port, &mode));
        if (mode == BCM_PORT_DISCARD_ALL){
            enable = DRV_SPECIAL_MAC_ENABLE_NORX;
        }
    }

    rv = DRV_PORT_SET(unit, pbmp, DRV_PORT_PROP_ENABLE, enable);    
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d MAC_ENABLE_SET TRUE rv=%d\n"),
                  unit, port, rv));
        return rv;
    }
    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_update
 * Purpose:
 *  Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *  unit -  RoboSwitch Unit #.
 *  port -  RoboSwitch port #.
 *  link -  TRUE - process as link up.
 *          FALSE - process as link down.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_update(int unit, bcm_port_t port, int link)
{
    int     rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_update()..port=%d, link=%d\n"),
              port, link));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_update(unit, port, link);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_update: u=%d p=%d link=%d rv=%d\n"),
              unit, port, link, rv));

    return(rv);

}
    
/*
 * Function:
 *  bcm_port_frame_max_get
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
bcm_robo_port_frame_max_get(int unit, bcm_port_t port, int * size)
{
    int     rv = BCM_E_NONE;
    uint32  temp = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_frame_max_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = DRV_PORT_GET(unit, port, DRV_PORT_PROP_MAX_FRAME_SZ, &temp);
            
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_INTERFACE failed: %s\n"), 
                     bcm_errmsg(rv)));
    }

    *size = temp;

    return rv;
}

/*
 * Function:
 *  bcm_port_frame_max_set
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
bcm_robo_port_frame_max_set(int unit,bcm_port_t port, int size)
{
    int     rv = BCM_E_NONE;
    pbmp_t  pbm;
    uint32      temp = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_frame_max_set()..\n")));
    
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);

    temp = size;
    rv = DRV_PORT_SET(unit, pbm, DRV_PORT_PROP_MAX_FRAME_SZ, temp);
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_MAX_FRAME_SZ failed: %s\n"), 
                     bcm_errmsg(rv)));
    }

    return rv;
}
    

/*
 * Function:
 *  bcm_port_jam_get
 * Description:
 *  Return half duplex jamming state
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  enable - (OUT) non-zero if jamming enabled
 * Return Value:
 *  BCM_E_XXX
 */
int
bcm_robo_port_jam_get(int unit, bcm_port_t port, int * enable)
{
    uint32    drv_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_jam_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_E_PORT(unit, port)) {    /* CPU ports */
        *enable = 0;
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(DRV_PORT_GET(
            unit, port, DRV_PORT_PROP_JAM, &drv_value));
    *enable = drv_value ? TRUE : FALSE;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_port_jam_set
 * Description:
 *  Enable or disable half duplex jamming on a port
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  enable - non-zero to enable jamming
 * Return Value:
 *  BCM_E_XXX
 * Note :
 *  1. RoboSwitch allowed per port flow control setting but such 
 *     setting effect on Pause(FD) and Jamming(HD) at the same time.
 *     The Pause and Jamming can't be set independent.
 *  2. This API for setting Jamming flow control in Robo chip might 
 *     effects pause flow control setting for above reason.
 */
int
bcm_robo_port_jam_set(int unit, bcm_port_t port, int enable)
{
    pbmp_t pbmp;
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_jam_set()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_E_PORT(unit, port)) {    /* CPU ports */
        return BCM_E_NONE;
    }
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, port);
    BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, pbmp, DRV_PORT_PROP_JAM, enable));
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_port_ifg_get
 * Description:
 *  Gets the new ifg (Inter-frame gap) value
 * Parameters:
 *  unit   - Device number
 *  port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *  ifg    - Inter-frame gap in bit-times
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 */
int
bcm_robo_port_ifg_get( int unit, bcm_port_t port, int speed,
                bcm_port_duplex_t duplex, int * bit_times)
{
    int rv = BCM_E_NONE;
    
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_ifg_get()..\n")));
            
    if (speed == 1000){
        /* get GE IPG */
        rv = DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_IPG_GE, (uint32 *) bit_times);
    } else {
        /* get FE IPG */
        rv = DRV_PORT_GET(unit, port,
                DRV_PORT_PROP_IPG_FE, (uint32 *) bit_times);
    }

    return (rv);
}

/*
 * Function:
 *  bcm_port_ifg_set
 * Description:
 *  Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *  unit   - Device number
 *  port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *  ifg    - Inter-frame gap in bit-times
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 *      1. The function makes sure the IFG value makes sense and updates the
 *         IPG register in case the speed/duplex match the current settings
 *      2. Robo5324 can only set the IPG(IFG) per device but per port basis.
 *         That's why this API is unavailable.
 */
int
bcm_robo_port_ifg_set(int unit, bcm_port_t port, int speed,
                bcm_port_duplex_t duplex, int bit_times)
{
    return BCM_E_UNAVAIL;
}
                            

/*
 * Additional PHY-related APIs
 */

/*
 * Function:
 *  bcm_port_phy_drv_name_get
 * Purpose:
 *  Return the name of the PHY driver being used on a port.
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 * Returns:
 *  Pointer to static string
 * Note :
 *  This API is un-dispatchable, if any upper layer application
 *  trying to call this API should give the full name(with "robo_"
 *  string appended to this API's name after "bcm_xxx") to point 
 *  the process to this API.
 */

int
bcm_robo_port_phy_drv_name_get(int unit, bcm_port_t port, char *name, int len)
{
    uint32 val = FALSE;
    int rv = BCM_E_NONE;
    int str_len = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_phy_drv_name_get()..\n")));

    rv = DRV_PORT_STATUS_GET(unit, 0, DRV_PORT_STATUS_INIT, &val);
    SOC_IF_ERROR_RETURN(rv);    

    if (!val) {
        str_len = sal_strlen("driver not initialized");
        if (str_len <= len) {
            sal_strncpy(name, "driver not initialized",str_len);
        }
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        str_len = sal_strlen("invalid port");
        if (str_len <= len) {
            sal_strncpy(name, "invalid port", str_len);
        }
        return BCM_E_PORT;
    }
    return (soc_phyctrl_drv_name_get(unit, port, name, len)); 
}

/*
 * Direct PHY register access
 */

/*
 * Function:
 *  int bcm_port_phy_get
 * Description:
 *  General PHY register read
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  flags - Logical OR of one or more of the following flags:
 *      BCM_PORT_PHY_INTERNAL
 *          Address internal SERDES PHY for port
 *      BCM_PORT_PHY_NOMAP
 *          Instead of mapping port to PHY MDIO address,
 *          treat port parameter as actual PHY MDIO address.
 *      BCM_PORT_PHY_CLAUSE45
 *          Assume Clause 45 device instead of Clause 22
 *  phy_addr - PHY internal register address
 *  phy_data - (OUT) Data that was read
 * Returns:
 *  BCM_E_XXX
 */

int
bcm_robo_port_phy_get(int unit, bcm_port_t port, uint32 flags,
                    uint32 phy_reg_addr, uint32 * phy_data)
{
    uint8  phy_id = 0;
    uint16 phy_reg = 0;
    uint16 phy_rd_data = 0;
    int    rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_phy_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    if (flags & BCM_PORT_PHY_NOMAP) {
        phy_id = port;
    } else {
        if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }
    }
    
    if (flags & BCM_PORT_PHY_CLAUSE45) {
        return BCM_E_UNAVAIL;
    } else {
        phy_reg = phy_reg_addr;
        rv = soc_robo_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
    }

    if (rv == SOC_E_NONE) {
        *phy_data = phy_rd_data;
    }

    return rv;
}

/*
 * Function:
 *  int bcm_port_phy_set
 * Description:
 *  General PHY register write
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  flags - Logical OR of one or more of the following flags:
 *      BCM_PORT_PHY_INTERNAL
 *          Address internal SERDES PHY for port
 *      BCM_PORT_PHY_NOMAP
 *          Instead of mapping port to PHY MDIO address,
 *          treat port parameter as actual PHY MDIO address.
 *      BCM_PORT_PHY_CLAUSE45
 *          Assume Clause 45 device instead of Clause 22
 *  phy_addr - PHY internal register address
 *  phy_data - Data to write
 * Returns:
 *  BCM_E_XXX
 */

int
bcm_robo_port_phy_set(int unit, bcm_port_t port, uint32 flags,
                    uint32 phy_reg_addr, uint32 phy_data)
{
    uint8  phy_id = 0;
    uint16 phy_reg = 0;
    uint16 phy_wr_data = 0;
    int    rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_phy_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    if (flags & BCM_PORT_PHY_NOMAP) {
        phy_id = port;
    } else {
        if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }
    }
    
    if (flags & BCM_PORT_PHY_CLAUSE45) {
        return BCM_E_UNAVAIL;
    } else {
        phy_reg = phy_reg_addr;
        phy_wr_data = phy_data;
        rv = soc_robo_miim_write(unit, phy_id, phy_reg, phy_wr_data);
    }

    return rv;
}

/*
 * Function:
 *  int bcm_port_phy_modify
 * Description:
 *  General PHY register modify
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  flags - Logical OR of one or more of the following flags:
 *      BCM_PORT_PHY_INTERNAL
 *          Address internal SERDES PHY for port
 *      BCM_PORT_PHY_NOMAP
 *          Instead of mapping port to PHY MDIO address,
 *          treat port parameter as actual PHY MDIO address.
 *      BCM_PORT_PHY_CLAUSE45
 *          Assume Clause 45 device instead of Clause 22
 *  phy_addr - PHY internal register address
 *  phy_data - Data to write
 *  phy_mask - Bits to modify using phy_data
 * Returns:
 *  BCM_E_XXX
 */

int
bcm_robo_port_phy_modify(int unit, bcm_port_t port, uint32 flags,
                         uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint8  phy_id = 0;
    uint16 phy_reg = 0;
    uint16 phy_rd_data = 0;
    uint16 phy_wr_data = 0;
    int    rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_phy_modify()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    if (flags & BCM_PORT_PHY_NOMAP) {
        phy_id = port;
    } else {
        if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }
    }
    
    if (flags & BCM_PORT_PHY_CLAUSE45) {
        return BCM_E_UNAVAIL;
    } else {
        phy_reg = phy_reg_addr;
        rv = soc_robo_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
        if (BCM_SUCCESS(rv)) {
            phy_wr_data = (phy_data & phy_mask);
            phy_wr_data |= (phy_rd_data & ~phy_mask);
            rv = soc_robo_miim_write(unit, phy_id, phy_reg, phy_wr_data);
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_reset
 * Description:
 *      This function performs the low-level PHY reset and is intended to be
 *      called ONLY from callback function registered with 
 *      bcm_port_phy_reset_register. Attempting to call it from any other 
 *      place will break lots of things.
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_port_phy_reset(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    pbmp_t pbmp;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_phy_reset()..\n")));

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, port);
    if ((rv = DRV_PORT_SET(unit, pbmp, 
            DRV_PORT_PROP_PHY_RESET, TRUE)) < 0) {
        return rv;
    }
    return rv;
}

/* 
 * Function:
 *      bcm_port_phy_reset_register
 * Description:
 *      Register a callback function to be called whenever a PHY driver
 *      needs to perform a PHY reset 
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function 
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data} 
 *                           combination have not been registered before
 */
int
bcm_robo_port_phy_reset_register(int unit, bcm_port_t port, 
                        bcm_port_phy_reset_cb_t callback, void * user_data)
{
    return soc_phy_reset_register(unit, port, callback, user_data, FALSE);
}

/* 
 * Function:
 *      bcm_port_phy_reset_unregister
 * Description:
 *      Unregister a callback function to be called whenever a PHY driver
 *      needs to perform a PHY reset 
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function 
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data} 
 *                           combination have not been registered before
 */
int
bcm_robo_port_phy_reset_unregister(int unit,bcm_port_t port,
                        bcm_port_phy_reset_cb_t callback,void * user_data)
{
    return soc_phy_reset_unregister(unit, port, callback, user_data);
}


/*
 * MDI crossover control and status
 */
 

/*
 * Function:
 *  bcm_port_mdix_get
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
bcm_robo_port_mdix_get(int unit, bcm_port_t port, bcm_port_mdix_t * mode)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_mdix_get()..\n")));

    /* get port mdix mode */
    rv = DRV_PORT_GET(unit, port,  DRV_PORT_PROP_PHY_MDIX, &drv_value);
    *mode = (drv_value == SOC_PORT_MDIX_AUTO) ? BCM_PORT_MDIX_AUTO : 
            (drv_value == SOC_PORT_MDIX_FORCE_AUTO) ? BCM_PORT_MDIX_FORCE_AUTO :
            (drv_value == SOC_PORT_MDIX_NORMAL) ? BCM_PORT_MDIX_NORMAL :
                            BCM_PORT_MDIX_XOVER;
    

    return (rv);
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
bcm_robo_port_mdix_set(
    int unit,
    bcm_port_t  port,
    bcm_port_mdix_t mode)
{
    int rv = BCM_E_NONE;
    pbmp_t      t_pbm;
    uint32      drv_value = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_mdix_set()..\n")));
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    /* set port mdix mode */
    drv_value = (mode == BCM_PORT_MDIX_AUTO) ? SOC_PORT_MDIX_AUTO : 
            (mode == BCM_PORT_MDIX_FORCE_AUTO) ? SOC_PORT_MDIX_FORCE_AUTO :
            (mode == BCM_PORT_MDIX_NORMAL) ? SOC_PORT_MDIX_NORMAL : 
                            SOC_PORT_MDIX_XOVER;
    rv = DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_PHY_MDIX, drv_value);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_robo_port_mdix_set()=%d: \n\t port_set()>>drv_value=%x\n"), 
              rv, drv_value));
    return (rv);
}

/*
 * Function:
 *  bcm_port_mdix_status_get
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
bcm_robo_port_mdix_status_get(
    int unit,
    bcm_port_t  port,
    bcm_port_mdix_status_t *    status)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;
    
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_mdix_status_get()..\n")));
    /* get port mdix status */
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_PHY_MDIX_STATUS, &drv_value);
    *status = (drv_value == SOC_PORT_MDIX_STATUS_NORMAL) ? 
                                BCM_PORT_MDIX_STATUS_NORMAL : 
                                BCM_PORT_MDIX_STATUS_XOVER;
    
    return (rv);
}
                                   

/*
 * Combo port control/status 
 */

/*
 * Function:
 *      bcm_port_medium_config_get
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
bcm_robo_port_medium_config_get(
    int unit,
    bcm_port_t  port,
    bcm_port_medium_t   medium,
    bcm_phy_config_t *  config)
{
    int     rv = BCM_E_NONE;
    
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_medium_config_get()..\n")));
	
    /* get port medium config */
    rv = soc_phyctrl_medium_config_get(unit, port,
		medium, config);

    return (rv);
}

/*
 * Function:
 *      bcm_port_medium_config_set
 * Description:
 *      Set the medium-specific configuration for a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 to apply the configuration to
 *      config   - per-medium configuration
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_robo_port_medium_config_set(
    int unit,
    bcm_port_t  port,
    bcm_port_medium_t   medium,
    bcm_phy_config_t *  config)
{
    int rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_medium_config_set()..\n")));
                
    /* set port medium config */
	rv = soc_phyctrl_medium_config_set(unit, port,
		medium, config);
    return (rv);
}
                                      
/*
 * Function:
 *      bcm_port_medium_config_get
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
bcm_robo_port_medium_get(
    int unit,
    bcm_port_t  port,
    bcm_port_medium_t * medium)
{
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_medium_get()..\n")));
    /* get port medium type */
    rv = DRV_PORT_GET(unit, port, DRV_PORT_PROP_PHY_MEDIUM, &drv_value);
    *medium = drv_value;
    return (rv);
}

/* 
 * Function:
 *      bcm_robo_port_medium_status_register
 * Description:
 *      Register a callback function to be called on medium change event
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function 
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM  -- NULL function pointer or bad {unit, port} combination
 *      BCM_E_FULL   -- Cannot register more than 1 callback per {unit, port}
 */
int
bcm_robo_port_medium_status_register(
    int unit,
    bcm_port_t  port,
    bcm_port_medium_status_cb_t callback,
    void *  user_data)
{
    return (soc_phy_medium_status_register(
                    unit, port, callback, user_data));
}

/* 
 * Function:
 *      bcm_robo_port_medium_status_unregister
 * Description:
 *      Unegister a callback function to be called on medium change event
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function 
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data} 
 *                           combination have not been registered before
 */
int
bcm_robo_port_medium_status_unregister(
    int unit,
    bcm_port_t  port,
    bcm_port_medium_status_cb_t callback,
    void *  user_data)
{
    return (soc_phy_medium_status_unregister(
                    unit, port, callback, user_data));
}

/*
 * Flags used for loopback modes
 */

 
/*
 * Function:
 *  bcm_robo_port_loopback_set
 * Purpose:
 *  Setting the speed for a given port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  loopback - one of:
 *      BCM_PORT_LOOPBACK_NONE
 *      BCM_PORT_LOOPBACK_MAC
 *      BCM_PORT_LOOPBACK_PHY
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
#define _LINK_FORCE_WAIT_TIME  10
int
bcm_robo_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    int         rv = BCM_E_NONE;
    pbmp_t      t_pbm;
    int         mode = 0;
    
#if defined(BCM_53125) || defined(BCM_53128)
    uint32 temp = 0;
    soc_timeout_t to;
#endif /* BCM_53125 || BCM_53128 */
#ifdef BCM_TB_SUPPORT
    char *s;
#endif
    


    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_loopback_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    /* 
     * Always force link before changing hardware to avoid 
     * race with the linkscan thread.
     */
    if (!(loopback == BCM_PORT_LOOPBACK_NONE)) {
        if ((rv = DRV_PORT_SET(unit, t_pbm, 
            DRV_PORT_PROP_MAC_ENABLE, FALSE)) < 0) {
            return rv;
        }
        rv = _bcm_robo_link_force(unit, port, TRUE, FALSE);
        sal_usleep(_LINK_FORCE_WAIT_TIME);

        /* If MAC low power mode is enabled, disabling it. */
#if defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128) ||\
    defined(BCM_POLAR_SUPPORT)
        if (SOC_MAC_LOW_POWER_ENABLED(unit)) {
            rv = DRV_DEV_PROP_SET(unit, 
                DRV_DEV_PROP_LOW_POWER_ENABLE, 0);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Port %s: Failed to disable the Low Power Mode: %s\n"), 
                           SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
            } else {
                MAC_LOW_POWER_LOCK(unit);
                SOC_MAC_LOW_POWER_ENABLED(unit) = 0;
                MAC_LOW_POWER_UNLOCK(unit);
#if defined(BCM_53101) || defined (BCM_POLAR_SUPPORT)
                if (SOC_IS_LOTUS(unit) || SOC_IS_POLAR(unit)) {
                    BCM_IF_ERROR_RETURN
                    	(_bcm_robo53101_phy_cfg_restore(unit));
                }
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */                
            }
        }
#endif /* BCM_53101 || BCM_53125 || BCM_53128 || BCM_POLAR_SUPPORT || */
    }
    if (BCM_SUCCESS(rv)) {
        /* Set MAC loopback status */
        rv = DRV_PORT_SET(unit, t_pbm,DRV_PORT_PROP_MAC_LOOPBACK, 
                (loopback == BCM_PORT_LOOPBACK_MAC));
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_robo_port_loopback_set(%d):set MAC loopback, rv=%d\n"), 
                  (loopback == BCM_PORT_LOOPBACK_MAC),
                  rv));
    }
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TB_SUPPORT
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
            bcm_robo_port_autoneg_set(unit, port, 0);
            bcm_robo_port_speed_set(unit, port, 100);
        }
#endif        
        /* Set PHY loopback status */
        rv = DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_PHY_LOOPBACK, 
                (loopback == BCM_PORT_LOOPBACK_PHY));
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_robo_port_loopback_set(%d):set PHY loopback, rv=%d\n"), 
                  (loopback == BCM_PORT_LOOPBACK_PHY),
                  rv));
    }

    if ((loopback == BCM_PORT_LOOPBACK_NONE) || !BCM_SUCCESS(rv)) {
        _bcm_robo_link_force(unit, port, FALSE, DONT_CARE);
    }else {                                              
        _bcm_robo_link_force(unit, port, TRUE, TRUE);        
        sal_usleep(_LINK_FORCE_WAIT_TIME);
        rv = bcm_robo_port_discard_get(unit, port, &mode);
        /* The fix process for GNATS 40551 on TB device could be observed the 
         *  return code at BCM_E_FAIL but this failed condition normally 
         *  will be treated as BCM_E_NONE to prevent some unexpect thread  
         *  processes been broken
         */
        if (rv == BCM_E_NONE || (SOC_IS_TBX(unit) && rv == BCM_E_FAIL)){
            if (mode != BCM_PORT_DISCARD_ALL) {
                if ((rv = DRV_PORT_SET(unit, t_pbm, 
                        DRV_PORT_PROP_MAC_ENABLE, TRUE)) < 0) {
                    return rv;            
                }
            }
        }

    }

#if defined(BCM_53125) || defined(BCM_53128)
    /* If the internal CPU is enable, wait to leave the low power mode */
    if ((SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) &&
        (SOC_CONTROL(unit)->int_cpu_enabled) &&
        (loopback == BCM_PORT_LOOPBACK_PHY)) {
            /* Wait 8051 leave MAC LOW POWER mode */
            rv = DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_LOW_POWER_ENABLE, &temp);
            soc_timeout_init(&to, BCM_ROBO_LOW_POWER_WAIT_TIME, 0);
            while (temp != FALSE) {
                if (soc_timeout_check(&to)) {
                    LOG_CLI((BSL_META_U(unit,
                                        "bcm_robo_port_loopback_set: the switch is in low power mode.\n")));
                    break;
                }
                rv = DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_LOW_POWER_ENABLE, &temp);
                sal_usleep(10000);
            }
            
        }
#endif /* BCM_53125 || BCM_53128 */

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_loopback_set: u=%d p=%d lb=%d rv=%d\n"),
              unit, port, loopback, rv));

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_loopback_get
 * Purpose:
 *  Recover the current loopback operation for the specified port.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  loopback - (OUT) one of:
 *      BCM_PORT_LOOPBACK_NONE
 *      BCM_PORT_LOOPBACK_MAC
 *      BCM_PORT_LOOPBACK_PHY
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{
    int     rv = BCM_E_NONE;
    int     phy_lb = 0, mac_lb = 0;
    uint32  drv_act_value = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_loopback_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* Get PHY loopback status */
    drv_act_value = 0;
    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_PHY_LOOPBACK, &drv_act_value);
    phy_lb = drv_act_value;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_loopback_get: phy_lb=%x rv=%d\n"),
              drv_act_value, rv));
    if (rv >= 0) {
        /* Get MAC loopback status */
        drv_act_value = 0;
        rv = DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_MAC_LOOPBACK, &drv_act_value);
        mac_lb = drv_act_value;
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_port_loopback_get: mac_lb=%x rv=%d\n"),
                  drv_act_value, rv));
    }

    if (rv >= 0) {
        if (mac_lb) {
            *loopback = BCM_PORT_LOOPBACK_MAC;
        } else if (phy_lb) {
            *loopback = BCM_PORT_LOOPBACK_PHY;
        } else {
            *loopback = BCM_PORT_LOOPBACK_NONE;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_loopback_get: u=%d p=%d lb=%d rv=%d\n"),
              unit, port, *loopback, rv));

    return rv;
}
    

/*
 * Function:
 *      bcm_robo_port_stp_set
 * Purpose:
 *      Set the spanning tree state for a port.
 *  All STGs containing all VLANs containing the port are updated.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      port - RoboSwitch port number.
 *      stp_state - State to place port in, one of BCM_STG_STP_xxx.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 * Notes:
 *  BCM_LOCK is taken so that the current list of VLANs
 *  can't change during the operation.
 */

int
bcm_robo_port_stp_set(int unit, bcm_port_t port, int stp_state)
{
    bcm_stg_t  *list = NULL;
    int         count = 0, i;
    int         rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_stp_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = bcm_stg_list(unit, &list, &count);

    if (BCM_SUCCESS(rv)) {
        for (i = 0; i < count; i++) {
            if ((rv = bcm_stg_stp_set(unit, list[i], 
                                          port, stp_state)) < 0) {
                break;
            }
        }
        
        bcm_stg_list_destroy(unit, list, count);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_stp_set: u=%d p=%d state=%d rv=%d\n"),
              unit, port, stp_state, rv));

    return rv;
}



/*
 * Function:
 *  bcm_robo_port_stp_get
 * Purpose:
 *  Get the spanning tree state for a port in the default STG.
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  port - RoboSwitch port number.
 *  stp_state - Pointer where state stored.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL
 */

int
bcm_robo_port_stp_get(int unit, bcm_port_t port, int *stp_state)
{
    int stg_defl = 0, rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_stp_get()..\n")));
    rv = bcm_stg_default_get(unit, &stg_defl);
    if (BCM_SUCCESS(rv))  {
        rv = bcm_stg_stp_get(unit, stg_defl, port, stp_state);
    }
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_stp_get: u=%d p=%d state=%d rv=%d\n"),
              unit, port, *stp_state, rv));    
    return rv;         
}

/*
 * Modes for discard and the configuration combination set.
 *  - DISCARD_NONE  : !drop_untag + !drop_pritag + !drop_1Qtag
 *  - DISCARD_ALL   : drop_untag + drop_pritag + drop_1Qtag
 *  - DISCARD_UNTAG : !drop_untag + drop_pritag + drop_1Qtag
 *  - DISCARD_TAG   : drop_untag + drop_pritag + drop_1Qtag
 */
#define _BCM_ROBO_DISCARD_UNKNOW    -1
#define _BCM_ROBO_DISCARD_MODE_SELECT(dp_untag, dp_pritag, dp_1qtag)    \
        ((!(dp_untag) && !(dp_pritag) && !(dp_1qtag)) ? BCM_PORT_DISCARD_NONE :\
        ((dp_untag) && (dp_pritag) && (dp_1qtag)) ? BCM_PORT_DISCARD_ALL :\
        ((dp_untag) && !(dp_pritag) && !(dp_1qtag)) ? BCM_PORT_DISCARD_UNTAG :\
        (!(dp_untag) && (dp_pritag) && (dp_1qtag)) ? BCM_PORT_DISCARD_TAG :\
            _BCM_ROBO_DISCARD_UNKNOW)

/*
 * Function:
 *  bcm_robo_port_discard_get
 * Purpose:
 *  Get port discard attributes for the specified port
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      mode - (OUT) Port discard mode, one of BCM_PORT_DISCARD_xxx
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_discard_get(int unit, bcm_port_t port, int *mode)
{
    bcm_port_cfg_t * pcfg;
    int     rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_discard_get()..\n")));
    pcfg = &robo_port_config[unit][port];
    
    /* GNATS 40551 : 
     *  - Inter-Operation issue between bcm_robo_port_discard_set() and 
     *      bcm_robo_vlan_port_control_set().
     *
     * Note : 
     *  1. TB chips can approach all defined discard modes, i.e. NONE, ALL, 
     *      UNTAG and TAG, through some register configurations.
     *      a. Report BCM_E_FAIL if the current register configurations is not 
     *          match any one of those four discard modes.
     *      b. Report BCM_E_NONE if current register configurations can match 
     *          any one of those four discard modes. Besides, to update the SW 
     *          is requirred once this latest retrieved mode is different from 
     *          current SW database.
     *  2. None TB chips to support DISCARD_ALL need to disable MAC_RX to 
     *      approach API request.
     */
    if (SOC_IS_TBX(unit)){
        uint32  drop_untag = 0, drop_pritag = 0, drop_1qtag = 0;
        int     temp_mode;

        /* no drop on Untag / pri-tag / 1Qtag */
        BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_ENABLE_DROP_UNTAG, &drop_untag));
        BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_ENABLE_DROP_PRITAG, &drop_pritag));
                
        /* TB's DROP_1Q is deisgned by two combined register configuration.
         *  both on or off will be expected for proper configuration by this 
         *  drv type, a.k.a. DRV_PORT_PROP_ENABLE_DROP_1Q.
         *
         *  If these two combined register configurations are different in 
         *  each other. the error code at SOC_E_FAIL will be reported.
         */ 
        rv = DRV_PORT_GET(unit, port, 
                DRV_PORT_PROP_ENABLE_DROP_1Q, &drop_1qtag);

        if (rv == BCM_E_NONE){
            temp_mode = _BCM_ROBO_DISCARD_MODE_SELECT(
                    drop_untag, drop_pritag, drop_1qtag);
            if (temp_mode == _BCM_ROBO_DISCARD_UNKNOW){
                rv = BCM_E_FAIL;
            } else {
                if (temp_mode != pcfg->pc_disc){
                    /* update the sw database.*/
                    pcfg->pc_disc = temp_mode;
                }
            }
        }
    }
    
    /* Report by SW database for the DISCARD_ALL mode can't be sperated  
     *  on those none-TB chips by checking the MAC_RX when the port is 
     *  at link-down status.
     *   - MAC_RX will be disabled also when the port is link-down.
     */
    *mode = pcfg->pc_disc;
        
    return rv;   
}

/*
 * Function:
 *  bcm_robo_port_discard_set
 * Purpose:
 *  Set port discard attributes for the specified port.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  mode - Port discard mode, one of BCM_PORT_DISCARD_xxx
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_discard_set(int unit, bcm_port_t port, int mode)
{
    bcm_port_cfg_t  *pcfg;
    pbmp_t          t_pbm;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_discard_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (SOC_IS_TBX(unit)){
        switch (mode){
            case BCM_PORT_DISCARD_NONE:
                /* no drop on Untag / pri-tag / 1Qtag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_UNTAG, FALSE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_PRITAG, FALSE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_1Q, FALSE));
                break;
            case BCM_PORT_DISCARD_ALL:
                /* drop on Untag / pri-tag / 1Qtag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_UNTAG, TRUE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_PRITAG, TRUE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_1Q, TRUE));
                break;
            case BCM_PORT_DISCARD_UNTAG:
                /* drop on Untag only */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_UNTAG, TRUE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_PRITAG, FALSE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_1Q, FALSE));
                break;
            case BCM_PORT_DISCARD_TAG:
                /* drop on pri-tag / 1Qtag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_UNTAG, FALSE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_PRITAG, TRUE));
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_1Q, TRUE));
                break;
            default:
                return BCM_E_PARAM;
        }
    } else {
        switch (mode){
            case BCM_PORT_DISCARD_NONE:
                /* enable Rx */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_RX, TRUE));
                /* enable Untag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_NON1Q, FALSE));
                break;
            
            case BCM_PORT_DISCARD_ALL:
                /* disable Rx */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_RX, FALSE));
                /* disable Untag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_NON1Q, TRUE));
                break;
            
            case BCM_PORT_DISCARD_UNTAG:
                /* enable Rx */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_RX, TRUE));
                /* disable Untag */
                BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                        DRV_PORT_PROP_ENABLE_DROP_NON1Q, TRUE));
                break;
            case BCM_PORT_DISCARD_TAG:
                return BCM_E_UNAVAIL;
                break;
            
            default:
                return BCM_E_PARAM;
        }
    }
                           
    pcfg = &robo_port_config[unit][port];
    pcfg->pc_disc = mode;
        
    return BCM_E_NONE;   
}

/*
 * Flags for learn mode
 *
 * This call takes flags to turn on and off mutually-independent actions
 * that should be taken when a packet is received with an unknown source
 * address, or source lookup failure (SLF).
 *
 * The set call returns BCM_E_UNAVAIL for flag combinations that are
 * not supported by the hardware.
 */

/*
 * Function:
 *      _bcm_port_learn_modify
 * Purpose:
 *      Main part of bcm_port_learn_modify.
 */

STATIC int
_bcm_robo_port_learn_modify(int unit, bcm_port_t port, 
                           uint32 add, uint32 remove)
{
    uint32      flags = 0;

    SOC_IF_ERROR_RETURN(bcm_port_learn_get(unit, port, &flags));

    flags |= add;
    flags &= ~remove;

    SOC_IF_ERROR_RETURN(bcm_port_learn_set(unit, port, flags));

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_learn_modify
 * Purpose:
 *  Modify the port learn flags, adding add and removing remove flags.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  add  - Flags to set.
 *      remove - Flags to clear.
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_robo_port_learn_modify(int unit, 
                bcm_port_t port, 
                uint32 add, 
                uint32 remove)
{
    int         rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = _bcm_robo_port_learn_modify(unit, port, add, remove);

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_learn_get
 * Purpose:
 *  Get the ARL hardware learning options for this port.
 *  This defines what the hardware will do when a packet
 *  is seen with an unknown address.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  flags - (OUT) Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
int
bcm_robo_port_learn_get(int unit, bcm_port_t port, uint32 *flags)
{
    bcm_port_cfg_t      *pcfg;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    pcfg = &robo_port_config[unit][port];

    if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
        uint32  temp = 0;

        *flags = pcfg->pc_cml;
        BCM_IF_ERROR_RETURN(DRV_PORT_GET
            (unit, port, DRV_PORT_PROP_DISABLE_LEARN, &temp));
        if (!temp) {
            *flags |= BCM_PORT_LEARN_ARL;
        }
        BCM_IF_ERROR_RETURN(DRV_PORT_GET
            (unit, port, DRV_PORT_PROP_SA_UNKNOWN_DROP, &temp));
        if (!temp) {
            *flags |= BCM_PORT_LEARN_FWD;
        }
        BCM_IF_ERROR_RETURN(DRV_PORT_GET
            (unit, port, DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY, &temp));
        if (temp) {
            *flags |= BCM_PORT_LEARN_CPU;
        }
        BCM_IF_ERROR_RETURN(DRV_PORT_GET
            (unit, port, DRV_PORT_PROP_SW_LEARN_MODE, &temp));
        if (temp) {
            *flags |= BCM_PORT_LEARN_PENDING;
        }
#endif /* BCM_TB_SUPPORT */
    } else {
        switch (pcfg->pc_cml) {
        case PVP_CML_SWITCH:
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_FWD |
                      (pcfg->pc_cpu ? BCM_PORT_LEARN_CPU : 0));
            break;
        case PVP_CML_CPU:
            *flags = BCM_PORT_LEARN_CPU;
            break;
        case PVP_CML_FORWARD:
            *flags = BCM_PORT_LEARN_FWD;
            break;
        case PVP_CML_DROP:
            *flags = 0;
            break;
        case PVP_CML_CPU_SWITCH:            /* Possible on Draco only */
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_CPU |
                      BCM_PORT_LEARN_FWD);
            break;
        case PVP_CML_CPU_FORWARD:           /* Possible on Draco only */
            *flags = BCM_PORT_LEARN_CPU | BCM_PORT_LEARN_FWD;
            break;
        default:
            return BCM_E_INTERNAL;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_learn_set
 * Purpose:
 *  Set the ARL hardware learning options for this port.
 *  This defines what the hardware will do when a packet
 *  is seen with an unknown address.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  flags - Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */
#define Arl     BCM_PORT_LEARN_ARL
#define Cpu     BCM_PORT_LEARN_CPU
#define Fwd     BCM_PORT_LEARN_FWD

int
bcm_robo_port_learn_set(int unit, bcm_port_t port, uint32 flags)
{
    int             rv = BCM_E_NONE;
    bcm_port_cfg_t  *pcfg;
    bcm_pbmp_t      pbmp;
    int             cml = 0, drv_cml = -1;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(pbmp); 
    BCM_PBMP_PORT_ADD(pbmp, port);

    pcfg = &robo_port_config[unit][port];

    pcfg->pc_cpu = ((flags & BCM_PORT_LEARN_CPU) != 0);
    if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
        int     temp;

        /* the pc_cml is the chip specific control filed in ESW Ptable.
         *  - Here we redefine the pc_cml for ROBO to be a generic control 
         *      bitmap to represent the learning mode.
         */
        pcfg->pc_cml = 0;
        temp  = (flags & BCM_PORT_LEARN_ARL) ? FALSE : TRUE;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_DISABLE_LEARN, temp));
        pcfg->pc_cml |= flags & BCM_PORT_LEARN_ARL;
        
        temp  = (flags & BCM_PORT_LEARN_FWD) ? FALSE : TRUE;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_SA_UNKNOWN_DROP, temp));
        pcfg->pc_cml |= flags & BCM_PORT_LEARN_FWD;

        temp  = (flags & BCM_PORT_LEARN_CPU) ? TRUE : FALSE;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY, temp));
        pcfg->pc_cml |= flags & BCM_PORT_LEARN_CPU;

        temp  = (flags & BCM_PORT_LEARN_PENDING) ? TRUE : FALSE;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_SW_LEARN_MODE, temp));
        pcfg->pc_cml |= flags & BCM_PORT_LEARN_PENDING;

        rv = BCM_E_NONE;
#endif /* BCM_TB_SUPPORT */
    } else {
        /* Use shortened names to handle each flag combination individually */
    
        switch (flags) {
        case ((!Arl) | (!Cpu) | (!Fwd)):
            cml = PVP_CML_DROP;
            drv_cml = DRV_PORT_DROP;
            break;
        case ((!Arl) | (!Cpu) | ( Fwd)):
            cml = PVP_CML_FORWARD;
            drv_cml = DRV_PORT_DISABLE_LEARN;
            break;
        case ((!Arl) | ( Cpu) | (!Fwd)):
            cml = PVP_CML_CPU;
            drv_cml = DRV_PORT_SWLRN_DROP;
            break;
        case ((!Arl) | ( Cpu) | ( Fwd)):
            cml = PVP_CML_CPU_FORWARD;
            drv_cml = DRV_PORT_SW_LEARN;
            break;
        case (( Arl) | (!Cpu) | (!Fwd)):
            return BCM_E_UNAVAIL;
            break;
        case (( Arl) | (!Cpu) | ( Fwd)):
            cml = PVP_CML_SWITCH;
            drv_cml = DRV_PORT_HW_LEARN;
            break;
        case (( Arl) | ( Cpu) | (!Fwd)):
            return BCM_E_UNAVAIL;
            break;
        case (( Arl) | ( Cpu) | ( Fwd)):
            cml = PVP_CML_CPU_SWITCH;
            drv_cml = DRV_PORT_HW_SW_LEARN;
            break;
        default :
            return BCM_E_UNAVAIL;
        }
    
        rv = DRV_ARL_LEARN_ENABLE_SET(unit, pbmp, drv_cml);
        if (BCM_SUCCESS(rv)){
            pcfg->pc_cml = cml;
        }
    }

    return rv;
}

#undef Arl
#undef Cpu
#undef Fwd
                 

/*
 * Function:
 *  bcm_robo_port_link_status_get
 * Purpose:
 *  Return current Link up/down status, queries linkscan, if unable to
 *  retrieve status queries the PHY.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *  up - (OUT) Boolean value, FALSE for link down and TRUE for link up
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 */

int
bcm_robo_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    int     rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_link_status_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

        rv = _bcm_robo_link_get(unit, port, up);
        if (rv == BCM_E_DISABLED) {
            rv = _bcm_robo_port_link_get(unit, port, 0, up);
        }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_linkstatus_get: u=%d p=%d up=%d rv=%d\n"),
              unit, port, *up, rv));

    return rv;
}
                
    

/*
 * Modes for VLAN input filtering
 * (see also VLAN-based ifilter setting in vlan.h)
 */
/*
 * Function:
 *  bcm_robo_port_ifilter_get
 * Description:
 *  Return input filter mode for a port.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number to operate on
 *  mode - (OUT) Filter mode, one of BCM_PORT_IFILTER_xxx
 * Returns:
 *  BCM_E_NONE      Success.
 *  BCM_E_INTERNAL      Chip access failure.
 */
int
bcm_robo_port_ifilter_get(int unit, bcm_port_t port, int *mode)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *  bcm_robo_port_ifilter_set
 * Description:
 *  Set input filter mode for a port.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number to operate on
 *  mode - BCM_PORT_IFILTER_ON or BCM_PORT_IFILTER_OFF.
 * Returns:
 *  BCM_E_NONE      Success.
 *  BCM_E_INTERNAL      Chip access failure.
 * Notes:
 *  When input filtering is turned on for a port, packets received
 *  on the port that do not match the port's VLAN classifications
 *  are discarded.
 */
int
bcm_robo_port_ifilter_set(int unit, bcm_port_t port, int mode)
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *  bcm_robo_port_bpdu_enable_set
 * Purpose:
 *  Enable/Disable BPDU reception on the specified port.
 * Parameters:
 *  unit - SOC unit #
 *  port - Port number (0 based)
 *  enable - TRUE to enable, FALSE to disable (reject bpdu).
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL
 */

int
bcm_robo_port_bpdu_enable_set(int unit, bcm_port_t port, int enable)
{
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_bpdu_enable_set()..\n")));
    /* Robo Chips could not per-port set */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *  bcm_robo_port_bpdu_enable_get
 * Purpose:
 *  Return whether BPDU reception is enabled on the specified port.
 * Parameters:
 *  unit - SOC unit #
 *  port - Port number (0 based)
 *  enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_INTERNAL
 */

int
bcm_robo_port_bpdu_enable_get(int unit, bcm_port_t port, int *enable)
{
    int         rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_bpdu_enable_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_BPDU_RX, (uint32 *) enable);

    return rv;
}
    
/*
 * Function:
 *  bcm_port_l3_enable_get
 * Purpose:
 *  Return whether L3 switching is enabled on the specified port.
 * Parameters:
 *  unit -      device number
 *  port -      port number
 *  enable -    (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_port_l3_enable_get(
    int unit,
    bcm_port_t  port,
    int *   enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *  bcm_port_l3_enable_set
 * Purpose:
 *  Enable/Disable L3 switching on the specified port.
 * Parameters:
 *  unit -      device number
 *  port -      port number
 *  enable -    TRUE to enable, FALSE to disable.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_port_l3_enable_set(
    int unit,
    bcm_port_t  port,
    int enable)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_port_tgid_get
 * Purpose:
 *       Get the trunk group for a given port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      tid - trunk ID
 *      psc - trunk selection criterion
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int
bcm_robo_port_tgid_get(
    int unit,
    bcm_port_t  port,
    int *   tgid,
    int *   psc)
{
    int                 rv = BCM_E_NONE;
    pbmp_t              t_pbmp, trunk_pbmp;
    int                 i;
    uint32              hash_op = 0;
    bcm_trunk_chip_info_t   ti;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_tgid_get()..\n")));
    BCM_IF_ERROR_RETURN(bcm_trunk_chip_info_get(unit, &ti));
    for (i = ti.trunk_id_min ; i <= ti.trunk_id_max ; i++){
        BCM_PBMP_CLEAR(trunk_pbmp);
        rv = DRV_TRUNK_GET(unit, i, &trunk_pbmp, 
                DRV_TRUNK_FLAG_BITMAP, &hash_op);
        BCM_IF_ERROR_RETURN(rv);
        
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "\nbcm_robo_port_tgid_get():tgid=%d,pbmp=%x\n"), \
                  i,SOC_PBMP_WORD_GET(trunk_pbmp, 0)));
        BCM_PBMP_CLEAR(t_pbmp);
        SOC_PBMP_ASSIGN(t_pbmp, trunk_pbmp);
        if (PBMP_MEMBER(t_pbmp, port)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "bcm_robo_port_tgid_get():port in tgid=%d\n"),i));
            break;
        }
    }

    if (i <= ti.trunk_id_max){
        
        *tgid = i;
        BCM_IF_ERROR_RETURN(bcm_trunk_psc_get(unit, i, psc));
    } else {
        *tgid = BCM_TRUNK_INVALID;
        *psc = 0;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_tgid_set
 * Purpose:
 *       Set the trunk group for a given port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      tid - trunk ID
 *      psc - trunk selection criterion
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int
bcm_robo_port_tgid_set(
    int unit,
    bcm_port_t  port,
    int tgid,
    int psc)
{
    return BCM_E_UNAVAIL;

}
    
/*
 * Port Filter Modes
 */
#define MLF_FORWARD_FLOOD 0x0
#define MLF_FORWARD_BY_MLF_MAP 0x1


/*
 * Function:
 *      _bcm_robo_port_pfm_set
 * Purpose:
 *      Main part of function bcm_port_pfm_set
 */

STATIC int
_bcm_robo_port_pfm_set(int unit, bcm_port_t port, int mode)
{
    int reg_len = 0, rv = BCM_E_NONE;
    int cur_mode = 0;
    uint32 reg_addr = 0, reg_value = 0, fld_value = 0;
    uint64 reg_val64;
    int reg_id = 0, field_id = 0;

    rv = bcm_robo_port_pfm_get(unit, port, &cur_mode);
    if (rv < 0) {
        return rv;
    }

    if (cur_mode == mode) {
        return BCM_E_NONE;
    }

    if (port != -1) {
        return BCM_E_UNAVAIL;
    }
    
    COMPILER_64_ZERO(reg_val64);
    
    switch (mode) {
        case BCM_PORT_MCAST_FLOOD_NONE:
            /* Clear MLF packet forward map */
            if (NUM_ALL_PORT(unit) > 32) {
                BCM_IF_ERROR_RETURN(REG_WRITE_MLF_DROP_MAPr(unit, &reg_val64));
            } else {
                BCM_IF_ERROR_RETURN(REG_WRITE_MLF_DROP_MAPr(unit, &reg_value));
            }

            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                reg_id = INDEX(NEW_CTRLr);
                field_id = INDEX(MC_FWD_ENf);
            } else if (SOC_IS_DINO(unit)) {
                reg_id = INDEX(NEW_CONTROLr);
                field_id = INDEX(MC_DLF_FWDf);
            } else {
                reg_id = INDEX(NEW_CONTROLr);
                field_id = INDEX(MLF_FM_ENf);
            }
            /* MLF Forward Configure register */
            reg_len = DRV_REG_LENGTH_GET(unit, reg_id);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_id, 0, 0);
            
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                            (unit, reg_addr, &reg_value, reg_len)) < 0)  {
                return rv;
            }
            fld_value = MLF_FORWARD_BY_MLF_MAP;
            BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_id, &reg_value, field_id, &fld_value));

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                            (unit, reg_addr, &reg_value, reg_len)) < 0)  {
                return rv;
            }
            break;
        case BCM_PORT_MCAST_FLOOD_UNKNOWN:
            
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                reg_id = INDEX(NEW_CTRLr);
                field_id = INDEX(MC_FWD_ENf);
            } else if (SOC_IS_DINO(unit)) {
                reg_id = INDEX(NEW_CONTROLr);
                field_id = INDEX(MC_DLF_FWDf);
            } else {
                reg_id = INDEX(NEW_CONTROLr);
                field_id = INDEX(MLF_FM_ENf);
            }
            /* MLF Forward Configure register */
            reg_len = DRV_REG_LENGTH_GET(unit, reg_id);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_id, 0, 0);
            
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                            (unit, reg_addr, &reg_value, reg_len)) < 0)  {
                return rv;
            }
            fld_value = MLF_FORWARD_FLOOD;
            BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_id, &reg_value, field_id, &fld_value));

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                            (unit, reg_addr, &reg_value, reg_len)) < 0)  {
                return rv;
            }
            break;
        default : 
            rv = BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_robo_port_pfm_set
 * Purpose:
 *  Set current port filtering mode (see port.h)
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #, -1 apply to all ports.
 *      mode - mode for PFM bits (see port.h)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  The is system-base configure for Robo chips.
 *    Need to asssign port number as -1.
 */
int
bcm_robo_port_pfm_set(int unit, bcm_port_t port, int mode)
{
    int                 rv = BCM_E_NONE;
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pfm_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }
    }

    if (SOC_IS_TBX(unit)){
        /* Like XGS3 chips, this pfm feature is moved to VLAN table and not 
         *  not suitable for port basis configuration.
         *  - Defualt behavior is MCAST_FLOOD_UNKNOWN
         */
        return BCM_E_UNAVAIL;
    }
    rv = _bcm_robo_port_pfm_set(unit, loc_port, mode);

    return rv;
}

/*
 * Function:
 *  bcm_robo_port_pfm_get
 * Purpose:
 *  Return current port filtering mode (see port.h)
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      mode - (OUT) mode read from RoboSwitch for PFM bits (see port.h)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  The is system-base configure for Robo chips.
 */
int
bcm_robo_port_pfm_get(int unit, bcm_port_t port, int *mode)
{
    int reg_len = 0, rv = BCM_E_NONE;
    uint32 reg_addr = 0, reg_value = 0, temp = 0;
    uint64   reg_val64, fld_val64;
    int reg_id = 0, field_id = 0;
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_pfm_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1) {
        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }
    }
    
    COMPILER_64_ZERO(reg_val64);
    COMPILER_64_ZERO(fld_val64);

    if (SOC_IS_TBX(unit)){
        /* TB's multicsast flood control is implemented in 
         *  bcm_port_discard_get.
         *  - Defualt behavior is MCAST_FLOOD_UNKNOWN
         */
        return BCM_E_UNAVAIL;
    }
    
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        reg_id = INDEX(NEW_CTRLr);
        field_id = INDEX(MC_FWD_ENf);
    } else if (SOC_IS_DINO(unit)) {
        reg_id = INDEX(NEW_CONTROLr);
        field_id = INDEX(MC_DLF_FWDf);
    } else {
        reg_id = INDEX(NEW_CONTROLr);
        field_id = INDEX(MLF_FM_ENf);
    }
    
    /* Get MLF configure value */
    reg_len = DRV_REG_LENGTH_GET(unit, reg_id);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_id, 0, 0);
    
    if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0)  {
        return rv;
    }
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
    (unit, reg_id, &reg_value, field_id, &temp));
    
    if (temp == MLF_FORWARD_FLOOD) {
        *mode = BCM_PORT_MCAST_FLOOD_UNKNOWN;
    } else if (temp == MLF_FORWARD_BY_MLF_MAP) {
    
        /* Check Mcast Forwarding Map */
        if (NUM_ALL_PORT(unit) > 32) {
            
            COMPILER_64_ZERO(reg_val64);
            BCM_IF_ERROR_RETURN(REG_READ_MLF_DROP_MAPr(unit, &reg_val64));
            BCM_IF_ERROR_RETURN(soc_MLF_DROP_MAPr_field_get
                (unit, (void *)&reg_val64, MLF_FWD_MAPf, (void *)&fld_val64));
            if (COMPILER_64_IS_ZERO(fld_val64)) {
                *mode = BCM_PORT_MCAST_FLOOD_NONE;
            } else {
                *mode = BCM_PORT_MCAST_FLOOD_UNKNOWN;
            }
        } else {
            reg_value = 0;
            BCM_IF_ERROR_RETURN(REG_READ_MLF_DROP_MAPr(unit, &reg_value));
            if (SOC_IS_ROBO_GE_SWITCH(unit) || SOC_IS_LOTUS(unit)) {
                BCM_IF_ERROR_RETURN(soc_MLF_DROP_MAPr_field_get
                    (unit, &reg_value, MUL_LOOKUP_FAIL_FRW_MAPf, &temp));
            } else {
                BCM_IF_ERROR_RETURN(soc_MLF_DROP_MAPr_field_get
                    (unit, &reg_value, MLF_FWD_MAPf, &temp));
            }
            if (temp == 0) {
                *mode = BCM_PORT_MCAST_FLOOD_NONE;
            } else {
                *mode = BCM_PORT_MCAST_FLOOD_UNKNOWN;
            }
        }
        *mode = BCM_PORT_MCAST_FLOOD_NONE;
    } else {
        rv = BCM_E_INTERNAL;
    }
    return rv;
}

/*
 * Function:
 *  bcm_robo_port_queued_count_get
 * Purpose:
 *  Returns the count of packets (or cells) currently buffered
 *  for a port.  Useful to know when a port has drained all
 *  data and can then be re-configured.
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  count (OUT) - count of packets currently buffered
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *    "packets" may actually be cells on most chips,
 *  If no packets are buffered, the cell count is 0,
 *  If some packets are buffered the cell count will be
 *  greater than or equal to the packet count.
 */
int
bcm_robo_port_queued_count_get(int unit, bcm_port_t port, uint32 *count)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *  _bcm_robo_vlan_port_protocol_action_add
 * Description:
 *  Add the action on an created protocol valn.
 * Parameters:
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 *      frame     (IN) Frame type
 *      ether     (IN) 16 bit ether type
 *      action    (IN) Action for outer tag and inner tag
 * Returns:
 *      BCM_E_xxxx
 * Note:
 */
int 
_bcm_robo_vlan_port_protocol_action_add(int unit,
                                      bcm_port_t port,
                                      bcm_port_frametype_t frame,
                                      bcm_port_ethertype_t ether,
                                      bcm_vlan_action_set_t *action)
{
    protocol2vlan_entry_t  protcolment;
    uint32  temp = 0;
    bcm_vlan_t  vid;
    bcm_cos_t   priority;
    
    /* only bcm53242/53262 provide HW protocol valn supported only */
    if (!(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* set valid field */
    temp = 1;
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&protcolment, (uint32 *)&temp));

    /* set ether_type field */
    temp = ether;
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
                (uint32 *)&protcolment, (uint32 *)&temp));

    if (NULL == action) {
        return BCM_E_PARAM;
    }

    /* Check invalid inner vid and VLAN tag actions which will be ignored for Harrier */
    _ROBO_VLAN_CHK_ID(unit, action->new_inner_vlan);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_outer_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_inner_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_outer_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_inner_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ut_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->ut_inner);
    /* Cannot delete outer tag on a SIT pkt */
    /* Cannot delete inner tag on a SOT pkt */
    /* Cannot add inner tag to a DT pkt */
    if ((action->it_outer == bcmVlanActionDelete) ||
        (action->ot_inner == bcmVlanActionDelete) ||
        (action->dt_inner == bcmVlanActionAdd)) {
        return BCM_E_PARAM;
    }

    /* retreive the vid and pri from action 
     *  1. vid : get new_outer_vlan from action.
     *      - ignore inner vid setting in action 
     *  2. pri : get priority from action.
     */
    _ROBO_VLAN_CHK_ID(unit, action->new_outer_vlan);
    _ROBO_VLAN_CHK_PRIO(unit,  action->priority);

    vid = action->new_outer_vlan;
    priority = action->priority;
    
    /* set vid field */
    temp = vid;
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VLANID,
                (uint32 *)&protcolment, (uint32 *)&temp));
    
    /* set pri field */
    temp = priority;
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_PRIORITY,
                (uint32 *)&protcolment, (uint32 *)&temp));
                
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_insert)
                (unit, DRV_MEM_PROTOCOLVLAN, (uint32 *)&protcolment, 
                DRV_MEM_OP_REPLACE));
    
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_protocol_vlan_add
 * Purpose:
 *  Adds a protocol based vlan to a port.  The protocol
 *  is matched by frame type and ether type.  Returns an
 *  error if hardware does not support protocol vlans
 *    (Strata and Hercules).
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  frame - one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *  ether - 16 bit Ethernet type field
 *  vid - vlan id
 * Returns:
 *  BCM_E_XXX
 * Note:
 *    This API in robobcm53242 won't process the port and frame 
 *      parameters.
 */
int
bcm_robo_port_protocol_vlan_add(int unit,
               bcm_port_t port,
               bcm_port_frametype_t frame,
               bcm_port_ethertype_t ether,
               bcm_vlan_t vid)
{
    bcm_vlan_action_set_t action;
    
    bcm_vlan_action_set_t_init(&action);
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        uint32 pri = 0, tc = 0, dp = 0;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        action.new_outer_vlan = vid;
        action.new_inner_vlan = 0;
        /* Get port default priority */
        BCM_IF_ERROR_RETURN(
            bcm_port_untagged_priority_get(unit, port, (int *)&pri));
        /* Get traffic class by port default priority */
        DRV_PORT_PRI_MAPOP_GET(unit, port, 
                DRV_PORT_OP_PCP2TC, pri, 0, &tc, &dp);
        action.priority = tc; /* Traffic class as the internal priority */
        action.ut_outer = bcmVlanActionAdd;
        action.it_outer = bcmVlanActionAdd;
        action.it_inner = bcmVlanActionDelete;
        action.it_inner_prio = bcmVlanActionDelete;
        action.ot_outer_prio = bcmVlanActionReplace;
        action.dt_outer_prio = bcmVlanActionReplace;

        return bcm_vlan_port_protocol_action_add(unit, port, frame,
                                                     ether, &action);
    }
#endif
    
    /* action value assignment : */
    action.new_outer_vlan = vid;
    action.new_inner_vlan = vid;
    action.priority = 0;    /* default priority */
    
    /* other action items are not supported in ROBO device yet */
    
    return _bcm_robo_vlan_port_protocol_action_add(unit, port, frame,
                                        ether, &action);
    
}

/*
 * Function:
 *  bcm_robo_port_protocol_vlan_delete
 * Purpose:
 *  Remove an already created proto protocol based vlan
 *  on a port.
 * Parameters:
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 *      frame     (IN) Frame type
 *      ether     (IN) 16 bit ether type
 *      action    (IN) Action for outer tag and inner tag
 * Returns:
 *  BCM_E_XXX
 * Note:
 *    This API in robobcm53242 won't process the port, frame 
 *      parameters.
 */
int
bcm_robo_port_protocol_vlan_delete(int unit,
                  bcm_port_t port,
                  bcm_port_frametype_t frame,
                  bcm_port_ethertype_t ether)
{
    int rv = BCM_E_NONE;
    protocol2vlan_entry_t  protcolment;
    uint32  temp = 0;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s()\n"),FUNCTION_NAME()));

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        return bcm_robo_vlan_port_protocol_action_delete(unit, port, frame, ether);
    }
#endif

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    sal_memset(&protcolment, 0, sizeof(protcolment));

    /* set ether_type field */
    temp = ether;
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
        (uint32 *)&protcolment, (uint32 *)&temp));

    rv = (DRV_SERVICES(unit)->mem_delete)
        (unit, DRV_MEM_PROTOCOLVLAN, (uint32 *)&protcolment, 0);
    return rv;
}

/*
 * Function:
 *  bcm_robo_port_protocol_vlan_delete_all
 * Purpose:
 *  Remove all protocol based vlans on a port.
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_port_protocol_vlan_delete_all(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s()\n"),FUNCTION_NAME()));
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        return bcm_vlan_port_protocol_action_delete_all(unit, port);
    }
#endif

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    rv = (DRV_SERVICES(unit)->mem_clear)
        (unit, DRV_MEM_PROTOCOLVLAN);

    return rv;
}

/*
 * Function:
 *  bcm_port_egress_set
 * Description:
 *  Set switching only to indicated ports from given (modid, port).
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *  pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      if port < 0, means all/any port
 *      if modid < 0, means all/any modid
 */
int bcm_robo_port_egress_set(int unit, bcm_port_t port, int modid,
                        bcm_pbmp_t pbmp)
{
    bcm_pbmp_t t_pbm, i_pbm;
    bcm_port_t  p, loc_port;
    int rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_egress_set()..\n")));

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    BCM_PBMP_CLEAR(i_pbm);
    /* if port = -1, the configuration is applied to all source ports on the specified module */
    if (loc_port == -1) {
        BCM_PBMP_ASSIGN(i_pbm, PBMP_ALL(unit));
    } else {
        PORT_PARAM_CHECK(unit, loc_port);
        BCM_PBMP_PORT_SET(i_pbm, loc_port);
    }

    if (modid != 0) {
        return BCM_E_PARAM;
    }
    BCM_LOCK(unit);

    BCM_PBMP_ITER(i_pbm, p) {
        BCM_PBMP_CLEAR(t_pbm);
        rv = DRV_PORT_VLAN_GET(unit, p, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }
        BCM_PBMP_AND(pbmp, PBMP_ALL(unit));
        if (!BCM_PBMP_EQ(pbmp, t_pbm)) {
            rv = DRV_PORT_VLAN_SET(unit, p, pbmp);
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }
        }
    }
    BCM_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_port_egress_get
 * Description:
 *  Retrieve bitmap of ports for which switching is enabled
 *      for (modid, port).
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *  pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int bcm_robo_port_egress_get(int unit, bcm_port_t port, int modid,
                        bcm_pbmp_t *pbmp)
{
    bcm_pbmp_t t_pbm;
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_egress_get()..\n")));
     
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }
 
    if ((modid != 0) || (!SOC_PORT_VALID(unit, loc_port))) {
        return BCM_E_PARAM;
    }

    BCM_PBMP_CLEAR(t_pbm);
    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_GET(unit, loc_port, &t_pbm));

    BCM_PBMP_AND(t_pbm, PBMP_ALL(unit));
    BCM_PBMP_ASSIGN(*pbmp, t_pbm);

    return BCM_E_NONE;
}
                               

/* Source module control */

/*
 * Function:
 *      bcm_port_modid_egress_get
 * Description:
 *      Retrieve port bitmap of egress ports on which the incoming packets
 *      will be forwarded.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      This API is designed for Hercules15. Not suitable for Robo.
 */
int 
bcm_robo_port_modid_egress_get(int unit, bcm_port_t port,
                              bcm_module_t modid, bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_modid_egress_set
 * Description:
 *      Set port bitmap of egress ports on which the incoming packets
 *      will be forwarded.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      if port < 0, means all/any port
 *      if modid < 0, means all/any modid
 * Note:
 *      This API is designed for Hercules15. Not suitable for Robo.
 */
int 
bcm_robo_port_modid_egress_set(int unit, bcm_port_t port,
                              bcm_module_t modid, bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *    bcm_robo_port_modid_enable_set
 * Purpose:
 *    Enable/block packets from a specific module on a port.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    port - RoboSwitch port number.
 *    modid - Which source module id to enable/disable
 *    enable - TRUE/FALSE Enable/disable forwarding packets from
 *             source module arriving on port.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_robo_port_modid_enable_set(int unit, 
                bcm_port_t port, 
                int modid, 
                int enable)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *    bcm_robo_port_modid_enable_get
 * Purpose:
 *    Return enable/block state for a specific module on a port.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    port - RoboSwitch port number.
 *    modid - Which source module id to enable/disable
 *    enable - (OUT) TRUE/FALSE Enable/disable forwarding packets from
 *             source module arriving on port.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_robo_port_modid_enable_get(int unit, 
                bcm_port_t port, 
                int modid, 
                int *enable)
{
    return (BCM_E_UNAVAIL);
}
                                     
/* Flood blocking */
/*
 * Function:
 *  bcm_robo_port_flood_block_set
 * Purpose:
 *  Set selective per-port blocking of flooded VLAN traffic
 * Parameters:
 *  unit        - unit number
 *  ingress_port    - Port traffic is ingressing
 *  egress_port - Port for which the traffic should be blocked.
 *  flags       - Specifies the type of traffic to block
 * Returns:
 *  BCM_E_UNAVAIL   - Functionality not available
 *  BCM_E_NONE
 */
int
bcm_robo_port_flood_block_set(int unit,
             bcm_port_t ingress_port, 
             bcm_port_t egress_port,
             uint32 flags)
{
    pbmp_t t_pbm, opbm;
    int rv = BCM_E_NONE;
#ifdef  BCM_TB_SUPPORT
    uint32  ori_block_types = 0, changed = 0;
#endif  /* BCM_TB_SUPPORT */

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_flood_block_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, ingress_port, &ingress_port));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, egress_port, &egress_port));

    BCM_LOCK(unit);

    BCM_PBMP_CLEAR(t_pbm);
    if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT

        /* the same designing style with ESW to set egress blocking port on 
         * all supported flood block type.
         */
        rv = bcm_robo_port_flood_block_get(
                unit, ingress_port, egress_port, &ori_block_types);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            
        /* set block all */
        changed = ((flags & BCM_PORT_FLOOD_BLOCK_ALL)^ 
                (ori_block_types & BCM_PORT_FLOOD_BLOCK_ALL)) ? 1 : 0;
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_ALL, &opbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            
        
        if (changed) {
            BCM_PBMP_ASSIGN(t_pbm, opbm);
            if (flags & BCM_PORT_FLOOD_BLOCK_ALL){
                BCM_PBMP_PORT_ADD(t_pbm, egress_port);
            } else {
                BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
            }
            rv = DRV_PORT_BLOCK_SET(
                    unit, ingress_port, DRV_BLOCK_ALL, t_pbm);            
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }            
        }
                
        /* set block unicast dlf */
        changed = ((flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST)^ 
                (ori_block_types & BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST)) ? 
                1 : 0;
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_UCAST, &opbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            

        
        if (changed) {
            BCM_PBMP_ASSIGN(t_pbm, opbm);
            if (flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST){
                BCM_PBMP_PORT_ADD(t_pbm, egress_port);
            } else {
                BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
            }
            rv = DRV_PORT_BLOCK_SET(
                    unit, ingress_port, DRV_BLOCK_DLF_UCAST, t_pbm);
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }                        
        }

        /* set block broadcast */
        changed = ((flags & BCM_PORT_FLOOD_BLOCK_BCAST)^ 
                (ori_block_types & BCM_PORT_FLOOD_BLOCK_BCAST)) ? 1 : 0;
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_BCAST, &opbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            
        
        if (changed) {
            BCM_PBMP_ASSIGN(t_pbm, opbm);
            if (flags & BCM_PORT_FLOOD_BLOCK_BCAST){
                BCM_PBMP_PORT_ADD(t_pbm, egress_port);
            } else {
                BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
            }
            rv = DRV_PORT_BLOCK_SET(
                    unit, ingress_port, DRV_BLOCK_BCAST, t_pbm);
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }            
        }

        /* set block mcast dlf else check ip_mcast and nonid_mcast */
        if (flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST) {
            flags |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST | 
                    BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST;
        }
        if (ori_block_types & BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST) {
            ori_block_types |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST | 
                    BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST;
        }

        /* set ip mcast */
        changed = ((flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST)^ 
                (ori_block_types & BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST)) ? 
                1 : 0;
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_IP_MCAST, &opbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            
        
        if (changed) {
            BCM_PBMP_ASSIGN(t_pbm, opbm);
            if (flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST){
                BCM_PBMP_PORT_ADD(t_pbm, egress_port);
            } else {
                BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
            }
            rv = DRV_PORT_BLOCK_SET(
                    unit, ingress_port, DRV_BLOCK_DLF_IP_MCAST, t_pbm);            
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }            
        }
        
        /* set nonip mcast */
        changed = ((flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST)^ 
                (ori_block_types & BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST)) ? 
                1 : 0;
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_NONIP_MCAST, 
                &opbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }            
                
        if (changed) {
            BCM_PBMP_ASSIGN(t_pbm, opbm);
            if (flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST){
                BCM_PBMP_PORT_ADD(t_pbm, egress_port);
            } else {
                BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
            }
            rv = DRV_PORT_BLOCK_SET(
                    unit, ingress_port, DRV_BLOCK_DLF_NONIP_MCAST, t_pbm);            
            if (rv < 0){
                BCM_UNLOCK(unit);
                return rv;
            }            
        }
     
#else   /* BCM_TB_SUPPORT */
        BCM_UNLOCK(unit);
        return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
    } else {
        /* original design support BLOCK_ALL type only */
        rv = DRV_PORT_VLAN_GET(unit, ingress_port, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }     
        BCM_PBMP_ASSIGN(opbm, t_pbm);
        if (flags) {
            BCM_PBMP_PORT_REMOVE(t_pbm, egress_port);
        } else {
            BCM_PBMP_PORT_ADD(t_pbm, egress_port);
        }
    
        if ((!flags) || (flags & BCM_PORT_FLOOD_BLOCK_ALL)) {
            if (BCM_PBMP_NEQ(t_pbm, opbm)) {
                rv = DRV_PORT_VLAN_SET(
                        unit, ingress_port, t_pbm);
                if (rv < 0){
                    BCM_UNLOCK(unit);
                    return rv;
                } 
            }
        } else {
            BCM_UNLOCK(unit);        
            return BCM_E_UNAVAIL;
        }
    }
    BCM_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_flood_block_get
 * Purpose:
 *  Get the current per-port flood block settings
 * Parameters:
 *  unit        - unit number
 *  ingress_port    - Port traffic is ingressing
 *  egress_port - Port for which the traffic would be blocked
 *  flags       - (OUT) Returns the current settings
 * Returns:
 *  BCM_E_UNAVAIL   - Functionality not available
 *  BCM_E_NONE
 */
int
bcm_robo_port_flood_block_get(int unit,
             bcm_port_t ingress_port, bcm_port_t egress_port,
             uint32 *flags)
{
    pbmp_t t_pbm;
    int rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_flood_block_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, ingress_port, &ingress_port));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, egress_port, &egress_port));

    BCM_LOCK(unit);

    BCM_PBMP_CLEAR(t_pbm);
    *flags = 0;
    if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
        /* check block all */
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_ALL, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }

        if (BCM_PBMP_MEMBER(t_pbm, egress_port)){
            *flags |= BCM_PORT_FLOOD_BLOCK_ALL;
        }
        
        /* check block unicast dlf */
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_UCAST, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }        
        if (BCM_PBMP_MEMBER(t_pbm, egress_port)){
            *flags |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST;
        }

        /* check block broadcast */
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_BCAST, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }        
        if (BCM_PBMP_MEMBER(t_pbm, egress_port)){
            *flags |= BCM_PORT_FLOOD_BLOCK_BCAST;
        }

        /* check block mcast dlf (check ip_mcast and nonid_mcast for TB) */
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_NONIP_MCAST, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }        
        if (BCM_PBMP_MEMBER(t_pbm, egress_port)){
            *flags |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST;
        }
        rv = DRV_PORT_BLOCK_GET(
                unit, ingress_port, DRV_BLOCK_DLF_IP_MCAST, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }        
        if (BCM_PBMP_MEMBER(t_pbm, egress_port)){
            *flags |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST;
        }
        
        if ((*flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST) && 
                (*flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST)){
            *flags |= BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST;
            *flags &= ~(BCM_PORT_FLOOD_BLOCK_UNKNOWN_IP_MCAST | 
                    BCM_PORT_FLOOD_BLOCK_UNKNOWN_NONIP_MCAST);
        }
        
#else   /* BCM_TB_SUPPORT */
        BCM_UNLOCK(unit);
        return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
    } else {
        /* original design support BLOCK_ALL type only */
        rv = DRV_PORT_VLAN_GET(unit, ingress_port, &t_pbm);
        if (rv < 0){
            BCM_UNLOCK(unit);
            return rv;
        }    
        *flags = (!BCM_PBMP_MEMBER(t_pbm, egress_port)) ? 
                BCM_PORT_FLOOD_BLOCK_ALL : 0;
    }
    BCM_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_rate_egress_set
 * Purpose:
 *  Set egress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - Rate in kilobits (1000 bits) per second.
 *          Rate of 0 disables rate limiting.
 *  kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  1. Robo Switch support 2 ingress buckets for different packet type.
 *     And the bucket1 contains higher priority if PKT_MSK confilict 
 *       with bucket0's PKT_MSK.
 *  2. Robo Switch allowed system basis rate/packet type assignment for 
 *     Rate Control. The RATE_TYPE and PKT_MSK will be set once in the 
 *       initial routine.
 */
int bcm_robo_port_rate_egress_set(int unit,
                 bcm_port_t port,
                 uint32 kbits_sec,
                 uint32 kbits_burst)
{
    pbmp_t      t_pbm;
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i;
    int queue = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
        for (i=0; i<SOC_PBMP_WORD_MAX; i++){
            SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
        }

        BCM_PBMP_AND(t_pbm, valid_bmp);
    }
    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_SET
                        (unit, t_pbm, queue,
                        DRV_RATE_CONTROL_DIRECTION_EGRESSS,
                        0, 0, kbits_sec, kbits_burst));

    return BCM_E_NONE;

}

/*
 * Function:
 *  bcm_robo_port_rate_egress_get
 * Purpose:
 *  Get egress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *              zero if rate limiting is disabled.
 *  kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 */
int bcm_robo_port_rate_egress_get(int unit,
                 bcm_port_t port,
                 uint32 *kbits_sec,
                 uint32 *kbits_burst)
{
    int queue = 0;
    uint32 flags = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    }

    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_GET
                        (unit, port, queue,
                        DRV_RATE_CONTROL_DIRECTION_EGRESSS,
                        &flags, NULL, kbits_sec, kbits_burst));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_rate_egress_pps_set
 * Purpose:
 *      Set egress rate limiting parameter in packet mode.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      pps -  Rate in pps
 *             Rate of 0 disables rate limiting.
 *      burst - Maximum burst size
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_port_rate_egress_pps_set(int unit,
                             bcm_port_t port,
                             uint32 pps,
                             uint32 burst)
{
    pbmp_t      t_pbm;
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX], flags = 0;
    int i;
    int queue = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_pps_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
        for (i=0; i<SOC_PBMP_WORD_MAX; i++){
            SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
        }

        BCM_PBMP_AND(t_pbm, valid_bmp);
    }

    if (SOC_IS_POLAR(unit)||SOC_IS_NORTHSTAR(unit)|| \
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        flags = DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED;
    } else if (SOC_IS_TBX(unit)) {
        /* type = 1: pps; 0: kbps */
        BCM_IF_ERROR_RETURN(bcm_cosq_control_set(unit, port, -1, \
                            bcmCosqControlEgressRateType, 1));
    } else {
        return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(DRV_RATE_SET
                        (unit, t_pbm, queue,
                        DRV_RATE_CONTROL_DIRECTION_EGRESSS,
                        flags, 0, pps, burst));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_rate_egress_pps_get
 * Purpose:
 *      Get egress rate limiting parameter in packet mode.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      pps -  (OUT) Rate in pps
 *                   0 if rate limiting is disabled
 *      burst -(OUT) burst size
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_port_rate_egress_pps_get(int unit,
                             bcm_port_t port,
                             uint32 *pps,
                             uint32 *burst)
{
    int queue = 0;
    uint32 flags = 0;
    int rate_type = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_pps_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    }

    if (SOC_IS_POLAR(unit)||SOC_IS_NORTHSTAR(unit)|| \
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        BCM_IF_ERROR_RETURN(DRV_RATE_GET
                            (unit, port, queue,
                             DRV_RATE_CONTROL_DIRECTION_EGRESSS,
                             &flags, NULL, pps, burst));
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            if (!(flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED)) {
                /* Packets-based mode was not enabled */
                *pps = 0;
                *burst = 0;
            }
        }
    } else if (SOC_IS_TBX(unit)) {
        /* type = 1: pps; 0: kbps */
        BCM_IF_ERROR_RETURN(bcm_cosq_control_get(unit, port, -1, \
                            bcmCosqControlEgressRateType, &rate_type));
        if (rate_type) {
            BCM_IF_ERROR_RETURN(DRV_RATE_GET
                            (unit, port, queue,
                             DRV_RATE_CONTROL_DIRECTION_EGRESSS,
                             &flags, NULL, pps, burst));
        } else {
            /* Packets-based mode was not enabled */
            *pps = 0;
            *burst = 0;
        }
    } else {
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_rate_egress_traffic_set
 * Purpose:
 *      Set egress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      traffic_types - egress traffic type
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *          Rate of 0 disables rate limiting.
 *      kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 * Note :
 *  1. Robo Switch support 2 ingress buckets for different packet type.
 *     And the bucket1 contains higher priority if PKT_MSK confilict 
 *       with bucket0's PKT_MSK.
 *  2. Robo Switch allowed system basis rate/packet type assignment for 
 *     Rate Control. The RATE_TYPE and PKT_MSK will be set once in the 
 *       initial routine.
 */
int bcm_robo_port_rate_egress_traffic_set(int unit, 
                 bcm_port_t port, 
                 uint32 traffic_types, 
                 uint32 kbits_sec, 
                 uint32 kbits_burst)
{
    bcm_pbmp_t  t_pbm;
    bcm_pbmp_t  valid_bmp;
    uint32  value[SOC_PBMP_WORD_MAX];
    int  i;
    int  queue = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_set()..\n")));

    /* Only support BCM_PORT_RATE_TRAFFIC_ALL for ROBO */
    if (traffic_types != BCM_PORT_RATE_TRAFFIC_ALL) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
        for (i = 0; i < SOC_PBMP_WORD_MAX; i++) {
            SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
        }

        BCM_PBMP_AND(t_pbm, valid_bmp);
    }
    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_SET
        (unit, t_pbm, queue, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 0, 
        0, kbits_sec, kbits_burst));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_rate_egress_traffic_get
 * Purpose:
 *      Get egress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      traffic_types - egress traffic type
 *      kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *              zero if rate limiting is disabled.
 *      kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_port_rate_egress_traffic_get(int unit,
                 bcm_port_t port, 
                 uint32 *traffic_types, 
                 uint32 *kbits_sec, 
                 uint32 *kbits_burst)
{
    int  queue = 0;
    uint32 flags = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_egress_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    }

    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_GET
        (unit, port, queue, DRV_RATE_CONTROL_DIRECTION_EGRESSS, &flags, NULL, 
        kbits_sec, kbits_burst));

    /* Only support BCM_PORT_RATE_TRAFFIC_ALL for ROBO */
    *traffic_types = BCM_PORT_RATE_TRAFFIC_ALL;

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_rate_ingress_set
 * Purpose:
 *  Set ingress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - Rate in kilobits (1000 bits) per second.
 *            Rate of 0 disables rate limiting.
 *  kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  1. Robo Switch support 2 ingress buckets for different packet type.
 *     And the bucket1 contains higher priority if PKT_MSK confilict 
 *       with bucket0's PKT_MSK.
 *  2. Robo Switch allowed system basis rate/packet type assignment for 
 *     Rate Control. The RATE_TYPE and PKT_MSK will be set once in the 
 *       initial routine.
 */
int bcm_robo_port_rate_ingress_set(int unit,
                  bcm_port_t port,
                  uint32 kbits_sec,
                  uint32 kbits_burst)
{
    pbmp_t      t_pbm;
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i, queue = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_ingress_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
               (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
        for (i=0; i < SOC_PBMP_WORD_MAX; i++){
            SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
        }
        BCM_PBMP_AND(t_pbm, valid_bmp);
    }

    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_SET
                        (unit, t_pbm, queue,
                        DRV_RATE_CONTROL_DIRECTION_INGRESSS,
                        0, 0, kbits_sec, kbits_burst));

    return BCM_E_NONE;

}

/*
 * Function:
 *  bcm_robo_port_rate_ingress_get
 * Purpose:
 *  Get ingress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *                  zero if rate limiting is disabled.
 *  kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 */
int bcm_robo_port_rate_ingress_get(int unit,
                  bcm_port_t port,
                  uint32 *kbits_sec,
                  uint32 *kbits_burst)
{
    int queue = 0;
    uint32  flags = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_ingress_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    }

    /* The 3rd parameter in this BCM API is for setting the rate with 
     *    kbit/sec unit. But for the resolution in Robo register setting 
     *    is not the same with this unit(kbit/sec), we might get different 
     *    setting from register when compare with the original user's 
     *    setting value.
     */   
    BCM_IF_ERROR_RETURN(DRV_RATE_GET
                        (unit, port, queue,
                        DRV_RATE_CONTROL_DIRECTION_INGRESSS,
                        &flags, NULL, kbits_sec, kbits_burst));

    return BCM_E_NONE;
}
                     

/*
 * Function:
 *  bcm_robo_port_rate_pause_set
 * Purpose:
 *  Set ingress rate limiting pause frame control parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_pause - Pause threshold in kbits (1000 bits).
 *      A value of zero disables the pause/resume mechanism.
 *  kbits_resume - Resume threshold in kbits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
int bcm_robo_port_rate_pause_set(int unit,
                bcm_port_t port,
                uint32 kbits_pause,
                uint32 kbits_resume)
{

    pbmp_t      t_pbm;
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_pause_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
               (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
        for (i=0; i < SOC_PBMP_WORD_MAX; i++){
            SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
        }
        BCM_PBMP_AND(t_pbm, valid_bmp);
    }

    /* Pause on threshold */
    BCM_IF_ERROR_RETURN(
        DRV_RATE_CONFIG_SET(unit, t_pbm, 
            DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON, kbits_pause));

    /* Pause off threshold */
    BCM_IF_ERROR_RETURN(
        DRV_RATE_CONFIG_SET(unit, t_pbm, 
            DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF, kbits_resume));

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_rate_pause_get
 * Purpose:
 *  Get ingress rate limiting pause frame control parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_pause - (OUT) Pause threshold in kbits (1000 bits).
 *      Zero indicates the pause/resume mechanism is disabled.
 *  kbits_resume - (OUT) Resume threshold in kbits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 */
int bcm_robo_port_rate_pause_get(int unit,
                bcm_port_t port,
                uint32 *kbits_pause,
                uint32 *kbits_resume)
{

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_rate_pause_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    if (!IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    }

    /* Pause on threshold */
    BCM_IF_ERROR_RETURN(
        DRV_RATE_CONFIG_GET(unit, port, 
            DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON, kbits_pause));

    /* Pause off threshold */
    BCM_IF_ERROR_RETURN(
        DRV_RATE_CONFIG_GET(unit, port, 
            DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF, kbits_resume));

    return BCM_E_NONE;
}
                   

/* Double tagging */

/*
 * Function:
 *  bcm_robo_port_dtag_mode_set
 * Description:
 *  Set the double-tagging mode of a port.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - Double-tagging mode, one of:
 *      BCM_PORT_DTAG_MODE_NONE     No double tagging
 *      BCM_PORT_DTAG_MODE_INTERNAL Service provider port
 *      BCM_PORT_DTAG_MODE_EXTERNAL Customer port
 * Return Value:
 *  BCM_E_XXX
 */
int
bcm_robo_port_dtag_mode_set(int unit, bcm_port_t port, int mode)
{
    pbmp_t  t_pbm, clear_pbm;
    int     pre_mode = 0;
    int is_dtag = 0;
    bcm_port_t  p = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : %s(port=%d,mode=%d)..\n"),
              FUNCTION_NAME(),port,mode));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(bcm_robo_port_dtag_mode_get(unit, port, &pre_mode));

    BCM_PBMP_CLEAR(clear_pbm); 
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    
    switch (mode) {
    case BCM_PORT_DTAG_MODE_NONE:
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, clear_pbm, 
                    DRV_PORT_PROP_DTAG_MODE, FALSE));
        is_dtag = 0;
        break;
        
    case BCM_PORT_DTAG_MODE_INTERNAL:
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, clear_pbm, 
                    DRV_PORT_PROP_DTAG_MODE, TRUE));
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                    DRV_PORT_PROP_DTAG_ISP_PORT, TRUE));
        is_dtag = 1;
        break;

    case BCM_PORT_DTAG_MODE_EXTERNAL:
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, clear_pbm, 
                    DRV_PORT_PROP_DTAG_MODE, TRUE));
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm,
                    DRV_PORT_PROP_DTAG_ISP_PORT, FALSE));
        is_dtag = 1;
        break;
       
    default: 
           return BCM_E_UNAVAIL; 
    }
    
    /* The dtag mode is global setting for ROBO */
    BCM_PBMP_ITER(PBMP_ALL(unit), p) {
        _bcm_robo_dtag_mode_sw_update(unit, p, is_dtag);
    }
    
    if(pre_mode != mode){
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
            if (IS_VT_CFP_INIT){
                /* rebuild cfp for those created VT entries */
                BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_isp_change(unit));
            }
            /* Egress VID remarking table rebuild is requirred while dtag mode 
             *  is changed. 
             */
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_vlan_vtevr_isp_change(unit, t_pbm));
        }
#endif  /* Vulacan/StarFighter/Polar/Northstar/Northstar-Plus */
        
#if QVLAN_UTBMP_BACKUP
        /* if mode setting is changed, we have to reprogram whole valid 1Q VLAN 
         * untag bitmap. 
         */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_utbmp_dt_rebuild(unit, mode));
#endif  /* QVLAN_UTBMP_BACKUP */
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_dtag_mode_get
 * Description:
 *  Return the current double-tagging mode of a port.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - (OUT) Double-tagging mode
 * Return Value:
 *  BCM_E_XXX
 */
int
bcm_robo_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{

    uint32  drv_value = 0, temp = 0;
    bcm_port_cfg_t  * pcfg;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_dtag_mode_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* 
      * If it is cpu port and the software copy is available, return it.
      * Otherwise, read from register.
      */
    if (robo_port_config[unit] && IS_CPU_PORT(unit, port)) {
        pcfg = &robo_port_config[unit][port];
        *mode = pcfg->pc_dt_mode;
    } else {
        BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                  DRV_PORT_PROP_DTAG_MODE, &drv_value));

        if (drv_value) {
            BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_DTAG_ISP_PORT, &temp));
            if (temp) {
                *mode = BCM_PORT_DTAG_MODE_INTERNAL;
            } else {
                *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
            }
        } else {
            *mode = BCM_PORT_DTAG_MODE_NONE;
        }
    }
    return BCM_E_NONE;

}

/*
 * Function:
 *  bcm_robo_port_tpid_set
 * Description:
 *  Set the default Tag Protocol ID for a port.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  tpid - Tag Protocol ID
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 *  This API is not specifically double-tagging-related, but
 *  the port TPID becomes the service provider TPID when double-tagging
 *  is enabled on a port.  The default TPID is 0x8100.
 *  On BCM5673, only 0x8100 is allowed for the inner (customer) tag.
 */
int
bcm_robo_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{

    pbmp_t      t_pbm;
    bcm_port_cfg_t  * pcfg;
    bcm_port_t  p = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_tpid_set()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    BCM_IF_ERROR_RETURN(DRV_PORT_SET(
            unit, t_pbm,DRV_PORT_PROP_DTAG_TPID, tpid));

    /* Update software copy */
    if (robo_port_config[unit]) {
        /* The dtag tpid is global setting for ROBO */
        BCM_PBMP_ITER(PBMP_ALL(unit), p) {
            pcfg = &robo_port_config[unit][p];
            pcfg->pc_dt_tpid = tpid;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_tpid_get
 * Description:
 *  Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  tpid - (OUT) Tag Protocol ID
 * Return Value:
 *  BCM_E_XXX
 */
int
bcm_robo_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    uint32 drv_val;
    bcm_port_cfg_t  * pcfg;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_tpid_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* 
      * If it is cpu port and the software copy is available, return it.
      * Otherwise, read from register.
      */
    if (robo_port_config[unit] && IS_CPU_PORT(unit, port)) {
        pcfg = &robo_port_config[unit][port];
        drv_val = pcfg->pc_dt_tpid;
    } else {
        BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                  DRV_PORT_PROP_DTAG_TPID, &drv_val));
    }

    *tpid = (uint16)drv_val;
    return BCM_E_NONE;
}
    
/*
 * Function:
 *      bcm_robo_port_tpid_add
 * Description:
 *      Add allowed TPID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_robo_port_tpid_add(int unit, bcm_port_t port, 
                uint16 tpid, int color_select)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_robo_port_tpid_delete
 * Description:
 *      Delete allowed TPID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_robo_port_tpid_delete(int unit, bcm_port_t port, uint16 tpid)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_robo_port_tpid_delete_all
 * Description:
 *      Delete all allowed TPID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 * Return Value:
 *      BCM_E_XXX
 */
int 
bcm_robo_port_tpid_delete_all(int unit, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *  bcm_port_inner_tpid_get
 * Purpose:
 *      Get the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  tpid - (OUT) Tag Protocol ID
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_XXX
 */
int
bcm_robo_port_inner_tpid_get(
    int unit,
    bcm_port_t  port,
    uint16 *    tpid)
{
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_inner_tpid_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    *tpid = 0x8100;
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_port_inner_tpid_set
 * Purpose:
 *      Set the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  tpid - Tag Protocol ID
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_XXX
 */
int
bcm_robo_port_inner_tpid_set(
    int unit,
    bcm_port_t  port,
    uint16  tpid)
{
    return BCM_E_UNAVAIL;
}
    
/*
 * Function:
 *  bcm_port_cable_diag
 * Description:
 *  Run Cable Diagnostics on port
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  status - (OUT) cable diag status structure
 * Return Value:
 *  BCM_E_XXX
 * Notes:
 *  Cable diagnostics are only supported by some phy types
 *    (currently 5248 10/100 phy and 546x 10/100/1000 phys)
 */
int
bcm_robo_port_cable_diag(
    int unit,
    bcm_port_t  port,
    bcm_port_cable_diag_t * status)
{

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_PHY_CABLE_DIAG, (uint32 *)status));

    return BCM_E_NONE;
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
 *  bcm_robo_port_info_get
 * Purpose:
 *  Get all information on the port
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 */

int
bcm_robo_port_info_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_info_get()..\n")));
    info->action_mask = BCM_PORT_ATTR_ALL_MASK;

    return bcm_robo_port_selective_get(unit, port, info);
}

/*
 * Function:
 *  bcm_robo_port_info_set
 * Purpose:
 *  Set all information on the port
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  Checks if AN is on, and if so, clears the
 *  proper bits in the action mask.
 */

int
bcm_robo_port_info_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_info_set()..\n")));
    info->action_mask = BCM_PORT_ATTR_ALL_MASK;

    /* If autoneg is set, remove those attributes controlled by it */
    if (info->autoneg){
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return bcm_robo_port_selective_set(unit, port, info);
}

/*
 * Function:
 *  bcm_robo_port_selective_get
 * Purpose:
 *  Get all available port parameters at once.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      info - (IN/OUT) Structure to pass the information (see port.h)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  None.
 */

int
bcm_robo_port_selective_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int         r = BCM_E_NONE;
    uint32      mask = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_selective_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENABLE_MASK){

        r = bcm_robo_port_enable_get(unit, port, &info->enable);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LINKSTAT_MASK){
        r = bcm_robo_port_link_status_get(unit, port, &info->linkstatus);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK){
        r = bcm_robo_port_autoneg_get(unit, port, &info->autoneg);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK){
        r = bcm_robo_port_advert_get(unit, port, &info->local_advert);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK){
        if ((r = bcm_robo_port_advert_remote_get(unit, port, 
            &info->remote_advert)) < 0) {
            info->remote_advert = 0;
            info->remote_advert_valid = FALSE;
        } else {
            info->remote_advert_valid = TRUE;
        }
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK){
        if ((r = bcm_robo_port_speed_get(unit, port, &info->speed)) < 0){
            if (r != BCM_E_BUSY){
                return(r);
            } else {
                info->speed = 0;
            }
        }
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK){
        if ((r = bcm_robo_port_duplex_get(unit, port, &info->duplex)) < 0){
            if (r != BCM_E_BUSY){
                return(r);
            } else {
                info->duplex = 0;
            }
        }
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MASK){
        r = bcm_robo_port_pause_get(
                unit, port, &info->pause_tx, &info->pause_rx);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_get failed: %s\n"), bcm_errmsg(r)));
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        r = bcm_robo_port_pause_addr_get(unit, port, info->pause_mac);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_addr_get failed: %s\n"), bcm_errmsg(r)));
        }
        if (r == BCM_E_UNAVAIL){
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }


    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK){
        r = bcm_robo_port_linkscan_get(unit, port, &info->linkscan);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK) {
        r = bcm_robo_port_learn_get(unit, port, &info->learn);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;  /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK){
        r = bcm_robo_port_discard_get(unit, port, &info->discard);
        /* The fix process on  GNATS 40551 on TB device may observed the 
         *  return code been BCM_E_FAIL but this failed condition normally 
         *  will be treated as BCM_E_NONE.
         */
        if (r == BCM_E_FAIL){
            r = BCM_E_NONE;
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK){
        r = bcm_robo_port_vlan_member_get(unit, port, &info->vlanfilter);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK){
        r = bcm_robo_port_untagged_priority_get(unit, port,
                                &info->untagged_priority);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK){
        r = bcm_robo_port_untagged_vlan_get(unit, port, &info->untagged_vlan);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK){
        r = bcm_robo_port_stp_get(unit, port, &info->stp_state);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        r = bcm_robo_port_pfm_get(unit, port, &info->pfm);
        if (r != BCM_E_UNAVAIL) {
            if (SOC_FAILURE(r)) {
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "bcm_port_pfm_get failed: %s\n"), bcm_errmsg(r)));
            }
        }
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK){
        r = bcm_robo_port_loopback_get(unit, port, &info->loopback);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK){
        r = bcm_robo_port_master_get(unit, port, &info->phy_master);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK){
        r = bcm_robo_port_interface_get(unit, port, &info->interface);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ENCAP_MASK){
        r = BCM_E_NONE;     /* for unsupported chip */
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        /* following routine is defined in rate.c file */
        r = bcm_robo_rate_mcast_get(unit, &info->mcast_limit,
                                &info->mcast_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
        /* following routine is defined in rate.c file */
        r = bcm_robo_rate_bcast_get(unit, &info->bcast_limit,
                                &info->bcast_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
        /* following routine is defined in rate.c file */
        r = bcm_robo_rate_dlfbc_get(unit, &info->dlfbc_limit,
                                    &info->dlfbc_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MAX_MASK){
        r = bcm_robo_port_speed_max(unit, port, &info->speed_max);

        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ABILITY){
        r = bcm_robo_port_ability_get(unit, port, &info->ability);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        r = bcm_robo_port_frame_max_get(unit, port, &info->frame_max);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_frame_max_get failed: %s\n"), bcm_errmsg(r)));
        }
        if (r == BCM_E_UNAVAIL){
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        r = bcm_robo_port_mdix_get(unit, port, &info->mdix);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_mdix_get failed: %s\n"), bcm_errmsg(r)));
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_STATUS_MASK) {
        r = bcm_robo_port_mdix_status_get(unit, port, &info->mdix_status);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_mdix_status_get failed: %s\n"), bcm_errmsg(r)));
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MEDIUM_MASK) {
        r = bcm_robo_port_medium_get(unit, port, &info->medium);
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_medium_get failed: %s\n"), bcm_errmsg(r)));
        }
        BCM_IF_ERROR_RETURN(r);
    }
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "\t bcm_robo_port_selective_get()..mask=0x%0x\n"),
              mask));

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_selective_set
 * Purpose:
 *  Set all available port parameters at once.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch port #.
 *      info - Structure to pass the information (see port.h)
 * Returns:
 *  BCM_E_NONE
 *  BCM_E_XXX
 * Notes:
 *  Does not set spanning tree state.
 */

int
bcm_robo_port_selective_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int         r = BCM_E_NONE;
    uint32      mask = 0;
    int         flags = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    mask = info->action_mask;
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_selective_set()..mask=0x%0x\n"),
              mask));

    if (mask & BCM_PORT_ATTR_ENABLE_MASK){
        r = bcm_robo_port_enable_set(unit, port, info->enable);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK){
        r = bcm_robo_port_pause_addr_set(unit, port, info->pause_mac);
        if (r == BCM_E_UNAVAIL){
            r = BCM_E_NONE;     /* for unsupported chip */
        }
        if (SOC_FAILURE(r)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "bcm_port_pause_addr_set failed: %s\n"), bcm_errmsg(r)));
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK){
        r = bcm_robo_port_interface_set(unit, port, info->interface);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK){
        r = bcm_robo_port_master_set(unit, port, info->phy_master);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK){
        r = bcm_robo_port_linkscan_set(unit, port, info->linkscan);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK){
        r = bcm_robo_port_learn_set(unit, port, info->learn);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;  /* for unsupported chip */
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK){
        r = bcm_robo_port_discard_set(unit, port, info->discard);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK){
        r = bcm_robo_port_vlan_member_set(unit, port, info->vlanfilter);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK){
        r = bcm_robo_port_untagged_priority_set(unit, port, 
                                info->untagged_priority);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK){
        r = bcm_robo_port_untagged_vlan_set(unit, port, info->untagged_vlan);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        r = bcm_robo_port_pfm_set(unit, port, info->pfm);
        if (r != BCM_E_UNAVAIL) {
            if (SOC_FAILURE(r)) {
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "bcm_port_pfm_set failed: %s\n"), bcm_errmsg(r)));
            }
        }
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(r);
    }

    /*
     * Set loopback mode before setting the speed/duplex, since it may
     * affect the allowable values for speed/duplex.
     */
    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK){
        r = bcm_robo_port_loopback_set(unit, port, info->loopback);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK){
        r = bcm_robo_port_advert_set(unit, port, info->local_advert);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK){
        r = bcm_robo_port_autoneg_set(unit, port, info->autoneg);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK){
        r = bcm_robo_port_speed_set(unit, port, info->speed);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK){
        r = bcm_robo_port_duplex_set(unit, port, info->duplex);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MASK){
        int tpause, rpause;
        
        tpause = rpause = -1;
        if (mask & BCM_PORT_ATTR_PAUSE_TX_MASK) {
            tpause = info->pause_tx;
        }
        if (mask & BCM_PORT_ATTR_PAUSE_RX_MASK) {
            rpause = info->pause_rx;
        }
        r = bcm_robo_port_pause_set(unit, port, tpause, rpause);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        r = BCM_E_NONE;     /* for unsupported chip */
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        /* following routine is defined in rate.c file */
        flags = (info->mcast_limit_enable) ? BCM_RATE_MCAST : 0;

        r = bcm_rate_mcast_set(unit, info->mcast_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;
        }
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK){
        /* following routine is defined in rate.c file */
        flags = (info->bcast_limit_enable) ? BCM_RATE_BCAST : 0;

        r = bcm_rate_bcast_set(unit, info->bcast_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;
        }
        
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK){
        /* following routine is defined in rate.c file */
        flags = (info->dlfbc_limit_enable) ? BCM_RATE_DLF : 0;

        r = bcm_rate_dlfbc_set(unit, info->dlfbc_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;
        }
        
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK){
        r = bcm_robo_port_stp_set(unit, port, info->stp_state);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        r = bcm_robo_port_frame_max_set(unit, port, info->frame_max);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        r = bcm_robo_port_mdix_set(unit, port, info->mdix);
        BCM_IF_ERROR_RETURN(r);
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *  bcm_robo_port_info_save
 * Purpose:
 *  Save the current settings of a port
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  info - Pointer to structure in which to save values
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  The action_mask will be adjusted so that the
 *  proper values will be set when a restore is made.
 *  This mask should not be altered between these calls.
 */

int
bcm_robo_port_info_save(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    info->action_mask = BCM_PORT_ATTR_ALL_MASK;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_info_save()..\n")));
    BCM_IF_ERROR_RETURN(bcm_robo_port_selective_get(unit, port, info));

    /* If autoneg is set, remove those attributes controlled by it */
    if (info->autoneg) {
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_port_info_restore
 * Purpose:
 *  Restore port settings saved by info_save
 * Parameters:
 *  unit - RoboSwitch unit #
 *  port - RoboSwitch port #
 *  info - Pointer to structure with info from port_info_save
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  bcm_port_info_save has done all the work.
 *  We just call port_selective_set.
 */

int
bcm_robo_port_info_restore(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_info_restore()..\n")));
    return bcm_port_selective_set(unit, port, info);
}
                              
int 
bcm_robo_port_sample_rate_get(int unit, bcm_port_t port,int *ingress_rate,int *egress_rate)
{
    int rv = BCM_E_NONE;
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_SFLOW_INGRESS_RATE,(uint32 *) ingress_rate));
    BCM_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_SFLOW_EGRESS_RATE,(uint32 *) egress_rate));

    return rv;
}

int 
bcm_robo_port_sample_rate_set(int unit,bcm_port_t port,int ingress_rate, int egress_rate)
{
    int rv = BCM_E_NONE;
    pbmp_t  pbm;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_ADD(pbm, port);
    /* 
     * Robo chips that support sFlow feature are BCM5348/5347/53242/53262.
     * According to the above chips' ability of sFlow,
     * the meaning of the parameters of the API are,
     * For ingress_rate,
     * ingress_rate = 0, disable the port's sFlow feature.
     * ingress_rate != 0, change the sample rate of all enabled ports.
     *
     * For egress_rate, only one port can do egress sFlow at a time.
     * egress_rate = 0, disable the port's sFlow feature.
     * egress_rate != 0, change the sample rate of the port.
     * 
     */
    BCM_IF_ERROR_RETURN(DRV_PORT_SET(
            unit, pbm, DRV_PORT_PROP_SFLOW_INGRESS_RATE, ingress_rate));
    BCM_IF_ERROR_RETURN(DRV_PORT_SET(
            unit, pbm, DRV_PORT_PROP_SFLOW_EGRESS_RATE, egress_rate));

    return rv;
}

int
bcm_robo_port_fault_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_port_vlan_member_get(int unit, bcm_port_t port, uint32 *flags)
{
    int     rv = BCM_E_NONE;
    uint32      temp = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_vlan_member_get()..\n")));
    
    *flags = 0;
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    /* For Robo chips, this is global setting (not per-port) */ 
    rv = DRV_PORT_GET(unit, port, DRV_PORT_PROP_INGRESS_VLAN_CHK, &temp);
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_INGRESS_VLAN_CHK failed: %s\n"), 
                     bcm_errmsg(rv)));
    }
    if (temp) {
        *flags |= BCM_PORT_VLAN_MEMBER_INGRESS;
    }

    return rv;
}
int
bcm_robo_port_vlan_member_set(int unit, bcm_port_t port, uint32 flags)
{
    int     rv = BCM_E_NONE;
    pbmp_t  pbm;
    uint32  temp = 0;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_vlan_member_set()..\n")));
    
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);

    /* For Robo chips, this is global setting (not per-port) */ 

    /* There is no support for VLAN egress filtering for ROBO. */
    if (flags & BCM_PORT_VLAN_MEMBER_EGRESS) {
        return BCM_E_UNAVAIL;
    }

    if (flags & BCM_PORT_VLAN_MEMBER_INGRESS) {
        temp  = 1;
    } else {
        temp = 0;
    }
    rv = DRV_PORT_SET(unit, pbm,DRV_PORT_PROP_INGRESS_VLAN_CHK, temp);
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "DRV_PORT_PROP_INGRESS_VLAN_CHK failed: %s\n"), 
                     bcm_errmsg(rv)));
    }

    return rv;
}

int
bcm_robo_port_trunk_index_get(int unit, bcm_port_t port, int *port_index)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_port_trunk_index_set(int unit, bcm_port_t port, int port_index)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_cfi_color_set
 * Purpose:
 *      Specify the color selection for the given CFI.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      cfi -   Canonical format indicator (TRUE/FALSE) 
 *      color - color assigned to packets with indicated CFI.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on Thunderbolt, NS+, SF3 only for ROBO.
 */
int
bcm_robo_port_cfi_color_set(int unit, bcm_port_t port,
                           int cfi, bcm_color_t color)
{
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        int  pkt_pri;
        uint32  internal_pri;
        bcm_color_t  t_color;

        if (cfi < 0 || cfi > 1) {
            return BCM_E_PARAM;
        }

        /* Set the color field by index port and cfi with all pkt_pri (0 ~ 7) 
          * on PCP2TCDP mapping talbe.
          */
        for (pkt_pri = 0; pkt_pri <= 7; pkt_pri++) {
            /* Get the original internal_pri value */
            BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                    (unit, port, DRV_PORT_OP_PCP2TC, 
                    pkt_pri, cfi, &internal_pri, (void *)&t_color));

            /* Set the new color field for PCP2TCDP mapping table */
            BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
                    (unit, port, DRV_PORT_OP_PCP2TC, 
                    pkt_pri, cfi, internal_pri, _BCM_COLOR_ENCODING(unit, color)));
        }

        return BCM_E_NONE;
    }
#endif /* TB || NORTHSTARPLUS || SF3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_cfi_color_get
 * Purpose:
 *      Get the color selection for the given CFI.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      cfi -   Canonical format indicator (TRUE/FALSE) 
 *      color - (OUT) color assigned to packets with indicated CFI.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on Thunderbolt, NS+, SF3 only for ROBO.
 */
int
bcm_robo_port_cfi_color_get(int unit, bcm_port_t port,
                           int cfi, bcm_color_t *color)
{
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        int  pkt_pri;
        uint32  internal_pri;

        if (cfi < 0 || cfi > 1) {
            return BCM_E_PARAM;
        }

        /* Get the color field by index port and cfi with pkt_pri = 0
          * on PCP2TCDP mapping talbe.
          */
        pkt_pri = 0;
        /* Get the color field value from PCP2TCDP mapping table */
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                (unit, port, DRV_PORT_OP_PCP2TC, 
                pkt_pri, cfi, &internal_pri, (uint32 *)color));

        *color = _BCM_COLOR_DECODING(unit, *color);

        return BCM_E_NONE;
    }
#endif /* TB || NORTHSTARPLUS || SF3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_priority_color_set
 * Purpose:
 *      Specify the color selection for the given priority.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      prio -  priority (aka 802.1p CoS)
 *      color - color assigned to packets with indicated priority.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on Thunderbolt, NS+, SF3 only for ROBO.
 */
int
bcm_robo_port_priority_color_set(int unit, bcm_port_t port,
                                int prio, bcm_color_t color)
{
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        int  cfi;
        uint32  internal_pri;
        bcm_color_t  t_color;

        _ROBO_VLAN_CHK_PRIO(unit, prio);

        /* Set the color field by index port and priority with all cfi (0 or 1) 
          * on PCP2TCDP mapping talbe.
          */
        for (cfi = 0; cfi <= 1; cfi++) {
            /* Get the original internal_pri value */
            BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                    (unit, port, DRV_PORT_OP_PCP2TC, 
                    prio, cfi, &internal_pri, (void *)&t_color));

            /* Set the new color field for PCP2TCDP mapping table */
            BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
                    (unit, port, DRV_PORT_OP_PCP2TC, 
                    prio, cfi, internal_pri, _BCM_COLOR_ENCODING(unit, color)));
        }

        return BCM_E_NONE;
    }
#endif /* TB || NORTHSTARPLUS || SF3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_priority_color_get
 * Purpose:
 *      Get the color selection for the given priority.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      prio -  priority (aka 802.1p CoS)
 *      color - (OUT) color assigned to packets with indicated priority.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on Thunderbolt, NS+, SF3 only for ROBO.
 */
int
bcm_robo_port_priority_color_get(int unit, bcm_port_t port,
                                int prio, bcm_color_t *color)
{
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        int  cfi;
        uint32  internal_pri;

        _ROBO_VLAN_CHK_PRIO(unit, prio);

        /* Get the color field by index port and priority with cfi = 0
          * on PCP2TCDP mapping talbe.
          */
        cfi = 0;
        /* Get the color field value from PCP2TCDP mapping table */
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                (unit, port, DRV_PORT_OP_PCP2TC, 
                prio, cfi, &internal_pri, (uint32 *)color));

        *color = _BCM_COLOR_DECODING(unit, *color);

        return BCM_E_NONE;
    }
#endif /* TB || NORTHSTARPLUS || SF3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_map_set
 * Description:
 *      Define the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *  Since bcm_port_vlan_priority_mapping_set() is the API to replace this API, 
 *  here we redesign this API to call to new API.
 */

int
bcm_robo_port_vlan_priority_map_set(int unit, bcm_port_t port, int pkt_pri,
                        int cfi, int internal_pri, bcm_color_t color)
{
    bcm_priority_mapping_t pri_map;

    pri_map.internal_pri = internal_pri;
    pri_map.color = color;

    return bcm_robo_port_vlan_priority_mapping_set(unit, 
            port, 0, pkt_pri, cfi, &pri_map);

}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_map_get
 * Description:
 *      Get the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      internal_pri - (OUT) Internal priority
 *      color        - (OUT) color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *  Since bcm_port_vlan_priority_mapping_set() is the API to replace this API, 
 *  here we redesign this API to call to new API.
 */

int
bcm_robo_port_vlan_priority_map_get(int unit, bcm_port_t port, int pkt_pri,
                       int cfi, int *internal_pri, bcm_color_t *color)
{
    bcm_priority_mapping_t pri_map;

    BCM_IF_ERROR_RETURN(bcm_robo_port_vlan_priority_mapping_get(unit, 
            port, 0, pkt_pri, cfi, &pri_map));

    *internal_pri = pri_map.internal_pri;
    *color = pri_map.color;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_mapping_set
 * Description:
 *      Define the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      vid          - (IN) Vlan id (unused for ROBO)
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      pri_map      - (IN) Internal priority and color or drop precedence
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      1. This API programs only the mapping table. 
 *      2. In Robo chip, the pri_map->color and cfi parameters are supported for TB, NS+, SF3 only
 *          so far (not supported for bcm53115/bcm53118).
 *      3. for no pri_map->color feature in ROBO chips, the feature define is not 
 *          applied here(soc_feature_color_prio_map).
 */
int
bcm_robo_port_vlan_priority_mapping_set(int unit, bcm_port_t port, 
    bcm_vlan_t vid, int pkt_pri, int cfi, bcm_priority_mapping_t *pri_map)
{
    bcm_pbmp_t  t_pbm;
    uint32  val;
    
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    if (pkt_pri == -1 && cfi == -1) {
        /* For the untagged packets */
        BCM_PBMP_CLEAR(t_pbm); 
        BCM_PBMP_PORT_ADD(t_pbm, port);
        val = pri_map->internal_pri;
        BCM_IF_ERROR_RETURN(DRV_PORT_SET(
            unit, t_pbm, DRV_PORT_PROP_UNTAG_DEFAULT_TC, val));
        if (SOC_IS_TBX(unit)) {
            BCM_IF_ERROR_RETURN(DRV_PORT_SET(
                unit, t_pbm, DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE, 
                _BCM_COLOR_ENCODING(unit, pri_map->color)));
        }
        return BCM_E_NONE;
    }
    
    if ((pkt_pri > 7) || (cfi > 1) || (pkt_pri < 0) || (cfi < 0)) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
            (unit, port, DRV_PORT_OP_PCP2TC, pkt_pri, cfi, 
            pri_map->internal_pri, _BCM_COLOR_ENCODING(unit, pri_map->color)));
#endif /* TB || NORTHSTARPLUS || SF3 */
    } else {
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
            (unit, port, DRV_PORT_OP_PCP2TC, pkt_pri, 0, 
            pri_map->internal_pri, 0));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_mapping_get
 * Description:
 *      Get the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      vid          - (IN) Vlan id (unused for ROBO)
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      pri_map      - (OUT) Internal priority and color or drop precedence
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      1. This API programs only the mapping table. 
 *      2. In Robo chip, the pri_map->color and cfi parameters are supported for TB, NS+, SF3 only
 *          so far (not supported for bcm53115/bcm53118).
 */
int
bcm_robo_port_vlan_priority_mapping_get(int unit, bcm_port_t port, 
    bcm_vlan_t vid, int pkt_pri, int cfi, bcm_priority_mapping_t *pri_map)
{
    uint32  val_pri = 0, val_color = 0;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    if (pkt_pri == -1 && cfi == -1) {
        /* For the untagged packets */
        BCM_IF_ERROR_RETURN(DRV_PORT_GET(
            unit, port, DRV_PORT_PROP_UNTAG_DEFAULT_TC, &val_pri));
        pri_map->internal_pri = val_pri;
        if (SOC_IS_TBX(unit)) {
            BCM_IF_ERROR_RETURN(DRV_PORT_GET(
                unit, port, DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE, &val_color));
            pri_map->color = _BCM_COLOR_DECODING(unit, val_color);
        }
        return BCM_E_NONE;
    }

    if ((pkt_pri > 7) || (cfi > 1) || (pkt_pri < 0) || (cfi < 0)) {
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, u=%d p=%d pkt_pri=%d\n"), FUNCTION_NAME(), unit, port, pkt_pri));

    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        /* currently, the cfi and color are used for TB, NS+, SF3 only on ROBO chip for PCP2TC */
        /* PCP2TCDP mapping table */
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
            (unit, port, DRV_PORT_OP_PCP2TC, pkt_pri, cfi, 
            &val_pri, &val_color));

        pri_map->internal_pri = val_pri;
        pri_map->color = _BCM_COLOR_DECODING(unit, val_color);
#endif /* TB || NORTHSTARPLUS || SF3 */
    } else {
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
            (unit, port, DRV_PORT_OP_PCP2TC, pkt_pri, 0, 
            &val_pri, NULL));

        pri_map->internal_pri = val_pri;    
        pri_map->color = _BCM_ROBO_COLOR_NONE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_unmap_set
 * Description:
 *      Define the mapping of outgoing port, internal priority, and color to
 *  outgoing packet priority and cfi bit.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) Color
 *      pkt_pri      - (IN) Packet priority
 *                         BCM_PRIO_STAG can be or'ed into the packet priority
 *                         BCM_PRIO_CTAG(default) can be or'ed into the packet priority
 *      cfi          - (IN) Packet CFI
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *  1. in ESW device, API return UNAVAIL if no "soc_feature_color_prio_map"
 *      feature designed.
 *  2. In Robo device, internal priority can be mapped to out-going packet
 *      no matter the feature "soc_feature_color_prio_map" is designed or not.
 *  3. For TB, BCM_PRIO_STAG and BCM_PRIO_CTAG(default)
 *      can be or'ed into the packet priority(pkt_pri) to indicate the PCP/DEI value 
 *      are used for S-Tag or C-Tag(default) in TCDP2PCP mapping table.
 */
int
bcm_robo_port_vlan_priority_unmap_set(int unit, bcm_port_t port, 
                        int internal_pri, bcm_color_t color, 
                        int pkt_pri, int cfi)
{
    uint32 color_op = 0;
#ifdef BCM_TB_SUPPORT
    uint32 dp;
#endif    
    
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));
    
    if (!_BCM_COSQ_PRIO_VALID(unit, internal_pri)) {
        return (BCM_E_PARAM);
    }

    if ((color != bcmColorGreen) && (color != bcmColorYellow) &&
             (color != bcmColorRed) && (color != bcmColorPreserve)) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        if (pkt_pri & BCM_PRIO_STAG) {
            color_op = DRV_PORT_OP_OUTBAND_TC2PCP;
            pkt_pri &= ~BCM_PRIO_STAG;
        } else if (pkt_pri & BCM_PRIO_CTAG) {
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
            pkt_pri &= ~BCM_PRIO_CTAG;
        } else {
            /* default as C-Tag */
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
        }
        dp = _BCM_COLOR_ENCODING(unit, color);

        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
                (unit, port, color_op, internal_pri, dp, 
                pkt_pri, cfi));
#endif
    } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (pkt_pri & BCM_PRIO_STAG) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
            pkt_pri &= ~BCM_PRIO_STAG;
        } else if (pkt_pri & BCM_PRIO_CTAG) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2CPCP : DRV_PORT_OP_NORMAL_TC2CPCP);
            pkt_pri &= ~BCM_PRIO_CTAG;
        } else {
            /* default as S-Tag (for out most tag) */
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
        }

        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
                (unit, port, color_op, internal_pri, 0, 
                pkt_pri, cfi));
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    } else {    
        if (soc_feature(unit, soc_feature_color_prio_map)) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
        } else {
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
        }
    
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET
                (unit, port, color_op, internal_pri, 0, 
                pkt_pri, cfi));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_vlan_priority_unmap_get
 * Description:
 *      Get the mapping of outgoing port, internal priority, and color to
 *  outgoing packet priority and cfi bit.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) Color
 *      pkt_pri      - (OUT) Packet priority
 *                         May have BCM_PRIO_STAG or'ed into it
 *                         May have BCM_PRIO_CTAG(default) or'ed into it
 *      cfi          - (OUT) Packet CFI
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *  1. This API programs only the mapping table. 
 *  2. For TB, BCM_PRIO_STAG and BCM_PRIO_CTAG(default)
 *      can be or'ed into the packet priority(pkt_pri) to get the PCP/DEI value 
 *      are used for S-Tag or C-Tag(default) in TCDP2PCP mapping table.
 */
int
bcm_robo_port_vlan_priority_unmap_get(int unit, bcm_port_t port, 
                         int internal_pri, bcm_color_t color,
                         int *pkt_pri, int *cfi)
{
    uint32 color_op = 0;
#ifdef BCM_TB_SUPPORT
    uint32 dp;
#endif    
    
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    if (!_BCM_COSQ_PRIO_VALID(unit, internal_pri)) {
        return (BCM_E_PARAM);
    }

    if ((color != bcmColorGreen) && (color != bcmColorYellow) &&
             (color != bcmColorRed) && (color != bcmColorPreserve) ) {
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, u=%d p=%d internal_pri=%d color=%d\n"),
              FUNCTION_NAME(), unit, port, internal_pri, color));

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        if (*pkt_pri & BCM_PRIO_STAG) {
            color_op = DRV_PORT_OP_OUTBAND_TC2PCP;
        } else if (*pkt_pri & BCM_PRIO_CTAG) {
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
        } else {
            /* default as C-Tag */
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
        }

        /* currently, the color is used for TB only on ROBO chip for PCP2TC */
        dp = _BCM_COLOR_ENCODING(unit, color);

        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                (unit, port, color_op, internal_pri, dp,
                (uint32 *) pkt_pri, (uint32 *) cfi));
#endif
    } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (*pkt_pri & BCM_PRIO_STAG) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
        } else if (*pkt_pri & BCM_PRIO_CTAG) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2CPCP : DRV_PORT_OP_NORMAL_TC2CPCP);
        } else {
            /* default as S-Tag (for out most tag) */
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
        }

        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                (unit, port, color_op, internal_pri, 0,
                (uint32 *) pkt_pri, (uint32 *) cfi));
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    } else {    
        if (soc_feature(unit, soc_feature_color_prio_map)) {
            color_op = (_BCM_COLOR_ENCODING(unit, color) ? 
                    DRV_PORT_OP_OUTBAND_TC2PCP : DRV_PORT_OP_NORMAL_TC2PCP);
        } else {
            color_op = DRV_PORT_OP_NORMAL_TC2PCP;
        }
    
        /* currently, the color is used for TB only on ROBO chip for PCP2TC */
        BCM_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_GET
                (unit, port, color_op, internal_pri, 0,
                (uint32 *) pkt_pri, (uint32 *) cfi));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_l3_modify_set
 * Description:
 *      Enable/Disable ingress port based L3 unicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - bitmap of the packet operations to be enabled or disabled.
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_robo_port_l3_modify_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_l3_modify_get
 * Description:
 *      Get ingress port based L3 unicast packet operations status.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - (OUT) pointer to uint32 where bitmap of the current L3 packet 
 *              operations is returned.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_robo_port_l3_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_ipmc_modify_set
 * Description:
 *      Enable/Disable ingress port based L3 multicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - bitmap of the packet operations to be enabled or disabled.
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_robo_port_ipmc_modify_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_ipmc_modify_get
 * Description:
 *      Enable/Disable ingress port based L3 multicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - (OUT) pointer to uint32 where bitmap of the current L3 packet
 *              operations is returned.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_robo_port_ipmc_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_control_set
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
bcm_robo_port_control_set(int unit, bcm_port_t port, 
                         bcm_port_control_t type, int value)
{
    pbmp_t  t_pbm;
    int rv = BCM_E_NONE;
    uint32 val = 0, flags = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_control_set()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    switch(type) {
        case bcmPortControlIngressTcSelectTable:
            /* value to represent the TC map modes on all frame conditions */
            rv = DRV_PORT_SET(unit, 
                    t_pbm, DRV_PORT_PROP_RAW_TC_MAP_MODE_SELECT, 
                    (uint32)value);
            break;
            
        case bcmPortControlFloodPktDropTc:
            /* value is a TC bitmap */
            rv = (DRV_QUEUE_QOS_CONTROL_SET
                    (unit, port, DRV_QOS_CTL_FLOOD_DROP_TCMAP, (uint32)value));
            break;
            
        case bcmPortControlEgressModifyOuterPktPri:
            rv = (DRV_PORT_SET
                    (unit, t_pbm, DRV_PORT_PROP_EGRESS_S_PCP_REMARK, 
                    (value ? 1 : 0)));
            break;
            
        case bcmPortControlEgressModifyInnerPktPri:
            rv = (DRV_PORT_SET
                    (unit, t_pbm, DRV_PORT_PROP_EGRESS_C_PCP_REMARK, 
                    (value ? 1 : 0)));
            break;
            
        case bcmPortControlPreservePacketPriority:
            rv = (DRV_QUEUE_MAPPING_TYPE_SET
                         (unit, t_pbm, 
                         DRV_QUEUE_MAP_PRIO, (uint8)value));
            break;
            
        case bcmPortControlEgressVlanPriUsesPktPri:

            /* this type value definition is :
             *  1: use incomming packet priority to be outgoing pri.
             *  0: use internal packet priority(TC) to obtain outgoing pri.
             */
            rv = (DRV_PORT_SET
                         (unit, t_pbm, DRV_PORT_PROP_EGRESS_PCP_REMARK, 
                         (value ? 0 : 1)));
            break;
            
        case bcmPortControlEgressVlanCfiUsesPktPri:
            /* this type value definition is :
             *  1: the outgoing CFI is the same as incomming packet CFI.
             *  0: use internal packet priority(TC) to obtain outgoing CFI.
             */
            rv = (DRV_PORT_SET
                         (unit, t_pbm, DRV_PORT_PROP_EGRESS_CFI_REMARK, 
                         (value ? 0 : 1)));
            break;
        case bcmPortControlIngressRateControlDrop:
            if (!(SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)||
                SOC_IS_TBX(unit))) {
                /* Other chips are per-system configuration*/
                BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));
            }
            rv = DRV_RATE_CONFIG_SET(unit, t_pbm, 
                    DRV_RATE_CONFIG_DROP_ENABLE, (value ? 1 : 0));
            break;

        case bcmPortControlEgressModifyDscp:
            /* this type value definition is :
             *  0: use incomming packet DSCP value to be outgoing DSCP.
             *  1: use internal TCDP to DSCP mapping value to obtain outgoing DSCP.
             */
            rv = (DRV_PORT_SET
                         (unit, t_pbm, 
                         DRV_PORT_PROP_EGRESS_DSCP_REMARK, (value ? 1 : 0)));
            break;
        case bcmPortControlEgressModifyECN:
            /* this type value definition is :
             *  0: use incomming packet ECN value to be outgoing ECN.
             *  1: use internal TCDP to DSCP-ECN mapping value to obtain outgoing ECN.
             */
            rv = (DRV_PORT_SET
                         (unit, t_pbm, 
                         DRV_PORT_PROP_EGRESS_ECN_REMARK, (value ? 1 : 0)));
            break;

        case bcmPortControlTCPriority:
            /* this type value definition is default internal priority (TC) */
            if (!_BCM_COSQ_PRIO_VALID(unit, value)) {
                return BCM_E_PARAM;
            }
            rv = (DRV_PORT_SET
                         (unit, t_pbm, 
                         DRV_PORT_PROP_DEFAULT_TC_PRIO, value));
            break;

        case bcmPortControlDropPrecedence:
            /* this type value definition is default Drop Precedence (DP=0~3) */
            rv = (DRV_PORT_SET
                         (unit, t_pbm, 
                         DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE, value));
            break;

        case bcmPortControlEEEEnable:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                    return BCM_E_PORT;
                        
            if (soc_feature (unit, soc_feature_eee) && 
                (soc_phyctrl_control_get(unit, port,
                BCM_PORT_PHY_CONTROL_EEE, &val) != BCM_E_UNAVAIL)) {

                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                
                /* 1. If MAC/Switch is EEE aware (Native EEE mode is supported)
                 *    and PHY also supports Native EEE mode */ 

                /* Enable/Disable Native EEE in PHY if supported */

                if (value) {
                    BCM_IF_ERROR_RETURN(
                        soc_phyctrl_control_set (unit, port, 
                        BCM_PORT_PHY_CONTROL_EEE, TRUE));
                    BCM_IF_ERROR_RETURN(
                        DRV_PORT_SET(unit, t_pbm, 
                        DRV_PORT_PROP_EEE_ENABLE, TRUE));
                } else { /* Disable */
                    BCM_IF_ERROR_RETURN(
                        DRV_PORT_SET(unit, t_pbm, 
                        DRV_PORT_PROP_EEE_ENABLE, FALSE));
                    BCM_IF_ERROR_RETURN(
                        soc_phyctrl_control_set (unit, port, 
                        BCM_PORT_PHY_CONTROL_EEE, FALSE));
                }

            } else /*2. Neither Native EEE nor AutoGrEEEn mode is supported */
                rv = BCM_E_UNAVAIL;
            break;

        case bcmPortControlEEETransmitIdleTime:
            /* DET = Time (in microsecs) for which condition to move to LPI state 
             * is satisfied, at the end of which MAC TX transitions to LPI state */
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;
        case bcmPortControlEEETransmitIdleTimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;

        case bcmPortControlEEETransmitWakeTime:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;
            
        case bcmPortControlEEETransmitWakeTimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;

        case bcmPortControlEEETransmitMinLPITime:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;

        case bcmPortControlEEETransmitMinLPITimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee))
            {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_SET(unit, t_pbm, 
                    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H, value);
            } else
                rv = BCM_E_UNAVAIL;
            break;
        case bcmPortControlFilterTcpUdpPktEnable:
            /* Enable AutoVoIP feature */
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_SET(unit, 
                    DRV_CFP_ENABLE, port, (value)? 1: 0);
            } else {
                return BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlL2Learn:
            /* for unknown address learning configuration */
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_learn_set(unit, port, value));
            break;
        case bcmPortControlL2Move:
            /* for station movement address learning configuration */            
            flags = (uint32)value;
            BCM_IF_ERROR_RETURN(_bcm_robo_port_sa_move_flags2soc(unit, 
                    flags, &val));
            
            BCM_IF_ERROR_RETURN(DRV_PORT_SET(
                    unit, t_pbm, DRV_PORT_PROP_ROAMING_OPT, val));
            break;
        case bcmPortControlForwardStaticL2MovePkt:
            /* there no ROBO chips support this feature so far */
            rv = BCM_E_UNAVAIL;
            break;
        default:
            rv = BCM_E_UNAVAIL;
            break;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_control_set: u=%d p=%d type=%d value=%d rv=%d\n"),
              unit, port, (int)type, value, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_control_get
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
bcm_robo_port_control_get(int unit, bcm_port_t port, 
                         bcm_port_control_t type, int *value)                         
{
    uint8 state = 0;
    int rv = BCM_E_NONE;
    uint32 val = 0, flags = 0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_control_get()..\n")));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    switch(type) {
        case bcmPortControlIngressTcSelectTable:
            /* value to represent the TC map modes on all frame conditions */
            rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_RAW_TC_MAP_MODE_SELECT, &val);
            *value = (int)val;
            break;
            
        case bcmPortControlFloodPktDropTc:
            /* value is a TC bitmap */
            rv = (DRV_QUEUE_QOS_CONTROL_GET
                    (unit, port, DRV_QOS_CTL_FLOOD_DROP_TCMAP, &val));
            *value = (int)val;
            break;
            
        case bcmPortControlEgressModifyOuterPktPri:
            rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EGRESS_S_PCP_REMARK, &val);
            *value = val ? 1 : 0;
            break;
            
        case bcmPortControlEgressModifyInnerPktPri:
            rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EGRESS_C_PCP_REMARK, &val);
            *value = val ? 1 : 0;
            break;
            
        case bcmPortControlPreservePacketPriority:
            rv = (DRV_QUEUE_MAPPING_TYPE_GET
                         (unit, port, 
                        DRV_QUEUE_MAP_PRIO, &state));

            *value = (int)state;
            break;
            
        case bcmPortControlEgressVlanPriUsesPktPri:
            /* this type value definition is :
             *  1: use incomming packet priority to be outgoing pri.
             *  0: use internal packet priority(TC) to obtain outgoing pri.
             */
            rv = DRV_PORT_GET(unit, port, 
                        DRV_PORT_PROP_EGRESS_PCP_REMARK, (uint32 *)value);
            *value = *value ? 0 : 1;
            break;
            
        case bcmPortControlEgressVlanCfiUsesPktPri:
            /* this type value definition is :
             *  1: the outgoing CFI is the same as incomming packet CFI.
             *  0: use internal packet priority(TC) to obtain outgoing CFI.
             */
            rv = DRV_PORT_GET(unit, port, 
                        DRV_PORT_PROP_EGRESS_CFI_REMARK, (uint32 *)value);

            *value = *value ? 0 : 1;
            break;
        case bcmPortControlIngressRateControlDrop:
            rv = DRV_RATE_CONFIG_GET(unit, port, 
                        DRV_RATE_CONFIG_DROP_ENABLE, (uint32 *)value);

            *value = *value ? 1 : 0;
            break;
            
        case bcmPortControlEgressModifyDscp:
            /* this type value definition is :
             *  0: use incomming packet DSCP value to be outgoing DSCP.
             *  1: use internal TCDP to DSCP mapping value to obtain outgoing DSCP.
             */
            rv = DRV_PORT_GET(unit, port, 
                        DRV_PORT_PROP_EGRESS_DSCP_REMARK, (uint32 *)value);
            break;

        case bcmPortControlEgressModifyECN:
            /* this type value definition is :
             *  0: use incomming packet ECN value to be outgoing ECN.
             *  1: use internal TCDP to DSCP-ECN mapping value to obtain outgoing ECN.
             */
            rv = DRV_PORT_GET(unit, port, 
                        DRV_PORT_PROP_EGRESS_ECN_REMARK, (uint32 *)value);
            break;

        case bcmPortControlTCPriority:
            /* this type value definition is default internal priority (TC) */
            rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_DEFAULT_TC_PRIO, (uint32 *)value);
            break;

        case bcmPortControlDropPrecedence:
            /* this type value definition is default Drop Precedence (DP=0~3) */
            rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE, (uint32 *)value);
            break;

        case bcmPortControlEEEEnable:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;      

            if (soc_feature (unit, soc_feature_eee) &&
                (soc_phyctrl_control_get(unit, port,
                         BCM_PORT_PHY_CONTROL_EEE, &val) != BCM_E_UNAVAIL)) {

                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;

                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 *  then get Native EEE status from MAC */
                BCM_IF_ERROR_RETURN(
                    DRV_PORT_GET(unit, port, DRV_PORT_PROP_EEE_ENABLE, &val));
                *value = val;
            } else if (soc_phyctrl_control_get(unit, port,
                         BCM_PORT_PHY_CONTROL_EEE_AUTO, &val) != BCM_E_UNAVAIL) { 
                /* If PHY supports only AutoGrEEEn mode              
                 * then get AutoGrEEEn status from PHY */
                *value = val;
            } else { /*3. Neither Native EEE nor AutoGrEEEn mode is supported */
                rv = BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlEEETransmitIdleTime:
            /* DET = Time (in microsecs) for which condition to move to LPI state 
             * is satisfied, at the end of which MAC TX transitions to LPI state */
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case bcmPortControlEEETransmitIdleTimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlEEETransmitWakeTime:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
            
        case bcmPortControlEEETransmitWakeTimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlEEETransmitMinLPITime:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlEEETransmitMinLPITimeHund:
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;

            if (soc_feature (unit, soc_feature_eee)) {
                if (!IS_GE_PORT(unit, port))
                    return BCM_E_PORT;
                rv = DRV_PORT_GET(unit, port, 
                    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H, &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlEEETransmitEventCount:
            /* Number of time MAC TX enters LPI state for 
             * a given measurement interval*/
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;
            if (!IS_GE_PORT(unit, port))
                return BCM_E_PORT;

            if ((rv = soc_phyctrl_control_get(unit, port,
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_EVENTS, &val)) != BCM_E_UNAVAIL) {
                *value = val;
            } else if (soc_feature (unit, soc_feature_eee)) {
                rv = soc_robo_counter_get32
                    (unit, port, INDEX(EEE_LPI_EVENTr), &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case bcmPortControlEEETransmitDuration:
            /* 
             * Time in (microsecs) for which MAC TX enters LPI state 
             * during a measurement interval
             */
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;
            if (!IS_GE_PORT(unit, port))
                return BCM_E_PORT;

            if ((rv = soc_phyctrl_control_get(unit, port,
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_DURATION, &val)) != BCM_E_UNAVAIL) {
                *value = val;
            } else if (soc_feature (unit, soc_feature_eee)) {
                rv = soc_robo_counter_get32
                    (unit, port, INDEX(EEE_LPI_DURATIONr), &val);
                *value = val;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case bcmPortControlFilterTcpUdpPktEnable:
            /* Enable AutoVoIP feature */
            if (BCM_GPORT_IS_WLAN_PORT(port))
                return BCM_E_UNAVAIL;
            if (!SOC_PORT_VALID(unit, port))
                return BCM_E_PORT;
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_GET(unit, 
                    DRV_CFP_ENABLE, port, &val);
                *value = val;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlL2Learn:
            /* for unknown address learning configuration */
            BCM_IF_ERROR_RETURN(
                    bcm_robo_port_learn_get(unit, port, &val));
            *value = val;
            break;
        case bcmPortControlL2Move:
            /* for station movement address learning configuration */
            BCM_IF_ERROR_RETURN(DRV_PORT_GET(
                    unit, port, DRV_PORT_PROP_ROAMING_OPT, &val));
            BCM_IF_ERROR_RETURN(_bcm_robo_port_sa_move_soc2flags(unit, 
                    val, &flags));
            *value = (int)flags;
            break;
        case bcmPortControlForwardStaticL2MovePkt:
            /* there no ROBO chips support this feature so far */
            rv = BCM_E_UNAVAIL;
            break;

        default:
            rv = BCM_E_UNAVAIL;
            break;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_control_get: u=%d p=%d type=%d value=%d rv=%d\n"),
              unit, port, (int)type, *value, rv));

    return rv;
}

/* 
 * Function:
 *      bcm_port_vlan_inner_tag_set
 * Purpose:
 *      Set the inner tag to be added to the packet.
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      inner_tag  - (IN) Inner tag. 
 *                    Priority[15:13] CFI[12] VLAN ID [11:0]
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_port_vlan_inner_tag_set(int unit, bcm_port_t port, uint16 inner_tag)
{
#ifdef BCM_TB_SUPPORT
    uint32          temp_tag = 0, outer_tag = 0;
    bcm_port_cfg_t  *pcfg;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET(
            unit, port, &outer_tag, &temp_tag));

    if (temp_tag ^ inner_tag){
        BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_SET(
                unit, port, outer_tag, inner_tag));
                
        pcfg = &robo_port_config[unit][port];
        pcfg->pc_ivlan = BCM_VLAN_CTRL_ID(inner_tag);
    }
    
    return BCM_E_NONE;
#else /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TB_SUPPORT */
}

/* 
 * Function:
 *      bcm_port_vlan_inner_tag_get
 * Purpose:
 *      Get the inner tag to be added to the packet.
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      inner_tag  - (OUT) Inner tag. 
 *                    Priority[15:13] CFI[12] VLAN ID [11:0]
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_port_vlan_inner_tag_get(int unit, bcm_port_t port, uint16 *inner_tag)
{
#ifdef BCM_TB_SUPPORT
    uint32          temp_tag = 0, outer_tag = 0;
   
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_port_vlan_inner_tag_get()..\n")));
    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET(
            unit, port, &outer_tag, &temp_tag));

    *inner_tag = (uint16)temp_tag;
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_robo_port_vlan_inner_tag_get: u=%d p=%d inner=0x%04x\n"),
              unit, port, *inner_tag));

    return BCM_E_NONE;
#else /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TB_SUPPORT */
}


int
bcm_robo_port_l3_mtu_get(int unit, bcm_port_t port, int *size)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_port_l3_mtu_set(int unit, bcm_port_t port, int size)

{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_port_force_forward_set(int unit, bcm_port_t port, 
                               bcm_port_t egr_port, int enable)
{
    bcm_pbmp_t bmp;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, egr_port, &egr_port));
    
    BCM_PBMP_CLEAR(bmp);

    /* Get current settings */
    BCM_IF_ERROR_RETURN(
        DRV_PORT_CROSS_CONNECT_GET(unit, port, &bmp));

    if (enable) {
        if (BCM_PBMP_MEMBER(bmp, egr_port)) {
            return BCM_E_NONE;
        } else {
            /* Add this port to the forwarding bitmap */
            BCM_PBMP_PORT_ADD(bmp, egr_port);
            
            BCM_IF_ERROR_RETURN(
                DRV_PORT_CROSS_CONNECT_SET(unit, port, bmp));
        }
    } else { /* disable */
        if (BCM_PBMP_MEMBER(bmp, egr_port)) {
            BCM_PBMP_PORT_REMOVE(bmp, egr_port);

            BCM_IF_ERROR_RETURN(
                DRV_PORT_CROSS_CONNECT_SET(unit, port, bmp));
        }
    }
    
    return BCM_E_NONE;
}
int 
bcm_robo_port_force_forward_get(int unit, bcm_port_t port, 
                               bcm_port_t *egr_port, int *enabled)
{
    bcm_pbmp_t bmp;
    int p = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_PBMP_CLEAR(bmp);

    /* Get current settings */
    BCM_IF_ERROR_RETURN(
        DRV_PORT_CROSS_CONNECT_GET(unit, port, &bmp));

    if (BCM_PBMP_IS_NULL(bmp)) {
        *enabled = FALSE;
    } else {
        BCM_PBMP_ITER(bmp, p) {
            *egr_port = p;
            *enabled = TRUE;
            break;
        }
    }
    
    return BCM_E_NONE;
}
int 
bcm_robo_port_class_get(int unit, bcm_port_t port, 
                bcm_port_class_t pclass, uint32 *pclass_id)
{
#ifdef  BCM_TB_SUPPORT
    int     rv = BCM_E_NONE;
    uint32  drv_value = 0;
    
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));
    
    switch (pclass) {
    case bcmPortClassIngress :
        if (SOC_IS_TBX(unit)){
            BCM_IF_ERROR_RETURN(DRV_PORT_GET(
                    unit, port, DRV_PORT_PROP_PROFILE, &drv_value));
            *pclass_id = drv_value;
            break;
        }
    case bcmPortClassFieldLookup :
    case bcmPortClassFieldIngress :
    case bcmPortClassFieldEgress :
    case bcmPortClassVlanTranslateEgress :
        rv = BCM_E_UNAVAIL;
        break;
    default :
        rv = BCM_E_PARAM;
        break;
    }
    
    return rv;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}
int 
bcm_robo_port_class_set(int unit, bcm_port_t port, 
                bcm_port_class_t pclass, uint32 pclass_id)
{
#ifdef  BCM_TB_SUPPORT
    int     rv = BCM_E_NONE;
    pbmp_t  t_pbm;
    uint32  drv_value = 0;
    
    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
   
    switch (pclass) {
    case bcmPortClassIngress :
        if (SOC_IS_TBX(unit)){
            drv_value = pclass_id;
            BCM_IF_ERROR_RETURN(DRV_PORT_SET(
                    unit, t_pbm, DRV_PORT_PROP_PROFILE, drv_value));
            break;
        }
    case bcmPortClassFieldLookup :
    case bcmPortClassFieldIngress :
    case bcmPortClassFieldEgress :
    case bcmPortClassVlanTranslateEgress :
        rv = BCM_E_UNAVAIL;
        break;
    default :
        rv = BCM_E_PARAM;
        break;
    }
        
    return rv;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}

/*
 * Function    : _bcm_robo_port_config_set
 * Description : Internal function to set port configuration.
 * Parameters  : (IN)unit  - BCM device number.
 *               (IN)port  - Port number.
 *               (IN)type  - Port property.   
 *               (IN)value - New property value.
 * Returns     : BCM_E_XXX
 */
int 
_bcm_robo_port_config_set(int unit, bcm_port_t port, 
                         _bcm_port_config_t type, int value)
{
    int rv = BCM_E_UNAVAIL;    /* Operation return status. */
    pbmp_t      t_pbm;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    switch (type) { 
    case _bcmPortUseInnerPri:
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            rv = (DRV_QUEUE_MAPPING_TYPE_SET
                         (unit, t_pbm, 
                         DRV_QUEUE_MAP_PRIO_C1P, (uint8)value));
        } 
#endif
        break;
    case _bcmPortUseOuterPri:
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            rv = (DRV_QUEUE_MAPPING_TYPE_SET
                         (unit, t_pbm, 
                         DRV_QUEUE_MAP_PRIO_S1P, (uint8)value));
        } 
#endif
        break;
    default:
        rv = BCM_E_INTERNAL;
    }

    return (rv);
}

/*
 * Function    : _bcm_robo_port_config_get
 * Description : Internal function to get port configuration.
 * Parameters  : (IN)unit  - BCM device number.
 *               (IN)port  - Port number.
 *               (IN)type  - Port property.   
 *               (OUT)value -Port property value.
 * Returns     : BCM_E_XXX
 */
int 
_bcm_robo_port_config_get(int unit, bcm_port_t port, 
                     _bcm_port_config_t type, int *value)
{
#ifdef BCM_TB_SUPPORT
    uint8 state = 0;
#endif
    int rv = BCM_E_UNAVAIL;    /* Operation return status. */

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    switch (type) { 
    case _bcmPortUseInnerPri:
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            rv = (DRV_QUEUE_MAPPING_TYPE_GET
                         (unit, port, 
                         DRV_QUEUE_MAP_PRIO_C1P, &state));
            *value = (int)state;
        } 
#endif
        break;
    case _bcmPortUseOuterPri:
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            rv = (DRV_QUEUE_MAPPING_TYPE_GET
                         (unit, port, 
                         DRV_QUEUE_MAP_PRIO_S1P, &state));
            *value = (int)state;
        } 
#endif
        break;
    default:
        rv = BCM_E_INTERNAL;
    }

    return (rv);
}

int 
bcm_robo_port_force_vlan_get(int unit, bcm_port_t port, 
                                bcm_vlan_t *vlan, int *pkt_prio, uint32 *flags)
{
 return BCM_E_UNAVAIL;
}

int 
bcm_robo_port_force_vlan_set(int unit, bcm_port_t port, 
                                bcm_vlan_t vlan, int pkt_prio, uint32 flags)
{
 return BCM_E_UNAVAIL;
}

#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT

#define _BCM_ROBO_PHY_AUTO_PD_WAR_INFO_GET(_u, _p, _info)   \
        if (robo_port_config[unit] != NULL){\
            (_info) = robo_port_config[unit][(_p)].ge_phy_auto_pd_war_info;\
        } else {    \
            (_info) = NULL; \
        }

/*
 * Define:
 *  APD_WAR_LOCK/LC_UNLOCK
 * Purpose:
 *  Provide the Lock/Unlock in Auto-PowerDown-WAR process
 *
 */
#define APD_WAR_LOCK(apd_war_info) \
        sal_mutex_take((apd_war_info)->auto_pd_war_lock, sal_mutex_FOREVER)

#define APD_WAR_UNLOCK(apd_war_info) \
        sal_mutex_give((apd_war_info)->auto_pd_war_lock)

/*
 * Function:
 *     _bcm_robo_port_phy_auto_pd_war
 * Description:
 *     SW WAR for link issue when the GE PHY is at Auto-PowerDown mode. 
 *      - The Link problem only occurred when GE PHY's Auto-PD enabled and the
 *          the link partner is Inter-NIC or Marvel_PHY.
 *      - Auto-PD is not a IEEE Spec. The interoperability with other vendors 
 *          is not guaranteed. *     
 * Parameters:
 *     unit        device number
 *     port        port number
 *     power_type  PHY power type between 
 *                  - BCM_PORT_PHY_CONTROL_POWER_AUTO and 
 *                  - BCM_PORT_PHY_CONTROL_POWER_FULL
 *
 * Return:
 *     BCM_E_NONE.
 *     Other error will be informed through warning message only to avoid
 *      the depended working thread been broken unexpectly.
 *
 * Note :
 *  - This WAR is designed for the GE PHY at Auto-PD mode and the link is not
 *      established condition.
 */

int
_bcm_robo_port_phy_auto_pd_war(int unit, 
                bcm_port_t port, int link)
{
    int         rv = BCM_E_NONE;
    sal_time_t  this_time = 0;
    bcm_robo_port_phy_auto_pd_t *phy_apd_war_info;

    /* early return if this WAR is not suitable */
    if (!IS_GE_PORT(unit, port)){
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s, Auto-PowerDown WAR is for GE PHY only!\n"), 
                  FUNCTION_NAME()));
        return BCM_E_NONE;
    }

    _BCM_ROBO_PHY_AUTO_PD_WAR_INFO_GET(unit, port, phy_apd_war_info);
    assert(phy_apd_war_info);

    APD_WAR_LOCK(phy_apd_war_info);

    if (phy_apd_war_info->war_enable == TRUE){
        if (link){
            if (phy_apd_war_info->current_link == FALSE){
                /* link change from down to up :
                 *  - restore user's Auto-PD configuration.
                 *  - set Auto-PD is assumed no effect to link status.
                 */
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s, p=%d,link changed to up! WAR check...\n"), 
                          FUNCTION_NAME(),port));
                if (phy_apd_war_info->current_auto_pd_mode != 
                        SOC_PHY_CONTROL_POWER_AUTO){
                    rv = soc_phyctrl_control_set(unit, port, 
                            SOC_PHY_CONTROL_POWER, 
                            SOC_PHY_CONTROL_POWER_AUTO);
                    if (rv != BCM_E_NONE){
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, failed in Auto-PowerDown WAR!"
                                             "Can't set to Auto-PowerDown mode(rv=%d)\n"), 
                                  FUNCTION_NAME(), rv));
                    }
                }
                /* update SW WAR info */
                phy_apd_war_info->current_link = TRUE;
                phy_apd_war_info->last_soc_time = 0;
                phy_apd_war_info->current_auto_pd_mode = 
                        SOC_PHY_CONTROL_POWER_AUTO;
                
            }
        } else {
            if (phy_apd_war_info->current_link == TRUE){
                /* link change from up to down :
                 *  - WAR will be take place after a predefined time period.
                 *  - SW update only for the link drop event.
                 */
                
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s,p=%d,link changed to down! WAR sw update...\n"), 
                          FUNCTION_NAME(),port));
               /* update SW WAR info */
                phy_apd_war_info->current_link = FALSE;
                phy_apd_war_info->last_soc_time = sal_time();

            } else {
                /* WAR process :
                 *  - check if time frame reachs the predefined time period.
                 *  - switch the power configuration between Auto-PD and FULL
                 *  
                 *  P.S. phy_apd_war_info->last_soc_time is used for two wait 
                 *      period. 
                 *      i.e. BCM_ROBO_PHY_AUTO_PD_WAR_FULL_TIME_FRAME  
                 *          and BCM_ROBO_PHY_AUTO_PD_WAR_FULL_TIME_FRAME
                 */
                this_time = sal_time();
                if (phy_apd_war_info->current_auto_pd_mode == 
                        SOC_PHY_CONTROL_POWER_AUTO){
                    if ((this_time - phy_apd_war_info->last_soc_time) >= 
                            BCM_ROBO_PHY_AUTO_PD_WAR_PD_TIME_FRAME){
                        LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "%s,p=%d,link down WAR(to FULL),"
                                             "this_time(%lu),last_time(%lu)!\n"), 
                                  FUNCTION_NAME(),port,this_time, 
                                  phy_apd_war_info->last_soc_time));
                        rv = soc_phyctrl_control_set(unit, port, 
                                SOC_PHY_CONTROL_POWER, 
                                SOC_PHY_CONTROL_POWER_FULL);
                        if (rv != BCM_E_NONE){
                            LOG_WARN(BSL_LS_BCM_COMMON,
                                     (BSL_META_U(unit,
                                                 "%s, failed in Auto-PowerDown WAR!"
                                                 " Can't set to FULL mode!(rv=%d)\n"), 
                                      FUNCTION_NAME(), rv));
                        } else {
                            phy_apd_war_info->last_soc_time = sal_time();
                            phy_apd_war_info->current_auto_pd_mode = 
                                    SOC_PHY_CONTROL_POWER_FULL;
                        }
                    }
                    
                } else if (phy_apd_war_info->current_auto_pd_mode == 
                        SOC_PHY_CONTROL_POWER_FULL){
                    if ((this_time - phy_apd_war_info->last_soc_time) >= 
                            BCM_ROBO_PHY_AUTO_PD_WAR_FULL_TIME_FRAME){
                        LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "%s,p=%d,link down WAR(to AUTO-PD),"
                                             "this_time(%ld),last_time(%ld)!\n"), 
                                  FUNCTION_NAME(),port,this_time, 
                                  phy_apd_war_info->last_soc_time));
                        rv = soc_phyctrl_control_set(unit, port, 
                                SOC_PHY_CONTROL_POWER, 
                                SOC_PHY_CONTROL_POWER_AUTO);
                        if (rv != BCM_E_NONE){
                            LOG_WARN(BSL_LS_BCM_COMMON,
                                     (BSL_META_U(unit,
                                                 "%s, failed in Auto-PowerDown WAR!"
                                                 " Can't set to Auto-PowerDown mode!"
                                                 "(rv=%d)\n"), 
                                      FUNCTION_NAME(), rv));
                        } else {
                            phy_apd_war_info->last_soc_time = sal_time();
                            phy_apd_war_info->current_auto_pd_mode = 
                                    SOC_PHY_CONTROL_POWER_AUTO;
                        }
                    }
                }                
            }
        }
    }
    APD_WAR_UNLOCK(phy_apd_war_info);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_robo_port_phy_power_mode_sw_update
 * Description:
 *     Update the SW information about the WAR for PHY Auto-PowerDown mode
 * Parameters:
 *     unit        device number
 *     port        port number
 *     power_type  PHY power type between 
 *                  - BCM_PORT_PHY_CONTROL_POWER_AUTO and 
 *                  - BCM_PORT_PHY_CONTROL_POWER_FULL
 *
 * Return:
 *     void
 */
static void 
_bcm_robo_port_phy_power_mode_sw_update(int unit, 
                bcm_port_t port, uint32 power_type)
{
    bcm_robo_port_phy_auto_pd_t *phy_apd_war_info;

    /* no GPORT in this internal routine */
    assert(!BCM_GPORT_IS_SET(port));
    if (!IS_GE_PORT(unit, port)){
        return ;    /* no Auto-PowerDown WAR for none GE port */
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, port=%d, power=%s\n"), FUNCTION_NAME(), port,
              (power_type == BCM_PORT_PHY_CONTROL_POWER_AUTO) ? 
              "Auto_PowerDown" : "Full"));
    
    _BCM_ROBO_PHY_AUTO_PD_WAR_INFO_GET(unit, port, phy_apd_war_info);
    assert(phy_apd_war_info);
    
    if (power_type == BCM_PORT_PHY_CONTROL_POWER_AUTO) {
        if (phy_apd_war_info->war_enable == TRUE){
            return;     /* do nothing for WAR is running */
        } else {
            phy_apd_war_info->war_enable = TRUE;
            phy_apd_war_info->last_soc_time = 0;
            phy_apd_war_info->current_auto_pd_mode = 
                    BCM_PORT_PHY_CONTROL_POWER_AUTO;
        }
    } else if (power_type == BCM_PORT_PHY_CONTROL_POWER_FULL) {
        if (phy_apd_war_info->war_enable == FALSE){
            return;     /* do nothing for WAR is not running */
        } else {
            phy_apd_war_info->war_enable = FALSE;
            phy_apd_war_info->last_soc_time = 0;
            phy_apd_war_info->current_auto_pd_mode = 
                    BCM_PORT_PHY_CONTROL_POWER_FULL;
        }
    } else {
        return;     /* do nothing */
    }
    
}
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */

/*
 * Function:
 *     bcm_port_phy_control_set
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
bcm_robo_port_phy_control_set(int unit, bcm_port_t port, 
                             bcm_port_phy_control_t type, uint32 value)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));

    rv = soc_phyctrl_control_set(unit, port, (soc_phy_control_t)type, value);

/* SW WAR for GE PHY's link issue on PHY's Auto-PowerDown mode */
#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
    if (rv == BCM_E_NONE){
        if ((type == BCM_PORT_PHY_CONTROL_POWER) && 
                ((value == BCM_PORT_PHY_CONTROL_POWER_AUTO) || 
                (value == BCM_PORT_PHY_CONTROL_POWER_FULL))){
            (void)_bcm_robo_port_phy_power_mode_sw_update(unit, port, value);
        }
    }
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */

    return rv; 

}

/*
 * Function:
 *     bcm_port_phy_control_get
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
bcm_robo_port_phy_control_get(int unit, bcm_port_t port,
                             bcm_port_phy_control_t type, uint32 *value)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, port, &port));
    if (NULL == value) {
        return BCM_E_PARAM;
    }

    rv = soc_phyctrl_control_get(unit, port, (soc_phy_control_t)type, value); 

    return rv;
}

int
bcm_robo_port_phy_firmware_set(int unit, bcm_port_t port, uint32 flags,
                              int offset, uint8 *array, int length)
{
 return BCM_E_UNAVAIL;
}

/*
 * Function    : _bcm_gport_dest_t_init
 * Description : Initialize gport_dest structure
 * Parameters  : (IN/OUT)  gport_dest - Structure to initialize
 * Returns     : None
 */

void
_bcm_robo_gport_dest_t_init(_bcm_gport_dest_t *gport_dest)
{
    sal_memset(gport_dest, 0, sizeof (_bcm_gport_dest_t));
}

/*
 * Function    : _bcm_robo_gport_modport_hw2api_map
 * Description : Remaps module and port from HW space to API space 
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  mod_in    - Module ID to map   
 *               (IN)  port_in   - Physical port to map   
 *               (OUT)  mod_out  - Module ID after mapping
 *               (OUT)  port_out - Port number after mapping 
 * Returns     : BCM_E_XXX
 * Notes       : If mod_out == NULL then port_out will be local physical port.
 */

int 
_bcm_robo_gport_modport_hw2api_map(int unit, bcm_module_t mod_in, 
            bcm_port_t port_in, bcm_module_t *mod_out, bcm_port_t *port_out)
{
    if (port_out == NULL) {
        return (BCM_E_PARAM);
    }

    if (NUM_MODID(unit) == 1) { /* HW device has single modid */
        if (mod_out != NULL) {
            *mod_out = mod_in;
        }
        *port_out = port_in;

        return (BCM_E_NONE);
    } else {
        /* ROBO sdk support no stacking so far */
        return (BCM_E_UNAVAIL);
    }
    
}

/*
 * Function    : _bcm_robo_gport_modport_api2hw_map
 * Description : Remaps module and port gport from API space to HW space.
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  mod_in    - Module ID to map   
 *               (IN)  port_in   - Physical port to map   
 *               (OUT)  mod_out  - Module ID after mapping
 *               (OUT)  port_out - Port number after mapping 
 * Returns     : BCM_E_XXX
 */
int 
_bcm_robo_gport_modport_api2hw_map(int unit, bcm_module_t mod_in, 
            bcm_port_t port_in, bcm_module_t *mod_out, bcm_port_t *port_out)
{
    if (port_out == NULL || mod_out == NULL) {
        return (BCM_E_PARAM);
    }

    *mod_out = mod_in;
    *port_out = port_in;

    return (BCM_E_NONE);
}

/*
 * Function    : bcm_port_gport_get
 * Description : Get the GPORT ID for the specified physical port.
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  port      - Port number
 *               (OUT) gport     - GPORT ID
 * Returns     : BCM_E_XXX
 */
int
bcm_robo_port_gport_get(int unit, bcm_port_t port, bcm_gport_t *gport)
{
    int                 num_modid = 0;
    _bcm_gport_dest_t   dest;

    _bcm_robo_gport_dest_t_init(&dest);

    BCM_IF_ERROR_RETURN(bcm_robo_stk_modid_count(unit, &num_modid));
    if (!num_modid) {
        /* No modid for this device */
        return BCM_E_UNAVAIL;
    }

    PORT_PARAM_CHECK(unit, port);

    dest.port = port;
    /* in latest API document, this API must return the gport at 
     * modport type */
    dest.gport_type = _SHR_GPORT_TYPE_MODPORT;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_modport_api2hw_map(unit, dest.modid, dest.port, 
                                      &(dest.modid), &(dest.port)));
    
    return _bcm_robo_gport_construct(unit, &dest, gport); 
}

/*
 * Function    : bcm_robo_port_local_get
 * Description : Get the local port from the given GPORT ID.
 *
 * Parameters  : (IN)  unit         - BCM device number
 *               (IN)  gport        - global port identifier
 *               (OUT) local_port   - local port encoded in gport
 * Returns     : BCM_E_XXX
 *
 * Note : 
 *  1. This API for ROBO is designed as different processing flow with ESW.
 *      - No Module info.
 */
int
bcm_robo_port_local_get(int unit, bcm_gport_t gport, bcm_port_t *local_port)
{
    int rv = BCM_E_NONE;

    if (SOC_GPORT_IS_LOCAL(gport)) {
        *local_port = SOC_GPORT_LOCAL_GET(gport);
    } else if (SOC_GPORT_IS_DEVPORT(gport)) {
        *local_port = SOC_GPORT_DEVPORT_PORT_GET(gport);
        if (unit != SOC_GPORT_DEVPORT_DEVID_GET(gport)) {
            return BCM_E_PORT;
        }
    } else {
        bcm_module_t    mod;
        bcm_trunk_t     tgid;
        int             id;
        
        rv = _bcm_robo_gport_resolve(
                unit, gport, &mod, local_port, &tgid, &id);
        
        if (rv){
            return BCM_E_PORT;
        }
    }

    return BCM_E_NONE;
}

int
bcm_robo_port_default_vlan_action_set(int unit, bcm_port_t port,
                                     bcm_vlan_action_set_t *action)
{
 return BCM_E_UNAVAIL;
}

int
bcm_robo_port_default_vlan_action_get(int unit, bcm_port_t port, 
                                     bcm_vlan_action_set_t *action)
{
 return BCM_E_UNAVAIL;
}

/* 
 * Function:
 *      bcm_port_egress_default_vlan_action_set
 * Purpose:
 *      Set the egress port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (IN) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_port_egress_default_vlan_action_set(int unit, bcm_port_t port,
                                            bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_egress_default_vlan_action_get
 * Purpose:
 *      Get the egress port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (OUT) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_port_egress_default_vlan_action_get(int unit, bcm_port_t port, 
                                            bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function    : _bcm_robo_gport_resolve
 * Description : Internal function to get modid, port, and trunk_id
 *               from a bcm_gport_t (global port)
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  gport     - Global port identifier
 *               (OUT) modid     - Module ID
 *               (OUT) port      - Port number
 *               (OUT) trunk_id  - Trunk ID
 *               (OUT) id        - HW ID
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               application space to local modid/port space
 */
int 
_bcm_robo_gport_resolve(int unit, bcm_gport_t gport,
                       bcm_module_t *modid, bcm_port_t *port, 
                       bcm_trunk_t *trunk_id, int *id)
{
    int             local_id, rv = BCM_E_NONE;
    bcm_module_t    mod_in, local_modid;
    bcm_port_t      port_in, local_port;
    bcm_trunk_t     local_tgid;
    
    local_modid = -1;
    local_port = -1;
    local_id = -1;
    local_tgid = BCM_TRUNK_INVALID;

    if (SOC_GPORT_IS_TRUNK(gport)) {
        local_tgid = SOC_GPORT_TRUNK_GET(gport);
    } else if (SOC_GPORT_IS_LOCAL(gport)) {
        BCM_IF_ERROR_RETURN 
            (bcm_stk_my_modid_get(unit, &local_modid));
        local_port = SOC_GPORT_LOCAL_GET(gport);

        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s, local_port=0x%x, not addressable!\n"),
                      FUNCTION_NAME(), local_port));
            return BCM_E_PORT;
        }
    } else if (SOC_GPORT_IS_DEVPORT(gport)) {
        BCM_IF_ERROR_RETURN 
            (bcm_stk_my_modid_get(unit, &local_modid));
        local_port = SOC_GPORT_DEVPORT_PORT_GET(gport);

        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            return BCM_E_PORT;
        }
    } else if (SOC_GPORT_IS_MODPORT(gport)) {
        mod_in = SOC_GPORT_MODPORT_MODID_GET(gport);
        port_in = SOC_GPORT_MODPORT_PORT_GET(gport);
        if ((NUM_MODID(unit) == 2) && (port_in > 31)) {
            return BCM_E_PORT;
        }
        rv = bcm_robo_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                    mod_in, port_in, &local_modid, &local_port);

        if (!SOC_MODID_ADDRESSABLE(unit, local_modid)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s, local_mod=0x%x, not addressable!\n"),
                      FUNCTION_NAME(), local_modid));
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s, local_port=0x%x, not addressable!\n"),
                      FUNCTION_NAME(), local_port));
            return BCM_E_PORT;
        }
    } else if (SOC_GPORT_IS_SUBPORT_GROUP(gport) || 
            SOC_GPORT_IS_SUBPORT_PORT(gport)) {
        if (soc_feature(unit, soc_feature_subport)) {
            int     group_id;
            rv = _bcm_robo_subport_group_resolve(unit, gport, 
                    &local_modid, &local_port, &group_id, &local_id);
            if (!SOC_MODID_ADDRESSABLE(unit, local_modid)) {
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s, local_mod=0x%x, not addressable!\n"),
                          FUNCTION_NAME(), local_modid));
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s, local_port=0x%x, not addressable!\n"),
                          FUNCTION_NAME(), local_port));
                return BCM_E_PORT;
            }
        }
#ifdef BCM_POLAR_SUPPORT
    } else if (SOC_GPORT_IS_SCHEDULER(gport)) {
        BCM_IF_ERROR_RETURN 
            (bcm_stk_my_modid_get(unit, &local_modid));
        rv = _bcm_robo_cosq_gport_resolve(unit, gport, &local_port);
        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s, local_port=0x%x, not addressable!\n"),
                      FUNCTION_NAME(), local_port));
            return BCM_E_PORT;
        }
#endif  /* BCM_POLAR_SUPPORT */
    /* BCM_GPORT_INVALID should return an error */
    } else if ((gport != BCM_GPORT_TYPE_BLACK_HOLE)) {
        return BCM_E_PORT;
    }

    *modid = local_modid;
    *port = local_port;
    *trunk_id = local_tgid;
    *id = local_id; 
    
    return (rv);
}

/*
 * Function    : _bcm_robo_gport_construct
 * Description : Internal function to construct a gport from 
 *                given parameters 
 * Parameters  : (IN)  unit       - BCM device number
 *               (IN)  gport_dest - Structure that contains destination
 *                                   to encode into a gport
 *               (OUT) gport      - Global port identifier
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               local modid/port space to application space
 */
int 
_bcm_robo_gport_construct(int unit, _bcm_gport_dest_t *gport_dest, bcm_gport_t *gport)
{

    bcm_gport_t     l_gport = 0;
    bcm_module_t    mod_out;
    bcm_port_t      port_out;
    
    if ((NULL == gport_dest) || (NULL == gport) ){
        return BCM_E_PARAM;
    }

    switch (gport_dest->gport_type) {
        case _SHR_GPORT_TYPE_TRUNK:
            SOC_GPORT_TRUNK_SET(l_gport, gport_dest->tgid);
            break;
        case _SHR_GPORT_TYPE_SUBPORT_GROUP:
            SOC_GPORT_SUBPORT_GROUP_SET(l_gport, gport_dest->subport_id);
            break;
        case _SHR_GPORT_TYPE_SUBPORT_PORT:
            SOC_GPORT_SUBPORT_PORT_SET(l_gport, gport_dest->subport_id);
            break;
        case _SHR_GPORT_TYPE_LOCAL:
            SOC_GPORT_LOCAL_SET(l_gport, gport_dest->port);
            break;
        case _SHR_GPORT_TYPE_MODPORT:
            BCM_IF_ERROR_RETURN (
                _bcm_robo_gport_modport_hw2api_map(unit, gport_dest->modid, 
                                              gport_dest->port, &mod_out,
                                              &port_out));
            SOC_GPORT_MODPORT_SET(l_gport, mod_out, port_out);
            break;
        default:    
            return BCM_E_PARAM;
    }

    *gport = l_gport;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_port_phy_timesync_config_set
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      conf - (IN) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_robo_port_phy_timesync_config_set(int unit, bcm_port_t port, bcm_port_phy_timesync_config_t *conf)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_robo_port_timesync_config_set()..\n")));
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
        BCM_IF_ERROR_RETURN(DRV_TIMESYNC_CONFIG_SET
            (unit, (uint32)port, (soc_port_phy_timesync_config_t *)conf));
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || STARFIGHTER3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_phy_timesync_config_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      conf - (OUT) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_robo_port_phy_timesync_config_get(int unit, bcm_port_t port, bcm_port_phy_timesync_config_t *conf)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_robo_port_timesync_config_get()..\n")));
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
        BCM_IF_ERROR_RETURN(DRV_TIMESYNC_CONFIG_GET
            (unit, (uint32)port, (soc_port_phy_timesync_config_t *)conf));
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || STARFIGHTER3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_control_phy_timesync_set
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      type - (IN) Operation
 *      value- (IN) Arg to operation
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_robo_port_control_phy_timesync_set(int unit, bcm_port_t port, bcm_port_control_phy_timesync_t type, uint64 value)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_robo_port_control_timesync_set()..\n")));
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
        BCM_IF_ERROR_RETURN(DRV_CONTROL_TIMESYNC_SET
            (unit, (uint32)port, type, value));
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || STARFIGHTER3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_control_phy_timesync_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      type - (IN) Operation
 *      value- (OUT) Pointer to arg for operation
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_robo_port_control_phy_timesync_get(int unit, bcm_port_t port, bcm_port_control_phy_timesync_t type, uint64 *value)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "BCM API : bcm_robo_port_control_timesync_get()..\n")));
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
        BCM_IF_ERROR_RETURN(DRV_CONTROL_TIMESYNC_GET
            (unit, (uint32)port, type, value));
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || STARFIGHTER3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_port_phy_timesync_enhanced_capture_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      value - (OUT) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_robo_port_phy_timesync_enhanced_capture_get(int unit, bcm_port_t port, 
                                     bcm_port_phy_timesync_enhanced_capture_t *value)
{
    return BCM_E_UNAVAIL;
}


