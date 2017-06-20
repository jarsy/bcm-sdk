/*
 * $Id: trunk.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains TRUNK definitions internal to the BCM library.
 */

#ifndef _BCM_INT_TRUNK_H
#define _BCM_INT_TRUNK_H

typedef struct trunk_private_s {
    int     tid;            /* trunk group ID */
    int     in_use;
    int     psc;            /* port spec criterion (aka rtag) */
    int	    mc_index_spec;  /* requested by user */
    int	    mc_index_used;  /* actually used */
    int	    mc_port_used;   /* actually used or -1 */
} trunk_private_t;

#define TRUNK_CTRL_ADDR         soc_reg_addr(unit, TRUNK_TRUNK_CTRLr, 0, 0)
#define TRUNK_GROUP_ADDR(grp)   soc_reg_addr(unit, TRUNK_TRNK_GROUPr, 0, grp)

#define _BCM_TRUNK_PSC_VALID_VAL  0xf
#define _BCM_TRUNK_PSC_ORABLE_VALID_VAL  \
    (BCM_TRUNK_PSC_IPMACSA | BCM_TRUNK_PSC_IPMACDA | \
     BCM_TRUNK_PSC_IPSA | BCM_TRUNK_PSC_IPDA | \
     BCM_TRUNK_PSC_MACSA | BCM_TRUNK_PSC_MACDA | \
     BCM_TRUNK_PSC_VID)

void bcm5324_trunk_patch_linkscan(int unit, soc_port_t port, bcm_port_info_t *info);
int _bcm_robo_trunk_id_validate(int unit, bcm_trunk_t tid);
int _bcm_robo_trunk_gport_resolve
    (int unit, int member_count, bcm_trunk_member_t *member_array);

#endif  /* !_BCM_INT_TRUNK_H */
