/*
 * $Id: l3_priv.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l3_priv.h
 * Purpose:     Forward definitions only used by l3.c
 *
 */

#ifndef _BCM_INT_SBX_CALADAN3_L3_PRIV_H_
#define _BCM_INT_SBX_CALADAN3_L3_PRIV_H_

#include <bcm_int/sbx/caladan3/l3.h>


#if 0
STATIC int
_bcm_caladan3_l3_intf_alloc_l2_ete(_caladan3_l3_fe_instance_t *l3_fe,
                                uint32                 *reserve_etes,
                                _caladan3_l3_intf_t        *l3_intf);
#endif

STATIC int
_bcm_caladan3_l3_update_intf(_caladan3_l3_fe_instance_t *l3_fe,
                           bcm_l3_intf_t          *bcm_intf,
                           _caladan3_l3_intf_t        *l3_intf);

STATIC int
_bcm_caladan3_l3_add_intf(_caladan3_l3_fe_instance_t *l3_fe,
                        bcm_l3_intf_t          *bcm_intf,
                        _caladan3_l3_intf_t        *l3_intf);

STATIC int
_bcm_caladan3_l3_alloc_intf(_caladan3_l3_fe_instance_t *l3_fe,
                          bcm_l3_intf_t          *bcm_intf,
                          uint32                 *reserve_etes,
                          _caladan3_l3_intf_t        **ret_intf);

STATIC int
_bcm_caladan3_link_fte2ete(_caladan3_l3_ete_t         *l3_ete,
                         _caladan3_l3_ete_fte_t     *module_fte);

STATIC int
_bcm_caladan3_unlink_fte2ete(_caladan3_l3_ete_t         *l3_ete,
                           _caladan3_l3_ete_fte_t     *module_fte);

STATIC int
_bcm_caladan3_free_l3_ete(_caladan3_l3_fe_instance_t *l3_fe,
                        _caladan3_l3_ete_t         **p_l3_sw_ete);

STATIC int
_bcm_caladan3_destroy_fte(_caladan3_l3_fe_instance_t *l3_fe,
                        int                     action,
                        uint32                  fte_idx,
                        bcm_module_t            module_id,
                        uint32                  ohi);

STATIC int
_bcm_caladan3_l3_mpath_get(_caladan3_l3_fe_instance_t *l3_fe,
                         uint32                 mpbase,
                         _caladan3_egr_mp_info_t    **ret_info);

int
_bcm_caladan3_l3_egress_dmac_update(_caladan3_l3_fe_instance_t *l3_fe,
                                  bcm_l3_egress_t        *bcm_egr);



#endif /* _BCM_INT_SBX_CALADAN3_L3_PRIV_H_ */
