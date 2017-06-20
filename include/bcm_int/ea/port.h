/*
 * $Id: port.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     port.h
 * Purpose:
 *
 */
#ifndef _BCM_INT_EA_PORT_H
#define _BCM_INT_EA_PORT_H

#include <bcm/types.h>
#include <bcm/port.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/event.h>
#include <soc/ea/tk371x/CtcMiscApi.h>
#include <soc/ea/tk371x/TkOnuApi.h>
#include <soc/ea/tk371x/TkTmApi.h>

#define _BCM_EA_PORT_DEBUG		0
#define _BCM_EA_PORT_SUPPORTED	0

extern int _bcm_ea_port_detach(int unit);
extern int _bcm_tk371x_port_selective_get(
		int unit, bcm_port_t port, bcm_port_info_t *info);
extern int _bcm_ea_port_autoneg_get(
	    int unit, bcm_port_t port, int *autoneg);
extern int _bcm_ea_port_autoneg_set(
	    int unit, bcm_port_t port, int autoneg);
extern int _bcm_ea_port_config_get(
	    int unit,bcm_port_config_t *config);
extern int _bcm_ea_port_control_get(
	    int unit, bcm_port_t port, bcm_port_control_t type, int *value);
extern int _bcm_ea_port_control_set(
	    int unit, bcm_port_t port, bcm_port_control_t type, int value);
extern int _bcm_ea_port_duplex_get(
	    int unit, bcm_port_t port, int *duplex);
extern int  _bcm_ea_port_duplex_set(
	    int unit, bcm_port_t port, int duplex);
extern int _bcm_ea_port_enable_get(
	    int unit, bcm_port_t port, int *enable);
extern int _bcm_ea_port_enable_set(
	    int unit, bcm_port_t port, int enable);
extern int _bcm_ea_port_frame_max_get(
	    int unit, bcm_port_t port, int *size);
extern int _bcm_ea_port_frame_max_set(
	    int unit, bcm_port_t port, int size);
extern int _bcm_ea_port_info_get(
	    int unit, bcm_port_t port, bcm_port_info_t *info);
extern int _bcm_ea_port_info_restore(
	    int unit, bcm_port_t port, bcm_port_info_t *info);
extern int _bcm_ea_port_info_save(
	    int unit, bcm_port_t port, bcm_port_info_t *info);
/* Get or set multiple port characteristics. */
extern int _bcm_ea_port_info_set(
	    int unit, bcm_port_t port, bcm_port_info_t *info);
extern int _bcm_ea_port_init(int units);
extern int _bcm_ea_port_learn_get(
		int unit, bcm_port_t port, uint32 *flags);
extern int _bcm_ea_port_learn_modify(
	    int unit, bcm_port_t port, uint32 add, uint32 remove);
extern int _bcm_ea_port_learn_set(
	    int unit, bcm_port_t port, uint32 flags);
extern int _bcm_ea_port_link_status_get(
	    int unit, bcm_port_t port, int *status);
extern int _bcm_ea_port_phy_control_get(
	    int unit, bcm_port_t port, bcm_port_phy_control_t type, uint32 *value);
extern int _bcm_ea_port_phy_control_set(
	    int unit, bcm_port_t port, bcm_port_phy_control_t type, uint32 value);
extern int _bcm_ea_port_speed_get(
	    int unit, bcm_port_t port, int *speed);
extern int _bcm_ea_port_speed_set(
	    int unit, bcm_port_t port, int speed);
extern int _bcm_ea_port_link_status_get(
		int unit,
		bcm_port_t port, int *up);
extern int _bcm_ea_port_advert_get(
		int unit, bcm_port_t port, bcm_port_abil_t *ability_mask);
extern int _bcm_ea_port_advert_remote_get(
		int unit, bcm_port_t port, bcm_port_abil_t *ability_mask);

#endif /* _BCM_INT_EA_PORT_H */
