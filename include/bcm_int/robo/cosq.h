/*
 * $Id: cosq.h,v 1.3 Broadcom SDK 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_COSQ_H
#define _BCM_INT_COSQ_H

#include <bcm/types.h>

#define _BCM_COS_DEFAULT(unit) \
        ((SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) ? BCM_COS_COUNT: BCM_COS_DEFAULT)

#define _BCM_PRIO_MAX(unit) \
        (SOC_IS_TBX(unit) ? 15: BCM_PRIO_MAX)
                                          
#define _BCM_COSQ_PRIO_VALID(unit, prio) \
        (SOC_IS_TBX(unit) ? ((prio) >= 0 && (prio < 16)) : \
                           ((prio) >= 0 && (prio < 8)))

extern int bcm_robo_5380_cosq_config_set(int unit, 
                uint32 hq_pbmp, uint8 hweight, uint8 lweight);
extern int bcm_robo_5380_cosq_config_get(int unit, 
                uint32 *hq_pbmp, uint8 *weight);

extern int _bcm_robo_cosq_gport_resolve(int unit, bcm_gport_t gport,
                bcm_port_t *port);
#endif
