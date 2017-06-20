/*
 * $Id: phy5464robo.c,v 1.69 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        phy5464.c
 * Purpose: PHY driver for BCM5461 and BCM5464
 *
 * Supported BCM546X Family of PHY devices:
 *
 *  Device  Ports   Media               MAC Interface
 *  5461    1   Copper              GMII
 *  5461S   1   Copper, Serdes          GMII, SGMII
 *  5464    4   Copper, Serdes          GMII, SGMII
 *  5464R   4   Copper, Serdes          SGMII
 *  5464S   4   Copper, Serdes, Passthru    GMII, SGMII
 *  5464SR  4   Copper, Serdes, Passthru    SGMII
 *  5466    4   Copper, Serdes          GMII, SGMII
 *  5466R   4   Copper, Serdes          SGMII
 *  5466S   4   Copper, Serdes, Passthru    GMII, SGMII
 *  5466SR  4   Copper, Serdes, Passthru    SGMII
 *
 * The 5466 devices have a strapping option that will make them act
 * identically to the corresponding 5464 device (including PHY ID
 * register values).
 *
 * Other devices in this family, but not supported by this driver:
 *  5464H   4   Copper              RGMII(HSTL)
 *  5465    4   Copper              RGMII
 *
 * This driver works with both 569x switches that have an SGMII interface
 * and with Strata and 5665 switches with a GMII interface.  There is no
 * support in this driver for RGMII, TBI, or RTBI interfaces.
 *
 * All of these PHY devices support a 100FX media mode, but currently this
 * driver does not.
 *
 * If this driver is compiled without 569x support, then only GMII (Copper,
 * Fiber) and SGMII (Copper) support will be enabled.
 *
 * SGMII support:
 *  The 5690 and 5695 switch devices provide a serial GMII (SGMII)
 *  MAC interface to a PHY device.  The PHY devices above with an SGMII
 *  interface and no Passthru support are supported as copper only
 *  PHYs.  Although the PHYs have a Serdes media interface suitable for
 *  fiber media, it shares the same electronics as the SGMII MAC interface.
 *  The 569x internal serdes is kept into passthru mode and the PHY
 *  performs all signaling.
 *
 *  Those PHYs with a Passthru media interface can support both Copper
 *  (as above) and Fiber media (by putting the PHY into passthru mode
 *  and having the 569x serdes handle all signaling).  There is no way
 *  for the driver to determine from the PHY device if it has a Passthru
 *  media interface, so properties must be set if Fiber media is to be
 *  used.  The property "phy_5464S" or "phy_5464S_portXX" must be set if
 *  the PHY device is a 5464S or 5464SR.
 *
 *  In all of the cases for 569x connections, the PHY's 1000BASE-X
 *  registers are never enabled.
 *
 *  The 569x switches do not support SGMII auto-negotiation.  They only
 *  support Serdes auto-negotiation.  In Copper mode, 569x autoneg must
 *  be off and the PHY device autoneg on the SGMII mac interface must
 *  also be off.  In Passthru (Fiber) mode, the 569x is used for Serdes
 *  autoneg and the 5464 is in passthru.
 *
 * GMII support:
 *  The Strata and 5665 family of switch devices provide a GMII MAC
 *  interface to a PHY device.  The PHY devices with a GMII interface
 *  can be connected to these switches.  Any PHY device with a Serdes
 *  media interface can be used for both Copper and Fiber media
 *  connections.  Those PHY devices without a Serdes interface can only
 *  be used in Copper mode.
 *
 *  The Passthru mode in any PHY devices is ignored.
 *
 * Auto-medium selection:
 *  Automatic selection of copper vs. fiber medium type is enabled when
 *  the "phy_fiber_automedium" property is 1 (default 0 for 5464 on 569x
 *  and 1 otherwise).  If both copper and fiber signals are present, the
 *  driver consults the configuration property "phy_fiber_pref_portXX"
 *  (default 1) to determine whether copper or fiber is preferred.
 *
 *  See sdk/doc/combo-ports.txt for more information.
 *
 *  NOTE: auto-medium REQUIRES the fiber module energy detect pin to be
 *  wired to the LED4 pin on the 5464S.
 *
 * Workarounds:
 *  ERRATA_FIX: (default 0)
 *  The driver will deal with various errata for 5464 A0 and 5461 A0
 *  revision PHY devices.  It is not enabled by default because
 *  production use of these PHYs should be using A1 or later revisions.
 *
 *  LINK_DOWN_RESET_WAR: (default 1)
 *      The PHY will be reset each time link down is detected on copper.
 *  This is due to the 100TX_SWAP erratum on 5464 A0 revision parts.
 */


#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/core/spl.h> 
#include <shared/alloc.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/phyreg.h>
#include <soc/phy.h>
#include <soc/brcm_osl.h>
 
#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phyident.h"
#include "phyreg.h"
#include "phyfege.h"
#include "phy5464robo.h"
#include "phy5464.h"

#if defined(INCLUDE_PHY_5464) && defined(BCM_ROBO_SUPPORT)

/*
 * Please see the errata sheet for the PHY devices for a description of
 * the errata to decide which workarounds (WARs) to enable for your
 * application.
 */

#define LINK_DOWN_RESET_WAR 1   /* Reset PHY on copper link down */
#define ERRATA_FIX       0   /* rev A0 errata */

#define AUTO_MDIX_WHEN_AN_DIS   0   /* Non-standard-compliant option */
#define DISABLE_CLK125      0   /* Disable if unused to reduce */
                    /*  EMI emissions */

#define DPRINTF(_s)             LOG_INFO(BSL_LS_SOC_PHY, _s)
#define DPRINTF_VERBOSE(_s)     LOG_VERBOSE(BSL_LS_SOC_PHY, _s)


#ifndef robo_phy_5464_cable_diag_HW
#define robo_phy_5464_cable_diag_HW  0
#endif

#define PHY65nm_LINK2MRVL_WORKAROUND  1

#define PHY53115_ERRATA_WORKAROUND  1

#define PHY53118_B0_WORKAROUND  1
/* Note for "PHY53118_B0 Workaround": 
 *  - phy testing problem
 *
 *  Workaround for two testing:
 *  1. IEEE 1000BASE-T testing
 *  2. EMI testing
 */

#define PHY53118_B1_WORKAROUND  1
/* Note for "PHY53118_B1 Workaround": 
 *
 *  Workaround for:
 *  ADC clock 
 *    - for 1000/100 BT link issue
 */

#if PHY53115_ERRATA_WORKAROUND

#define PHY53115_A0_AN100TX_WORKAROUND  1
#if PHY53115_A0_AN100TX_WORKAROUND
/* Note for "AN-100TX Workaround": 
 *  - link problem with AN enabled at 100TX
 *
 *  Workaround :
 *  1. set phy to work at force 100HD mode.
 *  2. set expansion reg FFh as value=0x20c0.
 *
 */
STATIC int  phy53115_an100tx_war = 1; /* enable this workaround by default */

STATIC uint32 phy53115_errata_an100tx_status = 0;
STATIC uint32 phy53115_errata_an100tx_linkup = 0;
STATIC uint32 phy53115_errata_an100tx_linkERR_detect = 0;
#define PHY53115_ERRATA_AN100TX_STAT_SET(_p)   \
            (phy53115_errata_an100tx_status |= (uint32)(0x1 << (_p)))
#define PHY53115_ERRATA_AN100TX_STAT_GET(_p)   \
            ((phy53115_errata_an100tx_status >> (_p)) & 0x1)
#define PHY53115_ERRATA_AN100TX_STAT_CLEAR(_p)   \
            (phy53115_errata_an100tx_status &= ~((uint32)(0x1 << (_p))))
            
#define PHY53115_ERRATA_AN100TX_LINK_SET(_p)   \
            (phy53115_errata_an100tx_linkup |= (uint32)(0x1 << (_p)))
#define PHY53115_ERRATA_AN100TX_LINK_GET(_p)   \
            ((phy53115_errata_an100tx_linkup >> (_p)) & 0x1)
#define PHY53115_ERRATA_AN100TX_LINK_CLEAR(_p)   \
            (phy53115_errata_an100tx_linkup &= ~((uint32)(0x1 << (_p))))

#define PHY53115_ERRATA_AN100TX_LINKERR_SET(_p)   \
        (phy53115_errata_an100tx_linkERR_detect |= (uint32)(0x1 << (_p)))
#define PHY53115_ERRATA_AN100TX_LINKERR_GET(_p)   \
        ((phy53115_errata_an100tx_linkERR_detect >> (_p)) & 0x1)
#define PHY53115_ERRATA_AN100TX_LINKERR_CLEAR(_p)   \
        (phy53115_errata_an100tx_linkERR_detect &= ~((uint32)(0x1 << (_p))))

STATIC uint32 phy53115_errata_an100tx_failed_cnt[SOC_MAX_NUM_PORTS];
#define PHY53115_ERRATA_AN100TX_RESET_CNT   100

void _phy53115_a0_errata(int unit, soc_port_t port, int link);
void _phy53115_a0_errata_link_check(int unit, soc_port_t port, int link);

#endif  /* PHY53115_A0_AN100TX_WORKAROUND */

#define PHY53115_IOP_WORKAROUND  1
/* Note for "IOP Workaround": 
 *  - IOP(inter-operability) is the link problem when work with VTSS PHY.
 *  - This problem occurred in BCM53115 A0/A1/B0/B1 only.
 *  - Also, this problem was assumed will be existed in BCM53118 A0 also.
 *
 *  == A0/A1/B1 Workaround : IOP-WAR-#1==
 *   - set following step in init or reset stage on each phy.
 *      1. mii-reg 18h set to 0x0c00    'Enable SM_DSP
 *      2. mii-reg 17h set to 0x0f08    'Select Expansion
 *      3. mii-reg 15h set to 0x0201    'Set Low-Power 10BT
 *      4. mii-reg 17h set to 0x001f    'select Misc. Reg 1F Ch0
 *      5. mii-reg 15h set to 0x0300    'Increase HPF corner
 *      6. mii-reg 17h set to 0x001f    'Select Misc. Reg 1F Ch3
 *      7. mii-reg 15h set to 0x0002    'Disable LPF
 *      8. mii-reg 17h set to 0x0f75    'set VDACctrl
 *      9. mii-reg 15h set to 0x003c    'incress driver
 *  - set follow step on phy-id specified (In the PHY team script the section 
 *      below is init once during system up).
 *      1. phy(3), mii-reg 1Ch set to 0xb012    'Increase 10BT amplitude
 *      2. phy(3), mii-reg 1Ch set to 0xac60    'Improve 10BT return loss
 *      3. phy(2), mii-reg 1Ch set to 0xac4e    'Improve 1000T template
 *      4. phy(4), mii-reg 17h set to 0x0f96    'Increase 10BT amplitude
 *      5. phy(4), mii-reg 15h set to 0x0010    'Increase 10BT amplitude
 *      6. phy(4), mii-reg 17h set to 0x0f97    'Increase 10BT amplitude
 *      7. phy(4), mii-reg 15h set to 0x0c0c    '
 *
 *  == B0 Workaround : IOP-WAR-#2==
 *   - this workaround serves for helping the link establish between short 
 *      and long cable at Auto-Nego mode.
 *   - tow configurings for this workaround.
 *      a. HalfOut mode : "Half Amplitude + Wire-Speed Off" - for short cable.
 *          >> mii-reg 18h, shadow 4 set to 0x040c
 *          >> mii-reg 18h, shadow 7 set to 0x8067
 *      b. Normal mode : "Normal Amplitude + Wire-Speed On" - for long cable.
 *          >> mii-reg 18h, shadow 4 set to 0x0004
 *          >> mii-reg 18h, shadow 7 set to 0x8077
 *   - Implementation.
 *      1. If linked-up, check if the port is at HalfOut mode.
 *          1.1. Set the Normal mode if HalfOut mode is set.
 *          1.2. Do nothing if HalfOut mode is not set.
 *      2. If linked-down, change the HalfOut setting circularly(under 3 sec.)
 *          2.1. if current mode is HalfOut mode, set to Normal mode.
 *          2.2. else set to HalfOut mode
 */

#if PHY53115_IOP_WORKAROUND
STATIC uint32 phy53115_errata_iop_cnt[SOC_MAX_NUM_PORTS];
#define PHY53115_ERRATA_IOP_RESET_CNT   100

/* ========= IOP WAR for bcm53115_A0/A1/B1 only : IOP-WAR-#1 ========== */
void _phy53115_a0_iop_errata(int unit, soc_port_t port);

/* === IOP WAR for bcm53115_B0 and bcm53118_A0 only : IOP-WAR-#2 === */
STATIC int  phy53115_b0_iop_war = 1; /* enable this workaround by default */
STATIC uint32 phy53115_errata_iop_flag = 0;
#define PHY53115_ERRATA_IOP_SET(_p)   \
            (phy53115_errata_iop_flag |= (uint32)(0x1 << (_p)))
#define PHY53115_ERRATA_IOP_GET(_p)   \
            ((phy53115_errata_iop_flag >> (_p)) & 0x1)
#define PHY53115_ERRATA_IOP_CLEAR(_p)   \
            (phy53115_errata_iop_flag &= ~((uint32)(0x1 << (_p))))

void _phy53115_b0_iop_errata(int unit, soc_port_t port, int link);

#endif  /* PHY53115_IOP_WORKAROUND */

#endif  /* PHY53115_ERRATA_WORKAROUND */

/*
 * passthru mode is used for fiber support on 569x where the phy
 * is put into serdes passthru mode and the serdes support in the
 * 569x is used.
 */

#define ADVERT_ALL_COPPER \
    (SOC_PM_PAUSE | SOC_PM_10MB | SOC_PM_100MB | SOC_PM_1000MB)
#define ADVERT_ALL_FIBER \
    (SOC_PM_PAUSE | SOC_PM_1000MB_FD)
#define INIT_FORCE_SPEED 1000   

#define PHY_REV_A0      0
#define PHY_REV_A1      1

/* In the serial phy546x(includes phy5466/phy5482), the fiber SD is required 
 * for supporting the combo medium solution. The SD might be active high or 
 * active low depends on HW designing in customer.
 *   SDK phy driver might indicate such definition by "config.bcm". And here we 
 * define HW_DEFAULT_SD_VALUE for such preference when no config.bcm definition 
 * been loaded.
 * --------------------------------------------------
 *  1. bcm95324a0 : SD active low
 *  2. bcm95324a1 : SD active low
 *  3. tobo ADD.....
 */
#define HW_DEFAULT_SD_VALUE         -10

typedef struct phy_5464_state_s {
    uint16      phy_model;      /* PHY model from ID reg */
    uint16      phy_rev;        /* PHY revision from ID reg */
    uint8       ledmode[4];     /* LED1..4 selectors */
    uint16      ledctrl;        /* LED control */
    int         automedium;     /* Medium auto-select active */
    int         fiber_detect;       /* Fiber Signal Detection */
    int                 halfout;            /* Transmit half amplitude */
    soc_phy_config_t    copper;         /* Current copper config */
    soc_phy_config_t    fiber;          /* Current fiber config */
} phy_5464_state_t;


STATIC int robo_phy_5464_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int robo_phy_5464_speed_set(int unit, soc_port_t port, int speed);
STATIC int robo_phy_5464_master_set(int unit, soc_port_t port, int master);
STATIC int robo_phy_5464_adv_local_set(int unit, soc_port_t port,
                  soc_port_mode_t mode);
STATIC int robo_phy_5464_autoneg_set(int unit, soc_port_t port, int autoneg);
STATIC int robo_phy_5464_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode);
STATIC int _robo_phy_5464_medium_change(int unit, soc_port_t port,
                                        int force_update);

/*
 * Function:
 *  _robo_phy_5464_medium_check
 * Purpose:
 *  Determine if chip should operate in copper or fiber mode.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  medium - (OUT) SOC_PORT_MEDIUM_XXX
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
_robo_phy_5464_medium_check(int unit, soc_port_t port, int *medium)
{
    phy_ctrl_t        *pc;    /* PHY state */
    uint16             tmp;    /* Temp variable */
    int                copper; /* Copper medium is active */

    pc    = EXT_PHY_SW_STATE(unit, port);
    /* Read Mode Register (0x1c shadow 11111) */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_MODE_CTRLr(unit, pc, &tmp));
    if (PHY_FORCED_COPPER_MODE(unit, port)) {
        copper = TRUE;
    } else {
        if (!pc->fiber.enable) {
            copper = TRUE;
        } else if (!pc->copper.enable) {
            copper = FALSE;
        } else {
            if (pc->fiber.preferred) {
                /* 0x10 Fiber Signal Detect
                 * 0x20 Copper Energy Detect
                 */
                copper = ((tmp & 0x30) == 0x20); 
            } else {
                copper = ((tmp & 0x20) == 0x20);
            }
        }
    }

    if (SOC_IS_DINO(unit)) {
        *medium = SOC_PORT_MEDIUM_COPPER;
    } else {
        *medium = copper ? SOC_PORT_MEDIUM_COPPER : SOC_PORT_MEDIUM_FIBER;
    }

    DPRINTF_VERBOSE((BSL_META_U(unit,
                                "_phy_5464_medium_check: "
                                "u=%d p=%d fiber_pref=%d 0x1c(11111)=%04x\n"),
                    unit, port, pc->fiber.preferred, tmp));
    DPRINTF_VERBOSE((BSL_META_U(unit,
                                "_phy_5464_medium_check: u=%d p=%d fiber=%d\n"),
                     unit, port, !copper));

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_medium_status
 * Purpose:
 *  Indicate the current active medium
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  medium - (OUT) One of:
 *      SOC_PORT_MEDIUM_COPPER
 *      SOC_PORT_MEDIUM_FIBER
 * Returns:
 *  SOC_E_NONE
 */

STATIC int
robo_phy_5464_medium_status(int unit, soc_port_t port, soc_port_medium_t *medium)
{
    if (PHY_COPPER_MODE(unit, port)) {
        *medium = SOC_PORT_MEDIUM_COPPER;
    } else {
        *medium =SOC_PORT_MEDIUM_FIBER;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *  _phy_5464_medium_config_update
 * Purpose:
 *  Update the PHY with config parameters
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  cfg - Config structure.
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
_phy_5464_medium_config_update(int unit, soc_port_t port,
                soc_phy_config_t *cfg)
{
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_speed_set(unit, port, cfg->force_speed));
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_duplex_set(unit, port, cfg->force_duplex));
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_master_set(unit, port, cfg->master));
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_adv_local_set(unit, port, cfg->autoneg_advert));
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_autoneg_set(unit, port, cfg->autoneg_enable));
    SOC_IF_ERROR_RETURN
        (robo_phy_5464_mdix_set(unit, port, cfg->mdix));

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_medium_config_set
 * Purpose:
 *  Set the operating parameters that are automatically selected
 *  when medium switches type.
 * Parameters:
 *  unit - Switch unit #
 *  port - Port number
 *  medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *  cfg - Operating parameters
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
robo_phy_5464_medium_config_set(int unit, soc_port_t port, 
                           soc_port_medium_t  medium,
                           soc_phy_config_t  *cfg)
{

    phy_ctrl_t       *pc;
    soc_phy_config_t *active_medium;  /* Currently active medium */
    soc_phy_config_t *change_medium;  /* Requested medium */
    soc_phy_config_t *other_medium;   /* The other medium */
    int               medium_update;
    soc_port_mode_t   advert_mask;

    if (NULL == cfg) {
        return SOC_E_PARAM;
    }
    
    pc            = EXT_PHY_SW_STATE(unit, port);
    medium_update = FALSE;

    switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        if (!pc->automedium && !PHY_COPPER_MODE(unit, port)) {
             return SOC_E_UNAVAIL;
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
        /* The configuration may cause medium change. Check
         * and update medium.
         */
        SOC_IF_ERROR_RETURN       
            (_robo_phy_5464_medium_change(unit, port, TRUE));
    } else {
        active_medium = (PHY_COPPER_MODE(unit, port)) ?  
                          &pc->copper : &pc->fiber;
        if (active_medium == change_medium) {
            /* If the medium to update is active, update the configuration */
            SOC_IF_ERROR_RETURN
                (_phy_5464_medium_config_update(unit, port, 
                                                       change_medium));
        }
    }
    return SOC_E_NONE;

}

/*
 * Function:
 *  robo_phy_5464_medium_config_get
 * Purpose:
 *  Get the operating parameters that are automatically selected
 *  when medium switches type.
 * Parameters:
 *  unit - Switch unit #
 *  port - Port number
 *  medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *  cfg - (OUT) Operating parameters
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
robo_phy_5464_medium_config_get(int unit, soc_port_t port, 
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
 * Helper defines for expansion register access
 * Used throughout the cable diagnostic code
 */
#define WRITE_EXP(_r, _v) \
        SOC_IF_ERROR_RETURN \
                (WRITE_PHY_REG(unit, pc, 0x17, 0x0f00 | _r)); \
        SOC_IF_ERROR_RETURN \
                (WRITE_PHY_REG(unit, pc, 0x15, _v))

#define READ_EXP(_r, _v) \
        SOC_IF_ERROR_RETURN \
                (WRITE_PHY_REG(unit, pc, 0x17, 0x0f00 | _r)); \
        SOC_IF_ERROR_RETURN \
                (READ_PHY_REG(unit, pc, 0x15, _v))
                
/*
 * Function:
 *  _robo_phy_5464_reset_setup
 * Purpose:
 *  Function to reset the PHY and set up initial operating registers.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Configures for copper/fiber according to the current
 *  setting of PHY_FLAGS_FIBER.
 */

STATIC int
_robo_phy_5464_reset_setup(int unit, soc_port_t port)
{
    phy_ctrl_t        *int_pc;
    phy_ctrl_t        *pc;
    uint16             tmp;
    uint16 dev_id = 0;
    uint8 rev_id = 0;

    DPRINTF((BSL_META_U(unit,
                        "_robo_phy_5464_reset_setup: u=%d p=%d medium=%s\n"),
             unit, port,
             PHY_COPPER_MODE(unit, port) ? "COPPER" : "FIBER"));

    SOC_IF_ERROR_RETURN(phy_ge_init(unit, port));

    pc     = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    /* Configure interface to MAC */    
    
    /* GNATS 40581 : 
     *  - CableDiag may causes link dropped problem on the built-in GE PHY,
     *      if those PHy is linked at 10/100 speed. 
     *
     *  The reason is the built-in GE PHY on bcm53115 have no fiber capable 
     *  and unable to select to 1000X register space. Thus AN will be 
     *  unexpected turn off in the section below on configuration of 1000X AN.
     *  (bcm53118/53125/53128 will have the same problem)
     */
    if (!PHY_FORCED_COPPER_MODE(unit, port) || 
            ((pc->automedium) && (pc->fiber.enable))){
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_1000X_MII_CTRLr(unit, pc, &tmp));
    
        /* phy_ge_init has reset the phy, powering down the unstrapped 
         * interface 
         */
        /* make sure enabled interfaces are powered up */
        /* SGMII (passthrough fiber) or GMII fiber regs */
        if ((pc->fiber.enable) || (pc->interface == SOC_PORT_IF_SGMII)) {
            tmp &= ~MII_CTRL_PD;     /* remove power down */
        } else {
            tmp |= MII_CTRL_PD;     /* Keep the media in power down state if
                                                   * not use. 
                                                   */
        }
    
        if (PHY_SGMII_AUTONEG_MODE(unit, port)) {
            /*
             * Enable SGMII autonegotiation on the switch side so that the
             * link status changes are reflected in the switch. 
             */ 
            tmp |= MII_CTRL_AE;
        } else {
            tmp &= ~MII_CTRL_AE;    /* disable SGMII autoneg. */
        }
    
        SOC_IF_ERROR_RETURN
            (WRITE_PHY5464_1000X_MII_CTRLr(unit, pc, tmp));
       
        /* copper regs */
        /* remove power down */
        if (pc->copper.enable) {
            tmp = PHY_DISABLED_MODE(unit, port) ? MII_CTRL_PD : 0;
        } else {
            tmp = MII_CTRL_PD;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_CTRLr(unit, pc, tmp, MII_CTRL_PD));
    }

    if (NULL != int_pc) {
        SOC_IF_ERROR_RETURN
            (PHY_INIT(int_pc->pd, unit, port));
    } 

#if ERRATA_FIX
    /*
     * Perform Errata workarounds for 5464SA0 silicon.
     * NOTE: Assumes PHY internal port number 1 to 4 is ((port & 3) + 1).
     */

    if (PHY_IS_BCM5464(pc) && pc->phy_rev == PHY_REV_A0 &&
        (port & 3) == 0) {

        uint8       phy_addr2 = PORT_TO_PHY_ADDR(unit, port + 1);
        DPRINTF((BSL_META_U(unit,
                            "_phy_5464_reset_setup: u=%d p=%d ERRATA\n"),
                 unit, port));

        /* Internal Regulators Errata */

        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x18, 0x0282));

        /* Twisted Pair Output Voltage Errata */

        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x1e, 0x0012));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_write)
            (unit, phy_addr2, 0x18, 0x0007));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_read)
            (unit, phy_addr2, 0x18, &tmp));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_write)
            (unit, phy_addr2, 0x18, tmp | 0x0800));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_write)
            (unit, phy_addr2, 0x17, 0x000a));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_read)
            (unit, phy_addr2, 0x15, &tmp));
        SOC_IF_ERROR_RETURN
            ((DRV_SERVICES(unit)->miim_write)
            (unit, phy_addr2, 0x15, tmp | 0x1000));

    }

    if (PHY_IS_BCM5461(pc) && pc->phy_rev == PHY_REV_A0) {
        /* Internal Regulators Erratum for 5461 (reg 0x18 shadow b'010) */

        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x18, 0x0602));
    }
#endif /* ERRATA_FIX */

    /* Disable super-isolate */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_MII_POWER_CTRLr(unit, pc, 0x0, 1U<<5));

#if DISABLE_CLK125
    /* Reduce EMI emissions by disabling the CLK125 pin if not used */
    SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x1c, 0x1400));
    SOC_IF_ERROR_RETURN
            (READ_PHY_REG(unit, pc, 0x1c, &tmp));
    tmp &= ~1;
    SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x1c, tmp|0x8000));
#endif

    if (pc->automedium || pc->fiber.preferred) {
        if (pc->interface == SOC_PORT_IF_SGMII) {
            /* passthru mode support only in sgmii mode */
            /* Enable SerDes Pass-Through */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_EXP_SERDES_PASSTHRU_ENr(unit, pc, 0x0001, 0x0001));

            if (!pc->automedium && pc->fiber.preferred) {
                /* Set register 0x1c shadow "10111" bit 1 to force SerDes 
                 * passthru */
                /* Misc 1000-X Control register (0x1c shadow 10111) */
                /* Medium Resolution override for SerDes passthru mode */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_MISC_1000Xr(unit, pc, 0x0002, 0x0002));
            }
        }
        
        if (PHY_IS_BCM5461(pc)) {
            switch (pc->fiber_detect) {
            case 1:             /* PECL SD */
            case -1:
            case 0:             /* default */
                SOC_IF_ERROR_RETURN
                    (phy_reg_ge_write(unit, pc, 0x00, 0x000c, 0x1c, 0x0020));
                break;
            case 10:            /* EN_10B */
            case -10:
                SOC_IF_ERROR_RETURN
                    (phy_reg_ge_write(unit, pc, 0x00, 0x000c, 0x1c, 0x0000));
                break;
            default:
                return SOC_E_CONFIG;
            }
        } else {        /* multiport */
            switch (pc->fiber_detect) {
            case 4:             /* LED4 */
            case -4:
            case 0:             /* default */
                /* GPIO Control/Status Register (0x1c shadow 01111) */
                /* Change LED4 pin into an input */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_LED_GPIO_CTRLr(unit, pc, 0x0008, 0x0008));
                break;
            default:
                return SOC_E_CONFIG;
            }
        }
    }

    /* Configure Auto-detect Medium (0x1c shadow 11110) */
    /* In SGMII mode, the priority must be copper.
     * 1) Optical modules generate a SD or LOS event if there is no activity.
     *    This would cause 5464S to falsely go into fiber mode when there is
     *    no link partner connected and cause 5464S to never link up a valid
     *    copper link partner.
     * 2) When 5464S has a copper link, it cannot send out any signals on
     *    it PTOUT+/- because 5464S is in SSGMII to copper mode and not
     *    SerDes pass-through mode. This will cause link partner to never
     *    link up even if a fiber cable is connected between them because
     *    their PTOUT+/- pins are disabled.
     */
    tmp = 0;
    if (pc->automedium &&
        !(PHY_IS_BCM5464(pc) && pc->phy_rev == PHY_REV_A0)) {
        /* Must NOT be set on 5464A0, and it will always auto-detect */
        tmp |= 0x01;    /* Bit 0: Enable auto-detect */
    }
    if (pc->fiber.preferred) {
        tmp |= 0x06;    /* Bit 1/2: always prefer SERDES */
    } else {
        tmp |= 0x04;    /* Bit 2: prefer SERDES if no medium active */
    }

    /*
     * Enable internal inversion of LED4/SD input pin.  Used only if
     * the fiber module provides RX_LOS instead of Signal Detect.
     */
    if (pc->fiber_detect < 0) {
        tmp |= 0x100;
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_AUTO_DETECT_MEDIUMr(unit, pc, tmp, 0x011f));

    if ((!pc->automedium) && 
        (pc->interface == SOC_PORT_IF_GMII)) {
        /* Set operating mode only after auto-detect is disabled.
         * Otherwise, the auto-detect will override the mode settings.
         */
        /* Mode Control Register
         * Mode Select = >  00:     GMII Copper
         *    [2:1]         01:     GMII Fiber
         *                  10:     SGMII
         *                  11:     GBIC(Media Converter)
         */
        if (PHY_COPPER_MODE(unit, port)) {
            tmp = (0 << 1);         /* copper/GMII */
        } else {
            tmp = (1 << 1);         /* fiber/GMII */
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MODE_CTRLr(unit, pc, tmp, 0x0006));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_ECRr(unit, pc, 0, (1 << 15)));
    }

    /*
     * Configure Auxiliary 1000BASE-X control register to turn off
     * carrier extension.  The Intel 7131 NIC does not accept carrier
     * extension and gets CRC errors.
     */
    /* Disable carrier extension */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_AUX_1000X_CTRLr(unit, pc, 0x0040, 0x0040));

    /* Configure Extended Control Register */
    tmp  = 0x0020;      /* Enable LEDs to indicate traffic status */
    tmp |= 0x0001;      /* Set high FIFO elasticity to support jumbo frames */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_MII_ECRr(unit, pc, tmp, tmp));

    if (PHY_IS_BCM54980(pc) || PHY_IS_BCM54980C(pc) ||
        PHY_IS_BCM54980V(pc) || PHY_IS_BCM54980VC(pc)) {

        /* set 54980 elasticity fifo to support 13.5k frame size*/
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MISC_1000X_CTRL2r(unit,pc, 1, 1));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUX_1000X_CTRLr(unit,pc, 0, (1 << 1)));
    }

#if AUTO_MDIX_WHEN_AN_DIS
    /* Enable Auto-MDIX When autoneg disabled */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200));
#endif

    /* Enable extended packet length (4.5k through 25k) */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY5464_MII_AUX_CTRLr(unit, pc, 0x4000, 0x4000));

    /* Configure LED selectors */
    tmp = ((pc->ledmode[1] & 0xf) << 4) | (pc->ledmode[0] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5464_LED_SELECTOR_1r(unit, pc, tmp));

    tmp = ((pc->ledmode[3] & 0xf) << 4) | (pc->ledmode[2] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5464_LED_SELECTOR_2r(unit, pc, tmp));

    tmp = (pc->ledctrl & 0x3ff);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5464_LED_CTRLr(unit, pc, tmp));

    SOC_IF_ERROR_RETURN
        (WRITE_PHY5464_EXP_LED_SELECTORr(unit, pc, pc->ledselect));

    /* if using led link/activity mode, disable led traffic mode */
    if (pc->ledctrl & 0x10 || pc->ledselect == 0x01) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_ECRr(unit, pc, 0, 0x0020));
    }

    if (PHY_IS_BCM5478(pc)  || PHY_IS_BCM5488(pc)  ||
        PHY_IS_BCM54980(pc) || PHY_IS_BCM54980C(pc) ||
        PHY_IS_BCM54980V(pc) || PHY_IS_BCM54980VC(pc)) {
        uint16 half, half1k;
        uint32 power_mode; 
  
        tmp    = 0;
        half   = 0;
        half1k = 0;
        if (pc->halfout) {
            tmp &= (~0x40);     /* Class A mode */
            if ((pc->halfout == 10) || (pc->halfout == 100)) { 
                half |= 0x8;    /* Enable half amplitude for all speed */
            } else {
                half1k |= 0x2;  /* Enable half amplitude for Giga */
            }
        } else {
            tmp |= 0x40;        /* Class A/B mode */ 
            half &= (~0x8);     /* Disable half amplitude for Giga */
            half1k &= (~0x2);   /* Disable half amplitude for all speed */

            /* For extended reach, default to highest performance mode 
             * (REF: 5488R-ES104-R)
             */
            power_mode = soc_property_port_get(unit, port, 
                                               spn_PHY_LOW_POWER_MODE, 7);
            tmp |= (power_mode & 0x7) << 13; /* Set power mode */
        }

        /* Update Power/MII Control Register (0x18 shadow b'010) */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_POWER_CTRLr(unit, pc, tmp, 0xe040));

        /* Update Test Register 1 (0x18 shadow b'100) */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_MISC_TESTr(unit, pc, half, 0x0008));

        /* Update Expansion Register 0xF9 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_EXP_PASSTHRU_LED_MODEr(unit, pc, half1k, 0x02));
    }

    /* BCM54980 revB 10BASE-T LOW TRANSMITTER AMPLITUDE workaround.
     * See 54980 errata 54980-ES105-R
     */
    if (PHY_IS_BCM54980(pc) || PHY_IS_BCM54980C(pc) ||
        PHY_IS_BCM54980V(pc) || PHY_IS_BCM54980VC(pc)) {

        /* only applies to B revisions */
        if ((pc->phy_id1 & 0xF) <= 2) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY5464_EXP_10BaseT_TxAMPr(unit,pc,0x3800));

            SOC_IF_ERROR_RETURN
                (WRITE_PHY5464_TEST1r(unit,pc,0x0011));

            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0C00,0x0FFF));

            /* write 0xA to reg 0x17 and 0x90B to reg 15 */
            SOC_IF_ERROR_RETURN
                (PHY5464_REG_WRITE(unit,pc,0,0xA,0x15,0x090B));

            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0400,0x0FFF));
        }
    }

    if (PHY_IS_BCM5488(pc) || PHY_IS_BCM5482(pc)) {
        /* Adjust timing */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_SPARE_CTRLr(unit, pc, 0x0002, 0x0002));
    }

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

#if PHY53115_A0_AN100TX_WORKAROUND
    if (phy53115_an100tx_war){ /* config set to stop this workaround */

        if (PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                        PHY_BCM53115_OUI, BCM53115_A0_REV_ID)){
            PHY53115_ERRATA_AN100TX_STAT_CLEAR(port);
            PHY53115_ERRATA_AN100TX_LINK_CLEAR(port);
            PHY53115_ERRATA_AN100TX_LINKERR_CLEAR(port);
            phy53115_errata_an100tx_failed_cnt[port] = 0;
            DPRINTF((BSL_META_U(unit,
                                "_robo_phy_5464_reset_setup,"
                                "Clear ge%d workaround sw-info!!\n"),
                     port));
        }
    }
#endif  /* PHY53115_A0_AN100TX_WORKAROUND */
        
#if PHY53115_IOP_WORKAROUND
    if (PHY_MODEL_CHECK(pc, PHY_BCM53115_OUI, PHY_BCM53115_MODEL)){
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53115_A0_REV_ID) || 
                (rev_id == BCM53115_A1_REV_ID) || 
                (rev_id == BCM53115_B1_REV_ID)){
            
            _phy53115_a0_iop_errata(unit, port);
        } else if ((rev_id == BCM53115_B0_REV_ID)){
            PHY53115_ERRATA_IOP_CLEAR(port);
            phy53115_errata_iop_cnt[port] = 0;
            DPRINTF((BSL_META_U(unit,
                                "_robo_phy_5464_reset_setup,"
                                "Reset ge%d IOP-WAR-#2 sw-info!!\n"),
                     port));
        }
    }
    
    /* for bcm53118_a0 also */
    if (PHY_MODEL_CHECK(pc, PHY_BCM53118_OUI, PHY_BCM53118_MODEL)){
        if ((rev_id == BCM53118_A0_REV_ID)){
            PHY53115_ERRATA_IOP_CLEAR(port);
            phy53115_errata_iop_cnt[port] = 0;
            DPRINTF((BSL_META_U(unit,
                                "_robo_phy_5464_reset_setup,"
                                "Reset ge%d IOP-WAR-#2 sw-info!!\n"),
                     port));
        }
    }

#endif  /* PHY53115_IOP_WORKAROUND */

#if PHY53118_B0_WORKAROUND
    /* For bcm53118_B0 only.
     *
     *  Workaround for two testing:
     *  1. IEEE 1000BASE-T testing
     *  2. EMI testing
     */
    if (PHY_MODEL_CHECK(pc, PHY_BCM53118_OUI, PHY_BCM53118_MODEL)){
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53118_B0_REV_ID)){

            /* write to reg 18h, shadow 000b */
            SOC_IF_ERROR_RETURN (
                WRITE_PHY5464_MII_AUX_CTRLr(unit, pc,0x0c00));

            /* write expansion register */
            WRITE_EXP(0x0f75, 0x0029);
            WRITE_EXP(0x0f76, 0x0100);
            WRITE_EXP(0x0f74, 0xd087);

            SOC_IF_ERROR_RETURN \
                    (WRITE_PHY_REG(unit, pc, 0x17, 0x601f));
            SOC_IF_ERROR_RETURN \
                    (WRITE_PHY_REG(unit, pc, 0x15, 0x0110));

            DPRINTF((BSL_META_U(unit,
                                "_robo_phy_5464_reset_setup, "
                                "ge%d BCM53118_B0 PHY-WAR-sw-info!!\n"),
                     port));
        }
    }
#endif  /* PHY53118_B0_WORKAROUND */

#if PHY53118_B1_WORKAROUND
    /* Note for "PHY53118_B1 Workaround": 
     *
     *  Workaround for:
     *  ADC clock 
     *    - for 1000/100 BT link issue
     */
    if (PHY_MODEL_CHECK(pc, PHY_BCM53118_OUI, PHY_BCM53118_MODEL)){
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53118_B1_REV_ID)){

            /* write expansion register */

            /* ADC clock : for 1000/100 BT link issue */
            WRITE_EXP(0x0f76, 0x0040);
            /* Adjusted per design team's comment */
            WRITE_EXP(0x0f75, 0x003d);

            /* Restart Autonego */
            SOC_IF_ERROR_RETURN
                (robo_phy_5464_autoneg_set(unit, port, 1));

            DPRINTF((BSL_META_U(unit,
                                "_robo_phy_5464_reset_setup, "
                                "ge%d BCM53118_B1 PHY-WAR-sw-info!!\n"),
                     port));
        }
    }
#endif  /* PHY53118_B1_WORKAROUND */


#if PHY65nm_LINK2MRVL_WORKAROUND
    /* Unstable link issue with Marvell PHY
     * Workaroud (PHY team confirmed workaround has been identified) :
     * Set Exp register 0x08.bit[9]=0.
     * (BIT9 is AUTO EARLY DAC WAKE power savings mode)
     *
     */
    if (PHY_IS_BCM53115(pc) || PHY_IS_BCM53118(pc)){
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53115_C0_REV_ID) || 
                (rev_id == BCM53118_B0_REV_ID) ||
                (rev_id == BCM53118_B1_REV_ID)){
            SOC_IF_ERROR_RETURN(
                    MODIFY_PHY5464_10BT_CTRLr(unit, pc, 0x0000, 0x0200));
        }
    }
#endif  /* PHY65nm_LINK2MRVL_WORKAROUND */
    
    return SOC_E_NONE;

}

/*
 * Function:
 *  robo_phy_5464_init
 * Purpose:
 *  Init function for 5464 PHY.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 * Returns:
 *  SOC_E_NONE
 */

STATIC int
robo_phy_5464_init(int unit, soc_port_t port)
{

    phy_ctrl_t         *pc, *int_pc;
    int                 fiber_capable, fiber_preferred, type_s=0;
    uint8               phy_addr;
    uint16              tmp;
    int                 temp = 0;

    phy_addr = PORT_TO_PHY_ADDR(unit, port);

    DPRINTF((BSL_META_U(unit,
                        "robo_phy_5464_init: u=%d p=%d phy_addr=%d\n"),
             unit, port, (int)phy_addr));
        
    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc  = INT_PHY_SW_STATE(unit, port);

    /* bcm5348/5347/53262 bcm5389/bcm5396 interface == sgmii */
    if (soc_feature(unit, soc_feature_dodeca_serdes)){
        pc->interface = SOC_PORT_IF_SGMII;
    } else {
        if (int_pc != NULL){
            pc->interface = SOC_PORT_IF_SGMII;
        } else {
            pc->interface = SOC_PORT_IF_GMII;
        }
    }

#if PHY53115_A0_AN100TX_WORKAROUND
    phy53115_an100tx_war = 
            soc_property_port_get(unit, port, spn_PHY_53115_AN100TX_WAR, 1);
#endif  /* PHY53115_A0_AN100TX_WORKAROUND */

#if PHY53115_IOP_WORKAROUND
    phy53115_b0_iop_war = 
            soc_property_port_get(unit, port, spn_PHY_53115_B0_IOP_WAR, 1);
#endif  /* PHY53115_IOP_WORKAROUND */

   if (PHY_IS_BCM5461(pc)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_MODE_CTRLr(unit, pc, &tmp));
        type_s = (tmp & 0x8) ? 1 : 0;
    } else if (PHY_IS_BCM5464(pc) || PHY_IS_BCM5466(pc)) {

        /* Give default configuration for bcm5348 SDK board */
        if (SOC_IS_ROBO53262(unit)) {
            temp = 1;
        }
        /*
         * On the 5464 family, we need the user to set a property to know
         * if we are on an S part.  See comments at top of file.
         */
        type_s = soc_property_port_get(unit, port, spn_PHY_5464S, temp);
    }

    if (soc_feature(unit, soc_feature_dodeca_serdes) && int_pc != NULL) {
        if (PHY_IS_BCM5461(pc)) {
            if (!type_s) {
                return SOC_E_FAIL;
            }
            fiber_capable = 0;
        } else {
            fiber_capable = type_s;
        }
    } else {
        if (PHY_IS_BCM5461(pc)) {
            fiber_capable = type_s;
        } else {
            fiber_capable = 1;
        }
    }
    
    /* this flag is set in soc_phyctrl_init() */
    if (PHY_FORCED_COPPER_MODE(unit, port)){
        /* 5398, 5395, 53115, 53118, 5396, 5389, 53125, 53128*/
        fiber_capable = 0;
    }
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
        pc->automedium = 0;
        pc->fiber_detect = 0;
    } else {
        /* default FIBER PREF = 0 */
        tmp = 0;
        fiber_preferred =
            soc_property_port_get(unit, port, spn_PHY_FIBER_PREF, tmp);
        pc->automedium =
            soc_property_port_get(unit, port, spn_PHY_AUTOMEDIUM, 1);

        temp  = 0;
        /* Give default configuration for bcm5348 SDK board */
        if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            if (port != CMIC_PORT(unit)) {
                temp = -4;
            } else { /* CMIC Port */
                temp = 0;
            }
        }
        pc->fiber_detect =
            soc_property_port_get(unit, port, spn_PHY_FIBER_DETECT, temp);
    }

    DPRINTF((BSL_META_U(unit,
                        "phy_5464_init: u=%d p=%d "
                        "type=546%d%s automedium=%d fiber_pref=%d "
                        "detect=%d if=%s\n"),
             unit, port,
             pc->phy_model == PHY_BCM5461_MODEL ? 1 : 4,
             type_s ? "S" : "",
             pc->automedium, fiber_preferred, pc->fiber_detect,
             (pc->interface == SOC_PORT_IF_SGMII) ? "SGMII" : "GMII"));

    pc->copper.enable = TRUE;
    pc->copper.preferred = !fiber_preferred;
    pc->copper.autoneg_enable = TRUE;
    pc->copper.autoneg_advert = ADVERT_ALL_COPPER;
    pc->copper.force_speed = 1000;
    pc->copper.force_duplex = TRUE;
    pc->copper.master = SOC_PORT_MS_AUTO;
    pc->copper.mdix = SOC_PORT_MDIX_AUTO;
    DPRINTF((BSL_META_U(unit,
                        "phy_5464_init: u=%d p=%d COPPER "
                        "enable=%d pref=%d an=%d an_adv=0x%x\n"),
             unit, port,
             pc->copper.enable, pc->copper.preferred, 
             pc->copper.autoneg_enable, pc->copper.autoneg_advert));

    pc->fiber.enable = fiber_capable;
    pc->fiber.preferred = fiber_preferred;
    pc->fiber.autoneg_enable = fiber_capable;
    pc->fiber.autoneg_advert = ADVERT_ALL_FIBER;
    pc->fiber.force_speed = 1000;
    pc->fiber.force_duplex = TRUE;
    pc->fiber.master = SOC_PORT_MS_NONE;
    pc->fiber.mdix = SOC_PORT_MDIX_NORMAL;

    DPRINTF((BSL_META_U(unit,
                        "phy_5464_init: u=%d p=%d FIBER "
                        "enable=%d pref=%d an=%d an_adv=0x%x\n"),
             unit, port,
             pc->fiber.enable, pc->fiber.preferred, 
             pc->fiber.autoneg_enable, pc->fiber.autoneg_advert));

    /* Initially configure for the preferred medium. */

    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_PASSTHRU);
    if (pc->fiber.preferred) {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
        if (pc->interface == SOC_PORT_IF_SGMII) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_PASSTHRU);
        }
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
    }

    /* Get Requested LED selectors (defaults are hardware defaults) */
    pc->ledmode[0] = soc_property_port_get(unit, port, spn_PHY_LED1_MODE, 0);
    pc->ledmode[1] = soc_property_port_get(unit, port, spn_PHY_LED2_MODE, 1);
    pc->ledmode[2] = soc_property_port_get(unit, port, spn_PHY_LED3_MODE, 3);
    pc->ledmode[3] = soc_property_port_get(unit, port, spn_PHY_LED4_MODE, 6);
    pc->ledctrl    = soc_property_port_get(unit, port, spn_PHY_LED_CTRL, 0x8);
    pc->ledselect  = soc_property_port_get(unit, port, spn_PHY_LED_SELECT, 0);

    /* Get half amplitude setting */
    pc->halfout = soc_property_port_get(unit, port, spn_PHY_HALF_POWER, 0);
    
    SOC_IF_ERROR_RETURN
        (_robo_phy_5464_reset_setup(unit, port));

    SOC_IF_ERROR_RETURN
        (_phy_5464_medium_config_update(unit, port,
                                        PHY_COPPER_MODE(unit, port) ?
                                        &pc->copper :
                                        &pc->fiber));

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_enable_set
 * Purpose:
 *  Enable or disable the physical interface.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  enable - Boolean, true = enable PHY, false = disable.
 * Returns:
 *  SOC_E_XXX
 * Note : 
 *      1. We use power down to force both the local and remote port been 
 *          linked down but using the supoer isolate setting for below reason.
 *          - set super isolate might causes the combo medium solution been 
 *             incorrect when the fiber medium connected. 
 *          - Suport isolate mode can only be active in copper medium. 
 *              So the fiber medium will be link up on both port but driver 
 *              reports this indicated port as link-down for the SW 
 *              PHY_DISABLE_FLAG.
 *              And this causes remote fiber port been linked still.
 *      2. Here we set the disable flag and the power down setting will be 
 *          processed at reset_setup routine.
 */

STATIC int
robo_phy_5464_enable_set(int unit, soc_port_t port, int enable)
{

    phy_ctrl_t    *pc;
    uint16 power_down;

    pc = EXT_PHY_SW_STATE(unit, port);

    power_down = (enable) ? 0 : MII_CTRL_PD;

    if ((pc->copper.enable) &&
        (pc->automedium || PHY_COPPER_MODE(unit, port))) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_CTRLr(unit, pc, power_down, MII_CTRL_PD));
    }

    if ((pc->fiber.enable) && 
        (pc->automedium || PHY_FIBER_MODE(unit, port))) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_1000X_MII_CTRLr(unit, pc, 
                    power_down, MII_CTRL_PD));       
    }

    /* Update software state */
    SOC_IF_ERROR_RETURN(phy_fe_ge_enable_set(unit, port, enable));

    return SOC_E_NONE;

}

/*
 * Function:
 *  robo_phy_5464_enable_get
 * Purpose:
 *  Enable or disable the physical interface for a 5464 device.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  enable - (OUT) Boolean, true = enable PHY, false = disable.
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
robo_phy_5464_enable_get(int unit, soc_port_t port, int *enable)
{
    return phy_fe_ge_enable_get(unit, port, enable);
}

STATIC int
_robo_phy_5464_medium_change(int unit, soc_port_t port, int force_update)
{
    phy_ctrl_t    *pc;
    int            medium;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (_robo_phy_5464_medium_check(unit, port, &medium));

    /*
     * If the medium is changing, reset the PHY, change its
     * operating mode and 5690 SERDES operating mode, and reprogram
     * the persistent copper/fiber parameters.  Also make a user
     * callback to inform them of medium status change.  An
     * application could also use this to reprogram the port
     * parameters quickly after a medium status change.
     */

    if (medium == SOC_PORT_MEDIUM_COPPER) {
        if ((!PHY_COPPER_MODE(unit, port)) || force_update) {
            DPRINTF((BSL_META_U(unit,
                                "_phy_5464_link_auto_detect: "
                                "u=%d p=%d [F->C]\n"),
                     unit, port));

            PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_PASSTHRU);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
            SOC_IF_ERROR_RETURN
                (_robo_phy_5464_reset_setup(unit, port));

            if (pc->copper.enable) {
                SOC_IF_ERROR_RETURN
                    (_phy_5464_medium_config_update(unit, port, &pc->copper));
            }
        } else {
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
            /* Configure Auto-detect Medium (0x1c shadow 11110) */
            /* should not prefer SERDES */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_AUTO_DETECT_MEDIUMr(unit, pc, 0, 0x0006));
        }
    } else {        /* Fiber */
        if (PHY_COPPER_MODE(unit, port) || force_update) {
            DPRINTF((BSL_META_U(unit,
                                "_phy_5464_link_auto_detect: "
                                "u=%d p=%d [C->F]\n"),
                     unit, port));

            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
            if (pc->interface == SOC_PORT_IF_SGMII) {
                PHY_FLAGS_SET(unit, port, PHY_FLAGS_PASSTHRU);
            }
            SOC_IF_ERROR_RETURN
                (_robo_phy_5464_reset_setup(unit, port));
            if (pc->fiber.enable) {
                SOC_IF_ERROR_RETURN
                    (_phy_5464_medium_config_update(unit, port, &pc->fiber));
            }
        } else {
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_MEDIUM_CHANGE);
        } 
    }
    return SOC_E_NONE;
}

STATIC int
_robo_phy_5464_fiber_link_get(int unit, soc_port_t port, int *link)
{
    uint16              mii_stat;
    uint16              mii_ctrl;
    soc_timeout_t       to;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_STATr(unit, pc, &mii_stat));

    if (!(mii_stat & MII_STAT_LA) || (mii_stat == 0xffff)) {
        *link = FALSE;
        return SOC_E_NONE;
    }

    /* Link appears to be up; we are done if autoneg is off. */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_CTRLr(unit, pc, &mii_ctrl));

    if (!(mii_ctrl & MII_CTRL_AE)) {
        *link = TRUE;
        return SOC_E_NONE;
    }

    /*
     * If link appears to be up but autonegotiation is still in
     * progress, wait for it to complete.  For BCM5228, autoneg can
     * still be busy up to about 200 usec after link is indicated.  Also
     * continue to check link state in case it goes back down.
     */
    soc_timeout_init(&to, PHY_AN_TIMEOUT(unit, port), 0);
    for (;;) {
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_1000X_MII_STATr(unit, pc, &mii_stat));

        if (!(mii_stat & MII_STAT_LA)) {
            return SOC_E_NONE;
        }

        if (mii_stat & MII_STAT_AN_DONE) {
            break;
        }

        if (soc_timeout_check(&to)) {
            return SOC_E_BUSY;
        }
    }

    /* Return link state at end of polling */
    *link = ((mii_stat & MII_STAT_LA) != 0);

    return SOC_E_NONE;
}


/*
 * Function:
 *  robo_phy_5464_link_get
 * Purpose:
 *  Determine the current link up/down status for a 5464 device.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  link - (OUT) Boolean, true indicates link established.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  If using automedium, also switches the mode.
 */
STATIC int
robo_phy_5464_link_get(int unit, soc_port_t port, int *link)
{
    phy_ctrl_t    *pc;
    int           rv = SOC_E_NONE;

    pc = EXT_PHY_SW_STATE(unit, port);

    *link = FALSE;      /* Default return */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }

    if (!pc->fiber.enable && !pc->copper.enable) {
        return SOC_E_NONE;
    }

    if (pc->automedium) {
       SOC_IF_ERROR_RETURN
           (_robo_phy_5464_medium_change(unit, port, FALSE));
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
                *link = (mii_stat & MII_STAT_LA) ? TRUE : FALSE;
            } else {
                return rv;
            }
        } else if (rv){
            return rv;
        }
            
#if PHY53115_IOP_WORKAROUND
        /* for bcm53115_b0 and bcm53118 ONLY */
        if (SOC_IS_VULCAN(unit) || PHY_IS_BCM53118(pc)){
            _phy53115_b0_iop_errata(unit, port, *link);
        }
#endif  /* PHY53115_IOP_WORKAROUND */
            
#if PHY53115_A0_AN100TX_WORKAROUND  /* phy53115_a0 workaround */
        if (SOC_IS_VULCAN(unit)){    /* for bcm53115_a0 ONLY */
            _phy53115_a0_errata(unit, port, *link);
            
            _phy53115_a0_errata_link_check(unit, port, *link);
        }
#endif  /* phy53115_a0 workaround */

    } else { /* fiber mode */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            
            phy_ctrl_t    *int_pc;   /* PHY software state */
            int_pc = INT_PHY_SW_STATE(unit, port);

            if (NULL != int_pc) {
                /* In passthrough mode, call internal PHY driver */
                SOC_IF_ERROR_RETURN
                    (PHY_LINK_GET(int_pc->pd, unit, port, link));
            } else {
                return SOC_E_INTERNAL;
            }            
        } else { /* GMII fiber mode */
            /* PHYs that do not support passthrough mode must read
             * link status from 1000X status register if the port
             * is in fiber mode.
             */ 
            SOC_IF_ERROR_RETURN
                (_robo_phy_5464_fiber_link_get(unit, port, link));
        }
    }
    if ((pc->automedium) && (pc->fiber.preferred)) {
        uint16      copper_transmit;

        copper_transmit = 0;
        /* Disable/Enable Copper transmitter */
        if (PHY_FIBER_MODE(unit, port)) {
            copper_transmit = (*link) ? 0x2000 : 0; 
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_ECRr(unit, pc, copper_transmit, 0x2000));
    }

    DPRINTF_VERBOSE((BSL_META_U(unit,
                                "phy_5464_link_get: u=%d p=%d "
                                "mode=%s%s%s link=%d\n"),
                     unit, port,
                     PHY_COPPER_MODE(unit, port) ? "C" : "",
                     PHY_FIBER_MODE(unit, port) ? "F" : "",
                     PHY_PASSTHRU_MODE(unit, port) ? "P" : "",
                     *link));

    return SOC_E_NONE;

}

/*
 * Function:
 *  robo_phy_5464_duplex_set
 * Purpose:
 *  Set the current duplex mode (forced).
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  duplex - Boolean, true indicates full duplex, false indicates half.
 * Returns:
 *  SOC_E_NONE
 *  SOC_E_UNAVAIL - Half duplex requested, and not supported.
 * Notes:
 *  The duplex is set only for the ACTIVE medium.
 *  No synchronization performed at this level.
 *  Autonegotiation is not manipulated.
 */

STATIC int
robo_phy_5464_duplex_set(int unit, soc_port_t port, int duplex)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);

    DPRINTF((BSL_META_U(unit,
                        "robo_phy_5464_duplex_set: u=%d p=%d d=%d\n"),
             unit, port, duplex));

    if (PHY_COPPER_MODE(unit, port)) {
        /* Note: mac.c uses phy_5690_notify_duplex to update 5690 */
        SOC_IF_ERROR_RETURN(phy_fe_ge_duplex_set(unit, port, duplex));
        pc->copper.force_duplex = duplex;
    } else {    /* Fiber */
        if (!duplex) {
            return SOC_E_UNAVAIL;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_duplex_get
 * Purpose:
 *  Get the current operating duplex mode. If autoneg is enabled,
 *  then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  duplex - (OUT) Boolean, true indicates full duplex, false
 *      indicates half.
 * Returns:
 *  SOC_E_NONE
 * Notes:
 *  The duplex is retrieved for the ACTIVE medium.
 *  No synchronization performed at this level. Autonegotiation is
 *  not manipulated.
 */

STATIC int
robo_phy_5464_duplex_get(int unit, soc_port_t port, int *duplex)
{
    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_duplex_get(unit, port, duplex);
    }
    *duplex = TRUE;
    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_speed_set
 * Purpose:
 *  Set the current operating speed (forced).
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  speed - Requested speed, only 1000 supported.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The speed is set only for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_speed_set(int unit, soc_port_t port, int speed)
{
    int            rv;
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);

    DPRINTF((BSL_META_U(unit,
                        "robo_phy_5464_speed_set: u=%d p=%d s=%d fiber=%d\n"),
             unit, port, speed, PHY_FIBER_MODE(unit, port)));

    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        /* Note: mac.c uses phy_5690_notify_speed to update 5690 */
        rv = (phy_fe_ge_speed_set(unit, port, speed));
        if (SOC_SUCCESS(rv)) {
            pc->copper.force_speed = speed;
            
#if PHY53115_A0_AN100TX_WORKAROUND
            /* the forece cases at 100 speed have link problem also on 
             * phy53115_a0 also. 
             *
             * This section is applied when user set force speed(AN disable).
             * The phy configuration after this workaround will the same with 
             *  user setting. So we don't need to keep the workaround sw-info
             *  when applying this workaround section in below.
             */
            if (SOC_IS_VULCAN(unit)){
                
                if (phy53115_an100tx_war) {
                    if(PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                                    PHY_BCM53115_OUI, BCM53115_A0_REV_ID)){
                        if(speed == 100){
                            DPRINTF((BSL_META_U(unit,
                                                "robo_phy_5464_speed_set,"
                                                "Set ge%d WAR for "
                                                "phy53115_a0!!\n"), 
                                     port));
                            rv = WRITE_PHY_REG(unit, pc, 0x17, 0x0fff);
                            rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x20c0);
                            if (rv){
                                DPRINTF((BSL_META_U(unit,
                                                    "robo_phy_5464_speed_set, "
                                                    "%d,"
                                                    "Failed PHY Reg wrtie!\n"), 
                                         __LINE__));
                            }
                        }
                    }
                }
            }
#endif  /* PHY53115_A0_AN100TX_WORKAROUND */
        }
    } else {    /* Fiber */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            phy_ctrl_t    *int_pc;   /* PHY software state */

            int_pc = INT_PHY_SW_STATE(unit, port);

            if (NULL != int_pc) {
                /* In passthrough mode, call internal PHY driver */
                rv = PHY_SPEED_SET(int_pc->pd, unit, port, speed);
            } else {
                rv = SOC_E_INTERNAL;
            }
 
        } else if (speed != 0 && speed != 1000) {
            rv = SOC_E_CONFIG;
        }

        if (SOC_SUCCESS(rv)) {
            pc->fiber.force_speed = speed;
        }
    }

    return rv;

}

/*
 * Function:
 *  robo_phy_5464_speed_get
 * Purpose:
 *  Get the current operating speed for a 5464 device.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  duplex - (OUT) Boolean, true indicates full duplex, false
 *      indicates half.
 * Returns:
 *  SOC_E_NONE
 * Notes:
 *  The speed is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_speed_get(int unit, soc_port_t port, int *speed)
{
    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_speed_get(unit, port, speed);
    } else {
        if (PHY_PASSTHRU_MODE(unit, port)) {
            /* If the PHY is in passthrough mode, phyctrl calls the inner 
             * PHY driver speed_get().
             */
            return SOC_E_INTERNAL;
        } else {
            *speed = 1000;
        }
    }
    
    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_master_set
 * Purpose:
 *  Set the current master mode
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  master - SOC_PORT_MS_*
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The master mode is set only for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_master_set(int unit, soc_port_t port, int master)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);

    DPRINTF((BSL_META_U(unit,
                        "robo_phy_5464_master_set: "
                        "u=%d p=%d master=%d fiber=%d\n"),
             unit, port, master, PHY_FIBER_MODE(unit, port)));

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN(phy_fe_ge_master_set(unit, port, master));
        pc->copper.master = master;
        return SOC_E_NONE;
    } else {
        return SOC_E_NONE;
    }
}

/*
 * Function:
 *  robo_phy_5464_master_get
 * Purpose:
 *  Get the current master mode for a 5464 device.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  master - (OUT) SOC_PORT_MS_*
 * Returns:
 *  SOC_E_NONE
 * Notes:
 *  The master mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_master_get(int unit, soc_port_t port, int *master)
{
    if (PHY_COPPER_MODE(unit, port)) {
        return phy_fe_ge_master_get(unit, port, master);
    } else {
        if (PHY_PASSTHRU_MODE(unit, port)) {
            /* If the PHY is in passthrough mode, phyctrl calls the inner 
             * PHY driver master_get().
             */
            return SOC_E_INTERNAL;
        } else {
            *master = SOC_PORT_MS_NONE;
        }
    }
    return SOC_E_NONE;    
}

/*
 * Function:
 *  robo_phy_5464_autoneg_set
 * Purpose:
 *  Enable or disable auto-negotiation on the specified port.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  autoneg - Boolean, if true, auto-negotiation is enabled
 *      (and/or restarted). If false, autonegotiation is disabled.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The autoneg mode is set only for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_autoneg_set(int unit, soc_port_t port, int autoneg)
{
    uint16               autoneg_enable;
    int                  rv;
    phy_ctrl_t    *pc;

    pc             = EXT_PHY_SW_STATE(unit, port);
    rv             = SOC_E_NONE;
    autoneg_enable = 0;

    if (PHY_COPPER_MODE(unit, port)) {
        /* Set auto-neg on PHY */
        rv = phy_fe_ge_an_set(unit, port, autoneg);
        if (SOC_SUCCESS(rv)) {
            pc->copper.autoneg_enable = autoneg ? 1 : 0;
        }
    } else { /* Fiber mode */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            phy_ctrl_t    *int_pc;   /* PHY software state */

            int_pc = INT_PHY_SW_STATE(unit, port);

            if (NULL != int_pc) {
                /* In passthrough mode, call internal PHY driver */
                rv = PHY_AUTO_NEGOTIATE_SET(int_pc->pd, unit, port, autoneg);
            } else {
                rv = SOC_E_INTERNAL;
            }
        } else {
            autoneg_enable = (autoneg) ? MII_CTRL_AE | MII_CTRL_RAN: 0;
            rv = MODIFY_PHY5464_1000X_MII_CTRLr(unit, pc, autoneg_enable,
                                                MII_CTRL_AE | MII_CTRL_RAN);
        }
        if (SOC_SUCCESS(rv)) {
            pc->fiber.autoneg_enable = autoneg ? 1 : 0;
        }  
    }

    DPRINTF((BSL_META_U(unit,
                        "phy_5464_autoneg_set: "
                        "u=%d p=%d autoneg=%d auto=%d rv=%d\n"),
             unit, port, autoneg, pc->automedium, rv));

    return rv;

}

STATIC int
_robo_phy_5464_fiber_an_get(int unit, soc_port_t port,
                       int *autoneg, int *autoneg_done)
{
    uint16               mii_ctrl;
    uint16               mii_stat;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_CTRLr(unit, pc, &mii_ctrl));

    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_STATr(unit, pc, &mii_stat));

    *autoneg      = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
    *autoneg_done = (mii_stat & MII_STAT_AN_DONE) ? TRUE : FALSE;

    return(SOC_E_NONE);
}

/*
 * Function:
 *  robo_phy_5464_autoneg_get
 * Purpose:
 *  Get the current auto-negotiation status (enabled/busy).
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  autoneg - (OUT) if true, auto-negotiation is enabled.
 *  autoneg_done - (OUT) if true, auto-negotiation is complete. This
 *          value is undefined if autoneg == FALSE.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The autoneg mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_autoneg_get(int unit, soc_port_t port,
             int *autoneg, int *autoneg_done)
{
    int rv;

    rv = SOC_E_NONE;

    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_an_get(unit, port, autoneg, autoneg_done);
    } else { /* Fiber mode */
        if (PHY_PASSTHRU_MODE(unit, port)) { /* switch SerDes passthrough PHY */
            /* If the PHY is in passthrough mode, phyctrl calls the inner
             * PHY driver autoneg_set().
             */
            rv = SOC_E_INTERNAL;
        } else { /* switch GMII to SerDes optical module */ 
            rv = _robo_phy_5464_fiber_an_get(unit, port, autoneg, autoneg_done);
        }
    }
    return rv;
}

STATIC int 
_robo_phy_5464_fiber_adv_local_set(int unit, soc_port_t port, 
                              soc_port_mode_t mode)
{
    uint16               mii_adv;  /* Value to advertise */
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    mii_adv = 0;
    if (mode & SOC_PM_1000MB_FD)    mii_adv |= (1<<5);
    if (mode & SOC_PM_1000MB_HD)    mii_adv |= (1<<6);
    switch (mode & SOC_PM_PAUSE) {
    case SOC_PM_PAUSE:
        mii_adv |= (0x1 << 7);
        break;
    case SOC_PM_PAUSE_TX:
        mii_adv |= (0x2 << 7);
        break;
    case SOC_PM_PAUSE_RX:
        mii_adv |= (0x3 << 7);
        break;
    }
    SOC_IF_ERROR_RETURN
        (WRITE_PHY5464_1000X_MII_ANAr(unit, pc, mii_adv));

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_adv_local_set
 * Purpose:
 *  Set the current advertisement for auto-negotiation.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  mode - Port mode mask indicating supported options/speeds.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The advertisement is set only for the ACTIVE medium.
 *  No synchronization performed at this level.
 */

STATIC int
robo_phy_5464_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    int            rv;   /* Return value */
    phy_ctrl_t    *pc;       /* PHY software state */
    soc_port_ability_t    ability;

    pc = EXT_PHY_SW_STATE(unit, port);

    rv = SOC_E_NONE;

    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_adv_local_set(unit, port, mode);
        if (SOC_SUCCESS(rv)) {
            pc->copper.autoneg_advert = mode & ADVERT_ALL_COPPER;
        }
    } else {    /* PHY_FIBER_MODE */
        if (PHY_PASSTHRU_MODE(unit, port)) { /* passthrough fiber mode */
            phy_ctrl_t    *int_pc;   /* PHY software state */

            int_pc = INT_PHY_SW_STATE(unit, port);

            if (NULL != int_pc) {
                /* In passthrough mode, call internal PHY driver */
                rv = PHY_ADV_SET(int_pc->pd, unit, port, mode);
                if (SOC_E_UNAVAIL == rv) {
                    rv = soc_port_mode_to_ability(mode, &ability);
                    if (SOC_SUCCESS(rv)) {
                        rv = (PHY_ABILITY_ADVERT_SET(int_pc->pd, unit,                                                 port, &ability));
                    }
                }
            } else {
                rv = SOC_E_INTERNAL;
            }
        } else { /* GMII Fiber mode */ 
                rv = _robo_phy_5464_fiber_adv_local_set(unit, port, mode);
        }

        if (SOC_SUCCESS(rv)) {
            pc->fiber.autoneg_advert = mode & ADVERT_ALL_FIBER;
        } 
    }

    DPRINTF((BSL_META_U(unit,
                        "phy_5464_adv_local_set: u=%d p=%d mode=0x%x rv=%d\n"),
             unit, port, mode, rv));

    return rv;

}

STATIC int
_robo_phy_5464_fiber_adv_local_get(int unit, soc_port_t port, 
                              soc_port_mode_t *mode)
{
    uint16              mii_adv;   /* MII advertise register of the PHY */ 
    phy_ctrl_t    *pc;       /* PHY software state */
        
    pc = EXT_PHY_SW_STATE(unit, port);

    /* read the 1000BASE-X Auto-Neg Advertisement Reg( Address 0x04 ) */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_ANAr(unit, pc, &mii_adv));

    *mode = 0;
    if (mii_adv & (1 << 5))         *mode |= SOC_PM_1000MB_FD;
    if (mii_adv & (1 << 6))         *mode |= SOC_PM_1000MB_HD;
    switch ((mii_adv >> 7) & 0x3) {
    case 0x1:
        *mode |= SOC_PM_PAUSE;
        break;
    case 0x2:
        *mode |= SOC_PM_PAUSE_TX;
        break;
    case 0x3:
        *mode |= SOC_PM_PAUSE_RX;
        break;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_adv_local_get
 * Purpose:
 *  Get the current advertisement for auto-negotiation.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The advertisement is retrieved for the ACTIVE medium.
 *  No synchronization performed at this level.
 */

STATIC int
robo_phy_5464_adv_local_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{

    int     rv;    /* Return value */

    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_adv_local_get(unit, port, mode);
    } else {    /* PHY_FIBER_MODE */
        if (PHY_PASSTHRU_MODE(unit, port)) { /* passthrough fiber mode */
            /* If the PHY is in passthrough mode, phyctrl calls the inner
             * PHY driver adv_local_get.
             */
            rv = SOC_E_INTERNAL;
        } else { /* GMII fiber mode */
            rv = _robo_phy_5464_fiber_adv_local_get(unit, port, mode);
        }
    }

    return rv;

}

STATIC int
_robo_phy_5464_fiber_adv_remote_get(int unit, soc_port_t port,
                               soc_port_mode_t *mode)
{
    uint16               mii_ctrl;  /* MII control register */ 
    uint16               mii_stat;  /* MII status register */
    uint16               mii_anp;   /* MII link partner ability reg */ 
    phy_ctrl_t    *pc;        /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    /* read the 1000BASE-X Control Reg ( Address 0x00 ) */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_CTRLr(unit, pc, &mii_ctrl));
    if (!(mii_ctrl & MII_CTRL_AE)) {
        return SOC_E_DISABLED;
    } 

    /* read the 1000Base-X Status Reg  ( Address 0x01 ) */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_STATr(unit, pc, &mii_stat));
    if (!(mii_stat & MII_STAT_AN_DONE)) {
        return SOC_E_NONE;
    }

    /* read the auto-Neg Link Partner Ability Reg ( Address 0x05 ) */
    SOC_IF_ERROR_RETURN
        (READ_PHY5464_1000X_MII_ANPr(unit, pc, &mii_anp));

    *mode = 0;
    if (mii_anp & (1 << 5))         *mode |= SOC_PM_1000MB_FD;
    if (mii_anp & (1 << 6))         *mode |= SOC_PM_1000MB_HD;
    switch ((mii_anp >> 7) & 0x3) {
    case 0x1:
        *mode |= SOC_PM_PAUSE;
        break;
    case 0x2:
        *mode |= SOC_PM_PAUSE_TX;
        break;
    case 0x3:
        *mode |= SOC_PM_PAUSE_RX;
        break;
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *  robo_phy_5464_adv_remote_get
 * Purpose:
 *  Get partners current advertisement for auto-negotiation.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The remote advertisement is retrieved for the ACTIVE medium.
 *  No synchronization performed at this level. If Autonegotiation is
 *  disabled or in progress, this routine will return an error.
 */

STATIC int
robo_phy_5464_adv_remote_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{

    int                  rv;

    if (NULL == mode) {
        return SOC_E_PARAM;
    }

    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_adv_remote_get(unit, port, mode);
    } else {    /* PHY_FIBER_MODE */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            /* If the PHY is in passthrough mode, the inner PHY driver
             * adv_remote_get is called by phyctrl. 
             */
            rv = SOC_E_INTERNAL;
        } else { /* GMII fiber mode */
            rv = _robo_phy_5464_fiber_adv_remote_get(unit, port, mode);
        }
    }
    return rv;
}

/*
 * Function:
 *  robo_phy_5464_lb_set
 * Purpose:
 *  Set the local PHY loopback mode.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  loopback - Boolean: true = enable loopback, false = disable.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The loopback mode is set only for the ACTIVE medium.
 *  No synchronization performed at this level.
 */

STATIC int
robo_phy_5464_lb_set(int unit, soc_port_t port, int enable)
{

    int            rv;   /* Return value */
    phy_ctrl_t    *pc;   /* PHY software state */
    int retry = 0;
    uint16 phy_data;
    
    rv = SOC_E_NONE;
    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
#if AUTO_MDIX_WHEN_AN_DIS
        {
        uint16          tmp;

        /* Disable Auto-MDIX When autoneg disabled */
        /* Enable Auto-MDIX When autoneg disabled */
        tmp = (enable) ? 0x0000: 0x0200;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_MISC_CTRLr(unit, pc, tmp, 0x0200));
        }
#endif
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
                    READ_PHY5464_MII_STATr(unit, pc, &phy_data));
                if (!(phy_data & MII_STAT_LA)) {
                    break;
                }
            }
        }

        /* this is for the workaround to fix the problem on reporting link
         * status.(GNATS 29609)
         */
        if (enable) {
            SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_TEST1r(unit, pc, 0x1000, 0x1000));
        } else {
            SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_TEST1r(unit, pc, 0x0000, 0x1000));
        }

    } else { /* Fiber mode */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            phy_ctrl_t    *int_pc;   /* PHY software state */

            int_pc = INT_PHY_SW_STATE(unit, port);

            if (NULL != int_pc) {
                rv = (PHY_LOOPBACK_SET(int_pc->pd, unit, port, enable));
            } else {
                rv = SOC_E_INTERNAL;
            }
        } else {  /* GMII fiber mode */ 
            uint16 lb;
            lb = (enable) ? MII_CTRL_LE : 0;
            rv = MODIFY_PHY5464_1000X_MII_CTRLr(unit, pc, lb, MII_CTRL_LE);
        }
    }

    DPRINTF((BSL_META_U(unit,
                        "phy_5464_lb_set: u=%d p=%d en=%d rv=%d\n"), 
             unit, port, enable, rv));
    return rv;

}

/*
 * Function:
 *  robo_phy_5464_lb_get
 * Purpose:
 *  Get the local PHY loopback mode.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  loopback - (OUT) Boolean: true = enable loopback, false = disable.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  The loopback mode is retrieved for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_lb_get(int unit, soc_port_t port, int *enable)
{
    int            rv;
    phy_ctrl_t    *pc;

    rv = SOC_E_NONE;
    pc = EXT_PHY_SW_STATE(unit, port);
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_fe_ge_lb_get(unit, port, enable);
    } else { /* fiber mode */
        if (PHY_PASSTHRU_MODE(unit, port)) {
            /* If the PHY is in passthrough mode, phyctrl calls inner PHY
             * driver loopback get().
             */
            rv = SOC_E_INTERNAL;
        } else { /* GMII fiber mode */
            uint16 mii_ctrl;
            rv = READ_PHY5464_1000X_MII_CTRLr(unit, pc, &mii_ctrl);
            *enable = (mii_ctrl & MII_CTRL_LE) ? 1 : 0;
        }
    }
    return rv;  
}

/*
 * Function:
 *  robo_phy_5464_interface_set
 * Purpose:
 *  Set the current operating mode of the PHY.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  pif - one of SOC_PORT_IF_*
 * Returns:
 *  SOC_E_NONE - success
 *  SOC_E_UNAVAIL - unsupported interface
 */

STATIC int
robo_phy_5464_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    switch (pif) {
    case SOC_PORT_IF_GMII:
    case SOC_PORT_IF_SGMII:
    case SOC_PORT_IF_RGMII: /* For Robo5324 GE ports */
    case SOC_PORT_IF_MII:   /* IMP port */
        return SOC_E_NONE;
    default:
        return SOC_E_UNAVAIL;
    }
}

/*
 * Function:
 *  robo_phy_5464_interface_get
 * Purpose:
 *  Get the current operating mode of the PHY.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *  SOC_E_NONE
 */

STATIC int
robo_phy_5464_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    phy_ctrl_t    *pc;
    
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    pc = EXT_PHY_SW_STATE(unit, port);

    /* Robo5396 with soc_feature_dodeca_serdes feature currently and will 
     * return SGMII interface 
     */
    if (soc_feature(unit, soc_feature_dodeca_serdes)) {
        *pif = SOC_PORT_IF_SGMII;
    } else {
        *pif = pc->interface;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_phy_5464_ability_get
 * Purpose:
 *  Get the abilities of the local gigabit PHY.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 *  mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *  SOC_E_NONE
 * Notes:
 *  The ability is retrieved only for the ACTIVE medium.
 */

STATIC int
robo_phy_5464_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    int         rv;
    phy_ctrl_t *pc;

    rv = SOC_E_NONE;
    if (PHY_COPPER_MODE(unit, port)) {
        rv = phy_ge_ability_get(unit, port, mode);
    } else { /* fiber mode */
        *mode = (SOC_PM_AN | SOC_PM_LB_PHY | 
                 SOC_PM_PAUSE | SOC_PM_PAUSE_ASYMM |
                 SOC_PM_GMII | SOC_PM_SGMII | SOC_PM_1000MB_FD);
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    if (pc->automedium) {
        *mode |= SOC_PM_COMBO;
    }

    return rv;
}

/*
 * Function:
 *  robo_phy_5464_mdix_set
 * Description:
 *  Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - One of:
 *          SOC_PORT_MDIX_AUTO
 *          Enable auto-MDIX when autonegotiation is enabled
 *          SOC_PORT_MDIX_FORCE_AUTO
 *          Enable auto-MDIX always
 *      SOC_PORT_MDIX_NORMAL
 *          Disable auto-MDIX
 *      SOC_PORT_MDIX_XOVER
 *          Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *  SOC_E_XXX
 */
STATIC int
robo_phy_5464_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{

    phy_ctrl_t    *pc;
    int            speed;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_COPPER_MODE(unit, port)) {
        switch (mode) {
        case SOC_PORT_MDIX_AUTO:
            /* Clear bit 14 for automatic MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_ECRr(unit, pc, 0, 0x4000));

            /*
             * Write the result in the register 0x18, shadow copy 7
             */
            /* Clear bit 9 to disable forced auto MDI xover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_MISC_CTRLr(unit, pc, 0, 0x0200));
            break;

        case SOC_PORT_MDIX_FORCE_AUTO:
            /* Clear bit 14 for automatic MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_ECRr(unit, pc, 0, 0x4000));

            /*
             * Write the result in the register 0x18, shadow copy 7
             */
            /* Set bit 9 to force automatic MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY5464_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200));
            break;

        case SOC_PORT_MDIX_NORMAL:
            SOC_IF_ERROR_RETURN(robo_phy_5464_speed_get(unit, port, &speed));
            if (speed == 0 || speed == 10 || speed == 100) {
                /* Set bit 14 for manual MDI crossover */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_MII_ECRr(unit, pc, 0x4000, 0x4000));

                SOC_IF_ERROR_RETURN
                    (WRITE_PHY5464_TEST1r(unit, pc, 0));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        case SOC_PORT_MDIX_XOVER:
            SOC_IF_ERROR_RETURN(robo_phy_5464_speed_get(unit, port, &speed));
            if (speed == 0 || speed == 10 || speed == 100) {
                 /* Set bit 14 for manual MDI crossover */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY5464_MII_ECRr(unit, pc, 0x4000, 0x4000));

                SOC_IF_ERROR_RETURN
                    (WRITE_PHY5464_TEST1r(unit, pc, 0x0080));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        default:
            return SOC_E_PARAM;
        }

        pc->copper.mdix = mode;
        return SOC_E_NONE;
    } else {    /* fiber */
        if (mode != SOC_PORT_MDIX_NORMAL) {
            return SOC_E_UNAVAIL;
        }
        return SOC_E_NONE;
    }

}

/*
 * Function:
 *  robo_phy_5464_mdix_get
 * Description:
 *  Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  mode - (Out) One of:
 *          SOC_PORT_MDIX_AUTO
 *          Enable auto-MDIX when autonegotiation is enabled
 *          SOC_PORT_MDIX_FORCE_AUTO
 *          Enable auto-MDIX always
 *      SOC_PORT_MDIX_NORMAL
 *          Disable auto-MDIX
 *      SOC_PORT_MDIX_XOVER
 *          Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *  SOC_E_XXX
 */
STATIC int
robo_phy_5464_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mode)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);
    int                  speed;

    if (mode == NULL || pc == NULL) {
        return SOC_E_PARAM;
    }

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN(robo_phy_5464_speed_get(unit, port, &speed));
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
 *  robo_phy_5464_mdix_status_get
 * Description:
 *  Get the current MDIX status on a port/PHY
 * Parameters:
 *  unit    - Device number
 *  port    - Port number
 *  status  - (OUT) One of:
 *          SOC_PORT_MDIX_STATUS_NORMAL
 *          Straight connection
 *          SOC_PORT_MDIX_STATUS_XOVER
 *          Crossover has been performed
 * Return Value:
 *  SOC_E_XXX
 */
STATIC int
robo_phy_5464_mdix_status_get(int unit, soc_port_t port, 
                         soc_port_mdix_status_t *status)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);
    uint16               tmp;

    if (status == NULL || pc == NULL) {
        return SOC_E_PARAM;
    }

    if (PHY_COPPER_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_MII_ESRr(unit, pc, &tmp));
        if (tmp & 0x2000) {
            *status = SOC_PORT_MDIX_STATUS_XOVER;
        } else {
            *status = SOC_PORT_MDIX_STATUS_NORMAL;
        }
    } else {
        *status = SOC_PORT_MDIX_STATUS_NORMAL;
    }

    return SOC_E_NONE;
}    

/* saved state when running cable diagnostics */
typedef struct {
    uint16  reg_1c;
    uint16  reg_00;
    uint16  reg_18_0;
    uint16  reg_18_7;
    uint16  reg_17_0;
    uint16  reg_17_8100;
} cdiag_setup_t;

/*
 * Function:
 *  robo_phy_5464_cable_diag_setup
 * Purpose:
 *  Put phy into a state that it can run cable diagnostics.
 *  Various internal state machines must be stopped.
 */
STATIC int
robo_phy_5464_cable_diag_setup(int unit, uint8 phy_addr, cdiag_setup_t *setup)
{
    phy_ctrl_t  *pc;
    uint16       tmp;
    soc_port_t   port;

    port = soc_phy_addr_to_port(unit, phy_addr); 
    pc   = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x1c, 0x7c00));
    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x1c, &tmp));
    setup->reg_1c = tmp;
    tmp &= ~0x0001;     /* force copper regs */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x1c, tmp | 0x8000));

    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x00, &tmp));
    setup->reg_00 = tmp;
    tmp = 0x0040;       /* force 1G, no autoneg */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x00, tmp));

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18, 0x0007));
    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x18, &tmp));
    setup->reg_18_0 = tmp;
    tmp |= 0x0800;      /* enable sm_dsp clock */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18, tmp));

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18, 0x7007));
    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x18, &tmp));
    setup->reg_18_7 = tmp;
    tmp &= ~0x200;      /* disable auto-mdix */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18, tmp | 0x8000));

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x0000));
    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x15, &tmp));
    setup->reg_17_0 = tmp;
    tmp = 0xf8b5;       /* disable agc */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, tmp));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x2000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, tmp));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x4000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, tmp));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x6000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, tmp));

    /* freeze internal state machines */
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x8100));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x16, 0x0002));
    SOC_IF_ERROR_RETURN(READ_PHY_REG(unit, pc, 0x15, &tmp));
    setup->reg_17_8100 = tmp;
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, 0x0030));

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x8200));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x16, 0x0002));

    return SOC_E_NONE;
}

#ifdef robo_phy_5464_cable_diag_UNSETUP
/*
 * Function:
 *  robo_phy_5464_cable_diag_unsetup
 * Purpose:
 *  Restore phy into a working state after cable diagnostics
 *  have been run.
 */
STATIC int
robo_phy_5464_cable_diag_unsetup(int unit, uint8 phy_addr, cdiag_setup_t *setup)
{
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18, setup->reg_18_0));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x18,
                                       setup->reg_18_7 | 0x8000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x0000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, setup->reg_17_0));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x2000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, setup->reg_17_0));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x4000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, setup->reg_17_0));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x6000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15, setup->reg_17_0));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x8100));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x16, 0x0000));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x15,
                                       setup->reg_17_8100));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x17, 0x8200));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x16, 0x0000));

    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x00, setup->reg_00));
    SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit, pc, 0x1c,
                                       setup->reg_1c | 0x8000));
    return SOC_E_NONE;
}
#endif /* robo_phy_5464_cable_diag_UNSETUP */

/* cable diag pass info */
typedef struct {
    int     skip_14_19;
    uint16  set_14;     /* expansion regs to set */
    uint16  set_15;
    uint16  set_16;
    uint16  set_17;
    uint16  set_18;
    uint16  set_19;
    uint16  set_1a;
    uint16  set_1b;
    uint16  set_1c;
    uint16  set_1d;
    uint16  set_10;

    uint16      get_10;         /* expansion reg results */
    uint16      get_11;        
    uint16  get_12;
    uint16  get_13;
} cdiag_pass_t;

/*
 * Function:
 *  robo_phy_5464_cable_diag_pass
 * Purpose:
 *  Run a cable diagnostic pass
 */
STATIC int
robo_phy_5464_cable_diag_pass(int unit, uint8 phy_addr, cdiag_pass_t *pass)
{
    phy_ctrl_t *pc;
    uint16      tmp;
    int         loop, maxloop;
    soc_port_t  port;

    port = soc_phy_addr_to_port(unit, phy_addr); 
    pc = EXT_PHY_SW_STATE(unit, port);

    maxloop = 50;       /* usually takes about 5 */

    if (pass->skip_14_19 == 0) {
        WRITE_EXP(0x14, pass->set_14);
        WRITE_EXP(0x15, pass->set_15);
        WRITE_EXP(0x16, pass->set_16);
        WRITE_EXP(0x17, pass->set_17);
        if (PHY_IS_BCM5478(pc) || 
            PHY_IS_BCM5488(pc)) {
            WRITE_EXP(0x1a, pass->set_18);
            WRITE_EXP(0x1b, pass->set_19);
        } else {
            WRITE_EXP(0x18, pass->set_18);
            WRITE_EXP(0x19, pass->set_19);
        }
    }
    if (PHY_IS_BCM5478(pc) || 
        PHY_IS_BCM5488(pc)) {
        WRITE_EXP(0x1d, pass->set_1a);
        WRITE_EXP(0x1e, pass->set_1b);
        WRITE_EXP(0x1f, pass->set_1c);
        WRITE_EXP(0x10, pass->set_10 & 0x00ff);
        WRITE_EXP(0x28, pass->set_10 & 0xff00);
    } else {
        WRITE_EXP(0x1a, pass->set_1a);
        WRITE_EXP(0x1b, pass->set_1b);
        WRITE_EXP(0x1c, pass->set_1c);
        WRITE_EXP(0x1d, pass->set_1d);
        WRITE_EXP(0x10, pass->set_10);
    }

    for (loop = 0; loop < maxloop; loop++) {
        sal_usleep(1000);
        if ((pc->phy_model == PHY_BCM5478_MODEL) || 
            (pc->phy_model == PHY_BCM5488_MODEL)) {
            READ_EXP(0x10, &tmp);
            if (tmp & 0x0200) {
                pass->get_10 = tmp;
                READ_EXP(0x11, &tmp);
                break;
            }
        } else {
            READ_EXP(0x11, &tmp);
            if (tmp & 0x0001) {
                break;
            }
        }
    }
    if (loop >= maxloop) {
        return SOC_E_TIMEOUT;
    }

    pass->get_11 = tmp;
    READ_EXP(0x12, &pass->get_12);
    READ_EXP(0x13, &pass->get_13);
    return SOC_E_NONE;
}

#undef  WRITE_EXP
#undef  READ_EXP

/*
 * Function:
 *  robo_phy_5464_cable_diag
 * Purpose:
 *  Run 546x cable diagnostics
 * Parameters:
 *  unit - device number
 *  port - port number
 *  status - (OUT) cable diagnotic status structure
 * Returns: 
 *  SOC_E_XXX
 */
extern int phy_5464_cable_diag_sw(int, soc_port_t, soc_port_cable_diag_t *);
STATIC int
robo_phy_5464_cable_diag(int unit, soc_port_t port,
            soc_port_cable_diag_t *status)
{
    phy_ctrl_t    *pc;
    uint8          phy_addr;
    int            rv, rv2, i;
    int   bcm53314_speed = 0, bcm53314_an = 0, bcm53314_an_done = 0, bcm53314_link = 0;  /* BCM53314 WAR Fix */

    pc       = EXT_PHY_SW_STATE(unit, port);
    phy_addr = pc->phy_id;

    if (status == NULL) {
        return SOC_E_PARAM;
    }

    if (!PHY_IS_BCM5464(pc) && 
        !PHY_IS_BCM5461(pc) &&
        !PHY_IS_BCM5482(pc) &&
        !PHY_IS_BCM5488(pc) &&
        !PHY_IS_BCM54980(pc) &&
        !PHY_IS_BCM54980C(pc) &&
        !PHY_IS_BCM54980V(pc) &&
        !PHY_IS_BCM54980VC(pc) &&
        !PHY_IS_BCM5398(pc) &&
        !PHY_IS_BCM5395(pc) &&
        !PHY_IS_BCM53115(pc) &&
        !PHY_IS_BCM53118(pc)) {
        return SOC_E_UNAVAIL;
    }

    status->state = SOC_PORT_CABLE_STATE_OK;
    status->npairs = 4;
    status->fuzz_len = 0;
    for (i = 0; i < 4; i++) {
        status->pair_state[i] = SOC_PORT_CABLE_STATE_OK;
    }

    if (soc_property_get(unit, spn_CABLE_DIAG_HW, robo_phy_5464_cable_diag_HW)) {
        uint16              tmp, result;
        cdiag_setup_t       setup_state;
        cdiag_pass_t        pass;

        if (!PHY_IS_BCM5464(pc) && !PHY_IS_BCM5461(pc)) {
            return SOC_E_UNAVAIL;
        }
 
        SOC_IF_ERROR_RETURN
            (READ_PHY_REG(unit, pc, 0x11, &tmp));

        if (tmp & 0x0100) {         /* link up? */
            /* is linked at 10 or 100Mbit, diags will fail */
            SOC_IF_ERROR_RETURN
                (READ_PHY_REG(unit, pc, 0x19, &tmp));
            if ((tmp & 0x0600) != 0x0600) {
                status->npairs = 2;
                status->pair_len[0] = -1;
                status->pair_len[1] = -1;
                return SOC_E_NONE;
            }

            /* run length checks for defect free cable */
            pass.skip_14_19 = 1;
            pass.set_14 = 0;
            pass.set_15 = 0;
            pass.set_16 = 0;
            pass.set_17 = 0;
            pass.set_18 = 0;
            pass.set_19 = 0;
            pass.set_1a = 0x012c;
            pass.set_1b = 0x00c8;
            pass.set_1c = 0x00a0;
            pass.set_1d = 0x6096;
            pass.set_10 = 0x0207;
            rv = robo_phy_5464_cable_diag_pass(unit, phy_addr, &pass);
            if (rv >= 0) {
                status->pair_len[0] = pass.get_12 & 0xff;
                status->pair_len[1] = (pass.get_12 >> 8) & 0xff;
                status->pair_len[2] = pass.get_13 & 0xff;
                status->pair_len[3] = (pass.get_13 >> 8) & 0xff;
                if (pc->phy_model == PHY_BCM5464_MODEL &&
                    pc->phy_rev == PHY_REV_A0) {
                    /* A0 has a tendency to give pairs B, D a length of 152 */
                    if (status->pair_len[0] < 128 && status->pair_len[1] > 140) {
                        status->pair_len[1] = status->pair_len[0];
                    }
                    if (status->pair_len[1] < 128 && status->pair_len[0] > 140) {
                        status->pair_len[0] = status->pair_len[1];
                    }
                    if (status->pair_len[2] < 128 && status->pair_len[3] > 140) {
                        status->pair_len[3] = status->pair_len[2];
                    }
                    if (status->pair_len[3] < 128 && status->pair_len[2] > 140) {
                        status->pair_len[2] = status->pair_len[3];
                    }
                }
            }
            return rv;
        }

        /* link not up, so run defect checks */
    
        /* setup phy for cable diags: disable various automated things */
        MIIM_LOCK(unit);    /* this locks out linkscan, essentially */
        rv = robo_phy_5464_cable_diag_setup(unit, phy_addr, &setup_state);
        if (rv < 0) {
            MIIM_UNLOCK(unit);
            return rv;
        }

        /*
         * perhaps some of these per pass magic numbers should come
         * from properties, but that's a lot of properties...
         */
        

        /* pass 1 */
        pass.skip_14_19 = 0;
        pass.set_14 = 0x5014;       /* 0x4650 */
        pass.set_15 = 0x0ed8;       /* 0x0bb8 */
        pass.set_16 = 0x0af0;       /* 0x09c4 */
        pass.set_17 = 0x047e;       /* 0x03e8 */
        pass.set_18 = 0x0a14;
        pass.set_19 = 0x3c00;
        pass.set_1a = 0x0640;
        pass.set_1b = 0x00fa;
        pass.set_1c = 0x012c;
        pass.set_1d = 0x0a60;
        pass.set_10 = 0x000b;
        rv = robo_phy_5464_cable_diag_pass(unit, phy_addr, &pass);

        if ((pc->phy_model == PHY_BCM5478_MODEL) || 
            (pc->phy_model == PHY_BCM5488_MODEL)) {
            result = pass.get_10 & 0x0400;
        } else {
            result = pass.get_11 & 0x0002;
        }
        if (rv >= 0 && result == 0) {
            /* pass 2 (no errors found in pass 1) */
            pass.set_14 = 0xffff;
            pass.set_15 = 0x04b0;
            pass.set_16 = 0x0320;
            pass.set_17 = 0x0320;
            pass.set_18 = 0x608c;
            pass.set_19 = 0xa000;
            pass.set_1a = 0x03e8;
            pass.set_1b = 0x0258;
            pass.set_1c = 0x00b4;
            pass.set_1d = 0x0a14;
            pass.set_10 = 0x000b;
            rv = robo_phy_5464_cable_diag_pass(unit, phy_addr, &pass);
        }

        if ((pc->phy_model == PHY_BCM5478_MODEL) || 
            (pc->phy_model == PHY_BCM5488_MODEL)) {
            result = pass.get_10 & 0x0400;
        } else {
            result = pass.get_11 & 0x0002;
        }
        if (rv >= 0 && result == 0) {
            /* pass 3 (no errors found in pass 1 or 2) */
            pass.set_14 = 0xffff;
            pass.set_15 = 0xffff;
            pass.set_16 = 0xffff;
            pass.set_17 = 0xffff;
            pass.set_18 = 0x608c;
            pass.set_19 = 0xa000;
            pass.set_1a = 0x03e8;
            pass.set_1b = 0x0258;
            pass.set_1c = 0x00b4;
            pass.set_1d = 0x0a14;
            pass.set_10 = 0x000b;
            rv = robo_phy_5464_cable_diag_pass(unit, phy_addr, &pass);
        }

        if (rv >= 0) {
            if ((pc->phy_model == PHY_BCM5478_MODEL) || 
                (pc->phy_model == PHY_BCM5488_MODEL)) {
                if (pass.get_10 & 0x0400) {
                    status->state = SOC_PORT_CABLE_STATE_OPENSHORT;
                }
                if (pass.get_11 & 0x0003) {
                    status->pair_state[0] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x1000) {
                    status->pair_state[0] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x000c) {
                    status->pair_state[1] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x2000) {
                    status->pair_state[1] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x0030) {
                    status->pair_state[2] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x4000) {
                    status->pair_state[2] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x00c0) {
                    status->pair_state[3] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x8000) {
                    status->pair_state[3] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
            } else {
                if (pass.get_11 & 0x0002) {
                    status->state = SOC_PORT_CABLE_STATE_OPENSHORT;
                }
                if (pass.get_11 & 0x0100) {
                    status->pair_state[0] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x1000) {
                    status->pair_state[0] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x0200) {
                    status->pair_state[1] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x2000) {
                    status->pair_state[1] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x0400) {
                    status->pair_state[2] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x4000) {
                    status->pair_state[2] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
                if (pass.get_11 & 0x0800) {
                    status->pair_state[3] = SOC_PORT_CABLE_STATE_OPENSHORT;
                } else if (pass.get_11 & 0x8000) {
                    status->pair_state[3] = SOC_PORT_CABLE_STATE_UNKNOWN;
                }
            }
            status->pair_len[0] = pass.get_12 & 0xff;
            status->pair_len[1] = (pass.get_12 >> 8) & 0xff;
            status->pair_len[2] = pass.get_13 & 0xff;
            status->pair_len[3] = (pass.get_13 >> 8) & 0xff;
        }

        /* restore various control values */
#ifdef robo_phy_5464_cable_diag_UNSETUP
        rv2 = robo_phy_5464_cable_diag_unsetup(unit, phy_addr, &setup_state);
#endif
        MIIM_UNLOCK(unit);
        rv2 = _robo_phy_5464_reset_setup(unit, port);
    
        if (rv >= 0 && rv2 < 0) {
            return rv2;
        }
        return rv;
    }

/* BCM53314 WAR Fix */
    if (PHY_IS_BCM53314(pc)) {
       SOC_IF_ERROR_RETURN(robo_phy_5464_link_get(unit, port, &bcm53314_link));
       SOC_IF_ERROR_RETURN(robo_phy_5464_autoneg_get(unit, port, &bcm53314_an, &bcm53314_an_done));
       SOC_IF_ERROR_RETURN(robo_phy_5464_speed_get(unit, port, &bcm53314_speed));
    }

    MIIM_LOCK(unit);    /* this locks out linkscan, essentially */
    rv = phy_5464_cable_diag_sw(unit, port, status);
    MIIM_UNLOCK(unit);
    rv2 = 0;
    if (rv <= 0) {      /* don't reset if > 0 -- link was up */
        rv2 = _robo_phy_5464_reset_setup(unit, port);

        /* GNATS 40594 : 
         *  - PHY configuration been unexpected changed after performed cable
         *      diag on the GE port if this port is linked at 10/100 speed.
         *
         *  Note : 
         *  1. if the PHY linked at 10/100 due to Force speed or local AN  
         *      ability were changed to speed up to 10/100, original PHY  
         *      configuration will be lost due to PHY reset above.
         */
        SOC_IF_ERROR_RETURN(_phy_5464_medium_config_update(unit, 
                port,PHY_COPPER_MODE(unit, port) ?
                &pc->copper :
                &pc->fiber));
    }

/* BCM53314 WAR Fix */
    if (bcm53314_an == TRUE && bcm53314_link == TRUE) {
       if (bcm53314_speed == 100 || bcm53314_speed == 10) {
          SOC_IF_ERROR_RETURN(robo_phy_5464_autoneg_set(unit, port, TRUE));
       }
    }

    if (rv >= 0 && rv2 < 0) {
        return rv2;
    }
    return rv;

 }

STATIC int _robo_phy_5464_power_mode_set (int unit, soc_port_t port, int mode)
{
    phy_ctrl_t    *pc;

    pc       = EXT_PHY_SW_STATE(unit, port);

    if (pc->power_mode == mode) {
        return SOC_E_NONE;
    }

    if (mode == SOC_PHY_CONTROL_POWER_LOW) {
        /* enable dsp clock */
        SOC_IF_ERROR_RETURN             
            (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0c00,0x0c00));
         /* enable low power 136 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,0x80,0x80));

        /* reduce tx bias current to -20% */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x17, 0x0f75));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x15, 0x1555));

        /* disable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0400,0x0c00));
        pc->power_mode = mode;

    } else if (mode == SOC_PHY_CONTROL_POWER_FULL) {

        /* back to normal mode */
        /* enable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0c00,0x0c00));

        /* disable low power 136 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,0x00,0x80));

        /* reduce tx bias current to -20% */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x17, 0x0f75));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x15, 0x0));

        /* disable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_MII_AUX_CTRLr(unit,pc,0x0400,0x0c00));

        /* disable the auto power mode */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,
                        0,
                        PHY_5464_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    } else if (mode == SOC_PHY_CONTROL_POWER_AUTO) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,
                        PHY_5464_AUTO_PWRDWN_EN,
                        PHY_5464_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      robo_phy_5464_control_set
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
robo_phy_5464_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    int rv;
    uint16 data16;
    phy_ctrl_t *pc;

    /* coverity[mixed_enums] */
    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;
    switch(type) {
    case SOC_PHY_CONTROL_POWER:
        rv = _robo_phy_5464_power_mode_set(unit,port,value);
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        if (value <= PHY_5464_AUTO_PWRDWN_WAKEUP_MAX) {

            /* at least one unit */
            if (value < PHY_5464_AUTO_PWRDWN_WAKEUP_UNIT) {
                value = PHY_5464_AUTO_PWRDWN_WAKEUP_UNIT;
            }
        } else { /* anything more then max, set to the max */
            value = PHY_5464_AUTO_PWRDWN_WAKEUP_MAX;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,
                      value/PHY_5464_AUTO_PWRDWN_WAKEUP_UNIT,
                      PHY_5464_AUTO_PWRDWN_WAKEUP_MASK));
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        /* sleep time configuration is either 2.7s or 5.4 s, default is 2.7s */
        if (value < PHY_5464_AUTO_PWRDWN_SLEEP_MAX) {
            data16 = 0; /* anything less than 5.4s, default to 2.7s */
        } else {
            data16 = PHY_5464_AUTO_PWRDWN_SLEEP_MASK;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY5464_AUTO_POWER_DOWNr(unit,pc,
                      data16,
                      PHY_5464_AUTO_PWRDWN_SLEEP_MASK));
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 * Function:
 *      robo_phy_5464_control_get
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
robo_phy_5464_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
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
    case SOC_PHY_CONTROL_POWER:
        *value = pc->power_mode;
        break;
    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_AUTO_POWER_DOWNr(unit,pc, &data16));

        if (data16 & PHY_5464_AUTO_PWRDWN_SLEEP_MASK) {
            *value = PHY_5464_AUTO_PWRDWN_SLEEP_MAX;
        } else {
            *value = 2700;
        }
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY5464_AUTO_POWER_DOWNr(unit,pc, &data16));

        data16 &= PHY_5464_AUTO_PWRDWN_WAKEUP_MASK;
        *value = data16 * PHY_5464_AUTO_PWRDWN_WAKEUP_UNIT;
        break;

    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        /* Not supported for 10/100/1000BASE-T interfaces */
        rv = SOC_E_UNAVAIL;
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*  robo_phy_5464_link_change
 * Purpose:
 *  Run 546x link change
 * Parameters:
 *  unit - device number
 *  port - port number
 *  link_change - (OUT) linkchange or not.
 * Returns: 
 *  SOC_E_XXX
 */
STATIC int
robo_phy_5464_link_change(int unit, soc_port_t port,
            int *link_change)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);
    int         rv = SOC_E_NONE;
    uint16 tmp;

    /* Read Mode Register (0x1c shadow 11100) */
    SOC_IF_ERROR_RETURN(
        READ_PHY5464_AUX_1000X_STATr(unit, pc, &tmp));

    if (tmp & 0x0200) {
        *link_change = 1;
    }
    else {
        *link_change = 0;
    }
    
    return rv;
}

#if LINK_DOWN_RESET_WAR

/*
 * Function:
 *  robo_phy_5464_link_down
 * Purpose:
 *  Perform reset work-around on link down (A0 silicon only)
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
robo_phy_5464_link_down(int unit, soc_port_t port)
{
    phy_ctrl_t    *pc = EXT_PHY_SW_STATE(unit, port);

    /* Only applies to copper link down on rev A0 */

    if (PHY_IS_BCM5464(pc) && pc->phy_rev == PHY_REV_A0 &&
        PHY_COPPER_MODE(unit, port)) {

        DPRINTF((BSL_META_U(unit,
                            "5464_link_down: port=%d activated\n"),
                 port));
                    
        /* Reset the chip, then restore user settings. */

        SOC_IF_ERROR_RETURN
            (_robo_phy_5464_reset_setup(unit, port));

        SOC_IF_ERROR_RETURN
            (_phy_5464_medium_config_update(unit, port,
                                            PHY_COPPER_MODE(unit, port) ?
                                            &pc->copper :
                                            &pc->fiber));
    }

    return SOC_E_NONE;
}

#else
#define robo_phy_5464_link_down NULL
#endif /* LINK_DOWN_RESET_WAR */

#if PHY53115_IOP_WORKAROUND
/* phy53115_a0/a1/b1 specific errata  */
void _phy53115_a0_iop_errata(int unit, soc_port_t port)
{
    phy_ctrl_t  *pc;
    int regrw_rv = SOC_E_NONE;
    
    pc = EXT_PHY_SW_STATE(unit, port);

    if (!((PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_A0_REV_ID)) ||
         (PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_A1_REV_ID)) ||
         (PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_B1_REV_ID)))){
        return;
    }

    /* port basis IOP war : 
     *  . Enable SM_DSP
     *  . Set Low-Power 10BT
     *  . Increase HPF corner
     *  . Disable LPF
     *  . set VDACctrl
     *  . incress driver
     */
    regrw_rv = WRITE_PHY_REG(unit, pc, 0x18, 0x0c00);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x0f08);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x0201);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x001f);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x0300);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x601f);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x0002);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x0f75);
    regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x003c);
  
    /* device basis IOP war */
    /* phy0-3 are in the Quad phy seciton and phy4 is in the single phy 
     *  section. These two section is init individually.
     *  - Increase 10BT amplitude
     *  - Improve 10BT return loss 
     *  - Improve 1000T template 
     */
    if (port == 2){
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x1C, 0xac4e);
    } else if (port == 3) {
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x1C, 0xb012);
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x1C, 0xac60);
    } else if (port ==4) {
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x0f96);
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x0010);
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x0f97);
        regrw_rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x0c0c);
    }
    
    if (regrw_rv) {
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_iop_errata,%d,"
                            "port%d, failed on writing to PHY reg!!\n"), 
                 __LINE__, port));
    } else {
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_iop_errata,%d,port%d, "
                            "IOP-WAR-#1 Done!!\n"), 
                 __LINE__, port));
    }
}

/* phy53115_b0 specific errata 
 */
void _phy53115_b0_iop_errata(int unit, soc_port_t port, int link)
{
    uint16	    mii_ctrl, temp;
    int         an;
    phy_ctrl_t  *pc;
    int regrw_rv = SOC_E_NONE;
    
    pc = EXT_PHY_SW_STATE(unit, port);

    if (! phy53115_b0_iop_war){ /* config set to stop this workaround */
        return;
    }

    if (!((PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_B0_REV_ID)) ||
         (PHY_REVISION_CHECK(pc, PHY_BCM53118_MODEL, 
                    PHY_BCM53118_OUI, BCM53118_A0_REV_ID)))){
        return;
    }
    
    regrw_rv = READ_PHYFEGE_MII_CTRLr(unit, pc, &mii_ctrl);
    if (regrw_rv){
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_b0_iop_errata,%d,port%d failed "
                            "on PHY Reg read!\n"), 
                 __LINE__, port));
    }
    
    /* 1. check AN */
    an = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
    if (!an){
        return;
    }
    
    if (link){  /* 2.1 link-up procedure */
        
        /* remove this workaround after link-up(Lito/phy-team suggest) */
        if (PHY53115_ERRATA_IOP_GET(port)){
            regrw_rv = WRITE_PHY_REG(unit, pc, 0x18, 0x0004);
            regrw_rv |= WRITE_PHY_REG(unit, pc, 0x18, 0x8077);
            
            if (regrw_rv){
                DPRINTF((BSL_META_U(unit,
                                    "_phy53115_b0_iop_errata,%d,"
                                    "port%d failed on writing to PHY Reg!!\n"),
                         __LINE__, port));
            }
            
            /* SW-info update */
            PHY53115_ERRATA_IOP_CLEAR(port);
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_b0_iop_errata,%d,"
                                "port%d link-up! IOP-WAR-#2 removed!!\n"),
                     __LINE__, port));
        }
        phy53115_errata_iop_cnt[port] = 0;
        
    } else {    /* 2.1 link-down procedure */
        uint16  temp2 = 0;
        
        phy53115_errata_iop_cnt[port] ++;
        if (phy53115_errata_iop_cnt[port] >= 
                    PHY53115_ERRATA_IOP_RESET_CNT){
            /* switch halfout-amplitude and wire-speed setting. 
             *  - miireg 18h shadow 4 for halfout-amplitude setting
             *  - miireg 18h shadow 7 for wire-speed setting
             */
            if (PHY53115_ERRATA_IOP_GET(port)){
                temp = 0x0004;  /* halfout-amplitude disabled */
                temp2 = 0x8077; /* wire-speed enabled */
            
                /* SW-info update */
                PHY53115_ERRATA_IOP_CLEAR(port);
            } else {
                /* halfout-amplitude enabled and wire-speed disabled */
                temp = 0x040C;  /* halfout-amplitude enabled */
                temp2 = 0x8067; /* wire-speed enabled */
            
                /* SW-info update */
                PHY53115_ERRATA_IOP_SET(port);
            }
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_b0_iop_errata,%d,"
                                "port%d IOP-WAR-#2,HALFOUT change to "
                                "0x%04x!!\n"),
                     __LINE__, port, temp));

            regrw_rv = WRITE_PHY_REG(unit, pc, 0x18, temp);
            regrw_rv |= WRITE_PHY_REG(unit, pc, 0x18, temp2);

            if (regrw_rv){
                DPRINTF((BSL_META_U(unit,
                                    "_phy53115_b0_iop_errata,"
                                    "%d,port%d failed on writing to "
                                    "PHY Reg!!\n"),
                         __LINE__, port));
            }

            phy53115_errata_iop_cnt[port] = 0;
        }
    }
}
#endif  /* PHY53115_IOP_WORKAROUND */

#if PHY53115_A0_AN100TX_WORKAROUND
/* phy53115_a0 specific errata 
 *  
 *  Errata 1. :link problem on 100TX+AN mode.
 */
void _phy53115_a0_errata(int unit, soc_port_t port, int link)
{
    uint16	    mii_ctrl, reg_value, temp;
    int         an;
    phy_ctrl_t  *pc;
    int         rv;
    
    pc = EXT_PHY_SW_STATE(unit, port);

    if (! phy53115_an100tx_war){ /* config set to stop this workaround */
        return;
    }
    
    if (!(PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_A0_REV_ID))){
        return;
    }

    /* ------- Errata 1 -------- */
    /* 0. check workaround status */
    if (PHY53115_ERRATA_AN100TX_STAT_GET(port)){
        return;
    }
    
    /* 1. Detect AN setting. (this workaround is for AN+100TX only */
    rv = READ_PHYFEGE_MII_CTRLr(unit, pc, &mii_ctrl);
    if (rv){
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_errata,%d,port%d failed on "
                            "PHY Reg read!\n"),
                 __LINE__, port));
    }
    
    an = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
    if (!an){
        return;
    }
    
    /* 2. check the Errata condition.
     *  1). Link up condition : check CRC or Carrier error
     *      >> reg 11h, bit7/bit5/bit3/bit0 = 1
     *  2). Link down condition : check AN stat and 100TX stat
     *      >> reg 19h, bit15=0, bit14=1, bit<10:8>="101" or "011"
     */
    
    /* get the speed information in auxi status register first */
    rv = READ_PHYFEGE_MII_ASSRr(unit, pc, &reg_value);
    if (rv){
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_errata,%d,port%d failed on "
                            "PHY Reg read!\n"),
                 __LINE__, port));
    }
    temp = (reg_value & 0x0700) >> 8;
    
    if (link){      /* for link up condition */
        
        /* early return if not at 100 speed */
        if (!((temp == 0x5) || (temp == 0x3))){
            PHY53115_ERRATA_AN100TX_LINKERR_CLEAR(port);
            return;
        }
        
        if (! PHY53115_ERRATA_AN100TX_LINKERR_GET(port)){

            DPRINTF((BSL_META_U(unit,
                                "_phy53115_a0_errata,"
                                "%d,checking WAR link-up condition!!\n"),
                     __LINE__));
            /* early return if link-up but error detected */
            rv = READ_PHY_REG(unit, pc, 0x11, &reg_value);
            if (rv){
                DPRINTF((BSL_META_U(unit,
                                    "_phy53115_a0_errata,%d,"
                                    "port%d failed on PHY Reg read!\n"),
                         __LINE__, port));
            }
            PHY53115_ERRATA_AN100TX_LINKERR_SET(port);
            if (!(reg_value & 0xa9)){  /* check bit7/5/3/0 */
                return;
            }
        } else {
            return;
        }
        
    } else {        /* for link down condition */
        PHY53115_ERRATA_AN100TX_LINKERR_CLEAR(port);
        if ((reg_value & 0x8000) || (!(reg_value & 0x4000))){
            return;
        } else {
            if (!((temp == 0x5) || (temp == 0x3))){
                return;
            }
        }
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_errata,%d,"
                            "ge%d,checked WAR link-down condition!!\n"),
                 __LINE__, port));
    }
    
    DPRINTF((BSL_META_U(unit,
                        "_phy53115_a0_errata,%d,"
                        "phy53115_a0 an100tx sw-info stat=0x%08x, "
                        "link=0x%08x!!\n"),
             __LINE__, phy53115_errata_an100tx_status, 
             phy53115_errata_an100tx_linkup));
    DPRINTF((BSL_META_U(unit,
                        "_phy53115_a0_errata,%d,Set ge%d workaround for "
                        "phy53115_a0!!\n"),
             __LINE__, port));
    /* enable the force AUTO-MDIX for this workaround */
    rv = MODIFY_PHY5464_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200);
    if ( rv != SOC_E_NONE) {
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_errata, "
                            "failed on force AUTO-MDIX for this WAR.\n")));
    }

    /* 3. set 100 half duplex with force mode */
    mii_ctrl = 0x2000;
    rv = WRITE_PHYFEGE_MII_CTRLr(unit, pc, mii_ctrl);
    
    /* 4. write expansion reg FFh as value=0x20c0 */
    rv |= WRITE_PHY_REG(unit, pc, 0x17, 0x0fff);
    rv |= WRITE_PHY_REG(unit, pc, 0x15, 0x20c0);
    if (rv){
        DPRINTF((BSL_META_U(unit,
                            "_phy53115_a0_errata,%d,"
                            "port%d failed on writing to PHY Reg!!\n"),
                 __LINE__, port));
    }
    
    /* set SW control bit */
    PHY53115_ERRATA_AN100TX_STAT_SET(port); 
} 

void _phy53115_a0_errata_link_check(int unit, soc_port_t port, int link)
{
    phy_ctrl_t  *pc;
    
    pc = EXT_PHY_SW_STATE(unit, port);

    if (! phy53115_an100tx_war){ /* config set to stop this workaround */
        return;
    }
    
    if (!(PHY_REVISION_CHECK(pc, PHY_BCM53115_MODEL, 
                    PHY_BCM53115_OUI, BCM53115_A0_REV_ID))){
        return;
    }

    if (link) {
        /* if current status is link up, check if this errata is did but not 
         *  link-up in SW-info yet.
         */
        if (PHY53115_ERRATA_AN100TX_STAT_GET(port) && 
                !(PHY53115_ERRATA_AN100TX_LINK_GET(port))){
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_a0_errata_link_check,%d,"
                                "ge%d link-up at phy53115_a0 workaround !!\n"),
                     __LINE__, port));
            PHY53115_ERRATA_AN100TX_LINK_SET(port);
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_a0_errata_link_check,%d,"
                                "sw-info stat=0x%08x,link=0x%08x!!\n"),
                     __LINE__, phy53115_errata_an100tx_status, 
                     phy53115_errata_an100tx_linkup));
                        
            phy53115_errata_an100tx_failed_cnt[port] = 0;
        }
    } else {
        /* This workaround might not work to link-up for a long time.
         * add a flag count to control such condition.
         */
        if (PHY53115_ERRATA_AN100TX_STAT_GET(port)){
            phy53115_errata_an100tx_failed_cnt[port] ++;
        }
        
        /* if current status is link down, check if this errata is did but  
         *  the SW-info still at link-up status.
         *  (p.s. the errata link-down set will be processed in the 
         *  _robo_phy_5464_reset_setup() routine.
         */
        if (PHY53115_ERRATA_AN100TX_STAT_GET(port) && 
                PHY53115_ERRATA_AN100TX_LINK_GET(port)){
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_a0_errata_link_check,%d,"
                                "ge%d reset to free phy53115_a0 "
                                "workaround !!\n"),
                     __LINE__, port));
        
            robo_phy_5464_init(unit, port);
        } else if (phy53115_errata_an100tx_failed_cnt[port] > 
                    PHY53115_ERRATA_AN100TX_RESET_CNT){
            DPRINTF((BSL_META_U(unit,
                                "_phy53115_a0_errata_link_check,%d,"
                                "ge%d reset for phy53115_a0 "
                                "workaround failed!!\n"),
                     __LINE__, port));
            robo_phy_5464_init(unit, port);
        }
    }
}

#endif  /* PHY53115_A0_AN100TX_WORKAROUND */

#ifdef BROADCOM_DEBUG

/*
 * Function:
 *  phy_5464_shadow_dump
 * Purpose:
 *  Debug routine to dump all shadow registers.
 * Parameters:
 *  unit - Switch unit #.
 *  port - Switch port #.
 */

void
robo_phy_5464_shadow_dump(int unit, soc_port_t port)
{
    uint16      tmp;
    int         i;
    phy_ctrl_t *pc;
    int         regrw_rv = SOC_E_NONE;

    pc       = EXT_PHY_SW_STATE(unit, port);

    /* Register 0x18 Shadows */
    for (i = 0; i <= 7; i++) {
        regrw_rv = WRITE_PHY_REG(unit, pc, 0x18, (i << 12) | 0x7);
        regrw_rv |= READ_PHY_REG(unit, pc, 0x18, &tmp);
        
        if (regrw_rv){
            LOG_CLI((BSL_META_U(unit,
                                "robo_phy_5464_shadow_dump,%d, "
                                "failed on PHY reg read/write!\n"), 
                     __LINE__));
        }
        
        if ((tmp & ~7) == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x18[0x%x]=0x%04x\n"), i, tmp));
    }

    /* Register 0x1c Shadows */
    for (i = 0; i <= 0x1f; i++) {
        regrw_rv = WRITE_PHY_REG(unit, pc, 0x1c, i << 10);
        regrw_rv |= READ_PHY_REG(unit, pc, 0x1c, &tmp);
        if (regrw_rv){
            LOG_CLI((BSL_META_U(unit,
                                "robo_phy_5464_shadow_dump,%d, "
                                "failed on PHY reg read/write!\n"), 
                     __LINE__));
        }
        if ((tmp & ~0x7c00) == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x1c[0x%x]=0x%04x\n"), i, tmp));
    }

    /* Register 0x17/0x15 Shadows */
    for (i = 0; i <= 0x13; i++) {
        regrw_rv = WRITE_PHY_REG(unit, pc, 0x17, 0xf00 | i);
        regrw_rv |= READ_PHY_REG(unit, pc, 0x15, &tmp);
        if (regrw_rv){
            LOG_CLI((BSL_META_U(unit,
                                "robo_phy_5464_shadow_dump,%d, "
                                "failed on PHY reg read/write!\n"), 
                     __LINE__));
        }
        if (tmp  == 0x0000) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            "0x17[0x%x]=0x%04x\n"), i, tmp));
    }

}

#endif /* BROADCOM_DEBUG */

/*
 * Variable:    phy_5464drv_ge
 * Purpose: PHY driver for 5464
 */

phy_driver_t phy_5464robodrv_ge = {
    "ROBO 546X Gigabit PHY Driver",
    robo_phy_5464_init,
    phy_fe_ge_reset,
    robo_phy_5464_link_get,
    robo_phy_5464_enable_set,
    robo_phy_5464_enable_get,
    robo_phy_5464_duplex_set,
    robo_phy_5464_duplex_get,
    robo_phy_5464_speed_set,
    robo_phy_5464_speed_get,
    robo_phy_5464_master_set,
    robo_phy_5464_master_get,
    robo_phy_5464_autoneg_set,
    robo_phy_5464_autoneg_get,
    robo_phy_5464_adv_local_set,
    robo_phy_5464_adv_local_get,
    robo_phy_5464_adv_remote_get,
    robo_phy_5464_lb_set,
    robo_phy_5464_lb_get,
    robo_phy_5464_interface_set,
    robo_phy_5464_interface_get,
    robo_phy_5464_ability_get,
    NULL,                          /* Phy link up event */
#if LINK_DOWN_RESET_WAR
    robo_phy_5464_link_down,
#else
    NULL,
#endif
    robo_phy_5464_mdix_set,
    robo_phy_5464_mdix_get,
    robo_phy_5464_mdix_status_get,
    robo_phy_5464_medium_config_set,
    robo_phy_5464_medium_config_get,
    robo_phy_5464_medium_status,
    robo_phy_5464_cable_diag,
    robo_phy_5464_link_change,
    robo_phy_5464_control_set,    /* phy_control_set */ 
    robo_phy_5464_control_get,    /* phy_control_get */
    NULL,                         /* Phy register read */
    NULL,                         /* Phy register write */
    NULL,                         /* Phy register modify */
    NULL                          /* Phy event notify */
};

#endif /* INCLUDE_PHY_5464 && BCM_ROBO_SUPPORT */
