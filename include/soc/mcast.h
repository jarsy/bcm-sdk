/*
 * $Id: mcast.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mcast.h
 * Purpose:     
 */
#ifndef   _SOC_ROBO_MCAST_H_
#define   _SOC_ROBO_MCAST_H_

#include <soc/types.h>

typedef struct {
    int		l2mc_size;
    int	*l2mc_used;
} _soc_robo_mcast_t;

/* Soc layer device-independent L2 multicast address. */
typedef struct soc_mcast_addr_s {
    uint8 mac[6];                      /* 802.3 MAC address. */
    vlan_id_t vid;                     /* VLAN identifier. */
    soc_cos_t cos_dst;                  /* COS based on destination address. */
    soc_pbmp_t pbmp;                    /* Port bitmap. */
    soc_pbmp_t ubmp;                    /* Untagged port bitmap. */
    uint32 l2mc_index;                  /* L2MC index. */
    uint32 flags;                       /* See BCM_MCAST_XXX flag definitions. */
    int distribution_class;             /* Fabric Distribution Class. */
} soc_mcast_addr_t;

extern _soc_robo_mcast_t    robo_l2mc_info[SOC_MAX_NUM_DEVICES];

#define L2MC_INFO(unit)		(&robo_l2mc_info[unit])
#define	L2MC_SIZE(unit)		L2MC_INFO(unit)->l2mc_size
#define	L2MC_USED(unit)		L2MC_INFO(unit)->l2mc_used
#define L2MC_USED_SET(unit, n)	L2MC_USED(unit)[n] += 1
#define L2MC_USED_CLR(unit, n)	L2MC_USED(unit)[n] -= 1
#define L2MC_USED_ISSET(unit, n) (L2MC_USED(unit)[n] > 0)

#define L2MC_INIT(unit) \
	if (L2MC_USED(unit) == NULL) { return BCM_E_INIT; }
#define L2MC_ID(unit, id) \
	if (id < 0 || id >= L2MC_SIZE(unit)) { return BCM_E_PARAM; }


#endif	/* !_SOC_ROBO_MCAST_H_ */

