/*
 * $Id: serdesrobo.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        serdes.c
 * Purpose:
 *	Fiber driver for 5348 using internal QuadSerdes PHY.
 * 
 * When operating with an external PHY, this driver is not used.
 * However the speed/duplex of the internal PHY must be programmed to
 * match the MAC and external PHY settings so the data can pass through.
 * This file supplies some routines to allow mac.c to accomplish this
 * (think of the internal PHY as part of the MAC in this case):
 *
 *	phy_5690_notify_duplex
 *	phy_5690_notify_speed
 *	phy_5690_notify_stop
 *	phy_5690_notify_resume
 *
 * CMIC MIIM operations can be performed to the internal register set
 * using internal MDIO address (PORT_TO_PHY_ADDR_INT), and an external
 * PHY register set (such as BCM5424/34/64) can be programmed using the
 * external MDIO address (PORT_TO_PHY_ADDR).
 *
 * MDIO accesses to the internal PHY are not modeled on Quickturn.
 */
#include <sal/types.h>
#include <sal/core/spl.h>
#include <shared/bsl.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>
#include <soc/phy.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */

#if defined(INCLUDE_SERDES_ROBO)
#include "phyconfig.h"    /* Must be the first phy include after phydefs.h */
#include "phyident.h"
#include "phyreg.h"
#include "phynull.h"
#include "serdesrobo.h"
#include "serdes.h"


typedef struct phy_serdesrobo_state_s {
    sal_usecs_t		an_state_time;
    int			an_state_valid;
    uint16		sgmii_an_state;
    uint32		stop;
} phy_serdesrobo_state_t;

STATIC phy_serdesrobo_state_t
    *phy_serdesrobo_state[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS];

#define PHY_serdes_AN_STATE_ACCESS_INTERVAL 60000 /* 60000 Usec */

#define PHY_serdes_AN_TIME(_u, _p) \
	(phy_serdesrobo_state[(_u)][(_p)]->an_state_time)
#define PHY_serdes_AN_VALID(_u, _p) \
	(phy_serdesrobo_state[(_u)][(_p)]->an_state_valid)
#define PHY_serdes_SGMII_STATE(_u, _p) \
	(phy_serdesrobo_state[(_u)][(_p)]->sgmii_an_state)

#define PHY_serdes_AN_RECORD_TIME(_u, _p)  \
	phy_serdesrobo_state[(_u)][(_p)]->an_state_time = sal_time_usecs()
#define PHY_serdes_AN_RECORD_STATE(_u, _p, _state) \
	phy_serdesrobo_state[(_u)][(_p)]->sgmii_an_state = (_state);
#define PHY_serdes_AN_VALID_SET(_u, _p) \
	(phy_serdesrobo_state[(_u)][(_p)]->an_state_valid) = TRUE
#define PHY_serdes_AN_VALID_RESET(_u, _p) \
	(phy_serdesrobo_state[(_u)][(_p)]->an_state_valid) = FALSE

#define	PHY_serdes_AN_ACCESS_TIMEOUT(_u, _p) \
	((uint32)SAL_USECS_SUB(sal_time_usecs(), PHY_serdes_AN_TIME((_u), (_p))) \
		>= PHY_serdes_AN_STATE_ACCESS_INTERVAL)


/***********************************************************************
 *
 * FIBER DRIVER ROUTINES
 *
 ***********************************************************************/

/*
 * Function:
 *      phy_serdesrobo_attach
 * Purpose:
 *      Create serdes internal PHY driver structure
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_serdesrobo_attach(int unit, soc_port_t port)
{
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_serdesrobo_attach: u=%d p=%d\n"),
              unit, port));

    /*
     * The structure is only zeroed on the initial allocation (the flags
     * must be retained across multiple calls to phy_5464_init).
     */

    if (phy_serdesrobo_state[unit][port] == NULL) {
        phy_serdesrobo_state[unit][port] = 
            sal_alloc( sizeof (phy_serdesrobo_state_t), "physerdes");
        if (NULL == phy_serdesrobo_state[unit][port]) {
            return (SOC_E_MEMORY);
        }
        sal_memset(phy_serdesrobo_state[unit][port], 0,
                   sizeof (phy_serdesrobo_state_t));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *	phy_serdesrobo_init
 * Purpose:	
 *	Initialize serdes internal PHY driver
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 * Returns:	
 *	SOC_E_NONE
 */

STATIC int
phy_serdesrobo_init(int unit, soc_port_t port)
{
    uint8		phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16		ctrl, tmp;
    soc_timeout_t 	to;
    int			timeout = 0;

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_serdesrobo_init: u=%d p=%d\n"),
              unit, port));
    SOC_IF_ERROR_RETURN(phy_serdesrobo_attach(unit, port));

    if (!PHY_EXTERNAL_MODE(unit, port)) {
         /* PHY reset callbacks */
        SOC_IF_ERROR_RETURN(soc_phy_reset(unit, port));

        /* No external PHY. SerDes is always in fiber mode. */
        if (soc_property_port_get(unit, port,
                                  spn_SERDES_FIBER_PREF, 1)) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
        } else {
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
        }

    }

    /*
     * The internal SERDES PHY's reset bit must be held high for at
     * least 1 usec and is not self-clearing.
     */

    SOC_IF_ERROR_RETURN
	(soc_miim_read(unit, phy_addr,
		       QS_MII_CTRL_REG, &ctrl));

    ctrl |= QS_MII_CTRL_RESET;

    SOC_IF_ERROR_RETURN
	(soc_miim_write(unit, phy_addr, QS_MII_CTRL_REG, ctrl));

    soc_timeout_init(&to,
		     soc_property_get(unit,
				      spn_PHY_RESET_TIMEOUT,
				      10000000), 1);

    do {
	SOC_IF_ERROR_RETURN
	    (soc_miim_read(unit, phy_addr, QS_MII_CTRL_REG, &tmp));
	if (soc_timeout_check(&to)) {
	    timeout = 1;
	    break;
	}
    } while ((tmp & QS_MII_CTRL_RESET) != 0);

    if (timeout) {
        LOG_WARN(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_fe_ge_reset: timeout on u=%d p=%d\n"),
                  unit, port));
        
	SOC_IF_ERROR_RETURN
	    (soc_miim_write(unit, phy_addr, QS_MII_CTRL_REG, ctrl));
    }

    SOC_IF_ERROR_RETURN
	(soc_miim_read(unit, phy_addr,
		       QS_SGMII_CTRL1_REG, &ctrl));
    ctrl &= ~QS_SGMII_AUTO_DETECT;
    ctrl |= QS_SGMII_FIBER_MODE;
    SOC_IF_ERROR_RETURN
	(soc_miim_write(unit, phy_addr,
			QS_SGMII_CTRL1_REG, ctrl));

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_link_get
 * Purpose:	
 *	Determine the current link up/down status
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	link - (OUT) Boolean, true indicates link established.
 * Returns:
 *	SOC_E_XXX
 * Notes:
 *	MII_STATUS bit 2 reflects link state.
 *	This bit gets set immediately when the link is plugged in,
 *	before autoneg completes.
 *	Energy control (MISC_CONTROL bit 3) is not used.
 */

STATIC int
phy_serdesrobo_link_get(int unit, soc_port_t port, int *link)
{
    uint8		phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16		mii_stat;
#ifdef BCM_DINO16_SUPPORT
    uint32		reg_value = 0, temp = 0;

    if (SOC_IS_DINO16(unit)) {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_link_get, u = %d p = %d\n"), unit, port));

        /* get the link status from serdes phy register */
        SOC_IF_ERROR_RETURN(REG_READ_SDMIISTSr
            (unit, port, &reg_value));
        soc_SDMIISTSr_field_get(unit, &reg_value, 
            LINK_STAf, &temp);

        *link = (int)temp;
        
        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_STAT_REG, &mii_stat));

    *link = ((mii_stat & QS_MII_STAT_LA) != 0);


    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_enable_set
 * Purpose:	
 *	Enable or disable the physical interface.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	enable - Boolean, true = enable PHY, false = disable.
 * Returns:	
 *	SOC_E_XXX
 * Notes:
 *	The PHY is held in reset when disabled.  Also, the
 *	MISC_CONTROL.IDDQ bit must be set to 1 so the remote partner
 *	sees link down.
 */

STATIC int
phy_serdesrobo_enable_set(int unit, soc_port_t port, int enable)
{
    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_serdesrobo_enable_set: u=%d p=%d en=%d\n"),
              unit, port, enable));
    if (enable) {
	PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
    } else {
	PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
    }

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_enable_get
 * Purpose:	
 *	Return physical interface enable/disable state.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	enable - (OUT) Boolean, true = enable PHY, false = disable.
 * Returns:
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_enable_get(int unit, soc_port_t port, int *enable)
{
    *enable = !PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE);

    return(SOC_E_NONE);
}

/*
 * Function: 	
 *	phy_serdesrobo_duplex_set
 * Purpose:	
 *	Set the current duplex mode (forced).
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	duplex - Boolean, TRUE indicates full duplex, FALSE indicates half.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_duplex_set(int unit, soc_port_t port, int duplex)
{
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0, temp = (uint32)duplex;
#endif /* BCM_DINO16_SUPPORT */

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

#ifdef BCM_DINO16_SUPPORT
    if (SOC_IS_DINO16(unit)) {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_duplex_set, u = %d p = %d dpx = %d\n"), 
                  unit, port, duplex));

        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_set(unit, &reg_value, 
            DUPLEX_MODf, &temp);
        
        SOC_IF_ERROR_RETURN(REG_WRITE_SDMIICTLr
            (unit, port, &reg_value));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    return (duplex ? SOC_E_NONE : SOC_E_UNAVAIL);
}

/*
 * Function: 	
 *	phy_serdesrobo_duplex_get
 * Purpose:	
 *	Get the current operating duplex mode. If autoneg is enabled, 
 *	then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	duplex - (OUT) Boolean, true indicates full duplex, false 
 *		indicates half.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_duplex_get(int unit, soc_port_t port, int *duplex)
{
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0, temp = 0;
#endif /* BCM_DINO16_SUPPORT */

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

#ifdef BCM_DINO16_SUPPORT
    if (SOC_IS_DINO16(unit)) {
        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_get(unit, &reg_value, 
            DUPLEX_MODf, &temp);

        *duplex = (int)temp;
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_duplex_get, u = %d p = %d dpx = %d\n"),
                  unit, port, *duplex));
    }
#endif /* BCM_DINO16_SUPPORT */

    *duplex = TRUE;
    
    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_speed_set
 * Purpose:	
 *	Set the current operating speed (forced).
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	duplex - (OUT) Boolean, true indicates full duplex, false 
 *		indicates half.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_speed_set(int unit, soc_port_t port, int speed)
{
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0;
#endif /* BCM_DINO16_SUPPORT */

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

#ifdef BCM_DINO16_SUPPORT
    if (SOC_IS_DINO16(unit)) {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_speed_set, u = %d p = %d sp = %d\n"), 
                  unit, port, speed));
        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        /* SDMIICTLr reference soc_MIICTLr_fields, doesn't provide field
         * "Speed (MSB)".
         */
        /*Don't use reg_field_set, but set bit[6,13] directly */
        if (1000 == speed) {
            reg_value |= 0x00000040L;      /* set bit[6] */
        } else {
            reg_value &= ~0x00002040;      /* bit[6,13] = 00 */
            if (100 == speed) {
                reg_value |= 0x00002000L;  /* set bit[13] */
            }
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_SDMIICTLr
            (unit, port, &reg_value));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    if (speed == 0 || speed == 1000) {
        return SOC_E_NONE;
    }

    return SOC_E_CONFIG;
}

/*
 * Function: 	
 *	phy_serdesrobo_speed_get
 * Purpose:	
 *	Get the current operating speed. If autoneg is enabled, 
 *	then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	duplex - (OUT) Boolean, true indicates full duplex, false 
 *		indicates half.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_speed_get(int unit, soc_port_t port, int *speed)
{
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0;
#endif /* BCM_DINO16_SUPPORT */

    *speed = 1000;

#ifdef BCM_DINO16_SUPPORT
    if (SOC_IS_DINO16(unit)) {
        SOC_IF_ERROR_RETURN( REG_READ_SDMIICTLr
            (unit, port, &reg_value));

        *speed = (reg_value & 0x0040L) ? 1000 :        /* check bit[6]  */
                (reg_value & 0x2000L) ? 100 : 10;    /* check bit[13] */

        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_speed_get, u = %d p = %d sp = %d\n"), 
                  unit, port, *speed));
    }
#endif /* BCM_DINO16_SUPPORT */

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_an_set
 * Purpose:	
 *	Enable or disabled auto-negotiation on the specified port.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	an - Boolean, if true, auto-negotiation is enabled 
 *		(and/or restarted). If false, autonegotiation is disabled.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_an_set(int unit, soc_port_t port, int an)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	sgmii_ctrl;
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0, temp = (uint32)an;

    if (SOC_IS_DINO16(unit)) {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_an_set, u = %d p = %d an = %d\n"), 
                  unit, port, an));

        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_set(unit, &reg_value, 
            AN_ENf, &temp);

        SOC_IF_ERROR_RETURN(REG_WRITE_SDMIICTLr
            (unit, port, &reg_value));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_CTRL_REG, &sgmii_ctrl));

    if (an) {
        sgmii_ctrl |= QS_MII_CTRL_AE |QS_MII_CTRL_RAN;	
    } else {
        sgmii_ctrl &= ~(QS_MII_CTRL_AE | QS_MII_CTRL_RAN);
    }

    SOC_IF_ERROR_RETURN
            (soc_miim_write(unit, phy_addr, QS_MII_CTRL_REG, sgmii_ctrl));

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_an_get
 * Purpose:	
 *	Get the current auto-negotiation status (enabled/busy)
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	an - (OUT) if true, auto-negotiation is enabled.
 *	an_done - (OUT) if true, auto-negotiation is complete.
 *	        This value is undefined if an == false.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_an_get(int unit, soc_port_t port, int *an, int *an_done)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	sgmii_ctrl, sgmii_stat;
#ifdef BCM_DINO16_SUPPORT
    uint32  reg_value = 0, temp = (uint32)an;

    if (SOC_IS_DINO16(unit)) {
        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_get(unit, &reg_value, 
            AN_ENf, &temp);

        *an = (int) temp ;
        
        SOC_IF_ERROR_RETURN(REG_READ_SDMIISTSr
            (unit, port, &reg_value));
        soc_SDMIISTSr_field_get(unit, &reg_value, 
            AUTO_NEGO_COMPf, &temp);

        *an_done = (int)temp;
        
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_an_get, u = %d p = %d an = %d an_done = %d\n"),
                  unit, port, *an, *an_done));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_CTRL_REG, &sgmii_ctrl));

    *an = (sgmii_ctrl & QS_MII_CTRL_AE) ? TRUE : FALSE;

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_STAT_REG, &sgmii_stat));

    *an_done = (sgmii_stat & QS_MII_ANA_COMPLETE) != 0;

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_adv_local_set
 * Purpose:	
 *	Set the current advertisement for auto-negotiation.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	mode - Port mode mask indicating supported options/speeds.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int 
phy_serdesrobo_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	an_adv;


    SOC_IF_ERROR_RETURN
	(soc_miim_read(unit, phy_addr, QS_ANA_REG, &an_adv));


    an_adv = 0;
    if (mode & SOC_PM_1000MB_FD) {
	an_adv |= MII_ANA_C37_FD;
    }
    if (mode & SOC_PM_1000MB_HD){
	an_adv |= MII_ANA_C37_HD;
    }

    switch (mode & SOC_PM_PAUSE) {
        case SOC_PM_PAUSE:
            an_adv |= (0x1 << 7);
            break;
        case SOC_PM_PAUSE_TX:
            an_adv |= (0x2 << 7);
            break;
        case SOC_PM_PAUSE_RX:
            an_adv |= (0x3 << 7);
            break;
        }    

    SOC_IF_ERROR_RETURN
	(soc_miim_write(unit, phy_addr, QS_ANA_REG, an_adv));

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_adv_local_get
 * Purpose:	
 *	Get the current advertisement for auto-negotiation.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_adv_local_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	an_adv;

    *mode = 0;

    SOC_IF_ERROR_RETURN
	(soc_miim_read(unit, phy_addr, QS_ANA_REG, &an_adv));

    if (an_adv & MII_ANA_C37_FD) {
	*mode |= SOC_PM_1000MB_FD;
    }
    if (an_adv & MII_ANA_C37_HD) {
	*mode |= SOC_PM_1000MB_HD;
    }
    switch ((an_adv >> 7) & 0x3) {
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
 *	phy_serdesrobo_adv_remote_get
 * Purpose:	
 *	Get partner's current advertisement for auto-negotiation.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_adv_remote_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	anlpa;

    *mode = 0;

    SOC_IF_ERROR_RETURN
	(soc_miim_read(unit, phy_addr, QS_ANP_REG, &anlpa));

    if (anlpa & QS_MII_ANP_FIBER_FD) {
	*mode |= SOC_PM_1000MB_FD;
    }

    if (anlpa & QS_MII_ANP_FIBER_HD) {
	*mode |= SOC_PM_1000MB_HD;
    }

    switch (anlpa &
	    (QS_MII_ANP_FIBER_PAUSE_SYM | QS_MII_ANP_FIBER_PAUSE_ASYM)) {
    case QS_MII_ANP_FIBER_PAUSE_SYM:
	*mode |= SOC_PM_PAUSE_TX | SOC_PM_PAUSE_RX;
	break;
    case QS_MII_ANP_FIBER_PAUSE_ASYM:
	*mode |= SOC_PM_PAUSE_TX;
	break;
    case QS_MII_ANP_FIBER_PAUSE_SYM | QS_MII_ANP_FIBER_PAUSE_ASYM:
	*mode |= SOC_PM_PAUSE_RX;
	break;
    }

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_lb_set
 * Purpose:	
 *	Set the internal PHY loopback mode.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	loopback - Boolean: true = enable loopback, false = disable.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_lb_set(int unit, soc_port_t port, int enable)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	ctrl;
#ifdef BCM_DINO16_SUPPORT
    uint32	reg_value = 0, temp = (uint32)enable;

    if (SOC_IS_DINO16(unit)) {
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_lb_set, u = %d p = %d lb = %d\n"), 
                  unit, port, enable));

        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_set(unit, &reg_value, 
            LOOPBACKf, &temp);

        SOC_IF_ERROR_RETURN(REG_WRITE_SDMIICTLr
            (unit, port, &reg_value));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_CTRL_REG, &ctrl));

    if (enable) {
        ctrl |= QS_MII_CTRL_LE;
    } else {
        ctrl &= ~(QS_MII_CTRL_LE);
    }

    SOC_IF_ERROR_RETURN
        (soc_miim_write(unit, phy_addr, QS_MII_CTRL_REG, ctrl));

    return SOC_E_NONE;
}

/*
 * Function: 	
 *	phy_serdesrobo_lb_get
 * Purpose:	
 *	Get the local PHY loopback mode.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	loopback - (OUT) Boolean: true = enable loopback, false = disable.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_lb_get(int unit, soc_port_t port, int *enable)
{
    uint8	phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    uint16	ctrl;
#ifdef BCM_DINO16_SUPPORT
    uint32	reg_value = 0, temp =0;

    if (SOC_IS_DINO16(unit)) {
        SOC_IF_ERROR_RETURN(REG_READ_SDMIICTLr
            (unit, port, &reg_value));
        soc_SDMIICTLr_field_get(unit, &reg_value, 
            LOOPBACKf, &temp);

        *enable = (int)temp;
        LOG_INFO(BSL_LS_SOC_PHY,
                 (BSL_META_U(unit,
                             "phy_serdesrobo_lb_get, u = %d p = %d lb = %d\n"), 
                  unit, port, *enable));

        return SOC_E_NONE;
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, QS_MII_CTRL_REG, &ctrl));

    *enable = (ctrl & QS_MII_CTRL_LE) != 0;

    return SOC_E_NONE;
}

/*
 * Function:
 *	phy_serdesrobo_interface_set
 * Purpose:
 *	Set the current operating mode of the internal PHY.
 *	(Pertaining to the MAC/PHY interface, not the line interface).
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	pif - one of SOC_PORT_IF_*
 * Returns:
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    int			rv;

    switch (pif) {
    case SOC_PORT_IF_MII:
    case SOC_PORT_IF_GMII:
    case SOC_PORT_IF_SGMII:
    case SOC_PORT_IF_NOCXN:
	rv = SOC_E_NONE;
	break;
    default:
        rv = SOC_E_UNAVAIL;
	break;
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit,
                         "phy_serdesrobo_interface_set: u=%d p=%d pif=%d rv=%d\n"),
              unit, port, pif, rv));

    return rv;
}

/*
 * Function:
 *	phy_serdesrobo_interface_get
 * Purpose:
 *	Get the current operating mode of the internal PHY.
 *	(Pertaining to the MAC/PHY interface, not the line interface).
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    *pif = SOC_PORT_IF_GMII;

    return(SOC_E_NONE);
}

/*
 * Function: 	
 *	phy_serdesrobo_ability_get
 * Purpose:	
 *	Get the abilities of the internal PHY.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *	port - StrataSwitch port #. 
 *	mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:	
 *	SOC_E_XXX
 */

STATIC int
phy_serdesrobo_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    *mode = (SOC_PM_1000MB_FD | SOC_PM_PAUSE | SOC_PM_PAUSE_ASYMM |
	     SOC_PM_LB_PHY | SOC_PM_GMII);

    return SOC_E_NONE;
}

/*
 * Variable:	phy_serdesrobo_ge
 * Purpose:	Phy driver callouts for SERDES interface
 */
phy_driver_t phy_serdesrobo_ge = {
    "ROBO Internal SERDES PHY Driver",
    phy_serdesrobo_init, 
    phy_null_reset,
    phy_serdesrobo_link_get, 
    phy_serdesrobo_enable_set, 
    phy_serdesrobo_enable_get, 
    phy_serdesrobo_duplex_set, 
    phy_serdesrobo_duplex_get, 
    phy_serdesrobo_speed_set, 
    phy_serdesrobo_speed_get, 
    phy_serdes_master_set,   /* master_set */
    phy_serdes_master_get,  /* master_get */
    phy_serdesrobo_an_set, 
    phy_serdesrobo_an_get, 
    phy_serdesrobo_adv_local_set,
    phy_serdesrobo_adv_local_get, 
    phy_serdesrobo_adv_remote_get,
    phy_serdesrobo_lb_set,
    phy_serdesrobo_lb_get,
    phy_serdesrobo_interface_set, 
    phy_serdesrobo_interface_get, 
    phy_serdesrobo_ability_get,
    NULL,
    NULL,
    phy_null_mdix_set,
    phy_null_mdix_get,
    phy_null_mdix_status_get,
    NULL,
    NULL,
    phy_serdes_medium_status,
    NULL,                       /* phy_cable_diag */
    NULL,                       /* phy_link_change */
    NULL,                       /* phy_control_set */
    NULL                        /* phy_control_get */
};

#else /* INCLUDE_SERDES_ROBO */
int _phy_serdesrobo_not_empty;
#endif /* INCLUDE_SERDES_ROBO */

