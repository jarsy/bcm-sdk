/*
 * $Id: init.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     init.h
 * Purpose:
 *
 */
#ifndef _BCM_INT_EA_INIT_H
#define _BCM_INT_EA_INIT_H
#include <soc/drv.h>
#include <soc/ea/tk371x/init.h>
#include <soc/ea/tk371x/onu.h>
#include <bcm/types.h>
#include <bcm/init.h>

#define _BCM_EA_INIT_DEBUG		0

#define BCM_TK371X_UNIT_VALID(unit)		((unit) >=0) && ((unit) < 7)

#define _BCM_EA_TK371X_MODULE	"tk371x"
#define _BCM_EA_PORT_MODULE		"port"
#define _BCM_EA_L2_MODULE		"l2"
#define _BCM_EA_STATS_MODULE	"stat"
#define _BCM_EA_COSQ_MODULE		"cosq"
#define _BCM_EA_FP_MODULE		"field"

#define BCM_EA_INIT_MODULE(_mod, _part) \
		extern int bcm_##_mod##_part##_init(int unit);

extern int bcm_tk371x_port_init(int unit);
extern int bcm_tk371x_l2_init(int unit);
extern int bcm_tk371x_stat_init(int unit);
extern int bcm_tk371x_cosq_init(int unit);
extern int bcm_tk371x_cosq_detach(int unit);
extern int bcm_tk371x_field_init(int unit);
extern int _bcm_ea_clear(int unit);
extern int _bcm_ea_info_get(int unit, bcm_info_t *info);
extern void _bcm_ea_info_t_init(bcm_info_t *info);
extern int _bcm_ea_init(int unit);
extern int _bcm_ea_init_check(int unit);



#endif /* _BCM_INT_EA_INIT_H */
