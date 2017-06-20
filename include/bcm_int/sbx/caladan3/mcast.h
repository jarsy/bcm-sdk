/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_CALADAN3_MCAST_H_
#define _BCM_INT_SBX_CALADAN3_MCAST_H_

/*
 *  These are the spec_flags for the function below (yes, it's a bitmap).
 *
 *  ...AUTO_L2MCIDX = add function chooses l2mc_index
 *  ...SPEC_L2MCIDX = add function tries to use provided l2mc_index
 *  ...INCLIDE_ARG = add function includes all_routers group ports
 *  ...INHIBIT_ARG = add function does not include all_routers group ports
 */
#define BCM_CALADAN3_MCAST_ADD_AUTO_L2MCIDX    0x00000000
#define BCM_CALADAN3_MCAST_ADD_SPEC_L2MCIDX    0x00000001
#define BCM_CALADAN3_MCAST_ADD_INCLUDE_ARG     0x00000000
#define BCM_CALADAN3_MCAST_ADD_INHIBIT_ARG     0x00000002
extern int
_bcm_caladan3_mcast_addr_add(int unit,
                             bcm_mcast_addr_t *mcAddr,
                             uint32 flags,
                             uint32 spec_flags,
                             uint32 *new_ftIndex,
                             uint32 *new_mcGroup);

#endif  /* _BCM_INT_SBX_CALADAN3_MCAST_H_ */
