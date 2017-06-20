/*
 * $Id: phy5482robo.c,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        phy5482.c
 * Purpose:     PHY driver for BCM5482 and BCM5482S
 *
 * Supported BCM546X Family of PHY devices:
 *
 *      Device  Ports   Media                           MAC Interface
 *      5482    2       Copper, Serdes                  SGMII
 *      5482S   2       Copper, Serdes                  SGMII
 *
 * Workarounds:
 *
 * References:
 * MCS5482S-DS200R    BCM5482S Preliminary Data Sheet
 * 5482S-AN103-R      BCM5482S Configuration Guide for SFP Module with
 *                    Optical/Copper Applications
 * 5482S-ES101-R      BCM5482S Errata Sheet
 * 5482_5482S-AN102-R Designing with BCM5482/BCM5482S
 *     
 *    +----------+   +---------------------------+
 *    |          |   |  SGMII     +->Copper      |<--> Magnetic
 *    |  SGMII   |<->| (Primary <-+              |
 *    | (SerDes) |   |  SerDes)   |              |
 *    |          |   |            +->SGMII/      |<--> (PHY)SFP (10/100/1000)
 *    +----------+   |               1000BASE-X/ |<--> 1000BASE-X SFP
 *        MAC        |               100BASE-FX  |<--> 100BASE-FX SFP
 *                   |               (Secondary  |
 *                   |                SerDes)    |
 *                   +---------------------------+
 *                               5482S
 *
 * Notes:
 * 1. Media auto-negotiation and MAC SGMII auto-negotiation must be enabled 
 *    to automatically detect between
 *    SGMII SFP and 1000BASE-X SFP.
 * 2. Auto detecting 100FX SFP is not supported. 
 * 3. To use 100BASE-FX,
 *       First, insert the 100BASE-FX SFP and connect the fiber cable. 
 *       The PHY will detect energy on fiber side and switch to fiber mode.
 *       Second, disable autoneg mode.
 *       Third, force speed to 100. Since the PHY is in fiber mode, forcing
 *       speed to 100 will put the PHY to 100BASE-FX mode.
 * 4. Forcing speed on the 5482S PHY with SGMII SFP is not supported in this
 *    driver. To use SGMII SFP, autoneg must be enabled.
 * 5. Some SFP continues to provide energy to the PHY even when the media 
 *    connected is removed. For those SFPs, the PHY will not switch to
 *    copper medium in combo mode even when there is no link on the fiber 
 *    medium because the PHY is not switching to copper mode.  
 * 6. If the MAC/SerDes attached to the PHY support SGMII autoneg, SGMII
 *    autoneg should be enabled with "config add phy_sgmii_autoneg_ge0=1".
 * 7. If the SFP module report loss of signal instead of signal detect, the
 *    signal detect should be inverted with "config add phy_fiber_detect=-2".
 */                        

#include <sal/types.h>
#include <sal/core/spl.h>
#include <shared/bsl.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */

#if defined(INCLUDE_PHY_5482_ROBO)
#include "phyconfig.h"    /* Must be the first phy include after phydefs.h */
#include "phyident.h"
#include "phyreg.h"
#include "phyfege.h"
#include "phy5482.h"


#define AUTO_MDIX_WHEN_AN_DIS   0       /* Non-standard-compliant option */
#define DISABLE_CLK125          0       /* Disable if unused to reduce */

#define ADVERT_ALL_COPPER \
        (SOC_PM_PAUSE | SOC_PM_10MB | SOC_PM_100MB | SOC_PM_1000MB)
#define ADVERT_ALL_FIBER \
        (SOC_PM_PAUSE | SOC_PM_1000MB_FD)

STATIC int _robo_phy_5482_no_reset_setup(int unit, soc_port_t port);
STATIC int _robo_phy_5482_medium_change(int unit, soc_port_t port,
                                        int force_update);
STATIC int robo_phy_5482_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int robo_phy_5482_speed_set(int unit, soc_port_t port, int speed);
STATIC int robo_phy_5482_master_set(int unit, soc_port_t port, int master);
STATIC int robo_phy_5482_adv_local_set(int unit, soc_port_t port,
                                  soc_port_mode_t mode);
STATIC int robo_phy_5482_autoneg_set(int unit, soc_port_t port, int autoneg);
STATIC int robo_phy_5482_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode);
STATIC int robo_phy_5482_speed_get(int unit, soc_port_t port, int *speed);
STATIC int _robo_phy_5482_fiber_100fx_setup(int unit, soc_port_t port);
STATIC int _robo_phy_5482_fiber_1000x_setup(int unit, soc_port_t port);
/***********************************************************************
 *
 * HELPER FUNCTIONS
 *
 ***********************************************************************/
/*
 * Function:
 *      _robo_phy_5482_medium_check
 * Purpose:
 *      Determine if chip should operate in copper or fiber mode.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      medium - (OUT) SOC_PORT_MEDIUM_XXX
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_robo_phy_5482_medium_check(int unit, soc_port_t port, int *medium)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);
    uint16         tmp = 0xffff;

    *medium = SOC_PORT_MEDIUM_COPPER;

    if (pc->automedium) {
        /* Read Mode Register (0x1c shadow 11111) */
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_MODE_CTRLr(unit, pc, &tmp));

        if (pc->fiber.preferred) {
            /* 0x10 Fiber Signal Detect
             * 0x20 Copper Energy Detect
             */
            if ((tmp & 0x30) == 0x20) {
                /* Only copper is link up */
                *medium = SOC_PORT_MEDIUM_COPPER;
            } else {
                *medium = SOC_PORT_MEDIUM_FIBER;
            }
        } else {
            if ((tmp & 0x30) == 0x10) {
                /* Only fiber is link up */
                *medium = SOC_PORT_MEDIUM_FIBER;
            } else {
                *medium = SOC_PORT_MEDIUM_COPPER;
            }
        }
    } else {
        *medium = pc->fiber.preferred ? SOC_PORT_MEDIUM_FIBER:
                  SOC_PORT_MEDIUM_COPPER;
    }

    LOG_VERBOSE(BSL_LS_SOC_PHY,
                (BSL_META_U(unit,
                            "_robo_phy_5482_medium_check: "
                            "u=%d p=%d fiber_pref=%d 0x1c(11111)=%04x fiber=%d\n"),
                 unit, port, pc->fiber.preferred, tmp, 
                 (*medium == SOC_PORT_MEDIUM_FIBER) ? 1 : 0));

    return SOC_E_NONE;
}


/*
 * Function:
 *      robo_phy_5482_medium_status
 * Purpose:
 *      Indicate the current active medium
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      medium - (OUT) One of:
 *              SOC_PORT_MEDIUM_COPPER
 *              SOC_PORT_MEDIUM_FIBER
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
robo_phy_5482_medium_status(int unit, soc_port_t port, soc_port_medium_t *medium)
{
    if (PHY_COPPER_MODE(unit, port)) {
        *medium = SOC_PORT_MEDIUM_COPPER;
    } else {
        *medium = SOC_PORT_MEDIUM_FIBER;
    }
    return SOC_E_NONE;
}
/*
 * Function:
 *      _robo_phy_5482_medium_config_update
 * Purpose:
 *      Update the PHY with config parameters
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      cfg - Config structure.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_robo_phy_5482_medium_config_update(int unit, soc_port_t port,
                                soc_phy_config_t *cfg)
{
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_speed_set(unit, port, cfg->force_speed));
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_duplex_set(unit, port, cfg->force_duplex));
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_master_set(unit, port, cfg->master));
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_adv_local_set(unit, port, cfg->autoneg_advert));
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_autoneg_set(unit, port, cfg->autoneg_enable));
    SOC_IF_ERROR_RETURN
        (robo_phy_5482_mdix_set(unit, port, cfg->mdix));

    return SOC_E_NONE;
}


/***********************************************************************
 *
 * PHY5482 DRIVER ROUTINES
 *
 ***********************************************************************/

/*
 * Function:
 *      robo_phy_5482_medium_config_set
 * Purpose:
 *      Set the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - Switch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - Operating parameters
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
robo_phy_5482_medium_config_set(int unit, soc_port_t port, 
                           soc_port_medium_t  medium,
                           soc_phy_config_t  *cfg)
{
    phy_ctrl_t    *pc;
    soc_phy_config_t *active_medium;  /* Currently active medium */
    soc_phy_config_t *change_medium;  /* Requested medium */
    soc_phy_config_t *other_medium;   /* The other medium */
    int               medium_update;
    soc_port_mode_t   advert_mask;
    int             fiber_action = 0;    /* for fiber 100/1000 speed set */

    if (NULL == cfg) {
        return SOC_E_PARAM;
    }

    pc            = EXT_PHY_SW_STATE(unit, port);
    medium_update = FALSE;
    switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        if (!pc->automedium) {
            if (!PHY_COPPER_MODE(unit, port)) {
                return SOC_E_UNAVAIL;
            }
            /* check if device is fiber capable before switching */
            if (cfg->preferred == 0) {
                uint16 data;

                SOC_IF_ERROR_RETURN
                    (READ_PHY5482_MODE_CTRLr(unit, pc, &data));

                /* return if not fiber capable*/
                if (!(data & 0x8)) {
                    return SOC_E_UNAVAIL;
                }
            }
        }

        change_medium  = &pc->copper;
        other_medium   = &pc->fiber;
        advert_mask    = ADVERT_ALL_COPPER;
        break;
    case SOC_PORT_MEDIUM_FIBER:
        if (!pc->automedium && !PHY_FIBER_MODE(unit, port)) {
            return SOC_E_UNAVAIL;
        }
        change_medium  = &pc->fiber;
        other_medium   = &pc->copper;
        advert_mask    = ADVERT_ALL_FIBER;
        fiber_action = TRUE;
        break;
    default:
        return SOC_E_PARAM;
    }

    /*
     * Changes take effect immediately if the target medium is active or
     * the preferred medium changes.
     */
    if (change_medium->enable != cfg->enable) {
        medium_update = TRUE;
    }
    if (change_medium->preferred != cfg->preferred) {
        /* Make sure that only one medium is preferred */
        other_medium->preferred = !cfg->preferred;
        medium_update = TRUE;
    }

    sal_memcpy(change_medium, cfg, sizeof(*change_medium));
    change_medium->autoneg_advert &= advert_mask;

    if (medium_update) {
        /* The new configuration may cause medium change. Check
         * and update medium.
         */
        SOC_IF_ERROR_RETURN
            (_robo_phy_5482_medium_change(unit, port,TRUE));
    } else {
        active_medium = (PHY_COPPER_MODE(unit, port)) ?  
                            &pc->copper : &pc->fiber;
       /* If user is setting the speed to 100 or 1000 on the fiber medium 
         *  which is not in active status, the internal routine for 
         *  phy5482_100fx_setup() or phy5482_1000x_setup() might not been 
         *  processed(for (active_medium == change_medium).
         * This case might causes problem espacially for 100FX setting but the 
         *  device within copper(linked) and fiber(100FX plugged) both connect 
         *  to the target port at the same time. 
         */
        if (active_medium == change_medium) {
            /* If the medium to update is active, update the configuration */
            SOC_IF_ERROR_RETURN
                (_robo_phy_5482_medium_config_update(unit, port, change_medium));
        } else {
            /* means the target medium is fiber */
            if (fiber_action){
                /* following section is for phy5482 only right now */
                if (change_medium->force_speed == 100) {
                    SOC_IF_ERROR_RETURN(
                            _robo_phy_5482_fiber_100fx_setup(unit, port));
                } else { /* else set to 1000 */
                    SOC_IF_ERROR_RETURN(
                            _robo_phy_5482_fiber_1000x_setup(unit, port));
                }
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_medium_config_get
 * Purpose:
 *      Get the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - Switch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - (OUT) Operating parameters
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
robo_phy_5482_medium_config_get(int unit, soc_port_t port, 
                           soc_port_medium_t medium,
                           soc_phy_config_t *cfg)
{
    int            rv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        if (pc->automedium || PHY_COPPER_MODE(unit, port)) {
            sal_memcpy(cfg, &pc->copper, sizeof (*cfg));
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
    case SOC_PORT_MEDIUM_FIBER:
        if (pc->automedium || PHY_FIBER_MODE(unit, port)) {
            sal_memcpy(cfg, &pc->fiber, sizeof (*cfg));
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}

/*
 * Function:
 *      _robo_phy_5482_reset_setup
 * Purpose:
 *      Function to reset the PHY and set up initial operating registers.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      Configures for copper/fiber according to the current
 *      setting of PHY_FLAGS_FIBER.
 */

STATIC int
_robo_phy_5482_reset_setup(int unit, soc_port_t port)
{
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    /* Reset the PHY with previously registered callback function. 
     */
    SOC_IF_ERROR_RETURN(soc_phy_reset(unit, port));

    /* Reset the secondary SerDes         */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, MII_CTRL_RESET));
    sal_usleep(10000);

    /* Disable super-isolate */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_MII_POWER_CTRLr(unit, pc, 0x0, 1U<<5));

    SOC_IF_ERROR_RETURN
        (_robo_phy_5482_no_reset_setup(unit, port));

    return SOC_E_NONE;
}

STATIC int
_robo_phy_5482_no_reset_setup(int unit, soc_port_t port)
{
    phy_ctrl_t    *int_pc;
    phy_ctrl_t    *pc;
    uint16         data, mask;
    uint16 dev_id = 0;
    uint8 rev_id = 0;

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "_robo_phy_5482_no_reset_setup: u=%d p=%d medium=%s\n"),
                         unit, port,
              PHY_COPPER_MODE(unit, port) ? "COPPER" : "FIBER"));

    pc      = EXT_PHY_SW_STATE(unit, port);
    int_pc  = INT_PHY_SW_STATE(unit, port);
    /* SGMII interface to MAC */
    /* soc_phy_reset has reset the phy, powering down the unstrapped interface.
     * Make sure enabled interfaces are powered up. 
     */
    data = 0;
    if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
        data = pc->fiber.enable? 0: MII_CTRL_PD;
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "%s: u=%d p=%d Pri-SerDes PowerDown=%d!\n"),
                         FUNCTION_NAME(), unit, port, 0));
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, 0, MII_CTRL_PD));

    /* 1000Base-X FullDuplex will be clear after the phy been 
     * reset in phy5482s 
     */
    data |= MII_CTRL_FD | MII_CTRL_SS_1000 | MII_CTRL_RAN | MII_CTRL_AE;

    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_1000X_MII_CTRLr(unit, pc, data));
   
    /* copper regs */
    if (!pc->copper.enable || (!pc->automedium && pc->fiber.preferred)) {
        /* Copper interface is not used. Powered down. */
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s: u=%d p=%d Copper PowerDown=%d!\n"),
                             FUNCTION_NAME(), unit, port, 1));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_MII_CTRLr(unit, pc, MII_CTRL_PD, MII_CTRL_PD));
    } else {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s: u=%d p=%d Copper PowerDown=%d!\n"),
                             FUNCTION_NAME(), unit, port, 0));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_MII_CTRLr(unit, pc, 0, MII_CTRL_PD));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_MII_GB_CTRLr(unit, pc,  
                            MII_GB_CTRL_ADV_1000FD | MII_GB_CTRL_PT));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_MII_CTRLr(unit, pc, 
                MII_CTRL_FD | MII_CTRL_SS_1000 | MII_CTRL_AE));
    }
          
    /* Adjust timing */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_SPARE_CTRLr(unit, pc, 0x0002, 0x0002));

    /*
     * For BCM53242B0, workaround of giga phy timing adjustment. 
     * Register a phy_reset callback function to do the workaround. 
     * When phy_reset is invoked, WAR will be applied
     */
    if (SOC_IS_ROBO53242(unit)) {
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_B0_REV_ID)
            && IS_GE_PORT(unit, port)){
            /* For BCM53242B0, workaround of giga phy timing adjustment. */
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_AUX_REG, 0xf0e7));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_GSR_REG, 0x8c2b));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_GSR_REG, 0x8d2b));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_GSR_REG, 0x8c29));                
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_GSR_REG, 0x8d29));                
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, MII_GSR_REG, 0x8e29));      
        }
    }

    if (PHY_SGMII_AUTONEG_MODE(unit, port)) {
        /* Disable parallel detect on SGMII interface with MAC to elimates
         * the possibility of the BCM5482S turning off the SGMII auto-neg 
         * interface connected to the MAC inadvertly.
         * Do not disable parallel detect if SGMII autoneg is disabled. 
         * Otherwise, the PHY will not able to detect the configuration
         * of MAC SerDes.
         */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_1ST_SERDES_CTRLr(unit, pc, 0));
    }

    if (NULL != int_pc) {
        SOC_IF_ERROR_RETURN
            (PHY_INIT(int_pc->pd, unit, port));
        } 


    if (PHY_SECONDARY_SERDES_MODE(unit, port)){
        /* Power down secondary SerDes */
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s: u=%d p=%d Sec-SerDes PowerDown=%d!\n"),
                             FUNCTION_NAME(), unit, port, 1));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, MII_CTRL_PD, 
                                               MII_CTRL_PD));
        if (pc->fiber.enable && (pc->automedium || pc->fiber.preferred)) {
            if (pc->fiber.force_speed == 1000) {
                /* Secondary SerDes Control Register
                 * Select secondary serdes 1000BaseX in full duplex
                 */
                data  = 1 << 0;     /* Enable secondary SerDes */
                data |= 1 << 2;     /* Select Sync function from secondary SerDes */ 
                data |= 1 << 3;     /* Enable secondary SerDes to driver LEDs     */
                data |= 1 << 5;     /* Enable secondary SerDes 100FX full duplex  */
            } else {
                /* Secondary SerDes Control Register
                 * Select secondary serdes 100BaseX 
                 */
                data  = 1 << 0;     /* Enable secondary SerDes */
                data |= 1 << 2;     /* Select Sync function from secondary SerDes */ 
                data |= 1 << 3;     /* Enable secondary SerDes to driver LEDs     */
                data |= 1 << 4;     /* Operate secondary SerDes in 100Base-FX mode */
                data |= 1 << 5;     /* Enable secondary SerDes 100FX full duplex  */
            }

            SOC_IF_ERROR_RETURN
                (WRITE_PHY5482_2ND_SERDES_CTRLr(unit, pc, data));
        
            /* remove power down of secondary SerDes */
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "%s: u=%d p=%d Sec-SerDes PowerDown=%d!\n"),
                                 FUNCTION_NAME(), unit, port, 0));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, 0, MII_CTRL_PD));

            /* Enable auto-detection between SGMII-slave and 1000BASE-X */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_2ND_SERDES_SGMII_SLAVEr(unit, pc, 1, 1));

            /* set the advertisement of secondary serdes */
            /* Disable half-duplex, full-duplex capable */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_2ND_SERDES_ANAr(unit, pc, (1 << 5), 
                                                (1 << 5) | (1 << 6)));
      
            /* Restart secondary SerDes autonegotiation */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc,
                    MII_CTRL_FD | MII_CTRL_SS_1000 | MII_CTRL_AE));
        }
    }
    
    switch (pc->fiber_detect) {
        case  2:         /* LED 2 */
        case -2:
        case  4:         /* LED 4 : kept for backward compatibility */
        case -4:
        case  10:        /* EN_10B for signal detect */
        case -10:
        case 0:         /* default */
            /* GPIO Control/Status Register (0x1c shadow 01111) */
            /* Change LED2 pin into an input */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_LED_GPIO_CTRLr(unit, pc, 0x08, 0x08));
            break;
        case  1:         /* PECL , no such pin in 5482 data sheet */
        case -1:
        default:
            return SOC_E_CONFIG;
    }

    /* Configure Auto-detect Medium (0x1c shadow 11110) */
    mask = 0x33f;               /* Auto-detect bit mask */
    data = 0;
    if (pc->automedium) {
        data |= 1 << 0;    /* Enable auto-detect medium */
        if (PHY_SECONDARY_SERDES_MODE(unit, port)) {
            data |= 1 << 9;    /* Enable Secondary SerDes auto-detect */
        }
    }
    /* When no link partner exists, fiber should be default medium */
    /* When both link partners exipc, fiber should be prefered medium
     * for the following reasons.
     * 1. Most fiber module generates signal detect when the fiber cable is
     *    plugged in regardless of any activity even if the link partner is
     *    powered down.
     * 2. Fiber mode consume much lesser power than copper mode.
     */
    if (pc->fiber.preferred) {
        data |= 1 << 1;    /* Fiber selected when both media are active */
        data |= 1 << 2;    /* Fiber selected when no medium is active */
    }

    /*
     * Enable internal inversion of LED2/SD input pin.  Used only if
     * the fiber module provides RX_LOS instead of Signal Detect.
     */
    if (pc->fiber_detect < 0) {
        data |= 1 << 8;    /* Fiber signal detect is active low from pin */
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_AUTO_DETECT_MEDIUMr(unit, pc, data, mask));
        
#if DISABLE_CLK125
    /* Note: CLK125 does not function properly on BCM5482S A0 and A1 */
    /*       Refer to 5482S-ES101-R for details.                     */ 
    /* Reduce EMI emissions by disabling the CLK125 pin if not used  */
    data = 0x001c;  /* When write, bit 9 and 7 must be zero and */ 
    data = 0x029c;  /* bit 2, 3, and 4 must be one. */ 
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_SPARE_CTRL_3r(unit, pc, 0, 1));
#endif

#if AUTO_MDIX_WHEN_AN_DIS
    /* Enable Auto-MDIX When autoneg disabled */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_MII_MISC_CTRLr(unit, pc, 0x200, 0x200));
#endif

    /*
     * Configure Auxiliary 1000BASE-X control register to turn off
     * carrier extension.  The Intel 7131 NIC does not accept carrier
     * extension and gets CRC errors.
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_AUX_1000X_CTRLr(unit, pc, 0x0040, 0x0040));

    /* Configure extended control register for led mode and jumbo frame support
     */
    mask  = (1 << 5) | (1 << 0);

    /* if using led link/activity mode, disable led traffic mode */
    if (pc->ledctrl & 0x10 || pc->ledselect == 0x01) {
        data = 0;
    } else {
        data  = 1 << 5;      /* Enable LEDs to indicate traffic status */
    }
    /* Configure Extended Control Register for Jumbo frames support */
    data |= 1 << 0;      /* Set high FIFO elasticity to support jumbo frames */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_MII_ECRr(unit, pc, data, mask));

    /* Enable extended packet length (4.5k through 25k) */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5482_MII_AUX_CTRLr(unit, pc, 0x4000, 0x4000));

    /* Configure LED selectors */
    data = ((pc->ledmode[2] & 0xf) << 4) | (pc->ledmode[0] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_LED_SELECTOR_1r(unit, pc, data));

    data = ((pc->ledmode[1] & 0xf) << 4) | (pc->ledmode[3] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_LED_SELECTOR_2r(unit, pc, data));

    data = (pc->ledctrl & 0x3ff);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_LED_CTRLr(unit, pc, data));

    SOC_IF_ERROR_RETURN
        (WRITE_PHY5482_EXP_LED_SELECTORr(unit, pc, pc->ledselect));

   return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_init
 * Purpose:
 *      Init function for 5482 PHY.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
robo_phy_5482_init(int unit, soc_port_t port)
{
    phy_ctrl_t          *pc;
    phy_ctrl_t          *int_pc;
    int                 fiber_capable;
    int                 fiber_preferred;
    uint16              data;

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "robo_phy_5482_init: u=%d p=%d\n"),
                         unit, port));

    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    /* bcm5348/5347/53262 bcm5389/bcm5396 interface == sgmii */
    if (soc_feature(unit, soc_feature_dodeca_serdes)){
        pc->interface = SOC_PORT_IF_SGMII;
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_SECONDARY_SERDES);
    } else {
        if (int_pc == NULL){
            pc->interface = SOC_PORT_IF_RGMII;
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_PRIMARY_SERDES);
        } else {
            pc->interface = SOC_PORT_IF_SGMII;
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_SECONDARY_SERDES);
        }
    }

    SOC_IF_ERROR_RETURN
        (READ_PHY5482_MODE_CTRLr(unit, pc, &data));
    fiber_capable = (data & 0x8) ? 1 : 0;

    /*
     * In automedium mode, the property phy_fiber_pref_port<X> is used
     * to set a preference for fiber mode in the event both copper and
     * fiber links are up.
     *
     * If NOT in automedium mode, phy_fiber_pref_port<X> is used to
     * select which of fiber or copper is forced.
     */
    if (!fiber_capable) {
        fiber_preferred = 0;
        pc->automedium      = 0;
        pc->fiber_detect    = 0;
    } else {
        fiber_preferred =
            soc_property_port_get(unit, port, spn_PHY_FIBER_PREF, 1);
        pc->automedium =
            soc_property_port_get(unit, port, spn_PHY_AUTOMEDIUM, 1);
        pc->fiber_detect =
            soc_property_port_get(unit, port, spn_PHY_FIBER_DETECT, 0);
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_5482_init: "
                         "u=%d p=%d type=5482%s automedium=%d fiber_pref=%d detect=%d\n"),
              unit, port, fiber_capable ? "S" : "",
              pc->automedium, fiber_preferred, pc->fiber_detect));

    pc->copper.enable = TRUE;
    pc->copper.preferred = !fiber_preferred;
    pc->copper.autoneg_enable = TRUE;
    pc->copper.autoneg_advert = ADVERT_ALL_COPPER;
    pc->copper.force_speed = 1000;
    pc->copper.force_duplex = TRUE;
    pc->copper.master = SOC_PORT_MS_AUTO;
    pc->copper.mdix = SOC_PORT_MDIX_AUTO;

    pc->fiber.enable = fiber_capable;
    pc->fiber.preferred = fiber_preferred;
    pc->fiber.autoneg_enable = fiber_capable;
    pc->fiber.autoneg_advert = ADVERT_ALL_FIBER;
    pc->fiber.force_speed = 1000;
    pc->fiber.force_duplex = TRUE;
    pc->fiber.master = SOC_PORT_MS_NONE;
    pc->fiber.mdix = SOC_PORT_MDIX_NORMAL;

    /* Initially configure for the preferred medium. */
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_PASSTHRU);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_100FX);
    if (pc->fiber.preferred) {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
    }

    /* Get Requested LED selectors (defaults are hardware defaults) */
    pc->ledmode[0] = soc_property_port_get(unit, port, spn_PHY_LED1_MODE, 0);
    pc->ledmode[1] = soc_property_port_get(unit, port, spn_PHY_LED2_MODE, 1);
    pc->ledmode[2] = soc_property_port_get(unit, port, spn_PHY_LED3_MODE, 3);
    pc->ledmode[3] = soc_property_port_get(unit, port, spn_PHY_LED4_MODE, 6);
    pc->ledctrl = soc_property_port_get(unit, port, spn_PHY_LED_CTRL, 0x8);
    pc->ledselect = soc_property_port_get(unit, port, spn_PHY_LED_SELECT, 0);

    SOC_IF_ERROR_RETURN
        (_robo_phy_5482_reset_setup(unit, port));

    SOC_IF_ERROR_RETURN
        (_robo_phy_5482_medium_config_update(unit, port,
                                        PHY_COPPER_MODE(unit, port) ?
                                        &pc->copper :
                                        &pc->fiber));
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_enable_set
 * Purpose:
 *      Enable or disable the physical interface.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      enable - Boolean, true = enable PHY, false = disable.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
robo_phy_5482_enable_set(int unit, soc_port_t port, int enable)
{
    uint16         power;
    phy_ctrl_t    *pc;

    pc     = EXT_PHY_SW_STATE(unit, port);
    power  = (enable) ? 0 : MII_CTRL_PD;

    if (pc->automedium || PHY_COPPER_MODE(unit, port)) {
        if (pc->copper.enable){
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_CTRLr(unit, pc, power, MII_CTRL_PD));
    
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "phy_5482_enable_set: "
                                 "Power %s copper medium\n"), (enable) ? "up" : "down"));
        }
    }

    if (pc->automedium || PHY_FIBER_MODE(unit, port)) {

        if (pc->fiber.enable) {        
            if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, power, 
                                                   MII_CTRL_PD));            
            } else{
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, power, 
                                                   MII_CTRL_PD));
            }
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "robo_phy_5482_enable_set: "
                                 "Power %s fiber medium\n"), (enable) ? "up" : "down"));
        }

    }

    /* Update software state */
    SOC_IF_ERROR_RETURN(phy_fe_ge_enable_set(unit, port, enable));

    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_enable_get
 * Purpose:
 *      Enable or disable the physical interface for a 5482 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - (OUT) Boolean, true = enable PHY, false = disable.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
robo_phy_5482_enable_get(int unit, soc_port_t port, int *enable)
{
    return phy_fe_ge_enable_get(unit, port, enable);
}

/*
 * Function:
 *      _robo_phy_5482_100fx_setup
 * Purpose:
 *      Switch from 1000X mode to 100FX.  
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_robo_phy_5482_fiber_100fx_setup(int unit, soc_port_t port) 
{
    uint16               data;
    phy_ctrl_t    *pc;

    pc          = EXT_PHY_SW_STATE(unit, port);

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "_robo_phy_5482_fiber_100fx_setup: u=%d p=%d \n"),
              unit, port));
    if (PHY_PRIMARY_SERDES_MODE(unit, port)){
        /* Operate primary SerDes in 1000X */
        /* 1.power down copper interface: indicating as optional in "AN105" */
        /*    -- Not implemented. ("optional" in document) */
                            
        /* 2. enable primary SerDes register */                    
        /* 3. reset the power down of the SerDes function */
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s:p=%d,Pri-SerDes mode. Fiber PowerDown(%s)!\n"),
                  FUNCTION_NAME(), port, "No"));
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, 0, MII_CTRL_PD));

        /* 4. set to fiber mode */ /* bit[2:1]=1 */
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_MODE_CTRLr(unit, pc, 0x2,0x6));                    

        /* 5. set loopback */
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, 1, MII_CTRL_LE));

        /* 6. select 100FX mode */
            /* bit[0]=1 */
        data = 1;
        data |= pc->fiber.force_duplex? PHY_5482_PRIMARY_SERDES_100FX_FD:0;

        SOC_IF_ERROR_RETURN(    
            MODIFY_PHY5482_100FX_CTRLr(unit, pc, data, 
              1 | PHY_5482_PRIMARY_SERDES_100FX_FD));

        /* 7. Power down SerDes RX path (Expention register)*/
        /* 8. Power up SerDes RX path */ /* select Exp_reg 05 */
        SOC_IF_ERROR_RETURN(
            WRITE_PHY5482_EXP_LED_FLASH_CTRLr(unit, pc, 0x0C3B));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY5482_EXP_LED_FLASH_CTRLr(unit, pc, 0x0C3A));

        /* 9. reset loopback to switch clock back to SerDes RX clock */
            /* bit[14]=0 */
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, 0, MII_CTRL_LE));
    } else {
        /* PHY_SECONDARY_SERDES_MODE(unit, port) */
        /* Operate secondary SerDes in 100FX  */
        data  = 1 << 0;     /* Enable secondary SerDes */
        data |= 1 << 2;     /* Select Sync function from secondary SerDes */
        data |= 1 << 3;     /* Enable secondary SerDes to driver LEDs     */
        data |= 1 << 4;     /* Operate secondary SerDes in 100Base-FX mode */
        if (pc->fiber.force_duplex) {
            data |= 1 << 5;   /* Enable secondary SerDes 100FX full duplex  */
        }
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_CTRLr(unit, pc, data));

        /* Secondary SerDes */
        /* Reset the secondary SerDes         */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, MII_CTRL_RESET));
        sal_usleep(10000);

        /* Power up the secondary SerDes      */ 
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s:p=%d,Sec-SerDes mode. Fiber PowerDown(%s)!\n"),
                  FUNCTION_NAME(), port, "No"));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, 
                     MII_CTRL_AE | MII_CTRL_FD | MII_CTRL_SS_100 |
                     (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE) ? MII_CTRL_PD : 0)));

        /* Enable extended packet length (100fx-mode) */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_2ND_SERDES_MISC1r(unit, pc, 0x01, 0x01));
    }
    
    PHY_FLAGS_SET(unit, port, PHY_FLAGS_100FX);
    pc->fiber.force_speed    = 100;
    pc->fiber.autoneg_enable = FALSE;

    return SOC_E_NONE;
}

/*
 * Function:
 *      _robo_phy_5482_1000x_setup
 * Purpose:
 *      Switch from 100FX mode to 1000X.  
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_robo_phy_5482_fiber_1000x_setup(int unit, soc_port_t port) 
{
    uint16         data;
    phy_ctrl_t    *pc;

    pc          = EXT_PHY_SW_STATE(unit, port);

    LOG_VERBOSE(BSL_LS_SOC_PHY,
                (BSL_META_U(unit,
                            "_robo_phy_5482_fiber_1000x_setup: u=%d p=%d \n"),
                 unit, port));
 
    if (PHY_PRIMARY_SERDES_MODE(unit, port)){
        /* Operate primary SerDes in 1000X */
        /* 1.power down copper interface: indicating as optional in "AN105" */
        /*    -- Not implemented. (for the "optional" in document) */
        
        /* 2.set to Fiber mode and enable primary SerDes register */
            /* bit[2:1]=01*/
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_MODE_CTRLr(unit, pc, 0x2,0x6));            
        /* 2.1 enable 1000X, ie. disable 100FX(phy5482S-DS06.pdf) */
            /* bit[0]=0 */
        SOC_IF_ERROR_RETURN(    
            MODIFY_PHY5482_100FX_CTRLr(unit,pc, 0, 1));
        /* 2.2 add the reset at 1000Base-X (phy5482S-DS06.pdf) */
#if 0   /* marked for no proper ferture init routine after reset */        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->miim_read)
                    (unit, phy_addr, MII_CTRL_REG, &data));
        data |= 0x8000;  /* bit[15]=0 */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->miim_write)
                    (unit, phy_addr, MII_CTRL_REG, data));
#endif /* if 0 */
        
        /* 3.clear the SerDes power down bit */ 
            /* bit[11]=0 */
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s:p=%d,Pri-SerDes Mode. Fiber PowerDown(%s)!\n"),
                  FUNCTION_NAME(), port, "No"));
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, 0, MII_CTRL_PD));
        
    } else {
        /* PHY_SECONDARY_SERDES_MODE(unit, port) */
        /* Operate secondary SerDes in 1000X  */
        data  = 1 << 0;     /* Enable secondary SerDes */
        data |= 1 << 2;     /* Select Sync function from secondary SerDes */
        data |= 1 << 3;     /* Enable secondary SerDes to driver LEDs     */
        data |= 1 << 5;     /* Enable secondary SerDes 100FX full duplex  */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_CTRLr(unit, pc, data));

        /* Reset the secondary SerDes         */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, MII_CTRL_RESET));
        sal_usleep(10000);
    
        /* Power up the secondary SerDes      */ 
        if (pc->fiber.autoneg_enable) {
            data = MII_CTRL_AE | MII_CTRL_RAN | MII_CTRL_FD | MII_CTRL_SS_1000;
        } else {
            data = MII_CTRL_RAN | MII_CTRL_FD | MII_CTRL_SS_1000;
        }

        if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
            data |= MII_CTRL_PD;
        }

        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s:p=%d,Sec-SerDes Mode. Fiber PowerDown(%s)!\n"),
                  FUNCTION_NAME(), port, "No"));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, data));
    }

    /* 1000BaseX always full duplex */
    pc->fiber.force_duplex = TRUE;
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_100FX);

    return SOC_E_NONE;
}

STATIC int
_robo_phy_5482_medium_change(int unit, soc_port_t port, int force_update)
{
    phy_ctrl_t    *pc;
    int            medium;

    pc    = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (_robo_phy_5482_medium_check(unit, port, &medium));

    if (medium == SOC_PORT_MEDIUM_COPPER) {
        if ((!PHY_COPPER_MODE(unit, port)) || force_update) { /* Was fiber */ 
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_100FX);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
            SOC_IF_ERROR_RETURN
                (_robo_phy_5482_no_reset_setup(unit, port));

            if (pc->copper.enable) {
                SOC_IF_ERROR_RETURN
                     (_robo_phy_5482_medium_config_update(unit, port, 
                                                          &pc->copper));
            }
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "%s: u=%d p=%d [F->C]\n"),
                      FUNCTION_NAME(), unit, port));
        }
    } else {        /* Fiber */
        if (PHY_COPPER_MODE(unit, port) || force_update) { /* Was copper */
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
            SOC_IF_ERROR_RETURN
                (_robo_phy_5482_no_reset_setup(unit, port));

            if (pc->fiber.enable) {
                SOC_IF_ERROR_RETURN
                     (_robo_phy_5482_medium_config_update(unit, port, 
                                &pc->fiber));
            }
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "%s: u=%d p=%d [C->F]\n"),
                      FUNCTION_NAME(), unit, port));
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_link_get
 * Purpose:
 *      Determine the current link up/down status for a 5482 device.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      link - (OUT) Boolean, true indicates link established.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      If using automedium, also switches the mode.
 */
STATIC int
robo_phy_5482_link_get(int unit, soc_port_t port, int *link)
{
    phy_ctrl_t    *pc;
    uint16         data; 
    uint16         mask;
    int            rv = SOC_E_NONE;

    pc    = EXT_PHY_SW_STATE(unit, port);
    *link = FALSE;      /* Default return */

    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }

    if (!pc->fiber.enable && !pc->copper.enable) {
        return SOC_E_NONE;
    }

    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
    if (pc->automedium) {
        /* Check for medium change and update HW/SW accordingly. */
        SOC_IF_ERROR_RETURN
            (_robo_phy_5482_medium_change(unit, port,FALSE));
    }
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_link_get(unit, port, link);
        if (rv == SOC_E_BUSY){
            int     lb_stat = 0;
            uint16  mii_stat;

            SOC_IF_ERROR_RETURN(phy_fe_ge_lb_get(unit, port, &lb_stat));
            if (lb_stat) {
                /* phy_fe_ge_link_get() reutrn SOC_E_BUSY means no AN_DONE is
                 * detected when AN is enabled.
                 */
                SOC_IF_ERROR_RETURN
                        (READ_PHYFEGE_MII_STATr(unit, pc, &mii_stat));
                *link = (mii_stat != 0xffff) && (mii_stat & MII_STAT_LA) ? TRUE : FALSE;
            } else {
                return rv;
            }
        } else if (rv){
            return rv;
        }


    } else {
        if (PHY_SECONDARY_SERDES_MODE(unit, port)) {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_1000X_STATr(unit, pc, &data));    
            *link = (data != 0xffff) && (data & MII_STAT_LA) ? TRUE : FALSE;
        } else {
            /* get link from expension register */
            SOC_IF_ERROR_RETURN(
                READ_PHY5482_EXP_OPT_MODE_STATr(unit, pc,&data));
            *link = (data != 0xffff) && (data & (1 << 15)) ? 1 : 0;
        }
    }

    /* Fix problem, 1G copper linked and then plug 100FX, no link 
     * donw event can't help linkscan to update MAC. This will 
     * cause the MAC stay at 1G speed but phy is at 100 speed.
     * 
     *
     *  Return linkdown will help upper layer linkscan to update 
     *  MAC properly. This is for medium changed from copper 
     *  to fiber only.
     */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE)){
        *link = 0;
    }
    
    /* if no medium changed, no action requirred in below */
    if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE)){
        return SOC_E_NONE;
    }

    /* If preferred medium is up, disable the other medium 
     * to prevent false link. */
    if (pc->automedium) {
        mask = MII_CTRL_PD;
        if (pc->copper.preferred) {
            if (pc->fiber.enable) {
                data = (*link && PHY_COPPER_MODE(unit, port)) ? MII_CTRL_PD : 0;
            } else {
                data = MII_CTRL_PD;
            }            
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "%s:p=%d,Copper preferred. Fiber PowerDown(%s)!\n"),
                      FUNCTION_NAME(), port, 
                      (data == MII_CTRL_PD) ? "Yes" : "No"));
            if (PHY_SECONDARY_SERDES_MODE(unit, port)){
                /* power down secondary serdes */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, data, mask));
            } else {
                /* power down primary serdes */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, data, mask));
            }
        } else {  /* pc->fiber.preferred */
            if (pc->copper.enable) {
                data = (*link && PHY_FIBER_MODE(unit, port)) ? MII_CTRL_PD : 0;
            } else {
                data = MII_CTRL_PD;
            }
            LOG_INFO(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "%s:p=%d,Fiber preferred. Copper PowerDown(%s)!\n"),
                      FUNCTION_NAME(), port, 
                      (data == MII_CTRL_PD) ? "Yes" : "No"));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_CTRLr(unit, pc, data, mask));
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_PHY,
                (BSL_META_U(unit,
                            "robo_phy_5482_link_get: u=%d p=%d mode=%s%s link=%d\n"),
                 unit, port,
                 PHY_COPPER_MODE(unit, port) ? "C" : "F",
                 PHY_FIBER_100FX_MODE(unit, port)? "(100FX)" : "",
                 *link));

    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_duplex_set
 * Purpose:
 *      Set the current duplex mode (forced).
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      duplex - Boolean, true indicates full duplex, false indicates half.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_UNAVAIL - Half duplex requested, and not supported.
 * Notes:
 *      The duplex is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 *      Autonegotiation is not manipulated.
 */

STATIC int
robo_phy_5482_duplex_set(int unit, soc_port_t port, int duplex)
{
    int                  rv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        /* Note: mac.c uses phy_5690_notify_duplex to update 5690 */
        rv = phy_fe_ge_duplex_set(unit, port, duplex);
        if (SOC_SUCCESS(rv)) {
            pc->copper.force_duplex = duplex;
        }
    } else {    /* Fiber */
        if (PHY_FIBER_100FX_MODE(unit, port)) {
            if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_100FX_CTRLr(unit, pc, 
                           duplex?PHY_5482_PRIMARY_SERDES_100FX_FD:0,
                           PHY_5482_PRIMARY_SERDES_100FX_FD));
            } else {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_2ND_SERDES_CTRLr(unit, pc,
                           duplex? PHY_5482_2ND_SERDES_100FX_FD:0,
                           PHY_5482_2ND_SERDES_100FX_FD));
            }
            pc->fiber.force_duplex = duplex;
        } else {  /* 1000X always full duplex */
            if (!duplex) {
                rv = SOC_E_UNAVAIL;
            }
        }
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_5482_duplex_set: u=%d p=%d d=%d rv=%d\n"),
              unit, port, duplex, rv));

    return rv;
}

/*
 * Function:
 *      robo_phy_5482_duplex_get
 * Purpose:
 *      Get the current operating duplex mode. If autoneg is enabled,
 *      then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      duplex - (OUT) Boolean, true indicates full duplex, false
 *              indicates half.
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The duplex is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level. Autonegotiation is
 *      not manipulated.
 */

STATIC int
robo_phy_5482_duplex_get(int unit, soc_port_t port, int *duplex)
{
    phy_ctrl_t    *pc;
    uint16 data16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_duplex_get(unit, port, duplex);    
    } else {
        if (PHY_FIBER_100FX_MODE(unit, port)) {
            if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
                SOC_IF_ERROR_RETURN
                    (READ_PHY5482_100FX_CTRLr(unit, pc,&data16));
                *duplex = (data16 & PHY_5482_PRIMARY_SERDES_100FX_FD) ?
                          TRUE: FALSE;
            } else { 
                SOC_IF_ERROR_RETURN
                    (READ_PHY5482_2ND_SERDES_CTRLr(unit, pc, &data16));
                *duplex = (data16 & PHY_5482_2ND_SERDES_100FX_FD) ?
                          TRUE: FALSE;
            }
        } else { /* 1000X */ 
            *duplex = TRUE;
        }
    }
        
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_speed_set
 * Purpose:
 *      Set the current operating speed (forced).
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      speed - Requested speed, only 1000 supported.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The speed is set only for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_speed_set(int unit, soc_port_t port, int speed)
{
    int            rv; 
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        /* Note: mac.c uses phy_5690_notify_speed to update 5690 */
        rv = phy_fe_ge_speed_set(unit, port, speed);
        if (SOC_SUCCESS(rv)) {
            pc->copper.force_speed = speed;
        }
    } else {    /* Fiber */
        if (speed == 100) {
            rv = _robo_phy_5482_fiber_100fx_setup(unit, port);
       } else  if (speed == 0 || speed == 1000) {
            rv = _robo_phy_5482_fiber_1000x_setup(unit, port);
       } else {
            rv = SOC_E_CONFIG;
       }
       if (SOC_SUCCESS(rv)) {
           pc->fiber.force_speed = speed;
       }
    }
    
    if (SOC_SUCCESS(rv) && !PHY_SGMII_AUTONEG_MODE(unit, port)) {
        uint16 mii_ctrl;

        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "%s: u=%d p=%d NONE SGMII_AUTONEG set spd(%d) ON 1000X!\n"),
                  FUNCTION_NAME(), unit, port, speed));
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_1000X_MII_CTRLr(unit, pc, &mii_ctrl));

        mii_ctrl &= ~(MII_CTRL_SS_LSB | MII_CTRL_SS_MSB);
        switch(speed) {
        case 10:
            mii_ctrl |= MII_CTRL_SS_10;
            break;
        case 100:
            mii_ctrl |= MII_CTRL_SS_100;
            break;
        case 0: 
        case 1000:
            mii_ctrl |= MII_CTRL_SS_1000;
            break;
        default:
            return(SOC_E_CONFIG);
        }

        SOC_IF_ERROR_RETURN
            (WRITE_PHY5482_1000X_MII_CTRLr(unit, pc, mii_ctrl));
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_5482_speed_set: u=%d p=%d s=%d fiber=%d rv=%d\n"),
              unit, port, speed, PHY_FIBER_MODE(unit, port), rv));

    return rv;
}

/*
 * Function:
 *      robo_phy_5482_speed_get
 * Purpose:
 *      Get the current operating speed for a 5482 device.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      duplex - (OUT) Boolean, true indicates full duplex, false
 *              indicates half.
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The speed is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_speed_get(int unit, soc_port_t port, int *speed)
{
    phy_ctrl_t    *pc;
    uint16         opt_mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_speed_get(unit, port, speed);
    } else {
        *speed = 1000;
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_EXP_OPT_MODE_STATr(unit, pc, &opt_mode));
        switch(opt_mode & (0x3 << 13)) {
        case (0 << 13):
            *speed = 10;
            break;
        case (1 << 13):
            *speed = 100;
            break;
        case (2 << 13):
            *speed = 1000;
            break;
        default:
            LOG_WARN(BSL_LS_SOC_PHY,
                     (BSL_META_U(unit,
                                 "phy_5482_speed_get: u=%d p=%d invalid speed\n"),
                      unit, port));
            break;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_master_set
 * Purpose:
 *      Set the current master mode
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      master - SOC_PORT_MS_*
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The master mode is set only for the ACTIVE medium.
 */
STATIC int
robo_phy_5482_master_set(int unit, soc_port_t port, int master)
{
    int                  rv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_master_set(unit, port, master);
        if (SOC_SUCCESS(rv)) {
            pc->copper.master = master;
        }
    }
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "robo_phy_5482_master_set: u=%d p=%d master=%d fiber=%d rv=%d\n"),
              unit, port, master, PHY_FIBER_MODE(unit, port), rv));
    return rv;
}

/*
 * Function:
 *      robo_phy_5482_master_get
 * Purpose:
 *      Get the current master mode for a 5482 device.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      master - (OUT) SOC_PORT_MS_*
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The master mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_master_get(int unit, soc_port_t port, int *master)
{
    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_master_get(unit, port, master);
    } else {
        *master = SOC_PORT_MS_NONE;
        return SOC_E_NONE;
    }
}

/*
 * Function:
 *      robo_phy_5482_autoneg_set
 * Purpose:
 *      Enable or disable auto-negotiation on the specified port.
 * Parameters:
 *      unit -Switch unit #.
 *      port - Switch port #.
 *      autoneg - Boolean, if true, auto-negotiation is enabled
 *              (and/or restarted). If false, autonegotiation is disabled.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is set only for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_autoneg_set(int unit, soc_port_t port, int autoneg)
{
    int                 rv;
    uint16              data, mask;
    phy_ctrl_t   *pc;

    /* don't allow AN if port is disabled  */

    if (autoneg) {
        if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
            return SOC_E_NONE;
        }
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_an_set(unit, port, autoneg);
        if (SOC_SUCCESS(rv)) {
            pc->copper.autoneg_enable = autoneg ? 1 : 0;
        }
    } else { 
        if (PHY_PRIMARY_SERDES_MODE(unit, port)){
            data = (autoneg) ? MII_CTRL_AE | MII_CTRL_RAN: 0;
            rv = MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, data,
                                                MII_CTRL_AE | MII_CTRL_RAN);
        } else {
            /* Disable/Enable auto-detection between SGMII-slave and 1000BASE-X */
            if (autoneg) {
                /* When enabling autoneg, set the default speed to 1000Mbps first.
                 * PHY will not enable autoneg if the PHY is in 100FX mode.
                 */
                SOC_IF_ERROR_RETURN
                    (robo_phy_5482_speed_set(unit, port, 1000));
            }
            data = (autoneg) ? (1 << 0) : 0;
            mask  = (1 << 0);     /* Switch between 1000BASE-X and SGMII slave 
                               * modes based on SerDes-received auto-neg
                               * code word. */
            mask |= (1 << 1);     /* Enable/Disable SGMII slave mode */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_2ND_SERDES_SGMII_SLAVEr(unit, pc, data, mask));
        
            SOC_IF_ERROR_RETURN
                (robo_phy_5482_adv_local_set(unit, port, pc->fiber.autoneg_advert));

            /* If enable autoneg, also enable parallel detect */
            data = (autoneg) ? (1 << 1) : 0;
            mask = (1 << 1);
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_EXP_2ND_SERDES_CTRLr(unit, pc, data, mask));

            data = MII_CTRL_FD;
            data |= (autoneg) ? MII_CTRL_AE | MII_CTRL_RAN : 0;
            mask = MII_CTRL_AE | MII_CTRL_RAN | MII_CTRL_FD;
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, data, mask));

        }
        pc->fiber.autoneg_enable = autoneg ? TRUE : FALSE;
    }
 
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "robo_phy_5482_autoneg_set: u=%d p=%d autoneg=%d rv=%d\n"),
              unit, port, autoneg, rv));

    return rv;
}

/*
 * Function:
 *      robo_phy_5482_autoneg_get
 * Purpose:
 *      Get the current auto-negotiation status (enabled/busy).
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      autoneg - (OUT) if true, auto-negotiation is enabled.
 *      autoneg_done - (OUT) if true, auto-negotiation is complete. This
 *              value is undefined if autoneg == FALSE.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_autoneg_get(int unit, soc_port_t port,
                     int *autoneg, int *autoneg_done)
{
    int           rv;
    uint16        data;
    phy_ctrl_t   *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    rv            = SOC_E_NONE;
    *autoneg      = FALSE;
    *autoneg_done = FALSE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_an_get(unit, port, autoneg, autoneg_done);
    } else {
        /* autoneg is not enabled in 100FX mode */
        if (PHY_FIBER_100FX_MODE(unit, port)) {
            *autoneg = FALSE;
            *autoneg_done = TRUE;
            return rv;
        }

        if (PHY_PRIMARY_SERDES_MODE(unit, port)){
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_1000X_MII_CTRLr(unit, pc, &data));
            if (data & MII_CTRL_AE) {
                *autoneg = TRUE;

                SOC_IF_ERROR_RETURN
                    (READ_PHY5482_1000X_MII_STATr(unit, pc, &data));
                if (data & MII_STAT_AN_DONE) {
                    *autoneg_done = TRUE;
                }
            }
        } else {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, &data));
            if (data & MII_CTRL_AE) {
                *autoneg = TRUE;

                SOC_IF_ERROR_RETURN
                    (READ_PHY5482_2ND_SERDES_1000X_STATr(unit, pc, &data));
                if (data & MII_STAT_AN_DONE) {
                    *autoneg_done = TRUE;
                }
            }
        }
    }

    return rv;
}

/*
 * Function:
 *      robo_phy_5482_adv_local_set
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      mode - Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
robo_phy_5482_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    int                  rv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_adv_local_set(unit, port, mode);
        if (SOC_SUCCESS(rv)) {
            pc->copper.autoneg_advert = mode & ADVERT_ALL_COPPER;
        }
    } else { 
        uint16  mii_adv=0;

        if (mode & SOC_PM_1000MB_FD)    mii_adv |= MII_ANA_C37_FD;
        if (mode & SOC_PM_1000MB_HD)    mii_adv |= MII_ANA_C37_HD;    
        switch (mode & SOC_PM_PAUSE) {
        case SOC_PM_PAUSE:
            mii_adv |= MII_ANA_C37_PAUSE;
            break;
        case SOC_PM_PAUSE_TX:
            mii_adv |= MII_ANA_C37_ASYM_PAUSE;
            break;
        case SOC_PM_PAUSE_RX:
            mii_adv |= (MII_ANA_C37_ASYM_PAUSE | MII_ANA_C37_PAUSE);
            break;
        }
        if (PHY_PRIMARY_SERDES_MODE(unit, port)){            
            rv = WRITE_PHY5482_1000X_MII_ANAr(unit, pc, mii_adv);
        } else {
            /* Write to Secondary SerDes Auto-Neg Advertisement Reg */
            rv = WRITE_PHY5482_2ND_SERDES_ANAr(unit, pc, mii_adv);
        }

        if (SOC_SUCCESS(rv)) {
            pc->fiber.autoneg_advert = mode & ADVERT_ALL_FIBER;
        }
    }
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "robo_phy_5482_adv_local_set: u=%d p=%d mode=0x%x, rv=%d\n"),
              unit, port, mode, rv));
    return rv;
}

/*
 * Function:
 *      robo_phy_5482_adv_local_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
robo_phy_5482_adv_local_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    uint16         mii_adv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        return phy_ge_adv_local_get(unit, port, mode);
    } else {    /* PHY_FIBER_MODE */
        /* read the 1000BASE-X Auto-Neg Advertisement Reg ( Address 0x04 ) */
        if (PHY_PRIMARY_SERDES_MODE(unit, port)){            
            SOC_IF_ERROR_RETURN    
                (READ_PHY5482_1000X_MII_ANAr(unit, pc, &mii_adv));
        } else {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_ANAr(unit, pc, &mii_adv));
        }
        *mode = 0;
        *mode |= (mii_adv & MII_ANA_C37_FD) ? SOC_PM_1000MB_FD : 0;
        *mode |= (mii_adv & MII_ANA_C37_HD) ? SOC_PM_1000MB_HD : 0;
        switch (mii_adv & (MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE)) {
        case MII_ANA_C37_PAUSE:
            *mode |= SOC_PM_PAUSE;
            break;
        case MII_ANA_C37_ASYM_PAUSE:
            *mode |= SOC_PM_PAUSE_TX;
            break;
        case (MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE):
            *mode |= SOC_PM_PAUSE_RX;
            break;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_adv_remote_get
 * Purpose:
 *      Get partners current advertisement for auto-negotiation.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The remote advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level. If Autonegotiation is
 *      disabled or in progress, this routine will return an error.
 */

STATIC int
robo_phy_5482_adv_remote_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    uint16               data;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    if (PHY_COPPER_MODE(unit, port)) {
        return phy_ge_adv_remote_get(unit, port, mode);
    } else {    /* PHY_FIBER_MODE */
        *mode = 0;
        /* read the 1000BASE-X     Control Reg ( Address 0x00 ),
         *   Auto-Neg Link Partner Ability Reg ( Address 0x05 )
         */
        if (PHY_PRIMARY_SERDES_MODE(unit, port)){            

            /* read the 1000BASE-X Control Reg ( Address 0x00 ) */
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_1000X_MII_CTRLr(unit, pc, &data));
        } else {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, &data));
        }

        if (!(data & MII_CTRL_AE)) {
            return SOC_E_DISABLED;
        }

        if (PHY_PRIMARY_SERDES_MODE(unit, port)){            
            /* read the 1000Base-X Status Reg  ( Address 0x01 ) */
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_1000X_MII_STATr(unit, pc, &data));
        } else {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_1000X_STATr(unit, pc, &data));
        }

        if (!(data & MII_STAT_AN_DONE)) {
            return SOC_E_NONE;
        }

        if (PHY_PRIMARY_SERDES_MODE(unit, port)){            
            /* read the auto-Neg Link Partner Ability Reg ( Address 0x05 ) */
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_1000X_MII_ANPr(unit, pc, &data));
        } else {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_2ND_SERDES_ANPr(unit, pc, &data));
        }

        if (data & MII_ANP_SGMII_MODE) {
            /* Link partner is SGMII master */
            switch(data & MII_ANP_SGMII_SS_MASK) {
            case MII_ANP_SGMII_SS_10:
                *mode = SOC_PM_10MB;
                break;
            case MII_ANP_SGMII_SS_100:
                *mode = SOC_PM_100MB;
                break;
            case MII_ANP_SGMII_SS_1000:
                *mode = SOC_PM_1000MB;
                break;
            }

            *mode &= (data & MII_ANP_SGMII_FD) ? SOC_PM_FD : SOC_PM_HD;
        } else {
            *mode |= (data & MII_ANP_C37_FD) ? SOC_PM_1000MB_FD : 0;
            *mode |= (data & MII_ANP_C37_HD) ? SOC_PM_1000MB_HD : 0;
            switch (data & (MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE)) {
            case MII_ANP_C37_PAUSE:
                *mode |= SOC_PM_PAUSE;
                break;
            case MII_ANP_C37_ASYM_PAUSE:
                *mode |= SOC_PM_PAUSE_TX;
                break;
            case (MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE):
                *mode |= SOC_PM_PAUSE_RX;
                break;
            }
        }

    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5482_lb_set
 * Purpose:
 *      Set the local PHY loopback mode.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      loopback - Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
robo_phy_5482_lb_set(int unit, soc_port_t port, int enable)
{
    int            rv;
    uint16         data;
    phy_ctrl_t    *pc;
    int retry = 0;
    
    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

#if AUTO_MDIX_WHEN_AN_DIS
    {
        /* Disable Auto-MDIX When autoneg disabled */
        /* Enable Auto-MDIX When autoneg disabled */
        data = (enable) ? 0x0000 : 0x0200;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_MII_MISC_CTRLr(unit, pc, data, 0x0200)); 
    }
#endif

    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_lb_set(unit, port, enable);
        /* Reference GNATS:13729
          * Fix for 546x Gigabit PHY loopback disabled issue, wait for phy link 
          * status down completely(phy link down need some time to complete), 
          * avoid side effect between disable one port's phy loopback and 
          * enable another port's phy loopback soon.
          */
        if (!enable) {
            for (retry = 0 ; retry < 5 ; retry++) {
                sal_usleep(500000);
                SOC_IF_ERROR_RETURN(
                    READ_PHY5482_MII_STATr(unit, pc, &data));
                if (!(data & MII_STAT_LA)) {
                    break;
                }
            }
        }
        /* this is for the workaround to fix the problem on reporting link
         * status.(GNATS 29609)
         */
        if (enable) {
            SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_TEST1r(unit, pc, 0x1000, 0x1000));
        } else {
            SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_TEST1r(unit, pc, 0x0000, 0x1000));
        }
    } else { 
        data = (enable) ? MII_CTRL_LE : 0;
        if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
            rv = MODIFY_PHY5482_1000X_MII_CTRLr(unit, pc, data, MII_CTRL_LE);
        } else {
            rv   = MODIFY_PHY5482_2ND_SERDES_1000X_CTRLr
                  (unit, pc, data, MII_CTRL_LE);
        }
        if (enable && SOC_SUCCESS(rv)) {
            uint16        stat;
            int           link;
            soc_timeout_t to;

            /* Wait up to 5000 msec for link up */
            soc_timeout_init(&to, 5000000, 0);
            link = 0;
            while (!soc_timeout_check(&to)) {
                if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
                    rv = READ_PHY5482_1000X_MII_STATr(unit, pc, &stat);
                } else {
                    rv = READ_PHY5482_2ND_SERDES_1000X_STATr(unit, pc, &stat);
                }
                link = stat & MII_STAT_LA;
                if (link || SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (!link) {
                LOG_WARN(BSL_LS_SOC_PHY,
                         (BSL_META_U(unit,
                                     "phy_5482_lb_set: u=%d p=%d TIMEOUT\n"),
                          unit, port));

                rv = SOC_E_TIMEOUT;
            }
        }
    }
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "robo_phy_5482_lb_set: u=%d p=%d en=%d rv=%d\n"), 
              unit, port, enable, rv));

    return rv; 
}

/*
 * Function:
 *      robo_phy_5482_lb_get
 * Purpose:
 *      Get the local PHY loopback mode.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      loopback - (OUT) Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_lb_get(int unit, soc_port_t port, int *enable)
{
    uint16               data;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (phy_fe_ge_lb_get(unit, port, enable));
    } else {
        if (PHY_PRIMARY_SERDES_MODE(unit, port)) {
           SOC_IF_ERROR_RETURN
               (READ_PHY5482_1000X_MII_CTRLr(unit, pc, &data));
        } else {
           SOC_IF_ERROR_RETURN
               (READ_PHY5482_2ND_SERDES_1000X_CTRLr(unit, pc, &data));
        }
        *enable = (data & MII_CTRL_LE) ? 1 : 0;
    }

    return SOC_E_NONE; 
}

/*
 * Function:
 *      robo_phy_5482_interface_set
 * Purpose:
 *      Set the current operating mode of the PHY.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      pif - one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_UNAVAIL - unsupported interface
 */

STATIC int
robo_phy_5482_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    switch (pif) {
    case SOC_PORT_IF_SGMII:
    case SOC_PORT_IF_GMII: 
    case SOC_PORT_IF_RGMII: /* For Robo5324 GE ports */
    case SOC_PORT_IF_MII:   /* IMP port */
        return SOC_E_NONE;
    default:
        return SOC_E_UNAVAIL;
    }
}

/*
 * Function:
 *      robo_phy_5482_interface_get
 * Purpose:
 *      Get the current operating mode of the PHY.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
robo_phy_5482_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    phy_ctrl_t    *pc;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    pc = EXT_PHY_SW_STATE(unit, port);

    if (soc_feature(unit, soc_feature_dodeca_serdes)) {
        *pif = SOC_PORT_IF_SGMII;
    } else {
        *pif = pc->interface;
    }


    return SOC_E_NONE;
}


/*
 * Function:
 *      robo_phy_5482_ability_get
 * Purpose:
 *      Get the abilities of the local gigabit PHY.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The ability is retrieved only for the ACTIVE medium.
 */

STATIC int
robo_phy_5482_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    int         rv;
    phy_ctrl_t *pc;

    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_ability_get(unit, port, mode);
    } else { /* fiber mode */
        *mode = (SOC_PM_AN | SOC_PM_LB_PHY | 
                 SOC_PM_PAUSE | SOC_PM_PAUSE_ASYMM |
                 SOC_PM_GMII | SOC_PM_SGMII | SOC_PM_1000MB_FD |
                 SOC_PM_100MB_FD | SOC_PM_100MB_HD);
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    if (pc->automedium) {
        *mode |= SOC_PM_COMBO;
    }

    return rv;
}

/*
 * Function:
 *      robo_phy_5482_mdix_set
 * Description:
 *      Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - One of:
 *              SOC_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              SOC_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              SOC_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              SOC_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
robo_phy_5482_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{
    phy_ctrl_t    *pc;
    int                  speed;

    pc = EXT_PHY_SW_STATE(unit, port);
    if (PHY_COPPER_MODE(unit, port)) {
        switch (mode) {
        case SOC_PORT_MDIX_AUTO:
            /* Clear bit 14 for automatic MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_ECRr(unit, pc, 0, 0x4000));

            /* Clear bit 9 to disable forced auto MDI xover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_MISC_CTRLr(unit, pc, 0, 0x0200));
            break;

        case SOC_PORT_MDIX_FORCE_AUTO:
            /* Clear bit 14 for automatic MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_ECRr(unit, pc, 0, 0x4000));

            /* Set bit 9 to disable forced auto MDI xover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5482_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200));
            break;

        case SOC_PORT_MDIX_NORMAL:
            SOC_IF_ERROR_RETURN(robo_phy_5482_speed_get(unit, port, &speed));
            if (speed == 0 || speed == 10 || speed == 100) {
                /* Set bit 14 for automatic MDI crossover */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_MII_ECRr(unit, pc, 0x4000, 0x4000));

                SOC_IF_ERROR_RETURN
                    (WRITE_PHY5482_TEST1r(unit, pc, 0));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        case SOC_PORT_MDIX_XOVER:
            SOC_IF_ERROR_RETURN(robo_phy_5482_speed_get(unit, port, &speed));
            if (speed == 0 || speed == 10 || speed == 100) {
                /* Set bit 14 for automatic MDI crossover */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5482_MII_ECRr(unit, pc, 0x4000, 0x4000));

                SOC_IF_ERROR_RETURN
                    (WRITE_PHY5482_TEST1r(unit, pc, 0x0080));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        default:
            return SOC_E_PARAM;
        }
        pc->copper.mdix = mode;
    } else {    /* fiber */
        if (mode != SOC_PORT_MDIX_NORMAL) {
            return SOC_E_UNAVAIL;
        }
    }
    return SOC_E_NONE;
}        

/*
 * Function:
 *      robo_phy_5482_mdix_get
 * Description:
 *      Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (Out) One of:
 *              SOC_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              SOC_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              SOC_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              SOC_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
robo_phy_5482_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mode)
{
    phy_ctrl_t    *pc;
    int            speed;

    if (mode == NULL) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN(robo_phy_5482_speed_get(unit, port, &speed));
        if (speed == 1000) {
           *mode = SOC_PORT_MDIX_AUTO;
        } else {
            *mode = pc->copper.mdix;
        }
    } else {
        *mode = SOC_PORT_MDIX_NORMAL;
    }

    return SOC_E_NONE;
}    

/*
 * Function:
 *      robo_phy_5482_mdix_status_get
 * Description:
 *      Get the current MDIX status on a port/PHY
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 *      status  - (OUT) One of:
 *              SOC_PORT_MDIX_STATUS_NORMAL
 *                      Straight connection
 *              SOC_PORT_MDIX_STATUS_XOVER
 *                      Crossover has been performed
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
robo_phy_5482_mdix_status_get(int unit, soc_port_t port, 
                         soc_port_mdix_status_t *status)
{
    phy_ctrl_t    *pc;
    uint16          data;

    if (status == NULL) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_MII_ESRr(unit, pc, &data));
        if (data & 0x2000) {
            *status = SOC_PORT_MDIX_STATUS_XOVER;
        } else {
            *status = SOC_PORT_MDIX_STATUS_NORMAL;
        }
    } else {
        *status = SOC_PORT_MDIX_STATUS_NORMAL;
    }

    return SOC_E_NONE;
}    

STATIC int
_robo_phy_5482_power_mode_set (int unit, soc_port_t port, int mode)
{
    phy_ctrl_t    *pc;

    pc       = EXT_PHY_SW_STATE(unit, port);

    if (pc->power_mode == mode) {
        return SOC_E_NONE;
    }

    if (mode == SOC_PHY_CONTROL_POWER_FULL) {
        /* disable the auto power mode */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_AUTO_POWER_DOWNr(unit,pc,
                        0,
                        PHY_5482_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    } else if (mode == SOC_PHY_CONTROL_POWER_AUTO) {         
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_AUTO_POWER_DOWNr(unit,pc,
                        PHY_5482_AUTO_PWRDWN_EN,
                        PHY_5482_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    }
    return SOC_E_NONE; 
}

/*
 * Function:
 *      robo_phy_5482_control_set
 * Purpose:
 *      Configure PHY device specific control fucntion.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - New setting for the control
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
robo_phy_5482_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    int rv;
    phy_ctrl_t *pc;
    uint16 data16;

    /* coverity[mixed_enums] */
    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch(type) {
        case SOC_PHY_CONTROL_CLOCK_ENABLE:
        if (pc->phy_rev == 0xB) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY5482_EXP_SGMII_REC_CTRLr(unit, pc, value ? (1U<<4) : 0));
        }
        break;

    case SOC_PHY_CONTROL_POWER:
        rv = _robo_phy_5482_power_mode_set(unit,port,value);
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        if (value <= PHY_5482_AUTO_PWRDWN_WAKEUP_MAX) {

            /* at least one unit */
            if (value < PHY_5482_AUTO_PWRDWN_WAKEUP_UNIT) {
                value = PHY_5482_AUTO_PWRDWN_WAKEUP_UNIT;
            }
        } else { /* anything more then max, set to the max */
            value = PHY_5482_AUTO_PWRDWN_WAKEUP_MAX;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_AUTO_POWER_DOWNr(unit,pc,
                      value/PHY_5482_AUTO_PWRDWN_WAKEUP_UNIT,
                      PHY_5482_AUTO_PWRDWN_WAKEUP_MASK));
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        /* sleep time configuration is either 2.7s or 5.4 s, default is 2.7s */
        if (value < PHY_5482_AUTO_PWRDWN_SLEEP_MAX) {
            data16 = 0; /* anything less than 5.4s, default to 2.7s */
        } else {
            data16 = PHY_5482_AUTO_PWRDWN_SLEEP_MASK;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5482_AUTO_POWER_DOWNr(unit,pc,
                      data16,
                      PHY_5482_AUTO_PWRDWN_SLEEP_MASK));
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}


/*
 * Function:
 *      robo_phy_5482_control_get
 * Purpose:
 *      Get current control settign of the PHY.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - (OUT)Current setting for the control
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
robo_phy_5482_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    int rv;
    phy_ctrl_t *pc;
    uint16 data;

    /* coverity[mixed_enums] */
    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch(type) {
        case SOC_PHY_CONTROL_CLOCK_ENABLE:
        if (pc->phy_rev == 0xB) {
            SOC_IF_ERROR_RETURN
                (READ_PHY5482_EXP_SGMII_REC_CTRLr(unit, pc, &data));
            *value = (data & (1U << 4)) ? TRUE : FALSE;
        }
        break;

    case SOC_PHY_CONTROL_POWER:
        *value = pc->power_mode;
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_AUTO_POWER_DOWNr(unit,pc, &data));

        if (data & PHY_5482_AUTO_PWRDWN_SLEEP_MASK) {
            *value = PHY_5482_AUTO_PWRDWN_SLEEP_MAX;
        } else {
            *value = 2700;
        }
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY5482_AUTO_POWER_DOWNr(unit,pc, &data));

        data &= PHY_5482_AUTO_PWRDWN_WAKEUP_MASK;
        *value = data * PHY_5482_AUTO_PWRDWN_WAKEUP_UNIT;
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 * Function:
 *      robo_phy_5482_cable_diag
 * Purpose:
 *      Run 546x cable diagnostics
 * Parameters:
 *      unit - device number
 *      port - port number
 *      status - (OUT) cable diagnotic status structure
 * Returns:     
 *      SOC_E_XXX
 */
STATIC int
robo_phy_5482_cable_diag(int unit, soc_port_t port,
                    soc_port_cable_diag_t *status)
{
    int             rv, rv2, i;
    phy_ctrl_t      *pc;

    extern int phy_5464_cable_diag_sw(int, soc_port_t , 
                                      soc_port_cable_diag_t *);

    if (status == NULL) {
        return SOC_E_PARAM;
    }

    status->state = SOC_PORT_CABLE_STATE_OK;
    status->npairs = 4;
    status->fuzz_len = 0;
    for (i = 0; i < 4; i++) {
        status->pair_state[i] = SOC_PORT_CABLE_STATE_OK;
    }

    MIIM_LOCK(unit);    /* this locks out linkscan, essentially */
    rv = phy_5464_cable_diag_sw(unit,port, status);
    MIIM_UNLOCK(unit);
    rv2 = 0;
    if (rv <= 0) {      /* don't reset if > 0 -- link was up */
        rv2 = _robo_phy_5482_reset_setup(unit, port);

        /* GNATS 40594 : 
         *  - PHY configuration been unexpected changed after performed cable
         *      diag on the GE port if this port is linked at 10/100 speed.
         *
         *  Note : 
         *  1. if the PHY linked at 10/100 due to Force speed or local AN  
         *      ability were changed to speed up to 10/100, original PHY  
         *      configuration will be lost due to PHY reset above.
         */
        pc = EXT_PHY_SW_STATE(unit, port);
        SOC_IF_ERROR_RETURN(_robo_phy_5482_medium_config_update(unit, 
                port, PHY_COPPER_MODE(unit, port) ?
                &pc->copper :
                &pc->fiber));

    }
    if (rv >= 0 && rv2 < 0) {
        return rv2;
    }
    return rv;
}


#ifdef BROADCOM_DEBUG

/*
 * Function:
 *      phy_5482_shadow_dump
 * Purpose:
 *      Debug routine to dump all shadow registers.
 * Parameters:
 *      unit - Switch unit #.
 *      port - Switch port #.
 */

void
robo_phy_5482_shadow_dump(int unit, soc_port_t port)
{
    phy_ctrl_t *pc;
    uint16      tmp;
    int         i;

    pc       = EXT_PHY_SW_STATE(unit, port);

    /* Register 0x18 Shadows */
    for (i = 0; i <= 7; i++) {
        WRITE_PHY_REG(unit, pc, 0x18, (i << 12) | 0x7);
        READ_PHY_REG(unit, pc, 0x18, &tmp);
        if ((tmp & ~7) == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x18[0x%x]=0x%04x\n"), i, tmp));
    }

    /* Register 0x1c Shadows */
    for (i = 0; i <= 0x1f; i++) {
        WRITE_PHY_REG(unit, pc, 0x1c, i << 10);
        READ_PHY_REG(unit, pc, 0x1c, &tmp);
        if ((tmp & ~0x7c00) == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x1c[0x%x]=0x%04x\n"), i, tmp));
    }

    /* Register 0x17/0x15 Shadows */
    for (i = 0; i <= 0x13; i++) {
        WRITE_PHY_REG(unit, pc, 0x17, 0xf00 | i);
        READ_PHY_REG(unit, pc, 0x15, &tmp);
        if (tmp  == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x17[0x%x]=0x%04x\n"), i, tmp));
    }
}

#endif /* BROADCOM_DEBUG */

/*
 * Variable:    phy_5482drv_ge
 * Purpose:     PHY driver for 5482
 */

phy_driver_t phy_5482robodrv_ge = {
    "5482 Gigabit PHY Driver",
    robo_phy_5482_init,
    phy_fe_ge_reset,
    robo_phy_5482_link_get,
    robo_phy_5482_enable_set,
    robo_phy_5482_enable_get,
    robo_phy_5482_duplex_set,
    robo_phy_5482_duplex_get,
    robo_phy_5482_speed_set,
    robo_phy_5482_speed_get,
    robo_phy_5482_master_set,
    robo_phy_5482_master_get,
    robo_phy_5482_autoneg_set,
    robo_phy_5482_autoneg_get,
    robo_phy_5482_adv_local_set,
    robo_phy_5482_adv_local_get,
    robo_phy_5482_adv_remote_get,
    robo_phy_5482_lb_set,
    robo_phy_5482_lb_get,
    robo_phy_5482_interface_set,
    robo_phy_5482_interface_get,
    robo_phy_5482_ability_get,
    NULL,                        /* Phy link up event */
    NULL,                        /* Phy link down event */
    robo_phy_5482_mdix_set,
    robo_phy_5482_mdix_get,
    robo_phy_5482_mdix_status_get,
    robo_phy_5482_medium_config_set,
    robo_phy_5482_medium_config_get,
    robo_phy_5482_medium_status,
    robo_phy_5482_cable_diag,
    NULL,                       /* phy_link_change */
    robo_phy_5482_control_set,  /* phy_control_set */ 
    robo_phy_5482_control_get,  /* phy_control_get */
    phy_ge_reg_read,
    phy_ge_reg_write,
    phy_ge_reg_modify,
    NULL                        /* Phy event notify */ 
};

#else /* INCLUDE_PHY_5482_ROBO */
int _soc_phy_5482_not_empty_robo;
#endif /* INCLUDE_PHY_5482_ROBO */


