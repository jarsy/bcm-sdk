/*
 * $Id: link.c,v 1.83 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Link Scan
 *
 * Linkscan should always run for all chips in a system. It manages
 * the current chip LINK state (EPC_LINK or equivalent), and programs
 * MACs to match auto-negotiated links. 
 *
 * Linkscan is also responsible for determining the link up/down
 * status for a port. Since link down events signaled by most PHYs are
 * latched low and cleared on read, it is important that accesses to
 * to the PHY itself be managed carefully. Linkscan is intended to be
 * the only process that reads the actual PHY (using
 * _bcm_port_link_get). All other calls to retrieve link status
 * results in calls to _bcm_robo_link_get which returns the linkscan view
 * of the link status. This ensures linkscan is the only process that
 * reads the actual PHYs.
 *
 * All modifications to the linkscan state are protected by LC_LOCK.
 *
 * Linkscan maintains the following port bitmaps
 *
 *     lc_pbm_link:
 *                 Current physical link up/down status. When a bit
 *                 in this mask is set, a link up or link down event
 *                 is recognized and signaled to any registered
 *                 handlers.
 *
 *     lc_pbm_link_change:
 *                 Mask of ports that need to recognize a link
 *                 down event. Ports are added to this mask by the
 *                 function bcm_link_change. 
 *
 *     lc_pbm_override_ports:
 *                 Bit map of ports that are currently
 *                 being explicitly set to a value. These actual value
 *                 is determined by lb_pbm_override_link. Ports are
 *                 added and removed from this mode by the routine
 *                 _bcm_link_force. 
 *
 *                 Ports that are forced to an UP state do NOT result
 *                 in a call to bcm_port_update. It is the
 *                 responsibility of the caller to configure the
 *                 correct MAC and PHY state.
 *
 *     lc_pbm_override_link:
 *                 Bit map indicating the link-up/link-down
 *                 status for those ports with override set.
 *
 * Calls to _bcm_robo_link_get always returns the current status as
 * indicated by lc_pbm_link. 
 */

#include <shared/bsl.h>

#include <sal/types.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>

#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/cmic.h>
#include <soc/phyctrl.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/link.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/robo/link.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo_dispatch.h>

typedef struct ls_handler_s {
    struct ls_handler_s         *lh_next;
    bcm_linkscan_handler_t      lh_f;
} ls_handler_t;

typedef struct ls_errstate_s {
    int         limit;      /* # errors to enter error state */
    int         delay;      /* Length of error state in seconds */
    int         count;      /* # of errors so far */
    int         wait;       /* Boolean, TRUE when in error state */
    sal_usecs_t     time;       /* Time error state was entered */
} ls_errstate_t;

typedef struct ls_cntl_s {
    char        lc_taskname[16];
    sal_mutex_t         lc_lock;        /* Synchronization */
    pbmp_t              lc_pbm_hw;      /* Hardware link scan ports */
    pbmp_t              lc_pbm_sw;      /* Software link scan ports */
    pbmp_t              lc_pbm_link;    /* Ports currently up */
    pbmp_t              lc_pbm_link_change;/* Ports needed to recognize down */
    pbmp_t              lc_pbm_override_ports; /* Force up/Down ports */
    pbmp_t              lc_pbm_override_link;  /* Force up/Down status */
    ls_handler_t        *lc_handler;    /* List of handlers */
    VOL int             lc_us;          /* Time between scans (us) */
    VOL sal_thread_t    lc_thread;      /* Link scan thread */
    sal_sem_t           lc_sema;        /* Link scan semaphore */
    ls_errstate_t   lc_error[SOC_MAX_NUM_PORTS];   /* Link error state */
    bcm_linkscan_port_handler_t lc_f[SOC_MAX_NUM_PORTS]; /* Handler for link fn */
} ls_cntl_t;

#if defined(BCM_53134_B0)
#define BCM53134_STRAP_PIN_IMP_MODE_MASK    0x200
#define BCM53134_STRAP_PIN_IMP_MODE_SGMII   0x200
#define BCM53134_STRAP_PIN_RESET_SM_MASK    0x20000
#define BCM53134_STRAP_PIN_XTAL_50MHZ       0x20000
#endif


/*
 * Define:
 *  LC_LOCK/LC_UNLOCK
 * Purpose:
 *  Serialization Macros for access to lc_cntl structure.
 */

#define LC_LOCK(unit) \
        sal_mutex_take(robo_link_control[unit]->lc_lock, sal_mutex_FOREVER)

#define LC_UNLOCK(unit) \
        sal_mutex_give(robo_link_control[unit]->lc_lock)

#define LC_CHECK_INIT(unit) \
        if (robo_link_control[unit] == NULL) { \
            return(BCM_E_INIT); \
        }

/*
 * Variable:
 *  robo_link_control
 * Purpose:
 *  Hold per-unit global status for linkscan
 */

STATIC ls_cntl_t       *robo_link_control[BCM_MAX_NUM_UNITS];

#ifdef WAN_PORT_SUPPORT
#if defined(BCM_5397_A0)
#define WAN_PORT_NUMBER 7
static int wan_port_cur_link;
#endif /* BCM_5397_A0 */
#endif /* WAN_PORT_SUPPORT */

/* 
 * Internal routines
 */

/*
 * Function:
 *  _robo_lc_pbm_init
 * Purpose:
 *  Initialize the port bitmaps in the link_control structure
 * Parameters:
 *      unit - RoboSwitch unit #.
 */

static void
_robo_lc_pbm_init(int unit)
{
    ls_cntl_t   *lc = robo_link_control[unit];
#if defined (BCM_POLAR_SUPPORT)
    soc_port_t      port;
#endif /* BCM_POLAR_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT)||defined (BCM_NORTHSTARPLUS_SUPPORT)
    pbmp_t pbmp;
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */

    /*
     * Force initial scan by setting link change while pretending link
     * was initially up.
     */
    BCM_PBMP_ASSIGN(lc->lc_pbm_link, PBMP_ALL(unit));
    BCM_PBMP_ASSIGN(lc->lc_pbm_link_change, PBMP_PORT_ALL(unit));

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
    	/* Don't let link change on BR PHY of Polar to support rapid boot feature */
        PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            if (soc_property_port_get(unit, port, spn_BCM89500_RAPID_BOOT, 0)) {
                BCM_PBMP_PORT_REMOVE(lc->lc_pbm_link_change, port);
            }
        }
    }
#endif /* BCM_POLAR_SUPPORT */

    BCM_PBMP_CLEAR(lc->lc_pbm_override_ports);
    BCM_PBMP_CLEAR(lc->lc_pbm_override_link);
    
    /* Force the CPU port always link with override assigning(Robo specific)
     *  - for the MII port in Robo Switch is defined as a port type.
     */
    BCM_PBMP_PORT_ADD(lc->lc_pbm_override_ports, CMIC_PORT(unit));
    BCM_PBMP_PORT_ADD(lc->lc_pbm_override_link, CMIC_PORT(unit));

    if(SOC_IS_VO(unit)) {
        BCM_PBMP_OR(lc->lc_pbm_override_ports, SOC_INFO(unit).s_pbm);
        BCM_PBMP_OR(lc->lc_pbm_override_link, SOC_INFO(unit).s_pbm);
    }

#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
        BCM_PBMP_CLEAR(pbmp);
        if (soc_property_get(unit, spn_BCM89500_RAPID_BOOT, 0)) {
            pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
            BCM_PBMP_AND(pbmp, PBMP_ALL(unit));
        }
        if (BCM_PBMP_NOT_NULL(pbmp)) {
            BCM_PBMP_OR(lc->lc_pbm_override_ports, pbmp);
            BCM_PBMP_OR(lc->lc_pbm_override_link, pbmp);
            BCM_PBMP_REMOVE(lc->lc_pbm_link_change, pbmp);
        }

        if (SOC_IS_NORTHSTAR(unit) || 
            (SOC_IS_NORTHSTARPLUS(unit) &&
            (!BCM_PBMP_MEMBER(PBMP_E_ALL(unit), 5)))) {
            BCM_PBMP_PORT_ADD(lc->lc_pbm_override_ports, 5);
            BCM_PBMP_PORT_ADD(lc->lc_pbm_override_link, 5);
        }
        BCM_PBMP_PORT_ADD(lc->lc_pbm_override_ports, 7);
        BCM_PBMP_PORT_ADD(lc->lc_pbm_override_link, 7);

    } 
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */
}
 
/*
 * Function:    
 *      _bcm_robo_linkscan_hw_interrupt
 * Purpose:     
 *      Link scan interrupt handler if using CMIC_LINK_SCAN.
 * Parameters:  
 *      RoboSwitch unit #
 * Returns:     
 *      Nothing
 */

STATIC void
_bcm_robo_linkscan_hw_interrupt(int unit)
{
    /* Robo Chip not support HW interrupt */
}

/*
 * Function:    
 *      _bcm_robo_linkscan_update_port
 * Purpose:     
 *      Check for and process a link event on one port
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      port - Port to process.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_linkscan_update_port(int unit, int port)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    int         cur_link, new_link = 0,  notify = FALSE, change;
    bcm_port_info_t info;
    ls_handler_t    *lh, *lh_next = NULL;
    int         rv;


    assert(SOC_PORT_VALID(unit, port));

    /*
     * Current link status is calculated in the following order:
     *   1) If link status is in an override state, the override
     *      state is used.
     *   2) If link is required to recognize a link down event, the
     *      current scan will recognize the link down (if the link
     *      was previously down, this will result in no action
     *      being taken)
     *   3) Use real detected link status.
     *        a) If using Hardware link scanning, use captured H/W
     *           value since the H/W scan will clear the latched
     *           link down event.
     *        b) If using S/W link scanning, retrieve the current
     *           link status from the PHY.
     */

    cur_link = SOC_PBMP_MEMBER(lc->lc_pbm_link, port);

    change = SOC_PBMP_MEMBER(lc->lc_pbm_link_change, port);
    SOC_PBMP_PORT_REMOVE(lc->lc_pbm_link_change, port);

    if (change) {                             /* 2) */
        new_link = FALSE;
        rv = BCM_E_NONE;
    } else if (SOC_PBMP_MEMBER(lc->lc_pbm_override_ports,
                   port)) {               /* 1) */
        new_link = SOC_PBMP_MEMBER(lc->lc_pbm_override_link, port);
        rv = BCM_E_NONE;
        
    } else if (SOC_PBMP_MEMBER(lc->lc_pbm_hw, port)) {            /* 3a) */
        rv = _bcm_robo_port_link_get(unit, port, 1, &new_link);
    } else if (SOC_PBMP_MEMBER(lc->lc_pbm_sw, port)) {            /* 3b) */

        if (lc->lc_f[port]) {
            int state;

            rv = lc->lc_f[port](unit, port, &state);
            if (rv == BCM_E_NONE) {
                new_link = state ? TRUE : FALSE;
            } else if (rv == BCM_E_UNAVAIL) {
                rv = _bcm_robo_port_link_get(unit, port, 0, &new_link);
            }
        } else {
            rv = _bcm_robo_port_link_get(unit, port, 0, &new_link);
        }

#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT

        if (!(SOC_IS_STARFIGHTER3(unit))) {
            /* GE_PHY need to apply the WAR for PHY_Auto_PowerDown mode */
            if (IS_GE_PORT(unit, port)){
                rv = _bcm_robo_port_phy_auto_pd_war(unit, port, new_link);
                if (BCM_FAILURE(rv)){
                    LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Port%d is failed on applying Auto-PD WAR(rv=%d)!\n"),
                          port, rv));
                }
            }
        }
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */         
    } else {
        return BCM_E_NONE;  /* Port not being scanned */
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Port %s: Failed to recover link status: %s\n"), 
                   SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
        return rv;
    }

    if (cur_link == new_link){
        return BCM_E_NONE;
    }

    /*
     * If disabling, stop ingresses from sending any more traffic to
     * this port.
     */

    if (!new_link) {
        SOC_PBMP_PORT_REMOVE(lc->lc_pbm_link, port);
        
        rv = soc_link_fwd_set(unit, lc->lc_pbm_link);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Port %s: soc_link_fwd_set failed: %s\n"),
                       SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
            return rv;
        }
        /* update the link bit in port state override register */
        soc_robo_link_sw_update(unit, new_link, port);
    }

    /* Program MACs
         * (only if port is not forced, and link state changed) */

    if (!SOC_PBMP_MEMBER(lc->lc_pbm_override_ports, port)) {
        rv = bcm_port_update(unit, port, new_link);
        
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Port %s: bcm_port_update failed: %s\n"),
                       SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
            return rv;
        }
    }

    /*
     * If enabling, allow traffic to go to this port.
     */

    if (new_link) {
      
        SOC_PBMP_PORT_ADD(lc->lc_pbm_link, port);
        rv = soc_link_fwd_set(unit, lc->lc_pbm_link);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Port %s: soc_link_fwd_set failed: %s\n"),
                       SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
            return rv;
        }
        
        /* update the link bit in port state override register */
        soc_robo_link_sw_update(unit, new_link, port);  
    }
    /*
     * Call registered handlers with complete link info.
     * Display link status message, if requested.
     *
     * In case link status changed again for bcm_port_info_get,
     * overwrite the linkstatus field with logical_link.  This ensures
     * the handler is presented with a consistent alternating
     * sequence of link up/down.
     */

    notify = (cur_link  != new_link);

    LOG_VERBOSE(BSL_LS_BCM_LINK,
                (BSL_META_U(unit,
                            "Unit %d, Port %s: Link: Current %s, New %s\n"),
                 unit,
                 SOC_PORT_NAME(unit, port),
                 cur_link ? "up" : "down",
                 new_link ? "up" : "down"));

    if (notify) {
        rv = bcm_port_info_get(unit, port, &info);

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Port %s: bcm_port_info_get failed: %s\n"), 
                       SOC_PORT_NAME(unit, port),
                       bcm_errmsg(rv)));
            return rv;
        }
    }

    if (new_link) {
        LOG_INFO(BSL_LS_BCM_LINK,
                 (BSL_META_U(unit,
                             "Port %s: link up (%dMb %s %s)\n"),
                  SOC_PORT_NAME(unit, port),
                  info.speed,
                  info.duplex ? "Full Duplex" : "Half Duplex",
                  PHY_FIBER_MODE(unit, port) ?
                  "Fiber" : "Copper"));
    } else {
        LOG_INFO(BSL_LS_BCM_LINK,
                 (BSL_META_U(unit,
                             "Port %s: link down\n"),
                  SOC_PORT_NAME(unit, port)));
    }    


    if (notify) {
        LOG_VERBOSE(BSL_LS_BCM_LINK,
                    (BSL_META_U(unit,
                                "Unit %d, Port %s: logical link notification - %s\n"),
                     unit, SOC_PORT_NAME(unit, port),
                     new_link  ? "up" : "down"));
        info.linkstatus = new_link;

        for (lh = lc->lc_handler; lh; lh = lh_next) {
            /*
             * save the next linkscan handler first, in case current handler
             * unregister itself inside the handler function
             */
            lh_next = lh->lh_next;        
            lh->lh_f(unit, port, &info);
        }
    }   

    return BCM_E_NONE;
}


#if defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128) || defined(BCM_POLAR_SUPPORT)
/*
 * Function:    
 *      _bcm_robo_mac_low_power_update
 * Purpose:     
 *      Check for link status on each port.  
 *      If all ports are link-down, enable the MAC low power mode.
 * Parameters:  
 *      unit - RoboSwitch unit #.
 * Returns:
 *      Nothing.
 */

STATIC void
_bcm_robo_mac_low_power_update(int unit) 
{
     ls_cntl_t           *lc = robo_link_control[unit];
    int rv = BCM_E_NONE;
    uint32      temp = 0, cur_link = 0;
    
    if (!SOC_MAC_LOW_POWER_ENABLED(unit)) {
        /* Get the supported port bitmap of low power mode */
        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP, &temp);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Failed to get the supported port bitmap of MAC Low power mode!\n")));
        } else {

            /* Check if all ports are link-down */
            cur_link = SOC_PBMP_WORD_GET(lc->lc_pbm_override_ports, 0) | 
                SOC_PBMP_WORD_GET(lc->lc_pbm_override_link, 0);
            cur_link = cur_link | SOC_PBMP_WORD_GET(lc->lc_pbm_link, 0);
            if (!(cur_link & temp)) {
                
                if (!SOC_CONTROL(unit)->all_link_down_detected) {
                    /* Update the timestamp of all ports link down */
                    MAC_LOW_POWER_LOCK(unit);
                    SOC_CONTROL(unit)->all_link_down_detected = sal_time();
                    MAC_LOW_POWER_UNLOCK(unit);
                }
            

                /* Wait 2 sec to enter the low power mode */ 
                if ((sal_time() - (SOC_CONTROL(unit)->all_link_down_detected)) > 
                    BCM_ROBO_MAC_LOW_POWER_WAITING_TIME) {
                    
                    /* Enable to Low Power Mode */
                    if (SOC_IS_LOTUS(unit) || 
                        (SOC_IS_POLAR(unit) && !SOC_CONTROL(unit)->int_cpu_enabled)) {
#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
                        /* Save te PHY configuration */
                        rv = _bcm_robo53101_phy_cfg_save(unit);
                        if (BCM_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_BCM_COMMON,
                                      (BSL_META_U(unit,
                                                  "Failed to save the PHY configuration!\n")));
                        }
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */
                        if (SOC_IS_POLAR(unit)) {
                            temp  = 6250; /* 6.25Mhz */
                        } else {
                            temp = 12500; /* 12.5Mhz */
                        }
                    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
                        temp  = 3125; /* 3.125Mhz */
                    }
                    rv= DRV_DEV_PROP_SET(unit, 
                        DRV_DEV_PROP_LOW_POWER_ENABLE, temp);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "Enabling the MAC Low Power Mode was failed!\n")));
                    } else {
                        MAC_LOW_POWER_LOCK(unit);
                        
                        SOC_MAC_LOW_POWER_ENABLED(unit) = 1;

                        MAC_LOW_POWER_UNLOCK(unit);
                    }
#ifdef BCM_POLAR_SUPPORT
                    if(SOC_IS_POLAR(unit)) { 
                        MAC_LOW_POWER_LOCK(unit);
                        SOC_CONTROL(unit)->ultra_low_power_detected = sal_time();
                        MAC_LOW_POWER_UNLOCK(unit);
                    }
#endif /* BCM_POLAR_SUPPORT */                    
                }
            } else {
                if (SOC_CONTROL(unit)->all_link_down_detected) {
                    MAC_LOW_POWER_LOCK(unit);
                    SOC_CONTROL(unit)->all_link_down_detected = 0;
#ifdef BCM_POLAR_SUPPORT
                    if(SOC_IS_POLAR(unit)) { 
                        SOC_CONTROL(unit)->ultra_low_power_detected = 0;
                    }
#endif /* BCM_POLAR_SUPPORT */

                    MAC_LOW_POWER_UNLOCK(unit);
                }

            }
        }
    } else {
        /* Reset the timestamp if low power is enabled. */
        if (SOC_CONTROL(unit)->all_link_down_detected) {
            MAC_LOW_POWER_LOCK(unit);
            SOC_CONTROL(unit)->all_link_down_detected = 0;
            MAC_LOW_POWER_UNLOCK(unit);
        }

#ifdef BCM_POLAR_SUPPORT
        if(SOC_IS_POLAR(unit)) { 
            if ((sal_time() - (SOC_CONTROL(unit)->ultra_low_power_detected)) > 
                soc_property_get(unit, spn_POWER_DOWN_TIMEOUT,
                                 BCM_ROBO_MAC_ULTRA_LOW_POWER_WAITING_TIME)) {
                SOC_CONTROL(unit)->ultra_low_power_detected = sal_time();
                rv= DRV_DEV_PROP_SET(unit, DRV_DEV_PROP_ULTRA_LOW_POWER, 0x0);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Enabling the MAC Ultra Low Power Mode was failed!\n")));
                }
            }
        }
#endif /* BCM_POLAR_SUPPORT */
    }
}

#endif /* BCM_53101 || BCM_53125 || BCM_53128 || BCM_POLAR_SUPPORT */

/*
 * Function:    
 *      _bcm_robo_linkscan_update
 * Purpose:     
 *      Check for a change in link status on each link.  If a change
 *      is detected, call bcm_port_update to program the MACs for that
 *      link, and call the list of registered handlers.
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      pbm - bit map of ports to scan. 
 * Returns:
 *      Nothing.
 */

STATIC void
_bcm_robo_linkscan_update(int unit, pbmp_t pbm)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    pbmp_t      save_link_change;
    int                 rv;
    bcm_port_t          port;
#ifdef BCM_53134_B0
    uint32 reg1 = 0, reg2 = 0;
    uint32 temp1 = 0, temp2 = 0;
    uint32 reg_value = 0;
#endif

    LC_LOCK(unit);

    /*
     * Suspend hardware link scan here to avoid overhead of pause/resume
     * on MDIO accesses.
     */

    soc_linkscan_pause(unit);           /* Suspend linkscan */

    /* In SF3 B0, when port 8 is in SGMII mode, link status needs to be updated
     * for both port 8 SGMII and port 5 RGMII using status override registers.
     * This is done because hardware is not reporting correct link status for
     * Port 8 and Port 5 in switch registers */
#ifdef BCM_53134_B0
    if (SOC_IS_STARFIGHTER3_B0(unit)) {
        REG_READ_STRAP_PIN_STATUSr(unit, &reg_value);
        /* Check for IMP_MODE SGMII, Check for 50MHz board */
        if ((reg_value & BCM53134_STRAP_PIN_IMP_MODE_MASK)
                    == BCM53134_STRAP_PIN_IMP_MODE_SGMII) {
            if ((reg_value & BCM53134_STRAP_PIN_RESET_SM_MASK)
                        == BCM53134_STRAP_PIN_XTAL_50MHZ) {
                REG_READ_SGMII_STATUSr(unit, &reg1);
                REG_READ_STS_OVERRIDE_IMPr(unit, 8, &reg2);
                soc_SGMII_STATUSr_field_get(unit, &reg1, LNK_STATf, &temp1);
                soc_STS_OVERRIDE_IMPr_field_get(unit, &reg2, LINK_STSf, &temp2);
        /* If Link status change is detected in SGMII status reg,
         * update link status in IMP status override reg */
                if (temp1 ^ temp2) {
                    soc_STS_OVERRIDE_IMPr_field_set(unit, &reg2,
                            LINK_STSf, &temp1);
                    temp2 = 0x1;
                    soc_STS_OVERRIDE_IMPr_field_set(unit, &reg2,
                            MII_SW_ORf, &temp2);
                    REG_WRITE_STS_OVERRIDE_IMPr(unit, 8, &reg2);
                }
            }
            if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), 5)) {
                REG_READ_G_MIISTS_EXT_P5r(unit, &reg1);
                REG_READ_STS_OVERRIDE_P5r(unit, &reg2);
                soc_G_MIISTS_EXT_P5r_field_get(unit, &reg1, LINK_STAf, &temp1);
                soc_STS_OVERRIDE_P5r_field_get(unit, &reg2, LINK_STSf, &temp2);
        /* If Link status change is detected in P5 external MII status reg,
         * update link status in P5 status override reg */
                if (temp1 ^ temp2) {
                    soc_STS_OVERRIDE_P5r_field_set(unit, &reg2,
                            LINK_STSf, &temp1);
                    temp2 = 0x1;
                    soc_STS_OVERRIDE_P5r_field_set(unit, &reg2,
                            SW_OVERRIDEf, &temp2);
                    REG_WRITE_STS_OVERRIDE_P5r(unit, &reg2);
                }
            }
        }
    }
#endif

    PBMP_ITER(pbm, port) {
        ls_errstate_t *err = &lc->lc_error[port];
        
        if (err->wait) {    /* Port waiting in error state */
            if (SAL_USECS_SUB(sal_time_usecs(), err->time) >= err->delay) {
                err->wait = 0;  /* Exit error state */
                err->count = 0;
                
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "Port %s: restored\n"),
                             SOC_PORT_NAME(unit, port)));
            } else {
                continue;
            }
        }
        
        save_link_change = lc->lc_pbm_link_change;
        
        rv = _bcm_robo_linkscan_update_port(unit, port);

        if (BCM_FAILURE(rv)) {
            lc->lc_pbm_link_change = save_link_change;
        
            if (++err->count >= err->limit && err->limit > 0) {
                /* Enter error state */
                
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Port %s: temporarily removed from linkscan\n"),
                           SOC_PORT_NAME(unit, port)));
                
                err->time = sal_time_usecs();
                err->wait = 1;
            }
        } else if (err->count > 0) {
            err->count--;   /* Reprieve */
        }
    }

#if defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128) || defined(BCM_POLAR_SUPPORT)
    /* Check if all ports are link-down.
     * If yes, enabling MAC low power mode.
     */
    if ((SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit)) && 
        (SOC_AUTO_MAC_LOW_POWER(unit))) {
        _bcm_robo_mac_low_power_update(unit);    
    }
#endif /* BCM_53101 || BCM_53125 || BCM_53128 || BCM_POLAR_SUPPORT */
    
    soc_linkscan_continue(unit);        /* Restart H/W link scan */

    LC_UNLOCK(unit);
}

/*
 * Function:    
 *      bcm_linkscan_update
 * Purpose:     
 *      Check for a change in link status on each link.  If a change
 *      is detected, call bcm_port_update to program the MACs for that
 *      link, and call the list of registered handlers.
 * Parameters:  
 *      unit - StrataSwitch unit #.
 *      pbm - bit map of ports to scan. 
 * Returns:
 *      BCM_E_XXX.
 */
int 
bcm_robo_linkscan_update(int unit, bcm_pbmp_t pbm)
{                       
    LC_CHECK_INIT(unit);

    if (BCM_PBMP_IS_NULL(pbm)) {
        return BCM_E_NONE;
    }

    _bcm_robo_linkscan_update(unit, pbm);

    return BCM_E_NONE;
}

/*
 * Function:    
 *      _bcm_robo_linkscan_thread
 * Purpose:     
 *      Scan the ports on the specified unit for link status
 *      changes and process them as they occur.
 * Parameters:  
 *      unit - RoboSwitch unit #.
 * Returns:     
 *      Nothing
 */

STATIC void
_bcm_robo_linkscan_thread(int unit)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    sal_usecs_t         interval;
    int                 rv;
    soc_port_t      port;

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "Linkscan starting on unit %d\n"), unit));

    /* Re-initialize port bitmaps */
    _robo_lc_pbm_init(unit);
    
    sal_memset(lc->lc_error, 0, sizeof (lc->lc_error));

    PBMP_ITER(PBMP_PORT_ALL(unit), port) {
    lc->lc_error[port].limit =
        soc_property_port_get(unit, port,
                  spn_BCM_LINKSCAN_MAXERR, 5);
    lc->lc_error[port].delay =
        soc_property_port_get(unit, port,
                  spn_BCM_LINKSCAN_ERRDELAY, 10000000);
    }

    /* Clear initial value of forwarding ports. */

    rv = soc_link_fwd_set(unit, lc->lc_pbm_link);


#if defined(BCM_53115_A0) || defined(BCM_53118_A0) || defined(BCM_53101_A0) || \
    defined(BCM_53125_A0) || defined(BCM_53128_A0) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_DINO16_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    PBMP_ITER(lc->lc_pbm_link, port) {
        /* program MAC link (for Robo5396 only) :
         *   - Robo5396 port link will be controled by SW override.
         *   - Robo chip have no link mask register like Strata/XGS chip. 
         *      So it is not properly to program in soc_link_fwd_set().
         */
        if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO(unit)) {
            soc_robo_link_sw_update(unit, 1, port);
        }
    }
#endif

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Failed to clear forwarding ports: %s\n"), 
                   bcm_errmsg(rv)));
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                SOC_SWITCH_EVENT_THREAD_LINKSCAN, __LINE__, rv);
        sal_thread_exit(0);
    }

    /* Register for hardware linkscan interrupt. */

    rv = soc_linkscan_register(unit, _bcm_robo_linkscan_hw_interrupt);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Failed to register handler: %s\n"), 
                   bcm_errmsg(rv)));
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                SOC_SWITCH_EVENT_THREAD_LINKSCAN, __LINE__, rv);
        sal_thread_exit(0);
    }

    lc->lc_thread = sal_thread_self();

    while ((interval = lc->lc_us) != 0) {
        pbmp_t change;

        BCM_PBMP_ASSIGN(change, lc->lc_pbm_link_change); /* sample changed */

        _bcm_robo_linkscan_update(unit, PBMP_PORT_ALL(unit));

        if (!BCM_PBMP_IS_NULL(change)) {   /* Re-scan due to hardware force */
            continue;
        }

        if (BCM_PBMP_IS_NULL(lc->lc_pbm_sw)) {
            interval = sal_sem_FOREVER;
        }

        (void)sal_sem_take(lc->lc_sema, interval);
    }

    (void)soc_linkscan_register(unit, NULL);

    /* Before exiting, re-enable all ports that were being scanned. */

    BCM_PBMP_ITER(lc->lc_pbm_sw, port) {
        (void)bcm_port_enable_set(unit, port, TRUE);
    }

    BCM_PBMP_ITER(lc->lc_pbm_hw, port) {
        (void)bcm_port_enable_set(unit, port, TRUE);
    }

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "Linkscan exiting\n")));

    lc->lc_thread = NULL;
    sal_thread_exit(0);
}


/*
 * Function:    
 *      _bcm_robo_link_force
 * Purpose:
 *      Set linkscan's current link status for a port.
 * Parameters:  
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch physical port number.
 *      force - If TRUE, link status is forced to new link status.
 *              If FALSE, link status is no longer forced.
 *      link - New link status.
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_INIT - Not initialized.
 * Notes:
 *  When a link is forced up or down, linkscan will stop scanning
 *  that port and _bcm_link_get will always return the forced status.
 *  This is used for purposes such as when a link is placed in MAC
 *  loopback.  If software forces a link up, it is responsible for
 *  configuring that port.
 */

int
_bcm_robo_link_force(int unit, bcm_port_t port, int force, int link)
{
    ls_cntl_t   *lc = robo_link_control[unit];
    pbmp_t  pbm;

    LC_CHECK_INIT(unit);

    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    LC_LOCK(unit);

    if (force) {
        BCM_PBMP_PORT_REMOVE(lc->lc_pbm_override_link, port);
    if (link) {
        BCM_PBMP_PORT_ADD(lc->lc_pbm_override_link, port);
    }
        BCM_PBMP_PORT_ADD(lc->lc_pbm_override_ports, port);
    } else {
        BCM_PBMP_PORT_REMOVE(lc->lc_pbm_override_ports, port);
        BCM_PBMP_PORT_REMOVE(lc->lc_pbm_override_link, port);
        BCM_PBMP_PORT_ADD(lc->lc_pbm_link_change, port);
    }

    /*
     * Force immediate update to just this port - this allows loopback 
     * forces to take effect immediately.
     */
    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_ADD(pbm, port);

    _bcm_robo_linkscan_update(unit, pbm);
    LC_UNLOCK(unit);
    /*
     * Wake up master thread to notice changes - required if using hardware
     * link scanning.
     */
    if (lc->lc_sema != NULL) {
        sal_sem_give(lc->lc_sema);
    }

    return(BCM_E_NONE);
}

/************************************************************************
 *********                                                      *********
 *********         Start of BCM API Exported Routines           *********
 *********                                                      *********
 ************************************************************************/

/*
 * Function:
 *      bcm_robo_linkscan_init
 * Purpose:
 *      Initialize the linkscan software module.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_init(int unit)
{
    ls_cntl_t   *lc;

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_init()..\n")));
    if (robo_link_control[unit] != NULL) {
        BCM_IF_ERROR_RETURN(bcm_linkscan_detach(unit));
    }

    if ((lc = sal_alloc(sizeof (ls_cntl_t), "link_control")) == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(lc, 0, sizeof (ls_cntl_t));

    lc->lc_lock = sal_mutex_create("bcm_link_LOCK");
    if (lc->lc_lock == NULL) {
        sal_free(lc);
        return BCM_E_MEMORY;
    }

    lc->lc_sema = sal_sem_create("bcm_link_SLEEP", 
                 sal_sem_BINARY, 0);
    if (lc->lc_sema == NULL) {
        sal_mutex_destroy(lc->lc_lock);
        sal_free(lc);
        return BCM_E_MEMORY;
    }

    robo_link_control[unit] = lc;

    /*
     * Initialize link_control port bitmaps so bcm_port_update works
     * reasonably even if the linkscan thread is never started.
     */

    _robo_lc_pbm_init(unit);

    return BCM_E_NONE;
}   

/*
 * Function:
 *  bcm_robo_linkscan_detach
 * Purpose:
 *  Prepare linkscan module to detach specified unit.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 * Returns:
 *  BCM_E_NONE - detach successful.
 *  BCM_E_XXX - detach failed.
 * Notes:
 *  This is safe to call at any time, but linkscan should only be
 *  initialized or detached from the main application thread.
 */
int bcm_robo_linkscan_detach(int unit)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    ls_handler_t        *lh;
    pbmp_t      empty_pbm;

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_detach()..\n")));
    if (lc == NULL) {
        return BCM_E_NONE;
    }

    BCM_PBMP_CLEAR(empty_pbm);

    BCM_IF_ERROR_RETURN(soc_linkscan_config(unit, empty_pbm, empty_pbm));

    BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0));

    /* Clean up list of handlers */

    while (lc->lc_handler != NULL) {
        lh = lc->lc_handler;
        lc->lc_handler = lh->lh_next;
            sal_free(lh);
    }

    /* Mark and not initialized and free mutex */

    if (lc->lc_sema != NULL) {
        sal_sem_destroy(lc->lc_sema);
        lc->lc_sema = NULL;
    }

    if (lc->lc_lock != NULL) {
        sal_mutex_destroy(lc->lc_lock);
        lc->lc_lock = NULL;
    }

    robo_link_control[unit] = NULL;

    sal_free(lc);

    return(BCM_E_NONE);
}   
    
/*
 * Function:    
 *      bcm_robo_linkscan_enable_set(
 * Purpose:
 *      Enable or disable the link scan feature.
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      us - Specifies the software polling interval in micro-seconds;
 *      the value 0 disables linkscan.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_enable_set(int unit, int us)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    int                 rv = BCM_E_NONE;
    soc_timeout_t       to;

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_enable_set()..\n")));
    if (!us && lc == NULL) {    /* No error to disable if not inited */
        return(BCM_E_NONE);
    }

    LC_CHECK_INIT(unit);

    if (!soc_property_get(unit, "reg_write_log", FALSE)) {
        sal_snprintf(lc->lc_taskname,
             sizeof (lc->lc_taskname),
             "bcmLINK.%d",
             unit);
    }

    if (us) {
        if (us < BCM_LINKSCAN_INTERVAL_MIN) {
            us = BCM_LINKSCAN_INTERVAL_MIN;
        }

        lc->lc_us = us;

        if (lc->lc_thread != NULL) {
            /* Linkscan is already running; just update the period */
            sal_sem_give(lc->lc_sema);
        } else if (sal_thread_create(lc->lc_taskname,
                    SAL_THREAD_STKSZ,
                    soc_property_get(unit, spn_LINKSCAN_THREAD_PRI,50),
                    (void (*)(void*))_bcm_robo_linkscan_thread, 
                    INT_TO_PTR(unit)) == SAL_THREAD_ERROR) {
            lc->lc_us = 0;
            rv = BCM_E_MEMORY;
        } else {
            soc_timeout_init(&to, 3000000, 0);

            while (lc->lc_thread == NULL) {
                if (soc_timeout_check(&to)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "%s: Thread did not start\n"),
                               lc->lc_taskname));
                    lc->lc_us = 0;
                    rv = BCM_E_INTERNAL;
                    break;
                }
            }
        }
    } else if (lc->lc_thread != NULL) {
        lc->lc_us = 0;

        sal_sem_give(lc->lc_sema);

        soc_timeout_init(&to, 10000000, 0);   /* Enough time for Quickturn */

        while (lc->lc_thread != NULL) {
            if (soc_timeout_check(&to)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Thread did not exit\n"),
                           lc->lc_taskname));
                rv = BCM_E_INTERNAL;
                break;
            }
        }
    }

    return(rv);
}   
    
/*
 * Function:    
 *      bcm_robo_linkscan_enable_get
 * Purpose:
 *      Retrieve the current linkscan mode.
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      us - (OUT) Pointer to microsecond scan time for software scanning, 
 *              0 if not enabled.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_enable_get(int unit, int *us)
{
    LC_CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_enable_get()..\n")));
    *us = robo_link_control[unit]->lc_us;

    return(BCM_E_NONE);
}   
    
/*
 * Function:
 *  bcm_robo_linkscan_enable_port_get
 * Purpose:
 *  Determine whether or not linkscan is managing a given port
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      port - Port to check.
 * Returns:
 *  BCM_E_NONE - port being scanned
 *  BCM_E_DISABLED - port not being scanned
 */
int bcm_robo_linkscan_enable_port_get(int unit, bcm_port_t port)
{
    ls_cntl_t           *lc = robo_link_control[unit];

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_enable_get()..port=0x%x\n"),
              port));
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (lc == NULL || lc->lc_us == 0 ||
                    (!BCM_PBMP_MEMBER(lc->lc_pbm_sw, port) &&
                     !BCM_PBMP_MEMBER(lc->lc_pbm_hw, port) &&
                     !BCM_PBMP_MEMBER(lc->lc_pbm_override_ports, port))) {
        return (BCM_E_DISABLED);
    }

    return (BCM_E_NONE);
}   

/*
 * Function:    
 *      bcm_robo_linkscan_mode_set
 * Purpose:
 *      Set the current scanning mode for the specified port.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch Port #.
 *      mode - New scan mode for specified port.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_mode_set(int unit, bcm_port_t port, int mode)
{
    ls_cntl_t   *lc = robo_link_control[unit];
    int         rv = BCM_E_NONE;
    pbmp_t  empty_pbm;
    int     added = 0;

    LC_CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_mode_set()..port=%d,mode=%d\n"),
              port, mode));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_robo_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }
    BCM_PBMP_CLEAR(empty_pbm);

    LC_LOCK(unit);

    /* First, remove from current configuration */
    BCM_PBMP_PORT_REMOVE(lc->lc_pbm_sw, port);
    BCM_PBMP_PORT_REMOVE(lc->lc_pbm_hw, port);

    /* Now add back to proper map */
    switch (mode) {
    case BCM_LINKSCAN_MODE_NONE:
        break;
    case BCM_LINKSCAN_MODE_SW:
        BCM_PBMP_PORT_ADD(lc->lc_pbm_sw, port);
        added = 1;
        break;
    case BCM_LINKSCAN_MODE_HW:
        BCM_PBMP_PORT_ADD(lc->lc_pbm_hw, port);
        added = 1;
        break;
    default:
        break;
    }

    /* Reconfigure HW linkscan in case changed */
    rv = soc_linkscan_config(unit, lc->lc_pbm_hw, empty_pbm);

    LC_UNLOCK(unit);

    if (lc->lc_sema != NULL) {
        sal_sem_give(lc->lc_sema);  /* register change now */
    }

    /* When no longer scanning a port, return it to the enabled state. */
    if (BCM_SUCCESS(rv) && !added) {
        rv = bcm_port_enable_set(unit, port, TRUE);
    }

    return(rv);
}   
    
/*
 * Function:    
 *      bcm_linkscan_mode_set_pbm
 * Purpose:
 *      Set the current scanning mode for the specified ports.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      pbm  - Port bit map indicating port to set.
 *      mode - New scan mode for specified ports.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_mode_set_pbm(int unit, bcm_pbmp_t pbm, int mode)
{
    bcm_port_t  port;

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_mode_set_pbm()..\n")));
    LC_CHECK_INIT(unit);

    PBMP_ITER(pbm, port) {
        BCM_IF_ERROR_RETURN
            (bcm_linkscan_mode_set(unit, port, mode));
    }

    return (BCM_E_NONE);
}   
    

/*
 * Function:    
 *      bcm_robo_linkscan_mode_get
 * Purpose:
 *      Recover the current scanning mode for the specified port.
 * Parameters:
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch Port #.
 *      mode - (OUT) current scan mode for specified port.
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_linkscan_mode_get(int unit, bcm_port_t port, int *mode)
{
    ls_cntl_t   *lc = robo_link_control[unit];

    LC_CHECK_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_robo_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }
    if (NULL == mode) {
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_mode_get()..\n")));
    if (PBMP_MEMBER(lc->lc_pbm_hw, port)) {
        *mode = BCM_LINKSCAN_MODE_HW;
    } else if (PBMP_MEMBER(lc->lc_pbm_sw, port)) {
        *mode = BCM_LINKSCAN_MODE_SW;
    } else {
        *mode = BCM_LINKSCAN_MODE_NONE;
    }

    return(BCM_E_NONE);
}   
    
/*
 * Function:    
 *      bcm_robo_linkscan_register
 * Purpose:
 *      Register a handler to be called when a link status change is noticed.
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      f - pointer to function to call when link status change is seen
 * Returns:
 *      BCM_E_XXX
 */

int bcm_robo_linkscan_register(int unit, bcm_linkscan_handler_t f)
{
    ls_cntl_t   *lc = robo_link_control[unit];
    ls_handler_t *lh;
    int found = FALSE;

    LC_CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_register()..\n")));
    /* First, see if this handler already registered */
    LC_LOCK(unit);

    for (lh = lc->lc_handler; lh != NULL; lh = lh->lh_next) {
        if (lh->lh_f == f) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        LC_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if ((lh = sal_alloc(sizeof(*lh), "bcm_linkscan_register")) == NULL) {
        LC_UNLOCK(unit);
        return(BCM_E_MEMORY);
    }

    lh->lh_next = lc->lc_handler;
    lh->lh_f = f;
    lc->lc_handler = lh;

    LC_UNLOCK(unit);

    return(BCM_E_NONE);
}   
    
/*
 * Function:
 *      bcm_robo_linkscan_unregister
 * Purpose:
 *      Remove a previously registered handler from the callout list.
 * Parameters:
 *  unit - roboSwitch unit #.
 *  f    - pointer to function registered in call to 
 *             bcm_linkscan_register.
 * Returns:
 *      BCM_E_NOT_FOUND     Could not find matching handler
 *  BCM_E_NONE      Success
 */
int bcm_robo_linkscan_unregister(int unit, bcm_linkscan_handler_t f)
{
    ls_cntl_t   *lc = robo_link_control[unit];
    ls_handler_t *lh, *p;

    LC_CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_linkscan_unregister()..\n")));
    LC_LOCK(unit);

    for (p = NULL, lh = lc->lc_handler; lh; p = lh, lh = lh->lh_next) { 
        if (lh->lh_f == f) {
            if (p == NULL) {
                lc->lc_handler = lh->lh_next;
            } else {
                p->lh_next = lh->lh_next;
            }
            break;
        }
    }

    LC_UNLOCK(unit);

    if (lh != NULL) {
        sal_free(lh);
        return BCM_E_NONE;
    } else {
        return BCM_E_NOT_FOUND;
    }
}   
    

/*
 * Function:    
 *      bcm_robo_link_wait
 * Purpose:
 *      Wait for all links in the mask to be "link up".
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      pbm - (IN/OUT) Port bit map to wait for, mask of those link up on 
 *              return.
 *      us - number of microseconds to wait.
 * Returns:
 *      BCM_E_NONE - all links are ready.
 *      BCM_E_TIMEOUT - not all links ready in specified time.
 *  BCM_E_DISABLED - linkscan not running on one or more of the ports.
 */
int bcm_robo_link_wait(int unit, bcm_pbmp_t *pbm, int us)
{
    ls_cntl_t           *lc = robo_link_control[unit];
    soc_timeout_t       to;
    pbmp_t      sofar_pbm;
    soc_port_t      port;

    /* Input parameters check. */
    if ((NULL == pbm) || (us < 0)) {
        return (BCM_E_PARAM);
    }

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_link_wait()..pbm=0x%x\n"),
              SOC_PBMP_WORD_GET(*pbm, 0)));
    BCM_PBMP_ITER(*pbm, port) {
        BCM_IF_ERROR_RETURN
            (bcm_linkscan_enable_port_get(unit, port));
    }

    /*
     * If a port was just configured, it may have gone down but
     * lc_pbm_link may not reflect that until the next time linkscan
     * runs.  This is avoided by forcing an update of lc_pbm_link.
     */

    _bcm_robo_linkscan_update(unit, *pbm);

    soc_timeout_init(&to, (sal_usecs_t)us, 0);

    for (;;) {
        BCM_PBMP_ASSIGN(sofar_pbm, lc->lc_pbm_link);
        BCM_PBMP_AND(sofar_pbm, *pbm);
        if (BCM_PBMP_EQ(sofar_pbm, *pbm)) {
            break;
        }

        if (soc_timeout_check(&to)) {
            BCM_PBMP_AND(*pbm, lc->lc_pbm_link);
            return (BCM_E_TIMEOUT);
        } 

        sal_usleep(lc->lc_us / 4);
    }

    return (BCM_E_NONE);
}   

/*
 * Function:    
 *         bcm_robo_link_change
 * Purpose:
 *         Force a transient link down event to be recognized,
 *         regardless of the current physical up/down state of the
 *         port.  This does not affect the physical link status. 
 * Parameters:  
 *         unit - RoboSwitch Unit #.
 *         pbm - Bitmap of ports to operate on.
 * Returns:
 *         BCM_E_XXX
 */

int
bcm_robo_link_change(int unit, pbmp_t pbm)
{
    ls_cntl_t   *lc = robo_link_control[unit];

    LC_CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_link_change()..pbm=0x%x\n"),
              SOC_PBMP_WORD_GET(pbm, 0)));
    BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));

    LC_LOCK(unit);
    BCM_PBMP_OR(lc->lc_pbm_link_change, pbm);
    LC_UNLOCK(unit);

    /*
     * Wake up master thread to notice changes - required if using hardware
     * link scanning.
     */
    if (lc->lc_sema != NULL) {
        sal_sem_give(lc->lc_sema);
    }

    return(BCM_E_NONE);
}

/*
 * Function:    
 *      _bcm_robo_link_get
 * Purpose:
 *      Return linkscan's current link status for the given port.
 * Parameters:  
 *      unit - RoboSwitch Unit #.
 *      port - RoboSwitch physical port number.
 *      link - (OUT) Current link status.
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_DISABLED - Port not being scanned.
 * Note:
 *    This routine does not acquire the LC_LOCK, as it only reads a 
 *    snapshot of the link bitmaps.  It also cannot hold the LC_LOCK 
 *    since it is called indirectly from the linkscan thread 
 *    when requesting port info.
 */

int
_bcm_robo_link_get(int unit, bcm_port_t port, int *link)
{
    ls_cntl_t   *lc = robo_link_control[unit];

    if (SOC_PBMP_MEMBER(lc->lc_pbm_override_ports, port)) {
        *link =  SOC_PBMP_MEMBER(lc->lc_pbm_override_link, port);
        return (SOC_E_NONE);
    }

    BCM_IF_ERROR_RETURN(bcm_linkscan_enable_port_get(unit, port));

    *link = SOC_PBMP_MEMBER(lc->lc_pbm_link, port);

    return(BCM_E_NONE);
}
/*
 * Function:
 *      bcm_robo_linkscan_port_register
 * Purpose:
 *      Register a handler to be called when a link status
 *      is to be determined by a caller provided function.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      port - RoboSwitch port number.
 *      f - pointer to function to call for true link status
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *  This function works with software linkscan only.  
 */
int
bcm_robo_linkscan_port_register(int unit, bcm_port_t port,
                               bcm_linkscan_port_handler_t f)
{
    ls_cntl_t   *lc = robo_link_control[unit];
  
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_robo_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    LC_CHECK_INIT(unit);

    LC_LOCK(unit);
    lc->lc_f[port] = f;
    LC_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_linkscan_port_unregister
 * Purpose:
 *      Remove a previously registered handler that is used
 *      for setting link status. 
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      port - RoboSwitch port number.
 *      f    - pointer to function registered in call to 
 *             bcm_linkscan_port_register.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_linkscan_port_unregister(int unit, bcm_port_t port,
                                 bcm_linkscan_port_handler_t f)
{
    ls_cntl_t   *lc = robo_link_control[unit];
    int rv;
    
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_robo_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    LC_CHECK_INIT(unit);

    LC_LOCK(unit);
    if (f == lc->lc_f[port]) {
        lc->lc_f[port] = NULL;
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_NOT_FOUND;
    }
    LC_UNLOCK(unit);

    return rv;
}
#if defined(BROADCOM_DEBUG)
int
bcm_robo_linkscan_dump(int unit)
{
    ls_handler_t *ent;

    for (unit = 0; unit < 3; unit++) {
        if (robo_link_control[unit] == NULL) {
            LOG_CLI((BSL_META_U(unit,
                                "BCM linkscan not initialized for unit %d\n"), unit));
            continue;
        }

        LOG_CLI((BSL_META_U(unit,
                            "BCM linkscan callbacks for unit %d\n"), unit));
        for (ent = robo_link_control[unit]->lc_handler; ent != NULL;
             ent = ent->lh_next) {
            LOG_CLI((BSL_META_U(unit,
                                "    Fn 0x%08x\n"), PTR_TO_INT(ent->lh_f)));
        }
    }

    return BCM_E_NONE;
}
#endif  /* BROADCOM_DEBUG */
