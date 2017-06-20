/*
 * $Id: port.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * StrataSwitch port control API
 */
#include <shared/bsl.h>
#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/portmode.h>
#include <soc/port_ability.h>
#ifdef PORTMOD_SUPPORT
#include <soc/esw/portctrl.h>
#endif /* PORTMOD_SUPPORT */

int
soc_port_mode_to_ability(soc_port_mode_t mode, soc_port_ability_t *ability)
{
    uint32    port_abil;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }
 
    /* Half duplex speeds */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_10MB_HD) ? SOC_PA_SPEED_10MB : 0;
    port_abil |= (mode & SOC_PM_100MB_HD) ? SOC_PA_SPEED_100MB : 0;  
    port_abil |= (mode & SOC_PM_1000MB_HD) ? SOC_PA_SPEED_1000MB : 0; 
    port_abil |= (mode & SOC_PM_2500MB_HD) ? SOC_PA_SPEED_2500MB : 0;
    port_abil |= (mode & SOC_PM_3000MB_HD) ? SOC_PA_SPEED_3000MB : 0;
    port_abil |= (mode & SOC_PM_10GB_HD) ? SOC_PA_SPEED_10GB : 0;   
    port_abil |= (mode & SOC_PM_12GB_HD) ? SOC_PA_SPEED_12GB : 0;   
    port_abil |= (mode & SOC_PM_13GB_HD) ? SOC_PA_SPEED_13GB : 0;   
    port_abil |= (mode & SOC_PM_16GB_HD) ? SOC_PA_SPEED_16GB : 0;   
    ability->speed_half_duplex = port_abil;

    /* Full duplex speeds */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_10MB_FD) ? SOC_PA_SPEED_10MB : 0;   
    port_abil |= (mode & SOC_PM_100MB_FD) ? SOC_PA_SPEED_100MB : 0;  
    port_abil |= (mode & SOC_PM_1000MB_FD) ? SOC_PA_SPEED_1000MB : 0;
    port_abil |= (mode & SOC_PM_2500MB_FD) ? SOC_PA_SPEED_2500MB : 0;
    port_abil |= (mode & SOC_PM_3000MB_FD) ? SOC_PA_SPEED_3000MB : 0;
    port_abil |= (mode & SOC_PM_10GB_FD) ? SOC_PA_SPEED_10GB : 0;   
    port_abil |= (mode & SOC_PM_12GB_FD) ? SOC_PA_SPEED_12GB : 0;   
    port_abil |= (mode & SOC_PM_13GB_FD) ? SOC_PA_SPEED_13GB : 0;   
    port_abil |= (mode & SOC_PM_16GB_FD) ? SOC_PA_SPEED_16GB : 0;   
    ability->speed_full_duplex = port_abil;

    /* Pause Modes */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_PAUSE_TX)? SOC_PA_PAUSE_TX : 0;
    port_abil |= (mode & SOC_PM_PAUSE_RX) ? SOC_PA_PAUSE_RX : 0;
    port_abil |= (mode & SOC_PM_PAUSE_ASYMM) ? SOC_PA_PAUSE_ASYMM : 0;
    ability->pause = port_abil;

    /* Interface Types */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_TBI) ? SOC_PA_INTF_TBI : 0;
    port_abil |= (mode & SOC_PM_MII) ? SOC_PA_INTF_MII : 0;
    port_abil |= (mode & SOC_PM_GMII) ? SOC_PA_INTF_GMII : 0;
    port_abil |= (mode & SOC_PM_SGMII) ? SOC_PA_INTF_SGMII : 0; 
    port_abil |= (mode & SOC_PM_XGMII) ? SOC_PA_INTF_XGMII : 0;
    ability->interface = port_abil;

    /* Loopback Mode */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_LB_MAC) ? SOC_PA_LB_MAC : 0;
    port_abil |= (mode & SOC_PM_LB_PHY) ? SOC_PA_LB_PHY : 0;
    port_abil |= (mode & SOC_PM_LB_NONE) ? SOC_PA_LB_NONE : 0;
    ability->loopback = port_abil;

    /* Remaining Flags */
    port_abil = 0;
    port_abil |= (mode & SOC_PM_AN) ? SOC_PA_AUTONEG : 0; 
    port_abil |= (mode & SOC_PM_COMBO) ? SOC_PA_COMBO : 0;
    ability->flags = port_abil;

    return (SOC_E_NONE);
}

int
soc_port_ability_to_mode(soc_port_ability_t *ability, soc_port_mode_t *mode)
{
    uint32          port_abil;
    soc_port_mode_t port_mode;

    if ((NULL == ability) || (NULL == mode)) {
        return (SOC_E_PARAM);
    }
  
    port_mode = 0;

    /* Half duplex speeds */
    port_abil = ability->speed_half_duplex;
    port_mode |= (port_abil & SOC_PA_SPEED_10MB) ? SOC_PM_10MB_HD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_100MB) ? SOC_PM_100MB_HD : 0;  
    port_mode |= (port_abil & SOC_PA_SPEED_1000MB) ? SOC_PM_1000MB_HD : 0; 
    port_mode |= (port_abil & SOC_PA_SPEED_2500MB) ? SOC_PM_2500MB_HD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_3000MB) ? SOC_PM_3000MB_HD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_10GB) ? SOC_PM_10GB_HD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_12GB) ? SOC_PM_12GB_HD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_13GB) ? SOC_PM_13GB_HD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_16GB) ? SOC_PM_16GB_HD : 0;   

    /* Full duplex speeds */
    port_abil = ability->speed_full_duplex;
    port_mode |= (port_abil & SOC_PA_SPEED_10MB) ? SOC_PM_10MB_FD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_100MB) ? SOC_PM_100MB_FD : 0;  
    port_mode |= (port_abil & SOC_PA_SPEED_1000MB) ? SOC_PM_1000MB_FD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_2500MB) ? SOC_PM_2500MB_FD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_3000MB) ? SOC_PM_3000MB_FD : 0;
    port_mode |= (port_abil & SOC_PA_SPEED_10GB) ? SOC_PM_10GB_FD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_12GB) ? SOC_PM_12GB_FD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_13GB) ? SOC_PM_13GB_FD : 0;   
    port_mode |= (port_abil & SOC_PA_SPEED_16GB) ? SOC_PM_16GB_FD : 0;   

    /* Pause Modes */
    port_abil = ability->pause;
    port_mode |= (port_abil & SOC_PA_PAUSE_TX)? SOC_PM_PAUSE_TX : 0;
    port_mode |= (port_abil & SOC_PA_PAUSE_RX) ? SOC_PM_PAUSE_RX : 0;
    port_mode |= (port_abil & SOC_PA_PAUSE_ASYMM) ? SOC_PM_PAUSE_ASYMM : 0;

    /* Interface Types */
    port_abil = ability->interface;
    port_mode |= (port_abil & SOC_PA_INTF_TBI) ? SOC_PM_TBI : 0;
    port_mode |= (port_abil & SOC_PA_INTF_MII) ? SOC_PM_MII : 0;
    port_mode |= (port_abil & SOC_PA_INTF_GMII) ? SOC_PM_GMII : 0;
    port_mode |= (port_abil & SOC_PA_INTF_SGMII) ? SOC_PM_SGMII : 0; 
    port_mode |= (port_abil & SOC_PA_INTF_XGMII) ? SOC_PM_XGMII : 0;

    /* Loopback port_abil */
    port_abil = ability->loopback;
    port_mode |= (port_abil & SOC_PA_LB_MAC) ? SOC_PM_LB_MAC : 0;
    port_mode |= (port_abil & SOC_PA_LB_PHY) ? SOC_PM_LB_PHY : 0;
    port_mode |= (port_abil & SOC_PA_LB_NONE) ? SOC_PM_LB_NONE : 0;

    /* Remaining Flags */
    port_abil = ability->flags;
    port_mode |= (port_abil & SOC_PA_AUTONEG) ? SOC_PM_AN : 0; 
    port_mode |= (port_abil & SOC_PA_COMBO) ? SOC_PM_COMBO : 0;

    *mode = port_mode;
    return (SOC_E_NONE);
}

int
soc_port_phy_pll_os_set(int unit, soc_port_t port, uint32 vco_freq, uint32 oversample_mode, uint32 pll_divider)
{
    if(!SOC_PORT_VALID(unit, port)) {
        return SOC_E_PORT;
    }
    
#if defined(PORTMOD_SUPPORT) && defined(BCM_ESW_SUPPORT)
    if (SOC_USE_PORTCTRL(unit)) {
     SOC_IF_ERROR_RETURN(soc_portctrl_phy_control_set(unit, port, -1, -1, 0,
                         SOC_PHY_CONTROL_VCO_FREQ, vco_freq));
     SOC_IF_ERROR_RETURN(soc_portctrl_phy_control_set(unit, port, -1, -1, 0,
                         SOC_PHY_CONTROL_OVERSAMPLE_MODE, oversample_mode));
     SOC_IF_ERROR_RETURN(soc_portctrl_phy_control_set(unit, port, -1, -1, 0,
                         SOC_PHY_CONTROL_PLL_DIVIDER, pll_divider));
     /* Programming of SOC_PHY_NON_CANNED_SPEED is not supported for new chips.
      * Only Warpcore and fe1600 use it. So we do not program speed. If any
      * future device needs it, speed should be programmed here.
      */
    } else
#endif /* PORTMOD_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(soc_phyctrl_control_set(unit, port,
                            SOC_PHY_CONTROL_VCO_FREQ, vco_freq));
        SOC_IF_ERROR_RETURN(soc_phyctrl_control_set(unit, port, 
                            SOC_PHY_CONTROL_OVERSAMPLE_MODE, oversample_mode));
        SOC_IF_ERROR_RETURN(soc_phyctrl_control_set(unit, port, 
                            SOC_PHY_CONTROL_PLL_DIVIDER, pll_divider));
        SOC_IF_ERROR_RETURN(soc_phyctrl_speed_set(unit, port, 
                            SOC_PHY_NON_CANNED_SPEED));
    }
    
    return SOC_E_NONE;
}


int
soc_port_ability_mask(soc_port_ability_t *ability, soc_port_ability_t *mask)
{
    if (ability == NULL || mask == NULL) {
        return SOC_E_PARAM;
    }

    ability->flags             &= mask->flags;
    ability->loopback          &= mask->loopback;
    ability->medium            &= mask->medium;
    ability->eee               &= mask->eee;
    ability->pause             &= mask->pause;
    ability->speed_full_duplex &= mask->speed_full_duplex;
    ability->speed_half_duplex &= mask->speed_half_duplex;
    ability->interface         &= mask->interface;
    ability->channel           &= mask->channel;
    ability->fec               &= mask->fec;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_port_speed_higig2eth
 * Purpose:
 *      Convert Higig speed into related Ethernet speed
 *
 * Parameters:
 *      speed    - (IN) Higig speed (Mbps).
 * Returns:
 *      Ethernet Speed
 * Note:
 */
int
soc_port_speed_higig2eth(int speed)
{
    switch (speed) {
    case 11000:
        /* 10G */
        return 10000;
    case 21000:
        /* 20G */
        return 20000;
    case 27000:
        /* 25G */
        return 25000;
    case 42000:
        /* 40G */
        return 40000;
    case 53000:
        /* 50G */
        return 50000;
    case 106000:
        /* 100G */
        return 100000;
    default:
        return speed;
    }
}

/*
 * Function:
 *      soc_port_sister_validate
 * Purpose:
 *      Check all configured sister ports:
 *          1. has same encap mode
 *          2. has same oversub mode
 *          3. has same speed in oversub mode
 * Parameters:
 *      unit     - (IN) Unit number.
 * Returns:
 *      SOC_E_XXX
 * Note:
 *      This function is called at early stage of initialization for
 *      port configuration check.
 */
int
soc_port_sister_validate(int unit)
{
    int encap, sister_encap, speed, sister_speed, ovs, sister_ovs, lport, blk;
    soc_info_t *si = &SOC_INFO(unit);

    for (blk = 0; blk < SOC_MAX_NUM_BLKS; blk++) {
        if (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLPORT ||
            SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLPORT) {
            encap = SOC_ENCAP_COUNT;
            sister_encap = SOC_ENCAP_COUNT;
            speed = 0;
            sister_speed = 0;
            ovs = 0;
            sister_ovs = 0;
            SOC_PBMP_ITER(si->block_bitmap[blk], lport) {
                if (SOC_PBMP_MEMBER(si->all.disabled_bitmap, lport)) {
                    continue;
                }
                if (encap == SOC_ENCAP_COUNT) {
                    /* Encap and Speed of first port in one block */
                    encap = IS_HG_PORT(unit, lport) ?
                            SOC_ENCAP_HIGIG2 : SOC_ENCAP_IEEE;
                    speed = si->port_speed_max[lport];
                    ovs = SOC_PBMP_MEMBER(si->oversub_pbm, lport) ? 1 : 0;
                }
                /* Encap and Speed of sister port in one block */
                sister_encap = IS_HG_PORT(unit, lport) ?
                               SOC_ENCAP_HIGIG2 : SOC_ENCAP_IEEE;
                sister_speed = si->port_speed_max[lport];
                sister_ovs = SOC_PBMP_MEMBER(si->oversub_pbm, lport) ? 1 : 0;

                if (encap != sister_encap) {
                        LOG_ERROR(BSL_LS_SOC_PORT,
                                  (BSL_META_U(unit,
                                              "Mixed port encap is not "
                                              "acceptable on port macro %d\n"),
                                  SOC_BLOCK_NUMBER(unit, blk)));
                        return SOC_E_CONFIG;
                }

                if (ovs != sister_ovs) {
                    LOG_ERROR(BSL_LS_SOC_PORT,
                              (BSL_META_U(unit,
                                          "Mixed port oversub mode is not "
                                          "acceptable on port macro %d\n"),
                              SOC_BLOCK_NUMBER(unit, blk)));
                    return SOC_E_CONFIG;
                }

                if (ovs && speed != sister_speed) {
                    LOG_ERROR(BSL_LS_SOC_PORT,
                              (BSL_META_U(unit,
                                          "Mixed port speed is not acceptable "
                                          "in oversub mode on port macro %d\n"),
                              SOC_BLOCK_NUMBER(unit, blk)));
                    return SOC_E_CONFIG;
                }
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_port_speed_encap_validate
 * Purpose:
 *      Check speed and encap mode
 *          - HG speed with IEEE encap mode is not supported.
 * Parameters:
 *      unit     - (IN) Unit number.
 * Returns:
 *      SOC_E_XXX
 * Note:
 *      This function is called at early stage of initialization for
 *      port configuration check.
 */
int
soc_port_speed_encap_validate(int unit)
{
    soc_info_t *si = &SOC_INFO(unit);
    int port, speed_mask;

    PBMP_PORT_ITER(unit, port) {
        if (SOC_PBMP_MEMBER(si->all.disabled_bitmap, port)) {
            continue;
        }
        speed_mask = SOC_PA_SPEED(si->port_speed_max[port]);
        if (SOC_PA_IS_HG_SPEED(speed_mask) && !IS_HG_PORT(unit, port)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unmatched speed and encapsulation "
                                  "configuration for port=%d\n"),
                       port));
            return SOC_E_CONFIG;
        }
    }

    return SOC_E_NONE;
}
