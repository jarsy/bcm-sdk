/*
 *  Broadcom Switch API
 * 
 *  Robo L2 Switch API compatible with Strata/XGS Enterprise switch API.
 *  OS-Independet API Interface
 * 
 *  Copyright 2002, Broadcom Corporation
 *  All Rights Reserved.
 *  
 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 *
 * $Id: swapi.c,v 1.4 2011/09/13 07:23:29 mlarsen Exp $
 *
 */

#include <if_robo.h>
#include <swapi.h>
#if 0 /* remove for vxworks */
#include <typedefs.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <sbpci.h>
#include <bcmdevs.h>

#ifdef _CFE_
/* XXX Fix: Does not find memcpy */
#include <cfe_osl.h>
#include "lib_string.h"
#elif __linux__
#ifndef __KERNEL__
#include <memory.h>
#else
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#endif
#endif

#endif /* remove for vxworks */

/* Descriptors */
void *robo = NULL;
void *hSb = NULL;


/* defines for finding entry in ARL bins */
typedef enum
{
    BIN_TGT,    /* valid and target */
    BIN_VALID,  /* valid and not target */
    BIN_INVALID /* invalid */
} bin_stat_t;


int robo_MII_ports[] = {
    ROBO_PORT0_MII_PAGE,
    ROBO_PORT1_MII_PAGE,
    ROBO_PORT2_MII_PAGE,
    ROBO_PORT3_MII_PAGE,
    ROBO_PORT4_MII_PAGE,
    ROBO_PORT5_MII_PAGE, /* BCM5380 only */
    ROBO_PORT6_MII_PAGE, /* BCM5380 only */
    ROBO_PORT7_MII_PAGE, /* BCM5380 only */
    ROBO_IM_PORT_PAGE
};
int robo_ports[] = {
    ROBO_PORT0_CTRL,
    ROBO_PORT1_CTRL,
    ROBO_PORT2_CTRL,
    ROBO_PORT3_CTRL,
    ROBO_PORT4_CTRL,
    ROBO_PORT5_CTRL, /* BCM5380 only */
    ROBO_PORT6_CTRL, /* BCM5380 only */
    ROBO_PORT7_CTRL, /* BCM5380 only */
    ROBO_IM_PORT_CTRL
};
int robo_MIB_ports[] = {
    ROBO_PORT0_MIB_PAGE,
    ROBO_PORT1_MIB_PAGE,
    ROBO_PORT2_MIB_PAGE,
    ROBO_PORT3_MIB_PAGE,
    ROBO_PORT4_MIB_PAGE,
    ROBO_PORT5_MIB_PAGE, /* BCM5380 only */
    ROBO_PORT6_MIB_PAGE, /* BCM5380 only */
    ROBO_PORT7_MIB_PAGE, /* BCM5380 only */
    ROBO_IM_PORT_MIB_PAGE
};
int robo_VLAN_tag_ports[] = {
    ROBO_VLAN_PORT0_DEF_TAG,
    ROBO_VLAN_PORT1_DEF_TAG,
    ROBO_VLAN_PORT2_DEF_TAG,
    ROBO_VLAN_PORT3_DEF_TAG,
    ROBO_VLAN_PORT4_DEF_TAG,
    ROBO_VLAN_PORT5_DEF_TAG,  /* BCM5380 only */
    ROBO_VLAN_PORT6_DEF_TAG,  /* BCM5380 only */
    ROBO_VLAN_PORT7_DEF_TAG   /* BCM5380 only */
};

/*
 *  Portmapping: maps h/w (internal port id's) to logical
 *  port ID's
 */
robo_port_map_t
robo_port_map[] = {

    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
#ifdef BCM5380
    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
    {0, ROBO_PORT_TYPE_FE},
#else
    {0, ROBO_PORT_TYPE_NONE},
    {0, ROBO_PORT_TYPE_NONE},
    {0, ROBO_PORT_TYPE_NONE},
#endif
    {0, ROBO_PORT_TYPE_MII}
};

ROBO_MIB_AC_STRUCT port_stats[BCM_NUM_PORTS];
ROBO_MIB_AC_STRUCT mii_stats;
int mibac_count_port[BCM_NUM_PORTS];
int mibac_count_MII;
int bMIBAC = 0;

/* Required by OSL */
int bcm_get_num_ports(void)
{
    return NUM_PORTS;
}

/* API calls begin here */

/*
 * Strata/XGS compatible Port API calls.
 */
int
bcms5_port_speed_get(bcm_port_t port, int *speed)
{
    uint16 speed_status;
    uint16 port_mask;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return retval;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    port_mask = PBMP_PORT(hwport);
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_SPEED_STAT_SUMMARY,
	      (uint8 *)&speed_status, sizeof(speed_status));

    if (( speed_status & port_mask) > 0 ) {
	    *speed = 100;
    } else {
	    *speed = 10;
    }
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_speed_set(bcm_port_t port, int speed)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    int retval = BCM_RET_SUCCESS;
    uint unit = portcid(hwport);

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;

    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    if (speed == 10)
        control_reg.speed = 0;
    else if (speed == 100)
        control_reg.speed = 1;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_autoneg_get(bcm_port_t port, int *autoneg)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	*autoneg = control_reg.ANenable;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_autoneg_set(bcm_port_t port, int autoneg)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.ANenable = autoneg;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_restart_autoneg(bcm_port_t port)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.restartAN = 1;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_poweroff_get(bcm_port_t port, int *poweroff)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	*poweroff = control_reg.powerDown;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_poweroff_set(bcm_port_t port, int poweroff)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.powerDown = poweroff;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_duplex_get(bcm_port_t port, int *duplex)
{
    uint16 duplex_status;
    uint16 port_mask;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    port_mask = PBMP_PORT(hwport);
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_DUPLEX_STAT_SUMMARY,
	      (uint8 *)&duplex_status, sizeof(duplex_status));

    if (( duplex_status & port_mask) > 0 ) {
	    *duplex = ROBO_FULL_DUPLEX;
    } else {
	    *duplex = ROBO_HALF_DUPLEX;
    }
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_duplex_set(bcm_port_t port, int duplex)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.duplex = duplex;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_pause_get(bcm_port_t port,
		   int *pause_tx, int *pause_rx)
{	
    uint16 pause_status;
    uint16 port_mask;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    port_mask = PBMP_PORT(hwport);
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_PAUSE_STAT_SUMMARY,
	      (uint8 *)&pause_status, sizeof(pause_status));

    if (( pause_status & port_mask) > 0 ) {
	    *pause_tx = 1;
	    *pause_rx = 1;
    } else {
	    *pause_tx = 0;
	    *pause_rx = 0;
    }
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_pause_set(bcm_port_t port,
		   int pause_tx, int pause_rx)
{
    /* BCM5365 does not support setting pause capability */
    return 0;
}

int
bcms5_port_loopback_set(bcm_port_t port, int loopback)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.loopback = loopback;

    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_loopback_get(bcm_port_t port, int *loopback)
{
    ROBO_MII_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&control_reg, sizeof(control_reg));
	*loopback = control_reg.loopback;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_enable_set(bcm_port_t port, int enable)
{
    ROBO_PORT_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.rx_disable = !enable;
	control_reg.tx_disable = !enable;

    ROBO_WREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_enable_get(bcm_port_t port, int *enable)
{
    ROBO_PORT_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
	*enable = !control_reg.rx_disable && !control_reg.tx_disable;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_stp_get(bcm_port_t port, int *state)
{
    ROBO_PORT_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
	*state = control_reg.stp_state;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_stp_set(bcm_port_t port, int state)
{
    ROBO_PORT_CTRL_STRUC control_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
	control_reg.stp_state = state;

    ROBO_WREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_link_status_get(bcm_port_t port, int *up)
{
    uint16 port_status;
    uint16 port_mask;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    port_mask = PBMP_PORT(hwport);
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_LINK_STAT_SUMMARY,
	      (uint8 *)&port_status, sizeof(port_status));

    if (( port_status & port_mask) > 0 ) {
	    *up = 1;
    } else {
	    *up = 0;
    }
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_info_get(bcm_port_t port,
		  bcm_port_info_t* portinfo)
{	
    uint16 port_status,speed_status,duplex_status,mirror_status;
    uint16 port_mask;
    ROBO_PORT_CTRL_STRUC control_reg;
    ROBO_MII_CTRL_STRUC MII_control_reg;
    ROBO_GLOBAL_CONFIG_STRUC global_config_reg;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    /* link status */
    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    port_mask = PBMP_PORT(hwport);
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_LINK_STAT_SUMMARY,
	      (uint8 *)&port_status, sizeof(port_status));
    if (( port_status & port_mask) > 0 ) {
	    portinfo->link = 1;
    } else {
	    portinfo->link = 0;
    }
    /* port enable */
    ROBO_RREG( unit, ROBO_CTRL_PAGE,
	      robo_ports[hwport],
	      (uint8 *)&control_reg, sizeof(control_reg));
	portinfo->porten_tx = !control_reg.tx_disable;
	portinfo->porten_rx = !control_reg.rx_disable;
    /* auto-negotiate enable */
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_CTRL,
	      (uint8 *)&MII_control_reg, sizeof(MII_control_reg));
	portinfo->anmode = MII_control_reg.ANenable;
    /* speed */
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_SPEED_STAT_SUMMARY,
	      (uint8 *)&speed_status, sizeof(speed_status));
    if (( speed_status & port_mask) > 0 ) {
	    portinfo->speed = 100;
    } else {
	    portinfo->speed = 10;
    }

    /* duplex */
    ROBO_RREG(unit, ROBO_STAT_PAGE,
	      ROBO_DUPLEX_STAT_SUMMARY,
	      (uint8 *)&duplex_status, sizeof(duplex_status));
    if (( duplex_status & port_mask) > 0 ) {
	    portinfo->dpx = ROBO_FULL_DUPLEX;
    } else {
	    portinfo->dpx = ROBO_HALF_DUPLEX;
    }

    /* autocast */
    ROBO_RREG(unit, ROBO_MGMT_PAGE,
	      ROBO_GLOBAL_CONFIG,
	      (uint8 *)&global_config_reg, sizeof(global_config_reg));
	portinfo->autocast = global_config_reg.MIBac;
	
    /* mirror */
    ROBO_RREG(unit, ROBO_MGMT_PAGE,
	      ROBO_MIRROR_CAP_CTRL,
	      (uint8 *)&mirror_status, sizeof(mirror_status));
    if (( mirror_status & port_mask) > 0 ) {
	    portinfo->portmirror = 1;
    } else {
	    portinfo->portmirror = 0;
    }
	
    /* spanning tree state */
	portinfo->stp_state = control_reg.stp_state;
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_advert_get(bcm_port_t port, bcm_an_advert_t* an_advert)
{
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_ANA_REG,
	      (uint8 *)an_advert, sizeof(ROBO_MII_AN_ADVERT_STRUC));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_advert_set(bcm_port_t port, bcm_an_advert_t* an_advert)
{
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;
    bcm_an_advert_t regval;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    /* always set advertise selector field bit */
    regval = *an_advert;
    regval.selector = 1;   
    ROBO_WREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_ANA_REG,
	      (uint8 *)&regval, sizeof(ROBO_MII_AN_ADVERT_STRUC));
    bcm_rel_sema();
    return retval;
}

int
bcms5_port_advert_remote_get(bcm_port_t port, bcm_an_advert_t* an_advert)
{
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG( unit, robo_MII_ports[hwport],
	      ROBO_MII_ANP_REG,
	      (uint8 *)an_advert, sizeof(ROBO_MII_AN_ADVERT_STRUC));
    bcm_rel_sema();
    return retval;
}

int
bcms5_stat_clear(bcm_port_t port)
{
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;
    uint regAddr = 0;
    uint regValue = 0;
    unsigned long long regValueLong = 0;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;

    for (regAddr = ROBO_MIB_TX_OCTETS; regAddr <= ROBO_MIB_RX_SYMBOL_ERROR;
            regAddr += sizeof(uint))
    {
        ROBO_WREG(unit, robo_MIB_ports[hwport],
    	      regAddr,
    	      (uint8 *)&regValue, sizeof(regValue));
    }
    /* clear 64-bit regs separately */
    ROBO_WREG(unit, robo_MIB_ports[hwport],
	      ROBO_MIB_TX_OCTETS,
	      (uint8 *)&regValueLong, sizeof(regValueLong));
    ROBO_WREG(unit, robo_MIB_ports[hwport],
	      ROBO_MIB_RX_OCTETS,
	      (uint8 *)&regValueLong, sizeof(regValueLong));
    bcm_rel_sema();
    return retval;
}

int
bcms5_stat_clear_all(int unit)
{
    ROBO_GLOBAL_CONFIG_STRUC config;
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG(unit, ROBO_MGMT_PAGE,
	      ROBO_GLOBAL_CONFIG,
	      (uint8 *)&config, sizeof(config));
	config.resetMIB = 1;
    ROBO_WREG(unit, ROBO_MGMT_PAGE,
	      ROBO_GLOBAL_CONFIG,
	      (uint8 *)&config, sizeof(config));
	config.resetMIB = 0;
    ROBO_WREG(unit, ROBO_MGMT_PAGE,
	      ROBO_GLOBAL_CONFIG,
	      (uint8 *)&config, sizeof(config));
    bcm_rel_sema();
    return retval;
}

/* get a register value from a MIB autocast port structure */
int brcm_get_mibac_reg(int port, int offset, int size, uint8 *regval)
{
    ROBO_MIB_AC_STRUCT *pStats;

    /* if not enabled, just return */
    if (!bMIBAC)
        return 0;

    if (port <= BCM_NUM_PORTS)
        pStats = &port_stats[port-1];
    else
        pStats = &mii_stats;

    switch(offset)
    {
        case ROBO_MIB_TX_OCTETS:
        memcpy(regval,&pStats->TxOctets,size);
        break;
        case ROBO_MIB_TX_DROP_PKTS:
        memcpy(regval,&pStats->TxDropPkts,size);
        break;
        case ROBO_MIB_TX_BC_PKTS:
        memcpy(regval,&pStats->TxBroadcastPkts,size);
        break;
        case ROBO_MIB_TX_MC_PKTS:
        memcpy(regval,&pStats->TxMulticastPkts,size);
        break;
        case ROBO_MIB_TX_UC_PKTS:
        memcpy(regval,&pStats->TxUnicastPkts,size);
        break;
        case ROBO_MIB_TX_COLLISIONS:
        memcpy(regval,&pStats->TxCollisions,size);
        break;
        case ROBO_MIB_TX_SINGLE_COLLISIONS:
        memcpy(regval,&pStats->TxSingleCollision,size);
        break;
        case ROBO_MIB_TX_MULTI_COLLISIONS:
        memcpy(regval,&pStats->TxMultiCollision,size);
        break;
        case ROBO_MIB_TX_DEFER_TX:
        memcpy(regval,&pStats->TxDeferredTransmit,size);
        break;
        case ROBO_MIB_TX_LATE_COLLISIONS:
        memcpy(regval,&pStats->TxLateCollision,size);
        break;
        case ROBO_MIB_EXCESS_COLLISIONS:
        memcpy(regval,&pStats->TxExcessiveCollision,size);
        break;
        case ROBO_MIB_FRAME_IN_DISCARDS:
        memcpy(regval,&pStats->TxFrameInDiscards,size);
        break;
        case ROBO_MIB_TX_PAUSE_PKTS:
        memcpy(regval,&pStats->TxPausePkts,size);
        break;
        case ROBO_MIB_RX_OCTETS:
        memcpy(regval,&pStats->RxOctets,size);
        break;
        case ROBO_MIB_RX_UNDER_SIZE_PKTS:
        memcpy(regval,&pStats->RxUndersizePkts,size);
        break;
        case ROBO_MIB_RX_PAUSE_PKTS:
        memcpy(regval,&pStats->RxPausePkts,size);
        break;
        case ROBO_MIB_RX_PKTS_64:
        memcpy(regval,&pStats->RxPkts64Octets,size);
        break;
        case ROBO_MIB_RX_PKTS_65_TO_127:
        memcpy(regval,&pStats->RxPkts64to127Octets,size);
        break;
        case ROBO_MIB_RX_PKTS_128_TO_255:
        memcpy(regval,&pStats->RxPkts128to255Octets,size);
        break;
        case ROBO_MIB_RX_PKTS_256_TO_511:
        memcpy(regval,&pStats->RxPkts256to511Octets,size);
        break;
        case ROBO_MIB_RX_PKTS_512_TO_1023:
        memcpy(regval,&pStats->RxPkts512to1023Octets,size);
        break;
        case ROBO_MIB_RX_PKTS_1024_TO_1522:
        memcpy(regval,&pStats->RxPkts1024to1522Octets,size);
        break;
        case ROBO_MIB_RX_OVER_SIZE_PKTS:
        memcpy(regval,&pStats->RxOversizePkts,size);
        break;
        case ROBO_MIB_RX_JABBERS:
        memcpy(regval,&pStats->RxJabbers,size);
        break;
        case ROBO_MIB_RX_ALIGNMENT_ERRORS:
        memcpy(regval,&pStats->RxAlignmentErrors,size);
        break;
        case ROBO_MIB_RX_FCS_ERRORS:
        memcpy(regval,&pStats->RxFCSErrors,size);
        break;
        case ROBO_MIB_RX_GOOD_OCTETS:
        memcpy(regval,&pStats->RxGoodOctets,size);
        break;
        case ROBO_MIB_RX_DROP_PKTS:
        memcpy(regval,&pStats->RxDropPkts,size);
        break;
        case ROBO_MIB_RX_UC_PKTS:
        memcpy(regval,&pStats->RxUnicastPkts,size);
        break;
        case ROBO_MIB_RX_MC_PKTS:
        memcpy(regval,&pStats->RxMulticastPkts,size);
        break;
        case ROBO_MIB_RX_BC_PKTS:
        memcpy(regval,&pStats->RxBroadcastPkts,size);
        break;
        case ROBO_MIB_RX_SA_CHANGES:
        memcpy(regval,&pStats->RxSAChanges,size);
        break;
        case ROBO_MIB_RX_FRAGMENTS:
        memcpy(regval,&pStats->RxFragments,size);
        break;
        case ROBO_MIB_RX_EXCESS_SZ_DISC:
        memcpy(regval,&pStats->RxExcessSizeDisc,size);
        break;
        case ROBO_MIB_RX_SYMBOL_ERROR:
        memcpy(regval,&pStats->RxSymbolError,size);
        break;

    }
    return 1;
}

int
bcms5_stat_get(bcm_port_t port,
	     bcm_stat_val_t type, uint64 *value)
{
    static int sum1Regs[] = ROBO_MIB_SUM1_REGS;
    static int sum2Regs[] = ROBO_MIB_SUM2_REGS;
    static int sum3Regs[] = ROBO_MIB_SUM3_REGS;
    static int sum4Regs[] = ROBO_MIB_SUM4_REGS;
    static int sum5Regs[] = ROBO_MIB_SUM5_REGS;
    static int sum6Regs[] = ROBO_MIB_SUM6_REGS;
    static int sum7Regs[] = ROBO_MIB_SUM7_REGS;
    static int sum8Regs[] = ROBO_MIB_SUM8_REGS;
    static int sum9Regs[] = ROBO_MIB_SUM9_REGS;
    int regAddr = type, *regAddrPtr = &regAddr, i;
    int numRegs = 1, szReg;
    unsigned long long retVal = 0, sumVal = 0;
    uint hwport = hwport(port);
    uint unit = portcid(hwport);
    int retval = BCM_RET_SUCCESS;

    /*assert(port < NUM_MII_PORTS);*/

    if (!IF_PORT_EXISTS(hwport))
        return BCM_RET_SUCCESS;
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    if (type >= ROBO_MIB_NOT_DEFINED)
    {
        switch (type)
        {
            case ROBO_MIB_SUM1:
            regAddrPtr = &sum1Regs[0];
            numRegs = sizeof(sum1Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM2:
            regAddrPtr = &sum2Regs[0];
            numRegs = sizeof(sum2Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM3:
            regAddrPtr = &sum3Regs[0];
            numRegs = sizeof(sum3Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM4:
            regAddrPtr = &sum4Regs[0];
            numRegs = sizeof(sum4Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM5:
            regAddrPtr = &sum5Regs[0];
            numRegs = sizeof(sum5Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM6:
            regAddrPtr = &sum6Regs[0];
            numRegs = sizeof(sum6Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM7:
            regAddrPtr = &sum7Regs[0];
            numRegs = sizeof(sum7Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM8:
            regAddrPtr = &sum8Regs[0];
            numRegs = sizeof(sum8Regs)/sizeof(int);
            break;

            case ROBO_MIB_SUM9:
            regAddrPtr = &sum9Regs[0];
            numRegs = sizeof(sum9Regs)/sizeof(int);
            break;

            default:
            numRegs = 0;

        }
    }
    /* if using MIB autocast, check to see if stats struct needs updating */
#if 0 /* remove for vxworks */
    bcm_update_MIB_AC_stats(port);
#endif /* remove for vxworks */
    for (i=0;i<numRegs;i++)
    {
        if (*regAddrPtr == ROBO_MIB_TX_OCTETS || *regAddrPtr == ROBO_MIB_RX_OCTETS
                || *regAddrPtr == ROBO_MIB_RX_GOOD_OCTETS)
            szReg = sizeof(uint64);
        else
            szReg = sizeof(uint32);
        /* if mib autocast not running (ie, returns 0), read reg instead */
        if (!brcm_get_mibac_reg(port,*regAddrPtr,szReg,(uint8 *)&retVal))
        {
            ROBO_RREG(unit, robo_MIB_ports[hwport], *regAddrPtr, (uint8 *)&retVal, szReg);
        }
        sumVal += retVal;
    	regAddrPtr++;
    }
    memcpy (value,&sumVal,sizeof(*value));
    bcm_rel_sema();
    return retval;
}

int
bcms5_stat_get32(bcm_port_t port, 
	       bcm_stat_val_t type, uint32 *value)
{
    uint64  retVal;

    bcms5_stat_get(port, type, &retVal);
    memcpy (value,&retVal,sizeof(*value));
    return 0;
}



/*
 * 802.1Q and Port Based VLAN support API's
 */
void
bcm_get_vaddr(bcm_vlan_t vid, uint *vaddr, uint *vindex)
{
    uint retval = 0, retvalUpper = 0, lsbmask = 0x01, msbmask = 0x0100, i;

    /* convert wire form to canonical form by bit-swapping lower byte */
    /* and upper nibble */
    for (i=0;i<8;i++)
    {
        retval |= (vid & lsbmask);
        if (i<4)
        {
            retvalUpper |= (vid & msbmask);
            if (i<3)
                /* don't shift after last mask */
                retvalUpper <<=1;
        }
        vid >>= 1;
        if (i<7)
            /* don't shift after last mask */
            retval <<= 1;
    }
    retval |= retvalUpper;
    /* lsb is upper/lower index */
    *vindex = retval & 0x1;
    *vaddr = (retval>>1) + VLAN_TABLE_ADDR;
    return;
}

void
bcm_get_vid(bcm_vlan_t *vid, uint vaddr, uint vindex)
{
    uint retval = 0, retvalUpper = 0, lsbmask = 0x01, msbmask = 0x0100, i;

    /* convert wire form to canonical form by bit-swapping lower byte */
    /* and upper nibble */
    vaddr = ((vaddr - VLAN_TABLE_ADDR)<<1) | vindex;
    for (i=0;i<8;i++)
    {
        retval |= (vaddr & lsbmask);
        if (i<4)
        {
            retvalUpper |= (vaddr & msbmask);
            if (i<3)
                /* don't shift after last mask */
                retvalUpper <<=1;
        }
        vaddr >>= 1;
        if (i<7)
            /* don't shift after last mask */
            retval <<= 1;
    }
    retval |= retvalUpper;
    *vid = retval;
    return;
}

void
bcm_byteswap(bcm_mac_t *macAddr)
{
    bcm_mac_t retmac;

    retmac[0] = (*macAddr)[5];
    retmac[1] = (*macAddr)[4];
    retmac[2] = (*macAddr)[3];
    retmac[3] = (*macAddr)[2];
    retmac[4] = (*macAddr)[1];
    retmac[5] = (*macAddr)[0];
    memcpy(macAddr, &retmac, sizeof(retmac));
}

void
bcm_bitswap(bcm_mac_t *macAddr)
{
    uint retval, lsbmask, i, j;
    bcm_mac_t retmac;

    /* first byteswap the bytes and then bitswap the bits */
    retmac[0] = (*macAddr)[5];
    retmac[1] = (*macAddr)[4];
    retmac[2] = (*macAddr)[3];
    retmac[3] = (*macAddr)[2];
    retmac[4] = (*macAddr)[1];
    retmac[5] = (*macAddr)[0];
    for (i=0;i<6;i++)
    {
        lsbmask = 0x01;
        retval = 0;
        for (j=0;j<8;j++)
        {
            retval |= (retmac[i] & lsbmask);
            retmac[i] >>= 1;
            if (j<7)
                /* don't shift after last mask */
                retval <<= 1;
        }
        retmac[i] = retval;
    }
    memcpy(macAddr, &retmac, sizeof(retmac));
}

uint
bcm_bitswap_vlan(uint vlan)
{
    uint retvalLower, retvalUpper, mask, j;
    uint vlanLocal;

    mask = 0x01;
    retvalLower = 0;
    vlanLocal = vlan;
    for (j=0;j<8;j++)
    {
        retvalLower |= (vlanLocal & mask);
        vlanLocal >>= 1;
        if (j<7)
            /* don't shift after last mask */
            retvalLower <<= 1;
    }
    mask = 0x100;
    retvalUpper = 0;
    vlanLocal = vlan;
    for (j=0;j<4;j++)
    {
        retvalUpper |= (vlanLocal & mask);
        vlanLocal >>= 1;
        if (j<3)
            /* don't shift after last mask */
            retvalUpper <<= 1;
    }
    return retvalLower | retvalUpper;
}

int
bcms5_vlan_enable(int unit, int enable)
{
    ROBO_VLAN_CTRL0_STRUC controlReg;
    int retval = BCM_RET_SUCCESS;

    /* read control reg, modify bit and re-write */
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_CTRL0,
	      (uint8 *)&controlReg, sizeof(controlReg));
	controlReg.VLANen = enable;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_CTRL0,
	      (uint8 *)&controlReg, sizeof(controlReg));
    bcm_rel_sema();
    return retval;
}

int
bcms5_vlan_create(int unit, bcm_vlan_t vid)
{
#ifdef BCM5380
    uint vaddr;
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    ROBO_MEM_ACCESS_DATA_STRUC tableContent;
    uint  vlanIndex;
#else
    ROBO_VLAN_TABLE_ACCESS_STRUC tableAccessReg;
    uint16  tableContent = 0;
#endif
    int count;
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
#ifdef BCM5380
    /* write null register value.  since there are 2 vlan's per 64-bit entry, */
    /* need to do read/modify/write */
    /* convert vid to canonical form */
    bcm_get_vaddr(vid,&vaddr,&vlanIndex);
    tableAccessReg.memAddr = vaddr;
    tableAccessReg.readEn = MEM_TABLE_READ;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&tableContent.memData[0], sizeof(tableContent));
	tableContent.memData[vlanIndex] = 0;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&tableContent, sizeof(tableContent));
	/* now enable table write */
    tableAccessReg.memAddr = vaddr;
    tableAccessReg.readEn = MEM_TABLE_WRITE;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
bcm_exit:
#else
    /* write null register value */
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_WRITE,
	      (uint8 *)&tableContent, sizeof(tableContent));
	/* now enable table write */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_WRITE;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* now check to make sure write suceeded */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
#endif
    bcm_rel_sema();
    return retval;
}
int
bcms5_vlan_port_add(int unit,
		  bcm_vlan_t vid,
		  bcm_pbmp_t pbmp, bcm_pbmp_t ubmp)
{
    uint16 defaultTag = vid;
#ifdef BCM5380
    uint vaddr;
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    ROBO_MEM_ACCESS_DATA_STRUC tableContent;
    uint vlanPortSettingsNew = pbmp | (ubmp<<VLAN_UNTAG_SHIFT) | VLAN_VALID;
    uint vlanIndex = vid & 0x1;
#else
    ROBO_VLAN_TABLE_ACCESS_STRUC tableAccessReg;
    uint16 vlanPortSettings = 0;
    uint16 vlanPortSettingsNew = pbmp | (ubmp<<VLAN_UNTAG_SHIFT) | VLAN_VALID;
#endif
    int i, count;
    int retval = BCM_RET_SUCCESS;

    /* first write default tag (same as VID) to all selected ports */
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    for (i=0;i<NUM_SWITCH_PORTS;i++)
    {
        if (pbmp & PBMP_PORT(i))
        { 	
            ROBO_WREG(unit, ROBO_VLAN_PAGE,
        	      robo_VLAN_tag_ports[i],
        	      (uint8 *)&defaultTag, sizeof(defaultTag));
        }
    }
#ifdef BCM5380
    /* now read/modify/write bitmaps. note that table entries are 64-bits */
    /* wide, with 2 vid's per entry */
    /* convert vid to canonical form */
    bcm_get_vaddr(vid,&vaddr,&vlanIndex);
    memset(&tableAccessReg,0,sizeof(tableAccessReg));
    tableAccessReg.memAddr = vaddr;
    tableAccessReg.readEn = MEM_TABLE_READ;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&tableContent.memData[0], sizeof(tableContent));
	tableContent.memData[vlanIndex] |= vlanPortSettingsNew;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&tableContent.memData[0], sizeof(tableContent));
	
	/* now enable table write */
    tableAccessReg.memAddr = vaddr;
    tableAccessReg.readEn = MEM_TABLE_WRITE;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
#else
    /* now read/modify/write bitmaps */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_READ;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_WRITE,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	vlanPortSettings |= vlanPortSettingsNew;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_WRITE,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	/* now enable table write */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_WRITE;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
#endif
bcm_exit:
    bcm_rel_sema();
    return retval;
}

#if 0
int
bcm_vlan_port_remove(int unit,
		     bcm_vlan_t vid,
		     bcm_pbmp_t pbmp)
{
#ifdef BCM5380
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    uint vlanPortSettings[2] = {0,0};
    uint vlanIndex = vid & 0x1;
#else
    ROBO_VLAN_TABLE_ACCESS_STRUC tableAccessReg;
    uint16 vlanPortSettings = 0;
#endif
    int count;
    int retval = BCM_RET_SUCCESS;


    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
#ifdef BCM5380
    /* now read/modify/write bitmaps. note that table entries are 64-bits */
    /* wide, with 2 vid's per entry */
    tableAccessReg.memAddr = vid>>1;
    tableAccessReg.readEn = MEM_TABLE_READ;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	vlanPortSettings[vlanIndex] &= ~pbmp;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	/* now enable table write */
    tableAccessReg.memAddr = vid>>1;
    tableAccessReg.readEn = MEM_TABLE_WRITE;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
#else
    /* read/modify/write bitmaps */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_READ;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_WRITE,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	vlanPortSettings &= ~pbmp;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_WRITE,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	/* now enable table write */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_WRITE;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	    retval = BCM_RET_TIMEOUT;
#endif
bcm_exit:
    bcm_rel_sema();
    return retval;
}
int
bcm_vlan_port_get(int unit,
		  bcm_vlan_t vid,
		  bcm_pbmp_t *pbmp, bcm_pbmp_t *ubmp)
{	
#ifdef BCM5380
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    uint vlanPortSettings[2] = {0,0};
    uint vlanIndex = vid & 0x1;
#else
    ROBO_VLAN_TABLE_ACCESS_STRUC tableAccessReg;
#endif
    ROBO_VLAN_READ_WRITE_STRUC readWriteReg;
    int count;
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
#ifdef BCM5380
    /* read bitmaps */
    tableAccessReg.memAddr = vid>>1;
    tableAccessReg.readEn = MEM_TABLE_READ;
    tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
    ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_CTRL,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
	      ROBO_MEM_ACCESS_DATA,
	      (uint8 *)&vlanPortSettings, sizeof(vlanPortSettings));
	memcpy(&readWriteReg,&vlanPortSettings[vlanIndex],sizeof(uint));
#else
    /* read bitmaps */
    tableAccessReg.VLANid = vid;
    tableAccessReg.VLANidHi = VLAN_ID_HIGH_BITS;
    tableAccessReg.readWriteState = VLAN_TABLE_READ;
    tableAccessReg.readWriteEnable = 1;
    ROBO_WREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_TABLE_ACCESS,
	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
	/* wait for complete */
	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
	{
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
    	if (!tableAccessReg.readWriteEnable)
    	    break;
	};
	if (count >= BCM_TIMEOUT_VAL)
	{
	    retval = BCM_RET_TIMEOUT;
	    goto bcm_exit;
	}
    ROBO_RREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_READ,
	      (uint8 *)&readWriteReg, sizeof(readWriteReg));
#endif
	if (!readWriteReg.valid)
	    retval = BCM_RET_INVALID_VID;
	*pbmp = readWriteReg.VLANgroup;
	*ubmp = readWriteReg.VLANuntag;
bcm_exit:
    bcm_rel_sema();
    return retval;
}

/* Native VLAN Support (the VLAN ID the switch should tag an untagged frame
 * with as the packet ingresses the switch fabric).
 */
int
bcm_vlan_default_get(int unit, bcm_vlan_t *vid_ptr)
{
    return 0;
}

int
bcm_vlan_default_set(int unit, bcm_vlan_t vid)
{
    return 0;
}
#endif
/*
 * dump valid entries in ARL table (up to 'length' in bytes)
 */
int
bcm_l2_addr_dumptable(int unit, bcm_arl_table_entry_t *table, int *numEntries)
{
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
#ifdef BCM5380
    int i;
    ROBO_ARL_TABLE_DATA_STRUC tableData;
#else
    ROBO_ARL_SEARCH_CTRL_STRUC tableSearchCtrlReg;
    ROBO_ARL_SEARCH_RESULT_STRUC tableSearchResultReg;
    ROBO_ARL_SEARCH_RESULT_MCAST_STRUC tableSearchResultMcastReg;
    bcm_mac_t   mcastAddr = BRIDGE_GROUP_MAC_ADDR;
    uint16 arlAddr;
#endif
    int count, tableIndex = 0;
    int retval = BCM_RET_SUCCESS;
    
    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
#ifdef BCM5380
    /* read all table entries, but only save valid ones (up to size of table */
    /* provided */
    for (i=0;i<NUM_ARL_TABLE_ENTRIES;i++)
    {
        if (tableIndex >= (*numEntries))
          goto bcm_exit;
        tableAccessReg.memAddr = i;
        tableAccessReg.readEn = MEM_TABLE_READ;
        tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
        ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
        /* wait for complete */
        for (count = 0;count<BCM_TIMEOUT_VAL;count++)
        {
             ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
         	      ROBO_MEM_ACCESS_CTRL,
         	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
         	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
         	    break;
        };
        if (count >= BCM_TIMEOUT_VAL)
        {
           retval = BCM_RET_TIMEOUT;
           goto bcm_exit;
        }
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_DATA,
    	      (uint8 *)&tableData, sizeof(ROBO_ARL_TABLE_DATA_STRUC));
        /* only save valid entries */
        if (tableData.valid)
        {
            table[tableIndex].vid = bcm_bitswap_vlan((uint)tableData.vid);
            memcpy(&table[tableIndex].MACaddr,&tableData.MACaddr,sizeof(bcm_mac_t));
            bcm_bitswap(&table[tableIndex].MACaddr);
            table[tableIndex].arl_addr = i;
            table[tableIndex].port = tableData.portID;
            table[tableIndex].staticAddr = tableData.staticAddr;
            table[tableIndex].highPrio = tableData.highPrio;
            table[tableIndex].age = tableData.age;
            tableIndex++;
        }
  }
#else
  /* for BCM5365, use search function to dump table */
  tableSearchCtrlReg.valid = 0;
  tableSearchCtrlReg.ARLStart = 1;
  /* start search */
  ROBO_WREG(unit, ROBO_ARLIO_PAGE,
        ROBO_ARL_SEARCH_CTRL,
        (uint8 *)&tableSearchCtrlReg, sizeof(tableSearchCtrlReg));
  /* now loop till all valid entries read */
  while (TRUE)
  {
    if (tableIndex >= (*numEntries))
      goto bcm_exit;
    /* wait for complete */
    for (count = 0;count<BCM_TIMEOUT_VAL;count++)
    {
         ROBO_RREG(unit, ROBO_ARLIO_PAGE,
     	      ROBO_ARL_SEARCH_CTRL,
     	      (uint8 *)&tableSearchCtrlReg, sizeof(tableSearchCtrlReg));
     	if (tableSearchCtrlReg.valid)
     	    break;
        /* check to see if end of search */
        if (!tableSearchCtrlReg.ARLStart)
            goto bcm_exit;
    };
    if (count >= BCM_TIMEOUT_VAL)
    {
       retval = BCM_RET_TIMEOUT;
       goto bcm_exit;
    }
    /* read ARL addr */
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
          ROBO_ARL_SEARCH_ADDR,
          (uint8 *)&arlAddr, sizeof(arlAddr));
    /* now read result */
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
          ROBO_ARL_SEARCH_RESULT,
          (uint8 *)&tableSearchResultReg, sizeof(tableSearchResultReg));
    /* transfer result to output */
    table[tableIndex].arl_addr = arlAddr & 0x7fff; /* mask off valid bit */
    memcpy(&table[tableIndex].MACaddr,&tableSearchResultReg.mac.macBytes,
    		sizeof(bcm_mac_t));
    bcm_byteswap(&table[tableIndex].MACaddr);
    table[tableIndex].port = tableSearchResultReg.ctrl.portID;
    table[tableIndex].age = tableSearchResultReg.ctrl.age;
    table[tableIndex].staticAddr = tableSearchResultReg.ctrl.staticEn;
    table[tableIndex].vid = tableSearchResultReg.ctrl.vid;
    table[tableIndex].highPrio = 0;
    /* check to see if this is a mcast addr, if so, need to read port mask bits */
    if (!memcmp(&table[tableIndex].MACaddr[5],&mcastAddr[5],1))
    {
		/* do mem table read to get mcast bits */
		memset(&tableAccessReg,0,sizeof(tableAccessReg));
        tableAccessReg.memAddr = table[tableIndex].arl_addr + ARL_TABLE_ADDR;
        tableAccessReg.readEn = MEM_TABLE_READ;
        tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
        ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
        /* wait for complete */
        for (count = 0;count<BCM_TIMEOUT_VAL;count++)
        {
             ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
         	      ROBO_MEM_ACCESS_CTRL,
         	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
         	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
         	    break;
        };
        if (count >= BCM_TIMEOUT_VAL)
        {
           retval = BCM_RET_TIMEOUT;
           goto bcm_exit;
        }
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_DATA,
    	      (uint8 *)&tableSearchResultMcastReg, sizeof(tableSearchResultMcastReg));
        /* now transfer port mask bits to output structure */
        table[tableIndex].port = tableSearchResultMcastReg.ctrl.portMask;
  	}
    tableIndex++;
  }
#endif
bcm_exit:
    *numEntries = tableIndex;
    bcm_rel_sema();
    return retval;
}

/*
 * dump valid entries in vlan table (up to 'length' in bytes)
 */
int
bcm_vlan_dumptable(int unit, bcm_vlan_table_entry_t *table, int *numEntries)
{
#ifdef BCM5380
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    ROBO_MEM_ACCESS_DATA_STRUC tableData;
#else
    ROBO_VLAN_READ_WRITE_STRUC entry;
    ROBO_VLAN_TABLE_ACCESS_STRUC vlanTableAccessReg;
#endif
    ROBO_VLAN_READ_WRITE_STRUC *pentry;
    int count, i, tableIndex = 0;
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
#ifdef BCM5380
    /* read all table entries, but only save valid ones (up to size of table */
    /* provided */
    for (i=0;i<NUM_VLAN_TABLE_ENTRIES;i++)
    {
        if (tableIndex >= (*numEntries))
          goto bcm_exit;
        tableAccessReg.memAddr = i + VLAN_TABLE_ADDR;
        tableAccessReg.readEn = MEM_TABLE_READ;
        tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
        ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
        /* wait for complete */
        for (count = 0;count<BCM_TIMEOUT_VAL;count++)
        {
             ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
         	      ROBO_MEM_ACCESS_CTRL,
         	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
         	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
         	    break;
        };
        if (count >= BCM_TIMEOUT_VAL)
        {
           retval = BCM_RET_TIMEOUT;
           goto bcm_exit;
        }
        ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_DATA,
    	      (uint8 *)&tableData, sizeof(ROBO_MEM_ACCESS_DATA_STRUC));
        /* only save valid entries */
        pentry = (ROBO_VLAN_READ_WRITE_STRUC *)&tableData.memData[0];
        if (pentry->valid)
        {
            table[tableIndex].vlan_entry.VLANgroup = pentry->VLANgroup;
            table[tableIndex].vlan_entry.VLANuntag = pentry->VLANuntag;
            table[tableIndex].vlan_addr = i + VLAN_TABLE_ADDR;
            table[tableIndex].vlan_index = 0;
            bcm_get_vid(&table[tableIndex].vlan_id, i + VLAN_TABLE_ADDR, 0);
            tableIndex++;
        }
        if (tableIndex >= (*numEntries))
          goto bcm_exit;
        pentry = (ROBO_VLAN_READ_WRITE_STRUC *)&tableData.memData[1];
        if (pentry->valid)
        {
            table[tableIndex].vlan_entry.VLANgroup = pentry->VLANgroup;
            table[tableIndex].vlan_entry.VLANuntag = pentry->VLANuntag;
            table[tableIndex].vlan_addr = i + VLAN_TABLE_ADDR;
            table[tableIndex].vlan_index = 1;
            bcm_get_vid(&table[tableIndex].vlan_id, i + VLAN_TABLE_ADDR, 1);
            tableIndex++;
        }
  	}
#else
    /* read all table entries, but only save valid ones (up to size of table */
    /* provided */
    memset(&vlanTableAccessReg,0,sizeof(vlanTableAccessReg));
    for (i=0;i<NUM_VLAN_TABLE_ENTRIES;i++)
    {
        if (tableIndex >= (*numEntries))
          goto bcm_exit;
        vlanTableAccessReg.VLANid = i;
        vlanTableAccessReg.readWriteState = VLAN_TABLE_READ;
        vlanTableAccessReg.readWriteEnable = 1;
        ROBO_WREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_TABLE_ACCESS,
    	      (uint8 *)&vlanTableAccessReg, sizeof(vlanTableAccessReg));
        /* wait for complete */
        for (count = 0;count<BCM_TIMEOUT_VAL;count++)
        {
             ROBO_RREG(unit, ROBO_VLAN_PAGE,
         	      ROBO_VLAN_TABLE_ACCESS,
         	      (uint8 *)&vlanTableAccessReg, sizeof(vlanTableAccessReg));
         	if (!vlanTableAccessReg.readWriteEnable)
         	    break;
        };
        if (count >= BCM_TIMEOUT_VAL)
        {
           retval = BCM_RET_TIMEOUT;
           goto bcm_exit;
        }
        ROBO_RREG(unit, ROBO_VLAN_PAGE,
    	      ROBO_VLAN_READ,
    	      (uint8 *)&entry, sizeof(entry));
        /* only save valid entries */
        pentry = &entry;
        if (pentry->valid)
        {
            table[tableIndex].vlan_entry.VLANgroup = pentry->VLANgroup;
            table[tableIndex].vlan_entry.VLANuntag = pentry->VLANuntag;
            table[tableIndex].vlan_addr = i;
            table[tableIndex].vlan_index = 0;
            table[tableIndex].vlan_id = i;
            tableIndex++;
        }
  	}
#endif
bcm_exit:
    *numEntries = tableIndex;
    bcm_rel_sema();
    return retval;
}

/*
 * clear all entries of ARL table
 */
int
bcm_l2_addr_cleartable(int unit)
{
    ROBO_MEM_ACCESS_CTRL_STRUC tableAccessReg;
    ROBO_ARL_TABLE_DATA_STRUC tableEntry;
    int count, i;
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    /* write zero to all table entries */
    memset(&tableEntry,0,sizeof(tableEntry));
    for (i=0;i<NUM_ARL_TABLE_ENTRIES;i++)
    {
        tableAccessReg.memAddr = i + ARL_TABLE_ADDR;
        tableAccessReg.readEn = MEM_TABLE_WRITE;
        tableAccessReg.startDone = MEM_TABLE_ACCESS_START;
        ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_DATA,
    	      (uint8 *)&tableEntry, sizeof(ROBO_ARL_TABLE_DATA_STRUC));
        ROBO_WREG(unit, ROBO_MEM_ACCESS_PAGE,
    	      ROBO_MEM_ACCESS_CTRL,
    	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
        /* wait for complete */
        for (count = 0;count<BCM_TIMEOUT_VAL;count++)
        {
             ROBO_RREG(unit, ROBO_MEM_ACCESS_PAGE,
         	      ROBO_MEM_ACCESS_CTRL,
         	      (uint8 *)&tableAccessReg, sizeof(tableAccessReg));
         	if (MEM_TABLE_ACCESS_DONE == tableAccessReg.startDone)
         	    break;
        };
        if (count >= BCM_TIMEOUT_VAL)
        {
           retval = BCM_RET_TIMEOUT;
           goto bcm_exit;
        }
  }
bcm_exit:
    bcm_rel_sema();
    return retval;
}

/*
 * Initialize a bcm_l2_addr_t to a specified MAC address and VLAN,
 * zeroing all other fields.
 */
void
bcms5_l2_addr_init(bcm_l2_addr_t *l2addr,
		 bcm_mac_t mac_addr, bcm_vlan_t vid)
{
    memset(l2addr,0,sizeof(bcm_l2_addr_t));
    memcpy(l2addr->mac,mac_addr,sizeof(bcm_mac_t));
    l2addr->vid = vid;
}

/*
 * Add address to L2 table
 */
int
bcms5_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    uint16 vid0,vid1;
    ROBO_ARL_ENTRY_STRUC entry0, entry1;
    ROBO_ARL_ENTRY_MCAST_STRUC entry0_mcast, entry1_mcast;
    ROBO_ARL_RW_CTRL_STRUC control;
    bcm_mac_t mac_addr_wf;
    bcm_vlan_t vid_wf;
    int count, tableNum = -1;
    struct
    {
        ROBO_ARL_ENTRY_STRUC *pentry;
        ROBO_ARL_ENTRY_MCAST_STRUC *pentry_mcast;
        uint8 *pvid;
        uint eaddr;
        uint vaddr;
    } tableEntry[2] = {{&entry0,&entry0_mcast,(uint8 *)&vid0,ROBO_ARL_ENTRY0,
                        ROBO_ARL_VID_ENTRY0},
                       {&entry1,&entry1_mcast,(uint8 *)&vid1,ROBO_ARL_ENTRY1,
                        ROBO_ARL_VID_ENTRY1}};
    int retval = BCM_RET_SUCCESS;

    /* in order to add to table, must first read both bin entries and */
    /* find the indicated one.  here are the cases: */
    /* bin 0    bin 1    action */
    /* -----    -----    ------ */
    /* tgt      d/c      overwrite bin 0 entry */
    /* valid    tgt      overwrite bin 1 entry */
    /* invalid  d/c      use bin 0 entry */
    /* valid    invalid  use bin 1 entry */
    /* valid    valid    fail */

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    vid_wf = l2addr->vid;
    memcpy(&mac_addr_wf[0],&l2addr->mac[0],sizeof(bcm_mac_t));
#ifdef BCM5380
    /* for 5380, convert from canonical to wire format */
    bcm_bitswap(&mac_addr_wf);
    vid_wf = bcm_bitswap_vlan(l2addr->vid);
#else
    bcm_byteswap(&mac_addr_wf);
#endif
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_MAC_ADDR_IDX,
	      (uint8 *)&(mac_addr_wf), sizeof(bcm_mac_t));
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_VID_TABLE_IDX,
	      (uint8 *)&(vid_wf), ARL_VID_BYTES);
    control.ARLrw = ARL_TABLE_READ;
    control.ARLStart = 1;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_RW_CTRL,
	      (uint8 *)&control, sizeof(control));
  	/* wait for complete */
  	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
  	{
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_RW_CTRL,
      	      (uint8 *)&control, sizeof(control));
      	if (!control.ARLStart)
      	    break;
  	};
  	if (count >= BCM_TIMEOUT_VAL)
  	{
  	    retval = BCM_RET_TIMEOUT;
  	    goto bcm_exit;
  	}
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY0,
	      (uint8 *)&entry0, sizeof(entry0));
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY1,
	      (uint8 *)&entry1, sizeof(entry1));
    vid0 = 0;
    vid1 = 0;
  	if (entry0.ctrl.valid)
  	{
  	    /* this entry if valid, check to see if this is the vid & MAC */
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_VID_ENTRY0,
      	      (uint8 *)&vid0, ARL_VID_BYTES);
      	if (vid0 == (vid_wf & VLAN_ID_MASK)
      	        && !memcmp(entry0.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
      	    tableNum = 0;
      }
      else
          tableNum = 0;
      if (entry1.ctrl.valid)
      {
  	    /* this entry if valid, check to see if this is the vid & MAC */
        ROBO_RREG(unit, ROBO_ARLIO_PAGE, ROBO_ARL_VID_ENTRY1,
    	      (uint8 *)&vid1, ARL_VID_BYTES);
      	if (vid1 == (vid_wf & VLAN_ID_MASK)
      	        && !memcmp(entry1.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
          if (tableNum == -1)
              tableNum = 1;
      }
      else
          if (tableNum == -1)
              tableNum = 1;
    /* if no entry found, fail */
    if (tableNum == -1)
    {
        retval = BCM_RET_ARL_TABLE_FULL;
        goto bcm_exit;
    }
    /* fill in selected entry and write */
    /* fill in table differently for unicast and multicast */
    if (l2addr->pbmp)
    {
      /* this is multicast */
      tableEntry[tableNum].pentry_mcast->ctrl.portMask = l2addr->pbmp;
      tableEntry[tableNum].pentry_mcast->ctrl.staticEn = 1;
      tableEntry[tableNum].pentry_mcast->ctrl.valid = 1;
      tableEntry[tableNum].pentry_mcast->ctrl.prio = 0;
      tableEntry[tableNum].pentry_mcast->ctrl.gigPort = 0;
      memcpy(tableEntry[tableNum].pentry_mcast->mac.macBytes,
              mac_addr_wf,sizeof(bcm_mac_t));
      ROBO_WREG(unit, ROBO_ARLIO_PAGE, tableEntry[tableNum].eaddr,
  	      (uint8 *)(tableEntry[tableNum].pentry_mcast),
          sizeof(ROBO_ARL_ENTRY_MCAST_STRUC));
    } else {
      tableEntry[tableNum].pentry->ctrl.portID = l2addr->port;
      tableEntry[tableNum].pentry->ctrl.chipID = l2addr->unit;
      tableEntry[tableNum].pentry->ctrl.staticEn = 1;
      tableEntry[tableNum].pentry->ctrl.valid = 1;
      memcpy(tableEntry[tableNum].pentry->mac.macBytes,
              mac_addr_wf,sizeof(bcm_mac_t));
      ROBO_WREG(unit, ROBO_ARLIO_PAGE,
  	      tableEntry[tableNum].eaddr,
  	      (uint8 *)(tableEntry[tableNum].pentry), sizeof(ROBO_ARL_ENTRY_STRUC));
    }
	  *(tableEntry[tableNum].pvid) = (uint8)vid_wf;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      tableEntry[tableNum].vaddr,
	      (uint8 *)(tableEntry[tableNum].pvid), ARL_VID_BYTES);
    control.ARLrw = ARL_TABLE_WRITE;
    control.ARLStart = 1;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_RW_CTRL,
	      (uint8 *)&control, sizeof(control));
  	/* wait for complete */
  	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
  	{
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_RW_CTRL,
      	      (uint8 *)&control, sizeof(control));
      	if (!control.ARLStart)
      	    break;
  	};
  	if (count >= BCM_TIMEOUT_VAL)
  	    retval = BCM_RET_TIMEOUT;

bcm_exit:
    bcm_rel_sema();
    return retval;
}

int
bcms5_l2_addr_remove(int unit,
		   bcm_mac_t mac_addr, bcm_vlan_t vid)
{
    uint8 vid0,vid1;
    ROBO_ARL_ENTRY_STRUC entry0, entry1;
    ROBO_ARL_RW_CTRL_STRUC control;
    int count;
    bin_stat_t bin0 = BIN_INVALID, bin1 = BIN_INVALID;
    int retval = BCM_RET_SUCCESS;
    bcm_mac_t mac_addr_wf;
    bcm_vlan_t vid_wf;

    /* in order to do remove from table, must first read both bin entries and */
    /* find the indicated one.  here are the cases: */
    /* bin 0    bin 1    action */
    /* -----    -----    ------ */
    /* tgt      invalid  mark bin 0 invalid */
    /* tgt      valid    copy bin 1 to bin 0 and mark bin 1 invalid */
    /* valid    tgt      mark bin 1 invalid */

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    vid_wf = vid;
    memcpy(&mac_addr_wf[0],&mac_addr[0],sizeof(bcm_mac_t));
#ifdef BCM5380
    /* for 5380, convert from canonical to wire format */
    bcm_bitswap(&mac_addr_wf);
    vid_wf = bcm_bitswap_vlan(vid);
#endif
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_MAC_ADDR_IDX,
	      (uint8 *)&mac_addr_wf, sizeof(bcm_mac_t));
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_VID_TABLE_IDX,
	      (uint8 *)&vid_wf, 2*sizeof(uint8));
    control.ARLrw = ARL_TABLE_READ;
    control.ARLStart = 1;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_RW_CTRL,
	      (uint8 *)&control, sizeof(control));
  	/* wait for complete */
  	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
  	{
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_RW_CTRL,
      	      (uint8 *)&control, sizeof(control));
      	if (!control.ARLStart)
      	    break;
  	};
  	if (count >= BCM_TIMEOUT_VAL)
  	{
  	    retval = BCM_RET_TIMEOUT;
  	    goto bcm_exit;
  	}
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY0,
	      (uint8 *)&entry0, sizeof(entry0));
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY1,
	      (uint8 *)&entry1, sizeof(entry1));
  	if (entry0.ctrl.valid)
  	{
  	    /* this entry if valid, check to see if this is the vid & MAC */
        ROBO_RREG(unit, ROBO_ARLIO_PAGE, ROBO_ARL_VID_ENTRY0,
    	      (uint8 *)&vid0, sizeof(vid0));
      	if (vid0 == (vid_wf & VLAN_ID_MASK)
      	        && !memcmp(entry0.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
      	    bin0 = BIN_TGT;
      	else
      	    bin0 = BIN_VALID;
    }
    if (entry1.ctrl.valid)
    {
	    /* this entry if valid, check to see if this is the vid & MAC */
        ROBO_RREG(unit, ROBO_ARLIO_PAGE,
    	      ROBO_ARL_VID_ENTRY1,
    	      (uint8 *)&vid1, sizeof(vid1));
    	if (vid1 == (vid_wf & VLAN_ID_MASK)
    	        && !memcmp(entry1.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
    	    bin1 = BIN_TGT;
    	else
    	    bin1 = BIN_VALID;
    }
    /* no determine case and do indicated action */
    if (bin0 == BIN_TGT && bin1 == BIN_INVALID)
        entry0.ctrl.valid = 0;
    else if (bin0 == BIN_TGT && bin1 == BIN_VALID)
    {
        entry0 = entry1;
        entry1.ctrl.valid = 0;
    }
    else if (bin0 == BIN_VALID && bin1 == BIN_TGT)
        entry1.ctrl.valid = 0;
    /* now write back */
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY0,
	      (uint8 *)&entry0, sizeof(entry0));
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY1,
	      (uint8 *)&entry1, sizeof(entry1));
    control.ARLrw = ARL_TABLE_WRITE;
    control.ARLStart = 1;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_RW_CTRL,
	      (uint8 *)&control, sizeof(control));
  	/* wait for complete */
  	for (count = 0;count<BCM_TIMEOUT_VAL;count++)
  	{
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_RW_CTRL,
      	      (uint8 *)&control, sizeof(control));
      	if (!control.ARLStart)
      	    break;
  	};
  	if (count >= BCM_TIMEOUT_VAL)
  	    retval = BCM_RET_TIMEOUT;
bcm_exit:
    bcm_rel_sema();
    return retval;
}

int
bcms5_l2_addr_remove_by_port(int unit,
			   bcm_pbmp_t pbmp, uint32 flags)
{
    return BCM_RET_SUCCESS;
}

int
bcms5_l2_addr_remove_by_vlan(int unit,
			   bcm_vlan_t vid, uint32 flags)
{
    return 0;
}

/*
 * Look up address in L2 table
 */
int
bcms5_l2_addr_get(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
		bcm_l2_addr_t *l2addr)
{
    uint8 vid0,vid1;
    bcm_mac_t mac_addr_wf;
    bcm_vlan_t vid_wf;
    ROBO_ARL_ENTRY_STRUC entry0, entry1;
    ROBO_ARL_RW_CTRL_STRUC control;
    int count, tableNum = -1;
    struct
    {
        ROBO_ARL_ENTRY_STRUC *pentry;
        uint8 *pvid;
        uint eaddr;
        uint vaddr;
    } tableEntry[2] = {{&entry0,&vid0,ROBO_ARL_ENTRY0,ROBO_ARL_VID_ENTRY0},
                       {&entry1,&vid1,ROBO_ARL_ENTRY1,ROBO_ARL_VID_ENTRY1}};
    int retval = BCM_RET_SUCCESS;
    ROBO_VLAN_CTRL0_STRUC vlan_ctrl;
    int vlan_enabled;

    /* in order to find entry in table, must first read both bin entries and */
    /* find the indicated one */
    /* bin 0    bin 1    action */
    /* -----    -----    ------ */
    /* tgt      d/c      get bin 0 entry */
    /* valid    tgt      get bin 1 entry */
    /* invalid  d/c      fail */
    /* valid    invalid  fail */
    /* valid    valid    fail */

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    /* determine if vlan enabled */
    ROBO_RREG(unit, ROBO_VLAN_PAGE,
	      ROBO_VLAN_CTRL0,
	      (uint8 *)&vlan_ctrl, sizeof(ROBO_VLAN_CTRL0_STRUC));
    vlan_enabled = vlan_ctrl.VLANen;
    
    vid_wf = vid;
    memcpy(&mac_addr_wf[0],&mac_addr[0],sizeof(bcm_mac_t));
#ifdef BCM5380
    /* for 5380, convert from canonical to wire format */
    bcm_bitswap(&mac_addr_wf);
    vid_wf = bcm_bitswap_vlan(vid);
#endif
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_MAC_ADDR_IDX,
	      (uint8 *)&mac_addr_wf, sizeof(bcm_mac_t));
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_VID_TABLE_IDX,
	      (uint8 *)&vid_wf, 2*sizeof(uint8));
    control.ARLrw = ARL_TABLE_READ;
    control.ARLStart = 1;
    ROBO_WREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_RW_CTRL,
	      (uint8 *)&control, sizeof(control));
    /* wait for complete */
    for (count = 0;count<BCM_TIMEOUT_VAL;count++)
    {
          ROBO_RREG(unit, ROBO_ARLIO_PAGE,
      	      ROBO_ARL_RW_CTRL,
      	      (uint8 *)&control, sizeof(control));
      	if (!control.ARLStart)
      	    break;
    };
    if (count >= BCM_TIMEOUT_VAL)
    {
        retval = BCM_RET_TIMEOUT;
        goto bcm_exit;
    }
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY0,
	      (uint8 *)&entry0, sizeof(entry0));
    ROBO_RREG(unit, ROBO_ARLIO_PAGE,
	      ROBO_ARL_ENTRY1,
	      (uint8 *)&entry1, sizeof(entry1));
    if (entry0.ctrl.valid)
    {
      /* this entry if valid, check to see if this is the vid & MAC */
      ROBO_RREG(unit, ROBO_ARLIO_PAGE, ROBO_ARL_VID_ENTRY0,
    	      (uint8 *)&vid0, sizeof(vid0));
      if ((!vlan_enabled || vid0 == (vid_wf & VLAN_ID_MASK))
          && !memcmp(entry0.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
        tableNum = 0;
    }
    if (entry1.ctrl.valid)
    {
	    /* this entry if valid, check to see if this is the vid & MAC */
      ROBO_RREG(unit, ROBO_ARLIO_PAGE, ROBO_ARL_VID_ENTRY1,
    	      (uint8 *)&vid1, sizeof(vid1));
    	if ((!vlan_enabled || vid1 == (vid_wf & VLAN_ID_MASK))
          && !memcmp(entry1.mac.macBytes,mac_addr_wf,sizeof(bcm_mac_t)))
        if (tableNum == -1)
          tableNum = 1;
    }
    /* if no entry found, fail */
    if (tableNum == -1)
    {
        retval = BCM_RET_FAIL;
        goto bcm_exit;
    }
    /* get return values for selected entry */
    l2addr->port = tableEntry[tableNum].pentry->ctrl.portID;
    l2addr->unit = tableEntry[tableNum].pentry->ctrl.chipID;
    l2addr->pbmp = ((ROBO_ARL_TABLE_MCAST_DATA_STRUC *)
        tableEntry[tableNum].pentry)->portMask;
    memcpy(&l2addr->mac, &mac_addr, sizeof(bcm_mac_t));
    memcpy(&l2addr->vid, &vid, sizeof(bcm_vlan_t));
bcm_exit:
    bcm_rel_sema();
    return retval;
}

#if 0
int
bcm_mirror_mode(int unit, int mode)
{
    return 0;
}

int
bcm_mirror_to_set(int unit, bcm_port_t port)
{
    return 0;
}

int
bcm_mirror_to_get(int unit, bcm_port_t *port)
{
    return 0;
}

int
bcm_mirror_ingress_set(int unit, bcm_port_t port, int val)
{
    return 0;
}

int
bcm_mirror_ingress_get(int unit, bcm_port_t port, int *val)
{
    return 0;
}

int
bcm_mirror_egress_set(int unit, bcm_port_t port, int val)
{
    return 0;
}

int
bcm_mirror_egress_get(int unit, bcm_port_t port, int *val)
{
    return 0;
}

int
bcm_mirror_to_pbmp_set(int unit, bcm_port_t port, bcm_pbmp_t pbmp)
{
    return 0;
}

int
bcm_mirror_to_pbmp_get(int unit, bcm_port_t port, bcm_pbmp_t  *pbmp)
{
    return 0;
}
#endif

#if 0 /* remove for vxworks */
/* set port attributes */
int
bcm_set_port_attributes(PORT_ATTRIBS *port_attribs, uint portid)
{
    bcm_an_advert_t an_advert;

    memset(&an_advert,0,sizeof(an_advert));

    /* always set pause */
    an_advert.pause = 1;     

    /* turn on power and enable, if not forced off or down */
    if (port_attribs->force != FORCE_DOWN)
    	bcm_port_enable_set(portid, 1);

    if (port_attribs->force != POWER_OFF)
    	bcm_port_poweroff_set(portid, 0);

    switch (port_attribs->force){
    case FORCE_OFF:
	if (port_attribs->autoneg){
	    an_advert.T10BaseT = 1;
	    an_advert.T10BaseTFull = 1;
	    an_advert.T100BaseX = 1;
	    an_advert.T100BaseXFull = 1;
	    bcm_port_advert_set(portid, &an_advert);
	} else {
	    /* reset defaults */
	    bcm_port_duplex_set(portid, ROBO_HALF_DUPLEX);
	    bcm_port_speed_set(portid, 100);
	}
	break;
    case FORCE_10H:
	if (port_attribs->autoneg){
	    an_advert.T10BaseT = 1;
	    bcm_port_advert_set(portid, &an_advert);
	} else {
	    bcm_port_duplex_set(portid, ROBO_HALF_DUPLEX);
	    bcm_port_speed_set(portid, 10);
	}
	break;
    case FORCE_10F:
	if (port_attribs->autoneg){
	    an_advert.T10BaseT = 1;
	    an_advert.T10BaseTFull = 1;
	    bcm_port_advert_set(portid, &an_advert);
	} else {
	    bcm_port_duplex_set(portid, ROBO_FULL_DUPLEX);
	    bcm_port_speed_set(portid, 10);
	}
	break;
    case FORCE_100H:
	if (port_attribs->autoneg){
	    an_advert.T10BaseT = 1;
	    an_advert.T10BaseTFull = 1;
	    an_advert.T100BaseX = 1;
	    bcm_port_advert_set(portid, &an_advert);
	} else {
	    bcm_port_duplex_set(portid, ROBO_HALF_DUPLEX);
	    bcm_port_speed_set(portid, 100);
	}
	break;
    case FORCE_100F:
	if (port_attribs->autoneg){
	    an_advert.T10BaseT = 1;
	    an_advert.T10BaseTFull = 1;
	    an_advert.T100BaseX = 1;
	    an_advert.T100BaseXFull = 1;
	    bcm_port_advert_set(portid, &an_advert);
	} else {
	    bcm_port_duplex_set(portid, ROBO_FULL_DUPLEX);
	    bcm_port_speed_set(portid, 100);
	}
	break;
    case FORCE_DOWN:
	bcm_port_enable_set(portid, 0);
	break;
    case POWER_OFF:
	bcm_port_poweroff_set(portid, 1);
	break;
    }
    if (port_attribs->autoneg){
	/* set autoneg */
	bcm_port_autoneg_set(portid, 1);
	/* restart autoneg */
	bcm_port_restart_autoneg(portid);
    } else
	bcm_port_autoneg_set(portid, 0);

    return 0;
}
#endif /* remove for vxworks */



